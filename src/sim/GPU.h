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
 * $RCSfile: GPU.h,v $
 * $Revision: 1.41 $
 * $Author: jroca $
 * $Date: 2008-05-05 14:19:11 $
 *
 * GPU global types and definitions.
 *
 */

#ifndef _GPU_

#define _GPU_

#include "GPUTypes.h"

namespace gpu3d
{


/***  Maximum horizontal display resolution.  */
static const u32bit MAX_DISPLAY_RES_X = 4096;

/***  Maximum vertical display resolution.  */
static const u32bit MAX_DISPLAY_RES_Y = 4096;

/***  Minimum viewport x coordinate.  */
static const u32bit MIN_VIEWPORT_X = 0;

/***  Minimum viewport y coordinate.  */
static const u32bit MIN_VIEWPORT_Y = 0;

/***  Maximum viewport x coordinate.  */
static const u32bit MAX_VIEWPORT_X = 4096;

/***  Maximum viewport y coordinate.  */
static const u32bit MAX_VIEWPORT_Y = 4096;

/***  Maximun number of vertex shader constants.
      SHOULD BE THE SAME IN SHADEREMULATOR!!!  */
static const u32bit MAX_VERTEX_CONSTANTS = 512;

/***  Maximun number of fragment shader constants.
      SHOULD BE THE SAME IN SHADEREMULATOR!!!  */
static const u32bit MAX_FRAGMENT_CONSTANTS = 1024;

/***  Maximun number of vertex input attributes.
      SHOULD BE THE SAME IN SHADEREMULATOR!!!  */
static const u32bit MAX_VERTEX_ATTRIBUTES = 16;

/**
 *  Defines the number of triangle attributes (for triangle setup in the shader).
 *  NOTE: ONLY 4 REQUIRED, BUT SHADER EMULATOR IMPLEMENTATION MAY REQUIRE 16.
 */
static const u32bit MAX_TRIANGLE_ATTRIBUTES = 16;

/**  Maximum number of stream buffers.  */
static const u32bit MAX_STREAM_BUFFERS = 32;

/***  Maximum number of user clip planes.  */
static const u32bit MAX_USER_CLIP_PLANES = 6;

/**  Stream buffer ID for attribute default value (attribute
     inactive).  */
static const u32bit ST_INACTIVE_ATTRIBUTE = 255;

/***  Maximum number of fragment attributes.  */
static const u32bit MAX_FRAGMENT_ATTRIBUTES = 16;

/***  Maximum number of microtriangle fragment attributes.  */
static const u32bit MAX_MICROFRAGMENT_ATTRIBUTES = MAX_FRAGMENT_ATTRIBUTES * 3;

/***  Number of texture units.  */
static const u32bit MAX_TEXTURES = 16;

/***  Max texture size (2^n).  */
static const u32bit MAX_TEXTURE_SIZE = 13;

/***  Number of images (2D images) in a cubemap.  */
static const u32bit CUBEMAP_IMAGES = 6;

/***  Maximum level of anisotropy.  */
static const u32bit MAX_ANISOTROPY = 16;

/***  Defines the maximum number of samples to be used by the MSAA (MultiSampling AntiAliasing) algorithm.  */
static const u32bit MAX_MSAA_SAMPLES = 16;

/***  Defines the subpixel precission for MSAA samples.  */
static const f64bit MSAA_SUBPIXEL_PRECISSION = 128.0f;

/***  Defines the number of patterns that can be stored for MSAA.  */
static const u32bit MSAA_PATTERNS = 2;

/***  Defines the maximum number of render targets.  */
static const u32bit MAX_RENDER_TARGETS = 8;

/**
*
*  Defines the maximum number of bytes required to represent a color.
*
*/

static const u32bit MAX_BYTES_PER_COLOR = 8;  //  RGBA16F.

/**
 *
 *  Defines the maximum number of bytes required to store data for a pixel (depth or color).
 *
 */

static const u32bit MAX_BYTES_PER_PIXEL = 8;  // RGBA16F

/**
 *
 *  Defines the size of a stamp (quad) in fragments.
 *
 */
static const u32bit STAMP_WIDTH = 2;
static const u32bit STAMP_HEIGHT = 2;
static const u32bit STAMP_FRAGMENTS = 4;

/**
 *
 *  Defines fixed vertex and fragment attribute names.
 *
 */
static const u32bit POSITION_ATTRIBUTE = 0;
static const u32bit COLOR_ATTRIBUTE = 1;
static const u32bit COLOR_ATTRIBUTE_SEC = 2;
static const u32bit COLOR_ATTRIBUTE_BACK_PRI = 3;
static const u32bit COLOR_ATTRIBUTE_BACK_SEC = 4;
static const u32bit FACE_ATTRIBUTE = 15;

/**
 *
 *  Defines the attribute that will be used to pass the vertex index in Streamer Loader
 *  bypass mode.
 *
 */
 
static const u32bit INDEX_ATTRIBUTE = 0; 

  
/**
 *
 *  Defines fixed triangle attribute names.
 *
 */
static const u32bit X_ATTRIBUTE = 0;
static const u32bit Y_ATTRIBUTE = 1;
static const u32bit Z_ATTRIBUTE = 2;
static const u32bit W_ATTRIBUTE = 3;
static const u32bit A_ATTRIBUTE = 0;
static const u32bit B_ATTRIBUTE = 1;
static const u32bit C_ATTRIBUTE = 2;
static const u32bit AREA_ATTRIBUTE = 3;

/**  Instruction memory size (number of instructions) in the unified shader model. */
static const u32bit UNIFIED_INSTRUCTION_MEMORY_SIZE = 2048;

/**  Number of registers in the input register bank for the unified shader model.  */
static const u32bit UNIFIED_INPUT_NUM_REGS = 16;

/**  Number of registers in the output register bank for the unified shader model.  */
static const u32bit UNIFIED_OUTPUT_NUM_REGS = 16;

/**  Number of registers in the temporary register bank for the unified shader model.  */
static const u32bit UNIFIED_TEMPORARY_NUM_REGS = 32;

/**  Number of registers in the address register bank (integer) for the unified shader model.  */
static const u32bit UNIFIED_ADDRESS_NUM_REGS = 2;

/**  Number of registers in the constant register bank for the unified shader model.  */
static const u32bit UNIFIED_CONSTANT_NUM_REGS = 512;

/**  Number of registers in the predicate register bank for the unified shader model.  */
static const u32bit UNIFIED_PREDICATE_NUM_REGS = 32;


/***  Status of the GPU.  */
enum GPUStatus
{
    GPU_ST_RESET,               /**<  GPU Initialized.  */
    GPU_READY,                  /**<  GPU can receive register and memory operations and commands.  */
    GPU_DRAWING,                /**<  The GPU is displaying a vertex batch.  */
    GPU_END_GEOMETRY,           /**<  The GPU has finished the geometry phase of the current batch rendering.  */
    GPU_END_FRAGMENT,           /**<  The GPU has finished the fragment phase of the current batch rendering.  */
    GPU_MEMORY_READ,            /**<  The command processor is reading data from memory.  */
    GPU_MEMORY_WRITE,           /**<  The command processor is writing data to memory (from AGP).  */
    GPU_MEMORY_PRELOAD,         /**<  Command processor preloads data into the memory.  */
    GPU_SWAP,                   /**<  The GPU is swapping buffers.  */
    GPU_DUMPBUFFER,             /**<  The GPU is dumping a buffer (debug only!).  */
    GPU_BLITTING,               /**<  The GPU is performing a bit blit operation of a framebuffer region. */
    GPU_CLEAR_COLOR,            /**<  The GPU is clearing the color buffer.  */
    GPU_CLEAR_Z,                /**<  The GPU is clearing the Z buffer.  */
    GPU_FLUSH_COLOR,            /**<  GPU is flushing color caches.  */
    GPU_FLUSH_Z,                /**<  GPU is flushing z and stencil caches.  */
    GPU_SAVE_STATE_COLOR,       /**<  GPU is saving the color block state info into memory.  */
    GPU_RESTORE_STATE_COLOR,    /**<  GPU is restoring the color block state info from memory.  */
    GPU_SAVE_STATE_Z,           /**<  GPU is saving the z stencil block state info into memory.  */
    GPU_RESTORE_STATE_Z,        /**<  GPU is restoring the z stencil block state info from memory.  */
    GPU_ERROR                   /**<  The GPU is halted because an error (?).  */
};

/**
 *
 *  Defines the modes (vertex order) for defining a front
 *  facing triangle.
 *
 */
enum FaceMode
{
    GPU_CW,         /**<  Clock wise.  */
    GPU_CCW         /**<  Counter-clock wise.  */
};

/*** Culling modes.  */
enum CullingMode
{
    NONE,
    FRONT,
    BACK,
    FRONT_AND_BACK
};

/***  Primitive modes.  */
enum PrimitiveMode
{
    TRIANGLE,
    TRIANGLE_STRIP,
    TRIANGLE_FAN,
    QUAD,
    QUAD_STRIP,
    LINE,
    LINE_STRIP,
    LINE_FAN,
    POINT
};

/***  Stream Data modes.  */
enum StreamData
{
    SD_UNORM8,
    SD_SNORM8,
    SD_UNORM16,
    SD_SNORM16,
    SD_UNORM32,
    SD_SNORM32,
    SD_FLOAT16,
    SD_FLOAT32,
    SD_UINT8,
    SD_SINT8,
    SD_UINT16,
    SD_SINT16,
    SD_UINT32,
    SD_SINT32
};

//
//  NOTE:  ADDED FOR COMPATIBILITY
//

static const StreamData SD_U8BIT = SD_UNORM8;
static const StreamData SD_U16BIT = SD_UNORM16;
static const StreamData SD_U32BIT = SD_UNORM32;
static const StreamData SD_FLOAT = SD_FLOAT32;

/**
 *
 *  Defines the texture modes.
 *
 */
enum TextureMode
{
    GPU_TEXTURE1D,
    GPU_TEXTURE2D,
    GPU_TEXTURE3D,
    GPU_TEXTURECUBEMAP
};

/**
 *
 *  Defines the texture format modes.
 *
 */

enum TextureFormat
{
    GPU_ALPHA8,
    GPU_ALPHA12,                    /*  Supported but stored as 16 bits.  */
    GPU_ALPHA16,
    GPU_DEPTH_COMPONENT16,
    GPU_DEPTH_COMPONENT24,          /*  Supported but stored as 32 bits.  */
    GPU_DEPTH_COMPONENT32,
    GPU_LUMINANCE8,
    GPU_LUMINANCE8_SIGNED,
    GPU_LUMINANCE12,                /*  Supported but stored as 16 bits.  */
    GPU_LUMINANCE16,
    GPU_LUMINANCE4_ALPHA4,
    GPU_LUMINANCE6_ALPHA2,
    GPU_LUMINANCE8_ALPHA8,
    GPU_LUMINANCE8_ALPHA8_SIGNED,
    GPU_LUMINANCE12_ALPHA4,
    GPU_LUMINANCE12_ALPHA12,        /*  Supported but stored as 32 bits.  */
    GPU_LUMINANCE16_ALPHA16,
    GPU_INTENSITY8,
    GPU_INTENSITY12,                /*  Supported but stored as 16 bits.  */
    GPU_INTENSITY16,
    GPU_RGB332,
    GPU_RGB444,                     /*  Supported but stored as 16 bits.  */
    GPU_RGB555,                     /*  Supported but stored as 16 bits.  */
    GPU_RGB565,                     /*  Supported but stored as 16 bits.  */
    GPU_RGB888,                     /*  Supported but stored as 32 bits.  */
    GPU_RGB101010,                  /*  Supported but stored as 32 bits.  */
    GPU_RGB121212,
    GPU_RGBA2222,
    GPU_RGBA4444,
    GPU_RGBA5551,
    GPU_RGBA8888,
    GPU_RGBA1010102,
    GPU_R16,
    GPU_RG16,
    GPU_RGBA16,
    GPU_R16F,
    GPU_RG16F,
    GPU_RGBA16F,
    GPU_R32F,
    GPU_RG32F,
    GPU_RGBA32F
};

/**
 *
 *  Defines the texture compression modes.
 *
 */

enum TextureCompression
{
    GPU_NO_TEXTURE_COMPRESSION,
    GPU_S3TC_DXT1_RGB,
    GPU_S3TC_DXT1_RGBA,
    GPU_S3TC_DXT3_RGBA,
    GPU_S3TC_DXT5_RGBA,
    GPU_LATC1,
    GPU_LATC1_SIGNED,
    GPU_LATC2,
    GPU_LATC2_SIGNED    
};

/**
 *
 *  Defines the tiling/blocking formats avaiable for textures.
 *
 */
enum TextureBlocking
{
    GPU_TXBLOCK_TEXTURE,        ///<  Tiling/Blocking mode optimized for bilinear texel accesses in the Texture Cache.
    GPU_TXBLOCK_FRAMEBUFFER     ///<  Tiling/Blocking mode for compatibility with the frame buffer (color and depth) tiling/blocking mode.
}; 
 
/**
 *
 *  Defines the texture clamp modes.
 *
 */
enum ClampMode
{
    GPU_TEXT_CLAMP,
    GPU_TEXT_CLAMP_TO_EDGE,
    GPU_TEXT_REPEAT,
    GPU_TEXT_CLAMP_TO_BORDER,
    GPU_TEXT_MIRRORED_REPEAT
};

/**
 *
 *  Defines the texture filtering modes.
 *
 */

enum FilterMode
{
    GPU_NEAREST,
    GPU_LINEAR,
    GPU_NEAREST_MIPMAP_NEAREST,
    GPU_NEAREST_MIPMAP_LINEAR,
    GPU_LINEAR_MIPMAP_NEAREST,
    GPU_LINEAR_MIPMAP_LINEAR
};

/**
 *
 *  Defines the names of the 6 cubemap 2D images.
 *
 */

enum CubeMapFace
{
    GPU_CUBEMAP_POSITIVE_X = 0,
    GPU_CUBEMAP_NEGATIVE_X,
    GPU_CUBEMAP_POSITIVE_Y,
    GPU_CUBEMAP_NEGATIVE_Y,
    GPU_CUBEMAP_POSITIVE_Z,
    GPU_CUBEMAP_NEGATIVE_Z
};

/**
 *
 *  Defines the comparation modes for Stencil and Depth Tests.
 *
 */
enum CompareMode
{
    GPU_NEVER,      /**<  Never pass.  */
    GPU_ALWAYS,     /**<  Always pass.  */
    GPU_LESS,       /**<  Pass if less.  */
    GPU_LEQUAL,     /**<  Pass if less or equal.  */
    GPU_EQUAL,      /**<  Pass if equal.  */
    GPU_GEQUAL,     /**<  Pass if greater or equal.  */
    GPU_GREATER,    /**<  Pass if greater.  */
    GPU_NOTEQUAL    /**<  Pass if not equal.  */
};


/**
 *
 *  Defines the update functions for Stencil.
 *
 */
enum StencilUpdateFunction
{
    STENCIL_KEEP,       /**<  Keeps the value in the Stencil buffer.  */
    STENCIL_ZERO,       /**<  Writes a zero in the Stencil buffer.  */
    STENCIL_REPLACE,    /**<  Replaces the value in the Stencil Buffer.  */
    STENCIL_INCR,       /**<  Saturated increment of the value in the Stencil Buffer.  */
    STENCIL_DECR,       /**<  Saturated decrement of the value in the Stencil Buffer.  */
    STENCIL_INVERT,     /**<  Invert (negate) the value in the Stencil Buffer.  */
    STENCIL_INCR_WRAP,  /**<  Increment with wrap of the value in the Stencil Buffer.  */
    STENCIL_DECR_WRAP   /**<  Decrement with wrap of the value in the Stencil Buffer.  */
};

/**
 *
 *  Defines the blending equation modes.
 *
 */
enum BlendEquation
{
    BLEND_FUNC_ADD,                 /**<  C = Cs S + Cd D  */
    BLEND_FUNC_SUBTRACT,            /**<  C = Cs S - Cd D  */
    BLEND_FUNC_REVERSE_SUBTRACT,    /**<  C = Cd D - Cs S  */
    BLEND_MIN,                      /**<  C = min(Cs, Cd)  */
    BLEND_MAX                       /**<  C = max(Cs, Cd)  */
};


/**
 *
 *  Defines the blending function modes.
 *
 */
enum BlendFunction
{
                                    /**<  (Sr, Sg, Sb)  |   Sa   */
                                    /**<        or      |   or   */
                                    /**<  (Dr, Dg, Db)  |   Sb   */

    BLEND_ZERO,                     /**<  (0, 0, 0)     |   0    */
    BLEND_ONE,                      /**<  (1, 1, 1)     |   1    */
    BLEND_SRC_COLOR,                /**<  (Rs, Gs, Bs)  |   As   */
    BLEND_ONE_MINUS_SRC_COLOR,      /**<  (1, 1, 1) - (Rs, Gs, Bs) | 1 - As  */
    BLEND_DST_COLOR,                /**<  (Rd, Gd, Bd)  |   Ad   */
    BLEND_ONE_MINUS_DST_COLOR,      /**<  (1, 1, 1) - (Rd, Gd, Bd) | 1 - Ad  */
    BLEND_SRC_ALPHA,                /**<  (As, As, As)  |   As   */
    BLEND_ONE_MINUS_SRC_ALPHA,      /**<  (1, 1, 1) - (As, As, As) | 1 - As  */
    BLEND_DST_ALPHA,                /**<  (Ad, Ad, Ad)  |   Ad   */
    BLEND_ONE_MINUS_DST_ALPHA,      /**<  (1, 1, 1) - (Ad, Ad, Ad) | 1 - Ad  */
    BLEND_CONSTANT_COLOR,           /**<  (Rc, Gc, Bc)  |   Ac   */
    BLEND_ONE_MINUS_CONSTANT_COLOR, /**<  (1, 1, 1) - (Rc, Gc, Bc) | 1 - Ac  */
    BLEND_CONSTANT_ALPHA,           /**<  (Ac, Ac, Ac)  |   Ac   */
    BLEND_ONE_MINUS_CONSTANT_ALPHA, /**<  (1, 1, 1) - (Ac, Ac, Ac) | 1 - Ac  */
    BLEND_SRC_ALPHA_SATURATE        /**<  (f, f, f)^2   |   1 [Note: f = min(As, 1 - Ad) ]  */
};


/**
 *
 *  Defines the Logical Operation modes.
 *
 */
enum LogicOpMode
{
    LOGICOP_CLEAR,          /**<  0  */
    LOGICOP_AND,            /**<  s and d  */
    LOGICOP_AND_REVERSE,    /**<  a and not d  */
    LOGICOP_COPY,           /**<  s  */
    LOGICOP_AND_INVERTED,   /**<  not s and d  */
    LOGICOP_NOOP,           /**<  d  */
    LOGICOP_XOR,            /**<  s xor d  */
    LOGICOP_OR,             /**<  s or d  */
    LOGICOP_NOR,            /**<  not(s or d)  */
    LOGICOP_EQUIV,          /**<  not(s xor d)  */
    LOGICOP_INVERT,         /**<  not d  */
    LOGICOP_OR_REVERSE,     /**<  s or not d  */
    LOGICOP_COPY_INVERTED,  /**<  not s  */
    LOGICOP_OR_INVERTED,    /**<  not s or d  */
    LOGICOP_NAND,           /**<  not(s and d)  */
    LOGICOP_SET             /**<  all 1's  */
};

/**
 *
 *  Defines the different shader targets.
 *
 */
 
enum ShaderTarget
{
    VERTEX_TARGET = 0,
    FRAGMENT_TARGET = 1,
    TRIANGLE_TARGET = 2,
    MICROTRIANGLE_TARGET = 3,
    MAX_SHADER_TARGETS = 4
};


/**
 *
 *  This structure stores the GPU state registers.
 *
 */

struct GPUState
{

    /*  GPU state registers.  */
    GPUStatus statusRegister;       /**<  GPU status register.  */

    /*  GPU display registers.  */
    u32bit displayResX;             /**<  Display horizontal resolution.  */
    u32bit displayResY;             /**<  Display vertical resolution.  */

    /*  GPU memory registers.  */
    u32bit frontBufferBaseAddr;     /**<  Frontbuffer GPU memory base address.  */
    u32bit backBufferBaseAddr;      /**<  Backbuffer GPU memory base address.  */
    u32bit zStencilBufferBaseAddr;  /**<  Z/Stencil Buffer GPU memory base address.  */
    u32bit textureMemoryBaseAddr;   /**<  Texture memory GPU memory base address.  */
    u32bit programMemoryBaseAddr;   /**<  Shader program memory GPU memory base address.  */

    /*  GPU vertex shader registers.  */
    u32bit vertexProgramAddr;       /**<  Active vertex program address in program memory.  */
    u32bit vertexProgramStartPC;    /**<  Active vertex program initial PC.  */
    u32bit vertexProgramSize;       /**<  Size of the vertex program to load.  */
    u32bit vertexThreadResources;   /**<  Amount of 'resources' used per vertex thread in the vertex shader.  */
    QuadFloat vConstants[MAX_VERTEX_CONSTANTS];     /**<  Vertex shader float point vector constant bank.  */
    bool outputAttribute[MAX_VERTEX_ATTRIBUTES];    /**<  Stores if the vertex attribute is an output of the current vertex shader.  */

    /*  GPU vertex stream buffer registers.  */
    u32bit attributeMap[MAX_VERTEX_ATTRIBUTES];     /**<  Mapping from vertex input attributes and vertex streams.  */
    QuadFloat attrDefValue[MAX_VERTEX_ATTRIBUTES];  /**<  Defines the vertex attribute default values.  */
    u32bit streamAddress[MAX_STREAM_BUFFERS];       /**<  Address in GPU memory for the vertex stream buffers.  */
    u32bit streamStride[MAX_STREAM_BUFFERS];        /**<  Stride for the stream buffers.  */
    StreamData streamData[MAX_STREAM_BUFFERS];      /**<  Data type for the stream buffer.  */
    u32bit streamElements[MAX_STREAM_BUFFERS];      /**<  Number of stream data elements (vectors) per stream buffer entry.  */
    u32bit streamFrequency[MAX_STREAM_BUFFERS];     /**<  Frequency at which the stream is sampled: 0 -> per index/vertex, >0 -> MOD(instance, freq).  */
    u32bit streamStart;                             /**<  Start position (entry) from where to start streaming data.  */
    u32bit streamCount;                             /**<  Count number of elements to stream to the Vertex Shader.  */
    u32bit streamInstances;                         /**<  Count number of instances (loops) for the current stream.  */
    bool indexedMode;                               /**<  Indexed primitive mode.  */
    u32bit indexStream;                             /**<  Index stream buffer.  */
    bool d3d9ColorStream[MAX_STREAM_BUFFERS];       /**<  Read components of the color attributes in the order defined by D3D9.  */
    bool attributeLoadBypass;                       /**<  Flag that defines if the attribute load stage is bypassed (StreamerLoader) to implement attribute load in the shader.  */

    /*  GPU primitive assembly registers.  */
    PrimitiveMode primitiveMode;                    /**<  Primitive mode.  */

    /*  GPU clipping and culling registers.  */
    bool frustumClipping;           /**<  Frustum clipping flag.  */
    QuadFloat userClip[MAX_USER_CLIP_PLANES];       /**<  User clip planes.  */
    bool userClipPlanes;            /**<  User clip planes enabled or disabled.  */
    FaceMode faceMode;              /**<  Mode that defines a front facing triangle.  */
    CullingMode cullMode;           /**<  Culling Mode.  */

    /*  Hierarchical Z registers.  */
    bool hzTest;                    /**<  Hierarchical Z test enable flag.  */

    /*  Early Z registers.  */
    bool earlyZ;                    /**<  Early Z/Stencil test enable flag.  */

    /*  GPU rasterization registers.  */
    bool d3d9PixelCoordinates;      /**<  Use D3D9 convention for pixel coordinates -> top left corner is (0,0).  */
    s32bit viewportIniX;            /**<  Viewport initial (lower left) X coordinate.  */
    s32bit viewportIniY;            /**<  Viewport initial (lower left) Y coordinate.  */
    u32bit viewportHeight;          /**<  Viewport height.  */
    u32bit viewportWidth;           /**<  Viewport width.  */
    bool scissorTest;               /**<  Flag that enables scissor test.  */
    s32bit scissorIniX;             /**<  Scissor initial (lower left) X coordinate.  */
    s32bit scissorIniY;             /**<  Scissor initial (lower left) Y coordinate.  */
    u32bit scissorHeight;           /**<  Scissor height.  */
    u32bit scissorWidth;            /**<  Scissor width.  */
    f32bit nearRange;               /**<  Near depth range.  */
    f32bit farRange;                /**<  Far depth range.  */
    f32bit slopeFactor;             /**<  Depth offset as a factor of the triangle z slope.  */
    f32bit unitOffset;              /**<  Depth offset in depth precission units.  */
    bool d3d9DepthRange;                /**<  Use D3D9 ranger for Z values in clip space.  */
    u32bit zBufferBitPrecission;    /**<  Z Buffer bit precission.  */
    bool d3d9RasterizationRules;    /**<  Use the D3D9 rasterization rules (pixel center, filling convention).  */
    bool twoSidedLighting;          /**<  Flag that enables or disables two sided lighting.  */
    bool multiSampling;             /**<  Flag that stores if multisampling antialiasing is enabled (MSAA).  */
    u32bit msaaSamples;             /**<  Number of z samples to generate per fragment when MSAA is enabled.  */
    bool interpolation[MAX_FRAGMENT_ATTRIBUTES];            /**<  Interpolation enabled or disabled (FLAT/SMOTH).  */
    bool fragmentInputAttributes[MAX_FRAGMENT_ATTRIBUTES];  /**<  Stores the active fragment attributes.  */

    /*  GPU fragment shader registers.  */
    u32bit fragProgramAddr;         /**<  Current fragment program address in program memory.  */
    u32bit fragProgramStartPC;      /**<  Current fragment program initial PC.  */
    u32bit fragProgramSize;         /**<  Current fragment program size.  */
    u32bit fragThreadResources;     /**<  Amount of 'resources' used per fragment thread in the fragment shader.  */
    QuadFloat fConstants[MAX_FRAGMENT_CONSTANTS];   /**<  Fragment shader float point vector constant bank.  */
    bool modifyDepth;               /**<  Flag storing if the fragment shader modifies the fragment depth component.  */

    //  Shader program registers.
    u32bit programAddress;                          /**<  Address of memory from where to load the shader program.  */
    u32bit programSize;                             /**<  Size in bytes of the shader program to load.  */
    u32bit programLoadPC;                           /**<  Position in the shader instruction memory where the shader program has to be loaded.  */
    u32bit programStartPC[MAX_SHADER_TARGETS];      /**<  Initial PC for the different shader targets.  */
    u32bit programResources[MAX_SHADER_TARGETS];    /**<  Amount of 'resources' per shader thread for a given shader target.  */    
    
    /*  GPU Texture Unit registers.  */
    bool textureEnabled[MAX_TEXTURES];      /**<  Texture unit enable flag.  */
    TextureMode textureMode[MAX_TEXTURES];  /**<  Current texture mode active in the texture unit.  */
    u32bit textureAddress[MAX_TEXTURES][MAX_TEXTURE_SIZE][CUBEMAP_IMAGES];  /**<  Address in GPU memory of the active texture mipmap levels.  */
    u32bit textureWidth[MAX_TEXTURES];      /**<  Active texture width in texels.  */
    u32bit textureHeight[MAX_TEXTURES];     /**<  Active texture height in texels.  */
    u32bit textureDepth[MAX_TEXTURES];      /**<  Active texture depth in texels.  */
    u32bit textureWidth2[MAX_TEXTURES];     /**<  Log2 of the texture width (base mipmap size).  */
    u32bit textureHeight2[MAX_TEXTURES];    /**<  Log2 of the texture height (base mipmap size).  */
    u32bit textureDepth2[MAX_TEXTURES];     /**<  Log2 of the texture depth (base mipmap size).  */
    u32bit textureBorder[MAX_TEXTURES];     /**<  Texture border in texels.  */
    QuadFloat textBorderColor[MAX_TEXTURES];    /**<  Texture border color.  */
    TextureFormat textureFormat[MAX_TEXTURES];  /**<  Texture format of the active texture.  */
    TextureCompression textureCompr[MAX_TEXTURES];  /**<  Texture compression mode for the active texture.  */
    TextureBlocking textureBlocking[MAX_TEXTURES];  /**<  Texture tiling/blocking mode for the texture.  */
    bool textureReverse[MAX_TEXTURES];              /**<  Reverses texture data order (from little to big endian).  */
    bool textD3D9ColorConv[MAX_TEXTURES];           /**<  Read the color components in the order defined by D3D9.  */
    bool textD3D9VInvert[MAX_TEXTURES];             /**<  Invert the coordinate u for D3D9 support.  */
    ClampMode textureWrapS[MAX_TEXTURES];       /**<  Texture wrap mode for s coordinate.  */
    ClampMode textureWrapT[MAX_TEXTURES];       /**<  Texture wrap mode for t coordinate.  */
    ClampMode textureWrapR[MAX_TEXTURES];       /**<  Texture wrap mode for r coordinate.  */
    bool textureNonNormalized[MAX_TEXTURES];    /**<  Texture coordinates are non normalized.  */
    FilterMode textureMinFilter[MAX_TEXTURES];  /**<  Texture minification filter.  */
    FilterMode textureMagFilter[MAX_TEXTURES];  /**<  Texture Magnification filter.  */
    bool textureEnableComparison[MAX_TEXTURES]; /**<  Enable texture comparison filter (PCF).  */
    CompareMode textureComparisonFunction[MAX_TEXTURES];    /**<  Texture comparison function (PCF).  */
    bool textureSRGB[MAX_TEXTURES];         /**<  Texture in sRGB space, conversion to linear space required.  */
    f32bit textureMinLOD[MAX_TEXTURES];     /**<  Texture minimum lod.  */
    f32bit textureMaxLOD[MAX_TEXTURES];     /**<  Texture maximum lod.  */
    f32bit textureLODBias[MAX_TEXTURES];    /**<  Texture lod bias.  */
    u32bit textureMinLevel[MAX_TEXTURES];   /**<  Texture minimum mipmap level.  */
    u32bit textureMaxLevel[MAX_TEXTURES];   /**<  Texture maximum mipmap level.  */
    f32bit textureUnitLODBias[MAX_TEXTURES];    /**<  Texture unit lod bias (not texture lod!!).  */
    u32bit maxAnisotropy[MAX_TEXTURES];     /**<  Max anisotropy level for the texture.  */

    /*  GPU per fragment operations and tests registers.  */

    /*  Depth and stencil clear values.  */
    u32bit zBufferClear;            /**<  Z Buffer clear depth.  */
    u32bit stencilBufferClear;      /**<  Stencil Buffer clear value.  */
    u32bit zstencilStateBufferAddr; /**<  Z Stencil block state buffer memory address.  */

    /*  GPU stencil test registers.  */
    bool stencilTest;               /**<  Enable/Disable Stencil test flag.  */
    CompareMode stencilFunction;    /**<  Stencil test comparation function.  */
    u8bit stencilReference;         /**<  Stencil reference value for the test.  */
    u8bit stencilTestMask;          /**<  Stencil mask for the stencil operands test.  */
    u8bit stencilUpdateMask;        /**<  Stencil mask for the stencil update.  */
    StencilUpdateFunction stencilFail;  /**<  Update function when stencil test fails.  */
    StencilUpdateFunction depthFail;    /**<  Update function when depth test fails.  */
    StencilUpdateFunction depthPass;    /**<  Update function when depth test passes.  */

    /*  GPU depth test registers.  */
    bool depthTest;                 /**<  Enable/Disable depth test flag.  */
    CompareMode depthFunction;      /**<  Depth test comparation function.  */
    bool depthMask;                 /**<  If depth buffer update is enabled or disabled.  */

    bool zStencilCompression;       /**<  Flag used to enable/disable compression of the Z and Stencil buffer.  */
    
    //  Render Target and Color Buffer Registers.
    TextureFormat colorBufferFormat;            /**<  Format of the color buffer.  */
    bool colorCompression;                      /**<  Flag to enable/disable compression of the color buffer.  */
    bool colorSRGBWrite;                        /**<  Flag to enable/disable conversion to sRGB space on color write.  */
    bool rtEnable[MAX_RENDER_TARGETS];          /**<  Flag that enables/disables a render target.  */
    TextureFormat rtFormat[MAX_RENDER_TARGETS]; /**<  Format of the render target.  */
    u32bit rtAddress[MAX_RENDER_TARGETS];       /**<  Base address in memory of the render target.  */
    
    /*  Color buffer clear value.  */
    QuadFloat colorBufferClear;     /**<  Color Buffer clear color.  */
    u32bit colorStateBufferAddr;    /**<  Color block state buffer memory address.  */

    /*  GPU color blend registers.  */
    bool colorBlend[MAX_RENDER_TARGETS];                        /**<  Enable/disable color blending flag.  */
    BlendEquation blendEquation[MAX_RENDER_TARGETS];            /**<  Color blend equation.  */
    BlendFunction blendSourceRGB[MAX_RENDER_TARGETS];           /**<  Weight factor function for source RGB color components.  */
    BlendFunction blendDestinationRGB[MAX_RENDER_TARGETS];      /**<  Weight factor function for destination RGB color components.  */
    BlendFunction blendSourceAlpha[MAX_RENDER_TARGETS];         /**<  Weight factor function for source Alpha color component.  */
    BlendFunction blendDestinationAlpha[MAX_RENDER_TARGETS];    /**<  Weight factor function for destination Alpha color component.  */
    QuadFloat blendColor[MAX_RENDER_TARGETS];                   /**<  Blend constant color.  */
    bool colorMaskR[MAX_RENDER_TARGETS];                        /**<  Update to color buffer Red component enabled.  */
    bool colorMaskG[MAX_RENDER_TARGETS];                        /**<  Update to color buffer Green component enabled.  */
    bool colorMaskB[MAX_RENDER_TARGETS];                        /**<  Update to color buffer Blue component enabled.  */
    bool colorMaskA[MAX_RENDER_TARGETS];                        /**<  Update to color buffer Alpha component enabled.  */
 
    /*  GPU color logical operation registers.  */
    bool logicalOperation;          /**<  Enable/disabe color logical operation flag.  */
    LogicOpMode logicOpFunction;    /**<  Color logical operation function.  */
    
    /*  Memory Controller (V2) registers.  */
    u32bit mcSecondInterleavingStartAddr; /** Second interleaving start address (0 means not used) */
    
    /* Framebuffer bit blit operation registers. */

    u32bit blitIniX;                                 /**<  Framebuffer initial (lower left) X coordinate.  */
    u32bit blitIniY;                                 /**<  Framebuffer initial (lower left) Y coordinate.  */
    u32bit blitXOffset;                              /**<  Texture initial (lower left) X coordinate.  */
    u32bit blitYOffset;                              /**<  Texture initial (lower left) Y coordinate.  */
    u32bit blitHeight;                               /**<  Blit region height.  */
    u32bit blitWidth;                                /**<  Blit region width.  */
    u32bit blitTextureWidth2;                        /**<  Ceiling log 2 of the destination texture width.  */
    u32bit blitDestinationAddress;                   /**<  GPU memory address for blit operation destination. */
    TextureFormat blitDestinationTextureFormat;      /**<  Texture format for blit destination.  */
    TextureBlocking blitDestinationTextureBlocking;  /**<  Texture tiling/blocking format used for the destination texture.  */
    
};


/***  GPU registers identifiers.  */
enum GPURegister
{
    /*  GPU state registers.  */
    GPU_STATUS,
    GPU_MEMORY,

    /*  GPU vertex shader.  */
    GPU_VERTEX_PROGRAM,
    GPU_VERTEX_PROGRAM_PC,
    GPU_VERTEX_PROGRAM_SIZE,
    GPU_VERTEX_THREAD_RESOURCES,
    GPU_VERTEX_CONSTANT,
    GPU_VERTEX_OUTPUT_ATTRIBUTE,

    /*  GPU vertex stream buffer registers.  */
    GPU_VERTEX_ATTRIBUTE_MAP,
    GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE,
    GPU_STREAM_ADDRESS,
    GPU_STREAM_STRIDE,
    GPU_STREAM_DATA,
    GPU_STREAM_ELEMENTS,
    GPU_STREAM_FREQUENCY,
    GPU_STREAM_START,
    GPU_STREAM_COUNT,
    GPU_STREAM_INSTANCES,
    GPU_INDEX_MODE,
    GPU_INDEX_STREAM,
    GPU_D3D9_COLOR_STREAM,
    GPU_ATTRIBUTE_LOAD_BYPASS,
    
    /*  GPU primitive assembly registers.  */
    GPU_PRIMITIVE,

    /*  GPU clipping and culling registers.  */
    GPU_FRUSTUM_CLIPPING,
    GPU_USER_CLIP,
    GPU_USER_CLIP_PLANE,

    /*  Marks the last geometry related GPU register.  */
    GPU_LAST_GEOMETRY_REGISTER,

    /*  GPU memory registers.  */
    GPU_FRONTBUFFER_ADDR,
    GPU_BACKBUFFER_ADDR,
    GPU_ZSTENCILBUFFER_ADDR,
    GPU_TEXTURE_MEM_ADDR,
    GPU_PROGRAM_MEM_ADDR,

    /* GPU face culling registers. */
    GPU_FACEMODE,
    GPU_CULLING,

    /*  GPU hierarchical Z registers.  */
    GPU_HIERARCHICALZ,

    /*  GPU early Z registers.  */
    GPU_EARLYZ,

    /*  GPU display registers.  */
    GPU_DISPLAY_X_RES,
    GPU_DISPLAY_Y_RES,

    /*  GPU rasterization registers.  */
    GPU_D3D9_PIXEL_COORDINATES,
    GPU_VIEWPORT_INI_X,
    GPU_VIEWPORT_INI_Y,
    GPU_VIEWPORT_WIDTH,
    GPU_VIEWPORT_HEIGHT,
    GPU_SCISSOR_TEST,
    GPU_SCISSOR_INI_X,
    GPU_SCISSOR_INI_Y,
    GPU_SCISSOR_WIDTH,
    GPU_SCISSOR_HEIGHT,
    GPU_DEPTH_RANGE_NEAR,
    GPU_DEPTH_RANGE_FAR,
    GPU_DEPTH_SLOPE_FACTOR,
    GPU_DEPTH_UNIT_OFFSET,
    GPU_Z_BUFFER_BIT_PRECISSION,
    GPU_D3D9_DEPTH_RANGE,
    GPU_D3D9_RASTERIZATION_RULES,
    GPU_TWOSIDED_LIGHTING,
    GPU_MULTISAMPLING,
    GPU_MSAA_SAMPLES,
    GPU_INTERPOLATION,
    GPU_FRAGMENT_INPUT_ATTRIBUTES,

    /*  GPU fragment registers.  */
    GPU_FRAGMENT_PROGRAM,
    GPU_FRAGMENT_PROGRAM_PC,
    GPU_FRAGMENT_PROGRAM_SIZE,
    GPU_FRAGMENT_THREAD_RESOURCES,
    GPU_FRAGMENT_CONSTANT,
    GPU_MODIFY_FRAGMENT_DEPTH,

    //  GPU shader program registers.
    GPU_SHADER_PROGRAM_ADDRESS,
    GPU_SHADER_PROGRAM_SIZE,
    GPU_SHADER_PROGRAM_LOAD_PC,
    GPU_SHADER_PROGRAM_PC,
    GPU_SHADER_THREAD_RESOURCES,
    
    /*  GPU Texture Unit registers.  */
    GPU_TEXTURE_ENABLE,
    GPU_TEXTURE_MODE,
    GPU_TEXTURE_ADDRESS,
    GPU_TEXTURE_WIDTH,
    GPU_TEXTURE_HEIGHT,
    GPU_TEXTURE_DEPTH,
    GPU_TEXTURE_WIDTH2,
    GPU_TEXTURE_HEIGHT2,
    GPU_TEXTURE_DEPTH2,
    GPU_TEXTURE_BORDER,
    GPU_TEXTURE_FORMAT,
    GPU_TEXTURE_REVERSE,
    GPU_TEXTURE_D3D9_COLOR_CONV,
    GPU_TEXTURE_D3D9_V_INV,
    GPU_TEXTURE_COMPRESSION,
    GPU_TEXTURE_BLOCKING,
    GPU_TEXTURE_BORDER_COLOR,
    GPU_TEXTURE_WRAP_S,
    GPU_TEXTURE_WRAP_T,
    GPU_TEXTURE_WRAP_R,
    GPU_TEXTURE_NON_NORMALIZED,
    GPU_TEXTURE_MIN_FILTER,
    GPU_TEXTURE_MAG_FILTER,
    GPU_TEXTURE_ENABLE_COMPARISON,
    GPU_TEXTURE_COMPARISON_FUNCTION,
    GPU_TEXTURE_SRGB,
    GPU_TEXTURE_MIN_LOD,
    GPU_TEXTURE_MAX_LOD,
    GPU_TEXTURE_LOD_BIAS,
    GPU_TEXTURE_MIN_LEVEL,
    GPU_TEXTURE_MAX_LEVEL,
    GPU_TEXT_UNIT_LOD_BIAS,
    GPU_TEXTURE_MAX_ANISOTROPY,
    
    /*  GPU depth and stencil clear values.  */
    GPU_Z_BUFFER_CLEAR,
    GPU_STENCIL_BUFFER_CLEAR,
    GPU_ZSTENCIL_STATE_BUFFER_MEM_ADDR,

    /*  GPU Stencil test registers.  */
    GPU_STENCIL_TEST,
    GPU_STENCIL_FUNCTION,
    GPU_STENCIL_REFERENCE,
    GPU_STENCIL_COMPARE_MASK,
    GPU_STENCIL_UPDATE_MASK,
    GPU_STENCIL_FAIL_UPDATE,
    GPU_DEPTH_FAIL_UPDATE,
    GPU_DEPTH_PASS_UPDATE,

    /*  GPU Depth test registers.  */
    GPU_DEPTH_TEST,
    GPU_DEPTH_FUNCTION,
    GPU_DEPTH_MASK,
    
    GPU_ZSTENCIL_COMPRESSION,

    //  Render Target and Color Buffer registers.
    GPU_COLOR_BUFFER_FORMAT,
    GPU_COLOR_COMPRESSION,
    GPU_COLOR_SRGB_WRITE,
    GPU_RENDER_TARGET_ENABLE,
    GPU_RENDER_TARGET_FORMAT,
    GPU_RENDER_TARGET_ADDRESS,
    
    /*  GPU color clear value.  */
    GPU_COLOR_BUFFER_CLEAR,
    GPU_COLOR_STATE_BUFFER_MEM_ADDR,

    /*  GPU Color Blend registers.  */
    GPU_COLOR_BLEND,
    GPU_BLEND_EQUATION,
    GPU_BLEND_SRC_RGB,
    GPU_BLEND_DST_RGB,
    GPU_BLEND_SRC_ALPHA,
    GPU_BLEND_DST_ALPHA,
    GPU_BLEND_COLOR,
    GPU_COLOR_MASK_R,
    GPU_COLOR_MASK_G,
    GPU_COLOR_MASK_B,
    GPU_COLOR_MASK_A,

    /*  GPU Color Logical Operation registers.  */
    GPU_LOGICAL_OPERATION,
    GPU_LOGICOP_FUNCTION,

    /* Register indicating the start address of the second interleaving. 
       Must be set to 0 to use a single (CHANNEL/BANK) memory interleaving */
    GPU_MCV2_2ND_INTERLEAVING_START_ADDR, 

    /* Registers for Framebuffer bit blit operation */
    GPU_BLIT_INI_X,
    GPU_BLIT_INI_Y,
    GPU_BLIT_X_OFFSET,
    GPU_BLIT_Y_OFFSET,
    GPU_BLIT_WIDTH,
    GPU_BLIT_HEIGHT,
    GPU_BLIT_DST_ADDRESS,
    GPU_BLIT_DST_TX_WIDTH2,
    GPU_BLIT_DST_TX_FORMAT,
    GPU_BLIT_DST_TX_BLOCK,

    /*  Last GPU register name mark.  */
    GPU_LAST_REGISTER
};

/**  This type describes the data input/output for a GPU register.  */

struct GPURegData
{
    union{
        u32bit uintVal;
        s32bit intVal;
        f32bit qfVal[4];
        f32bit f32Val;
        bool booleanVal;
        GPUStatus status;
        FaceMode faceMode;
        CullingMode culling;
        PrimitiveMode primitive;
        StreamData streamData;
        CompareMode compare;
        StencilUpdateFunction stencilUpdate;
        BlendEquation blendEquation;
        BlendFunction blendFunction;
        LogicOpMode logicOp;
        TextureMode txMode;
        TextureFormat txFormat;
        TextureCompression txCompression;
        TextureBlocking txBlocking;
        ClampMode txClamp;
        FilterMode txFilter;
    };

    GPURegData()
    {
        qfVal[0] = qfVal[1] = qfVal[2] = qfVal[3] = 0;
    }

    bool operator!=(const GPURegData &data)
    {
        return (qfVal[0] != data.qfVal[0]) || (qfVal[1] != data.qfVal[1]) ||
            (qfVal[2] != data.qfVal[2]) || (qfVal[3] != data.qfVal[3]);
    }
};


//  GPU events.
enum GPUEvent
{
    GPU_END_OF_FRAME_EVENT,   //  End of frame event
    GPU_UNNAMED_EVENT,        //  Unnamed/generic event
    GPU_NUM_EVENTS            //  Only to create arrays based on events!!!
};


/***  GPU commands.  */
enum GPUCommand
{
    GPU_RESET,                  /**<  Perform a reset of the GPU.  */
    GPU_DRAW,                   /**<  Draw a batch of vertexes.  */
    GPU_SWAPBUFFERS,            /**<  Swap the front and back buffer.  */
    GPU_DUMPCOLOR,              /**<  Dumps the color buffer into a file (debug only!).  */
    GPU_DUMPDEPTH,              /**<  Dumps the depth buffer into a file (debug only!).  */
    GPU_DUMPSTENCIL,            /**<  Dumps the stencil buffer into a file (debug only!).  */
    GPU_BLIT,                   /**<  Perform a bit blit operation.  */
    GPU_CLEARBUFFERS,           /**<  Clear the Z/Stencil and color buffers.  */
    GPU_CLEARZBUFFER,           /**<  Clear the Z Buffer.  */
    GPU_CLEARZSTENCILBUFFER,    /**<  Clear the Z/Stencil buffer.  */
    GPU_CLEARCOLORBUFFER,       /**<  Clear the color buffer.  */
    GPU_LOAD_VERTEX_PROGRAM,    /**<  Load a vertex program into the vertex instruction memory.  */
    GPU_LOAD_FRAGMENT_PROGRAM , /**<  Load a fragment program into the fragment instruction memory.  */
    GPU_LOAD_SHADER_PROGRAM,    /**<  Load a shader program into the shader instruction memory.  */
    GPU_FLUSHZSTENCIL,          /**<  Flush the z and stencil caches.  */
    GPU_FLUSHCOLOR,             /**<  Flush the color caches.  */
    GPU_SAVE_COLOR_STATE,       /**<  Save to memory the block state info for the color buffer.  */
    GPU_RESTORE_COLOR_STATE,    /**<  Restore from memory the block state info for the color buffer.  */
    GPU_SAVE_ZSTENCIL_STATE,    /**<  Save to memory the block state info for the z and stencil buffer.  */
    GPU_RESTORE_ZSTENCIL_STATE, /**<  Restore from memory the block state info for the z and stencil buffer.  */
    GPU_RESET_COLOR_STATE,      /**<  Sets the color buffer block state info to uncompressed.  */
    GPU_RESET_ZSTENCIL_STATE    /**<  Sets the z and stencil buffer block state info to uncompressed and clears HZ.  */
};


/**
 *
 *  Defines the clock domains of a GPU.
 *
 */
enum GPUClockDomain
{
    GPU_CLOCK_DOMAIN    = 0,        /**<  Main GPU clock domain (everything except Memory and Shaders).  */
    SHADER_CLOCK_DOMAIN = 1,        /**<  Shader processor clock domain. */
    MEMORY_CLOCK_DOMAIN = 2         /**<  Memory clock domain.  */
};

} // namespace gpu3d

#endif
