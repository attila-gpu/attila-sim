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
 * $RCSfile: Interpolator.h,v $
 * $Revision: 1.12 $
 * $Author: vmoya $
 * $Date: 2005-07-15 11:47:06 $
 *
 * Interpolator box definition file.
 *
 */


/**
 *
 *  @file Interpolator.h
 *
 *  This file defines the Interpolator box.
 *
 *  The Interpolator box simulates the fragment attribute
 *  interpolators of a GPU.
 *
 */

#ifndef _INTERPOLATOR_

#define _INTERPOLATOR_

#include "GPUTypes.h"
#include "Box.h"
#include "GPU.h"
#include "RasterizerCommand.h"
#include "RasterizerEmulator.h"
#include "RasterizerState.h"

namespace gpu3d
{
/*** Maximum interpolation latency.  */
static const u32bit MAX_INTERPOLATION_LATENCY = 8;

/*** Default fragment attribute value.  */
#define DEFAULT_FRAGMENT_ATTRIBUTE(qf) qf.setComponents(0.0, 0.0, 0.0, 0.0);

/***  Interpolation latency.  */
static const u32bit INTERPOLATION_LATENCY = 4;

/**
 *
 *  This class implements the Interpolator box.
 *
 *  The Interpolator box simulates the interpolation
 *  of fragment attributes.
 *
 *  Inherits from the Box class that offers basic simulation
 *  support.
 *
 */

class Interpolator : public Box
{
private:

    /*  Interpolator signals.  */
    Signal *interpolatorCommand;    /**<  Command signal from the main rasterizer box.  */
    Signal *interpolatorRastState;  /**<  State signal to the main rasterizer box.  */
    Signal *newFragment;            /**<  New fragment signal from Triangle Traversal.  */
    Signal *interpolationStart;     /**<  Interpolation start signal.  */
    Signal *interpolationEnd;       /**<  Interpolation end signal.  */
    Signal *interpolatorOutput;     /**<  Fragment output signal to the Fragment FIFO unit.  */

    /*  Interpolator registers.  */
    u32bit depthBitPrecission;      /**<  Depth buffer bit precission.  */
    bool interpolation[MAX_FRAGMENT_ATTRIBUTES];        /**<  Flag storing if interpolation is enabled.  */
    bool fragmentAttributes[MAX_FRAGMENT_ATTRIBUTES];   /**<  Stores the fragment input attributes that are enabled and must be calculated.  */

    /*  Interpolator parameters.  */
    RasterizerEmulator &rastEmu;    /**<  Reference to the Rasterizer Emulator used to interpolate the fragment attributes.  */
    u32bit interpolators;           /**<  Number of hardware interpolators.  */
    u32bit stampsCycle;             /**<  Number of stamps received per cycle.  */
    u32bit numStampUnits;           /**<  Number of stamps units.  */

    /*  Interpolator state.  */
    RasterizerState state;          /**<  Current rasterization state.  */
    RasterizerCommand *lastRSCommand;   /**<  Stores the last Rasterizer Command received (for signal tracing).  */
    u32bit triangleCounter;     /**<  Number of processed triangles.  */
    u32bit fragmentCounter;     /**<  Number of fragments processed in the current batch.  */
    bool interpolationFinished; /**<  Stores if the interpolation of the current batch of fragments has finished.  */
    u32bit cyclesFragment;      /**<  Cycles needed before starting the interpolation of another group of fragments.  */
    u32bit remainingCycles;     /**<  Remaining cycles for the current group of fragments.  */
    u32bit currentTriangle;     /**<  Identifier of the current triangle being processed (used to count triangles).  */
    u32bit currentSetupTriangle;/**<  Identifier of the current setup triangle.  */
    bool lastTriangle;          /**<  Stores if the current triangle is the last triangle.  */
    bool firstTriangle;         /**<  First triangle already received flag.  */
    u32bit lastFragments;       /**<  Number of last fragments received for the current batch.  */

    /*  Private functions.  */

    /**
     *
     *  Processes a rasterizer command.
     *
     *  @param command The rasterizer command to process.
     *
     */

    void processCommand(RasterizerCommand *command);

    /**
     *
     *  Processes a register write.
     *
     *  @param reg The Interpolator register to write.
     *  @param subreg The register subregister to writo to.
     *  @param data The data to write to the register.
     *
     */

    void processRegisterWrite(GPURegister reg, u32bit subreg, GPURegData data);

public:

    /**
     *
     *  Interpolator box constructor.
     *
     *  Creates and initializes a new Interpolator box.
     *
     *  @param rastEmu The rasterizer emulator to be used to interpolate
     *  the fragment attributes.
     *  @param interpolators The number of hardware interpolators supported
     *  (number of interpolated attributes calculated per cycle).
     *  @param stampsCycle Number of stamps received per cycle.
     *  @param stampUnits Number of stamp units in the GPU.
     *  @param name The box name.
     *  @param parent The box parent box.
     *
     *  @return A new initialized Interpolator box.
     *
     */

    Interpolator(RasterizerEmulator &rastEmu, u32bit interpolators,
        u32bit stampsCycle, u32bit stampUnits, char *name, Box *parent);

    /**
     *
     *  Interpolator box simulation function.
     *
     *  Simulates a cycle of the Interpolator hardware.
     *
     *  @param cycle The current simulation cycle.
     *
     */

    void clock(u64bit cycle);
};

} // namespace gpu3d

#endif
