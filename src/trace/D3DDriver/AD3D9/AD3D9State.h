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

#ifndef AD3D9STATE_H
#define AD3D9STATE_H

#include "ACDDevice.h"
#include "ACDSampler.h"
#include "ACDTextureCubeMap.h"
#include "ACDZStencilStage.h"
#include "ACDBlendingStage.h"
#include <vector>
#include "ShaderTranslator.h"
#include "FFShaderGenerator.h"
#include "D3DTrace.h"

class AIVertexBufferImp9;
class AIIndexBufferImp9;
class AIVertexDeclarationImp9;
class AIVertexShaderImp9;
class AIPixelShaderImp9;
class AISurfaceImp9;
class AIVolumeImp9;
class AITextureImp9;
class AIBaseTextureImp9;
class AICubeTextureImp9;
class AIVolumeTextureImp9;
class AIDeviceImp9;

//
//  Enables a library hack to reduce the resolution of render targets.
//
//#define LOWER_RESOLUTION_HACK

typedef enum {
    AD3D9_NONDEFINED,
    AD3D9_TEXTURE2D,
    AD3D9_CUBETEXTURE,
    AD3D9_RENDERTARGET,
    AD3D9_VOLUMETEXTURE
} TextureType;

class AD3D9State {

public:
    
    static AD3D9State &instance();

    void initialize(AIDeviceImp9 *device, UINT width, UINT height);
    void destroy();
    
    void setD3DTrace(D3DTrace *trace);
    bool isPreloadCall();

    //acdlib::ACDDevice* getACDDevice();

    AISurfaceImp9 *getDefaultRenderSurface();
    acdlib::ACDTexture2D* createTexture2D();
    acdlib::ACDTextureCubeMap* createCubeMap();
    acdlib::ACDTexture3D* createVolumeTexture();
    acdlib::ACDBuffer* createBuffer(UINT Length);
    acdlib::ACDShaderProgram* createShaderProgram();
    acdlib::ACDRenderTarget* createRenderTarget(acdlib::ACDTexture* resource, const acdlib::ACD_RT_DIMENSION rtdimension, D3DCUBEMAP_FACES face, UINT mipmap);

    //void addVertexShader(AIVertexShaderImp9* vs, CONST DWORD* func); // ok
    //void addPixelShader(AIPixelShaderImp9* ps, CONST DWORD* func); // ok

    //void addVertexBuffer(AIVertexBufferImp9* vb, UINT size);
    //void updateVertexBuffer(AIVertexBufferImp9* vb, UINT offset, UINT size, BYTE* data);
    //void addVertexDeclaration(AIVertexDeclarationImp9* vd, CONST D3DVERTEXELEMENT9* elements);

    //void addIndexBuffer(AIIndexBufferImp9* ib, UINT size); // ok
    //void updateIndexBuffer(AIIndexBufferImp9* ib, UINT offset, UINT size, BYTE* data); // ok
    void addTypeTexture(AIBaseTextureImp9* tex, TextureType type);

    //void addTexture2D(AITextureImp9* tex, DWORD usage);
    //void addCubeTexture(AICubeTextureImp9* tex);
    //void addVolumeTexture(AIVolumeTextureImp9* tex);
    //void addMipLevel(AITextureImp9* tex, AISurfaceImp9* mip, UINT mipLevel, UINT Width, UINT Height, D3DFORMAT format);
    //void addCubeTextureMipLevel(AICubeTextureImp9* tex, AISurfaceImp9* mip, D3DCUBEMAP_FACES face, UINT mipLevel, UINT width, D3DFORMAT format);
    //void addVolumeLevel(AIVolumeTextureImp9* tex, AIVolumeImp9* vol, UINT mipLevel, UINT width, UINT height, UINT depth, D3DFORMAT format);
    //void updateSurface(AISurfaceImp9* surf, CONST RECT* rect, D3DFORMAT format, BYTE* data);
    //void updateVolume(AIVolumeImp9* vol, CONST D3DBOX* pBox, D3DFORMAT format, BYTE* data);
    void copySurface(AISurfaceImp9* srcSurf , CONST RECT* srcRect , AISurfaceImp9* destSurf , CONST RECT* destRect , D3DTEXTUREFILTERTYPE Filter);

    void setVertexShader(AIVertexShaderImp9* vs);
    void setPixelShader(AIPixelShaderImp9* ps);

    void setVertexShaderConstant(UINT sr, CONST float* data, UINT vectorCount);
    void setVertexShaderConstant(UINT sr, CONST int* data, UINT vectorCount);
    void setVertexShaderConstantBool(UINT sr, CONST BOOL *data, UINT vectorCount);
    void setPixelShaderConstant(UINT sr, CONST float* data, UINT vectorCount);
    void setPixelShaderConstant(UINT sr, CONST int* data, UINT vectorCount);
    void setPixelShaderConstantBool(UINT sr, CONST BOOL *data, UINT vectorCount);

    void setVertexBuffer(AIVertexBufferImp9* vb, UINT str, UINT offset, UINT stride);
    AIVertexBufferImp9* getVertexBuffer(UINT str, UINT& offset, UINT& stride);
    void setVertexDeclaration(AIVertexDeclarationImp9* vd);
    AIVertexDeclarationImp9* getVertexDeclaration();
    void setFVF(DWORD FVF);

    void setIndexBuffer(AIIndexBufferImp9* ib);
    AIIndexBufferImp9* getIndexBuffer();

    void setTexture(AIBaseTextureImp9* tex, UINT samp);
    void setLOD(AIBaseTextureImp9* tex, DWORD lod);
    void setSamplerState(UINT samp, D3DSAMPLERSTATETYPE type, DWORD value);

    void setFixedFunctionState(D3DRENDERSTATETYPE state, DWORD value);    
    void setBlendingState(D3DRENDERSTATETYPE state , DWORD value);
    void setFogState(D3DRENDERSTATETYPE state , DWORD value);
    void setAlphaTestState(D3DRENDERSTATETYPE state , DWORD value);
    void setZState(D3DRENDERSTATETYPE state , DWORD value);
    void setStencilState(D3DRENDERSTATETYPE state , DWORD value);
    void setColorWriteEnable(DWORD RenderTargetIndex, DWORD value);
    void setColorSRGBWrite(DWORD value);
    void setCullMode(DWORD value);
    void enableScissorTest(DWORD value);
    void scissorRect(UINT x1, UINT y1, UINT x2, UINT y2);

    void setLight(DWORD Index, CONST D3DLIGHT9* pLight);
    void enableLight(DWORD Index, BOOL bEnable);
    void setMaterial(CONST D3DMATERIAL9* pMaterial);
    void setTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX * pMatrix);
    void setTextureStage(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
        
    void setFreqType(UINT stream, UINT type);
    void setFreqValue(UINT stream, UINT value);

    void clear(DWORD count, CONST D3DRECT* rects, DWORD flags, D3DCOLOR color, float z, DWORD stencil);

    void setViewport(DWORD x, DWORD y, DWORD width, DWORD height, float minZ, float maxZ);

    void setRenderTarget(DWORD RenderTargetIndex, AISurfaceImp9 *surface);
    void setZStencilBuffer(AISurfaceImp9 *surface);
    AISurfaceImp9 *getRenderTarget(DWORD RenderTargetIndex);
    AISurfaceImp9 *getZStencilBuffer();
    void addRenderTarget(AISurfaceImp9* rt, UINT width , UINT height , D3DFORMAT format);
    
    void draw(D3DPRIMITIVETYPE type, UINT start, UINT count);
    void drawIndexed(D3DPRIMITIVETYPE type, INT baseVertexIndex, UINT minVertexIndex, UINT numVertices, UINT start, UINT count);
int getBatchCounter ();
    void swapBuffers();

    acdlib::ACD_FORMAT getACDFormat(D3DFORMAT Format);
    acdlib::ACD_CUBEMAP_FACE getACDCubeMapFace(D3DCUBEMAP_FACES face);

private:

    static const acdlib::acd_uint MAX_VERTEX_SHADER_CONSTANTS = 256;
    static const acdlib::acd_uint MAX_PIXEL_SHADER_CONSTANTS = 256;
    static const acdlib::acd_uint MAX_RENDER_TARGETS = 4;

    UINT batchCount;
    
    AD3D9State();
    
    D3DTrace *d3dTrace;

    acdlib::acd_uint getElementLength(D3DDECLTYPE Type);
    void getTypeAndComponents(D3DDECLTYPE Type, acdlib::acd_uint* components, acdlib::ACD_STREAM_DATA* acdType);
    acdlib::ACD_TEXTURE_FILTER getACDTextureMinFilter(D3DTEXTUREFILTERTYPE Filter, D3DTEXTUREFILTERTYPE FilterMip);
    acdlib::ACD_TEXTURE_FILTER getACDTextureMagFilter(D3DTEXTUREFILTERTYPE Filter);
    acdlib::ACD_BLEND_OPTION getACDBlendOption(D3DBLEND mode);
    acdlib::ACD_BLEND_FUNCTION getACDBlendOperation(D3DBLENDOP operation);
    acdlib::ACD_PRIMITIVE getACDPrimitiveType(D3DPRIMITIVETYPE primitive);
    acdlib::ACD_TEXTURE_ADDR_MODE getACDTextureAddrMode(D3DTEXTUREADDRESS address);
    acdlib::ACD_STENCIL_OP getACDStencilOperation(D3DSTENCILOP operation);
    acdlib::ACD_COMPARE_FUNCTION getACDCompareFunction(D3DCMPFUNC func);
    void createFVFVertexDeclaration();

    void redefineACDBlend();
    void redefineACDStencil();

    bool setACDShaders(NativeShader* &nativeVertexShader);
    void setACDStreams(NativeShader* nativeVertexShader, UINT count, UINT maxVertex);
    void setACDTextures();
    void setACDIndices();

    // ACD main device
    acdlib::ACDDevice* acdDev;

    // Vertex shaders created
    //std::map<AIVertexShaderImp9*, DWORD*> vertexShaders;
    // Pixel shaders created
    //std::map<AIPixelShaderImp9*, DWORD*> pixelShaders;

    // Vertex buffers created
    //std::map<AIVertexBufferImp9*, acdlib::ACDBuffer*> vertexBuffers;

    // Vertex declarations created
    //std::map<AIVertexDeclarationImp9*, std::vector<D3DVERTEXELEMENT9> > vertexDeclarations;

    // Index buffers created
    //std::map<AIIndexBufferImp9*, acdlib::ACDBuffer*> indexBuffers;

    // Textures 2D created
    //std::map<AITextureImp9*, acdlib::ACDTexture2D*> textures2D;
    // Textures Cube Map created
    //std::map<AICubeTextureImp9*, acdlib::ACDTextureCubeMap*> cubeTextures;
    // Volume Textures created
    //std::map<AIVolumeTextureImp9*, acdlib::ACDTexture3D*> volumeTextures;
    // Texture 2D mip levels created
    //std::map<AISurfaceImp9*, AITextureImp9*> mipLevelsTextures2D;
    // Texture Cube Map mip levels created
    //std::map<AISurfaceImp9*, AICubeTextureImp9*> mipLevelsCubeTextures;
    // Mip level number of each mip level
    //std::map<AISurfaceImp9*, UINT> mipLevelsLevels;
    // Face of each mip level
    //std::map<AISurfaceImp9*, acdlib::ACD_CUBEMAP_FACE> mipLevelsFaces;
    
    //std::map<AIVolumeImp9*, AIVolumeTextureImp9*> mipVolumesVolumeTextures;

    //std::map<AIVolumeImp9*, UINT> mipVolumesLevels;

    //std::map<AISurfaceImp9*, acdlib::ACDTexture2D*> renderTargetSurface;


    //std::map<AISurfaceImp9*, TextureType> mipLevelsTextureType;
    std::map<AIBaseTextureImp9*, TextureType> textureTextureType;

    AISurfaceImp9 *defaultRenderSurface;
    AISurfaceImp9 *defaultZStencilSurface;
    
    AISurfaceImp9 *currentRenderSurface[MAX_RENDER_TARGETS];
    AISurfaceImp9 *currentZStencilSurface;
    
    //std::map<AISurfaceImp9 *, acdlib::ACDRenderTarget *> renderTargets;
    
    AIVertexShaderImp9* settedVertexShader;
    AIPixelShaderImp9* settedPixelShader;
    
    acdlib::acd_float *settedVertexShaderConstants[MAX_VERTEX_SHADER_CONSTANTS];
    acdlib::acd_bool settedVertexShaderConstantsTouched[MAX_VERTEX_SHADER_CONSTANTS];

    acdlib::acd_int *settedVertexShaderConstantsInt[16];
    acdlib::acd_bool settedVertexShaderConstantsIntTouched[16];

    acdlib::acd_bool settedVertexShaderConstantsBool[16];
    acdlib::acd_bool settedVertexShaderConstantsBoolTouched[16];

    acdlib::acd_float *settedPixelShaderConstants[MAX_PIXEL_SHADER_CONSTANTS];
    acdlib::acd_bool settedPixelShaderConstantsTouched[MAX_VERTEX_SHADER_CONSTANTS];

    acdlib::acd_int *settedPixelShaderConstantsInt[16];
    acdlib::acd_bool settedPixelShaderConstantsIntTouched[16];

    acdlib::acd_bool settedPixelShaderConstantsBool[16];
    acdlib::acd_bool settedPixelShaderConstantsBoolTouched[16];

    static const u32bit BOOLEAN_CONSTANT_START = 256;
    static const u32bit INTEGER_CONSTANT_START = 272;

    typedef struct stDesc{

        AIVertexBufferImp9* streamData;
        UINT offset;
        UINT stride;
        UINT freqType;
        UINT freqValue;

        stDesc():
            streamData(NULL),
            offset(0),
            stride(0),
            freqType(0),
            freqValue(1) {}

    } StreamDescription;

    acdlib::acd_uint maxStreams;

    //std::map<UINT, StreamDescription> settedVertexBuffers;
    std::vector<StreamDescription> settedVertexBuffers;

    // Instancing mode
    bool instancingMode;
    UINT instancesLeft;
    UINT instancesDone;
    UINT instancingStride;
    acdlib::ACDBuffer* transfBuffer;

    AIVertexDeclarationImp9* settedVertexDeclaration;

    DWORD settedFVF;
    std::vector<D3DVERTEXELEMENT9> fvfVertexDeclaration;

    AIIndexBufferImp9* settedIndexBuffer;

    typedef struct saDesc{

        AIBaseTextureImp9* samplerData;
        DWORD minFilter;
        DWORD mipFilter;
        DWORD magFilter;
        DWORD maxAniso;
        DWORD maxLevel; //  Base (Min) level in the simulator!!!
        DWORD addressU;
        DWORD addressV;
        DWORD addressW;
        DWORD sRGBTexture;

        saDesc():
            samplerData(NULL),
            minFilter(D3DTEXF_POINT),
            mipFilter(D3DTEXF_NONE),
            magFilter(D3DTEXF_POINT),
            maxAniso(1),
            maxLevel(0),
            addressU(D3DTADDRESS_WRAP),
            addressV(D3DTADDRESS_WRAP),
            addressW(D3DTADDRESS_WRAP),
            sRGBTexture(0) {}

    } SamplerDescription;

    static const acdlib::acd_uint maxSamplers = 16;
    static const acdlib::acd_uint maxVertexSamplers = 4;

    //std::map<UINT, SamplerDescription> settedTextures;
    std::vector<SamplerDescription> settedTextures;
    std::vector<SamplerDescription> settedVertexTextures;

    bool fogEnable;
    DWORD fogColor;
    
    bool alphaTest;
    BYTE alphaRef;
    D3DCMPFUNC alphaFunc;

    bool alphaBlend;
    bool separateAlphaBlend;
    acdlib::ACD_BLEND_OPTION srcBlend;
    acdlib::ACD_BLEND_OPTION srcBlendAlpha;
    acdlib::ACD_BLEND_OPTION dstBlend;
    acdlib::ACD_BLEND_OPTION dstBlendAlpha;
    acdlib::ACD_BLEND_FUNCTION blendOp;
    acdlib::ACD_BLEND_FUNCTION blendOpAlpha;

    acdlib::acd_bool stencilEnabled;
    acdlib::ACD_STENCIL_OP stencilFail;
    acdlib::ACD_STENCIL_OP stencilZFail;
    acdlib::ACD_STENCIL_OP stencilPass;
    acdlib::ACD_COMPARE_FUNCTION stencilFunc;
    acdlib::acd_uint stencilRef;
    acdlib::acd_uint stencilMask;
    acdlib::acd_uint stencilWriteMask;
    
    acdlib::acd_bool twoSidedStencilEnabled;
    acdlib::ACD_STENCIL_OP ccwStencilFail;
    acdlib::ACD_STENCIL_OP ccwStencilZFail;
    acdlib::ACD_STENCIL_OP ccwStencilPass;
    acdlib::ACD_COMPARE_FUNCTION ccwStencilFunc;

    acdlib::acd_bool colorwriteRed[MAX_RENDER_TARGETS];
    acdlib::acd_bool colorwriteGreen[MAX_RENDER_TARGETS];
    acdlib::acd_bool colorwriteBlue[MAX_RENDER_TARGETS];
    acdlib::acd_bool colorwriteAlpha[MAX_RENDER_TARGETS];

    
    acdlib::ACDTexture2D *blitTex2D;
    
    //  Stores the fixed function state for the fixed function generator.
    FFState fixedFunctionState;
    acdlib::ACDShaderProgram *fixedFunctionVertexShader;
    acdlib::ACDShaderProgram *fixedFunctionPixelShader;
    NativeShader *nativeFFVSh;
};

#endif
