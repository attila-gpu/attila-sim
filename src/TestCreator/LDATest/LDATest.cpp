
#include "ALLA.h"

#include <iostream>
#include <string.h>

using namespace std;

int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        printf("Usage : \n");
        printf("  LDATest <output file>\n");
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
    attilaLowLevelAPI.setViewport(0, 0, 100, 100);

    string errorString;
    bool result;
    
    u8bit vpCode[8192];
    u32bit vpCodeSize = 8192;
    
    result = attilaLowLevelAPI.assembleProgramFromFile("vprogram.asm", vpCode, vpCodeSize, errorString);
    
    if (!result)
    {
        printf("Error assembling vertex program.\n");
        printf("%s\n", errorString.c_str());
        return -1;
    }
    
    u8bit fpCode[8192];
    u32bit fpCodeSize = 8192;

    result = attilaLowLevelAPI.assembleProgramFromFile("fprogram.asm", fpCode, fpCodeSize, errorString);
    
    if (!result)
    {
        printf("Error assembling fragment program.\n");
        printf("%s\n", errorString.c_str());
        return -1;
    }

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

    f32bit indexBuffer[16];
    for(u32bit i = 0; i < 16; i++)
        ((u32bit *) indexBuffer)[i] = i;
        
    attilaLowLevelAPI.writeBuffer(0x00008000, sizeof(indexBuffer), (u8bit *) indexBuffer, nextBufferID++);            
    
    attilaLowLevelAPI.setAttributeStream(0, POSITION_ATTRIBUTE, 0x00004000, 12, SD_FLOAT, 3);
    attilaLowLevelAPI.setAttributeStream(1,    COLOR_ATTRIBUTE, 0x00006000, 12, SD_FLOAT, 3);
    attilaLowLevelAPI.setAttributeStream(2,                  2, 0x00008000,  4, SD_FLOAT, 1);

    attilaLowLevelAPI.writeGPURegister(GPU_INTERPOLATION, COLOR_ATTRIBUTE, true);

    attilaLowLevelAPI.clearColorBuffer();

    attilaLowLevelAPI.draw(TRIANGLE, 0, 9);

    attilaLowLevelAPI.swap();
    
    //  Second frame.  Test attribute load bypass mode.
    
    attilaLowLevelAPI.clearColorBuffer();
    
    vpCodeSize = 8192;
    
    result = attilaLowLevelAPI.assembleProgramFromFile("vprogram2.asm", vpCode, vpCodeSize, errorString);
    
    if (!result)
    {
        printf("Error assembling vertex program.\n");
        printf("%s\n", errorString.c_str());
        return -1;
    }

    attilaLowLevelAPI.loadVertexProgram(0x00000000, 0, vpCodeSize, vpCode, 1, nextBufferID++);

    attilaLowLevelAPI.resetStreamer();
    
    attilaLowLevelAPI.setAttributeStream(0, POSITION_ATTRIBUTE, 0x00004000, 12, SD_FLOAT, 3);
    attilaLowLevelAPI.setAttributeStream(1,    COLOR_ATTRIBUTE, 0x00006000, 12, SD_FLOAT, 3);

    attilaLowLevelAPI.writeGPURegister(GPU_ATTRIBUTE_LOAD_BYPASS, 0, true);
    
    attilaLowLevelAPI.draw(TRIANGLE, 0, 9);
    
    attilaLowLevelAPI.swap();
}


