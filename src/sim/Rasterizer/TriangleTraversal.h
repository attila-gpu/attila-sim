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
 * $RCSfile: TriangleTraversal.h,v $
 * $Revision: 1.10 $
 * $Author: vmoya $
 * $Date: 2006-12-12 17:41:17 $
 *
 * Triangle Traversal box definition file.
 *
 */


/**
 *
 *  @file TriangleTraversal.h
 *
 *  This file defines the Triangle Traversal box.
 *
 *  This box simulates the traversal and fragment generation
 *  inside a triangle
 *
 */

#include "GPUTypes.h"
#include "Box.h"
#include "GPU.h"
#include "RasterizerCommand.h"
#include "RasterizerEmulator.h"
#include "TriangleSetupOutput.h"
#include "RasterizerState.h"
#include "PixelMapper.h"

#ifndef _TRIANGLETRAVERSAL_

#define _TRIANGLETRAVERSAL_

namespace gpu3d
{

/**  Defines the different Triangle Traversal  states.  */
enum TriangleTraversaState
{
    TT_READY,
    TT_TRAVERSING,
};

/**
 *
 *  This class implements the Triangle Traversal box.
 *
 *  This class implements the Triangle Traversal box that
 *  simulates the Triangle Traversal and Fragment Generation
 *  unit of a GPU.
 *
 *  Inherits from the Box class that offers basic simulation
 *  support.
 *
 */

class TriangleTraversal : public Box
{
private:

    /*  Triangle Traversal signals.  */
    Signal *traversalCommand;   /**<  Command signal from the main Rasterizer Box.  */
    Signal *traversalState;     /**<  State signal to the main Rasterizer Box.  */
    Signal *setupTriangle;      /**<  Setup triangle signal from the Setup box.  */
    Signal *setupRequest;       /**<  Triangle Traversal request to the Triangle Setup box.  */
    Signal *newFragment;        /**<  New fragment signal to the Hierarchical Z box.  */
    Signal *hzState;            /**<  State signal from the Hierarchical Z box.  */

    //  Triangle Traversal registers.
    u32bit hRes;                /**<  Display horizontal resolution.  */
    u32bit vRes;                /**<  Display vertical resolution.  */
    s32bit startX;              /**<  Viewport initial x coordinate.  */
    s32bit startY;              /**<  Viewport initial y coordinate.  */
    u32bit width;               /**<  Viewport width.  */
    u32bit height;              /**<  Viewport height.  */
    bool multisampling;         /**<  Flag that stores if MSAA is enabled.  */
    u32bit msaaSamples;         /**<  Number of MSAA Z samples generated per fragment when MSAA is enabled.  */
    bool perspectTransform;     /**<  Current vertex position transformation is perspective.  */

    /*  Triangle Traversal parameters.  */
    RasterizerEmulator &rastEmu;    /**<  Reference to the Rasterizer Emulator used to generate fragments.  */
    u32bit trianglesCycle;          /**<  Numbers of triangles to receive from Triangle Setup per cycle.  */
    u32bit setupLatency;            /**<  Latency of the triangle bus with Triangle Setup.  */
    u32bit stampsCycle;             /**<  Stamps to generate each cycle.  */
    u32bit samplesCycle;            /**<  Number of depth samples per fragment that can be generated per cycle when MSAA is enabled.  */
    u32bit triangleBatch;           /**<  Number of triangles to batch for (recursive) traversal.  */
    u32bit triangleQSize;           /**<  Size of the triangle queue.  */
    bool recursiveMode;             /**<  Flags that determines the rasterization method (TRUE: recursive, FALSE: scanline based).  */
    u32bit numStampUnits;           /**<  Number of stamp units in the GPU.  */
    u32bit overW;                    /**<  Over scan tile width in scan tiles.  */
    u32bit overH;                    /**<  Over scan tile height in scan tiles.  */
    u32bit scanW;                    /**<  Scan tile width in generation tiles.  */
    u32bit scanH;                    /**<  Scan tile height in generation tiles.  */
    u32bit genW;                     /**<  Generation tile width in stamps.  */
    u32bit genH;                     /**<  Generation tile height in stamps.  */

    /*  Triangle Traversal state.  */
    RasterizerState state;                  /**<  Current triangle traversal state.  */
    RasterizerCommand *lastRSCommand;       /**<  Stores the last Rasterizer Command received (for signal tracing).  */
    u32bit triangleCounter;                 /**<  Number of processed triangles.  */
    TriangleSetupOutput **triangleQueue;    /**<  Pointer an array of stored triangles to be traversed.  */
    u32bit storedTriangles;                 /**<  Number of stored triangles.  */
    u32bit nextTriangle;                    /**<  Pointer to the first triangle in the next batch to traverse.  */
    u32bit nextStore;                       /**<  Pointer to the next position where to store a new triangle.  */
    u32bit batchID;                         /**<  Identifier of the current batch of triangle being rasterized.  */
    u32bit batchTriangles;                  /**<  Number of triangles in the current batch.  */
    u32bit requestedTriangles;              /**<  Number of triangles requested to Triangle Setup.  */
    u32bit fragmentCounter;                 /**<  Number of fragments generated in the current batch/stream.  */
    u32bit nextMSAAQuadCycles;              /**<  Number of cycles until the next group of quads can be processed by the MSAA sample generator.  */
    u32bit msaaCycles;                      /**<  Number of cycles that it takes to generate all the MSAA samples for a fragment/quad.  */
    bool traversalFinished;                 /**<  Stores if the traversal of the current triangle has finished.  */
    bool lastFragment;                      /**<  Last batch fragment was generated.  */

    //  Pixel Mapper
    PixelMapper pixelMapper;    /**<  Maps pixels to addresses and processing units.  */

    /*  Statistics.  */
    GPUStatistics::Statistic *inputs;            /**<  Input triangles.  */
    GPUStatistics::Statistic *requests;          /**<  Triangle requests.  */
    GPUStatistics::Statistic *outputs;           /**<  Output fragments.  */
    GPUStatistics::Statistic* utilizationCycles; /**<  Cycle when rasterizer is doing real work (ie: waiting HZ is not accounted as utilizationCycles) */

    /* Stamp coverage related stats */
    GPUStatistics::Statistic *stamps0frag;           /**<  Empty stamps.                              */
    GPUStatistics::Statistic *stamps1frag;           /**<  Output stamps with 1 covered fragment.     */
    GPUStatistics::Statistic *stamps2frag;           /**<  Output stamps with 2 covered fragments.    */
    GPUStatistics::Statistic *stamps3frag;           /**<  Output stamps with 3 covered fragments.    */
    GPUStatistics::Statistic *stamps4frag;           /**<  Full covered stamps.                       */

    /*  Private functions.  */

    /**
     *
     *  Processes a rasterizer command.
     *
     *  @param command The rasterizer command to process.
     *  @param cycle  Current simulation cycle.
     *
     */

    void processCommand(RasterizerCommand *command, u64bit cycle);

    /**
     *
     *  Processes a register write.
     *
     *  @param reg The Triangle Traversal register to write.
     *  @param subreg The register subregister to writo to.
     *  @param data The data to write to the register.
     *
     */

    void processRegisterWrite(GPURegister reg, u32bit subreg, GPURegData data);

public:

    /**
     *
     *  Triangle Traversal constructor.
     *
     *  Creates and initializes a Triangle Traversal box.
     *
     *  @param rasEmu Reference to the Rasterizer Emulator to be used by Triangle Traversal to generate fragments.
     *  @param trianglesCycle Number of triangle received from Triangle Setup per cycle.
     *  @param setupLat Latency of the triangle bus with Triangle Setup.
     *  @param stampsCycle Number of stamps that Triangle Traversal has to generate per cycle.
     *  @param samplesCycle Number of depth samples per fragment that can be generated per cycle when MSAA is enabled.
     *  @param batchTriangles Number of triangles to batch for triangle traversal.
     *  @param trQueueSize Number of triangles to store in the triangle queue.
     *  @param recursiveMode Flags that determines the rasterization method to use.  If the
     *  flag is true it uses recursive rasterization and if it is false scanline based rasterization.
     *  @param microTrisAsFrag Process micro triangles directly as fragments, skipping setup and traversal operations on the triangle.
     *  @param nStampUnits Number of stamp units in the GPU.
     *  @param overW Over scan tile width in scan tiles (may become a register!).
     *  @param overH Over scan tile height in scan tiles (may become a register!).
     *  @param scanW Scan tile width in fragments.
     *  @param scanH Scan tile height in fragments.
     *  @param genW Generation tile width in fragments.
     *  @param genH Generation tile height in fragments.
     *  @param name The box name.
     *  @param parent The box parent box.
     *
     *  @return A new initialized Triangle Traversal box.
     *
     */

    TriangleTraversal(RasterizerEmulator &rasEmu, u32bit trianglesCycle, u32bit setupLat,
        u32bit stampsCycle, u32bit samplesCycle, u32bit batchTriangles, u32bit trQueueSize,       
        bool recursiveMode, bool microTrisAsFrag, u32bit nStampUnits,
        u32bit overW, u32bit overH, u32bit scanW, u32bit scanH, u32bit genW, u32bit genH,
        char *name, Box *parent);

    /**
     *
     *  Triangle Traversal simulation function.
     *
     *  Performs the simulation of a cycle of the Triangle
     *  Traversal box.
     *
     *  @param cycle The current simulation cycle.
     *
     */

    void clock(u64bit cycle);

    /**
     *
     *  Returns a single line string with state and debug information about the
     *  Triangle Traversal box.
     *
     *  @param stateString Reference to a string where to store the state information.
     *
     */

    void getState(std::string &stateString);

};

} // namespace gpu3d

#endif

