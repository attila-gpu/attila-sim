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
 * $RCSfile: StreamerCommit.h,v $
 * $Revision: 1.12 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:54 $
 *
 * Streamer Commit box definition file.
 *
 */

/**
 *
 *  @file StreamerCommit.h
 *
 *  This file defines the Streamer Commit box.
 *
 */


#ifndef _STREAMERCOMMIT_

#define _STREAMERCOMMIT_

namespace gpu3d
{
    class StreamerCommit;
} // namespace gpu3d


#include "GPUTypes.h"
#include "Box.h"
#include "Streamer.h"
#include "StreamerCommand.h"
#include "StreamerControlCommand.h"

namespace gpu3d
{

/**  Latency signal of the Streamer - Primitive Assembly bus.  */
//#define PRIMITIVE_ASSEMBLY_BASE_LATENCY 2

/**  Transmission latency for sending a single vertex attribute to primitive assembly.  */
//#define ATTRIBUTE_LATENCY 0.5

/**
 *
 *  This struct defines an output FIFO entry.
 *
 */

struct OFIFO
{
    u32bit index;       /**<  The index stored in this output FIFO entry.  */
    u32bit instance;    /**<  The instance for the stored index.  */
    u32bit omLine;      /**<  Output Memory line where the output for the index is stored.  */
    StreamerControlCommand *stCCom; /**<  Pointer to the new index streamer control command that carried the index.  */
    bool isShaded;                  /**<  If the index (output) has been shaded.  */
    u64bit startCycle;              /**<  Simulation cycle that entry was added to the Output FIFO.  */
    u64bit shadingLatency;          /**<  Shading latency of the output FIFO entry.  */
};

/**
 *
 *  This class implements the simulator box for the Streamer Commit unit.
 *
 *  The Streamer Commit box receives new index allocations from
 *  the Streamer Output Cache and outputs from the Shader.  Then
 *  sends the outputs to the Rasterizer in the order in which
 *  the indexes arrived.  It also manages when an output memory
 *  line can be deallocated.
 *
 *  This class inherits from the Box class that offers basic simulation
 *  support.
 *
 */

class StreamerCommit : public Box
{

private:

    /*  Streamer Commit signals.  */
    Signal **shOutputSignal;            /**<  Pointer to an array of shader output signals.  */
    Signal **shConsumerSignal;          /**<  Pointer to an array of consumer state (streamer) signals to the Shaders.  */
    Signal *streamerCommitCommand;      /**<  Command signal from the Streamer main box.  */
    Signal *streamerCommitState;        /**<  State signal to the Streamer main box.  */
    Signal *streamerCommitNewIndex;     /**<  New index signal from the Streamer Output cache.  */
    Signal *streamerCommitDeAlloc;      /**<  Deallocation signal to the Streamer Fetch.  */
    Signal *streamerCommitDeAllocOC;    /**<  Deallocation signal to the Streamer Output cache.  */
    Signal *assemblyOutputSignal;       /**<  Output signal to the Primitive Assembly unit.  */
    Signal *assemblyRequestSignal;      /**<  Request signal from the Primitive Assembly unit.  */

    /*  Streamer Commit parameters.  */
    u32bit indicesCycle;        /**<  Number of indices from Streamer Output Cache per cycle.  */
    u32bit outputFIFOSize;      /**<  Size (in entries) of the output FIFO.  */
    u32bit outputMemorySize;    /**<  Size (lines/outputs) of the output memory size.  */
    u32bit numShaders;          /**<  Number of shaders that attached to the Streamer.  */
    u32bit shMaxOutLat;         /**<  Maximum latency of the output signal from the Shader.  */
    u32bit verticesCycle;       /**<  Number of vertices per cycle to send to Primitive Assembly.  */
    u32bit attributesCycle;     /**<  Vertex attributes that can be send to Primitive Assembly per cycle.  */
    u32bit outputBusLat;        /**<  Latency of the vertex bus to Primitive Assembly.  */

    /*  Streamer Commit state variables.  */
    bool outputAttribute[MAX_VERTEX_ATTRIBUTES];/**<  Stores if a vertex output attribute is active/written.  */
    u32bit activeOutputs;                       /**<  Number of vertex output attributes active.  */
    u32bit streamCount;                         /**<  Number of index/input/outputs for the current batch.  */
    u32bit streamInstances;                     /**<  Number of instances of the current stream to process.  */
    StreamerState state;                        /**<  The current Streamer state.  */
    StreamerCommand *lastStreamCom;             /**<  Pointer to the last streamer command received from the Streamer main box.  */
    bool lastOutputSent;                        /**<  Whether the last index/vertex in the batch was sent to primitive assembly this cycle. */
    bool firstOutput;                           /**<  Whether the first index of a new batch has been sent by StreamerOutputCache this cycle.  */

    /*  Output Memory Variables.  */
    QuadFloat **outputMemory;   /**<  Output memory.  */
    u32bit *lastUseTable;       /**<  Last use table for the output memory lines.  */

    /*  Output FIFO variables.  */
    OFIFO *outputFIFO;          /**<  The Streamer Output FIFO.  */
    u32bit freeOFIFOEntries;    /**<  Number of free output FIFO entries.  */
    u32bit nextOutput;          /**<  Pointer to the next output FIFO to send to the rasterizer.  */

    /*  Deallocated Output Memory Lines structures.  */
    u32bit **outputMemoryDeAlloc;                     /**<  Array storing those deallocated indices in the last cycle that also deallocated its memory line.  */
    StreamerControlCommand** outputMemoryDeAllocCCom; /**<  Array storing the streamer control commands of the above deallocated indices.  */
    u32bit outputMemoryDeAllocs;                      /**<  Number of above deallocated indices the last cycle.  */
    bool *outputMemoryDeAllocHit;                     /**<  Stores if above deallocated indices match with any new index of the next cycle.   */

    /*  Streamer primitive assembly state.  */
    u32bit sentOutputs;         /**<  Number of outputs already sent to the Primitive Assembly unit.  */
    u32bit requestVertex;       /**<  Number of vertices requested by the Primitive Assembly unit.  */

    /*  Data transmission variables.  */
    u32bit *transCycles;        /**<  Array of counter storing the cycles remaining for the transmission of the current vertices to Primitive Assembly.  */

    /*  Statistics.  */
    GPUStatistics::Statistic *indices;        /**<  Indices received from Streamer Output Cache.  */
    GPUStatistics::Statistic *shOutputs;      /**<  Number of outputs received from the Shaders.  */
    GPUStatistics::Statistic *paOutputs;      /**<  Number of ouputs sent to Primitive Assembly.  */
    GPUStatistics::Statistic *paRequests;     /**<  Number of outputs requested by Primitive Assembly.  */
    GPUStatistics::Statistic *outAttributes;  /**<  Number of active output attributes per vertex.  */
    GPUStatistics::Statistic *avgVtxShadeLat; /**<  Average shading latency of vertices in the Output FIFO.  */

    //  Debug/validation.
    bool validationMode;                /**<  Flag that stores if the validation mode is enabled.  */
    u32bit nextReadLog;                 /**<  Pointer to the next shaded vertex log to read from.  */
    u32bit nextWriteLog;                /**<  Pointer to the next shaded vertex log to write to.  */
    ShadedVertexMap shadedVertexLog[4]; /**<  Map, indexed by vertex index, of the shaded vertices in the current draw call.  */

    /*  Private functions.  */

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



public:

    /**
     *
     *  Streamer Commit contructor.
     *
     *  Creates and initializes a new Streamer Commit box.
     *
     *  @param idxCycle Indices received from Streamer Output Cache per cycle.
     *  @param oFIFOSize Output FIFO Size (entries).
     *  @param omSize Size of the output memory in lines (outputs).
     *  @param numShaders Number of Shaders attached to the Streamer.
     *  @param maxOutLat Maximum latency of the shader output signal.
     *  @param vertCycle Vertices to send to Primite Assembly per cycle.
     *  @param attrCycle Number of attributes that can be sent per cycle to Primitive Assembly.
     *  @param outLat Latency of the vertex bus with Primitive Assembly.
     *  @param shPrefixArray Array of prefixes for the shader signals.
     *  @param name  The box name.
     *  @param parent The box parent box.
     *
     *  @return An initialized Streamer Commit box.
     *
     */

    StreamerCommit(u32bit idxCycle, u32bit oFIFOSize, u32bit omSize, u32bit numShaders, u32bit maxOutLat,
        u32bit vertCycle, u32bit attrCycle, u32bit outLat, char **shPrefixArray, char *name, Box *parent);

    /**
     *
     *  Streamer Commit simulation rutine.
     *
     *  Simulates a cycle of the Streamer Commit unit.
     *
     *  @param cycle The cycle to simulate of the Streamer
     *  Commit unit.
     *
     */

    void clock(u64bit cycle);

    /** 
     *
     *  Set Streamer Commit validation mode.
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


};

} // namespace gpu3d

#endif

