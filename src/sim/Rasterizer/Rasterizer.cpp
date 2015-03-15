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
 * $RCSfile: Rasterizer.cpp,v $
 * $Revision: 1.25 $
 * $Author: vmoya $
 * $Date: 2007-01-21 18:57:45 $
 *
 * Rasterizer class implementation file (unified version).
 *
 */

#include "Rasterizer.h"
#include "RasterizerStateInfo.h"
#include <cstring>
#include <sstream>

/**
 *
 *  @file Rasterizer.cpp
 *
 *  This file implements the Rasterizer class (unified shaders version).
 *
 *  The Rasterizer class implements the main Rasterizer box.
 *
 */

/*  Rasterizer constructor.  */

using namespace gpu3d;

Rasterizer::Rasterizer(RasterizerEmulator &rsEmu, u32bit trCycle, u32bit tsFIFOSize,
    u32bit trSetupUnits, u32bit trSetupLat, u32bit trSetupStartLat, u32bit trInputLat, u32bit trOutputLat,
    bool shSetup, u32bit trShQSz, u32bit stampsPerCycle, u32bit depthSamplesCycle, u32bit overWidth, u32bit overHeight,
    u32bit scanWidth, u32bit scanHeight, u32bit genWidth, u32bit genHeight, u32bit trBatchSz, u32bit trBatchQSz,
    bool recursive, bool hzDisabled, u32bit stampsBlock, u32bit hzBufferSz, u32bit hzCacheLins,
    u32bit hzCacheLineSz, u32bit hzQueueSz, u32bit hzBufferLat, u32bit hzUpLat, u32bit clearBCycle,
    u32bit numInterpolators, u32bit shInQSz, u32bit shOutQSz, u32bit shInBSz, bool shTileDistro, bool unified,
    u32bit nVShaders, char **vshPrefix, u32bit nFShaders, char **fshPrefix, u32bit shInCycle,
    u32bit shOutCycle, u32bit thGroup, u32bit fshOutLat, u32bit vInQSz, u32bit vOutQSz,
    u32bit trInQSz, u32bit trOutQSz, u32bit rastQSz, u32bit testQSz, u32bit intQSz, u32bit shQSz,
    u32bit nStampUnits, char **suPrefixes, char *name, Box *parent):

    /*  Initializations.  */
    rastEmu(rsEmu), triangleSetupFIFOSize(tsFIFOSize), trianglesCycle(trCycle), triangleSetupUnits(trSetupUnits),
    triangleSetupLatency(trSetupLat), triangleSetupStartLat(trSetupStartLat), triangleInputLat(trInputLat),
    triangleOutputLat(trOutputLat), shadedSetup(shSetup), triangleShQSz(trShQSz), stampsCycle(stampsPerCycle),
    samplesCycle(depthSamplesCycle), overH(overHeight), overW(overWidth), scanH(scanHeight), scanW(scanWidth),
    genH(genHeight), genW(genWidth), trBatchSize(trBatchSz), trBatchQueueSize(trBatchQSz), recursiveMode(recursive),
    disableHZ(hzDisabled), blockStamps(stampsBlock), hzBufferSize(hzBufferSz),
    hzCacheLines(hzCacheLins), hzCacheLineSize(hzCacheLineSz), hzQueueSize(hzQueueSz), hzBufferLatency(hzBufferLat),
    hzUpdateLatency(hzUpLat), clearBlocksCycle(clearBCycle), interpolators(numInterpolators),
    shInputQSz(shInQSz), shOutputQSz(shOutQSz), shInputBatchSz(shInBSz), tiledShDistro(shTileDistro),
    unifiedModel(unified), numVShaders(nVShaders), numFShaders(nFShaders),
    shInputsCycle(shInCycle), shOutputsCycle(shOutCycle), threadGroup(thGroup), maxFShOutLat(fshOutLat),
    vInputQueueSz(vInQSz), vShadedQueueSz(vOutQSz), trInQueueSz(trInQSz), trOutQueueSz(trOutQSz),
    rastQueueSz(rastQSz), testQueueSz(testQSz), intQueueSz(intQSz), shQueueSz(shQSz),
    numStampUnits(nStampUnits), Box(name, parent)

{
    u32bit genStamps;
    DynamicObject *defSignal[1];

    /*  Check parameters.  */
    GPU_ASSERT(
        if (stampsCycle < numStampUnits)
            panic("Rasterizer", "Rasterizer", "One stamp generated per cycle and stamp unit required.");
        if (shadedSetup && !unifiedModel)
            panic("Rasterizer", "Rasterizer", "Triangle setup in the shaders only supported for unified shader model.");
    )

    /*  Create command signal from the Command Processor.  */
    rastCommSignal = newInputSignal("RasterizerCommand", 1, 1, NULL);

    /*  Create state signal to the Command Processor.  */
    rastStateSignal = newOutputSignal("RasterizerState", 1, 1, NULL);

    /*  Build initial signal state.  */
    defSignal[0] = new RasterizerStateInfo(RAST_RESET);

    /*  Set default state signal value.  */
    rastStateSignal->setData(defSignal);

    /*  Create command signal to Triangle Setup.  */
    triangleSetupComm = newOutputSignal("TriangleSetupCommand", 1, 1, NULL);

    /*  Create state signal from Triangle Setup.  */
    triangleSetupState = newInputSignal("TriangleSetupRasterizerState", 1, 1, NULL);

    /*  Create command signal to Triangle Traversal.  */
    triangleTraversalComm = newOutputSignal("TriangleTraversalCommand", 1, 1, NULL);

    /*  Create state signal from Triangle Traversal.  */
    triangleTraversalState = newInputSignal("TriangleTraversalState", 1, 1, NULL);

    /*  Create command signal to Hierarchical Z.  */
    hzCommand = newOutputSignal("HZCommand", 1, 1, NULL);

    /*  Create state sginal from Hierarchical Z.  */
    hzState = newInputSignal("HZState", 1, 1, NULL);

    /*  Create command signal to Interpolator box.  */
    interpolatorComm = newOutputSignal("InterpolatorCommand", 1, 1, NULL);

    /*  Create state signal from Interpolator box.  */
    interpolatorState = newInputSignal("InterpolatorRasterizerState", 1, 1, NULL);

    /*  Create command signal to the Fragment FIFO.  */
    fFIFOCommand = newOutputSignal("FFIFOCommand", 1, 1, NULL);

    /*  Create state signal from the Fragment FIFO.  */
    fFIFOState = newInputSignal("FFIFOState", 1, 1, NULL);


    /*  Create rasterizer boxes.  */

    /*  Create Triangle Setup box.  */
    triangleSetup = new TriangleSetup(
        rastEmu,                    /*  Reference to the rasterizer emulator object to use.  */
        trianglesCycle,             /*  Triangles received from Clipper and sent to Triangle Traversal per cycle.  */
        tsFIFOSize,                 /*  Size in triangles of the triangle FIFO.  */
        triangleSetupUnits,         /*  Number of setup units in the box.  */
        triangleSetupLatency,       /*  Latency of the setup unit.  */
        triangleSetupStartLat,      /*  Start latency of the setup unit.  */
        triangleInputLat,           /*  Latency of the bus with primitive assembly/clipper.  */
        0,                          /*  Latency of the bus with Triangle Bound unit (only in the Micropolygon rasterizer).  */
        triangleOutputLat,          /*  Latency of the bus with triangle traversal/fragment generation.  */
        shadedSetup,                /*  Performed triangle setup in the unified shaders.  */
        false,                      /*  Not using the Triangle Bound unit (only in the Micropolygon rasterizer).  */
        false,                      /*  Microtriangle bypass is disabled in the classic rasterizer.  */
        triangleShQSz,              /*  Size of the triangle shader queue.  */
        "TriangleSetup", this);

    /*  Calculate number of stamps per generation tile.  */
    genStamps = stampsCycle * (genW / STAMP_WIDTH) * (genH / STAMP_HEIGHT);

    /*  Create Triangle Traversal box.  */
    triangleTraversal = new TriangleTraversal(
        rastEmu,                /*  Reference to the rasterizer emulator object to be used.  */
        trianglesCycle,         /*  Triangles received from Triangle Setup per cycle.  */
        triangleOutputLat,      /*  Latency of the triangle bus from Triangle Setup.  */
        genStamps,              /*  Number of stamps to generate per cycle.  */
        samplesCycle,           /*  Number of MSAA samples to generate per fragment and cycle.  */
        trBatchSize,            /*  Number of triangles per rasterization batch.  */
        trBatchQueueSize,       /*  Number of triangles in the triangle queue.  */
        recursiveMode,          /*  Rasterization method: recursive or scanline.  */
        false,                  /*  Microtriangle bypass is disabled in the classic rasterizer.  */
        numStampUnits,          /*  Number of stamp units (Z Stencil/Color Write) in the GPU.  */
        overW,                  /*  Over scan tile width in scan tiles.  */
        overH,                  /*  Over scan tile height in scan tiles.  */
        scanW,                  /*  Scan tile height in registers.  */
        scanH,                  /*  Scan tile width in registers.  */
        genW,                   /*  Generation tile width in registers.  */
        genH,                   /*  Generation tile height in registers.  */
        "TriangleTraversal", this);

    /*  Create Hierarchical Z box.  */
    hierarchicalZ = new HierarchicalZ(
        genStamps,              /*  Stamps received per cycle.  */
        overW,                  /*  Over scan tile width in scan tiles.  */
        overH,                  /*  Over scan tile height in scan tiles.  */
        scanW,                  /*  Scan tile height in registers.  */
        scanH,                  /*  Scan tile width in registers.  */
        genW,                   /*  Generation tile width in registers.  */
        genH,                   /*  Generation tile height in registers.  */
        disableHZ,              /*  Flag that disables Hierarchical Z.  */
        blockStamps,            /*  Fragment stamps per HZ block.  */
        hzBufferSize,           /*  Size in block representatives of the Hierarchical Z Buffer.  */
        hzCacheLines,           /*  Hierarchical Z cache lines.  */
        hzCacheLineSize,        /*  Block info stored per HZ Cache line.  */
        hzQueueSize,            /*  Size of the Hierarchical Z test queue.  */
        hzBufferLatency,        /*  Access latency to the Hierarchical Z Buffer.  */
        hzUpdateLatency,        /*  Latency of the update signal from Z Stencil.  */
        clearBlocksCycle,       /*  Block representatives cleared per cycle in the HZ Buffer.  */
        numStampUnits,          /*  Number of stamp units (Z Stencil) attached to Fragment FIFO.  */
        suPrefixes,             /*  Array with the stamp unit prefixes.  */
        true,                   /*  Process microtriangle fragments (enabled in the Micropolygon Rasterizer).  */
        0,                      /*  Microtriangle mode (enabled in the Micropolygon Rasterizer).  */

        "HierarchicalZ", this);

    /*  Create Interpolator box.  */
    interpolator = new Interpolator(
        rastEmu,                /*  Reference to the rasterizer emulator to use for interpolation.  */
        interpolators,          /*  Number of attribute interpolator units.  */
        stampsCycle,            /*  Stamps received per cycle.  */
        numStampUnits,          /*  Number of stamp units in the GPU.  */
        "Interpolator", this);

    /*  Create Fragment FIFO box.  */
    fFIFO = new FragmentFIFO(
        genStamps,              /*  Stamps received from HZ per cycle.  */
        stampsCycle,            /*  Stamps send down the pipeline per cycle.  */
        unifiedModel,           /*  Simulate an unified shader model.  */
        shadedSetup,            /*  Perform setup in the unified shaders.  */
        triangleSetupUnits,     /*  Triangles received/sent from Triangle Setup per cycle.  */
        triangleSetupLatency,   /*  Latency of the Triangle Setup triangle input/output signals.  */
        numVShaders,            /*  Number of virtual vertex shaders attached.  */
        numFShaders,            /*  Number of fragment shaders attached.  */
        shInputsCycle,          /*  Shader inputs per cycle and shader.  */
        shOutputsCycle,         /*  Shader outputs per cycle and shader.  */
        threadGroup,            /*  Threads in a shader processing group.  */
        maxFShOutLat,           /*  Maximum latency of the Shader output signal.  */
        interpolators,          /*  Number of attribute interpolators in the Interpolator.  */
        shInputQSz,             /*  Shader input queue size (per shader unit).  */
        shOutputQSz,            /*  Shader output queue size (per shader unit).  */
        shInputBatchSz,         /*  Shader input batch size.  */
        tiledShDistro,          /*  Enable/disable distribution of fragment inputs to the shader units on a tile basis.  */
        vInputQueueSz,          /*  Size of the vertex input queue.  */
        vShadedQueueSz,         /*  Size of the vertex output queue.  */
        trInQueueSz,            /*  Size of the triangle input queue.  */
        trOutQueueSz,           /*  Size of the triangle output queue.  */
        rastQueueSz,            /*  Generated stamp queue size (per stamp unit).  */
        testQueueSz,            /*  Early Z tested stamp queue size.  */
        intQueueSz,             /*  Interpolated stamp queue size.  */
        shQueueSz,              /*  Shaded stamp queue size (per stamp unit).  */
        vshPrefix,              /*  Array with the virtual vertex shaders prefixes.  */
        fshPrefix,              /*  Array with the fragment shaders prefixes.  */
        numStampUnits,          /*  Number of stamp units (Z Stencil) attached to Fragment FIFO.  */
        suPrefixes,             /*  Array with the stamp unit prefixes.  */
        "FragmentFIFO", this);

    /*  Initialize the rasterizer state.  */
    state = RAST_RESET;

    /*  Create first command.  */
    lastRastComm = new RasterizerCommand(RSCOM_RESET);
}

//  Rasterizer destructor
Rasterizer::~Rasterizer()
{
    delete triangleSetup;
    delete triangleTraversal;
    delete hierarchicalZ;
    delete interpolator;
    delete fFIFO;
}
    
/*  Rasterizer simulation function.  */
void Rasterizer::clock(u64bit cycle)
{
    RasterizerCommand *rastComm;
    RasterizerStateInfo *rastStateInfo;
    RasterizerState tsState;
    RasterizerState ttState;
    RasterizerState hierZState;
    RasterizerState intState;
    RasterizerState ffState;
    int i;

    GPU_DEBUG_BOX( printf("Rasterizer => Clock %lld\n", cycle); )

    /*  Clock all rasterizer boxes.  */

    /*  Clock Triangle Setup.  */
    triangleSetup->clock(cycle);

    /*  Clock Triangle Traversal.  */
    triangleTraversal->clock(cycle);

    /*  Clock Hierarchical Z.  */
    hierarchicalZ->clock(cycle);

    /*  Clock Interpolator.  */
    interpolator->clock(cycle);

    /*  Clock Fragment FIFO.  */
    fFIFO->clock(cycle);

    /*  Read state from all rasterizer boxes.  */

    /*  Get the Triangle Setup state.  */
    if (triangleSetupState->read(cycle, (DynamicObject *&) rastStateInfo))
    {
        /*  Get triangle setup state.  */
        tsState = rastStateInfo->getState();

        /*  Delete rasterizer state info object.  */
        delete rastStateInfo;
    }
    else
    {
        panic("Rasterizer", "clock", "Missing signal from Triangle Setup.");
    }

    /*  Get the Triangle Traversal state.  */
    if (triangleTraversalState->read(cycle, (DynamicObject *&) rastStateInfo))
    {
        /*  Get Triangle Traversal state.  */
        ttState = rastStateInfo->getState();

        /*  Delete rasterizer state info object.  */
        delete rastStateInfo;
    }
    else
    {
        panic("Rasterizer", "clock", "Missing signal from Triangle Traversal.");
    }

    /*  Get the Hierarchical Z state.  */
    if (hzState->read(cycle, (DynamicObject *&) rastStateInfo))
    {
        /*  Get Hierarchical Z state.  */
        hierZState = rastStateInfo->getState();

        /*  Delete rasterizer state info object.  */
        delete rastStateInfo;
    }
    else
    {
        panic("Rasterizer", "clock", "Missing signal from Hierarchical Z.");
    }

    /*  Read state from Interpolator.  */
    if (interpolatorState->read(cycle, (DynamicObject *&) rastStateInfo))
    {
        /*  Get interpolator state.  */
        intState = rastStateInfo->getState();

        /*  Delete rasterizer state info object.  */
        delete rastStateInfo;
    }
    else
    {
        panic("Rasterizer", "clock", "Missing signal from Interpolator.");
    }

    /*  Read state from Fragment FIFO.  */
    if (fFIFOState->read(cycle, (DynamicObject *&) rastStateInfo))
    {
        /*  Get Fragment FIFO state.  */
        ffState = rastStateInfo->getState();

        /*  Delete rasterizer state info object.  */
        delete rastStateInfo;
    }
    else
    {
        panic("Rasterizer", "clock", "Missing signal from Fragment FIFO.");
    }

    /*  Simulate current cycle.  */
    switch(state)
    {
        case RAST_RESET:
            /*  The rasterizer is reset state.  */

            GPU_DEBUG_BOX( printf("Rasterizer => State RESET.\n"); )

            /*  Reset Rasterizer registers.  */
            hRes = 400;
            vRes = 400;
            d3d9PixelCoordinates = false;
            viewportIniX = 0;
            viewportIniY = 0;
            viewportWidth = 400;
            viewportHeight = 400;
            scissorTest = false;
            scissorIniX = 0;
            scissorIniY = 0;
            scissorWidth = 400;
            scissorHeight = 400;
            nearRange = 0.0;
            farRange = 1.0;
            d3d9DepthRange = false;
            hzEnable = TRUE;
            earlyZ = TRUE;
            modifyDepth = FALSE;
            clearDepth = 0x00ffffff;
            zBufferBitPrecission = 24;
            frustumClipping = TRUE;
            userClipPlanes = FALSE;
            cullMode = BACK;
            d3d9RasterizationRules = false;
            twoSidedLighting = FALSE;
            depthFunction = GPU_LESS;

            /*  Set user clip plane default values.  */
            for(i = 0; i < MAX_USER_CLIP_PLANES; i++)
                userClip[i] = QuadFloat(0.0, 0.0, 0.0, 0.0);

            /*  Set default fragment input attributes interpolation and active flags.  */
            for(i = 0; i < MAX_FRAGMENT_ATTRIBUTES; i++)
            {
                /*  Set all fragment attributes as interpolated.  */
                interpolation[i] = TRUE;

                /*  Set all fragment attributes as not active.  */
                fragmentAttributes[i] = FALSE;
            }

            //  Reset the render target state.
            for(u32bit rt = 0; rt < MAX_RENDER_TARGETS; rt++)
            {
                rtEnable[rt] = false;
                colorMaskR[rt] = true;
                colorMaskG[rt] = true;
                colorMaskB[rt] = true;
                colorMaskA[rt] = true;
            }
            
            //  Backbuffer/render target 0 enabled by default.
            rtEnable[0] = true;

            /*  Change state to ready. */
            state = RAST_READY;
            break;

        case RAST_READY:
            /*  The rasterizer is ready to receive commands from the
                Command Processor.  */

            GPU_DEBUG_BOX( printf("Rasterizer => State READY.\n"); )

            /*  Check if there is an available rasterizer command from the
                Command Processor.  */
            if (rastCommSignal->read(cycle, (DynamicObject *&) rastComm))
            {
                /*  Process the command.  */
                processRasterizerCommand(rastComm, cycle);
            }

            break;

        case RAST_DRAWING:
            /*  The rasterizer is drawing a batch for primitives.  */

            GPU_DEBUG_BOX( printf("Rasterizer => State DRAWING.\n"); )

            /*  Check if the rasterizer units has finished the current
                batch.  */
            if ((tsState == RAST_END) && (ttState == RAST_END) &&
                (hierZState == RAST_END) && (intState == RAST_END) && (ffState == RAST_END))
            {
                /*  Change to END state.  */
                state = RAST_END;
            }

            break;

        case RAST_END:
            /*  The rasterizer has finished drawing all the primitives
                in the current batch.  Waiting for response from the
                Command Processor.  */

            GPU_DEBUG_BOX( printf("Rasterizer => State RAST_END.\n"); )

            /*  Check if there is an available rasterizer command from the
                Command Processor.  */
            if (rastCommSignal->read(cycle, (DynamicObject *&) rastComm))
            {
                /*  Process the command.  */
                processRasterizerCommand(rastComm, cycle);
            }

        case RAST_CLEAR:

            /*  Clear state.  */

            GPU_DEBUG_BOX(
                printf("Rasterizer => State RAST_CLEAR.\n");
            )

            /*  Wait until Hierarchical Z ends clearing the HZ buffer.  */
            if (hierZState == RAST_CLEAR_END)
            {
                /*  Change state to end.  */
                state = RAST_CLEAR_END;
            }

            break;

        case RAST_CLEAR_END:
            /*  The rasterizer has finished clearing the buffers. Waiting
                for response from the Command Processor.  */

            GPU_DEBUG_BOX( printf("Rasterizer => State RAST_CLEAR_END.\n"); )

            /*  Check if there is an available rasterizer command from the
                Command Processor.  */
            if (rastCommSignal->read(cycle, (DynamicObject *&) rastComm))
            {
                /*  Process the command.  */
                processRasterizerCommand(rastComm, cycle);
            }

    }

    /*  Write state signal to the Command Processor.  */
    rastStateSignal->write(cycle, new RasterizerStateInfo(state));
}

/*  Process the rasterizer command.  */
void Rasterizer::processRasterizerCommand(RasterizerCommand *rastComm,
    u64bit cycle)
{
    RasterizerCommand *rastAuxComm;

    /*  Delete previous rasterizer command.  */
    delete lastRastComm;

    /*  Set new command as last command.  */
    lastRastComm = rastComm;

    /*  Process command.  */
    switch(rastComm->getCommand())
    {
        case RSCOM_RESET:
            /*  Reset the rasterizer.  */

            GPU_DEBUG_BOX( printf("Rasterizer => RESET Command.\n"); )

            /*  Send reset signal to all rasterizer boxes.  */

            /*  Send reset command to Triangle Setup.  */
            rastAuxComm = new RasterizerCommand(RSCOM_RESET);

            /*  Copy cookies from original command.  */
            rastAuxComm->copyParentCookies(*rastComm);

            /*  Send command to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastAuxComm);


            /*  Send reset command to Triangle Traversal.  */
            rastAuxComm = new RasterizerCommand(RSCOM_RESET);

            /*  Copy cookies from original command.  */
            rastAuxComm->copyParentCookies(*rastComm);

            /*  Send command to Triangle Traversal.  */
            triangleTraversalComm->write(cycle, rastAuxComm);


            /*  Send reset command to Hierarchical Z.  */
            rastAuxComm = new RasterizerCommand(RSCOM_RESET);

            /*  Copy cookies from original command.  */
            rastAuxComm->copyParentCookies(*rastComm);

            /*  Send command to Hierarchical Z.  */
            hzCommand->write(cycle, rastAuxComm);


            /*  Send reset command to Interpolator.  */
            rastAuxComm = new RasterizerCommand(RSCOM_RESET);

            /*  Copy cookies from original command.  */
            rastAuxComm->copyParentCookies(*rastComm);

            /*  Send command to Interpolator.  */
            interpolatorComm->write(cycle, rastAuxComm);


            /*  Send reset command to Fragment FIFO.  */
            rastAuxComm = new RasterizerCommand(RSCOM_RESET);

            /*  Copy cookies from original command.  */
            rastAuxComm->copyParentCookies(*rastComm);

            /*  Send command to Fragment FIFO.  */
            fFIFOCommand->write(cycle, rastAuxComm);

            /*  Change state to reset.  */
            state = RAST_RESET;

            break;

        case RSCOM_REG_WRITE:
            /*  Write to a Rasterizer register.  */

            /*  Check that current state is RAST_READY.  */
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("Rasterizer", "processRasterizerCommand", "Rasterizer register writes can only be received in READY state.");
            )

            GPU_DEBUG_BOX( printf("Rasterizer => REG_WRITE Command.\n"); )

            /*  Process the rasterizer register write.  */
            processRegisterWrite(rastComm->getRegister(),
                rastComm->getSubRegister(), rastComm->getRegisterData(), cycle);

            break;

        case RSCOM_REG_READ:
            panic("Rasterizer", "processRasterizerCommand", "Register read not supported.");
            break;

        case RSCOM_DRAW:
            /*  Start drawing primitives.  */

            GPU_DEBUG_BOX( printf("Rasterizer => DRAW Command.\n"); )

            /*  Check the current state.  */
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("Rasterizer", "processRasterizerCommand", "Rasterizer DRAW command can only be received in READY state.");
            )

            /*  Send DRAW command to Triangle Setup.  */
            rastAuxComm = new RasterizerCommand(RSCOM_DRAW);

            /*  Copy cookies from original command.  */
            rastAuxComm->copyParentCookies(*rastComm);

            /*  Send command to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastAuxComm);


            /*  Send DRAW command to Triangle Traversal.  */
            rastAuxComm = new RasterizerCommand(RSCOM_DRAW);

            /*  Copy cookies from original command.  */
            rastAuxComm->copyParentCookies(*rastComm);

            /*  Send command to Triangle Traversal.  */
            triangleTraversalComm->write(cycle, rastAuxComm);


            /*  Send DRAW command to Hierarchical Z.  */
            rastAuxComm = new RasterizerCommand(RSCOM_DRAW);

            /*  Copy cookies from original command.  */
            rastAuxComm->copyParentCookies(*rastComm);

            /*  Send command to Hierarchical Z.  */
            hzCommand->write(cycle, rastAuxComm);


            /*  Send DRAW command to Interpolator.  */
            rastAuxComm = new RasterizerCommand(RSCOM_DRAW);

            /*  Copy cookies from original command.  */
            rastAuxComm->copyParentCookies(*rastComm);

            /*  Send command to Interpolator.  */
            interpolatorComm->write(cycle, rastAuxComm);


            /*  Send DRAW command to Fragment FIFO.  */
            rastAuxComm = new RasterizerCommand(RSCOM_DRAW);

            /*  Copy cookies from original command.  */
            rastAuxComm->copyParentCookies(*rastComm);

            /*  Send command to Fragment FIFO.  */
            fFIFOCommand->write(cycle, rastAuxComm);


            /*  Change state to batch drawing.  */
            state = RAST_DRAWING;

            break;

        case RSCOM_END:
            /*  End drawing primitives.  */

            GPU_DEBUG_BOX( printf("Rasterizer => END Command.\n"); )

            /*  Check current state.  */
            if (state == RAST_END)
            {
                /*  Send END command to Triangle Setup.  */
                rastAuxComm = new RasterizerCommand(RSCOM_END);

                /*  Copy cookies from original command.  */
                rastAuxComm->copyParentCookies(*rastComm);

                /*  Send command to Triangle Setup.  */
                triangleSetupComm->write(cycle, rastAuxComm);


                /*  Send END command to Triangle Traversal.  */
                rastAuxComm = new RasterizerCommand(RSCOM_END);

                /*  Copy cookies from original command.  */
                rastAuxComm->copyParentCookies(*rastComm);

                /*  Send command to Triangle Traversal.  */
                triangleTraversalComm->write(cycle, rastAuxComm);


                /*  Send END command to Hierarchical Z.  */
                rastAuxComm = new RasterizerCommand(RSCOM_END);

                /*  Copy cookies from original command.  */
                rastAuxComm->copyParentCookies(*rastComm);

                /*  Send command to Hierarchical Z.  */
                hzCommand->write(cycle, rastAuxComm);


                /*  Send END command to Interpolator.  */
                rastAuxComm = new RasterizerCommand(RSCOM_END);

                /*  Copy cookies from original command.  */
                rastAuxComm->copyParentCookies(*rastComm);

                /*  Send command to Interpolator.  */
                interpolatorComm->write(cycle, rastAuxComm);


                /*  Send END command to Fragment FIFO.  */
                rastAuxComm = new RasterizerCommand(RSCOM_END);

                /*  Copy cookies from original command.  */
                rastAuxComm->copyParentCookies(*rastComm);

                /*  Send command to Fragment FIFO.  */
                fFIFOCommand->write(cycle, rastAuxComm);
            }
            else if (state == RAST_CLEAR_END)
            {
                /*  Send END command to Hierarchical Z.  */
                rastAuxComm = new RasterizerCommand(RSCOM_END);

                /*  Copy cookies from original command.  */
                rastAuxComm->copyParentCookies(*rastComm);

                /*  Send command to Hierarchical Z.  */
                hzCommand->write(cycle, rastAuxComm);
            }
            else
            {
                panic("Rasterizer", "processRasterizerCommand", "Rasterizer END command can only be received in END state.");
            }

            /*  Change state to ready.  */
            state = RAST_READY;

            break;

        case RSCOM_CLEAR_ZSTENCIL_BUFFER:


            /*  Send CLEAR command to Hierarchical Z.  */
            rastAuxComm = new RasterizerCommand(RSCOM_CLEAR_ZSTENCIL_BUFFER);

            /*  Copy cookies from original command.  */
            rastAuxComm->copyParentCookies(*rastComm);

            /*  Send command to Hierarchical Z.  */
            hzCommand->write(cycle, rastAuxComm);


            /*  Change state to clear.  */
            state = RAST_CLEAR;

            break;

        default:

            panic("Rasterizer", "processRasterizerCommand", "Unsuppored Rasterizer Command.");

            break;
    }

}

/*  Process the register write.  */
void Rasterizer::processRegisterWrite(GPURegister reg, u32bit subreg,
    GPURegData data, u64bit cycle)
{
    RasterizerCommand *rastComm;

    /*  Process the register.  */
    switch(reg)
    {
        case GPU_DISPLAY_X_RES:
            /*  Write display horizontal resolution register.  */
            hRes = data.uintVal;

            GPU_DEBUG_BOX(
                printf("Rasterizer => Write GPU_DISPLAY_X_RES = %d.\n", hRes);
            )

            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);


            /*  Send register write to Triangle Traversal.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Traversal.  */
            triangleTraversalComm->write(cycle, rastComm);


            /*  Send register write to Hierarchical Z.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Hierarchical Z.  */
            hzCommand->write(cycle, rastComm);

            break;

        case GPU_DISPLAY_Y_RES:
            /*  Write display vertical resolution register.  */
            vRes = data.uintVal;

            GPU_DEBUG_BOX(
                printf("Rasterizer => Write GPU_DISPLAY_Y_RES = %d.\n", vRes);
            )

            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);


            /*  Send register write to Triangle Traversal.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Traversal.  */
            triangleTraversalComm->write(cycle, rastComm);


            /*  Send register write to Hierarchical Z.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Hierarchical Z.  */
            hzCommand->write(cycle, rastComm);

            break;

        case GPU_D3D9_PIXEL_COORDINATES:
        
            //  Write the use D3D9 pixel coordinates convention register.
            
            d3d9PixelCoordinates = data.booleanVal;
            
            GPU_DEBUG_BOX(
                printf("Rasterizer => GPU_D3D9_PIXEL_COORDINATES = %s\n", d3d9PixelCoordinates ? "TRUE" : "FALSE");
            )
            
            //  Send register write to Triangle Setup.

            //  Create register write command.
            rastComm = new RasterizerCommand(reg, subreg, data);

            //  Copy cookies from last command.
            rastComm->copyParentCookies(*lastRastComm);

            //  Send register write to Triangle Setup.
            triangleSetupComm->write(cycle, rastComm);

            break;

            
        case GPU_VIEWPORT_INI_X:
            /*  Write the viewport initial x coordinate.  */
            viewportIniX = data.intVal;

            GPU_DEBUG_BOX( printf("Rasterizer => GPU_VIEWPORT_INI_X = %d.\n", data.intVal); )


            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);


            /*  Send register write to Triangle Traversal.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Traversal.  */
            triangleTraversalComm->write(cycle, rastComm);


            /*  Send register write to Hierarchical Z.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Hierarchical Z.  */
            hzCommand->write(cycle, rastComm);

            break;

        case GPU_VIEWPORT_INI_Y:
            /*  Write the viewport initial y coordinate.  */
            viewportIniY = data.intVal;

            GPU_DEBUG_BOX( printf("Rasterizer => GPU_VIEWPORT_INI_Y = %d.\n", data.intVal); )

            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);


            /*  Send register write to Triangle Traversal.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Traversal.  */
            triangleTraversalComm->write(cycle, rastComm);


            /*  Send register write to Hierarchical Z.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Hierarchical Z.  */
            hzCommand->write(cycle, rastComm);

            break;

        case GPU_VIEWPORT_HEIGHT:
            /*  Write the viewport height register.  */
            viewportHeight = data.uintVal;

            GPU_DEBUG_BOX( printf("Rasterizer => GPU_VIEWPORT_HEIGHT = %d.\n", data.uintVal); )

            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);


            /*  Send register write to Triangle Traversal.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Traversal.  */
            triangleTraversalComm->write(cycle, rastComm);


            /*  Send register write to Hierarchical Z.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Hierarchical Z.  */
            hzCommand->write(cycle, rastComm);

            break;

        case GPU_VIEWPORT_WIDTH:
            /*  Write the viewport width register.  */
            viewportWidth = data.uintVal;

            GPU_DEBUG_BOX( printf("Rasterizer => GPU_VIEWPORT_WIDTH = %d.\n", data.uintVal); )

            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);


            /*  Send register write to Triangle Traversal.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Traversal.  */
            triangleTraversalComm->write(cycle, rastComm);
            

            /*  Send register write to Hierarchical Z.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Hierarchical Z.  */
            hzCommand->write(cycle, rastComm);

            break;

        case GPU_SCISSOR_TEST:
            /*  Write the scissor test enable falg.  */
            scissorTest = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("Rasterizer => GPU_SCISSOR_TEST = %s.\n", scissorTest?"TRUE":"FALSE");
            )

            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);


            /*  Send register write to Hierarchical Z.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Hierarchical Z.  */
            hzCommand->write(cycle, rastComm);

            break;

        case GPU_SCISSOR_INI_X:
            /*  Write the scissor initial x position register.  */
            scissorIniX = data.intVal;

            GPU_DEBUG_BOX( printf("Rasterizer => GPU_SCISSOR_INIT_X = %d.\n", data.intVal); )

            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);


            /*  Send register write to Hierarchical Z.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Hierarchical Z.  */
            hzCommand->write(cycle, rastComm);

            break;

        case GPU_SCISSOR_INI_Y:
            /*  Write the scissor initial y position register.  */
            scissorIniY = data.intVal;

            GPU_DEBUG_BOX( printf("Rasterizer => GPU_SCISSOR_INIT_Y = %d.\n", data.intVal); )

            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);


            /*  Send register write to Hierarchical Z.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Hierarchical Z.  */
            hzCommand->write(cycle, rastComm);

            break;

        case GPU_SCISSOR_WIDTH:
            /*  Write the scissor width register.  */
            scissorWidth = data.uintVal;

            GPU_DEBUG_BOX( printf("Rasterizer => GPU_SCISSOR_WIDTH = %d.\n", data.uintVal); )

            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);


            /*  Send register write to Hierarchical Z.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Hierarchical Z.  */
            hzCommand->write(cycle, rastComm);

            break;

        case GPU_SCISSOR_HEIGHT:
            /*  Write the scissor height register.  */
            scissorHeight = data.uintVal;

            GPU_DEBUG_BOX( printf("Rasterizer => GPU_SCISSOR_HEIGHT = %d.\n", data.uintVal); )

            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);


            /*  Send register write to Hierarchical Z.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Hierarchical Z.  */
            hzCommand->write(cycle, rastComm);

            break;

        case GPU_DEPTH_RANGE_NEAR:
            /*  Write the near depth range register.  */
            nearRange = data.f32Val;

            GPU_DEBUG_BOX( printf("Rasterizer => GPU_DEPTH_RANGE_NEAR = %f.\n", data.f32Val); )

            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);

            break;

        case GPU_DEPTH_RANGE_FAR:
            /*  Write the far depth range register.  */
            nearRange = data.f32Val;

            GPU_DEBUG_BOX( printf("Rasterizer => GPU_DEPTH_RANGE_FAR = %f.\n", data.f32Val); )

            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);

            break;

        case GPU_D3D9_DEPTH_RANGE:
        
            //  Write the D3D9 depth range in clip space register.
            d3d9DepthRange = data.booleanVal;

            GPU_DEBUG_BOX( printf("Rasterizer => GPU_D3D9_DEPTH_RANGE = %s.\n", data.booleanVal ? "T" : "F"); )

            //  Send register write to Triangle Setup.

            //  Create register write command.
            rastComm = new RasterizerCommand(reg, subreg, data);

            //  Copy cookies from last command.
            rastComm->copyParentCookies(*lastRastComm);

            //  Send register write to Triangle Setup.
            triangleSetupComm->write(cycle, rastComm);

            break;

        case GPU_DEPTH_SLOPE_FACTOR:
            /*  Write the depth slope factor register.  */
            slopeFactor = data.f32Val;

            GPU_DEBUG_BOX( printf("Rasterizer => GPU_DEPTH_SLOPE_FACTOR = %f.\n", data.f32Val); )

            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);

            break;

        case GPU_DEPTH_UNIT_OFFSET:
            /*  Write the depth unit offset register.  */
            unitOffset = data.f32Val;

            GPU_DEBUG_BOX( printf("Rasterizer => GPU_DEPTH_UNIT_OFFSET = %f.\n", data.f32Val); )

            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);

            break;

        case GPU_Z_BUFFER_CLEAR:
            /*  Write depth clear value.  */
            clearDepth = data.uintVal;

            GPU_DEBUG_BOX(
                printf("Rasterizer => Write GPU_Z_BUFFER_CLEAR = %x.\n", clearDepth);
            )


            /*  Send register write to Hierarchical Z.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Hierarchical Z.  */
            hzCommand->write(cycle, rastComm);

            break;


        case GPU_Z_BUFFER_BIT_PRECISSION:
            /*  Write the z buffer bit precisison register.  */
            zBufferBitPrecission = data.uintVal;

            GPU_DEBUG_BOX( printf("Rasterizer => GPU_Z_BUFFER_BIT_PRECISSION = %d.\n", data.uintVal); )

            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);


            /*  Send register write to Hierarchical Z.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Hierarchical Z.  */
            hzCommand->write(cycle, rastComm);

            break;

        case GPU_HIERARCHICALZ:

            /*  Write HZ test enable flag.  */

            hzEnable = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("Rasterizer => Writing register GPU_HIERARCHICALZ = %s.\n",
                    hzEnable?"TRUE":"FALSE");
            )


            /*  Send register write to Hierarchical Z.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Hierarchical Z.  */
            hzCommand->write(cycle, rastComm);

            break;

        case GPU_EARLYZ:

            /*  Write early Z test enable flag.  */

            earlyZ = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("Rasterizer => Writing register GPU_EARLYZ = %s.\n",
                    earlyZ?"TRUE":"FALSE");
            )


            /*  Send register write to Hierarchical Z.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Fragment FIFO.  */
            fFIFOCommand->write(cycle, rastComm);

            break;

        case GPU_USER_CLIP:
            /*  Write an user clip plane register.  */
            userClip[subreg][0] = data.qfVal[0];
            userClip[subreg][1] = data.qfVal[1];
            userClip[subreg][2] = data.qfVal[2];
            userClip[subreg][3] = data.qfVal[3];

            GPU_DEBUG_BOX(
                printf("Rasterizer => GPU_USER_CLIP(%d) = (%f, %f, %f, %f).\n",
                    subreg, data.qfVal[0], data.qfVal[1], data.qfVal[2], data.qfVal[3]);
            )

            break;

        case GPU_USER_CLIP_PLANE:
            /*  Write user clip mode enable register.  */
            userClipPlanes = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("Rasterizer => GPU_USER_CLIP_PLANE = %s.\n",
                    data.booleanVal?"TRUE":"FALSE");
            )

            break;

        case GPU_FACEMODE:
            /*  Write the face mode register.  */
            faceMode = data.faceMode;

            GPU_DEBUG_BOX(
                printf("Rasterizer => GPU_FACEMODE = ");
                switch(data.culling)
                {
                    case GPU_CW:
                        printf("CW.\n");
                        break;

                    case GPU_CCW:
                        printf("CCW.\n");
                        break;
                }
            )

            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);

            break;

        case GPU_CULLING:
            /*  Write the culling mode register.  */
            cullMode = data.culling;

            GPU_DEBUG_BOX(
                printf("Rasterizer => GPU_CULLING = ");
                switch(data.culling)
                {
                    case NONE:
                        printf("NONE.\n");
                        break;

                    case FRONT:
                        printf("FRONT.\n");
                        break;

                    case BACK:
                        printf("BACK.\n");
                        break;

                    case FRONT_AND_BACK:
                        printf("FRONT_AND_BACK.\n");
                        break;

                }
            )

            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);

            break;

        case GPU_D3D9_RASTERIZATION_RULES:

            //  Write use D3D9 rasterization rules register.
            d3d9RasterizationRules = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("Rasterizer => GPU_D3D9_RASTERIZATION_RULES = %s\n", data.booleanVal?"ENABLED":"DISABLED");
            )

            //  Send register write to Triangle Setup.

            //  Create register write command.
            rastComm = new RasterizerCommand(reg, subreg, data);

            //  Copy cookies from last command.
            rastComm->copyParentCookies(*lastRastComm);

            //  Send register write to Triangle Setup.
            triangleSetupComm->write(cycle, rastComm);

            break;

        case GPU_TWOSIDED_LIGHTING:

            /*  Write two sided color selection register.  */
            twoSidedLighting = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("Rasterizer => GPU_TWOSIDED_LIGHTING = %s\n", data.booleanVal?"ENABLED":"DISABLED");
            )

            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);

            break;

        case GPU_MULTISAMPLING:
        
            //  Write Multisampling enable flag.
            multisampling = data.booleanVal;
            
            GPU_DEBUG_BOX(
                printf("Rasterizer => Write GPU_MULTISAMPLING = %s\n", multisampling?"TRUE":"FALSE");
            )
                       
            /*  Send register write to Triangle Traversal.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Traversal.  */
            triangleTraversalComm->write(cycle, rastComm);


            /*  Send register write to Hierarchical Z.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Hierarchical Z.  */
            hzCommand->write(cycle, rastComm);

            break;
            
        case GPU_MSAA_SAMPLES:
        
            //  Write MSAA z samples per fragment register.
            msaaSamples = data.uintVal;
            
            GPU_DEBUG_BOX(
                printf("Rasterizer => Write GPU_MSAA_SAMPLES = %d\n", msaaSamples);
            )

            /*  Send register write to Triangle Traversal.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Traversal.  */
            triangleTraversalComm->write(cycle, rastComm);


            /*  Send register write to Hierarchical Z.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Hierarchical Z.  */
            hzCommand->write(cycle, rastComm);

            break;            

        case GPU_INTERPOLATION:

            /*  Check if the attribute identifier is correct.  */
            GPU_ASSERT(
                if (subreg >= MAX_FRAGMENT_ATTRIBUTES)
                    panic("Rasterizer", "processRegisterWrite", "Out of range fragment attribute identifier.");
            )

            GPU_DEBUG_BOX(
                printf("Rasterizer => GPU_INTERPOLATION = %s.\n",
                    data.booleanVal?"TRUE":"FALSE");
            )

            /*  Write the interpolation mode enable register.  */
            interpolation[subreg] = data.booleanVal;

            /*  Send register write to Interpolator.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Interpolator.  */
            interpolatorComm->write(cycle, rastComm);


            break;

        case GPU_FRAGMENT_INPUT_ATTRIBUTES:
            /*  Write in fragment input attribute active flags register.  */

            /*  Check if the attribute identifier is correct.  */
            GPU_ASSERT(
                if (subreg >= MAX_FRAGMENT_ATTRIBUTES)
                    panic("Rasterizer", "processRegisterWrite", "Out of range fragment attribute identifier.");
            )

            /*  Set fragment attribute activation flag.  */
            fragmentAttributes[subreg] = data.booleanVal;

            /*  Send register write to Interpolator.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Interpolator.  */
            interpolatorComm->write(cycle, rastComm);


            /*  Send register write to Fragment FIFO.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Fragment FIFO.  */
            fFIFOCommand->write(cycle, rastComm);


            break;

        case GPU_MODIFY_FRAGMENT_DEPTH:

            /*  Write modify depth register.  */
            modifyDepth = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("Rasterizer => Write GPU_MODIFY_FRAGMENT_DEPTH = %s\n",
                    data.booleanVal?"TRUE":"FALSE");
            )

            /*  Send register write to Hierarchical Z.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Hierarchical Z.  */
            hzCommand->write(cycle, rastComm);

            break;

        case GPU_FRUSTUM_CLIPPING:
            /*  Write the frustum clipping flag register.  */
            frustumClipping = data.booleanVal;

            GPU_DEBUG_BOX( printf("Rasterizer => GPU_FRUSTUM_CLIPPING = %s.\n",
                data.booleanVal?"ENABLE":"DISABLED"); )

            break;

        case GPU_STENCIL_TEST:

            /*  Write the stencil test enable flag register.  */
            stencilTest = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("Rasterizer => GPU_STENCIL_TEST = %s.\n", data.booleanVal?"ENABLED":"DISABLED");
            )

            /*  Send register write to Fragment FIFO.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Fragment FIFO.  */
            fFIFOCommand->write(cycle, rastComm);

            break;

        case GPU_DEPTH_TEST:

            /*  Write the depth test enable flag register.  */
            depthTest = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("Rasterizer => GPU_DEPTH_TEST = %s.\n", data.booleanVal?"ENABLED":"DISABLED");
            )

            /*  Send register write to Fragment FIFO.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Fragment FIFO.  */
            fFIFOCommand->write(cycle, rastComm);

            break;


        case GPU_DEPTH_FUNCTION:

            /*  Write depth compare function register.  */
            depthFunction = data.compare;

            GPU_DEBUG_BOX(
                printf("Rasterizer => Write GPU_DEPTH_FUNCTION = ");

                switch(depthFunction)
                {
                    case GPU_NEVER:
                        printf("NEVER.\n");
                        break;

                    case GPU_ALWAYS:
                        printf("ALWAYS.\n");
                        break;

                    case GPU_LESS:
                        printf("LESS.\n");
                        break;

                    case GPU_LEQUAL:
                        printf("LEQUAL.\n");
                        break;

                    case GPU_EQUAL:
                        printf("EQUAL.\n");
                        break;

                    case GPU_GEQUAL:
                        printf("GEQUAL.\n");
                        break;

                    case GPU_GREATER:
                        printf("GREATER.\n");
                        break;

                    case GPU_NOTEQUAL:
                        printf("NOTEQUAL.\n");

                    default:
                        panic("Rasterizer", "processRegisterWrite", "Undefined compare function mode");
                        break;
                }
            )

            /*  Send register write to Hierarchical Z.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Hierarchical Z.  */
            hzCommand->write(cycle, rastComm);

            break;

        case GPU_RENDER_TARGET_ENABLE:

            //  Write the render target enable register.
            rtEnable[subreg] = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("Rasterizer => GPU_RENDER_TARGET_ENABLE[%d] = %s.\n", subreg, data.booleanVal?"TRUE":"FALSE");
            )

            //  Send register write to Fragment FIFO.

            //  Create register write command.
            rastComm = new RasterizerCommand(reg, subreg, data);

            //  Copy cookies from last command.
            rastComm->copyParentCookies(*lastRastComm);

            //  Send register write to Fragment FIFO.
            fFIFOCommand->write(cycle, rastComm);

            break;

        case GPU_COLOR_MASK_R:

            //  Write the red component color write mask register.
            colorMaskR[subreg] = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("Rasterizer => GPU_COLOR_MASK_R[%d] = %s.\n", subreg, data.booleanVal?"TRUE":"FALSE");
            )

            //  Send register write to Fragment FIFO.

            //  Create register write command.
            rastComm = new RasterizerCommand(reg, subreg, data);

            //  Copy cookies from last command.
            rastComm->copyParentCookies(*lastRastComm);

            //  Send register write to Fragment FIFO.
            fFIFOCommand->write(cycle, rastComm);

            break;

        case GPU_COLOR_MASK_G:

            //  Write the green component color write mask register.
            colorMaskG[subreg] = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("Rasterizer => GPU_COLOR_MASK_G[%d] = %s.\n", subreg, data.booleanVal?"TRUE":"FALSE");
            )

            //  Send register write to Fragment FIFO.

            //  Create register write command.
            rastComm = new RasterizerCommand(reg, subreg, data);

            //  Copy cookies from last command.
            rastComm->copyParentCookies(*lastRastComm);

            //  Send register write to Fragment FIFO.
            fFIFOCommand->write(cycle, rastComm);

            break;

        case GPU_COLOR_MASK_B:

            //  Write the blue component color write mask register.
            colorMaskB[subreg] = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("Rasterizer => GPU_COLOR_MASK_B[%d] = %s.\n", subreg, data.booleanVal?"TRUE":"FALSE");
            )

            //  Send register write to Fragment FIFO.

            //  Create register write command.
            rastComm = new RasterizerCommand(reg, subreg, data);

            //  Copy cookies from last command.
            rastComm->copyParentCookies(*lastRastComm);

            //  Send register write to Fragment FIFO.
            fFIFOCommand->write(cycle, rastComm);

            break;

        case GPU_COLOR_MASK_A:

            //  Write the alpha component color write mask register.
            colorMaskA[subreg] = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("Rasterizer => GPU_COLOR_MASK_A[%d] = %s.\n", subreg, data.booleanVal?"TRUE":"FALSE");
            )

            //  Send register write to Fragment FIFO.

            //  Create register write command.
            rastComm = new RasterizerCommand(reg, subreg, data);

            //  Copy cookies from last command.
            rastComm->copyParentCookies(*lastRastComm);

            //  Send register write to Fragment FIFO.
            fFIFOCommand->write(cycle, rastComm);

            break;


        default:
            /*  Unsupported register.  */
            panic("Rasterizer", "processRegisterWrite", "Unsupported Rasterizer register.");
            break;
    }
}

void Rasterizer::getState(string &stateString)
{
    stringstream stateStream;

    stateStream.clear();

    stateStream << " state = ";

    switch(state)
   {
        case RAST_RESET:
            stateStream << "RAST_RESET";
            break;
        case RAST_READY:
            stateStream << "RAST_READY";
            break;
        case RAST_DRAWING:
            stateStream << "RAST_DRAW";
            break;
        case RAST_END:
            stateStream << "RAST_END";
            break;
        case RAST_CLEAR:
            stateStream << "RAST_CLEAR";
            break;
        case RAST_CLEAR_END:
            stateStream << "RAST_CLEAR_END";
            break;
        default:
            stateStream << "undefined";
            break;
    }

    stateString.assign(stateStream.str());
}

void Rasterizer::saveHZBuffer()
{
    hierarchicalZ->saveHZBuffer();
}

void Rasterizer::loadHZBuffer()
{
    hierarchicalZ->loadHZBuffer();
}

void Rasterizer::detectStall(u64bit cycle, bool &active, bool &stalled)
{
    bool detectionImplemented;
    bool stallDetected;
    
    active = false;
    stalled = false;
    
    //  Detect stalls on all the boxes instantiated inside Rasterizer.
    triangleSetup->detectStall(cycle, detectionImplemented, stallDetected);
    active |= detectionImplemented;
    stalled |= detectionImplemented & stallDetected;
    
    triangleTraversal->detectStall(cycle, detectionImplemented, stallDetected);
    active |= detectionImplemented;
    stalled |= detectionImplemented & stallDetected;

    hierarchicalZ->detectStall(cycle, detectionImplemented, stallDetected);
    active |= detectionImplemented;
    stalled |= detectionImplemented & stallDetected;
    
    interpolator->detectStall(cycle, detectionImplemented, stallDetected);
    active |= detectionImplemented;
    stalled |= detectionImplemented & stallDetected;
    
    fFIFO->detectStall(cycle, detectionImplemented, stallDetected);
    active |= detectionImplemented;
    stalled |= detectionImplemented & stallDetected;
}

void Rasterizer::stallReport(u64bit cycle, string &stallReport)
{
    stringstream reportStream;

    string report;
        
    //  Obtain the stall report from all the boxes instantiated inside Rasterizer.
    triangleSetup->stallReport(cycle, report);
    reportStream << report << endl << endl;
    triangleTraversal->stallReport(cycle, report);
    reportStream << report << endl << endl;
    hierarchicalZ->stallReport(cycle, report);
    reportStream << report << endl << endl;
    interpolator->stallReport(cycle, report);
    reportStream << report << endl << endl;
    fFIFO->stallReport(cycle, report);
    reportStream << report << endl;
    
    stallReport.assign(reportStream.str());
}
