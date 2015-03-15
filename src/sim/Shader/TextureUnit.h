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
 * $RCSfile: TextureUnit.h,v $
 * $Revision: 1.21 $
 * $Author: vmoya $
 * $Date: 2008-03-02 19:09:17 $
 *
 * Shader Texture Unit Box.
 *
 */

/**
 *  @file TextureUnit.h
 *
 *  This file defines the Shader texture unit box.
 *
 *  This box implements the simulation a Texture Unit for
 *  a (fragment) shader unit.
 *
 */

#ifndef _TEXTUREUNIT_

#define _TEXTUREUNIT_

#include "support.h"
#include "GPUTypes.h"
#include "Box.h"
#include "GPU.h"
#include "TextureEmulator.h"
#include "ShaderCommand.h"
#include "ShaderFetch.h"
#include "TextureCacheGen.h"
#include "TextureCache.h"
#include "TextureCacheL2.h"
#include "TextureRequest.h"
#include "TextureResult.h"

//#include <fstream>
#include "zfstream.h"

using namespace std;

namespace gpu3d
{

/**
 *
 *  Texture Unit State.
 *
 */

enum TextureUnitState
{
    TU_READY,   /**<  The Shader accepts new texture read requests.  */
    TU_BUSY     /**<  The Shader can not accept new texture requests.  */
};

/**
 *
 *  Defines the maximum number of loops that can be executed in the filter
 *  pipeline.
 *
 */

static const u32bit MAX_FILTER_LOOPS = 4;

/**
 *
 *  Defines the maximum number of bytes to be read per texel.
 *
 */

static const u32bit MAX_TEXEL_DATA = 16;

/**
 *
 *  TextureUnit implements the texture unit attached to a shader.
 *
 *  Inherits from Box class.
 *
 */

class TextureUnit : public Box
{

private:

#ifdef GPU_TEX_TRACE_ENABLE
    //ofstream texTrace;
    //ofstream texAddrTrace;
    gzofstream texTrace;
    gzofstream texAddrTrace;
#endif

    /*  Texture Unit signals.  */
    Signal *commProcSignal;     /**<  Command signal from the Command Processor.  */
    Signal *readySignal;        /**<  Ready signal to the ShaderDecodeExecute box.  */
    Signal *textDataSignal;     /**<  Texture data signal to the Decode/Execute box.  */
    Signal *textRequestSignal;  /**<  Texture request signal from Decode/Execute.  */
    Signal *filterInput;        /**<  Input signal for the filter unit (simulates filter latency).  */
    Signal *filterOutput;       /**<  Output signal for the filter unit (simulates filter latency).  */
    Signal *addressALUInput;    /**<  Input signal for the address ALU (simulates address calculation latency).  */
    Signal *addressALUOutput;   /**<  Output signal for the address ALU (simulates address calculation latency).  */
    Signal *memDataSignal;      /**<  Data signal from the Memory Controller.  */
    Signal *memRequestSignal;   /**<  Request signal to the Memory Controller.  */

    /*  Texture Unit registers.  */
    bool textureEnabled[MAX_TEXTURES];      /**<  Texture unit enable flag.  */
    TextureMode textureMode[MAX_TEXTURES];  /**<  Current texture mode active in the texture unit.  */
    u32bit textureAddress[MAX_TEXTURES][MAX_TEXTURE_SIZE][CUBEMAP_IMAGES];  /**<  Address in GPU memory of the active texture mipmaps.  */
    u32bit textureWidth[MAX_TEXTURES];      /**<  Active texture width in texels.  */
    u32bit textureHeight[MAX_TEXTURES];     /**<  Active texture height in texels.  */
    u32bit textureDepth[MAX_TEXTURES];      /**<  Active texture depth in texels.  */
    u32bit textureWidth2[MAX_TEXTURES];     /**<  Log2 of the texture width (base mipmap size).  */
    u32bit textureHeight2[MAX_TEXTURES];    /**<  Log2 of the texture height (base mipmap size).  */
    u32bit textureDepth2[MAX_TEXTURES];     /**<  Log2 of the texture depth (base mipmap size).  */
    u32bit textureBorder[MAX_TEXTURES];     /**<  Texture border in texels.  */
    TextureFormat textureFormat[MAX_TEXTURES];  /**<  Texture format of the active texture.  */
    bool textureReverse[MAX_TEXTURES];          /**<  Reverses texture data (little to big endian).  */
    TextureCompression textureCompr[MAX_TEXTURES];  /**<  Texture compression mode of the active texture.  */
    TextureBlocking textureBlocking[MAX_TEXTURES];  /**<  Texture blocking mode for the texture.  */
    bool textD3D9ColorConv[MAX_TEXTURES];           /**<  Defines if the texture color components have to read in the order defined by D3D9.  */
    bool textD3D9VInvert[MAX_TEXTURES];             /**<  Defines if the texture u coordinate has to be inverted as defined by D3D9.  */
    QuadFloat textBorderColor[MAX_TEXTURES];    /**<  Texture border color.  */
    ClampMode textureWrapS[MAX_TEXTURES];       /**<  Texture wrap mode for s coordinate.  */
    ClampMode textureWrapT[MAX_TEXTURES];       /**<  Texture wrap mode for t coordinate.  */
    ClampMode textureWrapR[MAX_TEXTURES];       /**<  Texture wrap mode for r coordinate.  */
    bool textureNonNormalized[MAX_TEXTURES];    /**<  Texture coordinates are non-normalized.  */
    FilterMode textureMinFilter[MAX_TEXTURES];  /**<  Texture minification filter.  */
    FilterMode textureMagFilter[MAX_TEXTURES];  /**<  Texture Magnification filter.  */
    bool textureEnableComparison[MAX_TEXTURES]; /**<  Texture Enable Comparison Filter (PCF).  */
    CompareMode textureComparisonFunction[MAX_TEXTURES];   /**<  Texture Comparison Function (PCF).  */
    bool textureSRGB[MAX_TEXTURES];         /**<  Texture sRGB space to linear space conversion.  */
    f32bit textureMinLOD[MAX_TEXTURES];     /**<  Texture minimum lod.  */
    f32bit textureMaxLOD[MAX_TEXTURES];     /**<  Texture maximum lod.  */
    f32bit textureLODWays[MAX_TEXTURES];    /**<  Texture lod ways.  */
    u32bit textureMinLevel[MAX_TEXTURES];   /**<  Texture minimum mipmap level.  */
    u32bit textureMaxLevel[MAX_TEXTURES];   /**<  Texture maximum mipmap level.  */
    f32bit textureUnitLODWays[MAX_TEXTURES];    /**<  Texture unit lod ways (not texture lod!!).  */
    u32bit maxAnisotropy[MAX_TEXTURES];     /**<  Maximum anisotropy allowed for the texture unit.  */

    //  Stream input registers.
    u32bit attributeMap[MAX_VERTEX_ATTRIBUTES];     /**<  Mapping from vertex input attributes and vertex streams.  */
    QuadFloat attrDefValue[MAX_VERTEX_ATTRIBUTES];  /**<  Defines the vertex attribute default values.  */
    u32bit streamAddress[MAX_STREAM_BUFFERS];       /**<  Address in GPU memory for the vertex stream buffers.  */
    u32bit streamStride[MAX_STREAM_BUFFERS];        /**<  Stride for the stream buffers.  */
    StreamData streamData[MAX_STREAM_BUFFERS];      /**<  Data type for the stream buffer.  */
    u32bit streamElements[MAX_STREAM_BUFFERS];      /**<  Number of stream data elements (vectors) per stream buffer entry.  */
    bool d3d9ColorStream[MAX_STREAM_BUFFERS];       /**<  Read components of the color attributes in the order defined by D3D9.  */

    
    /*  Texture unit queues.  */

    TextureRequest **textRequestsQ;         /**<  Queue for the texture requests.  */
    u32bit nextRequest;                     /**<  Next texture request to be processed.  */
    u32bit nextFreeRequest;                 /**<  Next empty entry in the texture request queue.  */
    u32bit requests;                        /**<  Number of texture requests in the request queue.  */
    u32bit freeRequests;                    /**<  Number of free entries in the the request queue.  */

    TextureAccess **texAccessQ;             /**<  Queue for the texture accesses being processed.  */
    TextureRequest **texAccessReqQ;         /**<  Stores the requests that generated the texture accesses to keep the original cookies.  */
    u32bit numCalcTexAcc;                   /**<  Number of texture accesses for which to calculate the texel addresses.  */
    u32bit numFetchTexAcc;                  /**<  Number of texture accesses waiting to be fetched.  */
    u32bit numFreeTexAcc;                   /**<  Number of free entries in the texture access queue.  */
    u32bit *freeTexAccessQ;                 /**<  Stores the free entries in the texture access queue.  */
    u32bit nextFreeTexAcc;                  /**<  Pointer to the next entry in the free texture access queue.  */
    u32bit lastFreeTexAcc;                  /**<  Pointer to the last entry in the free texture access queue.  */
    u32bit *calcTexAccessQ;                 /**<  Stores the pointers to the texture access queue entries waiting for address calculation.  */
    u32bit nextCalcTexAcc;                  /**<  Next entry in the address calculation queue.  */
    u32bit lastCalcTexAcc;                  /**<  Last entry in the address calculation queue.  */
    u32bit *fetchTexAccessQ;                /**<  Stores the pointers to the texture access queue entries waiting to be fetched.  */
    u32bit nextFetchTexAcc;                 /**<  Next entry in the fetch queue.  */
    u32bit lastFetchTexAcc;                 /**<  Last entry in the fetch queue.  */

    TextureResult **textResultQ;            /**<  Queue for the texture results being waiting to be sent back to the shader.  */
    u32bit nextResult;                      /**<  Next texture result to be sent back to the Shader.  */
    u32bit nextFreeResult;                  /**<  Next free entry in the texture result queue.  */
    u32bit numResults;                      /**<  Number of texture results waiting to be sent back to the Shader.  */
    u32bit freeResults;                     /**<  Number of free entries in the texture result queue.  */

    /**
     *
     *  Defines a trilinear processing element from a texture access
     *  awaiting processing (fetch, read or filter).
     *
     */

    struct TrilinearElement
    {
        u32bit access;          /**<  Pointer to the texture access for the trilinear processing element.  */
        u32bit trilinear;       /**<  Pointer to the trilinear processing element inside the texture access.  */
    };

    TrilinearElement *readyReadQ;   /**<  Stores the trilinear processing elements that can be read from the texture cache.  */
    u32bit nextFreeReadyRead;       /**<  Next free entry in the trilinear ready read queue.  */
    u32bit nextReadyRead;           /**<  Next trilinear element to read in the trilinear ready read queue.  */
    u32bit numFreeReadyReads;       /**<  Number of free entries in the trilinear ready read queue.  */
    u32bit numReadyReads;           /**<  Number of triliner elements in the trilinear ready read queue.  */

    TrilinearElement *waitReadW;    /**<  Stores trilinear elements that are waiting for a line in the texture cache.  */
    u32bit *freeWaitReadQ;          /**<  Stores the free entries in the trilinear wait read window.  */
    u32bit *waitReadQ;              /**<  Stores the waiting entries in the trilinear wait read window.  */
    u32bit nextFreeWaitRead;        /**<  Pointer to the next entry in the free wait read entry queue.  */
    u32bit lastFreeWaitRead;        /**<  Pointer to the last entry in the free wait read entry queue.  */
    u32bit numFreeWaitReads;        /**<  Number of free trilinear wait read entries.  */
    u32bit numWaitReads;            /**<  Number of trilinear elements waiting in the trilinear wait read queue.  */
    u32bit pendingMoveToReady;      /**<  Number of trilinear elements pending to be moved to the ready read queue.  */

    TrilinearElement *filterQ;      /**<  Stores the trilinear processing elements waiting to be filtered.  */
    u32bit nextFreeFilter;          /**<  Pointer to the next free entry in the trilinear filter queue.  */
    u32bit nextToFilter;            /**<  Pointer to the next trilinear element to be filtered in the trilinear filter queue.  */
    u32bit nextFiltered;            /**<  Pointer to the next trilinear element already filtered in the filter queue.  */
    u32bit numFreeFilters;          /**<  Number of free entries in the trilinear filter queue.  */
    u32bit numToFilter;             /**<  Number of trilinear elements to be filtered in the trilinear filter queue.  */
    u32bit numFiltered;             /**<  Number of trilinear elements filtered in the trilinear filter queue.  */

    /*  Texture Cache.  */
    TextureCacheGen *textCache;     /**<  Pointer to the generic texture cache interface that will be used by Texture Unit implementation.  */
    TextureCache *textCacheL1;      /**<  Pointer to the single level texture cache.  */
    TextureCacheL2 *textCacheL2;    /**<  Pointer to the two level texture cache object.  */

    /*  Address ALU state.  */
    u32bit addressALUCycles;    /**<  Cycles remaining until the address ALU can receive a new input.  */

    /*  Filter unit state.  */
    u32bit filterCycles;        /**<  Cycles remaining until the filter unit can receive a new input.  */

    /*  Texture Unit state.  */
    TextureUnitState txState;   /**<  State of the Texture Unit.  */
    ShaderState state;          /**<  Current state of the Shader Unit.  */
    MemState memoryState;       /**<  State of the Memory Controller.  */
    u32bit textureTickets;      /**<  Number of texture tickets offered to the Shader.  */

    /*  Texture Unit parameters.  */
    TextureEmulator &txEmul;    /**<  Reference to the Texture Emulator object attached to the Texture Unit.  */
    u32bit stampFragments;      /**<  Number of fragments in a stamp.  */
    u32bit requestsCycle;       /**<  Texture requests that can be received per cycle.  */
    u32bit requestQueueSize;    /**<  Size of the texture request queue.  */
    u32bit accessQueueSize;     /**<  Size of the texture access queue.  */
    u32bit resultQueueSize;     /**<  Size of the texture result queue.  */
    u32bit waitReadWindowSize;  /**<  Size of the read wait window.  */
    bool useTwoLevelCache;      /**<  Enables the two level texture cache or the single level texture cache.  */
    u32bit textCacheLineSizeL0; /**<  Size in bytes of the texture cache lines for L0 cache or single level.  */
    u32bit textCacheWaysL0;     /**<  Number of ways in the texture cache for L0 cache or single level.  */
    u32bit textCacheLinesL0;    /**<  Number of lines per way in the texture cache for L0 cache or single level.  */
    u32bit textCachePortWidth;  /**<  Width in bytes of the texture cace ports for L0 cache or single level.  */
    u32bit textCacheReqQSize;   /**<  Size of the texture cache request queue for L0 cache or single level.  */
    u32bit textCacheInReqSizeL0;/**<  Size of the texture cache input request queue for L0 cache or single level.  */
    u32bit textCacheMaxMiss;    /**<  Misses supported per cycle in the texture cache for L0 cache or single level.  */
    u32bit textCacheDecomprLatency; /**<  Decompression latency for compressed texture cache lines for L0 cache or single level.  */
    u32bit textCacheLineSizeL1; /**<  Size in bytes of the texture cache lines for L1 cache (two level version).  */
    u32bit textCacheWaysL1;     /**<  Number of ways in the texture cache for L1 cache (two level version).  */
    u32bit textCacheLinesL1;    /**<  Number of lines per way in the texture cache for L1 cache (two level version).  */
    u32bit textCacheInReqSizeL1;/**<  Size of the texture cache input request queue for L1 cache (two level version).  */

    u32bit addressALULatency;   /**<  Latency of calculating the address of a bilinear sample.  */
    u32bit filterLatency;       /**<  Latency of filtering a bilinear sample.  */
    
    /*  Statistics.  */
    GPUStatistics::Statistic *txRequests;   /**<  Number of texture requests from Shader Decode Execute.  */
    GPUStatistics::Statistic *results;      /**<  Number of texture results from Shader Decode Execute.  */
    GPUStatistics::Statistic *fetchOK;      /**<  Succesful fetch operations.  */
    GPUStatistics::Statistic *fetchSkip;    /**<  Skip already fetched texel.  */
    GPUStatistics::Statistic *fetchFail;    /**<  Unsuccesful fetch operations.  */
    GPUStatistics::Statistic *fetchStallFetch;      /**<  Number of cycles the fetch stage stalls because there are no fetches waiting in the queue.  */
    GPUStatistics::Statistic *fetchStallAddress;    /**<  Number of cycles the fetch stage stalls because the address for the next fetch has not been calculated.  */
    GPUStatistics::Statistic *fetchStallWaitRead;   /**<  Number of cycles the fetch stage stalls because there are no free entries in the wait read window.  */
    GPUStatistics::Statistic *fetchStallReadyRead;  /**<  Number of cycles the fetch stage stalls because the ready read queue has no free entries.  */
    GPUStatistics::Statistic *readOK;       /**<  Succesful read operations.  */
    GPUStatistics::Statistic *readFail;     /**<  Failed read operations.  */
    GPUStatistics::Statistic *readBytes;    /**<  Bytes read from memory.  */
    GPUStatistics::Statistic *readTrans;    /**<  Read transactions to memory.  */
    GPUStatistics::Statistic *readyReadLevel;   /**<  Occupation of the ready read queue.  */
    GPUStatistics::Statistic *waitReadLevel;    /**<  Occupation of the wait read window.  */
    GPUStatistics::Statistic *resultLevel;      /**<  Occupation of the texture result queue.  */
    GPUStatistics::Statistic *accessLevel;      /**<  Occupation of the texture access queue.  */
    GPUStatistics::Statistic *requestLevel;     /**<  Occupation of the texture request queue.  */
    GPUStatistics::Statistic *addrALUBusy;      /**<  Address ALU busy cycles.  */
    GPUStatistics::Statistic *filterALUBusy;    /**<  Filter ALU busy cycles.  */
    GPUStatistics::Statistic *textResLat;       /**<  Texture result latency in cycles.  */
    GPUStatistics::Statistic *bilinearSamples;  /**<  Number of bilinear samples per texture access.  */
    GPUStatistics::Statistic *anisoRatio;       /**<  Anisotropy ratio per texture access.  */
    GPUStatistics::Statistic *addrCalcFinished; /**<  Number of texture access that have finished address calculation.  */
    GPUStatistics::Statistic *anisoRatioHisto[MAX_ANISOTROPY + 1];  /**<  Histogram of computed anisotropy ratio.  */
    GPUStatistics::Statistic *bilinearsHisto[MAX_ANISOTROPY * 2];   /**<  Histogram of bilinear sampler required per request.  */
    GPUStatistics::Statistic *magnifiedPixels;  /**<  Number of magnified pixels.  */
    GPUStatistics::Statistic *mipsSampled[2];   /**<  Number of mipmaps sampled (per request).  */
    GPUStatistics::Statistic *pointSampled;     /**<  Number of point sampled pixels.  */

    /*  Private functions.  */


    /**
     *
     *  Processes a Shader Command.
     *
     *  @param cycle Current simulation cycle.
     *  @param shComm The ShaderCommand to process.
     *
     */

    void processShaderCommand(u64bit cycle, ShaderCommand *shComm);

    /**
     *
     *  Processes a texture unit register write.
     *
     *  @param cycle Current simulation cycle.
     *  @param reg The register to write into.
     *  @param subReg The register subregister to write into.
     *  @param data The data to write in the register.
     *
     */

    void processRegisterWrite(u64bit cycle, GPURegister reg, u32bit subReg, GPURegData data);

    /**
     *
     *  Processes a new Texture Request.
     *
     *  @param request The received texture request.
     *
     */

    void processTextureRequest(TextureRequest *request);

    /**
     *
     *  Processes a memory transaction received by the Texture Unit from the Memory Controller.
     *
     *  @param cycle The current simulation cycle.
     *  @param memTrans Pointer to the received memory transaction.
     *
     */

    void processMemoryTransaction(u64bit cycle, MemoryTransaction *memTrans);

    /**
     *
     *  Send texture samples already filtered from the texture access queue
     *  back to the Shader Unit.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void sendTextureSample(u64bit cycle);

    /**
     *
     *  Filters the texels from a texture access that has all the texels read from
     *  the texture cache.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void filter(u64bit cycle);

    /**
     *
     *  Reads the texel from texture accesses in the texture access queue which have
     *  completed the fetching of all its texel.
     *
     */

    void read(u64bit cycle);

    /**
     *
     *  Fetches texels for a texture access in the texture access queue.
     *
     */

    void fetch(u64bit cycle);

    /**
     *
     *  Calculates the texture addresses for texture sample (up to trilinear supported):
     *
     */

    void calculateAddress(u64bit cycle);

    /**
     *
     *  Generates a new texture access from one of the texture requests in the texture request queue.
     *
     */

    void genTextureAccess();

public:

    /**
     *
     *  Constructor for Texture Unit.
     *
     *  Creates and initializes a new Texture Unit box.
     *
     *  @param txEmu Reference to a TextureEmulator object that will be
     *  used to emulate the Texture Unit functionality.
     *  @param frStamp Fragments per stamp.
     *  @param reqCycle Number of texture requests that can be received per cycle.
     *  @param reqQueueSz Texture request queue size.
     *  @param accQueueSz Texture access queue size.
     *  @param resQueueSz Texture result queue size.
     *  @param readWaitWSz Read wait window size.
     *  @param twoLevel Enables the two level texture cache version or the single level version.
     *  @param cacheLineSzL0 Size of the texture cache lines for L0 cache or single level cache.
     *  @param cacheWaysL0 Number of texture cache ways for L0 cache or single level cache.
     *  @param cacheLinesL0 Number of texture cache lines per way for L0 cache or single level cache.
     *  @param portWidth Width in bytes of the texture cache buses for L0 cache or single level cache.
     *  @param reqQSz Size of the texture cache request queue for L0 cache or single level cache.
     *  @param inputReqQSzL0 Size of the texture input request queue for L0 cache or single level cache.
     *  @param missesCycle Number of misses per cycle supported by the Texture Cache for L0 cache or single level cache.
     *  @param decomprLat Decompression latency for compressed texture cache lines for L0 cache or single level cache.
     *  @param cacheLineSzL1 Size of the texture cache lines for L1 cache (two level version).
     *  @param cacheWaysL1 Number of texture cache ways for L1 cache (two level version).
     *  @param cacheLinesL1 Number of texture cache lines per way for L1 cache (two level version).
     *  @param inputReqQSzL1 Size of the texture input request queue for L1 cache (two level version).
     *  @param addrALULat Latency for calculating the addresses for a bilinear sample.
     *  @param filterLat Latency for filtering a bilinear sample.
     *  @param name Name of the Texture Unit box.
     *  @param prefix String used to prefix the box signals names.
     *  @param parent Pointer to the parent box for this box.
     *
     *  @return A new Texture Unit object.
     *
     */

    TextureUnit(TextureEmulator &txEmu, u32bit frStamp, u32bit reqCycle, u32bit reqQueueSz, u32bit accQueueSz,
        u32bit resQueueSz, u32bit readWaitWSz, bool twoLevel, u32bit cacheLineSzL0, u32bit cacheWaysL0, u32bit cacheLinesL0,
        u32bit portWidth, u32bit reqQSz, u32bit inputReqQSzL0, u32bit missesCycle, u32bit decomprLat,
        u32bit cacheLineSzL1, u32bit cacheWaysL1, u32bit cacheLinesL1, u32bit inputReqQSzL1,
        u32bit addrALULat, u32bit filterLat, char *name, char *prefix = 0, Box* parent = 0);

    /**
     *
     *  Texture Unit destructor.
     *
     */
     
    ~TextureUnit();
     
    /**
     *
     *  Clock rutine.
     *
     *  Carries the simulation of the Texture Unit.  It is called
     *  each clock to perform the simulation.
     *
     *  @param cycle  Cycle to simulate.
     *
     */

    void clock(u64bit cycle);

    /**
     *
     *  Sets the debug flag for the box.
     *
     *  @param debug New value of the debug flag.
     *
     */

    void setDebugMode(bool debug);

    /**
     *
     *  Returns a single line string with state and debug information about the
     *  Texture Unit box.
     *
     *  @param stateString Reference to a string where to store the state information.
     *
     */

    void getState(string &stateString);

    /**
     *
     *  Writes into a string a report about the stall condition of the box.
     *
     *  @param cycle Current simulation cycle.
     *  @param stallReport Reference to a string where to store the stall state report for the box.
     *
     */
     
    void stallReport(u64bit cycle, string &stallReport);
    
    /**
     *
     *  Prints information about the TextureAccess object.
     *
     *  @param texAccess Pointer to the TextureAccess object for which to print information.
     *
     */
     
    void printTextureAccessInfo(TextureAccess *texAccess);
};

} // namespace gpu3d

#endif
