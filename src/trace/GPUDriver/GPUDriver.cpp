/**************************************************************************
 *
 * Copyright (c) 2002 - 2011 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 * $RCSfile: GPUDriver.cpp,v $
 * $Revision: 1.53 $
 * $Author: vmoya $
 * $Date: 2008-04-23 20:00:53 $
 *
 * GPU Driver implementation.
 *
 */

#include "GPUDriver.h"
#include "GPUTypes.h"
#include "ShaderOptimization.h"
#include "PixelMapper.h"

#include "GlobalProfiler.h"

#include <cstdio>
#include <cmath>
#include <iostream>
#include <sstream>
#include <cstring>

using namespace std;
using namespace gpu3d;

/**
 * Define these flags to enable debug of AGPTransactions
 *
 * - The 1st flag enables the dump of all transactions returned by nextAGPTransaction() method
 *   in "Driver_nextAGPTransactions.txt" file
 *
 * - The 2nd flag enables the dump of all transactions received by the internal method sendAGPTransaction
 *   in "Driver_SentAGPTransactions.txt" file.
 *
 * - The 3rd flag disables the driver WriteBuffer cache -> Set its operation mode to WriteBuffer::Inmediate
 *
 * - The 4th flag disables the dump of the transaction sequence number
 */
//#define ENABLE_DUMP_NEXTAGPTRANSACTION_TO_FILE
//#define ENABLE_DUMP_SENDAGPTRANSTION_TO_FILE
//#define DISABLE_WRITEBUFFER_CACHE

#define DISABLE_DUMP_TRANSACTION_COUNTERS

#ifdef ENABLE_DUMP_NEXTAGPTRANSACTION_TO_FILE
    #define DUMP_NEXTAGPTRANSACTION_TO_FILE(code) { code }
#else
    #define DUMP_NEXTAGPTRANSACTION_TO_FILE(code) {}
#endif

#ifdef ENABLE_DUMP_SENDAGPTRANSTION_TO_FILE
    #define DUMP_SENDAGPTRANSTION_TO_FILE(code) { code }
#else
    #define DUMP_SENDAGPTRANSTION_TO_FILE(code) {}
#endif

#ifdef DISABLE_DUMP_TRANSACTION_COUNTERS
    #define DUMP_TRANSACTION_COUNTERS(code) {}
#else
    #define DUMP_TRANSACTION_COUNTERS(code) { code }
#endif

const s32bit GPUDriver::outputAttrib[MAX_VERTEX_ATTRIBUTES] =
{
    0, // VS_VERTEX
    VS_NOOUTPUT, // VS_WEIGHT
    VS_NOOUTPUT, // VS_NORMAL
    1, // VS_COLOR
    2, // VS_SEC_COLOR
    3, // VS_FOG
    VS_NOOUTPUT, // VS_SEC_WEIGHT
    VS_NOOUTPUT, // VS_THIRD_WEIGHT
    4, // VS_TEXTURE_0
    5, // VS_TEXTURE_1
    6, // VS_TEXTURE_2
    7, // VS_TEXTURE_3
    8, // VS_TEXTURE_4
    9, // VS_TEXTURE_5
    10, // VS_TEXTURE_6
    11 // VS_TEXTURE_7
};


#define _DRIVER_STATISTICS

#define MSG(msg) printf( "%s\n", msg )

#define INC_MOD(a) a = GPU_MOD(a + 1, agpBufferSize)

#define CHECK_BIT(bits,bit) ((((1<<(bit))&(bits))!=0)?true:false)

#define CHECK_MEMORY_ACCESS(md, offset, dataSize)\
((((offset)+(dataSize))>((md)->lastAddress-(md)->firstAddress+1))?false:true)

#define UPDATE_HIGH_ADDRESS_WRITTEN(memDesc,offset,dataSize)\
if ( (memDesc)->highAddressWritten < (memDesc)->firstAddress+(offset)+(dataSize)-1 ) {\
    (memDesc)->highAddressWritten = (memDesc)->firstAddress+(offset)+(dataSize)-1;\
}

u8bit GPUDriver::_mortonTable[256];
#define max(a,b)\
    (a>b?a:b)

//const s32bit GPUDriver::outputAttrib[MAX_VERTEX_ATTRIBUTES];

/**
 * This version:
 *    - controls AGPBuffer underrun ( informs when AGPTransaction is lost )
 *    - controls access to AGPBuffer
 *    - controls 'in' var for AGPBuffer access
 *    - controls AGPcount
 */
bool GPUDriver::_sendAGPTransaction( AGPTransaction* agpt )
{
    // Remember to select register buffer write policy to RegisterBuffer::Inmediate to
    // see _sendAGPTransactions instantly when writing registers
    // NEW: This can be done defining DISABLE_WRITEBUFFER_CACHE
    DUMP_SENDAGPTRANSTION_TO_FILE(
        static ofstream f;
        static u32bit tCounter = 0;
        if ( !f.is_open() )
            f.open("Driver_SentAGPTransactions.txt");

        DUMP_TRANSACTION_COUNTERS( f << tCounter++ << ": ";)

        agpt->dump(f);
        f.flush();
    )

    #ifdef _DRIVER_STATISTICS
        agpTransactionsGenerated++;
    #endif

    if ( agpCount >= agpBufferSize )
        panic("GPUDriver", "_sendAGPTransaction", "Buffer overflow in AGPBuffer");

    agpBuffer[in] = agpt;
    INC_MOD(in);
    agpCount++;

    return true;
}


/**
 * Singleton GPUDriver object
 */
GPUDriver* GPUDriver::driver = NULL;


GPUDriver::GPUDriver() : agpCount(0), in(0), out(0), nextMemId(1), setGPUParametersCalled(false),
setResolutionCalled(false), hRes(0), vRes(0), lastBlock(0),
gpuAllocBlocks(0), systemAllocBlocks(0),
// statistics
agpTransactionsGenerated(0), memoryAllocations(0), memoryDeallocations(0), mdSearches(0),
addressSearches(0), memPreloads(0), memWrites(0), memPreloadBytes(0), memWriteBytes(0), ctx(0), preloadMemory(false), 
#ifdef DISABLE_WRITEBUFFER_CACHE
    registerWriteBuffer(this, RegisterWriteBuffer::Inmediate),
#else
    registerWriteBuffer(this, RegisterWriteBuffer::WaitUntilFlush),
#endif
//vshSched(this, 512, ShaderProgramSched::VERTEX),
//fshSched(this, 512, ShaderProgramSched::FRAGMENT)
shSched(this, 4096), frame(0), batch(0)
{
    //GPU_DEBUG( cout << "Creating Driver object" << endl; )

    /*
     * @note: Code moved to setGPUParameters
     * map = new u8bit[physicalCardMemory / 4]; // 1 byte per 4 Kbytes
     * memset( map, 0, physicalCardMemory / 4 ); // all memory available
     */
    agpBuffer = new AGPTransaction*[agpBufferSize];

    registerWriteBuffer.initAllRegisterStatus();

    memoryDescriptors.clear();

    MortonTableBuilder();
}

GPUDriver::~GPUDriver()
{
    //GPU_DEBUG( cout << "Destroying Driver object" << endl; )
    delete[] agpBuffer;
    delete[] gpuMap;
    delete[] systemMap;

    map<u32bit, _MemoryDescriptor*>::iterator it;
    it = memoryDescriptors.begin();
    while (it != memoryDescriptors.end())
    {
        delete it->second;
        it++;
    }

    memoryDescriptors.clear();

    //_MemoryDescriptor* md;
    //for ( md = mdList; md != NULL; md = md->next )
    //{
    //    delete md;
    //}
}


u32bit GPUDriver::getFetchRate() const
{
    if ( !setGPUParametersCalled )
        panic("GPUDriver", "getFetchRate", "setGPUParameters was not called");

    return fetchRate;
}

void GPUDriver::setGPUParameters(u32bit gpuMemSz, u32bit sysMemSz, u32bit blocksz_, u32bit sblocksz_, u32bit scanWidth_,
    u32bit scanHeight_, u32bit overScanWidth_, u32bit overScanHeight_,
    bool doubleBuffer_, bool forceMSAA_, u32bit msaaSamples, bool forceFP16CB_, u32bit fetchRate_,
    bool memoryControllerV2_, bool secondInterleavingEnabled_,
    bool convertToLDA, bool convertToSOA, bool enableTransformations, bool microTrisAsFrags
    )
{
    if ( !setGPUParametersCalled )
    {
        gpuMemory = gpuMemSz;
        systemMemory = sysMemSz;
        blocksz = blocksz_;
        sblocksz = sblocksz_;
        scanWidth = scanWidth_;
        scanHeight = scanHeight_;
        overScanWidth = overScanWidth_;
        overScanHeight = overScanHeight_;
        doubleBuffer = doubleBuffer_;
        fetchRate = fetchRate_;
        forceMSAA = forceMSAA_;
        forcedMSAASamples = msaaSamples;
        forceFP16ColorBuffer = forceFP16CB_;
        secondInterleavingEnabled = memoryControllerV2_ && secondInterleavingEnabled_;
        convertShaderProgramToLDA = convertToLDA;
        convertShaderProgramToSOA = convertToSOA;
        enableShaderProgramTransformations = enableTransformations;
        microTrianglesAsFragments = microTrisAsFrags;

        /*  Allocate GPU memory map.  */
        gpuMap = new u8bit[gpuMemory / BLOCK_SIZE]; // 1 byte per 4 Kbytes
        memset( gpuMap, 0, gpuMemory / BLOCK_SIZE ); // all memory available

        /*  Allocate system memory map.  */
        systemMap = new u8bit[systemMemory / BLOCK_SIZE]; // 1 byte per 4 Kbytes
        memset( systemMap, 0, systemMemory / BLOCK_SIZE ); // all memory available

        setGPUParametersCalled = true;
    }
    else
        panic("GPUDriver", "setGPUParameters", "This method must be called once");

}


void GPUDriver::getGPUParameters(u32bit& gpuMemSz, u32bit &sysMemSz, u32bit& blocksz_, u32bit& sblocksz_,
    u32bit& scanWidth_, u32bit& scanHeight_, u32bit& overScanWidth_, u32bit& overScanHeight_,
    bool& doubleBuffer_, u32bit& fetchRate_) const
{
    if ( !setGPUParametersCalled )
        panic("GPUDriver", "getGPUParameters", "setGPUParameters was not called");

    gpuMemSz = gpuMemory;
    sysMemSz = systemMemory;
    blocksz_ = blocksz;
    sblocksz_ = sblocksz;
    scanWidth_ = scanWidth;
    scanHeight_ = scanHeight;
    overScanWidth_ = overScanWidth;
    overScanHeight_ = overScanHeight;
    doubleBuffer_ = doubleBuffer;
    fetchRate_ = fetchRate;
}

/*  Get the texture tiling parameters.  */
void GPUDriver::getTextureTilingParameters(u32bit &blockSz, u32bit &sBlockSz) const
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")

    blockSz = blocksz;
    sBlockSz = sblocksz;

    GLOBALPROFILER_EXITREGION()

}


void GPUDriver::getFrameBufferParameters(bool &multisampling, u32bit &samples, bool &fp16color)
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")

    multisampling = forceMSAA;
    samples = forcedMSAASamples;
    fp16color = forceFP16ColorBuffer;

    GLOBALPROFILER_EXITREGION()
}

void GPUDriver::initBuffers(u32bit* mdCWFront_, u32bit* mdCWBack_, u32bit* mdZS_)
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")

    if ( !isResolutionDefined() )
        panic("GPUDriver", "initBuffers", "Resolution have not been set");

    if ( !setGPUParametersCalled )
        panic("GPUDriver", "initBuffers", "setGPUParameters must be called before calling this method");

    u32bit cwBufSz;
    u32bit zsBufSz;

    u32bit bytesPerPixelCW;
    u32bit bytesPerPixelZST;

    if (!forceFP16ColorBuffer)
        bytesPerPixelCW = 4;
    else
        bytesPerPixelCW = 8;

    bytesPerPixelZST = 4;

    //  Initialize the pixel mapper used to compute the size of the render buffer.
    //
    //  NOTE:  The render buffer size shouldn't be directly affected by the dimensions
    //  of the stamp tile or the generation tile.  However the generation tile size must
    //  be at least 4x4 due to limitations of the current implementation.  For this reason
    //  the stamp tile is force to 1x1 and generation tile to 4x4 even if those aren't
    //  the real values used.  The size of the scan tile is adjusted to match the selected
    //  sizes of the stamp and generation tiles.
    //
    //  WARNING: Check correctness of the current implementation.
    //

    PixelMapper cwPixelMapper;
    PixelMapper zstPixelMapper;
    u32bit samples = forceMSAA ? forcedMSAASamples : 1;
    cwPixelMapper.setupDisplay(hRes, vRes, 1, 1, 4, 4, scanWidth / 4, scanHeight / 4, overScanWidth, overScanHeight, samples, bytesPerPixelCW);
    zstPixelMapper.setupDisplay(hRes, vRes, 1, 1, 4, 4, scanWidth / 4, scanHeight / 4, overScanWidth, overScanHeight, samples, bytesPerPixelZST);

    //  Check if forced multisampling is enabled.
    if (!forceMSAA)
    {
        cwBufSz = cwPixelMapper.computeFrameBufferSize();
        zsBufSz = zstPixelMapper.computeFrameBufferSize();
    }
    else
    {
        cout << "GPUDriver ==> Forcing MSAA to " << forcedMSAASamples << "X" << endl;

        cwBufSz = cwPixelMapper.computeFrameBufferSize();
        zsBufSz = zstPixelMapper.computeFrameBufferSize();
    }



    u32bit mdCWFront = obtainMemory(cwBufSz);

    u32bit mdCWBack = mdCWFront;

    /*  Check if double buffer is enabled. */
    if (doubleBuffer)
    {
        /*  Get a color buffer for the back buffer.  */
        mdCWBack = obtainMemory(cwBufSz);
    }

    u32bit mdZS = obtainMemory(zsBufSz);

    writeGPUAddrRegister(GPU_FRONTBUFFER_ADDR, 0, mdCWFront);
    writeGPUAddrRegister(GPU_BACKBUFFER_ADDR, 0, mdCWBack);
    writeGPUAddrRegister(GPU_ZSTENCILBUFFER_ADDR, 0, mdZS);

    //  Check if forced multisampling is enabled.
    if (forceMSAA)
    {
        GPURegData data;

        //  Enable MSAA in the GPU.
        data.booleanVal = true;
        writeGPURegister(GPU_MULTISAMPLING, data);

        //  Set samples per pixel for MSAA.
        data.uintVal = forcedMSAASamples;
        writeGPURegister(GPU_MSAA_SAMPLES, data);
    }

    if (forceFP16ColorBuffer)
    {
        GPURegData data;

        cout << "GPUDriver ==> Forcing Color Buffer format to RGBA16F." << endl;

        //  Change format of the color buffer to RGBA16F.
        data.txFormat = GPU_RGBA16F;
        writeGPURegister(GPU_COLOR_BUFFER_FORMAT, data);
    }

    // secondInterleavingEnabled == using memoryControllerV2 AND secondInterleaving is enabled
    if ( secondInterleavingEnabled )
    {
        // obtain start address of the second interleaving
        mdZS  = obtainMemory(1);
        // write the start address of the second interleaving
        writeGPUAddrRegister(GPU_MCV2_2ND_INTERLEAVING_START_ADDR, 0, mdZS);
        // deallocate the descriptor used to compute memory start address
        releaseMemory(mdZS);

        // MANDATORY FLUSH !!!
        // force a flush in the RegisterWriteBuffer object to update the register start
        // address of the second interleaving if not, preaload memory transactions
        // of previous skipped frames are, very likely, written in incorrect memory
        // addresses (mapping is based in the contents of the
        // GPU_MCV2_2ND_INTERLEAVING_START_ADDR)
        registerWriteBuffer.flush();
    }

    // if requested, assign memory descriptors
    if(mdCWFront_ != 0) *mdCWFront_ = mdCWFront;
    if(mdCWBack_ != 0) *mdCWBack_ = mdCWBack;
    if(mdZS_ != 0) *mdZS_ = mdZS;

    GLOBALPROFILER_EXITREGION()

}

//  Allocates space in GPU memory for a render buffer with the defined characteristics and returns a memory descriptor.
u32bit GPUDriver::createRenderBuffer(u32bit width, u32bit height, bool multisampling, u32bit samples, TextureFormat format)
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")

    //  Determine the bytes per pixel based on the requested format.
    u32bit bytesPerPixel;
    switch(format)
    {
        case GPU_RGBA8888:
        case GPU_RG16F:
        case GPU_DEPTH_COMPONENT24:
        case GPU_R32F:

            bytesPerPixel = 4;
            break;

        case GPU_RGBA16:
        case GPU_RGBA16F:

            bytesPerPixel = 8;
            break;

        default:

            panic("GPUDriver", "createRenderBuffer", "Unsupported render buffer format.");
            break;
    }

    //  Determine the samples per pixel based on the requested MSAA mode.
    u32bit samplesPerPixel = multisampling ? samples : 1;

    //  Initialize the pixel mapper used to compute the size of the render buffer.
    //
    //  NOTE:  The render buffer size shouldn't be directly affected by the dimensions
    //  of the stamp tile or the generation tile.  However the generation tile size must
    //  be at least 4x4 due to limitations of the current implementation.  For this reason
    //  the stamp tile is force to 1x1 and generation tile to 4x4 even if those aren't
    //  the real values used.  The size of the scan tile is adjusted to match the selected
    //  sizes of the stamp and generation tiles.
    //
    //  WARNING: Check correctness of the current implementation.
    //

    PixelMapper pixelMapper;
    pixelMapper.setupDisplay(width, height, 1, 1, 4, 4, scanWidth / 4, scanHeight / 4, overScanWidth, overScanHeight,
                             samplesPerPixel, bytesPerPixel);

    //  Obtain the size of the render buffer in bytes.
    u32bit renderBufferSize = pixelMapper.computeFrameBufferSize();

    //  Allocate memory and obtain a memory descriptor for the requested render buffer.
    u32bit mdRenderBuffer = obtainMemory(renderBufferSize);

    GLOBALPROFILER_EXITREGION()

    //  Return the memory descriptor of the requested render buffer.
    return mdRenderBuffer;
}

void GPUDriver::tileRenderBufferData(u8bit *sourceData, u32bit width, u32bit height, bool multisampling, u32bit samples,
                                     gpu3d::TextureFormat format, bool invertColors, u8bit* &destData, u32bit &renderBufferSize)
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")

    //  Determine the bytes per pixel based on the requested format.
    u32bit bytesPerPixel;
    switch(format)
    {
        case GPU_RGBA8888:
        case GPU_RG16F:
        case GPU_DEPTH_COMPONENT24:
        case GPU_R32F:

            bytesPerPixel = 4;
            break;

        case GPU_RGBA16:
        case GPU_RGBA16F:

            bytesPerPixel = 8;
            break;

        default:

            panic("GPUDriver", "tileRenderBufferData", "Unsupported render buffer format.");
            break;
    }

    if (multisampling)
        panic("GPUDriver", "tileRenderBufferData", "MSAA render buffers not supported.");

    //  Determine the samples per pixel based on the requested MSAA mode.
    u32bit samplesPerPixel = multisampling ? samples : 1;

    //  Initialize the pixel mapper used to compute the size of the render buffer.
    //
    //  NOTE:  The render buffer size shouldn't be directly affected by the dimensions
    //  of the stamp tile or the generation tile.  However the generation tile size must
    //  be at least 4x4 due to limitations of the current implementation.  For this reason
    //  the stamp tile is force to 1x1 and generation tile to 4x4 even if those aren't
    //  the real values used.  The size of the scan tile is adjusted to match the selected
    //  sizes of the stamp and generation tiles.
    //
    //  WARNING: Check correctness of the current implementation.
    //

    PixelMapper pixelMapper;
    pixelMapper.setupDisplay(width, height, 2, 2, 4, 4, scanWidth / 8, scanHeight / 8, overScanWidth, overScanHeight,
                             samplesPerPixel, bytesPerPixel);

    //  Obtain the size of the render buffer in bytes.
    renderBufferSize = pixelMapper.computeFrameBufferSize();

    //  Allocate the array for the tiled render buffer data.
    destData = new u8bit[renderBufferSize];

    for(u32bit y = 0; y < height; y++)
    {
        for(u32bit x = 0; x < width; x++)
        {
            // Compute address in the input surface.
            u32bit sourceAddress = (y * width + x) * bytesPerPixel;

            // Compute address in the tiled render buffer.
            u32bit destAddress = pixelMapper.computeAddress(x, y);

            switch(bytesPerPixel)
            {
                case 4:

                    if (invertColors)
                    {
                        destData[destAddress + 0] = sourceData[sourceAddress + 2];
                        destData[destAddress + 1] = sourceData[sourceAddress + 1];
                        destData[destAddress + 2] = sourceData[sourceAddress + 0];
                        destData[destAddress + 3] = sourceData[sourceAddress + 3];
                    }
                    else
                    {
                        *((u32bit *) &destData[destAddress]) = *((u32bit *) &sourceData[sourceAddress]);
                    }
                    break;

                case 8:

                    if (invertColors)
                    {
                        *((u16bit *) &destData[destAddress + 0]) = *((u16bit *) &sourceData[sourceAddress + 2]);
                        *((u16bit *) &destData[destAddress + 1]) = *((u16bit *) &sourceData[sourceAddress + 1]);
                        *((u16bit *) &destData[destAddress + 2]) = *((u16bit *) &sourceData[sourceAddress + 0]);
                        *((u16bit *) &destData[destAddress + 3]) = *((u16bit *) &sourceData[sourceAddress + 3]);
                    }
                    else
                    {                    
                        *((u64bit *) &destData[destAddress]) = *((u64bit *) &sourceData[sourceAddress]);
                    }
                    break;

                default:

                    panic("GPUDriver", "tileRenderBufferData", "Unsupported pixel byte size.");
                    break;
            }
        }
    }

    GLOBALPROFILER_EXITREGION()
}


void GPUDriver::writeGPURegister( GPURegister regId, GPURegData data, u32bit md )
{
    registerWriteBuffer.writeRegister(regId, 0, data, md);
    //_sendAGPTransaction(new AGPTransaction( regId, 0, data ));
}

void GPUDriver::writeGPURegister( GPURegister regId, u32bit index, GPURegData data, u32bit md )
{
    registerWriteBuffer.writeRegister(regId, index, data);
    //_sendAGPTransaction(new AGPTransaction( regId, index, data ));
}

void GPUDriver::readGPURegister(gpu3d::GPURegister regID, u32bit index, gpu3d::GPURegData &data)
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")

    registerWriteBuffer.readRegister(regID, index, data);

    GLOBALPROFILER_EXITREGION()
}

void GPUDriver::readGPURegister(gpu3d::GPURegister regID, gpu3d::GPURegData &data)
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")

    registerWriteBuffer.readRegister(regID, 0, data);

    GLOBALPROFILER_EXITREGION()
}

void GPUDriver::writeGPUAddrRegister( GPURegister regId, u32bit index, u32bit md, u32bit offset )
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")
    _MemoryDescriptor* mdesc = _findMD(md);
    if ( mdesc == NULL )
    {
        cout << "regID = " << regId << "  | index = " << index << " | md = " << md << " | offset = " << offset << endl;
        panic("GPUDriver", "writeGPUAddrRegister", "MemoryDescription does not exist");
    }
    GPURegData data;
    data.uintVal = mdesc->firstAddress + offset;

    if ( data.uintVal > mdesc->lastAddress )
        panic("GPUDriver", "writeGPUAddrRegister", "offset out of bounds");

    registerWriteBuffer.writeRegister(regId, index, data, md);

    GLOBALPROFILER_EXITREGION()

    //_sendAGPTransaction(new AGPTransaction(regId, index, data));
}

GPUDriver* GPUDriver::getGPUDriver()
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")

    if ( driver == NULL )
        driver = new GPUDriver;

    GLOBALPROFILER_EXITREGION()

    return driver;
}


void GPUDriver::sendCommand( GPUCommand com )
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")

    // This code moved before "if (preloadMemory)" to release pendentReleases
    if ( com == gpu3d::GPU_DRAW )
    {
        //vshSched.dump();
        //fshSched.dump();

        //vshSched.dumpStatistics();
        //fshSched.dumpStatistics();

        vector<u32bit>::iterator it = pendentReleases.begin();
        for ( ; it != pendentReleases.end(); it ++ )
            _releaseMD(*it); // do real release
        pendentReleases.clear();
        batchCounter++; // next batch
#ifdef  DUMP_SYNC_REGISTERS_TO_GPU
        dumpRegisterStatus(frame, batch);
#endif
        batch++;
    }

    if (com == gpu3d::GPU_SWAPBUFFERS)
    {
        frame++;
        batch = 0;
    }

    if ( preloadMemory )
    {
        GLOBALPROFILER_EXITREGION()
        return ;
    }

    // Before perform a command update the GPU register state machine
    registerWriteBuffer.flush();

    //  Send end of frame signal to the simulator before the swap command.
    if (com == gpu3d::GPU_SWAPBUFFERS)
    {
        signalEvent(GPU_END_OF_FRAME_EVENT, "Frame rendered.");
        frame++;
        batch = 0;
    }

    _sendAGPTransaction( new AGPTransaction( com ) );

    GLOBALPROFILER_EXITREGION()
}


void GPUDriver::sendCommand( GPUCommand* listOfCommands, int numberOfCommands )
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")

    int i;
    for ( i = 0; i < numberOfCommands; i++ )
        sendCommand(listOfCommands[i]);

    GLOBALPROFILER_EXITREGION()
}

void GPUDriver::signalEvent(GPUEvent gpuEvent, string eventMsg)
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")

    _sendAGPTransaction(new AGPTransaction(gpuEvent, eventMsg));

    GLOBALPROFILER_EXITREGION()
}

AGPTransaction* GPUDriver::nextAGPTransaction()
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")
    if ( agpCount != 0 ) {
        int oldOut = out;
        INC_MOD(out);
        agpCount--;
        if ( agpBuffer[oldOut]->getDebugInfo() != "" )
            cout << "AGPTransaction debug info: " << agpBuffer[oldOut]->getDebugInfo() << endl;

        DUMP_NEXTAGPTRANSACTION_TO_FILE(
            static ofstream f;
            static u32bit tCounter = 0;
            if ( !f.is_open() )
                f.open("DriverAGPTransactions.txt");
            DUMP_TRANSACTION_COUNTERS( f << tCounter++ << ": "; )
            agpBuffer[oldOut]->dump(f);
            f << endl;
        )

        GLOBALPROFILER_EXITREGION()

        return agpBuffer[oldOut];
    }

    GLOBALPROFILER_EXITREGION()

    return NULL;
}

bool GPUDriver::setSequentialStreamingMode( u32bit count, u32bit start )
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")
    GPURegData data;

    data.uintVal = count; /* elements in streams */
    registerWriteBuffer.writeRegister(GPU_STREAM_COUNT, 0, data);

    // disable GPU indexMode
    data.uintVal = 0;
    registerWriteBuffer.writeRegister(GPU_INDEX_MODE, 0, data);

    data.uintVal = start;
    registerWriteBuffer.writeRegister(GPU_STREAM_START,0, data);

    GLOBALPROFILER_EXITREGION()

    return true;
}



bool GPUDriver::setIndexedStreamingMode( u32bit stream, u32bit count, u32bit start,
                                         u32bit mdIndices, u32bit offsetBytes, StreamData indicesType )
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")
    if ( stream >= MAX_STREAM_BUFFERS )
    {
        panic("GPUDriver", "setIndexedStreamingMode()", "invalid vertex attribute identifier");
    }

    GPURegData data;

    // enable GPU indexMode
    data.uintVal = 1;
    registerWriteBuffer.writeRegister(GPU_INDEX_MODE, 0, data);

    // select 'stream' as the stream used for streaming indices
    data.uintVal = stream;
    registerWriteBuffer.writeRegister(GPU_INDEX_STREAM, 0, data);

    // set the address of indices in gpu local memory
    _MemoryDescriptor* mdesc = _findMD( mdIndices );
    data.uintVal = mdesc->firstAddress + offsetBytes;
    registerWriteBuffer.writeRegister(GPU_STREAM_ADDRESS, stream, data, mdIndices);

    // no stride for indices
    data.uintVal = 0;
    registerWriteBuffer.writeRegister(GPU_STREAM_STRIDE, stream, data);

    // Type of indices
    data.streamData = indicesType;
    registerWriteBuffer.writeRegister(GPU_STREAM_DATA, stream, data);

    // One element(index) contains just one component (the index value)
    data.uintVal = 1;
    registerWriteBuffer.writeRegister(GPU_STREAM_ELEMENTS, stream, data);

    // Number of elements that will be streamed to vertex shader
    data.uintVal = count;
    registerWriteBuffer.writeRegister(GPU_STREAM_COUNT, 0, data);

    // Set first index (commonly 0)
    data.uintVal = start;
    registerWriteBuffer.writeRegister(GPU_STREAM_START, 0, data);

    GLOBALPROFILER_EXITREGION()

    return false;
}

bool GPUDriver::setVShaderOutputWritten( ShAttrib attrib , bool isWritten )
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")
    panic("GPUDriver", "setVShaderOutputWritten", "Method deprecated, cannot be used");

    GPURegData data;
    data.booleanVal = isWritten;

    if ( outputAttrib[attrib] == VS_NOOUTPUT && isWritten )
    {
        char msg[256];
        sprintf(msg, "Attribute (%d) cannot be written (it is not and output in VS)", attrib);
        panic("GPUDriver", "setVShaderOutputWritten()", msg);
    }

    if ( isWritten )
        registerWriteBuffer.writeRegister( GPU_VERTEX_OUTPUT_ATTRIBUTE, outputAttrib[attrib], data );

    GLOBALPROFILER_EXITREGION()

    return true;
}

bool GPUDriver::configureVertexAttribute( const VertexAttribute& va )
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")
    u32bit stream = va.stream;
    u32bit attrib = va.attrib;

    if (attrib >= MAX_VERTEX_ATTRIBUTES )
    {
        panic("GPUDriver", "configureVertexAttribute()", "invalid vertex attribute identifier");
    }

    GPURegData data;

    if ( va.enabled )
    {
        data.uintVal = stream;

        if ( stream >= MAX_STREAM_BUFFERS )
            panic("GPUDriver","configureVertexAttribute()", "Stream out of range");

        registerWriteBuffer.writeRegister( GPU_VERTEX_ATTRIBUTE_MAP, attrib, data );

        // Set the local GPU stream buffer address for this attribute
        _MemoryDescriptor* md = _findMD( va.md );
        // compute start address in GPU local memory for this stream
        data.uintVal = md->firstAddress + va.offset;
        registerWriteBuffer.writeRegister( GPU_STREAM_ADDRESS, stream, data, va.md);

        // Set stream buffer stride for this attribute
        u32bit stride = va.stride;
        if ( stride == 0 )
        {
            stride = va.components;
            switch ( va.componentsType )
            {
                case SD_UNORM8:
                case SD_SNORM8:
                case SD_UINT8:
                case SD_SINT8:
                    // nothing to do
                    break;

                case SD_UNORM16:
                case SD_SNORM16:
                case SD_UINT16:
                case SD_SINT16:
                case SD_FLOAT16:
                    stride *= 2;
                    break;

                case SD_UNORM32:
                case SD_SNORM32:
                case SD_FLOAT32:
                case SD_UINT32:
                case SD_SINT32:
                    stride *= 4;
                    break;
                default:
                    panic("GPUDriver", "configureVertexAttribute()", "Unknown components type");
            }
        }
        // else: use specific stride
        data.uintVal = stride;
        registerWriteBuffer.writeRegister( GPU_STREAM_STRIDE, stream, data );

        // Set stream data type for this attribute
        data.streamData = va.componentsType;
        registerWriteBuffer.writeRegister( GPU_STREAM_DATA, stream, data );

        // Set number of components for this attribute
        data.uintVal = va.components;
        registerWriteBuffer.writeRegister( GPU_STREAM_ELEMENTS, stream, data );
    }
    else
    {
        // disable this attribute
        data.uintVal = ST_INACTIVE_ATTRIBUTE;
        registerWriteBuffer.writeRegister( GPU_VERTEX_ATTRIBUTE_MAP, attrib, data );
    }

    GLOBALPROFILER_EXITREGION()

    return true;
}



bool GPUDriver::commitVertexProgram( u32bit memDesc, u32bit programSize, u32bit startPC )
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")

    /*

    GPURegData data;

    //_MemoryDescriptor* md = _findMD( memDesc );
    //if ( md == NULL )
    //{
    //    panic("GPUDriver", "commitVertexProgram", "Memory descriptor not found");
    //    return false;
    //}


     // PACTH to avoid:
     //   MemoryController:issueTransaction => GPU Memory operation out of range.
    data.uintVal = 0;
    registerWriteBuffer.writeRegister( GPU_PROGRAM_MEM_ADDR, 0, data );

    writeGPUAddrRegister(GPU_VERTEX_PROGRAM, 0, memDesc);
    //data.uintVal = md->firstAddress;
    //_sendAGPTransaction( new AGPTransaction( GPU_VERTEX_PROGRAM, 0, data ) );

    data.uintVal = programSize;
    registerWriteBuffer.writeRegister( GPU_VERTEX_PROGRAM_SIZE, 0, data );

    data.uintVal = startPC;
    registerWriteBuffer.writeRegister( GPU_VERTEX_PROGRAM_PC, 0, data );

    sendCommand( GPU_LOAD_VERTEX_PROGRAM );
    */


    if ( programSize % ShaderProgramSched::InstructionSize != 0 )
    {
        stringstream ss;
        ss << "program size: " << programSize << " is not multiple of instruction size: "
           << ShaderProgramSched::InstructionSize;
        panic("GPUDriver", "commitVertexProgram", ss.str().c_str());
    }


//printf("GPUDriver::commitVertexProgram => memDesc %d programSize %d startPC = %x\n", memDesc, programSize, startPC);
//_MemoryDescriptor *mdDesc = _findMD(memDesc);
//if (mdDesc != NULL)
//printf("GPUDriver::commitVertexProgram => firstAddress %x lastAddress %x size %d\n", mdDesc->firstAddress, mdDesc->lastAddress, mdDesc->size);
//else
//printf("GPUDriver::commitVertexProgram => Memory Descriptor not found\n");

    shSched.select(memDesc, programSize / ShaderProgramSched::InstructionSize, VERTEX_TARGET);

    GLOBALPROFILER_EXITREGION()

    return true;
}


bool GPUDriver::commitFragmentProgram( u32bit memDesc, u32bit programSize, u32bit startPC )
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")

    /*

    GPURegData data;

    //_MemoryDescriptor* md = _findMD( memDesc );
    //if ( md == NULL )
    //{
    //    panic("GPUDriver", "commitFragmentProgram", "Memory descriptor not found");
    //    return false;
    //}


     //PACTH to avoid:
    //    MemoryController:issueTransaction => GPU Memory operation out of range.

    data.uintVal = 0;
    registerWriteBuffer.writeRegister( GPU_PROGRAM_MEM_ADDR, 0, data );

    writeGPUAddrRegister(GPU_FRAGMENT_PROGRAM, 0, memDesc);
    //data.uintVal = md->firstAddress;
    //_sendAGPTransaction( new AGPTransaction( GPU_FRAGMENT_PROGRAM, 0, data ) );

    data.uintVal = programSize;
    registerWriteBuffer.writeRegister( GPU_FRAGMENT_PROGRAM_SIZE, 0, data );

    data.uintVal = startPC;
    registerWriteBuffer.writeRegister( GPU_FRAGMENT_PROGRAM_PC, 0, data );

    sendCommand( GPU_LOAD_FRAGMENT_PROGRAM );
    */


    if ( programSize % ShaderProgramSched::InstructionSize != 0 )
    {
        stringstream ss;
        ss << "program size: " << programSize << " is not multiple of instruction size: "
           << ShaderProgramSched::InstructionSize;
        panic("GPUDriver", "commitFragmentProgram", ss.str().c_str());
    }

    shSched.select(memDesc, programSize / ShaderProgramSched::InstructionSize, FRAGMENT_TARGET);

    GLOBALPROFILER_EXITREGION()

    return true;
}

GPUDriver::_MemoryDescriptor* GPUDriver::_createMD( u32bit firstAddress, u32bit lastAddress, u32bit size )
{
    //  Get a new memory descriptor id.
    u32bit mdID = nextMemId++;

    //  Verify that the new MD doesn't already exist.
    map <u32bit, _MemoryDescriptor*>::iterator it;
    it = memoryDescriptors.find(mdID);

    if (it == memoryDescriptors.end())
    {
        _MemoryDescriptor *md = new _MemoryDescriptor;
        md->size = size;
        md->firstAddress = firstAddress;
        md->lastAddress = lastAddress;
        md->memId = mdID;

        // debug
        md->highAddressWritten = firstAddress;

        //  Add to the map of Memory Descriptors.
        memoryDescriptors[mdID] = md;

        return md;
    }
    else
    {
        //  Error, memory descriptor identifier already used!!
        return NULL;
    }

    /*u32bit i = nextMemId++;
    for ( i; i != nextMemId - 2 ; i++ )
    {
        _MemoryDescriptor* md = mdList;
        for ( md; md != NULL; md = md->next )
        { // check all memory id's
            if ( md->memId == i ) // occupied
                break; // ( md != NULL )
        }
        if ( md == NULL ) { // identifier available
            md = new _MemoryDescriptor;
            md->size = size;
            md->firstAddress = firstAddress;
            md->lastAddress = lastAddress;
            md->memId = i;
            md->next = mdList;
            mdList = md;
            // debug
            md->highAddressWritten = firstAddress;
            return md;
        }
    }
    return NULL; // max id's used...*/
}


GPUDriver::_MemoryDescriptor* GPUDriver::_findMD( u32bit memId )
{
    GLOBALPROFILER_ENTERREGION("_findMD", "GPUDriver", "_findMD");
    mdSearches++;

    map<u32bit, _MemoryDescriptor*>::iterator it;

    it = memoryDescriptors.find(memId);

    if (it != memoryDescriptors.end())
    {
        GLOBALPROFILER_EXITREGION()
        return it->second;
    }

    //_MemoryDescriptor* md = mdList;
    //for ( md; md != NULL; md = md->next ) {
    //    if ( md->memId == memId )
    //        return md;
    //}
    GLOBALPROFILER_EXITREGION()
    return NULL;
}

GPUDriver::_MemoryDescriptor* GPUDriver::_findMDByAddress( u32bit physicalAddress )
{
    //_MemoryDescriptor* md = mdList;
    //for ( md; md != NULL; md = md->next ) {
    //    if ( md->firstAddress <= physicalAddress && physicalAddress <= md->lastAddress )
    //        return md;
    //}

    addressSearches++;

    bool found = false;

    map<u32bit, _MemoryDescriptor*>::iterator it;
    it = memoryDescriptors.begin();

    while (!found && it != memoryDescriptors.end())
    {
        found = (it->second->firstAddress <= physicalAddress) && (physicalAddress <= it->second->lastAddress);
        if (!found)
            it++;
    }

    if (found)
        return it->second;

    return NULL;
}


/*  Release a memory descriptor and deallocate the associated memory.  */
void GPUDriver::_releaseMD( u32bit memId )
{
    // Remove memId from Shader Program Schedulers (if memId points to a Program)
    //vshSched.remove(memId);
    //fshSched.remove(memId);
    shSched.remove(memId);

    u32bit isGPUMem;
    u32bit i;
    //_MemoryDescriptor* md = mdList;
    //_MemoryDescriptor* prev = mdList;

    ///*  Search the memory descriptor.  */
    //for (; (md != NULL) && (md->memId != memId); prev = md, md = md->next );

    _MemoryDescriptor *md;

    //  Search for the memory descriptor.
    map<u32bit, _MemoryDescriptor*>::iterator it;
    it = memoryDescriptors.find(memId);
    if (it != memoryDescriptors.end())
        md = it->second;
    else
        md = NULL;

    /*  Check if the memory descriptor was found.  */
    if ((md != NULL) && (md->memId == memId))
    {
        /*  Update statistics.  */
#ifdef _DRIVER_STATISTICS
            memoryDeallocations++;
#endif

        /*  Determine the address space for the memory descriptor.  */
        switch(md->firstAddress & ADDRESS_SPACE_MASK)
        {
            case GPU_ADDRESS_SPACE:

                /*  Deallocate blocks.  */
                for ( i = (md->firstAddress & SPACE_ADDRESS_MASK) / (BLOCK_SIZE*1024);
                    i <= (md->lastAddress & SPACE_ADDRESS_MASK) / (BLOCK_SIZE*1024); i++ )
                    gpuMap[i] = 0;

                /*  Update number of allocated blocks in GPU memory.  */
                gpuAllocBlocks -= ((md->lastAddress & SPACE_ADDRESS_MASK) / (BLOCK_SIZE*1024)) -
                    ((md->firstAddress & SPACE_ADDRESS_MASK) / (BLOCK_SIZE*1024)) + 1;

                break;

            case SYSTEM_ADDRESS_SPACE:

                /*  Deallocate blocks.  */
                for ( i = (md->firstAddress & SPACE_ADDRESS_MASK) / (BLOCK_SIZE*1024);
                    i <= (md->lastAddress & SPACE_ADDRESS_MASK) / (BLOCK_SIZE*1024); i++ )
                    systemMap[i] = 0;

                /*  Update number of allocated blocks system GPU memory.  */
                systemAllocBlocks -= ((md->lastAddress & SPACE_ADDRESS_MASK) / (BLOCK_SIZE*1024)) -
                    ((md->firstAddress & SPACE_ADDRESS_MASK) / (BLOCK_SIZE*1024)) + 1;

                break;

            default:
                panic("GPUDriver", "_releaseMD", "Unsupported GPU address space.");
                break;
        }

        //  Delete and remove memory descriptor from the map.
        memoryDescriptors.erase(it);
        delete md;

        /*  destroy _MemoryDescriptor  */
        //if ( md == mdList )
        //{
        //    mdList = md->next;
        //    delete md;
        //}
        //else
        //{ // last or in the middle ( prev exists )
        //    prev->next = md->next;
        //    delete md;
        //}
    }
    else
    {
        panic("GPUDriver", "_releaseMD", "Memory descriptor not found.");
    }
}


void GPUDriver::releaseMemory( u32bit md )
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")
    if ( md == 0 )
    {
        GLOBALPROFILER_EXITREGION()
        return ; // releasing NULL (ignored)
    }

    // Comment this code to have a exact memory footprint when using preload/normal write
    /*
    if ( preloadMemory )
    {
        // do inmediate release of memory
        _releaseMD(md);
        return ;
    }
    */

    pendentReleases.push_back(md); // deferred releasing (to support batch pipelining)
    //_releaseMD( md );
    GLOBALPROFILER_EXITREGION()
}


/*  Search a memory map for consecutive blocks for the required memory.  */
u32bit GPUDriver::searchBlocks(u32bit &first, u8bit *map, u32bit memSize, u32bit memRequired)
{
    GLOBALPROFILER_ENTERREGION("searchBlock", "GPUDriver", "searchBlocks");

    u32bit i;
    //u32bit first = lastBlock;
    u32bit accum = 0;
    u32bit blocks = 0;

    /*  Search for empty blocks until enough have been r.  */
    for (i = first = 0; (i < (memSize / BLOCK_SIZE)) && (accum < memRequired); i++ )
    {
        /*  Check if the block is already used.  */
        if (map[i] == 1)
        {
            /*  Restart search of consecutive blocks.  */
            accum = 0;
            blocks = 0;
            first = i + 1;
        }
        else
        {
            /*  Block is empty.   map[i] == 0.  */

            /*  Update consecutive empty blocks counter.  */
            accum += BLOCK_SIZE*1024;
            blocks++;
        }
    }

    /*  Check if search was successful.  */
    if (accum >= memRequired)
    {
        GLOBALPROFILER_EXITREGION()
        /*  Return the address of the first of the consecutive blocks.  */
        return blocks;
    }
    else
    {
        GLOBALPROFILER_EXITREGION()
        /*  Return no consecutive blocks available.  */
        return 0;
    }
}

u32bit GPUDriver::obtainMemory( u32bit memRequired, MemoryRequestPolicy memRequestPolicy )
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")
    bool useGPUMem;
    u32bit first;
    u32bit blocks;
    u32bit i;
    u32bit firstAddress;
    u32bit lastAddress;

    if ( !setGPUParametersCalled )
        panic("GPUDriver", "obtainMemory", "Before obtaining memory is required to call setGPUParameters");

    if ( memRequired == 0 )
        panic("GPUDriver", "obtainMemory()", "0 bytes required ??? (programming error?)");

    if ( memRequestPolicy == GPUMemoryFirst )
    {
        blocks = searchBlocks(first, gpuMap, gpuMemory, memRequired);
        if ( blocks == 0 ) // memory couldn't be allocated in GPU local memory, try with system memory
        {
            blocks = searchBlocks(first, systemMap, systemMemory, memRequired);
            useGPUMem = false;
        }
        else
            useGPUMem = true;
    }
    else // SystemMemoryFirst
    {
        blocks = searchBlocks(first, systemMap, systemMemory, memRequired);
        if ( blocks == 0 ) // memory couldn't be allocated in system memory, try with GPU local memory
        {
            blocks = searchBlocks(first, gpuMap, gpuMemory, memRequired);
            useGPUMem = true;
        }
        else
            useGPUMem = false;
    }


    /*  Check if a block was found.  */
    if (blocks > 0)
    {
        /*  Update statistics.  */
        #ifdef _DRIVER_STATISTICS
            memoryAllocations++;
        #endif

        /*  Check if reserve GPU memory.  */
        if (useGPUMem)
        {
            /*  Set the gpu memory blocks as allocated.  */
            for (i = first; i < (first + blocks); i++)
                gpuMap[i] = 1;

            /*  Update the number of allocated blocks in GPU memory.  */
            gpuAllocBlocks += blocks;
        }
        else
        {
            /*  Set the system memory blocks as allocated.  */
            for (i = first; i < (first + blocks); i++)
                systemMap[i] = 1;

            /*  Update the number of allocated blocks in system memory.  */
            systemAllocBlocks += blocks;
        }

        /*  Calculate the start address for the allocated memory.  */
        firstAddress = first * BLOCK_SIZE * 1024;

        /*  Calculate the last address for the allocated memory.  */
        lastAddress =((first + blocks) * BLOCK_SIZE * 1024) - 1;

        /*  Apply address space offsets.  */
        if (useGPUMem)
        {
            firstAddress += GPU_ADDRESS_SPACE;
            lastAddress += GPU_ADDRESS_SPACE;
        }
        else
        {
            firstAddress += SYSTEM_ADDRESS_SPACE;
            lastAddress += SYSTEM_ADDRESS_SPACE;
        }

        /*  Create the memory descriptor.  */
        _MemoryDescriptor* md = _createMD(firstAddress, lastAddress, memRequired);

        /*  Check if the memory descriptor was correctly allocated.  */
        if ( md == NULL )
        {
            GLOBALPROFILER_EXITREGION()
            panic("GPUDriver","obtainMemory()", "No memory descriptors available");
            return 0;
        }

        /*  Set next empty block search point.  */
        lastBlock = first + blocks;

        GLOBALPROFILER_EXITREGION()
        return md->memId;
    }
    else
    {
        GLOBALPROFILER_EXITREGION()
        panic("GPUDriver", "obtainMemory()", "No memory available");
        return 0;
    }

}


bool GPUDriver::writeMemory( u32bit md, const u8bit* data, u32bit dataSize, bool isLocked )
{
    return writeMemory( md, 0, data, dataSize, isLocked );
}

/*
bool GPUDriver::writeMemoryDebug(u32bit md, u32bit offset, const u8bit* data, u32bit dataSize, const std::string& debugInfo)
{
    _MemoryDescriptor* memDesc = _findMD( md );
    if ( memDesc == NULL )
        panic("GPUDriver", "writeMemoryDebug()", "Memory descriptor does not exist");
    if ( !CHECK_MEMORY_ACCESS(memDesc,offset,dataSize) )
        panic("GPUDriver", "writeMemory()", "Access memory out of range");

    UPDATE_HIGH_ADDRESS_WRITTEN(memDesc,offset,dataSize);

    AGPTransaction* agpt = new AGPTransaction( memDesc->firstAddress, dataSize,(u8bit*)data, true );
    char buf[256];
    sprintf(buf, "%d", dataSize);
    agpt->setDebugInfo(string() + debugInfo + "  Size: " + buf);
    _sendAGPTransaction(agpt);
    return true;
}

*/

bool GPUDriver::writeMemory( u32bit md, u32bit offset, const u8bit* data, u32bit dataSize, bool isLocked )
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")

    _MemoryDescriptor* memDesc = _findMD( md );
    if ( memDesc == NULL )
        panic("GPUDriver", "writeMemory()", "Memory descriptor does not exist");

    if ( !CHECK_MEMORY_ACCESS(memDesc,offset,dataSize) )
        panic("GPUDriver", "writeMemory()", "Access memory out of range");

    UPDATE_HIGH_ADDRESS_WRITTEN(memDesc,offset,dataSize);

    if ( preloadMemory )
    {
        _sendAGPTransaction( new AGPTransaction( memDesc->firstAddress + offset, dataSize,(u8bit*)data, md) );
        
        memPreloads++;
        memPreloadBytes += dataSize;
    }
    else
    {
        _sendAGPTransaction( new AGPTransaction( memDesc->firstAddress + offset, dataSize,(u8bit*)data, md, true, isLocked) );
        
        memWrites++;
        memWriteBytes += dataSize;
    }

    GLOBALPROFILER_EXITREGION()

    return true;
}

bool GPUDriver::writeMemoryPreload(u32bit md, u32bit offset, const u8bit* data, u32bit dataSize)
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")

    _MemoryDescriptor* memDesc = _findMD(md);
    
    if (memDesc == NULL)
        panic("GPUDriver", "writeMemoryPreload", "Memory descriptor does not exist");

    if (!CHECK_MEMORY_ACCESS(memDesc,offset,dataSize))
        panic("GPUDriver", "writeMemoryPreload", "Access memory out of range");

    UPDATE_HIGH_ADDRESS_WRITTEN(memDesc,offset,dataSize);

    //  Create AGP_PRELOAD transaction.
    _sendAGPTransaction(new AGPTransaction(memDesc->firstAddress + offset, dataSize, (u8bit*) data, md));

    memPreloads++;
    memPreloadBytes += dataSize;
    
    GLOBALPROFILER_EXITREGION()

    return true;
}

void GPUDriver::printMemoryUsage()
{
    printf("GPUDriver => Memory usage : GPU %d blocks | System %d blocks\n", gpuAllocBlocks,
        systemAllocBlocks);
}

void GPUDriver::dumpMemoryAllocation( bool contents )
{
    if ( !setGPUParametersCalled )
        panic("GPUDriver", "dumpMemoryAllocation", "Before tracing memory is required to call setGPUParameters");

    u32bit i;
    printf( "GPU memory mapping: \n" );
    printf( "%d Kbytes of available GPU memory\n", gpuMemory );
    printf( "%d KBytes per Block\n", BLOCK_SIZE );
    printf( "-------------------------------------\n" );
    for ( i = 0; i < gpuMemory / BLOCK_SIZE; i++ )
    {
        if ( gpuMap[i] == 0 )
            printf( "GPU Map %d : FREE\n", i );
        else {
            _MemoryDescriptor* md = _findMDByAddress( i*BLOCK_SIZE*1024 );
            printf( "Map %d : OCCUPIED --> MD:%d ", i, md->memId );
            if ( contents )
                printf( "high address written = %d\n", md->highAddressWritten );
            else
                printf( "\n" );
        }
    }
    printf( "-------------------------------------\n" );
    printf( "System memory mapping: \n" );
    printf( "%d Kbytes of available GPU memory\n", systemMemory );
    printf( "%d KBytes per Block\n", BLOCK_SIZE );
    printf( "-------------------------------------\n" );
    for ( i = 0; i <systemMemory / BLOCK_SIZE; i++ )
    {
        if ( systemMap[i] == 0 )
            printf( "System Map %d : FREE\n", i );
        else {
            _MemoryDescriptor* md = _findMDByAddress( i*BLOCK_SIZE*1024 );
            printf( "Map %d : OCCUPIED --> MD:%d ", i, md->memId );
            if ( contents )
                printf( "high address written = %d\n", md->highAddressWritten );
            else
                printf( "\n" );
        }
    }
    printf( "-------------------------------------\n" );
}



void GPUDriver::dumpMD( u32bit md )
{
    _MemoryDescriptor* memDesc = _findMD( md );
    if ( memDesc != NULL ) {
        printf( "Descriptor ID: %d\n", memDesc->memId );
        printf( "First Address: %d\n", memDesc->firstAddress );
        printf( "Last Address: %d\n", memDesc->lastAddress );
    }
    else
        printf( "Descriptor unused.\n" );
}


void GPUDriver::dumpMDs()
{
    //_MemoryDescriptor* memDesc = mdList;
    //for ( memDesc ; memDesc != NULL; memDesc = memDesc->next ) {
    //    printf( "Descriptor ID: %d\n", memDesc->memId );
    //    printf( "First Address: %d\n", memDesc->firstAddress );
    //    printf( "Last Address: %d\n", memDesc->lastAddress );
    //}

    map<u32bit, _MemoryDescriptor*>::iterator it;
    it = memoryDescriptors.begin();
    while (it != memoryDescriptors.end())
    {
        printf( "Descriptor ID: %d\n", it->second->memId );
        printf( "First Address: %d\n", it->second->firstAddress );
        printf( "Last Address: %d\n", it->second->lastAddress );
    }
}


void GPUDriver::dumpAGPBuffer()
{
    if ( agpCount == 0 ) {
        printf( "AGPBuffer is empty\n" );
        return ;
    }
    printf( "AGPBuffer contents (AGPTRansaction objects):\n" );
    int i = out;
    int c = 1;
    while ( i != in ) {
        printf( "%d:", c );
        switch ( agpBuffer[i]->getAGPCommand() ) {
            case AGP_WRITE :
                printf( "AGP_WRITE\n" );
                break;
            case AGP_READ :
                printf( "AGP_READ\n" );
                break;
            case AGP_REG_WRITE :
                printf( "AGP_REG_WRITE\n" );
                break;
            case AGP_REG_READ :
                printf( "AGP_REG_READ\n" );
                break;
            case AGP_COMMAND :
                printf( "AGP_COMMAND\n" );
                break;
        }
        c++;
        INC_MOD(i);
    }
}

void GPUDriver::dumpStatistics()
{
    printf("---\n" );
    printf("agpTransactionsGenerated : %d\n", agpTransactionsGenerated );
    printf("memoryAllocations : %d\n", memoryAllocations );
    printf("memoryDeallocations : %d\n", memoryDeallocations );
    printf("mdSearches : %d\n", mdSearches);
    printf("addressSearches : %d\n", addressSearches);
    printf("memPreloads : %d\n", memPreloads);
    printf("memPreloadBytes : %d\n", memPreloadBytes);
    printf("memWrites : %d\n", memWrites);
    printf("memWriteBytes : %d\n", memWriteBytes);
}


u32bit GPUDriver::getTextureUnits() const
{
    return MAX_TEXTURES;
}


u32bit GPUDriver::getMaxMipmaps() const
{
    return MAX_TEXTURE_SIZE;
}


void GPUDriver::setResolution(u32bit width, u32bit height)
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")
    hRes = width;
    vRes = height;
    setResolutionCalled = true;
    GPURegData data;
    data.uintVal = width;
    driver->writeGPURegister( GPU_DISPLAY_X_RES, data );
    data.uintVal = height;
    driver->writeGPURegister( GPU_DISPLAY_Y_RES, data );
    GLOBALPROFILER_EXITREGION()
}

void GPUDriver::getResolution(u32bit& width, u32bit& height) const
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")
    width = hRes;
    height = vRes;
    GLOBALPROFILER_EXITREGION()
}

bool GPUDriver::isResolutionDefined() const
{
    return setResolutionCalled;
}


void GPUDriver::VertexAttribute::dump() const
{
    std::cout << "Stream: " << stream << std::endl;
    std::cout << "Attrib ID: " << attrib << std::endl;
    std::cout << "Enabled: " << enabled << std::endl;
    std::cout << "Memory descriptor: " << md << std::endl;
    std::cout << "Offset: " << offset << std::endl;
    std::cout << "Stride: " << stride << std::endl;
    std::cout << "ComponentsType: " << componentsType << std::endl;
    std::cout << "# components: " << components << std::endl;
}

u32bit GPUDriver::getMaxVertexAttribs() const
{
    return MAX_VERTEX_ATTRIBUTES;
}

u32bit GPUDriver::getMaxAddrRegisters() const
{
    /* Information extracted from ShaderEmulator.h, line 77:  "VS2_ADDRESS_NUM_REGS = 2" */
    return 2;
}

void GPUDriver::setPreloadMemory(bool enable)
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")
    preloadMemory = enable;
    if ( !preloadMemory )
    {
        // clear ProgramShaderSched's since commands (GPU_LOAD_VERTEX_PROGRAM & GPU_LOAD_FRAGMENT_PROGRAM
        // have been ignored and their contents are wrong
        //vshSched.clear();
        //fshSched.clear();
        shSched.clear();
    }
    GLOBALPROFILER_EXITREGION()
}

bool GPUDriver::getPreloadMemory() const
{
    return preloadMemory;
}

void GPUDriver::translateShaderProgram(u8bit *inCode, u32bit inSize, u8bit *outCode, u32bit &outSize,
                                       bool isVertexProgram, u32bit &maxLiveTempRegs, MicroTriangleRasterSettings settings)
{
    GLOBALPROFILER_ENTERREGION("gpudriver", "", "")

    //  Check if shader translation is disabled.
    if (!enableShaderProgramTransformations)
    {
        //  If disabled just copy the original program.

        GPU_ASSERT(
            if (inSize > outSize)
                panic("GPUDriver", "translateShaderProgram", "Output shader program buffer size is too small.");
        )

        memcpy(outCode, inCode, inSize);
        outSize = inSize;

        GLOBALPROFILER_EXITREGION()
        return;
    }

    vector<ShaderInstruction *> inputProgram;
    vector<ShaderInstruction *> programTemp1;
    vector<ShaderInstruction *> programTemp2;
    vector<ShaderInstruction *> programOptimized;
    vector<ShaderInstruction *> programFinal;

    bool verbose = false;
    bool optimization = true;
    bool setWaitPoints = true;

    u32bit numTemps;
    bool hasJumps;
    
    ShaderOptimization::decodeProgram(inCode, inSize, inputProgram, numTemps, hasJumps);

    //  Disable optimizations if the shader has jumps (not supported yet).
    optimization = optimization && !hasJumps;
    
    if (hasJumps)
        printf("GPUDriver::translateShaderProgram => Jump instructions found in the program.  Disabling optimizations.\n");
    
    if (verbose)
    {
        printf("Starting new shader program translation.\n");
        printf(" >>> vertex program = %s | convert to LDA = %s | convert to SOA = %s\n\n",
            isVertexProgram ? "Yes" : "No", convertShaderProgramToLDA ? "Yes" : "No", convertShaderProgramToSOA ? "Yes" : "No");
        printf("Input Program : \n");
        printf("----------------------------------------\n");

        ShaderOptimization::printProgram(inputProgram);

        printf("\n");
    }

    if (convertShaderProgramToLDA && isVertexProgram)
    {
        ShaderOptimization::attribute2lda(inputProgram, programTemp1);

        if (verbose)
        {
            printf("Convert input registers to LDA : \n");
            printf("----------------------------------------\n");

            ShaderOptimization::printProgram(programTemp1);

            printf("\n");
        }
    }
    else
        ShaderOptimization::copyProgram(inputProgram, programTemp1);

    ShaderOptimization::deleteProgram(inputProgram);

    if (convertShaderProgramToSOA)
    {
        ShaderOptimization::aos2soa(programTemp1, programTemp2);
        ShaderOptimization::deleteProgram(programTemp1);
        ShaderOptimization::copyProgram(programTemp2, programTemp1);
        ShaderOptimization::deleteProgram(programTemp2);

        if (verbose)
        {
            printf("AOS to SOA conversion : \n");
            printf("----------------------------------------\n");

            ShaderOptimization::printProgram(programTemp1);

            printf("\n");
        }
    }

    if (optimization)
        ShaderOptimization::optimize(programTemp1, programOptimized, maxLiveTempRegs, false, convertShaderProgramToSOA, verbose);
    else
    {
        ShaderOptimization::copyProgram(programTemp1, programOptimized);
        maxLiveTempRegs = numTemps;
    }
    
    ShaderOptimization::deleteProgram(programTemp1);

    if (verbose)
    {
        printf("Optimized program : \n");
        printf("----------------------------------------\n");

        ShaderOptimization::printProgram(programOptimized);

        printf("\n");
    }

    if (setWaitPoints)
    {
        ShaderOptimization::assignWaitPoints(programOptimized, programFinal);

        if (verbose)
        {
            printf("Program with wait points : \n");
            printf("----------------------------------------\n");

            ShaderOptimization::printProgram(programFinal);

            printf("\n");
        }
    }
    else
    {
        ShaderOptimization::copyProgram(programOptimized, programFinal);
    }

    ShaderOptimization::deleteProgram(programOptimized);

    ShaderOptimization::encodeProgram(programFinal, outCode, outSize);

    ShaderOptimization::deleteProgram(programFinal);

    GLOBALPROFILER_EXITREGION()
}

u32bit GPUDriver::assembleShaderProgram(u8bit *program, u8bit *code, u32bit size)
{
    ShaderOptimization::assembleProgram(program, code, size);
    
    return size;
}

u32bit GPUDriver::disassembleShaderProgram(u8bit *code, u32bit codeSize, u8bit *program, u32bit size, bool &disableEarlyZ)
{
    ShaderOptimization::disassembleProgram(code, codeSize, program, size, disableEarlyZ);
    
    return size;
}

void GPUDriver::dumpRegisterStatus (int frame, int batch)
{
    registerWriteBuffer.dumpRegisterStatus(frame, batch);
}

void GPUDriver::MortonTableBuilder()
{
    for(u32bit i = 0; i < 256; i++)
    {
        u32bit t1 = i & 0x0F;
        u32bit t2 = (i >> 4) & 0x0F;
        u32bit m = 0;

        for(u32bit nextBit = 0; nextBit < 4; nextBit++)
        {
            m += ((t1 & 0x01) << (2 * nextBit)) + ((t2 & 0x01) << (2 * nextBit + 1));

            t1 = t1 >> 1;
            t2 = t2 >> 1;
        }

        _mortonTable[i] = m;
    }
}

u8bit* GPUDriver::getDataInMortonOrder( u8bit* originalData, u32bit width, u32bit height, u32bit depth, TextureCompression format, u32bit texelSize, u32bit& mortonDataSize)
{

    u8bit* mortonData;
    //  Check for compressed formats and compute the compressed block size.
    u32bit s3tcBlockSz;
    switch ( format )
    {
        case GPU_S3TC_DXT1_RGB:
        case GPU_S3TC_DXT1_RGBA:
        case GPU_LATC1:
        case GPU_LATC1_SIGNED:
            s3tcBlockSz = 8;
            break;
        case GPU_S3TC_DXT3_RGBA:
        case GPU_S3TC_DXT5_RGBA:
        case GPU_LATC2:
        case GPU_LATC2_SIGNED:
            s3tcBlockSz = 16;
            break;
        default: //GPU_NO_TEXTURE_COMPRESSION
            s3tcBlockSz = 0;
    }

    u32bit w2;

    //  Check if compressed texture.
    if (s3tcBlockSz != 0)
    {
        // Compressed texture

        //  Compute the size of the mipmap data in morton order.
        //  NOTE: The width and height of the mipmap must be clamped to 1 block (4x4).
        u32bit w2 = (u32bit) ceil(logTwo(max(width >> 2, u32bit(1))));
        mortonDataSize = s3tcBlockSz * (_texel2address(w2, blocksz - 2, sblocksz,
                                                                      max((width >> 2), u32bit(1)) - 1,
                                                                      max((height >> 2), u32bit(1)) - 1) + 1);

        //  Allocate the memory buffer for the mipmap data in morton order.
        mortonData = new u8bit[mortonDataSize];

        // Convert mipmap data to morton order.
        //  NOTE: The width and height of the mipmap must be clamped to 1 block (4x4).
        for( u32bit i = 0; i < (max(height >> 2, u32bit(1))); ++i )
        {
            for( u32bit j = 0; j < (max(width >> 2, u32bit(1))); ++j )
            {
                u32bit a = _texel2address(w2, blocksz - 2, sblocksz, j, i);
                for(u32bit k = 0; k < (s3tcBlockSz >> 2); ++k )
                    ((u32bit *) mortonData)[(a * (s3tcBlockSz >> 2)) + k] = ((u32bit *) originalData)[(i * max((width >> 2), u32bit(1)) + j) * (s3tcBlockSz >> 2) + k];
            }
        }


        return mortonData;
    }
    else
    {
        // Uncompressed texture.

        //  Compute the size of the mipmap data in morton order.
        w2 = (u32bit)ceil(logTwo(width));
        u32bit mortonDataSliceSize = texelSize*(_texel2address(w2, blocksz, sblocksz, width-1, height-1) + 1);
        mortonDataSize = mortonDataSliceSize * depth;
        //  Allocate the memory buffer for the mipmap data in morton order.
        mortonData = new u8bit[mortonDataSize];
        u8bit* tmpMortonData = mortonData;
        switch ( texelSize )
        {
            case 1:
                {
                    for ( u32bit k = 0; k < depth; ++k)
                    {
                        for ( u32bit i = 0; i < height; ++i )
                        {
                            for( u32bit j = 0; j < width; ++j )
                            {
                                u32bit a = _texel2address(w2, blocksz, sblocksz, j, i);
                                ((u8bit *) tmpMortonData)[a] = ((u8bit *) originalData)[(width * height * k) + i * width + j];
                            }
                        }
                        tmpMortonData += mortonDataSliceSize;
                    }
                }
                break;
            case 2:
                {
                    for ( u32bit k = 0; k < depth; ++k)
                    {
                        for ( u32bit i = 0; i < height; ++i )
                        {
                            for( u32bit j = 0; j < width; ++j )
                            {
                                u32bit a = _texel2address(w2, blocksz, sblocksz, j, i);
                                ((u16bit *) tmpMortonData)[a] = ((u16bit *) originalData)[(width * height * k) + i * width + j];
                            }
                        }
                        tmpMortonData += mortonDataSliceSize;
                    }
                }
                break;
            case 4:
                {
                    for ( u32bit k = 0; k < depth; ++k)
                    {
                        for( u32bit i = 0; i < height; ++i )
                        {
                            for( u32bit j = 0; j < width; ++j )
                            {
                                u32bit a = _texel2address(w2, blocksz, sblocksz, j, i);
                                ((u32bit *) tmpMortonData)[a] = ((u32bit *) originalData)[(width * height * k) + i * width + j];
                            }
                        }
                        tmpMortonData += mortonDataSliceSize;
                    }
                }
                break;
            case 8:
                {
                    for ( u32bit k = 0; k < depth; ++k)
                    {
                        for( u32bit i = 0; i < height; ++i )
                        {
                            for( u32bit j = 0; j < width; ++j )
                            {
                                u32bit a = _texel2address(w2, blocksz, sblocksz, j, i);
                                ((u64bit *) tmpMortonData)[a] = ((u64bit *) originalData)[(width * height * k) + i * width + j];
                            }
                        }
                        tmpMortonData += mortonDataSliceSize;
                    }
                }
                break;
            case 16:
                {
                    for ( u32bit k = 0; k < depth; ++k)
                    {
                        for( u32bit i = 0; i < height; ++i )
                        {
                            for( u32bit j = 0; j < width; ++j )
                            {
                                u32bit a = _texel2address(w2, blocksz, sblocksz, j, i);
                                ((u64bit *) tmpMortonData)[a * 2] = ((u64bit *) originalData)[((width * height * k) + i * width + j) * 2];
                                ((u64bit *) tmpMortonData)[a * 2 + 1] = ((u64bit *) originalData)[((width * height * k) + i * width + j) * 2 + 1];
                            }
                        }
                        tmpMortonData += mortonDataSliceSize;
                    }
                }
                break;
            default:
            {
                stringstream ss;
                ss << "Only morton transformations with texel size 1, 4, 8 or 16 bytes supported. texel size = "
                   << texelSize;
                panic("GPUDriver", "getDataInMortonOrder", ss.str().c_str());
            }
        }

        return mortonData;
    }
}

u32bit GPUDriver::_mortonFast(u32bit size, u32bit i, u32bit j)
{
    u32bit address;
    u32bit t1 = i;
    u32bit t2 = j;

    switch(size)
    {
        case 0:
            return 0;
        case 1:
            return _mortonTable[((t2 & 0x0F) << 4) | (t1 & 0x0F)] & 0x03;
        case 2:
            return _mortonTable[((t2 & 0x0F) << 4) | (t1 & 0x0F)] & 0x0F;
        case 3:
            return _mortonTable[((t2 & 0x0F) << 4) | (t1 & 0x0F)] & 0x3F;
        case 4:
            return _mortonTable[((t2 & 0x0F) << 4) | (t1 & 0x0F)];
        case 5:
            address = _mortonTable[((t2 & 0x0F) << 4) | (t1 & 0x0F)];
            address += (_mortonTable[(((t2 >> 4) & 0x0F) << 4) | ((t1 >> 4) & 0x0F)] & 0x03) << 8;
            return address;
        case 6:
            address = _mortonTable[((t2 & 0x0F) << 4) | (t1 & 0x0F)];
            address += (_mortonTable[(((t2 >> 4) & 0x0F) << 4) | ((t1 >> 4) & 0x0F)] & 0x0F) << 8;
            return address;
        case 7:
            address = _mortonTable[((t2 & 0x0F) << 4) | (t1 & 0x0F)];
            address += (_mortonTable[(((t2 >> 4) & 0x0F) << 4) | ((t1 >> 4) & 0x0F)] & 0x3F) << 8;
            return address;
        case 8:
            address = _mortonTable[((t2 & 0x0F) << 4) | (t1 & 0x0F)];
            address += _mortonTable[(((t2 >> 4) & 0x0F) << 4) | ((t1 >> 4) & 0x0F)] << 8;
            return address;
        default:
            panic("GPUDriver", "_mortonFast", "Fast morton not supported for this tile size");
            break;
    }

    return 0;
}
u32bit GPUDriver::_texel2address(u32bit width, u32bit blockSz, u32bit sBlockSz, u32bit i, u32bit j)
{
    u32bit address;
    u32bit texelAddr;
    u32bit blockAddr;
    u32bit sBlockAddr;

    /*  Calculate the address of the texel inside the block using Morton order.  */
    //texelAddr = morton(blockSz, i, j);
    texelAddr = _mortonFast(blockSz, i, j);

    /*  Calculate the address of the block inside the superblock using Morton order.  */
    //blockAddr = morton(sBlockSz, i >> blockSz, j >> blockSz);
    blockAddr = _mortonFast(sBlockSz, i >> blockSz, j >> blockSz);

    /*  Calculate thte address of the superblock inside the cache.  */
    sBlockAddr = ((j >> (sBlockSz + blockSz)) << max(s32bit(width - (sBlockSz + blockSz)), s32bit(0))) + (i >> (sBlockSz + blockSz));

    address = (((sBlockAddr << (2 * sBlockSz)) + blockAddr) << (2 * blockSz)) + texelAddr;

    return address;
}

f64bit GPUDriver::ceil(f64bit x)
{
    return (x - std::floor(x) > 0)?std::floor(x) + 1:std::floor(x);
}

f64bit GPUDriver::logTwo(f64bit x)
{
    return std::log(x)/std::log(2.0);
}

