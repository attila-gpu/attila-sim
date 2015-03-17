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

#include "Common.h"

#include "ACD.h"
#include "ACDRasterizationStage.h"
#include "ACDZStencilStage.h"
#include "ACDBlendingStage.h"
#include "ACDShaderProgram.h"
#include "ACDBuffer.h"
#include "ACDTexture2D.h"
#include "ACDTextureCubeMap.h"
#include "ACDSampler.h"
#include "ACDRenderTarget.h"

#include "AD3D9State.h"

#include "AISurfaceImp_9.h"
#include "AIVolumeImp_9.h"
#include "AIIndexBufferImp_9.h"
#include "AIVertexBufferImp_9.h"
#include "AIVertexDeclarationImp_9.h"
#include "AIVertexShaderImp_9.h"
#include "AIPixelShaderImp_9.h"
#include "AITextureImp_9.h"
#include "AICubeTextureImp_9.h"
#include "AIVolumeTextureImp_9.h"

#include "ShaderTranslator.h"
#include "Utils.h"

#include <cstring>
#include <stdio.h>

AD3D9State& AD3D9State::instance()
{
	static AD3D9State inst;

	return inst;
}

AD3D9State::AD3D9State()
{
    acdDev = NULL;

    batchCount = 0;

    settedVertexDeclaration = NULL;
    blitTex2D = NULL;

    fogEnable = false;
    fogColor = 0;
    
    alphaTest = false;
    alphaRef = 0;
    alphaFunc = D3DCMP_ALWAYS;

    alphaBlend = false;
    separateAlphaBlend = false;
    srcBlend = acdlib::ACD_BLEND_ONE;
    srcBlendAlpha = acdlib::ACD_BLEND_ONE;
    dstBlend = acdlib::ACD_BLEND_ZERO;
    dstBlendAlpha = acdlib::ACD_BLEND_ZERO;
    blendOp = acdlib::ACD_BLEND_ADD;
    blendOpAlpha = acdlib::ACD_BLEND_ADD;

    stencilEnabled = false;
    stencilFail = acdlib::ACD_STENCIL_OP_KEEP;
    stencilZFail = acdlib::ACD_STENCIL_OP_KEEP;
    stencilPass = acdlib::ACD_STENCIL_OP_KEEP;
    stencilFunc = acdlib::ACD_COMPARE_FUNCTION_ALWAYS;
    stencilRef = 0;
    stencilMask = 0xFFFFFFFF;
    stencilWriteMask = 0xFFFFFFFF;

    twoSidedStencilEnabled = false;
    ccwStencilFail = acdlib::ACD_STENCIL_OP_KEEP;
    ccwStencilZFail = acdlib::ACD_STENCIL_OP_KEEP;
    ccwStencilPass = acdlib::ACD_STENCIL_OP_KEEP;
    ccwStencilFunc = acdlib::ACD_COMPARE_FUNCTION_ALWAYS;

    for(u32bit c = 0; c < MAX_VERTEX_SHADER_CONSTANTS; c++)
    {
        settedVertexShaderConstants[c] = new acdlib::acd_float[4];
        settedVertexShaderConstants[c][0] = 0.0f;
        settedVertexShaderConstants[c][1] = 0.0f;
        settedVertexShaderConstants[c][2] = 0.0f;
        settedVertexShaderConstants[c][3] = 0.0f;
        settedVertexShaderConstantsTouched[c] = false;
    }
    
    for(u32bit c = 0; c < 16; c++)
    {
        settedVertexShaderConstantsInt[c] = new acdlib::acd_int[4];
        settedVertexShaderConstantsInt[c][0] = 0;
        settedVertexShaderConstantsInt[c][1] = 0;
        settedVertexShaderConstantsInt[c][2] = 0;
        settedVertexShaderConstantsInt[c][3] = 0;
        settedVertexShaderConstantsIntTouched[c] = false;
        
        settedVertexShaderConstantsBool[c] = false;
        settedVertexShaderConstantsBoolTouched[c] = false;
    }

    for(u32bit c = 0; c < MAX_PIXEL_SHADER_CONSTANTS; c++)
    {
        settedPixelShaderConstants[c] = new acdlib::acd_float[4];
        settedPixelShaderConstants[c][0] = 0.0f;
        settedPixelShaderConstants[c][1] = 0.0f;
        settedPixelShaderConstants[c][2] = 0.0f;
        settedPixelShaderConstants[c][3] = 0.0f;
        settedPixelShaderConstantsTouched[c] = false;
    }

    for(u32bit c = 0; c < 16; c++)
    {
        settedPixelShaderConstantsInt[c] = new acdlib::acd_int[4];
        settedPixelShaderConstantsInt[c][0] = 0;
        settedPixelShaderConstantsInt[c][1] = 0;
        settedPixelShaderConstantsInt[c][2] = 0;
        settedPixelShaderConstantsInt[c][3] = 0;
        settedPixelShaderConstantsIntTouched[c] = false;
        
        settedPixelShaderConstantsBool[c] = false;
        settedPixelShaderConstantsBoolTouched[c] = false;
    }
    
    for (u32bit i = 0; i < MAX_RENDER_TARGETS; i++){
        colorwriteRed[i] = true;
        colorwriteGreen[i] = true;
        colorwriteBlue[i] = true;
        colorwriteAlpha[i] = true;
    }

    instancingMode = false;
    instancesLeft = 0;
    instancesDone = 0;
    instancingStride = 1;
    transfBuffer = NULL;

    for (u32bit i = 0; i < MAX_RENDER_TARGETS; i++)
        currentRenderSurface[i] = NULL;

    nativeFFVSh = NULL;
}

void AD3D9State::initialize(AIDeviceImp9 *device, UINT width, UINT height)
{

    // Create a new device
    if (acdDev == NULL)
        acdDev = acdlib::createDevice(0);

    maxStreams = acdDev->availableStreams();
    settedVertexBuffers.resize(maxStreams);

    //  Allocate the sate for the 16 pixel shader samplers.
    settedTextures.resize(maxSamplers);
    
    //  Allocate the state for the 4 vertex shader samplers.
    settedVertexTextures.resize(maxVertexSamplers);

    // Set viewport resolution
    acdDev->setResolution(width, height);
    acdDev->rast().setViewport(0, 0, width, height);
    acdDev->zStencil().setDepthRange(0.f, 1.f);

    // Set default values
    acdDev->zStencil().setZFunc(acdlib::ACD_COMPARE_FUNCTION_LESS_EQUAL);
    acdDev->rast().setFaceMode(acdlib::ACD_FACE_CW);
    acdDev->rast().setCullMode(acdlib::ACD_CULL_BACK);
    //acdDev->alphaTestEnabled(alphaTest);
    acdDev->alphaTestEnabled(true);
    acdDev->rast().useD3D9RasterizationRules(true);
    acdDev->rast().useD3D9PixelCoordConvention(true);
    acdDev->zStencil().setD3D9DepthRangeMode(true);

    for (u32bit i = 0; i < MAX_RENDER_TARGETS; i++)
        acdDev->blending().setColorMask(i, colorwriteRed[i], colorwriteGreen[i], colorwriteBlue[i], colorwriteAlpha[i]);

    for (u32bit i = 0; i < MAX_RENDER_TARGETS; i++)
        acdDev->blending().setBlendColor(i, 1.0f, 1.0f, 1.0f, 1.0f);

    redefineACDBlend();
    redefineACDStencil();

    //  Get the current/default render buffers.
    acdlib::ACDRenderTarget *currentRT = acdDev->getRenderTarget(0);
    acdlib::ACDRenderTarget *currentZStencil = acdDev->getZStencilBuffer();

    //  Create default surfaces for the default render buffers.
    defaultRenderSurface = new AISurfaceImp9(device, width, height, D3DUSAGE_RENDERTARGET, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT);
    defaultZStencilSurface = new AISurfaceImp9(device, width, height, D3DUSAGE_DEPTHSTENCIL, D3DFMT_D24S8, D3DPOOL_DEFAULT);

    defaultRenderSurface->setAsRenderTarget(currentRT);
    defaultZStencilSurface->setAsRenderTarget(currentZStencil);
    
    defaultRenderSurface->AddRef();
    defaultZStencilSurface->AddRef();

    currentRenderSurface[0] = defaultRenderSurface;
    currentZStencilSurface = defaultZStencilSurface;

    //  Associate the default render surfaces with the default render targets.
    //renderTargets[defaultRenderSurface] = currentRT;
    //mipLevelsTextureType[defaultRenderSurface] = AD3D9_RENDERTARGET;
    //renderTargets[defaultZStencilSurface] = currentZStencil;
    //mipLevelsTextureType[defaultZStencilSurface] = AD3D9_RENDERTARGET;

    fixedFunctionVertexShader = acdDev->createShaderProgram();
    fixedFunctionPixelShader = acdDev->createShaderProgram();
}

void AD3D9State::destroy() {

    // Destroy the current device
    acdlib::destroyDevice(acdDev);
    acdDev = NULL;

}

void AD3D9State::setD3DTrace(D3DTrace *trace)
{
    d3dTrace = trace;
}

bool AD3D9State::isPreloadCall()
{
    return d3dTrace->isPreloadCall();
}

/*acdlib::ACDDevice* AD3D9State::getACDDevice() {

    return acdDev;

}*/

AISurfaceImp9* AD3D9State::getDefaultRenderSurface()
{
    return defaultRenderSurface;
}



acdlib::ACDTexture2D* AD3D9State::createTexture2D() {

    return acdDev->createTexture2D();

}

acdlib::ACDTextureCubeMap* AD3D9State::createCubeMap() {

    return acdDev->createTextureCubeMap();

}

acdlib::ACDTexture3D* AD3D9State::createVolumeTexture() {

    return acdDev->createTexture3D();

}

acdlib::ACDBuffer* AD3D9State::createBuffer(UINT Length) {

    return acdDev->createBuffer(Length);

}

acdlib::ACDShaderProgram* AD3D9State::createShaderProgram() {

    return acdDev->createShaderProgram();

}

acdlib::ACDRenderTarget* AD3D9State::createRenderTarget(acdlib::ACDTexture* resource, const acdlib::ACD_RT_DIMENSION rtdimension, D3DCUBEMAP_FACES face, UINT mipmap) {

    return acdDev->createRenderTarget(resource, rtdimension, getACDCubeMapFace(face), mipmap);

}

/*void AD3D9State::addVertexShader(AIVertexShaderImp9* vs, CONST DWORD* func) {

    // Get shader lenght
    int i = 0;
    while ((func[i] & D3DSI_OPCODE_MASK) != D3DSIO_END)
        i++;

    UINT token_count = i + 1;
    UINT size = sizeof(DWORD) * token_count;

    DWORD* program = new DWORD[token_count];

    // Copy the content of the program
    memcpy(program, func, size);

    // Add the vertex shader to the known vertex shaders
    vertexShaders[vs] = program;

}*/

/*void AD3D9State::addPixelShader(AIPixelShaderImp9* ps, CONST DWORD* func) {

    // Get shader lenght
    int i = 0;
    while ((func[i] & D3DSI_OPCODE_MASK) != D3DSIO_END)
        i++;

    UINT token_count = i + 1;
    UINT size = sizeof(DWORD) * token_count;

    DWORD* program = new DWORD[token_count];

    // Copy the content of the program
    memcpy(program, func, size);

    // Add the pixel shader to the known pixel shaders
    pixelShaders[ps] = program;

}*/

/*void AD3D9State::addVertexBuffer(AIVertexBufferImp9* vb, UINT size) {

    // Create a new ACD Buffer
    acdlib::ACDBuffer* acdStreamBuffer = acdDev->createBuffer(size);

    // Add the vertex buffer to the known vertex buffers
    vertexBuffers[vb] = acdStreamBuffer;

}*/

/*void AD3D9State::updateVertexBuffer(AIVertexBufferImp9* vb, UINT offset, UINT size, BYTE* data) {

    // Update vertex buffer data
    vertexBuffers[vb]->updateData(offset, size, (acdlib::acd_ubyte*)data);

}*/

/*void AD3D9State::addVertexDeclaration(AIVertexDeclarationImp9* vd, CONST D3DVERTEXELEMENT9* elements) {

    std::vector< D3DVERTEXELEMENT9 > vElements;
    D3DVERTEXELEMENT9 end = D3DDECL_END();
    UINT numElements = 0;

    // Copy the vertex declaration to a more easy to use structure
    while(elements[numElements].Stream != end.Stream) {

        D3DVERTEXELEMENT9 tmpElem;

        tmpElem.Stream = elements[numElements].Stream;
        tmpElem.Offset = elements[numElements].Offset;
        tmpElem.Type = elements[numElements].Type;
        tmpElem.Method = elements[numElements].Method;
        tmpElem.Usage = elements[numElements].Usage;
        tmpElem.UsageIndex = elements[numElements].UsageIndex;

        vElements.push_back(tmpElem);
        numElements++;

    }

    // Add the vertex declaration to the known vertex declarations
    vertexDeclarations[vd] = vElements;

}*/

/*void AD3D9State::addIndexBuffer(AIIndexBufferImp9* ib, UINT size) {

    // Create a new ACD Buffer
    acdlib::ACDBuffer* acdIndexBuffer = acdDev->createBuffer(size);

    // Add the index buffer to the known index buffers
    indexBuffers[ib] = acdIndexBuffer;

}*/

/*void AD3D9State::updateIndexBuffer(AIIndexBufferImp9* ib, UINT offset, UINT size, BYTE* data) {

    // Update vertex buffer data
    indexBuffers[ib]->updateData(offset, size, (acdlib::acd_ubyte*)data);

}*/

/*void AD3D9State::addTexture2D(AITextureImp9* tex, DWORD usage) {

    // Create a new ACD Texture 2D
    acdlib::ACDTexture2D* acdTexture = acdDev->createTexture2D();

    // Add the texture 2D to the known textures 2D
    textures2D[tex] = acdTexture;

    //  Set layout to render buffer.
    if ((usage & D3DUSAGE_DEPTHSTENCIL) || (usage & D3DUSAGE_RENDERTARGET))
    {
        acdTexture->setMemoryLayout(acdlib::ACD_LAYOUT_RENDERBUFFER);
    }

    textureTextureType[(AIBaseTextureImp9*)tex] = AD3D9_TEXTURE2D;

}*/

/*void AD3D9State::addCubeTexture(AICubeTextureImp9* tex) {

    // Create a new ACD Texture Cube Map
    acdlib::ACDTextureCubeMap* acdTexture = acdDev->createTextureCubeMap();

    // Add the cube texture to the known cube textures
    cubeTextures[tex] = acdTexture;

    textureTextureType[(AIBaseTextureImp9*)tex] = AD3D9_CUBETEXTURE;

}*/

/*void AD3D9State::addVolumeTexture(AIVolumeTextureImp9* tex) {

    // Create a new ACD Volume Texture
    acdlib::ACDTexture3D* acdTexture = acdDev->createTexture3D();

    // Add the volume texture to the known cube textures
    volumeTextures[tex] = acdTexture;

    textureTextureType[(AIBaseTextureImp9*)tex] = AD3D9_VOLUMETEXTURE;

}*/

/*void AD3D9State::addMipLevel(AITextureImp9* tex, AISurfaceImp9* mip, UINT mipLevel, UINT width, UINT height, D3DFORMAT format) {

    // Reference to the mip level texture
    mipLevelsTextures2D[mip] = tex;

    // Reference to the mip level number
    mipLevelsLevels[mip] = mipLevel;

    mipLevelsTextureType[mip] = AD3D9_TEXTURE2D;

    // Get mip level size
    UINT size = getSurfaceSize(width, height, format);

    //BYTE* data = new BYTE[size];

    // If isn't a compressed texture, the size passed to ACD must be 0
    if (!is_compressed(format))
        size = 0;

    // Set a void buffer to create the new mipmap in ACD
    //textures2D[tex]->setData(mipLevel, width, height, getACDFormat(format), 0, (acdlib::acd_ubyte*)data, size);
    textures2D[tex]->setData(mipLevel, width, height, getACDFormat(format), 0, NULL, size);

    //delete[] data;
}*/

/*void AD3D9State::addCubeTextureMipLevel(AICubeTextureImp9* tex, AISurfaceImp9* mip, D3DCUBEMAP_FACES face, UINT mipLevel, UINT width, D3DFORMAT format) {

    // Reference to the mip level texture
    mipLevelsCubeTextures[mip] = tex;

    // Reference to the mip level number
    mipLevelsLevels[mip] = mipLevel;

    // Reference to the face
    mipLevelsFaces[mip] = getACDCubeMapFace(face);

    mipLevelsTextureType[mip] = AD3D9_CUBETEXTURE;

    // Get mip level size
    UINT size = getSurfaceSize(width, width, format);

    //BYTE* data = new BYTE[size];

    // If isn't a compressed texture, the size passed to ACD must be 0
    if (!is_compressed(format)) size = 0;

    // Set a void buffer to create the new mipmap in ACD
    //cubeTextures[tex]->setData(getACDCubeMapFace(face), mipLevel, width, width, getACDFormat(format), 0, (acdlib::acd_ubyte*)data, size);
    cubeTextures[tex]->setData(getACDCubeMapFace(face), mipLevel, width, width, getACDFormat(format), 0, NULL, size);

    //delete[] data;
}*/

/*void AD3D9State::addVolumeLevel(AIVolumeTextureImp9* tex, AIVolumeImp9* vol, UINT mipLevel, UINT width, UINT height, UINT depth, D3DFORMAT format) {

    mipVolumesVolumeTextures[vol] = tex;

    mipVolumesLevels[vol] = mipLevel;

    UINT size = getVolumeSize(width, height, depth, format);

    BYTE* data = new BYTE[size];

    // If isn't a compressed texture, the size passed to ACD must be 0
    if (!is_compressed(format)) size = 0;

    volumeTextures[tex]->setData(mipLevel, width, height, depth, getACDFormat(format), 0, (acdlib::acd_ubyte*)data, size);

    delete[] data;

}*/

/*void AD3D9State::updateSurface(AISurfaceImp9* surf, CONST RECT* rect, D3DFORMAT format, BYTE* data) {

    switch (mipLevelsTextureType[surf])
    {

        case AD3D9_TEXTURE2D:
            // Update the mipmap that corresponts to the passed surface with the passed data
            textures2D[mipLevelsTextures2D[surf]]->updateData(mipLevelsLevels[surf], rect->left, rect->top, (rect->right - rect->left), (rect->bottom - rect->top), getACDFormat(format), getSurfacePitchACD((rect->right - rect->left), format), (acdlib::acd_ubyte*)data);
            break;

        case AD3D9_CUBETEXTURE:
            cubeTextures[mipLevelsCubeTextures[surf]]->updateData(mipLevelsFaces[surf], mipLevelsLevels[surf], rect->left, rect->top, (rect->right - rect->left), (rect->bottom - rect->top), getACDFormat(format), getSurfacePitchACD((rect->right - rect->left), format), (acdlib::acd_ubyte*)data);
            break;

        case AD3D9_RENDERTARGET:
            //renderTargetSurface[surf]->updateData(0, rect->left, rect->top, (rect->right - rect->left), (rect->bottom - rect->top), getACDFormat(format), getSurfacePitchACD((rect->right - rect->left), format), (acdlib::acd_ubyte*)data);
            break;

        default:
            panic("AD3D9State", "updateSurface", "Unsupported texture type");
            break;
    }

}*/

/*void AD3D9State::updateVolume(AIVolumeImp9* vol, CONST D3DBOX* pBox, D3DFORMAT format, BYTE* data) {

    volumeTextures[mipVolumesVolumeTextures[vol]]->updateData(mipVolumesLevels[vol], pBox->Left, pBox->Top, pBox->Front, (pBox->Right - pBox->Left), (pBox->Bottom - pBox->Top), (pBox->Back - pBox->Front), getACDFormat(format), getSurfacePitchACD((pBox->Right - pBox->Left), format), (acdlib::acd_ubyte*)data);

}*/

/*const acdlib::acd_ubyte* AD3D9State::getDataSurface(AISurfaceImp9* surf) {

    switch (mipLevelsTextureType[surf])
    {

        case AD3D9_TEXTURE2D:
            //return textures2D[mipLevelsTextures2D[surf]]->getData(mipLevelsLevels[surf]);
            break;

        case AD3D9_CUBETEXTURE:
            //return cubeTextures[mipLevelsCubeTextures[surf]]->getData(mipLevelsFaces[surf], mipLevelsLevels[surf]);
            break;

        case AD3D9_RENDERTARGET:
            //return renderTargetSurface[surf]->getData(0);
            break;

        default:
            panic("AD3D9State", "updateSurface", "Unsupported texture type");
            break;
    }
return NULL;
}*/

/*void AD3D9State::copySurface(AISurfaceImp9* srcSurf , CONST RECT* srcRect , AISurfaceImp9* destSurf , CONST POINT* destPoint) {

    acdlib::acd_uint inX;
    acdlib::acd_uint inY;
    acdlib::acd_uint inWidth;
    acdlib::acd_uint inHeight;
    acdlib::acd_uint outX;
    acdlib::acd_uint outY;

    if (mipLevelsTextureType[destSurf] == AD3D9_RENDERTARGET) return;

    if (destPoint) {

        outX = destPoint->x;
        outY = destPoint->y;

    }
    else {

        outX = 0;
        outY = 0;

    }

    if (srcRect) {

        inX = srcRect->left;
        inY = srcRect->top;

        inWidth = srcRect->right - srcRect->left;
        inHeight = srcRect->bottom - srcRect->top;

    }
    else {

        inX = 0;
        inY = 0;

        D3DSURFACE_DESC srcDesc;
        srcSurf->GetDesc(&srcDesc);

        inWidth = srcDesc.Width;
        inHeight = srcDesc.Height;

    }

    switch (mipLevelsTextureType[srcSurf])
    {
        case AD3D9_TEXTURE2D:
            acdDev->copyMipmap(textures2D[mipLevelsTextures2D[srcSurf]],
                                (acdlib::ACD_CUBEMAP_FACE)0,
                                mipLevelsLevels[srcSurf],
                                inX,
                                inY,
                                inWidth,
                                inHeight,
                                textures2D[mipLevelsTextures2D[destSurf]],
                                (acdlib::ACD_CUBEMAP_FACE)0,
                                mipLevelsLevels[destSurf],
                                outX,
                                outY);

            break;

        case AD3D9_CUBETEXTURE:
            acdDev->copyMipmap(cubeTextures[mipLevelsCubeTextures[srcSurf]],
                                mipLevelsFaces[srcSurf],
                                mipLevelsLevels[srcSurf],
                                inX,
                                inY,
                                inWidth,
                                inHeight,
                                cubeTextures[mipLevelsCubeTextures[destSurf]],
                                mipLevelsFaces[srcSurf],
                                mipLevelsLevels[destSurf],
                                outX,
                                outY);
            break;

        default:
            panic("AD3D9State", "copySurface", "Unsupported texture type");
            break;
    }

}*/

void AD3D9State::addTypeTexture(AIBaseTextureImp9* tex, TextureType type) {

    textureTextureType[tex] = type;

}

void AD3D9State::copySurface(AISurfaceImp9* srcSurf , CONST RECT* srcRect , AISurfaceImp9* destSurf , CONST RECT* destRect , D3DTEXTUREFILTERTYPE Filter) {

    // Temporal implementation. Ignoring rects an updating full surface...


    AISurfaceImp9* ppRenderTarget;
    AISurfaceImp9* ppZStencilSurface;

    D3DSURFACE_DESC srcDesc;
    D3DSURFACE_DESC destDesc;

    srcSurf->GetDesc(&srcDesc);
    destSurf->GetDesc(&destDesc);

    if ((srcDesc.Format == D3DFMT_NULL) || (destDesc.Format == D3DFMT_NULL))
        return;

    if (((is_compressed(srcDesc.Format)) && (srcSurf->getAD3D9Type() == AD3D9_TEXTURE2D)) || 
         (is_compressed((destDesc.Format)) && (destSurf->getAD3D9Type() == AD3D9_TEXTURE2D)))
    {
        //  Get the pointer to the ACD Texture associated with the source surface.
        acdlib::ACDTexture2D *destTex2D = destSurf->getACDTexture2D();
        acdlib::ACDTexture2D *srcTex2D = srcSurf->getACDTexture2D();

        srcTex2D->copyData(srcSurf->getMipMapLevel(), destSurf->getMipMapLevel(), destTex2D);

        return;
    }
    
    /*if (!((srcRect->top == 0) && (srcRect->bottom == srcDesc.Height) && (srcRect->left == 0) && (srcRect->right == srcDesc.Width) &&
        (destRect->top == 0) && (destRect->bottom == destDesc.Height) && (destRect->left == 0) && (destRect->right == destDesc.Width))) {
            panic("AD3D9State", "copySurface", "Partial copy not supported.");
    }*/

    //  Check easy case copying from texture surface to render target surface.
    if ((srcSurf->getAD3D9Type() == AD3D9_TEXTURE2D) && (destSurf->getAD3D9Type() == AD3D9_RENDERTARGET) &&
        (srcRect->top == 0) && (srcRect->bottom == srcDesc.Height) && (srcRect->left == 0) && (srcRect->right == srcDesc.Width) &&
        (destRect->top == 0) && (destRect->bottom == destDesc.Height) && (destRect->left == 0) && (destRect->right == destDesc.Width))
    {
        //  Get the pointer to the ACD Texture associated with the source surface.
        acdlib::ACDTexture2D *sourceTex2D = srcSurf->getACDTexture2D();

        //  Get the mip level associated with the source surface.
        u32bit sourceMipLevel = srcSurf->getMipMapLevel();

        //  Get the pointer to the ACD Render Target associated with the destination surface.
        acdlib::ACDRenderTarget *destinationRT = destSurf->getACDRenderTarget();


        //  Copy the surface data to the render buffer (same dimensions and format).
        acdDev->copySurfaceDataToRenderBuffer(sourceTex2D, sourceMipLevel, destinationRT, isPreloadCall());

        return;
    }
    else if ((srcSurf->getAD3D9Type() == AD3D9_TEXTURE2D) && (destSurf->getAD3D9Type() == AD3D9_TEXTURE2D) &&
        (srcRect->top == 0) && (srcRect->bottom == srcDesc.Height) && (srcRect->left == 0) && (srcRect->right == srcDesc.Width) &&
        (destRect->top == 0) && (destRect->bottom == destDesc.Height) && (destRect->left == 0) && (destRect->right == destDesc.Width))
    {
        //  Get the pointer to the ACD Texture associated with the destination surface.
        acdlib::ACDTexture2D *destTex2D = destSurf->getACDTexture2D();

        //  Check if the destination surface uses the render buffer layout.
        if (destTex2D->getMemoryLayout() == acdlib::ACD_LAYOUT_RENDERBUFFER)
        {
            //  Get the mip level associated with the destination surface.
            //u32bit destMipLevel = mipLevelsLevels[destSurf];

            //  Get the pointer to the ACD Texture associated with the source surface.
            acdlib::ACDTexture2D *sourceTex2D = srcSurf->getACDTexture2D();

            //  Get the mip level associated with the source surface.
            u32bit sourceMipLevel = srcSurf->getMipMapLevel();

            /*//  Create a render target for the destination surface.
            acdlib::ACD_RT_DESC rtDesc;
            rtDesc.format = destTex2D->getFormat(destMipLevel);
            rtDesc.dimension = acdlib::ACD_RT_DIMENSION_TEXTURE2D;
            rtDesc.texture2D.mipmap = destMipLevel;
            acdlib::ACDRenderTarget *destinationRT = acdDev->createRenderTarget(destTex2D, acdlib::ACD_RT_DIMENSION_TEXTURE2D,
                                                                                 static_cast<acdlib::ACD_CUBEMAP_FACE>(0), destMipLevel);

            //  Add the new render target.
            renderTargets[destSurf] = destinationRT;*/

            acdlib::ACDRenderTarget *destinationRT = destSurf->getACDRenderTarget();
			
            //  Copy the surface data to the render buffer (same dimensions and format).
            acdDev->copySurfaceDataToRenderBuffer(sourceTex2D, sourceMipLevel, destinationRT, isPreloadCall());

            //  Copy the surface data to the render buffer (same dimensions and format).
            //acdDev->copySurfaceDataToRenderSurface(sourceTex2D, sourceMipLevel, destTex2D, destMipLevel);

            return;
        }

    }

    //  Evaluate if the source texture is a render target.
    bool sourceIsRenderTarget = (srcSurf->getAD3D9Type() == AD3D9_RENDERTARGET);
    
    //  Evaluate if the source texture is a compressed render target.
    bool sourceIsCompressedRenderTarget = false;
    if (sourceIsRenderTarget)
        sourceIsCompressedRenderTarget = (srcSurf->getACDRenderTarget()->allowsCompression());
    
    //  Check for compressed render targets.  They need to be blitted and uncompressed to a normal texture.
    if (sourceIsCompressedRenderTarget)
    {    
        //cout << " -- render target." << endl;
        //cout << "      -- render target address: " << hex << renderTargets[srcSurf] << endl;
        //cout << "      -- render target texture address: " << hex << renderTargets[srcSurf]->getTexture() << dec << endl;

        //acdDev->sampler(0).setTexture(renderTargetSurface[srcSurf]);
        //acdDev->sampler(0).setTexture(renderTargets[srcSurf]->getTexture());

        /***************************
        Blit
        ***************************/
        
        // Set source surface as a render target
        // Get the current render target
        ppRenderTarget = getRenderTarget(0);
        // Get the current depth stencil
        ppZStencilSurface = getZStencilBuffer();

        setRenderTarget(0, srcSurf);
        setZStencilBuffer(NULL);

        /*if (blitTex2D != NULL)
            acdDev->destroy(blitTex2D);*/

        blitTex2D = acdDev->createTexture2D();
        //blitTex2D->setData(0,destDesc.Width, destDesc.Height, getACDFormat(destDesc.Format), 0, NULL, 0);
    
        acdlib::ACD_FORMAT destFormat = getACDFormat(destDesc.Format);

        //  Conversion needed because for render buffers the inverted formats are not
        //  implemented and are just aliased with the non inverted formats.
        switch(destFormat)
        {
            case acdlib::ACD_FORMAT_XRGB_8888:
            case acdlib::ACD_FORMAT_ARGB_8888:
                destFormat = acdlib::ACD_FORMAT_RGBA_8888;
                break;

            default:
                //  Keep the format.
                break;
        }
        
        blitTex2D->setData(0,srcDesc.Width, srcDesc.Height, destFormat, 0, NULL, 0);

        //acdDev->sampler(0).setTexture(textures2D[mipLevelsTextures2D[destSurf]]);
        acdDev->sampler(0).setTexture(/*destSurf->getACDTexture2D()*/ blitTex2D);
        //acdDev->sampler(0).performBlitOperation2D(0, 0, 0, 0, srcDesc.Width, srcDesc.Height, destDesc.Width, getACDFormat(srcDesc.Format), /*textures2D[mipLevelsTextures2D[destSurf]]*//*destSurf->getACDTexture2D()*/blitTex2D, 0);
        acdDev->performBlitOperation2D(0, 0, 0, 0, 0, srcDesc.Width, srcDesc.Height, srcDesc.Width, getACDFormat(srcDesc.Format), /*textures2D[mipLevelsTextures2D[destSurf]]*//*destSurf->getACDTexture2D()*/blitTex2D, 0);
        
        setRenderTarget(0, ppRenderTarget);
        setZStencilBuffer(ppZStencilSurface);

    }
    //else if (/*mipLevelsTextureType[srcSurf]*/srcSurf->getAD3D9Type() == AD3D9_TEXTURE2D) {

    acdlib::acd_float xoffset = (1.f / (acdlib::acd_float)srcDesc.Width) / 4.f;
    acdlib::acd_float yoffset = (1.f / (acdlib::acd_float)srcDesc.Height) / 4.f;

    acdlib::acd_float srcX1 = (acdlib::acd_float)srcRect->left / (acdlib::acd_float)srcDesc.Width;
    acdlib::acd_float srcY1 = (acdlib::acd_float)srcRect->top / (acdlib::acd_float)srcDesc.Height;
    acdlib::acd_float srcX2 = (acdlib::acd_float)srcRect->right / (acdlib::acd_float)srcDesc.Width;
    acdlib::acd_float srcY2 = (acdlib::acd_float)srcRect->bottom / (acdlib::acd_float)srcDesc.Height;

    acdlib::acd_float dstX1 = ((acdlib::acd_float)destRect->left / (acdlib::acd_float)destDesc.Width) * 2.f - 1.f;
    acdlib::acd_float dstY1 = -((acdlib::acd_float)destRect->top / (acdlib::acd_float)destDesc.Height) * 2.f + 1.f;
    acdlib::acd_float dstX2 = ((acdlib::acd_float)destRect->right / (acdlib::acd_float)destDesc.Width) * 2.f - 1.f;
    acdlib::acd_float dstY2 = -((acdlib::acd_float)destRect->bottom / (acdlib::acd_float)destDesc.Height) * 2.f + 1.f;

    /*acdlib::acd_float dataBuffer[] = {-1.f, -1.f, 0.f, 0.f + xoffset, 1.f + yoffset,
                                    1.f, -1.f, 0.f, 1.f + xoffset, 1.f + yoffset,
                                    1.f, 1.f, 0.f, 1.f + xoffset, 0.f + yoffset,
                                    -1.f, 1.f, 0.f, 0.f + xoffset, 0.f + yoffset};*/
    
    acdlib::acd_float dataBuffer[] = {dstX1, dstY2, 0.f, srcX1 + xoffset, srcY2 + yoffset,
                                    dstX2, dstY2, 0.f, srcX2 + xoffset, srcY2 + yoffset,
                                    dstX2, dstY1, 0.f, srcX2 + xoffset, srcY1 + yoffset,
                                    dstX1, dstY1, 0.f, srcX1 + xoffset, srcY1 + yoffset};

    acdlib::acd_uint indexBuffer[] = {0, 1, 2, 2, 3, 0};
    //acdlib::acd_uint indexBuffer[] = {0, 2, 1, 2, 0, 3};

    acdlib::acd_ubyte vertexShader[] = "mov o0, i0\n"
                                       "mov o5, i1\n";
                                       
    acdlib::acd_ubyte fragmentShader[] = "tex r0, i5, t0\n"
                                         "mov o1, r0\n";
                                         
    acdlib::ACDBuffer* dataACDBuffer = acdDev->createBuffer((sizeof (acdlib::acd_float)) * 5 * 4, (acdlib::acd_ubyte *)&dataBuffer);

    //cout << " -- data buffer created: " << hex << dataACDBuffer << endl;

    acdlib::ACD_STREAM_DESC streamDescription;
    streamDescription.offset = 0;
    streamDescription.stride = (sizeof (acdlib::acd_float)) * 5;
    streamDescription.components = 3;
    streamDescription.frequency = 0;
    streamDescription.type = acdlib::ACD_SD_FLOAT32;

    //cout << " -- streamDescription 0 setted: " << endl;
    //cout << "     -- offset: " << dec << streamDescription.offset << endl;
    //cout << "     -- stride: " << dec << streamDescription.stride << endl;
    //cout << "     -- components: " << dec << streamDescription.components << endl;
    //cout << "     -- frequency: " << dec << streamDescription.frequency << endl;
    //cout << "     -- type: " << dec << streamDescription.type << endl;

    // Set the vertex buffer to ACD
    acdDev->stream(0).set(dataACDBuffer, streamDescription);
    acdDev->enableVertexAttribute(0, 0);

    //cout << " -- data buffer 0 setted and enabled." << endl;

    streamDescription.offset = (sizeof (acdlib::acd_float)) * 3;
    streamDescription.stride = (sizeof (acdlib::acd_float)) * 5;
    streamDescription.components = 2;
    streamDescription.frequency = 0;
    streamDescription.type = acdlib::ACD_SD_FLOAT32;

    //cout << " -- streamDescription 1 setted: " << endl;
    //cout << "     -- offset: " << dec << streamDescription.offset << endl;
    //cout << "     -- stride: " << dec << streamDescription.stride << endl;
    //cout << "     -- components: " << dec << streamDescription.components << endl;
    //cout << "     -- frequency: " << dec << streamDescription.frequency << endl;
    //cout << "     -- type: " << dec << streamDescription.type << endl;

    // Set the vertex buffer to ACD
    acdDev->stream(1).set(dataACDBuffer, streamDescription);
    acdDev->enableVertexAttribute(1, 1);

    //cout << " -- data buffer 1 setted and enabled." << endl;

    acdlib::ACDBuffer* indexACDBuffer = acdDev->createBuffer((sizeof (acdlib::acd_uint)) * 6, (acdlib::acd_ubyte *)&indexBuffer);

    //cout << " -- index buffer created: " << hex << indexACDBuffer << endl;

    acdDev->setIndexBuffer(indexACDBuffer, 0, acdlib::ACD_SD_UINT32);

    //cout << " -- index buffer setted." << endl;

    // Set source surface as a Texture
    acdDev->sampler(0).setEnabled(true);

    //cout << " -- sampler 0 enabled." << endl;
        //cout << " -- texture 2d." << dec << endl;

    if (sourceIsCompressedRenderTarget)
        acdDev->sampler(0).setTexture(blitTex2D);
    else if (sourceIsRenderTarget)
        acdDev->sampler(0).setTexture(srcSurf->getACDRenderTarget()->getTexture());
    else
        acdDev->sampler(0).setTexture(/*textures2D[mipLevelsTextures2D[srcSurf]]*/ srcSurf->getACDTexture2D());

                        //cout << " - Format: " << textures2D[mipLevelsTextures2D[srcSurf]]->getFormat(0) << endl;
                        //cout << " - Width: " << textures2D[mipLevelsTextures2D[srcSurf]]->getWidth(0) << endl;
                        //cout << " - Height: " << textures2D[mipLevelsTextures2D[srcSurf]]->getHeight(0) << endl;
                        //cout << " - MemoryLayout: " << textures2D[mipLevelsTextures2D[srcSurf]]->getMemoryLayout() << endl;
                        //cout << " - SettedMipmaps: " << textures2D[mipLevelsTextures2D[srcSurf]]->getSettedMipmaps() << endl;



    acdDev->sampler(0).setMagFilter(getACDTextureMagFilter(Filter));
    acdDev->sampler(0).setMinFilter(acdlib::ACD_TEXTURE_FILTER_NEAREST);
    acdDev->sampler(0).setMaxAnisotropy(1);

    acdDev->sampler(0).setTextureAddressMode(acdlib::ACD_TEXTURE_S_COORD, acdlib::ACD_TEXTURE_ADDR_CLAMP_TO_EDGE);
    acdDev->sampler(0).setTextureAddressMode(acdlib::ACD_TEXTURE_T_COORD, acdlib::ACD_TEXTURE_ADDR_CLAMP_TO_EDGE);
    acdDev->sampler(0).setTextureAddressMode(acdlib::ACD_TEXTURE_R_COORD, acdlib::ACD_TEXTURE_ADDR_CLAMP_TO_EDGE);

    acdlib::ACDShaderProgram* ACDVertexShader = acdDev->createShaderProgram();
    acdlib::ACDShaderProgram* ACDFragmentShader = acdDev->createShaderProgram();

    ACDVertexShader->setProgram(vertexShader);
    ACDFragmentShader->setProgram(fragmentShader);

    acdDev->setVertexShader(ACDVertexShader);
    acdDev->setFragmentShader(ACDFragmentShader);

    // Set dest surface as a render target
    // Get the current render target
    ppRenderTarget = getRenderTarget(0);
    // Get the current depth stencil
    ppZStencilSurface = getZStencilBuffer();

    setRenderTarget(0, destSurf);
    setZStencilBuffer(NULL);

    //acdDev->clearColorBuffer(0, 0, 0, 0);
    acdDev->rast().setCullMode(acdlib::ACD_CULL_NONE);
    acdDev->zStencil().setStencilEnabled(false);
    acdDev->blending().setColorMask(0, true, true, true, true);
    acdDev->blending().setEnable(0, false);


    // Draw
    acdDev->setPrimitive(acdlib::ACD_TRIANGLES);

    acdDev->drawIndexed(0, 6, 0, 0, 0);

    acdDev->disableVertexAttributes();

    // Swap buffers
    //acdDev->swapBuffers();

    //acdDev->destroy(dataACDBuffer);
    //acdDev->destroy(indexACDBuffer);

    //acdDev->destroy(ACDVertexShader);
    //acdDev->destroy(ACDFragmentShader);

    setRenderTarget(0, ppRenderTarget);
    setZStencilBuffer(ppZStencilSurface);

    // TODO: restore cull mode...
    redefineACDStencil();    
    acdDev->blending().setColorMask(0, colorwriteRed[0], colorwriteGreen[0], colorwriteBlue[0], colorwriteAlpha[0]);
    acdDev->blending().setEnable(0, alphaBlend);

    batchCount++;

    /*}
    else {
        panic("AD3D9State", "copySurface", "Not a texture or a render target.");
    }*/

}

/*void AD3D9State::copySurface(AISurfaceImp9* srcSurf , CONST RECT* srcRect , AISurfaceImp9* destSurf , CONST RECT* destRect , D3DTEXTUREFILTERTYPE Filter) {

    if (mipLevelsTextureType[srcSurf] == AD3D9_RENDERTARGET) {

        if (mipLevelsTextureType[destSurf] == AD3D9_RENDERTARGET) {

            acdDev->copyMipmap(renderTargets[srcSurf]->getTexture(), (acdlib::ACD_CUBEMAP_FACE)0, 0,
                srcRect->left, srcRect->top, srcRect->right - srcRect->left, srcRect->bottom - srcRect->top,
                renderTargets[destSurf]->getTexture(), (acdlib::ACD_CUBEMAP_FACE)0, 0,
                destRect->left, destRect->top, destRect->right - destRect->left, destRect->bottom - destRect->top,
                getACDTextureMagFilter(Filter));

        }
        else if (mipLevelsTextureType[destSurf] == AD3D9_TEXTURE2D) {

            acdDev->copyMipmap(renderTargets[srcSurf]->getTexture(), (acdlib::ACD_CUBEMAP_FACE)0, 0,
                srcRect->left, srcRect->top, srcRect->right - srcRect->left, srcRect->bottom - srcRect->top,
                textures2D[mipLevelsTextures2D[destSurf]], (acdlib::ACD_CUBEMAP_FACE)0, mipLevelsLevels[destSurf],
                destRect->left, destRect->top, destRect->right - destRect->left, destRect->bottom - destRect->top,
                getACDTextureMagFilter(Filter));

        }

    }
    else if (mipLevelsTextureType[srcSurf] == AD3D9_TEXTURE2D) {

        if (mipLevelsTextureType[destSurf] == AD3D9_RENDERTARGET) {

            acdDev->copyMipmap(textures2D[mipLevelsTextures2D[srcSurf]], (acdlib::ACD_CUBEMAP_FACE)0, mipLevelsLevels[srcSurf],
                srcRect->left, srcRect->top, srcRect->right - srcRect->left, srcRect->bottom - srcRect->top,
                renderTargets[destSurf]->getTexture(), (acdlib::ACD_CUBEMAP_FACE)0, 0,
                destRect->left, destRect->top, destRect->right - destRect->left, destRect->bottom - destRect->top,
                getACDTextureMagFilter(Filter));

        }
        else if (mipLevelsTextureType[destSurf] == AD3D9_TEXTURE2D) {

            acdDev->copyMipmap(textures2D[mipLevelsTextures2D[srcSurf]], (acdlib::ACD_CUBEMAP_FACE)0, mipLevelsLevels[srcSurf],
                srcRect->left, srcRect->top, srcRect->right - srcRect->left, srcRect->bottom - srcRect->top,
                textures2D[mipLevelsTextures2D[destSurf]], (acdlib::ACD_CUBEMAP_FACE)0, mipLevelsLevels[destSurf],
                destRect->left, destRect->top, destRect->right - destRect->left, destRect->bottom - destRect->top,
                getACDTextureMagFilter(Filter));

        }

    }

}*/

void AD3D9State::setVertexShader(AIVertexShaderImp9* vs) {

    // Set the vertex shader to use
    settedVertexShader = vs;

}

void AD3D9State::setPixelShader(AIPixelShaderImp9* ps) {

    // Set the pixel shader to use
    settedPixelShader = ps;

}

void AD3D9State::setVertexShaderConstant(UINT first, CONST float* data, UINT vectorCount)
{
    // For each constant
    for(UINT c = 0; c < vectorCount; c++)
    {
        if ((first + c) >= MAX_VERTEX_SHADER_CONSTANTS)
        {
            printf("WARNING!!! AD3D9State::setVertexShaderConstants => Vertex shader constant out of range.  First = %d Count = %d\n",
                first, vectorCount);
            break;
            //panic("AD3D9State", "setVertexShaderConstants", "Vertex shader constant index out of range.");
        }

        // Add the new constant setted to the setted constants map
        settedVertexShaderConstants[first + c][0] = data[c * 4 + 0];
        settedVertexShaderConstants[first + c][1] = data[c * 4 + 1];
        settedVertexShaderConstants[first + c][2] = data[c * 4 + 2];
        settedVertexShaderConstants[first + c][3] = data[c * 4 + 3];

        settedVertexShaderConstantsTouched[first + c] = true;
    }
}

void AD3D9State::setVertexShaderConstant(UINT first, CONST int* data, UINT vectorCount)
{
    // For each constant
    for(UINT c = 0; c < vectorCount; c++)
    {
        // Add the new constant setted to the setted constants map
        settedVertexShaderConstantsInt[first + c][0] = data[c * 4 + 0];
        settedVertexShaderConstantsInt[first + c][1] = data[c * 4 + 1];
        settedVertexShaderConstantsInt[first + c][2] = data[c * 4 + 2];
        settedVertexShaderConstantsInt[first + c][3] = data[c * 4 + 3];

        settedVertexShaderConstantsIntTouched[first + c] = true;
    }
}

void AD3D9State::setVertexShaderConstantBool(UINT first, CONST BOOL* data, UINT vectorCount)
{
    // For each constant
    for(UINT c = 0; c < vectorCount; c++)
    {
        // Add the new constant setted to the setted constants map
        settedVertexShaderConstantsBool[first + c] = data[c];
        settedVertexShaderConstantsBoolTouched[first + c] = true;
    }
}


void AD3D9State::setPixelShaderConstant(UINT first, CONST float* data, UINT vectorCount)
{
    // For each constant
    for(UINT c = 0; c < vectorCount; c++)
    {
        if ((first + c) >= MAX_PIXEL_SHADER_CONSTANTS)
        {
            printf("WARNING!!! AD3D9State::setPixelShaderConstants => Pixel shader constant out of range.  First = %d Count = %d\n",
                first, vectorCount);
            break;
            //panic("AD3D9State", "setPixelShaderConstants", "Pixel shader constant index out of range.");
        }

        // Add the new constant setted to the setted constants map
        settedPixelShaderConstants[first + c][0] = data[c * 4 + 0];
        settedPixelShaderConstants[first + c][1] = data[c * 4 + 1];
        settedPixelShaderConstants[first + c][2] = data[c * 4 + 2];
        settedPixelShaderConstants[first + c][3] = data[c * 4 + 3];

        settedPixelShaderConstantsTouched[first + c] = true;
    }
}

void AD3D9State::setPixelShaderConstant(UINT first, CONST int* data, UINT vectorCount)
{
    // For each constant
    for(UINT c = 0; c < vectorCount; c++)
    {
        // Add the new constant setted to the setted constants map
        settedPixelShaderConstantsInt[first + c][0] = data[c * 4 + 0];
        settedPixelShaderConstantsInt[first + c][1] = data[c * 4 + 1];
        settedPixelShaderConstantsInt[first + c][2] = data[c * 4 + 2];
        settedPixelShaderConstantsInt[first + c][3] = data[c * 4 + 3];

        settedPixelShaderConstantsIntTouched[first + c] = true;
    }
}

void AD3D9State::setPixelShaderConstantBool(UINT first, CONST BOOL* data, UINT vectorCount)
{
    // For each constant
    for(UINT c = 0; c < vectorCount; c++)
    {
        // Add the new constant setted to the setted constants map
        settedPixelShaderConstantsBool[first + c] = data[c];
        settedPixelShaderConstantsBoolTouched[first + c] = true;
    }
}


void AD3D9State::setVertexBuffer(AIVertexBufferImp9* vb, UINT str, UINT offset, UINT stride) 
{

    // Set the vertex buffer to the indicated stream
    if (str < maxStreams) 
    {
        settedVertexBuffers[str].streamData = vb;
        settedVertexBuffers[str].offset = offset;
        settedVertexBuffers[str].stride = stride;
    }
    else
        panic("AD3D9State","getVertexBuffer","Selected stream is bigger than the maximum number of streams");

}

AIVertexBufferImp9* AD3D9State::getVertexBuffer(UINT str, UINT& offset, UINT& stride) 
{

    // Set the vertex buffer to the indicated stream
    if (str < maxStreams) 
    {
        return settedVertexBuffers[str].streamData;
    }
    else
        panic("AD3D9State","getVertexBuffer","Selected stream is bigger than the maximum number of streams");

    return 0;

}

void AD3D9State::setVertexDeclaration(AIVertexDeclarationImp9* vd) {

    // Set the vertex declaration to use
    settedVertexDeclaration = vd;
        
    //  Set the vertex declaration for the fixed function generator.    
    fixedFunctionState.vertexDeclaration.clear();
    for(u32bit d = 0; d < vd->getVertexElements().size(); d++)
        fixedFunctionState.vertexDeclaration.push_back(vd->getVertexElements()[d]);
}

AIVertexDeclarationImp9* AD3D9State::getVertexDeclaration()
{
    return settedVertexDeclaration;
}

void AD3D9State::setFVF(DWORD FVF)
{
    // Set the fixed function vertex declaration
    settedFVF = FVF;

    //  Set the fixed function vertex declaration for the fixed function generator.
    fixedFunctionState.fvf = FVF;
}

void AD3D9State::setIndexBuffer(AIIndexBufferImp9* ib) {

    // Set the index buffer to use
    settedIndexBuffer = ib;

}

AIIndexBufferImp9* AD3D9State::getIndexBuffer ()
{
    return settedIndexBuffer;
}

void AD3D9State::setTexture(AIBaseTextureImp9* tex, UINT samp)
{
    // Set the texture to the indicated sampler
    if (samp < maxSamplers)
    {
        settedTextures[samp].samplerData = tex;

        //  Update fixed function state.  Only the first 8 samplers affect the fixed function state.
        if (samp < 8)
        {
            //  Set if a texture is defined for the sampler.
            fixedFunctionState.settedTexture[samp] = (tex != NULL);
            
            if (tex != NULL)
            {
                //  Set the texture type.
                switch(textureTextureType[settedTextures[samp].samplerData])
                {
                    case AD3D9_TEXTURE2D:
                    case AD3D9_RENDERTARGET:
                        fixedFunctionState.textureType[samp] = D3DSTT_2D;
                        break;
                    case AD3D9_CUBETEXTURE:
                        fixedFunctionState.textureType[samp] = D3DSTT_CUBE;
                        break;
                    case AD3D9_VOLUMETEXTURE:
                        fixedFunctionState.textureType[samp] = D3DSTT_VOLUME;
                        break;
                    default:
                        panic("AD3D9State", "setTexture", "Undefined texture type.");
                        break;                    
                }
            }
        }
        
                /*if (batchCount == 619) {
                    if (textureTextureType[tex] == AD3D9_TEXTURE2D) {
                        cout << "---- setTexture ----" << endl;
                        cout << "Sampler: " << samp << endl;
                        cout << "Format: " << textures2D[(AITextureImp9*)tex]->getFormat(0) << endl;
                        cout << "Width: " << textures2D[(AITextureImp9*)tex]->getWidth(0) << endl;
                        cout << "Height: " << textures2D[(AITextureImp9*)tex]->getHeight(0) << endl;
                        cout << "MemoryLayout: " << textures2D[(AITextureImp9*)tex]->getMemoryLayout() << endl;
                        cout << "SettedMipmaps: " << textures2D[(AITextureImp9*)tex]->getSettedMipmaps() << endl;
                    }
                }*/
    }
    else if (samp == D3DDMAPSAMPLER)
    {
        printf("WARNING!!! AD3D9State::setTexture => Map sampler not implemented (tesselator).\n");
    }
    else if ((samp >= D3DVERTEXTEXTURESAMPLER0) && (samp <= D3DVERTEXTEXTURESAMPLER3))
    {
        settedVertexTextures[samp - D3DVERTEXTEXTURESAMPLER0].samplerData = tex;
    }
    else
    {
        panic("AD3D9State", "setTexture", "Sampler identifier out of range.");   
    }

}

void AD3D9State::setLOD(AIBaseTextureImp9* tex, DWORD lod)
{
    //  NOTE:  SetLOD in Direct3D9 just affects how mipmaps are loaded in video memory
    //  not the lod computation or mipmap selection.
   
    //switch (textureTextureType[tex])
    //{
    //    case AD3D9_TEXTURE2D:
    //        /*textures2D[(AITextureImp9*)tex]*/((AITextureImp9*)tex)->getACDTexture2D()->setBaseLevel(lod);
    //        break;
    //
    //    case AD3D9_CUBETEXTURE:
    //        /*cubeTextures[(AICubeTextureImp9*)tex]*/((AICubeTextureImp9*)tex)->getACDCubeTexture()->setBaseLevel(lod);
    //        break;
    //
    //    default:
    //        panic("AD3D9State", "setLOD", "Unsupported texture type");
    //        break;
    //}
}

void AD3D9State::setSamplerState(UINT samp, D3DSAMPLERSTATETYPE type, DWORD value) {

    if (samp < maxSamplers)
    {
        switch(type)
        {
            case D3DSAMP_MINFILTER:
                settedTextures[samp].minFilter = value;
                break;

            case D3DSAMP_MIPFILTER:
                settedTextures[samp].mipFilter = value;
                break;

            case D3DSAMP_MAGFILTER:
                settedTextures[samp].magFilter = value;
                break;

            case D3DSAMP_MAXANISOTROPY:
                settedTextures[samp].maxAniso = value;
                break;

            case D3DSAMP_ADDRESSU:
                settedTextures[samp].addressU = value;
                break;

            case D3DSAMP_ADDRESSV:
                settedTextures[samp].addressV = value;
                break;

            case D3DSAMP_ADDRESSW:
                settedTextures[samp].addressW = value;
                break;

            case D3DSAMP_MAXMIPLEVEL:
                settedTextures[samp].maxLevel = value;
                break;

            case D3DSAMP_BORDERCOLOR:
            case D3DSAMP_MIPMAPLODBIAS:

                // NOT IMPLEMENTED!!!!
                break;

            case D3DSAMP_SRGBTEXTURE:
                settedTextures[samp].sRGBTexture = value;
                break;
                
            case D3DSAMP_ELEMENTINDEX:
            case D3DSAMP_DMAPOFFSET:
                // NOT SUPPORTED BY THE SIMULATOR!!!

                break;

            default:
                //D3D_DEBUG( D3D_DEBUG( cout << "AD3D9State: WARNING: Sampler state " << samplerstate2string(type) << ", value " << (int)value << " not supported." << endl; ) )
                {
                    char message[50];
                    sprintf(message, "Unsupported sampler state identifier with value = %d", type);
                    panic("AD3D9State", "setSamplerState", message);
                }
                break;
        }
    }
    else if (samp == D3DDMAPSAMPLER)
    {
        printf("WARNING!!! AD3D9State::setTexture => Map sampler not implemented (tesselator).\n");
    }
    else if ((samp >= D3DVERTEXTEXTURESAMPLER0) && (samp <= D3DVERTEXTEXTURESAMPLER3))
    {
        samp = samp - D3DVERTEXTEXTURESAMPLER0;
        
        switch(type)
        {
            case D3DSAMP_MINFILTER:
                settedVertexTextures[samp].minFilter = value;
                break;

            case D3DSAMP_MIPFILTER:
                settedVertexTextures[samp].mipFilter = value;
                break;

            case D3DSAMP_MAGFILTER:
                settedVertexTextures[samp].magFilter = value;
                break;

            case D3DSAMP_MAXANISOTROPY:
                settedVertexTextures[samp].maxAniso = value;
                break;

            case D3DSAMP_ADDRESSU:
                settedVertexTextures[samp].addressU = value;
                break;

            case D3DSAMP_ADDRESSV:
                settedVertexTextures[samp].addressV = value;
                break;

            case D3DSAMP_ADDRESSW:
                settedVertexTextures[samp].addressW = value;
                break;

            case D3DSAMP_MAXMIPLEVEL:
                settedVertexTextures[samp].maxLevel = value;
                break;

            case D3DSAMP_BORDERCOLOR:
            case D3DSAMP_MIPMAPLODBIAS:

                // NOT IMPLEMENTED!!!!
                break;

            case D3DSAMP_SRGBTEXTURE:
                settedVertexTextures[samp].sRGBTexture = value;
                break;
                
            case D3DSAMP_ELEMENTINDEX:
            case D3DSAMP_DMAPOFFSET:
                // NOT SUPPORTED BY THE SIMULATOR!!!

                break;

            default:
                //D3D_DEBUG( D3D_DEBUG( cout << "AD3D9State: WARNING: Sampler state " << samplerstate2string(type) << ", value " << (int)value << " not supported." << endl; ) )
                {
                    char message[50];
                    sprintf(message, "Unsupported sampler state identifier with value = %d", type);
                    panic("AD3D9State", "setSamplerState", message);
                }
                break;
        }
    }
    else
    {
        panic("AD3D9State", "setSamplerState", "Sampler identifier out of range.");   
    }
}

void AD3D9State::setFixedFunctionState(D3DRENDERSTATETYPE state , DWORD value)
{
    switch(state)
    {
        case D3DRS_FOGENABLE:         
        
            fixedFunctionState.fogEnable = true;
            
            setFogState(state, value);
            break;
        
        case D3DRS_FOGCOLOR:
        
            fixedFunctionState.fogColor.r = f32bit((value >> 16) && 0xff) / 255.0f;
            fixedFunctionState.fogColor.g = f32bit((value >> 8) && 0xff) / 255.0f;
            fixedFunctionState.fogColor.b = f32bit(value & 0x0ff) / 255.0f;
            fixedFunctionState.fogColor.a = f32bit((value >> 24) && 0xff) / 255.0f;
            
            setFogState(state, value);           
            break;
            
        case D3DRS_FOGTABLEMODE:
        
            fixedFunctionState.fogPixelMode = static_cast<D3DFOGMODE>(value);
            break;
            
        case D3DRS_FOGVERTEXMODE:
        
            fixedFunctionState.fogVertexMode = static_cast<D3DFOGMODE>(value);
            break;

        case D3DRS_FOGSTART:
        
            fixedFunctionState.fogStart = *((f32bit *) &value);
            break;
            
        case D3DRS_FOGEND:

            fixedFunctionState.fogEnd = *((f32bit *) &value);
            break;
            
        case D3DRS_FOGDENSITY:
        
            fixedFunctionState.fogDensity = *((f32bit *) &value);
            break;
        
        case D3DRS_RANGEFOGENABLE:
        
            fixedFunctionState.fogRange = (value != FALSE);
            break;
            
        case D3DRS_WRAP0:
        case D3DRS_WRAP1:
        case D3DRS_WRAP2:
        case D3DRS_WRAP3:
        case D3DRS_WRAP4:
        case D3DRS_WRAP5:
        case D3DRS_WRAP6:
        case D3DRS_WRAP7:
        case D3DRS_WRAP8:
        case D3DRS_WRAP9:
        case D3DRS_WRAP10:
        case D3DRS_WRAP11:
        case D3DRS_WRAP12:
        case D3DRS_WRAP13:
        case D3DRS_WRAP14:
        case D3DRS_WRAP15:
        
            printf("WARNING!!! AD3D9State::setFixedFunctionState => WRAP state not implemented.\n");
            break;
        
        case D3DRS_SPECULARENABLE:
        
            fixedFunctionState.specularEnable = (value != FALSE);            
            break;
            
        case D3DRS_LIGHTING:
        
            fixedFunctionState.lightingEnabled = (value != FALSE);
            break;
            
        case D3DRS_AMBIENT:

            fixedFunctionState.ambient.r = f32bit((value >> 16) && 0xff) / 255.0f;
            fixedFunctionState.ambient.g = f32bit((value >> 8) && 0xff) / 255.0f;
            fixedFunctionState.ambient.b = f32bit(value & 0x0ff) / 255.0f;
            fixedFunctionState.ambient.a = f32bit((value >> 24) && 0xff) / 255.0f;
            break;
            
        case D3DRS_COLORVERTEX:
        
            fixedFunctionState.vertexColor = (value != FALSE);
            break;
            
        case D3DRS_LOCALVIEWER:
        
            fixedFunctionState.localViewer = (value != FALSE);
            break;
            
        case D3DRS_NORMALIZENORMALS:
        
            fixedFunctionState.normalizeNormals = (value != FALSE);
            break;
            
        case D3DRS_DIFFUSEMATERIALSOURCE:
        
            fixedFunctionState.diffuseMaterialSource = static_cast<D3DMATERIALCOLORSOURCE>(value);
            break;
            
        case D3DRS_SPECULARMATERIALSOURCE:

            fixedFunctionState.specularMaterialSource = static_cast<D3DMATERIALCOLORSOURCE>(value);
            break;

        case D3DRS_AMBIENTMATERIALSOURCE:

            fixedFunctionState.ambientMaterialSource = static_cast<D3DMATERIALCOLORSOURCE>(value);
            break;

        case D3DRS_EMISSIVEMATERIALSOURCE:

            fixedFunctionState.emissiveMaterialSource = static_cast<D3DMATERIALCOLORSOURCE>(value);
            break;

        case D3DRS_VERTEXBLEND:
            
            fixedFunctionState.vertexBlend = static_cast<D3DVERTEXBLENDFLAGS>(value);
            break;
            
        case D3DRS_INDEXEDVERTEXBLENDENABLE:
        
            fixedFunctionState.indexedVertexBlend = (value != FALSE);
            break;
            
        case D3DRS_TWEENFACTOR:       
               
            fixedFunctionState.tweenFactor = *((f32bit *) &value);
            break;
        
        case D3DRS_TEXTUREFACTOR:
            
            fixedFunctionState.textureFactor.r = f32bit((value >> 16) && 0xff) / 255.0f;
            fixedFunctionState.textureFactor.g = f32bit((value >> 8) && 0xff) / 255.0f;
            fixedFunctionState.textureFactor.b = f32bit(value & 0x0ff) / 255.0f;
            fixedFunctionState.textureFactor.a = f32bit((value >> 24) && 0xff) / 255.0f;
            break;
            
        default:
            panic("AD3D9State", "setFixedFunctionState", "Undefined fixed function state.");
            break;
    }    
}

void AD3D9State::setLight(DWORD index , CONST D3DLIGHT9* pLight)
{
    //  Check light index.
    if (index < 8)
        memcpy(&fixedFunctionState.lights[index], pLight, sizeof(D3DLIGHT9));
    else
        panic("AD3D9State", "setLight", "Light index out of range");
}

void AD3D9State::enableLight(DWORD index, BOOL enable)
{
    //  Check light index.
    if (index < 8)
        fixedFunctionState.lightsEnabled[index] = (enable != FALSE);
    else
        panic("AD3D9State", "enableLight", "Light index out of range");
}

void AD3D9State::setMaterial(CONST D3DMATERIAL9* pMaterial)
{
    memcpy(&fixedFunctionState.material, pMaterial, sizeof(D3DMATERIAL9));
}

void AD3D9State::setTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX * pMatrix)
{
    if (State < 256)
    {
        switch(State)
        {
            case D3DTS_VIEW:

                memcpy(&fixedFunctionState.view, pMatrix, sizeof(D3DMATRIX));
                break;

            case D3DTS_PROJECTION:

                memcpy(&fixedFunctionState.view, pMatrix, sizeof(D3DMATRIX));
                break;

            case D3DTS_TEXTURE0:

                memcpy(&fixedFunctionState.texture[0], pMatrix, sizeof(D3DMATRIX));
                break;
            
            case D3DTS_TEXTURE1:

                memcpy(&fixedFunctionState.texture[1], pMatrix, sizeof(D3DMATRIX));
                break;

            case D3DTS_TEXTURE2:

                memcpy(&fixedFunctionState.texture[2], pMatrix, sizeof(D3DMATRIX));
                break;

            case D3DTS_TEXTURE3:

                memcpy(&fixedFunctionState.texture[3], pMatrix, sizeof(D3DMATRIX));
                break;

            case D3DTS_TEXTURE4:

                memcpy(&fixedFunctionState.texture[4], pMatrix, sizeof(D3DMATRIX));
                break;

            case D3DTS_TEXTURE5:

                memcpy(&fixedFunctionState.texture[5], pMatrix, sizeof(D3DMATRIX));
                break;

            case D3DTS_TEXTURE6:

                memcpy(&fixedFunctionState.texture[6], pMatrix, sizeof(D3DMATRIX));
                break;

            case D3DTS_TEXTURE7:

                memcpy(&fixedFunctionState.texture[7], pMatrix, sizeof(D3DMATRIX));
                break;

            default:
                panic("AD3D9State", "setTransform", "Undefined matrix.");
                break;
        }
    }
    else
    {
        //  Check for world matrix 0.
        if (State == 256)
            memcpy(&fixedFunctionState.world, pMatrix, sizeof(D3DMATRIX));
        /*else
            printf("WARNING!!! AD3D9State::setTransform => Only one world matrix supported.\n");*/
    }
}

void AD3D9State::setTextureStage(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
    if (Stage > 7)
        panic("setTextureStage", "setTextureStage", "Texture stage identifier out of range");
        
    switch(Type)
    {
        case D3DTSS_COLOROP:

            fixedFunctionState.textureStage[Stage].colorOp = static_cast<D3DTEXTUREOP>(Value);
            break;
            
        case D3DTSS_COLORARG1:
        
            fixedFunctionState.textureStage[Stage].colorArg1 = Value;
            break;
            
        case D3DTSS_COLORARG2:

            fixedFunctionState.textureStage[Stage].colorArg2 = Value;
            break;

        case D3DTSS_ALPHAOP:
        
            fixedFunctionState.textureStage[Stage].alphaOp = static_cast<D3DTEXTUREOP>(Value);
            break;
            
        case D3DTSS_ALPHAARG1:
        
            fixedFunctionState.textureStage[Stage].alphaArg1 = Value;
            break;
            
        case D3DTSS_ALPHAARG2:
        
            fixedFunctionState.textureStage[Stage].alphaArg2 = Value;
            break;

        case D3DTSS_BUMPENVMAT00:

            fixedFunctionState.textureStage[Stage].bumpEnvMatrix[0][0] = *((f32bit *) &Value);
            break;
            
        case D3DTSS_BUMPENVMAT01:

            fixedFunctionState.textureStage[Stage].bumpEnvMatrix[0][1] = *((f32bit *) &Value);
            break;

        case D3DTSS_BUMPENVMAT10:

            fixedFunctionState.textureStage[Stage].bumpEnvMatrix[1][0] = *((f32bit *) &Value);
            break;

        case D3DTSS_BUMPENVMAT11:

            fixedFunctionState.textureStage[Stage].bumpEnvMatrix[1][1] = *((f32bit *) &Value);
            break;

        case D3DTSS_TEXCOORDINDEX:
            
            fixedFunctionState.textureStage[Stage].index = Value;
            break;
            
        case D3DTSS_BUMPENVLSCALE:
        
            fixedFunctionState.textureStage[Stage].bumpEnvLScale = *((f32bit *) &Value);
            break;
            
        case D3DTSS_BUMPENVLOFFSET:
        
            fixedFunctionState.textureStage[Stage].bumpEnvLOffset = *((f32bit *) &Value);
            break;
            
        case D3DTSS_TEXTURETRANSFORMFLAGS:
            
            fixedFunctionState.textureStage[Stage].transformFlags = static_cast<D3DTEXTURETRANSFORMFLAGS>(Value);
            break;
            
        case D3DTSS_COLORARG0:

            fixedFunctionState.textureStage[Stage].colorArg0 = Value;
            break;
            
        case D3DTSS_ALPHAARG0:

            fixedFunctionState.textureStage[Stage].alphaArg0 = Value;
            break;

        case D3DTSS_RESULTARG:
            
            fixedFunctionState.textureStage[Stage].resultArg = Value;
            break;
            
        case D3DTSS_CONSTANT:

            fixedFunctionState.textureStage[Stage].constant.r = f32bit((Value >> 16) && 0xff) / 255.0f;
            fixedFunctionState.textureStage[Stage].constant.g = f32bit((Value >> 8) && 0xff) / 255.0f;
            fixedFunctionState.textureStage[Stage].constant.b = f32bit(Value && 0xff) / 255.0f;
            fixedFunctionState.textureStage[Stage].constant.a = f32bit((Value >> 24) && 0xff) / 255.0f;
            break;
            
        default:
            panic("AD3D9State", "setTextureStage", "Undefined texture stage state.");
            break;
    }
}

void AD3D9State::setBlendingState(D3DRENDERSTATETYPE state , DWORD value) {

    switch (state)
    {
        case D3DRS_ALPHABLENDENABLE:
            alphaBlend = (value != FALSE);
            break;

        case D3DRS_SEPARATEALPHABLENDENABLE:
            separateAlphaBlend = (value != FALSE);
            break;

        case D3DRS_SRCBLEND:
            srcBlend = getACDBlendOption((D3DBLEND)value);
            break;

        case D3DRS_SRCBLENDALPHA:
            srcBlendAlpha = getACDBlendOption((D3DBLEND)value);
            break;

        case D3DRS_DESTBLEND:
            dstBlend = getACDBlendOption((D3DBLEND)value);
            break;

        case D3DRS_DESTBLENDALPHA:
            dstBlendAlpha = getACDBlendOption((D3DBLEND)value);
            break;

        case D3DRS_BLENDOP:
            if (value != 0)
                blendOp = getACDBlendOperation((D3DBLENDOP)value);
            break;

        case D3DRS_BLENDOPALPHA:
            blendOpAlpha = getACDBlendOperation((D3DBLENDOP)value);
            break;

        case D3DRS_BLENDFACTOR:
            acdDev->blending().setBlendColor(0, acdlib::acd_float((value >> 16) & 0xff) / 255.0f,
                                                acdlib::acd_float((value >> 8) & 0xff) / 255.0f,
                                                acdlib::acd_float((value) & 0xff) / 255.0f,
                                                acdlib::acd_float((value >> 24) & 0xff) / 255.0f);
            break;

        default:
            panic("AD3D9State", "setBlendingState", "Unsupported blending state.");
            break;

    }
    redefineACDBlend();

}

void AD3D9State::setFogState(D3DRENDERSTATETYPE state , DWORD value)
{
    switch (state)
    {
        case D3DRS_FOGENABLE:
            fogEnable = (value != FALSE);
            break;

        case D3DRS_FOGCOLOR:
            fogColor = value;
            break;

        default:
            panic("AD3D9State", "setFogState", "Unsupported alpha test state.");
            break;
    }
}

void AD3D9State::setAlphaTestState(D3DRENDERSTATETYPE state , DWORD value)
{
    switch (state)
    {
        case D3DRS_ALPHATESTENABLE:
            alphaTest = (value != FALSE);
            break;

        case D3DRS_ALPHAREF:
            alphaRef = value;
            break;

        case D3DRS_ALPHAFUNC:
            alphaFunc = (D3DCMPFUNC)value;
            break;

        default:
            panic("AD3D9State", "setAlphaTestState", "Unsupported alpha test state.");
            break;
    }
}

void AD3D9State::setZState(D3DRENDERSTATETYPE state , DWORD value)
{
    switch (state)
    {
        case D3DRS_ZENABLE:

            switch(value)
            {
                case D3DZB_FALSE:
                    acdDev->zStencil().setZEnabled(false);
                    break;

                case D3DZB_TRUE:
                    acdDev->zStencil().setZEnabled(true);
                    break;

                case D3DZB_USEW:
                    cout << "AD3D9State: WARNING: D3DZB_USEW is not supported." << endl;
                    break;

                default:
                    panic("AD3D9State", "setZState", "Undefined z enable parameter value.");
                    break;
            }

            break;

        case D3DRS_ZFUNC:
            switch(value)
            {

                case D3DCMP_LESS:
                    acdDev->zStencil().setZFunc(acdlib::ACD_COMPARE_FUNCTION_LESS);
                    //cout << "AD3D9State: ZFUNC = LESS" << endl;
                    break;

                case D3DCMP_LESSEQUAL:
                    acdDev->zStencil().setZFunc(acdlib::ACD_COMPARE_FUNCTION_LESS_EQUAL);
                    //cout << "AD3D9State: ZFUNC = LESS_EQUAL" << endl;
                    break;

                case D3DCMP_GREATER:
                    acdDev->zStencil().setZFunc(acdlib::ACD_COMPARE_FUNCTION_GREATER);
                    break;

                case D3DCMP_EQUAL:
                    acdDev->zStencil().setZFunc(acdlib::ACD_COMPARE_FUNCTION_EQUAL);
                    break;

                case D3DCMP_ALWAYS:
                    acdDev->zStencil().setZFunc(acdlib::ACD_COMPARE_FUNCTION_ALWAYS);
                    break;
                
                default:
                    panic("AD3D9State", "setZState", "Unsupported z function parameter value.");
                    break;

            }
            break;


        case D3DRS_ZWRITEENABLE:
            if (value != FALSE)
                acdDev->zStencil().setZMask(true);
            else
                acdDev->zStencil().setZMask(false);

            break;

        default:
            panic("AD3D9State", "setZState", "Unsupported z test parameter.");
            break;
    }
}

void AD3D9State::setStencilState(D3DRENDERSTATETYPE state , DWORD value)
{
    switch (state)
    {
        case D3DRS_STENCILENABLE:
            switch(value)
            {
                case D3DZB_FALSE:
                    stencilEnabled = false;
                    break;

                case D3DZB_TRUE:
                    stencilEnabled = true;
                    break;

                default:
                    panic("AD3D9State", "setStencilState", "Undefined stencil enable parameter value.");
                    break;
            }
            break;

        case D3DRS_STENCILFAIL:
            stencilFail = getACDStencilOperation((D3DSTENCILOP)value);
            break;

        case D3DRS_STENCILZFAIL:
            stencilZFail = getACDStencilOperation((D3DSTENCILOP)value);
            break;

        case D3DRS_STENCILPASS:
            stencilPass = getACDStencilOperation((D3DSTENCILOP)value);
            break;

        case D3DRS_STENCILFUNC:
            stencilFunc = getACDCompareFunction((D3DCMPFUNC)value);
            break;

        case D3DRS_STENCILREF:
            stencilRef = value;
            break;

        case D3DRS_STENCILMASK:
            stencilMask = value;
            break;

        case D3DRS_STENCILWRITEMASK:
            stencilWriteMask = value;
            break;

        case D3DRS_TWOSIDEDSTENCILMODE:
            switch(value)
            {
                case D3DZB_FALSE:
                    twoSidedStencilEnabled = false;
                    break;

                case D3DZB_TRUE:
                    twoSidedStencilEnabled = true;
                    break;

                default:
                    panic("AD3D9State", "setStencilState", "Undefined two sided stencil mode parameter value.");
                    break;
            }
            break;

        case D3DRS_CCW_STENCILFAIL:
            ccwStencilFail = getACDStencilOperation((D3DSTENCILOP)value);
            break;

        case D3DRS_CCW_STENCILZFAIL:
            ccwStencilZFail = getACDStencilOperation((D3DSTENCILOP)value);
            break;

        case D3DRS_CCW_STENCILPASS:
            ccwStencilPass = getACDStencilOperation((D3DSTENCILOP)value);
            break;

        case D3DRS_CCW_STENCILFUNC:
            ccwStencilFunc = getACDCompareFunction((D3DCMPFUNC)value);
            break;

        default:
            panic("AD3D9State", "setStencilState", "Unsupported stencil paramter.");
            break;
    }

    redefineACDStencil();

}

void AD3D9State::setColorWriteEnable(DWORD RenderTargetIndex, DWORD value)
{
    colorwriteRed[RenderTargetIndex]   = ((value & D3DCOLORWRITEENABLE_RED)   != 0);
    colorwriteGreen[RenderTargetIndex] = ((value & D3DCOLORWRITEENABLE_GREEN) != 0);
    colorwriteBlue[RenderTargetIndex]  = ((value & D3DCOLORWRITEENABLE_BLUE)  != 0);
    colorwriteAlpha[RenderTargetIndex] = ((value & D3DCOLORWRITEENABLE_ALPHA) != 0);

    acdDev->blending().setColorMask(RenderTargetIndex, colorwriteRed[RenderTargetIndex], colorwriteGreen[RenderTargetIndex], colorwriteBlue[RenderTargetIndex], colorwriteAlpha[RenderTargetIndex]);
}

void AD3D9State::setColorSRGBWrite(DWORD value)
{
    acdDev->setColorSRGBWrite(value != FALSE);
}

void AD3D9State::setCullMode(DWORD value) {

    switch (value)
    {
        case D3DCULL_NONE:
            acdDev->rast().setCullMode(acdlib::ACD_CULL_NONE);
            break;

        case D3DCULL_CW:
            acdDev->rast().setCullMode(acdlib::ACD_CULL_FRONT);
            //acdDev->rast().setCullMode(acdlib::ACD_CULL_BACK);
            //acdDev->rast().setFaceMode(acdlib::ACD_FACE_CCW);
            break;

        case D3DCULL_CCW:
            acdDev->rast().setCullMode(acdlib::ACD_CULL_BACK);
            //acdDev->rast().setFaceMode(acdlib::ACD_FACE_CW);
            break;

        default:
            panic("AD3D9State", "setCullMode", "Undefined cull mode value.");
            break;
    }

}

void AD3D9State::enableScissorTest(DWORD value)
{
    if (value != FALSE)
        acdDev->rast().enableScissor(true);
    else
        acdDev->rast().enableScissor(false);

}

void AD3D9State::scissorRect(UINT x1, UINT y1, UINT x2, UINT y2) {

    acdDev->rast().setScissor(x1, y1, x2 - x1, y2 - y1);

}


void AD3D9State::setFreqType(UINT stream, UINT type) {

    if (stream < maxStreams)
        settedVertexBuffers[stream].freqType = type;

}

void AD3D9State::setFreqValue(UINT stream, UINT value) {

    if (stream < maxStreams)
        settedVertexBuffers[stream].freqValue = value;

}

void AD3D9State::clear(DWORD count, CONST D3DRECT* rects, DWORD flags, D3DCOLOR color, float z, DWORD stencil) {

    // Color mask doesn't afect clear
    //acdDev->blending().setColorMask(true, true, true, true);

    //  Check if clear on the color buffer must be disabled.
    
    D3DSURFACE_DESC sDesc;
    bool disableColorClear;
    
    //  Check if z stencil surface is set to NULL;
    bool disableZStencilClear = (currentZStencilSurface == NULL);
    
    //  Check if the current render surface is set to NULL.
    if (currentRenderSurface[0] == NULL)
    {
        //  Render surface is disabled.
        disableColorClear = true;
    }
    else
    {
        //  Get information about the current render surface.
        currentRenderSurface[0]->GetDesc(&sDesc);

        //  Check if the render surface is of NULL FOURCC type.
        if (sDesc.Format == D3DFMT_NULL)
        {
            //  Render surface is disabled.
            disableColorClear = true;
        }
        else
        {
            //  Check if the z stencil surface is defined.
            if (!disableZStencilClear)
            {
                acdlib::ACDRenderTarget *rt = currentRenderSurface[0]->getACDRenderTarget();
                acdlib::ACDRenderTarget *zst = currentZStencilSurface->getACDRenderTarget();
                disableColorClear = (rt != NULL) && (zst != NULL) &&
                                    !(colorwriteRed[0] || colorwriteBlue[0] || colorwriteGreen[0] || colorwriteAlpha[0]) &&
                                    ((rt->getWidth() != zst->getWidth()) || (rt->getHeight() != zst->getHeight()));
            }
            else
                disableColorClear = false;
        }
    }
        
    if (rects == NULL)
    {
        if ((flags & D3DCLEAR_TARGET) && !disableColorClear)
        {
                // Clear color buffer with the setted color
                acdDev->clearColorBuffer( (color >> 16) & 0xff, (color >> 8) & 0xff, (color) & 0xff, (color >> 24) & 0xff );
        }

        if ((flags & D3DCLEAR_ZBUFFER) && (flags & D3DCLEAR_STENCIL) && !disableZStencilClear)
        {
            // If flags to clear depth buffer and stencil buffer are active, clear at the same time
            acdDev->clearZStencilBuffer(true, true, z, stencil);
        }
        else if ((flags & D3DCLEAR_STENCIL) && !disableZStencilClear)
        {
            // else clear stencil buffer
            acdDev->clearZStencilBuffer(false, true, z, stencil);
        }
        else if ((flags & D3DCLEAR_ZBUFFER) && !disableZStencilClear)
        {
            // or clear depth buffer
            acdDev->clearZStencilBuffer(true, false, z, stencil);
        }

    }
    else {

        acdlib::acd_bool senabled;
        acdlib::acd_int sx, sy;
        acdlib::acd_uint swidth, sheight;

        acdDev->rast().getScissor(senabled, sx, sy, swidth, sheight);

        acdDev->rast().enableScissor(true);

        for (acdlib::acd_uint i = 0; i < count; i++) {

            acdDev->rast().setScissor(rects[i].x1, rects[i].y1, rects[i].x2 - rects[i].x1, rects[i].y2 - rects[i].y1);

            if ((flags & D3DCLEAR_TARGET) && !disableColorClear)
            {
                // Clear color buffer with the setted color
                acdDev->clearColorBuffer( (color >> 16) & 0xff, (color >> 8) & 0xff, (color) & 0xff, (color >> 24) & 0xff );
            }

            if ((flags & D3DCLEAR_ZBUFFER) && (flags & D3DCLEAR_STENCIL) && !disableZStencilClear)
            {
                // If flags to clear depth buffer and stencil buffer are active, clear at the same time
                acdDev->clearZStencilBuffer(true, true, z, stencil);
            }
            else if ((flags & D3DCLEAR_STENCIL) && !disableZStencilClear)
            {
                // else clear stencil buffer
                acdDev->clearZStencilBuffer(false, true, z, stencil);
            }
            else if ((flags & D3DCLEAR_ZBUFFER) && !disableZStencilClear)
            {
                // or clear depth buffer
                acdDev->clearZStencilBuffer(true, false, z, stencil);
            }
        }

        acdDev->rast().enableScissor(senabled);
        acdDev->rast().setScissor(sx, sy, swidth, sheight);

    }

    //acdDev->blending().setColorMask(colorwriteRed, colorwriteGreen, colorwriteBlue, colorwriteAlpha);

    batchCount++;
}

void AD3D9State::setViewport(DWORD x, DWORD y, DWORD width, DWORD height, float minZ, float maxZ)
{
    // Set a new viewport position and size
    acdDev->rast().setViewport(x, y, width, height);

    acdDev->zStencil().setDepthRange(minZ, maxZ);
}

void AD3D9State::draw(D3DPRIMITIVETYPE type, UINT start, UINT count)
{

    //  Check if all rendering surfaces are set to NULL.
    if ((currentRenderSurface == NULL) && (currentZStencilSurface == NULL))
        return;
  
    //  Check if the primitive count is 0.
    if (count == 0)
        return;
        
    NativeShader* nativeVertexShader;

    // Set setted shaders to ACD.  Check for problems.
    if (!setACDShaders(nativeVertexShader))
        return;

    //  Reset instancing information.
    instancingMode = false;
    instancesLeft = 1;
    instancesDone = 0;
    
    // Set setted vertex buffers in streams to ACD
    setACDStreams(nativeVertexShader, get_vertex_count(type, count), 0);

    // Set setted textures in samplers to ACD
    setACDTextures();

    // Draw
    acdDev->setPrimitive(getACDPrimitiveType(type));

#ifdef SOFTWARE_INSTANCING
    if (instancingMode) {

        acdDev->draw(start, get_vertex_count(type, count));

        batchCount++;
                /*cout << "Batch: " << batchCount << endl;
                cout << "Instancing: " << instancesLeft << ", " << instancesDone << endl;*/

        acdDev->DBG_dump("acd_dump.txt",0);

        acdDev->disableVertexAttributes();

        instancesLeft--; instancesDone++;

        for (; instancesLeft != 0; --instancesLeft) {

            // Set setted vertex buffers in streams to ACD
            setACDStreams(nativeVertexShader, get_vertex_count(type, count), 0);

            acdDev->draw(start, get_vertex_count(type, count));

            batchCount++;
                /*cout << "Batch: " << batchCount << endl;
                cout << "Instancing: " << instancesLeft << ", " << instancesDone << endl;*/

            acdDev->DBG_dump("acd_dump.txt",0);

            acdDev->disableVertexAttributes();

            instancesDone++;

        }

    }
    else {

        acdDev->draw(start, get_vertex_count(type, count));

        batchCount++;
                //cout << "Batch: " << batchCount << endl;

        acdDev->DBG_dump("acd_dump.txt",0);

        acdDev->disableVertexAttributes();

    }
#else       //  !SOFTWARE_INSTANCING

    acdDev->draw(start, get_vertex_count(type, count), instancesLeft);

    batchCount++;
    //cout << "Batch: " << batchCount << endl;

    //acdDev->DBG_dump("acd_dump.txt",0);

    acdDev->disableVertexAttributes();

#endif      //  SOFTWARE_INSTANCIG

    //acdDev->swapBuffers();

    instancingMode = false;

}

int AD3D9State::getBatchCounter ()
{
    return batchCount;
}

void AD3D9State::drawIndexed(D3DPRIMITIVETYPE type, INT baseVertexIndex, UINT minVertexIndex, UINT numVertices, UINT start, UINT count) 
{
    //  Check if all rendering surfaces are set to NULL.
    if ((currentRenderSurface == NULL) && (currentZStencilSurface == NULL))
        return;

    //  Check if the count is 0.
    if (count == 0)
        return;
    
    //  Reset instancing information.
    instancingMode = false;
    instancesLeft = 1;
    instancesDone = 0;

    NativeShader* nativeVertexShader;
    
    // Set setted shaders to ACD.  Check for problems.
    if (!setACDShaders(nativeVertexShader))
        return;

    // Set setted vertex buffers in streams to ACDdu
    setACDStreams(nativeVertexShader, get_vertex_count(type, count), minVertexIndex + numVertices);

    // Set setted textures in samplers to ACD
    setACDTextures();

    // Set setted indices buffer to ACD
    setACDIndices();

    // Draw
    acdDev->setPrimitive(getACDPrimitiveType(type));

#ifdef SOFTWARE_INSTANCING
    if (instancingMode) {

        acdDev->drawIndexed(start, get_vertex_count(type, count), minVertexIndex, 0,baseVertexIndex);

        batchCount++;
                /*cout << "Batch: " << batchCount << endl;
                cout << "Instancing: " << instancesLeft << ", " << instancesDone << endl;*/

        acdDev->DBG_dump("acd_dump.txt",0);

        acdDev->disableVertexAttributes();

        instancesLeft--; instancesDone++;

        for (; instancesLeft != 0; --instancesLeft) {

            // Set setted vertex buffers in streams to ACD
            setACDStreams(nativeVertexShader, get_vertex_count(type, count), minVertexIndex + numVertices);

            // Set setted indices buffer to ACD
            //setACDIndices();

            acdDev->drawIndexed(start, get_vertex_count(type, count), minVertexIndex, 0,baseVertexIndex);

            batchCount++;
                /*cout << "Batch: " << batchCount << endl;
                cout << "Instancing: " << instancesLeft << ", " << instancesDone << endl;*/

            acdDev->DBG_dump("acd_dump.txt",0);

            acdDev->disableVertexAttributes();

            instancesDone++;

        }

    }
    else {

        acdDev->drawIndexed(start, get_vertex_count(type, count), minVertexIndex, 0,baseVertexIndex);

        batchCount++;
                //cout << "Batch: " << batchCount << endl;

        acdDev->DBG_dump("acd_dump.txt",0);

        acdDev->disableVertexAttributes();

    }
#else       //  !SOFTWARE_INSTANCING

    acdDev->drawIndexed(start, get_vertex_count(type, count), minVertexIndex, 0, baseVertexIndex, instancesLeft);

    //char dump[128];
    //sprintf(dump, "ACDDevice_batch_%d", batchCount);
    //acdDev->DBG_dump(dump,0);

    batchCount++;
    //cout << "Batch: " << batchCount << endl;

    acdDev->disableVertexAttributes();

#endif      //  SOFTWARE_INSTANCING


    //acdDev->swapBuffers();

    instancingMode = false;

}

void AD3D9State::swapBuffers() {

    // Swap buffers
    acdDev->swapBuffers();

}



bool AD3D9State::setACDShaders(NativeShader* &nativeVertexShader)
{
    FFShaderGenerator ffShGen;

    //  Check for POSITIONT in the vertex declaration.
    bool disableVertexProcessing = false;    
    
    //  Check if there is a vertex declaration defined.
    if (settedVertexDeclaration != NULL)
    {
        //  Get the vertex declaration.
        vector<D3DVERTEXELEMENT9> vElements = settedVertexDeclaration->getVertexElements();
        
        //  Search for POSITIONT usage which disables vertex processing.
        for(u32bit d = 0; d < vElements.size() && !disableVertexProcessing; d++)
            disableVertexProcessing = (vElements[d].Usage == D3DDECLUSAGE_POSITIONT);
    }

    //if (disableVertexProcessing)
    //    printf("AD3D9State::setACDShaders => Disabling vertex processing due to POSITIONT in vertex declaration.\n");

    // Set vertex shader
    if ((settedVertexShader != NULL) && !disableVertexProcessing)
    {
        nativeVertexShader = settedVertexShader->getNativeVertexShader();
        
        // If the setted shaders have a jump instruction the draw is skipped.
        if (nativeVertexShader->untranslatedControlFlow)
        {
            cout << " * WARNING: This batch is using shaders with untranslated jump instructionc, skiping it.\n";
            return false;
        }
        
        acdlib::ACDShaderProgram* acdVertexShader = settedVertexShader->getAcdVertexShader();      

        D3D_DEBUG( D3D_DEBUG( cout << "AD3D9State shader lenght: " << nativeVertexShader->lenght << endl; ) )
        D3D_DEBUG( D3D_DEBUG( cout << "AD3D9State shader assembler: \n" << nativeVertexShader->debug_assembler << endl; ) )
        //D3D_DEBUG( D3D_DEBUG( cout << "AD3D9State shader binary: \n" << nativeVertexShader->debug_binary << endl; ) )
        //D3D_DEBUG( D3D_DEBUG( cout << "AD3D9State shader ir: \n" << nativeVertexShader->debug_ir << endl; ) )

        for(acdlib::acd_uint c = 0; c < MAX_VERTEX_SHADER_CONSTANTS; c++)
        {
            if (settedVertexShaderConstantsTouched[c])
                acdVertexShader->setConstant(c, settedVertexShaderConstants[c]);
        }

        for (acdlib::acd_uint c = 0; c < 16; c++)
        {
            if (settedVertexShaderConstantsIntTouched[c])
                acdVertexShader->setConstant(INTEGER_CONSTANT_START + c, (acdlib::acd_float *) settedVertexShaderConstantsInt[c]);
           
            if (settedVertexShaderConstantsBoolTouched[c])
            {
                acdlib::acd_float psConstant[4];
                *((BOOL *) &psConstant[0]) = settedVertexShaderConstantsBool[c];
                psConstant[1] = 0.0f;
                psConstant[2] = 0.0f;
                psConstant[3] = 0.0f;
                acdVertexShader->setConstant(BOOLEAN_CONSTANT_START + c, psConstant);
            }
        }

        std::list<ConstRegisterDeclaration>::iterator itCRD;
        for (itCRD = nativeVertexShader->declaration.constant_registers.begin(); itCRD != nativeVertexShader->declaration.constant_registers.end(); itCRD++) {
            if (itCRD->defined) {
                //D3D_DEBUG( D3D_DEBUG( cout << "AD3D9State vertex shader constants defined: \n  * constNum: " << itCRD->native_register << "\n  * constant: " << itCRD->value.x << ", " << itCRD->value.y << ", " << itCRD->value.z << ", " << itCRD->value.w << endl; ) )
                acdlib::acd_float vsConstant[4];
                vsConstant[0] = itCRD->value.value.floatValue.x;
                vsConstant[1] = itCRD->value.value.floatValue.y;
                vsConstant[2] = itCRD->value.value.floatValue.z;
                vsConstant[3] = itCRD->value.value.floatValue.w;

                acdVertexShader->setConstant(itCRD->native_register, vsConstant);
            }
        }

        acdDev->setVertexShader(acdVertexShader);
    }
    else
    {
        //cout << " * WARNING: This batch is using vertex fixed function.\n";

        //  Delete previous native vertex shader if required.
        if (nativeFFVSh != NULL)
            delete nativeFFVSh;
        
        //  Generate a D3D9 pixel shader for the current fixed function state.        
        FFGeneratedShader *ffVSh;
        ffVSh = ffShGen.generate_vertex_shader(fixedFunctionState);

        //  Build the intermediate representation for the generated D3D9 vertex shader.
        IR *ffVShIR;
        ShaderTranslator::get_instance().buildIR(ffVSh->code, ffVShIR);

        //printf("Translating fixed function vertex shader.\n");

        //  Translate the generated D3D9 vertex shader to ATTILA instructions.
        nativeVertexShader = nativeFFVSh = ShaderTranslator::get_instance().translate(ffVShIR);

        //printf("-------- End of FF VSH translation --------\n");
        
        //  Set the generated code in the ACD fixed function vertex shader.
        fixedFunctionVertexShader->setCode(nativeFFVSh->bytecode, nativeFFVSh->lenght);

        //  Update the constants defined with value in the generated D3D9 vertex shader.
        std::list<ConstRegisterDeclaration>::iterator itCRD;
        for (itCRD = nativeFFVSh->declaration.constant_registers.begin();
             itCRD != nativeFFVSh->declaration.constant_registers.end();
             itCRD++)
        {
            //  Check if the constant was defined with a value.
            if (itCRD->defined)
            {
                //D3D_DEBUG( D3D_DEBUG( cout << "AD3D9State vertex shader constants defined: \n  * constNum: " << itCRD->native_register << "\n  * constant: " << itCRD->value.x << ", " << itCRD->value.y << ", " << itCRD->value.z << ", " << itCRD->value.w << endl; ) )
                acdlib::acd_float vsConstant[4];
                vsConstant[0] = itCRD->value.value.floatValue.x;
                vsConstant[1] = itCRD->value.value.floatValue.y;
                vsConstant[2] = itCRD->value.value.floatValue.z;
                vsConstant[3] = itCRD->value.value.floatValue.w;

                fixedFunctionVertexShader->setConstant(itCRD->native_register, vsConstant);
            }
        }

        //  Update the fixed function constants used by the generated D3D9 vertex shader.
        std::list<FFConstRegisterDeclaration>::iterator itFFCRD;
        for(itFFCRD = ffVSh->const_declaration.begin(); itFFCRD != ffVSh->const_declaration.end(); itFFCRD++)
        {
            acdlib::acd_float vsConstant[4];
            
            //  Translate the D3D9 constant identifier to the ATTILA constant identifier.
            u32bit constantID;
            bool found = false;
            std::list<ConstRegisterDeclaration>::iterator itCRD;
            for (itCRD = nativeFFVSh->declaration.constant_registers.begin();
                 itCRD != nativeFFVSh->declaration.constant_registers.end() && !found;
                 itCRD++)
            {
                //  Check if the register matches.
                if (itCRD->d3d_register == itFFCRD->constant.num)
                {
                    constantID = itCRD->native_register;
                    found = true;
                }
            }
            
            //  Check the constant usage type.
            switch(itFFCRD->usage.usage)
            {
	            case FF_NONE:
	                
	                // I think this one shouldn't be allowedd.
	                break;
	                
	            case FF_VIEWPORT:
	            
	                //  Check the usage index.
	                if (itFFCRD->usage.index == 0)
	                {
	                    u32bit width;
	                    u32bit height;
	                    
	                    //  Use the current RT dimensions.
	                    if (currentRenderSurface[0] != NULL)
	                    {
	                        width = currentRenderSurface[0]->getACDRenderTarget()->getWidth();
	                        height = currentRenderSurface[0]->getACDRenderTarget()->getHeight();
                        }
                        else
                        {
                            if (currentZStencilSurface != NULL)
                            {
                                width = currentZStencilSurface->getACDRenderTarget()->getWidth();
                                height = currentZStencilSurface->getACDRenderTarget()->getHeight();
                            }
                            else
                            {
                                width = 1;
                                height = 1;
                            }
                        }

#ifdef LOWER_RESOLUTION_HACK
                        width = (width > 1) ? (width << 1) : 1;
                        height = (height > 1) ? (height << 1) : 1;
#endif
	                    
	                    //  viewport0 = {2/width, -2/height, 1, 1}
	                    vsConstant[0] = 2.0f / f32bit(width);
	                    vsConstant[1] = -2.0f / f32bit(height);
	                    vsConstant[2] =
	                    vsConstant[3] = 1.0f;
                                               
                        //  Set the constant in the fixed function vertex shader.
                        fixedFunctionVertexShader->setConstant(constantID, vsConstant);
	                }
	                
	                break;
	                
	            case FF_WORLDVIEWPROJ:
	            case FF_WORLDVIEW:
	            case FF_WORLDVIEW_IT:
	            case FF_VIEW_IT:
	            case FF_WORLD:
	            case FF_MATERIAL_EMISSIVE:
	            case FF_MATERIAL_SPECULAR:
	            case FF_MATERIAL_DIFFUSE:
	            case FF_MATERIAL_AMBIENT:
	            case FF_MATERIAL_POWER:
	            case FF_AMBIENT:
	            case FF_LIGHT_POSITION:
	            case FF_LIGHT_DIRECTION:
	            case FF_LIGHT_AMBIENT:
	            case FF_LIGHT_DIFFUSE:
	            case FF_LIGHT_SPECULAR:
	            case FF_LIGHT_RANGE:
	            case FF_LIGHT_ATTENUATION:
	            case FF_LIGHT_SPOT:
	                
	                //  Not implemented.
	                break;
	                
	            default:
	                panic("AD3D9State", "setACDShaders", "Undefined fixed function constant usage.");
	                break;
            }
        }
        
        //  Set the fixed function vertex shader as the current ACD vertex shader.
        acdDev->setVertexShader(fixedFunctionVertexShader);
    }

    // Set pixel shader
    if (settedPixelShader != NULL)
    {
        NativeShader* nativePixelShader;

        if (alphaTest)
            nativePixelShader = settedPixelShader->getNativePixelShader(alphaFunc, fogEnable);
        else
            nativePixelShader = settedPixelShader->getNativePixelShader(D3DCMP_ALWAYS, fogEnable);

        // If the setted shaders have a jump instruction the draw is skipped.
        if (nativePixelShader->untranslatedControlFlow)
        {
            cout << " * WARNING: This batch is using shaders with untranslated jump instructionc, skiping it.\n";
            return false;
        }
    
        acdlib::ACDShaderProgram* acdPixelShader;

        if (alphaTest)
            acdPixelShader = settedPixelShader->getAcdPixelShader(alphaFunc, fogEnable);
        else
            acdPixelShader = settedPixelShader->getAcdPixelShader(D3DCMP_ALWAYS, fogEnable);

        D3D_DEBUG( D3D_DEBUG( cout << "AD3D9State shader lenght: " << nativePixelShader->lenght << endl; ) )
        D3D_DEBUG( D3D_DEBUG( cout << "AD3D9State shader assembler: \n" << nativePixelShader->debug_assembler << endl; ) )
        //D3D_DEBUG( D3D_DEBUG( cout << "AD3D9State shader binary: \n" << nativePixelShader->debug_binary << endl; ) )
        //D3D_DEBUG( D3D_DEBUG( cout << "AD3D9State shader ir: \n" << nativePixelShader->debug_ir << endl; ) )

        for(acdlib::acd_uint c = 0; c < MAX_PIXEL_SHADER_CONSTANTS; c++)
        {
            if (settedPixelShaderConstantsTouched[c])
                acdPixelShader->setConstant(c, settedPixelShaderConstants[c]);
        }
        
        for (acdlib::acd_uint c = 0; c < 16; c++)
        {
            if (settedPixelShaderConstantsIntTouched[c])
                acdPixelShader->setConstant(INTEGER_CONSTANT_START + c, (acdlib::acd_float *) settedPixelShaderConstantsInt[c]);
           
            if (settedPixelShaderConstantsBoolTouched[c])
            {
                acdlib::acd_float psConstant[4];
                *((BOOL *) &psConstant[0]) = settedPixelShaderConstantsBool[c];
                psConstant[1] = 0.0f;
                psConstant[2] = 0.0f;
                psConstant[3] = 0.0f;
                acdPixelShader->setConstant(BOOLEAN_CONSTANT_START + c, psConstant);
            }
        }

        std::list<ConstRegisterDeclaration>::iterator itCRD;
        for (itCRD = nativePixelShader->declaration.constant_registers.begin(); itCRD != nativePixelShader->declaration.constant_registers.end(); itCRD++)
        {
            if (itCRD->defined) {
                //D3D_DEBUG( D3D_DEBUG( cout << "AD3D9State pixel shader constants defined: \n  * constNum: " << itCRD->native_register << "\n  * constant: " << itCRD->value.x << ", " << itCRD->value.y << ", " << itCRD->value.z << ", " << itCRD->value.w << endl; ) )
                acdlib::acd_float psConstant[4];
                psConstant[0] = itCRD->value.value.floatValue.x;
                psConstant[1] = itCRD->value.value.floatValue.y;
                psConstant[2] = itCRD->value.value.floatValue.z;
                psConstant[3] = itCRD->value.value.floatValue.w;

                acdPixelShader->setConstant(itCRD->native_register, psConstant);
            }
        }

        if (alphaTest)
        {
            acdlib::acd_float alphaR[4];
            alphaR[0] = (float)alphaRef / 255.f;
            alphaR[1] = (float)alphaRef / 255.f;
            alphaR[2] = (float)alphaRef / 255.f;
            alphaR[3] = (float)alphaRef / 255.f;
            acdPixelShader->setConstant(nativePixelShader->declaration.alpha_test.alpha_const_ref, alphaR);

            acdlib::acd_float alphaM1[4];
            alphaM1[0] = -1.f;
            alphaM1[1] = -1.f;
            alphaM1[2] = -1.f;
            alphaM1[3] = -1.f;
            acdPixelShader->setConstant(nativePixelShader->declaration.alpha_test.alpha_const_minus_one, alphaM1);
        }
        
        if (fogEnable)
        {
            acdlib::acd_float fogColorConst[4];
            fogColorConst[0] = float((fogColor >> 16) & 0xFF) / 255.0f;
            fogColorConst[1] = float((fogColor >>  8) & 0xFF) / 255.0f;
            fogColorConst[2] = float( fogColor        & 0xFF) / 255.0f;
            fogColorConst[3] = 1.0f;
            acdPixelShader->setConstant(nativePixelShader->declaration.fog.fog_const_color, fogColorConst);
        }

        acdDev->setFragmentShader(acdPixelShader);
    }
    else
    {
        //cout << " * WARNING: This batch is using pixel fixed function, skiping it.\n";

        FFGeneratedShader *ffPSh;
        ffPSh = ffShGen.generate_pixel_shader(fixedFunctionState);

        IR *ffPShIR;
        ShaderTranslator::get_instance().buildIR(ffPSh->code, ffPShIR);
        
        //printf("Translating fixed function pixel shader.\n");            
        NativeShader *nativeFFPSh = ShaderTranslator::get_instance().translate(ffPShIR);
        //printf("-------- End of FF PSH translation --------\n");

        //  Set the generated code in the ACD fixed function pixel shader.
        fixedFunctionPixelShader->setCode(nativeFFPSh->bytecode, nativeFFPSh->lenght);

        //  Update the constants defined with value in the generated D3D9 pixel shader.
        std::list<ConstRegisterDeclaration>::iterator itCRD;
        for (itCRD = nativeFFPSh->declaration.constant_registers.begin();
             itCRD != nativeFFPSh->declaration.constant_registers.end();
             itCRD++)
        {
            //  Check if the constant was defined with a value.
            if (itCRD->defined)
            {
                //D3D_DEBUG( D3D_DEBUG( cout << "AD3D9State pixel shader constants defined: \n  * constNum: " << itCRD->native_register << "\n  * constant: " << itCRD->value.x << ", " << itCRD->value.y << ", " << itCRD->value.z << ", " << itCRD->value.w << endl; ) )
                acdlib::acd_float psConstant[4];
                psConstant[0] = itCRD->value.value.floatValue.x;
                psConstant[1] = itCRD->value.value.floatValue.y;
                psConstant[2] = itCRD->value.value.floatValue.z;
                psConstant[3] = itCRD->value.value.floatValue.w;

                fixedFunctionPixelShader->setConstant(itCRD->native_register, psConstant);
            }
        }

        //  Set the fixed function pixel shader as the current ACD pixel shader.
        acdDev->setFragmentShader(fixedFunctionPixelShader);
    }

    return true;
}

void AD3D9State::setACDStreams(NativeShader* nativeVertexShader, UINT count, UINT maxVertex) 
{

    // Set stream buffers
    std::vector<StreamDescription>::iterator itSS;
    acdlib::acd_uint acdStream = 0;

    std::vector<D3DVERTEXELEMENT9> vElements;

    if (settedVertexDeclaration != NULL)
    {
        // If exist the setted vertex declaration, use it
        vElements = settedVertexDeclaration->getVertexElements();
    }
    else 
    {
        // If not exist the setted vertex declaration or is NULL, create one using FVF
        createFVFVertexDeclaration();
        vElements = fvfVertexDeclaration;
    }

    // For each vertex buffer setted
    for (acdlib::acd_uint i = 0; i < maxStreams; i++) 
    {
        if (settedVertexBuffers[i].streamData != NULL) 
        {
            acdlib::ACDBuffer* currentBuffer;
            UINT currentOffset;
            UINT currentStride;

            
#ifdef SOFTWARE_INSTANCING
            if (settedVertexBuffers[i].freqType == D3DSTREAMSOURCE_INDEXEDDATA) {

                if (instancingMode == false)
                {
                    instancingMode = true;
                    instancesLeft = settedVertexBuffers[i].freqValue;
                    instancesDone = 0;
                }

                currentBuffer = settedVertexBuffers[i].streamData->getAcdVertexBuffer();
                currentOffset = settedVertexBuffers[i].offset;
                currentStride = settedVertexBuffers[i].stride;
            }
            else if (settedVertexBuffers[i].freqType == D3DSTREAMSOURCE_INSTANCEDATA)
            {
                instancingStride = settedVertexBuffers[i].freqValue;

                // Build a new buffer
                acdlib::acd_ubyte* transfData = new acdlib::acd_ubyte[(count + maxVertex) * settedVertexBuffers[i].stride];

                const acdlib::acd_ubyte* origData = settedVertexBuffers[i].streamData->getAcdVertexBuffer()->getData();

                for (acdlib::acd_uint j = 0; j < count + maxVertex; j++) {
                    memcpy(&transfData[j * settedVertexBuffers[i].stride], &origData[settedVertexBuffers[i].offset + (instancesDone * instancingStride) * settedVertexBuffers[i].stride], settedVertexBuffers[i].stride);
                }

                if (transfBuffer != NULL)
                    acdDev->destroy(transfBuffer);

                transfBuffer = acdDev->createBuffer((count + maxVertex) * settedVertexBuffers[i].stride, transfData);

                //cout << "Batch: " << batchCount << endl;

                /*if (instancingMode == true) {
                    cout << "origData: " << "(" << *((float *)&(origData[0])) << ", " << *((float *)&(origData[4])) << ", " << *((float *)&(origData[8])) << ", " << *((float *)&(origData[12])) << ")" << endl;
                    cout << "origData: " << "(" << *((float *)&(origData[0 + 16])) << ", " << *((float *)&(origData[4 + 16])) << ", " << *((float *)&(origData[8 + 16])) << ", " << *((float *)&(origData[12 + 16])) << ")" << endl;
                    cout << "origData: " << "(" << *((float *)&(origData[0 + 32])) << ", " << *((float *)&(origData[4 + 32])) << ", " << *((float *)&(origData[8 + 32])) << ", " << *((float *)&(origData[12 + 32])) << ")" << endl;

                    cout << "transfData: " << "(" << *((float *)&(transfData[0])) << ", " << *((float *)&(transfData[4])) << ", " << *((float *)&(transfData[8])) << ", " << *((float *)&(transfData[12])) << ")" << endl;
                    cout << "transfData: " << "(" << *((float *)&(transfData[0 + 16])) << ", " << *((float *)&(transfData[4 + 16])) << ", " << *((float *)&(transfData[8 + 16])) << ", " << *((float *)&(transfData[12 + 16])) << ")" << endl;
                    cout << "transfData: " << "(" << *((float *)&(transfData[0 + 32])) << ", " << *((float *)&(transfData[4 + 32])) << ", " << *((float *)&(transfData[8 + 32])) << ", " << *((float *)&(transfData[12 + 32])) << ")" << endl;

                    ofstream out;

                    out.open("ACDorigData.txt");

                    if ( !out.is_open() )
                        panic("ACDDumpStream", "ACDStreamImp", "Dump failed (output file could not be opened)");


                    //out << " Dumping Stream " << _STREAM_ID << endl;
                    out << "\t Stride: " << settedVertexBuffers[i].stride << endl;
                    //out << "\t Data type: " << _componentsType << endl;
                    out << "\t Size: " << settedVertexBuffers[i].streamData->getAcdVertexBuffer()->getSize() << endl;
                    //out << "\t Offset: " << _offset << endl;
                    out << "\t Dumping stream content: " << endl;

                    //((ACDBufferImp*)_buffer)->dumpBuffer(out, _componentsType, _components, _stride, _offset);

                    for (int j = 0; j < ((settedVertexBuffers[i].streamData->getAcdVertexBuffer()->getSize()) / settedVertexBuffers[i].stride); j++) {
                                    cout << "(" << *((float *)&(origData[0 + 48 * j])) << ", " << *((float *)&(origData[4 + 48 * j])) << ", " << *((float *)&(origData[8 + 48 * j])) << ", " << *((float *)&(origData[12 + 48 * j])) << ") ";
                                    cout << "(" << *((float *)&(origData[0 + 16 + 48 * j])) << ", " << *((float *)&(origData[4 + 16 + 48 * j])) << ", " << *((float *)&(origData[8 + 16 + 48 * j])) << ", " << *((float *)&(origData[12 + 16 + 48 * j])) << ") ";
                                    cout << "(" << *((float *)&(origData[0 + 32 + 48 * j])) << ", " << *((float *)&(origData[4 + 32 + 48 * j])) << ", " << *((float *)&(origData[8 + 32 + 48 * j])) << ", " << *((float *)&(origData[12 + 32 + 48 * j])) << ")" << endl;
                    }

                    out.close();

                    }*/

                delete[] transfData;

                currentBuffer = transfBuffer;
                currentOffset = 0;
                currentStride = settedVertexBuffers[i].stride;
            }
            else
            {
                currentBuffer = settedVertexBuffers[i].streamData->getAcdVertexBuffer();
                currentOffset = settedVertexBuffers[i].offset;
                currentStride = settedVertexBuffers[i].stride;
            }

#else   //  !SOFTWARE_INSTANCING


            //  Check if instancing is enabled in the current stream.
            if (settedVertexBuffers[i].freqType == D3DSTREAMSOURCE_INDEXEDDATA)
            {
                //  Check if instancing was already enabled.
                if (!instancingMode)
                {
                    //  Enable instancing.
                    instancingMode = true;
                    instancesLeft = settedVertexBuffers[i].freqValue;
                    instancesDone = 0;
                }
            }
            
            currentBuffer = settedVertexBuffers[i].streamData->getAcdVertexBuffer();
            currentOffset = settedVertexBuffers[i].offset;
            currentStride = settedVertexBuffers[i].stride;

#endif  //  SOFTWARE_INSTANCING

            std::vector<D3DVERTEXELEMENT9>::iterator itVE;

            // For each element declared
            for (itVE = vElements.begin(); itVE != vElements.end(); itVE++)
            {
                // If is the vertex buffer of the element
                if (itVE->Stream == i)
                {
                    acdlib::ACD_STREAM_DESC streamDescription;
                    streamDescription.offset = itVE->Offset + currentOffset;
                    streamDescription.stride = currentStride;
                    getTypeAndComponents((D3DDECLTYPE)itVE->Type, &(streamDescription.components), &(streamDescription.type));
                    

                    //  Check per index/vertex frequency or per instance frequency.
                    if (settedVertexBuffers[i].freqType == D3DSTREAMSOURCE_INSTANCEDATA)
                        streamDescription.frequency = 1;
                    else if (settedVertexBuffers[i].freqType == D3DSTREAMSOURCE_INDEXEDDATA)
                        streamDescription.frequency = 0;
                    else
                        streamDescription.frequency = 0;

                    // Set the vertex buffer to ACD
                    acdDev->stream(acdStream).set(currentBuffer, streamDescription);

                    D3D_DEBUG( D3D_DEBUG( cout << "IDEVICE9: Stream " << dec << acdStream << ":\n  * Offset: " << streamDescription.offset << "\n  * Stride: " << streamDescription.stride << "\n  * Components: " << streamDescription.components << "\n  * Type: " << streamDescription.type << endl; ) )

                    std::list<InputRegisterDeclaration>::iterator itIRD;

                    bool equivalentInput = false;

                    // Search for an input attribute in the shader corresponding with the vertex declaration.
                    for (itIRD = nativeVertexShader->declaration.input_registers.begin();
                         itIRD != nativeVertexShader->declaration.input_registers.end() && !equivalentInput; itIRD++)
                    {
                        //  There is a special case for the vertex position attribute.  It can be defined with usage
                        //  D3DDECLUSAGE_POSITION (0) or D3DDECLUSAGE_POSITIONT (9).  The difference is that
                        //  D3DDECLUSAGE_POSITIONT represents transformed vertices, in viewport space, coordinates
                        //  from [0,0] to [viewport width, viewport height], and disables vertex processing (however
                        //  I'm not sure if it actually disables vertex shading or only applies to the fixed function).
                        //
                        //  CHECK: More special cases may be possible.
                        equivalentInput = ((itIRD->usage == itVE->Usage) && (itIRD->usage_index == itVE->UsageIndex)) ||
                                          ((itIRD->usage == D3DDECLUSAGE_POSITION) && (itVE->Usage == D3DDECLUSAGE_POSITIONT) &&
                                           (itIRD->usage_index == itVE->UsageIndex));

                        // If the vertex buffer have the same usage as the register
                        if (equivalentInput)
                        {
                            // Enable the vertex attribute with this stream
                            D3D_DEBUG( D3D_DEBUG( cout << "IDEVICE9: Stream usage: " << (unsigned int)itIRD->usage << "  usage index: " << (unsigned int)itIRD->usage_index << endl; ) )
                            acdDev->enableVertexAttribute(itIRD->native_register ,acdStream);
                            D3D_DEBUG( D3D_DEBUG( cout << "IDEVICE9: Stream " << dec << acdStream << " setted as vertex attribute number " << itIRD->native_register << endl; ) )
                        }
                    } //End FOR
                    acdStream++;
                } // end IF
            } // end FOR
        }
    }// End FOR each vertexBuffer

}

void AD3D9State::setACDTextures()
{
    //std::vector<SamplerDescription>::iterator itTS;

    bool usedSamplers[maxSamplers];

    for (acdlib::acd_uint i = 0; i < maxSamplers; i++)
    {
    //for (itTS = settedTextures.begin(); itTS != settedTextures.end(); itTS++) {

        //if (itTS->first < 16) {
            if (settedTextures[i].samplerData == NULL)
            {
                //  Set this sampler as not used by a pixel shader sampler.
                usedSamplers[i] = false;

                acdDev->sampler(i).setEnabled(false);
            }
            else
            {
                //  Set this sampler as already used by a pixel shader sampler.
                usedSamplers[i] = true;
                
                acdDev->sampler(i).setEnabled(true);

                /*if (batchCount == 619) {
                    if (textureTextureType[settedTextures[i].samplerData] == AD3D9_TEXTURE2D) {
                        cout << "Sampler: " << i << endl;
                        cout << " - Format: " << textures2D[(AITextureImp9*)settedTextures[i].samplerData]->getFormat(0) << endl;
                        cout << " - Width: " << textures2D[(AITextureImp9*)settedTextures[i].samplerData]->getWidth(0) << endl;
                        cout << " - Height: " << textures2D[(AITextureImp9*)settedTextures[i].samplerData]->getHeight(0) << endl;
                        cout << " - MemoryLayout: " << textures2D[(AITextureImp9*)settedTextures[i].samplerData]->getMemoryLayout() << endl;
                        cout << " - SettedMipmaps: " << textures2D[(AITextureImp9*)settedTextures[i].samplerData]->getSettedMipmaps() << endl;
                    }
                }*/

                if (textureTextureType[settedTextures[i].samplerData] == AD3D9_TEXTURE2D) {
                    acdDev->sampler(i).setTexture(/*textures2D[(AITextureImp9*)settedTextures[i].samplerData]*/((AITextureImp9*)settedTextures[i].samplerData)->getACDTexture2D());
                    if (((AITextureImp9*)settedTextures[i].samplerData)->usePCF()) {
                        acdDev->sampler(i).setEnableComparison(true);
                        acdDev->sampler(i).setComparisonFunction(acdlib::ACD_TEXTURE_COMPARISON_GEQUAL);
                    }
                    else {
                        acdDev->sampler(i).setEnableComparison(false);
                    }
                }
                else if (textureTextureType[settedTextures[i].samplerData] == AD3D9_CUBETEXTURE) {
                    acdDev->sampler(i).setTexture(/*cubeTextures[(AICubeTextureImp9*)settedTextures[i].samplerData]*/((AICubeTextureImp9*)settedTextures[i].samplerData)->getACDCubeTexture());
                    acdDev->sampler(i).setEnableComparison(false);
                }
                else if (textureTextureType[settedTextures[i].samplerData] == AD3D9_VOLUMETEXTURE) {
                    acdDev->sampler(i).setTexture(/*volumeTextures[(AIVolumeTextureImp9*)settedTextures[i].samplerData]*/((AIVolumeTextureImp9*)settedTextures[i].samplerData)->getACDVolumeTexture());
                    acdDev->sampler(i).setEnableComparison(false);
                }
               
                acdDev->sampler(i).setSRGBConversion((settedTextures[i].sRGBTexture == 1));
                
                acdDev->sampler(i).setMagFilter(getACDTextureMagFilter((D3DTEXTUREFILTERTYPE)settedTextures[i].magFilter));
                acdDev->sampler(i).setMinFilter(getACDTextureMinFilter((D3DTEXTUREFILTERTYPE)settedTextures[i].minFilter, (D3DTEXTUREFILTERTYPE)settedTextures[i].mipFilter));

                if (settedTextures[i].minFilter == D3DTEXF_ANISOTROPIC)
                    acdDev->sampler(i).setMaxAnisotropy(settedTextures[i].maxAniso);
                else
                    acdDev->sampler(i).setMaxAnisotropy(1);

                acdDev->sampler(i).setTextureAddressMode(acdlib::ACD_TEXTURE_S_COORD, getACDTextureAddrMode((D3DTEXTUREADDRESS)settedTextures[i].addressU));
                acdDev->sampler(i).setTextureAddressMode(acdlib::ACD_TEXTURE_T_COORD, getACDTextureAddrMode((D3DTEXTUREADDRESS)settedTextures[i].addressV));
                acdDev->sampler(i).setTextureAddressMode(acdlib::ACD_TEXTURE_R_COORD, getACDTextureAddrMode((D3DTEXTUREADDRESS)settedTextures[i].addressW));

            }
        //}

    }

    //  Update state for the vertex texture samplers.  Map the vertex texture samplers to sampler 12 - 15 if those are available.
    for (acdlib::acd_uint s = 0; s < maxVertexSamplers; s++)
    {
        //  Check if a texture is set for the vertex texture sampler.
        if (settedVertexTextures[s].samplerData == NULL)
        {
            //  Do nothing.  The corresponding ACD samplers were disabled by the previous loop setting the pixel samplers.
        }
        else
        {
            //  Remap the vertex texture sampler to the ACD samplers 12 to 15.
            u32bit mappedSampler = s + 12;
            
            //  Check if the ACD sampler wasn't used by the pixel texture samplers.
            if (!usedSamplers[mappedSampler])
            {
                //  Enable the ACD sampler.
                acdDev->sampler(mappedSampler).setEnabled(true);

                //  Check the type of texture attached to the sampler.
                switch(textureTextureType[settedVertexTextures[s].samplerData])
                {
                
                    case AD3D9_TEXTURE2D:

                        acdDev->sampler(mappedSampler).setTexture(((AITextureImp9 *) settedVertexTextures[s].samplerData)->getACDTexture2D());
                        if (((AITextureImp9 *) settedVertexTextures[s].samplerData)->usePCF())
                        {
                            acdDev->sampler(mappedSampler).setEnableComparison(true);
                            acdDev->sampler(mappedSampler).setComparisonFunction(acdlib::ACD_TEXTURE_COMPARISON_GEQUAL);
                        }
                        else 
                        {
                            acdDev->sampler(mappedSampler).setEnableComparison(false);
                        }
                        break;
                        
                    case AD3D9_CUBETEXTURE:
                    
                        acdDev->sampler(mappedSampler).setTexture(((AICubeTextureImp9 *) settedVertexTextures[s].samplerData)->getACDCubeTexture());
                        acdDev->sampler(mappedSampler).setEnableComparison(false);
                        break;
                        
                    case AD3D9_VOLUMETEXTURE:
                    
                        acdDev->sampler(mappedSampler).setTexture(((AIVolumeTextureImp9 *) settedVertexTextures[s].samplerData)->getACDVolumeTexture());
                        acdDev->sampler(mappedSampler).setEnableComparison(false);
                        break;
                        
                    default:
                        panic("AD3D9State", "setACDTextures", "Undefined texture type.");
                        break;
                }
                               
                acdDev->sampler(mappedSampler).setSRGBConversion((settedVertexTextures[s].sRGBTexture == 1));
                
                acdDev->sampler(mappedSampler).setMagFilter(getACDTextureMagFilter((D3DTEXTUREFILTERTYPE) settedVertexTextures[s].magFilter));
                acdDev->sampler(mappedSampler).setMinFilter(getACDTextureMinFilter((D3DTEXTUREFILTERTYPE) settedVertexTextures[s].minFilter,
                                                                                   (D3DTEXTUREFILTERTYPE) settedVertexTextures[s].mipFilter));

                if (settedVertexTextures[s].minFilter == D3DTEXF_ANISOTROPIC)
                    acdDev->sampler(mappedSampler).setMaxAnisotropy(settedVertexTextures[s].maxAniso);
                else
                    acdDev->sampler(mappedSampler).setMaxAnisotropy(1);

                acdDev->sampler(mappedSampler).setTextureAddressMode(acdlib::ACD_TEXTURE_S_COORD, getACDTextureAddrMode((D3DTEXTUREADDRESS) settedVertexTextures[s].addressU));
                acdDev->sampler(mappedSampler).setTextureAddressMode(acdlib::ACD_TEXTURE_T_COORD, getACDTextureAddrMode((D3DTEXTUREADDRESS) settedVertexTextures[s].addressV));
                acdDev->sampler(mappedSampler).setTextureAddressMode(acdlib::ACD_TEXTURE_R_COORD, getACDTextureAddrMode((D3DTEXTUREADDRESS) settedVertexTextures[s].addressW));
            }
            else
            {
                panic("AD3D9State", "setACDTextures", "ACD sampler slot for vertex texture was already taken by a pixel sampler.");
            }
        }
    }
}

void AD3D9State::setACDIndices() 
{
    AIIndexBufferImp9* currentIndexBuffer;

    if (settedIndexBuffer) 
        currentIndexBuffer = settedIndexBuffer;
    else
        panic("AD3D9State", "setACDIndices", "No index buffer is setted, and a indexded draw call is performed");

    
    D3DINDEXBUFFER_DESC indexBufferDescriptor;

    settedIndexBuffer->GetDesc(&indexBufferDescriptor);

    D3D_DEBUG( D3D_DEBUG( cout << "AD3D9State indices lenght: " << indexBufferDescriptor.Size << endl; ) )

    switch (indexBufferDescriptor.Format)
    {
        case D3DFMT_INDEX16:
            acdDev->setIndexBuffer(currentIndexBuffer->getAcdIndexBuffer(), 0, acdlib::ACD_SD_UINT16);
            break;

        case D3DFMT_INDEX32:
            acdDev->setIndexBuffer(currentIndexBuffer->getAcdIndexBuffer(), 0, acdlib::ACD_SD_UINT32);
            break;

        default:
            panic("AD3D9State", "setACDIndices", "Unsupported index buffer format.");
            break;
    }

}


acdlib::ACD_FORMAT AD3D9State::getACDFormat(D3DFORMAT Format) {

    switch (Format)
    {
        case D3DFMT_X8R8G8B8:
            return acdlib::ACD_FORMAT_XRGB_8888;
            break;

        case D3DFMT_A8R8G8B8:
            return acdlib::ACD_FORMAT_ARGB_8888;
            break;

        case D3DFMT_Q8W8V8U8:
            return acdlib::ACD_FORMAT_QWVU_8888;
            break;

        case D3DFMT_DXT1:
            return acdlib::ACD_COMPRESSED_S3TC_DXT1_RGBA;
            break;

        case D3DFMT_DXT3:
            return acdlib::ACD_COMPRESSED_S3TC_DXT3_RGBA;
            break;

        case D3DFMT_DXT5:
            return acdlib::ACD_COMPRESSED_S3TC_DXT5_RGBA;
            break;

        case D3DFMT_A16B16G16R16:
            return acdlib::ACD_FORMAT_ABGR_161616;
            break;

        case D3DFMT_A16B16G16R16F:
            return acdlib::ACD_FORMAT_RGBA16F;
            break;

        case D3DFMT_R32F:
            return acdlib::ACD_FORMAT_R32F;
            break;

        case D3DFMT_A32B32G32R32F:
            return acdlib::ACD_FORMAT_RGBA32F;
            break;

        case D3DFMT_G16R16F:
            return acdlib::ACD_FORMAT_RG16F;
            break;

        case D3DFMT_A8:
            return acdlib::ACD_FORMAT_ALPHA_8;
            break;

        case D3DFMT_L8:
            return acdlib::ACD_FORMAT_LUMINANCE_8;
            break;

        case D3DFMT_A1R5G5B5:
            return acdlib::ACD_FORMAT_S8D24;
            break;

        case D3DFMT_A8L8:
            return acdlib::ACD_FORMAT_LUMINANCE8_ALPHA8;
            break;
            
        case D3DFMT_D24S8:
            return acdlib::ACD_FORMAT_S8D24;
            break;

        case D3DFMT_D16_LOCKABLE:
            return acdlib::ACD_FORMAT_S8D24;
            break;
            
        case D3DFMT_D24X8:
        case D3DFMT_D16:
            return acdlib::ACD_FORMAT_S8D24;
            break;

        case D3DFMT_A8B8G8R8:
            return acdlib::ACD_FORMAT_RGBA_8888; // TODO: OK???
            break;
            
        case D3DFMT_ATI2: // FOURCC: ATI2
            //  Hack for Crysis.
            return acdlib::ACD_COMPRESSED_S3TC_DXT5_RGBA;
            break;

        case D3DFMT_NULL:
            return acdlib::ACD_FORMAT_NULL;
            break;

        case D3DFMT_R5G6B5:
            ///return acdlib::ACD_FORMAT_R5G6B5;
            
            //  Hack for S.T.A.L.K.E.R. as it seems to be using a surface of this type
            //  but with all the write mask set to false, so seems a way to implement
            //  a null surface (depth only passes) with the minimum texel size (that
            //  is supported for a render target in D3D9).
            return acdlib::ACD_FORMAT_NULL;
            break;

        default:
            {
                char message[50];
                sprintf(message, "Unknown format: 0x%X", Format);
                panic("AD3D9State", "getACDFormat", message);
                return static_cast<acdlib::ACD_FORMAT>(0);
            }
            break;
    }
}


acdlib::ACD_BLEND_OPTION AD3D9State::getACDBlendOption(D3DBLEND mode)
{
    switch (mode)
    {
        case D3DBLEND_ZERO:
            return acdlib::ACD_BLEND_ZERO;
            break;

        case D3DBLEND_ONE:
            return acdlib::ACD_BLEND_ONE;
            break;

        case D3DBLEND_SRCCOLOR:
            return acdlib::ACD_BLEND_SRC_COLOR;
            break;

        case D3DBLEND_INVSRCCOLOR:
            return acdlib::ACD_BLEND_INV_SRC_COLOR;
            break;

        case D3DBLEND_SRCALPHA:
            return acdlib::ACD_BLEND_SRC_ALPHA;
            break;

        case D3DBLEND_INVSRCALPHA:
            return acdlib::ACD_BLEND_INV_SRC_ALPHA;
            break;

        case D3DBLEND_DESTALPHA:
            return acdlib::ACD_BLEND_DEST_ALPHA;
            break;

        case D3DBLEND_INVDESTALPHA:
            return acdlib::ACD_BLEND_INV_DEST_ALPHA;
            break;

        case D3DBLEND_DESTCOLOR:
            return acdlib::ACD_BLEND_DEST_COLOR;
            break;

        case D3DBLEND_INVDESTCOLOR:
            return acdlib::ACD_BLEND_INV_DEST_COLOR;
            break;

        case D3DBLEND_SRCALPHASAT:
            return acdlib::ACD_BLEND_SRC_ALPHA_SAT;
            break;

        /*case D3DBLEND_BOTHSRCALPHA:
            return acdlib::;
            break;

        case D3DBLEND_BOTHINVSRCALPHA:
            return acdlib::;
            break;*/

        case D3DBLEND_BLENDFACTOR:
            return acdlib::ACD_BLEND_BLEND_FACTOR;
            break;

        case D3DBLEND_INVBLENDFACTOR:
            return acdlib::ACD_BLEND_INV_BLEND_FACTOR;
            break;

        /*case D3DBLEND_SRCCOLOR2:
            return acdlib::;
            break;

        case D3DBLEND_INVSRCCOLOR2:
            return acdlib::;
            break;*/

        default:
            panic("AD3D9State", "getACDBlendOption", "Undefined blend mode.");
            return static_cast<acdlib::ACD_BLEND_OPTION>(0);
            break;
    }
}

acdlib::ACD_CUBEMAP_FACE AD3D9State::getACDCubeMapFace(D3DCUBEMAP_FACES face)
{
    switch (face)
    {
        case D3DCUBEMAP_FACE_POSITIVE_X:
            return acdlib::ACD_CUBEMAP_POSITIVE_X;
            break;

        case D3DCUBEMAP_FACE_NEGATIVE_X:
            return acdlib::ACD_CUBEMAP_NEGATIVE_X;
            break;

        case D3DCUBEMAP_FACE_POSITIVE_Y:
            return acdlib::ACD_CUBEMAP_POSITIVE_y;
            break;

        case D3DCUBEMAP_FACE_NEGATIVE_Y:
            return acdlib::ACD_CUBEMAP_NEGATIVE_Y;
            break;

        case D3DCUBEMAP_FACE_POSITIVE_Z:
            return acdlib::ACD_CUBEMAP_POSITIVE_Z;
            break;

        case D3DCUBEMAP_FACE_NEGATIVE_Z:
            return acdlib::ACD_CUBEMAP_NEGATIVE_Z;
            break;

        default:
            panic("AD3D9State", "getACDCubeMapFace", "Undefined cubemap face identifier.");
            return static_cast<acdlib::ACD_CUBEMAP_FACE>(0);
            break;
    }
}

acdlib::ACD_BLEND_FUNCTION AD3D9State::getACDBlendOperation(D3DBLENDOP operation)
{
    switch (operation)
    {

        case D3DBLENDOP_ADD:
            return acdlib::ACD_BLEND_ADD;
            break;

        case D3DBLENDOP_SUBTRACT:
            return acdlib::ACD_BLEND_SUBTRACT;
            break;

        case D3DBLENDOP_REVSUBTRACT:
            return acdlib::ACD_BLEND_REVERSE_SUBTRACT;
            break;

        case D3DBLENDOP_MIN:
            return acdlib::ACD_BLEND_MIN;
            break;

        case D3DBLENDOP_MAX:
            return acdlib::ACD_BLEND_MAX;
            break;

        default:
            panic("AD3D9State", "getACDBlendOperation", "Undefined blend operation.");
            return static_cast<acdlib::ACD_BLEND_FUNCTION>(0);
            break;
    }
}

acdlib::ACD_TEXTURE_FILTER AD3D9State::getACDTextureMinFilter(D3DTEXTUREFILTERTYPE Filter, D3DTEXTUREFILTERTYPE FilterMip)
{
    switch (Filter)
    {
        case D3DTEXF_NONE:
            switch (FilterMip)
            {
                case D3DTEXF_NONE:
                    return acdlib::ACD_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST;
                    break;
                case D3DTEXF_POINT:
                    return acdlib::ACD_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST;
                    break;
                case D3DTEXF_LINEAR:
                    return acdlib::ACD_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR;
                    break;
                default:
                    panic("AD3D9State", "getACDTextureMinFilter", "Undefined mip filter mode for filter mode NONE.");
                    return static_cast<acdlib::ACD_TEXTURE_FILTER>(0);
                    break;
            }
            break;

        case D3DTEXF_POINT:
            switch (FilterMip)
            {
                case D3DTEXF_NONE:
                    return acdlib::ACD_TEXTURE_FILTER_NEAREST;
                    break;
                case D3DTEXF_POINT:
                    return acdlib::ACD_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST;
                    break;
                case D3DTEXF_LINEAR:
                    return acdlib::ACD_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR;
                    break;
                default:
                    panic("AD3D9State", "getACDTextureMinFilter", "Undefined mip filter mode for filter mode POINT.");
                    return static_cast<acdlib::ACD_TEXTURE_FILTER>(0);
                    break;
            }
            break;

        case D3DTEXF_LINEAR:
            switch (FilterMip)
            {
                case D3DTEXF_NONE:
                    return acdlib::ACD_TEXTURE_FILTER_LINEAR;
                    break;
                case D3DTEXF_POINT:
                    return acdlib::ACD_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST;
                    break;
                case D3DTEXF_LINEAR:
                    return acdlib::ACD_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
                    break;
                default:
                    panic("AD3D9State", "getACDTextureMinFilter", "Undefined mip filter mode for filter mode LINEAR.");
                    return static_cast<acdlib::ACD_TEXTURE_FILTER>(0);
                    break;
            }
            break;

        case D3DTEXF_ANISOTROPIC:
            switch (FilterMip)
            {
                case D3DTEXF_NONE:
                    return acdlib::ACD_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST;
                    break;
                case D3DTEXF_POINT:
                    return acdlib::ACD_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST;
                    break;
                case D3DTEXF_LINEAR:
                    return acdlib::ACD_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
                    break;
                default:
                    panic("AD3D9State", "getACDTextureMinFilter", "Undefined mip filter mode for filter mode NEAREST.");
                    return static_cast<acdlib::ACD_TEXTURE_FILTER>(0);
                    break;
            }
            break;

        default:
            panic("AD3D9State", "getACDTextureMinFilter", "Undefined filter mode.");
            return static_cast<acdlib::ACD_TEXTURE_FILTER>(0);
            break;
    }
}

acdlib::ACD_TEXTURE_FILTER AD3D9State::getACDTextureMagFilter(D3DTEXTUREFILTERTYPE Filter)
{
    switch (Filter)
    {
        case D3DTEXF_NONE:
            return acdlib::ACD_TEXTURE_FILTER_NEAREST;
            break;

        case D3DTEXF_POINT:
            return acdlib::ACD_TEXTURE_FILTER_NEAREST;
            break;

        case D3DTEXF_LINEAR:
            return acdlib::ACD_TEXTURE_FILTER_LINEAR;
            break;

        case D3DTEXF_ANISOTROPIC:
            return acdlib::ACD_TEXTURE_FILTER_LINEAR;
            break;

        default:
            panic("AD3D9State", "getACDTextureMagFilter", "Undefined filter mode.");
            return static_cast<acdlib::ACD_TEXTURE_FILTER>(0);
            break;
    }
}

acdlib::acd_uint AD3D9State::getElementLength(D3DDECLTYPE Type)
{
    switch (Type)
    {
        case D3DDECLTYPE_FLOAT1:
            return 4;
            break;

        case D3DDECLTYPE_FLOAT2:
            return 8;
            break;

        case D3DDECLTYPE_FLOAT3:
            return 12;
            break;

        case D3DDECLTYPE_FLOAT4:
            return 16;
            break;

        case D3DDECLTYPE_D3DCOLOR:
            return 4;
            break;

        case D3DDECLTYPE_SHORT2:
            return 4;
            break;

        case D3DDECLTYPE_SHORT4N:
            return 4;
            break;

        case D3DDECLTYPE_UBYTE4N:
            return 4;
            break;

        default:
            panic("AD3D9State", "getElementLength", "Unsupported vertex declaration type.");
            return 0;
            break;
    }
}

void AD3D9State::getTypeAndComponents(D3DDECLTYPE Type, acdlib::acd_uint* components, acdlib::ACD_STREAM_DATA* acdType)
{
    switch (Type)
    {
        case D3DDECLTYPE_FLOAT1:
            *components = 1;
            *acdType = acdlib::ACD_SD_FLOAT32;
            break;

        case D3DDECLTYPE_FLOAT2:
            *components = 2;
            *acdType = acdlib::ACD_SD_FLOAT32;
            break;

        case D3DDECLTYPE_FLOAT3:
            *components = 3;
            *acdType = acdlib::ACD_SD_FLOAT32;
            break;

        case D3DDECLTYPE_FLOAT4:
            *components = 4;
            *acdType = acdlib::ACD_SD_FLOAT32;
            break;

        case D3DDECLTYPE_D3DCOLOR:
            *components = 4;
            *acdType = acdlib::ACD_SD_INV_UNORM8;
            break;

        case D3DDECLTYPE_SHORT2:
            *components = 2;
            *acdType = acdlib::ACD_SD_SINT16;
            break;

        case D3DDECLTYPE_SHORT2N:
            *components = 2;
            *acdType = acdlib::ACD_SD_SNORM16;
            break;

        case D3DDECLTYPE_SHORT4:
            *components = 4;
            *acdType = acdlib::ACD_SD_SINT16;
            break;
            
        case D3DDECLTYPE_SHORT4N:
            *components = 4;
            *acdType = acdlib::ACD_SD_SNORM16;
            break;

        case D3DDECLTYPE_UBYTE4N:
            *components = 4;
            *acdType = acdlib::ACD_SD_UNORM8;
            break;

        case D3DDECLTYPE_UBYTE4:
            *components = 4;
            *acdType = acdlib::ACD_SD_UINT8;
            break;

        case D3DDECLTYPE_FLOAT16_2:
            *components = 2;
            *acdType = acdlib::ACD_SD_FLOAT16;
            break;

        case D3DDECLTYPE_FLOAT16_4:
            *components = 4;
            *acdType = acdlib::ACD_SD_FLOAT16;
            break;

        default:
            cout << Type << endl;
            panic("AD3D9State", "getTypeAndComponents", "Undefined vertex declaration type.");
            break;
    }
}

acdlib::ACD_PRIMITIVE AD3D9State::getACDPrimitiveType(D3DPRIMITIVETYPE primitive)
{
    switch (primitive)
    {
        case D3DPT_POINTLIST:
            return acdlib::ACD_POINTS;
            break;

        case D3DPT_LINELIST:
            return acdlib::ACD_LINES;
            break;

        case D3DPT_LINESTRIP:
            return acdlib::ACD_LINE_STRIP;
            break;

        case D3DPT_TRIANGLELIST:
            return acdlib::ACD_TRIANGLES;
            break;

        case D3DPT_TRIANGLESTRIP:
            return acdlib::ACD_TRIANGLE_STRIP;
            break;

        case D3DPT_TRIANGLEFAN:
            return acdlib::ACD_TRIANGLE_FAN;
            break;

        default:
            panic("AD3D9State", "getACDPrimitiveType", "Unsupported primitive.");
            return static_cast<acdlib::ACD_PRIMITIVE>(0);
            break;
    }
}

acdlib::ACD_TEXTURE_ADDR_MODE AD3D9State::getACDTextureAddrMode(D3DTEXTUREADDRESS address)
{
    switch (address)
    {
        case D3DTADDRESS_CLAMP:
            //return acdlib::ACD_TEXTURE_ADDR_CLAMP;
            return acdlib::ACD_TEXTURE_ADDR_CLAMP_TO_EDGE;
            break;

        case D3DTADDRESS_WRAP:
            return acdlib::ACD_TEXTURE_ADDR_REPEAT;
            break;

        case D3DTADDRESS_BORDER:
            return acdlib::ACD_TEXTURE_ADDR_CLAMP_TO_BORDER;
            break;

        case D3DTADDRESS_MIRROR:
            return acdlib::ACD_TEXTURE_ADDR_MIRRORED_REPEAT;
            break;

        default:
            panic("AD3D9State", "getACDTextureAddrMode", "Undefined texture address mode.");
            return static_cast<acdlib::ACD_TEXTURE_ADDR_MODE>(0);
            break;
    }
}

acdlib::ACD_STENCIL_OP AD3D9State::getACDStencilOperation(D3DSTENCILOP operation)
{
    switch (operation)
    {
        case D3DSTENCILOP_KEEP:
            return acdlib::ACD_STENCIL_OP_KEEP;
            break;

        case D3DSTENCILOP_ZERO:
            return acdlib::ACD_STENCIL_OP_ZERO;
            break;

        case D3DSTENCILOP_REPLACE:
            return acdlib::ACD_STENCIL_OP_REPLACE;
            break;

        case D3DSTENCILOP_INCRSAT:
            return acdlib::ACD_STENCIL_OP_INCR_SAT;
            break;

        case D3DSTENCILOP_DECRSAT:
            return acdlib::ACD_STENCIL_OP_DECR_SAT;
            break;

        case D3DSTENCILOP_INVERT:
            return acdlib::ACD_STENCIL_OP_INVERT;
            break;

        case D3DSTENCILOP_INCR:
            return acdlib::ACD_STENCIL_OP_INCR;
            break;

        case D3DSTENCILOP_DECR:
            return acdlib::ACD_STENCIL_OP_DECR;
            break;

        default:
            panic("AD3D9State", "getACDStencil", "Undefined stencil operation.");
            return static_cast<acdlib::ACD_STENCIL_OP>(0);
            break;

    }
}

acdlib::ACD_COMPARE_FUNCTION AD3D9State::getACDCompareFunction(D3DCMPFUNC func)
{
    switch (func)
    {
        case D3DCMP_NEVER:
            return acdlib::ACD_COMPARE_FUNCTION_NEVER;
            break;

        case D3DCMP_LESS:
            return acdlib::ACD_COMPARE_FUNCTION_LESS;
            break;

        case D3DCMP_EQUAL:
            return acdlib::ACD_COMPARE_FUNCTION_EQUAL;
            break;

        case D3DCMP_LESSEQUAL:
            return acdlib::ACD_COMPARE_FUNCTION_LESS_EQUAL;
            break;

        case D3DCMP_GREATER:
            return acdlib::ACD_COMPARE_FUNCTION_GREATER;
            break;

        case D3DCMP_NOTEQUAL:
            return acdlib::ACD_COMPARE_FUNCTION_NOT_EQUAL;
            break;

        case D3DCMP_GREATEREQUAL:
            return acdlib::ACD_COMPARE_FUNCTION_GREATER_EQUAL;
            break;

        case D3DCMP_ALWAYS:
            return acdlib::ACD_COMPARE_FUNCTION_ALWAYS;
            break;

        default:
            panic("AD3D9State", "getACDCompareFunction", "Undefined compare function.");
            return static_cast<acdlib::ACD_COMPARE_FUNCTION>(0);
            break;
    }
}

void AD3D9State::redefineACDBlend()
{

    for (u32bit i = 0; i < MAX_RENDER_TARGETS; i++) {
        // Enable/Disable Alpha Blending state
        acdDev->blending().setEnable(i, alphaBlend);

        if (alphaBlend) {

            acdDev->blending().setSrcBlend(i, srcBlend);
            acdDev->blending().setDestBlend(i, dstBlend);
            acdDev->blending().setBlendFunc(i, blendOp);

            if (separateAlphaBlend) {

                acdDev->blending().setSrcBlendAlpha(i, srcBlendAlpha);
                acdDev->blending().setDestBlendAlpha(i, dstBlendAlpha);
                //acdDev->blending().setBlendFuncAlpha(0, blendOpAlpha);

            }
            else {

                acdDev->blending().setSrcBlendAlpha(i, srcBlend);
                acdDev->blending().setDestBlendAlpha(i, dstBlend);
                //acdDev->blending().setBlendFuncAlpha(0, blendOp);

            }

        }
    }

}

void AD3D9State::redefineACDStencil() {

    // Enable/Disable Stencil
    acdDev->zStencil().setStencilEnabled(stencilEnabled || twoSidedStencilEnabled);

    if (stencilEnabled) {
        acdDev->zStencil().setStencilOp(acdlib::ACD_FACE_FRONT, stencilFail, stencilZFail, stencilPass);
        acdDev->zStencil().setStencilFunc(acdlib::ACD_FACE_FRONT, stencilFunc, stencilRef, stencilMask);
    }
    else {
        acdDev->zStencil().setStencilOp(acdlib::ACD_FACE_FRONT, acdlib::ACD_STENCIL_OP_KEEP, acdlib::ACD_STENCIL_OP_KEEP, acdlib::ACD_STENCIL_OP_KEEP);
        acdDev->zStencil().setStencilFunc(acdlib::ACD_FACE_FRONT, acdlib::ACD_COMPARE_FUNCTION_ALWAYS, 0, 0xFFFFFFFF);
    }

    if (twoSidedStencilEnabled) {
        acdDev->zStencil().setStencilOp(acdlib::ACD_FACE_BACK, ccwStencilFail, ccwStencilZFail, ccwStencilPass);
        acdDev->zStencil().setStencilFunc(acdlib::ACD_FACE_BACK, ccwStencilFunc, stencilRef, stencilMask);
    } else {
        acdDev->zStencil().setStencilOp(acdlib::ACD_FACE_BACK, acdlib::ACD_STENCIL_OP_KEEP, acdlib::ACD_STENCIL_OP_KEEP, acdlib::ACD_STENCIL_OP_KEEP);
        acdDev->zStencil().setStencilFunc(acdlib::ACD_FACE_BACK, acdlib::ACD_COMPARE_FUNCTION_ALWAYS, 0, 0xFFFFFFFF);
    }

    acdDev->zStencil().setStencilUpdateMask(stencilWriteMask);

}

void AD3D9State::createFVFVertexDeclaration() {

    fvfVertexDeclaration.clear();

    DWORD offset = 0;

    if (settedFVF & D3DFVF_XYZ) {

        D3D_DEBUG( D3D_DEBUG( cout << "AD3D9State FVF position." << endl; ) )

        D3DVERTEXELEMENT9 tmpElem;

        tmpElem.Stream = 0;
        tmpElem.Offset = offset;
        tmpElem.Type = D3DDECLTYPE_FLOAT3;
        tmpElem.Method = D3DDECLMETHOD_DEFAULT;
        tmpElem.Usage = D3DDECLUSAGE_POSITION;
        tmpElem.UsageIndex = 0;

        fvfVertexDeclaration.push_back(tmpElem);
        offset += 12;

    }

    if (settedFVF & D3DFVF_NORMAL) {

        D3D_DEBUG( D3D_DEBUG( cout << "AD3D9State FVF normal." << endl; ) )

        D3DVERTEXELEMENT9 tmpElem;

        tmpElem.Stream = 0;
        tmpElem.Offset = offset;
        tmpElem.Type = D3DDECLTYPE_FLOAT3;
        tmpElem.Method = D3DDECLMETHOD_DEFAULT;
        tmpElem.Usage = D3DDECLUSAGE_NORMAL;
        tmpElem.UsageIndex = 0;

        fvfVertexDeclaration.push_back(tmpElem);
        offset += 12;

    }

    if (settedFVF & D3DFVF_DIFFUSE) {

        D3D_DEBUG( D3D_DEBUG( cout << "AD3D9State FVF diffuse." << endl; ) )

        D3DVERTEXELEMENT9 tmpElem;

        tmpElem.Stream = 0;
        tmpElem.Offset = offset;
        tmpElem.Type = D3DDECLTYPE_D3DCOLOR;
        tmpElem.Method = D3DDECLMETHOD_DEFAULT;
        tmpElem.Usage = D3DDECLUSAGE_COLOR;
        tmpElem.UsageIndex = 0;

        fvfVertexDeclaration.push_back(tmpElem);
        offset += 4;
    }

    if (settedFVF & D3DFVF_TEX1) {

        D3D_DEBUG( D3D_DEBUG( cout << "AD3D9State FVF TEX1." << endl; ) )

        D3DVERTEXELEMENT9 tmpElem;

        tmpElem.Stream = 0;
        tmpElem.Offset = offset;
        tmpElem.Type = D3DDECLTYPE_FLOAT2;
        tmpElem.Method = D3DDECLMETHOD_DEFAULT;
        tmpElem.Usage = D3DDECLUSAGE_TEXCOORD;
        tmpElem.UsageIndex = 0;

        fvfVertexDeclaration.push_back(tmpElem);
        offset += 8;
    }

}

void AD3D9State::setRenderTarget(DWORD RenderTargetIndex, AISurfaceImp9 *surface)
{
    //  Check for NULL (disable render target).
    if (surface == NULL)
    {
        acdDev->setRenderTarget(RenderTargetIndex, NULL);
        currentRenderSurface[RenderTargetIndex] = surface;
        return;
    }

    D3DSURFACE_DESC sDesc;

    surface->GetDesc(&sDesc);

    // Reset the viewport to the full render target (D3D SDK dixit).  Only for render target 0!!
    if (RenderTargetIndex == 0)
        acdDev->rast().setViewport(0, 0, sDesc.Width, sDesc.Height);
    
    if (sDesc.Format == D3DFMT_NULL)
    {
        acdDev->setRenderTarget(RenderTargetIndex, NULL);
        currentRenderSurface[RenderTargetIndex] = surface;
        return;
    }

    acdDev->setRenderTarget(RenderTargetIndex, surface->getACDRenderTarget());

    currentRenderSurface[RenderTargetIndex] = surface;
    
    /*map<AISurfaceImp9*, acdlib::ACDRenderTarget *>::iterator it;

    it = renderTargets.find(surface);

    if (it != renderTargets.end())
    {
        //  Set as the new render target.
        acdDev->setRenderTarget(it->second);

        //  Set as the new render surface.
        currentRenderSurface = surface;
    }
    else
    {
        //  Create a new render target from the provided texture mipmap.
        map<AISurfaceImp9*, TextureType>::iterator it;
        it = mipLevelsTextureType.find(surface);

        //  Check if surface was defined.
        if (it != mipLevelsTextureType.end())
        {
            //  Check if texture type is supported.
            if (it->second != AD3D9_TEXTURE2D)
                panic("AD3D9State", "setRenderTarget", "Render surfaces are only implemented for 2D textures.");

            //  Get the ACD Texture2D associated with the surface.
            AITextureImp9 *tex2D = mipLevelsTextures2D[surface];
            acdlib::ACDTexture2D *acdTex2D = textures2D[tex2D];

            //  Get the miplevel for the defined surface.
            acdlib::acd_uint mipLevel = mipLevelsLevels[surface];

            //  Create the new render target.
            acdlib::ACD_RT_DESC rtDesc;
            rtDesc.format = acdTex2D->getFormat(mipLevel);
            rtDesc.dimension = acdlib::ACD_RT_DIMENSION_TEXTURE2D;
            rtDesc.texture2D.mipmap = mipLevel;
            acdlib::ACDRenderTarget *newRT = acdDev->createRenderTarget(acdTex2D, acdlib::ACD_RT_DIMENSION_TEXTURE2D, static_cast<acdlib::ACD_CUBEMAP_FACE>(0), mipLevel);

            //  Add the new render target.
            renderTargets[surface] = newRT;

            //  Set as new render target.
            acdDev->setRenderTarget(newRT);

            //  Set as new render surface.
            currentRenderSurface = surface;
        }
        else
        {
            panic("AD3D9State", "setRenderTarget", "Setting an undefined surface as render target.");
        }
    }*/
}


void AD3D9State::addRenderTarget(AISurfaceImp9* rt, UINT width , UINT height , D3DFORMAT format) {

    D3DSURFACE_DESC sDesc;

    rt->GetDesc(&sDesc);

    if (sDesc.Format == D3DFMT_NULL)
        return;

    acdlib::ACDTexture2D *tex2D = acdDev->createTexture2D();

    // Get mip level size
    UINT size = getSurfaceSize(width, height, format);

    //BYTE* data = new BYTE[size];

    //tex2D->setData(0, width, height, getACDFormat(format), 0, data, 0);
    tex2D->setData(0, width, height, getACDFormat(format), 0, NULL, 0);

    //renderTargetSurface[rt] = tex2D;
    //mipLevelsTextureType[rt] = AD3D9_RENDERTARGET;

    //  Create the new render target.
    /*acdlib::ACD_RT_DESC rtDesc;
    rtDesc.format = getACDFormat(format);
    rtDesc.dimension = acdlib::ACD_RT_DIMENSION_TEXTURE2D;
    rtDesc.texture2D.mipmap = 0;*/
    acdlib::ACDRenderTarget *newRT = acdDev->createRenderTarget(tex2D, acdlib::ACD_RT_DIMENSION_TEXTURE2D, static_cast<acdlib::ACD_CUBEMAP_FACE>(0), 0);

    //  Add the new render target.
    //renderTargets[rt] = newRT;

    //delete[] data;

    rt->setAsRenderTarget(newRT);

}

AISurfaceImp9 *AD3D9State::getRenderTarget(DWORD RenderTargetIndex)
{
    return currentRenderSurface[RenderTargetIndex];
}


void AD3D9State::setZStencilBuffer(AISurfaceImp9 *surface)
{
    currentZStencilSurface = surface;

    //  Check for NULL TODO: Disable depth stencil operation
    if (surface == NULL)
        acdDev->setZStencilBuffer(NULL);
    else
        acdDev->setZStencilBuffer(surface->getACDRenderTarget());

    /*map<AISurfaceImp9*, acdlib::ACDRenderTarget *>::iterator it;

    it = renderTargets.find(surface);

    if (it != renderTargets.end())
    {
        //  Set as the new z stencil buffer.
        acdDev->setZStencilBuffer(it->second);

        //  Set as the new z stencil surface.
        currentZStencilSurface = surface;
    }
    else
    {
        //  Create a new render target from the provided texture mipmap.
        map<AISurfaceImp9*, TextureType>::iterator it;
        it = mipLevelsTextureType.find(surface);

        //  Check if surface was defined.
        if (it != mipLevelsTextureType.end())
        {
            //  Check if texture type is supported.
            if (it->second != AD3D9_TEXTURE2D)
                panic("AD3D9State", "setRenderTarget", "Render surfaces are only implemented for 2D textures.");

            //  Get the ACD Texture2D associated with the surface.
            AITextureImp9 *tex2D = mipLevelsTextures2D[surface];
            acdlib::ACDTexture2D *acdTex2D = textures2D[tex2D];

            //  Get the miplevel for the defined surface.
            acdlib::acd_uint mipLevel = mipLevelsLevels[surface];

            //  Create the new render target.
            acdlib::ACD_RT_DESC rtDesc;
            rtDesc.format = acdTex2D->getFormat(mipLevel);
            rtDesc.dimension = acdlib::ACD_RT_DIMENSION_TEXTURE2D;
            rtDesc.texture2D.mipmap = mipLevel;
            acdlib::ACDRenderTarget *newRT = acdDev->createRenderTarget(acdTex2D, acdlib::ACD_RT_DIMENSION_TEXTURE2D, static_cast<acdlib::ACD_CUBEMAP_FACE>(0), mipLevel);

            //  Add the new render target.
            renderTargets[surface] = newRT;

            //  Set as new render target.
            acdDev->setZStencilBuffer(newRT);

            //  Set as new render surface.
            currentZStencilSurface = surface;
        }
        else
        {
            panic("AD3D9State", "setZStencilBuffer", "Setting an undefined surface as z stencil buffer.");
        }
    }*/
}

AISurfaceImp9 *AD3D9State::getZStencilBuffer()
{
    return currentZStencilSurface;
}


