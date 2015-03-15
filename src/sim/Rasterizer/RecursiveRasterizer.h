/**************************************************************************
 *
 * Copyright (c) 2002, 2003 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 * $RCSfile: RecursiveRasterizer.h,v $
 * $Revision: 1.4 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:56 $
 *
 * Recursive Rasterizer box class definition file.
 *
 */

/**
 *
 *  @file RecursiveRasterizer.h
 *
 *  This file defines the Recursive Rasterizer box.
 *
 *  The Rasterizer box controls all the rasterizer boxes:
 *  Triangle Setup, Recursive Descent, Tile Evaluators,
 *  Interpolator and Fragment FIFO.
 *
 */


#ifndef _RECURSIVERASTERIZER_

#define _RECURSIVERASTERIZER_

#include "GPUTypes.h"
#include "support.h"
#include "Box.h"
#include "RasterizerCommand.h"
#include "GPU.h"
#include "RasterizerState.h"
#include "TriangleSetup.h"
#include "RecursiveDescent.h"
#include "TileEvaluator.h"
#include "Interpolator.h"

namespace gpu3d
{

/**
 *
 *  Recursive Rasterizer box class definition.
 *
 *  This class implements a Recursive Rasterizer box that renders
 *  triangles from the vertex calculated in the vertex
 *  shader.
 *
 */

class RecursiveRasterizer: public Box
{
private:

    /*  Recursive Rasterizer signals.  */
    Signal *rastCommSignal;     /**<  Command signal from the Command Processor.  */
    Signal *rastStateSignal;    /**<  Recursive Rasterizer state signal to the Command Processor.  */
    Signal *triangleSetupComm;  /**<  Command signal to Triangle Setup.  */
    Signal *triangleSetupState; /**<  State signal from Triangle Setup.  */
    Signal *recDescComm;        /**<  Command signal to Recursive Descent.  */
    Signal *recDescState;       /**<  State signal from Recursive Descent.  */
    Signal **tileEvalComm;      /**<  Command signals to the Tile Evaluators.  */
    Signal **tileEvalState;     /**<  State signals from the Tile Evaluators.  */
    Signal *interpolatorComm;   /**<  Command signal to Interpolator box.  */
    Signal *interpolatorState;  /**<  State signal from Interpolator box.  */

    /*  Recursive Rasterizer boxes.  */
    TriangleSetup *triangleSetup;   /**<  Triangle Setup box.  */
    RecursiveDescent *recDescent;   /**<  Recursive Descent box.  */
    TileEvaluator **tileEvaluator;  /**<  Tile Evaluators boxes.  */
    Interpolator *interpolator;     /**<  Interpolator box.  */

    /*  Recursive Rasterizer registers.  */
    s32bit viewportIniX;            /**<  Viewport initial (lower left) X coordinate.  */
    s32bit viewportIniY;            /**<  Viewport initial (lower left) Y coordinate.  */
    u32bit viewportHeight;          /**<  Viewport height.  */
    u32bit viewportWidth;           /**<  Viewport width.  */
    f32bit nearRange;               /**<  Near depth range.  */
    f32bit farRange;                /**<  Far depth range.  */
    u32bit zBufferBitPrecission;    /**<  Z Buffer bit precission.  */
    bool frustumClipping;           /**<  Frustum clipping flag.  */
    QuadFloat userClip[MAX_USER_CLIP_PLANES];       /**<  User clip planes.  */
    bool userClipPlanes;            /**<  User clip planes enabled or disabled.  */
    CullingMode cullMode;           /**<  Culling Mode.  */
    bool interpolation;             /**<  Interpolation enabled or disabled (FLAT/SMOTH).  */
    bool fragmentAttributes[MAX_FRAGMENT_ATTRIBUTES];   /**<  Fragment input attributes active flags.  */

    /*  Recursive Rasterizer parameters.  */
    RasterizerEmulator &rastEmu;    /**<  Reference to the Rasterizer Emulator that is going to be used.  */
    u32bit triangleSetupFIFOSize;   /**<  Size of the Triangle Setup FIFO.  */
    u32bit recDescTrFIFOSize;       /**<  Recursive Descent triangle FIFO size.  */
    u32bit recDescTileStackSize;    /**<  Size of the Tile Stack at Recursive Descent.  */
    u32bit recDescFrBufferSize;     /**<  Recursive Descent fragment buffer size.  */
    u32bit stampFragments;          /**<  Number of fragments per stamp (lowest level tile).  */
    u32bit stampsCycle;             /**<  Number of stamps that can be send to Interpolator per cycle.  */
    u32bit numTileEvaluators;       /**<  Number of Tile Evaluators attached to Recursive Descent.  */
    u32bit tilesCycle;              /**<  Tiles that can be generated and sent per cycle from the Tile Evaluator to Recursive Descent.  */
    u32bit teInputBufferSize;       /**<  Tile Evaluator input tile buffer size.  */
    u32bit teOutputBufferSize;      /**<  Tile Evaluator output tile buffer size.  */
    u32bit tileEvalLatency;         /**<  Tile Evaluator evaluation latency per tile.  */
    char **tileEvalPrefixes;        /**<  Array with the name prefixes for the differente Tile Evaluator boxes attached to Recursive Descent.  */
    u32bit interpolators;           /**<  Number of hardware interpolators.  */

    /*  Recursive Rasterizer state.  */
    RasterizerState state;          /**<  Current state of the Recursive Rasterizer unit.  */
    RasterizerCommand *lastRastComm;/**<  Last rasterizer command received.  */
    RasterizerState *teState;       /**<  Array for the Tile Evaluators current state.  */

    /*  Private functions.  */

    /**
     *
     *  Processes new rasterizer commands received from the
     *  Command Processor.
     *
     *  @param rastComm The RasterizerCommand to process.
     *  @param cycle The current simulation cycle.
     *
     */

    void processRasterizerCommand(RasterizerCommand *rastComm, u64bit cycle);

    /**
     *
     *  Processes a rasterizer register write.
     *
     *  @param reg The rasterizer register to write to.
     *  @param subreg The Rasterizer register subregister to write to.
     *  @param data The data to write to the register.
     *  @param cycle The current simulation cycle.
     *
     */

    void processRegisterWrite(GPURegister reg, u32bit subreg, GPURegData data,
        u64bit cycle);

public:

    /**
     *
     *  Recursive Rasterizer constructor.
     *
     *  This function creates and initializates the Recursive Rasterizer
     *  box state and  structures.
     *
     *  @param rastEmu Reference to the Rasterizer Emulator that will be
     *  used for emulation.
     *  @param tsFIFOSize Size of the Triangle Setup FIFO.
     *  @param rdTriangleFIFOSize Recursive Descent Triangle FIFO size.
     *  @param rdTileStackSize Recursive Descent Tile Stack size.
     *  @param rdFrBufferSize Recursive Descent Fragment Buffer Size.
     *  @param stampFragments Fragments per stamp (lowest level tile).
     *  @param stampsCycle Stamps sent to Interpolator and Fragment FIFO
     *  boxes per cycle.
     *  @param tileEvals Number of Tile Evaluators attached to the
     *  Recursive Descent box.
     *  @param tilesCycle Number of tiles that can be generated and sent
     *  back to Recursive Descent per Tile Evaluator each cycle.
     *  @param teInputBufferSize Tile Evaluator Input Tile Buffer size.
     *  @param teOutputBufferSize Tile Evaluator Output Tile Buffer size.
     *  @param tileEvalLatency Tile Evaluator evaluation latency per tile.
     *  @param tileEvalPrefixes Tile Evaluator name and signal prefixes array.
     *  @param interp  Number of hardware interpolators available.
     *  @param name  Name of the box.
     *  @param parent  Pointer to a parent box.
     *
     */

    RecursiveRasterizer(RasterizerEmulator &rastEmu, u32bit tsFIFOSize,
        u32bit rdTriangleFIFOSize, u32bit rdTileStackSize,
        u32bit rdFrBufferSize, u32bit stampFragments, u32bit stampsCycle,
        u32bit tileEvals, u32bit tilesCycle, u32bit teInputBufferSize,
        u32bit teOutputBufferSize, u32bit tileEvalLatency,
        char **tileEvalPrefixes, u32bit interp,
        char *name, Box *parent = 0);

    /**
     *
     *  Performs the simulation and emulation of the rasterizer
     *  cycle by cycle.
     *
     *  @param cycle  The cycle to simulate.
     *
     */

    void clock(u64bit cycle);
};

} // namespace gpu3d

#endif
