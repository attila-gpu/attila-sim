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
 * $RCSfile: TileEvaluator.h,v $
 * $Revision: 1.3 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:57 $
 *
 * Tile Evaluator box definition file.
 *
 */

 /**
 *
 *  @file TileEvaluator.h
 *
 *  This file defines the Tile Evaluator box.
 *
 *  This class simulates the Tile Evaluator used for recursive descent
 *  rasterization in a GPU.  The Tile Evaluator evaluates a triangle
 *  (or triangles) against a tiled section of the viewport.
 *
 */

#ifndef _TILEEVALUATOR_

#define _TILEEVALUATOR_

#include "GPUTypes.h"
#include "Box.h"
#include "Signal.h"
#include "GPU.h"
#include "support.h"
#include "RasterizerEmulator.h"
#include "RasterizerState.h"
#include "RasterizerCommand.h"
#include "TileInput.h"

namespace gpu3d
{

/**
 *
 *  Defines the Tile Evaluator state for Recursive Descent.
 */
enum TileEvaluatorState
{
    TE_READY,       /**<  The Tile Evaluator can receive new tiles.  */
    TE_FULL         /**<  The Tile Evaluator can not receive new tiles.  */
};

/**
 *
 *  This class implements the Tile Evaluator simulation box.
 *
 *  The Tile Evaluator box simulates the Tile Evaluator used for recursive
 *  descent rasterization in a GPU.
 *  Receives tiles from Recursive Descent box and evaluates them against the
 *  triangles being rasterized and the hierarchical Z buffer.
 *
 *  This class inherits from the Box class that offers basic simulation
 *  support.
 *
 */

class TileEvaluator : public Box
{
private:

    /*  Tile Evaluator signals.  */
    Signal *rastCommand;        /**<  Rasterizer command signal from the Rasterizer main box.  */
    Signal *rastState;          /**<  Rasterizer state signal to the Rasterizer main box.  */
    Signal *evalTileStart;      /**<  Evaluate tile signal to the Tile Evaluator (evaluation latency).  */
    Signal *evalTileEnd;        /**<  Evaluated tile signal from the Tile Evaluator.  (evaluation latency).  */
    Signal *newTiles;           /**<  New tile signals to the Recursive Descent.  */
    Signal *inputTileSignal;    /**<  Input tile signal from the Recursive Descent.  */
    Signal *evaluatorState;     /**<  State signal to the Recursive Descent.  */
    Signal *newFragments;       /**<  Generated fragment signal to the Recursive Descent.  */
    Signal *hzRequest;          /**<  Data signal from the Hierarchical Z Buffer.  */

    /*  Tile Evaluator state.  */
    RasterizerState state;      /**<  Current state of the Tile Evaluator box.  */
    RasterizerCommand *lastRSCommand;   /**<  Stores the last Rasterizer Command received (for signal tracing).  */
    u32bit tileCounter;         /**<  Number of processed tiles.  */
    bool lastTile;              /**<  Last batch tile/triangle received flag.  */
    u32bit fragmentCounter;     /**<  Number of fragments generated in the current batch/stream.  */

    /*  Tile Evaluator registers.  */

    /*  Tile Evaluator parameters.  */
    RasterizerEmulator &rastEmu;/**<  Reference to the rasterizer emulator used by the Tiled Evaluator.  */
    u32bit tilesCycle;          /**<  Number of generated tiles sent back to Recursive Descent per cycle.  */
    u32bit stampFragments;      /**<  Number of fragments per stamp.  */
    u32bit inputBufferSize;     /**<  Size of the input tile buffer.  */
    u32bit outputBufferSize;    /**<  Size of the output tile buffer.  */
    u32bit evaluationLatency;   /**<  Tile evaluation latency.  */
    char *prefix;               /**<  Tile evaluator prefix.  */

    /*  Tile Evaluator Input buffer.  */
    TileInput **inputBuffer;    /**<  Input tile buffer, stores tiles to be evaluated.  */
    u32bit firstInput;          /**<  Pointer to the first tile in the input buffer.  */
    u32bit nextFreeInput;       /**<  Pointer to the next free entry in the input buffer.  */
    u32bit inputTiles;          /**<  Number of stored tiles in the input buffer.  */

    /*  Tile Evaluator Output Buffer.  */
    TileInput **outputBuffer;   /**<  Output tile buffer, stores generated tiles to be sent to Recursive descent.  */
    u32bit firstOutput;         /**<  Pointer to the first tile in the output buffer.  */
    u32bit nextFreeOutput;      /**<  Pointer to the next free entry in the output buffer.  */
    u32bit outputTiles;         /**<  Number of tiles stored in the output buffer.  */
    u32bit reservedOutputs;     /**<  Number of reserved output buffer entries.  */

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
     *  Tile Evaluator constructor.
     *
     *  Creates and initializes a new Tile Evaluator box.
     *
     *  @param rastEmu Reference to the rasterizer emulator to be used
     *  to emulate rasterization operations.
     *  @param tilesCycle Number of generated tiles that can be sent
     *  back to Recursive Descent per cycle.
     *  @param stampFragments Number of fragments per stamp.
     *  @param inputBufferSize Size of the input tile buffer.
     *  @param outputBufferSize Size of the output tile buffer.
     *  @param evalLatency Tile evaluation latency.
     *  @param prefix Prefix name for the Tile Evaluator.
     *  @param name Name of the box.
     *  @param parent Parent box of the box.
     *
     *  @return A new initialized Tile Evaluator box.
     *
     */

    TileEvaluator(RasterizerEmulator &rastEmu, u32bit tilesCycle,
        u32bit stampFragments, u32bit inputBufferSize, u32bit outputBufferSize,
        u32bit evalLatency, char *prefix, char *name, Box* parent);

    /**
     *
     *  Tile Evaluator simulation rutine.
     *
     *  Simulates a cycle of the Tile Evaluator box.
     *
     *  @param cycle The cycle to simulate of the Tile Evaluator box.
     *
     */

    void clock(u64bit cycle);

};

} // namespace gpu3d

#endif
