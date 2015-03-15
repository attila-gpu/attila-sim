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
 * $RCSfile: RecursiveDescent.h,v $
 * $Revision: 1.3 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:56 $
 *
 * Recursive Descent box definition file.
 *
 */

 /**
 *
 *  @file RecursiveDescent.h
 *
 *  This file defines the Recursive Descent box.
 *
 *  This class simulates the Recursive Descent used for recursive descent
 *  rasterization in a GPU.
 *
 */

#ifndef _RECURSIVEDESCENT_

#define _RECUSRIVEDESCENT_

#include "GPUTypes.h"
#include "Box.h"
#include "Signal.h"
#include "GPU.h"
#include "support.h"
#include "RasterizerEmulator.h"
#include "TileEvaluator.h"
#include "RasterizerState.h"
#include "RasterizerCommand.h"
#include "TileInput.h"
#include "FragmentInput.h"
#include "TriangleSetupOutput.h"

namespace gpu3d
{


/***  Stamp tile level (lowest tile level).  */
static const u32bit STAMPLEVEL = 1;

/***  Maximum number of tiles that can be generated from a top level tile.  */
static const u32bit MAXGENERATEDTILES = 4;

/**
 *
 *  This class implements the Recursive Descent simulation box.
 *
 *  The Recursive Descent box simulates the Recursive Descent hardware
 *  used for recursive descent rasterization in a GPU.
 *
 *  This class inherits from the Box class that offers basic simulation
 *  support.
 *
 */

class RecursiveDescent : public Box
{
private:

    /*  Recursive Descent signals.  */
    Signal *rastCommand;        /**<  Rasterizer command signal from the Rasterizer main box.  */
    Signal *rastState;          /**<  Rasterizer state signal to the Rasterizer main box.  */
    Signal *newTriangle;        /**<  New setup triangle signal from Triangle Setup.  */
    Signal *triangleRequest;    /**<  Triangle request signal to Triangle Setup.  */
    Signal **evaluateTile;      /**<  Evaluate tile signal to the Tile Evaluators.  */
    Signal **newTiles;          /**<  New tile signals from the Tile Evaluators.  */
    Signal **newFragment;       /**<  New fragment signals from the Tile Evaluators.  */
    Signal **evaluatorState;    /**<  State signal from the Tile Evaluators.  */
    Signal *hzData;             /**<  Data signal from the Hierarchical Z Buffer.  */
    Signal *interpolator;       /**<  Fragment signal to the Interpolator.  */
    Signal *interpolatorState;  /**<  State signal from the Interpolator.  */

    /*  Recursive Descent state.  */
    RasterizerState state;      /**<  Current state of the Recursive Descent box.  */
    TileEvaluatorState *tileEvalStates; /**<  Current state of the Tile Evaluators connected to the box.  */
    RasterizerCommand *lastRSCommand;   /**<  Stores the last Rasterizer Command received (for signal tracing).  */
    u32bit triangleCounter;     /**<  Number of received setup triangles.  */
    bool lastTriangle;          /**<  Last batch triangle received flag.  */
    u32bit topLevel;            /**<  Current top tile level/size based in the current viewport size.  */

    /*  Recursive Descent registers.  */

    /*  Recursive Descent parameters.  */
    RasterizerEmulator &rastEmu;/**<  Reference to the rasterizer emulator used by Recursive Descent.  */
    u32bit triangleFIFOSize;    /**<  Size of the setup triangle FIFO.  */
    u32bit numTileEvaluators;   /**<  Number of Tile evaluators connected to the Recursive Descent.  */
    u32bit tilesCycle;          /**<  Tiles that can be received per cycle from each Tile Evaluator.  */
    u32bit tileStackSize;       /**<  Size of the Tile Stack (entries) for Recursive Descent.  */
    u32bit stampFragments;      /**<  Number of fragments per stamp (lowest level tile).  */
    u32bit stampsCycle;         /**<  Number of stamps per cycle to send down the pipeline.  */
    u32bit fragmentBufferSize;  /**<  Size (entries) of the fragment buffer.  */
    char **tileEvalPrefixes;    /**<  Array with the Tile Evaluators prefixes.  */

    /*  Recursive Descent Triangle FIFO.  */
    TriangleSetupOutput **triangleFIFO; /**<  Setup triangle FIFO where to store triangles pending for rasterization.  */
    u32bit firstTriangle;   /**<  Pointer to the first available triangle in the Setup Triangle FIFO.  */
    u32bit nextFreeSTEntry; /**<  Pointer to the next free entry in the Setup Triangle FIFO.  */
    u32bit storedTriangles; /**<  Number of triangles stored in the Setup Triangle FIFO.  */

    /*  Recursive Descent Tile Stack.  */
    TileInput ***tileStack; /**<  Tile Stack that stores the remaining tiles to be evaluated by the Tile Evaluators.  */
    u32bit *nextTile;       /**<  Pointer to the next (last) tile in the Tile Stack.  */
    u32bit *storedTiles;    /**<  Total number of stored tiles.  */
    u32bit *reservedTiles;  /**<  Reserved tile stack entries counter.  */
    u32bit totalStored;     /**<  Total stored tiles in the tile stacks counter.  */
    u32bit totalReserved;   /**<  Total reserved tile entries in the tile stacks counter.  */

    /*  Recursive Descent Fragment Buffer.  */
    FragmentInput **fragmentBuffer; /**<  Fragment buffer storing the fragments generated by the Tile Evaluators at the lowest tile level.  */
    u32bit firstFragment;           /**<  First fragment in the buffer.  */
    u32bit storedFragments;         /**<  Number of fragments in the buffer.  */
    u32bit nextFreeFBEntry;         /**<  Next free entry in the fragment buffer.  */

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
     *  Recursive Descent constructor.
     *
     *  Creates and initializes a new Recursive Descent box.
     *
     *  @param rastEmu Reference to the rasterizer emulator to be used
     *  to emulate rasterization operations.
     *  @param triangleFIFOSize Size of the setup triangle FIFO in the
     *  Recursive Descent hardware.
     *  @param numTileEvaluators Number of Tile Evaluators connected to the
     *  Recursive Descent.
     *  @param tilesCycleEvaluator Number of Tiles that can be received from
     *  each Tile Evaluator per cycle.
     *  @param tileStackSize Size of the Tile Stack in the Recursive Descent
     *  hardware.
     *  @param stampFragments Number of fragments per stamp (lowest level tile).
     *  @param stampsCycle Number of stamps per cycle to send down the pixel
     *  pipeline.
     *  @param fragmentBufferSize  Size of the fragment buffer.
     *  @param tileEvalPrefix Pointer to an array of Tile Evaluator name
     *  prefixes.
     *  @param name Name of the box.
     *  @param parent Parent box of the box.
     *
     *  @return A new initialized Recursive Descent box.
     *
     */

    RecursiveDescent(RasterizerEmulator &rastEmu, u32bit triangleFIFOSize,
        u32bit numTileEvaluators,  u32bit tilesCyclesEvaluator,
        u32bit tileStackSize, u32bit stampFragments, u32bit stampsCycle,
        u32bit fragmentBufferSize, char **tileEvalPrefix,
        char *name, Box* parent);

    /**
     *
     *  Recursive Descent simulation rutine.
     *
     *  Simulates a cycle of the Recursive Descent box.
     *
     *  @param cycle The cycle to simulate of the Recursive Descent box.
     *
     */

    void clock(u64bit cycle);

};

} // namespace gpu3d

#endif
