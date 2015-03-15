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
 * $RCSfile: RecursiveRasterizer.cpp,v $
 * $Revision: 1.4 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:56 $
 *
 * Recursive Rasterizer class implementation file.
 *
 */

#include "RecursiveRasterizer.h"
#include "RasterizerStateInfo.h"
#include <cstring>

using namespace gpu3d;

/**
 *
 *  @file RecursiveRasterizer.cpp
 *
 *  This file implements the RecursiveRasterizer class.
 *
 *  The Recursive Rasterizer class implements the main Recursive
 *  Rasterizer box.
 *
 */

/*  RecursiveRasterizer constructor.  */

RecursiveRasterizer::RecursiveRasterizer(RasterizerEmulator &rsEmu,
    u32bit tsFIFOSize, u32bit rdTrFIFO, u32bit rdTileStack, u32bit rdFrBuffer,
    u32bit stampFr, u32bit stampPerCycle, u32bit tileEval, u32bit tilesPerCycle,
    u32bit inputBuffer, u32bit outputBuffer, u32bit tileLatency,
    char **tePrefixes, u32bit numInterpolators, char *name, Box *parent):

/*  Initializations.  */
rastEmu(rsEmu), triangleSetupFIFOSize(tsFIFOSize), recDescTrFIFOSize(rdTrFIFO),
recDescTileStackSize(rdTileStack), recDescFrBufferSize(rdFrBuffer),
stampFragments(stampFr), stampsCycle(stampPerCycle),
numTileEvaluators(tileEval), tilesCycle(tilesPerCycle),
teInputBufferSize(inputBuffer), teOutputBufferSize(outputBuffer),
tileEvalLatency(tileLatency), tileEvalPrefixes(tePrefixes),
interpolators(numInterpolators), Box(name, parent)

{
    DynamicObject *defSignal[1];
    char teName[255];
    u32bit i;

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

    /*  Create command signal to Recursive Descent.  */
    recDescComm = newOutputSignal("RecursiveDescentCommand", 1, 1, NULL);

    /*  Create state signal from Recursive Descent.  */
    recDescState = newInputSignal("RecursiveDescentState", 1, 1, NULL);


    /*  Create signals to the Tile Evaluators.  */

    /*  Create array of command signals to the Tile Evaluators.  */
    tileEvalComm = new Signal*[numTileEvaluators];

    /*  Check allocation */
    GPU_ASSERT(
        if (tileEvalComm == NULL)
            panic("RecursiveRasterizer", "RecursiveRasterizer", "Error allocating command signal array to the Tile Evaluators.");
    )

    /*  Create array of state signals from the Tile Evaluators.  */
    tileEvalState = new Signal*[numTileEvaluators];

    /*  Check allocation */
    GPU_ASSERT(
        if (tileEvalState == NULL)
            panic("RecursiveRasterizer", "RecursiveRasterizer", "Error allocating state signal array to the Tile Evaluators.");
    )

    /*  Create staten and command signals to the Tile Evaluators.  */
    for(i = 0; i < numTileEvaluators; i++)
    {
        /*  Create command signal to a Tile Evaluator.  */
        tileEvalComm[i] = newOutputSignal("TileEvaluatorCommand", 1, 1,
            tileEvalPrefixes[i]);

        /*  Create state signal to a Tile Evaluator.  */
        tileEvalState[i] = newInputSignal("TileEvaluatorRastState", 1, 1,
            tileEvalPrefixes[i]);
    }


    /*  Create command signal to Interpolator box.  */
    interpolatorComm = newOutputSignal("InterpolatorCommand", 1, 1, NULL);

    /*  Create state signal from Interpolator box.  */
    interpolatorState = newInputSignal("InterpolatorRasterizerState", 1, 1, NULL);

    /*  Create rasterizer boxes.  */

    /*  Create Triangle Setup box.  */
    triangleSetup = new TriangleSetup(rastEmu, tsFIFOSize,
        "TriangleSetup", this);

    /*  Create Recursive Descent box.  */
    recDescent = new RecursiveDescent(rastEmu, recDescTrFIFOSize,
        numTileEvaluators, tilesCycle, recDescTileStackSize,
        stampFragments, stampsCycle, recDescFrBufferSize, tileEvalPrefixes,
        "RecursiveDescent", this);


    /*  Allocate the array for the Tile Evaluator boxes.  */
    tileEvaluator = new TileEvaluator*[numTileEvaluators];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (tileEvaluator == NULL)
            panic("RecursiveRasterizer", "RecursiveRasterizer", "Error allocating the array of Tile Evaluators.");
    )

    /*  Create Tile Evaluator boxes.  */
    for(i = 0; i < numTileEvaluators; i++)
    {

        /*  Concatenate tile evaluator name prefix.  */
        sprintf(teName,"%sTileEvaluator", tileEvalPrefixes[i]);

        /*  Create tile evaluator box.  */
        tileEvaluator[i] = new TileEvaluator(rastEmu, tilesCycle,
            stampFragments, teInputBufferSize, teOutputBufferSize,
            tileEvalLatency, tileEvalPrefixes[i],
            teName, this);
    }

    /*  Create Interpolator box.  */
    interpolator = new Interpolator(rastEmu, interpolators,
        stampsCycle, stampFragments,
        "Interpolator", this);


    /*  Allocate the Tile Evaluators state array.  */
    teState = new RasterizerState[numTileEvaluators];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (teState == NULL)
            panic("RecursiveRasterizer", "RecursiveRasterizer", "Error allocating Tile Evaluators state array.");
    )

    /*  Initialize the rasterizer state.  */
    state = RAST_RESET;

    /*  Create first command.  */
    lastRastComm = new RasterizerCommand(RSCOM_RESET);
}


/*  Rasterizer simulation function.  */
void RecursiveRasterizer::clock(u64bit cycle)
{
    RasterizerCommand *rastComm;
    RasterizerStateInfo *rastStateInfo;
    RasterizerState tsState;
    RasterizerState rdState;
    RasterizerState intState;
    int i;

    GPU_DEBUG( printf("Rasterizer => Clock %ld\n", cycle); )

    /*  Clock all rasterizer boxes.  */

    /*  Clock Triangle Setup.  */
    triangleSetup->clock(cycle);

    /*  Clock Recursive Descent.  */
    recDescent->clock(cycle);

    /*  Clock the Tile Evaluator boxes.  */
    for(i = 0; i < numTileEvaluators; i++)
        tileEvaluator[i]->clock(cycle);

    /*  Clock Interpolator.  */
    interpolator->clock(cycle);

    /*  Read state from all recursive rasterizer boxes.  */

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
        panic("RecursiveRasterizer", "clock", "Missing signal from Triangle Setup.");
    }

    /*  Get the Recursive Descent state.  */
    if (recDescState->read(cycle, (DynamicObject *&) rastStateInfo))
    {
        /*  Get the Recursive Descent state.  */
        rdState = rastStateInfo->getState();

        /*  Delete rasterizer state info object.  */
        delete rastStateInfo;
    }
    else
    {
        panic("RecursiveRasterizer", "clock", "Missing signal from Recursive Descent.");
    }

    /*  Get state for the Tile Evaluators.  */
    for(i = 0; i < numTileEvaluators; i++)
    {
        /*  Get state from a Tile Evaluator.  */
        if (tileEvalState[i]->read(cycle, (DynamicObject *&) rastStateInfo))
        {
            /*  Get the Tile Evaluator state.  */
            teState[i] = rastStateInfo->getState();

            /*  Delete rasterizer state info object.  */
            delete rastStateInfo;
        }
        else
        {
            panic("RecursiveRasterizer", "clock", "Missing signal from a Tile Evaluator.");
        }
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
        panic("RecursiveRasterizer", "clock", "Missing signal from Interpolator.");
    }

    /*  Simulate current cycle.  */
    switch(state)
    {
        case RAST_RESET:
            /*  The rasterizer is reset state.  */

            GPU_DEBUG( printf("RecursiveRasterizer => State RESET.\n"); )

            /*  Reset RecursiveRasterizer registers.  */
            viewportIniX = 0;
            viewportIniY = 0;
            viewportWidth = 640;
            viewportHeight = 480;
            nearRange = 0.0;
            farRange = 1.0;
            zBufferBitPrecission = 24;
            frustumClipping = TRUE;
            userClipPlanes = FALSE;
            cullMode = BACK;
            interpolation = FALSE;

            /*  Set user clip plane default values.  */
            for(i = 0; i < MAX_USER_CLIP_PLANES; i++)
                userClip[i] = QuadFloat(0.0, 0.0, 0.0, 0.0);

            /*  Set default fragment input attributes active flags.  */
            for(i = 0; i < MAX_FRAGMENT_ATTRIBUTES; i++)
                fragmentAttributes[i] = FALSE;

            /*  Change state to ready. */
            state = RAST_READY;
            break;

        case RAST_READY:
            /*  The rasterizer is ready to receive commands from the
                Command Processor.  */

            GPU_DEBUG( printf("RecursiveRasterizer => State READY.\n"); )

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

            GPU_DEBUG( printf("RecursiveRasterizer => State DRAWING.\n"); )

            /*  Check if the recursive rasterizer units has finished the current
                batch.  */

            if ((tsState == RAST_END) && (rdState == RAST_END) &&
                (intState == RAST_END))
            {
                /*  Change to END state.  */
                state = RAST_END;
            }

            break;

        case RAST_END:
            /*  The rasterizer has finished drawing all the primitives
                in the current batch.  Waiting for response from the
                Command Processor.  */

            GPU_DEBUG( printf("RecursiveRasterizer => State RAST_END.\n"); )

            /*  Check if there is an available rasterizer command from the
                Command Processor.  */
            if (rastCommSignal->read(cycle, (DynamicObject *&) rastComm))
            {
                /*  Process the command.  */
                processRasterizerCommand(rastComm, cycle);
            }

            break;

    }

    /*  Write state signal to the Command Processor.  */
    rastStateSignal->write(cycle, new RasterizerStateInfo(state));
}


/*  Process the rasterizer command.  */
void RecursiveRasterizer::processRasterizerCommand(RasterizerCommand *rastComm,
    u64bit cycle)
{
    RasterizerCommand *rastAuxComm;
    int i;

    /*  Delete previous rasterizer command.  */
    delete lastRastComm;

    /*  Set new command as last command.  */
    lastRastComm = rastComm;

    /*  Process command.  */
    switch(rastComm->getCommand())
    {
        case RSCOM_RESET:
            /*  Reset the recursive rasterizer.  */

            GPU_DEBUG( printf("RecursiveRasterizer => RESET Command.\n"); )

            /*  Send reset signal to all rasterizer boxes.  */

            /*  Send reset command to Triangle Setup.  */
            rastAuxComm = new RasterizerCommand(RSCOM_RESET);

            /*  Copy cookies from original command.  */
            rastAuxComm->copyParentCookies(*rastComm);

            /*  Send command to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastAuxComm);


            /*  Send reset command to Recursive Descent.  */
            rastAuxComm = new RasterizerCommand(RSCOM_RESET);

            /*  Copy cookies from original command.  */
            rastAuxComm->copyParentCookies(*rastComm);

            /*  Send command to Recursive Descent.  */
            recDescComm->write(cycle, rastAuxComm);


            /*  Send reset commands to the Tile Evaluators.  */
            for (i = 0; i < numTileEvaluators; i++)
            {
                /*  Create reset command for a Tile Evaluator.  */
                rastAuxComm = new RasterizerCommand(RSCOM_RESET);

                /*  Copy cookies from original command.  */
                rastAuxComm->copyParentCookies(*rastComm);

                /*  Send command to a Tile Evaluator.  */
                tileEvalComm[i]->write(cycle, rastAuxComm);
            }


            /*  Send reset command to Interpolator.  */
            rastAuxComm = new RasterizerCommand(RSCOM_RESET);

            /*  Copy cookies from original command.  */
            rastAuxComm->copyParentCookies(*rastComm);

            /*  Send command to Interpolator.  */
            interpolatorComm->write(cycle, rastAuxComm);


            /*  Change state to reset.  */
            state = RAST_RESET;

            break;

        case RSCOM_REG_WRITE:
            /*  Write to a Recursive Rasterizer register.  */

            /*  Check that current state is RAST_READY.  */
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("RecursiveRasterizer", "processRasterizerCommand", "Rasterizer register writes can only be received in READY state.");
            )

            GPU_DEBUG( printf("RecursiveRasterizer => REG_WRITE Command.\n"); )

            /*  Process the rasterizer register write.  */
            processRegisterWrite(rastComm->getRegister(),
                rastComm->getSubRegister(), rastComm->getRegisterData(), cycle);

            break;

        case RSCOM_REG_READ:
            panic("RecursiveRasterizer", "processRasterizerCommand", "Register read not supported.");
            break;

        case RSCOM_DRAW:
            /*  Start drawing primitives.  */

            GPU_DEBUG( printf("RecursiveRasterizer => DRAW Command.\n"); )

            /*  Check the current state.  */
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("RecursiveRasterizer", "processRasterizerCommand", "Rasterizer DRAW command can only be received in READY state.");
           )

            /*  Send DRAW command to Triangle Setup.  */
            rastAuxComm = new RasterizerCommand(RSCOM_DRAW);

            /*  Copy cookies from original command.  */
            rastAuxComm->copyParentCookies(*rastComm);

            /*  Send command to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastAuxComm);


            /*  Send DRAW command to Recursive Descent.  */
            rastAuxComm = new RasterizerCommand(RSCOM_DRAW);

            /*  Copy cookies from original command.  */
            rastAuxComm->copyParentCookies(*rastComm);

            /*  Send command to Recursive Descent.  */
            recDescComm->write(cycle, rastAuxComm);

            /*  Send DRAW commants to the Tile Evaluators.  */
            for(i = 0; i < numTileEvaluators; i++)
            {
                /*  Create DRAW command to a Tile Evaluator.  */
                rastAuxComm = new RasterizerCommand(RSCOM_DRAW);

                /*  Copy cookies from original command.  */
                rastAuxComm->copyParentCookies(*rastComm);

                /*  Send command to a Tile Evaluator.  */
                tileEvalComm[i]->write(cycle, rastAuxComm);
            }

            /*  Send DRAW command to Interpolator.  */
            rastAuxComm = new RasterizerCommand(RSCOM_DRAW);

            /*  Copy cookies from original command.  */
            rastAuxComm->copyParentCookies(*rastComm);

            /*  Send command to Interpolator.  */
            interpolatorComm->write(cycle, rastAuxComm);


            /*  Change state to batch drawing.  */
            state = RAST_DRAWING;

            break;

        case RSCOM_END:
            /*  End drawing primitives.  */

            /*  Check current state.  */
            GPU_ASSERT(
                if (state != RAST_END)
                    panic("RecursiveRasterizer", "processRasterizerCommand", "Rasterizer END command can only be received in END state.");
            )

            GPU_DEBUG( printf("RecursiveRasterizer => END Command.\n"); )

            /*  Send END command to Triangle Setup.  */
           rastAuxComm = new RasterizerCommand(RSCOM_END);

            /*  Copy cookies from original command.  */
            rastAuxComm->copyParentCookies(*rastComm);

            /*  Send command to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastAuxComm);


            /*  Send END command to Recursive Descent.  */
            rastAuxComm = new RasterizerCommand(RSCOM_END);

            /*  Copy cookies from original command.  */
            rastAuxComm->copyParentCookies(*rastComm);

            /*  Send command to Recursive Descent.  */
            recDescComm->write(cycle, rastAuxComm);


            /*  Send END commants to the Tile Evaluators.  */
            for(i = 0; i < numTileEvaluators; i++)
            {
                /*  Create END command to a Tile Evaluator.  */
                rastAuxComm = new RasterizerCommand(RSCOM_END);

                /*  Copy cookies from original command.  */
                rastAuxComm->copyParentCookies(*rastComm);

                /*  Send command to a Tile Evaluator.  */
                tileEvalComm[i]->write(cycle, rastAuxComm);
            }

            /*  Send END command to Interpolator.  */
            rastAuxComm = new RasterizerCommand(RSCOM_END);

            /*  Copy cookies from original command.  */
            rastAuxComm->copyParentCookies(*rastComm);

            /*  Send command to Interpolator.  */
            interpolatorComm->write(cycle, rastAuxComm);


            /*  Change state to ready.  */
            state = RAST_READY;

            break;

        default:

            panic("RecursiveRasterizer", "processRasterizerCommand", "Unsuppored Rasterizer Command.");

            break;
    }

}

/*  Process the register write.  */
void RecursiveRasterizer::processRegisterWrite(GPURegister reg, u32bit subreg,
    GPURegData data, u64bit cycle)
{
    RasterizerCommand *rastComm;

    /*  Process the register.  */
    switch(reg)
    {
        case GPU_VIEWPORT_INI_X:
            /*  Write the viewport initial x coordinate.  */
            viewportIniX = data.intVal;

            GPU_DEBUG( printf("RecursiveRasterizer => GPU_VIEWPORT_INI_X = %d.\n", data.intVal); )


            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);

            break;

        case GPU_VIEWPORT_INI_Y:
            /*  Write the viewport initial y coordinate.  */
            viewportIniY = data.intVal;

            GPU_DEBUG( printf("RecursiveRasterizer => GPU_VIEWPORT_INI_Y = %d.\n", data.intVal); )

            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);

            break;

        case GPU_VIEWPORT_HEIGHT:
            /*  Write the viewport height register.  */
            viewportHeight = data.uintVal;

            GPU_DEBUG( printf("RecursiveRasterizer => GPU_VIEWPORT_HEIGHT = %d.\n", data.uintVal); )

            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);

            break;

        case GPU_VIEWPORT_WIDTH:
            /*  Write the viewport width register.  */
            viewportWidth = data.uintVal;

            GPU_DEBUG( printf("RecursiveRasterizer => GPU_VIEWPORT_WIDTH = %d.\n", data.uintVal); )

            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);

            break;

        case GPU_DEPTH_RANGE_NEAR:
            /*  Write the near depth range register.  */
            nearRange = data.f32Val;

            GPU_DEBUG( printf("RecursiveRasterizer => GPU_DEPTH_RANGE_NEAR = %f.\n", data.f32Val); )

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

            GPU_DEBUG( printf("RecursiveRasterizer => GPU_DEPTH_RANGE_FAR = %f.\n", data.f32Val); )

            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);

            break;

        case GPU_Z_BUFFER_BIT_PRECISSION:
            /*  Write the z buffer bit precisison register.  */
            zBufferBitPrecission = data.uintVal;

            GPU_DEBUG( printf("RecursiveRasterizer => GPU_Z_BUFFER_BIT_PRECISSION = %d.\n", data.uintVal); )

            /*  Send register write to Triangle Setup.  */

            /*  Create register write command.  */
            rastComm = new RasterizerCommand(reg, subreg, data);

            /*  Copy cookies from last command.  */
            rastComm->copyParentCookies(*lastRastComm);

            /*  Send register write to Triangle Setup.  */
            triangleSetupComm->write(cycle, rastComm);

            break;

        case GPU_USER_CLIP:
            /*  Write an user clip plane register.  */
            userClip[subreg][0] = data.qfVal[0];
            userClip[subreg][1] = data.qfVal[1];
            userClip[subreg][2] = data.qfVal[2];
            userClip[subreg][3] = data.qfVal[3];

            GPU_DEBUG(
                printf("RecursiveRasterizer => GPU_USER_CLIP(%d) = (%f, %f, %f, %f).\n",
                    subreg, data.qfVal[0], data.qfVal[1], data.qfVal[2], data.qfVal[3]);
            )

            break;

        case GPU_USER_CLIP_PLANE:
            /*  Write user clip mode enable register.  */
            userClipPlanes = data.booleanVal;

            GPU_DEBUG(
                printf("RecursiveRasterizer => GPU_USER_CLIP_PLANE = %s.\n",
                    data.booleanVal?"TRUE":"FALSE");
            )

            break;

        case GPU_CULLING:
            /*  Write the culling mode register.  */
            cullMode = data.culling;

            GPU_DEBUG(
                printf("RecursiveRasterizer => GPU_CULLING = ");
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

        case GPU_INTERPOLATION:
            /*  Write the interpolation mode enable register.  */
            interpolation = data.booleanVal;

            GPU_DEBUG(
                printf("RecursiveRasterizer => GPU_INTERPOLATION = %s.\n",
                    data.booleanVal?"TRUE":"FALSE");
            )

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
                    panic("RecursiveRasterizer", "processRegisterWrite", "Out of range fragment attribute identifier.");
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


            break;

        case GPU_FRUSTUM_CLIPPING:
            /*  Write the frustum clipping flag register.  */
            frustumClipping = data.booleanVal;

            GPU_DEBUG( printf("RecursiveRasterizer => GPU_FRUSTUM_CLIPPING = %s.\n",
                data.booleanVal?"ENABLED":"DISABLED"); )

            break;

        default:
            /*  Unsupported register.  */
            panic("RecursiveRasterizer", "processRegisterWrite", "Unsupported Rasterizer register.");
            break;
    }
}
