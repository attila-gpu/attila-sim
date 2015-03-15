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
 * $RCSfile: GPUDriver.h,v $
 * $Revision: 1.38 $
 * $Author: csolis $
 * $Date: 2008-05-31 14:50:42 $
 *
 * GPU Driver definitions.
 *
 */

#ifndef GPUDRIVER_H
    #define GPUDRIVER_H

#include "GPU.h"
#include "AGPTransaction.h"
#include "MemorySpace.h"
#include <vector>
#include "RegisterWriteBuffer.h"
#include "ShaderProgramSched.h"

/**
 * Driver for bGPU
 *
 * @version 0.2
 * @date 07/10/2003 ( previous 22/09/2003 )
 * @author Carlos González Rodríguez - cgonzale@ac.upc.es
 */

//#define DUMP_SYNC_REGISTERS_TO_GPU

class GPUDriver {

private:

    friend class RegisterWriteBuffer;

    /**
     * Write buffer for registers
     */
    RegisterWriteBuffer registerWriteBuffer;

    //ShaderProgramSched vshSched;
    //ShaderProgramSched fshSched;
    ShaderProgramSched shSched;

    u64bit batchCounter;

    // statistics counters
    u32bit agpTransactionsGenerated;
    u32bit memoryAllocations;
    u32bit memoryDeallocations;
    u32bit mdSearches;
    u32bit addressSearches;
    u32bit memPreloads;
    u32bit memWrites;
    u32bit memPreloadBytes;
    u32bit memWriteBytes;

    bool preloadMemory;

    /**
     * Descriptor for a sequence of GPU memory blocks
     */
    struct _MemoryDescriptor
    {
        // both physical addresses in GPU
        u32bit size; // size reclaimed with obtain memory...
        u32bit firstAddress;
        u32bit lastAddress;
        u32bit memId;
        u64bit lastBatchWritten; // used to support batch pipelining
        u32bit highAddressWritten; // debug
    };

    /**
     * _MemoryDescriptor map with current _MemoryDescriptors managed by the GPUDriver
     */
    std::map<u32bit, _MemoryDescriptor*> memoryDescriptors;
    


    /// Memory regions marked to be deleted, will be deleted when the next batch will finish
    /// This is done to support batch pipelining (avoids false locks)
    std::vector<u32bit> pendentReleases;

    /**
     * Finds the _MemoryDescriptor which have a Address in its range address
     *
     * @param physicalAddress the matching address
     *
     * @returns the _MemoryDescriptor that have 'physicalAddress' in its range
     */
    _MemoryDescriptor* _findMDByAddress( u32bit physicalAddress );

    /**
     * Finds the _MemoryDescriptor which have memId equals to memId parameter
     *
     * @param memId memId Identifier for the _MemoryDescriptor we want
     *
     * @returns the _MemoryDescriptor searched
     */
    _MemoryDescriptor* _findMD( u32bit memId );

    /**
     * Creates a new _MemoryDescriptor given a initial and final physical address
     *
     * The _MemoryDescritor is inserted in the _MemoryDescritor list
     *
     * @param firstAddress start memory address
     * @param lastAddress final memory address
     * @param size real reclaimed size by user of obtainMemory
     *
     * @returns a new _MemoryDescriptor initialized ( with memId assigned )
     */
    _MemoryDescriptor* _createMD( u32bit firstAddress, u32bit lastAddress, u32bit size );

    /**
     * Release the memory pointed by memId descriptor
     *
     * @param memId a memory descriptor
     */
    void _releaseMD( u32bit memId );


    /**
     *
     *  Search a memory map for empty blocks.
     *
     *  @param first Reference to a variable where to store the index to the first
     *  consecutive block found.
     *  @param map Pointer to the memory map where to search for blocks.
     *  @param memSize Size of the memory map (in KBytes).
     *  @param memRequired Amount of memory requested (in bytes).
     *
     *  @return The number of consecutive blocks found for the requested memory.
     *  Returns 0 if no consecutive blocks large enough are found.
     *
     */

    u32bit searchBlocks(u32bit &first, u8bit *map, u32bit memSize, u32bit memRequired);

    /**
     * Encapsulates AGP dispatch
     *
     * @note this is the only function that should be modified if AGPTransaction
     *       dispatches will be changed in the future
     *
     * @param agpt the AGPTransaction to be sent
     */
    bool _sendAGPTransaction( gpu3d::AGPTransaction* agpt );

    /**
     * Map for occupied memory blocks
     */
    u8bit* gpuMap;
    u8bit* systemMap;

    u32bit lastBlock;   /**<  Stores the address of the last block tested for free in the memory allocation algorithm.  */

    u32bit gpuAllocBlocks;  /**<  Number of blocks currently allocated in GPU memory.  */
    u32bit systemAllocBlocks;   /**<  Number of blocks currently allocated in system memory.  */

    /**
     * Used to generate memory descriptors
     */
    u32bit nextMemId;

    /**
     * Maximum number of buffered AGPTRansactions
     * Must be power of 2
     */
    enum { agpBufferSize = 2048 };

    /**
     * physical card memory ( in Kbytes ) (jnow is configurable)
     */
    //enum { physicalCardMemory = 16384 };

    /**
     * Minimum assignable size per obtainMemory command ( in Kbytes )
     */
    enum { BLOCK_SIZE = 4 };

    /**
     * Outstanding AGPTRansactions
     */
    gpu3d::AGPTransaction** agpBuffer;

    /**
     * Position available in the AGPBufferQueue to store an AGPTRansaction
     */
    int in;

    /**
     * First item to be returned from the AGPBufferQueue
     */
    int out;

    /**
     * Count outstanding AGPTransactions
     */
    int agpCount;

    /**
     * Driver pointer
     */
    static GPUDriver* driver;

    /**
     * Current resolution in GPU
     */
    u32bit hRes, vRes;
    bool setResolutionCalled;           /**<  GPU resolution was set.  */

    //
    //  GPU parameters
    //
    u32bit gpuMemory;
    u32bit systemMemory;
    u32bit blocksz;
    u32bit sblocksz;
    u32bit scanWidth;
    u32bit scanHeight;
    u32bit overScanWidth;
    u32bit overScanHeight;
    u32bit fetchRate;
    bool doubleBuffer;
    bool forceMSAA;
    u32bit forcedMSAASamples;
    bool forceFP16ColorBuffer;
    bool secondInterleavingEnabled;
    bool convertShaderProgramToLDA;             /**<  Stores if the conversion of input attribute reads to LDA instructions in shader programs is enabled.  */
    bool convertShaderProgramToSOA;             /**<  Stores if the conversion of shader programs to SOA format is enabled.  */
    bool enableShaderProgramTransformations;    /**<  Stores if shader program transformation/translation is enabled.  */
    bool microTrianglesAsFragments;             /**<  Stores if micro triangles are processed directly as fragments, which enables the microtriangle fragment shader transformation.  */
    bool setGPUParametersCalled;        /**<  GPU parameters received from the simulator.  */
    int batch;
    int frame;
   
    void* ctx;

    /**
     * Constructor performs driver setup automatically
     */
    GPUDriver();

    static u8bit _mortonTable[];
    void MortonTableBuilder();
    u32bit _mortonFast(u32bit size, u32bit i, u32bit j);
    u32bit _texel2address(u32bit width, u32bit blockSz, u32bit sBlockSz, u32bit i, u32bit j);
    f64bit ceil(f64bit x);
    f64bit logTwo(f64bit x);

public:

    /**
     * Shaders attribs
     */
    enum ShAttrib
    {
        VS_VERTEX = 0,
        VS_WEIGHT = 1,
        VS_NORMAL = 2,
        VS_COLOR = 3,
        VS_SEC_COLOR = 4,
        VS_FOG = 5,
        VS_SEC_WEIGHT = 6,
        VS_THIRD_WEIGHT = 7,
        VS_TEXTURE_0 = 8,
        VS_TEXTURE_1 = 9,
        VS_TEXTURE_2 = 10,
        VS_TEXTURE_3 = 11,
        VS_TEXTURE_4 = 12,
        VS_TEXTURE_5 = 13,
        VS_TEXTURE_6 = 14,
        VS_TEXTURE_7 = 15,
        VS_NOOUTPUT = -1 // Defines an attribute which is not passed from VSh to FSh
    };

    enum MemoryRequestPolicy
    {
        GPUMemoryFirst, // searches for free memory first in GPU local memory and if it fails tries in system memory
        SystemMemoryFirst // searches for free memory first in system memory and if it fails tries in gpu local memory
    };

    // Required information for the GPU Driver when performing the microtriangle transformation.
    struct MicroTriangleRasterSettings
    {
        bool zPerspective;
        bool smoothInterp[gpu3d::MAX_FRAGMENT_ATTRIBUTES];
        bool perspectCorrectInterp[gpu3d::MAX_FRAGMENT_ATTRIBUTES];
        bool faceCullEnabled;
        bool CWFaceCull;
        bool CCWFaceCull;
        unsigned int MSAA_samples;
    };

    /*
     * Default attributte input/output mapping
     * This translation table will be private soon
     *
     * From vertex shader to fragment shader: i -> outputAttrib[i]
     */
    static const s32bit outputAttrib[gpu3d::MAX_VERTEX_ATTRIBUTES];


    struct VertexAttribute
    {
        u32bit stream; /* stream identifier, from 0 to MAX_STREAM_BUFFERS */
        ShAttrib attrib; /* atributte id */
        bool enabled; /* enabled or disabled attribute */
        u32bit md; /* memory descriptor asociated to this VertexAttribute */
        u32bit offset; /* offset */
        u32bit stride; /* stride */
        gpu3d::StreamData componentsType; /* type of components */
        u32bit components; /* number of components per stream element */

        void dump() const;
    };


    /**
     * Returns the maximum number of vertex attribs
     */
    u32bit getMaxVertexAttribs() const;

    /**
     * Returns the maximum number of vertex attribs supported by the GPUïs shader model.
     */
    u32bit getMaxAddrRegisters() const;

    /**
     * Sets the GPU Registers that stores the current display resolution
     */
    void setResolution(u32bit width, u32bit height);

    /**
     * Gets the resolution specified in the trace
     */
    void getResolution(u32bit& width, u32bit& height) const;

    /**
     * Checks if setResolution has been called at least once
     */
    bool isResolutionDefined() const;

    /**
     *
     *  Sets the configuration parameters for the GPU.
     *
     *  This method is called by simulator. This method does not set anything in the simulator.
     *
     *  @param gpuMemSz GPU memory size in KBytes.
     *  @param sysMemSz System memory size in KBytes.
     *  @param blocksz Texture first tile level (block) size (as 2^blocksz x 2^blocksz).
     *  @param sblocksz Texture second tile level (superblock) size (as 2^blocksz x 2^blocksz).
     *  @param scanWidth Rasterizer scan tile height (in fragments).
     *  @param scanHeight Rasterizer scan tile width (in fragments).
     *  @param overScanWidth Rasterizer over scan tile height (in scan tiles).
     *  @param overScanHeight Rasterizer over scan tile widht (in scan tiles).
     *  @param doubleBuffers Uses double buffer (color buffer).
     *  @param forceMSAA Flag that forces multisampling for the whole trace.
     *  @param forcedMSAASamples Number of samples per pixel when multisampling is being forced.
     *  @param forceFP16ColorBuffer Force color buffer format to float16.
     *  @param fetchRate Instructions fetched per cycle and thread in the shader units.
     *  @param memoryControllerV2 Flag that enables the Memory Controller Version 2.
     *  @param secondInterleavingEnabled Flag that enables the two interleaving modes in the
     *  Memory Controller Version 2.
     *  @param convertToLDA Convert input attribute reads to LDA in shader programs.
     *  @param convertToSOA Convert shader programs to SOA format.
     *  @param enableTransformations Enable shader program transformations/optimizations by the driver.
     *  @param microTrisAsFrags Microtriangles are processed directly as fragments, which enables the microtriangle fragment shader transformation.
     * 
     */

    void setGPUParameters(u32bit gpuMemSz, u32bit sysMemSz, u32bit blocksz, u32bit sblocksz,
        u32bit scanWidth, u32bit scanHeight, u32bit overScanWidth, u32bit overScanHeight,
        bool doubleBuffer, bool forceMSAA, u32bit forcedMSAASamples, bool forceF16ColorBuffer,
        u32bit fetchRate, bool memoryControllerV2, bool secondInterleavingEnabled,
        bool converToLDA, bool convertToSOA, bool enableTransformations, bool microTrisAsFrags
        );

    /**
     *
     *  Stores the GPU configuration parameters in the variables passed as references.
     *
     *  @param gpuMemSz GPU memory size in KBytes.
     *  @param sysMemSz System memory size in KBytes.
     *  @param blocksz Texture first tile level (block) size (as 2^blocksz x 2^blocksz).
     *  @param sblocksz Texture second tile level (superblock) size (as 2^blocksz x 2^blocksz).
     *  @param scanWidth Rasterizer scan tile height (in fragments).
     *  @param scanHeight Rasterizer scan tile width (in fragments).
     *  @param overScanWidth Rasterizer over scan tile height (in scan tiles).
     *  @param overScanHeight Rasterizer over scan tile widht (in scan tiles).
     *  @param doubleBuffers Uses double buffer (color buffer).
     *  @param fetchRate Instructions fetched per cycle and thread in the shader units.
     *
     */

    void getGPUParameters(u32bit& gpuMemSz, u32bit& sysMemSz, u32bit& blocksz, u32bit& sblocksz,
        u32bit& scanWidth, u32bit& scanHeight, u32bit& overScanWidth, u32bit& overScanHeight,
        bool& doubleBuffer, u32bit& fetchRate) const;


    /**
     * Path for preloading data in GPU memory in 1 cycle
     * Used to implement GPU Hot Start Simulation
     *
     * @param enable true if the driver must preload data into gpu local memory, false means normal behaviour
     */
    void setPreloadMemory(bool enable);

    bool getPreloadMemory() const;

    /**
     *
     *  Returns the fetch instruction rate for the GPU shader units.
     *
     *  @return The instructions per cycle and thread fetched in the GPU shader units.
     */

    u32bit getFetchRate() const;

    /**
     *
     *  Returns the parameters for texture tiling.
     *
     *  @param blockSz Reference to a variable where to store the first tile level size in 2^n x 2^n texels.
     *  @param sblockSz Reference to a variable where to stored the second tile level size in 2^m x 2^m texels.
     *
     */

    void getTextureTilingParameters(u32bit &blockSz, u32bit &sBlockSz) const;

    /**
     * Reserve memory to front/back buffers and zstencil buffer
     * and sets the address registers of this buffers to point to the reserved memory
     * After the call, the memory descriptors of the buffers are assigned to
     * the parameters, if non zero pointers are provided.
     */
    void initBuffers(u32bit* mdFront = 0, u32bit* mdBack = 0, u32bit* mdZS = 0);

    /**
     *
     *  Gets framebuffer parameters forced by the driver.
     *
     *  @param forceMSAA Reference to a boolean variable that will store if multisampling is forced by the driver.
     *  @param samples Reference to a variable that will store the samples per pixel forced by the driver.
     *  @param forceFP16 Reference to a boolean variable that will store if a FP16 color buffer is forced by the driver. 
     *
     */
     
    void getFrameBufferParameters(bool &forceMSAA, u32bit &samples, bool &forceFP16);
    
    /**
     *
     *  Allocates space in GPU memory for a render buffer with the defined characteristics and returns a memory descriptor
     *
     *
     *  @param width Width in pixels of the requested render buffer.
     *  @param height Height in pixels of the requested render buffer.
     *  @param multisampling Defines if the render buffer supports multiple samples.
     *  @param samples Maximum number of samples to support in the render buffer if multisampling is enabled.
     *  @param format Format of the render buffer.
     *
     *  @return Returns the memory descriptor for the memory regions allocated for the created render buffer.
     *
     */
     
    u32bit createRenderBuffer(u32bit width, u32bit height, bool multisampling, u32bit samples, gpu3d::TextureFormat format);

    /**
     *
     *  Converts the input surface data to the tiled format internally used in ATTILA for render buffers.
     *  A byte array is allocated by the function to store the tiled data.
     *
     *  The surface and render buffer must have the same size.  The surface and render buffer must have
     *  the same format.
     *
     *  @param sourceData Pointer to a byte array storing the input surface data in row major order.
     *  @param width Width in pixels of the render buffer.
     *  @param height Height in pixels of the render buffer.
     *  @param multisampling Defines if the render buffer supports multiple samples.
     *  @param samples Number of samples to store per pixel in the render buffer.
     *  @param format Format of the render buffer.
     *  @param invertColors Invert colors.
     *  @param destData Reference to a pointer where to store the address of the allocated byte array
     *  with the tiled data for the render buffer.
     *  @param destSize Size of the render buffer data array (after applying tiling).
     *
     */
     
    void tileRenderBufferData(u8bit *sourceData, u32bit width, u32bit height, bool multisampling, u32bit samples,
                              gpu3d::TextureFormat format, bool invertColors, u8bit* &destData, u32bit &size);
     
    /**
     *
     *  Converts the input surface data to the mortonized tiled format required for ATTILA.
     *  The function allocates a byte array to store the mortonized tiled surface data and
     *  returns the pointer to that array.
     *
     *  @param originalData Pointer to a byte array with the surface data in a row-major order.
     *  @param width Width in texels of the input surface.
     *  @param height Height in texels of the input surface.
     *  @param depth Depth in texels of the input surface.
     *  @param format Format of the surface.
     *  @param texelSize Number of bytes per texel.
     *  @param mortonDataSize Reference to an integer variable where to return the size of 
     *  surface data after applying morton and tiling.
     *
     *  @return Pointer to a byte array with the surface data tiled morton order.
     *
     */
     
    u8bit* getDataInMortonOrder(u8bit* originalData, u32bit width, u32bit height, u32bit depth,
                                gpu3d::TextureCompression format, u32bit texelSize, u32bit& mortonDataSize);

    /**
     * Returns the amount of local memory available in the simulator (in KBytes)
     */
    //u32bit getGPUMemorySize() const;

    /**
     * Returns block and super block sizes
     */
   // u32bit getGPUBlockSize() const;
    //u32bit getGPUSuperBlockSize() const;

    /**
     * Opaque context pointer
     */
    void setContext(void* ctx) { this->ctx = ctx; }
    void* getContext() { return ctx; }


    bool configureVertexAttribute( const VertexAttribute& va );

    bool setVShaderOutputWritten(ShAttrib attrib, bool isWritten);

    bool setSequentialStreamingMode( u32bit count, u32bit start );

    /**
     * @param stream stream used for indices
     * @param count number of indexes
     * @param mIndices memory descriptor pointing to indices
     * @param offsetBytes offset (in bytes) within mdIndices
     * @param indicesType type of indices
     */
    bool setIndexedStreamingMode( u32bit stream, u32bit count, u32bit start, u32bit mdIndices, u32bit offsetBytes, gpu3d::StreamData indicesType );

    /**
     * Obtains the Singleton class which implements the GPU driver
     *
     * @returns the unique instance of GPUDriver
     */
    static GPUDriver* getGPUDriver();

    /**
     *
     *  Translates, optimizes or transforms the input shader program to the defined GPU architecture.
     *
     *  @param inCode Pointer to a buffer with the input shader program in binary format.
     *  @param inSize Size of the input shader program in binary format.
     *  @param outCode Pointer to a buffer where to store the transformed shader program in binary format.
     *  @param outSize Reference to a variable storing the size of the buffer where to store the
     *  transformed shader program.  The variable will be overwritten with the actual size of the transformed
     *  shader program.
     *  @param isVertexProgram Tells if the input shader program is a vertex shader program.  Used to activate
     *  the conversion of input attribute reads into load instructions.
     *  @param maxLiveTemRegs Reference to a variable where to store the maximum number of temporal registers
     *  alive at any point of the translated/optimized/transformed shader program.
     *  @param settings required API information to perform the microtriangle transformation.
     *
     */
     
    void translateShaderProgram(u8bit *inCode, u32bit inSize, u8bit *outCoce, u32bit &outSize, bool isVertexProgram,
                                u32bit &maxLiveTempRegs, MicroTriangleRasterSettings settings);     

    /**
     *
     *  Assembles a shader program written in ATTILA Shader Assembly.
     *
     *  @param program Pointer to a byte array with the shader program written in ATTILA Shader Assembly.
     *  @param code Pointer to a byte array where to store the assembled program.
     *  @param size Size of the byte array where to store the assembled program.
     *
     *  @return Size in bytes of the assembled program.
     *
     */
          
    static u32bit assembleShaderProgram(u8bit *program, u8bit *code, u32bit size);

    /**
     *
     *  Disassembles a shader program to ATTILA Shader Assembly.
     *
     *  @param code Pointer to a byte array with shader program byte code.
     *  @param codeSize Size of the encoded shader program in bytes.
     *  @param program Pointer to a byte array where to store the shader program in ATTILA Shader Assembly.
     *  @param size Size of the byte array where to store the disassembled program.
     *  @param disableEarlyZ Reference to a boolean variable where to store if the shader program requires
     *  early z to be disabled.
     *
     *  @return Size in bytes of the disassembled program.
     *
     */
          
    static u32bit disassembleShaderProgram(u8bit *code, u32bit codeSize, u8bit *program, u32bit size, bool &disableEarlyZ);
     
    /**
     * performs initialization for a vertex program
     *
     * initialize the vertex program identified by 'memDesc'
     * Load the program in vertex shader instruction memory, that is, make the program
     * the current vertex program
     *
     * @param memDesc memory descriptor pointing to a vertex program data
     * @param programSize program's size
     * @param startPC first instruction that will be executed for this vertex program
     *
     * @return true if all goes well, false otherwise
     *
     * @code
     *     // ex. Commit a vertex program ( make it current )
     *
     *     // get the singleton GPUDriver
     *     GPUDriver* driver = GPUDriver::getGPUDriver();
     *
     *     // vertex program in binary format
     *     static const u8bit vprogram[] = { 0x09, 0x00, 0x02, 0x00, 0x07, ... , 0x00 };
     *
     *     // assuption : program it is not still in local memory, transfer must be performed
     *     u32bit mdVP = driver->obtainMemory( sizeof(vprogram) );
     *     driver->writeMemory( mdVP, vprogram, sizeof(vprogram) ); // copy program to local memory
     *
     *     // load vertex program to vertex shader instruction memory and set PCinstr to 0
     *     driver->commitVertexProgram( mdVP, sizeof(vprogram), 0 );
     *
     * @endcode
     */
    bool commitVertexProgram( u32bit memDesc, u32bit programSize, u32bit startPC );

    /**
     * Same that commitVertexProgram for fragments
     */
    bool commitFragmentProgram(u32bit memDesc, u32bit programSize, u32bit startPC );

    /**
     * Reserves an amount of data of at least 'sizeBytes' bytes from local GPU memory
     *
     * @param sizeBytes amount of data required
     * @param memorySource source from where to get free memory
     * @return memory descriptor refering the memory just reserved
     *
     * @note now you can write in this memory calling GPUDriver::writeMemory() methods.
     *       Use the descriptor returned as first parameter in GPUDriver::writeMemory()
     */
    u32bit obtainMemory( u32bit sizeBytes, MemoryRequestPolicy memRequestPolicy = GPUMemoryFirst);


    /**
     * Deletes memory pointed by 'md' descriptor
     * mark memory associated with memory descriptor as free
     *
     * @param md memory descriptor pointing the memory we want to release
     */
    void releaseMemory( u32bit md );

    /**
     * Writes data in an specific portion of memory described by a memory descriptor
     *
     * @param md memory descriptor representing a portion of memory previously reserved
     * @param data data buffer we want to write in local memory
     * @param dataSize data size in bytes
     *
     * @return true if the writting was succesfull, false otherwise
     */
    bool writeMemory( u32bit md, const u8bit* data, u32bit dataSize, bool isLocked = false);

    /**
     * Writes data in an specific portion of memory described by a memory descriptor
     *
     * @param md memory descriptor representing a portion of memory previously reserved
     * @param offset logical offset added to the initial address of this memory space
     *        before writting
     * @param data data buffer we want to write in local memory
     * @param dataSize data size in bytes
     *
     * @return true if the writting was succesfull, false otherwise
     *
     * @code
     *     // using writeMemory() offset version to avoid multiple obtainMemory calls
     *
     *     GPUDriver driver = GPUDriver::getGPUDriver();
     *
     *     // ...
     *
     *     u8bit vertexs[] = { ... }; // 4 componets per vertex
     *     u8bit colors[]  = { ... }; // 4 components per color
     *     u8bit normals[] = { ... }; // 3 components per normal
     *
     *     u32bit oVertex = 0; // offset for stream of vertex
     *     u32bit oColors = 4*sizeof(vertexs); // offset for stream of colors
     *     u32bit oNormals = 4*sizeof(colors); // offset for stream of normals
     *     u32bit totalSize =  oColors + oNormals + 3*sizeof(normals);
     *
     *     // only one obtain call
     *     u32bit md = driver->obtainMemory( totalSize );
     *
     *     driver->writeMemory( md, oVertex, vertexs, sizeof(vertexs) );
     *     driver->writeMemory( md, oColors, colors, sizeof(colors) );
     *     driver->writeMemory( md, oNormals, normals, sizeof(normals) );
     */
    bool writeMemory( u32bit md, u32bit offset, const u8bit* data, u32bit dataSize, bool isLocked = false );

    //bool writeMemoryDebug(u32bit md, u32bit offset, const u8bit* data, u32bit dataSize, const std::string& debugInfo);

    /**
     *
     * Preloads data in an specific portion of memory described by a memory descriptor
     *
     * @param md memory descriptor representing a portion of memory previously reserved
     * @param offset logical offset added to the initial address of this memory space
     *        before writting
     * @param data data buffer we want to preload in local memory
     * @param dataSize data size in bytes
     *
     * @return true if the preload was succesful, false otherwise
     *
     */
    bool writeMemoryPreload(u32bit md, u32bit offset, const u8bit* data, u32bit dataSize);

    /**
     * Writes a GPU simulator's register
     *
     * @param regId register's identifier constant
     * @param value new register value
     * @param md Memory descriptor associated with this register write.
     *
     */
    void writeGPURegister( gpu3d::GPURegister regId, gpu3d::GPURegData data, u32bit md = 0 );

    /**
     * Writes a GPU simulator's register
     *
     * @param regId register's identifier constant
     * @param index subRegister indetifier
     * @param data new register value
     * @param md Memory descriptor associated with this register write.
     *
     */
    void writeGPURegister( gpu3d::GPURegister regId, u32bit index, gpu3d::GPURegData data, u32bit md = 0);

    /**
     * Writes a GPU simulator's register with the start address contained in the internal
     * MemoryDescriptor aliased to 'md' handler + the offset (expressed in bytes)
     */
    void writeGPUAddrRegister( gpu3d::GPURegister regId, u32bit index, u32bit md, u32bit offset = 0);

    /**
     *
     *  Reads the GPU simulator register.
     *
     *  @param regID GPU register identifier.
     *  @param data Reference to a variable where to store value read from the register.
     *
     */
     
    void readGPURegister(gpu3d::GPURegister regID, gpu3d::GPURegData &data);

    /**
     *
     *  Reads the GPU simulator register.
     *
     *  @param regID GPU register identifier.
     *  @param index GPU register subregister identifier.
     *  @param data Reference to a variable where to store value read from the register.
     *
     */
     
    void readGPURegister(gpu3d::GPURegister regID, u32bit index, gpu3d::GPURegData &data);
        
    /**
     * Sends a command to the GPU Simulator
     *
     * @param com command to be sent
     */
    void sendCommand( gpu3d::GPUCommand com );

    /**
     * Sends command list to the GPU Simulator
     *
     * @param listOfCommands commands to be sent
     * @param numberOfCommands quantity of commands in the list
     */
     
    void sendCommand( gpu3d::GPUCommand* listOfCommands, int numberOfCommands );

    /**
     *
     *  Signals an event to the GPU simulator.
     *
     *  @param gpuEvent The GPU event to signal to the simulator.
     *  @param eventMsg A message string for the event signaled.
     *
     */
     
    void signalEvent(gpu3d::GPUEvent gpuEvent, std::string eventMsg);
         
    /**
     * Returns the number of mipmaps supported for a texture
     */
    u32bit getMaxMipmaps() const;

    /**
     * Returns the number of texture units available
     */
    u32bit getTextureUnits() const;

    /**
     * Invoked by Command Processor
     * Implements elemental comunication between Driver and AGP Port
     */
    gpu3d::AGPTransaction* nextAGPTransaction();

    /**
     * Destructor
     */
    ~GPUDriver();

    /////////////////////
    // Debug functions //
    /////////////////////
    void printMemoryUsage();
    void dumpMemoryAllocation( bool contents = true  );
    void dumpMD( u32bit md );
    void dumpMDs();
    void dumpAGPBuffer();
    void dumpStatistics();
    void dumpRegisterStatus (int frame, int batch);

};

#endif
