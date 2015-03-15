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
 * $RCSfile: AGPTransaction.cpp,v $
 * $Revision: 1.20 $
 * $Author: vmoya $
 * $Date: 2008-03-02 19:09:17 $
 *
 * AGP Transaction class implementation file.
 *
 */

#include "AGPTransaction.h"
#include <iostream>
#include <cstring>

using namespace std;
using namespace gpu3d;

/* Macro for fast typing in AGPTransaction::dump() method */
#define CASE_PRINT(value)\
    case  value:\
        os << #value;\
        break;


/*  AGP Transaction constructor.  For memory operations.  */
AGPTransaction::AGPTransaction(u32bit addr, u32bit dataSize, u8bit *dataBuffer, u32bit _md, bool isWrite, bool isLocked):

/*  Set object attributes.  */
address(addr), size(dataSize), data(dataBuffer), md(_md), locked(isLocked)

{
    if ( dataSize == 0 )
        panic("AGPTransaction", "AGPTransaction", "Trying to create an AGPTransaction (memory op) with size 0");

    /* create a copy of data */
    data = new u8bit[dataSize]; // "slow new"
    memcpy(data, dataBuffer,dataSize);

    /*  Set as a write (system to local) memory operation.  */
    if (isWrite)
        agpTrans = AGP_WRITE;
    else
    /*  Or as a read (local to system) memory operation.  */
        agpTrans = AGP_READ;

    /*  Calculate number of packets for this transaction.  */
    numPackets = 1 + (size >> AGP_PACKET_SHIFT) + GPU_MOD(GPU_MOD(numPackets, AGP_PACKET_SIZE), 1);

    /*  Set object color for tracing.  */
    setColor(agpTrans);

    setTag("AGPTr");

    
    //dump();
}

/*  AGP Transaction constructor.  For preloading memory operations.  */
AGPTransaction::AGPTransaction(u32bit addr, u32bit dataSize, u8bit *dataBuffer, u32bit _md):

/*  Set object attributes.  */
address(addr), size(dataSize), data(dataBuffer), md(_md), locked(true)

{
    if ( dataSize == 0 )
        panic("AGPTransaction", "AGPTransaction", "Trying to create an AGPTransaction (memory preload) with size 0");

    /* create a copy of data */
    data = new u8bit[dataSize]; // "slow new"
    memcpy(data, dataBuffer,dataSize);

    /*  Set as a preload (system or local) memory operation.  */
    agpTrans = AGP_PRELOAD;

    /*  Calculate number of packets for this transaction.  */
    numPackets = 1;

    /*  Set object color for tracing.  */
    setColor(agpTrans);

    setTag("AGPTr");

    
    //dump();
}

/*  AGP Transaction constructor.  For register write operations.  */
AGPTransaction::AGPTransaction(GPURegister gpuR, u32bit subR, GPURegData rData, u32bit _md):

/*  Set object attributes.  */
agpTrans(AGP_REG_WRITE), gpuReg(gpuR), subReg(subR), regData(rData), md(_md),
numPackets(1), locked(false)
{
    /*  Set object color for tracing.  */
    setColor(AGP_REG_WRITE);

    setTag("AGPTr");

}


/*  AGP Transaction constructor.  For register read operations.  */
AGPTransaction::AGPTransaction(GPURegister gpuR, u32bit subR):

/*  Set object attributes.  */
agpTrans(AGP_REG_READ),gpuReg(gpuR), subReg(subR), md(0), numPackets(1), locked(false)
{
    /*  Set object color for tracing.  */
    setColor(AGP_REG_READ);

    setTag("AGPTr");
}


/*  AGP Transaction constructor.  For GPU control commands.  */
AGPTransaction::AGPTransaction(GPUCommand gpuComm):

/*  Set object attributes.  */
agpTrans(AGP_COMMAND), gpuCommand(gpuComm), md(0), numPackets(1), locked(false)
{
    /*  Set object color for tracing.  */
    setColor(AGP_COMMAND);

    setTag("AGPTr");
}

// AGP Transaction constructor.  Creates an AGP_INIT_END transaction.
AGPTransaction::AGPTransaction() :
    agpTrans(AGP_INIT_END), md(0), numPackets(1), locked(false)
{
    setTag("AGPTr");
}

//  AGP Transaction constructor.  Creates an AGP_EVENT transaction.
AGPTransaction::AGPTransaction(GPUEvent gpuEvent, string msg) :

//  Set object attributes.
agpTrans(AGP_EVENT), gpuEvent(gpuEvent), eventMsg(msg)

{
    //  Set object color for tracing.
    setColor(AGP_EVENT);
    
    setTag("AGPTr");
}

//  AGP Transaction constructor.  Load from AGP Transaction trace file.
AGPTransaction::AGPTransaction(AGPTransaction *sourceAGPTrans)
{
    //  Clone the transaction type.
    agpTrans = sourceAGPTrans->agpTrans;
    
    //  Clone transaction parameters based on the transaction type.
    switch(sourceAGPTrans->agpTrans)
    {
        case AGP_WRITE:
        case AGP_PRELOAD:
        
            address = sourceAGPTrans->address;
            md = sourceAGPTrans->md;
            locked = sourceAGPTrans->locked;
            size = sourceAGPTrans->size;
            
            //  Allocate space for the transaction data
            data = new u8bit[size];
            
            //  Check allocation
            if (data == NULL)
                panic("AGPTransaction", "AGPTransaction(AGPTransaction)", "Error allocating memory for AGP_WRITE/AGP_PRELOAD transaction data buffer.");

            memcpy(data, sourceAGPTrans->data, size);            
        
            break;
            
        case AGP_REG_WRITE:
        
            gpuReg = sourceAGPTrans->gpuReg;
            subReg = sourceAGPTrans->subReg;
            memcpy(&regData, &sourceAGPTrans->regData, sizeof(GPURegData));
            md = sourceAGPTrans->md;
            
            break;
            
        case AGP_COMMAND:
        
            gpuCommand = sourceAGPTrans->gpuCommand;
            
            break;
            
        case AGP_EVENT:
        
            gpuEvent = sourceAGPTrans->gpuEvent;
            eventMsg =  sourceAGPTrans->eventMsg;
                   
            break;
        
        case AGP_INIT_END:
        
            break;           

        case AGP_READ:
        
            panic("AGPTransaction", "AGPTransaction(load)", "AGP_READ transactions not supported.");
            break;

        case AGP_REG_READ:
            panic("AGPTransaction", "AGPTransaction(load)", "AGP_REG_READ transactions not supported.");
            break;
        
        default:
            panic("AGPTransaction", "AGPTransaction(load)", "Unknown AGP transaction type.");
            break;
    }
    
    //  Set dynamic object color for signal tracing.
    setColor(agpTrans);
    
    //  Set dynamic object tag.
    setTag("AGPTr");
}

//  AGP Transaction constructor.  Load from AGP Transaction trace file.
AGPTransaction::AGPTransaction(gzifstream *traceFile)
{
    u32bit stringLength;
    u8bit *stringData;
    
    traceFile->read((char *) &agpTrans, sizeof(agpTrans));
    
    if (traceFile->eof())
        return;
        
    //  Load transaction parameters based on the transaction type.
    switch(agpTrans)
    {
        case AGP_WRITE:
        case AGP_PRELOAD:
        
            traceFile->read((char *) &address, sizeof(address));
            traceFile->read((char *) &md, sizeof(md));
            traceFile->read((char *) &locked, sizeof(locked));
            traceFile->read((char *) &size, sizeof(size));
            
            if (traceFile->eof())
                return;
                
            //  Allocate space for the transaction data
            data = new u8bit[size];
            
            //  Check allocation
            if (data == NULL)
                panic("AGPTransaction", "AGPTransaction(load)", "Error allocating memory for AGP_WRITE/AGP_PRELOAD transaction data buffer.");
            
            traceFile->read((char *) data, size);
        
            break;
            
        case AGP_REG_WRITE:
        
            traceFile->read((char *) &gpuReg, sizeof(gpuReg));
            traceFile->read((char *) &subReg, sizeof(subReg));
            traceFile->read((char *) &regData, sizeof(regData));
            traceFile->read((char *) &md, sizeof(md));
            
            break;
            
        case AGP_COMMAND:
        
            traceFile->read((char *) &gpuCommand, sizeof(gpuCommand));
            
            break;
            
        case AGP_EVENT:
        
            traceFile->read((char *) &gpuEvent, sizeof(gpuEvent));
            traceFile->read((char *) &stringLength, sizeof(stringLength));

            //  Check if there was a message associated with the event.
            if (stringLength > 0)
            {
                //  Allocate the string.
                stringData = new u8bit[stringLength];
                
                // Read the string.
                traceFile->read((char *) stringData, stringLength);
                
                // Set the event string.
                eventMsg = (char *) stringData;   
            
                //  Deallocate the string.
                delete[] stringData;
            }
            else
            {
                //  Set empty event string.
                eventMsg = "";
            }
                   
            break;
        
        case AGP_INIT_END:
        
            panic("AGPTransaction", "AGPTransaction(load)", "AGP_INIT_END shouldn't be stored in an AGP Transaction trace.");
            break;           

        case AGP_READ:
        
            panic("AGPTransaction", "AGPTransaction(load)", "AGP_READ transactions not supported.");
            break;

        case AGP_REG_READ:
            panic("AGPTransaction", "AGPTransaction(load)", "AGP_REG_READ transactions not supported.");
            break;
        
        default:
            panic("AGPTransaction", "AGPTransaction(load)", "Unknown AGP transaction type.");
            break;
    }
    
    //  Set dynamic object color for signal tracing.
    setColor(agpTrans);
    
    //  Set dynamic object tag.
    setTag("AGPTr");
}

/*  Sets the register data field for a register read transaction.  */
void AGPTransaction::setGPURegData(GPURegData rData)
{
    regData = rData;
}

/*  Gets the register data field.  */
GPURegData AGPTransaction::getGPURegData()
{
    return regData;
}

/*  Gets the AGP Transaction type.  */
AGPComm AGPTransaction::getAGPCommand()
{
    return agpTrans;
}

/*  Gets the AGP Transaction memory address for memory transactions.  */
u32bit AGPTransaction::getAddress()
{
    return address;
}

/*  Gets the AGP Transaction data size for memory transactions.  */
u32bit AGPTransaction::getSize()
{
    return size;
}

/*  Gets the AGP Transaction pointer to data for memory transactions.  */
u8bit *AGPTransaction::getData()
{
    return data;
}

/*  Gets the AGP Transaction GPU register identifier for register transactions.  */
GPURegister AGPTransaction::getGPURegister()
{
    return gpuReg;
}

/*  Gets the AGP Transaction GPU subregister number for register transactions.  */
u32bit AGPTransaction::getGPUSubRegister()
{
    return subReg;
}

/*  Gets the GPU control command issued with this AGP Transaction.  */
GPUCommand AGPTransaction::getGPUCommand()
{
    return gpuCommand;
}

/*  Gets the number of AGP packets for this AGP transaction.  */
u32bit AGPTransaction::getNumPackets()
{
    return numPackets;
}

/*  Gets if the AGP transaction is locked against the current batch and must wait.  */
bool AGPTransaction::getLocked()
{
    return locked;
}

//  Gets the memory descriptor identifier associated with the AGP transaction.
u32bit AGPTransaction::getMD()
{
    return md;
}

//  Gets the GPU event.
GPUEvent AGPTransaction::getGPUEvent()
{
    return gpuEvent;
}

//  Gets the event message string.
string AGPTransaction::getGPUEventMsg()
{
    return eventMsg;
}

//  Changes a AGP_WRITE transaction into an AGP_PRELOAD transaction.
void AGPTransaction::forcePreload()
{
    //  Check if the transaction is a write to memory.
    if (agpTrans == AGP_WRITE)
    {
        //  Change to a preload memory write.
        agpTrans = AGP_PRELOAD;
    }
}

//  Save AGP transaction into a file.
void AGPTransaction::save(gzofstream *outFile)
{
    u32bit stringLength;
    
    outFile->write((char *) &agpTrans, sizeof(agpTrans));
 
    //  Write the AGP transaction parameters based on the AGP transaction type.   
    switch(agpTrans)
    {
        case AGP_WRITE:
        case AGP_PRELOAD:
        
            outFile->write((char *) &address, sizeof(address));
            outFile->write((char *) &md, sizeof(md));
            outFile->write((char *) &locked, sizeof(locked));
            outFile->write((char *) &size, sizeof(size));
            outFile->write((char *) data, size);
        
            break;
            
        case AGP_REG_WRITE:
        
            outFile->write((char *) &gpuReg, sizeof(gpuReg));
            outFile->write((char *) &subReg, sizeof(subReg));
            outFile->write((char *) &regData, sizeof(regData));
            outFile->write((char *) &md, sizeof(md));
        
            break;
            
        case AGP_COMMAND:
        
            outFile->write((char *) &gpuCommand, sizeof(gpuCommand));
            
            break;
        
        case AGP_EVENT:
        
            outFile->write((char *) &gpuEvent, sizeof(gpuEvent));
            
            //  Get the event message string lenght.
            stringLength = eventMsg.length();
            
            outFile->write((char *) &stringLength, sizeof(stringLength));
            
            //  Check if the event message is not empty.
            if (stringLength > 0)
                outFile->write((char *) eventMsg.c_str(), stringLength);
           
            break;
            
        case AGP_INIT_END:
            
            panic("AGPTransaction", "save", "AGP_INIT_END transactions can't be saved to an AGP Transaction trace file.");
            break;

        case AGP_READ:
        
            panic("AGPTransaction", "save", "AGP_READ transactions not supported.");
            break;

        case AGP_REG_READ:
            panic("AGPTransaction", "save", "AGP_REG_READ transactions not supported.");
            break;
        
        default:
            panic("AGPTransaction", "save", "Unknown AGP transaction type.");
            break;
    }
    
}

void AGPTransaction::dump(std::ostream& os) const
{
    os << "( AGPt: ";
    switch ( agpTrans )
    {
        CASE_PRINT( AGP_WRITE )
        CASE_PRINT( AGP_READ )
        CASE_PRINT( AGP_PRELOAD )
        CASE_PRINT( AGP_REG_WRITE )
        CASE_PRINT( AGP_REG_READ )
        CASE_PRINT( AGP_COMMAND )
        CASE_PRINT( AGP_EVENT )
        CASE_PRINT( AGP_INIT_END )
        default:
            os << agpTrans << "(unkown agp transaction)" << endl;
    }

    bool ignoreU32bit = false;

    if ( agpTrans == AGP_READ || agpTrans == AGP_WRITE || agpTrans == AGP_PRELOAD )
    {
        os << ", Address: " << hex << address << dec << ", Size: " << size <<
            ", Data HC: ";
        u32bit sum = 0;
        for ( u32bit i = 0; i < size; i++ )
        {
            sum += (((u32bit)data[i]) * i) % 255;
        }
        os << sum << ", ";
    }
    else if ( agpTrans == AGP_REG_READ || agpTrans == AGP_REG_WRITE ) {
        os << ", REG: ";
        switch ( gpuReg ) {
            CASE_PRINT( GPU_STATUS );
            CASE_PRINT( GPU_DISPLAY_X_RES )
            CASE_PRINT( GPU_DISPLAY_Y_RES )
            /*  GPU memory registers.  */
            CASE_PRINT( GPU_FRONTBUFFER_ADDR )
            CASE_PRINT( GPU_BACKBUFFER_ADDR )
            CASE_PRINT( GPU_ZSTENCILBUFFER_ADDR )
            CASE_PRINT( GPU_TEXTURE_MEM_ADDR )
            CASE_PRINT( GPU_PROGRAM_MEM_ADDR )
            /*  GPU vertex shader.  */
            CASE_PRINT( GPU_VERTEX_PROGRAM )
            CASE_PRINT( GPU_VERTEX_PROGRAM_PC )
            CASE_PRINT( GPU_VERTEX_PROGRAM_SIZE )
            // CASE_PRINT( GPU_VERTEX_CONSTANT )
            case GPU_VERTEX_CONSTANT:
                ignoreU32bit = true;
                os << "GPU_VERTEX_CONSTANT";
                os << ", SubREG: " << subReg << ", Data(4-f32bit): {" << regData.qfVal[0] << ","
                   << regData.qfVal[1] << ","<< regData.qfVal[2] << ","<< regData.qfVal[3] << "}";
                break;
            CASE_PRINT( GPU_VERTEX_THREAD_RESOURCES )
            CASE_PRINT( GPU_VERTEX_OUTPUT_ATTRIBUTE )
            /*  GPU vertex stream buffer registers.  */
            CASE_PRINT( GPU_VERTEX_ATTRIBUTE_MAP )
            CASE_PRINT( GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE )
            CASE_PRINT( GPU_STREAM_ADDRESS )
            CASE_PRINT( GPU_STREAM_STRIDE )
            CASE_PRINT( GPU_STREAM_DATA )
            CASE_PRINT( GPU_STREAM_ELEMENTS )
            CASE_PRINT( GPU_STREAM_FREQUENCY )
            CASE_PRINT( GPU_STREAM_START )
            CASE_PRINT( GPU_STREAM_COUNT )
            CASE_PRINT( GPU_STREAM_INSTANCES )
            CASE_PRINT( GPU_INDEX_MODE )
            CASE_PRINT( GPU_INDEX_STREAM )
            CASE_PRINT( GPU_D3D9_COLOR_STREAM )
            /*  GPU primitive assembly registers.  */
            CASE_PRINT( GPU_PRIMITIVE )
            /*  GPU clipping and culling registers.  */
            CASE_PRINT( GPU_FRUSTUM_CLIPPING )
            CASE_PRINT( GPU_USER_CLIP )
            CASE_PRINT( GPU_USER_CLIP_PLANE )
            //  End of geometry related GPU registers.
            CASE_PRINT( GPU_LAST_GEOMETRY_REGISTER )
            /*  GPU rasterization registers.  */
            CASE_PRINT( GPU_FACEMODE )
            CASE_PRINT( GPU_CULLING )
            CASE_PRINT( GPU_HIERARCHICALZ )
            CASE_PRINT( GPU_EARLYZ )
            CASE_PRINT( GPU_D3D9_PIXEL_COORDINATES )
            CASE_PRINT( GPU_VIEWPORT_INI_X )
            CASE_PRINT( GPU_VIEWPORT_INI_Y )
            CASE_PRINT( GPU_VIEWPORT_WIDTH )
            CASE_PRINT( GPU_VIEWPORT_HEIGHT )
            CASE_PRINT( GPU_SCISSOR_TEST )
            CASE_PRINT( GPU_SCISSOR_INI_X )
            CASE_PRINT( GPU_SCISSOR_INI_Y )
            CASE_PRINT( GPU_SCISSOR_WIDTH )
            CASE_PRINT( GPU_SCISSOR_HEIGHT )
            CASE_PRINT( GPU_DEPTH_RANGE_NEAR )
            CASE_PRINT( GPU_DEPTH_RANGE_FAR )
            CASE_PRINT( GPU_DEPTH_SLOPE_FACTOR )
            CASE_PRINT( GPU_DEPTH_UNIT_OFFSET )
            CASE_PRINT( GPU_Z_BUFFER_BIT_PRECISSION )
            CASE_PRINT( GPU_D3D9_DEPTH_RANGE )
            CASE_PRINT( GPU_D3D9_RASTERIZATION_RULES )
            CASE_PRINT( GPU_TWOSIDED_LIGHTING )
            CASE_PRINT( GPU_MULTISAMPLING )
            CASE_PRINT( GPU_MSAA_SAMPLES )
            CASE_PRINT( GPU_INTERPOLATION )
            CASE_PRINT( GPU_FRAGMENT_INPUT_ATTRIBUTES )
            /*  GPU fragment registers.  */
            CASE_PRINT( GPU_FRAGMENT_PROGRAM )
            CASE_PRINT( GPU_FRAGMENT_PROGRAM_PC )
            CASE_PRINT( GPU_FRAGMENT_PROGRAM_SIZE )
            CASE_PRINT( GPU_FRAGMENT_CONSTANT )
            CASE_PRINT( GPU_FRAGMENT_THREAD_RESOURCES )
            CASE_PRINT( GPU_MODIFY_FRAGMENT_DEPTH )
            // Shader program registers.
            CASE_PRINT( GPU_SHADER_PROGRAM_ADDRESS )
            CASE_PRINT( GPU_SHADER_PROGRAM_SIZE )
            CASE_PRINT( GPU_SHADER_PROGRAM_LOAD_PC )
            CASE_PRINT( GPU_SHADER_PROGRAM_PC )
            CASE_PRINT( GPU_SHADER_THREAD_RESOURCES )
            // Texture Unit registers
            CASE_PRINT( GPU_TEXTURE_ENABLE )
            CASE_PRINT( GPU_TEXTURE_MODE )
            CASE_PRINT( GPU_TEXTURE_ADDRESS )
            CASE_PRINT( GPU_TEXTURE_WIDTH )
            CASE_PRINT( GPU_TEXTURE_HEIGHT )
            CASE_PRINT( GPU_TEXTURE_DEPTH )
            CASE_PRINT( GPU_TEXTURE_WIDTH2 )
            CASE_PRINT( GPU_TEXTURE_HEIGHT2 )
            CASE_PRINT( GPU_TEXTURE_DEPTH2 )
            CASE_PRINT( GPU_TEXTURE_BORDER )
            CASE_PRINT( GPU_TEXTURE_FORMAT )
            CASE_PRINT( GPU_TEXTURE_REVERSE )
            CASE_PRINT( GPU_TEXTURE_D3D9_COLOR_CONV )
            CASE_PRINT( GPU_TEXTURE_D3D9_V_INV )
            CASE_PRINT( GPU_TEXTURE_COMPRESSION )
            CASE_PRINT( GPU_TEXTURE_BLOCKING )
            CASE_PRINT( GPU_TEXTURE_BORDER_COLOR )
            CASE_PRINT( GPU_TEXTURE_WRAP_S )
            CASE_PRINT( GPU_TEXTURE_WRAP_T )
            CASE_PRINT( GPU_TEXTURE_WRAP_R )
            CASE_PRINT( GPU_TEXTURE_MIN_FILTER )
            CASE_PRINT( GPU_TEXTURE_MAG_FILTER )
            CASE_PRINT( GPU_TEXTURE_ENABLE_COMPARISON )
            CASE_PRINT( GPU_TEXTURE_COMPARISON_FUNCTION )
            CASE_PRINT( GPU_TEXTURE_MIN_LOD )
            CASE_PRINT( GPU_TEXTURE_MAX_LOD )
            CASE_PRINT( GPU_TEXTURE_LOD_BIAS )
            CASE_PRINT( GPU_TEXTURE_MIN_LEVEL )
            CASE_PRINT( GPU_TEXTURE_MAX_LEVEL )
            CASE_PRINT( GPU_TEXT_UNIT_LOD_BIAS )
            CASE_PRINT( GPU_TEXTURE_MAX_ANISOTROPY )
            // Render Target and Color Buffer Registers
            CASE_PRINT( GPU_COLOR_BUFFER_FORMAT )
            CASE_PRINT( GPU_COLOR_COMPRESSION )
            CASE_PRINT( GPU_COLOR_SRGB_WRITE )
            CASE_PRINT( GPU_RENDER_TARGET_ENABLE )
            CASE_PRINT( GPU_RENDER_TARGET_FORMAT )
            CASE_PRINT( GPU_RENDER_TARGET_ADDRESS )            
            //  GPU depth and stencil clear values.
            CASE_PRINT( GPU_Z_BUFFER_CLEAR )
            CASE_PRINT( GPU_STENCIL_BUFFER_CLEAR )
            CASE_PRINT( GPU_ZSTENCIL_STATE_BUFFER_MEM_ADDR )
            //  GPU Stencil test registers.
            CASE_PRINT( GPU_STENCIL_TEST )
            CASE_PRINT( GPU_STENCIL_FUNCTION )
            CASE_PRINT( GPU_STENCIL_REFERENCE )
            CASE_PRINT( GPU_STENCIL_COMPARE_MASK )
            CASE_PRINT( GPU_STENCIL_UPDATE_MASK )
            CASE_PRINT( GPU_STENCIL_FAIL_UPDATE )
            CASE_PRINT( GPU_DEPTH_FAIL_UPDATE )
            CASE_PRINT( GPU_DEPTH_PASS_UPDATE )
            //  GPU Depth test registers.
            CASE_PRINT( GPU_DEPTH_TEST )
            CASE_PRINT( GPU_DEPTH_FUNCTION )
            CASE_PRINT( GPU_DEPTH_MASK )
            CASE_PRINT( GPU_ZSTENCIL_COMPRESSION )
            //  GPU color clear value.
            CASE_PRINT( GPU_COLOR_BUFFER_CLEAR )
            CASE_PRINT( GPU_COLOR_STATE_BUFFER_MEM_ADDR )
            //  GPU Color Blend registers.
            CASE_PRINT( GPU_COLOR_BLEND )
            CASE_PRINT( GPU_BLEND_EQUATION )
            CASE_PRINT( GPU_BLEND_SRC_RGB )
            CASE_PRINT( GPU_BLEND_DST_RGB )
            CASE_PRINT( GPU_BLEND_SRC_ALPHA )
            CASE_PRINT( GPU_BLEND_DST_ALPHA )
            CASE_PRINT( GPU_BLEND_COLOR )
            CASE_PRINT( GPU_COLOR_MASK_R )
            CASE_PRINT( GPU_COLOR_MASK_G )
            CASE_PRINT( GPU_COLOR_MASK_B )
            CASE_PRINT( GPU_COLOR_MASK_A )
            //  GPU Color Logical Operation registers. 
            CASE_PRINT( GPU_LOGICAL_OPERATION )
            CASE_PRINT( GPU_LOGICOP_FUNCTION )
            // Register indicating the start address of the second interleaving. 
            //   Must be set to 0 to use a single (CHANNEL/BANK) memory interleaving 
            CASE_PRINT( GPU_MCV2_2ND_INTERLEAVING_START_ADDR )
            //  Blitter registers.
            CASE_PRINT( GPU_BLIT_INI_X )
            CASE_PRINT( GPU_BLIT_INI_Y )
            CASE_PRINT( GPU_BLIT_X_OFFSET )
            CASE_PRINT( GPU_BLIT_Y_OFFSET )
            CASE_PRINT( GPU_BLIT_WIDTH )
            CASE_PRINT( GPU_BLIT_HEIGHT )
            CASE_PRINT( GPU_BLIT_DST_ADDRESS )
            CASE_PRINT( GPU_BLIT_DST_TX_WIDTH2 )
            CASE_PRINT( GPU_BLIT_DST_TX_FORMAT )
            CASE_PRINT( GPU_BLIT_DST_TX_BLOCK )
            
            //  Last GPU register name mark.
            CASE_PRINT( GPU_LAST_REGISTER )
            
            default:
                os << gpuReg << "(unknown constant)";
        }

        if ( !ignoreU32bit )
            os <<", SubREG: " << subReg << ", Data(u32bit): " << regData.uintVal;
    }
    else if (agpTrans == AGP_EVENT)
    {
        os << ", GPUEvent: ";
        switch( gpuEvent)
        {
            CASE_PRINT(GPU_UNNAMED_EVENT)
            CASE_PRINT(GPU_END_OF_FRAME_EVENT)
            default:
                os << gpuEvent << "(unknown event)";
        }
    }
    else {
        os << ", GPUComm: ";
        switch ( gpuCommand ) {
            CASE_PRINT( GPU_RESET )
            CASE_PRINT( GPU_DRAW )
            CASE_PRINT( GPU_SWAPBUFFERS )
            CASE_PRINT( GPU_DUMPCOLOR )
            CASE_PRINT( GPU_DUMPDEPTH )
            CASE_PRINT( GPU_DUMPSTENCIL )
            CASE_PRINT( GPU_BLIT )
            CASE_PRINT( GPU_CLEARBUFFERS )
            CASE_PRINT( GPU_CLEARZBUFFER )
            CASE_PRINT( GPU_CLEARZSTENCILBUFFER )
            CASE_PRINT( GPU_CLEARCOLORBUFFER )
            CASE_PRINT( GPU_LOAD_VERTEX_PROGRAM )
            CASE_PRINT( GPU_LOAD_FRAGMENT_PROGRAM )
            CASE_PRINT( GPU_LOAD_SHADER_PROGRAM )
            CASE_PRINT( GPU_FLUSHZSTENCIL )
            CASE_PRINT( GPU_FLUSHCOLOR )
            CASE_PRINT( GPU_SAVE_COLOR_STATE )
            CASE_PRINT( GPU_RESTORE_COLOR_STATE )
            CASE_PRINT( GPU_SAVE_ZSTENCIL_STATE )
            CASE_PRINT( GPU_RESTORE_ZSTENCIL_STATE )
            CASE_PRINT( GPU_RESET_COLOR_STATE )
            CASE_PRINT( GPU_RESET_ZSTENCIL_STATE )

            default:
                os << gpuCommand << "(unknown command)";
        }
    }

    os << ", NumPackets: " << numPackets << " )" << endl;

}


AGPTransaction::~AGPTransaction()
{
    if ( agpTrans == AGP_READ || agpTrans == AGP_WRITE || agpTrans == AGP_PRELOAD)
    {
        delete[] data;
    }
}
