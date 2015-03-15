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
 * $RCSfile: ConfigLoader.h,v $
 * $Revision: 1.38 $
 * $Author: cgonzale $
 * $Date: 2008-06-02 17:37:34 $
 *
 * ConfigLoader Definition file.
 *
 */

/**
 *
 *  @file ConfigLoader.h
 *
 *  This file defines the ConfigLoader class that loads and parses
 *  the simulator configuration file.
 *
 *
 */

#ifndef _CONFIGLOADER_

#define _CONFIGLOADER_

/*
    Needed for using getline (GNU extension).
    Compatibility problems??

    Must be the first include because Parser is also including it.
*/

#include <cstdio>
#include <map>
#include <string>

#include "GPUTypes.h"
#include "Parser.h"

namespace gpu3d
{

/***  The initial size of the buffer for the Signal section.  */
static const u32bit SIGNAL_BUFFER_SIZE = 200;

/**
 *
 *  Defines the configuration file sections.
 *
 */

enum Section
{
    SEC_SIM,    /*  Simulator section.  Global simulator parameters.  */
    SEC_GPU,    /*  GPU section.  Global architecture parameters.  */
    SEC_COM,    /*  Command Processor section.  Command Processor parameters.  */
    SEC_MEM,    /*  Memory Controller section.  Memory Controller parameters.  */
    SEC_STR,    /*  Streamer section.  Streamer parameters.  */
    SEC_VSH,    /*  Vertex Shader section.  Vertex Shader parameters.  */
    SEC_PAS,    /*  Primitive Assembly section.  Primitive Assembly parameters.  */
    SEC_CLP,    /*  Clipper section.  Cliper parameters.  */
    SEC_RAS,    /*  Rasterizer section.  Rasterizer parameters.  */
    SEC_FSH,    /*  Fragment Shader section.  Fragment shader parameters.  */
    SEC_ZST,    /*  Z and Stencil Test section.  Z and Stencil test parameters.  */
    SEC_CWR,    /*  Color Write section.  Color Write parameters.  */
    SEC_DAC,    /*  DAC section.  DAC parameters.  */
    SEC_SIG,    /*  Signal section.  Signal parameters.  */
    SEC_UKN     /*  Unknown section.  */
};

/**
 *
 *  Defines the parameters for a signal.
 *
 */

struct SigParameters
{
    char *name;         /**<  Signal name.  */
    u32bit bandwidth;   /**<  Signal bandwidth.  */
    u32bit latency;     /**<  Signal latency.  */
};

/**
 *
 *  Defines the GPU architecture parameters.
 *
 */

struct GPUParameters
{
    u32bit numVShaders;         /**<  Number of vertex shader units in the GPU.  */
    u32bit numFShaders;         /**<  Number of fragment shader units in the GPU.  */
    u32bit numStampUnits;       /**<  Number of stamp units (Z Stencil + Color Write) in the GPU.  */
    u32bit gpuClock;            /**<  Frequency of the GPU (main) domain clock in MHz.  */
    u32bit shaderClock;         /**<  Frequency of the shader domain clock in MHz.  */
    u32bit memoryClock;         /**<  Frequency of the memory domain clock in MHz.  */ 
};

/**
 *
 *  Defines the Command Processor parameters.
 *
 */

struct ComParameters
{
    bool pipelinedBatches;      /**<  Enable/disable pipelined batch rendering.  */
    bool dumpShaderPrograms;    /**<  Enable/disable dumping shader programs being loaded to files.  */
};


/**
 *
 *  Defines the Memory Controller parameters.
 *
 */

struct MemParameters
{
    bool memoryControllerV2;/**<  Specifies which MC will be selected (FALSE = Legacy MC). */
    u32bit memSize;         /**<  GPU memory size in bytes.  */
    u32bit clockMultiplier; /**<  Frequency multiplier applied to the GPU clock to be used as memory reference clock.  */
    u32bit memoryFrequency; /**<  GPU memory frequency in cycles from the memory reference clock.  */
    u32bit busWidth;        /**<  GPU memory bus width in bits.  */
    u32bit memBuses;        /**<  Number of buses to the memory modules.  */
    bool sharedBanks;       /**<  Enables shared access to memory banks from all the gpu memory buses.  */
    u32bit bankGranurality; /**<  Access granurality for distributed memory banks in bytes.  */
    u32bit burstLength;     /**<  Burst length of accesses (bus width) to GPU memory).  */
    u32bit readLatency;     /**<  Cycles from read command to data from GPU memory.  */
    u32bit writeLatency;    /**<  Cycles from write command to data to GPU memory.  */
    u32bit writeToReadLat;  /**<  Cycles from last written data to GPU memory to next read command.  */
    u32bit memPageSize;     /**<  Size of a GPU memory page.  */
    u32bit openPages;       /**<  Number of open memory pages per bus.  */
    u32bit pageOpenLat;     /**<  Latency of opening a new memory page.  */
    u32bit maxConsecutiveReads; /**<  Number of consecutive read transactions before the next write transaction.  */
    u32bit maxConsecutiveWrites;/**<  Number of consecutive write transactions before the next read transaction.  */
    u32bit comProcBus;      /**<  Command Processor memory bus width in bytes.  */
    u32bit strFetchBus;     /**<  Streamer Fetch memory bus width in bytes.  */
    u32bit strLoaderBus;    /**<  Streamer Loader memory bus width in byts.  */
    u32bit zStencilBus;     /**<  Z Stencil Test memory bus width in bytes.  */
    u32bit cWriteBus;       /**<  Color Write memory bus width in bytes.  */
    u32bit dacBus;          /**<  DAC memory bus width in bytes.  */
    u32bit textUnitBus;     /**<  Texture Unit memory bus width in bytes.  */
    u32bit mappedMemSize;   /**<  Amount of system memory mapped into the GPU address space in bytes.  */
    u32bit readBufferLines; /**<  Number of buffer lines for read transactions.  */
    u32bit writeBufferLines;/**<  Number of buffer lines for write transactions.  */
    u32bit reqQueueSz;      /**<  Request queue size.  */
    u32bit servQueueSz;     /**<  Service queue size.  */

    bool v2UseChannelRequestFIFOPerBank; ///< Use N queues per channel (being N = # of banks) to issue channel transactions to channel schedulers
    u32bit v2ChannelRequestFIFOPerBankSelection; ///< Algorithm used to select a bank to issue the next channel transation to channel scheduler

    /// Parameters exclusive for Memory Controller V2
    bool v2MemoryTrace; /**< Tells the Memory Controller V2 to generate a file with the memory transactions received */
    u32bit v2MemoryChannels; /**< Number of channels available (equivalent to old MemoryBuses) */
    u32bit v2BanksPerMemoryChannel; /**< Number of banks per channel (ie: per chip module) */
    u32bit v2MemoryRowSize; /**< Size in bytes of a page(row), equivalent to old MemoryPageSize */
    
    /**
     * 32bit data elements of a burst served per cycle
     * @note in DDR chips the usual value is 2
     * @note Greater values can be used to emulate several chips attached to one channel
     */
    // u32bit v2BurstElementsPerCycle;

    /**
     * Bytes of a burst read/written per cycle
     * @note in DDR chips the usual value is 8B/cycle
     * @note Greater values can be used to emulate several chips attached to one channel
     * @note Lower values can be used to emulate slower memories 
     */
    u32bit v2BurstBytesPerCycle;

    // Interleaving of memory at channel & bank level
    u32bit v2ChannelInterleaving;
    u32bit v2BankInterleaving;

    bool v2SecondInterleaving;
    u32bit v2SecondChannelInterleaving;
    u32bit v2SecondBankInterleaving;
    u32bit v2SplitterType;
    char* v2ChannelInterleavingMask;
    char* v2BankInterleavingMask;
    char* v2SecondChannelInterleavingMask;
    char* v2SecondBankInterleavingMask;
    char* v2BankSelectionPolicy;
    // Maximum number of in flight channel transactions per channel
    u32bit v2MaxChannelTransactions;
    u32bit v2DedicatedChannelReadTransactions;
    u32bit v2ChannelScheduler;/**< Type of channel scheduler. */
    u32bit v2PagePolicy;        /**< Page policy selected. */

    char* v2MemoryType; // should be defined as "gddr3"

    char* v2GDDR_Profile; // "custom", "600", "700" and so on

    u32bit v2GDDR_tRRD;
    u32bit v2GDDR_tRCD;
    u32bit v2GDDR_tWTR;
    u32bit v2GDDR_tRTW;
    u32bit v2GDDR_tWR;
    u32bit v2GDDR_tRP;
    u32bit v2GDDR_CAS;
    u32bit v2GDDR_WL;

    u32bit v2SwitchModePolicy;
    u32bit v2ActiveManagerMode;
    u32bit v2PrechargeManagerMode;

    bool v2DisablePrechargeManager; 
    bool v2DisableActiveManager;
    u32bit v2ManagerSelectionAlgorithm; // 0=active-then-precharge, 1=precharge-then-active, other-more-intelligent-aproaches

    char* v2DebugString;
    bool v2UseClassicSchedulerStates;

    bool v2UseSplitRequestBufferPerROP;

    // for compatibility with previous configuration files
    // By default disables the new memory controller
    // new option v2MaxChannelTransactions set with a default value to prevent
    // malfunction with previous "bGPU.ini" files
    MemParameters() : memoryControllerV2(false), 
                      v2MemoryTrace(false),
                      v2MaxChannelTransactions(8), 
                      v2SecondInterleaving(false), // disabled by default
                      v2SplitterType(0), // By default use legacy splitter
                      v2ChannelInterleavingMask(0),
                      v2BankInterleavingMask(0),
                      v2SecondChannelInterleavingMask(0),
                      v2SecondBankInterleavingMask(0),
                      v2BankSelectionPolicy(0),
                      v2MemoryType(0),
                      v2GDDR_Profile(0), // Use default
                      v2DebugString(0)
    {}
};


/**
 *
 *  Defines the Streamer parameters.
 *
 *
 */

struct StrParameters
{
    u32bit indicesCycle;        /**<  Indices to be processed by the Streamer per cycle.  */
    u32bit idxBufferSize;       /**<  Index buffer size.  */
    u32bit outputFIFOSize;      /**<  Output FIFO size.  */
    u32bit outputMemorySize;    /**<  Output Memory Size.  */
    u32bit verticesCycle;       /**<  Number of vertices sent to Primitive Assembly per cycle.  */
    u32bit attrSentCycle;       /**<  Number of shader output attributes that can be sent per cycle to Primitive assembly.  */
    u32bit streamerLoaderUnits; /**<  Number of Streamer Loader Units for the load of attribute data. */
   
    /* Individual Streamer Loader Unit parameters.  */
  
    u32bit slIndicesCycle;        /**<  Indices per cycle per Streamer Loader unit.  */
    u32bit slInputReqQueueSize;   /**<  Input request queue size.  */
    u32bit slAttributesCycle;     /**<  Shader input attributes filled with data per cycle.  */
    u32bit slInCacheLines;        /**<  Lines in the input cache.  */
    u32bit slInCacheLineSz;       /**<  Input cache line size in bytes.  */
    u32bit slInCachePortWidth;    /**<  Width in bytes of the input cache read and write ports.  */
    u32bit slInCacheReqQSz;       /**<  Input cache request queue size.  */
    u32bit slInCacheInputQSz;     /**<  Input cache input request queue size.  */
};


/**
 *
 *  Defines the Vertex Shader parameters.
 *
 *
 */

struct VShParameters
{
    u32bit numThreads;      /**<  Number of execution threads per shader.  */
    u32bit numInputBuffers; /**<  Number of input buffers per shader.  */
    u32bit numResources;    /**<  Number of thread resources (registers?) per shader.  */
    u32bit threadRate;      /**<  Vertex shader threads from which to fetch per cycle.  */
    u32bit fetchRate;       /**<  Vertex Shader instructions fetched per cycle and thread.  */
    u32bit threadGroup;     /**<  Number of threads that form a lock step group.  */
    bool lockedMode;        /**<  Threads execute in lock up mode flag.  */
    bool scalarALU;         /**<  Enable a scalar ALU in parallel with the SIMD ALU.  */
    bool threadWindow;      /**<  Enables the fetch unit to search for a ready thread in a window.  */
    u32bit fetchDelay;      /**<  Delay in cycles until the next instruction is fetched from a thread.  */
    bool swapOnBlock;       /**<  Enables swap thread on block or swap thread always.  */
    u32bit inputsCycle;     /**<  Vertex inputs that can be received per cycle.  */
    u32bit outputsCycle;    /**<  Vertex outputs that can be sent per cycle.  */
    u32bit outputLatency;   /**<  Vertex ouput transmission latency.  */
};


/**
 *
 *  Defines the Primitive Assembly parameters.
 *
 */

struct PAsParameters
{
    u32bit verticesCycle;       /**<  Number of vertices received per cycle from the Streamer.  */
    u32bit trianglesCycle;      /**<  Triangles per cycle sent to the Clipper.  */
    u32bit inputBusLat;         /**<  Latency of the input vertex bus from the Streamer.  */
    u32bit paQueueSize;         /**<  Size in vertices of the assembly queue.  */
};

/**
 *
 *  Defines the Clipper parameters.
 *
 */

struct ClpParameters
{
    u32bit trianglesCycle;      /**<  Triangles received from Primitive Assembly and sent to Triangle Setup per cycle.  */
    u32bit clipperUnits;        /**<  Clipper test units.  */
    u32bit startLatency;        /**<  Triangle clipping start latency.  */
    u32bit execLatency;         /**<  Triangle clipping execution latency.  */
    u32bit clipBufferSize;      /**<  Clipped triangle buffer size.  */
};

/**
 *
 *  Defines the Rasterizer parameters.
 *
 */

struct RasParameters
{
    u32bit trianglesCycle;          /**<  Number of triangles per cycle.  */
    u32bit setupFIFOSize;           /**<  Triangle Setup FIFO size.  */
    u32bit setupUnits;              /**<  Number of triangle setup units.  */
    u32bit setupLat;                /**<  Latency of the triangle setup units.  */
    u32bit setupStartLat;           /**<  Start latency of the triangle setup units.  */
    u32bit trInputLat;              /**<  Triangle input bus latency (from Primitive Assembly/Clipper).  */
    u32bit trOutputLat;             /**<  Triangle output bus latency (between Triangle Setup and TriangleTraversal/Fragment Generation).  */
    bool shadedSetup;               /**<  Triangle setup performed in the unified shader.  */
    u32bit triangleShQSz;           /**<  Size of the triangle shader queue in Triangle Setup.  */
    u32bit stampsCycle;             /**<  Stamps per cycle.  */
    u32bit samplesCycle;            /**<  Number of MSAA samples that can be generated per fragment and cycle in Triangle Traversal/Fragment Generation.  */
    u32bit overScanWidth;           /**<  Scan over tile width in scan tiles.  */
    u32bit overScanHeight;          /**<  Scan over tile height in scan tiles.  */
    u32bit scanWidth;               /**<  Scan tile width in fragments.  */
    u32bit scanHeight;              /**<  Scan tile height in fragments.  */
    u32bit genWidth;                /**<  Generation tile width in fragments.  */
    u32bit genHeight;               /**<  Generation tile heigh in fragments.  */
    u32bit rastBatch;               /**<  Number of triangles per rasterization batch (Triangle Traversal).  */
    u32bit batchQueueSize;          /**<  Number of triangles in the batch queue (Triangle Traversal).  */
    bool recursive;                 /**<  Determines the rasterization method (TRUE: recursive, FALSE: scanline) (Triangle Traversal).  */
    bool disableHZ;                 /**<  Disables the Hierarchical Z.  */
    u32bit stampsHZBlock;           /**<  Stamps per Hierarchical Z block.  */
    u32bit hzBufferSize;            /**<  Size of the Hierarchical Z buffer in blocks.  */
    u32bit hzCacheLines;            /**<  Lines in the HZ cache.  */
    u32bit hzCacheLineSize;         /**<  Block per HZ cache line.  */
    u32bit earlyZQueueSz;           /**<  Size of the Hierarchical/Early Z test queue.  */
    u32bit hzAccessLatency;         /**<  Access latency to the Hierarchical Z Buffer.  */
    u32bit hzUpdateLatency;         /**<  Latency of the update signal to the Hierarchical Z.  */
    u32bit hzBlockClearCycle;       /**<  Number of Hierarchical Z blocks cleared per cycle.  */
    u32bit numInterpolators;        /**<  Number of attribute interpolators.  */
    u32bit shInputQSize;            /**<  Shader input queue size (per shader unit).  */
    u32bit shOutputQSize;           /**<  Shader output queue size (per shader unit).  */
    u32bit shInputBatchSize;        /**<  Shader inputs per shader unit assigned work batch.  */
    bool tiledShDistro;             /**<  Enable/disable distribution of fragment inputs to the shader units on a tile basis.  */
    u32bit vInputQSize;             /**<  Vertex input queue size (Fragment FIFO) (only for unified shader version).  */
    u32bit vShadedQSize;            /**<  Shaded vertex queue size (Fragment FIFO) (only for unified shader version).  */
    u32bit trInputQSize;            /**<  Triangle input queue size (Fragment FIFO) (only for triangle setup on shaders).  */
    u32bit trOutputQSize;           /**<  Triangle output queue size (Fragment FIFO) (only for triangle setup on shaders).  */
    u32bit genStampQSize;           /**<  Generated stamp queue size (Fragment FIFO) (per stamp unit).  */
    u32bit testStampQSize;          /**<  Early Z tested stamp queue size (Fragment FIFO).  */
    u32bit intStampQSize;           /**<  Interpolated stamp queue size (Fragment FIFO).  */
    u32bit shadedStampQSize;        /**<  Shaded stamp queue size (Fragment FIFO) (per stamp unit).  */
    u32bit emuStoredTriangles;      /**<  Number of triangles that can be kept stored in the rasterizer emulator.  */

    //   MicroPolygon Rasterizer parameters.
    bool useMicroPolRast;           /**<  Use the MicroPolygon Rasterizer.  */
    bool preBoundTriangles;         /**<  Triangles are bound (compute its area and BB) prior to edge equation setup, enabling the Triangle Bound unit (between Clipper and Triangle Setup).  */
    u32bit trBndOutLat;             /**<  Triangle bus latency between Triangle Bound and Triangle Setup.  */
    u32bit trBndOpLat;              /**<  Latency of a Triangle Bound operation (mainly compute the triangle bounding box).  */
    u32bit trBndLargeTriFIFOSz;     /**<  Size of the TriangleBound unit queue storing large triangles to be sent to Triangle Setup (shared path).  */  
    u32bit trBndMicroTriFIFOSz;     /**<  Size of the TriangleBound unit queue storing microtriangles to be sent to Shader Work Distributor (independent path).  */ 
    bool useBBOptimization;         /**<  Use the bounding box size optimization pass.  */
    u32bit subPixelPrecision;       /**<  Number of bits used in the decimal part of fixed point operations to compute the subpixel positions.  */
    f32bit largeTriMinSz;           /**<  The minimum area to consider a large triangle.  */
    bool cullMicroTriangles;        /**<  Removes all detected microtriangles (according to the microtriangle size limit specified). Used for debug purposes.  */
    bool microTrisAsFragments;      /**<  Process micro triangles directly as fragments, skipping setup and traversal operations on the triangle.  */
    u32bit microTriSzLimit;         /**<  0: 1-pixel bound triangles only, 1: 1-pixel and 1-stamp bound triangles, 2: Any triangle bound to 2x2, 1x4 or 4x1 stamps.  */
    char* microTriFlowPath;         /**<  "shared": Use the same data path as normal (macro) triangles, "independent": use separate path for micro - allow z stencil and color write disorder.  */
    bool dumpBurstHist;             /**<  Dump of micro and macrotriangle burst size histogram is enabled/disabled.  */
};

/**
 *
 *  Defines the Fragment Shader parameters.
 *
 */

struct FShParameters
{
    //  Legacy shader parameters
    u32bit numThreads;      /**<  Number of execution threads per shader.  */
    u32bit numInputBuffers; /**<  Number of input buffers per shader.  */
    u32bit numResources;    /**<  Number of thread resources (registers?) per shader.  */
    u32bit threadRate;      /**<  Fragment shader threads from which to fetch per cycle.  */
    bool scalarALU;         /**<  Enable a scalar ALU in parallel with the SIMD ALU.  */
    u32bit fetchRate;       /**<  Fragment Shader instructions fetched per cycle and thread.  */
    u32bit threadGroup;     /**<  Number of threads that form a lock step group.  */
    bool lockedMode;        /**<  Threads execute in lock up mode flag.  */

    //  Vector shader parameters
    bool useVectorShader;       /**<  Flags to use the Vector Shader model for the unified shader.  */
    u32bit vectorThreads;       /**<  Number of vector threads supported by the vector shader.  */
    u32bit vectorResources;     /**<  Number of resources (per vector thread) available in the vector shader.  */
    u32bit vectorLength;        /**<  Number of elements in a vector.  */
    u32bit vectorALUWidth;      /**<  Number of ALUs in the vector ALU array.  */
    char *vectorALUConfig;      /**<  Configuration of the element ALUs in the vector array.  */
    bool vectorWaitOnStall;     /**<  Flag that defines if the instruction waits for stall conditions to be cleared or is discarded and repeated (refetched).  */
    bool vectorExplicitBlock;   /**<  Flag that defines if threads are blocked by an explicit trigger in the instruction stream or automatically on texture load.  */

    //  Common shader paramters
    bool vAttrLoadFromShader;   /**<  Vertex attribute load is performed in the shader (LDA instruction).  */
    bool threadWindow;      /**<  Enables the fetch unit to search for a ready thread in a window.  */
    u32bit fetchDelay;      /**<  Delay in cycles until the next instruction is fetched from a thread.  */
    bool swapOnBlock;       /**<  Enables swap thread on block or swap thread always.  */
    bool fixedLatencyALU;   /**<  Sets if the all the latency of the ALU is always the same for all the instructions.  */
    u32bit inputsCycle;     /**<  Fragment inputs that can be received per cycle.  */
    u32bit outputsCycle;    /**<  Fragment outputs that can be sent per cycle.  */
    u32bit outputLatency;   /**<  Fragment ouput transmission latency.  */
    u32bit textureUnits;    /**<  Number of texture units attached to the shader.  */
    u32bit textRequestRate; /**<  Texture request rate to the texture units.  */
    u32bit textRequestGroup;/**<  Size of a group of texture requests sent to a texture unit.  */
    
    /*  Texture unit parameters.  */
    u32bit addressALULat;   /**<  Latency of the address ALU.  */
    u32bit filterLat;       /**<  Latency of the filter unit.  */
    u32bit anisoAlgo;       /**<  Anisotropy detection algorithm to be used.  */
    bool forceMaxAniso;     /**<  Flag used to force the maximum anisotropy from the configuration file to all textures.  */
    u32bit maxAnisotropy;   /**<  Maximum anisotropy ratio allowed.  */
    u32bit triPrecision;    /**<  Trilinear fractional precision.  */
    u32bit briThreshold;    /**<  Brilinear threshold.  */
    u32bit anisoRoundPrec;  /**<  Aniso round precision.  */
    u32bit anisoRoundThres; /**<  Aniso round threshold.  */
    bool anisoRatioMultOf2; /**<  Aniso ratio must be a multiple of two.  */
    u32bit textBlockDim;    /**<  Texture block/line dimension in texels (2^n x 2^n).  */
    u32bit textSBlockDim;   /**<  Texture super block dimension in blocks (2^m x 2^m).  */
    u32bit textReqQSize;    /**<  Texture request queue size.  */
    u32bit textAccessQSize; /**<  Texture access queue size.  */
    u32bit textResultQSize; /**<  Texture result queue size.  */
    u32bit textWaitWSize;   /**<  Texture wait read window size.  */
    bool twoLevelCache;     /**<  Enables the two level texture cache or the single level.  */
    u32bit txCacheLineSz;   /**<  Size of a texture cache line in bytes (L0 for two level version).  */
    u32bit txCacheWays;     /**<  Ways in the texture cache  (L0 for two level version).  */
    u32bit txCacheLines;    /**<  Lines per way in the texture cache  (L0 for two level version).  */
    u32bit txCachePortWidth;/**<  Width in bytes of the texture cache read/write ports.  */
    u32bit txCacheReqQSz;   /**<  Texture Cache request queue size.  */
    u32bit txCacheInQSz;    /**<  Texture Cache input queue size  (L0 for two level version).  */
    u32bit txCacheMissCycle;/**<  Texture Cache misses supported per cycle.  */
    u32bit txCacheDecomprLatency;   /**<  Texture Cache compressed texture line decompression latency.  */
    u32bit txCacheLineSzL1; /**<  Size of a texture cache line in bytes (L1 for two level version).  */
    u32bit txCacheWaysL1;   /**<  Ways in the texture cache (L1 for two level version).  */
    u32bit txCacheLinesL1;  /**<  Lines per way in the texture cache (L1 for two level version).  */
    u32bit txCacheInQSzL1;  /**<  Texture Cache input queue size (L1 for two level version).  */

};


/**
 *
 *  Defines the Z Stencil Test parameters.
 *
 */
struct ZSTParameters
{
    u32bit stampsCycle;     /**<  Stamps per cycle.  */
    u32bit bytesPixel;      /**<  Bytes per pixel (depth).  */
    bool disableCompr;      /**<  Disables Z compression.  */
    u32bit zCacheWays;      /**<  Ways in the Z Cache.  */
    u32bit zCacheLines;     /**<  Lines per way in the Z cache.  */
    u32bit zCacheLineStamps;/**<  Stamps per Z Cache line.  */
    u32bit zCachePortWidth; /**<  Width in bytes of the Z Cache ports.  */
    bool extraReadPort;     /**<  Adds an additional read port to the Color Cache.  */
    bool extraWritePort;    /**<  Adds an additional write port to the Color Cache.  */
    u32bit zCacheReqQSz;    /**<  Z Cache request queue size.  */
    u32bit zCacheInQSz;     /**<  Z Cache input queue size.  */
    u32bit zCacheOutQSz;    /**<  Z Cache output queue size.  */
    u32bit blockStateMemSz; /**<  Size of the block state memory (in blocks).  */
    u32bit blockClearCycle; /**<  Blocks cleared per cycle.  */
    u32bit comprLatency;    /**<  Block compression latency.  */
    u32bit decomprLatency;  /**<  Block decompression latency.  */
    u32bit comprAlgo;       /**<  Compression algorithm id. */
    //u32bit zQueueSz;        /**<  Size of the Z/Stencil Test stamp queue.  */
    u32bit inputQueueSize;  /**<  Size of the input Z/Stencil Test stamp queue.  */
    u32bit fetchQueueSize;  /**<  Size of the fetched Z/Stencil Test stamp queue.  */
    u32bit readQueueSize;   /**<  Size of the read Z/Stencil Test stamp queue.  */
    u32bit opQueueSize;     /**<  Size of the operated Z/Stencil Test stamp queue.  */
    u32bit writeQueueSize;  /**<  Size of the written Z/Stencil Test stamp queue.  */
    u32bit zTestRate;       /**<  Rate of stamp testing (cycles between two stamps).  */
    u32bit zOpLatency;      /**<  Latency of Z/Stencil Test.  */
};

/**
 *
 *  Defines the Color Write parameters.
 *
 */
struct CWRParameters
{
    u32bit stampsCycle;     /**<  Stamps per cycle.  */
    u32bit bytesPixel;      /**<  Bytes per pixel (color).  */
    bool disableCompr;      /**<  Disables color compression.  */
    u32bit cCacheWays;      /**<  Ways in the Color Cache.  */
    u32bit cCacheLines;     /**<  Lines per way in the Color cache.  */
    u32bit cCacheLineStamps;/**<  Stamps per Color Cache line.  */
    u32bit cCachePortWidth; /**<  Width in bytes of the Color Cache ports.  */
    bool extraReadPort;     /**<  Adds an additional read port to the Color Cache.  */
    bool extraWritePort;    /**<  Adds an additional write port to the Color Cache.  */
    u32bit cCacheReqQSz;    /**<  Color Cache request queue size.  */
    u32bit cCacheInQSz;     /**<  Color Cache input queue size.  */
    u32bit cCacheOutQSz;    /**<  Color Cache output queue size.  */
    u32bit blockStateMemSz; /**<  Size of the block state memory (in blocks).  */
    u32bit blockClearCycle; /**<  Blocks cleared per cycle.  */
    u32bit comprLatency;    /**<  Block compression latency.  */
    u32bit decomprLatency;  /**<  Block decompression latency.  */
    u32bit comprAlgo;       /**<  Compression algorithm id. */
    //u32bit colorQueueSz;    /**<  Size of the Color/Blend stamp queue.  */
    u32bit inputQueueSize;  /**<  Size of the input Color Write stamp queue.  */
    u32bit fetchQueueSize;  /**<  Size of the fetched Color Write stamp queue.  */
    u32bit readQueueSize;   /**<  Size of the read Color Write stamp queue.  */
    u32bit opQueueSize;     /**<  Size of the operated Color Write stamp queue.  */
    u32bit writeQueueSize;  /**<  Size of the written Color Write stamp queue.  */
    u32bit blendRate;       /**<  Rate for stamp blending (cycles between two stamps).  */
    u32bit blendOpLatency;  /**<  Latency of blend operation.  */
};

/**
 *
 *  Defines the DAC parameters.
 *
 */
struct DACParameters
{
    u32bit bytesPixel;      /**<  Bytes per pixel (color).  */
    u32bit blockSize;       /**<  Size of an uncompressed block in bytes.  */
    u32bit blockUpdateLat;  /**<  Latency of the color block update signal.  */
    u32bit blocksCycle;     /**<  Blocks that can be update/written per cycle in the state memory.  */
    u32bit blockReqQSz;     /**<  Block request queue size.  */
    u32bit decomprLatency;  /**<  Block decompression latency.  */
    u64bit refreshRate;     /**<  Frame refresh/dumping rate in cycles.  */
    bool synchedRefresh;    /**<  Swap is syncrhonized with frame refresh/dumping.  */
    bool refreshFrame;      /**<  Dump/refresh the frame into a file.  */
    bool saveBlitSource;    /**<  Save the source data for blit operations to a PPM file.  */
};


/**
 *
 *  Defines the simulator parameters.
 *
 *
 */

struct SimParameters
{
    /*  Simulator parameters.  */
    u64bit simCycles;       /**<  Cycles to simulate.  */
    u32bit simFrames;       /**<  Number of cycles to simulate.  */
    char *inputFile;        /**<  Simulator trace input file.  */
    char *signalDumpFile;   /**<  Name of the signal trace dump file.  */
    char *statsFile;        /**<  Name of the statistics file.  */
    char *statsFilePerFrame;/**<  Nome of the per frame statistics file.  */
    char *statsFilePerBatch;/**<  Nome of the per batch statistics file.  */
    u32bit startFrame;      /**<  First frame to simulate.  */
    u64bit startDump;       /**<  First cycle to dump signal trace.  */
    u64bit dumpCycles;      /**<  Number of cycles to dump signal trace.  */
    u32bit statsRate;       /**<  Rate (in cycles) at which statistics are updated.  */
    bool dumpSignalTrace;   /**<  Enables signal trace dump.  */
    bool statistics;        /**<  Enables the generation of statistics.  */
    bool perFrameStatistics;/**<  Enable/disable per frame statistics generation.  */
    bool perBatchStatistics;/**<  Enable/disable per batch statistics generation.  */
    bool perCycleStatistics;/**<  Enable/disable per cycle statistics generation.  */
    bool detectStalls;      /**<  Enable/disable stall detection.  */
    bool fragmentMap;       /**<  Generate the fragment propierty map.  */
    u32bit fragmentMapMode; /**<  Fragment map mode: 0 => fragment latency from generation to color write/blend.  */
    bool doubleBuffer;      /**<  Enables double buffer (front/back) for the color frame buffer.  */
    bool forceMSAA;         /**<  Forces multisampling for the whole trace.  */
    u32bit msaaSamples;     /**<  Number of MSAA samples per pixel when multisampling is forced in the configuration file.  */
    bool forceFP16ColorBuffer;   /**<  Force float16 color buffer. */
    bool enableDriverShTrans;   /**<  Enables shader program translation in the driver.  */
    u32bit objectSize0;     /**<  Size in bytes of the objects in the optimized dynamic memory bucket 0.  */
    u32bit objectSize1;     /**<  Size in bytes of the objects in the optimized dynamic memory bucket 1.  */
    u32bit objectSize2;     /**<  Size in bytes of the objects in the optimized dynamic memory bucket 2.  */
    u32bit bucketSize0;     /**<  Number of objects in the optimized dynamic memory bucket 0;  */
    u32bit bucketSize1;     /**<  Number of objects in the optimized dynamic memory bucket 1;  */
    u32bit bucketSize2;     /**<  Number of objects in the optimized dynamic memory bucket 2;  */

    bool useACD; /**< Selects OpenGL implementation (false -> legacy, true -> new on ACD)*/

    /*  Per gpu unit parameters.  */
    GPUParameters gpu;      /**<  GPU architecture parameters.  */
    ComParameters com;      /**<  Command Processor parameters.  */
    MemParameters mem;      /**<  Memory Controller parameters.  */
    StrParameters str;      /**<  Streamer parameters.  */
    VShParameters vsh;      /**<  Vertex Shader parameters.  */
    PAsParameters pas;      /**<  Primitive Assembly parameters.  */
    ClpParameters clp;      /**<  Clipper parameters.  */
    RasParameters ras;      /**<  Rasterizer parameters.  */
    FShParameters fsh;      /**<  Fragment Shader parameters.  */
    ZSTParameters zst;      /**<  Z Stencil test parameters.  */
    CWRParameters cwr;      /**<  Color Write parameters.  */
    DACParameters dac;      /**<  DAC parameters.  */
    SigParameters *sig;     /**<  Signal parameters.  */
    u32bit numSignals;      /**<  Number of signal (parameters).  */
};


/**
 *
 *  Loads and parses the simulator configuration file.
 *
 *  Inherits from the Parser class that offers basic parsing
 *  functions.
 *
 */

class ConfigLoader : public Parser
{

private:

    /**
     * Helper class designed to add support for checking missing parameters and parameters not supported by ConfigLoader
     */
    class ParamsTracker
    {
    private:

        std::map<std::string,bool> paramsSeen;
        std::string secStr;
        bool paramSectionDefinedFlag;

    public:

        void startParamSectionDefinition(const std::string& sec);

        bool wasAnyParamSectionDefined() const;

        void registerParam(const std::string& paramName);

        void setParamDefined(const std::string& paramName);

        // bool isParamDefined(const std::string& secName, const std::string& paramName) const;

        void checkParamsIntegrity(const SimParameters& simP);

    }; // class ParamsTracker

    ParamsTracker paramsTracker;


    FILE *configFile;       /**<  File descriptor for the configuration file.  */

    /*  Private functions.  */

    /**
     *
     *  Parses a section of the configuration file.
     *
     *  @param simP Pointer to the Simulator Parameters structure where to
     *  store the section parameters.
     *
     *  @return TRUE if the section was succefully parsed.
     *
     */

    bool parseSection(SimParameters *simP);

    /**
     *
     *  Parses all the section parameters for a given section.
     *
     *  @param section The section that is going to be parsed.
     *  @param simP Pointer to the simulator parameters structure where
     *  to store the parsed parameters.
     *
     *  @return TRUE if the section parameters were parsed correctly.
     *
     */

    bool parseSectionParameters(Section section, SimParameters *simP);

    /**
     *
     *  Parses a Simulator section parameter.
     *
     *  @param simP Pointer to the Simulator Parameters structure where
     *  to store the parsed parameter.
     *
     *  @return TRUE if the parameter was parsed correctly.
     *
     */

    bool parseSimSectionParameter(SimParameters *simP);

    /**
     *
     *  Parses a GPU Architecture section parameter.
     *
     *  @param gpuP Pointer to the GPU Architecture Parameters structure where
     *  to store the parsed parameter.
     *
     *  @return TRUE if the parameter was parsed correctly.
     *
     */

    bool parseGPUSectionParameter(GPUParameters *gpuP);

    /**
     *
     *  Parses a Command Processor section parameter.
     *
     *  @param comP Pointer to the Command Processor Parameters structure
     *  where to store the parsed parameter.
     *
     *  @return TRUE if the parameter was parsed correctly.
     *
     */

    bool parseComSectionParameter(ComParameters *comP);

    /**
     *
     *  Parses a Memory Controller section parameter.
     *
     *  @param memP Pointer to the Memory Controller Parameters structure
     *  where to store the parsed parameter.
     *
     *  @return TRUE if the parameter was parsed correctly.
     *
     */

    bool parseMemSectionParameter(MemParameters *memP);


    /**
     *
     *  Parses a Streamer section parameter.
     *
     *  @param strP Pointer to the Streamer Parameters structure
     *  where to store the parsed parameter.
     *
     *  @return TRUE if the parameter was parsed correctly.
     *
     */

    bool parseStrSectionParameter(StrParameters *strP);


    /**
     *
     *  Parses a Vertex Shader section parameter.
     *
     *  @param vshP Pointer to the Vertex Shader Parameters structure
     *  where to store the parsed parameter.
     *
     *  @return TRUE if the parameter was parsed correctly.
     *
     */

    bool parseVShSectionParameter(VShParameters *vshP);


    /**
     *
     *  Parses a Primitive Assembly section parameter.
     *
     *  @param pasP Pointer to the Primitive Assembly Parameters structure
     *  where to store the parsed parameter.
     *
     *  @return TRUE if the parameter was parsed correctly.
     *
     */

    bool parsePAsSectionParameter(PAsParameters *pasP);

    /**
     *
     *  Parses a Clipper section parameter.
     *
     *  @param clpP Pointer to the Clipper Parameters structure
     *  where to store the parsed parameter.
     *
     *  @return TRUE if the parameter was parsed correctly.
     *
     */

    bool parseClpSectionParameter(ClpParameters *clpP);

    /**
     *
     *  Parses a Rasterizer section parameter.
     *
     *  @param rasP Pointer to the Rasterizer Parameters structure
     *  where to store the parsed parameter.
     *
     *  @return TRUE if the parameter was parsed correctly.
     *
     */

    bool parseRasSectionParameter(RasParameters *rasP);

    /**
     *
     *  Parses a Fragment Shader section parameter.
     *
     *  @param fshP Pointer to the Rasterizer Parameters structure
     *  where to store the parsed parameter.
     *
     *  @return TRUE if the parameter was parsed correctly.
     *
     */

    bool parseFShSectionParameter(FShParameters *fshP);

    /**
     *
     *  Parses a Z Stencil Test section parameter.
     *
     *  @param zstP Pointer to the Rasterizer Parameters structure
     *  where to store the parsed parameter.
     *
     *  @return TRUE if the parameter was parsed correctly.
     *
     */

    bool parseZSTSectionParameter(ZSTParameters *zstP);

    /**
     *
     *  Parses a Color Write section parameter.
     *
     *  @param cwrP Pointer to the Rasterizer Parameters structure
     *  where to store the parsed parameter.
     *
     *  @return TRUE if the parameter was parsed correctly.
     *
     */

    bool parseCWRSectionParameter(CWRParameters *cwrP);

    /**
     *
     *  Parses a DAC section parameter.
     *
     *  @param dacP Pointer to the Rasterizer Parameters structure
     *  where to store the parsed parameter.
     *
     *  @return TRUE if the parameter was parsed correctly.
     *
     */

    bool parseDACSectionParameter(DACParameters *dacP);

    /**
     *
     *  Parses a Signal section parameter.
     *
     *  @param sigpP Pointer to the Signal Parameters structure
     *  where to store the parsed parameter.
     *
     *  @return TRUE if the parameter was parsed correctly.
     *
     */

    bool parseSigSectionParameter(SigParameters *sigP);

    /**
     *
     *  Parses a decimal parameter.
     *
     *  @param paramName Name of the parameter being parsed.
     *  @param id Name of the parameter to parse.
     *  @param val Reference to a 32bit integer.
     *
     *  @return TRUE if the parameter was parsed.
     *
     */

    bool parseDecimalParameter(char *paramName, char *id, u32bit &val);

    /**
     *
     *  Parses a decimal parameter.
     *
     *  @param paramName Name of the parameter being parsed.
     *  @param id Name of the parameter to parse.
     *  @param val Reference to a 64 bit integer.
     *
     *  @return TRUE if the parameter was parsed.
     *
     */

    bool parseDecimalParameter(char *paramName, char *id, u64bit &val);

    /**
     *
     *  Parses a 32-bit floating point parameter.
     *
     *  @param paramName Name of the parameter being parsed.
     *  @param id Name of the parameter to parse.
     *  @param val Reference to a 32 bit floating point.
     *
     *  @return TRUE if the parameter was parsed.
     *
     */

    bool parseFloatParameter(char *paramName, char *id, f32bit &val);

    /**
     *
     *  Parses a boolean parameter.
     *
     *  @param paramName Name of the parameter being parsed.
     *  @param id Name of the parameter to parse.
     *  @param val Reference to a boolean variable were to write the value of the parameter.
     *
     *  @return TRUE if the parameter was parsed.
     *
     */

    bool parseBooleanParameter(char *paramName, char *id, bool &val);

    /**
     *
     *  Parses a string parameter.
     *
     *  @param paramName Name of the parameter being parsed.
     *  @param id Name of the parameter to parse.
     *  @param string Reference to a string pointer.
     *
     *  @param return TRUE if the parameter was correctly parsed.
     *
     */

    bool parseStringParameter(char *paramName, char *id, char *&string);

    int getline(char **lineptr, size_t *n, FILE *stream);

public:

    /**
     *
     *  Config Loader constructor.
     *
     *  Creates and initializes a ConfigLoader object.
     *
     *  Tries to open the configuration file and initializes
     *  the object.
     *
     *  @param configFile The config file name.
     *
     *  @return A new initialized Config Loader object.
     *
     */

    ConfigLoader(char *configFile);

    /**
     *
     *  ConfigLoader destructor.
     *
     *  Deletes a ConfigLoader object.
     *
     *  Closes the configuration file and deletes all the ConfigLoader
     *  object structures.
     *
     */

    ~ConfigLoader();

    /**
     *
     *  Parses the config file and filles the simulator
     *  parameter structure.
     *
     *  @param simParam Pointer to the Simulator Parameters
     *  structure where the parsed parameters are going to
     *  be stored.
     *
     */

    void getParameters(SimParameters *simParam);

};

} // namespace gpu3d

#endif
