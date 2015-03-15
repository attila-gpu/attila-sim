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
 * Streamer box definition file.
 *
 */

#ifndef _STREAMER_

#define _STREAMER_

#include "support.h"
#include "GPUTypes.h"
#include "Box.h"
#include "GPU.h"
#include "StreamerCommand.h"
#include "ShaderInput.h"
#include "ShaderFetch.h"
#include "ValidationInfo.h"

#include <map>

namespace gpu3d
{

/***  Unassigned output cache tag identifier.  */
static const u32bit UNASSIGNED_OUTPUT_TAG = 0xffffffff;

/***  This describes the streamer state/mode.  */
enum StreamerState
{
    ST_RESET,           /**<  Streamer is in reset mode.  */
    ST_READY,           /**<  Streamer can receive new commands and state changes.  */
    ST_STREAMING,       /**<  Streamer is streaming data to the vertex shaders.  */
    ST_WAITING,         /**<  Streamer is waiting for memory.  */
    ST_FINISHED         /**<  Streamer has finished streaming data to the vertex shaders.  */
};


/***  Table with the size in bytes of each Stream data type.  */
extern u32bit streamDataSize[];

} // namespace gpu3d

/*  Just after all the definitions these files need.  */
#include "StreamerFetch.h"
#include "StreamerOutputCache.h"
#include "StreamerLoader.h"
#include "StreamerCommit.h"

namespace gpu3d
{

/**
 *
 *  This class implements the simulation of the Streamer Box.
 *
 *  The Streamer Box request data to the memory controller
 *  from the buffers with vertex attribute data and sends
 *  vertices to the vertex shader.
 *
 */

class Streamer: public Box
{

private:

    /*  Streamer main box signals.  */
    Signal *streamCtrlSignal;       /**<  Control signal from the Command Processor.  */
    Signal *streamStateSignal;      /**<  State signal to the Command Processor.  */
    Signal *streamerFetchCom;       /**<  Command signal to the Streamer Fetch.  */
    Signal *streamerOCCom;          /**<  Command Signal to the Streamer Output Cache.  */
    Signal **streamerLoaderCom;     /**<  Command Signal to the Streamer Loader Units.  */
    Signal *streamerCommitCom;      /**<  Command Signal to the Streamer Commit.  */
    Signal *streamerFetchState;     /**<  State signal from the Streamer Fetch.  */
    Signal *streamerCommitState;    /**<  State signal from the Streamer Commit.  */

    /*  Streamer sub boxes.  */
    StreamerFetch *streamerFetch;               /**<  The Streamer Fetch subunit.  */
    StreamerOutputCache *streamerOutputCache;   /**<  The Streamer Output Cache subunit.  */
    StreamerLoader **streamerLoader;             /**<  The Streamer Loader subunits.  */
    StreamerCommit *streamerCommit;             /**<  The Streamer Commit subunit.  */

    /*  Streamer parameters.  */
    u32bit indicesCycle;            /**<  Indices per cycle processed in the Streamer.  */
    u32bit indexBufferSize;         /**<  The index data buffer size (bytes).  */
    u32bit outputFIFOSize;          /**<  The output FIFO size (entries).  */
    u32bit outputMemorySize;        /**<  The size in lines of the output Memory.  */
    u32bit numShaders;              /**<  Number of Shaders that must be feed from the streamer.  */
    u32bit maxShOutLat;             /**<  Maximum latency of the shader output signal.  */
    u32bit verticesCycle;           /**<  Vertices commited and sent to Primitive Assembly by the Streamer per cycle.  */
    u32bit paAttributes;            /**<  Vertex attributes that can be sent per cycle to Primitive Assembly.  */
    u32bit outputBusLat;            /**<  Latency of the vertex bus with Primitive Assembly.  */
    u32bit streamerLoaderUnits;     /**<  Number of Streamer Loader Units.  */
    u32bit slIndicesCycle;          /**<  Indices per cycle processed per Streamer Loader unit.  */
    u32bit slInputRequestQueueSize; /**<  The input request queue size (entries) (Streamer Loader unit).  */
    u32bit slFillAttributes;        /**<  Vertex attributes filled with data per cycle (Streamer Loader unit).  */
    u32bit slInputCacheLines;       /**<  Number of lines (per set) in the input cache (Streamer Loader unit).  */
    u32bit slInputCacheLineSize;    /**<  Input cache line size (Streamer Loader unit).  */
    u32bit slInputCachePortWidth;   /**<  Input cache port width (Streamer Loader unit).  */
    u32bit slInputCacheReqQSz;      /**<  Input cache request queue size (Streamer Loader unit).  */
    u32bit slInputCacheInQSz;       /**<  Input cache read request queue size (Streamer Loader unit).  */
    bool slForceAttrLoadBypass;     /**<  Forces attribute load bypass (Streamer Loader unit).  */

    /*  Streamer registers.  */
    u32bit bufferAddress[MAX_STREAM_BUFFERS];       /**<  Stream buffer address in GPU local memory.  */
    u32bit bufferStride[MAX_STREAM_BUFFERS];        /**<  Stream buffer stride.  */
    StreamData bufferData[MAX_STREAM_BUFFERS];      /**<  Stream buffer data type.  */
    u32bit bufferElements[MAX_STREAM_BUFFERS];      /**<  Number of elements (of any stream data type) per entry in the stream buffer.  */
    u32bit bufferFrequency[MAX_STREAM_BUFFERS];     /**<  Update/read frequency for the buffer: 0 -> per index/vertex, >0 -> MOD(instance, freq).  */
    u32bit attributeMap[MAX_VERTEX_ATTRIBUTES];     /**<  Mapping from vertex attributes to stream buffers..  */
    QuadFloat attrDefValue[MAX_VERTEX_ATTRIBUTES];  /**<  Vertex attributes default values.  */
    u32bit start;                                   /**<  Start position in the stream buffers from where start streaming data.  */
    u32bit count;                                   /**<  Count of data elements to stream to the Vertex Shaders.  */
    u32bit instances;                               /**<  Number of instances of the current stream to process.  */
    bool indexedMode;                               /**<  Use an index buffer to read vertex data.  */
    u32bit indexBuffer;                             /**<  Which stream buffer is used index buffer.  */
    bool outputAttribute[MAX_VERTEX_ATTRIBUTES];    /**<  Stores if the vertex output attribute is active/written.  */
    bool d3d9ColorStream[MAX_STREAM_BUFFERS];       /**<  Defines if the color stream must be read using the D3D9 color component order.  */
    bool attributeLoadBypass;                       /**<  Flag that is used to disable attribute load (StreamerLoader) when implementing attribute load in the shader.  */

    /*  Streamer state.  */
    StreamerState state;        /**<  Current state of the Streamer.  */
    StreamerCommand *lastStreamCom;     /**<  Keeps the last streamer command.  */

    /*  Private functions.  */

    /**
     *
     *  Processes a Streamer Command.
     *
     *  @param cycle Current simulation cycle.
     *  @param streamCom The Streamer Command to process.
     *
     */

    void processStreamerCommand(u64bit cycle, StreamerCommand *streamCom);

    /**
     *
     *  Processes a Streamer register write.
     *
     *  @param gpuReg Streamer register to write to.
     *  @param gpuSubReg Streamer register subregister to write to.
     *  @param gpuData Data to write to the Streamer register.
     *  @param cycle The current simulation cycle.
     *
     */

    void processGPURegisterWrite(GPURegister gpuReg, u32bit gpuSubReg,
        GPURegData gpuData, u64bit cycle);


public:

    /**
     *
     *  Streamer box constructor.
     *
     *  Creates and initializes a new Streamer box.
     *
     *  @param idxCycle Indices processed per cycle by the Streamer.
     *  @param indexBufferSize The size in bytes of the index data buffer.
     *  @param outputFIFOSize The size in entries of the output FIFO.
     *  @param outputMemorySize The size in lines of the output Memory.
     *  @param numShaders Number of Shaders fed by the Streamer.
     *  @param maxShOutLat Maximum latency of the shader output signal.
     *  @param vertCycle Number of vertices commited and sent to Primitive Assembly per cycle.
     *  @param paAttrCycle Vertex attributes that can be sent to Primitive Assembly per cycle.
     *  @param outLat Latency of the vertex bus with Primitive Assembly.
     *  @param sLoaderUnits Number of Streamer Loader units.
     *  @param slIdxCycle Indices per cycle processed per Streamer Loader unit.
     *  @param slInputReqSize The size in entries of the input request queue (Streamer Loader unit).
     *  @param slFillAttrCycle Vertex attributes filled per cycle by the Streamer (Streamer Loader unit).
     *  @param slInCacheLines Number of lines (per set) in the input cache (Streamer Loader unit).
     *  @param slInCacheLinesize Input cache line size (Streamer Loader unit).
     *  @param slInCachePortWidth Input cache port width (Streamer Loader unit).
     *  @param slInCacheReqQSz Input cache request queue size (Streamer Loader unit).
     *  @param slInCacheInQSz Input cache read request queue size (Streamer Loader unit).
     *  @param slForceAttrLoadBypass Forces attribute load bypass (Streamer Loader unit).
     *  @param slPrefixArray Array of prefixes for the streamer loader signals.     
     *  @param shPrefixArray Array of prefixes for the shader signals.
     *  @param name Name of the streamer box.
     *  @param parent Pointer to the parent box.
     *
     *  @return A new Streamer object.
     *
     */

    Streamer(u32bit idxCycle, u32bit indexBufferSize, u32bit outputFIFOSize, u32bit outputMemorySize,
        u32bit numShaders, u32bit maxShOutLat, u32bit vertCycle, u32bit paAttrCycle, u32bit outLat,
        u32bit sLoaderUnits, u32bit slIdxCycle, u32bit slInputReqQSize, u32bit slFillAttrCycle, u32bit slInCacheLines,
        u32bit slInCacheLinesize, u32bit slInCachePortWidth, u32bit slInCacheReqQSz, u32bit slInCacheInQSz,
        bool slForceAttrLoadBypass,
        char **slPrefixArray, char **shPrefixArray, char *name, Box *parent);

    /**
     *
     *  Streamer box simulation rutine.
     *
     *  Carries the cycle accurate simulation for the Streamer box.
     *  Receives commands and parameters from the Command Processor,
     *  requests memory transfers from the Memory Controller and
     *  sends vertex input data to the Vertex Shader.
     *
     *  @param cycle Cycle to simulate.
     *
     */

    void clock(u64bit cycle);

    /** 
     *
     *  Set Streamer validation mode.
     *
     *  @param enable Boolean value that defines if the validation mode is enabled.
     *
     */
     
    void setValidationMode(bool enable);
    
    /**
     *
     *  Get a map indexed per vertex index with the information about the vertices shaded in the current batch.
     *
     *  @return A reference to a map indexed per vertex index with the information about the vertices
     *  shaded in the current batch.
     *
     */
     
    ShadedVertexMap &getShadedVertexInfo();
    
    /**
     *
     *  Get a map indexed per vertex index with the information about the vertices read from a Streamer Loader unit
     *  in the current batch.
     *
     *  @param unit The Streamer Loader unit for which to obtain the information about read vertices.
     *    
     *  @return A reference to a map indexed per vertex index with the information about the vertices read in the
     *  current batch.
     *
     */
  
    VertexInputMap &getVertexInputInfo(u32bit unit);
    
};

} // namespace gpu3d

#endif

