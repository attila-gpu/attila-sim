
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "GPU.h"

using namespace std;
using namespace gpu3d;


void dumpRegisters(GPUState &state);

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Usage:\n");
        printf("  dumpRegisters <register snapshot file>\n");
        exit(-1);
    }

    ifstream inFile;

    inFile.open(argv[1]);

    if (!inFile.is_open())
    {
        printf("Error opening register snapshot file\n");
    }

    GPUState state;

    inFile.read((char *) &state, sizeof(state));

    inFile.close();

    dumpRegisters(state);
}


// Macro for fast typing in AGPTransaction::dump() method */
#define CASE_PRINT(value)\
    case  value:\
        printf(#value); \
        break;


void dumpRegisters(GPUState &state)
{

    printf("StatusRegister = ");
    switch (state.statusRegister)
    {
        CASE_PRINT(GPU_ST_RESET)
        CASE_PRINT(GPU_READY)
        CASE_PRINT(GPU_DRAWING)
        CASE_PRINT(GPU_END_GEOMETRY)
        CASE_PRINT(GPU_END_FRAGMENT)
        CASE_PRINT(GPU_MEMORY_READ)
        CASE_PRINT(GPU_MEMORY_WRITE)
        CASE_PRINT(GPU_MEMORY_PRELOAD)
        CASE_PRINT(GPU_SWAP)
        CASE_PRINT(GPU_DUMPBUFFER)
        CASE_PRINT(GPU_BLITTING)
        CASE_PRINT(GPU_CLEAR_COLOR)
        CASE_PRINT(GPU_CLEAR_Z)
        CASE_PRINT(GPU_FLUSH_COLOR)
        CASE_PRINT(GPU_FLUSH_Z)
        CASE_PRINT(GPU_SAVE_STATE_COLOR)
        CASE_PRINT(GPU_RESTORE_STATE_COLOR)
        CASE_PRINT(GPU_SAVE_STATE_Z)
        CASE_PRINT(GPU_RESTORE_STATE_Z)
        CASE_PRINT(GPU_ERROR)
        default:
            printf("Unknown");
            break;

    }
    printf("\n");

    printf("DisplayResX = %d\n", state.displayResX);
    printf("DisplayResY = %d\n", state.displayResY);

    printf("FrontBufferBaseAddr = %08x\n", state.frontBufferBaseAddr);
    printf("BackBufferBaseAddr = %08x\n", state.backBufferBaseAddr);
    printf("ZStencilBufferBaseAddr = %08x\n", state.backBufferBaseAddr);
    printf("TextureMemoryBaseAddr = %08x\n", state.textureMemoryBaseAddr);
    printf("ProgramMemoryBaseAddr = %08x\n", state.programMemoryBaseAddr);

    printf("VextexProgramAddr = %08x\n", state.vertexProgramAddr);
    printf("VertexProgramStartPC = %06x\n", state.vertexProgramStartPC);
    printf("VertexProgramSize = %d\n", state.vertexProgramSize);
    printf("VertexThreadResources = %d\n", state.vertexThreadResources);
    for (u32bit c = 0; c < MAX_VERTEX_CONSTANTS; c++)
        printf("VConstant[%04d] = {%f, %f, %f, %f}\n", c,
            state.vConstants[c][0], state.vConstants[c][1], state.vConstants[c][2], state.vConstants[c][3]);
    for (u32bit a = 0; a < MAX_VERTEX_ATTRIBUTES; a++)
        printf("OutputAttribute[%02d] = %s\n", a, state.outputAttribute[a] ? "T" : "F");

    for (u32bit a = 0; a < MAX_VERTEX_ATTRIBUTES; a++)
        printf("AttributeMap[%02d] = %d\n", a, state.attributeMap[a]);
    for (u32bit a = 0; a < MAX_VERTEX_ATTRIBUTES; a++)
        printf("attrDefValue[%02d] = {%f, %f, %f, %f}\n", a,
            state.attrDefValue[a][0], state.attrDefValue[a][1], state.attrDefValue[a][2], state.attrDefValue[a][3]);
    for (u32bit s = 0; s < MAX_STREAM_BUFFERS; s++)
        printf("StreamAddress[%02d] = %08x\n", s, state.streamAddress[s]);
    for (u32bit s = 0; s < MAX_STREAM_BUFFERS; s++)
        printf("StreamStride[%02d] = %d\n", s, state.streamStride[s]);
    for (u32bit s = 0; s < MAX_STREAM_BUFFERS; s++)
    {
        printf("StreamData[%02d] = ", s);
        switch(state.streamData[s])
        {
            CASE_PRINT(SD_UNORM8)
            CASE_PRINT(SD_SNORM8)
            CASE_PRINT(SD_UNORM16)
            CASE_PRINT(SD_SNORM16)
            CASE_PRINT(SD_UNORM32)
            CASE_PRINT(SD_SNORM32)
            CASE_PRINT(SD_FLOAT16)
            CASE_PRINT(SD_FLOAT32)
            CASE_PRINT(SD_UINT8)
            CASE_PRINT(SD_SINT8)
            CASE_PRINT(SD_UINT16)
            CASE_PRINT(SD_SINT16)
            CASE_PRINT(SD_UINT32)
            CASE_PRINT(SD_SINT32)
            default:
                printf("unknown");
                break;
        }
        printf("\n");
    }
    for (u32bit s = 0; s < MAX_STREAM_BUFFERS; s++)
        printf("StreamElements[%02d] = %d\n", s, state.streamElements[s]);
    for (u32bit s = 0; s < MAX_STREAM_BUFFERS; s++)
        printf("StreamFrequency[%02d] = %d\n", s, state.streamFrequency[s]);
    printf("StreamStart = %d\n", state.streamStart);
    printf("StreamCount = %d\n", state.streamCount);
    printf("StreamInstances = %d\n", state.streamInstances);
    printf("IndexedMode = %s\n", state.indexedMode ? "T" : "F");
    printf("IndexStream = %d\n", state.indexStream);
    for (u32bit s = 0; s < MAX_STREAM_BUFFERS; s++)
        printf("D3D9ColorStream[%02d] = %s\n", s, state.d3d9ColorStream[s] ? "T" : "F");
    printf("AttributeLoadBypass = %s\n", state.attributeLoadBypass ? "T" : "F");

    printf("PrimitiveMode = ");
    switch(state.primitiveMode)
    {
        CASE_PRINT(TRIANGLE)
        CASE_PRINT(TRIANGLE_STRIP)
        CASE_PRINT(TRIANGLE_FAN)
        CASE_PRINT(QUAD)
        CASE_PRINT(QUAD_STRIP)
        CASE_PRINT(LINE)
        CASE_PRINT(LINE_STRIP)
        CASE_PRINT(LINE_FAN)
        CASE_PRINT(POINT)
        default:
            printf("unknown");
            break;
    }
    printf("\n");

    printf("FrustumClipping = %s\n", state.frustumClipping ? "T" : "F");
    for (u32bit c = 0; c < MAX_USER_CLIP_PLANES; c++)
        printf("UserClip[%02d] = {%f, %f, %f, %f}\n", c, state.userClip);
    printf("UserClipPlanes = %s\n", state.userClipPlanes ? "T" : "F");
    printf("FaceMode = ");
    switch(state.faceMode)
    {
        CASE_PRINT(GPU_CW)
        CASE_PRINT(GPU_CCW)
        default:
            printf("unknown");
            break;
    }
    printf("\n");
    printf("CullMode = ");
    switch(state.cullMode)
    {
        CASE_PRINT(NONE)
        CASE_PRINT(FRONT)
        CASE_PRINT(BACK)
        CASE_PRINT(FRONT_AND_BACK)
        default:
            printf("unknown");
            break;
    }
    printf("\n");

    printf("HZTest = %s\n", state.hzTest ? "T" : "F");
    printf("EarlyZ = %s\n", state.earlyZ ? "T" : "F");

    printf("D3D9PixelCoordinates = %s\n", state.d3d9PixelCoordinates ? "T" : "F");
    printf("ViewportIniX = %d\n", state.viewportIniX);
    printf("ViewportIniY = %d\n", state.viewportIniY);
    printf("ViewportHeight = %d\n", state.viewportHeight);
    printf("ViewportWidth = %d\n", state.viewportWidth);
    printf("ScissorTest = %s\n", state.scissorTest ? "T" : "F");
    printf("ScissorIniX = %d\n", state.scissorIniX);
    printf("ScissorIniY = %d\n", state.scissorIniY);
    printf("ScissorHeight = %d\n", state.scissorHeight);
    printf("ScissorWidth = %d\n", state.scissorWidth);
    printf("NearRange = %f\n", state.nearRange);
    printf("FarRange = %f\n", state.farRange);
    printf("SlopeFactor = %f\n", state.slopeFactor);
    printf("UnitOffset = %f\n", state.unitOffset);
    printf("D3D9DepthRange = %s\n", state.d3d9DepthRange ? "T" : "F");
    printf("ZBufferBitPrecission = %d\n", state.zBufferBitPrecission);
    printf("D3D9RasterizationRules = %s\n", state.d3d9RasterizationRules ? "T" : "F");
    printf("TwoSidedLighting = %s\n", state.twoSidedLighting ? "T" : "F");
    printf("MultiSampling = %s\n", state.multiSampling ? "T" : "F");
    printf("MSAASamples = %d\n", state.msaaSamples);
    for(u32bit a = 0; a < MAX_FRAGMENT_ATTRIBUTES; a++)
        printf("Interpolation[%02d] = %s\n", a, state.interpolation[a] ? "T" : "F");
    for(u32bit a = 0; a < MAX_FRAGMENT_ATTRIBUTES; a++)
        printf("FragmentInputAttributes[%02d] = %s\n", a, state.fragmentInputAttributes[a] ? "T" : "F");

    printf("FragProgramAddr = %08x\n", state.fragProgramAddr);
    printf("FragProgramStartPC = %06x\n", state.fragProgramStartPC);
    printf("FragProgramSize = %d\n", state.fragProgramSize);
    printf("FragThreadResources = %d\n", state.fragThreadResources);
    for (u32bit c = 0; c < MAX_VERTEX_CONSTANTS; c++)
        printf("FConstants[%04d] = {%f, %f, %f, %f}\n", c,
            state.fConstants[c][0], state.fConstants[c][1], state.fConstants[c][2], state.fConstants[c][3]);
    printf("ModifyDepth = %s\n", state.modifyDepth ? "T" : "F");

    printf("programAddress = %08x\n", state.programAddress);
    printf("ProgramSize = %d\n", state.programSize);
    printf("ProgramLoadPC = %06x\n", state.programLoadPC);
    for(u32bit t = 0; t < MAX_SHADER_TARGETS; t++)
        printf("ProgramStartPC[%02d] = %d\n", t, state.programStartPC[t]);
    for(u32bit t = 0; t < MAX_SHADER_TARGETS; t++)
        printf("ProgramResources[%02d] = %d\n", t, state.programResources[t]);

    for(u32bit t = 0; t < MAX_TEXTURES; t++)
        printf("TextureEnabled[%02d] = %s\n", t, state.textureEnabled[t] ? "T" : "F");
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
    {
        printf("TextureMode[%02d] = ", t);
        switch(state.textureMode[t])
        {
            CASE_PRINT(GPU_TEXTURE1D)
            CASE_PRINT(GPU_TEXTURE2D)
            CASE_PRINT(GPU_TEXTURE3D)
            CASE_PRINT(GPU_TEXTURECUBEMAP)
            default:
                printf("unknown");
                break;
        }
        printf("\n");
    }
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
        for(u32bit m = 0; m < MAX_TEXTURE_SIZE; m++)
            for(u32bit c = 0; c < CUBEMAP_IMAGES; c++)
                printf("TextureAddress[%02d][%02d][%02d] = %08x\n", t, m, c, state.textureAddress[t][m][c]);
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
        printf("TextureWidth[%02d] = %d\n", t, state.textureWidth[t]);
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
        printf("TextureHeight[%02d] = %d\n", t, state.textureHeight[t]);
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
        printf("TextureDepth[%02d] = %d\n", t, state.textureDepth[t]);
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
        printf("TextureWidth2[%02d] = %d\n", t, state.textureWidth2[t]);
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
        printf("TextureHeight2[%02d] = %d\n", t, state.textureHeight2[t]);
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
        printf("TextureDepth2[%02d] = %d\n", t, state.textureDepth2[t]);
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
        printf("TextureBorder[%02d] = %d\n", t, state.textureBorder[t]);
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
        printf("TextBorderColor[%02d] = {%f, %f, %f, %f}\n", t,
            state.textBorderColor[t][0], state.textBorderColor[t][1], state.textBorderColor[t][2], state.textBorderColor[t][3]);
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
    {
        printf("TextureFormat[%02d] = ", t);
        switch(state.textureFormat[t])
        {
            CASE_PRINT(GPU_ALPHA8)
            CASE_PRINT(GPU_ALPHA12)
            CASE_PRINT(GPU_ALPHA16)
            CASE_PRINT(GPU_DEPTH_COMPONENT16)
            CASE_PRINT(GPU_DEPTH_COMPONENT24)
            CASE_PRINT(GPU_DEPTH_COMPONENT32)
            CASE_PRINT(GPU_LUMINANCE8)
            CASE_PRINT(GPU_LUMINANCE8_SIGNED)
            CASE_PRINT(GPU_LUMINANCE12)
            CASE_PRINT(GPU_LUMINANCE16)
            CASE_PRINT(GPU_LUMINANCE4_ALPHA4)
            CASE_PRINT(GPU_LUMINANCE6_ALPHA2)
            CASE_PRINT(GPU_LUMINANCE8_ALPHA8)
            CASE_PRINT(GPU_LUMINANCE8_ALPHA8_SIGNED)
            CASE_PRINT(GPU_LUMINANCE12_ALPHA4)
            CASE_PRINT(GPU_LUMINANCE12_ALPHA12)
            CASE_PRINT(GPU_LUMINANCE16_ALPHA16)
            CASE_PRINT(GPU_INTENSITY8)
            CASE_PRINT(GPU_INTENSITY12)
            CASE_PRINT(GPU_INTENSITY16)
            CASE_PRINT(GPU_RGB332)
            CASE_PRINT(GPU_RGB444)
            CASE_PRINT(GPU_RGB555)
            CASE_PRINT(GPU_RGB565)
            CASE_PRINT(GPU_RGB888)
            CASE_PRINT(GPU_RGB101010)
            CASE_PRINT(GPU_RGB121212)
            CASE_PRINT(GPU_RGBA2222)
            CASE_PRINT(GPU_RGBA4444)
            CASE_PRINT(GPU_RGBA5551)
            CASE_PRINT(GPU_RGBA8888)
            CASE_PRINT(GPU_RGBA1010102)
            CASE_PRINT(GPU_R16)
            CASE_PRINT(GPU_RG16)
            CASE_PRINT(GPU_RGBA16)
            CASE_PRINT(GPU_R16F)
            CASE_PRINT(GPU_RG16F)
            CASE_PRINT(GPU_RGBA16F)
            CASE_PRINT(GPU_R32F)
            CASE_PRINT(GPU_RG32F)
            CASE_PRINT(GPU_RGBA32F)
            default:
                printf("unknown");
                break;
        }
        printf("\n");
    }
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
    {
        printf("TextureCompr[%02d] = ", t);
        switch(state.textureCompr[t])
        {
            CASE_PRINT(GPU_NO_TEXTURE_COMPRESSION)
            CASE_PRINT(GPU_S3TC_DXT1_RGB)
            CASE_PRINT(GPU_S3TC_DXT1_RGBA)
            CASE_PRINT(GPU_S3TC_DXT3_RGBA)
            CASE_PRINT(GPU_S3TC_DXT5_RGBA)
            CASE_PRINT(GPU_LATC1)
            CASE_PRINT(GPU_LATC1_SIGNED)
            CASE_PRINT(GPU_LATC2)
            CASE_PRINT(GPU_LATC2_SIGNED)
            default:
                printf("unknown");
                break;
        }
        printf("\n");
    }
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
    {
        printf("TextureBlocking[%02d] = ", t);
        switch(state.textureBlocking[t])
        {
            CASE_PRINT(GPU_TXBLOCK_TEXTURE)
            CASE_PRINT(GPU_TXBLOCK_FRAMEBUFFER)
            default:
                printf("unknown");
                break;
        }
        printf("\n");
    }
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
        printf("TextureReverse[%02d] = %s\n", t, state.textureReverse[t] ? "T" : "F");
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
        printf("TextD3D9ColorConv[%02d] = %s\n", t, state.textD3D9ColorConv[t] ? "T" : "F");
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
        printf("TextD3D9VInvert[%02d] = %s\n", t, state.textD3D9VInvert[t] ? "T" : "F");
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
    {
        printf("TextureWrapS[%02d] = ", t);
        switch(state.textureWrapS[t])
        {
            CASE_PRINT(GPU_TEXT_CLAMP)
            CASE_PRINT(GPU_TEXT_CLAMP_TO_EDGE)
            CASE_PRINT(GPU_TEXT_REPEAT)
            CASE_PRINT(GPU_TEXT_CLAMP_TO_BORDER)
            CASE_PRINT(GPU_TEXT_MIRRORED_REPEAT)
            default:
                printf("unknown");
                break;
        }
        printf("\n");
    }
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
    {
        printf("TextureWrapT[%02d] = ", t);
        switch(state.textureWrapT[t])
        {
            CASE_PRINT(GPU_TEXT_CLAMP)
            CASE_PRINT(GPU_TEXT_CLAMP_TO_EDGE)
            CASE_PRINT(GPU_TEXT_REPEAT)
            CASE_PRINT(GPU_TEXT_CLAMP_TO_BORDER)
            CASE_PRINT(GPU_TEXT_MIRRORED_REPEAT)
            default:
                printf("unknown");
                break;
        }
        printf("\n");
    }
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
    {
        printf("TextureWrapR[%02d] = ", t);
        switch(state.textureWrapR[t])
        {
            CASE_PRINT(GPU_TEXT_CLAMP)
            CASE_PRINT(GPU_TEXT_CLAMP_TO_EDGE)
            CASE_PRINT(GPU_TEXT_REPEAT)
            CASE_PRINT(GPU_TEXT_CLAMP_TO_BORDER)
            CASE_PRINT(GPU_TEXT_MIRRORED_REPEAT)
            default:
                printf("unknown");
                break;
        }
        printf("\n");
    }
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
        printf("TextureNonNormalized[%02d] = %s\n", t, state.textureNonNormalized[t] ? "T" : "F");
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
    {
        printf("TextureMinFilter[%02d] = ", t);
        switch(state.textureMinFilter[t])
        {
            CASE_PRINT(GPU_NEAREST)
            CASE_PRINT(GPU_LINEAR)
            CASE_PRINT(GPU_NEAREST_MIPMAP_NEAREST)
            CASE_PRINT(GPU_NEAREST_MIPMAP_LINEAR)
            CASE_PRINT(GPU_LINEAR_MIPMAP_NEAREST)
            CASE_PRINT(GPU_LINEAR_MIPMAP_LINEAR)
            default:
                printf("unknown");
                break;
        }
        printf("\n");
    }
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
    {
        printf("TextureMagFilter[%02d] = ", t);
        switch(state.textureMagFilter[t])
        {
            CASE_PRINT(GPU_NEAREST)
            CASE_PRINT(GPU_LINEAR)
            CASE_PRINT(GPU_NEAREST_MIPMAP_NEAREST)
            CASE_PRINT(GPU_NEAREST_MIPMAP_LINEAR)
            CASE_PRINT(GPU_LINEAR_MIPMAP_NEAREST)
            CASE_PRINT(GPU_LINEAR_MIPMAP_LINEAR)
            default:
                printf("unknown");
                break;
        }
        printf("\n");
    }
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
        printf("TextureEnableComparison[%02d] = %s\n", t, state.textureEnableComparison[t] ? "T" : "F");
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
    {
        printf("TextureMagFilter[%02d] = ", t);
        switch(state.textureMagFilter[t])
        {
            CASE_PRINT(GPU_NEVER)
            CASE_PRINT(GPU_ALWAYS)
            CASE_PRINT(GPU_LESS)
            CASE_PRINT(GPU_LEQUAL)
            CASE_PRINT(GPU_EQUAL)
            CASE_PRINT(GPU_GEQUAL)
            CASE_PRINT(GPU_GREATER)
            CASE_PRINT(GPU_NOTEQUAL)
            default:
                printf("unknown");
                break;
        }
        printf("\n");
    }
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
        printf("TextureSRGB[%02d] = %s\n", t, state.textureSRGB[t] ? "T" : "F");
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
        printf("TextureMinLOD[%02d] = %f\n", t, state.textureMinLOD[t]);
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
        printf("TextureMaxLOD[%02d] = %f\n", t, state.textureMaxLOD[t]);
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
        printf("TextureLODBias[%02d] = %f\n", t, state.textureLODBias[t]);
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
        printf("TextureMinLevel[%02d] = %d\n", t, state.textureMinLevel[t]);
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
        printf("TextureMaxLevel[%02d] = %d\n", t, state.textureMaxLevel[t]);
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
        printf("TextureUnitLODBias[%02d] = %f\n", t, state.textureUnitLODBias[t]);
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
        printf("MaxAnisotropy[%02d] = %d\n", t, state.maxAnisotropy[t]);

    printf("ZBufferClear = %08x\n", state.zBufferClear);
    printf("ZSencilBufferClear = %08x\n", state.stencilBufferClear);
    printf("ZStencilStateBufferAddr = %08x\n", state.zstencilStateBufferAddr);

    printf("StencilTest = %s\n", state.stencilTest ? "T" : "F");
    printf("StencilFunction = ");
    switch(state.stencilFunction)
    {
        CASE_PRINT(GPU_NEVER)
        CASE_PRINT(GPU_ALWAYS)
        CASE_PRINT(GPU_LESS)
        CASE_PRINT(GPU_LEQUAL)
        CASE_PRINT(GPU_EQUAL)
        CASE_PRINT(GPU_GEQUAL)
        CASE_PRINT(GPU_GREATER)
        CASE_PRINT(GPU_NOTEQUAL)
        default:
            printf("unknown");
            break;
    }
    printf("\n");
    printf("StencilReference = %02x\n", state.stencilReference);
    printf("StencilTestMask = %02x\n", state.stencilTestMask);
    printf("StencilUpdateMask = %02x\n", state.stencilUpdateMask);
    printf("StencilFunction = ");
    switch(state.stencilFail)
    {
        CASE_PRINT(STENCIL_KEEP)
        CASE_PRINT(STENCIL_ZERO)
        CASE_PRINT(STENCIL_REPLACE)
        CASE_PRINT(STENCIL_INCR)
        CASE_PRINT(STENCIL_DECR)
        CASE_PRINT(STENCIL_INVERT)
        CASE_PRINT(STENCIL_INCR_WRAP)
        CASE_PRINT(STENCIL_DECR_WRAP)
        default:
            printf("unknown");
            break;
    }
    printf("\n");
    printf("DepthFail = ");
    switch(state.depthFail)
    {
        CASE_PRINT(STENCIL_KEEP)
        CASE_PRINT(STENCIL_ZERO)
        CASE_PRINT(STENCIL_REPLACE)
        CASE_PRINT(STENCIL_INCR)
        CASE_PRINT(STENCIL_DECR)
        CASE_PRINT(STENCIL_INVERT)
        CASE_PRINT(STENCIL_INCR_WRAP)
        CASE_PRINT(STENCIL_DECR_WRAP)
        default:
            printf("unknown");
            break;
    }
    printf("\n");
    printf("DepthPass = ");
    switch(state.depthPass)
    {
        CASE_PRINT(STENCIL_KEEP)
        CASE_PRINT(STENCIL_ZERO)
        CASE_PRINT(STENCIL_REPLACE)
        CASE_PRINT(STENCIL_INCR)
        CASE_PRINT(STENCIL_DECR)
        CASE_PRINT(STENCIL_INVERT)
        CASE_PRINT(STENCIL_INCR_WRAP)
        CASE_PRINT(STENCIL_DECR_WRAP)
        default:
            printf("unknown");
            break;
    }
    printf("\n");

    printf("DepthTest = %s\n", state.depthTest ? "T" : "F");
    printf("DepthFunction = ");
    switch(state.depthFunction)
    {
        CASE_PRINT(GPU_NEVER)
        CASE_PRINT(GPU_ALWAYS)
        CASE_PRINT(GPU_LESS)
        CASE_PRINT(GPU_LEQUAL)
        CASE_PRINT(GPU_EQUAL)
        CASE_PRINT(GPU_GEQUAL)
        CASE_PRINT(GPU_GREATER)
        CASE_PRINT(GPU_NOTEQUAL)
        default:
            printf("unknown");
            break;
    }
    printf("\n");
    printf("DepthMask = %s\n", state.depthMask ? "T" : "F");

    printf("ZStencilCompression = %s\n", state.zStencilCompression ? "T" : "F");

    printf("colorBufferFormat = ");
    switch(state.colorBufferFormat)
    {
        CASE_PRINT(GPU_ALPHA8)
        CASE_PRINT(GPU_ALPHA12)
        CASE_PRINT(GPU_ALPHA16)
        CASE_PRINT(GPU_DEPTH_COMPONENT16)
        CASE_PRINT(GPU_DEPTH_COMPONENT24)
        CASE_PRINT(GPU_DEPTH_COMPONENT32)
        CASE_PRINT(GPU_LUMINANCE8)
        CASE_PRINT(GPU_LUMINANCE8_SIGNED)
        CASE_PRINT(GPU_LUMINANCE12)
        CASE_PRINT(GPU_LUMINANCE16)
        CASE_PRINT(GPU_LUMINANCE4_ALPHA4)
        CASE_PRINT(GPU_LUMINANCE6_ALPHA2)
        CASE_PRINT(GPU_LUMINANCE8_ALPHA8)
        CASE_PRINT(GPU_LUMINANCE8_ALPHA8_SIGNED)
        CASE_PRINT(GPU_LUMINANCE12_ALPHA4)
        CASE_PRINT(GPU_LUMINANCE12_ALPHA12)
        CASE_PRINT(GPU_LUMINANCE16_ALPHA16)
        CASE_PRINT(GPU_INTENSITY8)
        CASE_PRINT(GPU_INTENSITY12)
        CASE_PRINT(GPU_INTENSITY16)
        CASE_PRINT(GPU_RGB332)
        CASE_PRINT(GPU_RGB444)
        CASE_PRINT(GPU_RGB555)
        CASE_PRINT(GPU_RGB565)
        CASE_PRINT(GPU_RGB888)
        CASE_PRINT(GPU_RGB101010)
        CASE_PRINT(GPU_RGB121212)
        CASE_PRINT(GPU_RGBA2222)
        CASE_PRINT(GPU_RGBA4444)
        CASE_PRINT(GPU_RGBA5551)
        CASE_PRINT(GPU_RGBA8888)
        CASE_PRINT(GPU_RGBA1010102)
        CASE_PRINT(GPU_R16)
        CASE_PRINT(GPU_RG16)
        CASE_PRINT(GPU_RGBA16)
        CASE_PRINT(GPU_R16F)
        CASE_PRINT(GPU_RG16F)
        CASE_PRINT(GPU_RGBA16F)
        CASE_PRINT(GPU_R32F)
        CASE_PRINT(GPU_RG32F)
        CASE_PRINT(GPU_RGBA32F)
        default:
            printf("unknown");
            break;
    }
    printf("\n");
    printf("ColorCompression = %s\n", state.colorCompression ? "T" : "F");
    printf("ColorSRGBWrite = %s\n", state.colorSRGBWrite ? "T" : "F");

    for(u32bit t = 0; t < MAX_RENDER_TARGETS; t++)
        printf("RTEnable[%02d] = %s\n", t, state.rtEnable[t] ? "T" : "F");
    for(u32bit t = 0; t < MAX_RENDER_TARGETS; t++)
    {
        printf("RTFormat[%02d] = ", t);
        switch(state.rtFormat[t])
        {
            CASE_PRINT(GPU_ALPHA8)
            CASE_PRINT(GPU_ALPHA12)
            CASE_PRINT(GPU_ALPHA16)
            CASE_PRINT(GPU_DEPTH_COMPONENT16)
            CASE_PRINT(GPU_DEPTH_COMPONENT24)
            CASE_PRINT(GPU_DEPTH_COMPONENT32)
            CASE_PRINT(GPU_LUMINANCE8)
            CASE_PRINT(GPU_LUMINANCE8_SIGNED)
            CASE_PRINT(GPU_LUMINANCE12)
            CASE_PRINT(GPU_LUMINANCE16)
            CASE_PRINT(GPU_LUMINANCE4_ALPHA4)
            CASE_PRINT(GPU_LUMINANCE6_ALPHA2)
            CASE_PRINT(GPU_LUMINANCE8_ALPHA8)
            CASE_PRINT(GPU_LUMINANCE8_ALPHA8_SIGNED)
            CASE_PRINT(GPU_LUMINANCE12_ALPHA4)
            CASE_PRINT(GPU_LUMINANCE12_ALPHA12)
            CASE_PRINT(GPU_LUMINANCE16_ALPHA16)
            CASE_PRINT(GPU_INTENSITY8)
            CASE_PRINT(GPU_INTENSITY12)
            CASE_PRINT(GPU_INTENSITY16)
            CASE_PRINT(GPU_RGB332)
            CASE_PRINT(GPU_RGB444)
            CASE_PRINT(GPU_RGB555)
            CASE_PRINT(GPU_RGB565)
            CASE_PRINT(GPU_RGB888)
            CASE_PRINT(GPU_RGB101010)
            CASE_PRINT(GPU_RGB121212)
            CASE_PRINT(GPU_RGBA2222)
            CASE_PRINT(GPU_RGBA4444)
            CASE_PRINT(GPU_RGBA5551)
            CASE_PRINT(GPU_RGBA8888)
            CASE_PRINT(GPU_RGBA1010102)
            CASE_PRINT(GPU_R16)
            CASE_PRINT(GPU_RG16)
            CASE_PRINT(GPU_RGBA16)
            CASE_PRINT(GPU_R16F)
            CASE_PRINT(GPU_RG16F)
            CASE_PRINT(GPU_RGBA16F)
            CASE_PRINT(GPU_R32F)
            CASE_PRINT(GPU_RG32F)
            CASE_PRINT(GPU_RGBA32F)
            default:
                printf("unknown");
                break;
        }
        printf("\n");
    }
    for(u32bit t = 0; t < MAX_RENDER_TARGETS; t++)
        printf("rtAddress[%02d] = %08x\n", t, state.rtAddress[t]);

    printf("ColorBufferClear = {%f, %f, %f, %f}\n",
        state.colorBufferClear[0], state.colorBufferClear[1], state.colorBufferClear[2], state.colorBufferClear[3]);
    printf("colorStateBufferAddr = %08x\n", state.colorStateBufferAddr);

    for(u32bit t = 0; t < MAX_RENDER_TARGETS; t++)
        printf("ColorBlend[%02d] = %s\n", t, state.colorBlend[t] ? "T" : "F");
    for(u32bit t = 0; t < MAX_RENDER_TARGETS; t++)
    {
        printf("BlendEquation[%02d] = ", t);
        switch(state.blendEquation[t])
        {
            CASE_PRINT(BLEND_FUNC_ADD)
            CASE_PRINT(BLEND_FUNC_SUBTRACT)
            CASE_PRINT(BLEND_FUNC_REVERSE_SUBTRACT)
            CASE_PRINT(BLEND_MIN)
            CASE_PRINT(BLEND_MAX)
            default:
                printf("unknown");
                break;
        }
        printf("\n");
    }
    for(u32bit t = 0; t < MAX_RENDER_TARGETS; t++)
    {
        printf("BlendSourceRGB[%02d] = ", t);
        switch(state.blendSourceRGB[t])
        {
            CASE_PRINT(BLEND_ZERO)
            CASE_PRINT(BLEND_ONE)
            CASE_PRINT(BLEND_SRC_COLOR)
            CASE_PRINT(BLEND_ONE_MINUS_SRC_COLOR)
            CASE_PRINT(BLEND_DST_COLOR)
            CASE_PRINT(BLEND_ONE_MINUS_DST_COLOR)
            CASE_PRINT(BLEND_SRC_ALPHA)
            CASE_PRINT(BLEND_ONE_MINUS_SRC_ALPHA)
            CASE_PRINT(BLEND_DST_ALPHA)
            CASE_PRINT(BLEND_ONE_MINUS_DST_ALPHA)
            CASE_PRINT(BLEND_CONSTANT_COLOR)
            CASE_PRINT(BLEND_ONE_MINUS_CONSTANT_COLOR)
            CASE_PRINT(BLEND_CONSTANT_ALPHA)
            CASE_PRINT(BLEND_ONE_MINUS_CONSTANT_ALPHA)
            CASE_PRINT(BLEND_SRC_ALPHA_SATURATE)
            default:
                printf("unknown");
                break;
        }
        printf("\n");
    }
    for(u32bit t = 0; t < MAX_RENDER_TARGETS; t++)
    {
        printf("BlendDestinationRGB[%02d] = ", t);
        switch(state.blendDestinationRGB[t])
        {
            CASE_PRINT(BLEND_ZERO)
            CASE_PRINT(BLEND_ONE)
            CASE_PRINT(BLEND_SRC_COLOR)
            CASE_PRINT(BLEND_ONE_MINUS_SRC_COLOR)
            CASE_PRINT(BLEND_DST_COLOR)
            CASE_PRINT(BLEND_ONE_MINUS_DST_COLOR)
            CASE_PRINT(BLEND_SRC_ALPHA)
            CASE_PRINT(BLEND_ONE_MINUS_SRC_ALPHA)
            CASE_PRINT(BLEND_DST_ALPHA)
            CASE_PRINT(BLEND_ONE_MINUS_DST_ALPHA)
            CASE_PRINT(BLEND_CONSTANT_COLOR)
            CASE_PRINT(BLEND_ONE_MINUS_CONSTANT_COLOR)
            CASE_PRINT(BLEND_CONSTANT_ALPHA)
            CASE_PRINT(BLEND_ONE_MINUS_CONSTANT_ALPHA)
            CASE_PRINT(BLEND_SRC_ALPHA_SATURATE)
            default:
                printf("unknown");
                break;
        }
        printf("\n");
    }
    for(u32bit t = 0; t < MAX_RENDER_TARGETS; t++)
    {
        printf("BlendSourceAlpha[%02d] = ", t);
        switch(state.blendSourceAlpha[t])
        {
            CASE_PRINT(BLEND_ZERO)
            CASE_PRINT(BLEND_ONE)
            CASE_PRINT(BLEND_SRC_COLOR)
            CASE_PRINT(BLEND_ONE_MINUS_SRC_COLOR)
            CASE_PRINT(BLEND_DST_COLOR)
            CASE_PRINT(BLEND_ONE_MINUS_DST_COLOR)
            CASE_PRINT(BLEND_SRC_ALPHA)
            CASE_PRINT(BLEND_ONE_MINUS_SRC_ALPHA)
            CASE_PRINT(BLEND_DST_ALPHA)
            CASE_PRINT(BLEND_ONE_MINUS_DST_ALPHA)
            CASE_PRINT(BLEND_CONSTANT_COLOR)
            CASE_PRINT(BLEND_ONE_MINUS_CONSTANT_COLOR)
            CASE_PRINT(BLEND_CONSTANT_ALPHA)
            CASE_PRINT(BLEND_ONE_MINUS_CONSTANT_ALPHA)
            CASE_PRINT(BLEND_SRC_ALPHA_SATURATE)
            default:
                printf("unknown");
                break;
        }
        printf("\n");
    }
    for(u32bit t = 0; t < MAX_RENDER_TARGETS; t++)
    {
        printf("BlendDestinationAlpha[%02d] = ", t);
        switch(state.blendDestinationAlpha[t])
        {
            CASE_PRINT(BLEND_ZERO)
            CASE_PRINT(BLEND_ONE)
            CASE_PRINT(BLEND_SRC_COLOR)
            CASE_PRINT(BLEND_ONE_MINUS_SRC_COLOR)
            CASE_PRINT(BLEND_DST_COLOR)
            CASE_PRINT(BLEND_ONE_MINUS_DST_COLOR)
            CASE_PRINT(BLEND_SRC_ALPHA)
            CASE_PRINT(BLEND_ONE_MINUS_SRC_ALPHA)
            CASE_PRINT(BLEND_DST_ALPHA)
            CASE_PRINT(BLEND_ONE_MINUS_DST_ALPHA)
            CASE_PRINT(BLEND_CONSTANT_COLOR)
            CASE_PRINT(BLEND_ONE_MINUS_CONSTANT_COLOR)
            CASE_PRINT(BLEND_CONSTANT_ALPHA)
            CASE_PRINT(BLEND_ONE_MINUS_CONSTANT_ALPHA)
            CASE_PRINT(BLEND_SRC_ALPHA_SATURATE)
            default:
                printf("unknown");
                break;
        }
        printf("\n");
    }
    for(u32bit t = 0; t < MAX_RENDER_TARGETS; t++)
        printf("BlendColor[%02d] = {%f, %f, %f, %f}\n", t,
            state.blendColor[t][0], state.blendColor[t][1], state.blendColor[t][2], state.blendColor[t][3]);
    for(u32bit t = 0; t < MAX_RENDER_TARGETS; t++)
        printf("ColorMaskR[%02d] = %s\n", t, state.colorMaskR[t] ? "T" : "F");
    for(u32bit t = 0; t < MAX_RENDER_TARGETS; t++)
        printf("ColorMaskG[%02d] = %s\n", t, state.colorMaskG[t] ? "T" : "F");
    for(u32bit t = 0; t < MAX_RENDER_TARGETS; t++)
        printf("ColorMaskB[%02d] = %s\n", t, state.colorMaskB[t] ? "T" : "F");
    for(u32bit t = 0; t < MAX_RENDER_TARGETS; t++)
        printf("ColorMaskA[%02d] = %s\n", t, state.colorMaskA[t] ? "T" : "F");

    printf("LogicalOperation= %s\n", state.logicalOperation ? "T" : "F");
    printf("LogicOpFunction = ");
    switch(state.logicOpFunction)
    {
        CASE_PRINT(LOGICOP_CLEAR)
        CASE_PRINT(LOGICOP_AND)
        CASE_PRINT(LOGICOP_AND_REVERSE)
        CASE_PRINT(LOGICOP_COPY)
        CASE_PRINT(LOGICOP_AND_INVERTED)
        CASE_PRINT(LOGICOP_NOOP)
        CASE_PRINT(LOGICOP_XOR)
        CASE_PRINT(LOGICOP_OR)
        CASE_PRINT(LOGICOP_NOR)
        CASE_PRINT(LOGICOP_EQUIV)
        CASE_PRINT(LOGICOP_INVERT)
        CASE_PRINT(LOGICOP_OR_REVERSE)
        CASE_PRINT(LOGICOP_COPY_INVERTED)
        CASE_PRINT(LOGICOP_OR_INVERTED)
        CASE_PRINT(LOGICOP_NAND)
        CASE_PRINT(LOGICOP_SET)
        default:
            printf("unknown");
            break;
    }
    printf("\n");

    printf("MCSecondInterleavingStartAddr = %08x\n", state.mcSecondInterleavingStartAddr);

    printf("BlitIniX = %d\n", state.blitIniX);
    printf("BlitIniY = %d\n", state.blitIniY);
    printf("BlitXOffset = %d\n", state.blitXOffset);
    printf("BlitYOffset = %d\n", state.blitYOffset);
    printf("BlitHeight = %d\n", state.blitHeight);
    printf("BlitWidth = %d\n", state.blitWidth);
    printf("BlitTextureWidth2 = %d\n", state.blitTextureWidth2);
    printf("BlitDestinationAddress = %08x\n", state.blitDestinationAddress);
    printf("BlitDestinationTextureFormat = ");
    switch(state.blitDestinationTextureFormat)
    {
        CASE_PRINT(GPU_ALPHA8)
        CASE_PRINT(GPU_ALPHA12)
        CASE_PRINT(GPU_ALPHA16)
        CASE_PRINT(GPU_DEPTH_COMPONENT16)
        CASE_PRINT(GPU_DEPTH_COMPONENT24)
        CASE_PRINT(GPU_DEPTH_COMPONENT32)
        CASE_PRINT(GPU_LUMINANCE8)
        CASE_PRINT(GPU_LUMINANCE8_SIGNED)
        CASE_PRINT(GPU_LUMINANCE12)
        CASE_PRINT(GPU_LUMINANCE16)
        CASE_PRINT(GPU_LUMINANCE4_ALPHA4)
        CASE_PRINT(GPU_LUMINANCE6_ALPHA2)
        CASE_PRINT(GPU_LUMINANCE8_ALPHA8)
        CASE_PRINT(GPU_LUMINANCE8_ALPHA8_SIGNED)
        CASE_PRINT(GPU_LUMINANCE12_ALPHA4)
        CASE_PRINT(GPU_LUMINANCE12_ALPHA12)
        CASE_PRINT(GPU_LUMINANCE16_ALPHA16)
        CASE_PRINT(GPU_INTENSITY8)
        CASE_PRINT(GPU_INTENSITY12)
        CASE_PRINT(GPU_INTENSITY16)
        CASE_PRINT(GPU_RGB332)
        CASE_PRINT(GPU_RGB444)
        CASE_PRINT(GPU_RGB555)
        CASE_PRINT(GPU_RGB565)
        CASE_PRINT(GPU_RGB888)
        CASE_PRINT(GPU_RGB101010)
        CASE_PRINT(GPU_RGB121212)
        CASE_PRINT(GPU_RGBA2222)
        CASE_PRINT(GPU_RGBA4444)
        CASE_PRINT(GPU_RGBA5551)
        CASE_PRINT(GPU_RGBA8888)
        CASE_PRINT(GPU_RGBA1010102)
        CASE_PRINT(GPU_R16)
        CASE_PRINT(GPU_RG16)
        CASE_PRINT(GPU_RGBA16)
        CASE_PRINT(GPU_R16F)
        CASE_PRINT(GPU_RG16F)
        CASE_PRINT(GPU_RGBA16F)
        CASE_PRINT(GPU_R32F)
        CASE_PRINT(GPU_RG32F)
        CASE_PRINT(GPU_RGBA32F)
        default:
            printf("unknown");
            break;
    }
    printf("\n");
    printf("BlitDestinationTextureBlocking = ");
    switch(state.blitDestinationTextureBlocking)
    {
        CASE_PRINT(GPU_TXBLOCK_TEXTURE)
        CASE_PRINT(GPU_TXBLOCK_FRAMEBUFFER)
        default:
            printf("unknown");
            break;
    }
    printf("\n");


}


