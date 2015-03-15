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
 */

#include "RegisterWriteBuffer.h"
#include "GPUDriver.h"
#include <iostream>

using namespace std;
using namespace gpu3d;

RegisterWriteBuffer::RegisterWriteBuffer(GPUDriver* driver, WritePolicy wp) : driver(driver), writePolicy(wp),
    registerWritesCount(0)
{}

void RegisterWriteBuffer::writeRegister(GPURegister reg, u32bit index,
                                        const GPURegData& data, u32bit md)
{
    RegisterIdentifier auxri(reg, index);
    WriteBufferIt aux = registerStatus.find(auxri);

    bool registerChanged;
    
    //  Check if the register has been defined in the register status table.
    if (aux != registerStatus.end())
    {
        //  Check if the register value has changed.
        registerChanged = (aux->second.data != data) || (aux->second.md != md);
        
        //  Update the register value in the register status table.
        aux->second.data = data;
        aux->second.md = md;
    }
    else
        panic("RegisterWriteBuffer", "writeRegister", "GPU Register not registered.");

    //  Check if register writes can be buffered.
    if (writePolicy == WaitUntilFlush)
    {
        //  Check if the value has changed before adding the update in the write buffer.
        if (registerChanged)
        {
            RegisterIdentifier ri(reg, index);
            WriteBufferIt it = writeBuffer.find(ri);
            
            //  Check if there is a previous update for the register buffered in the
            //  register write buffer.
            if (it != writeBuffer.end())
            {
                //  Overwrite previous update with the current update.
                it->second.data = data;
                it->second.md = md;
            }
            else
            {
                //  Add register update to the write buffer.
                RegisterData rd(data, md);                
                writeBuffer.insert(make_pair(ri, rd));
            }
            
            registerWritesCount++;
        }
    }
    else // Inmediate forwarding
        driver->_sendAGPTransaction(new AGPTransaction(reg, index, data, md));
}

void RegisterWriteBuffer::unbufferedWriteRegister(GPURegister reg, u32bit index, const GPURegData& data, u32bit md)
{
    RegisterIdentifier auxri(reg, index);
    WriteBufferIt aux = registerStatus.find(auxri);

    if (aux != registerStatus.end())
    {
        aux->second.data = data;
        aux->second.md = md;
    }
    else
        panic("RegisterWriteBuffer", "writeRegister", "GPU Register not registered.");

    driver->_sendAGPTransaction(new AGPTransaction(reg, index, data, md));
}

void RegisterWriteBuffer::readRegister(gpu3d::GPURegister reg, u32bit index, gpu3d::GPURegData &data)
{
    RegisterIdentifier auxri(reg, index);
    WriteBufferIt aux = registerStatus.find(auxri);
    
    if (aux != registerStatus.end())
    {
        data = aux->second.data;
        //md = aux->second.md;
    }
    else
        panic("RegisterWriteBuffer", "readRegister", "GPU Register not registered.");
}

void RegisterWriteBuffer::flush()
{
    //cout << "RegisterWriteBuffer::flush() -> Doing pendent register writes: " << writeBuffer.size() << " (saved "
    //<< registerWritesCount - writeBuffer.size() << " reg. writes)" << endl;

    registerWritesCount = 0; // reset counter

    /*  GPU_INDEX_STREAM must be sent before the configuration for the stream.
        But the storing order in the map is the inverse.  */

    RegisterIdentifier ri(GPU_INDEX_STREAM, 0);
    WriteBufferIt it = writeBuffer.find(ri);

    /*  Check if GPU_INDEX_STREAM was found.  */
    if ( it != writeBuffer.end() ) // register found, update contents
    {
        /*  Send the register update.  */
        driver->_sendAGPTransaction(new AGPTransaction(GPU_INDEX_STREAM, 0, it->second.data, 0));

        /*  Remove the register update from the write buffer.  */
        writeBuffer.erase(it);
    }

    // Update ATTILA registers
    for ( WriteBufferIt it = writeBuffer.begin(); it != writeBuffer.end(); it++ )
    {
        GPURegister reg = it->first.reg;
        u32bit index = it->first.index;
        driver->_sendAGPTransaction(new AGPTransaction(reg, index, it->second.data, it->second.md));
    }
    writeBuffer.clear(); // reset cache contents
}

void RegisterWriteBuffer::setWritePolicy(WritePolicy wp)
{
    flush(); // flush previous pendent register writes
    writePolicy = wp;
}

RegisterWriteBuffer::WritePolicy RegisterWriteBuffer::getWritePolicy() const
{
    return writePolicy;
}

void RegisterWriteBuffer::dumpRegisterStatus (int frame, int batch)
{

    ofstream out;
    char fileName[128];
    sprintf(fileName, "GPUDriverDump_frame%d_batch%d.txt", frame, batch);
    out.open(fileName);

    if ( !out.is_open() )
        panic("RegisterWriteBuffer", "dumpRegisterStatus", "Dump failed (output file could not be opened)");

    out << "///////////////////////////////////////////////////////////////////////////" << endl;
    out << "///////////                dumpRegisterState             //////////////////" << endl;
    out << "///////////////////////////////////////////////////////////////////////////" << endl;
    out << endl;

    for ( WriteBufferIt it = registerStatus.begin(); it != registerStatus.end(); it++ ) // register found, update contents
        dumpRegisterInfo(out, it->first.reg, it->first.index, it->second.data, it->second.md);

       // out << "Register: " << getRegisterName(it->first.reg) << "(Index: " << it->first.index << ", Data: " << printRegData(out, it->second.data) << ", MD:" << it->second.md << ")" << endl;
}

void RegisterWriteBuffer::initAllRegisterStatus ()
{

    GPURegData data;
    data.uintVal = 0;

    initRegisterStatus( GPU_STATUS, 0, data, 0); 
    initRegisterStatus( GPU_MEMORY, 0, data, 0); 
    initRegisterStatus( GPU_VERTEX_PROGRAM, 0, data, 0);
    initRegisterStatus( GPU_VERTEX_PROGRAM_PC, 0, data, 0);
    initRegisterStatus( GPU_VERTEX_PROGRAM_SIZE, 0, data, 0);
    initRegisterStatus( GPU_VERTEX_THREAD_RESOURCES, 0, data, 0);

    data.qfVal[0] = 0.0f;
    data.qfVal[1] = 0.0f;
    data.qfVal[2] = 0.0f;
    data.qfVal[3] = 0.0f;    
    for (int i = 0; i < MAX_VERTEX_CONSTANTS; i++)  //MAX_VERTEX_CONSTANTS -- Quad
        initRegisterStatus( GPU_VERTEX_CONSTANT, i, data, 0);

    for (int i = 0; i < MAX_VERTEX_ATTRIBUTES; i++)  //MAX_VERTEX_ATTRIBUTES -- Quad
    {
        data.uintVal = 0;
        initRegisterStatus( GPU_VERTEX_OUTPUT_ATTRIBUTE, i, data, 0);

        data.uintVal = 255;
        initRegisterStatus( GPU_VERTEX_ATTRIBUTE_MAP, i, data, 0); // ST_INACTIVE_ATTRIBUTE

        data.qfVal[0] = 0;
        data.qfVal[1] = 0;
        data.qfVal[2] = 0;
        data.qfVal[3] = 1;
        initRegisterStatus( GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE, i, data, 0); // 0 0 0 1
    }

    for (int i = 0; i < MAX_STREAM_BUFFERS; i++) //MAX_STREAM_BUFFERS
    {
        data.uintVal = 0;
        for (int j = 0; j < 100; j++)
            initRegisterStatus( GPU_STREAM_ADDRESS, i, data, j); //0

        initRegisterStatus( GPU_STREAM_STRIDE, i, data, 0); //0

        data.streamData = SD_U32BIT;
        initRegisterStatus( GPU_STREAM_DATA, i, data, 0); //SD_U32BIT

        data.uintVal = 0;
        initRegisterStatus( GPU_STREAM_ELEMENTS, i, data, 0); //0
        initRegisterStatus(GPU_STREAM_FREQUENCY, i, data, 0);  // 0
        
        data.booleanVal = false;
        initRegisterStatus( GPU_D3D9_COLOR_STREAM, i, data, 0); //false
    }

    data.uintVal = 0;
    initRegisterStatus( GPU_STREAM_START, 0, data, 0); //0
    initRegisterStatus( GPU_STREAM_COUNT, 0, data, 0); //0

    data.uintVal = 1;
    initRegisterStatus(GPU_STREAM_INSTANCES, 0, data, 0);  // 1

    data.booleanVal = true;
    initRegisterStatus( GPU_INDEX_MODE, 0, data, 0); // true

    data.uintVal = 0;
    initRegisterStatus( GPU_INDEX_STREAM, 0, data, 0); //0

    data.booleanVal = false;
    initRegisterStatus( GPU_ATTRIBUTE_LOAD_BYPASS, 0, data, 0); //false

    data.uintVal = 0;
    initRegisterStatus( GPU_PRIMITIVE, 0, data, 0);

    data.booleanVal = true;
    initRegisterStatus( GPU_FRUSTUM_CLIPPING, 0, data, 0); //true

    for (int i = 0; i < MAX_USER_CLIP_PLANES; i ++) //MAX_USER_CLIP_PLANES -- Quad
    {
        data.qfVal[0] = 0;
        data.qfVal[1] = 0;
        data.qfVal[2] = 0;
        data.qfVal[3] = 0;
        initRegisterStatus( GPU_USER_CLIP, i, data, 0); // 0 0 0 0
    }

    data.booleanVal = false;
    initRegisterStatus( GPU_USER_CLIP_PLANE, 0, data, 0); //false

    //data.uintVal = 0;
    //initRegisterStatus( GPU_LAST_GEOMETRY_REGISTER, 0, data, 0);

    data.f32Val = 0x200000;
    initRegisterStatus( GPU_FRONTBUFFER_ADDR, 0, data, 0); //0x200000
    data.f32Val = 0x400000;
    initRegisterStatus( GPU_BACKBUFFER_ADDR, 0, data, 0); //0x400000

    data.f32Val = 0;
    initRegisterStatus( GPU_ZSTENCILBUFFER_ADDR, 0, data, 0);
    initRegisterStatus( GPU_TEXTURE_MEM_ADDR, 0, data, 0);
    initRegisterStatus( GPU_PROGRAM_MEM_ADDR, 0, data, 0);

    data.faceMode = GPU_CCW;
    initRegisterStatus( GPU_FACEMODE, 0, data, 0); //GPU_CCW

    data.culling = NONE;
    initRegisterStatus( GPU_CULLING, 0, data, 0); //NONE

    data.booleanVal = false;
    initRegisterStatus( GPU_HIERARCHICALZ, 0, data, 0); //TRUE?

    data.booleanVal = true;
    initRegisterStatus( GPU_EARLYZ, 0, data, 0); //TRUE ??

    data.uintVal = 400;
    initRegisterStatus( GPU_DISPLAY_X_RES, 0, data, 0); //400
    initRegisterStatus( GPU_DISPLAY_Y_RES, 0, data, 0); //400

    data.booleanVal = false;
    initRegisterStatus( GPU_D3D9_PIXEL_COORDINATES, 0, data, 0); //false
    
    data.uintVal = 0;
    initRegisterStatus( GPU_VIEWPORT_INI_X, 0, data, 0); //0
    initRegisterStatus( GPU_VIEWPORT_INI_Y, 0, data, 0); //0

    data.uintVal = 400;
    initRegisterStatus( GPU_VIEWPORT_WIDTH, 0, data, 0); //400
    initRegisterStatus( GPU_VIEWPORT_HEIGHT, 0, data, 0); //400

    data.booleanVal = false;
    initRegisterStatus( GPU_SCISSOR_TEST, 0, data, 0); //false

    data.uintVal = 0;
    initRegisterStatus( GPU_SCISSOR_INI_X, 0, data, 0); //0
    initRegisterStatus( GPU_SCISSOR_INI_Y, 0, data, 0); //0

    data.uintVal = 400;
    initRegisterStatus( GPU_SCISSOR_WIDTH, 0, data, 0); //400
    initRegisterStatus( GPU_SCISSOR_HEIGHT, 0, data, 0); //400

    data.f32Val = 0.0f;
    initRegisterStatus( GPU_DEPTH_RANGE_NEAR, 0, data, 0); //0.0f

    data.f32Val = 1.0f;
    initRegisterStatus( GPU_DEPTH_RANGE_FAR, 0, data, 0); //1.0f

    data.f32Val = 0.0;
    initRegisterStatus( GPU_DEPTH_SLOPE_FACTOR, 0, data, 0); //0.0f
    initRegisterStatus( GPU_DEPTH_UNIT_OFFSET, 0, data, 0); //0.0f

    data.uintVal = 24;
    initRegisterStatus( GPU_Z_BUFFER_BIT_PRECISSION, 0, data, 0); //24

    data.booleanVal = false;
    initRegisterStatus(GPU_D3D9_DEPTH_RANGE, 0, data, 0);  // false
    
    data.booleanVal = false;
    initRegisterStatus(GPU_D3D9_RASTERIZATION_RULES, 0, data, 0);  // false
    
    data.booleanVal = false;
    initRegisterStatus( GPU_TWOSIDED_LIGHTING, 0, data, 0); // false

    data.booleanVal = false;
    initRegisterStatus( GPU_MULTISAMPLING, 0, data, 0); //false

    data.uintVal = 2;
    initRegisterStatus( GPU_MSAA_SAMPLES, 0, data, 0); //2

    for (int i = 0; i < MAX_FRAGMENT_ATTRIBUTES; i++) //MAX_FRAGMENT_ATTRIBUTES
    {
        data.booleanVal = true;
        initRegisterStatus( GPU_INTERPOLATION, i, data, 0); //true
        data.booleanVal = false;
        initRegisterStatus( GPU_FRAGMENT_INPUT_ATTRIBUTES, i, data, 0); //false
    }

    data.uintVal = 0;
    initRegisterStatus( GPU_FRAGMENT_PROGRAM, 0, data, 0);
    initRegisterStatus( GPU_FRAGMENT_PROGRAM_PC, 0, data, 0);
    initRegisterStatus( GPU_FRAGMENT_PROGRAM_SIZE, 0, data, 0);

    data.uintVal = 1;
    initRegisterStatus( GPU_FRAGMENT_THREAD_RESOURCES, 0, data, 0); // 1

    data.qfVal[0] = 0;
    data.qfVal[1] = 0;
    data.qfVal[2] = 0;
    data.qfVal[3] = 0;
    for (int i = 0; i < MAX_FRAGMENT_CONSTANTS; i++) //MAX_FRAGMENT_CONSTANTS -- Quad
    {
        initRegisterStatus( GPU_FRAGMENT_CONSTANT, i, data, 0);
    }

    data.uintVal = 0;
    initRegisterStatus( GPU_MODIFY_FRAGMENT_DEPTH, 0, data, 0);

    data.uintVal = 0;
    for (int t = 0; t < (MAX_TEXTURES * MAX_TEXTURE_SIZE * CUBEMAP_IMAGES); t++)
        initRegisterStatus( GPU_TEXTURE_ADDRESS, t, data, 0); //Special

    for (int i = 0; i < MAX_TEXTURES; i++) //MAX_TEXTURES
    {


        data.booleanVal = false;
        initRegisterStatus( GPU_TEXTURE_ENABLE, i, data, 0); //false

        data.txMode = GPU_TEXTURE2D;
        initRegisterStatus( GPU_TEXTURE_MODE, i, data, 0); //GPU_TEXTURE2D

        data.uintVal = 0;
        initRegisterStatus( GPU_TEXTURE_WIDTH, i, data, 0); //0
        initRegisterStatus( GPU_TEXTURE_HEIGHT, i, data, 0); //0
        initRegisterStatus( GPU_TEXTURE_DEPTH, i, data, 0); //0
        initRegisterStatus( GPU_TEXTURE_WIDTH2, i, data, 0); //0
        initRegisterStatus( GPU_TEXTURE_HEIGHT2, i, data, 0); //0
        initRegisterStatus( GPU_TEXTURE_DEPTH2, i, data, 0); //0
        initRegisterStatus( GPU_TEXTURE_BORDER, i, data, 0); //0

        data.txFormat = GPU_RGBA8888;
        initRegisterStatus( GPU_TEXTURE_FORMAT, i, data, 0); //GPU_RGBA8888

        data.booleanVal = false;
        initRegisterStatus( GPU_TEXTURE_REVERSE, i, data, 0); //false
        initRegisterStatus( GPU_TEXTURE_D3D9_COLOR_CONV, i, data, 0); //false
        initRegisterStatus( GPU_TEXTURE_D3D9_V_INV, i, data, 0); //false

        data.txCompression = GPU_NO_TEXTURE_COMPRESSION;
        initRegisterStatus( GPU_TEXTURE_COMPRESSION, i, data, 0); //GPU_NO_TEXTURE_COMPRESSION

        data.txBlocking = GPU_TXBLOCK_TEXTURE;
        initRegisterStatus( GPU_TEXTURE_BLOCKING, i, data, 0); //GPU_TXBLOCK_TEXTURE

        data.qfVal[0] = 0;
        data.qfVal[1] = 0;
        data.qfVal[2] = 0;
        data.qfVal[3] = 1;
        initRegisterStatus( GPU_TEXTURE_BORDER_COLOR, i, data, 0); // QUADD  0 0 0 1
        
        data.txClamp = GPU_TEXT_CLAMP;
        initRegisterStatus( GPU_TEXTURE_WRAP_S, i, data, 0); // GPU_TEXT_CLAMP
        initRegisterStatus( GPU_TEXTURE_WRAP_T, i, data, 0); // GPU_TEXT_CLAMP
        initRegisterStatus( GPU_TEXTURE_WRAP_R, i, data, 0); // GPU_TEXT_CLAMP

        data.booleanVal = false;
        initRegisterStatus( GPU_TEXTURE_NON_NORMALIZED, i, data, 0);  // FALSE
        
        data.txFilter = GPU_NEAREST;
        initRegisterStatus( GPU_TEXTURE_MIN_FILTER, i, data, 0); //GPU_NEAREST
        initRegisterStatus( GPU_TEXTURE_MAG_FILTER, i, data, 0); //GPU_NEAREST


        data.booleanVal = false;
        initRegisterStatus( GPU_TEXTURE_ENABLE_COMPARISON, i, data, 0);  // FALSE
        
        data.compare = GPU_LEQUAL;
        initRegisterStatus( GPU_TEXTURE_COMPARISON_FUNCTION, i, data, 0);  // GPU_LEQUAL
        
        data.booleanVal = false;
        initRegisterStatus( GPU_TEXTURE_SRGB, i, data, 0);  // FALSE

        data.f32Val = 0.0f;
        initRegisterStatus( GPU_TEXTURE_MIN_LOD, i, data, 0); //0.0f;

        data.f32Val = 12.0f;
        initRegisterStatus( GPU_TEXTURE_MAX_LOD, i, data, 0); //12.0f

        data.f32Val = 0.0f;
        initRegisterStatus( GPU_TEXTURE_LOD_BIAS, i, data, 0); //0.0f;

        data.uintVal = 0;
        initRegisterStatus( GPU_TEXTURE_MIN_LEVEL, i, data, 0); //0

        data.uintVal = 12;
        initRegisterStatus( GPU_TEXTURE_MAX_LEVEL, i, data, 0); //12

        data.f32Val = 0.0f;
        initRegisterStatus( GPU_TEXT_UNIT_LOD_BIAS, i, data, 0); //0.0f

        data.uintVal = 1;
        initRegisterStatus( GPU_TEXTURE_MAX_ANISOTROPY, i, data, 0); //1
    }

    data.f32Val = 0x00ffffff;
    initRegisterStatus( GPU_Z_BUFFER_CLEAR, 0, data, 0); //0x00ffffff
    initRegisterStatus( GPU_STENCIL_BUFFER_CLEAR, 0, data, 0); //0x00ffffff;

    data.uintVal = 0;
    initRegisterStatus( GPU_ZSTENCIL_STATE_BUFFER_MEM_ADDR, 0, data, 0);

    data.booleanVal = false;
    initRegisterStatus( GPU_STENCIL_TEST, 0, data, 0); //false

    data.compare = GPU_ALWAYS;
    initRegisterStatus( GPU_STENCIL_FUNCTION, 0, data, 0); //GPU_ALWAYS

    data.f32Val = 0x00;
    initRegisterStatus( GPU_STENCIL_REFERENCE, 0, data, 0); //0x00

    data.f32Val = 0xff;
    initRegisterStatus( GPU_STENCIL_COMPARE_MASK, 0, data, 0); //0xff
    initRegisterStatus( GPU_STENCIL_UPDATE_MASK, 0, data, 0); //0xff

    data.stencilUpdate = STENCIL_KEEP;
    initRegisterStatus( GPU_STENCIL_FAIL_UPDATE, 0, data, 0); //STENCIL_KEEP
    initRegisterStatus( GPU_DEPTH_FAIL_UPDATE, 0, data, 0); //STENCIL_KEEP
    initRegisterStatus( GPU_DEPTH_PASS_UPDATE, 0, data, 0); //STENCIL_KEEP

    data.booleanVal = false;
    initRegisterStatus( GPU_DEPTH_TEST, 0, data, 0); //false

    data.compare = GPU_LESS;
    initRegisterStatus( GPU_DEPTH_FUNCTION, 0, data, 0); //GPU_LESS

    data.booleanVal = true;
    initRegisterStatus( GPU_DEPTH_MASK, 0, data, 0); // true
    initRegisterStatus( GPU_ZSTENCIL_COMPRESSION, 0, data, 0); //true

    data.txFormat = GPU_RGBA8888;
    initRegisterStatus( GPU_COLOR_BUFFER_FORMAT, 0, data, 0); // GPU_RGBA8888

    data.booleanVal = true;
    initRegisterStatus( GPU_COLOR_COMPRESSION, 0, data, 0); //true

    data.booleanVal = false;
    initRegisterStatus( GPU_COLOR_SRGB_WRITE, 0, data, 0); //false

    for (int i = 0; i < MAX_RENDER_TARGETS; i++) //MAX_RENDER_TARGETS
    {
        data.booleanVal = false;
        initRegisterStatus( GPU_RENDER_TARGET_ENABLE, i, data, 0); //false

        data.txFormat = GPU_R32F;
        initRegisterStatus( GPU_RENDER_TARGET_FORMAT, i, data, 0); //GPU_R32F
        
        data.f32Val = 0x600000 + 0x100000 * i;
        initRegisterStatus( GPU_RENDER_TARGET_ADDRESS, i, data, 0); //0x600000 + 0x100000 * i

        data.booleanVal = false;
        initRegisterStatus( GPU_COLOR_BLEND, i, data, 0);

        data.blendEquation = BLEND_FUNC_ADD;
        initRegisterStatus( GPU_BLEND_EQUATION, i, data, 0); //BLEND_FUNC_ADD

        data.blendFunction = BLEND_ONE;
        initRegisterStatus( GPU_BLEND_SRC_RGB, i, data, 0); //BLEND_ONE

        data.blendFunction = BLEND_ZERO;
        initRegisterStatus( GPU_BLEND_DST_RGB, i, data, 0); //BLEND_ZERO

        data.blendFunction = BLEND_ONE;
        initRegisterStatus( GPU_BLEND_SRC_ALPHA, i, data, 0); //BLEND_ONE

        data.blendFunction = BLEND_ZERO;
        initRegisterStatus( GPU_BLEND_DST_ALPHA, i, data, 0); //BLEND_ZERO

        data.qfVal[0] = 0;
        data.qfVal[1] = 0;
        data.qfVal[2] = 0;
        data.qfVal[3] = 0;
        initRegisterStatus( GPU_BLEND_COLOR, i, data, 0); // QUAD

        data.booleanVal = true;
        initRegisterStatus( GPU_COLOR_MASK_R, i, data, 0); //true
        initRegisterStatus( GPU_COLOR_MASK_G, i, data, 0); //true
        initRegisterStatus( GPU_COLOR_MASK_B, i, data, 0); //true
        initRegisterStatus( GPU_COLOR_MASK_A, i, data, 0); //true
    }

    data.qfVal[0] = 0;
    data.qfVal[1] = 0;
    data.qfVal[2] = 0;
    data.qfVal[3] = 1;
    initRegisterStatus( GPU_COLOR_BUFFER_CLEAR, 0, data, 0); // QUAD 0 0 0 1

    data.uintVal = 0;
    initRegisterStatus( GPU_COLOR_STATE_BUFFER_MEM_ADDR, 0, data, 0);

    data.booleanVal = false;
    initRegisterStatus( GPU_LOGICAL_OPERATION, 0, data, 0); //false

    data.logicOp = LOGICOP_COPY;
    initRegisterStatus( GPU_LOGICOP_FUNCTION, 0, data, 0); //LOGICOP_COPY

    data.uintVal = 0;
    initRegisterStatus( GPU_MCV2_2ND_INTERLEAVING_START_ADDR, 0, data, 0);
    initRegisterStatus( GPU_BLIT_INI_X, 0, data, 0); //0
    initRegisterStatus( GPU_BLIT_INI_Y, 0, data, 0); //0
    initRegisterStatus( GPU_BLIT_X_OFFSET, 0, data, 0); //0
    initRegisterStatus( GPU_BLIT_Y_OFFSET, 0, data, 0); //0

    data.uintVal = 400;
    initRegisterStatus( GPU_BLIT_WIDTH, 0, data, 0); //400
    initRegisterStatus( GPU_BLIT_HEIGHT, 0, data, 0); //400

    data.f32Val = 0x8000000;
    initRegisterStatus( GPU_BLIT_DST_ADDRESS, 0, data, 0); //0x8000000

    data.uintVal = 9;
    initRegisterStatus( GPU_BLIT_DST_TX_WIDTH2, 0, data, 0); //9; // 2^9 = 512 (> 400)

    data.txFormat = GPU_RGBA8888;
    initRegisterStatus( GPU_BLIT_DST_TX_FORMAT, 0, data, 0); //GPU_RGBA8888

    data.txBlocking = GPU_TXBLOCK_TEXTURE;
    initRegisterStatus( GPU_BLIT_DST_TX_BLOCK, 0, data, 0); //GPU_TXBLOCK_TEXTURE

    //data.uintVal = 0;
    //initRegisterStatus( GPU_LAST_REGISTER, 0, data, 0);

    
}

void RegisterWriteBuffer::initRegisterStatus (GPURegister reg, u32bit index,
                                        const GPURegData& data, u32bit md)
{

    RegisterIdentifier ri(reg, index);
    RegisterData rd(data, md);

    registerStatus.insert(make_pair(ri, rd));
}

void RegisterWriteBuffer::dumpRegisterInfo(ofstream& out, GPURegister reg, u32bit index,const GPURegData& data, u32bit md)
{
    out << "Register: ";
    switch (reg)
    {
        case GPU_STATUS: out << "GPU_STATUS" << "(Index: " <<index<<", Data: "<<data.status<<", MD:"<< md; break; 
        case GPU_MEMORY: out << "GPU_MEMORY" << "(Index: " <<index<<", Data: "<<data.status<<", MD:"<< md; break; 
        case GPU_VERTEX_PROGRAM: out << "GPU_VERTEX_PROGRAM" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break; 
        case GPU_VERTEX_PROGRAM_PC: out << "GPU_VERTEX_PROGRAM_PC" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break; 
        case GPU_VERTEX_PROGRAM_SIZE: out << "GPU_VERTEX_PROGRAM_SIZE" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_VERTEX_THREAD_RESOURCES: out << "GPU_VERTEX_THREAD_RESOURCES" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break; 
        case GPU_VERTEX_CONSTANT: out << "GPU_VERTEX_CONSTANT" << "(Index: " <<index<<", Data: "<<data.qfVal[0] <<" "<<data.qfVal[1] <<" "<<data.qfVal[2] <<" "<<data.qfVal[3] <<", MD:"<< md; break; 
        case GPU_VERTEX_OUTPUT_ATTRIBUTE: out << "GPU_VERTEX_OUTPUT_ATTRIBUTE" << "(Index: " <<index<<", Data: "<<data.booleanVal <<", MD:"<< md; break; 
        case GPU_VERTEX_ATTRIBUTE_MAP: out << "GPU_VERTEX_ATTRIBUTE_MAP" << "(Index: " <<index<<", Data: "<<data.uintVal <<", MD:"<< md; break; 
        case GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE: out << "GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE" << "(Index: " <<index<<", Data: "<<data.qfVal[0] <<" "<<data.qfVal[1] <<" "<<data.qfVal[2] <<" "<<data.qfVal[3] <<", MD:"<< md; break; 
        case GPU_STREAM_ADDRESS: out << "GPU_STREAM_ADDRESS" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break; 
        case GPU_STREAM_STRIDE: out << "GPU_STREAM_STRIDE" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_STREAM_DATA: out << "GPU_STREAM_DATA" << "(Index: " <<index<<", Data: "<<data.streamData<<", MD:"<< md; break;
        case GPU_STREAM_ELEMENTS: out << "GPU_STREAM_ELEMENTS" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break; 
        case GPU_STREAM_START: out << "GPU_STREAM_START" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break; 
        case GPU_STREAM_COUNT: out << "GPU_STREAM_COUNT" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_INDEX_MODE: out << "GPU_INDEX_MODE" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break; 
        case GPU_INDEX_STREAM: out << "GPU_INDEX_STREAM" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break; 
        case GPU_D3D9_COLOR_STREAM: out << "GPU_D3D9_COLOR_STREAM" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break; 
        case GPU_ATTRIBUTE_LOAD_BYPASS: out << "GPU_ATTRIBUTE_LOAD_BYPASS" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break;     
        case GPU_PRIMITIVE: out << "GPU_PRIMITIVE" << "(Index: " <<index<<", Data: "<<data.primitive<<", MD:"<< md; break;
        case GPU_FRUSTUM_CLIPPING: out << "GPU_FRUSTUM_CLIPPING" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break;
        case GPU_USER_CLIP: out << "GPU_USER_CLIP" << "(Index: " <<index<<", Data: "<<data.qfVal[0] <<" "<<data.qfVal[1] <<" "<<data.qfVal[2] <<" "<<data.qfVal[3] <<", MD:"<< md; break; 
        case GPU_USER_CLIP_PLANE: out << "GPU_USER_CLIP_PLANE" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break; 
        case GPU_LAST_GEOMETRY_REGISTER: out << "GPU_LAST_GEOMETRY_REGISTER" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_FRONTBUFFER_ADDR: out << "GPU_FRONTBUFFER_ADDR" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break; 
        case GPU_BACKBUFFER_ADDR: out << "GPU_BACKBUFFER_ADDR" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_ZSTENCILBUFFER_ADDR: out << "GPU_ZSTENCILBUFFER_ADDR" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break; 
        case GPU_PROGRAM_MEM_ADDR: out << "GPU_PROGRAM_MEM_ADDR" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break; 
        case GPU_FACEMODE: out << "GPU_FACEMODE" << "(Index: " <<index<<", Data: "<<data.faceMode<<", MD:"<< md; break; 
        case GPU_CULLING: out << "GPU_CULLING" << "(Index: " <<index<<", Data: "<<data.culling<<", MD:"<< md; break; 
        case GPU_HIERARCHICALZ: out << "GPU_HIERARCHICALZ" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break;
        case GPU_EARLYZ: out << "GPU_EARLYZ" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break; 
        case GPU_DISPLAY_X_RES: out << "GPU_DISPLAY_X_RES" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_DISPLAY_Y_RES: out << "GPU_DISPLAY_Y_RES" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_D3D9_PIXEL_COORDINATES: out << "GPU_D3D9_PIXEL_COORDINATES" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break;         
        case GPU_VIEWPORT_INI_X: out << "GPU_VIEWPORT_INI_X" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break; 
        case GPU_VIEWPORT_INI_Y: out << "GPU_VIEWPORT_INI_Y" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break; 
        case GPU_VIEWPORT_WIDTH: out << "GPU_VIEWPORT_WIDTH" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break; 
        case GPU_VIEWPORT_HEIGHT: out << "GPU_VIEWPORT_HEIGHT" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break; 
        case GPU_SCISSOR_TEST: out << "GPU_SCISSOR_TEST" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break; 
        case GPU_SCISSOR_INI_X: out << "GPU_SCISSOR_INI_X" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break; 
        case GPU_SCISSOR_INI_Y: out << "GPU_SCISSOR_INI_Y" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break; 
        case GPU_SCISSOR_WIDTH: out << "GPU_SCISSOR_WIDTH" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_SCISSOR_HEIGHT: out << "GPU_SCISSOR_HEIGHT" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break; 
        case GPU_DEPTH_RANGE_NEAR: out << "GPU_DEPTH_RANGE_NEAR" << "(Index: " <<index<<", Data: "<<data.f32Val<<", MD:"<< md; break; 
        case GPU_DEPTH_RANGE_FAR: out << "GPU_DEPTH_RANGE_FAR" << "(Index: " <<index<<", Data: "<<data.f32Val<<", MD:"<< md; break; 
        case GPU_DEPTH_SLOPE_FACTOR: out << "GPU_DEPTH_SLOPE_FACTOR" << "(Index: " <<index<<", Data: "<<data.f32Val<<", MD:"<< md; break; 
        case GPU_DEPTH_UNIT_OFFSET: out << "GPU_DEPTH_UNIT_OFFSET" << "(Index: " <<index<<", Data: "<<data.f32Val<<", MD:"<< md; break; 
        case GPU_Z_BUFFER_BIT_PRECISSION: out << "GPU_Z_BUFFER_BIT_PRECISSION" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break; 
        case GPU_D3D9_DEPTH_RANGE: out << "GPU_D3D9_DEPTH_RANGE" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break; 
        case GPU_D3D9_RASTERIZATION_RULES: out << "GPU_D3D9_RASTERIZATION_RULES" << "(Index: " << index << ", Data: " << data.booleanVal << ", MD: " << md; break;
        case GPU_TWOSIDED_LIGHTING: out << "GPU_TWOSIDED_LIGHTING" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break; 
        case GPU_MULTISAMPLING: out << "GPU_MULTISAMPLING" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break;
        case GPU_MSAA_SAMPLES: out << "GPU_MSAA_SAMPLES" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_INTERPOLATION: out << "GPU_INTERPOLATION" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break;
        case GPU_FRAGMENT_INPUT_ATTRIBUTES: out << "GPU_FRAGMENT_INPUT_ATTRIBUTES" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break;
        case GPU_FRAGMENT_PROGRAM: out << "GPU_FRAGMENT_PROGRAM" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_FRAGMENT_PROGRAM_PC: out << "GPU_FRAGMENT_PROGRAM_PC" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_FRAGMENT_PROGRAM_SIZE: out << "GPU_FRAGMENT_PROGRAM_SIZE" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_FRAGMENT_THREAD_RESOURCES: out << "GPU_FRAGMENT_THREAD_RESOURCES" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_FRAGMENT_CONSTANT: out << "GPU_FRAGMENT_CONSTANT" << "(Index: " <<index<<", Data: "<<data.qfVal[0] <<" "<<data.qfVal[1] <<" "<<data.qfVal[2] <<" "<<data.qfVal[3] <<", MD:"<< md; break; 
        case GPU_MODIFY_FRAGMENT_DEPTH: out << "GPU_MODIFY_FRAGMENT_DEPTH" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break;
        case GPU_SHADER_PROGRAM_ADDRESS: out << "GPU_SHADER_PROGRAM_ADDRESS" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_SHADER_PROGRAM_SIZE: out << "GPU_SHADER_PROGRAM_SIZE" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_SHADER_PROGRAM_LOAD_PC: out << "GPU_SHADER_PROGRAM_LOAD_PC" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_SHADER_PROGRAM_PC: out << "GPU_SHADER_PROGRAM_PC" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_SHADER_THREAD_RESOURCES: out << "GPU_SHADER_THREAD_RESOURCES" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_TEXTURE_ENABLE: out << "GPU_TEXTURE_ENABLE" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break;
        case GPU_TEXTURE_ADDRESS: out << "GPU_TEXTURE_ADDRESS" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break; 
        case GPU_TEXTURE_WIDTH: out << "GPU_TEXTURE_WIDTH" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_TEXTURE_HEIGHT: out << "GPU_TEXTURE_HEIGHT" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_TEXTURE_DEPTH: out << "GPU_TEXTURE_DEPTH" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_TEXTURE_WIDTH2: out << "GPU_TEXTURE_WIDTH2" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_TEXTURE_HEIGHT2: out << "GPU_TEXTURE_HEIGHT2" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_TEXTURE_DEPTH2: out << "GPU_TEXTURE_DEPTH2" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_TEXTURE_BORDER: out << "GPU_TEXTURE_BORDER" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break; 
        case GPU_TEXTURE_FORMAT: out << "GPU_TEXTURE_FORMAT" << "(Index: " <<index<<", Data: "<<data.txFormat<<", MD:"<< md; break;
        case GPU_TEXTURE_REVERSE: out << "GPU_TEXTURE_REVERSE" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break;
        case GPU_TEXTURE_D3D9_COLOR_CONV: out << "GPU_TEXTURE_D3D9_COLOR_CONV" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break;
        case GPU_TEXTURE_D3D9_V_INV: out << "GPU_TEXTURE_D3D9_V_INV" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break;
        case GPU_TEXTURE_COMPRESSION: out << "GPU_TEXTURE_COMPRESSION" << "(Index: " <<index<<", Data: "<<data.txCompression<<", MD:"<< md; break;
        case GPU_TEXTURE_BLOCKING: out << "GPU_TEXTURE_BLOCKING" << "(Index: " <<index<<", Data: "<<data.txBlocking<<", MD:"<< md; break;
        case GPU_TEXTURE_BORDER_COLOR: out << "GPU_TEXTURE_BORDER_COLOR" << "(Index: " <<index<<", Data: "<<data.qfVal[0] <<" "<<data.qfVal[1] <<" "<<data.qfVal[2] <<" "<<data.qfVal[3] <<", MD:"<< md; break; 
        case GPU_TEXTURE_WRAP_S: out << "GPU_TEXTURE_WRAP_S" << "(Index: " <<index<<", Data: "<<data.txClamp<<", MD:"<< md; break;
        case GPU_TEXTURE_WRAP_T: out << "GPU_TEXTURE_WRAP_T" << "(Index: " <<index<<", Data: "<<data.txClamp<<", MD:"<< md; break; 
        case GPU_TEXTURE_WRAP_R: out << "GPU_TEXTURE_WRAP_R" << "(Index: " <<index<<", Data: "<<data.txClamp<<", MD:"<< md; break; 
        case GPU_TEXTURE_NON_NORMALIZED: out << "GPU_TEXTURE_NON_NORMALIZED" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break; 
        case GPU_TEXTURE_MIN_FILTER: out << "GPU_TEXTURE_MIN_FILTER" << "(Index: " <<index<<", Data: "<<data.txFilter<<", MD:"<< md; break; 
        case GPU_TEXTURE_MAG_FILTER: out << "GPU_TEXTURE_MAG_FILTER" << "(Index: " <<index<<", Data: "<<data.txFilter<<", MD:"<< md; break; 
        case GPU_TEXTURE_ENABLE_COMPARISON: out << "GPU_TEXTURE_ENABLE_COMPARISON" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break; 
        case GPU_TEXTURE_COMPARISON_FUNCTION: out << "GPU_TEXTURE_COMPARISON_FUNCTION" << "(Index: " <<index<<", Data: "<<data.compare<<", MD:"<< md; break; 
        case GPU_TEXTURE_MIN_LOD: out << "GPU_TEXTURE_MIN_LOD" << "(Index: " <<index<<", Data: "<<data.f32Val<<", MD:"<< md; break;
        case GPU_TEXTURE_MAX_LOD: out << "GPU_TEXTURE_MAX_LOD" << "(Index: " <<index<<", Data: "<<data.f32Val<<", MD:"<< md; break;
        case GPU_TEXTURE_LOD_BIAS: out << "GPU_TEXTURE_LOD_BIAS" << "(Index: " <<index<<", Data: "<<data.f32Val<<", MD:"<< md; break; 
        case GPU_TEXTURE_MIN_LEVEL: out << "GPU_TEXTURE_MIN_LEVEL" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_TEXTURE_MAX_LEVEL: out << "GPU_TEXTURE_MAX_LEVEL" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_TEXT_UNIT_LOD_BIAS: out << "GPU_TEXT_UNIT_LOD_BIAS" << "(Index: " <<index<<", Data: "<<data.f32Val<<", MD:"<< md; break;
        case GPU_TEXTURE_MAX_ANISOTROPY: out << "GPU_TEXTURE_MAX_ANISOTROPY" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_Z_BUFFER_CLEAR: out << "GPU_Z_BUFFER_CLEAR" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_STENCIL_BUFFER_CLEAR: out << "GPU_STENCIL_BUFFER_CLEAR" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_ZSTENCIL_STATE_BUFFER_MEM_ADDR: out << "GPU_ZSTENCIL_STATE_BUFFER_MEM_ADDR" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_STENCIL_TEST: out << "GPU_STENCIL_TEST" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break; 
        case GPU_STENCIL_FUNCTION: out << "GPU_STENCIL_FUNCTION" << "(Index: " <<index<<", Data: "<<data.compare<<", MD:"<< md; break;
        case GPU_STENCIL_REFERENCE: out << "GPU_STENCIL_REFERENCE" << "(Index: " <<index<<", Data: "<<u8bit(data.uintVal)<<", MD:"<< md; break;
        case GPU_STENCIL_COMPARE_MASK: out << "GPU_STENCIL_COMPARE_MASK" << "(Index: " <<index<<", Data: "<<u8bit(data.uintVal)<<", MD:"<< md; break;
        case GPU_STENCIL_UPDATE_MASK: out << "GPU_STENCIL_UPDATE_MASK" << "(Index: " <<index<<", Data: "<<u8bit(data.uintVal)<<", MD:"<< md; break; 
        case GPU_STENCIL_FAIL_UPDATE: out << "GPU_STENCIL_FAIL_UPDATE" << "(Index: " <<index<<", Data: "<<data.stencilUpdate<<", MD:"<< md; break;
        case GPU_DEPTH_FAIL_UPDATE: out << "GPU_DEPTH_FAIL_UPDATE" << "(Index: " <<index<<", Data: "<<data.stencilUpdate<<", MD:"<< md; break;
        case GPU_DEPTH_PASS_UPDATE: out << "GPU_DEPTH_PASS_UPDATE" << "(Index: " <<index<<", Data: "<<data.stencilUpdate<<", MD:"<< md; break; 
        case GPU_DEPTH_TEST: out << "GPU_DEPTH_TEST" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break;
        case GPU_DEPTH_FUNCTION: out << "GPU_DEPTH_FUNCTION" << "(Index: " <<index<<", Data: "<<data.compare<<", MD:"<< md; break;
        case GPU_DEPTH_MASK: out << "GPU_DEPTH_MASK" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break; 
        case GPU_ZSTENCIL_COMPRESSION: out << "GPU_ZSTENCIL_COMPRESSION" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break;
        case GPU_COLOR_BUFFER_FORMAT: out << "GPU_COLOR_BUFFER_FORMAT" << "(Index: " <<index<<", Data: "<<data.txFormat<<", MD:"<< md; break; 
        case GPU_COLOR_COMPRESSION: out << "GPU_COLOR_COMPRESSION" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break;
        case GPU_COLOR_SRGB_WRITE: out << "GPU_COLOR_SRGB_WRITE" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break;
        case GPU_RENDER_TARGET_ENABLE: out << "GPU_RENDER_TARGET_ENABLE" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break;
        case GPU_RENDER_TARGET_FORMAT: out << "GPU_RENDER_TARGET_FORMAT" << "(Index: " <<index<<", Data: "<<data.txFormat<<", MD:"<< md; break;
        case GPU_RENDER_TARGET_ADDRESS: out << "GPU_RENDER_TARGET_ADDRESS" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_COLOR_BUFFER_CLEAR: out << "GPU_COLOR_BUFFER_CLEAR" << "(Index: " <<index<<", Data: "<<data.qfVal[0] <<" "<<data.qfVal[1] <<" "<<data.qfVal[2] <<" "<<data.qfVal[3] <<", MD:"<< md; break;
        case GPU_COLOR_BLEND: out << "GPU_COLOR_BLEND" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break;
        case GPU_BLEND_EQUATION: out << "GPU_BLEND_EQUATION" << "(Index: " <<index<<", Data: "<<data.blendEquation<<", MD:"<< md; break;
        case GPU_BLEND_SRC_RGB: out << "GPU_BLEND_SRC_RGB" << "(Index: " <<index<<", Data: "<<data.blendFunction<<", MD:"<< md; break;
        case GPU_BLEND_DST_RGB: out << "GPU_BLEND_DST_RGB" << "(Index: " <<index<<", Data: "<<data.blendFunction<<", MD:"<< md; break;
        case GPU_BLEND_SRC_ALPHA: out << "GPU_BLEND_SRC_ALPHA" << "(Index: " <<index<<", Data: "<<data.blendFunction<<", MD:"<< md; break;
        case GPU_BLEND_DST_ALPHA: out << "GPU_BLEND_DST_ALPHA" << "(Index: " <<index<<", Data: "<<data.blendFunction<<", MD:"<< md; break;
        case GPU_BLEND_COLOR: out << "GPU_BLEND_COLOR" << "(Index: " <<index<<", Data: "<<data.qfVal[0] <<" "<<data.qfVal[1] <<" "<<data.qfVal[2] <<" "<<data.qfVal[3] <<", MD:"<< md; break;
        case GPU_COLOR_MASK_R: out << "GPU_COLOR_MASK_R" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break; 
        case GPU_COLOR_MASK_G: out << "GPU_COLOR_MASK_G" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break;
        case GPU_COLOR_MASK_B: out << "GPU_COLOR_MASK_B" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break; 
        case GPU_COLOR_MASK_A: out << "GPU_COLOR_MASK_A" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break;
        case GPU_LOGICAL_OPERATION: out << "GPU_LOGICAL_OPERATION" << "(Index: " <<index<<", Data: "<<data.booleanVal<<", MD:"<< md; break;
        case GPU_LOGICOP_FUNCTION: out << "GPU_LOGICOP_FUNCTION" << "(Index: " <<index<<", Data: "<<data.logicOp<<", MD:"<< md; break;
        case GPU_MCV2_2ND_INTERLEAVING_START_ADDR: out << "GPU_MCV2_2ND_INTERLEAVING_START_ADDR" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_BLIT_INI_X: out << "GPU_BLIT_INI_X" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break; 
        case GPU_BLIT_INI_Y: out << "GPU_BLIT_INI_Y" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_BLIT_X_OFFSET: out << "GPU_BLIT_X_OFFSET" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_BLIT_Y_OFFSET: out << "GPU_BLIT_Y_OFFSET" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_BLIT_WIDTH: out << "GPU_BLIT_WIDTH" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_BLIT_HEIGHT: out << "GPU_BLIT_HEIGHT" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_BLIT_DST_ADDRESS: out << "GPU_BLIT_DST_ADDRESS" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        case GPU_BLIT_DST_TX_WIDTH2: out << "GPU_BLIT_DST_TX_WIDTH2" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break; 
        case GPU_BLIT_DST_TX_FORMAT: out << "GPU_BLIT_DST_TX_FORMAT" << "(Index: " <<index<<", Data: "<<data.txFormat<<", MD:"<< md; break;
        case GPU_BLIT_DST_TX_BLOCK: out << "GPU_BLIT_DST_TX_BLOCK" << "(Index: " <<index<<", Data: "<<data.txBlocking<<", MD:"<< md; break;
        case GPU_LAST_REGISTER: out << "GPU_LAST_REGISTER" << "(Index: " <<index<<", Data: "<<data.uintVal<<", MD:"<< md; break;
        default: out << "undefined register : " << reg; break;
            
    }

    out << ")" << endl;
}

