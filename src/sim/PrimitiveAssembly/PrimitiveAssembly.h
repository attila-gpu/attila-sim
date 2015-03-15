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
 * $RCSfile: PrimitiveAssembly.h,v $
 * $Revision: 1.10 $
 * $Author: cgonzale $
 * $Date: 2006-10-09 16:46:11 $
 *
 * Primitive Assembly box definition file.
 *
 */


/**
 *
 *  @file PrimitiveAssembly.h
 *
 *  This file defines the Primitive Assembly box.
 *
 *
 */


#ifndef _PRIMITIVEASSEMBLY_

#define _PRIMITIVEASSEMBLY_

#include "GPUTypes.h"
#include "Box.h"
#include "GPU.h"
#include "PrimitiveAssemblyCommand.h"

namespace gpu3d
{

/**  Number of shaded vertices in the assembly vertex queue.  */
//#define NUM_ASSEMBLY_VERTEX 4

/**  Defines the different Primitive Assembly unit status states signaled
     to the other units.   */
enum AssemblyState
{
    PA_READY,       /**<  Ready to receive new vertices from the Streamer Commit.  */
    PA_FULL,        /**<  Can not receive new vertices from the Streamer Commit.  */
    PA_END          /**<  All the vertex in the current stream/batch have been procesed.  */
};

/**  Defines the different Primitive Assembly unit functional states.  */
enum PrimitiveAssemblyState
{
    PAST_RESET,     /**<  Reset state.  */
    PAST_READY,     /**<  Ready State.  */
    PAST_DRAWING,   /**<  Drawing state.  */
    PAST_DRAW_END,  /**<  End of drawing state.  */
};

/**
 *
 *  Defines a Primitive Assembly Queue entry.
 *
 */

struct AssemblyQueue
{
    u32bit index;           /**<  Vertex index.  */
    QuadFloat *attributes;  /**<  Vertex attributes.  */
};


/**
 *
 *  This class implements the simulation box for the Primitive Assembly
 *  unit for a GPU.
 *
 *  Primitive Assembly receives vertex outputs from the Streamer Commit
 *  in the original stream order and groups them in primitives (triangles)
 *  based in the active primitive mode.
 *
 *  This class inherits from the Box class that offers basic simulation
 *  support.
 *
 */

class PrimitiveAssembly : public Box
{
private:

    /*  Primitive Assembly signals.  */
    Signal *assemblyCommand;        /**<  Command signal from the Command Processor.  */
    Signal *commandProcessorState;  /**<  State signal to the Command Processor.  */
    Signal *streamerNewOutput;      /**<  New vertex output signal from the Streamer Commit.  */
    Signal *assemblyRequest;        /**<  Request signal from the Primitive Assembly unit to the Streamer Commit unit.  */
    Signal *clipperInput;           /**<  Triangle input signal to the Clipper unit.  */
    Signal *clipperRequest;         /**<  Request signal from the Clipper unit.  */

    /*  Primitive assembly registers.  */
    u32bit verticesCycle;           /**<  Number of vertices received from Streamer Commit per cycle.  */
    u32bit trianglesCycle;          /**<  Number of triangles sent to the Clipper per cycle.  */
    u32bit attributesCycle;         /**<  Vertex attributes sent from Streamer Commit per cycle.  */
    u32bit vertexBusLat;            /**<  Latency of the vertex bus from Streamer Commit.  */
    u32bit paQueueSize;             /**<  Size in vertices of the primitive assembly queue.  */
    u32bit clipperStartLat;         /**<  Start latency of the clipper test units.  */


    /*  Primitive assembly registers.  */
    bool activeOutput[MAX_VERTEX_ATTRIBUTES]; /**<  Stores if a vertex output attribute is active.  */
    u32bit activeOutputs;           /**<  Number of vertex active output attributes.  */
    PrimitiveMode primitiveMode;    /**<  Current primitive mode.  */
    u32bit streamCount;             /**<  Number of vertex in the current stream/batch.  */
    u32bit streamInstances;         /**<  Number of instances of the current stream to process.  */

    /*  Primitive Assembly state.  */
    PrimitiveAssemblyState state;   /**<  Current state of the Primitive Assembly unit.  */
    u32bit triangleCount;           /**<  Number (and ID) of the assembled triangles in the current stream/batch.  */
    bool oddTriangle;               /**<  Indicates if the current triangle is or odd or even (used for triangle strips).  */
    u32bit degenerateTriangles;     /**<  Degenerated triangles counter.  */
    PrimitiveAssemblyCommand *lastPACommand;    /**<  Last primitive assembly command received (for signal tracing).  */
    bool lastTriangle;              /**<  If the last batch triangle has been already sent.  */
    u32bit clipCycles;              /**<  Cycles since last time triangles were sent to the clipper.  */
    u32bit clipperRequests;         /**<  Number of triangles requested by the clipper.  */

    /*  Primitive Assembly Queue.  */
    AssemblyQueue *assemblyQueue;   /**<  Assembly vertex queue.  */
    u32bit receivedVertex;          /**<  Number of received vertices from the Streamer.  */
    u32bit storedVertex;            /**<  Number of vertices currently stored in the assembly queue.  */
    u32bit requestVertex;           /**<  Number of vertices currenty requested to the Streamer unit.  */
    u32bit nextFreeEntry;           /**<  Position of the next free assembly queue entry.  */
    u32bit lastVertex;              /**<  Pointer to the last vertex stored in the assembly queue.  */

    /*  Statistics.  */
    GPUStatistics::Statistic *vertices; /**<  Number of received vertices received from Streamer Commit.  */
    GPUStatistics::Statistic *requests; /**<  Number of requests send to Streamer Commit.  */
    GPUStatistics::Statistic *degenerated;  /**<  Number of degenerated triangles found.  */
    GPUStatistics::Statistic *triangles;    /**<  Number of triangles sent to Clipper.  */

    /*  Private functions.  */

    /**
     *
     *  Processes a new Primitive Assembly Command.
     *
     *  @param command The primitive assembly command to process.
     *
     */

    void processCommand(PrimitiveAssemblyCommand *command);

    /**
     *
     *  Processes a register write to the Primitive Assembly.
     *
     *  @param reg Primitive Assembly register to write.
     *  @param subreg Primitive Assembly subregister to write.
     *  @param data Data to write to the register.
     *
     */

    void processRegisterWrite(GPURegister reg, u32bit subreg, GPURegData data);

    /**
     *
     *  Assembles a new triangle and sends it to the Clipper.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void assembleTriangle(u64bit cycle);

public:

    /**
     *
     *  Creates and initializes a new Primitive Assembly box.
     *
     *  @param vertCycle Vertices per cycle received from Streamer Commit.
     *  @param trCycle Triangles per cycle sent to Clipper.
     *  @param attrCycle Vertex attributes received from Streamer Commit per cycle.
     *  @param vertBusLat Latency of the vertex bus from Streamer Commit.
     *  @param paQueueSize Size in vertices of the assembly queue.
     *  @param clipStartLat Start latency of the Clipper test units.
     *  @param name The box name.
     *  @param parent The box parent box.
     *
     *  @return A new initialized PrimitiveAssembly box.
     *
     */

    PrimitiveAssembly(u32bit vertCycle, u32bit trCycle, u32bit attrCycle, u32bit vertBustLat, u32bit paQueueSize,
        u32bit clipStartLat, char *name, Box *parent);

    /**
     *
     *  Primitive Assembly simulation function.
     *
     *  Performs the simulation of a cycle of the Primitive Assembly
     *  GPU unit.
     *
     *  @param cycle The cycle to simulate.
     *
     */

    void clock(u64bit cycle);

    /**
     *
     *  Returns a single line string with state and debug information about the
     *  Primitive Assembly box.
     *
     *  @param stateString Reference to a string where to store the state information.
     *
     */

    void getState(std::string &stateString);

};

} // namespace gpu3d

#endif
