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
 * GPU emulator definition file.
 *
 */

/**
 *
 *  @file GPUEmulator.h
 *
 *  This file contains definitions and includes for the ATTILA GPU emulator.
 *
 */

#ifndef _GPUEMULATOR_

#define _GPUEMULATOR_

//  Defines the GPU types for our code framework.
#include "GPUTypes.h"

//  Configuration file definitions.
#include "ConfigLoader.h"

//  TraceDriverInterface definition.
#include "TraceDriverInterface.h"

//  GPU definitions.
#include "GPU.h"

//  Emulator classes.
#include "ShaderEmulator.h"
#include "RasterizerEmulator.h"
#include "TextureEmulator.h"
#include "FragmentOpEmulator.h"
#include "PixelMapper.h"
#include "ValidationInfo.h"

#include <vector>
#include <map>


/**
 *
 *  Used to enable the generation of traces of the emulator execution.
 *
 */
#ifdef ENABLE_GPU_EMU_TRACE
    #define GPU_EMU_TRACE(x) {x}
#else
    #define GPU_EMU_TRACE(x) {}
#endif

namespace gpu3d
{

/**
 *
 *  GPU Emulator class.
 *
 *  The class defines structures and attributes to implement a GPU emulator.
 *
 */

class GPUEmulator
{
private:

    /**
     *
     *  Container for vertex attributes.
     *
     */
      
    class ShadedVertex
    {
    private:

        QuadFloat attributes[MAX_VERTEX_ATTRIBUTES];    /**<  Array storing the vertex attributes.  */

    public:

        /**
         *
         *  Constructor.
         *
         *  @param attrib Pointer to an array with the attributes for the vertex.
         *
         */
         
        ShadedVertex(QuadFloat *attrib);

        /**
         *
         *  Get a pointer to the vertex attribute array.
         *
         *  @return A pointer to the vertex attribute array.
         *
         */
         
        QuadFloat *getAttributes();

    };

    /**
     *
     *  Container for a fragment and the fragment attributes.
     *
     */
     
    class ShadedFragment
    {
    private:

        Fragment *fragment;                             /**<  Pointer to a Fragment object storing fragment information.  */
        QuadFloat attributes[MAX_FRAGMENT_ATTRIBUTES];  /**<  Array storing the fragment attributes.  */
        bool culled;                                    /**<  Flag that stores if the fragment is culled.  */

    public:

        /**
         *
         *  Constructor.
         *
         *  @param fr Pointer to a Fragment object storing the fragment information.
         *  @param culled A boolean storing if the fragment is culled.
         * 
         */
         
        ShadedFragment(Fragment *fr, bool culled);
        
        /**
         *
         *  Gets the pointer Fragment object storing the fragment information.
         *
         *  @return A pointer to the Fragment object storing fragment information.
         *
         */
        Fragment *getFragment();
        
        /**
         *
         *  Gets a pointer to the array of fragment attributes.
         *
         *  @return A pointer to the fragment attribute array.
         *
         */
         
        QuadFloat *getAttributes();
        
        /**
         *
         *  Checks if the fragment is culled.
         *
         *  @return TRUE if the fragment was marked as culled, FALSE otherwise.
         *
         */
         
        bool isCulled();
        
        /**
         *
         *  Marks the fragment as culled.
         *
         */
         
        void setAsCulled();
    };

    /**
     *
     *  Implements a cache for compressed texture data.
     *
     *  No replacement policy is implemented.  When the cache is full and a request to a new block
     *  is issued the whole cache is flushed.
     *
     */
     
    class CompressedTextureCache
    {
    private:

        GPUEmulator &emu;                   /**<  Reference to the GPU emulator instancing the Texture Cache.  */
        u32bit decompressedBlockSize;       /**<  Defines the size in bytes of a decompressed block.  */
        u32bit maxCachedBlocks;             /**<  Defines the maximum number of blocks cached by the object.  */
        u64bit decompressedBlockMask;       /**<  Defines the address mask used to access decompressed block data.  */
        u32bit compressionRatioShift;       /**<  Defines the address shift due to the compression-decompression ratio.  */
        void (*decompressionFunction) (u8bit *, u8bit *, u32bit);   /**<  Pointer to the decompression function.  */

        u8bit *decompressedData;            /**<  Pointer to data array storing decompressed data.  */
        map<u64bit, u8bit *> blockCache;    /**<  Maps addresses to data blocks in the decompressed data array.  */
        u32bit cachedBlocks;                /**<  Stores the number of texture data blocks currently cached.  */

    public:

        /**
         *
         *  Constructor.
         *
         *  @param blocks Defines the number of texture data blocks to keep in the cache.
         *  @param blockSize Defines the size in bytes of a decompressed texture data block.
         *  @param blockMask Defines the address mask used to access data in decompressed texture data blocks.
         *  @param ratioShift Defines the address shift due to the compression-decompression ratio.
         *  @param decompFunction Pointer to the function used to decompress texture data.  The function first
         *  parameter is a pointer to the source data array, the second parameter is a pointer to the destination
         *  data array, the third the number of compressed data bytes to decompress.
         *
         */
         
        CompressedTextureCache(GPUEmulator &emu, u32bit blocks, u32bit blockSize, u64bit blockMask, u32bit ratioShift,
            void (*decompFunc) (u8bit *, u8bit *, u32bit));
         
        /**
         *
         *  Reads texture data from the cache.  If the data for the requested address is not stored
         *  in the cache the corresponding compressed data is read from the emulated memory, 
         *  decompressed and added to the cache.
         *
         *  @param address The address to the requested texture data.
         *  @param data Pointer to a data array where to return the decompressed texture data requested.
         *  @param size Size of the requested texture data in bytes.
         *
         */
        void readData(u64bit address, u8bit *data, u32bit size);
        
        /**
         *
         *  Flushes the cache.
         *
         */
         
        void clear();
    };


    //  Emulator confi(UUIguRation.
    SimParameters simP;             /**<  Stores the emulator configuration parameters.  */

    //  Emulators.
    ShaderEmulator *shEmu;          /**<  Pointer to the shader emulator object.  */
    TextureEmulator *texEmu;        /**<  Pointer to the texture emulator object.  */
    RasterizerEmulator *rastEmu;    /**<  Pointer to the rasterization emulator object.  */
    FragmentOpEmulator *fragEmu;    /**<  Pointer to the fragment operation (z, stencil, color, blend) emulator object.  */

    //  Memory arrays.
    u8bit *gpuMemory;               /**<  Pointer to the array storing the emulated GPU memory.  */
    u8bit *sysMemory;               /**<  Pointer to the array storing the emulated system memory.  */

    //  Emulation structures.
    bool skipBatchMode;                             /**<  Flag that stores if draw calls must be skipped (only state is updated).  */
    bool traceEnd;                                  /**<  Flag that stores if the trace to be emulated has been completely processed.  */
    GPUState state;                                 /**<  Stores the current GPU state (registers).  */
    PixelMapper pixelMapper[MAX_RENDER_TARGETS];    /**<  Mappers of pixel coordinates to memory addresses for the different render targets.  */
    PixelMapper zPixelMapper;                       /**<  Mapper of pixel coordinates to memory addresses for the z stencil buffer.  */
    PixelMapper blitPixelMapper;                    /**<  Mapper of pixel coordinates to memory addresses used by the blitter emulation function.  */

    std::vector<u32bit> indexList;                  /**<  Stores the indices processed for the current draw call.  */
    std::map<u32bit, ShadedVertex*> vertexList;     /**<  Maps indices to vertices (and the associated attributes) for the current draw call.  */

    //  Caches for compressed texture data.
    
    //
    //  NOTE!!!  Texel addresses must be aligned to 4 bytes.
    //
    CompressedTextureCache cacheDXT1RGB;        /**<  Cache for DXT1 RGB texture data.  */
    CompressedTextureCache cacheDXT1RGBA;       /**<  Cache for DXT1 RGBA texture data.  */
    CompressedTextureCache cacheDXT3RGBA;       /**<  Cache for DXT3 RGBA texture data.  */
    CompressedTextureCache cacheDXT5RGBA;       /**<  Cache for DXT5 RGBA texture data.  */

    //
    //  NOTE!!!  Texel addresses are unaligned.
    //
    CompressedTextureCache cacheLATC1;          /**<  Cache for LATC1 texture data.  */
    CompressedTextureCache cacheLATC1_SIGNED;   /**<  Cache for LATC1_SIGNED texture data.  */

    //
    //  NOTE!!!  Texel addresses must be aligned to 2 bytes.
    //
    CompressedTextureCache cacheLATC2;          /**<  Cache for LATC2 texture data.  */
    CompressedTextureCache cacheLATC2_SIGNED;   /**<  Cache for LATC2_SIGNED texture data.  */

    //  Trace reader+driver.
    TraceDriverInterface *trDriver;     /**<  Pointer to the objects used to obtain AGP Transactions that drive the emulation.  */

    u32bit frameCounter;        /**<  Stores the current frame number.  */
    u32bit batchCounter;        /**<  Stores the current draw call (batch) number.  */
    u32bit triangleCounter;     /**<  Stores the number of triangles processed in the current draw call.  */

    bool abortEmulation;        /**<  Flag that stores if the emulation must be aborted due to an 'external' event.  */
    
    //  Debug variables
    bool traceLog;              /**<  Flag that stores if the debug log is enabled.  */
    bool traceBatch;            /**<  Flag that stores if the debug log for a draw call is enabled.  */
    bool traceVertex;           /**<  Flag that stores if the debug log for a vertex is enabled.  */
    bool tracePixel;            /**<  Flag that stores if the debug log for a pixel is enabled.  */
    bool traceTexture;          /**<  Flag that stores if dumping debug information about the texture operations for the current pixel is enabled.  */
    bool traceFShader;          /**<  Flag that stores if dumping debug information about the fragment shader execution for the current pixel is enabled.  */
    bool traceVShader;          /**<  Flag that stores if dumping debug information about the vertex shader execution for the current vertex is enabled.  */

    u32bit watchBatch;          /**<  Defines the identifier or sequence number for a draw call for which to dump debug information.  */
    u32bit watchPixelX;         /**<  Defines the horizontal coordinate for the pixel for which debug information will be dumped.  */
    u32bit watchPixelY;         /**<  Defines the vertical coordinate for the pixel for which debug information will be dumped.  */
    u32bit watchIndex;          /**<  Defines the index for the vertex for which debug information will be dumped.  */

    //  Debug/validation.
    bool validationMode;                /**<  Flag that stores if the validation mode is enabled in the emulator.  */
    ShadedVertexMap shadedVertexLog;    /**<  Map indexed by vertex index storing the shaded vertices in the current draw call.  */
    VertexInputMap vertexInputLog;      /**<  Map indexed by vertex index storing the vertices read in the current draw call.  */
    u32bit bytesPerPixelColor[MAX_RENDER_TARGETS];          /**<  Stores the bytes per color pixel.  */
    u32bit bytesPerPixelDepth;                              /**<  Stores the bytes per depth/stencil pixel.  */
    FragmentQuadMemoryUpdateMap zstencilMemoryUpdateMap;    /**<  Map indexed by fragment identifier storing updates to the z stencil buffer.  */
    FragmentQuadMemoryUpdateMap colorMemoryUpdateMap[MAX_RENDER_TARGETS];   /**<  Map indexed by fragment identifier storing updates to the color buffer.  */
    
public:

    /**
     *
     *  GPUEmulator constructor.
     *
     *  Creates and initializes the different emulator objects.  Allocates the emulated GPU memory.
     *  Initializes the caches for compressed textures.
     *
     *  @param simP Configuration parameters for the emulator.
     *  @param trDriver Pointer to a TraceDriverInterface object from which to obtain AGP Transactions.
     * 
     *  @return An initialized GPUEmulator object.
     *
     */

    GPUEmulator(gpu3d::SimParameters simP, TraceDriverInterface *trDriver);

    /**
     *
     *  Implements a fire and forget emulation main loop.
     *
     */
     
    void emulationLoop();

    /**
     *
     *  Implements the rendering of the current draw call (GPU_DRAW command).
     *
     *  Calls setupDraw() and cleanup() functions for initialization and at the end of the processing.
     *  Iteratively calls to emulateStreamer() and emulatePrimitiveAssembl() to implement instancing.
     *
     */
     
    void draw();

    /**
     *
     *  Initializes the state at the start of a draw call.
     *
     *  Updates the different emulator objects and initializes the required structures used to render
     *  a draw call.
     *
     */
     
    void setupDraw();

    /**
     *
     *  Clean the structures used to render a draw call.
     *
     */
     
    void cleanup();


    /**
     *
     *  Emulates the Command Processor.
     *
     *  Processes the AGP Transaction passed as a parameter.
     *
     *  @param agpTransaction Pointer to the AGP Transaction to emulate.
     *
     */
    void emulateCommandProcessor(AGPTransaction *agpTransaction);
    
    /**
     *
     *  Emulates the Streamer.
     *
     *  Generates or fetches vertex indices, reads vertex attributes from the emulated memory, issues vertices to the
     *  vertex shader emulator.  Supports instancing.
     *
     *  @param instante The current instance number/identifier for the draw call being processed.
     *
     */
     
    void emulateStreamer(u32bit instance);
    
    /**
     *
     *  Emulates the vertex shader.
     *
     *  Shades a vertex.  Calls to emulateTextureUnit() if required.
     *
     *  @param vertex Pointer to a ShadedVertex container with the data associated with the vertex to process.
     *  The result is returned in the same object.
     *
     */
    void emulateVertexShader(ShadedVertex *vertex);
    
    /**
     *
     *  Emulate Primitive Assembly.
     *
     *  Assembles the different primitive types into triangles and iteratively calls to emuleRasterization().
     *
     */
     
    void emulatePrimitiveAssembly();
    
    /**
     *
     *  Emulate Rasterization.
     *
     *  Performs clipping, triangle setup and culling, generates fragments, interpolates the fragment attributes,
     *  sends the fragments to the fragment shader emulator, sends fragments to the z stencil test emulator, sends
     *  fragments to the color write emulator for a single triangle.
     *
     *  @param vertex1 Pointer to a ShadedVertex container with the triangle first vertex.
     *  @param vertex2 Pointer to a ShadedVertex container with the triangle second vertex.
     *  @param vertex3 Pointer to a ShadedVertex container with the triangle third vertex.
     *
     */
    void emulateRasterization(ShadedVertex *vertex1, ShadedVertex *vertex2, ShadedVertex *vertex3);
    
    /**
     *
     *  Emulates Fragment Shading.
     *
     *  Shades a 2x2 fragment tile.  Calls to emulateTextureUnit().
     *
     *  @param quad Pointer to an array of ShadedFragment containers storing the data associated with the
     *  fragments to shade.
     *
     */
     
    void emulateFragmentShading(ShadedFragment **quad);
    
    /**
     *
     *  Emulates the Texture Unit.
     *
     *  Emulates a texture access for a 2x2 fragment tile.
     *
     *  @param texAccess Pointer to a TextureAccess object with the information associated with the
     *  texture request fora 2x2 fragment tile.
     *
     */
     
    void emulateTextureUnit(TextureAccess *texAccess);
    
    /**
     *
     *  Emulate Color Write.
     *
     *  Blends and updates the content of the active render targets for a 2x2 fragment tile.
     *
     *  @param quad Pointer to an array of ShadedFragment containers storing the data associated with
     *  the 2x2 fragment tile to process.
     *
     */
     
    void emulateColorWrite(ShadedFragment **quad);
    
    /**
     *
     *  Emulate Z Stencil Test.
     *
     *  Peforms the z and stencil tests and updates the z and stencil buffer.
     *
     *  @param quad Pointer to an array of ShadedFragment containers storing the data associated with
     *  the 2x2 fragment tile to process.
     *  
     */

    void emulateZStencilTest(ShadedFragment **quad);
    
    /**
     *
     *  Emulates the blitter.
     *
     */
    void emulateBlitter();
    
    /**
     *
     *  Resets the GPU state.
     *
     */
     
    void resetState();
    
    /**
     *
     *  Clears the z and stencil buffer.
     *
     */
     
    void clearZStencilBuffer();
    
    /**
     *
     *  Clears the color buffer (render target 0).
     *
     */
     
    void clearColorBuffer();
    
    /**
     *
     *  Loads a vertex program.
     *
     */
     
    void loadVertexProgram();
    
    /**
     *
     *  Loads a fragment program.
     *
     */
     
    void loadFragmentProgram();
    
    /**
     *
     *  Loads a shader program.
     *
     */
     
    void loadShaderProgram();

    /**
     *
     *  Processes a GPU register write and updates the GPU state.
     *
     *  @param reg Defines the GPU register to write.
     *  @param subReg Defines the component of the GPU register to write.
     *  @param data Defines the value to write into the GPU register.
     *
     */
     
    void processRegisterWrite(GPURegister reg, u32bit subReg, GPURegData data);

    /**
     *
     *  Dumps the content of the defined render target to an image file
     *
     *  @param filename Name/path of the image file where to dump the render target content.
     *  @param rt Identifier of the render target to dump.
     *  @param alpha Set to TRUE to dump alpha (broadcast to RGB channels) rather than alpha.
     *
     */
     
    void dumpFrame(char *filename, u32bit rt, bool dumpAlpha = false);
        
    /**
     *
     *  Dumps the content of the depth buffer to an image file.
     *
     *  @param filename Name/path of the image file where to dump the depth buffer content.
     *
     */

    void dumpDepthBuffer(char *filename);

    /**
     *
     *  Dumps the content of the stencil buffer to an image file.
     *
     *  @param filename Name/path of the image file where to dump the stencil buffer content.
     *
     */
     
    void dumpStencilBuffer(char *filename);
    
    /**
     *
     *  Converts color data from 4-component 8-bit normalized format to
     *  4-component 32-bit float point format.
     *
     *  The amount of data converted corresponds with 4 fragments/samples.
     *
     *  @param in Pointer to a data array with 4-component 8-bit normalized color data.
     *  @param out Pointer to an array of QuadFloat where to store the converted data in
     *  4-component 32-bit float point format.
     *
     */

    void colorRGBA8ToRGBA32F(u8bit *in, QuadFloat *out);

    /**
     *
     *  Converts color data from 4-component 16-bit normalized format to
     *  4-component 32-bit float point format.
     *
     *  The amount of data converted corresponds with 4 fragments/samples.
     *
     *  @param in Pointer to a data array with 4-component 16-bit normalized color data.
     *  @param out Pointer to an array of QuadFloat where to store the converted data in
     *  4-component 32-bit float point format.
     *
     */

    void colorRGBA16ToRGBA32F(u8bit *in, QuadFloat *out);

    /**
     *
     *  Converts color data from 2-component 16-bit float point format to
     *  4-component 32-bit float point format.
     *
     *  The amount of data converted corresponds with 4 fragments/samples.
     *
     *  @param in Pointer to a data array with 2-component 16-bit float point color data.
     *  @param out Pointer to an array of QuadFloat where to store the converted data in
     *  4-component 32-bit float point format.
     *
     */
     
    void colorRG16FToRGBA32F(u8bit *in, QuadFloat *out);

    /**
     *
     *  Converts color data from 4-component 16-bit float point format to
     *  4-component 32-bit float point format.
     *
     *  The amount of data converted corresponds with 4 fragments/samples.
     *
     *  @param in Pointer to a data array with 4-component 16-bit float point color data.
     *  @param out Pointer to an array of QuadFloat where to store the converted data in
     *  4-component 32-bit float point format.
     *
     */

    void colorRGBA16FToRGBA32F(u8bit *in, QuadFloat *out);

    /**
     *
     *  Converts color data from 1-component 32-bit float point format to
     *  4-component 32-bit float point format.
     *
     *  The amount of data converted corresponds with 4 fragments/samples.
     *
     *  @param in Pointer to a data array with 1-component 32-bit float point color data.
     *  @param out Pointer to an array of QuadFloat where to store the converted data in
     *  4-component 32-bit float point format.
     *
     */

    void colorR32FToRGBA32F(u8bit *in, QuadFloat *out);

    /**
     *
     *  Converts color data from 4-component 32-bit float point format to 
     *  4-component 8-bit normalized format.
     *
     *  The amount of data converted corresponds with 4 fragments/samples.
     *
     *  @param in Pointer to a data array with 4-component 32-bit float point data.
     *  @param out Pointer to an array of QuadFloat where to store the converted data in
     *  4-component 8-bit normalized format.
     *
     */

    void colorRGBA32FToRGBA8(QuadFloat *in, u8bit *out);

    /**
     *
     *  Converts color data from 4-component 32-bit float point format to 
     *  4-component 16-bit normalized format.
     *
     *  The amount of data converted corresponds with 4 fragments/samples.
     *
     *  @param in Pointer to a data array with 4-component 32-bit float point data.
     *  @param out Pointer to an array of QuadFloat where to store the converted data in
     *  4-component 16-bit normalized format.
     *
     */

    void colorRGBA32FToRGBA16(QuadFloat *in, u8bit *out);

    /**
     *
     *  Converts color data from 4-component 32-bit float point format to 
     *  2-component 16-bit float point format.
     *
     *  The amount of data converted corresponds with 4 fragments/samples.
     *
     *  @param in Pointer to a data array with 4-component 32-bit float point data.
     *  @param out Pointer to an array of QuadFloat where to store the converted data in
     *  2-component 16-bit float point format.
     *
     */

    void colorRGBA32FToRG16F(QuadFloat *in, u8bit *out);

    /**
     *
     *  Converts color data from 4-component 32-bit float point format to 
     *  4-component 16-bit float point format.
     *
     *  The amount of data converted corresponds with 4 fragments/samples.
     *
     *  @param in Pointer to a data array with 4-component 32-bit float point data.
     *  @param out Pointer to an array of QuadFloat where to store the converted data in
     *  4-component 16-bit float point format.
     *
     */

    void colorRGBA32FToRGBA16F(QuadFloat *in, u8bit *out);

    /**
     *
     *  Converts color data from 4-component 32-bit float point format to 
     *  1-component 32-bit float point format.
     *
     *  The amount of data converted corresponds with 4 fragments/samples.
     *
     *  @param in Pointer to a data array with 4-component 32-bit float point data.
     *  @param out Pointer to an array of QuadFloat where to store the converted data in
     *  1-component 32-bit float point format.
     *
     */
     
    void colorRGBA32FToR32F(QuadFloat *in, u8bit *out);
    
    /**
     *
     *  Converts color data in 32-bit float point format from linear to gamma sRGB color space.
     *
     *  The amount of data converted corresponds with 4 fragments/samples.
     *
     *  @param in Pointer to a QuadFloat array with the color data.  The same array is
     *  used to return the result of the conversion.
     *
     */
     
    void colorLinearToSRGB(QuadFloat *in);

    /**
     *
     *  Converts color data in 32-bit float point format from gamma sRGB to linear color space.
     *
     *  The amount of data converted corresponds with 4 fragments/samples.
     *
     *  @param in Pointer to a QuadFloat array with the color data.  The same array is
     *  used to return the result of the conversion.
     *
     */

    void colorSRGBToLinear(QuadFloat *in);
    
    /**
     *
     *  Selects the type of emulated memory to be accessed for the defined GPU memory address.
     *  Returns a pointer to either the emulated GPU memory or system memory data array.
     *
     *  @param address The address in the GPU memory address space.
     *
     *  @return A pointer to the data array for the associated memory type.
     *
     */

    u8bit *selectMemorySpace(u32bit address);
    
    /**
     *
     *  Gets the ShadedVertex container object associated with the defined vertex index.
     *
     *  @param index Vertex index for which to obtain the associated ShadedVertex container.
     *
     *  @return A pointer to a ShadedVertex container object for the index.  Returns NULL if
     *  the index is not found.
     *
     */ 
     
    ShadedVertex *getVertex(u32bit index);
    
    /**
     *
     *  Implements triangle culling after triangle setup.
     *
     *  @param triangleID Identifier of the Setup Triangle in the Rasterizer Emulator.
     *
     *  @return TRUE if the triangle is culled, FALSE otherwise.
     *
     */
     
    bool cullTriangle(u32bit triangleID);
    
    /**
     *
     *  Reads texture data from the emulated memory.  Decompresses texture data if required.
     *
     *  Uses the caches for texture decompressed data if required.
     *
     *  @param texelAddress Address in texture address space of the texture data to read.
     *  @param size Size in bytes of the texture data to read.
     *  @param data Pointer to a byte array where to store the read texture data.
     *
     */
     
    void readTextureData(u64bit texelAddress, u32bit size, u8bit *data);
    
    /**
     *
     *  Implements the texel to texture memory address mapping using Morton filling function.
     *
     *  Used by the blitter emulator.
     *
     *  @param i Texel horizontal coordinate.    
     *  @param j Texel vertical coordinate.
     *  @param blockDim Power for the dimension a texel data block defined as 2^n x 2^n texels.
     *  @param sBlockDim Power for the dimension a texel data super block defined as 2^n x 2^n blocks.
     *  @param width Horizontal dimension of the texture in texels.
     *
     */
    u32bit texel2MortonAddress(u32bit i, u32bit j, u32bit blockDim, u32bit sBlockDim, u32bit width);

    //  Trace functions.
    void printShaderInstruction(ShaderInstruction *shInstr);
    void printTextureAccessInfo(TextureAccess *texAccess);
   
    /**
     *
     *  Prints the values of the operands for the decoded shaded instruction.
     *  Should be called before the execution of the instruction.
     *
     *  @param shInstrDec Pointer to a ShaderInstructionDecoded object.
     *
     */

    void printShaderInstructionOperands(ShaderInstruction::ShaderInstructionDecoded *shInstrDec);

    /**
     *
     *  Prints the values of the results for the decoded shaded instruction.
     *  Should be called after the execution of the instruction.
     *
     *  @param shInstrDec Pointer to a ShaderInstructionDecoded object.
     *
     */

    void printShaderInstructionResult(ShaderInstruction::ShaderInstructionDecoded *shInstrDec);

    /**
     *
     *  Abort emulation.
     *
     */
     
     void setAbortEmulation();

    /**
     *
     *  Get current frame counter.
     *
     *  @return The current frame counter.
     *
     */
     
    u32bit getFrameCounter();
    
    /**
     *
     *  Get current draw call counter.
     *
     *  @return The current draw call (batch) counter.
     *
     */
    
    u32bit getBatchCounter();
    
    /**
     *
     *  Get current triangle counter.
     *
     *  @return The current triangle counter.
     *
     */
     
     u32bit getTriangleCounter();
     
     /**
      *
      *  Set the validation mode in the emulator.
      *
      *  @param enable Defines if the validation mode is enabled or disabled.
      *
      */
      
    void setValidationMode(bool enable);

    /**
     *
     *  Get the shaded vertex map for the current draw call.
     *
     *  @return A reference to the shaded vertex map.
     *
     */
     
    ShadedVertexMap &getShadedVertexLog();

    /**
     *
     *  Get the read vertex map for the current draw call.
     *
     *  @return A reference to the read vertex map.
     *
     */
     
    VertexInputMap &getVertexInputLog();

    /**
     *
     *  Enable/disable the full emulator trace log.
     *
     *  @param enable Boolean value that defines if the full emulator trace log is to be enabled or disabled.
     *
     */
     
     void setFullTraceLog(bool enable);
     
     /**
      *
      *  Enable/disable the emulator trace log for a defined draw call (batch).
      *
      *  @param enable Boolean value that defines the draw call trace log is to be enabled or disabled.
      *  @param batch The identifier of the draw call to log.
      *
      */
      
    void setBatchTraceLog(bool enable, u32bit batch);
    
     /**
      *
      *  Enable/disable the emulator trace log for a defined vertex.
      *
      *  @param enable Boolean value that defines the vertex trace log is to be enabled or disabled.
      *  @param index The index, identifier, of the vertex to log.
      *
      */
      
    void setVertexTraceLog(bool enable, u32bit index);
    
     /**
      *
      *  Enable/disable the emulator trace log for a defined pixel.
      *
      *  @param enable Boolean value that defines the pixel trace log is to be enabled or disabled.
      *  @param x The horizontal coordinate of the pixel to trace.
      *  @param y The vertical coordinate of the pixel to trace.
      *
      */
      
    void setPixelTraceLog(bool enable, u32bit x, u32bit y);

    /**
     *
     *  Get the z stencil memory update map for the current draw call.
     *
     *  @return A reference to the z stencil memory update map for the current draw call.
     *
     */    
     
    FragmentQuadMemoryUpdateMap &getZStencilUpdateMap();
    
    /**
     *
     *  Get the color memory update map of the defined render target for the current draw call.
     *
     *  @param rt Idenfier for the render target for which to obtain the color memory update map.
     *
     *  @return A reference to the color memory update map of the defined render target for the current draw call.
     *
     */
  
    FragmentQuadMemoryUpdateMap &getColorUpdateMap(u32bit rt);
    
    /**
     *
     *  Get the format defined for the specified render target.
     *
     *  @param rt Identifier of the render target for which to obtain the format.
     *
     *  @return The format of the specified render target.
     *
     */
  
    TextureFormat getRenderTargetFormat(u32bit rt);
    
    /**
     *
     *  Save a snapshot of the current emulator state.
     *
     */
     
    void saveSnapshot();  
    
    /**
     *
     *  Load a snapshot of the emulator state.
     *
     *
     */
     
    void loadSnapshot();
    
    /**
     *
     *  Set the skip draw call mode.
     *
     *  @param enable Boolean used to enable or disable the skip draw call mode.
     *
     */
    
    void setSkipBatch(bool enable);
    
    
};  // class GPUEmulator


};  //  namespace gpu3d

#endif
