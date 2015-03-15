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
 * GPU emulator implementation file.
 *
 */


/**
 *
 *  @file GPUEmulator.cpp
 *
 *  This file contains the implementation of functions for the ATTILA GPU emulator.
 *
 */


#include "GPUEmulator.h"
#include "GlobalProfiler.h"
#include "ImageSaver.h"
#include "ClipperEmulator.h"
#include <iostream>
#include <cstring>

using namespace std;

//#define DUMP_RT_AFTER_DRAW

namespace gpu3d
{

//  Constructor.
GPUEmulator::GPUEmulator(SimParameters simP, TraceDriverInterface *trDriver) :

    simP(simP), trDriver(trDriver), abortEmulation(false),
    cacheDXT1RGB(*this, 1024, 64, 0x003C, DXT1_SPACE_SHIFT, TextureEmulator::decompressDXT1RGB),
    cacheDXT1RGBA(*this, 1024, 64, 0x003C, DXT1_SPACE_SHIFT, TextureEmulator::decompressDXT1RGBA),
    cacheDXT3RGBA(*this, 1024, 64, 0x003C, DXT3_DXT5_SPACE_SHIFT, TextureEmulator::decompressDXT3RGBA),
    cacheDXT5RGBA(*this, 1024, 64, 0x003C, DXT3_DXT5_SPACE_SHIFT, TextureEmulator::decompressDXT5RGBA),
    cacheLATC1(*this, 1024, 64, 0x003F, LATC1_LATC2_SPACE_SHIFT, TextureEmulator::decompressLATC1),
    cacheLATC1_SIGNED(*this, 1024, 64, 0x003F, LATC1_LATC2_SPACE_SHIFT, TextureEmulator::decompressLATC1Signed),
    cacheLATC2(*this, 1024, 64, 0x003E, LATC1_LATC2_SPACE_SHIFT, TextureEmulator::decompressLATC2),
    cacheLATC2_SIGNED(*this, 1024, 64, 0x003E, LATC1_LATC2_SPACE_SHIFT, TextureEmulator::decompressLATC2Signed)
    
{
printf("GPUEmulator => Creating rasterizer emulator.\n");

    //  Create and initialize a rasterizer emulator for Rasterizer.
    rastEmu = new RasterizerEmulator(
        1,                              /*  Active triangles.  */
        MAX_FRAGMENT_ATTRIBUTES,        /*  Attributes per fragment.  */
        simP.ras.scanWidth,             /*  Scan tile width in fragments.  */
        simP.ras.scanHeight,            /*  Scan tile height in fragments.  */
        simP.ras.overScanWidth,         /*  Over scan tile width in scan tiles.  */
        simP.ras.overScanHeight,        /*  Over scan tile height in scan tiles.  */
        simP.ras.genWidth,              /*  Generation tile width in fragments.  */
        simP.ras.genHeight,             /*  Generation tile width in fragments.  */
        simP.ras.useBBOptimization,     /*  Use the Bounding Box optimization pass (micropolygon rasterizer).  */
        simP.ras.subPixelPrecision      /*  Precision bits for the decimal part of subpixel operations (micropolygon rasterizer).  */
        );

    GPU_ASSERT(
        if (rastEmu == NULL)
            panic("GPUEmulator", "GPUEmulator", "Error creating rasterizer emulator object.");
    )    

printf("GPUEmulator => Creating texture emulator.\n");

    //  Create texture emulator.
    texEmu = new TextureEmulator(
        STAMP_FRAGMENTS,                /*  Fragments per stamp.  */
        simP.fsh.textBlockDim,          /*  Texture block dimension (texels): 2^n x 2^n.  */
        simP.fsh.textSBlockDim,         /*  Texture superblock dimension (blocks): 2^m x 2^m.  */
        simP.fsh.anisoAlgo,             /*  Anisotropy algorithm selected.  */
        simP.fsh.forceMaxAniso,         /*  Force the maximum anisotropy from the configuration file for all textures.  */
        simP.fsh.maxAnisotropy,         /*  Maximum anisotropy allowed for any texture.  */
        simP.fsh.triPrecision,          /*  Trilinear precision.  */
        simP.fsh.briThreshold,          /*  Brilinear threshold.  */
        simP.fsh.anisoRoundPrec,        /*  Aniso ratio rounding precision.  */
        simP.fsh.anisoRoundThres,       /*  Aniso ratio rounding threshold.  */
        simP.fsh.anisoRatioMultOf2,     /*  Aniso ratio must be multiple of two.  */
        simP.ras.overScanWidth,         /*  Over scan tile width (scan tiles).  */
        simP.ras.overScanHeight,        /*  Over scan tile height (scan tiles).  */
        simP.ras.scanWidth,             /*  Scan tile width (pixels).  */
        simP.ras.scanHeight,            /*  Scan tile height (pixels).  */
        simP.ras.genWidth,              /*  Generation tile width (pixels).  */
        simP.ras.genHeight              /*  Generation tile height (pixels).  */
        );

    GPU_ASSERT(
        if (texEmu == NULL)
            panic("GPUEmulator", "GPUEmulator", "Error creating texture emulator object.");
    )    

printf("GPUEmulator => Creating shader emulator.\n");

    //  Create and initialize shader emulator.
    shEmu = new ShaderEmulator(
        "ShaderEmu",                                    //  Shader name.
        UNIFIED,                                        //  Shader model.
        STAMP_FRAGMENTS,                                //  Threads supported by the shader.
        true,                                           //  Store decoded instructions.
        texEmu,                                         //  Pointer to the texture emulator attached to the shader.
        STAMP_FRAGMENTS,                                //  Fragments per stamp for texture accesses.
        simP.ras.subPixelPrecision                      //  subpixel precision for shader fixed point operations.
        );

    GPU_ASSERT(
        if (shEmu == NULL)
            panic("GPUEmulator", "GPUEmulator", "Error creating shader emulator object.");
    )    

printf("GPUEmulator => Creating ROP emulator.\n");

    //  Create fragment operations emulator.
    fragEmu = new FragmentOpEmulator(
        STAMP_FRAGMENTS                 //  Fragments per stamp.
        );

    GPU_ASSERT(
        if (fragEmu == NULL)
            panic("GPUEmulator", "GPUEmulator", "Error creating fragment operations emulator object.");
    )    

printf("GPUEmulator => Allocating memory.\n");

    //  Allocate memory.
    gpuMemory = new u8bit[simP.mem.memSize * 1024 * 1024];
    sysMemory = new u8bit[simP.mem.mappedMemSize * 1024 * 1024];

    //  Check allocation.
    GPU_ASSERT(
        if (gpuMemory == NULL)
            panic("GPUEmulator", "GPUEmulator", "Error allocating gpu memory.");
        if (sysMemory == NULL)
            panic("GPUEmulator", "GPUEmulator", "Error allocating system memory.");
    )

    //  Reset content of the gpu memory.
    for(u32bit dw = 0; dw < ((simP.mem.memSize * 1024 * 1024) >> 2); dw++)
        ((u32bit *) gpuMemory)[dw] = 0xDEADCAFE;

    //  Reset content of the system mapped memory.
    for(u32bit dw = 0; dw < ((simP.mem.mappedMemSize * 1024 * 1024) >> 2); dw++)
        ((u32bit *) sysMemory)[dw] = 0xDEADCAFE;
        
    //  Set frame counter as start frame.
    frameCounter = simP.startFrame;

    //  Reset the counter of draw calls processed.
    batchCounter = 0;

    //  Reset the validation mode flag.
    validationMode = false;

    //  Reset skip draw call mode flag.
    skipBatchMode = false;
    
    //  Initialize debug log variables.
    traceLog = false;
    traceBatch = false;
    traceVertex = false;
    tracePixel = false;
    traceVShader = false;
    traceFShader = false;
    traceTexture = false;
    watchBatch = 0;
    watchPixelX = 0;
    watchPixelY = 0;
    watchIndex = 0;
}

//
//  Implementation of the ShadedVertex container class.
//

GPUEmulator::ShadedVertex::ShadedVertex(QuadFloat *attrib)
{
    for(u32bit a = 0; a < MAX_VERTEX_ATTRIBUTES; a++)
    {
        attributes[a][0] = attrib[a][0];
        attributes[a][1] = attrib[a][1];
        attributes[a][2] = attrib[a][2];
        attributes[a][3] = attrib[a][3];
    }
}

QuadFloat *GPUEmulator::ShadedVertex::getAttributes()
{
    return attributes;
}

//
//  Implementation of the ShadedFragment container class.
//


GPUEmulator::ShadedFragment::ShadedFragment(Fragment *fr, bool _culled)
{
    fragment = fr;
    culled = _culled;
}

Fragment *GPUEmulator::ShadedFragment::getFragment()
{
    return fragment;
}


QuadFloat *GPUEmulator::ShadedFragment::getAttributes()
{
    return attributes;
}

bool GPUEmulator::ShadedFragment::isCulled()
{
    return culled;
}

void GPUEmulator::ShadedFragment::setAsCulled()
{
    culled = true;
}


//
//  Implementation of the CompressedTextureCache helper class.
//

GPUEmulator::CompressedTextureCache::CompressedTextureCache(GPUEmulator &emu, u32bit blocks, u32bit blockSize, u64bit blockMask, u32bit ratioShift,
                                                            void (*decompFunc) (u8bit *, u8bit *, u32bit)) :
    emu(emu)
{
    decompressedBlockSize = blockSize;
    maxCachedBlocks = blocks;
    compressionRatioShift = ratioShift;
    decompressedBlockMask = blockMask;
    decompressionFunction = decompFunc;

    //  Allocate space for the decompressed texture data.
    decompressedData = new u8bit[decompressedBlockSize * maxCachedBlocks];

    //  Clear compressed texture cache data.
    cachedBlocks = 0;
    blockCache.clear();
}

void GPUEmulator::CompressedTextureCache::readData(u64bit address, u8bit *data, u32bit size)
{
    //  Search block in the block cache.
    u64bit blockAddress = address & ~(decompressedBlockMask);
    map<u64bit, u8bit *>::iterator it;
    it = blockCache.find(blockAddress);

    //  Check if decompressed block is in the cache.
    if (it != blockCache.end())
    {
        //  Copy decompressed data.
        for(u32bit b = 0; b < size; b++)
            data[b] = (it->second)[(address & decompressedBlockMask) + b];
    }
    else
    {
        //  Calculate address of the compressed data in memory.
        u32bit comprAddress = u32bit((blockAddress >> compressionRatioShift) & 0xffffffff);

        //  Check if block cache is full.
        if (cachedBlocks == maxCachedBlocks)
        {
            //  Clear the cache.
            cachedBlocks = 0;
            blockCache.clear();
        }

        u8bit *memory = emu.selectMemorySpace(comprAddress);
        comprAddress = comprAddress & SPACE_ADDRESS_MASK;
        
        //  Decompress into the next free cached block.
        decompressionFunction(&memory[comprAddress], &decompressedData[cachedBlocks * decompressedBlockSize],
            decompressedBlockSize >> compressionRatioShift);

        //  Copy decompressed data.
        for(u32bit b = 0; b < size; b++)
            data[b] = decompressedData[cachedBlocks * decompressedBlockSize + (address & decompressedBlockMask) + b];

        //  Add to the block cache.
        blockCache.insert(make_pair(blockAddress, &decompressedData[cachedBlocks * decompressedBlockSize]));
        cachedBlocks++;
    }
}

void GPUEmulator::CompressedTextureCache::clear()
{
    //  Clear the cache.
    cachedBlocks = 0;
    blockCache.clear();
}

//
//
//

//  Emulates the Command Processor.
void GPUEmulator::emulateCommandProcessor(AGPTransaction *currentTransaction)
{
    char filename[1024];

    bool rtEnabled;

    //  Process the current AGP transaction.
    switch(currentTransaction->getAGPCommand())
    {
        case AGP_WRITE:
            {
                //  Get address and size of the write.
                u32bit address = currentTransaction->getAddress();
                u32bit size = currentTransaction->getSize();

                GPU_DEBUG(
                    printf("AGP_WRITE address %08x size %d\n", address, size);
                )

                //  Check memory space for the write.
                if ((address & ADDRESS_SPACE_MASK) == GPU_ADDRESS_SPACE)
                {
                    //  Get address inside the memory space.
                    address = address & SPACE_ADDRESS_MASK;

                    //  Check address overflow.
                    GPU_ASSERT(
                        if ((address + size) > (simP.mem.memSize * 1024 * 1024))
                            panic("GPUEmulator", "emulateCommandProcessor", "AGP_WRITE out of memory.");
                    )

                    //  Perform the write.
                    memcpy(&gpuMemory[address], currentTransaction->getData(), size);
                }
                else if ((address & ADDRESS_SPACE_MASK) == SYSTEM_ADDRESS_SPACE)
                {
                    //  Get address inside the memory space.
                    address = address & SPACE_ADDRESS_MASK;

                    //  Check address overflow.
                    GPU_ASSERT(
                        if ((address + size) > (simP.mem.mappedMemSize * 1024 * 1024))
                            panic("GPUEmulator", "emulateCommandProcessor", "AGP_WRITE out of memory.");
                    )

                    //  Perform the write.
                    memcpy(&sysMemory[address], currentTransaction->getData(), size);
                }
            }

            delete currentTransaction;

            break;

        case AGP_PRELOAD:
            {
                //  Get address and size of the write.
                u32bit address = currentTransaction->getAddress();
                u32bit size = currentTransaction->getSize();

                GPU_DEBUG(
                    printf("AGP_PRELOAD address %08x size %d\n", address, size);
                )

                //  Check memory space for the write.
                if ((address & ADDRESS_SPACE_MASK) == GPU_ADDRESS_SPACE)
                {
                    //  Get address inside the memory space.
                    address = address & SPACE_ADDRESS_MASK;

                    //  Check address overflow.
                    GPU_ASSERT(
                        if ((address + size) > (simP.mem.memSize * 1024 * 1024))
                            panic("GPUEmulator", "emulateCommandProcessor", "AGP_PRELOAD out of memory.");
                    )

                    //  Perform the write.
                    memcpy(&gpuMemory[address], currentTransaction->getData(), size);
                }

                if ((address & ADDRESS_SPACE_MASK) == SYSTEM_ADDRESS_SPACE)
                {
                    //  Get address inside the memory space.
                    address = address & SPACE_ADDRESS_MASK;

                    //  Check address overflow.
                    GPU_ASSERT(
                        if ((address + size) > (simP.mem.mappedMemSize * 1024 * 1024))
                            panic("GPUEmulator", "emulateCommandProcessor", "AGP_PRELOAD out of memory.");
                    )

                    //  Perform the write.
                    memcpy(&sysMemory[address], currentTransaction->getData(), size);
                }
            }

            delete currentTransaction;

            break;

        case AGP_READ:

            panic("GPUEmulator", "emulateCommandProcessor", "AGP_READ not implemented.");
            break;

        case AGP_REG_WRITE:

            processRegisterWrite(currentTransaction->getGPURegister(),
                                 currentTransaction->getGPUSubRegister(),
                                 currentTransaction->getGPURegData());

            delete currentTransaction;

            break;

        case AGP_REG_READ:

            panic("GPUEmulator", "emulateCommandProcessor", "AGP_REG_READ not implemented.");
            break;

        case AGP_COMMAND:
            switch(currentTransaction->getGPUCommand())
            {
                case GPU_RESET:

                    GPU_DEBUG(
                        printf("GPU_RESET command received.\n");
                    )

                    resetState();
                    delete currentTransaction;
                    break;

                case GPU_DRAW:

                    GPU_DEBUG(
                        printf("GPU_DRAW command received.\n");
                    )

                    //  Found command to process.
                //if (batchCounter == watchBatch)
                //{
                    draw();

#ifdef DUMP_RT_AFTER_DRAW
                    printf("Rendering frame %d batch %d trace position %d\n", frameCounter, batchCounter, trDriver->getTracePosition());
                    
                    rtEnabled = false;
                    
                    for(u32bit rt = 0; rt < MAX_RENDER_TARGETS; rt++)
                    {
                        //  Check if color write is enabled in the current batch.
                        if (state.rtEnable[rt] && (state.colorMaskR[rt] || state.colorMaskG[rt] || state.colorMaskB[rt] || state.colorMaskA[rt]))
                        {
                            sprintf(filename, "frame%04d-batch%05d-pos%08d-rt%02d.emu", frameCounter, batchCounter, trDriver->getTracePosition(), rt);
                            dumpFrame(filename, rt, false);
                            
                            rtEnabled = true;
                        }
                    }
                    
                    if (!rtEnabled)
                    {
                        sprintf(filename, "depth%04d-batch%05d-pos%08d.emu", frameCounter, batchCounter, trDriver->getTracePosition());
                        dumpDepthBuffer(filename);
                    }
#endif
                //}
                
                    GPU_EMU_TRACE(
                        if (traceLog || (traceBatch && (batchCounter == watchBatch)))
                        {
                            sprintf(filename, "alpha%04d-batch%05d-pos%08d.emu", frameCounter, batchCounter, trDriver->getTracePosition());
                            dumpFrame(filename, 0, true);

                            sprintf(filename, "depth%04d-batch%05d-pos%08d.emu", frameCounter, batchCounter, trDriver->getTracePosition());
                            dumpDepthBuffer(filename);

                            sprintf(filename, "stencil%04d-batch%05d-pos%08d.emu", frameCounter, batchCounter, trDriver->getTracePosition());
                            dumpStencilBuffer(filename);
                        }
                    )
                    
                    //sprintf(filename, "profile.frame%04d-batch%05d", frameCounter, batchCounter);
                    //GLOBALPROFILER_GENERATEREPORT(filename)
                    batchCounter++;

                    delete currentTransaction;
                    break;

                case GPU_SWAPBUFFERS:

                    GPU_DEBUG(
                        printf("GPU_SWAPBUFFERS command received.\n");
                    )

                    //  Create current frame filename.
                    sprintf(filename, "frame%04d.emu", frameCounter);
                    dumpFrame(filename, 0);

                    //sprintf(filename, "depth%04.emu", frameCounter);
                    //dumpDepthBuffer(filename);

                    frameCounter++;
                    batchCounter = 0;

                    //  Clean compressed texture caches.
                    cacheDXT1RGB.clear();
                    cacheDXT1RGBA.clear();
                    cacheDXT3RGBA.clear();
                    cacheDXT5RGBA.clear();

                    delete currentTransaction;
                    break;

                case GPU_BLIT:

                    emulateBlitter();
                    delete currentTransaction;
                    break;

                case GPU_CLEARZSTENCILBUFFER:

                    GPU_DEBUG(
                        printf("GPU_CLEARZSTENCILBUFFER command received\n");
                    )

                    clearZStencilBuffer();
                    delete currentTransaction;
                    break;

                case GPU_CLEARCOLORBUFFER:

                    GPU_DEBUG(
                        printf("GPU_CLEARCOLORBUFFER command received\n");
                    )

                    clearColorBuffer();
                    delete currentTransaction;
                    break;

                case GPU_LOAD_VERTEX_PROGRAM:

                    GPU_DEBUG(
                        printf("GPU_LOAD_VERTEX_PROGRAM command received\n");
                    )

                    loadVertexProgram();
                    delete currentTransaction;
                    break;

                case GPU_LOAD_FRAGMENT_PROGRAM:

                    GPU_DEBUG(
                        printf("GPU_LOAD_FRAGMENT_PROGRAM command received\n");
                    )

                    loadFragmentProgram();
                    delete currentTransaction;
                    break;

                case GPU_LOAD_SHADER_PROGRAM:

                    GPU_DEBUG(
                        printf("GPU_LOAD_SHADER_PROGRAM command received\n");
                    )

                    loadShaderProgram();
                    delete currentTransaction;
                    break;

                case GPU_FLUSHZSTENCIL:

                    GPU_DEBUG(
                        printf("GPU_FLUSHZSTENCIL command ignored.\n");
                    )

                    //  Ignore command.
                    delete currentTransaction;
                    break;

                case GPU_FLUSHCOLOR:

                    GPU_DEBUG(
                        printf("GPU_FLUSHCOLOR command ignored.\n");
                    )

                    //  Ignore command.
                    delete currentTransaction;
                    break;

                case GPU_SAVE_COLOR_STATE:

                    GPU_DEBUG(
                        printf("GPU_SAVE_COLOR_STATE command ignored.\n");
                    )

                    //  Ignore command.
                    delete currentTransaction;
                    break;

                case GPU_RESTORE_COLOR_STATE:

                    GPU_DEBUG(
                        printf("GPU_RESTORE_COLOR_STATE command ignored.\n");
                    )

                    //  Ignore command.
                    delete currentTransaction;
                    break;

                case GPU_SAVE_ZSTENCIL_STATE:

                    GPU_DEBUG(
                        printf("GPU_SAVE_ZSTENCIL_STATE command ignored.\n");
                    )

                    //  Ignore command.
                    delete currentTransaction;
                    break;

                case GPU_RESTORE_ZSTENCIL_STATE:

                    GPU_DEBUG(
                        printf("GPU_RESTORE_ZSTENCIL_STATE command ignored.\n");
                    )

                    //  Ignore command.
                    delete currentTransaction;
                    break;

                case GPU_RESET_COLOR_STATE:

                    GPU_DEBUG(
                        printf("GPU_RESET_COLOR_STATE command ignored.\n");
                    )

                    //  Ignore command.
                    delete currentTransaction;
                    break;

                case GPU_RESET_ZSTENCIL_STATE:

                    GPU_DEBUG(
                        printf("GPU_RESET_ZSTENCIL_STATE command ignored.\n");
                    )

                    //  Ignore command.
                    delete currentTransaction;
                    break;

                case GPU_CLEARBUFFERS:

                    panic("GPUEmulator", "emulateCommandProcessor", "GPU_CLEARBUFFERS command not implemented.");
                    break;

                case GPU_CLEARZBUFFER:

                    panic("GPUEmulator", "emulateCommandProcessor", "GPU_CLEARZBUFFER command not implemented.");
                    break;

                default:
                    panic("GPUEmulator", "emulateCommandProcessor", "GPU command not supported.");
                    break;
            }
            break;

        case AGP_INIT_END:

            GPU_DEBUG(
                printf("AGP_INIT_END ignored.\n");
            )

            //  Ignore this transaction.
            delete currentTransaction;
            break;

        case AGP_EVENT:

            GPU_DEBUG(
                printf("AGP_EVENT ignored.\n");
            )

            //  Ignore this transaction.
            delete currentTransaction;
            break;

        default:
            panic("bGPU-Emu", "emulateCommandProcessor", "Unsupported agp command.");
            break;
    }
}

/*  Process a GPU register write.  */
void GPUEmulator::processRegisterWrite(GPURegister gpuReg, u32bit gpuSubReg, GPURegData gpuData)
{
    u32bit textUnit;
    u32bit mipmap;
    u32bit cubemap;

    //  Write request to a GPU register:
    //   *  Check that the AGP transaction can be processed.
    //   *  Write value to GPU register.
    //   *  Issue the state change to other GPU units.

    switch (gpuReg)
    {
        //  GPU state registers.

        case GPU_STATUS:

            //  Read Only register.
            panic("GPUEmulator", "processRegisterWrite", "Error writing to GPU status register not allowed.");
            break;

        //  GPU display registers.

        case GPU_DISPLAY_X_RES:

            GPU_DEBUG(
                printf("Write GPU_DISPLAY_X_RES = %d.\n", gpuData.uintVal);
            )

            //  Check x resolution range.
            GPU_ASSERT(
                if (gpuData.uintVal > MAX_DISPLAY_RES_X)
                    panic("GPUEmulator", "processRegisterWrite", "Horizontal display resolution not supported.");
            )

            //  Set GPU display x resolution register.
            state.displayResX = gpuData.uintVal;

            break;

        case GPU_DISPLAY_Y_RES:

            GPU_DEBUG(
                printf("Write GPU_DISPLAY_Y_RES = %d.\n", gpuData.uintVal);
            )

            //  Check y resolution range.
            GPU_ASSERT(
                if (gpuData.uintVal > MAX_DISPLAY_RES_Y)
                    panic("GPUEmulator", "processRegisterWrite", "Vertical display resolution not supported.");
            )

            //  Set GPU display y resolution register.
            state.displayResY = gpuData.uintVal;

            break;

        case GPU_D3D9_PIXEL_COORDINATES:

            GPU_DEBUG(
                printf("Write GPU_D3D9_PIXEL_COORDINATES = %s.\n", gpuData.booleanVal ? "TRUE" : "FALSE");
            )

            //  Set GPU use D3D9 pixel coordinates convention register.
            state.d3d9PixelCoordinates = gpuData.booleanVal;

            break;

        case GPU_VIEWPORT_INI_X:

            GPU_DEBUG(
                printf("Write GPU_VIEWPORT_INI_X = %d.\n", gpuData.intVal);
            )

            //  Check viewport x range.
            GPU_ASSERT(
                if((gpuData.intVal < MIN_VIEWPORT_X) || (gpuData.intVal > MAX_VIEWPORT_X))
                    panic("GPUEmulator", "processRegisterWrite", "Out of range viewport initial x position.");
            )

            //  Set GPU viewport initial x register.
            state.viewportIniX = gpuData.intVal;

            break;

        case GPU_VIEWPORT_INI_Y:

            GPU_DEBUG(
                printf("Write GPU_VIEWPORT_INI_Y = %d.\n", gpuData.intVal);
            )

            //  Check viewport y range.
            GPU_ASSERT(
                if((gpuData.intVal < MIN_VIEWPORT_Y) || (gpuData.intVal > MAX_VIEWPORT_Y))
                    panic("GPUEmulator", "processRegisterWrite", "Out of range viewport initial y position.");
            )

            //  Set GPU viewport initial y register.
            state.viewportIniY = gpuData.intVal;

            break;

        case GPU_VIEWPORT_HEIGHT:

            GPU_DEBUG(
                printf("Write GPU_VIEWPORT_HEIGHT = %d.\n", gpuData.intVal);
            )

            /*  Check viewport y range.  */
            GPU_ASSERT(
                if(((gpuData.intVal + state.viewportIniY) < MIN_VIEWPORT_Y) ||
                    ((gpuData.intVal + state.viewportIniY) > MAX_VIEWPORT_Y))
                    panic("GPUEmulator", "processRegisterWrite", "Out of range viewport final y position.");
            )

            //  Set GPU viewport width register.
            state.viewportHeight = gpuData.uintVal;

            break;

        case GPU_VIEWPORT_WIDTH:

            GPU_DEBUG(
                printf("Write GPU_VIEWPORT_WIDTH = %d.\n", gpuData.intVal);
            )

            /*  Check viewport x range.  */
            GPU_ASSERT(
                if(((gpuData.intVal + state.viewportIniX) < MIN_VIEWPORT_X) ||
                    ((gpuData.intVal + state.viewportIniX) > MAX_VIEWPORT_X))
                    panic("GPUEmulator", "processRegisterWrite", "Out of range viewport final x position.");
            )

            //  Set GPU viewport width register.
            state.viewportWidth = gpuData.uintVal;

            break;

        case GPU_DEPTH_RANGE_NEAR:

            GPU_DEBUG(
                printf("Write GPU_DEPTHRANGE_NEAR = %f.\n", gpuData.f32Val);
            )

            /*  Check depth near range (clamped to [0, 1]).  */
            GPU_ASSERT(
                if((gpuData.f32Val < 0.0) || (gpuData.f32Val > 1.0))
                    panic("GPUEmulator", "processRegisterWrite", "Near depth range must be clamped to 0..1.");
            )

            //  Set GPU near depth range register.
            state.nearRange = gpuData.f32Val;

            break;

        case GPU_DEPTH_RANGE_FAR:

            GPU_DEBUG(
                printf("Write GPU_DEPTHRANGE_FAR = %f.\n", gpuData.f32Val);
            )

            /*  Check depth far range (clamped to [0, 1]).  */
            GPU_ASSERT(
                if((gpuData.f32Val < 0.0) || (gpuData.f32Val > 1.0))
                    panic("GPUEmulator", "processRegisterWrite", "Far depth range must be clamped to 0..1.");
            )

            //  Set GPU far depth range register.
            state.farRange = gpuData.f32Val;

            break;

        case GPU_D3D9_DEPTH_RANGE:

            GPU_DEBUG(
                printf("Write GPU_D3D9_DEPTH_RANGE = %s\n", gpuData.booleanVal ? "T" : "F");
            )

            //  Set GPU use d39 depth range in clip space register.
            state.d3d9DepthRange = gpuData.booleanVal;

            break;

        case GPU_COLOR_BUFFER_CLEAR:

            GPU_DEBUG(
                printf("Write GPU_COLOR_BUFFER_CLEAR = {%f, %f, %f, %f}.\n",
                    gpuData.qfVal[0], gpuData.qfVal[1], gpuData.qfVal[2], gpuData.qfVal[3]);
            )

            //  Set GPU color buffer clear value register.
            state.colorBufferClear[0] = gpuData.qfVal[0];
            state.colorBufferClear[1] = gpuData.qfVal[1];
            state.colorBufferClear[2] = gpuData.qfVal[2];
            state.colorBufferClear[3] = gpuData.qfVal[3];

            break;

        case GPU_BLIT_INI_X:

            GPU_DEBUG(
                printf("Write GPU_BLIT_INI_X = %d.\n", gpuData.uintVal);
            )

            //  Set GPU blit initial x coordinate register.
            state.blitIniX = gpuData.uintVal;

            break;

        case GPU_BLIT_INI_Y:

            GPU_DEBUG(
                printf("Write GPU_BLIT_INI_Y = %d.\n", gpuData.uintVal);
            )

            //  Set GPU blit initial y coordinate register.
            state.blitIniY = gpuData.uintVal;

            break;

        case GPU_BLIT_X_OFFSET:

            GPU_DEBUG(
                printf("Write GPU_BLIT_X_OFFSET = %d.\n", gpuData.uintVal);
            )

            //  Set GPU blit x offset coordinate register.
            state.blitXOffset = gpuData.uintVal;

            break;

        case GPU_BLIT_Y_OFFSET:

            GPU_DEBUG(
                printf("Write GPU_BLIT_Y_OFFSET = %d.\n", gpuData.uintVal);
            )

            //  Set GPU blit x offset coordinate register.
            state.blitYOffset = gpuData.uintVal;

            break;

        case GPU_BLIT_WIDTH:

            GPU_DEBUG(
                printf("Write GPU_BLIT_WIDTH = %d.\n", gpuData.uintVal);
            )

            //  Set GPU blit width register.
            state.blitWidth = gpuData.uintVal;

            break;

        case GPU_BLIT_HEIGHT:

            GPU_DEBUG(
                printf("Write GPU_BLIT_HEIGHT = %d.\n", gpuData.uintVal);
            )

            //  Set GPU blit height register.
            state.blitHeight = gpuData.uintVal;

            break;

        case GPU_BLIT_DST_ADDRESS:

            GPU_DEBUG(
                printf("Write GPU_BLIT_DST_ADDRESS = %08x.\n", gpuData.uintVal);
            )

            //  Set GPU blit GPU memory destination address register.
            state.blitDestinationAddress = gpuData.uintVal;

            break;

        case GPU_BLIT_DST_TX_WIDTH2:

            GPU_DEBUG(
                printf("Write GPU_BLIT_DST_TX_WIDTH2 = %d.\n", gpuData.txFormat);
            )

            //  Set GPU blit GPU memory destination address register.
            state.blitTextureWidth2 = gpuData.uintVal;

            break;

        case GPU_BLIT_DST_TX_FORMAT:

            GPU_DEBUG(
                printf("Write GPU_BLIT_DST_TX_FORMAT = %d.\n", gpuData.txFormat);
            )

            //  Set GPU blit GPU memory destination address register.
            state.blitDestinationTextureFormat = gpuData.txFormat;

            break;

        case GPU_BLIT_DST_TX_BLOCK:

            GPU_DEBUG(
                printf("Write GPU_BLIT_DST_TX_BLOCK = ", gpuData.txFormat);
                switch(gpuData.txFormat)
                {
                    case GPU_ALPHA8:
                        printf("GPU_ALPHA8\n");
                        break;
                    case GPU_ALPHA12:
                        printf("GPU_ALPHA12\n");
                        break;

                    case GPU_ALPHA16:
                        printf("GPU_ALPHA16\n");
                        break;

                    case GPU_DEPTH_COMPONENT16:
                        printf("GPU_DEPTH_COMPONENT16\n");
                        break;

                    case GPU_DEPTH_COMPONENT24:
                        printf("GPU_DEPTH_COMPONENT24\n");
                        break;

                    case GPU_DEPTH_COMPONENT32:
                        printf("GPU_DEPTH_COMPONENT32\n");
                        break;

                    case GPU_LUMINANCE8:
                        printf("GPU_LUMINANCE8\n");
                        break;

                    case GPU_LUMINANCE8_SIGNED:
                        printf("GPU_LUMINANCE8_SIGNED\n");
                        break;

                    case GPU_LUMINANCE12:
                        printf("GPU_LUMINANCE12\n");
                        break;

                    case GPU_LUMINANCE16:
                        printf("GPU_LUMINANCE16\n");
                        break;

                    case GPU_LUMINANCE4_ALPHA4:
                        printf("GPU_LUMINANCE4_ALPHA4\n");
                        break;

                    case GPU_LUMINANCE6_ALPHA2:
                        printf("GPU_LUMINANCE6_ALPHA2\n");
                        break;

                    case GPU_LUMINANCE8_ALPHA8:
                        printf("GPU_LUMINANCE8_ALPHA8\n");
                        break;

                    case GPU_LUMINANCE8_ALPHA8_SIGNED:
                        printf("GPU_LUMINANCE8_ALPHA8_SIGNED\n");
                        break;

                    case GPU_LUMINANCE12_ALPHA4:
                        printf("GPU_LUMINANCE12_ALPHA4\n");
                        break;

                    case GPU_LUMINANCE12_ALPHA12:
                        printf("GPU_LUMINANCE12_ALPHA12\n");
                        break;

                    case GPU_LUMINANCE16_ALPHA16:
                        printf("GPU_LUMINANCE16_ALPHA16\n");
                        break;

                    case GPU_INTENSITY8:
                        printf("GPU_INTENSITY8\n");
                        break;

                    case GPU_INTENSITY12:
                        printf("GPU_INTENSITY12\n");
                        break;

                    case GPU_INTENSITY16:
                        printf("GPU_INTENSITY16\n");
                        break;
                    case GPU_RGB332:
                        printf("GPU_RGB332\n");
                        break;

                    case GPU_RGB444:
                        printf("GPU_RGB444\n");
                        break;

                    case GPU_RGB555:
                        printf("GPU_RGB555\n");
                        break;

                    case GPU_RGB565:
                        printf("GPU_RGB565\n");
                        break;

                    case GPU_RGB888:
                        printf("GPU_RGB888\n");
                        break;

                    case GPU_RGB101010:
                        printf("GPU_RGB101010\n");
                        break;
                    case GPU_RGB121212:
                        printf("GPU_RGB121212\n");
                        break;

                    case GPU_RGBA2222:
                        printf("GPU_RGBA2222\n");
                        break;

                    case GPU_RGBA4444:
                        printf("GPU_RGBA4444\n");
                        break;

                    case GPU_RGBA5551:
                        printf("GPU_RGBA5551\n");
                        break;

                    case GPU_RGBA8888:
                        printf("GPU_RGBA8888\n");
                        break;

                    case GPU_RGBA1010102:
                        printf("GPU_RGB1010102\n");
                        break;

                    case GPU_R16:
                        printf("GPU_R16\n");
                        break;

                    case GPU_RG16:
                        printf("GPU_RG16\n");
                        break;

                    case GPU_RGBA16:
                        printf("GPU_RGBA16\n");
                        break;

                    case GPU_R16F:
                        printf("GPU_R16F\n");
                        break;

                    case GPU_RG16F:
                        printf("GPU_RG16F\n");
                        break;

                    case GPU_RGBA16F:
                        printf("GPU_RGBA16F\n");
                        break;

                    case GPU_R32F:
                        printf("GPU_R32F\n");
                        break;

                    case GPU_RG32F:
                        printf("GPU_RG32F\n");
                        break;

                    case GPU_RGBA32F:
                        printf("GPU_RGBA32F\n");
                        break;

                    default:
                        printf("unknown format with code %0X\n", gpuSubReg);
                        break;
                }
            )

            //  Set GPU blit GPU memory destination address register.
            state.blitDestinationTextureBlocking = gpuData.txBlocking;

            break;

        case GPU_Z_BUFFER_CLEAR:

            GPU_DEBUG(
                printf("Write GPU_Z_BUFFER_CLEAR = %08x.\n", gpuData.uintVal);
            )

            //  Set GPU Z buffer clear value register.
            state.zBufferClear = gpuData.uintVal;

            break;

        case GPU_Z_BUFFER_BIT_PRECISSION:

            GPU_DEBUG(
                printf("Write GPU_Z_BUFFER_BIT_PRECISSION = %d.\n", gpuData.uintVal);
            )

            //  Check z buffer bit precission.
            GPU_ASSERT(
                if ((gpuData.uintVal != 16) && (gpuData.uintVal != 24) &&
                    (gpuData.uintVal != 32))
                    panic("GPUEmulator", "processRegisterWrite", "Not supported depth buffer bit precission.");
            )

            //  Set GPU Z buffer bit precission register.
            state.zBufferBitPrecission = gpuData.uintVal;

            break;

        case GPU_STENCIL_BUFFER_CLEAR:

            GPU_DEBUG(
                printf("Write GPU_STENCIL_BUFFER_CLEAR = %02x.\n", gpuData.uintVal);
            )

            //  Set GPU Z buffer clear value register.
            state.stencilBufferClear = gpuData.uintVal;

            break;

        //  GPU memory registers.
        case GPU_FRONTBUFFER_ADDR:

            GPU_DEBUG(
                printf("Write GPU_FRONTBUFFER_ADDR = %08x.\n", gpuData.uintVal);
            )

            //  Check address range.

            //  Set GPU front buffer address in GPU memory register.
            state.frontBufferBaseAddr = gpuData.uintVal;

            break;

        case GPU_BACKBUFFER_ADDR:

            GPU_DEBUG(
                printf("Write GPU_BACKBUFFER_ADDR = %08x.\n", gpuData.uintVal);
            )

            //  Check address range.

            //  Set GPU back buffer address in GPU memory register.
            state.backBufferBaseAddr = gpuData.uintVal;

            //  Aliased to render target 0.
            state.rtAddress[0] = gpuData.uintVal;

            break;

        case GPU_ZSTENCILBUFFER_ADDR:

            GPU_DEBUG(
                printf("Write GPU_ZSTENCILBUFFER_ADDR = %08x.\n", gpuData.uintVal);
            )

            //  Check address range.

            //  Set GPU Z/Stencil buffer address in GPU memory register.
            state.zStencilBufferBaseAddr = gpuData.uintVal;

            break;

        case GPU_COLOR_STATE_BUFFER_MEM_ADDR:

            GPU_DEBUG(
                printf("Write GPU_COLOR_STATE_BUFFER_MEM_ADDR = %08x.\n", gpuData.uintVal);
            )

            //  Check address range.

            //  Set GPU color block state buffer memory address register.
            state.colorStateBufferAddr = gpuData.uintVal;

            break;

        case GPU_ZSTENCIL_STATE_BUFFER_MEM_ADDR:

            GPU_DEBUG(
                printf("Write GPU_ZSTENCIL_STATE_BUFFER_MEM_ADDR = %08x.\n", gpuData.uintVal);
            )

            //  Check address range.

            //  Set GPU Z/Stencil block state buffer address in GPU memory register.
            state.zstencilStateBufferAddr = gpuData.uintVal;

            break;

        case GPU_TEXTURE_MEM_ADDR:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_MEM_ADDR = %08x.\n", gpuData.uintVal);
            )

            //  Check address range.

            //  Set GPU texture memory base address in GPU memory register.
            state.textureMemoryBaseAddr = gpuData.uintVal;

            break;

        case GPU_PROGRAM_MEM_ADDR:

            GPU_DEBUG(
                printf("Write GPU_PROGRAM_MEM_ADDR = %08x.\n", gpuData.uintVal);
            )

            //  Check address range.

            //  Set GPU program memory base address in GPU memory register.
            state.programMemoryBaseAddr = gpuData.uintVal;

            break;

        //  GPU vertex shader.
        case GPU_VERTEX_PROGRAM:

            GPU_DEBUG(
                printf("Write GPU_VERTEX_PROGRAM = %08x.\n", gpuData.uintVal);
            )

            //  Check address range.

            //  Set vertex program address register.
            state.vertexProgramAddr = gpuData.uintVal;

            break;

        case GPU_VERTEX_PROGRAM_PC:

            GPU_DEBUG(
                printf("Write GPU_VERTEX_PROGRAM_PC = %04x.\n", gpuData.uintVal);
            )

            //  Check vertex program PC range.

            //  Set vertex program start PC register.
            state.vertexProgramStartPC = gpuData.uintVal;

            break;

        case GPU_VERTEX_PROGRAM_SIZE:

            GPU_DEBUG(
                printf("Write GPU_VERTEX_PROGRAM_SIZE = %d.\n", gpuData.uintVal);
            )

            //  Set vertex program to load size (instructions).
            state.vertexProgramSize = gpuData.uintVal;

            break;

        case GPU_VERTEX_THREAD_RESOURCES:

            GPU_DEBUG(
                printf("Write GPU_VERTEX_THREAD_RESOURCES = %d.\n", gpuData.uintVal);
            )

            //  Set vertex program to load size (instructions).
            state.vertexThreadResources = gpuData.uintVal;

            break;

        case GPU_VERTEX_CONSTANT:

            GPU_DEBUG(
                printf("Write GPU_VERTEX_CONSTANT[%d] = {%f, %f, %f, %f}.\n", gpuSubReg,
                    gpuData.qfVal[0], gpuData.qfVal[1], gpuData.qfVal[2], gpuData.qfVal[3]);
            )

            //  Check constant range.
            GPU_ASSERT(
                if (gpuSubReg >= MAX_VERTEX_CONSTANTS)
                    panic("GPUEmulator", "processRegisterWrite", "Out of range vertex constant register.");
            )

            //  Set vertex constant register.
            state.vConstants[gpuSubReg].setComponents(gpuData.qfVal[0],
                gpuData.qfVal[1], gpuData.qfVal[2], gpuData.qfVal[3]);

            //  Load vertex constant in the shader emulator.
            shEmu->loadShaderState(0, gpu3d::PARAM, gpuSubReg + VERTEX_PARTITION * UNIFIED_CONSTANT_NUM_REGS, state.vConstants[gpuSubReg]);

            break;

        case GPU_VERTEX_OUTPUT_ATTRIBUTE:

            GPU_DEBUG(
                printf("Write GPU_VERTEX_OUTPUT_ATTRIBUTE[%d] = %s.\n",
                    gpuSubReg, gpuData.booleanVal?"ENABLED":"DISABLED");
            )

            //  Check vertex attribute identifier range.
            GPU_ASSERT(
                if (gpuSubReg >= MAX_VERTEX_ATTRIBUTES)
                    panic("GPUEmulator", "processRegisterWrite", "Out of range vertex attribute identifier.");
            )

            //  Set GPU vertex attribute mapping register.
            state.outputAttribute[gpuSubReg] = gpuData.booleanVal;

            break;

        //  GPU vertex stream buffer registers.
        case GPU_VERTEX_ATTRIBUTE_MAP:

            GPU_DEBUG(
                printf("Write GPU_VERTEX_ATTRIBUTE_MAP[%d] = %d.\n", gpuSubReg, gpuData.uintVal);
            )

            //  Check vertex attribute identifier range.
            GPU_ASSERT(
                if (gpuSubReg >= MAX_VERTEX_ATTRIBUTES)
                    panic("GPUEmulator", "processRegisterWrite", "Out of range vertex attribute identifier.");
            )

            //  Set GPU vertex attribute mapping register.
            state.attributeMap[gpuSubReg] = gpuData.uintVal;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE:

            GPU_DEBUG(
                printf("Write GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE[%d] = {%f, %f, %f, %f}.\n", gpuSubReg,
                    gpuData.qfVal[0], gpuData.qfVal[1], gpuData.qfVal[2], gpuData.qfVal[3]);
            )

            //  Check vertex attribute identifier range.
            GPU_ASSERT(
                if (gpuSubReg >= MAX_VERTEX_ATTRIBUTES)
                    panic("GPUEmulator", "processRegisterWrite", "Out of range vertex attribute identifier.");
            )

            //  Set GPU vertex attribute default value.
            state.attrDefValue[gpuSubReg][0] = gpuData.qfVal[0];
            state.attrDefValue[gpuSubReg][1] = gpuData.qfVal[1];
            state.attrDefValue[gpuSubReg][2] = gpuData.qfVal[2];
            state.attrDefValue[gpuSubReg][3] = gpuData.qfVal[3];

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_STREAM_ADDRESS:

            GPU_DEBUG(
                printf("Write GPU_STREAM_ADDRESS[%d] = %08x.\n", gpuSubReg, gpuData.uintVal);
            )

            //  Check stream identifier range.
            GPU_ASSERT(
                if (gpuSubReg >= MAX_STREAM_BUFFERS)
                    panic("GPUEmulator", "processRegisterWrite", "Out of range stream buffer identifier.");
            )

            //  Set GPU stream buffer address register.
            state.streamAddress[gpuSubReg] = gpuData.uintVal;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_STREAM_STRIDE:

            GPU_DEBUG(
                printf("Write GPU_STREAM_STRIDE[%d] = %d.\n", gpuSubReg, gpuData.uintVal);
            )

            //  Check stream identifier range.
            GPU_ASSERT(
                if (gpuSubReg >= MAX_STREAM_BUFFERS)
                    panic("GPUEmulator", "processRegisterWrite", "Out of range stream buffer identifier.");
            )

            //  Set GPU stream buffer stride register.
            state.streamStride[gpuSubReg] = gpuData.uintVal;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_STREAM_DATA:

            GPU_DEBUG(
                printf("Write GPU_STREAM_DATA[%d] = ", gpuSubReg);
                switch(gpuData.streamData)
                {
                    case SD_UNORM8:
                        printf("SD_UNORM8.\n");
                        break;
                    case SD_SNORM8:
                        printf("SD_SNORM8.\n");
                        break;
                    case SD_UNORM16:
                        printf("SD_UNORM16.\n");
                        break;
                    case SD_SNORM16:
                        printf("SD_SNORM16.\n");
                        break;
                    case SD_UNORM32:
                        printf("SD_UNORM32.\n");
                        break;
                    case SD_SNORM32:
                        printf("SD_SNORM32.\n");
                        break;
                    case SD_FLOAT16:
                        printf("SD_FLOAT16.\n");
                        break;
                    case SD_FLOAT32:
                        printf("SD_FLOAT32.\n");
                        break;
                    case SD_UINT8:
                        printf("SD_UINT8.\n");
                        break;
                    case SD_SINT8:
                        printf("SD_SINT8.\n");
                        break;
                    case SD_UINT16:
                        printf("SD_UINT16.\n");
                        break;
                    case SD_SINT16:
                        printf("SD_SINT16.\n");
                        break;
                    case SD_UINT32:
                        printf("SD_UINT32.\n");
                        break;
                    case SD_SINT32:
                        printf("SD_SINT32.\n");
                        break;
                    default:
                        panic("emu", "processRegisterWrite", "Undefined format.");
                        break;
                }
            )

            //  Check stream identifier range.
            GPU_ASSERT(
                if (gpuSubReg >= MAX_STREAM_BUFFERS)
                    panic("GPUEmulator", "processRegisterWrite", "Out of range stream buffer identifier.");
            )

            //  Set GPU stream buffer data type register.
            state.streamData[gpuSubReg] = gpuData.streamData;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_STREAM_ELEMENTS:

            GPU_DEBUG(
                printf("Write GPU_STREAM_ELEMENTS[%d] = %d.\n", gpuSubReg, gpuData.uintVal);
            )

            //  Check stream identifier range.
            GPU_ASSERT(
                if (gpuSubReg >= MAX_STREAM_BUFFERS)
                    panic("GPUEmulator", "processRegisterWrite", "Out of range stream buffer identifier.");
            )

            //  Set GPU stream buffer number of elements per entry register.
            state.streamElements[gpuSubReg] = gpuData.uintVal;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_STREAM_FREQUENCY:

            GPU_DEBUG(
                printf("Write GPU_STREAM_FREQUENCY[%d] = %d.\n", gpuSubReg, gpuData.uintVal);
            )

            //  Check stream identifier range.
            GPU_ASSERT(
                if (gpuSubReg >= MAX_STREAM_BUFFERS)
                    panic("GPUEmulator", "processRegisterWrite", "Out of range stream buffer identifier.");
            )

            //  Set GPU stream buffer read frequency register.
            state.streamFrequency[gpuSubReg] = gpuData.uintVal;

            //  Write register in the Texture Emulator.
            //texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_STREAM_START:

            GPU_DEBUG(
                printf("Write GPU_STREAM_START = %d.\n", gpuData.uintVal);
            )

            //  Set GPU stream start position register.
            state.streamStart = gpuData.uintVal;

            break;

        case GPU_STREAM_COUNT:

            GPU_DEBUG(
                printf("Write GPU_STREAM_COUNT = %d.\n", gpuData.uintVal);
            )

            //  Set GPU stream count register.
            state.streamCount = gpuData.uintVal;

            break;

        case GPU_STREAM_INSTANCES:

            GPU_DEBUG(
                printf("Write GPU_STREAM_INSTANCES = %d.\n", gpuData.uintVal);
            )

            //  Set GPU stream instances register.
            state.streamInstances = gpuData.uintVal;

            break;

        case GPU_INDEX_MODE:

            GPU_DEBUG(
                printf("Write GPU_INDEX_MODE = %s.\n", gpuData.booleanVal ? "ENABLED" : "DISABLED");
            )

            //  Set GPU indexed batching mode register.
            state.indexedMode = gpuData.booleanVal;

            break;

        case GPU_INDEX_STREAM:

            GPU_DEBUG(
                printf("Write GPU_INDEX_STREAM = %d.\n", gpuData.uintVal);
            )

            //  Check stream identifier range.
            GPU_ASSERT(
                if (gpuData.uintVal >= MAX_STREAM_BUFFERS)
                    panic("GPUEmulator", "processRegisterWrite", "Out of range stream buffer identifier.");
            )

            //  Set GPU stream buffer for index buffer register.
            state.indexStream = gpuData.uintVal;

            break;

        case GPU_D3D9_COLOR_STREAM:

            GPU_DEBUG(
                printf("Write GPU_D3D9_COLOR_STREAM[%d] = %s.\n", gpuSubReg, gpuData.booleanVal ? "T" : "F");
            )

            //  Check stream identifier range.
            GPU_ASSERT(
                if (gpuSubReg >= MAX_STREAM_BUFFERS)
                    panic("GPUEmulator", "processRegisterWrite", "Out of range stream buffer identifier.");
            )

            //  Set GPU D3D9 color stream register.
            state.d3d9ColorStream[gpuSubReg] = gpuData.booleanVal;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_ATTRIBUTE_LOAD_BYPASS:

            GPU_DEBUG(
                printf("Write GPU_ATTRIBUTE_LOAD_BYPASS = %s.\n", gpuData.booleanVal ? "T" : "F");
            )

            //  Set attribute load bypass register.
            state.attributeLoadBypass = gpuData.booleanVal;

            break;

        //  GPU primitive assembly registers.

        case GPU_PRIMITIVE:

            GPU_DEBUG(
                printf("Write GPU_PRIMITIVE = ");
                switch(gpuData.primitive)
                {
                    case gpu3d::TRIANGLE:
                        printf("TRIANGLE.\n");
                        break;
                    case gpu3d::TRIANGLE_STRIP:
                        printf("TRIANGLE_STRIP.\n");
                        break;
                    case gpu3d::TRIANGLE_FAN:
                        printf("TRIANGLE_FAN.\n");
                        break;
                    case gpu3d::QUAD:
                        printf("QUAD.\n");
                        break;
                    case gpu3d::QUAD_STRIP:
                        printf("QUAD_STRIP.\n");
                        break;
                    case gpu3d::LINE:
                        printf("LINE.\n");
                        break;
                    case gpu3d::LINE_STRIP:
                        printf("LINE_STRIP.\n");
                        break;
                    case gpu3d::LINE_FAN:
                        printf("LINE_FAN.\n");
                        break;
                    case gpu3d::POINT:
                        printf("POINT.\n");
                        break;
                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Unsupported primitive mode.");
                        break;
                }
            )

            //  Set GPU primitive mode register.
            state.primitiveMode = gpuData.primitive;

            break;


        //  GPU clipping and culling registers.

        case GPU_FRUSTUM_CLIPPING:

            GPU_DEBUG(
                printf("Write GPU_FRUSTUM_CLIPPING = %s.\n", gpuData.booleanVal ? "T":"F");
            )

            //  Set GPU frustum clipping flag register.
            state.frustumClipping = gpuData.booleanVal;

            break;

        case GPU_USER_CLIP:

            GPU_DEBUG(
                printf("Write GPU_USER_CLIP[%d] = {%f, %f, %f, %f}.\n", gpuSubReg,
                    gpuData.qfVal[0], gpuData.qfVal[1], gpuData.qfVal[2], gpuData.qfVal[3]);
            )

            //  Check user clip plane identifier range.
            GPU_ASSERT(
                if (gpuSubReg >= MAX_USER_CLIP_PLANES)
                    panic("GPUEmulator", "processRegisterWrite", "Out of range user clip plane identifier.");
            )

            //  Set GPU user clip plane.
            state.userClip[gpuSubReg].setComponents(gpuData.qfVal[0],
                gpuData.qfVal[1], gpuData.qfVal[2], gpuData.qfVal[3]);

            break;

        case GPU_USER_CLIP_PLANE:

            GPU_DEBUG(
                printf("Write GPU_USER_CLIP_PLANE = %s.\n", gpuData.booleanVal ? "T" : "F");
            )

            //  Set GPU user clip planes enable register.
            state.userClipPlanes = gpuData.booleanVal;

            break;

        case GPU_FACEMODE:

            GPU_DEBUG(
                printf("Write GPU_FACEMODE = ");
                switch(gpuData.culling)
                {
                    case GPU_CW:
                        printf("CW.\n");
                        break;

                    case GPU_CCW:
                        printf("CCW.\n");
                        break;
                }
            )

            //  Set GPU face mode (vertex order for front facing triangles) register.
            state.faceMode = gpuData.faceMode;

            break;

        case GPU_CULLING:

            GPU_DEBUG(
                printf("Write GPU_CULLING = ");
                switch(gpuData.culling)
                {
                    case NONE:
                        printf("NONE.\n");
                        break;

                    case FRONT:
                        printf("FRONT.\n");
                        break;

                    case BACK:
                        printf("BACK.\n");
                        break;

                    case FRONT_AND_BACK:
                        printf("FRONT_AND_BACK.\n");
                        break;

                }
            )

            //  Set GPU culling mode (none, front, back, front/back) register.
            state.cullMode = gpuData.culling;

            break;

        //  Hierarchical Z registers.
        case GPU_HIERARCHICALZ:

            GPU_DEBUG(
                printf("Write GPU_HIERARCHICALZ = %s.\n", gpuData.booleanVal ? "T" : "F");
            )

            //  Set GPU hierarchical Z enable flag register.
            state.hzTest = gpuData.booleanVal;

            break;

        //  Hierarchical Z registers.
        case GPU_EARLYZ:

            GPU_DEBUG(
                printf("Write GPU_EARLYZ = %s.\n", gpuData.booleanVal ? "T" : "F");
            )

            //  Set GPU early Z test enable flag register.
            state.earlyZ = gpuData.booleanVal;

            break;

        //  GPU rasterization registers.

        case GPU_SCISSOR_TEST:

            GPU_DEBUG(
                printf("Write GPU_SCISSOR_TEST = %s.\n", gpuData.booleanVal ? "T" : "F");
            )

            //  Set GPU scissor test enable register.
            state.scissorTest = gpuData.booleanVal;

            break;

        case GPU_SCISSOR_INI_X:

            GPU_DEBUG(
                printf("Write GPU_SCISSOR_INI_X = %d.\n", gpuData.intVal);
            )

            //  Set GPU scissor initial x register.
            state.scissorIniX = gpuData.intVal;

            break;

        case GPU_SCISSOR_INI_Y:

            GPU_DEBUG(
                printf("Write GPU_SCISSOR_INI_Y = %d.\n", gpuData.intVal);
            )

            //  Set GPU scissor initial y register.
            state.scissorIniY = gpuData.intVal;

            break;

        case GPU_SCISSOR_WIDTH:

            GPU_DEBUG(
                printf("Write GPU_SCISSOR_WIDTH = %d.\n", gpuData.uintVal);
            )

            //  Set GPU scissor width register.
            state.scissorWidth = gpuData.uintVal;

            break;

        case GPU_SCISSOR_HEIGHT:

            GPU_DEBUG(
                printf("Write GPU_SCISSOR_HEIGHT = %d.\n", gpuData.uintVal);
            )

            //  Set GPU scissor height register.
            state.scissorHeight = gpuData.uintVal;

            break;

        case GPU_DEPTH_SLOPE_FACTOR:

            GPU_DEBUG(
                printf("Write GPU_DEPTH_SLOPE_FACTOR = %f.\n", gpuData.f32Val);
            )

            //  Set GPU depth slope factor.
            state.slopeFactor = gpuData.f32Val;

            break;

        case GPU_DEPTH_UNIT_OFFSET:

            GPU_DEBUG(
                printf("Write GPU_DEPTH_UNIT_OFFSET = %f.\n", gpuData.f32Val);
            )

            //  Set GPU depth unit offset.
            state.unitOffset = gpuData.f32Val;

            break;

        case GPU_D3D9_RASTERIZATION_RULES:

            GPU_DEBUG(
                printf("Write GPU_D3D9_RASTERIZATION_RULES = %s\n", gpuData.booleanVal ? "T" : "F");
            )

            //  Set GPU use d39 rasterization rules.
            state.d3d9RasterizationRules = gpuData.booleanVal;

            break;

        case GPU_TWOSIDED_LIGHTING:

            GPU_DEBUG(
                printf("Write GPU_TWOSIDED_LIGHTING = %s.\n", gpuData.booleanVal ? "T" : "F");
            )

            //  Set GPU two sided lighting color selection register.
            state.twoSidedLighting = gpuData.booleanVal;

            break;

        case GPU_MULTISAMPLING:

            GPU_DEBUG(
                printf("Write GPU_MULTISAMPLING = %s.\n", gpuData.booleanVal ? "T" : "F");
            )

            //  Set multisampling (MSAA) enabling register.
            state.multiSampling = gpuData.booleanVal;

            break;

        case GPU_MSAA_SAMPLES:

            GPU_DEBUG(
                printf("Write GPU_MSAA_SAMPLES = %d.\n", gpuData.uintVal);
            )

            //  Set GPU number of z samples to generate per fragment for MSAA register.
            state.msaaSamples = gpuData.uintVal;

            break;

        case GPU_INTERPOLATION:

            GPU_DEBUG(
                printf("Write GPU_INTERPOLATION = %s.\n", gpuData.booleanVal ? "T" : "F");
            )

            //  Set GPU fragments attribute interpolation mode (GL flat/smoth) register.
            state.interpolation[gpuSubReg] = gpuData.booleanVal;

            break;

        case GPU_FRAGMENT_INPUT_ATTRIBUTES:

            GPU_DEBUG(
                printf("Write GPU_FRAGMENT_INPUT_ATTRIBUTES[%d] = %s.\n",
                    gpuSubReg, gpuData.booleanVal?"ENABLED":"DISABLED");
            )

            //  Check fragment attribute range.
            GPU_ASSERT(
                if (gpuSubReg >= MAX_FRAGMENT_ATTRIBUTES)
                    panic("GPUEmulator", "processRegisterWrite", "Fragment attribute identifier out of range.");
            )

            //  Set fragment input attribute active flag GPU register.
            state.fragmentInputAttributes[gpuSubReg] = gpuData.booleanVal;

            break;

        //  GPU fragment registers.
        case GPU_FRAGMENT_PROGRAM:

            GPU_DEBUG(
                printf("Write GPU_FRAGMENT_PROGRAM = %08x.\n", gpuData.uintVal);
            )

            //  Check address range.

            //  Set fragment program address register.
            state.fragProgramAddr = gpuData.uintVal;

            break;

        case GPU_FRAGMENT_PROGRAM_PC:

            GPU_DEBUG(
                printf("Write GPU_FRAGMENT_PROGRAM_PC = %04x.\n", gpuData.uintVal);
            )

            //  Check fragment program PC range.

            //  Set fragment program start PC register.
            state.fragProgramStartPC = gpuData.uintVal;

            break;

        case GPU_FRAGMENT_PROGRAM_SIZE:

            GPU_DEBUG(
                printf("Write GPU_FRAGMENT_PROGRAM_SIZE = %d.\n", gpuData.uintVal);
            )

            //  Check fragment program size.

            //  Set fragment program to load size (instructions).
            state.fragProgramSize = gpuData.uintVal;

            break;

        case GPU_FRAGMENT_THREAD_RESOURCES:

            GPU_DEBUG(
                printf("Write GPU_FRAGMENT_THREAD_RESOURCES = %d.\n", gpuData.uintVal);
            )

            //  Set per fragment thread resource usage.
            state.fragThreadResources = gpuData.uintVal;

            break;

        case GPU_FRAGMENT_CONSTANT:

            GPU_DEBUG(
                printf("Write GPU_FRAGMENT_CONSTANT[%d] = {%f, %f, %f, %f}.\n", gpuSubReg,
                    gpuData.qfVal[0], gpuData.qfVal[1], gpuData.qfVal[2], gpuData.qfVal[3]);
            )

            //  Check constant range.
            GPU_ASSERT(
                if (gpuSubReg >= MAX_FRAGMENT_CONSTANTS)
                    panic("GPUEmulator", "processRegisterWrite", "Out of range fragment constant register.");
            )

            //  Set fragment constant register.
            state.fConstants[gpuSubReg].setComponents(gpuData.qfVal[0],
                gpuData.qfVal[1], gpuData.qfVal[2], gpuData.qfVal[3]);

            //  Load fragment constant in the shader emulator.
            shEmu->loadShaderState(0, gpu3d::PARAM, gpuSubReg + FRAGMENT_PARTITION * UNIFIED_CONSTANT_NUM_REGS, state.fConstants[gpuSubReg]);

            break;

        case GPU_MODIFY_FRAGMENT_DEPTH:

            GPU_DEBUG(
                printf("Write GPU_MODIFY_FRAGMENT_DEPTH = %s.\n", gpuData.booleanVal ? "T" : "F");
            )

            //  Set fragment constant register.
            state.modifyDepth = gpuData.booleanVal;

            break;

        //  GPU Shader Program registers.
        case GPU_SHADER_PROGRAM_ADDRESS:

            //  Check address range.

            //  Set shader program address register.  */
            state.programAddress = gpuData.uintVal;

            GPU_DEBUG(
                printf("Write GPU_SHADER_PROGRAM_ADDDRESS = %x.\n", gpuData.uintVal);
            )

            break;

        case GPU_SHADER_PROGRAM_SIZE:

            //  Check shader program size.

            //  Set shader program size (bytes).
            state.programSize = gpuData.uintVal;

            GPU_DEBUG(
                printf("Write GPU_SHADER_PROGRAM_SIZE = %x.\n", gpuData.uintVal);
            )

            break;

        case GPU_SHADER_PROGRAM_LOAD_PC:

            //  Check fragment program PC range.

            //  Set shader program load PC register.
            state.programLoadPC = gpuData.uintVal;

            GPU_DEBUG(
                printf("Write GPU_SHADER_PROGRAM_LOAD_PC = %x.\n", gpuData.uintVal);
            )

            break;


        case GPU_SHADER_PROGRAM_PC:

            {

                //  Check fragment program PC range.

                //  Set shader program PC register for the corresponding shader target.
                state.programStartPC[gpuSubReg] = gpuData.uintVal;

                //  Get shader target for which the per-thread resource register is changed.
                ShaderTarget shTarget;
                switch(gpuSubReg)
                {
                    case VERTEX_TARGET:
                        shTarget = VERTEX_TARGET;
                        break;
                    case FRAGMENT_TARGET:
                        shTarget = FRAGMENT_TARGET;
                        break;
                    case TRIANGLE_TARGET:
                        shTarget = TRIANGLE_TARGET;
                        break;
                    case MICROTRIANGLE_TARGET:
                        shTarget = MICROTRIANGLE_TARGET;
                        break;
                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Undefined shader target.");
                        break;
                }

                GPU_DEBUG(
                    printf("Write GPU_SHADER_PROGRAM_LOAD_PC[");

                    switch(gpuSubReg)
                    {
                        case VERTEX_TARGET:
                            printf("VERTEX_TARGET");
                            break;
                        case FRAGMENT_TARGET:
                            printf("FRAGMENT_TARGET");
                            break;
                        case TRIANGLE_TARGET:
                            printf("TRIANGLE_TARGET");
                            break;
                        case MICROTRIANGLE_TARGET:
                            printf("MICROTRIANGLE_TARGET");
                            break;
                        default:
                            panic("GPUEmulator", "processRegisterWrite", "Undefined shader target.");
                            break;
                    }

                    printf("] = %x.\n", gpuData.uintVal);
                )
            }

            break;

        case GPU_SHADER_THREAD_RESOURCES:

            {
                //  Set per fragment thread resource usage register for the corresponding shader target.
                state.programResources[gpuSubReg] = gpuData.uintVal;

                //  Get shader target for which the per-thread resource register is changed.
                ShaderTarget shTarget;
                switch(gpuSubReg)
                {
                    case VERTEX_TARGET:
                        shTarget = VERTEX_TARGET;
                        break;
                    case FRAGMENT_TARGET:
                        shTarget = FRAGMENT_TARGET;
                        break;
                    case TRIANGLE_TARGET:
                        shTarget = TRIANGLE_TARGET;
                        break;
                    case MICROTRIANGLE_TARGET:
                        shTarget = MICROTRIANGLE_TARGET;
                        break;
                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Undefined shader target.");
                        break;
                }

                GPU_DEBUG(
                    printf("WriteGPU_SHADER_THREAD_RESOURCES[");

                    switch(gpuSubReg)
                    {
                        case VERTEX_TARGET:
                            printf("VERTEX_TARGET");
                            break;
                        case FRAGMENT_TARGET:
                            printf("FRAGMENT_TARGET");
                            break;
                        case TRIANGLE_TARGET:
                            printf("TRIANGLE_TARGET");
                            break;
                        case MICROTRIANGLE_TARGET:
                            printf("MICROTRIANGLE_TARGET");
                            break;
                        default:
                            panic("GPUEmulator", "processRegisterWrite", "Undefined shader target.");
                            break;
                    }

                    printf("] = %x.\n", gpuData.uintVal);
                )
            }

            break;


        //  GPU Texture Unit registers.
        case GPU_TEXTURE_ENABLE:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_ENABLE[%d] = %s.\n", gpuSubReg, gpuData.booleanVal ? "T" : "F");
            )

            //  Write texture enable register.
            state.textureEnabled[gpuSubReg] = gpuData.booleanVal;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_MODE:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_MODE[%d] = ", gpuSubReg);
                switch(gpuData.txMode)
                {
                    case GPU_TEXTURE1D:
                        printf("TEXTURE1D\n");
                        break;

                    case GPU_TEXTURE2D:
                        printf("TEXTURE2D\n");
                        break;

                    case GPU_TEXTURE3D:
                        printf("TEXTURE3D\n");
                        break;

                    case GPU_TEXTURECUBEMAP:
                        printf("TEXTURECUBEMAP\n");
                        break;

                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Unsupported texture mode.");
                }
            )

            //  Write texture mode register.
            state.textureMode[gpuSubReg] = gpuData.txMode;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_ADDRESS:

            //  WARNING:  As the current simulator only support an index per register we must
            //  decode the texture unit, mipmap and cubemap image.  To do so the cubemap images of
            //  a mipmap are stored sequentally and then the mipmaps for a texture unit
            //  are stored sequentially and then the mipmap addresses of the other texture units
            //  are stored sequentially.

            //  Calculate texture unit and mipmap level.
            textUnit = gpuSubReg / (MAX_TEXTURE_SIZE * CUBEMAP_IMAGES);
            mipmap = GPU_MOD(gpuSubReg / CUBEMAP_IMAGES, MAX_TEXTURE_SIZE);
            cubemap = GPU_MOD(gpuSubReg, CUBEMAP_IMAGES);

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_ADDRESS[%d][%d][%d] = %08x.\n", textUnit, mipmap, cubemap, gpuData.uintVal);
            )

            //  Write texture address register (per cubemap image, mipmap and texture unit).
            state.textureAddress[textUnit][mipmap][cubemap] = gpuData.uintVal;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_WIDTH:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_WIDTH[%d] = %d.\n", gpuSubReg, gpuData.uintVal);
            )

            //  Write texture width (first mipmap).
            state.textureWidth[gpuSubReg] = gpuData.uintVal;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_HEIGHT:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_HEIGHT[%d] = %d.\n", gpuSubReg, gpuData.uintVal);
            )

            //  Write texture height (first mipmap).
            state.textureHeight[gpuSubReg] = gpuData.uintVal;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_DEPTH:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_DEPTH[%d] = %d.\n", gpuSubReg, gpuData.uintVal);
            )

            //  Write texture width (first mipmap).
            state.textureDepth[gpuSubReg] = gpuData.uintVal;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_WIDTH2:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_WIDTH2[%d] = %d.\n", gpuSubReg, gpuData.uintVal);
            )

            //  Write texture width (log of 2 of the first mipmap).
            state.textureWidth2[gpuSubReg] = gpuData.uintVal;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_HEIGHT2:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_HEIGHT2[%d] = %d.\n", gpuSubReg, gpuData.uintVal);
            )

            //  Write texture height (log 2 of the first mipmap).
            state.textureHeight2[gpuSubReg] = gpuData.uintVal;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_DEPTH2:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_DEPTH2[%d] = %d.\n", gpuSubReg, gpuData.uintVal);
            )

            //  Write texture depth (log of 2 of the first mipmap).
            state.textureDepth2[gpuSubReg] = gpuData.uintVal;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_BORDER:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_BORDER[%d] = %d.\n", gpuSubReg, gpuData.uintVal);
            )

            //  Write texture border register.
            state.textureBorder[gpuSubReg] = gpuData.uintVal;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;


        case GPU_TEXTURE_FORMAT:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_FORMAT[%d] = ", gpuSubReg);
                switch(gpuData.txFormat)
                {
                    case GPU_ALPHA8:
                        printf("GPU_ALPHA8\n");
                        break;
                    case GPU_ALPHA12:
                        printf("GPU_ALPHA12\n");
                        break;

                    case GPU_ALPHA16:
                        printf("GPU_ALPHA16\n");
                        break;

                    case GPU_DEPTH_COMPONENT16:
                        printf("GPU_DEPTH_COMPONENT16\n");
                        break;

                    case GPU_DEPTH_COMPONENT24:
                        printf("GPU_DEPTH_COMPONENT24\n");
                        break;

                    case GPU_DEPTH_COMPONENT32:
                        printf("GPU_DEPTH_COMPONENT32\n");
                        break;

                    case GPU_LUMINANCE8:
                        printf("GPU_LUMINANCE8\n");
                        break;

                    case GPU_LUMINANCE12:
                        printf("GPU_LUMINANCE12\n");
                        break;

                    case GPU_LUMINANCE16:
                        printf("GPU_LUMINANCE16\n");
                        break;

                    case GPU_LUMINANCE4_ALPHA4:
                        printf("GPU_LUMINANCE4_ALPHA4\n");
                        break;

                    case GPU_LUMINANCE6_ALPHA2:
                        printf("GPU_LUMINANCE6_ALPHA2\n");
                        break;

                    case GPU_LUMINANCE8_ALPHA8:
                        printf("GPU_LUMINANCE8_ALPHA8\n");
                        break;

                    case GPU_LUMINANCE12_ALPHA4:
                        printf("GPU_LUMINANCE12_ALPHA4\n");
                        break;

                    case GPU_LUMINANCE12_ALPHA12:
                        printf("GPU_LUMINANCE12_ALPHA12\n");
                        break;

                    case GPU_LUMINANCE16_ALPHA16:
                        printf("GPU_LUMINANCE16_ALPHA16\n");
                        break;

                    case GPU_INTENSITY8:
                        printf("GPU_INTENSITY8\n");
                        break;

                    case GPU_INTENSITY12:
                        printf("GPU_INTENSITY12\n");
                        break;

                    case GPU_INTENSITY16:
                        printf("GPU_INTENSITY16\n");
                        break;
                    case GPU_RGB332:
                        printf("GPU_RGB332\n");
                        break;

                    case GPU_RGB444:
                        printf("GPU_RGB444\n");
                        break;

                    case GPU_RGB555:
                        printf("GPU_RGB555\n");
                        break;

                    case GPU_RGB565:
                        printf("GPU_RGB565\n");
                        break;

                    case GPU_RGB888:
                        printf("GPU_RGB888\n");
                        break;

                    case GPU_RGB101010:
                        printf("GPU_RGB101010\n");
                        break;
                    case GPU_RGB121212:
                        printf("GPU_RGB121212\n");
                        break;

                    case GPU_RGBA2222:
                        printf("GPU_RGBA2222\n");
                        break;

                    case GPU_RGBA4444:
                        printf("GPU_RGBA4444\n");
                        break;

                    case GPU_RGBA5551:
                        printf("GPU_RGBA5551\n");
                        break;

                    case GPU_RGBA8888:
                        printf("GPU_RGBA8888\n");
                        break;

                    case GPU_RGBA1010102:
                        printf("GPU_RGB1010102\n");
                        break;

                    case GPU_R16:
                        printf("GPU_R16\n");
                        break;

                    case GPU_RG16:
                        printf("GPU_RG16\n");
                        break;

                    case GPU_RGBA16:
                        printf("GPU_RGBA16\n");
                        break;

                    case GPU_R16F:
                        printf("GPU_R16F\n");
                        break;

                    case GPU_RG16F:
                        printf("GPU_RG16F\n");
                        break;

                    case GPU_RGBA16F:
                        printf("GPU_RGBA16F\n");
                        break;

                    case GPU_R32F:
                        printf("GPU_R32F\n");
                        break;

                    case GPU_RG32F:
                        printf("GPU_RG32F\n");
                        break;

                    case GPU_RGBA32F:
                        printf("GPU_RGBA32F\n");
                        break;

                    default:
                        printf("unknown format with code %0X\n", gpuData.txFormat);
                        break;
                }
            )

            //  Write texture format register.
            state.textureFormat[gpuSubReg] = gpuData.txFormat;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_REVERSE:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_REVERSE[%d] = %s.\n", gpuSubReg, gpuData.booleanVal ? "T" : "F");
            )

            //  Write texture reverse register.  */
            state.textureReverse[gpuSubReg] = gpuData.booleanVal;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_D3D9_COLOR_CONV:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_D3D9_COLOR_CONV[%d] = %s.\n", gpuSubReg, gpuData.booleanVal ? "T" : "F");
            )

            //  Write texture D3D9 color order conversion register.
            state.textD3D9ColorConv[gpuSubReg] = gpuData.booleanVal;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_D3D9_V_INV:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_D3D9_V_INV[%d] = %s.\n", gpuSubReg, gpuData.booleanVal ? "T" : "F");
            )

            //  Write texture D3D9 v coordinate inversion register.
            state.textD3D9VInvert[gpuSubReg] = gpuData.booleanVal;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_COMPRESSION:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_COMPRESSION[%d] = ", gpuSubReg);
                switch(gpuData.txCompression)
                {
                    case GPU_NO_TEXTURE_COMPRESSION:
                        printf("GPU_NO_TEXTURE_COMPRESSION.\n");
                        break;

                    case GPU_S3TC_DXT1_RGB:
                        printf("GPU_S3TC_DXT1_RGB.\n");
                        break;

                    case GPU_S3TC_DXT1_RGBA:
                        printf("GPU_S3TC_DXT1_RGBA.\n");
                        break;

                    case GPU_S3TC_DXT3_RGBA:
                        printf("GPU_S3TC_DXT3_RGBA.\n");
                        break;

                    case GPU_S3TC_DXT5_RGBA:
                        printf("GPU_S3TC_DXT5_RGBA.\n");
                        break;

                    case GPU_LATC1:
                        printf("GPU_LATC1.\n");
                        break;

                    case GPU_LATC1_SIGNED:
                        printf("GPU_LATC1_SIGNED.\n");
                        break;

                    case GPU_LATC2:
                        printf("GPU_LATC2.\n");
                        break;

                    case GPU_LATC2_SIGNED:
                        printf("GPU_LATC2_SIGNED.\n");
                        break;

                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Unsupported texture compression mode.");
                        break;
                }
            )

            //  Write texture compression register.
            state.textureCompr[gpuSubReg] = gpuData.txCompression;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_BLOCKING:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_BLOCKING[%d] = ", gpuSubReg);
                switch(gpuData.txBlocking)
                {
                    case GPU_TXBLOCK_TEXTURE:
                        printf("GPU_TXBLOCK_TEXTURE.\n");
                        break;

                    case GPU_TXBLOCK_FRAMEBUFFER:
                        printf("GPU_TXBLOCK_FRAMEBUFFER.\n");
                        break;

                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Unsupported texture blocking mode.");
                        break;
                }
            )

            //  Write texture blocking mode register.
            state.textureBlocking[gpuSubReg] = gpuData.txBlocking;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_BORDER_COLOR:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_COLOR[%d] = {%f, %f, %f, %f}.\n", gpuSubReg,
                    gpuData.qfVal[0], gpuData.qfVal[1], gpuData.qfVal[2], gpuData.qfVal[3]);
            )

            //  Write texture border color register.
            state.textBorderColor[gpuSubReg][0] = gpuData.qfVal[0];
            state.textBorderColor[gpuSubReg][1] = gpuData.qfVal[1];
            state.textBorderColor[gpuSubReg][2] = gpuData.qfVal[2];
            state.textBorderColor[gpuSubReg][3] = gpuData.qfVal[3];

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_WRAP_S:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_WRAP_S[%d] = ", gpuSubReg);
                switch(gpuData.txClamp)
                {
                    case GPU_TEXT_CLAMP:
                        printf("CLAMP\n");
                        break;

                    case GPU_TEXT_CLAMP_TO_EDGE:
                        printf("CLAMP_TO_EDGE\n");
                        break;

                    case GPU_TEXT_REPEAT:
                        printf("REPEAT\n");
                        break;

                    case GPU_TEXT_CLAMP_TO_BORDER:
                        printf("CLAMP_TO_BORDER\n");
                        break;

                    case GPU_TEXT_MIRRORED_REPEAT:
                        printf("MIRRORED_REPEAT\n");
                        break;

                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Unsupported texture clamp mode.");
                        break;
                }
            )

            //  Write texture wrap in s dimension register.
            state.textureWrapS[gpuSubReg] = gpuData.txClamp;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_WRAP_T:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_WRAP_T[%d] = ", gpuSubReg);
                switch(gpuData.txClamp)
                {
                    case GPU_TEXT_CLAMP:
                        printf("CLAMP\n");
                        break;

                    case GPU_TEXT_CLAMP_TO_EDGE:
                        printf("CLAMP_TO_EDGE\n");
                        break;

                    case GPU_TEXT_REPEAT:
                        printf("REPEAT\n");
                        break;

                    case GPU_TEXT_CLAMP_TO_BORDER:
                        printf("CLAMP_TO_BORDER\n");
                        break;

                    case GPU_TEXT_MIRRORED_REPEAT:
                        printf("MIRRORED_REPEAT\n");
                        break;

                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Unsupported texture clamp mode.");
                        break;
                }
            )

            //  Write texture wrap in t dimension register.
            state.textureWrapT[gpuSubReg] = gpuData.txClamp;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_WRAP_R:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_WRAP_R[%d] = ", gpuSubReg);
                switch(gpuData.txClamp)
                {
                    case GPU_TEXT_CLAMP:
                        printf("CLAMP\n");
                        break;

                    case GPU_TEXT_CLAMP_TO_EDGE:
                        printf("CLAMP_TO_EDGE\n");
                        break;

                    case GPU_TEXT_REPEAT:
                        printf("REPEAT\n");
                        break;

                    case GPU_TEXT_CLAMP_TO_BORDER:
                        printf("CLAMP_TO_BORDER\n");
                        break;

                    case GPU_TEXT_MIRRORED_REPEAT:
                        printf("MIRRORED_REPEAT\n");
                        break;

                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Unsupported texture clamp mode.");
                        break;
                }
            )

            //  Write texture wrap in r dimension register.
            state.textureWrapR[gpuSubReg] = gpuData.txClamp;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_NON_NORMALIZED:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_NON_NORMALIZED[%d] = ", gpuSubReg, gpuData.booleanVal ? "T" , "F");
            )

            //  Write texture non-normalized coordinates register.
            state.textureNonNormalized[gpuSubReg] = gpuData.booleanVal;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_MIN_FILTER:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_MIN_FILTER[%d] = ", gpuSubReg);
                switch(gpuData.txFilter)
                {
                    case GPU_NEAREST:
                        printf("NEAREST\n");
                        break;

                    case GPU_LINEAR:
                        printf("LINEAR\n");
                        break;

                    case GPU_NEAREST_MIPMAP_NEAREST:
                        printf("NEAREST_MIPMAP_NEAREST\n");
                        break;

                    case GPU_NEAREST_MIPMAP_LINEAR:
                        printf("NEAREST_MIPMAP_LINEAR\n");
                        break;

                    case GPU_LINEAR_MIPMAP_NEAREST:
                        printf("LINEAR_MIPMAP_NEAREST\n");
                        break;

                    case GPU_LINEAR_MIPMAP_LINEAR:
                        printf("LINEAR_MIPMAP_LINEAR\n");
                        break;

                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Unsupported texture minification filter mode.");
                        break;
                }
            )

            //  Write texture minification filter register.
            state.textureMinFilter[gpuSubReg] = gpuData.txFilter;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_MAG_FILTER:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_MAG_FILTER[%d] = ", gpuSubReg);
                switch(gpuData.txFilter)
                {
                    case GPU_NEAREST:
                        printf("NEAREST\n");
                        break;

                    case GPU_LINEAR:
                        printf("LINEAR\n");
                        break;

                    case GPU_NEAREST_MIPMAP_NEAREST:
                        printf("NEAREST_MIPMAP_NEAREST\n");
                        break;

                    case GPU_NEAREST_MIPMAP_LINEAR:
                        printf("NEAREST_MIPMAP_LINEAR\n");
                        break;

                    case GPU_LINEAR_MIPMAP_NEAREST:
                        printf("LINEAR_MIPMAP_NEAREST\n");
                        break;

                    case GPU_LINEAR_MIPMAP_LINEAR:
                        printf("LINEAR_MIPMAP_LINEAR\n");
                        break;

                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Unsupported texture minification filter mode.");
                        break;
                }
            )

            //  Write texture magnification filter register.
            state.textureMagFilter[gpuSubReg] = gpuData.txFilter;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_ENABLE_COMPARISON:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_ENABLE_COMPARISON[%d] = %s.\n", gpuSubReg, gpuData.booleanVal ? "T" : "F");
            )

            //  Write texture enable comparison (PCF) register.
            state.textureEnableComparison[gpuSubReg] = gpuData.booleanVal;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_COMPARISON_FUNCTION:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_COMPARISON_FUNCTION[%d] = ", gpuSubReg);
                switch(gpuData.compare)
                {
                    case GPU_NEVER:
                        printf("NEVER.\n");
                        break;

                    case GPU_ALWAYS:
                        printf("ALWAYS.\n");
                        break;

                    case GPU_LESS:
                        printf("LESS.\n");
                        break;

                    case GPU_LEQUAL:
                        printf("LEQUAL.\n");
                        break;

                    case GPU_EQUAL:
                        printf("EQUAL.\n");
                        break;

                    case GPU_GEQUAL:
                        printf("GEQUAL.\n");
                        break;

                    case GPU_GREATER:
                        printf("GREATER.\n");
                        break;

                    case GPU_NOTEQUAL:
                        printf("NOTEQUAL.\n");

                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Undefined compare function mode");
                        break;
                }
            )

            //  Write texture comparison function (PCF) register.
            state.textureComparisonFunction[gpuSubReg] = gpuData.compare;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_SRGB:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_SRGB[%d] = %s.\n", gpuSubReg, gpuData.booleanVal ? "T" : "F");
            )

            //  Write texture SRGB to linear conversion register.
            state.textureSRGB[gpuSubReg] = gpuData.booleanVal;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_MIN_LOD:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_MIN_LOD[%d] = %f.\n", gpuSubReg, gpuData.uintVal);
            )

            //  Write texture minimum lod register.
            state.textureMinLOD[gpuSubReg] = gpuData.f32Val;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_MAX_LOD:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_MAX_LOD[%d] = %f.\n", gpuSubReg, gpuData.uintVal);
            )

            //  Write texture maximum lod register.
            state.textureMaxLOD[gpuSubReg] = gpuData.f32Val;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_LOD_BIAS:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_LOD_BIAS[%d] = %f.\n", gpuSubReg, gpuData.f32Val);
            )

            //  Write texture lod bias register.
            state.textureLODBias[gpuSubReg] = gpuData.f32Val;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_MIN_LEVEL:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_MIN_LEVEL[%d] = %d.\n", gpuSubReg, gpuData.uintVal);
            )

            //  Write texture minimum mipmap level register.
            state.textureMinLevel[gpuSubReg] = gpuData.uintVal;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_MAX_LEVEL:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_MAX_LEVEL[%d] = %d.\n", gpuSubReg, gpuData.uintVal);
            )

            //  Write texture maximum mipmap level register.
            state.textureMaxLevel[gpuSubReg] = gpuData.uintVal;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXT_UNIT_LOD_BIAS:

            GPU_DEBUG(
                printf("Write GPU_TEXT_UNIT_LOD_BIAS[%d] = %f.\n", gpuSubReg, gpuData.f32Val);
            )

            //  Write texture unit lod bias register.
            state.textureUnitLODBias[gpuSubReg] = gpuData.f32Val;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        case GPU_TEXTURE_MAX_ANISOTROPY:

            GPU_DEBUG(
                printf("Write GPU_TEXTURE_MAX_ANISOTROPY[%d] = %d.\n", gpuSubReg, gpuData.uintVal);
            )

            //  Write texture unit max anisotropy register.
            state.maxAnisotropy[gpuSubReg] = gpuData.uintVal;

            //  Write register in the Texture Emulator.
            texEmu->writeRegister(gpuReg, gpuSubReg, gpuData);

            break;

        //  GPU Stencil test registers.
        case GPU_STENCIL_TEST:

            GPU_DEBUG(
                printf("Write GPU_STENCIL_TEST = %s.\n", gpuData.booleanVal ? "T" : "F");
            )

            //  Set stencil test enable flag.
            state.stencilTest = gpuData.booleanVal;

            break;

        case GPU_STENCIL_FUNCTION:

            GPU_DEBUG(
                printf("Write GPU_STENCIL_FUNCTION = ");
                switch(gpuData.compare)
                {
                    case GPU_NEVER:
                        printf("NEVER.\n");
                        break;

                    case GPU_ALWAYS:
                        printf("ALWAYS.\n");
                        break;

                    case GPU_LESS:
                        printf("LESS.\n");
                        break;

                    case GPU_LEQUAL:
                        printf("LEQUAL.\n");
                        break;

                    case GPU_EQUAL:
                        printf("EQUAL.\n");
                        break;

                    case GPU_GEQUAL:
                        printf("GEQUAL.\n");
                        break;

                    case GPU_GREATER:
                        printf("GREATER.\n");
                        break;

                    case GPU_NOTEQUAL:
                        printf("NOTEQUAL.\n");

                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Undefined compare function mode");
                        break;
                }
            )

            //  Set stencil test compare function.
            state.stencilFunction = gpuData.compare;

            break;

        case GPU_STENCIL_REFERENCE:

            GPU_DEBUG(
                printf("Write GPU_STENCIL_REFERENCE = %02x.\n", gpuData.uintVal);
            )

            //  Set stencil test reference value.
            state.stencilReference = u8bit(gpuData.uintVal);

            break;

        case GPU_STENCIL_COMPARE_MASK:

            GPU_DEBUG(
                printf("Write GPU_STENCIL_COMPARE_MASK = %02x.\n", gpuData.uintVal);
            )

            //  Set stencil test compare mask.
            state.stencilTestMask = u8bit(gpuData.uintVal);

            break;

        case GPU_STENCIL_UPDATE_MASK:

            GPU_DEBUG(
                printf("Write GPU_UPDATE_MASK = %02x.\n", gpuData.uintVal);
            )

            //  Set stencil test update mask.
            state.stencilUpdateMask = u8bit(gpuData.uintVal);

            break;

        case GPU_STENCIL_FAIL_UPDATE:

            GPU_DEBUG(
                printf("Write GPU_STENCIL_FAIL_UPDATE = ");
                switch(gpuData.stencilUpdate)
                {
                    case STENCIL_KEEP:
                        printf("KEEP.\n");
                        break;

                    case STENCIL_ZERO:
                        printf("ZERO.\n");
                        break;

                    case STENCIL_REPLACE:
                        printf("REPLACE.\n");
                        break;

                    case STENCIL_INCR:
                        printf("INCREMENT.\n");
                        break;

                    case STENCIL_DECR:
                        printf("DECREMENT.\n");
                        break;

                    case STENCIL_INVERT:
                        printf("INVERT.\n");
                        break;

                    case STENCIL_INCR_WRAP:
                        printf("INCREMENT AND WRAP.\n");
                        break;

                    case STENCIL_DECR_WRAP:
                        printf("DECREMENT AND WRAP.\n");
                        break;

                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Undefined stencil update function");
                        break;
                }
            )

            //  Set stencil fail stencil update function.
            state.stencilFail = gpuData.stencilUpdate;

            break;

        case GPU_DEPTH_FAIL_UPDATE:

            GPU_DEBUG(
                printf("Write GPU_DEPTH_FAIL_UPDATE = ");
                switch(gpuData.stencilUpdate)
                {
                    case STENCIL_KEEP:
                        printf("KEEP.\n");
                        break;

                    case STENCIL_ZERO:
                        printf("ZERO.\n");
                        break;

                    case STENCIL_REPLACE:
                        printf("REPLACE.\n");
                        break;

                    case STENCIL_INCR:
                        printf("INCREMENT.\n");
                        break;

                    case STENCIL_DECR:
                        printf("DECREMENT.\n");
                        break;

                    case STENCIL_INVERT:
                        printf("INVERT.\n");
                        break;

                    case STENCIL_INCR_WRAP:
                        printf("INCREMENT AND WRAP.\n");
                        break;

                    case STENCIL_DECR_WRAP:
                        printf("DECREMENT AND WRAP.\n");
                        break;

                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Undefined stencil update function");
                        break;
                }
            )

            //  Set stencil update function for depth fail.
            state.depthFail = gpuData.stencilUpdate;

            break;

        case GPU_DEPTH_PASS_UPDATE:

            GPU_DEBUG(
                printf("Write GPU_DEPTH_PASS_UPDATE = ");
                switch(gpuData.stencilUpdate)
                {
                    case STENCIL_KEEP:
                        printf("KEEP.\n");
                        break;

                    case STENCIL_ZERO:
                        printf("ZERO.\n");
                        break;

                    case STENCIL_REPLACE:
                        printf("REPLACE.\n");
                        break;

                    case STENCIL_INCR:
                        printf("INCREMENT.\n");
                        break;

                    case STENCIL_DECR:
                        printf("DECREMENT.\n");
                        break;

                    case STENCIL_INVERT:
                        printf("INVERT.\n");
                        break;

                    case STENCIL_INCR_WRAP:
                        printf("INCREMENT AND WRAP.\n");
                        break;

                    case STENCIL_DECR_WRAP:
                        printf("DECREMENT AND WRAP.\n");
                        break;

                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Undefined stencil update function");
                        break;
                }
            )

            //  Set stencil update function for depth pass.
            state.depthPass = gpuData.stencilUpdate;

            break;

        //  GPU Depth test registers.
        case GPU_DEPTH_TEST:

            GPU_DEBUG(
                printf("Write GPU_DEPTH_TEST = %s\n", gpuData.booleanVal ? "T" : "F");
            )

            //  Set depth test enable flag.
            state.depthTest = gpuData.booleanVal;

            break;

        case GPU_DEPTH_FUNCTION:

            GPU_DEBUG(
                printf("Write GPU_DEPTH_FUNCION = ");
                switch(gpuData.compare)
                {
                    case GPU_NEVER:
                        printf("NEVER.\n");
                        break;

                    case GPU_ALWAYS:
                        printf("ALWAYS.\n");
                        break;

                    case GPU_LESS:
                        printf("LESS.\n");
                        break;

                    case GPU_LEQUAL:
                        printf("LEQUAL.\n");
                        break;

                    case GPU_EQUAL:
                        printf("EQUAL.\n");
                        break;

                    case GPU_GEQUAL:
                        printf("GEQUAL.\n");
                        break;

                    case GPU_GREATER:
                        printf("GREATER.\n");
                        break;

                    case GPU_NOTEQUAL:
                        printf("NOTEQUAL.\n");

                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Undefined compare function mode");
                        break;
                }
            )

            //  Set depth test compare function.
            state.depthFunction = gpuData.compare;

            break;

        case GPU_DEPTH_MASK:

            GPU_DEBUG(
                printf("Write GPU_DEPTH_MASK = %s.\n", gpuData.booleanVal ? "T" : "F");
            )

            //  Set depth test update mask.
            state.depthMask = gpuData.booleanVal;

            break;

        case GPU_ZSTENCIL_COMPRESSION:

            GPU_DEBUG(
                printf("Write GPU_ZSTENCIL_COMPRESSION = %s.\n", gpuData.booleanVal ? "T" : "F");
            )

            //  Set Z/Stencil compression enable/disable register.
            state.zStencilCompression = gpuData.booleanVal;

            break;

        //  GPU Color Buffer and Render Target registers.

        case GPU_COLOR_BUFFER_FORMAT:

            GPU_DEBUG(
                printf("Write GPU_COLOR_BUFFER_FORMAT = ");
                switch(gpuData.txFormat)
                {
                    case GPU_RGBA8888:
                        printf(" GPU_RGBA8888.\n");
                        break;

                    case GPU_RG16F:
                        printf(" GPU_RG16F.\n");
                        break;

                    case GPU_R32F:
                        printf(" GPU_R32F.\n");
                        break;

                    case GPU_RGBA16:
                        printf(" GPU_RGBA16.\n");
                        break;

                    case GPU_RGBA16F:
                        printf(" GPU_RGBA16F.\n");
                        break;

                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Unsupported color buffer format.\n");
                        break;
                }
            )

            //  Set color buffer format register.
            state.colorBufferFormat = gpuData.txFormat;

            //  Aliased to render target 0.
            state.rtFormat[0] = gpuData.txFormat;

            break;

        case GPU_COLOR_COMPRESSION:

            GPU_DEBUG(
                printf("Write GPU_COLOR_COMPRESSION = %s.\n", gpuData.booleanVal ? "T" : "F");
            )

            //  Set color compression enable/disable register.
            state.colorCompression = gpuData.booleanVal;

            break;

        case GPU_COLOR_SRGB_WRITE:
            
            GPU_DEBUG(
                printf("Write GPU_COLOR_SRGB_WRITE = %s\n", gpuData.booleanVal ? "T" : "F");
            )
            
            //  Set sRGB conversion on color write register.
            state.colorSRGBWrite = gpuData.booleanVal;
            
            break;
            
        case GPU_RENDER_TARGET_ENABLE:

            GPU_DEBUG(
                printf("Write GPU_RENDER_TARGET_ENABLE[%d] = %s.\n", gpuSubReg, gpuData.booleanVal ? "T" : "F");
            )

            //  Set render target enable/disable register.
            state.rtEnable[gpuSubReg] = gpuData.booleanVal;

            break;

        case GPU_RENDER_TARGET_FORMAT:

            GPU_DEBUG(
                printf("Write GPU_RENDER_TARGET_FORMAT[%d] = ", gpuSubReg);
                switch(gpuData.txFormat)
                {
                    case GPU_RGBA8888:
                        printf(" GPU_RGBA8888.\n");
                        break;

                    case GPU_RG16F:
                        printf(" GPU_RG16F.\n");
                        break;

                    case GPU_R32F:
                        printf(" GPU_R32F.\n");
                        break;

                    case GPU_RGBA16:
                        printf(" GPU_RGBA16.\n");
                        break;

                    case GPU_RGBA16F:
                        printf(" GPU_RGBA16F.\n");
                        break;

                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Unsupported color buffer format.\n");
                        break;
                }
            )

            //  Set render target format register.
            state.rtFormat[gpuSubReg] = gpuData.txFormat;
            
            //  Render target 0 is aliased to the back buffer.
            if (gpuSubReg == 0)
                state.colorBufferFormat = gpuData.txFormat;            

            break;

        case GPU_RENDER_TARGET_ADDRESS:

            GPU_DEBUG(
                printf("Write GPU_RENDER_TARGET_ADDRESS[%d] = %08x.\n", gpuSubReg, gpuData.uintVal);
            )

            //  Set render target base address register.
            state.rtAddress[gpuSubReg] = gpuData.uintVal;
            
            //  Back buffer is aliased to render target 0.
            if (gpuSubReg == 0)            
                state.backBufferBaseAddr = gpuData.uintVal;

            break;

        //  GPU Color Blend registers.
        case GPU_COLOR_BLEND:

            GPU_DEBUG(
                printf("Write GPU_COLOR_BLEND[%d] = %s.\n", gpuSubReg, gpuData.booleanVal ? "T" : "F");
            )

            //  Set color blend enable flag.
            state.colorBlend[gpuSubReg] = gpuData.booleanVal;

            break;

        case GPU_BLEND_EQUATION:

            GPU_DEBUG(
                printf("Write GPU_BLEND_EQUATION[%d] = ", gpuSubReg);
                switch(gpuData.blendEquation)
                {
                    case BLEND_FUNC_ADD:
                        printf("FUNC_ADD.\n");
                        break;

                    case BLEND_FUNC_SUBTRACT:
                        printf("FUNC_SUBTRACT.\n");
                        break;

                    case BLEND_FUNC_REVERSE_SUBTRACT:
                        printf("FUNC_REVERSE_SUBTRACT.\n");
                        break;

                    case BLEND_MIN:
                        printf("MIN.\n");
                        break;

                    case BLEND_MAX:
                        printf("MAX.\n");
                        break;

                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Unsupported blend equation mode.");
                        break;
                }
           )

            //  Set color blend equation.
            state.blendEquation[gpuSubReg] = gpuData.blendEquation;

            break;


        case GPU_BLEND_SRC_RGB:

            GPU_DEBUG(
                printf("Write GPU_BLEND_SRC_RGB[%d] = ", gpuSubReg);
                switch(gpuData.blendFunction)
                {
                    case BLEND_ZERO:
                        printf("ZERO.\n");
                        break;

                    case BLEND_ONE:
                        printf("ONE.\n");
                        break;

                    case BLEND_SRC_COLOR:
                        printf("SRC_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_SRC_COLOR:
                        printf("ONE_MINUS_SRC_COLOR.\n");
                        break;

                    case BLEND_DST_COLOR:
                        printf("DST_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_DST_COLOR:
                        printf("ONE_MINUS_DST_COLOR.\n");
                        break;

                    case BLEND_SRC_ALPHA:
                        printf("SRC_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_SRC_ALPHA:
                        printf("ONE_MINUS_SRC_ALPHA.\n");
                        break;

                    case BLEND_DST_ALPHA:
                        printf("DST_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_DST_ALPHA:
                        printf("ONE_MINUS_DST_ALPHA.\n");
                        break;

                    case BLEND_CONSTANT_COLOR:
                        printf("CONSTANT_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_CONSTANT_COLOR:
                        printf("ONE_MINUS_CONSTANT_COLOR.\n");
                        break;

                    case BLEND_CONSTANT_ALPHA:
                        printf("CONSTANT_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_CONSTANT_ALPHA:
                        printf("ONE_MINUS_CONSTANT_ALPHA.\n");
                        break;

                    case BLEND_SRC_ALPHA_SATURATE:
                        printf("ALPHA_SATURATE.\n");
                        break;

                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Unsupported blend weight function.");
                        break;
                }
            )

            //  Set color blend source RGB weight factor.
            state.blendSourceRGB[gpuSubReg] = gpuData.blendFunction;

            break;

        case GPU_BLEND_DST_RGB:

            GPU_DEBUG(
                printf("Write GPU_BLEND_DST_RGB[%d] = ", gpuSubReg);
                switch(gpuData.blendFunction)
                {
                    case BLEND_ZERO:
                        printf("ZERO.\n");
                        break;

                    case BLEND_ONE:
                        printf("ONE.\n");
                        break;

                    case BLEND_SRC_COLOR:
                        printf("SRC_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_SRC_COLOR:
                        printf("ONE_MINUS_SRC_COLOR.\n");
                        break;

                    case BLEND_DST_COLOR:
                        printf("DST_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_DST_COLOR:
                        printf("ONE_MINUS_DST_COLOR.\n");
                        break;

                    case BLEND_SRC_ALPHA:
                        printf("SRC_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_SRC_ALPHA:
                        printf("ONE_MINUS_SRC_ALPHA.\n");
                        break;

                    case BLEND_DST_ALPHA:
                        printf("DST_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_DST_ALPHA:
                        printf("ONE_MINUS_DST_ALPHA.\n");
                        break;

                    case BLEND_CONSTANT_COLOR:
                        printf("CONSTANT_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_CONSTANT_COLOR:
                        printf("ONE_MINUS_CONSTANT_COLOR.\n");
                        break;

                    case BLEND_CONSTANT_ALPHA:
                        printf("CONSTANT_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_CONSTANT_ALPHA:
                        printf("ONE_MINUS_CONSTANT_ALPHA.\n");
                        break;

                    case BLEND_SRC_ALPHA_SATURATE:
                        printf("ALPHA_SATURATE.\n");
                        break;

                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Unsupported blend weight function.");
                        break;
                }
            )

            //  Set color blend destination RGB weight factor.
            state.blendDestinationRGB[gpuSubReg] = gpuData.blendFunction;

            break;

        case GPU_BLEND_SRC_ALPHA:

            GPU_DEBUG(
                printf("Write GPU_BLEND_SRC_ALPHA[%d] = ", gpuSubReg);
                switch(gpuData.blendFunction)
                {
                    case BLEND_ZERO:
                        printf("ZERO.\n");
                        break;

                    case BLEND_ONE:
                        printf("ONE.\n");
                        break;

                    case BLEND_SRC_COLOR:
                        printf("SRC_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_SRC_COLOR:
                        printf("ONE_MINUS_SRC_COLOR.\n");
                        break;

                    case BLEND_DST_COLOR:
                        printf("DST_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_DST_COLOR:
                        printf("ONE_MINUS_DST_COLOR.\n");
                        break;

                    case BLEND_SRC_ALPHA:
                        printf("SRC_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_SRC_ALPHA:
                        printf("ONE_MINUS_SRC_ALPHA.\n");
                        break;

                    case BLEND_DST_ALPHA:
                        printf("DST_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_DST_ALPHA:
                        printf("ONE_MINUS_DST_ALPHA.\n");
                        break;

                    case BLEND_CONSTANT_COLOR:
                        printf("CONSTANT_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_CONSTANT_COLOR:
                        printf("ONE_MINUS_CONSTANT_COLOR.\n");
                        break;

                    case BLEND_CONSTANT_ALPHA:
                        printf("CONSTANT_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_CONSTANT_ALPHA:
                        printf("ONE_MINUS_CONSTANT_ALPHA.\n");
                        break;

                    case BLEND_SRC_ALPHA_SATURATE:
                        printf("ALPHA_SATURATE.\n");
                        break;

                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Unsupported blend weight function.");
                        break;
                }
            )

            //  Set color blend source ALPHA weight factor.
            state.blendSourceAlpha[gpuSubReg] = gpuData.blendFunction;

            break;

        case GPU_BLEND_DST_ALPHA:

            GPU_DEBUG(
                printf("Write GPU_BLEND_DST_ALPHA[%d] = ", gpuSubReg);
                switch(gpuData.blendFunction)
                {
                    case BLEND_ZERO:
                        printf("ZERO.\n");
                        break;

                    case BLEND_ONE:
                        printf("ONE.\n");
                        break;

                    case BLEND_SRC_COLOR:
                        printf("SRC_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_SRC_COLOR:
                        printf("ONE_MINUS_SRC_COLOR.\n");
                        break;

                    case BLEND_DST_COLOR:
                        printf("DST_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_DST_COLOR:
                        printf("ONE_MINUS_DST_COLOR.\n");
                        break;

                    case BLEND_SRC_ALPHA:
                        printf("SRC_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_SRC_ALPHA:
                        printf("ONE_MINUS_SRC_ALPHA.\n");
                        break;

                    case BLEND_DST_ALPHA:
                        printf("DST_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_DST_ALPHA:
                        printf("ONE_MINUS_DST_ALPHA.\n");
                        break;

                    case BLEND_CONSTANT_COLOR:
                        printf("CONSTANT_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_CONSTANT_COLOR:
                        printf("ONE_MINUS_CONSTANT_COLOR.\n");
                        break;

                    case BLEND_CONSTANT_ALPHA:
                        printf("CONSTANT_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_CONSTANT_ALPHA:
                        printf("ONE_MINUS_CONSTANT_ALPHA.\n");
                        break;

                    case BLEND_SRC_ALPHA_SATURATE:
                        printf("ALPHA_SATURATE.\n");
                        break;

                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Unsupported blend weight function.");
                        break;
                }
            )

            //  Set color blend destination Alpha weight factor.
            state.blendDestinationAlpha[gpuSubReg] = gpuData.blendFunction;

            break;

        case GPU_BLEND_COLOR:

            GPU_DEBUG(
                printf("Write GPU_BLEND_COLOR[%d] = {%f, %f, %f, %f}.\n", gpuSubReg,
                    gpuData.qfVal[0], gpuData.qfVal[1], gpuData.qfVal[2], gpuData.qfVal[3]);
            )

            //  Set color blend constant color.
            state.blendColor[gpuSubReg][0] = gpuData.qfVal[0];
            state.blendColor[gpuSubReg][1] = gpuData.qfVal[1];
            state.blendColor[gpuSubReg][2] = gpuData.qfVal[2];
            state.blendColor[gpuSubReg][3] = gpuData.qfVal[3];

            break;

        case GPU_COLOR_MASK_R:

            GPU_DEBUG(
                printf("Write GPU_WRITE_MASK_R[%d] = %s.\n", gpuSubReg, gpuData.booleanVal ? "T" : "F");
            )

            //  Set color mask for red component.
            state.colorMaskR[gpuSubReg] = gpuData.booleanVal;

            break;

        case GPU_COLOR_MASK_G:

            GPU_DEBUG(
                printf("Write GPU_WRITE_MASK_G[%d] = %s.\n", gpuSubReg, gpuData.booleanVal ? "T": "F");
            )

            //  Set color mask for green component.
            state.colorMaskG[gpuSubReg] = gpuData.booleanVal;

            break;

        case GPU_COLOR_MASK_B:

            GPU_DEBUG(
                printf("Write GPU_WRITE_MASK_B[%d] = %s.\n", gpuSubReg, gpuData.booleanVal ? "T" : "F");
            )

            //  Set color mask for blue component.
            state.colorMaskB[gpuSubReg] = gpuData.booleanVal;

            break;

        case GPU_COLOR_MASK_A:

            GPU_DEBUG(
                printf("Write GPU_WRITE_MASK_A[%d] = %s.\n", gpuSubReg, gpuData.booleanVal ? "T" : "F");
            )

            //  Set color mask for alpha component.
            state.colorMaskA[gpuSubReg] = gpuData.booleanVal;

            break;

        //  GPU Color Logical Operation registers.
        case GPU_LOGICAL_OPERATION:

            GPU_DEBUG(
                printf("Write GPU_LOGICAL_OPERATION = %s.\n", gpuData.booleanVal ? "T" : "F");
            )

            //  Set color logical operation enable flag.
            state.logicalOperation = gpuData.booleanVal;

            break;

        case GPU_LOGICOP_FUNCTION:

            GPU_DEBUG(
                printf("Write GPU_LOGICOP_FUNCTION = ");
                switch(gpuData.logicOp)
                {
                    case LOGICOP_CLEAR:
                        printf("CLEAR.\n");
                        break;

                    case LOGICOP_AND:
                        printf("AND.\n");
                        break;

                    case LOGICOP_AND_REVERSE:
                        printf("AND_REVERSE.\n");
                        break;

                    case LOGICOP_COPY:
                        printf("COPY.\n");
                        break;

                    case LOGICOP_AND_INVERTED:
                        printf("AND_INVERTED.\n");
                        break;

                    case LOGICOP_NOOP:
                        printf("NOOP.\n");
                        break;

                    case LOGICOP_XOR:
                        printf("XOR.\n");
                        break;

                    case LOGICOP_OR:
                        printf("OR.\n");
                        break;

                    case LOGICOP_NOR:
                        printf("NOR.\n");
                        break;

                    case LOGICOP_EQUIV:
                        printf("EQUIV.\n");
                        break;

                    case LOGICOP_INVERT:
                        printf("INVERT.\n");
                        break;

                    case LOGICOP_OR_REVERSE:
                        printf("OR_REVERSE.\n");
                        break;

                    case LOGICOP_COPY_INVERTED:
                        printf("COPY_INVERTED.\n");
                        break;

                    case LOGICOP_OR_INVERTED:
                        printf("OR_INVERTED.\n");
                        break;

                    case LOGICOP_NAND:
                        printf("NAND.\n");
                        break;

                    case LOGICOP_SET:
                        printf("SET.\n");
                        break;

                    default:
                        panic("GPUEmulator", "processRegisterWrite", "Unsupported logical operation function.");
                        break;
                }
            )

            //  Set color mask for red component.
            state.logicOpFunction = gpuData.logicOp;

            break;

        case GPU_MCV2_2ND_INTERLEAVING_START_ADDR:

            GPU_DEBUG(
                printf("Write GPU_MCV2_2ND_INTERLEAVING_START_ADDR = %08x.\n", gpuData.uintVal);
            )

            state.mcSecondInterleavingStartAddr = gpuData.uintVal;

            break;

        default:

            panic("GPUEmulator", "processRegisterWrite", "Undefined GPU register identifier.");

            break;

    }
}

void GPUEmulator::clearColorBuffer()
{
    GLOBALPROFILER_ENTERREGION("clearColorBuffer", "", "clearColorBuffer")

    if (!skipBatchMode)
    {
        u8bit clearColor8bit[4 * 4];
        u8bit clearColor16bit[4 * 8];
        u8bit clearColor32bit[4 * 16];
        QuadFloat clearColor[4];

        for(u32bit p = 0; p < 4; p++)
        {
            for(u32bit c = 0; c < 4; c++)
            {
                clearColor[p][c] = state.colorBufferClear[c];
                ((f32bit *) clearColor32bit)[p * 4 + c] = state.colorBufferClear[c];
            }
        }

        colorRGBA32FToRGBA8(clearColor, clearColor8bit);
        colorRGBA32FToRGBA16F(clearColor, clearColor16bit);

        u32bit samples = state.multiSampling ? state.msaaSamples : 1;
        u32bit bytesPixel;

        //switch(state.colorBufferFormat)
        switch(state.rtFormat[0])
        {
            case GPU_RGBA8888:
            case GPU_RG16F:
            case GPU_R32F:
                bytesPixel = 4;
                break;
            case GPU_RGBA16:
            case GPU_RGBA16F:
                bytesPixel = 8;
                break;
        }

        pixelMapper[0].setupDisplay(state.displayResX, state.displayResY, STAMP_WIDTH, STAMP_WIDTH,
                                 simP.ras.genWidth / STAMP_WIDTH, simP.ras.genHeight / STAMP_HEIGHT,
                                 simP.ras.scanWidth / simP.ras.genWidth, simP.ras.scanHeight / simP.ras.genHeight,
                                 simP.ras.overScanWidth, simP.ras.overScanHeight,
                                 samples, bytesPixel);

        //u8bit *memory = selectMemorySpace(state.backBufferBaseAddr);
        u8bit *memory = selectMemorySpace(state.rtAddress[0]);

        for(u32bit y = 0; y < state.displayResY; y += 2)
        {
            for(u32bit x = 0; x < state.displayResX; x += 2)
            {
                u32bit address = pixelMapper[0].computeAddress(x, y);

                //address += state.backBufferBaseAddr;
                address += state.rtAddress[0];

                //  Get address inside the memory space.
                address = address & SPACE_ADDRESS_MASK;

                //  Check if multisampling is enabled.
                if (!state.multiSampling)
                {
                    //  Write clear color.
                    //switch(state.colorBufferFormat)
                    switch(state.rtFormat[0])
                    {
                        case GPU_RGBA8888:

                            *((u64bit *) &memory[address + 0]) = *((u64bit *) &clearColor8bit[0]);
                            *((u64bit *) &memory[address + 8]) = *((u64bit *) &clearColor8bit[8]);
                            break;

                        case GPU_RG16F:

                            *((u32bit *) &memory[address +  0]) = *((u32bit *) &clearColor16bit[0]);
                            *((u32bit *) &memory[address +  4]) = *((u32bit *) &clearColor16bit[2]);
                            *((u32bit *) &memory[address +  8]) = *((u32bit *) &clearColor16bit[0]);
                            *((u32bit *) &memory[address + 12]) = *((u32bit *) &clearColor16bit[2]);
                            break;

                        case GPU_R32F:

                            *((u32bit *) &memory[address +  0]) = *((u32bit *) &clearColor32bit[0]);
                            *((u32bit *) &memory[address +  4]) = *((u32bit *) &clearColor32bit[0]);
                            *((u32bit *) &memory[address +  8]) = *((u32bit *) &clearColor32bit[0]);
                            *((u32bit *) &memory[address + 12]) = *((u32bit *) &clearColor32bit[0]);
                            break;

                        case GPU_RGBA16:

                            *((u64bit *) &memory[address +  0]) = *((u64bit *) &clearColor16bit[24]);
                            *((u64bit *) &memory[address +  8]) = *((u64bit *) &clearColor16bit[16]);
                            *((u64bit *) &memory[address + 16]) = *((u64bit *) &clearColor16bit[8]);
                            *((u64bit *) &memory[address + 24]) = *((u64bit *) &clearColor16bit[0]);
                            break;

                        case GPU_RGBA16F:

                            *((u64bit *) &memory[address +  0]) = *((u64bit *) &clearColor16bit[0]);
                            *((u64bit *) &memory[address +  8]) = *((u64bit *) &clearColor16bit[8]);
                            *((u64bit *) &memory[address + 16]) = *((u64bit *) &clearColor16bit[16]);
                            *((u64bit *) &memory[address + 24]) = *((u64bit *) &clearColor16bit[24]);
                            break;
                    }
                }
                else
                {
                    for(u32bit s = 0; s < (STAMP_FRAGMENTS * state.msaaSamples); s++)
                    {
                        //  Write clear color.
                        //switch(state.colorBufferFormat)
                        switch(state.rtFormat[0])
                        {
                            case GPU_RGBA8888:

                                *((u32bit *) &memory[address + s * 4]) = *((u32bit *) &clearColor8bit[0]);
                                break;

                            case GPU_RG16F:

                                *((u32bit *) &memory[address + s * 4]) = *((u32bit *) &clearColor16bit[0]);
                                break;

                            case GPU_R32F:

                                *((u32bit *) &memory[address + s * 4]) = *((u32bit *) &clearColor32bit[0]);
                                break;

                            case GPU_RGBA16:
                            case GPU_RGBA16F:

                                *((u64bit *) &memory[address + s * 8]) = *((u64bit *) &clearColor16bit[0]);
                                break;
                        }
                    }
                }
            }
        }
    }
    
    GLOBALPROFILER_EXITREGION()
}


//  Converts color data from RGBA8 format to RGBA32F format.
void GPUEmulator::colorRGBA8ToRGBA32F(u8bit *in, QuadFloat *out)
{
    u32bit i;

    //  Convert all pixels in the stamp.
    for(i = 0; i < STAMP_FRAGMENTS; i++)
    {
        //  Convert 8 bit normalized color components to 32 bit float point color components.
        out[i][0] = f32bit(in[i * 4]) * (1.0f / 255.0f);
        out[i][1] = f32bit(in[i * 4 + 1]) * (1.0f / 255.0f);
        out[i][2] = f32bit(in[i * 4 + 2]) * (1.0f / 255.0f);
        out[i][3] = f32bit(in[i * 4 + 3]) * (1.0f / 255.0f);
    }
}

//  Converts color data from RGBA16 format to RGBA32F format.
void GPUEmulator::colorRGBA16ToRGBA32F(u8bit *in, QuadFloat *out)
{
    u32bit i;

    //  Convert all pixels in the stamp.
    for(i = 0; i < STAMP_FRAGMENTS; i++)
    {
        //  Convert 16 bit normalized color components to 32 bit float point color components.
        out[i][0] = f32bit(((u16bit *) in)[i * 4 + 0]) * (1.0f / 65535.0f);
        out[i][1] = f32bit(((u16bit *) in)[i * 4 + 1]) * (1.0f / 65535.0f);
        out[i][2] = f32bit(((u16bit *) in)[i * 4 + 2]) * (1.0f / 65535.0f);
        out[i][3] = f32bit(((u16bit *) in)[i * 4 + 3]) * (1.0f / 65535.0f);
    }
}

//  Converts color data in RGB32F format to RGBA8 format.
void GPUEmulator::colorRGBA32FToRGBA8(QuadFloat *in, u8bit *out)
{
    u32bit i;

    //  Convert all pixels in the stamp.
    for(i = 0; i < STAMP_FRAGMENTS; i++)
    {
        //  Convert 32bit float point color components to 8 bit normalized color components.
        out[i * 4] = u8bit(255.0f * GPU_CLAMP(in[i][0], 0.0f, 1.0f));
        out[i * 4 + 1] = u8bit(255.0f * GPU_CLAMP(in[i][1], 0.0f, 1.0f));
        out[i * 4 + 2] = u8bit(255.0f * GPU_CLAMP(in[i][2], 0.0f, 1.0f));
        out[i * 4 + 3] = u8bit(255.0f * GPU_CLAMP(in[i][3], 0.0f, 1.0f));
    }
}

//  Converts color data in RGB32F format to RGBA16 format.
void GPUEmulator::colorRGBA32FToRGBA16(QuadFloat *in, u8bit *out)
{
    u32bit i;

    //  Convert all pixels in the stamp.
    for(i = 0; i < STAMP_FRAGMENTS; i++)
    {
        //  Convert 32bit float point color components to 16 bit normalized color components.
        ((u16bit *) out)[i * 4 + 0] = u16bit(65535.0f * GPU_CLAMP(in[i][0], 0.0f, 1.0f));
        ((u16bit *) out)[i * 4 + 1] = u16bit(65535.0f * GPU_CLAMP(in[i][1], 0.0f, 1.0f));
        ((u16bit *) out)[i * 4 + 2] = u16bit(65535.0f * GPU_CLAMP(in[i][2], 0.0f, 1.0f));
        ((u16bit *) out)[i * 4 + 3] = u16bit(65535.0f * GPU_CLAMP(in[i][3], 0.0f, 1.0f));
    }
}

//  Converts color data from RGBA16F format to RGBA32F format.
void GPUEmulator::colorRGBA16FToRGBA32F(u8bit *in, QuadFloat *out)
{
    u32bit i;

    //  Convert all pixels in the stamp.
    for(i = 0; i < STAMP_FRAGMENTS; i++)
    {
        //  Convert 16 bit float point color components to 32 bit float point color components.
        out[i][0] = GPUMath::convertFP16ToFP32(((u16bit *) in)[i * 4]);
        out[i][1] = GPUMath::convertFP16ToFP32(((u16bit *) in)[i * 4 + 1]);
        out[i][2] = GPUMath::convertFP16ToFP32(((u16bit *) in)[i * 4 + 2]);
        out[i][3] = GPUMath::convertFP16ToFP32(((u16bit *) in)[i * 4 + 3]);
    }
}

//  Converts color data from RG16F format to RGBA32F format.
void GPUEmulator::colorRG16FToRGBA32F(u8bit *in, QuadFloat *out)
{
    u32bit i;

    //  Convert all pixels in the stamp.
    for(i = 0; i < STAMP_FRAGMENTS; i++)
    {
        //  Convert 16 bit float point color components to 32 bit float point color components.
        out[i][0] = GPUMath::convertFP16ToFP32(((u16bit *) in)[i * 2]);
        out[i][1] = GPUMath::convertFP16ToFP32(((u16bit *) in)[i * 2 + 1]);
        out[i][2] = 0.0f;
        out[i][3] = 1.0f;
    }
}

//  Converts color data in RGB32F format to RG16F format.
void GPUEmulator::colorRGBA32FToRG16F(QuadFloat *in, u8bit *out)
{
    u32bit i;

    //  Convert all pixels in the stamp.
    for(i = 0; i < STAMP_FRAGMENTS; i++)
    {
        //  Convert 32 bit float point color components to 16 bit float point color components.
        ((u16bit *) out)[i * 2]     = GPUMath::convertFP32ToFP16(in[i][0]);
        ((u16bit *) out)[i * 2 + 1] = GPUMath::convertFP32ToFP16(in[i][1]);
    }
}

//  Converts color data from R32F format to RGBA32F format.
void GPUEmulator::colorR32FToRGBA32F(u8bit *in, QuadFloat *out)
{
    u32bit i;

    //  Convert all pixels in the stamp.
    for(i = 0; i < STAMP_FRAGMENTS; i++)
    {
        //  Convert 16 bit float point color components to 32 bit float point color components.
        out[i][0] = ((f32bit *) in)[i];
        out[i][1] = 0.0f;
        out[i][2] = 0.0f;
        out[i][3] = 1.0f;
    }
}

//  Converts color data in RGB32F format to R32F format.
void GPUEmulator::colorRGBA32FToR32F(QuadFloat *in, u8bit *out)
{
    u32bit i;

    //  Convert all pixels in the stamp.
    for(i = 0; i < STAMP_FRAGMENTS; i++)
    {
        //  Convert 32 bit float point color components to 16 bit float point color components.
        ((f32bit *) out)[i] = in[i][0];
    }
}

//  Converts color data in RGB32F format to RGBA8 format.  */
void GPUEmulator::colorRGBA32FToRGBA16F(QuadFloat *in, u8bit *out)
{
    u32bit i;

    //  Convert all pixels in the stamp.
    for(i = 0; i < STAMP_FRAGMENTS; i++)
    {
        //  Convert 32 bit float point color components to 16 bit float point color components.
        ((u16bit *) out)[i * 4]     = GPUMath::convertFP32ToFP16(in[i][0]);
        ((u16bit *) out)[i * 4 + 1] = GPUMath::convertFP32ToFP16(in[i][1]);
        ((u16bit *) out)[i * 4 + 2] = GPUMath::convertFP32ToFP16(in[i][2]);
        ((u16bit *) out)[i * 4 + 3] = GPUMath::convertFP32ToFP16(in[i][3]);
    }
}

#define GAMMA(x) f32bit(GPU_POWER(f64bit(x), f64bit(1.0f / 2.2f)))
#define LINEAR(x) f32bit(GPU_POWER(f64bit(x), f64bit(2.2f)))
//#define GAMMA(x) (x)

//  Converts color data from linear space to sRGB space.
void GPUEmulator::colorLinearToSRGB(QuadFloat *in)
{
    for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
    {
        //  NOTE: Alpha shouldn't be affected by the color space conversion.

        //  Convert from linear space to sRGB space.
        in[f][0] = GAMMA(in[f][0]);
        in[f][1] = GAMMA(in[f][1]);
        in[f][2] = GAMMA(in[f][2]);
    }
}

//  Converts color data from sRGB space to linear space.
void GPUEmulator::colorSRGBToLinear(QuadFloat *in)
{
    for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
    {
        //  NOTE: Alpha shouldn't be affected by the color space conversion.

        //  Convert from sRGB space to linear space.
        in[f][0] = LINEAR(in[f][0]);
        in[f][1] = LINEAR(in[f][1]);
        in[f][2] = LINEAR(in[f][2]);
    }
}

//  Writes the current color buffer as a ppm file.
void GPUEmulator::dumpFrame(char *filename, u32bit rt, bool dumpAlpha)
{
    u32bit address;
    s32bit x,y;

    static u8bit *data = 0;
    static u32bit dataSize = 0;
    
    //  Check if the buffer is large enough.
    if (dataSize < (state.displayResX * state.displayResY * 4))
    {
        //  Check if the old buffer must be deleted.
        if (data != NULL)
            delete[] data;
            
        //  Set the new buffer size.            
        dataSize = state.displayResX * state.displayResY * 4;
        
        //  Create the new buffer.
        data = new u8bit[dataSize];
    }

/*#else

    FILE *fout;

    //  Add file extension.
    char filenameAux[256];
    sprintf(filenameAux, "%s.ppm", filename);
    
    //  Open/Create the file for the current frame.
    fout = fopen(filenameAux, "wb");

    //  Check if the file was correctly created.
    GPU_ASSERT(
        if (fout == NULL)
            panic("GPUEmulator", "dumpFrame", "Error creating frame color output file.");
    )

    //  Write file header.

    //  Write magic number.
    fprintf(fout, "P6\n");

    //  Write frame size.
    fprintf(fout, "%d %d\n", state.displayResX, state.displayResY);

    //  Write color component maximum value.
    fprintf(fout, "255\n");

#endif*/

    u32bit samples = state.multiSampling ? state.msaaSamples : 1;
    u32bit bytesPixel;

    switch(state.rtFormat[rt])
    {
        case GPU_RGBA8888:
        case GPU_RG16F:
        case GPU_R32F:
            bytesPixel = 4;
            break;
        case GPU_RGBA16:
        case GPU_RGBA16F:
            bytesPixel = 8;
            break;
    }

    pixelMapper[rt].setupDisplay(state.displayResX, state.displayResY, STAMP_WIDTH, STAMP_WIDTH,
                                simP.ras.genWidth / STAMP_WIDTH, simP.ras.genHeight / STAMP_HEIGHT,
                                simP.ras.scanWidth / simP.ras.genWidth, simP.ras.scanHeight / simP.ras.genHeight,
                                simP.ras.overScanWidth, simP.ras.overScanHeight,
                                samples, bytesPixel);

    u8bit red;
    u8bit green;
    u8bit blue;
    u8bit alpha;

    u8bit *memory = selectMemorySpace(state.rtAddress[rt]);

    //  Check if multisampling is enabled.
    if (!state.multiSampling)
    {
        s32bit top = state.d3d9PixelCoordinates ? 0 : (state.displayResY - 1);
        s32bit bottom = state.d3d9PixelCoordinates ? (state.displayResY - 1) : 0;
        s32bit nextLine = state.d3d9PixelCoordinates ? +1 : -1;

        //  Do this for the whole picture now.
        for (y = top; y != (bottom + nextLine); y = y + nextLine)
        {
            for (x = 0; x < s32bit(state.displayResX); x++)
            {
                //  Calculate address from pixel position.
                address = pixelMapper[rt].computeAddress(x, y);
                address += state.rtAddress[rt];

                //  Get address inside the memory space.
                address = address & SPACE_ADDRESS_MASK;

                //  Convert data from the color color buffer to 8-bit PPM format.
                switch(state.rtFormat[rt])
                {
                    case GPU_RGBA8888:
                        red   = memory[address];
                        green = memory[address + 1];
                        blue  = memory[address + 2];
                        alpha = memory[address + 3];
                        break;

                    case GPU_RG16F:
                        red   = u8bit(GPU_MIN(GPU_MAX(GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + 0])), 0.0f), 1.0f) * 255.0f);
                        green = u8bit(GPU_MIN(GPU_MAX(GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + 2])), 0.0f), 1.0f) * 255.0f);
                        blue  = 0;
                        alpha = 0;
                        break;

                    case GPU_R32F:
                        red   = u8bit(GPU_MIN(GPU_MAX(*((f32bit *) &memory[address]), 0.0f), 1.0f) * 255.0f);
                        green = 0;
                        blue  = 0;
                        alpha = 0;
                        break;

                    case GPU_RGBA16:
                        red   = u8bit(GPU_MIN(GPU_MAX((f32bit(*((u16bit *) &memory[address + 0])) / 65535.0f), 0.0f), 1.0f) * 255.0f);
                        green = u8bit(GPU_MIN(GPU_MAX((f32bit(*((u16bit *) &memory[address + 2])) / 65535.0f), 0.0f), 1.0f) * 255.0f);
                        blue  = u8bit(GPU_MIN(GPU_MAX((f32bit(*((u16bit *) &memory[address + 4])) / 65535.0f), 0.0f), 1.0f) * 255.0f);
                        alpha = u8bit(GPU_MIN(GPU_MAX((f32bit(*((u16bit *) &memory[address + 6])) / 65535.0f), 0.0f), 1.0f) * 255.0f);
                        break;

                    case GPU_RGBA16F:
                        red   = u8bit(GPU_MIN(GPU_MAX(GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + 0])), 0.0f), 1.0f) * 255.0f);
                        green = u8bit(GPU_MIN(GPU_MAX(GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + 2])), 0.0f), 1.0f) * 255.0f);
                        blue  = u8bit(GPU_MIN(GPU_MAX(GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + 4])), 0.0f), 1.0f) * 255.0f);
                        alpha = u8bit(GPU_MIN(GPU_MAX(GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + 6])), 0.0f), 1.0f) * 255.0f);
                        break;
                }

                if (!dumpAlpha)
                {
                    u32bit yImage = state.d3d9PixelCoordinates ? y : (state.displayResY - 1) - y;
                    data[(yImage * state.displayResX + x) * 4 + 2] = red;
                    data[(yImage * state.displayResX + x) * 4 + 1] = green;
                    data[(yImage * state.displayResX + x) * 4 + 0] = blue;
                    //data[(yImage * state.displayResX + x) * 4 + 3] = alpha;
                    data[(yImage * state.displayResX + x) * 4 + 3] = 255;
                }
                else
                {
                    u32bit yImage = state.d3d9PixelCoordinates ? y : (state.displayResY - 1) - y;
                    data[(yImage * state.displayResX + x) * 4 + 0] = 
                    data[(yImage * state.displayResX + x) * 4 + 1] = 
                    data[(yImage * state.displayResX + x) * 4 + 2] = 
                    data[(yImage * state.displayResX + x) * 4 + 3] = alpha;
                }
            }
        }
    }
    else
    {
        QuadFloat sampleColors[MAX_MSAA_SAMPLES];
        QuadFloat resolvedColor;
        QuadFloat referenceColor;
        QuadFloat currentColor;

        s32bit top = state.d3d9PixelCoordinates ? 0 : (state.displayResY - 1);
        s32bit bottom = state.d3d9PixelCoordinates ? (state.displayResY - 1) : 0;
        s32bit nextLine = state.d3d9PixelCoordinates ? +1 : -1;

        //  Do this for the whole picture now.
        for (y = top; y != (bottom + nextLine); y = y + nextLine)
        {
            for(x = 0; x < s32bit(state.displayResX); x++)
            {

                address = pixelMapper[rt].computeAddress(x, y);

                //  Calculate address for the first sample in the pixel.
                address += state.rtAddress[rt];

                //  Zero resolved color.
                resolvedColor[0] = 0.0f;
                resolvedColor[1] = 0.0f;
                resolvedColor[2] = 0.0f;
                resolvedColor[3] = 0.0f;

                QuadFloat referenceColor;

                //  Convert data from the color color buffer 32 bit fp internal format.
                switch(state.rtFormat[rt])
                {
                    case GPU_RGBA8888:
                        referenceColor[0] = f32bit(memory[address + 0]) / 255.0f;
                        referenceColor[1] = f32bit(memory[address + 1]) / 255.0f;
                        referenceColor[2] = f32bit(memory[address + 2]) / 255.0f;
                        referenceColor[3] = f32bit(memory[address + 3]) / 255.0f;
                        break;

                    case GPU_RG16F:
                        referenceColor[0] = GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + 0]));
                        referenceColor[1] = GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + 2]));
                        referenceColor[2] = 0.0f;
                        referenceColor[3] = 0.0f;
                        break;

                    case GPU_R32F:
                        referenceColor[0] = *((f32bit *) &memory[address]);
                        referenceColor[1] = 0.0f;
                        referenceColor[2] = 0.0f;
                        referenceColor[3] = 0.0f;
                        break;

                    case GPU_RGBA16:
                        referenceColor[0] = f32bit(*((u16bit *) &memory[address + 0])) / 65535.0f;
                        referenceColor[1] = f32bit(*((u16bit *) &memory[address + 2])) / 65535.0f;
                        referenceColor[2] = f32bit(*((u16bit *) &memory[address + 6])) / 65535.0f;
                        referenceColor[3] = f32bit(*((u16bit *) &memory[address + 8])) / 65535.0f;
                        break;

                    case GPU_RGBA16F:
                        referenceColor[0] = GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + 0]));
                        referenceColor[1] = GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + 2]));
                        referenceColor[2] = GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + 4]));
                        referenceColor[3] = GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + 6]));
                        break;
                }

                bool fullCoverage = true;

                //  Accumulate the color for all the samples in the pixel
                for(u32bit i = 0; i < state.msaaSamples; i++)
                {
                    //  Convert data from the color color buffer 32 bit fp internal format.
                    switch(state.rtFormat[rt])
                    {
                        case GPU_RGBA8888:
                            currentColor[0] = f32bit(memory[address + i * 4 + 0]) / 255.0f;
                            currentColor[1] = f32bit(memory[address + i * 4 + 1]) / 255.0f;
                            currentColor[2] = f32bit(memory[address + i * 4 + 2]) / 255.0f;
                            currentColor[3] = f32bit(memory[address + i * 4 + 3]) / 255.0f;
                            break;

                        case GPU_RG16F:
                            currentColor[0] = GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + i * 4 + 0]));
                            currentColor[1] = GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + i * 4 + 2]));
                            currentColor[2] = 0.0f;
                            currentColor[3] = 0.0f;
                            break;

                        case GPU_R32F:
                            currentColor[0] = *((f32bit *) &memory[address + i * 4]);
                            currentColor[1] = 0.0f;
                            currentColor[2] = 0.0f;
                            currentColor[3] = 0.0f;
                            break;

                        case GPU_RGBA16:
                            currentColor[0] = f32bit(*((u16bit *) &memory[address + i * 8 + 0])) / 65535.0f;
                            currentColor[1] = f32bit(*((u16bit *) &memory[address + i * 8 + 2])) / 65535.0f;
                            currentColor[2] = f32bit(*((u16bit *) &memory[address + i * 8 + 4])) / 65535.0f;
                            currentColor[3] = f32bit(*((u16bit *) &memory[address + i * 8 + 6])) / 65535.0f;
                            break;


                        case GPU_RGBA16F:
                            currentColor[0] = GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + i * 8 + 0]));
                            currentColor[1] = GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + i * 8 + 2]));
                            currentColor[2] = GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + i * 8 + 4]));
                            currentColor[3] = GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + i * 8 + 6]));
                            break;
                    }

                    sampleColors[i][0] = currentColor[0];
                    sampleColors[i][1] = currentColor[1];
                    sampleColors[i][2] = currentColor[2];
                    sampleColors[i][3] = currentColor[3];

                    resolvedColor[0] += LINEAR(currentColor[0]);
                    resolvedColor[1] += LINEAR(currentColor[1]);
                    resolvedColor[2] += LINEAR(currentColor[2]);
                    //resolvedColor[3] += LINEAR(currentColor[3]);
                    resolvedColor[3] += currentColor[3];

                    fullCoverage = fullCoverage && (referenceColor[0] == currentColor[0])
                                                && (referenceColor[1] == currentColor[1])
                                                && (referenceColor[2] == currentColor[2]);
                }


                //  Check if there is a single sample for the pixel
                if (fullCoverage)
                {
                    //  Convert from RGBA32F (internal format) to RGBA8 (PPM format)
                    red   = u8bit(GPU_MIN(GPU_MAX(referenceColor[0], 0.0f), 1.0f) * 255.0f);
                    green = u8bit(GPU_MIN(GPU_MAX(referenceColor[1], 0.0f), 1.0f) * 255.0f);
                    blue  = u8bit(GPU_MIN(GPU_MAX(referenceColor[2], 0.0f), 1.0f) * 255.0f);
                    alpha = u8bit(GPU_MIN(GPU_MAX(referenceColor[3], 0.0f), 1.0f) * 255.0f);

                }
                else
                {
                    resolvedColor[0] = GAMMA(resolvedColor[0] / f32bit(state.msaaSamples));
                    resolvedColor[1] = GAMMA(resolvedColor[1] / f32bit(state.msaaSamples));
                    resolvedColor[2] = GAMMA(resolvedColor[2] / f32bit(state.msaaSamples));
                    //resolvedColor[3] = GAMMA(resolvedColor[3] / f32bit(state.msaaSamples));
                    resolvedColor[3] = resolvedColor[3] / f32bit(state.msaaSamples);

                    //  Convert from RGBA32F (internal format) to RGBA8 (PPM format)
                    red   = u8bit(GPU_MIN(GPU_MAX(resolvedColor[0], 0.0f), 1.0f) * 255.0f);
                    green = u8bit(GPU_MIN(GPU_MAX(resolvedColor[1], 0.0f), 1.0f) * 255.0f);
                    blue  = u8bit(GPU_MIN(GPU_MAX(resolvedColor[2], 0.0f), 1.0f) * 255.0f);
                    alpha = u8bit(GPU_MIN(GPU_MAX(resolvedColor[3], 0.0f), 1.0f) * 255.0f);
                }

                if (!dumpAlpha)
                {
                    u32bit yImage = state.d3d9PixelCoordinates ? y : (state.displayResY - 1) - y;
                    data[(yImage * state.displayResX + x) * 4 + 2] = red;
                    data[(yImage * state.displayResX + x) * 4 + 1] = green;
                    data[(yImage * state.displayResX + x) * 4 + 0] = blue;
                    //data[(yImage * state.displayResX + x) * 4 + 3] = alpha;
                    data[(yImage * state.displayResX + x) * 4 + 3] = 255;
                }
                else
                {
                    u32bit yImage = state.d3d9PixelCoordinates ? y : (state.displayResY - 1) - y;
                    data[(yImage * state.displayResX + x) * 4 + 0] = 
                    data[(yImage * state.displayResX + x) * 4 + 1] = 
                    data[(yImage * state.displayResX + x) * 4 + 2] = 
                    data[(yImage * state.displayResX + x) * 4 + 3] = alpha;
                }
            }
        }
    }

    ImageSaver::getInstance().savePNG(filename, state.displayResX, state.displayResY, data);
}

//  Writes the current depth buffer as a ppm file.
void GPUEmulator::dumpDepthBuffer(char *filename)
{
    u32bit address;
    s32bit x,y;

    static u8bit *depthData = 0;
    static u32bit depthDataSize = 0;
    
    //  Check if the buffer is large enough.
    if (depthDataSize < (state.displayResX * state.displayResY * 4))
    {
        //  Check if the old buffer must be deleted.
        if (depthData != NULL)
            delete[] depthData;
            
        //  Set the new buffer size.            
        depthDataSize = state.displayResX * state.displayResY * 4;
        
        //  Create the new buffer.
        depthData = new u8bit[depthDataSize];
    }

/*#else

    FILE *fout;
    
    //  Add file extension.
    char filenameAux[256];
    sprintf(filenameAux, "%s.ppm", filename);

    //  Open/Create the file for the current frame.
    fout = fopen(filename, "wb");

    //  Check if the file was correctly created.
    GPU_ASSERT(
        if (fout == NULL)
            panic("GPUEmulator", "dumpDepthFrame", "Error creating frame color output file.");
    )

    //  Write file header.

    //  Write magic number.
    fprintf(fout, "P6\n");

    //  Write frame size.
    fprintf(fout, "%d %d\n", state.displayResX, state.displayResY);

    //  Write color component maximum value.
    fprintf(fout, "255\n");

#endif*/

    u32bit samples = state.multiSampling ? state.msaaSamples : 1;
    u32bit bytesPixel;

    bytesPixel = 4;

    zPixelMapper.setupDisplay(state.displayResX, state.displayResY, STAMP_WIDTH, STAMP_WIDTH,
                             simP.ras.genWidth / STAMP_WIDTH, simP.ras.genHeight / STAMP_HEIGHT,
                             simP.ras.scanWidth / simP.ras.genWidth, simP.ras.scanHeight / simP.ras.genHeight,
                             simP.ras.overScanWidth, simP.ras.overScanHeight,
                             samples, bytesPixel);

    u8bit red;
    u8bit green;
    u8bit blue;
    //u8bit zval;

    u8bit *memory = selectMemorySpace(state.zStencilBufferBaseAddr);

    //  Check if multisampling is enabled.
    if (!state.multiSampling)
    {
        s32bit top = state.d3d9PixelCoordinates ? 0 : (state.displayResY - 1);
        s32bit bottom = state.d3d9PixelCoordinates ? (state.displayResY - 1) : 0;
        s32bit nextLine = state.d3d9PixelCoordinates ? +1 : -1;

        //  Do this for the whole picture now.
        for (y = top; y != (bottom + nextLine); y = y + nextLine)
        {
            for (x = 0; x < s32bit(state.displayResX); x++)
            {
                //  Calculate address from pixel position.
                address = zPixelMapper.computeAddress(x, y);
                address += state.zStencilBufferBaseAddr;

                //  Get address inside the memory space.
                address = address & SPACE_ADDRESS_MASK;

                //red   = memory[address];
                //green = memory[address + 1];
                //blue  = memory[address + 2];

                // Invert for better reading with irfan view.
                red   = memory[address + 2];
                green = memory[address + 1];
                blue  = memory[address + 0];
                //zval = u8bit(f32bit(*((u32bit *)&(memory[address + 0]))  & 0x00FFFFFF) / 16777215.0f * 255.f);  //f32bit(*((u32bit *) data) & 0x00FFFFFF) / 16777215.0f;

                u32bit yImage = state.d3d9PixelCoordinates ? y : (state.displayResY - 1) - y;
                depthData[(yImage * state.displayResX + x) * 4 + 2] = red;
                depthData[(yImage * state.displayResX + x) * 4 + 1] = green;
                depthData[(yImage * state.displayResX + x) * 4 + 0] = blue;
                //depthData[(yImage * state.displayResX + x) * 4 + 3] = 0;
                depthData[(yImage * state.displayResX + x) * 4 + 3] = 255;
            }
        }
    }
    else
    {
        panic("GPUEmulator", "dumpDepthBuffer", "MSAA not implemented.");

        s32bit top = state.d3d9PixelCoordinates ? 0 : (state.displayResY - 1);
        s32bit bottom = state.d3d9PixelCoordinates ? (state.displayResY - 1) : 0;
        s32bit nextLine = state.d3d9PixelCoordinates ? +1 : -1;

        //  Do this for the whole picture now.
        for (y = top; y != (bottom + nextLine); y = y + nextLine)
        {
            for(x = 0; x < s32bit(state.displayResX); x++)
            {
                QuadFloat resolvedColor;

                u32bit sampleX;
                u32bit sampleY;

                sampleX = x - GPU_MOD(x, STAMP_WIDTH);
                sampleY = y - GPU_MOD(y, STAMP_HEIGHT);

                address = pixelMapper[0].computeAddress(sampleX, sampleY);

                //  Calculate address for the first sample in the pixel.
                address = address + state.msaaSamples * (GPU_MOD(y, STAMP_HEIGHT) * STAMP_WIDTH + GPU_MOD(x, STAMP_WIDTH)) * bytesPixel;
                address += state.backBufferBaseAddr;

                //  Zero resolved color.
                resolvedColor[0] = 0.0f;
                resolvedColor[1] = 0.0f;
                resolvedColor[2] = 0.0f;

                QuadFloat referenceColor;

                //  Convert data from the color color buffer 32 bit fp internal format.
                switch(state.colorBufferFormat)
                {
                    case GPU_RGBA8888:
                        referenceColor[0] = f32bit(memory[address + 0]) / 255.0f;
                        referenceColor[1] = f32bit(memory[address + 1]) / 255.0f;
                        referenceColor[2] = f32bit(memory[address + 2]) / 255.0f;
                        break;

                    case GPU_RGBA16:
                        referenceColor[0] = f32bit(*((u16bit *) &memory[address + 0])) / 65535.0f;
                        referenceColor[1] = f32bit(*((u16bit *) &memory[address + 2])) / 65535.0f;
                        referenceColor[2] = f32bit(*((u16bit *) &memory[address + 4])) / 65535.0f;
                        referenceColor[3] = f32bit(*((u16bit *) &memory[address + 6])) / 65535.0f;
                        break;

                    case GPU_RGBA16F:
                        referenceColor[0] = GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + 0]));
                        referenceColor[1] = GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + 2]));
                        referenceColor[2] = GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + 4]));
                        break;
                }

                bool fullCoverage = true;

                //  Accumulate the color for all the samples in the pixel
                for(u32bit i = 0; i < state.msaaSamples; i++)
                {
                    QuadFloat currentColor;

                    //  Convert data from the color color buffer 32 bit fp internal format.
                    switch(state.colorBufferFormat)
                    {
                        case GPU_RGBA8888:
                            currentColor[0] = f32bit(memory[address + i * 4 + 0]) / 255.0f;
                            currentColor[1] = f32bit(memory[address + i * 4 + 1]) / 255.0f;
                            currentColor[2] = f32bit(memory[address + i * 4 + 2]) / 255.0f;
                            break;

                        case GPU_RGBA16:
                            currentColor[0] = f32bit(*((u16bit *) &memory[address + i * 8 + 0])) / 65535.0f;
                            currentColor[1] = f32bit(*((u16bit *) &memory[address + i * 8 + 2])) / 65535.0f;
                            currentColor[2] = f32bit(*((u16bit *) &memory[address + i * 8 + 4])) / 65535.0f;
                            break;

                        case GPU_RGBA16F:
                            currentColor[0] = GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + i * 8 + 0]));
                            currentColor[1] = GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + i * 8 + 2]));
                            currentColor[2] = GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + i * 8 + 4]));
                            break;
                    }

                    resolvedColor[0] += currentColor[0];
                    resolvedColor[1] += currentColor[1];
                    resolvedColor[2] += currentColor[2];

                    fullCoverage = fullCoverage && (referenceColor[0] == currentColor[0])
                                                && (referenceColor[1] == currentColor[1])
                                                && (referenceColor[2] == currentColor[2]);
                }

                //  Check if there is a single sample for the pixel
                if (fullCoverage)
                {
                    //  Convert from RGBA32F (internal format) to RGBA8 (PPM format)
                    red   = u8bit(referenceColor[0] * 255.0f);
                    green = u8bit(referenceColor[1] * 255.0f);
                    blue  = u8bit(referenceColor[2] * 255.0f);

                    u32bit yImage = state.d3d9PixelCoordinates ? y : (state.displayResY - 1) - y;
                    depthData[(yImage * state.displayResX + x) * 4 + 2] = red;
                    depthData[(yImage * state.displayResX + x) * 4 + 1] = green;
                    depthData[(yImage * state.displayResX + x) * 4 + 0] = blue;
                    depthData[(yImage * state.displayResX + x) * 4 + 3] = 0;
                }
                else
                {
                    //  Resolve color as the average of all the sample colors.
                    resolvedColor[0] = GAMMA(resolvedColor[0] / f32bit(state.msaaSamples));
                    resolvedColor[1] = GAMMA(resolvedColor[1] / f32bit(state.msaaSamples));
                    resolvedColor[2] = GAMMA(resolvedColor[2] / f32bit(state.msaaSamples));

                    u32bit yImage = state.d3d9PixelCoordinates ? y : (state.displayResY - 1) - y;
                    depthData[(yImage * state.displayResX + x) * 4 + 2] = red;
                    depthData[(yImage * state.displayResX + x) * 4 + 1] = green;
                    depthData[(yImage * state.displayResX + x) * 4 + 0] = blue;
                    depthData[(yImage * state.displayResX + x) * 4 + 3] = 0;
                }
            }
        }
    }

    ImageSaver::getInstance().savePNG(filename, state.displayResX, state.displayResY, depthData);
}

//  Writes the current depth buffer as a ppm file.
void GPUEmulator::dumpStencilBuffer(char *filename)
{
    u32bit address;
    s32bit x,y;

    static u8bit *stencilData = 0;
    static u32bit stencilDataSize = 0;
    
    //  Check if the buffer is large enough.
    if (stencilDataSize < (state.displayResX * state.displayResY * 4))
    {
        //  Check if the old buffer must be deleted.
        if (stencilData != NULL)
            delete[] stencilData;
            
        //  Set the new buffer size.            
        stencilDataSize = state.displayResX * state.displayResY * 4;
        
        //  Create the new buffer.
        stencilData = new u8bit[stencilDataSize];
    }

/*#else

    FILE *fout;
    
    //  Add file extension.
    char filenameAux[256];
    sprintf(filenameAux, "%s.ppm", filename);
    //  Open/Create the file for the current frame.
    fout = fopen(filename, "wb");

    //  Check if the file was correctly created.
    GPU_ASSERT(
        if (fout == NULL)
            panic("GPUEmulator", "dumpStencilBuffer", "Error creating frame color output file.");
    )

    //  Write file header.

    //  Write magic number.
    fprintf(fout, "P6\n");

    //  Write frame size.
    fprintf(fout, "%d %d\n", state.displayResX, state.displayResY);

    //  Write color component maximum value.
    fprintf(fout, "255\n");

#endif*/

    u32bit samples = state.multiSampling ? state.msaaSamples : 1;
    u32bit bytesPixel;

    bytesPixel = 4;

    zPixelMapper.setupDisplay(state.displayResX, state.displayResY, STAMP_WIDTH, STAMP_WIDTH,
                             simP.ras.genWidth / STAMP_WIDTH, simP.ras.genHeight / STAMP_HEIGHT,
                             simP.ras.scanWidth / simP.ras.genWidth, simP.ras.scanHeight / simP.ras.genHeight,
                             simP.ras.overScanWidth, simP.ras.overScanHeight,
                             samples, bytesPixel);

    u8bit red;
    u8bit green;
    u8bit blue;

    u8bit *memory = selectMemorySpace(state.zStencilBufferBaseAddr);

    //  Check if multisampling is enabled.
    if (!state.multiSampling)
    {
        s32bit top = state.d3d9PixelCoordinates ? 0 : (state.displayResY - 1);
        s32bit bottom = state.d3d9PixelCoordinates ? (state.displayResY - 1) : 0;
        s32bit nextLine = state.d3d9PixelCoordinates ? +1 : -1;

        //  Do this for the whole picture now.
        for (y = top; y != (bottom + nextLine); y = y + nextLine)
        {
            for (x = 0; x < s32bit(state.displayResX); x++)
            {
                //  Calculate address from pixel position.
                address = zPixelMapper.computeAddress(x, y);
                address += state.zStencilBufferBaseAddr;

                //  Get address inside the memory space.
                address = address & SPACE_ADDRESS_MASK;

                //  Put some color to make small differences more easy to discover.
                red   = memory[address + 3];
                green = red & 0xF0;
                blue  = (red & 0x0F) << 4;

                u32bit yImage = state.d3d9PixelCoordinates ? y : (state.displayResY - 1) - y;
                stencilData[(yImage * state.displayResX + x) * 4 + 2] = red;
                stencilData[(yImage * state.displayResX + x) * 4 + 1] = green;
                stencilData[(yImage * state.displayResX + x) * 4 + 0] = blue;
                //stencilData[(yImage * state.displayResX + x) * 4 + 3] = 0;
                stencilData[(yImage * state.displayResX + x) * 4 + 3] = 255;
            }
        }
    }
    else
    {
        panic("GPUEmulator", "dumpStencilBuffer", "MSAA not implemented.");

        s32bit top = state.d3d9PixelCoordinates ? 0 : (state.displayResY - 1);
        s32bit bottom = state.d3d9PixelCoordinates ? (state.displayResY - 1) : 0;
        s32bit nextLine = state.d3d9PixelCoordinates ? +1 : -1;

        //  Do this for the whole picture now.
        for (y = top; y != (bottom + nextLine); y = y + nextLine)
        {
            for(x = 0; x < s32bit(state.displayResX); x++)
            {
                QuadFloat resolvedColor;

                u32bit sampleX;
                u32bit sampleY;

                sampleX = x - GPU_MOD(x, STAMP_WIDTH);
                sampleY = y - GPU_MOD(y, STAMP_HEIGHT);

                address = pixelMapper[0].computeAddress(sampleX, sampleY);

                //  Calculate address for the first sample in the pixel.
                address = address + state.msaaSamples * (GPU_MOD(y, STAMP_HEIGHT) * STAMP_WIDTH + GPU_MOD(x, STAMP_WIDTH)) * bytesPixel;
                address += state.backBufferBaseAddr;

                //  Zero resolved color.
                resolvedColor[0] = 0.0f;
                resolvedColor[1] = 0.0f;
                resolvedColor[2] = 0.0f;

                QuadFloat referenceColor;

                //  Convert data from the color color buffer 32 bit fp internal format.
                switch(state.colorBufferFormat)
                {
                    case GPU_RGBA8888:
                        referenceColor[0] = f32bit(memory[address + 0]) / 255.0f;
                        referenceColor[1] = f32bit(memory[address + 1]) / 255.0f;
                        referenceColor[2] = f32bit(memory[address + 2]) / 255.0f;
                        break;

                    case GPU_RGBA16:
                        referenceColor[0] = f32bit(*((u16bit *) &memory[address + 0])) / 65535.0f;
                        referenceColor[1] = f32bit(*((u16bit *) &memory[address + 2])) / 65535.0f;
                        referenceColor[2] = f32bit(*((u16bit *) &memory[address + 4])) / 65535.0f;
                        break;

                    case GPU_RGBA16F:
                        referenceColor[0] = GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + 0]));
                        referenceColor[1] = GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + 2]));
                        referenceColor[2] = GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + 4]));
                        break;
                }

                bool fullCoverage = true;

                //  Accumulate the color for all the samples in the pixel
                for(u32bit i = 0; i < state.msaaSamples; i++)
                {
                    QuadFloat currentColor;

                    //  Convert data from the color color buffer 32 bit fp internal format.
                    switch(state.colorBufferFormat)
                    {
                        case GPU_RGBA8888:
                            currentColor[0] = f32bit(memory[address + i * 4 + 0]) / 255.0f;
                            currentColor[1] = f32bit(memory[address + i * 4 + 1]) / 255.0f;
                            currentColor[2] = f32bit(memory[address + i * 4 + 2]) / 255.0f;
                            break;

                        case GPU_RGBA16:
                            currentColor[0] = f32bit(*((u16bit *) &memory[address + i * 8 + 0])) / 65535.0f;
                            currentColor[1] = f32bit(*((u16bit *) &memory[address + i * 8 + 2])) / 65535.0f;
                            currentColor[2] = f32bit(*((u16bit *) &memory[address + i * 8 + 4])) / 65535.0f;
                            break;

                        case GPU_RGBA16F:
                            currentColor[0] = GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + i * 8 + 0]));
                            currentColor[1] = GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + i * 8 + 2]));
                            currentColor[2] = GPUMath::convertFP16ToFP32(*((u16bit *) &memory[address + i * 8 + 4]));
                            break;
                    }

                    resolvedColor[0] += currentColor[0];
                    resolvedColor[1] += currentColor[1];
                    resolvedColor[2] += currentColor[2];

                    fullCoverage = fullCoverage && (referenceColor[0] == currentColor[0])
                                                && (referenceColor[1] == currentColor[1])
                                                && (referenceColor[2] == currentColor[2]);
                }

                //  Check if there is a single sample for the pixel
                if (fullCoverage)
                {
                    //  Convert from RGBA32F (internal format) to RGBA8 (PPM format)
                    red   = u8bit(referenceColor[0] * 255.0f);
                    green = u8bit(referenceColor[1] * 255.0f);
                    blue  = u8bit(referenceColor[2] * 255.0f);

                    u32bit yImage = state.d3d9PixelCoordinates ? y : (state.displayResY - 1) - y;
                    stencilData[(yImage * state.displayResX + x) * 4 + 2] = red;
                    stencilData[(yImage * state.displayResX + x) * 4 + 1] = green;
                    stencilData[(yImage * state.displayResX + x) * 4 + 0] = blue;
                    stencilData[(yImage * state.displayResX + x) * 4 + 3] = 0;
                }
                else
                {
                    //  Resolve color as the average of all the sample colors.
                    resolvedColor[0] = GAMMA(resolvedColor[0] / f32bit(state.msaaSamples));
                    resolvedColor[1] = GAMMA(resolvedColor[1] / f32bit(state.msaaSamples));
                    resolvedColor[2] = GAMMA(resolvedColor[2] / f32bit(state.msaaSamples));

                    u32bit yImage = state.d3d9PixelCoordinates ? y : (state.displayResY - 1) - y;
                    stencilData[(yImage * state.displayResX + x) * 4 + 2] = red;
                    stencilData[(yImage * state.displayResX + x) * 4 + 1] = green;
                    stencilData[(yImage * state.displayResX + x) * 4 + 0] = blue;
                    stencilData[(yImage * state.displayResX + x) * 4 + 3] = 0;
                }
            }
        }
    }

    ImageSaver::getInstance().savePNG(filename, state.displayResX, state.displayResY, stencilData);
}


//  Load the current vertex program in the shader emulator.
void GPUEmulator::loadVertexProgram()
{
    u8bit *code;

    u8bit *memory = selectMemorySpace(state.vertexProgramAddr);
    u32bit address = state.vertexProgramAddr & SPACE_ADDRESS_MASK;

    code = &memory[address];

    GPU_DEBUG(
        printf("Loading vertex program from address %08x to PC %04x size %d\n", state.vertexProgramAddr,
            state.vertexProgramStartPC, state.vertexProgramSize);
        bool endProgram = false;
        u32bit nopsAtTheEnd = 0;
        for(u32bit i = 0; i < (state.vertexProgramSize / ShaderInstruction::SHINSTRSIZE); i++)
        {
            ShaderInstruction shInstr(&code[i * ShaderInstruction::SHINSTRSIZE]);

            if (!endProgram)
            {
                char dis[1024];
                shInstr.disassemble(dis);
                printf(" %04x : ", state.vertexProgramStartPC + i);
                for(u32bit b = 0; b < ShaderInstruction::SHINSTRSIZE; b++)
                    printf("%02x ", code[i * ShaderInstruction::SHINSTRSIZE + b]);
                printf(" : %s\n", dis);
                fflush(stdout);
                endProgram = shInstr.getEndFlag();
            }
            else
            {
                if (shInstr.getOpcode() != NOP)
                    panic("GPUEmulator", "loadVertexProgram", "Unexpected instruction after end of program.");

                nopsAtTheEnd++;
            }
        }
        printf("Trail NOPs = %d\n", nopsAtTheEnd);
    )

    shEmu->loadShaderProgram(code, state.vertexProgramStartPC, state.vertexProgramSize, VERTEX_PARTITION);
}

//  Load the current fragment program in the shader emulator.
void GPUEmulator::loadFragmentProgram()
{
    u8bit *code;

    u8bit *memory = selectMemorySpace(state.fragProgramAddr);
    u32bit address = state.fragProgramAddr & SPACE_ADDRESS_MASK;

    code = &memory[address];

    GPU_DEBUG(
        printf("Loading fragment program from address %08x to PC %04x size %d\n", state.fragProgramAddr,
            state.fragProgramStartPC, state.fragProgramSize);
        bool endProgram = false;
        u32bit nopsAtTheEnd = 0;
        for(u32bit i = 0; i < (state.fragProgramSize / ShaderInstruction::SHINSTRSIZE); i++)
        {
            ShaderInstruction shInstr(&code[i * ShaderInstruction::SHINSTRSIZE]);

            if (!endProgram)
            {
                char dis[1024];
                shInstr.disassemble(dis);
                printf(" %04x : ", state.fragProgramStartPC + i);
                for(u32bit b = 0; b < ShaderInstruction::SHINSTRSIZE; b++)
                    printf("%02x ", code[i * ShaderInstruction::SHINSTRSIZE + b]);
                printf(" : %s\n", dis);
                fflush(stdout);
                endProgram = shInstr.getEndFlag();
            }
            else
            {
                if (shInstr.getOpcode() != NOP)
                    panic("GPUEmulator", "loadShaderProgram", "Unexpected instruction after end of program.");

                nopsAtTheEnd++;
            }
        }
        printf("Trail NOPs = %d\n", nopsAtTheEnd);
    )

    shEmu->loadShaderProgram(code, state.fragProgramStartPC, state.fragProgramSize, FRAGMENT_PARTITION);

}

//  Load the shader program in the shader emulator.
void GPUEmulator::loadShaderProgram()
{
    u8bit *code;

    u8bit *memory = selectMemorySpace(state.programAddress);
    u32bit address = state.programAddress & SPACE_ADDRESS_MASK;

    code = &memory[address];

    GPU_DEBUG(
        printf("Loading shader program from address %08x to PC %04x size %d\n", state.programAddress,
            state.programLoadPC, state.programSize);
        bool endProgram = false;
        u32bit nopsAtTheEnd = 0;
        for(u32bit i = 0; i < (state.programSize / ShaderInstruction::SHINSTRSIZE); i++)
        {
            ShaderInstruction shInstr(&code[i * ShaderInstruction::SHINSTRSIZE]);

            if (!endProgram)
            {
                char dis[1024];
                shInstr.disassemble(dis);
                printf(" %04x : ", state.programLoadPC + i);
                for(u32bit b = 0; b < ShaderInstruction::SHINSTRSIZE; b++)
                    printf("%02x ", code[i * ShaderInstruction::SHINSTRSIZE + b]);
                printf(" : %s\n", dis);
                endProgram = shInstr.getEndFlag();
            }
            else
            {
                if (shInstr.getOpcode() != NOP)
                    panic("GPUEmulator", "loadShaderProgram", "Unexpected instruction after end of program.");

                nopsAtTheEnd++;
            }
        }
        printf("Trail NOPs = %d\n", nopsAtTheEnd);
    )

    shEmu->loadShaderProgram(code, state.programLoadPC, state.programSize, FRAGMENT_PARTITION);

}

void GPUEmulator::clearZStencilBuffer()
{
    GLOBALPROFILER_ENTERREGION("clearZStencilBuffer", "", "clearZStencilBuffer")

    if (!skipBatchMode)
    {
        u8bit zStencilClear[4 * 4];

        for(u32bit p = 0; p < 4; p++)
            *((u32bit *) &zStencilClear[p * 4]) = (state.zBufferClear & 0x00FFFFFF) | ((state.stencilBufferClear && 0xFF) << 24);

        u32bit samples = state.multiSampling ? state.msaaSamples : 1;
        u32bit bytesPixel;

        bytesPixel = 4;

        zPixelMapper.setupDisplay(state.displayResX, state.displayResY, STAMP_WIDTH, STAMP_WIDTH,
                                  simP.ras.genWidth / STAMP_WIDTH, simP.ras.genHeight / STAMP_HEIGHT,
                                  simP.ras.scanWidth / simP.ras.genWidth, simP.ras.scanHeight / simP.ras.genHeight,
                                  simP.ras.overScanWidth, simP.ras.overScanHeight,
                                  samples, bytesPixel);

        u8bit *memory = selectMemorySpace(state.zStencilBufferBaseAddr);

        for(u32bit y = 0; y < state.displayResY; y += 2)
        {
            for(u32bit x = 0; x < state.displayResX; x += 2)
            {
                u32bit address = zPixelMapper.computeAddress(x, y);

                address += state.zStencilBufferBaseAddr;

                //  Get address inside the memory space.
                address = address & SPACE_ADDRESS_MASK;

                //  Check if multisampling is enabled.
                if (!state.multiSampling)
                {
                    //  Write z/stencil clear value.
                    *((u64bit *) &memory[address + 0]) = *((u64bit *) &zStencilClear[0]);
                    *((u64bit *) &memory[address + 8]) = *((u64bit *) &zStencilClear[8]);
                }
                else
                {
                    for(u32bit s = 0; s < (STAMP_FRAGMENTS * state.msaaSamples); s++)
                    {
                        //  Write z/stencil clear value..
                        *((u32bit *) &memory[address + s * 4]) = *((u32bit *) &zStencilClear[0]);
                    }
                }
            }
        }
    
    }
    
    GLOBALPROFILER_EXITREGION()
}

void GPUEmulator::resetState()
{
    state.statusRegister = GPU_ST_RESET;
    state.displayResX = 400;
    state.displayResY = 400;
    state.frontBufferBaseAddr = 0x00200000;
    state.backBufferBaseAddr =  0x00400000;
    state.zStencilBufferBaseAddr = 0x00000000;
    state.textureMemoryBaseAddr = 0x00000000;
    state.programMemoryBaseAddr = 0x00000000;

    state.vertexProgramAddr = 0;
    state.vertexProgramStartPC = 0;
    state.vertexProgramSize = 0;
    state.vertexThreadResources = 0;

    for(u32bit c = 0; c < MAX_VERTEX_CONSTANTS; c++)
    {
        state.vConstants[c][0] = 0.0f;
        state.vConstants[c][1] = 0.0f;
        state.vConstants[c][2] = 0.0f;
        state.vConstants[c][3] = 0.0f;
    }

    for(u32bit a = 0; a < MAX_VERTEX_ATTRIBUTES; a++)
    {
        state.outputAttribute[a] = false;
        state.attributeMap[a] = ST_INACTIVE_ATTRIBUTE;
    }

    for(u32bit s = 0; s < MAX_STREAM_BUFFERS; s++)
    {
        state.attrDefValue[s][0] = 0.0f;
        state.attrDefValue[s][1] = 0.0f;
        state.attrDefValue[s][2] = 0.0f;
        state.attrDefValue[s][3] = 1.0f;
        state.streamAddress[s] = 0x00000000;
        state.streamStride[s] = 0;
        state.streamElements[s] = 0;
        state.streamData[s] = SD_U32BIT;
        state.streamElements[s] = 0;
        state.streamFrequency[s] = 0;
        state.d3d9ColorStream[s] = false;
    }

    state.streamStart = 0;
    state.streamCount = 0;
    state.streamInstances = 1;
    state.indexedMode = true;
    state.indexStream = 0;
    state.attributeLoadBypass = false;

    state.primitiveMode = gpu3d::TRIANGLE;
    state.frustumClipping = false;

    for(u32bit c = 0; c < 6; c++)
    {
        state.userClip[c][0] = 0.0f;
        state.userClip[c][1] = 0.0f;
        state.userClip[c][2] = 0.0f;
        state.userClip[c][3] = 0.0f;
    }

    state.userClipPlanes = false;
    state.faceMode = gpu3d::GPU_CCW;
    state.cullMode = gpu3d::NONE;
    state.hzTest = false;
    state.earlyZ = true;

    state.d3d9PixelCoordinates = false;
    state.viewportIniX = 0;
    state.viewportIniY = 0;
    state.viewportHeight = 400;
    state.viewportWidth = 400;
    state.scissorTest = false;
    state.scissorIniX = 0;
    state.scissorIniY = 0;
    state.scissorHeight = 0;
    state.scissorWidth = 0;
    state.nearRange = 0.0f;
    state.farRange = 1.0f;
    state.d3d9DepthRange = false;
    state.slopeFactor = 0.0f;
    state.unitOffset = 0.0f;
    state.zBufferBitPrecission = 24;
    state.d3d9RasterizationRules = false;
    state.twoSidedLighting = false;
    state.multiSampling = false;
    state.msaaSamples = 1;

    for(u32bit a = 0; a < MAX_FRAGMENT_ATTRIBUTES; a++)
    {
        state.interpolation[a] = true;
        state.fragmentInputAttributes[a] = false;
    }

    state.fragProgramAddr = 0;
    state.fragProgramStartPC = 0;
    state.fragProgramSize = 0;
    state.fragThreadResources = 0;

    for(u32bit c = 0; c < MAX_FRAGMENT_CONSTANTS; c++)
    {
        state.fConstants[c][0] = 0.0f;
        state.fConstants[c][1] = 0.0f;
        state.fConstants[c][2] = 0.0f;
        state.fConstants[c][3] = 0.0f;
    }

    state.modifyDepth = false;

    for (u32bit t = 0; t < MAX_TEXTURES; t++)
    {
        state.textureEnabled[t] = false;
        state.textureMode[t] = GPU_TEXTURE2D;
        state.textureWidth[t] = 0;
        state.textureHeight[t] = 0;
        state.textureDepth[t] = 0;
        state.textureWidth2[t] = 0;
        state.textureHeight2[t] = 0;
        state.textureDepth2[t] = 0;
        state.textureBorder[t] = 0;
        state.textureFormat[t] = GPU_RGBA8888;
        state.textureReverse[t] = false;
        state.textD3D9ColorConv[t] = false;
        state.textD3D9VInvert[t] = false;
        state.textureCompr[t] = GPU_NO_TEXTURE_COMPRESSION;
        state.textureBlocking[t] = GPU_TXBLOCK_TEXTURE;
        state.textBorderColor[t][0] = 0.0f;
        state.textBorderColor[t][1] = 0.0f;
        state.textBorderColor[t][2] = 0.0f;
        state.textBorderColor[t][3] = 1.0f;
        state.textureWrapS[t] = GPU_TEXT_CLAMP;
        state.textureWrapT[t] = GPU_TEXT_CLAMP;
        state.textureWrapR[t] = GPU_TEXT_CLAMP;
        state.textureNonNormalized[t] = false;
        state.textureMinFilter[t] = GPU_NEAREST;
        state.textureMagFilter[t] = GPU_NEAREST;
        state.textureEnableComparison[t] = false;
        state.textureComparisonFunction[t] = GPU_LEQUAL;
        state.textureSRGB[t] = false;
        state.textureMinLOD[t] = 0.0f;
        state.textureMaxLOD[t] = 12.0f;
        state.textureLODBias[t] = 0.0f;
        state.textureMinLevel[t] = 0;
        state.textureMaxLevel[t] = 12;
        state.textureUnitLODBias[t] = 0.0f;
        state.maxAnisotropy[t] = 1;
    }

    state.zBufferClear = 0x00FFFFFF;
    state.stencilBufferClear = 0x00;
    state.zstencilStateBufferAddr = 0x00000000;
    state.stencilTest = false;
    state.stencilFunction = GPU_ALWAYS;
    state.stencilReference = 0x00;
    state.stencilTestMask = 0xFF;
    state.stencilUpdateMask = 0xFF;
    state.stencilFail = STENCIL_KEEP;
    state.depthFail = STENCIL_KEEP;
    state.depthPass = STENCIL_KEEP;
    state.depthTest = false;
    state.depthFunction = GPU_LESS;
    state.depthMask = true;
    state.zStencilCompression = true;

    state.colorBufferFormat = GPU_RGBA8888;
    state.colorCompression = true;
    state.colorSRGBWrite = false;

    for(u32bit r = 0; r < MAX_RENDER_TARGETS; r++)
    {
        state.rtEnable[r] = false;
        state.rtFormat[r] = GPU_R32F;
        state.rtAddress[r] = 0x600000 + 0x100000 * r;
        state.colorBlend[r] = false;
        state.blendEquation[r] = BLEND_FUNC_ADD;
        state.blendSourceRGB[r] = BLEND_ONE;
        state.blendDestinationRGB[r] = BLEND_ZERO;
        state.blendSourceAlpha[r] = BLEND_ONE;
        state.blendDestinationAlpha[r] = BLEND_ZERO;
        state.blendColor[r][0] = 0.0f;
        state.blendColor[r][1] = 0.0f;
        state.blendColor[r][2] = 0.0f;
        state.blendColor[r][3] = 1.0f;
        state.colorMaskR[r] = true;
        state.colorMaskG[r] = true;
        state.colorMaskB[r] = true;
        state.colorMaskA[r] = true;
    }

    //  The backbuffer is aliased to render target 0.
    state.rtEnable[0] = true;
    state.rtFormat[0] = state.colorBufferFormat;
    state.rtAddress[0] = state.backBufferBaseAddr;
    
    state.colorBufferClear[0] = 0.0f;
    state.colorBufferClear[1] = 0.0f;
    state.colorBufferClear[2] = 0.0f;
    state.colorBufferClear[3] = 1.0f;
    state.colorStateBufferAddr = 0x00000000;

    state.logicalOperation = false;
    state.logicOpFunction = LOGICOP_COPY;

    state.mcSecondInterleavingStartAddr = 0x00000000;
    state.blitIniX = 0;
    state.blitIniY = 0;
    state.blitXOffset = 0;
    state.blitYOffset = 0;
    state.blitWidth = 400;
    state.blitHeight = 400;
    state.blitTextureWidth2 = 9;
    state.blitDestinationAddress = 0x08000000;
    state.blitDestinationTextureFormat = GPU_RGBA8888;
    state.blitDestinationTextureBlocking = GPU_TXBLOCK_TEXTURE;

    texEmu->reset();
}

void GPUEmulator::draw()
{
    if (!skipBatchMode)
    {
        setupDraw();
        for(u32bit instance = 0; instance < state.streamInstances; instance++)
        {
            GLOBALPROFILER_ENTERREGION("emulateStreamer", "", "emulateStreamer")
            emulateStreamer(instance);
            GLOBALPROFILER_EXITREGION()
            GLOBALPROFILER_ENTERREGION("emulatePrimitiveAssembly", "", "emulatePrimitiveAssembly")
            emulatePrimitiveAssembly();
            GLOBALPROFILER_EXITREGION()
            cleanup();
        }
        printf("B");
        fflush(stdout);
    }
}

void GPUEmulator::emulateStreamer(u32bit instance)
{
    //  Obtain the size of the index data type in bytes (STRIDE IS NOT USED FOR INDEX STREAMS!!!).
    u32bit indexStreamDataSize;
    switch(state.streamData[state.indexStream])
    {
        case SD_UNORM8:
        case SD_SNORM8:
        case SD_UINT8:
        case SD_SINT8:
            indexStreamDataSize = 1;
            break;
        case SD_UNORM16:
        case SD_SNORM16:
        case SD_UINT16:
        case SD_SINT16:
        case SD_FLOAT16:
            indexStreamDataSize = 2;
            break;
        case SD_UNORM32:
        case SD_SNORM32:
        case SD_UINT32:
        case SD_SINT32:
        case SD_FLOAT32:
            indexStreamDataSize = 4;
            break;
    }

/*u32bit minIndex = 0xFFFFFFFF;
u32bit maxIndex = 0;
vector<u32bit> readIndices;
vector<u32bit> shadedIndices;*/

    u32bit currentIndex = state.streamStart;

    for(u32bit v = 0; v < state.streamCount; v++)
    {
        if (state.indexedMode)
        {
            //  Read the next index from the index buffer.
            u8bit *memory = selectMemorySpace(state.streamAddress[state.indexStream]);

            u32bit address = state.streamAddress[state.indexStream] + (state.streamStart + v) * indexStreamDataSize;

            //  Get address inside the memory space.
            address = address & SPACE_ADDRESS_MASK;

            switch(state.streamData[state.indexStream])
            {
                case SD_UINT8:
                case SD_UNORM8:
                    currentIndex = u32bit(memory[address]);
                    break;
                case SD_UINT16:
                case SD_UNORM16:
                    currentIndex = u32bit(*((u16bit *) &memory[address]));
                    break;
                case SD_UINT32:
                case SD_UNORM32:
                    currentIndex = *((u32bit *) &memory[address]);
                    break;
                case SD_SNORM8:
                case SD_SNORM16:
                case SD_SNORM32:
                case SD_FLOAT16:
                case SD_FLOAT32:
                case SD_SINT8:
                case SD_SINT16:
                case SD_SINT32:
                    panic("GPUEmulator", "emulateStreamer", "Format type not supported for index data..");
                    break;
                default:
                    panic("GPUEmulator", "emulateStreamer", "Undefined index data format type.");
                    break;
            }

            GPU_DEBUG(
                printf("Read index %d instance %d at address %08x from stream %d address %x stride %d format ", currentIndex, instance,
                    address, state.indexStream, state.streamAddress[state.indexStream], state.streamStride[state.indexStream]);
                switch(state.streamData[state.indexStream])
                {
                    case SD_UINT8:
                    case SD_UNORM8:    //  For compatibility with old library.
                        printf("SD_UINT8.\n");
                        break;
                    case SD_UINT16:
                    case SD_UNORM16:    //  For compatibility with old library.
                        printf("SD_UINT16.\n");
                        break;
                    case SD_UINT32:
                    case SD_UNORM32:    //  For compatibility with old library.
                        printf("SD_UINT32.\n");
                        break;
                    case SD_SNORM8:
                    case SD_SNORM16:
                    case SD_SNORM32:
                    case SD_FLOAT16:
                    case SD_FLOAT32:
                    case SD_SINT8:
                    case SD_SINT16:
                    case SD_SINT32:
                        panic("GPUEmulator", "emulateStreamer", "Format type not supported for index data..");
                        break;
                    default:
                        panic("GPUEmulator", "emulateStreamer", "Undefined index data format type.");
                        break;
                }
            )
        }
        else
        {
            GPU_DEBUG(
                printf("Generated index %d.\n", currentIndex);
            )
        }

        //  Add index to the index list.
        indexList.push_back(currentIndex);

        map<u32bit, ShadedVertex*>::iterator it;

/*if (state.indexedMode)
{
    readIndices.push_back(currentIndex);
}*/

        //  Search index in the vertex list.
        it = vertexList.find(currentIndex);

        //  Check if the vertex for the current index has been already shaded.
        if (it == vertexList.end())
        {
/*if (state.indexedMode)
{
    minIndex = GPU_MIN(minIndex, currentIndex);
    maxIndex = GPU_MAX(maxIndex, currentIndex);
    shadedIndices.push_back(currentIndex);
}*/

            //  Read input attributes for the vertex.
            QuadFloat attributes[MAX_VERTEX_ATTRIBUTES];

            for(u32bit a = 0; a < MAX_VERTEX_ATTRIBUTES; a++)
            {
                //  Check if the vertex input attribute is active.
                if (state.attributeMap[a] != ST_INACTIVE_ATTRIBUTE)
                {
                    //  Read attribute from the corresponding stream.
                    u32bit stream = state.attributeMap[a];
                    u32bit address = state.streamAddress[stream];
                    u8bit *memory = selectMemorySpace(address);

                    //  Check if the attribute is per-instance or per-index.
                    if (state.streamFrequency[stream] == 0)
                    {
                        //  Per-index attribute.
                        address = address + state.streamStride[stream] * currentIndex;
                    }
                    else
                    {
                        //  Per-instance attribute.
                        address = address + state.streamStride[stream] * (instance / state.streamFrequency[stream]);
                    }

                    //  Get address inside the memory space.
                    address = address & SPACE_ADDRESS_MASK;

                    u32bit e;
                    for(e = 0; e < state.streamElements[stream]; e++)
                    {
                        switch(state.streamData[stream])
                        {
                            case SD_UNORM8:
                                attributes[a][e] = f32bit(memory[address + 1 * e]) * (1.0f / 255.0f);
                                break;
                            case SD_SNORM8:
                                attributes[a][e] = f32bit(*((s8bit *) &memory[address + 1 * e])) * (1.0f / 127.0f);
                                break;
                            case SD_UNORM16:
                                attributes[a][e] = f32bit(*((u16bit *) &memory[address + 2 * e])) * (1.0f / 65535.0f);
                                break;
                            case SD_SNORM16:
                                attributes[a][e] = f32bit(*((s16bit *) &memory[address + 2 * e])) * (1.0f / 32767.0f);
                                break;
                            case SD_UNORM32:
                                attributes[a][e] = f32bit(*((u32bit *) &memory[address + 4 * e])) * (1.0f / 4294967295.0f);
                                break;
                            case SD_SNORM32:
                                attributes[a][e] = f32bit(*((s32bit *) &memory[address + 4 * e])) * (1.0f / 2147483647.0f);
                                break;
                            case SD_FLOAT16:
                                attributes[a][e] = GPUMath::convertFP16ToFP32(*((f16bit *) &memory[address + 2 * e]));
                                break;
                            case SD_FLOAT32:
                                attributes[a][e] = *((f32bit *) &memory[address + 4 * e]);
                                break;
                            case SD_UINT8:
                                attributes[a][e] = f32bit(memory[address + 1 * e]);
                                break;
                            case SD_SINT8:
                                attributes[a][e] = f32bit(*((s8bit *) &memory[address + 1 * e]));
                                break;
                            case SD_UINT16:
                                attributes[a][e] = f32bit(*((u16bit *) &memory[address + 2 * e]));
                                break;
                            case SD_SINT16:
                                attributes[a][e] = f32bit(*((s16bit *) &memory[address + 2 * e]));
                                break;
                            case SD_UINT32:
                                attributes[a][e] = f32bit(*((u32bit *) &memory[address + 4 * e]));
                                break;
                            case SD_SINT32:
                                attributes[a][e] = f32bit(*((s32bit *) &memory[address + 4 * e]));
                                break;
                            default:
                                panic("GPUEmulator", "emulateStreamer", "Undefined stream data format.");
                                break;
                        }

                        GPU_DEBUG(
                            printf("Read attribute %d element %d with value %f from stream %d address %x stride %d elements %d format ",
                                a, e, attributes[a][e], stream, state.streamAddress[stream], state.streamStride[stream],
                                state.streamElements[stream]);
                            switch(state.streamData[stream])
                            {
                                case SD_UNORM8:
                                    printf("SD_UNORM8.\n");
                                    break;
                                case SD_SNORM8:
                                    printf("SD_SNORM8.\n");
                                    break;
                                case SD_UNORM16:
                                    printf("SD_UNORM16.\n");
                                    break;
                                case SD_SNORM16:
                                    printf("SD_SNORM16.\n");
                                    break;
                                case SD_UNORM32:
                                    printf("SD_UNORM32.\n");
                                    break;
                                case SD_SNORM32:
                                    printf("SD_SNORM32.\n");
                                    break;
                                case SD_FLOAT16:
                                    printf("SD_FLOAT16.\n");
                                    break;
                                case SD_FLOAT32:
                                    printf("SD_FLOAT32.\n");
                                    break;
                                case SD_UINT8:
                                    printf("SD_UINT8.\n");
                                    break;
                                case SD_SINT8:
                                    printf("SD_SINT8.\n");
                                    break;
                                case SD_UINT16:
                                    printf("SD_UINT16.\n");
                                    break;
                                case SD_SINT16:
                                    printf("SD_SINT16.\n");
                                    break;
                                case SD_UINT32:
                                    printf("SD_UINT32.\n");
                                    break;
                                case SD_SINT32:
                                    printf("SD_SINT32.\n");
                                    break;
                                default:
                                    panic("GPUEmulator", "emulateStreamer", "Undefined format.");
                                    break;
                            }
                        )
                    }

                    //  Set default value for undefined attribute components.
                    for(; e < 4; e++)
                    {
                        attributes[a][e] = state.attrDefValue[a][e];

                        GPU_DEBUG(
                            printf("Setting default value for attribute %d element %d value %f\n",
                                a, e, attributes[a][e]);
                        )
                    }

                    //  Check if the D3D9 color order for the color components must be used.
                    if (state.d3d9ColorStream[stream])
                    {
                        //
                        //  The D3D9 color formats are stored in little endian order with the alpha in higher addresses:
                        //
                        //  For example:
                        //
                        //     D3DFMT_A8R8G8B8 is stored as B G R A
                        //     D3DFMT_X8R8G8B8 is stored as B G R X
                        //

                        f32bit red = attributes[a][2];
                        f32bit green = attributes[a][1];
                        f32bit blue = attributes[a][0];
                        f32bit alpha = attributes[a][3];

                        attributes[a][0] = red;
                        attributes[a][1] = green;
                        attributes[a][2] = blue;
                        attributes[a][3] = alpha;
                    }
                }
                else
                {
                    //  Set default value for inactive vertex input attribute.
                    attributes[a][0] = state.attrDefValue[a][0];
                    attributes[a][1] = state.attrDefValue[a][1];
                    attributes[a][2] = state.attrDefValue[a][2];
                    attributes[a][3] = state.attrDefValue[a][3];

                    GPU_DEBUG(
                        printf("Setting default value for attribute %d value {%f, %f, %f, %f}\n",
                            a, attributes[a][0], attributes[a][1], attributes[a][2], attributes[a][3]);
                    )
                }
            }

            //  Shade the vertex.
            ShadedVertex *vertex;
            vertex = new ShadedVertex(attributes);

            GPU_EMU_TRACE(
                if (traceLog || (traceBatch && (watchBatch == batchCounter)) || (traceVertex && (currentIndex == watchIndex)))
                {
                    printf("Vertex Shader input for batch %d index %d : \n", batchCounter, currentIndex);
                    for(u32bit a = 0; a < MAX_VERTEX_ATTRIBUTES; a++)
                    {
                        if (state.attributeMap[a] != ST_INACTIVE_ATTRIBUTE)
                            printf("i[%d] = {%f, %f, %f, %f}\n", a, attributes[a][0], attributes[a][1], attributes[a][2], attributes[a][3]);
                    }
                }
            )

            //  Check if validation mode is enabled.
            if (validationMode)
            {
                //  Copy the vertex information.
                VertexInputInfo vInfo;
                vInfo.vertexID.index = currentIndex;
                vInfo.vertexID.instance = instance;
                vInfo.differencesBetweenReads = false;
                vInfo.timesRead = 1;
                for(u32bit a = 0; a < MAX_VERTEX_ATTRIBUTES; a++)
                    vInfo.attributes[a] = attributes[a];
                
                //  Log the shaded vertex.    
                vertexInputLog.insert(make_pair(vInfo.vertexID, vInfo));
            }
            
            GPU_EMU_TRACE(
                traceVShader = traceLog || ((traceBatch && (batchCounter == watchBatch)) && (watchIndex == currentIndex)) ||
                    (traceVertex && (watchIndex == currentIndex));
            )
            
            emulateVertexShader(vertex);
            
            GPU_EMU_TRACE(
                traceVShader = false;
            )
            

            //  Clamp the color attribute to the [0, 1] range.
            QuadFloat *vattributes = vertex->getAttributes();
            vattributes[COLOR_ATTRIBUTE][0] = GPU_CLAMP(vattributes[COLOR_ATTRIBUTE][0], 0.0f, 1.0f);
            vattributes[COLOR_ATTRIBUTE][1] = GPU_CLAMP(vattributes[COLOR_ATTRIBUTE][1], 0.0f, 1.0f);
            vattributes[COLOR_ATTRIBUTE][2] = GPU_CLAMP(vattributes[COLOR_ATTRIBUTE][2], 0.0f, 1.0f);
            vattributes[COLOR_ATTRIBUTE][3] = GPU_CLAMP(vattributes[COLOR_ATTRIBUTE][3], 0.0f, 1.0f);

            GPU_EMU_TRACE(
                if (traceLog || (traceBatch && (watchBatch == batchCounter)) || (traceVertex && (watchIndex == currentIndex)))
                {
                    printf("Vertex Shader output for batch %d index %d : \n", batchCounter, currentIndex);
                    for(u32bit a = 0; a < MAX_VERTEX_ATTRIBUTES; a++)
                    {
                        if (state.outputAttribute[a])
                            printf("o[%d] = {%f, %f, %f, %f}\n", a, vattributes[a][0], vattributes[a][1], vattributes[a][2], vattributes[a][3]);
                    }
                }
            )

            //  Add the shaded vertex to the table of shaded vertices.
            vertexList.insert(make_pair(currentIndex, vertex));
            
            //  Check if validation mode is enabled.
            if (validationMode)
            {
                //  Copy the vertex information.
                ShadedVertexInfo vInfo;
                vInfo.vertexID.index = currentIndex;
                vInfo.vertexID.instance = instance;
                vInfo.differencesBetweenShading = false;
                vInfo.timesShaded = 1;
                for(u32bit a = 0; a < MAX_VERTEX_ATTRIBUTES; a++)
                    vInfo.attributes[a] = vattributes[a];
                
                //  Log the shaded vertex.    
                shadedVertexLog.insert(make_pair(vInfo.vertexID, vInfo));
            }
        }

        if (!state.indexedMode)
            currentIndex++;
    }

/*if (state.indexedMode)
{
    fprintf(fIndexList, "INDEXED DRAW Count = %d Range = [%d, %d] Range Count = %d Unique Indices = %d", state.streamCount,
        minIndex, maxIndex, maxIndex - minIndex + 1, shadedIndices.size());
    if (f32bit(maxIndex - minIndex + 1) < (shadedIndices.size() * 1.05))
        fprintf(fIndexList, "\n");
    else if (f32bit(maxIndex - minIndex + 1) < (shadedIndices.size() * 1.20))
        fprintf(fIndexList, " SPARSE\n");
    else
        fprintf(fIndexList, " VERY SPARSE\n");
    fprintf(fIndexList, " READ INDICES : \n");
    for(u32bit i = 0; i < readIndices.size(); i++)
       fprintf(fIndexList, "    %8d\n", readIndices[i]);
    fprintf(fIndexList, " SHADED INDICES : \n");
    for(u32bit i = 0; i < shadedIndices.size(); i++)
        fprintf(fIndexList, "    %8d\n", shadedIndices[i]);
}
else
    fprintf(fIndexList, "DRAW Count = %d Index Range = [%d, %d]\n",
        state.streamCount, state.streamStart, state.streamStart + state.streamCount - 1);
*/
}

u8bit *GPUEmulator::selectMemorySpace(u32bit address)
{
    u8bit *memory;

    //  Check address space.
    if ((address & ADDRESS_SPACE_MASK) == GPU_ADDRESS_SPACE)
        memory = gpuMemory;
    else if ((address & ADDRESS_SPACE_MASK) == SYSTEM_ADDRESS_SPACE)
        memory = sysMemory;
    else
        panic("GPUEmulator", "selectMemorySpaceFrame", "Undefined address space");

    return memory;
}

void GPUEmulator::emulateVertexShader(ShadedVertex *vertex)
{
    GLOBALPROFILER_ENTERREGION("emulateVertexShader", "", "emulateVertexShader")

    //  Initialize thread for the current vertex.
    shEmu->resetShaderState(0);

    //  Load the new shader input into the shader input register bank of the thread element in the shader emulator.
    shEmu->loadShaderState(0, gpu3d::IN, vertex->getAttributes());

    //  Set PC for the thread element in the shader emulator to the start PC of the vector thread.
    shEmu->setThreadPC(0, state.vertexProgramStartPC);

    bool programEnd = false;
    u32bit pc = state.vertexProgramStartPC;

    //  Execute all the instructions in the program.
    while (!programEnd)
    {
        ShaderInstruction::ShaderInstructionDecoded *shDecInstr;

        //  Fetch the instruction.
        shDecInstr = shEmu->fetchShaderInstruction(0, pc);

        GPU_ASSERT(
            char buffer[256];
            sprintf(buffer, "Error fetching vertex program instruction at %02x\n", pc);
            if (shDecInstr == NULL)
                panic("GPUEmulator", "emulateVertexShader", buffer);
        )

        GPU_DEBUG(
            char shInstrDisasm[256];
            shDecInstr->getShaderInstruction()->disassemble(shInstrDisasm);
            printf("VSh => Executing instruction @ %04x : %s\n", pc, shInstrDisasm);
        )

        GPU_EMU_TRACE(
            if (traceVShader)
            {
                char shInstrDisasm[256];
                shDecInstr->getShaderInstruction()->disassemble(shInstrDisasm);
                printf("     %04x : %s\n", pc, shInstrDisasm);
                
                printShaderInstructionOperands(shDecInstr);
            }
        )

        //  Execute instruction.
        shEmu->execShaderInstruction(shDecInstr);

        GPU_EMU_TRACE(
            if (traceVShader)
            {                        
                printShaderInstructionResult(shDecInstr);
                printf("-------------------\n");
                fflush(stdout);
            }
        )
       
        //  Process texture requests.  Texture requests are processed per fragment quad.
        TextureAccess *texAccess = shEmu->nextVertexTextureAccess();

        //  Check if there is a pending texture request from the previous shader instruction.
        if (texAccess != NULL)
        {
            //  Process the texture requests.
            emulateTextureUnit(texAccess);
        }

        bool jump = false;
        u32bit destPC = 0;
        
        //  Check for jump instructions.
        if (shDecInstr->getShaderInstruction()->isAJump())
        {
            //  Check if the jump is performed.
            jump = shEmu->checkJump(shDecInstr, 1, destPC);
        }
        
        //  Check if this is the last instruction in the program.
        programEnd = shDecInstr->getShaderInstruction()->getEndFlag();

        //  Update PC.
        if (jump)
            pc = destPC;
        else
            pc++;
    }

    //  Get output attributes for the vertex.
    shEmu->readShaderState(0, gpu3d::OUT, vertex->getAttributes());

    GPU_DEBUG(
        printf("VSh => Vertex Outputs :\n");
        for(u32bit a = 0; a < MAX_VERTEX_ATTRIBUTES; a++)
        {
            QuadFloat *attributes = vertex->getAttributes();
            printf(" OUT[%02d] = {%f, %f, %f, %f}\n", a, attributes[a][0], attributes[a][1], attributes[a][2], attributes[a][3]);
        }
    )

    GLOBALPROFILER_EXITREGION()
}

void GPUEmulator::emulatePrimitiveAssembly()
{
    ShadedVertex *vertex1, *vertex2, *vertex3;

    u32bit requiredVertices;

    u32bit remainingVertices = indexList.size();

    u32bit currentIndex;

    switch(state.primitiveMode)
    {
        case TRIANGLE:

            currentIndex = 0;
            requiredVertices = 3;
            break;

        case TRIANGLE_STRIP:
        case TRIANGLE_FAN:

            currentIndex = 2;
            remainingVertices -= 2;
            requiredVertices = 1;
            break;

        case QUAD:

            currentIndex = 0;
            requiredVertices = 4;
            break;

        case QUAD_STRIP:

            currentIndex = 3;
            remainingVertices -= 2;
            requiredVertices = 2;
            break;

        case LINE:
            panic("GPUEmulator", "emulatePrimitiveAssembly", "Line primitive not supported yet.");
            break;

        case LINE_FAN:
            panic("GPUEmulator", "emulatePrimitiveAssembly", "Line Fan primitive not supported yet.");
            break;

        case LINE_STRIP:
            panic("GPUEmulator", "emulatePrimitiveAssembly", "Line Strip primitive not supported yet.");
            break;

        case POINT:
            panic("GPUEmulator", "emulatePrimitiveAssembly", "Point primitive not supported yet.");
            break;

        default:
            panic("GPUEmulator", "emulatePrimitiveAssembly", "Primitive mode not supported.");
            break;
    }

    bool oddTriangle = true;

    while (remainingVertices >= requiredVertices)
    {
        switch(state.primitiveMode)
        {
            case TRIANGLE:

                //  Get three vertices.
                vertex1 = getVertex(indexList[currentIndex]);
                vertex2 = getVertex(indexList[currentIndex + 1]);
                vertex3 = getVertex(indexList[currentIndex + 2]);

                GPU_DEBUG(
                    printf("Assembled TRIANGLE with indices %d %d %d\n",
                        indexList[currentIndex], indexList[currentIndex + 1], indexList[currentIndex + 2]);
                )

                //  Check if all the vertices were found.
                GPU_ASSERT(
                    if ((vertex1 == NULL) || (vertex2 == NULL) || (vertex3 == NULL))
                        panic("GPUEmulator", "emulatePrimitiveAssembly", "Error assembling vertices.");
                )

                emulateRasterization(vertex1, vertex2, vertex3);

                currentIndex += 3;
                remainingVertices -= 3;

                break;

            case TRIANGLE_STRIP:

                //  Check odd/even triangle for propper vertex ordering.
                if (oddTriangle)
                {
                    //  Get three vertices.
                    vertex1 = getVertex(indexList[currentIndex - 2]);
                    vertex2 = getVertex(indexList[currentIndex - 1]);
                    vertex3 = getVertex(indexList[currentIndex]);

                    oddTriangle = false;

                    GPU_DEBUG(
                        printf("Assembled TRIANGLE(STRIP-odd) with indices %d %d %d\n",
                            indexList[currentIndex - 2], indexList[currentIndex - 1], indexList[currentIndex]);
                    )
                }
                else
                {
                    //  Get three vertices.
                    vertex1 = getVertex(indexList[currentIndex - 1]);
                    vertex2 = getVertex(indexList[currentIndex - 2]);
                    vertex3 = getVertex(indexList[currentIndex]);

                    oddTriangle = true;

                    GPU_DEBUG(
                        printf("Assembled TRIANGLE(STRIP-even) with indices %d %d %d\n",
                            indexList[currentIndex - 1], indexList[currentIndex - 2], indexList[currentIndex]);
                    )
                }

                emulateRasterization(vertex1, vertex2, vertex3);

                //  Check if all the vertices were found.
                GPU_ASSERT(
                    if ((vertex1 == NULL) || (vertex2 == NULL) || (vertex3 == NULL))
                        panic("GPUEmulator", "emulatePrimitiveAssembly", "Error assembling vertices.");
                )

                currentIndex++;
                remainingVertices -= 1;
                break;

            case TRIANGLE_FAN:

                //  Get three vertices.
                vertex1 = getVertex(indexList[0]);
                vertex2 = getVertex(indexList[currentIndex - 1]);
                vertex3 = getVertex(indexList[currentIndex]);

                GPU_DEBUG(
                    printf("Assembled TRIANGLE(FAN) with indices %d %d %d\n",
                        indexList[0], indexList[currentIndex - 1], indexList[currentIndex]);
                )

                emulateRasterization(vertex1, vertex2, vertex3);

                //  Check if all the vertices were found.
                GPU_ASSERT(
                    if ((vertex1 == NULL) || (vertex2 == NULL) || (vertex3 == NULL))
                        panic("GPUEmulator", "emulatePrimitiveAssembly", "Error assembling vertices.");
                )

                currentIndex++;
                remainingVertices -= 1;
                break;

            case QUAD:

                //  Get three vertices for the first triangle forming the quad.
                vertex1 = getVertex(indexList[currentIndex]);
                vertex2 = getVertex(indexList[currentIndex + 1]);
                vertex3 = getVertex(indexList[currentIndex + 3]);

                GPU_DEBUG(
                    printf("Assembled QUAD (1st TRIANGLE) with indices %d %d %d\n",
                        indexList[currentIndex], indexList[currentIndex + 1], indexList[currentIndex + 3]);
                )

                emulateRasterization(vertex1, vertex2, vertex3);

                //  Check if all the vertices were found.
                GPU_ASSERT(
                    if ((vertex1 == NULL) || (vertex2 == NULL) || (vertex3 == NULL))
                        panic("GPUEmulator", "emulatePrimitiveAssembly", "Error assembling vertices.");
                )

                //  Get three vertices for the second triangle forming the quad.
                vertex1 = getVertex(indexList[currentIndex + 1]);
                vertex2 = getVertex(indexList[currentIndex + 2]);
                vertex3 = getVertex(indexList[currentIndex + 3]);

                GPU_DEBUG(
                    printf("Assembled QUAD (2nd TRIANGLE) with indices %d %d %d\n",
                        indexList[currentIndex + 1], indexList[currentIndex + 2], indexList[currentIndex + 3]);
                )

                //  Check if all the vertices were found.
                GPU_ASSERT(
                    if ((vertex1 == NULL) || (vertex2 == NULL) || (vertex3 == NULL))
                        panic("GPUEmulator", "emulatePrimitiveAssembly", "Error assembling vertices.");
                )

                emulateRasterization(vertex1, vertex2, vertex3);

                currentIndex += 4;
                remainingVertices -= 4;
                break;

            case QUAD_STRIP:

                //  Get three vertices for the first triangle forming the quad.
                vertex1 = getVertex(indexList[currentIndex - 3]);
                vertex2 = getVertex(indexList[currentIndex - 2]);
                vertex3 = getVertex(indexList[currentIndex]);

                GPU_DEBUG(
                    printf("Assembled QUAD(STRIP) (1st TRIANGLE) with indices %d %d %d\n",
                        indexList[currentIndex - 3], indexList[currentIndex - 2], indexList[currentIndex]);
                )

                //  Check if all the vertices were found.
                GPU_ASSERT(
                    if ((vertex1 == NULL) || (vertex2 == NULL) || (vertex3 == NULL))
                        panic("GPUEmulator", "emulatePrimitiveAssembly", "Error assembling vertices.");
                )

                emulateRasterization(vertex1, vertex2, vertex3);

                //  Get three vertices for the second triangle forming the quad.
                vertex1 = getVertex(indexList[currentIndex - 1]);
                vertex2 = getVertex(indexList[currentIndex - 3]);
                vertex3 = getVertex(indexList[currentIndex]);

                GPU_DEBUG(
                    printf("Assembled QUAD-(STRIP) (2nd TRIANGLE) with indices %d %d %d\n",
                        indexList[currentIndex - 1], indexList[currentIndex - 3], indexList[currentIndex]);
                )

                //  Check if all the vertices were found.
                GPU_ASSERT(
                    if ((vertex1 == NULL) || (vertex2 == NULL) || (vertex3 == NULL))
                        panic("GPUEmulator", "emulatePrimitiveAssembly", "Error assembling vertices.");
                )

                emulateRasterization(vertex1, vertex2, vertex3);

                currentIndex += 2;
                remainingVertices -= 2;
                break;

            case LINE:
                panic("GPUEmulator", "emulatePrimitiveAssembly", "Line primitive not supported yet.");
                break;

            case LINE_FAN:
                panic("GPUEmulator", "emulatePrimitiveAssembly", "Line Fan primitive not supported yet.");
                break;

            case LINE_STRIP:
                panic("GPUEmulator", "emulatePrimitiveAssembly", "Line Strip primitive not supported yet.");
                break;

            case POINT:
                panic("GPUEmulator", "emulatePrimitiveAssembly", "Point primitive not supported yet.");
                break;

            default:
                panic("GPUEmulator", "emulatePrimitiveAssembly", "Primitive mode not supported.");
                break;
        }
    }
}

GPUEmulator::ShadedVertex *GPUEmulator::getVertex(u32bit index)
{
    map<u32bit, GPUEmulator::ShadedVertex *>::iterator it;
    it = vertexList.find(index);

    if (it == vertexList.end())
        return NULL;
    else
        return it->second;
}


void GPUEmulator::emulateRasterization(ShadedVertex *vertex1, ShadedVertex *vertex2, ShadedVertex *vertex3)
{
    GLOBALPROFILER_ENTERREGION("emulateRasterization", "", "emulateRasterization")

    QuadFloat *attribV1 = vertex1->getAttributes();
    QuadFloat *attribV2 = vertex2->getAttributes();
    QuadFloat *attribV3 = vertex3->getAttributes();

    if (ClipperEmulator::trivialReject(attribV1[0], attribV2[0], attribV3[0], state.d3d9DepthRange))
    {
        //  Cull the triangle that is outside the frustrum.

        GPU_DEBUG(
            printf("Culled triangle with positions :\n");
            printf("    v1 = (%f, %f, %f, %f)\n", attribV1[0][0], attribV1[0][1], attribV1[0][2], attribV1[0][3]);
            printf("    v2 = (%f, %f, %f, %f)\n", attribV2[0][0], attribV2[0][1], attribV2[0][2], attribV2[0][3]);
            printf("    v3 = (%f, %f, %f, %f)\n", attribV3[0][0], attribV3[0][1], attribV3[0][2], attribV3[0][3]);
        )

    }
    else
    {
        //  Rasterize the triangle.

        GPU_DEBUG(
            printf("Rasterize triangle with positions :\n");
            printf("    v1 = (%f, %f, %f, %f)\n", attribV1[0][0], attribV1[0][1], attribV1[0][2], attribV1[0][3]);
            printf("    v2 = (%f, %f, %f, %f)\n", attribV2[0][0], attribV2[0][1], attribV2[0][2], attribV2[0][3]);
            printf("    v3 = (%f, %f, %f, %f)\n", attribV3[0][0], attribV3[0][1], attribV3[0][2], attribV3[0][3]);
        )

        QuadFloat *attributesVertex1 = new QuadFloat[MAX_VERTEX_ATTRIBUTES];
        QuadFloat *attributesVertex2 = new QuadFloat[MAX_VERTEX_ATTRIBUTES];
        QuadFloat *attributesVertex3 = new QuadFloat[MAX_VERTEX_ATTRIBUTES];

        for(u32bit a = 0; a < MAX_VERTEX_ATTRIBUTES; a++)
        {
            attributesVertex1[a] = attribV1[a];
            attributesVertex2[a] = attribV2[a];
            attributesVertex3[a] = attribV3[a];
        }

        //  Setup the triangle.
        u32bit triangleID = rastEmu->setup(attributesVertex1, attributesVertex2, attributesVertex3);

        GPU_DEBUG(
            printf("Triangle setup ID %d\n", triangleID);
        )

        //  Perform cull face test.
        bool dropTriangle = cullTriangle(triangleID);

        //  Check if the triangle must be culled.
        if (!dropTriangle)
        {
            //  Generate fragments for the triangle.

            //  Initiate rasterization of the triangle.
            u32bit batchID = rastEmu->startRecursiveMulti(&triangleID, 1, state.multiSampling);

            GPU_DEBUG(
                printf("Starting recursive algorithm\n");
            )

            bool lastFragment = false;

            GPU_DEBUG(
                printf("Updating recursive algorithm\n");
            )

            //  Update the triangle rasterization algorithm.
            rastEmu->updateRecursiveMultiv2(batchID);

            //  Process all the triangle fragments.
            while(!rastEmu->lastFragment(triangleID))
            {
                u32bit currentTriangleID;

                //  Get the next fragment quad for the triangle.
                Fragment **stamp = rastEmu->nextStampRecursiveMulti(batchID, currentTriangleID);

                GPU_DEBUG(
                    printf("Requested next fragment quad. Empty = %s\n", (stamp == NULL) ? "T" : "F");
                )

                //  Check if fragments were obtained.
                if (stamp != NULL)
                {
                    // Remove fragments outside the viewport or scissor windows.

                    //  Check if multisampling is enabled.
                    if (state.multiSampling)
                    {
                        //  Compute samples for all the fragments in the quad.
                        for(u32bit p = 0; p < STAMP_FRAGMENTS; p++)
                            rastEmu->computeMSAASamples(stamp[p], state.msaaSamples);
                    }

                    //  Create array of shaded fragments.
                    ShadedFragment *quad[4];

                    bool notAllFragmentsCulled = false;
                    bool culled[4];

                    //  Cull fragments and compute fragments attributes for the quad.
                    for(u32bit p = 0; p < STAMP_FRAGMENTS; p++)
                    {
                        //  Check for the last triangle fragment.
                        lastFragment = stamp[p]->isLastFragment();

                        //  Cull fragments outside the triangle.
                        culled[p] = !stamp[p]->isInsideTriangle();

                        //  Cull fragments outside the screen, the viewport or the scissor rectangle.
                        if (!culled[p])
                            culled[p] = (stamp[p]->getX() < 0) ||
                                     (stamp[p]->getX() >= s32bit(state.displayResX)) ||
                                     (stamp[p]->getY() < 0) ||
                                     (stamp[p]->getY() >= s32bit(state.displayResY)) ||
                                     (stamp[p]->getX() < state.viewportIniX) ||
                                     (stamp[p]->getX() >= (state.viewportIniX + s32bit(state.viewportWidth))) ||
                                     (stamp[p]->getY() < state.viewportIniY) ||
                                     (stamp[p]->getY() >= (state.viewportIniY + s32bit(state.viewportHeight))) ||
                                     (state.scissorTest &&
                                      ((stamp[p]->getX() < state.scissorIniX) ||
                                       (stamp[p]->getX() >= (state.scissorIniX + s32bit(state.scissorWidth))) ||
                                       (stamp[p]->getY() < state.scissorIniY) ||
                                       (stamp[p]->getY() >= (state.scissorIniY + s32bit(state.scissorHeight)))
                                      )
                                     );


                        notAllFragmentsCulled = notAllFragmentsCulled || !culled[p];

                        GPU_DEBUG(
                            if (culled[p])
                            {
                                if (!stamp[p]->isInsideTriangle())
                                    printf("Fragment at (%d, %d) has been culled.  Outside triangle.\n", stamp[p]->getX(), stamp[p]->getY());
                                else if ((stamp[p]->getX() < 0) ||
                                         (stamp[p]->getX() >= state.displayResX) ||
                                         (stamp[p]->getY() < 0) ||
                                         (stamp[p]->getY() >= state.displayResY))
                                    printf("Fragment at (%d, %d) has been culled.  Outside display.\n", stamp[p]->getX(), stamp[p]->getY());
                                else if ((stamp[p]->getX() < state.viewportIniX) ||
                                         (stamp[p]->getX() >= (state.viewportIniX + state.viewportWidth)) ||
                                         (stamp[p]->getY() < state.viewportIniY) ||
                                         (stamp[p]->getY() >= (state.viewportIniY + state.viewportHeight)))
                                    printf("Fragment at (%d, %d) has been culled.  Outside viewport.\n", stamp[p]->getX(), stamp[p]->getY());
                                else if (state.scissorTest &&
                                         ((stamp[p]->getX() < state.scissorIniX) ||
                                          (stamp[p]->getX() >= (state.scissorIniX + state.scissorWidth)) ||
                                          (stamp[p]->getY() < state.scissorIniY) ||
                                          (stamp[p]->getY() >= (state.scissorIniY + state.scissorHeight))))
                                    printf("Fragment at (%d, %d) has been culled.  Outside scissor rectangle.\n", stamp[p]->getX(), stamp[p]->getY());
                            }
                            else
                                printf("Fragment at (%d, %d) generated\n", stamp[p]->getX(), stamp[p]->getY());
                        )
                    }

                    //  Check if all the fragments in the quad are culled.
                    if (notAllFragmentsCulled)
                    {
                        GLOBALPROFILER_ENTERREGION("emulateRasterization(attribute interpolation)", "", "emulateRasterization")

                        //  Interpolate attributes for all the fragments.
                        for(u32bit p = 0; p < STAMP_FRAGMENTS; p++)
                        {
                            quad[p] = new ShadedFragment(stamp[p], culled[p]);

                            QuadFloat *attributes = quad[p]->getAttributes();

                            //  Interpolate fragment attributes.
                            for(u32bit a = 0; a < MAX_FRAGMENT_ATTRIBUTES; a++)
                            {
                                //  Check if the attribute is active.
                                if (state.fragmentInputAttributes[a])
                                {
                                    //  Check if attribute interpolation is enabled.
                                    if (state.interpolation[a])
                                    {
                                        //  Interpolate attribute;
                                        attributes[a] = rastEmu->interpolate(stamp[p], a);

                                        GPU_DEBUG(
                                            printf("Interpolating attribute %d to fragment attribute : {%f, %f, %f, %f}\n",
                                                a, attributes[a][0],  attributes[a][1],  attributes[a][2],  attributes[a][3]);
                                        )
                                    }
                                    else
                                    {
                                        //  Copy attribute from the triangle last vertex attribute.
                                        attributes[a] = rastEmu->copy(stamp[p], a, 2);

                                        GPU_DEBUG(
                                            printf("Copying attribute %d from vertex 2 to fragment attribute : {%f, %f, %f, %f}\n",
                                                a, attributes[a][0],  attributes[a][1],  attributes[a][2],  attributes[a][3]);
                                        )
                                    }
                                }
                                else
                                {
                                    GPU_DEBUG(
                                        printf("Setting default value to fragment attribute %d\n", a);
                                    )

                                    //  Set default attribute value.
                                    attributes[a][0] = 0.0f;
                                    attributes[a][1] = 0.0f;
                                    attributes[a][2] = 0.0f;
                                    attributes[a][3] = 0.0f;
                                }
                            }


                            //  Set position attribute (special case).
                            attributes[POSITION_ATTRIBUTE][0] = f32bit(stamp[p]->getX());
                            attributes[POSITION_ATTRIBUTE][1] = f32bit(stamp[p]->getY());
                            attributes[POSITION_ATTRIBUTE][2] = ((f32bit) stamp[p]->getZ()) / ((f32bit) ((1 << state.zBufferBitPrecission) - 1));
                            attributes[POSITION_ATTRIBUTE][3] = 0.0f;
                        
                            //  Set face (triangle area) attribute (special case)
                            attributes[FACE_ATTRIBUTE][3] = f32bit(stamp[p]->getTriangle()->getArea());
                        }

                        GLOBALPROFILER_EXITREGION()

                        bool watchPixelFound = false;
                        u32bit watchPixelPosInQuad = 0;
                        
                        GPU_EMU_TRACE(
                            if (traceLog || (traceBatch && (batchCounter == watchBatch)) || tracePixel)
                            {
                                u32bit watchPixelPosInQuad = 0;
                                while (!watchPixelFound && (watchPixelPosInQuad < STAMP_FRAGMENTS))
                                {
                                    Fragment *fr = quad[watchPixelPosInQuad]->getFragment();
                                    watchPixelFound = ((fr->getX() == watchPixelX) && (fr->getY() == watchPixelY));
                                    if (!watchPixelFound)
                                        watchPixelPosInQuad++;
                                }
                                if (watchPixelFound)
                                {
                                    printf("emuPrimAssembly => Cull flag for pixel (%d, %d) before zstencil is %s\n",
                                        watchPixelX, watchPixelY, quad[watchPixelPosInQuad]->isCulled() ? "True" : "False");
                                }
                            }
                        )
                        
                        //  Perform early z.
                        if (state.earlyZ && !state.modifyDepth)
                        {
                            emulateZStencilTest(quad);
                        }

                        GPU_EMU_TRACE(
                            if (traceLog || (traceBatch && (batchCounter == watchBatch)) || tracePixel)
                            {
                                if (watchPixelFound)
                                {
                                    printf("emuPrimAssembly => Cull flag for pixel (%d, %d) after zstencil (earlyz) is %s\n",
                                        watchPixelX, watchPixelY, quad[watchPixelPosInQuad]->isCulled() ? "True" : "False");

                                    traceFShader = true;
                                    traceTexture = true;
                                }
                            }
                        )
                        
                        //  Shade the fragment quad.
                        emulateFragmentShading(quad);

                        GPU_EMU_TRACE(
                            if (traceLog || (traceBatch && (batchCounter == watchBatch)) || tracePixel)
                            {
                                if (watchPixelFound)
                                {
                                    QuadFloat *attributes = quad[watchPixelPosInQuad]->getAttributes();
                                    printf("emuPrimAssembly => Output color for pixel (%d, %d) -> {%f, %f, %f, %f} [%02x, %02x, %02x, %02x]\n",
                                        watchPixelX, watchPixelY,
                                        attributes[COLOR_ATTRIBUTE][0], attributes[COLOR_ATTRIBUTE][1],
                                        attributes[COLOR_ATTRIBUTE][2], attributes[COLOR_ATTRIBUTE][3],
                                        u8bit(attributes[COLOR_ATTRIBUTE][0] * 255.0f), u8bit(attributes[COLOR_ATTRIBUTE][1] * 255.0f),
                                        u8bit(attributes[COLOR_ATTRIBUTE][2] * 255.0f), u8bit(attributes[COLOR_ATTRIBUTE][3] * 255.0f));

                                    traceFShader = false;
                                    traceTexture = false;
                                }
                            }
                        )
                        
                        //  Perform late z.
                        if (!state.earlyZ || state.modifyDepth)
                        {
                            emulateZStencilTest(quad);
                        }

                        GPU_EMU_TRACE(
                            if (traceLog || (traceBatch && (batchCounter == watchBatch)) || tracePixel)
                            {
                                if (watchPixelFound)
                                {
                                    printf("emuPrimAssembly => Cull flag for pixel (%d, %d) after zstencil (late z) is %s\n",
                                        watchPixelX, watchPixelY, quad[watchPixelPosInQuad]->isCulled() ? "True" : "False");
                                }
                            }
                        )

                        //  Write/combine the shaded pixel color in/with the current color buffer.
                        emulateColorWrite(quad);

                        //  Delete fragment quad.
                        for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
                        {
                            delete quad[f]->getFragment();
                            delete quad[f];
                        }
                    }
                    else
                    {
                        GPU_DEBUG(
                            printf("All fragments in the quad culled\n");
                        )

                        for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
                            delete stamp[f];
                    }

                    //  Delete the arrays of pointers to fragments for the current quad.
                    delete[] stamp;
                }
                else
                {
                    GPU_DEBUG(
                        printf("Updating recursive algorithm\n");
                    )

                    //  Update the triangle rasterization algorithm.
                    rastEmu->updateRecursiveMultiv2(batchID);
                }
            }
        }

        //  Eliminate triangle.
        rastEmu->destroyTriangle(triangleID);
    }

    //char filename[1024];
    //sprintf(filename, "frame%04d-batch%05d-triangle%09d", frameCounter, batchCounter, triangleCounter);
    //dumpFrame(filename);

    triangleCounter++;

    GLOBALPROFILER_EXITREGION()
}


void GPUEmulator::setupDraw()
{
    //  Configure rasterizer.
    rastEmu->setViewport(state.d3d9PixelCoordinates, state.viewportIniX, state.viewportIniY, state.viewportWidth, state.viewportHeight);
    rastEmu->setScissor(state.displayResX, state.displayResY, state.scissorTest, state.scissorIniX, state.scissorIniY,
                       state.scissorWidth, state.scissorHeight);
    rastEmu->setDepthRange(state.d3d9DepthRange, state.nearRange, state.farRange);
    rastEmu->setPolygonOffset(state.slopeFactor, state.unitOffset);
    rastEmu->setFaceMode(state.faceMode);
    rastEmu->setDepthPrecission(state.zBufferBitPrecission);
    rastEmu->setD3D9RasterizationRules(state.d3d9RasterizationRules);
    
    //  Configure the blend emulation for all render targets.
    for(u32bit rt = 0; rt < MAX_RENDER_TARGETS; rt++)
    {
        //  Configure the blend emulation for a render target.
        fragEmu->setBlending(rt, state.blendEquation[rt], state.blendSourceRGB[rt], state.blendSourceAlpha[rt],
                             state.blendDestinationRGB[rt], state.blendDestinationAlpha[rt], state.blendColor[rt]);
    }
    
    //  Configure logical operation emulation.
    fragEmu->setLogicOpMode(state.logicOpFunction);


    for(u32bit rt = 0; rt < MAX_RENDER_TARGETS; rt++)
    {
        //  Set display parameters in the Pixel Mapper.
        u32bit samples = state.multiSampling ? state.msaaSamples : 1;
        u32bit bytesPixel;

        switch(state.rtFormat[rt])
        {
            case GPU_RGBA8888:
            case GPU_RG16F:
            case GPU_R32F:
                bytesPixel = 4;
                break;
            case GPU_RGBA16:
            case GPU_RGBA16F:
                bytesPixel = 8;
                break;
        }

        pixelMapper[rt].setupDisplay(state.displayResX, state.displayResY, STAMP_WIDTH, STAMP_WIDTH,
                                 simP.ras.genWidth / STAMP_WIDTH, simP.ras.genHeight / STAMP_HEIGHT,
                                 simP.ras.scanWidth / simP.ras.genWidth, simP.ras.scanHeight / simP.ras.genHeight,
                                 simP.ras.overScanWidth, simP.ras.overScanHeight,
                                 samples, bytesPixel);
    
        bytesPerPixelColor[rt] = bytesPixel;
    }
    
    u32bit samples = state.multiSampling ? state.msaaSamples : 1;
    u32bit bytesPixel = 4;
    bytesPerPixelDepth = bytesPixel;
    zPixelMapper.setupDisplay(state.displayResX, state.displayResY, STAMP_WIDTH, STAMP_WIDTH,
                             simP.ras.genWidth / STAMP_WIDTH, simP.ras.genHeight / STAMP_HEIGHT,
                             simP.ras.scanWidth / simP.ras.genWidth, simP.ras.scanHeight / simP.ras.genHeight,
                             simP.ras.overScanWidth, simP.ras.overScanHeight,
                             samples, bytesPixel);

    //  Configure Z test emulation.
    fragEmu->configureZTest(state.depthFunction, state.depthMask);
    fragEmu->setZTest(state.depthTest);

    //  Configure stencil test emulation.
    fragEmu->configureStencilTest(state.stencilFunction, state.stencilReference,
        state.stencilTestMask, state.stencilUpdateMask, state.stencilFail, state.depthFail, state.depthPass);
    fragEmu->setStencilTest(state.stencilTest);

    //  Reset the triangle counter.
    triangleCounter = 0;
}

bool GPUEmulator::cullTriangle(u32bit triangleID)
{
    bool dropTriangle;
    f32bit tArea;

    //  Get the triangle signed area.
    tArea = rastEmu->triangleArea(triangleID);

    GPU_DEBUG(
        printf("Triangle area is %f\n", tArea);
    )

    //  Check face culling mode.
    switch(state.cullMode)
    {
        case NONE:

            //  If area is negative.
            if (tArea < 0)
            {
                //  Change the triangle edge equation signs so backfacing triangles can be rasterized.
                rastEmu->invertTriangleFacing(triangleID);

                GPU_DEBUG(
                    printf("Cull NONE.  Inverting back facing triangle.\n");
                )

            }

            //  Do not drop any triangle because of face.
            dropTriangle = false;

            break;

        case FRONT:

            //  If area is positive and front face culling.
            if (tArea > 0)
            {
                //  Drop front facing triangle.
                dropTriangle = true;

                GPU_DEBUG(
                    printf("Cull FRONT.  Culling front facing triangle.\n");
                )
            }
            else
            {
                //  Change the triangle edge equation signs so back facing triangles can be rasterized.
                rastEmu->invertTriangleFacing(triangleID);

                //  Do not drop back facing triangles.
                dropTriangle = false;

                GPU_DEBUG(
                    printf("Cull FRONT.  Inverting back facing triangle.\n");
                )
            }

            break;

        case BACK:

            //  If area is negative and back face culling is selected.
            if (tArea < 0)
            {
                //  Drop back facing triangles.
                dropTriangle = true;

                GPU_DEBUG(
                    printf("Cull BACK.  Culling back facing triangle.\n");
                )
            }
            else
            {
                //  Do not drop front facing triangles.
                dropTriangle = false;
            }

            break;

        case FRONT_AND_BACK:

            //  Drop all triangles.
            dropTriangle = true;

            GPU_DEBUG(
                printf("Cull FRONT_AND_BACK.  Culling triangle.\n");
            )

            break;

        default:

            panic("bGPU-Emu", "cullTriangle", "Unsupported triangle culling mode.");
            break;
    }

    //  Check the triangle area.
    if (!dropTriangle && (tArea == 0.0f))
    {
        //  Drop the triangle.
        dropTriangle = true;

        GPU_DEBUG(
            printf("Triangle with zero area culled.\n");
        )
    }

    return dropTriangle;
}

void GPUEmulator::emulateFragmentShading(ShadedFragment **quad)
{
    GLOBALPROFILER_ENTERREGION("emulateFragmentShading", "", "emulateFragmentShading");

    GLOBALPROFILER_ENTERREGION("emulateFragmentShading (load attributes)", "", "emulateFragmentShading")

    //  Initialize four threads for the fragment quad.
    for(u32bit p = 0; p < STAMP_FRAGMENTS; p++)
    {
        //  Initialize thread for the current fragment.
        shEmu->resetShaderState(p);

        //  Load the new shader input into the shader input register bank of the thread element in the shader emulator.
        shEmu->loadShaderState(p, gpu3d::IN, quad[p]->getAttributes());

        //  Set PC for the thread element in the shader emulator to the start PC of the vector thread.
        shEmu->setThreadPC(p, state.fragProgramStartPC);
    }


    GLOBALPROFILER_EXITREGION()

    bool programEnd = false;
    bool fragmentEnd[4];
    fragmentEnd[0] = fragmentEnd[1] = fragmentEnd[2] = fragmentEnd[3] = false;
    u32bit pc = state.fragProgramStartPC;

    u32bit p;
    ShaderInstruction::ShaderInstructionDecoded *shDecInstr;
    TextureAccess *texAccess;

    //  Emulation trace generation macro.
    GPU_EMU_TRACE(        
        if (traceLog || (traceBatch && (batchCounter == watchBatch)))
        {
            printf("FSh => Executing program at %04x for pixels: \n", pc);
            for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
                printf("    P%d -> (%4d, %4d) \n", f, quad[f]->getFragment()->getX(), quad[f]->getFragment()->getY());
        }
    )
    
    
    //  Execute all the instructions in the program.
    while (!programEnd)
    {
        bool jump = false;
        u32bit destPC = 0;
        
        //  Execute the instruction for the four fragment/threads in the current quad.
        for(p = 0; p < STAMP_FRAGMENTS; p++)
        {
            //  Check if the program already finished for this fragment.
            //if (!fragmentEnd[p])
            //{
                GLOBALPROFILER_ENTERREGION("emulateFragmentShading (fetch)", "", "emulateFragmentShading")

                //  Fetch the instruction.
                shDecInstr = shEmu->fetchShaderInstruction(p, pc);

                GLOBALPROFILER_EXITREGION()

                GPU_ASSERT(
                    if (shDecInstr == NULL)
                    {
                        char buffer[256];
                        sprintf(buffer, "Error fetching fragment program instruction at %02x\n", pc);
                        panic("GPUEmulator", "emulateFragmentShading", buffer);
                    }
                )

                GPU_EMU_TRACE(
                    if (traceFShader)
                    {
                        char shInstrDisasm[256];
                        shDecInstr->getShaderInstruction()->disassemble(shInstrDisasm);
                        printf("P%1d => %04x : %s\n", p, pc, shInstrDisasm);
                        
                        printShaderInstructionOperands(shDecInstr);
                    }
                )
                
                GPU_DEBUG(
                    char shInstrDisasm[256];
                    shDecInstr->getShaderInstruction()->disassemble(shInstrDisasm);
                    printf("FSh => Executing instruction @ %04x : %s\n", pc, shInstrDisasm);
                )

                GLOBALPROFILER_ENTERREGION("emulateFragmentShading (exec)", "", "emulateFragmentShading")

                //  Execute instruction.
                shEmu->execShaderInstruction(shDecInstr);

                GLOBALPROFILER_EXITREGION();

                //  Check for jump instructions (only the first thread in the 4-way vector).
                if ((p == 0) && shDecInstr->getShaderInstruction()->isAJump())
                {
                    //  Check if the jump is performed.
                    jump = shEmu->checkJump(shDecInstr, 4, destPC);
                }

                //  Check if this is the last instruction in the program.
                fragmentEnd[p] = shDecInstr->getShaderInstruction()->getEndFlag() || shEmu->threadKill(p);

                GPU_EMU_TRACE(
                    if (traceFShader)
                    {
                        printShaderInstructionResult(shDecInstr);
                        printf("             KILL MASK -> %s\n", shEmu->threadKill(p) ? "true" : "false");
                        printf("             FRAGMENT END -> %s\n", fragmentEnd[p] ? "true" : "false");
                        if (p == (STAMP_FRAGMENTS - 1))
                            printf("-------------------\n");

                        fflush(stdout);
                    }
                )
            //}
            
        }

        //  Process texture requests.  Texture requests are processed per fragment quad.
        texAccess = shEmu->nextTextureAccess();

        //  Check if there is a pending texture request from the previous shader instruction.
        if (texAccess != NULL)
        {
            //  Process the texture requests.
            emulateTextureUnit(texAccess);
        }

        //  Program finishes when the four fragments in the quad finish.
        programEnd = fragmentEnd[0] && fragmentEnd[1] && fragmentEnd[2] && fragmentEnd[3];

        //  Update PC.
        if (jump)
        {
            pc = destPC;
            
            GPU_EMU_TRACE(
                if (traceFShader)
                    printf("Jumping to PC %04x\n", pc);
            )
        }
        else
            pc++;
    }
   
    GLOBALPROFILER_ENTERREGION("emulateFragmentShading (retrieve attributes)", "", "emulateFragmentShading")
    //  Get the result attributes for the four fragments in the quad.
    for(u32bit p = 0; p < STAMP_FRAGMENTS; p++)
    {
        //  Get output attributes for the fragment.
        shEmu->readShaderState(p, gpu3d::OUT, quad[p]->getAttributes());

        //  Set fragment as culled if the fragment was killed.
        if (shEmu->threadKill(p))
            quad[p]->setAsCulled();

        GPU_DEBUG(
            printf("FSh => Fragment Outputs :\n");
            //for(u32bit a = 0; a < MAX_FRAGMENT_ATTRIBUTES; a++)
            for(u32bit a = 0; a < 5; a++)
            {
                QuadFloat *attributes = quad[p]->getAttributes();
                printf(" OUT[%02d] = {%f, %f, %f, %f}\n", a, attributes[a][0], attributes[a][1], attributes[a][2], attributes[a][3]);
            }
        )
    }

    GLOBALPROFILER_EXITREGION()

    GLOBALPROFILER_EXITREGION()
}


//  Emulate the texture unit.
void GPUEmulator::emulateTextureUnit(TextureAccess *texAccess)
{
    GLOBALPROFILER_ENTERREGION("emulateTextureUnit", "", "emulateTextureUnit")

    //  Calculate addresses for all the aniso samples required for the texture request.
    for(texAccess->currentAnisoSample = 1; texAccess->currentAnisoSample <= texAccess->anisoSamples; texAccess->currentAnisoSample++)
        texEmu->calculateAddress(texAccess);

    //  Read texture data from memory.
    for(u32bit s = 0; s < texAccess->anisoSamples; s++)
    {
        //  For all fragments in the quad.
        for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
        {
            //  For all texels to read.
            for(u32bit t = 0; t < texAccess->trilinear[s]->texelsLoop[f] * texAccess->trilinear[s]->loops[f]; t++)
            {
                //  Read and convert texel data.
                u8bit data[128];

                //  Read memory.
                readTextureData(texAccess->trilinear[s]->address[f][t], texAccess->texelSize[f], data);

                //  Convert to internal format.
                texEmu->convertFormat(*texAccess, s, f, t, data);
                
                /*if (batchCounter == 51) {
                 printf("         Float32 (%f, %f) = (%f, %f, %f, %f)\n",
                     texAccess->coordinates[f][0], texAccess->coordinates[f][1],
                     texAccess->trilinear[s]->texel[f][t][0], texAccess->trilinear[s]->texel[f][t][1],
                     texAccess->trilinear[s]->texel[f][t][2], texAccess->trilinear[s]->texel[f][t][3]);
                }*/
            }
        }
    }

    //  Filter the texture request.
    for(u32bit s = 0; s < texAccess->anisoSamples; s++)
        texEmu->filter(*texAccess, s);

    GPU_EMU_TRACE(
        if (traceTexture)
            printTextureAccessInfo(texAccess);
    )
    
    u32bit threads[STAMP_FRAGMENTS];

    //  Send texture results to the shader emulator.
    shEmu->writeTextureAccess(texAccess->accessID, texAccess->sample, threads, false);

    delete texAccess;

    GLOBALPROFILER_EXITREGION()
}

void GPUEmulator::readTextureData(u64bit texelAddress, u32bit size, u8bit *data)
{

    //  Check for black texel address (out of bounds).
    if (texelAddress == BLACK_TEXEL_ADDRESS)
    {
        //  Return zeros.
        for(u32bit b = 0; b < size; b++)
            data[b] = 0;

        return;
    }

    //
    //  WARNING!!! Texel addresses must be aligned to 4 bytes before accessing memory.
    //

    //  Check if the address is for a compressed texture.
    switch(texelAddress & TEXTURE_ADDRESS_SPACE_MASK)
    {
        case UNCOMPRESSED_TEXTURE_SPACE:
            {
                u8bit *memory = selectMemorySpace(u32bit(texelAddress & 0xffffffff));
                u32bit address = u32bit(texelAddress & 0xfffffffc) & SPACE_ADDRESS_MASK;

                for(u32bit b = 0; b < size; b++)
                    data[b] = memory[address + b];
            }

            break;

        case COMPRESSED_TEXTURE_SPACE_DXT1_RGB:

            cacheDXT1RGB.readData(texelAddress, data, size);

            break;


        case COMPRESSED_TEXTURE_SPACE_DXT1_RGBA:

            cacheDXT1RGBA.readData(texelAddress, data, size);

            break;

        case COMPRESSED_TEXTURE_SPACE_DXT3_RGBA:

            cacheDXT3RGBA.readData(texelAddress, data, size);

            break;

        case COMPRESSED_TEXTURE_SPACE_DXT5_RGBA:

            cacheDXT5RGBA.readData(texelAddress, data, size);

            break;

        case COMPRESSED_TEXTURE_SPACE_LATC1:

            cacheLATC1.readData(texelAddress, data, size);

            break;

        case COMPRESSED_TEXTURE_SPACE_LATC1_SIGNED:

            cacheLATC1_SIGNED.readData(texelAddress, data, size);

            break;

        case COMPRESSED_TEXTURE_SPACE_LATC2:

            cacheLATC2.readData(texelAddress, data, size);

            break;

        case COMPRESSED_TEXTURE_SPACE_LATC2_SIGNED:

            cacheLATC2_SIGNED.readData(texelAddress, data, size);

            break;
        default:
            panic("GPUEmulator", "readTextureData", "Unsupported texture address space.");
            break;
    }

}

void GPUEmulator::emulateColorWrite(ShadedFragment **quad)
{
    GLOBALPROFILER_ENTERREGION("emulateColorWrite", "", "emulateColorWrite")

    u32bit bytesPixel;
    
    //  Write output for all render targets.
    for(u32bit rt = 0; rt < MAX_RENDER_TARGETS; rt++)
    {
        //  Check if the render targets is enabled and color mask is true.
        if (state.rtEnable[rt] && ((state.colorMaskR[rt] || state.colorMaskG[rt] || state.colorMaskB[rt] || state.colorMaskA[rt])))
        {
            QuadFloat destColorQF[STAMP_FRAGMENTS * MAX_MSAA_SAMPLES];
            QuadFloat inputColorQF[STAMP_FRAGMENTS * MAX_MSAA_SAMPLES];
            bool writeMask[STAMP_FRAGMENTS * MAX_MSAA_SAMPLES];
            u8bit *colorData;

            //  Compute the address in memory for the fragment quad.
            u32bit address = pixelMapper[rt].computeAddress(quad[0]->getFragment()->getX(), quad[0]->getFragment()->getY());

            //  Add back buffer base address to the address inside the color buffer.
            address += state.rtAddress[rt];

            u8bit *memory = selectMemorySpace(address);
            address = address & SPACE_ADDRESS_MASK;

            colorData = &memory[address];

            for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
            {
                //  Get fragment attributes.
                QuadFloat *attributes = quad[f]->getAttributes();

                //  Get fragment.
                Fragment *fr = quad[f]->getFragment();

/*if ((fr->getX() == 204) && (fr->getY() == 312))
{
bool *coverage = fr->getMSAACoverage();
printf("render target = %d\n", rt);
printf("coverage = %d %d %d %d\n", coverage[0], coverage[1], coverage[2], coverage[3]);
printf("color = %f %f %f %f\n", GPU_CLAMP(attributes[COLOR_ATTRIBUTE + rt][0], 0.0f, 1.0f),
GPU_CLAMP(attributes[COLOR_ATTRIBUTE + rt][1], 0.0f, 1.0f),
GPU_CLAMP(attributes[COLOR_ATTRIBUTE + rt][2], 0.0f, 1.0f),
GPU_CLAMP(attributes[COLOR_ATTRIBUTE + rt][3], 0.0f, 1.0f));
}*/

                //  Check if multisampling is enabled.
                if (!state.multiSampling)
                {
                    //  Get fragment input color from color attribute.
                    if (state.rtFormat[rt] == GPU_RGBA8888)
                    {
                        inputColorQF[f][0] = GPU_CLAMP(attributes[COLOR_ATTRIBUTE + rt][0], 0.0f, 1.0f);
                        inputColorQF[f][1] = GPU_CLAMP(attributes[COLOR_ATTRIBUTE + rt][1], 0.0f, 1.0f);
                        inputColorQF[f][2] = GPU_CLAMP(attributes[COLOR_ATTRIBUTE + rt][2], 0.0f, 1.0f);
                        inputColorQF[f][3] = GPU_CLAMP(attributes[COLOR_ATTRIBUTE + rt][3], 0.0f, 1.0f);
                    }
                    else
                    {
                        inputColorQF[f][0] = attributes[COLOR_ATTRIBUTE + rt][0];
                        inputColorQF[f][1] = attributes[COLOR_ATTRIBUTE + rt][1];
                        inputColorQF[f][2] = attributes[COLOR_ATTRIBUTE + rt][2];
                        inputColorQF[f][3] = attributes[COLOR_ATTRIBUTE + rt][3];
                    }

                    //  Build write mask for the fragment.
                    //switch(state.colorBufferFormat)
                    switch(state.rtFormat[rt])
                    {
                        case GPU_RGBA8888:

                            writeMask[f * 4    ] = state.colorMaskR[rt] && !quad[f]->isCulled();
                            writeMask[f * 4 + 1] = state.colorMaskG[rt] && !quad[f]->isCulled();
                            writeMask[f * 4 + 2] = state.colorMaskB[rt] && !quad[f]->isCulled();
                            writeMask[f * 4 + 3] = state.colorMaskA[rt] && !quad[f]->isCulled();
                            break;

                        case GPU_RG16F:

                            writeMask[f * 4    ] = writeMask[f * 4 + 1] = state.colorMaskR[rt] && !quad[f]->isCulled();
                            writeMask[f * 4 + 2] = writeMask[f * 4 + 3] = state.colorMaskG[rt] && !quad[f]->isCulled();
                            break;

                        case GPU_R32F:

                            writeMask[f * 4    ] = writeMask[f * 4 + 1] =
                            writeMask[f * 4 + 2] = writeMask[f * 4 + 3] = state.colorMaskR[rt] && !quad[f]->isCulled();
                            break;

                        case GPU_RGBA16:
                            writeMask[f * 8 + 0] = writeMask[f * 8 + 1] = state.colorMaskR[rt] && !quad[f]->isCulled();
                            writeMask[f * 8 + 2] = writeMask[f * 8 + 3] = state.colorMaskG[rt] && !quad[f]->isCulled();
                            writeMask[f * 8 + 4] = writeMask[f * 8 + 5] = state.colorMaskB[rt] && !quad[f]->isCulled();
                            writeMask[f * 8 + 6] = writeMask[f * 8 + 7] = state.colorMaskA[rt] && !quad[f]->isCulled();
                            break;

                        case GPU_RGBA16F:

                            writeMask[f * 8 + 0] = writeMask[f * 8 + 1] = state.colorMaskR[rt] && !quad[f]->isCulled();
                            writeMask[f * 8 + 2] = writeMask[f * 8 + 3] = state.colorMaskG[rt] && !quad[f]->isCulled();
                            writeMask[f * 8 + 4] = writeMask[f * 8 + 5] = state.colorMaskB[rt] && !quad[f]->isCulled();
                            writeMask[f * 8 + 6] = writeMask[f * 8 + 7] = state.colorMaskA[rt] && !quad[f]->isCulled();
                            break;
                    }
                }
                else
                {
                    //  Get sample coverage mask for the fragment.
                    bool *sampleCoverage = fr->getMSAACoverage();

                    //  Copy shaded fragment color to all samples for the fragment.
                    for(u32bit s = 0; s < state.msaaSamples; s++)
                    {
                        //  Get fragment input color from color attribute.
                        //if (state.colorBufferFormat == GPU_RGBA8888)
                        if (state.rtFormat[rt] == GPU_RGBA8888)
                        {
                            inputColorQF[f * state.msaaSamples + s][0] = GPU_CLAMP(attributes[COLOR_ATTRIBUTE + rt][0], 0.0f, 1.0f);
                            inputColorQF[f * state.msaaSamples + s][1] = GPU_CLAMP(attributes[COLOR_ATTRIBUTE + rt][1], 0.0f, 1.0f);
                            inputColorQF[f * state.msaaSamples + s][2] = GPU_CLAMP(attributes[COLOR_ATTRIBUTE + rt][2], 0.0f, 1.0f);
                            inputColorQF[f * state.msaaSamples + s][3] = GPU_CLAMP(attributes[COLOR_ATTRIBUTE + rt][3], 0.0f, 1.0f);
                        }
                        else
                        {
                            inputColorQF[f * state.msaaSamples + s][0] = attributes[COLOR_ATTRIBUTE + rt][0];
                            inputColorQF[f * state.msaaSamples + s][1] = attributes[COLOR_ATTRIBUTE + rt][1];
                            inputColorQF[f * state.msaaSamples + s][2] = attributes[COLOR_ATTRIBUTE + rt][2];
                            inputColorQF[f * state.msaaSamples + s][3] = attributes[COLOR_ATTRIBUTE + rt][3];
                        }

                        //  Build write mask for the fragment samples.
                        //switch(state.colorBufferFormat)
                        switch(state.rtFormat[rt])
                        {
                            case GPU_RGBA8888:

                                writeMask[(f * state.msaaSamples + s) * 4 + 0] = state.colorMaskR[rt] && sampleCoverage[s];
                                writeMask[(f * state.msaaSamples + s) * 4 + 1] = state.colorMaskG[rt] && sampleCoverage[s];
                                writeMask[(f * state.msaaSamples + s) * 4 + 2] = state.colorMaskB[rt] && sampleCoverage[s];
                                writeMask[(f * state.msaaSamples + s) * 4 + 3] = state.colorMaskA[rt] && sampleCoverage[s];
                                break;

                            case GPU_RG16F:

                                writeMask[(f * state.msaaSamples + s) * 4 + 0] =
                                writeMask[(f * state.msaaSamples + s) * 4 + 1] = state.colorMaskR[rt] && sampleCoverage[s];
                                writeMask[(f * state.msaaSamples + s) * 4 + 2] =
                                writeMask[(f * state.msaaSamples + s) * 4 + 3] = state.colorMaskG[rt] && sampleCoverage[s];
                                break;

                            case GPU_R32F:

                                writeMask[(f * state.msaaSamples + s) * 4 + 0] =
                                writeMask[(f * state.msaaSamples + s) * 4 + 1] =
                                writeMask[(f * state.msaaSamples + s) * 4 + 2] =
                                writeMask[(f * state.msaaSamples + s) * 4 + 3] = state.colorMaskR[rt] && sampleCoverage[s];

                                break;

                            case GPU_RGBA16:
                                
                                writeMask[(f * state.msaaSamples + s) * 8 + 0] =
                                writeMask[(f * state.msaaSamples + s) * 8 + 1] = state.colorMaskR[rt] && sampleCoverage[s];
                                writeMask[(f * state.msaaSamples + s) * 8 + 2] =
                                writeMask[(f * state.msaaSamples + s) * 8 + 3] = state.colorMaskG[rt] && sampleCoverage[s];
                                writeMask[(f * state.msaaSamples + s) * 8 + 4] =
                                writeMask[(f * state.msaaSamples + s) * 8 + 5] = state.colorMaskB[rt] && sampleCoverage[s];
                                writeMask[(f * state.msaaSamples + s) * 8 + 6] =
                                writeMask[(f * state.msaaSamples + s) * 8 + 7] = state.colorMaskA[rt] && sampleCoverage[s];
                                
                                break;
                                
                            case GPU_RGBA16F:
                            
                                writeMask[(f * state.msaaSamples + s) * 8 + 0] =
                                writeMask[(f * state.msaaSamples + s) * 8 + 1] = state.colorMaskR[rt] && sampleCoverage[s];
                                writeMask[(f * state.msaaSamples + s) * 8 + 2] =
                                writeMask[(f * state.msaaSamples + s) * 8 + 3] = state.colorMaskG[rt] && sampleCoverage[s];
                                writeMask[(f * state.msaaSamples + s) * 8 + 4] =
                                writeMask[(f * state.msaaSamples + s) * 8 + 5] = state.colorMaskB[rt] && sampleCoverage[s];
                                writeMask[(f * state.msaaSamples + s) * 8 + 6] =
                                writeMask[(f * state.msaaSamples + s) * 8 + 7] = state.colorMaskA[rt] && sampleCoverage[s];
                                break;
                        }
                    }
                }
            }

            GPU_DEBUG(
                printf("Write Mask = ");
                for(u32bit b = 0; b < (STAMP_FRAGMENTS * 4); b++)
                    printf("%d ", writeMask[b]);
                printf("\n");

                printf("Color Data = ");
                for(u32bit b = 0; b < (STAMP_FRAGMENTS * 4); b++)
                    printf("%02x ", colorData[b]);
                printf("\n");
            )

            //  Check if multisampling is enabled.
            if (!state.multiSampling)
            {
                //  Convert color buffer data to internal representation (RGBA32F).
                //switch(state.colorBufferFormat)
                switch(state.rtFormat[rt])
                {
                    case GPU_RGBA8888:

                        colorRGBA8ToRGBA32F(colorData, destColorQF);
                        break;

                    case GPU_RG16F:

                        colorRG16FToRGBA32F(colorData, destColorQF);
                        break;

                    case GPU_R32F:

                        colorR32FToRGBA32F(colorData, destColorQF);
                        break;

                    case GPU_RGBA16:

                        colorRGBA16ToRGBA32F(colorData, destColorQF);
                        break;

                    case GPU_RGBA16F:

                        colorRGBA16FToRGBA32F(colorData, destColorQF);
                        break;
                }
                
                //  Convert from sRGB to linear space if required.
                if (state.colorSRGBWrite)
                    colorSRGBToLinear(destColorQF);
            }
            else
            {
                //  Convert fixed point color data to float point data for all the samples in the stamp
                for(u32bit s = 0; s < state.msaaSamples; s++)
                {
                    //  Convert color buffer data to internal representation (RGBA32F).
                    switch(state.rtFormat[rt])
                    {
                        case GPU_RGBA8888:

                            colorRGBA8ToRGBA32F(&colorData[s * STAMP_FRAGMENTS * 4], &destColorQF[STAMP_FRAGMENTS * s]);
                            break;

                        case GPU_RG16F:

                            colorRG16FToRGBA32F(&colorData[s * STAMP_FRAGMENTS * 4], &destColorQF[STAMP_FRAGMENTS * s]);
                            break;

                        case GPU_R32F:

                            colorR32FToRGBA32F(&colorData[s * STAMP_FRAGMENTS * 4], &destColorQF[STAMP_FRAGMENTS * s]);
                            break;

                        case GPU_RGBA16:

                            colorRGBA16ToRGBA32F(&colorData[s * STAMP_FRAGMENTS * 8], &destColorQF[STAMP_FRAGMENTS * s]);
                            break;

                        case GPU_RGBA16F:

                            colorRGBA16FToRGBA32F(&colorData[s * STAMP_FRAGMENTS * 8], &destColorQF[STAMP_FRAGMENTS * s]);
                            break;
                    }
                    
                    //  Convert from sRGB to linear space if required.
                    if (state.colorSRGBWrite)
                        colorSRGBToLinear(&destColorQF[STAMP_FRAGMENTS * s]);
                }
            }

/*if (batchCounter == 622) {
    for(u32bit b = 0; b < (STAMP_FRAGMENTS); b++) {
        printf("Source Data = ");
        printf("(%f, %f, %f, %f)\n", inputColorQF[b][0], inputColorQF[b][1], inputColorQF[b][2], inputColorQF[b][3]);
        printf("Dest Data = ");
        printf("(%f, %f, %f, %f)\n", destColorQF[b][0], destColorQF[b][1], destColorQF[b][2], destColorQF[b][3]);
    }
}*/
            FragmentQuadMemoryUpdateInfo quadMemUpdate;
                            
            //  Determine if blend mode is active.
            if (state.colorBlend[rt])
            {
                //  Check if multisampling is enabled.
                if (!state.multiSampling)
                {
                    //  Check if validation mode is enabled.
                    if (validationMode)
                    {
                        //  Check if there are unculled fragments in the quad.
                        bool anyNotAlreadyCulled = false;            
                        for(u32bit f = 0; (f < STAMP_FRAGMENTS) && !anyNotAlreadyCulled; f++)
                            anyNotAlreadyCulled = anyNotAlreadyCulled || !quad[f]->isCulled();

                        //  Store the information for the quad if there is a valid fragment.
                        if (anyNotAlreadyCulled)
                        {
                            //  Set the quad identifier to the top left fragment of the quad.
                            quadMemUpdate.fragID.x = quad[0]->getFragment()->getX();
                            quadMemUpdate.fragID.y = quad[0]->getFragment()->getY();
                            quadMemUpdate.fragID.triangleID = triangleCounter;
                            
                            quadMemUpdate.fragID.sample = 0;

                            //  Copy the input (color computed per fragment) and read data (color from the render target) for
                            //  the quad.
                            for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
                            {
                                ((f32bit *) quadMemUpdate.inData)[f * 4 + 0] = inputColorQF[f][0];
                                ((f32bit *) quadMemUpdate.inData)[f * 4 + 1] = inputColorQF[f][1];
                                ((f32bit *) quadMemUpdate.inData)[f * 4 + 2] = inputColorQF[f][2];
                                ((f32bit *) quadMemUpdate.inData)[f * 4 + 3] = inputColorQF[f][3];
                            }
                            for(u32bit qw = 0; qw < ((bytesPerPixelColor[rt] * 4) >> 3); qw++)
                                ((u64bit *) quadMemUpdate.readData)[qw] = ((u64bit *) colorData)[qw];
                        }
                    }
                    
                    /*  Perform blend operation.  */
                    fragEmu->blend(rt, inputColorQF, inputColorQF, destColorQF);
                }
                else
                {
                    //  Perform blending for all the samples in the stamp.
                    for(u32bit s = 0; s < state.msaaSamples; s++)
                    {
                        //  Perform blending for a group of samples.
                        fragEmu->blend(rt, &inputColorQF[STAMP_FRAGMENTS * s], &inputColorQF[STAMP_FRAGMENTS * s],
                                       &destColorQF[STAMP_FRAGMENTS * s]);
                    }
                }
            }
            else
            {
                //  Check if validation mode is enabled.
                if (validationMode)
                {
                    //  Check if there are unculled fragments in the quad.
                    bool anyNotAlreadyCulled = false;            
                    for(u32bit f = 0; (f < STAMP_FRAGMENTS) && !anyNotAlreadyCulled; f++)
                        anyNotAlreadyCulled = anyNotAlreadyCulled || !quad[f]->isCulled();

                    //  Store the information for the quad if there is a valid fragment.
                    if (anyNotAlreadyCulled)
                    {
                        //  Set the quad identifier to the top left fragment of the quad.
                        quadMemUpdate.fragID.x = quad[0]->getFragment()->getX();
                        quadMemUpdate.fragID.y = quad[0]->getFragment()->getY();
                        quadMemUpdate.fragID.triangleID = triangleCounter;
                        
                        quadMemUpdate.fragID.sample = 0;

                        //  Copy the input (color computed per fragment) and read data (color from the render target) for
                        //  the quad.
                        for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
                        {
                            ((f32bit *) quadMemUpdate.inData)[f * 4 + 0] = inputColorQF[f][0];
                            ((f32bit *) quadMemUpdate.inData)[f * 4 + 1] = inputColorQF[f][1];
                            ((f32bit *) quadMemUpdate.inData)[f * 4 + 2] = inputColorQF[f][2];
                            ((f32bit *) quadMemUpdate.inData)[f * 4 + 3] = inputColorQF[f][3];
                        }
                        for(u32bit qw = 0; qw < ((bytesPerPixelColor[rt] * 4) >> 3); qw++)
                            ((u64bit *) quadMemUpdate.readData)[qw] = 0xDEFADA7ADEFADA7AULL;
                    }
                }
            }

/*if (batchCounter == 622) {
    for(u32bit b = 0; b < (STAMP_FRAGMENTS); b++) {
        printf("Color Data = ");
        printf("(%f, %f, %f, %f)\n", inputColorQF[b][0], inputColorQF[b][1], inputColorQF[b][2], inputColorQF[b][3]);
    }
}*/

            u8bit outColor[STAMP_FRAGMENTS * MAX_MSAA_SAMPLES * 8];
            u32bit bytesPerSample;

            GPU_DEBUG(
                printf("Color Data = ");
                for(u32bit b = 0; b < (STAMP_FRAGMENTS * 4); b++)
                    printf("%02x ", colorData[b]);
                printf("\n");
            )

            //  Determine if logical operation is active.
            if ((rt == 0) && state.logicalOperation)
            {
                GPU_ASSERT(
                    //if (state.colorBufferFormat != GPU_RGBA8888)
                    if (state.rtFormat[rt] != GPU_RGBA8888)
                        panic("GPUEmulator", "emulateColorWrite", "Logic operation only supported with RGBA8 color buffer format.");
                )

                bytesPerSample = 4;
                
                //  Check if multisampling is enabled
                if (!state.multiSampling)
                {
                    //  Convert output color from linear space to sRGB space if required.
                    if (state.colorSRGBWrite)
                        colorLinearToSRGB(inputColorQF);
            
                    //  Convert the stamp color data to integer format.
                    colorRGBA32FToRGBA8(inputColorQF, outColor);

                    //  Perform logical operation.
                    fragEmu->logicOp(outColor, colorData, outColor);
                }
                else
                {
                    //  Convert to integer format and perform logical op for all the samples in the stamp.
                    for(u32bit s = 0; s < state.msaaSamples; s++)
                    {
                        //  Convert output color from linear space to sRGB space if required.
                        if (state.colorSRGBWrite)
                            colorLinearToSRGB(&inputColorQF[STAMP_FRAGMENTS * s]);

                        //  Convert the sample color data to integer format.
                        colorRGBA32FToRGBA8(&inputColorQF[STAMP_FRAGMENTS * s], &outColor[STAMP_FRAGMENTS * s * 4]);

                        //  Perform logical operation for a group of samples.
                        fragEmu->logicOp(&outColor[STAMP_FRAGMENTS * s * 4], &colorData[STAMP_FRAGMENTS * s * 4], &outColor[STAMP_FRAGMENTS * s * 4]);
                    }
                }
            }
            else
            {
                //  Check if multisampling is enabled.
                if (!state.multiSampling)
                {
                    //  Convert output color from linear space to sRGB space if required.
                    if (state.colorSRGBWrite)
                        colorLinearToSRGB(inputColorQF);
                        
                    //  Convert the stamp color data in internal format to the color buffer format.
                    //switch(state.colorBufferFormat)
                    switch(state.rtFormat[rt])
                    {
                        case GPU_RGBA8888:

                            colorRGBA32FToRGBA8(inputColorQF, outColor);
                            bytesPerSample = 4;
                            break;

                        case GPU_RG16F:

                            colorRGBA32FToRG16F(inputColorQF, outColor);
                            bytesPerSample = 4;
                            break;

                        case GPU_R32F:

                            colorRGBA32FToR32F(inputColorQF, outColor);
                            bytesPerSample = 4;
                            break;

                        case GPU_RGBA16:

                            colorRGBA32FToRGBA16(inputColorQF, outColor);
                            bytesPerSample = 8;
                            break;

                        case GPU_RGBA16F:

                            colorRGBA32FToRGBA16F(inputColorQF, outColor);
                            bytesPerSample = 8;
                            break;
                    }
                }
                else
                {
                    //  Convert the sample color data for all the stamp.
                    for(u32bit s = 0; s < state.msaaSamples; s++)
                    {
                        //  Convert output color from linear space to sRGB space if required.
                        if (state.colorSRGBWrite)
                            colorLinearToSRGB(&inputColorQF[STAMP_FRAGMENTS * s]);

                        //  Convert the stamp color data in internal format to the color buffer format.
                        //switch(state.colorBufferFormat)
                        switch(state.rtFormat[rt])
                        {
                            case GPU_RGBA8888:

                                colorRGBA32FToRGBA8(&inputColorQF[STAMP_FRAGMENTS * s], &outColor[STAMP_FRAGMENTS * s * 4]);
                                bytesPerSample = 4;
                                break;

                            case GPU_RG16F:

                                colorRGBA32FToRG16F(&inputColorQF[STAMP_FRAGMENTS * s], &outColor[STAMP_FRAGMENTS * s * 4]);
                                bytesPerSample = 4;
                                break;

                            case GPU_R32F:

                                colorRGBA32FToR32F(&inputColorQF[STAMP_FRAGMENTS * s], &outColor[STAMP_FRAGMENTS * s * 4]);
                                bytesPerSample = 4;
                                break;

                            case GPU_RGBA16:

                                colorRGBA32FToRGBA16(&inputColorQF[STAMP_FRAGMENTS * s], &outColor[STAMP_FRAGMENTS * s * 8]);
                                bytesPerSample = 8;
                                break;

                            case GPU_RGBA16F:

                                colorRGBA32FToRGBA16F(&inputColorQF[STAMP_FRAGMENTS * s], &outColor[STAMP_FRAGMENTS * s * 8]);
                                bytesPerSample = 8;
                                break;
                        }
                    }
                }
            }

            GPU_DEBUG(
                printf("Color Data = ");
                for(u32bit b = 0; b < (STAMP_FRAGMENTS * 4); b++)
                    printf("%02x ", colorData[b]);
                printf("\n");
                printf("Out Color = ");
                for(u32bit b = 0; b < (STAMP_FRAGMENTS * 4); b++)
                    printf("%02x ", outColor[b]);
                printf("\n");
            )

            //  Check if multisampling is enabled.
            if (!state.multiSampling)
            {
                //  Masked write of the result color.
                for(u32bit b = 0; b < STAMP_FRAGMENTS * bytesPerSample; b++)
                {
                    if (writeMask[b])
                        colorData[b] = outColor[b];
                }

                if (validationMode)
                {
                    //  Check if a write is performed (fragment not culled).
                    bool anyNotCulled = false;                       
                    for(u32bit f = 0; (f < STAMP_FRAGMENTS) && !anyNotCulled; f++)
                        anyNotCulled = anyNotCulled || !quad[f]->isCulled();
                    
                    //  Store information for the quad and add to the z/stencil memory update map.
                    if (anyNotCulled)
                    {
                        //  Store the result z and stencil results for the quad.
                        for(u32bit qw = 0; qw < ((bytesPerPixelColor[rt] * 4) >> 3) ; qw++)
                            ((u64bit *) quadMemUpdate.writeData)[qw] = ((u64bit *) outColor)[qw];
                            
                        //  Store the write mask for the quad.
                        for(u32bit b = 0; b < (STAMP_FRAGMENTS * bytesPerPixelColor[rt]); b++)
                            quadMemUpdate.writeMask[b] = writeMask[b];
                            
                        //  Store the cull mask for the quad.
                        for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
                            quadMemUpdate.cullMask[f] = quad[f]->isCulled();
                            
                        //  Store the information for the quad in the z stencil memory update map.
                        colorMemoryUpdateMap[rt].insert(make_pair(quadMemUpdate.fragID, quadMemUpdate));
                    }
                }
            }
            else
            {
                //  Masked write of the result color.
                for(u32bit b = 0; b < STAMP_FRAGMENTS * state.msaaSamples * bytesPerSample; b++)
                {
                    if (writeMask[b])
                        colorData[b] = outColor[b];
                }
            }

            GPU_DEBUG(
                printf("Color Data = ");
                for(u32bit b = 0; b < (STAMP_FRAGMENTS * 4); b++)
                    printf("%02x ", colorData[b]);
                printf("\n");
            )
        }
    }
    
    GLOBALPROFILER_EXITREGION()
}

void GPUEmulator::emulateZStencilTest(ShadedFragment **quad)
{
    //  Optimization.
    if (!state.depthTest && !state.stencilTest)
        return;

    GLOBALPROFILER_ENTERREGION("emulateZStencilTest", "", "emulateZStencilTest")

    u32bit inputDepth[STAMP_FRAGMENTS * MAX_MSAA_SAMPLES];
    bool sampleCullMask[STAMP_FRAGMENTS * MAX_MSAA_SAMPLES];
    bool writeMask[STAMP_FRAGMENTS * MAX_MSAA_SAMPLES * 4];
    bool culledFragments[STAMP_FRAGMENTS];
    u8bit *zStencilData;
    u32bit zStencilInOutData[STAMP_FRAGMENTS * MAX_MSAA_SAMPLES];


    u32bit bytesPerSample = 4;

    //  Compute the address in memory for the fragment quad.
    u32bit address = zPixelMapper.computeAddress(quad[0]->getFragment()->getX(), quad[0]->getFragment()->getY());

    //  Add back buffer base address to the address inside the color buffer.
    address += state.zStencilBufferBaseAddr;

    u8bit *memory = selectMemorySpace(address);
    address = address & SPACE_ADDRESS_MASK;

    GPU_DEBUG(
        printf("ZStencilTest for quad at (%d, %d) at address %08x (base address %08x)\n",
        quad[0]->getFragment()->getX(), quad[0]->getFragment()->getY(),
        address, state.zStencilBufferBaseAddr);
    )

    zStencilData = &memory[address];

    //  Check if multisampling is enabled.
    if (!state.multiSampling)
    {
        //  Read z stencil data from the buffer.
        for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
            zStencilInOutData[f] = *((u32bit *) &zStencilData[f * bytesPerSample]);
    }
    else
    {
        //  Read z stencil data from the buffer.
        for(u32bit s = 0; s < STAMP_FRAGMENTS * state.msaaSamples; s++)
            zStencilInOutData[s] = *((u32bit *) &zStencilData[s * bytesPerSample]);
    }

    GPU_DEBUG(
        printf("Z Stencil Buffer Data = ");
        for(u32bit b = 0; b < (STAMP_FRAGMENTS * 4); b++)
            printf("%02x ", zStencilData[b]);
        printf("\n");
    )

    //  Get the input depth data from the fragments.
    for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
    {
        culledFragments[f] = quad[f]->isCulled();

        //  Get fragment attributes.
        QuadFloat *attributes = quad[f]->getAttributes();

        //  Get fragment.
        Fragment *fr = quad[f]->getFragment();

        //  Check if fragment depth was modified by the fragment shader.
        if (state.modifyDepth)
        {
            //  Check if multisampling is enabled.
            if (!state.multiSampling)
            {
                //  Convert modified fragment depth to integer format.
                inputDepth[f] = rastEmu->convertZ(GPU_CLAMP(attributes[POSITION_ATTRIBUTE][3], 0.0f, 1.0f));
            }
            else
            {
                //  Get pointer to the coverage mask for the fragment.
                bool *fragmentCoverage = fr->getMSAACoverage();

                //  Convert modified fragment depth to integer format.
                inputDepth[f * state.msaaSamples] = rastEmu->convertZ(GPU_CLAMP(attributes[POSITION_ATTRIBUTE][3], 0.0f, 1.0f));
                sampleCullMask[f * state.msaaSamples] = !fragmentCoverage[0];

                //  Copy Z sample values from the fragment z value computed in the shader.
                for(u32bit s = 1; s < state.msaaSamples; s++)
                {
                    inputDepth[f * state.msaaSamples + s] = inputDepth[f * state.msaaSamples];
                    sampleCullMask[f * state.msaaSamples + s] = !fragmentCoverage[s];
                }
            }
        }
        else
        {
            //  Check if multisampling is enabled.
            if (!state.multiSampling)
            {
                //  Copy input z from the fragment.
                inputDepth[f] = fr->getZ();
            }
            else
            {
                //  Get pointer to the Z samples for the fragment.
                u32bit *fragmentZSamples = fr->getMSAASamples();

                //  Get pointer to the coverage mask for the fragment.
                bool *fragmentCoverage = fr->getMSAACoverage();

                //  Copy Z sample values.
                for(u32bit s = 0; s < state.msaaSamples; s++)
                {
                    inputDepth[f * state.msaaSamples + s] = fragmentZSamples[s];
                    sampleCullMask[f * state.msaaSamples + s] = !fragmentCoverage[s];
                }
            }
        }

        //  Check if multisampling is enabled.
        if (!state.multiSampling)
        {
            //  Build write mask for the stamp.
            writeMask[f * bytesPerSample + 3] = (state.stencilUpdateMask == 0) ? false : true;
            writeMask[f * bytesPerSample + 2] =
            writeMask[f * bytesPerSample + 1] =
            writeMask[f * bytesPerSample + 0] = state.depthMask;
        }
        else
        {
            //  Create write mask for all the samples in the stamp.
            for(u32bit s = 0; s < state.msaaSamples; s++)
            {
                //  Build write mask for the stamp.
                writeMask[(f * state.msaaSamples + s) * bytesPerSample + 3] = (state.stencilUpdateMask == 0) ? false : true;
                writeMask[(f * state.msaaSamples + s) * bytesPerSample + 2] =
                writeMask[(f * state.msaaSamples + s) * bytesPerSample + 1] =
                writeMask[(f * state.msaaSamples + s) * bytesPerSample + 0] = state.depthMask;
            }
        }
    }

    GPU_DEBUG(
        printf("Input Depth = ");
        for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
            printf("%08x ", inputDepth[f]);
        printf("\n");
        printf("Culled = ");
        for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
            printf("%d ", culledFragments[f]);
        printf("\n");
        printf("Write Mask = ");
        for(u32bit b = 0; b < (STAMP_FRAGMENTS * 4); b++)
            printf("%02x ", writeMask[b]);
        printf("\n");
    )

    FragmentQuadMemoryUpdateInfo quadMemUpdate;
    bool anyNotAlreadyCulled;
    
    //  Check if multisampling is enabled
    if (!state.multiSampling)
    {
        //  Check if validation mode is enabled.
        if (validationMode)
        {
            //  Check if there are unculled fragments in the quad.
            anyNotAlreadyCulled = false;
            for(u32bit f = 0; (f < STAMP_FRAGMENTS) && !anyNotAlreadyCulled; f++)
                anyNotAlreadyCulled = anyNotAlreadyCulled || !culledFragments[f];

            //  Store the information for the quad if there is a valid fragment.
            if (anyNotAlreadyCulled)
            {
                //  Set the quad identifier to the top left fragment of the quad.
                quadMemUpdate.fragID.x = quad[0]->getFragment()->getX();
                quadMemUpdate.fragID.y = quad[0]->getFragment()->getY();
                quadMemUpdate.fragID.triangleID = triangleCounter;
                
                quadMemUpdate.fragID.sample = 0;
                
                //  Copy the input (z computed for fragment) and read data (z/stencil from the z stencil buffer) for
                //  the quad.
                for(u32bit qw = 0; qw < ((bytesPerPixelDepth * 4) >> 3); qw++)
                {
                    ((u64bit *) quadMemUpdate.inData)[qw] = ((u64bit *) inputDepth)[qw];
                    ((u64bit *) quadMemUpdate.readData)[qw] = ((u64bit *) zStencilInOutData)[qw];
                }
            }
        }
        
        //  Perform Stencil and Z tests.
        fragEmu->stencilZTest(inputDepth, zStencilInOutData, culledFragments);

        //  Update cull mask for the fragments.
        for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
        {
            if (culledFragments[f])
                quad[f]->setAsCulled();
        }

        //  Check if validation mode is enabled.
        if (validationMode)
        {
            //  Check if there are unculled fragments in the quad.
            anyNotAlreadyCulled = false;
            for(u32bit f = 0; (f < STAMP_FRAGMENTS) && !anyNotAlreadyCulled; f++)
                anyNotAlreadyCulled = anyNotAlreadyCulled || !culledFragments[f];

            //  Store information for the quad and add to the z/stencil memory update map.
            if (anyNotAlreadyCulled)
            {
                //  Store the result z and stencil results for the quad.
                for(u32bit qw = 0; qw < ((bytesPerPixelDepth * 4) >> 3); qw++)
                    ((u64bit *) quadMemUpdate.writeData)[qw] = ((u64bit *) zStencilInOutData)[qw];
                    
                //  Store the write mask for the quad after the write.
                for(u32bit b = 0; b < (STAMP_FRAGMENTS * bytesPerPixelDepth); b++)
                    quadMemUpdate.writeMask[b] = writeMask[b];
                    
                //  Store the write mask for the quad after write.
                for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
                    quadMemUpdate.cullMask[f] = culledFragments[f];
                    
                //  Store the information for the quad in the z stencil memory update map.
                zstencilMemoryUpdateMap.insert(make_pair(quadMemUpdate.fragID, quadMemUpdate));
            }
        }
    }
    else
    {
        //  Create cull mask for the samples

        //  Perform Stencil and Z tests for all the stamps.
        for(u32bit s = 0; s < state.msaaSamples; s++)
            fragEmu->stencilZTest(&inputDepth[s * STAMP_FRAGMENTS],
                                  &zStencilInOutData[s * STAMP_FRAGMENTS],
                                  &sampleCullMask[s * STAMP_FRAGMENTS]);

        //  Update coverage mask for all the fragments in the stamp.
        for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
        {
            //  Get pointer to the coverage mask for the fragment.
            bool *fragmentCoverage = quad[f]->getFragment()->getMSAACoverage();

            bool allSamplesCulled = true;

            //  Update coverage mask for the sample.
            for(u32bit s = 0; s < state.msaaSamples; s++)
            {
                fragmentCoverage[s] = !sampleCullMask[f * state.msaaSamples + s];

                //  Check if all the samples in the fragment were culled.
                allSamplesCulled = allSamplesCulled && sampleCullMask[f * state.msaaSamples + s];
            }

            //  Set cull mask for the fragment.
            if (allSamplesCulled)
                quad[f]->setAsCulled();
        }
    }

    GPU_DEBUG(
        printf("Result Z Stencil Data = ");
        for(u32bit b = 0; b < STAMP_FRAGMENTS; b++)
            printf("%08x ", zStencilInOutData[b]);
        printf("\n");
        printf("Culled = ");
        for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
            printf("%d ", culledFragments[f]);
        printf("\n");
    )

    //  Check if multisampling is enabled.
    if (!state.multiSampling)
    {
        //  Masked write of the result color.
        for(u32bit b = 0; b < STAMP_FRAGMENTS * bytesPerSample; b++)
        {
            if (writeMask[b])
                zStencilData[b] = ((u8bit *) zStencilInOutData)[b];
        }
    }
    else
    {
        //  Masked write of the result color.
        for(u32bit b = 0; b < STAMP_FRAGMENTS * state.msaaSamples * bytesPerSample; b++)
        {
            if (writeMask[b])
                zStencilData[b] = ((u8bit *) zStencilInOutData)[b];
        }
    }

    GPU_DEBUG(
        printf("Final Z Stencil Data = ");
        for(u32bit b = 0; b < (STAMP_FRAGMENTS * 4); b++)
            printf("%02x ", zStencilData[b]);
        printf("\n--------\n");
    )

    GLOBALPROFILER_EXITREGION()
}

void GPUEmulator::cleanup()
{
    indexList.clear();

    map<u32bit, ShadedVertex *>::iterator it = vertexList.begin();

    while (it != vertexList.end())
    {
        delete it->second;
        it++;
    }

    vertexList.clear();
}

void GPUEmulator::emulateBlitter()
{
    //if (state.colorBufferFormat != GPU_RGBA8888)
    if (state.rtFormat[0] != GPU_RGBA8888)
    {
        panic("GPUEmulator", "emulateBlitter", "Blitter only supports RGBA8 color buffer.");
    }
    //  Set display parameters in the Pixel Mapper.
    u32bit samples = state.multiSampling ? state.msaaSamples : 1;
    u32bit bytesPixel;

    switch(state.rtFormat[0])
    {
        case GPU_RGBA8888:
        case GPU_RG16F:
        case GPU_R32F:
            bytesPixel = 4;
            break;
        case GPU_RGBA16:
        case GPU_RGBA16F:
            bytesPixel = 8;
            break;
    }

    pixelMapper[0].setupDisplay(state.displayResX, state.displayResY, STAMP_WIDTH, STAMP_WIDTH,
                             simP.ras.genWidth / STAMP_WIDTH, simP.ras.genHeight / STAMP_HEIGHT,
                             simP.ras.scanWidth / simP.ras.genWidth, simP.ras.scanHeight / simP.ras.genHeight,
                             simP.ras.overScanWidth, simP.ras.overScanHeight,
                             samples, bytesPixel);

    //u8bit *sourceMemory = selectMemorySpace(state.backBufferBaseAddr);
    u8bit *sourceMemory = selectMemorySpace(state.rtAddress[0]);
    u8bit *destMemory = selectMemorySpace(state.blitDestinationAddress);

    //  Initialize blitter pixel mapper if required.
    if (state.blitDestinationTextureBlocking == GPU_TXBLOCK_FRAMEBUFFER)
    {
        // Setup display in the Pixel Mapper.
        //u32bit samples = state.multiSampling ? state.msaaSamples : 1;
        blitPixelMapper.setupDisplay(state.blitWidth, state.blitHeight, STAMP_WIDTH, STAMP_WIDTH,
                                     simP.ras.genWidth / STAMP_WIDTH, simP.ras.genHeight / STAMP_HEIGHT,
                                     simP.ras.scanWidth / simP.ras.genWidth, simP.ras.scanHeight / simP.ras.genHeight,
                                     simP.ras.overScanWidth, simP.ras.overScanHeight, 1, 4);
    }

    //  Compute the bytes per texel.
    u32bit bytesTexel;
    switch(state.blitDestinationTextureFormat)
    {
        case GPU_RGBA8888:

            bytesTexel = 4;
            break;

        default :

            panic("GPUEmulator", "emulateBlitter", "Unsupported blit format.");
            break;
    }

    //  Copy all pixel/texels to the destination buffer.
    for(u32bit y = 0; y < state.blitHeight; y++)
    {
        u32bit yPix = y + state.blitIniY;
        u32bit yTex = y + state.blitYOffset;

        for(u32bit x = 0; x < state.blitWidth; x++)
        {
            u32bit xPix = x + state.blitIniX;
            u32bit xTex = x + state.blitXOffset;

            //  Compute the address of the texel inside the texture.
            u32bit texelAddress;
            switch(state.blitDestinationTextureBlocking)
            {
                case GPU_TXBLOCK_TEXTURE:

                    // Compute the morton address for the texel.
                    texelAddress = texel2MortonAddress(xTex, yTex, simP.fsh.textBlockDim, simP.fsh.textSBlockDim, state.blitTextureWidth2) * bytesTexel;
                    break;

                case GPU_TXBLOCK_FRAMEBUFFER:

                    // Compute the framebuffer tiling/blocking address for the first corner texel of the block.
                    texelAddress = blitPixelMapper.computeAddress(xTex, yTex);
                    break;

                default:

                    panic("GPUEmulator", "emulateBlitter", "Unsupported blocking mode.");
                    break;
            }

            //  Compute the address of the pixel inside the color buffer.
            u32bit pixelAddress = pixelMapper[0].computeAddress(xPix, yPix);

            //pixelAddress += state.backBufferBaseAddr;
            pixelAddress += (state.rtAddress[0] & SPACE_ADDRESS_MASK);
            texelAddress += (state.blitDestinationAddress & SPACE_ADDRESS_MASK);

            //  Copy pixel data to texel data (only for RGBA8).
            *((u32bit *) &destMemory[texelAddress]) = *((u32bit *) &sourceMemory[pixelAddress]);
        }
    }
}

u32bit GPUEmulator::texel2MortonAddress(u32bit i, u32bit j, u32bit blockDim, u32bit sBlockDim, u32bit width)
{
    u32bit address;
    u32bit texelAddr;
    u32bit blockAddr;
    u32bit sBlockAddr;

    /*  Compute the address of the texel inside the block using Morton order.  */
    texelAddr = GPUMath::morton(blockDim, i, j);

    /*  Compute the address of the block inside the superblock using Morton order.  */
    blockAddr = GPUMath::morton(sBlockDim, i >> blockDim, j >> blockDim);

    /*  Compute the address of the superblock inside the cache.  */
    sBlockAddr = ((j >> (sBlockDim + blockDim)) << GPU_MAX(s32bit(width - (sBlockDim + blockDim)), s32bit(0))) + (i >> (sBlockDim + blockDim));

    /*  Compute the final address.  */
    address = (((sBlockAddr << (2 * sBlockDim)) + blockAddr) << (2 * blockDim)) + texelAddr;

    return address;
}

void GPUEmulator::printTextureAccessInfo(TextureAccess *texAccess)
{
    printf("Texture Trace\n");
    printf("-------------\n");
    printf("        Texture Unit = %d\n", texAccess->textUnit);
    printf("          Width = %d\n", state.textureWidth[texAccess->textUnit]);
    printf("          Height = %d\n", state.textureHeight[texAccess->textUnit]);
    if (state.textureMode[texAccess->textUnit] == GPU_TEXTURE3D)
        printf("          Depth = %d\n", state.textureDepth[texAccess->textUnit]);
    printf("          Format = %d\n", state.textureFormat[texAccess->textUnit]);
    printf("          Type = ");
    switch(state.textureMode[texAccess->textUnit])
    {
        case GPU_TEXTURE1D:
            printf("1D\n");
            break;
        case GPU_TEXTURE2D:
            printf("2D\n");
            break;
        case GPU_TEXTURECUBEMAP:
            printf("Cube\n");
            break;
        case GPU_TEXTURE3D:
            printf("3D\n");
            break;
        default:
            printf("Unknown\n");
            break;
    }
    printf("          Blocking = ");
    switch(state.textureBlocking[texAccess->textUnit])
    {
        case GPU_TXBLOCK_TEXTURE:
            printf("TEXTURE\n");
            break;
        case GPU_TXBLOCK_FRAMEBUFFER:
            printf("FRAMEBUFFER\n");
            break;
        default:
            printf("Unknown\n");
    }
    for(u32bit p = 0; p < STAMP_FRAGMENTS; p++)
    {
        printf(" Pixel %d\n", p);
        printf("------------\n");
        printf("         LOD = %f\n", texAccess->lod[p]);
        printf("         LOD0 = %d\n", texAccess->level[p][0]);
        printf("         LOD1 = %d\n", texAccess->level[p][1]);
        f32bit w = texAccess->lod[p] - static_cast<f32bit>(GPU_FLOOR(texAccess->lod[p]));
        printf("         WeightLOD = %f\n", w);
        printf("         Samples = %d\n", texAccess->anisoSamples);
        if (state.textureMode[texAccess->textUnit] == GPU_TEXTURECUBEMAP)
        {
            printf("Pixel %d Coord (orig) = (%f, %f, %f, %f)\n", p,
                texAccess->originalCoord[p][0], texAccess->originalCoord[p][1],
                texAccess->originalCoord[p][2], texAccess->originalCoord[p][3]);
            printf("Pixel %d Coord (post cube) = (%f, %f, %f, %f)\n", p,
                texAccess->coordinates[p][0], texAccess->coordinates[p][1],
                texAccess->coordinates[p][2], texAccess->coordinates[p][3]);
            printf("Face = %d (", texAccess->cubemap);
            switch(texAccess->cubemap)
            {
                case GPU_CUBEMAP_NEGATIVE_X:
                    printf("-X)\n");
                    break;
                case GPU_CUBEMAP_POSITIVE_X:
                    printf("+X)\n");
                    break;
                case GPU_CUBEMAP_NEGATIVE_Y:
                    printf("-Y)\n");
                    break;
                case GPU_CUBEMAP_POSITIVE_Y:
                    printf("+Y)\n");
                    break;
                case GPU_CUBEMAP_NEGATIVE_Z:
                    printf("-Z)\n");
                    break;
                case GPU_CUBEMAP_POSITIVE_Z:
                    printf("+Z)\n");
                    break;
                default:
                    printf("Unknown face)\n");
                    break;
            }
        }
        else
        {
            printf("Pixel %d Coord = (%f, %f, %f, %f)\n", p,
                texAccess->coordinates[p][0], texAccess->coordinates[p][1],
                texAccess->coordinates[p][2], texAccess->coordinates[p][3]);
        }
     
        for(u32bit s = 0; s < texAccess->anisoSamples; s++)
        {
            printf("  Sample %d\n", s);
            printf("-----------------\n");
            printf("         Sample from two mips = %s\n",
                texAccess->trilinear[s]->sampleFromTwoMips[p] ? "YES":"NO");
            printf("         LOD0\n");
            printf("          WeightA = %f\n", texAccess->trilinear[s]->a[p][0]);
            printf("          WeightB = %f\n", texAccess->trilinear[s]->b[p][0]);
            printf("          WeightC = %f\n", texAccess->trilinear[s]->c[p][0]);
            printf("         LOD1\n");
            printf("          WeightA = %f\n", texAccess->trilinear[s]->a[p][1]);
            printf("          WeightB = %f\n", texAccess->trilinear[s]->b[p][1]);
            printf("          WeightC = %f\n", texAccess->trilinear[s]->c[p][1]);
            printf("         Loops = %d\n", texAccess->trilinear[s]->loops[p]);
            for(u32bit b = 0; b < texAccess->trilinear[s]->loops[p]; b++)
            {
                u32bit texelsLoop = texAccess->trilinear[s]->texelsLoop[p];
                printf("  Loop %d\n", b);
                printf("-------------\n");
                printf("         Texels/Loop = %d\n", texelsLoop);
                for(u32bit t = 0; t < texelsLoop; t++)
                {
                    printf("         Texel = (%d, %d)\n",
                    texAccess->trilinear[s]->i[p][b * texelsLoop + t],
                    texAccess->trilinear[s]->j[p][b * texelsLoop + t]);
                    if (state.textureMode[texAccess->textUnit] == GPU_TEXTURE3D)
                        printf("         Slice = %d\n", texAccess->trilinear[s]->k[p][b * texelsLoop + t]);
                    printf("         Address = %08x\n",
                        texAccess->trilinear[s]->address[p][b * texelsLoop + t]);
                    printf("         Value = {%f, %f, %f, %f} [%02x, %02x, %02x, %02x]\n",
                        texAccess->trilinear[s]->texel[p][b * texelsLoop + t][0],
                        texAccess->trilinear[s]->texel[p][b * texelsLoop + t][1],
                        texAccess->trilinear[s]->texel[p][b * texelsLoop + t][2],
                        texAccess->trilinear[s]->texel[p][b * texelsLoop + t][3],
                        u32bit(255.0f * texAccess->trilinear[s]->texel[p][b * texelsLoop + t][0]),
                        u32bit(255.0f * texAccess->trilinear[s]->texel[p][b * texelsLoop + t][1]),
                        u32bit(255.0f * texAccess->trilinear[s]->texel[p][b * texelsLoop + t][2]),
                        u32bit(255.0f * texAccess->trilinear[s]->texel[p][b * texelsLoop + t][3]));
                }
            }
        }
        printf("         Result = {%f, %f, %f, %f}\n",
            texAccess->sample[p][0], texAccess->sample[p][1],
            texAccess->sample[p][2], texAccess->sample[p][3]);
    }

    printf("-------------\n");
}

void GPUEmulator::printShaderInstructionOperands(ShaderInstruction::ShaderInstructionDecoded *shDecInstr)
{
    if (shDecInstr->getShaderInstruction()->getNumOperands() > 0)
    {
        switch(shDecInstr->getShaderInstruction()->getBankOp1())
        {
            case gpu3d::TEXT:            
                break;
          
            case gpu3d::PRED:
                {
                    bool *op1 = (bool *) shDecInstr->getShEmulOp1();
                    printf("             OP1 -> %s\n", *op1 ? "true" : "false");
                }
                break;

            default:
            
                //  Check for predicator operator instruction with constant register operator.
                switch(shDecInstr->getShaderInstruction()->getOpcode())
                {
                    case gpu3d::ANDP:
                        {
                            bool *op1 = (bool *) shDecInstr->getShEmulOp1();
                            printf("             OP1 -> {%s, %s, %s, %s}\n", op1[0] ? "true" : "false", op1[1] ? "true" : "false",
                                op1[2] ? "true" : "false", op1[3] ? "true" : "false");
                        }
                        break;
                      
                    case gpu3d::ADDI:
                    case gpu3d::MULI:
                    case gpu3d::STPEQI:
                    case gpu3d::STPGTI:
                    case gpu3d::STPLTI:
                        {
                            s32bit *op1 = (s32bit *) shDecInstr->getShEmulOp1();
                            printf("             OP1 -> {%d, %d, %d, %d}\n", op1[0], op1[1], op1[2], op1[3]);
                        }                        
                        break;
                          
                    default:
                        {
                            f32bit *op1 = (f32bit *) shDecInstr->getShEmulOp1();
                            printf("             OP1 -> {%f, %f, %f, %f}\n", op1[0], op1[1], op1[2], op1[3]);
                        }                        
                        break;
                }
                break;                
        }
    }
    
    if (shDecInstr->getShaderInstruction()->getNumOperands() > 1)
    {
        switch(shDecInstr->getShaderInstruction()->getBankOp2())
        {
            case gpu3d::TEXT:            
                break;
          
            case gpu3d::PRED:
                {
                    bool *op2 = (bool *) shDecInstr->getShEmulOp2();
                    printf("             OP2 -> %s\n", *op2 ? "true" : "false");
                }
                break;
                
            default:
            
                //  Check for predicator operator instruction with constant register operator.
                switch(shDecInstr->getShaderInstruction()->getOpcode())
                {
                    case gpu3d::ANDP:
                        {
                            bool *op2 = (bool *) shDecInstr->getShEmulOp2();
                            printf("             OP2 -> {%s, %s, %s, %s}\n", op2[0] ? "true" : "false", op2[1] ? "true" : "false",
                                op2[2] ? "true" : "false", op2[3] ? "true" : "false");
                        }
                        break;
                      
                    case gpu3d::ADDI:
                    case gpu3d::MULI:
                    case gpu3d::STPEQI:
                    case gpu3d::STPGTI:
                    case gpu3d::STPLTI:
                        {
                            s32bit *op2 = (s32bit *) shDecInstr->getShEmulOp2();
                            printf("             OP2 -> {%d, %d, %d, %d}\n", op2[0], op2[1], op2[2], op2[3]);
                        }                        
                        break;
                          
                    default:
                        {
                            f32bit *op2 = (f32bit *) shDecInstr->getShEmulOp2();
                            printf("             OP2 -> {%f, %f, %f, %f}\n", op2[0], op2[1], op2[2], op2[3]);
                        }                        
                        break;
                }
                
                break;
        }
    }
    
    if (shDecInstr->getShaderInstruction()->getNumOperands() > 2)
    {
        switch(shDecInstr->getShaderInstruction()->getBankOp3())
        {
            case gpu3d::TEXT:            
                break;
          
            case gpu3d::PRED:
                {
                    bool *op3 = (bool *) shDecInstr->getShEmulOp3();
                    printf("             OP3 -> %s\n", *op3 ? "true" : "false");
                }
                break;
                
            default:
                //  Check for predicator operator instruction with constant register operator.
                switch(shDecInstr->getShaderInstruction()->getOpcode())
                {
                    case gpu3d::ANDP:
                        {
                            bool *op3 = (bool *) shDecInstr->getShEmulOp3();
                            printf("             OP3 -> {%s, %s, %s, %s}\n", op3[0] ? "true" : "false", op3[1] ? "true" : "false",
                                op3[2] ? "true" : "false", op3[3] ? "true" : "false");
                        }
                        break;
                      
                    case gpu3d::ADDI:
                    case gpu3d::MULI:
                    case gpu3d::STPEQI:
                    case gpu3d::STPGTI:
                    case gpu3d::STPLTI:
                        {
                            s32bit *op3 = (s32bit *) shDecInstr->getShEmulOp3();
                            printf("             OP3 -> {%d, %d, %d, %d}\n", op3[0], op3[1], op3[2], op3[3]);
                        }                        
                        break;
                          
                    default:
                        {
                            f32bit *op3 = (f32bit *) shDecInstr->getShEmulOp3();
                            printf("             OP3 -> {%f, %f, %f, %f}\n", op3[0], op3[1], op3[2], op3[3]);
                        }                        
                        break;
                }
                
                break;
        }
    }

    if (shDecInstr->getShaderInstruction()->getPredicatedFlag())
    {
        bool *pred = (bool *) shDecInstr->getShEmulPredicate();
        printf("             PREDICATE -> %s\n", *pred ? "true" : "false");
    }
}

void GPUEmulator::printShaderInstructionResult(ShaderInstruction::ShaderInstructionDecoded *shDecInstr)
{
    if (shDecInstr->getShaderInstruction()->hasResult())
    {
        switch(shDecInstr->getShaderInstruction()->getBankRes())
        {
            case PRED:
                {
                    bool *res = (bool *) shDecInstr->getShEmulResult();                
                    printf("             RESULT -> %s\n", *res ? "true" : "false");
                }                
                break;

            default:
            
                switch(shDecInstr->getShaderInstruction()->getOpcode())
                {
                    case ADDI:
                    case MULI:
                        {
                            s32bit *res = (s32bit *) shDecInstr->getShEmulResult();                
                            printf("             RESULT -> {%d, %d, %d, %d}\n", res[0], res[1], res[2], res[3]);
                        }
                        break;
                        
                    default:
                        {
                            f32bit *res = (f32bit *) shDecInstr->getShEmulResult();                
                            printf("             RESULT -> {%f, %f, %f, %f}\n", res[0], res[1], res[2], res[3]);
                        }
                        break;
                }
                break;
        }
    }
}

void GPUEmulator::emulationLoop()
{
    GLOBALPROFILER_ENTERREGION("emulationLoop", "", "")

    traceEnd = false;

    //  Start the trace driver.
    trDriver->startTrace();

    resetState();

    while(!traceEnd && (frameCounter < (simP.startFrame + simP.simFrames)) && !abortEmulation)
    {
        AGPTransaction *currentTransaction;

        GLOBALPROFILER_ENTERREGION("driver", "", "")

        //  Get next transaction from the driver
        currentTransaction = trDriver->nextAGPTransaction();

        GLOBALPROFILER_EXITREGION()

        if (currentTransaction != NULL)
        {
            emulateCommandProcessor(currentTransaction);
        }
        else
        {
            traceEnd = true;
            printf("End of trace.\n");
        }
    }

if (abortEmulation)
printf("Emulation aborted\n");

    GLOBALPROFILER_EXITREGION()
}

void GPUEmulator::setAbortEmulation()
{
    abortEmulation = true;
}

u32bit GPUEmulator::getFrameCounter()
{
    return frameCounter;
}

u32bit GPUEmulator::getBatchCounter()
{
    return batchCounter;
}

u32bit GPUEmulator::getTriangleCounter()
{
    return triangleCounter;
}

void GPUEmulator::setValidationMode(bool enable)
{
    validationMode = enable;
}

ShadedVertexMap &GPUEmulator::getShadedVertexLog()
{
    return shadedVertexLog;
}

VertexInputMap &GPUEmulator::getVertexInputLog()
{
    return vertexInputLog;
}

void GPUEmulator::setFullTraceLog(bool enable)
{
    traceLog = enable;
}

void GPUEmulator::setBatchTraceLog(bool enable, u32bit batch)
{
    traceBatch = enable;
    watchBatch = batch;
}

void GPUEmulator::setVertexTraceLog(bool enable, u32bit index)
{
    traceVertex = enable;
    watchIndex = index;
}

void GPUEmulator::setPixelTraceLog(bool enable, u32bit x, u32bit y)
{
    tracePixel = enable;
    watchPixelX = x;
    watchPixelY = y;
}

FragmentQuadMemoryUpdateMap &GPUEmulator::getZStencilUpdateMap()
{
    return zstencilMemoryUpdateMap;
}

FragmentQuadMemoryUpdateMap &GPUEmulator::getColorUpdateMap(u32bit rt)
{
    GPU_ASSERT(
        if (rt > MAX_RENDER_TARGETS)
            panic("GPUEmulator", "getColorUpdateMap", "Render target identifier out of range.");
    )
    
    return colorMemoryUpdateMap[rt];
}

TextureFormat GPUEmulator::getRenderTargetFormat(u32bit rt)
{
    GPU_ASSERT(
        if (rt > MAX_RENDER_TARGETS)
            panic("GPUEmulator", "getColorUpdateMap", "Render target identifier out of range.");
    )

    return state.rtFormat[rt];
}

//  Save a snapshot of the current emulator state.
void GPUEmulator::saveSnapshot()
{
    ofstream out;

    out.open("emu.registers.snapshot", ios::binary);

    if (!out.is_open())
        panic("GPUEmulator", "saveSnapshot", "Error creating register snapshot file.");

    out.write((char *) &state, sizeof(state));
    out.close();
    
    out.open("emu.gpumem.snapshot", ios::binary);
    
    if (!out.is_open())
        panic("GPUEmulator", "saveSnapshot", "Error creating gpu memory snapshot file.");
    
    out.write((char *) gpuMemory, simP.mem.memSize * 1024 * 1024);    
    out.close();
    
    out.open("emu.sysmem.snapshot", ios::binary);
   
    if (!out.is_open())
        panic("GPUEmulator", "saveSnapshot", "Error creating system memory snapshot file.");
        
    out.write((char *) sysMemory, simP.mem.mappedMemSize * 1024 * 1024);
    
    out.close();
}

//  Load an emulator state snapshot.
void GPUEmulator::loadSnapshot()
{
    ifstream input;

    input.open("emu.registers.snapshot", ios::binary);

    if (!input.is_open())
        panic("GPUEmulator", "loadSnapshot", "Error opening register snapshot file.");

    input.read((char *) &state, sizeof(state));
    input.close();
    
    input.open("emu.gpumem.snapshot", ios::binary);
    
    if (!input.is_open())
        panic("GPUEmulator", "loadSnapshot", "Error opening gpu memory snapshot file.");
    
    input.read((char *) gpuMemory, simP.mem.memSize * 1024 * 1024);    
    input.close();
    
    input.open("emu.sysmem.snapshot", ios::binary);
   
    if (!input.is_open())
        panic("GPUEmulator", "loadSnapshot", "Error opening system memory snapshot file.");
        
    input.read((char *) sysMemory, simP.mem.mappedMemSize * 1024 * 1024);
    
    input.close();
}

//  Set skip draw call mode.
void GPUEmulator::setSkipBatch(bool enable)
{
    skipBatchMode = enable;
}

//
//
//  TODO:
//
//   - bypass attributes in streamer stage
//   - two sided lighting at setup
//


}  // namespace gpu3d
