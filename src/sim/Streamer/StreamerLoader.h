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
 * $RCSfile: StreamerLoader.h,v $
 * $Revision: 1.18 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:54 $
 *
 * Streamer Loader class definition file.
 *
 */


/**
 *
 *  @file StreamerLoader.h
 *
 *  This file contains the definition of the Streamer Loader
 *  box.
 *
 */

namespace gpu3d
{
    class StreamerLoader;
} // namespace gpu3d

#ifndef _STREAMERLOADER_

#define _STREAMERLOADER_

#include "GPUTypes.h"
#include "Box.h"
#include "Streamer.h"
#include "StreamerCommand.h"
//#include "MemoryController.h"
#include "MemoryControllerDefs.h"
#include "GPU.h"
#include "StreamerControlCommand.h"
#include "InputCache.h"

namespace gpu3d
{

/**
 *
 *  This structure defines an Input Request Queue entry.
 *
 */

struct InputRequest
{
    u32bit index;                               /**<  The input index to load.  */
    u32bit instance;                            /**<  The instance index corresponding to the index.  */
    u32bit oFIFOEntry;                          /**<  Output FIFO entry where the index is stored.  */
    StreamerControlCommand *stCCom;             /**<  The streamer control command for this index.  */
    QuadFloat attributes[MAX_VERTEX_ATTRIBUTES];/**<  Input attributes.  */
    u32bit nextAttribute;                       /**<  Next attribute to fetch/read from memory.  */
    bool nextSplit;                             /**<  Fetching/reading the splited (second) part of the attribute.  */
    u32bit line[MAX_VERTEX_ATTRIBUTES][2];      /**<  Input cache line where the attribute is stored.  */
    u32bit address[MAX_VERTEX_ATTRIBUTES][2];   /**<  Address in memory where the attribute is stored.  */
    u32bit size[MAX_VERTEX_ATTRIBUTES][2];      /**<  Size in bytes of the attribute data to read from the cache.  */
    bool split[MAX_VERTEX_ATTRIBUTES];          /**<  Flag for attributes that must be splitted into two memory accesses.  */
};

/**
 *
 *  This class implements a Streamer Loader Box.
 *
 *  The Streamer Loader Box receives new indexes from the Streamer
 *  Output Cache or the Streamer Fetch, loads the attribute data for
 *  the index and issues it to the shaders.
 *
 *  Inherits from the Box class that offers simulation support.
 *
 */

class StreamerLoader : public Box
{

private:
    
    u32bit unitID;		    /**<  The Streamer Loader Unit identifier.  */

    /*  Streamer Loader signals.  */
    Signal *streamerFetchNewIndex;  /**<  New index signal from the Streamer Fetch.  */
    Signal *streamerNewIndex;       /**<  New index signal from the Streamer Output Cache.  */
    Signal *streamerLoaderDeAlloc;  /**<  Deallocation signal to the Streamer Fetch.  */
    Signal *memoryRequest;          /**<  Request signal to the Memory Controller.  */
    Signal *memoryData;             /**<  Data signal from the Memory Controller.  */
    Signal *streamerCommand;        /**<  Command signal from the main Streamer box.  */
    Signal **shCommSignal;          /**<  Pointer to an array of command signals to the Shaders.  */
    Signal **shStateSignal;         /**<  Pointer to an array of shader state signals.  */

    /*  Streamer Loader registers.  */
    u32bit streamAddress[MAX_STREAM_BUFFERS];   /**<  Stream buffer address in GPU local memory.  */
    u32bit streamStride[MAX_STREAM_BUFFERS];    /**<  Stream buffer stride.  */
    StreamData streamData[MAX_STREAM_BUFFERS];  /**<  Stream buffer data type.  */
    u32bit streamElements[MAX_STREAM_BUFFERS];  /**<  Number of elements (of any stream data type) per entry in the stream buffer.  */
    u32bit streamFrequency[MAX_STREAM_BUFFERS]; /**<  Update/read frequency for the buffer: 0 -> per index/vertex, >0 -> MOD(instance, freq).  */
    u32bit attributeMap[MAX_VERTEX_ATTRIBUTES]; /**<  Mapping from vertex attributes to stream buffers..  */
    QuadFloat attrDefValue[MAX_VERTEX_ATTRIBUTES];  /**<  Vertex attributes default values.  */
    u32bit streamStart;             /**<  Start position in the stream buffers from where start streaming data.  */
    bool d3d9ColorStream[MAX_STREAM_BUFFERS];   /**<  Sets if the color stream has be be read using the component order defined by D3D9.  */
    bool attributeLoadBypass;                   /**<  Flag used to disable attribute load, the index is passed to the shader through the index attribute
                                                      to implement attribute load in the shader using the LDA instruction.  */
                                                      
    /*  Streamer Loader parameters.  */
    u32bit indicesCycle;            /**<  Number of indices received from Streamer Output Cache per cycle.  */
    u32bit IRQSize;                 /**<  Size of the input request queue in entries.  */
    u32bit attributesCycle;         /**<  Number of vertex attributes filled per cycle.  */
    u32bit inputCacheLines;         /**<  Number of lines in the input cache.  */
    u32bit inputCacheLineSize;      /**<  Size in bytes of the input cache lines.  */
    u32bit inputCachePortWidth;     /**<  Width in bytes of the input cache ports.  */
    u32bit inputCacheReqQSize;      /**<  Number of entries in the cache request queue.  */
    u32bit inputCacheFillQSize;     /**<  Number of entries in the input/fill queue in the input cache.  */
    u32bit numShaders;              /**<  Number of shaders connected to the Streamer Loader.  */
    bool forceAttrLoadBypass;       /**<  Forces attribute load bypass (attribute load from the shader).  */

    /*  Streamer Loader state.  */
    StreamerState state;            /**<  State of the Streamer Loader.  */
    StreamerCommand *lastStreamCom; /**<  Pointer to the last Streamer Command received.  Kept for signal tracing.  */

    /*  Shader variables.  */
    ShaderState *shaderState;       /**<  Array storing the current state of the Shaders.  */
    u32bit nextShader;              /**<  Next shader unit where to send an input.  */

    /*  Input cache.  */
    InputCache *cache;              /**<  Pointer to the streamer loader input cache.  */
    u8bit buffer[32];               /**<  Buffer for the attribute being read from the input cache.  */
    
    //  Cache for instance attributes.
    u8bit instanceAttribData[MAX_VERTEX_ATTRIBUTES][32];    /**<  Stores per-instance attribute data.  */
    bool instanceAttribValid[MAX_VERTEX_ATTRIBUTES];        /**<  Stores if the per-instance attribute data is valid.  */
    u32bit instanceAttribTag[MAX_VERTEX_ATTRIBUTES];        /**<  Stores the instance index for the per-instance attribute stored.  */

    /*  Input request queue variables.  */
    InputRequest *inputRequestQ;    /**<  Input request queue.  */
    u32bit nextFreeInput;           /**<  Next free entry in the input request queue.  */
    u32bit nextFetchInput;          /**<  Next input to fetch from memory.  */
    u32bit nextReadInput;           /**<  Next input to read from memory.  */
    u32bit nextLoadInput;           /**<  Next input to be sent to the shaders.  */
    u32bit freeInputs;              /**<  Number of free entries in the input request queue.  */
    u32bit fetchInputs;             /**<  Number of inputs to fetch from memory.  */
    u32bit readInputs;              /**<  Number of inputs to be read from memory.  */
    u32bit loadInputs;              /**<  Number of inputs already loaded and ready to be sent to the shaders.  */

    /*  Memory variables.  */
    MemState memoryState;       /**<  Current state of the memory controller.  */

    /*  Statistics.  */
    GPUStatistics::Statistic *indices;             /**<  Number of indices (only counts indices that must be loaded) received.  */
    GPUStatistics::Statistic *transactions;        /**<  Number of transactions to Memory Controller.  */
    GPUStatistics::Statistic *bytesRead;           /**<  Bytes read from memory.  */
    GPUStatistics::Statistic *fetches;             /**<  Number of succesful fetch operations.  */
    GPUStatistics::Statistic *noFetches;           /**<  Number of non succesful fetch operations.  */
    GPUStatistics::Statistic *reads;               /**<  Number of succesful read operations.  */
    GPUStatistics::Statistic *noReads;             /**<  Number of non succesful read operations.  */
    GPUStatistics::Statistic *inputs;              /**<  Inputs sent to shaders.  */
    GPUStatistics::Statistic *splitted;            /**<  Number of splitted attributes.  */
    GPUStatistics::Statistic *mapAttributes;       /**<  Number of attributes mapped to a stream per vertex.  */
    GPUStatistics::Statistic *avgVtxSentThisCycle; /**<  Average vertices sent per cycle to the Shader units.  */ 
    GPUStatistics::Statistic *stallsShaderBusy;    /**<  Times the Streamer Loader unit cannot send a ready vertex because Shader Units are busy.  */ 

    //  Debug/validation.
    bool validationMode;                /**<  Flag that stores if the validation mode is enabled.  */
    u32bit nextReadLog;                 /**<  Pointer to the next read vertex log to read from.  */
    u32bit nextWriteLog;                /**<  Pointer to the next read vertex log to write to.  */
    VertexInputMap vertexInputLog[4];   /**<  Map, indexed by vertex index, of the vertices read in the current draw call.  */
    
    /*  Private functions.  */

    /**
     *
     *  Processes a Memory Transaction from the Memory Controller.
     *
     *  Checks the Memory Controller state and accepts requested
     *  data from the Memory Controller.
     *
     *  @param memTrans The Memory Transaction to process.
     *
     */

    void processMemoryTransaction(MemoryTransaction *memTrans);

    /**
     *
     *  Processes a Streamer Command.
     *
     *  @param streamCom The Streamer Command to process.
     *
     */

    void processStreamerCommand(StreamerCommand *streamCom);

    /**
     *
     *  Processes a Streamer register write.
     *
     *  @param gpuReg Streamer register to write to.
     *  @param gpuSubReg Streamer register subregister to write to.
     *  @param gpuData Data to write to the Streamer register.
     *
     */

    void processGPURegisterWrite(GPURegister gpuReg, u32bit gpuSubReg,
        GPURegData gpuData);

    /**
     *
     *  Converts from a stream buffer data format to a
     *  32 bit float point value.
     *
     *  @param format stream buffer data format.
     *  @param data pointer to the data to convert.
     *
     *  @return The converted 32 bit float.
     *
     */

    f32bit attributeDataConvert(StreamData format, u8bit *data);

    /**
     *
     *  Configures the addresses and sizes for the actives attributes
     *  of a stream input to be read from memory.
     *
     *  @param input Pointer to a input request queue entry.
     *
     */

    void configureAttributes(u64bit cycle, InputRequest *input);

    /**
     *
     *  Fills an input attribute with the read data using the format
     *  configuration currently active.
     *
     *  @param input Pointer to an input request queue entry.
     *  @param attr The input attribute to fill.
     *  @param data Pointer to a buffer with the attribute data read.
     *
     */

    void loadAttribute(InputRequest *input, u32bit attr, u8bit *data);


public:

    /**
     *
     *  Streamer Loader constructor.
     *
     *  Creates and initializes a new Streamer Loader object.
     *
     *  @param unitId The streamer loader unit identifier
     *  @param idxCycle Number of indices received from Streamer Output Cache per cycle.
     *  @param irqSize Size of the input request queue.
     *  @param attrCycle Number of vertex attributes filled per cycle.
     *  @param lines Number of lines in the input cache.
     *  @param linesize Size in bytes of the input cache lines.
     *  @param portWidth Width in bytes of the input cache ports.
     *  @param requestQSize Number of entries in the cache request queue.
     *  @param fillQSize Number of entries in the input/fill queue in the input cache.
     *  @param numShaders Number of Shaders fed by the Streamer Loader.
     *  @param forceAttrLoadBypass Forces attribute load bypass (attribute load is performed in the shader).
     *  @param shPrefixArray Array of prefixes for the shader signals.
     *  @param name The box name.
     *  @param prefix The Streamer Loader unit prefix
     *  @param parent The box parent box.
     *
     *  @return A new initialized Streamer Loader box.
     *
     */

    StreamerLoader(u32bit unitId, u32bit idxCycle, u32bit irqSize, u32bit attrCycle, u32bit lines, u32bit linesize,
        u32bit portWidth, u32bit requestQSize, u32bit fillQSize, bool forceAttrLoadBypass,
        u32bit numShaders, char **shPrefixArray, char *name, char *prefix, Box *parent);

    /**
     *
     *  Streamer Loader simulation rutine.
     *
     *  Simulates a cycle of the Streamer Loader.
     *
     *  @param cycle The cycle to simulate.
     *
     */

    void clock(u64bit cycle);

    /** 
     *
     *  Set Streamer Loader validation mode.
     *
     *  @param enable Boolean value that defines if the validation mode is enabled.
     *
     */
     
    void setValidationMode(bool enable);
    
    /**
     *
     *  Get a map indexed per vertex index with the information about the vertices read in the current batch.
     *
     *  @return A reference to a map indexed per vertex index with the information about the vertices
     *  read in the current batch.
     *
     */
     
    VertexInputMap &getVertexInputInfo();
};

} // namespace gpu3d

#endif


