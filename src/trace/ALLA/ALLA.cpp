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

#include "GPULibInternals.h"
#include "OptimizedDynamicMemory.h"
#include "ProgramManager.h"
#include "ProgramObject.h"
#include "glext.h"

#include "ALLA.h"

#include "GPUDriver.h"
#include "ConfigLoader.h"
#include "AGPTraceFile.h"
#include "ShaderInstruction.h"

#include "ShaderArchitectureParameters.h"

#include <fstream>
#include <sstream>
#include <vector>

using namespace libgl;

ALLAInterface::ALLAInterface()
{
}

void ALLAInterface::initialize(bool verboseExec)
{
    OptimizedDynamicMemory::initialize(512, 1024, 1024, 1024, 4096, 1024);

    ConfigLoader *cl;

    char *configFile = "bGPU.ini";

    cl = new ConfigLoader(configFile);

    simP = new SimParameters;
    
    cl->getParameters(simP);

    delete cl;

    //  Check if the vector alu configuration is scalar (SOA).
    string aluConf(simP->fsh.vectorALUConfig);    
    bool vectorScalarALU = simP->fsh.useVectorShader && (aluConf.compare("scalar") == 0);

    u32bit instructionsToSchedulePerCycle = 0;
    
    //  Compute the number of shader instructions to schedule per cycle that is passed to the driver.
    if (simP->fsh.useVectorShader)
    {
        //  Only the simd4+scalar architecture requires scheduling more than 1 instruction per cycle.
        if (aluConf.compare("simd4+scalar") == 0)
            instructionsToSchedulePerCycle = 2;
        else
            instructionsToSchedulePerCycle = 1;
    }
    else
    {
        //  Use the 'FetchRate' (instructions fetched per cycle) parameter for the old shader model.
        instructionsToSchedulePerCycle = simP->fsh.fetchRate;
    }
    
    //  Check configuration parameters.
    if (simP->fsh.vAttrLoadFromShader && !simP->enableDriverShTrans)
    {
        panic("ALLAInterface", "initialize", "Vertex attribute load from shader requires driver shader program translation to be enabled.");
    }
    if (simP->fsh.useVectorShader && vectorScalarALU && !simP->enableDriverShTrans)
    {
        panic("ALLAInterface", "initialize", "Vector Shader SOA architecture requires driver shader program translation to be enabled.");
    }

    GPUDriver::getGPUDriver()->setGPUParameters(
                                                simP->mem.memSize * 1024,
                                                simP->mem.mappedMemSize * 1024,
                                                simP->fsh.textBlockDim,
                                                simP->fsh.textSBlockDim,
                                                simP->ras.scanWidth,
                                                simP->ras.scanHeight,
                                                simP->ras.overScanWidth,
                                                simP->ras.overScanHeight,
                                                simP->doubleBuffer,
                                                simP->forceMSAA,
                                                simP->msaaSamples,
                                                simP->forceFP16ColorBuffer,
                                                instructionsToSchedulePerCycle,
                                                simP->mem.memoryControllerV2,
                                                simP->mem.v2SecondInterleaving,
                                                simP->fsh.vAttrLoadFromShader,
                                                vectorScalarALU,
                                                //simP->enableDriverShTrans  // Disabled when creating/processing AGPTrans traces
                                                false,
                                                //(simP.ras.useMicroPolRast && simP.ras.microTrisAsFragments) // Disabled when creating/processing AGPTrans traces
                                                false
                                                );

    if (simP->fsh.fixedLatencyALU)
    {
        if (simP->fsh.useVectorShader && vectorScalarALU)
            ShaderArchitectureParameters::getShaderArchitectureParameters()->selectArchitecture("FixedLatSOA");
        else
            ShaderArchitectureParameters::getShaderArchitectureParameters()->selectArchitecture("FixedLatAOS");
    }
    else
    {
        if (simP->fsh.useVectorShader && vectorScalarALU)
            ShaderArchitectureParameters::getShaderArchitectureParameters()->selectArchitecture("VarLatSOA");
        else
            ShaderArchitectureParameters::getShaderArchitectureParameters()->selectArchitecture("VarLatAOS");
    }

    initOGLLibrary(GPUDriver::getGPUDriver(), simP->ras.shadedSetup);

    verbose = verboseExec;
    
    /*trDriver = new GLTraceDriver(simP->inputFile,
                                simP->useACD, // False => Legacy, True => Uses ACD new implementation
                                GPUDriver::getGPUDriver(),
                                simP->ras.shadedSetup,
                                simP->startFrame);*/
}

void ALLAInterface::createTraceFile(char *traceName)
{

    //  Open output file
    outFile.open(traceName, ios::out | ios::binary);

    //  Check if output file was correctly created.
    if (!outFile.is_open())
    {
        panic("testGen", "main", "Error opening output AGP transaction trace file.");
    }

    AGPTraceFileHeader agpTraceHeader;

    //  Create header for the AGP Trace file.

    //  Write file type stamp
    //agpTraceHeader.signature = AGPTRACEFILE_SIGNATURE;
    for(u32bit i = 0; i < sizeof(AGPTRACEFILE_SIGNATURE); i++)
        agpTraceHeader.signature[i] = AGPTRACEFILE_SIGNATURE[i];

    agpTraceHeader.version = AGPTRACEFILE_CURRENT_VERSION;

    agpTraceHeader.parameters.startFrame = simP->startFrame;
    agpTraceHeader.parameters.traceFrames = simP->simFrames;
    agpTraceHeader.parameters.memSize = simP->mem.memSize;
    agpTraceHeader.parameters.mappedMemSize = simP->mem.mappedMemSize;
    agpTraceHeader.parameters.textBlockDim = simP->fsh.textBlockDim;
    agpTraceHeader.parameters.textSBlockDim = simP->fsh.textSBlockDim;
    agpTraceHeader.parameters.scanWidth = simP->ras.scanWidth;
    agpTraceHeader.parameters.scanHeight = simP->ras.scanHeight;
    agpTraceHeader.parameters.overScanWidth = simP->ras.overScanWidth;
    agpTraceHeader.parameters.overScanHeight = simP->ras.overScanHeight;
    agpTraceHeader.parameters.doubleBuffer = simP->doubleBuffer;
    agpTraceHeader.parameters.fetchRate = simP->fsh.fetchRate;
    agpTraceHeader.parameters.memoryControllerV2 = simP->mem.memoryControllerV2;
    agpTraceHeader.parameters.v2SecondInterleaving = simP->mem.v2SecondInterleaving;

    //  Write header (configuration parameters related with the OpenGL to ATTILA AGP commands translation).
    outFile.write((char *) &agpTraceHeader, sizeof(agpTraceHeader));
}

void ALLAInterface::closeTraceFile()
{
    //  Close output file.
    outFile.close();
}


void ALLAInterface::writeAGPTransaction(AGPTransaction *agpt)
{
    if (verbose)
        agpt->dump();
        
    agpt->save(&outFile);
    delete agpt;
}

void ALLAInterface::writeGPURegister(GPURegister reg, u32bit subReg, u32bit data)
{
    AGPTransaction *agpt;
    GPURegData regData;

    regData.uintVal = data;
    agpt = new AGPTransaction(reg, subReg, regData, 0);
    writeAGPTransaction(agpt);
}

void ALLAInterface::writeGPURegister(GPURegister reg, u32bit subReg, s32bit data)
{
    AGPTransaction *agpt;
    GPURegData regData;

    regData.intVal = data;
    agpt = new AGPTransaction(reg, subReg, regData, 0);
    writeAGPTransaction(agpt);
}

void ALLAInterface::writeGPURegister(GPURegister reg, u32bit subReg, f32bit data)
{
    AGPTransaction *agpt;
    GPURegData regData;

    regData.f32Val = data;
    agpt = new AGPTransaction(reg, subReg, regData, 0);
    writeAGPTransaction(agpt);
}

void ALLAInterface::writeGPURegister(GPURegister reg, u32bit subReg, bool data)
{
    AGPTransaction *agpt;
    GPURegData regData;

    regData.booleanVal = data;
    agpt = new AGPTransaction(reg, subReg, regData, 0);
    writeAGPTransaction(agpt);
}

void ALLAInterface::writeGPURegister(GPURegister reg, u32bit subReg, f32bit *data)
{
    AGPTransaction *agpt;
    GPURegData regData;

    regData.qfVal[0] = data[0];
    regData.qfVal[1] = data[1];
    regData.qfVal[2] = data[2];
    regData.qfVal[3] = data[3];
    agpt = new AGPTransaction(reg, subReg, regData, 0);
    writeAGPTransaction(agpt);
}

void ALLAInterface::writeGPURegister(GPURegister reg, u32bit subReg, TextureFormat data)
{
    AGPTransaction *agpt;
    GPURegData regData;

    regData.txFormat = data;
    agpt = new AGPTransaction(reg, subReg, regData, 0);
    writeAGPTransaction(agpt);
}

void ALLAInterface::writeGPUAddrRegister(GPURegister reg, u32bit subReg, u32bit data, u32bit mdID)
{
    AGPTransaction *agpt;
    GPURegData regData;

    regData.uintVal = data;
    agpt = new AGPTransaction(reg, subReg, regData, mdID);
    writeAGPTransaction(agpt);
}

void ALLAInterface::writeBuffer(u32bit address, u32bit size, u8bit *data, u32bit mdID)
{
    AGPTransaction *agpt;

    agpt = new AGPTransaction(address, size, data, mdID, true, true);
    writeAGPTransaction(agpt);
}

void ALLAInterface::compileARBProgram(u32bit target, char *program, u32bit size, u8bit *code, u32bit &codeSize)
{
    ProgramManager &pm = ProgramManager::instance();

    u32bit objID = (target == GL_VERTEX_PROGRAM_ARB) ? 1 : 2;

    ProgramObject &po = dynamic_cast<ProgramObject&> (pm.bindObject(target, objID));

    po.setSource(program, size);
    po.setFormat(GL_PROGRAM_FORMAT_ASCII_ARB);
    po.compile();

    GLsizei codeSz = codeSize;

    po.getBinary(code, codeSz);

    codeSize = codeSz;

    //char disassem[32*1024];
    //GLsizei disassemsize = 32*1024;
    //po.getASMCode((GLubyte *) disassem, disassemsize);
    //cout << "----------" << endl;
    //cout << "Disassembled code : " << endl;
    //cout << disassem << endl;
}

void ALLAInterface::loadVertexProgram(u32bit address, u32bit pc, u32bit size, u8bit *code, u32bit resources, u32bit mdID)
{
    AGPTransaction *agpt;
    GPURegData regData;

    agpt = new AGPTransaction(address, size, code, mdID, true, true);
    writeAGPTransaction(agpt);

    regData.uintVal = address;
    agpt = new AGPTransaction(GPU_VERTEX_PROGRAM, 0, regData, 1);
    writeAGPTransaction(agpt);

    regData.uintVal = pc;
    agpt = new AGPTransaction(GPU_VERTEX_PROGRAM_PC, 0, regData, 1);
    writeAGPTransaction(agpt);

    regData.uintVal = size;
    agpt = new AGPTransaction(GPU_VERTEX_PROGRAM_SIZE, 0, regData, 1);
    writeAGPTransaction(agpt);

    agpt = new AGPTransaction(GPU_LOAD_VERTEX_PROGRAM);
    writeAGPTransaction(agpt);

    regData.uintVal = resources;
    agpt = new AGPTransaction(GPU_VERTEX_THREAD_RESOURCES, 0, regData, 1);
    writeAGPTransaction(agpt);
}

void ALLAInterface::loadFragmentProgram(u32bit address, u32bit pc, u32bit size, u8bit *code, u32bit resources, u32bit mdID)
{
    AGPTransaction *agpt;
    GPURegData regData;

    agpt = new AGPTransaction(address, size, code, mdID, true, true);
    writeAGPTransaction(agpt);

    regData.uintVal = address;
    agpt = new AGPTransaction(GPU_FRAGMENT_PROGRAM, 0, regData, 1);

    writeAGPTransaction(agpt);

    regData.uintVal = pc;
    agpt = new AGPTransaction(GPU_FRAGMENT_PROGRAM_PC, 0, regData, 1);

    writeAGPTransaction(agpt);

    regData.uintVal = size;
    agpt = new AGPTransaction(GPU_FRAGMENT_PROGRAM_SIZE, 0, regData, 1);

    writeAGPTransaction(agpt);

    agpt = new AGPTransaction(GPU_LOAD_FRAGMENT_PROGRAM);

    writeAGPTransaction(agpt);

    regData.uintVal = resources;
    agpt = new AGPTransaction(GPU_FRAGMENT_THREAD_RESOURCES, 0, regData, 1);

    writeAGPTransaction(agpt);
}

void ALLAInterface::setResolution(u32bit w, u32bit h)
{
    AGPTransaction *agpt;
    GPURegData regData;

    regData.uintVal = w;
    agpt = new AGPTransaction(GPU_DISPLAY_X_RES, 0, regData, 0);

    writeAGPTransaction(agpt);

    regData.uintVal = h;
    agpt = new AGPTransaction(GPU_DISPLAY_Y_RES, 0, regData, 0);

    writeAGPTransaction(agpt);
}

void ALLAInterface::resetVertexOutputAttributes()
{
    AGPTransaction *agpt;
    GPURegData regData;

    for(u32bit i = 0; i < MAX_VERTEX_ATTRIBUTES; i++)
    {
        regData.booleanVal = false;
        agpt = new AGPTransaction(GPU_VERTEX_OUTPUT_ATTRIBUTE, i, regData, 0);
        writeAGPTransaction(agpt);
    }
}

void ALLAInterface::resetFragmentInputAttributes()
{
    AGPTransaction *agpt;
    GPURegData regData;

    for(u32bit i = 0; i < MAX_FRAGMENT_ATTRIBUTES; i++)
    {
        regData.booleanVal = false;
        agpt = new AGPTransaction(GPU_FRAGMENT_INPUT_ATTRIBUTES, i, regData, 0);
        writeAGPTransaction(agpt);
    }

}

void ALLAInterface::resetStreamer()
{
    AGPTransaction *agpt;
    GPURegData regData;

    for(u32bit i = 0; i < MAX_VERTEX_ATTRIBUTES; i++)
    {
        regData.uintVal = ST_INACTIVE_ATTRIBUTE;
        agpt = new AGPTransaction(GPU_VERTEX_ATTRIBUTE_MAP, i, regData, 0);
        writeAGPTransaction(agpt);

        regData.qfVal[0] = 0.0f;
        regData.qfVal[1] = 0.0f;
        regData.qfVal[2] = 0.0f;
        if ((i == COLOR_ATTRIBUTE) || (i == POSITION_ATTRIBUTE))
            regData.qfVal[3] = 1.0f;
        else
            regData.qfVal[3] = 0.0f;
        agpt = new AGPTransaction(GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE, i, regData, 0);
        writeAGPTransaction(agpt);
    }

    for(u32bit i = 0; i < MAX_STREAM_BUFFERS; i++)
    {
        regData.uintVal = 0;
        agpt = new AGPTransaction(GPU_STREAM_ADDRESS, i, regData, 0);
        writeAGPTransaction(agpt);

        regData.uintVal = 0;
        agpt = new AGPTransaction(GPU_STREAM_STRIDE, i, regData, 0);
        writeAGPTransaction(agpt);

        regData.streamData = SD_FLOAT;
        agpt = new AGPTransaction(GPU_STREAM_DATA, i, regData, 0);
        writeAGPTransaction(agpt);

        regData.uintVal = 0;
        agpt = new AGPTransaction(GPU_STREAM_ELEMENTS, i, regData, 0);
        writeAGPTransaction(agpt);

        regData.booleanVal = 0;
        agpt = new AGPTransaction(GPU_D3D9_COLOR_STREAM, i, regData, 0);
        writeAGPTransaction(agpt);
    }

    regData.uintVal = 0;
    agpt = new AGPTransaction(GPU_STREAM_START, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.uintVal = 0;
    agpt = new AGPTransaction(GPU_STREAM_COUNT, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.booleanVal = false;
    agpt = new AGPTransaction(GPU_INDEX_MODE, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.uintVal = 0;
    agpt = new AGPTransaction(GPU_INDEX_STREAM, 0, regData, 0);
    writeAGPTransaction(agpt);
}

void ALLAInterface::setAttributeStream(u32bit stream, u32bit attr, u32bit address, u32bit stride, StreamData format, u32bit elements)
{
    AGPTransaction *agpt;
    GPURegData regData;

    regData.uintVal = stream;
    agpt = new AGPTransaction(GPU_VERTEX_ATTRIBUTE_MAP, attr, regData, 0);
    writeAGPTransaction(agpt);

    regData.uintVal = address;
    agpt = new AGPTransaction(GPU_STREAM_ADDRESS, stream, regData, 0);
    writeAGPTransaction(agpt);

    regData.uintVal = stride;
    agpt = new AGPTransaction(GPU_STREAM_STRIDE, stream, regData, 0);
    writeAGPTransaction(agpt);

    regData.streamData = format;
    agpt = new AGPTransaction(GPU_STREAM_DATA, stream, regData, 0);
    writeAGPTransaction(agpt);

    regData.uintVal = elements;
    agpt = new AGPTransaction(GPU_STREAM_ELEMENTS, stream, regData, 0);
    writeAGPTransaction(agpt);
}

void ALLAInterface::setViewport(s32bit x, s32bit y, u32bit w, u32bit h)
{
    AGPTransaction *agpt;
    GPURegData regData;

    regData.intVal = x;
    agpt = new AGPTransaction(GPU_VIEWPORT_INI_X, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.intVal = y;
    agpt = new AGPTransaction(GPU_VIEWPORT_INI_Y, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.uintVal = w;
    agpt = new AGPTransaction(GPU_VIEWPORT_WIDTH, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.uintVal = h;
    agpt = new AGPTransaction(GPU_VIEWPORT_HEIGHT, 0, regData, 0);
    writeAGPTransaction(agpt);
}

void ALLAInterface::setScissor(s32bit x, s32bit y, u32bit w, u32bit h)
{
    AGPTransaction *agpt;
    GPURegData regData;

    writeGPURegister(GPU_PROGRAM_MEM_ADDR, 0, 0);
    writeGPURegister(GPU_TEXTURE_MEM_ADDR, 0, 0);

    regData.intVal = x;
    agpt = new AGPTransaction(GPU_SCISSOR_INI_X, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.intVal = y;
    agpt = new AGPTransaction(GPU_SCISSOR_INI_Y, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.uintVal = w;
    agpt = new AGPTransaction(GPU_SCISSOR_WIDTH, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.uintVal = h;
    agpt = new AGPTransaction(GPU_SCISSOR_HEIGHT, 0, regData, 0);
    writeAGPTransaction(agpt);
}

void ALLAInterface::miscReset()
{
    AGPTransaction *agpt;
    GPURegData regData;

    regData.booleanVal = true;
    agpt = new AGPTransaction(GPU_FRUSTUM_CLIPPING, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.faceMode = GPU_CW;
    agpt = new AGPTransaction(GPU_FACEMODE, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.culling = NONE;
    agpt = new AGPTransaction(GPU_CULLING, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.booleanVal = false;
    agpt = new AGPTransaction(GPU_HIERARCHICALZ, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.booleanVal = true;
    agpt = new AGPTransaction(GPU_EARLYZ, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.booleanVal = false;
    agpt = new AGPTransaction(GPU_SCISSOR_TEST, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.f32Val = 0.0f;
    agpt = new AGPTransaction(GPU_DEPTH_RANGE_NEAR, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.f32Val = 0.0f;
    agpt = new AGPTransaction(GPU_DEPTH_RANGE_FAR, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.booleanVal = false;
    agpt = new AGPTransaction(GPU_TWOSIDED_LIGHTING, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.booleanVal = false;
    agpt = new AGPTransaction(GPU_MULTISAMPLING, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.booleanVal = false;
    agpt = new AGPTransaction(GPU_MODIFY_FRAGMENT_DEPTH, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.booleanVal = false;
    agpt = new AGPTransaction(GPU_DEPTH_TEST, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.booleanVal = false;
    agpt = new AGPTransaction(GPU_STENCIL_TEST, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.booleanVal = true;
    agpt = new AGPTransaction(GPU_COLOR_MASK_R, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.booleanVal = true;
    agpt = new AGPTransaction(GPU_COLOR_MASK_G, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.booleanVal = true;
    agpt = new AGPTransaction(GPU_COLOR_MASK_B, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.booleanVal = true;
    agpt = new AGPTransaction(GPU_COLOR_MASK_A, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.booleanVal = false;
    agpt = new AGPTransaction(GPU_COLOR_BLEND, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.booleanVal = false;
    agpt = new AGPTransaction(GPU_LOGICAL_OPERATION, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.uintVal = 0;
    agpt = new AGPTransaction(GPU_MCV2_2ND_INTERLEAVING_START_ADDR, 0, regData, 0);
    writeAGPTransaction(agpt);
}

void ALLAInterface::resetTextures()
{
    AGPTransaction *agpt;
    GPURegData regData;

    for(u32bit i = 0; i < MAX_TEXTURES; i++)
    {
        regData.booleanVal = false;
        agpt = new AGPTransaction(GPU_TEXTURE_ENABLE, i, regData, 0);
        writeAGPTransaction(agpt);
    }
}

void ALLAInterface::enableTexture(u32bit textureID)
{
    GPURegData regData;
    AGPTransaction *agpt;

    regData.booleanVal = true;
    agpt = new AGPTransaction(GPU_TEXTURE_ENABLE, textureID, regData, 0);
    writeAGPTransaction(agpt);
}

void ALLAInterface::disableTexture(u32bit textureID)
{
    GPURegData regData;
    AGPTransaction *agpt;

    regData.booleanVal = false;
    agpt = new AGPTransaction(GPU_TEXTURE_ENABLE, textureID, regData, 0);
    writeAGPTransaction(agpt);
}

void ALLAInterface::setTexture(u32bit texID, TextureMode texMode, u32bit address, u32bit width, u32bit height,
                u32bit widthlog2, u32bit heightlog2, TextureFormat texFormat, TextureBlocking texBlockMode,
                ClampMode texClampS, ClampMode texClampT, FilterMode texMinFilter, FilterMode texMagFilter,
                bool invertV)

{
    GPURegData regData;
    AGPTransaction *agpt;

    regData.txMode = texMode;
    agpt = new AGPTransaction(GPU_TEXTURE_MODE, texID, regData, 0);
    writeAGPTransaction(agpt);

    regData.uintVal = address;
    agpt = new AGPTransaction(GPU_TEXTURE_ADDRESS, texID, regData, 0);
    writeAGPTransaction(agpt);

    regData.uintVal = width;
    agpt = new AGPTransaction(GPU_TEXTURE_WIDTH, texID, regData, 0);
    writeAGPTransaction(agpt);

    regData.uintVal = height;
    agpt = new AGPTransaction(GPU_TEXTURE_HEIGHT, texID, regData, 0);
    writeAGPTransaction(agpt);

    regData.uintVal = widthlog2;
    agpt = new AGPTransaction(GPU_TEXTURE_WIDTH2, texID, regData, 0);
    writeAGPTransaction(agpt);

    regData.uintVal = heightlog2;
    agpt = new AGPTransaction(GPU_TEXTURE_HEIGHT2, texID, regData, 0);
    writeAGPTransaction(agpt);

    regData.txFormat = texFormat;
    agpt = new AGPTransaction(GPU_TEXTURE_FORMAT, texID, regData, 0);
    writeAGPTransaction(agpt);

    regData.txBlocking = texBlockMode;
    agpt = new AGPTransaction(GPU_TEXTURE_BLOCKING, texID, regData, 0);
    writeAGPTransaction(agpt);

    regData.txClamp = texClampS;
    agpt = new AGPTransaction(GPU_TEXTURE_WRAP_S, texID, regData, 0);
    writeAGPTransaction(agpt);

    regData.txClamp = texClampT;
    agpt = new AGPTransaction(GPU_TEXTURE_WRAP_T, texID, regData, 0);
    writeAGPTransaction(agpt);

    regData.txFilter = texMinFilter;
    agpt = new AGPTransaction(GPU_TEXTURE_MIN_FILTER, texID, regData, 0);
    writeAGPTransaction(agpt);

    regData.txFilter = texMagFilter;
    agpt = new AGPTransaction(GPU_TEXTURE_MAG_FILTER, texID, regData, 0);
    writeAGPTransaction(agpt);

    regData.booleanVal = invertV;
    agpt = new AGPTransaction(GPU_TEXTURE_D3D9_V_INV, texID, regData, 0);
    writeAGPTransaction(agpt);
}

void ALLAInterface::draw(PrimitiveMode primitive, u32bit start, u32bit count)
{
    AGPTransaction *agpt;
    GPURegData regData;

    regData.primitive = primitive;
    agpt = new AGPTransaction(GPU_PRIMITIVE, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.uintVal = start;
    agpt = new AGPTransaction(GPU_STREAM_START, 0, regData, 0);
    writeAGPTransaction(agpt);

    regData.uintVal = count;
    agpt = new AGPTransaction(GPU_STREAM_COUNT, 0, regData, 0);
    writeAGPTransaction(agpt);

    agpt = new AGPTransaction(GPU_DRAW);
    writeAGPTransaction(agpt);
}

void ALLAInterface::swap()
{
    AGPTransaction *agpt;
    agpt = new AGPTransaction(GPU_SWAPBUFFERS);
    writeAGPTransaction(agpt);
}

void ALLAInterface::reset()
{
    AGPTransaction *agpt;
    agpt = new AGPTransaction(GPU_RESET);
    writeAGPTransaction(agpt);
}

void ALLAInterface::clearColorBuffer()
{
    AGPTransaction *agpt;
    agpt = new AGPTransaction(GPU_CLEARCOLORBUFFER);
    writeAGPTransaction(agpt);
}

void ALLAInterface::clearZStencilBuffer()
{
    AGPTransaction *agpt;
    agpt = new AGPTransaction(GPU_CLEARZSTENCILBUFFER);
    writeAGPTransaction(agpt);
}

void ALLAInterface::flushColorBuffer()
{
    AGPTransaction *agpt;
    agpt = new AGPTransaction(GPU_FLUSHCOLOR);
    writeAGPTransaction(agpt);
}

void ALLAInterface::flushZStencilBuffer()
{
    AGPTransaction *agpt;
    agpt = new AGPTransaction(GPU_FLUSHZSTENCIL);
    writeAGPTransaction(agpt);
}

void ALLAInterface::saveColorState()
{
    AGPTransaction *agpt;
    agpt = new AGPTransaction(GPU_SAVE_COLOR_STATE);
    writeAGPTransaction(agpt);
}

void ALLAInterface::restoreColorState()
{
    AGPTransaction *agpt;
    agpt = new AGPTransaction(GPU_RESTORE_COLOR_STATE);
    writeAGPTransaction(agpt);
}

void ALLAInterface::saveZStencilState()
{
    AGPTransaction *agpt;
    agpt = new AGPTransaction(GPU_SAVE_ZSTENCIL_STATE);
    writeAGPTransaction(agpt);
}

void ALLAInterface::restoreZStencilState()
{
    AGPTransaction *agpt;
    agpt = new AGPTransaction(GPU_RESTORE_ZSTENCIL_STATE);
    writeAGPTransaction(agpt);
}

void ALLAInterface::resetColorState()
{
    AGPTransaction *agpt;
    agpt = new AGPTransaction(GPU_RESET_COLOR_STATE);
    writeAGPTransaction(agpt);
}

void ALLAInterface::resetZStencilState()
{
    AGPTransaction *agpt;
    agpt = new AGPTransaction(GPU_RESET_ZSTENCIL_STATE);
    writeAGPTransaction(agpt);
}

bool ALLAInterface::assembleProgramFromText(char *programString, u8bit *code, u32bit &codeSize, string &errorString)
{
    istringstream inputStream;
    
    inputStream.clear();
    inputStream.str(string(programString));

    vector<ShaderInstruction*> program;

    program.clear();

    u32bit line = 1;

    bool error = false;

    //  Read until the end of the input file or until there is an error assembling the instruction.
    while(!error && !inputStream.eof())
    {
        ShaderInstruction *shInstr;

        string currentLine;
        string errorString;

        //  Read a line from the input file.
        getline(inputStream, currentLine);

        if ((currentLine.length() == 0) && inputStream.eof())
        {
            //  End loop.
        }
        else
        {
            //  Assemble the line.
            shInstr = ShaderInstruction::assemble((char *) currentLine.c_str(), errorString);

            //  Check if the instruction was assembled.
            if (shInstr == NULL)
            {
                char buff[256];
                sprintf(buff, "Line %d.  ERROR : %s\n", line, errorString.c_str());
                errorString = buff;
                error = true;
            }
            else
            {
                //  Add instruction to the program.
                program.push_back(shInstr);
            }

            //  Update line number.
            line++;
        }
    }

    //  Check if an error occurred.
    if (!error)
    {
        //  Check if there is space in the buffer for the whole program.
        if (codeSize <= (program.size() * 16))
        {
            errorString = "ERROR: program too large for provided buffer";            
            codeSize = u32bit(program.size()) * 16;
            return false;
        }
        
        //  Set final code size.
        codeSize = u32bit(program.size()) * 16;
                
        //  Encode the instructions and write to the output file.
        for(u32bit instr = 0; instr < program.size(); instr++)
        {
            //  Check if this is the last instruction.  If so mark the instruction with the end flag.
            if (instr == (program.size() - 1))
                program[instr]->setEndFlag(true);

            //  Encode instruction.
            program[instr]->getCode(&code[instr * 16]);                       
        }        
    }        
        
    return !error;
}

bool ALLAInterface::assembleProgramFromFile(char *filename, u8bit *code, u32bit &codeSize, string &errorString)
{
    string inputFileName = filename;

    ifstream inputFile;

    inputFile.open(inputFileName.c_str());

    if (!inputFile.is_open())
    {        
        errorString = "ERROR: could not open input file";
        return false;
    }

    vector<ShaderInstruction*> program;

    program.clear();

    u32bit line = 1;

    bool error = false;

    //  Read until the end of the input file or until there is an error assembling the instruction.
    while(!error && !inputFile.eof())
    {
        ShaderInstruction *shInstr;

        string currentLine;
        string errorString;

        //  Read a line from the input file.
        getline(inputFile, currentLine);

        if ((currentLine.length() == 0) && inputFile.eof())
        {
            //  End loop.
        }
        else
        {
            //  Assemble the line.
            shInstr = ShaderInstruction::assemble((char *) currentLine.c_str(), errorString);

            //  Check if the instruction was assembled.
            if (shInstr == NULL)
            {
                char buff[256];
                sprintf(buff, "Line %d.  ERROR : %s\n", line, errorString.c_str());
                errorString = string(buff);
                error = true;
            }
            else
            {
                //  Add instruction to the program.
                program.push_back(shInstr);
            }

            //  Update line number.
            line++;
        }
    }

    //  Check if an error occurred.
    if (!error)
    {
        //  Check if there is space in the buffer for the whole program.
        if (codeSize <= (program.size() * 16))
        {
            errorString = "ERROR: program too large for provided buffer";            
            codeSize = u32bit(program.size()) * 16;
            inputFile.close();
            return false;
        }
        
        //  Set final code size.
        codeSize = u32bit(program.size()) * 16;
        
        //  Encode the instructions and write to the output file.
        for(u32bit instr = 0; instr < program.size(); instr++)
        {
            //  Check if this is the last instruction.  If so mark the instruction with the end flag.
            if (instr == (program.size() - 1))
                program[instr]->setEndFlag(true);

            //  Encode instruction.
            program[instr]->getCode(&code[instr * 16]);                       
        }        
    }
     
    inputFile.close();
            
    return !error;
}

