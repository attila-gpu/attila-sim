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


#include "ALLA.h"

#include <iostream>
#include <string.h>

using namespace std;

char testVertexProgram[] =
"!!ARBvp1.0"
""
"MOV result.position, vertex.position;"
"MOV result.color, vertex.attrib[1];"
"END";

char testVertexProgram2[] =
"!!ARBvp1.0"
""
"MOV result.position, vertex.position;"
"MOV result.texcoord[0], vertex.attrib[8];"
"END";

char testFragmentProgram[] =
"!!ARBfp1.0"
""
"MOV result.color, fragment.color;"
"END";

char testFragmentProgram2[] =
"!!ARBfp1.0"
""
"TEX result.color, fragment.texcoord[0], texture[0], 2D;"
"END";


int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        printf("Usage : \n");
        printf("  TestCreator <output file>\n");
        return -1;        
    }
    
    u32bit nextBufferID = 0;

    ALLAInterface attilaLowLevelAPI;

    attilaLowLevelAPI.initialize();

    attilaLowLevelAPI.createTraceFile(argv[1]);

    attilaLowLevelAPI.miscReset();
    attilaLowLevelAPI.resetVertexOutputAttributes();
    attilaLowLevelAPI.resetFragmentInputAttributes();
    attilaLowLevelAPI.resetStreamer();
    attilaLowLevelAPI.resetTextures();

    attilaLowLevelAPI.setResolution(100, 100);

    u8bit vpCode[8192];
    u32bit vpCodeSize = 8192;
    attilaLowLevelAPI.compileARBProgram(GL_VERTEX_PROGRAM_ARB, testVertexProgram, strlen(testVertexProgram), vpCode, vpCodeSize);

    u8bit fpCode[8192];
    u32bit fpCodeSize = 8192;
    attilaLowLevelAPI.compileARBProgram(GL_FRAGMENT_PROGRAM_ARB, testFragmentProgram, strlen(testFragmentProgram), fpCode, fpCodeSize);

    attilaLowLevelAPI.loadVertexProgram(0x00000000, 0, vpCodeSize, vpCode, 1, nextBufferID++);
    attilaLowLevelAPI.loadFragmentProgram(0x00002000, 0, fpCodeSize, fpCode, 1, nextBufferID++);

    attilaLowLevelAPI.writeGPURegister(GPU_COLOR_BUFFER_FORMAT, 0, GPU_RGBA8888);
    attilaLowLevelAPI.writeGPURegister(GPU_FRONTBUFFER_ADDR, 0, 0x00400000U);
    attilaLowLevelAPI.writeGPURegister(GPU_BACKBUFFER_ADDR, 0, 0x00800000U);
    attilaLowLevelAPI.writeGPURegister(GPU_ZSTENCILBUFFER_ADDR, 0, 0x00C00000U);

    attilaLowLevelAPI.writeGPURegister(GPU_VERTEX_OUTPUT_ATTRIBUTE, POSITION_ATTRIBUTE, true);
    attilaLowLevelAPI.writeGPURegister(GPU_VERTEX_OUTPUT_ATTRIBUTE, COLOR_ATTRIBUTE, true);

    attilaLowLevelAPI.writeGPURegister(GPU_FRAGMENT_INPUT_ATTRIBUTES, POSITION_ATTRIBUTE, true);
    attilaLowLevelAPI.writeGPURegister(GPU_FRAGMENT_INPUT_ATTRIBUTES, COLOR_ATTRIBUTE, true);

    f32bit vertexBuffer[] =
    {
        -1.0f, -1.0f, 0.1f,
        -1.0f,  1.0f, 0.1f,
         1.0f,  1.0f, 0.1f,

         1.0f,  1.0f, 0.1f,
         1.0f, -1.0f, 0.1f,
        -1.0f, -1.0f, 0.1f,

        -0.5f, -0.5f, 0.1f,
         0.5f, -0.5f, 0.1f,
         0.0f,  0.5f, 0.1f
    };

    attilaLowLevelAPI.writeBuffer(0x00004000, sizeof(vertexBuffer), (u8bit *) vertexBuffer, nextBufferID++);

    f32bit colorBuffer[] =
    {
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,

        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
    };

    attilaLowLevelAPI.writeBuffer(0x00006000, sizeof(colorBuffer), (u8bit *) colorBuffer, nextBufferID++);

    attilaLowLevelAPI.setAttributeStream(0, POSITION_ATTRIBUTE, 0x00004000, 12, SD_FLOAT, 3);
    attilaLowLevelAPI.setAttributeStream(1,    COLOR_ATTRIBUTE, 0x00006000, 12, SD_FLOAT, 3);

    attilaLowLevelAPI.setViewport(0, 0, 100, 100);

    attilaLowLevelAPI.writeGPURegister(GPU_INTERPOLATION, COLOR_ATTRIBUTE, true);

    attilaLowLevelAPI.clearColorBuffer();

    attilaLowLevelAPI.writeGPURegister(GPU_COLOR_COMPRESSION, 0, false);

    attilaLowLevelAPI.draw(TRIANGLE, 0, 9);

    attilaLowLevelAPI.swap();

    //  End of first frame.

    attilaLowLevelAPI.setResolution(512, 512);

    vpCodeSize = 8192;
    attilaLowLevelAPI.compileARBProgram(GL_VERTEX_PROGRAM_ARB, testVertexProgram2, strlen(testVertexProgram2), vpCode, vpCodeSize);

    fpCodeSize = 8192;
    attilaLowLevelAPI.compileARBProgram(GL_FRAGMENT_PROGRAM_ARB, testFragmentProgram2, strlen(testFragmentProgram2), fpCode, fpCodeSize);

    attilaLowLevelAPI.loadVertexProgram(0x00008000, 0, vpCodeSize, vpCode, 1, nextBufferID++);
    attilaLowLevelAPI.loadFragmentProgram(0x0000A000, 0, fpCodeSize, fpCode, 1, nextBufferID++);

    attilaLowLevelAPI.setViewport(0, 0, 512, 512);

    f32bit texBuffer[] =
    {
        0.0f, 0.0f,
        0.0f, 4.0f,
        4.0f, 4.0f,

        4.0f, 4.0f,
        4.0f, 0.0f,
        0.0f, 0.0f
    };

    attilaLowLevelAPI.writeBuffer(0x0000C000, sizeof(texBuffer), (u8bit *) texBuffer, nextBufferID++);

    attilaLowLevelAPI.resetStreamer();

    attilaLowLevelAPI.setAttributeStream(0, POSITION_ATTRIBUTE, 0x00004000, 12, SD_FLOAT, 3);
    attilaLowLevelAPI.setAttributeStream(1,                  8, 0x0000C000,  8, SD_FLOAT, 2);

    attilaLowLevelAPI.writeGPURegister(GPU_INTERPOLATION, COLOR_ATTRIBUTE, false);
    attilaLowLevelAPI.writeGPURegister(GPU_INTERPOLATION, 6, true);
    attilaLowLevelAPI.writeGPURegister(GPU_FRAGMENT_INPUT_ATTRIBUTES, COLOR_ATTRIBUTE, false);
    attilaLowLevelAPI.writeGPURegister(GPU_FRAGMENT_INPUT_ATTRIBUTES, 6, true);

    attilaLowLevelAPI.setTexture(0, GPU_TEXTURE2D,  0x00800000, 100, 100, 7, 7, GPU_RGBA8888, GPU_TXBLOCK_FRAMEBUFFER,
               GPU_TEXT_REPEAT, GPU_TEXT_REPEAT, GPU_LINEAR, GPU_LINEAR, true);

    attilaLowLevelAPI.enableTexture(0);

    attilaLowLevelAPI.writeGPURegister(GPU_COLOR_COMPRESSION, 0, true);

    attilaLowLevelAPI.draw(TRIANGLE, 0, 6);

    attilaLowLevelAPI.swap();

    attilaLowLevelAPI.writeGPUAddrRegister(GPU_COLOR_STATE_BUFFER_MEM_ADDR, 0, 0x01000000, 0);

    attilaLowLevelAPI.saveColorState();

    f32bit clearColor[] = {0.0f, 1.0f, 0.0f, 0.0f};

    attilaLowLevelAPI.writeGPURegister(GPU_COLOR_BUFFER_CLEAR, 0, clearColor);

    attilaLowLevelAPI.clearColorBuffer();

    attilaLowLevelAPI.swap();

    attilaLowLevelAPI.restoreColorState();

    attilaLowLevelAPI.swap();
}


