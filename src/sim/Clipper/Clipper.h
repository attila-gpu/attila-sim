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
 * $RCSfile: Clipper.h,v $
 * $Revision: 1.9 $
 * $Author: vmoya $
 * $Date: 2006-08-25 06:57:45 $
 *
 * Clipper box definition file.
 *
 */

/**
 *
 *  @file Clipper.h
 *
 *  Defines the Clipper box class.
 *
 *  This class simulates the hardware clipper in a GPU.
 *
 *
 */

#ifndef _CLIPPER_

#define _CLIPPER_

#include "GPUTypes.h"
#include "Box.h"
#include "GPU.h"
#include "TriangleSetupInput.h"
#include "ClipperCommand.h"


namespace gpu3d
{


/**
 *
 *  Defines the clipper state.
 *
 */
enum ClipperState
{
    CLP_RESET,  /**<  Reset state.  */
    CLP_READY,  /**<  Ready state.  */
    CLP_DRAW,   /**<  Draw state.  */
    CLP_END     /**<  End state.  */
};


/**
 *
 *  Defines the clipper status values sent to Primitive Assembly
 *
 */
enum ClipperStatus
{
    CLIPPER_READY,  /**<  Clipper is ready to receive a new triangle.  */
    CLIPPER_BUSY    /**<  Clipper is busy.  */
};


/**
 *
 *  This class implements the simulation box for the Clipper unit
 *  in a GPU.
 *
 *  The Clipper unit receives triangles (3 vertices and their
 *  attributes) and clips them against the the frustum clip volume
 *  and/or the user clip planes.  The Clipper unit can generate
 *  new vertices and triangles.
 *
 *  The current version of the clipper only performs trivial
 *  triangle reject if the triangle is outside the frustum clip
 *  volume.
 *
 *  The Clipper class inherits from the Box class that provides
 *  basic simulation support.
 *
 */

class Clipper : public Box
{

private:

    /*  Clipper signals.  */
    Signal *clipperCommand;          /**<  Clipper command signal from the Command Processor.  */
    Signal *clipperCommState;        /**<  Clipper state signal to the Command Processor.  */
    Signal *clipperInput;            /**<  Clipper triangle input from the Primitive Assembly unit.  */
    Signal *clipperRequest;          /**<  Clipper request signal to the Primitive Assembly unit.  */
    Signal *rasterizerNewTriangle;   /**<  New triangle signal to the Rasterizer unit.  */
    Signal *rasterizerRequest;       /**<  Request signal from the Rasterizer unit.  */
    Signal *clipStart;               /**<  Clipping start signal to the Clipper unit.  */
    Signal *clipEnd;                 /**<  Clipping end signal from the Clipper unit.  */

    /*  Clipper parameters.  */
    u32bit trianglesCycle;       /**<  Number of triangles per cycle received from Primitive Assembly  and sent to Rasterizer.  */
    u32bit clipperUnits;         /**<  Number of clipper test units.  */
    u32bit startLatency;         /**<  Clipper start latency.  */
    u32bit execLatency;          /**<  Clipper execution latency (triangle reject test).  */
    u32bit clipBufferSize;       /**<  Size of the buffer for clipped triangles.  */
    u32bit rasterizerStartLat;   /**<  Start latency for rasterizer unit.  */
    u32bit rasterizerOutputLat;  /**<  Latency of the triangle bus to Rasterizer.  */

    /*  Clipper registers.  */
    bool frustumClip;           /**<  Frustum clipping enable flag.  */
    bool d3d9DepthRange;        /**<  Defines the range for depth dimension in clip space, [0, 1] for D3D9, [-1, 1] for OpenGL.  */

    /*  Clipper state.  */
    ClipperState state;                 /**<  The current clipper state.  */
    ClipperCommand *lastClipperCommand; /**<  Stores the last clipper command.  */
    u32bit clipCycles;                  /**<  Cycles remaining until the next triangle can be clipped.  */
    u32bit rasterizerCycles;            /**<  Remaining cycles until a triangle can be sent to rasterizer.  */
    u32bit triangleCount;               /**<  Number of triangles processed in the current batch.  */
    u32bit requestedTriangles;          /**<  Number of vertices requested by Rasterizer.  */
    u32bit lastTriangleCycles;          /**<  Stores the number of cycles remaining until the last triangle signal reaches Rasterizer.  */

    /*  Clipped triangle buffer.  */
    TriangleSetupInput **clipBuffer;    /**<  Buffer for clipped triangles.  */
    u32bit nextClipTriangle;            /**<  Next clipped triangle in the buffer.  */
    u32bit nextFreeEntry;               /**<  Next free entry in the buffer.  */
    u32bit clippedTriangles;            /**<  Number of clipped triangles in the buffer.  */
    u32bit reservedEntries;             /**<  Number of reserved (triangles in the clipper pipeline) clip buffer entries.  */

    /*  Statistics.  */
    GPUStatistics::Statistic *inputs;   /**<  Number of input triangles.  */
    GPUStatistics::Statistic *outputs;  /**<  Number of output triangles.  */
    GPUStatistics::Statistic *clipped;  /**<  Number of clipped triangles.  */

    /*  Private functions.  */

    /**
     *
     *  Processes a clipper command.
     *
     *  @param clipComm The clipper command to process.
     *
     */

    void processCommand(ClipperCommand *clipComm);

    /**
     *
     *  Process register write.
     *
     *  @param reg Clipper register to write.
     *  @param subReg Clipper subregister to write.
     *  @param data Data to write to the register.
     *
     */

    void processRegisterWrite(GPURegister reg, u32bit subReg, GPURegData data);



public:

    /**
     *
     *  Clipper box constructor.
     *
     *  Creates and initializes a new Clipper box.
     *
     *  @param trianglesCycle Number of triangles received from Primitive Assembly per cycle and sent to Rasterizer.
     *  @param clipperUnits Number of clipper test units.
     *  @param startLatency Start Clipper latency for input triangles.
     *  @param exeLatency Execution latency for frustum clip reject test.
     *  @param bufferSize Size of the buffer for the clipped triangles.
     *  @param rasterizerStartLat Start latency of the rasterizer unit.
     *  @param rasterizerOutputLat Latency of the triangle bus to Rasterizer.
     *  @param parent The Clipper parent box.
     *  @param name The Clipper box name.
     *
     *  @return A new initialize Clipper box object.
     *
     */

    Clipper(u32bit trianglesCycle, u32bit clipperUnits, u32bit startLatency, u32bit execLatency,
        u32bit bufferSize, u32bit rasterizerStartLat, u32bit rasterizerOutputLat, char *name, Box *parent);

    /**
     *
     *  Clipper box simulation function.
     *
     *  Performs the simulation of a cycle of the Clipper unit.
     *
     *  @param cycle The current simulation cycle.
     *
     */

    void clock(u64bit cycle);

    /**
     *
     *  Returns a single line string with state and debug information about the
     *  Clipper box.
     *
     *  @param stateString Reference to a string where to store the state information.
     *
     */

    void getState(std::string &stateString);

};

} // namespace gpu3d

#endif


