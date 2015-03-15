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
 * $RCSfile: Interpolator.cpp,v $
 * $Revision: 1.19 $
 * $Author: vmoya $
 * $Date: 2006-12-12 17:41:16 $
 *
 * Interpolator box implementation file.
 *
 */

 /**
 *
 *  @file Interpolator.cpp
 *
 *  This file implements the Interpolator box.
 *
 *  The Interpolator box implements the simulation
 *  of the fragment attribute interpolators in a GPU.
 *
 */

#include "Interpolator.h"
#include "FragmentInput.h"
#include "RasterizerStateInfo.h"
#include <cmath>

using namespace gpu3d;
using namespace std;

/*  Interpolator box constructor.  */
Interpolator::Interpolator(RasterizerEmulator &rastEmu, u32bit interp,
    u32bit stampsPerCycle, u32bit stampUnits, char *name, Box *parent) :

    rastEmu(rastEmu), interpolators(interp), stampsCycle(stampsPerCycle), numStampUnits(stampUnits),
    Box(name, parent)

{
    DynamicObject *defaultState[1];

    /*  Create the signals.  */

    /*  Create command signal from the main rasterizer box.  */
    interpolatorCommand = newInputSignal("InterpolatorCommand", 1, 1, NULL);

    /*  Create state signal to the main rasterizer box.  */
    interpolatorRastState = newOutputSignal("InterpolatorRasterizerState", 1, 1, NULL);

    /*  Create default signal value.  */
    defaultState[0] = new RasterizerStateInfo(RAST_RESET);

    /*  Set default signal value.  */
    interpolatorRastState->setData(defaultState);

    /*  Create input fragment signal from Fragment FIFO.  */
    newFragment = newInputSignal("InterpolatorInput", stampsCycle * STAMP_FRAGMENTS, 1, NULL);

    /*  Create interpolation start signal.  */
    interpolationStart = newOutputSignal("Interpolation",
        stampsCycle * STAMP_FRAGMENTS, MAX_INTERPOLATION_LATENCY, NULL);

    /*  Create interpolation end signal.  */
    interpolationEnd = newInputSignal("Interpolation",
        stampsCycle * STAMP_FRAGMENTS, MAX_INTERPOLATION_LATENCY, NULL);

    /*  Create interpolated fragment output signal to Fragment FIFO.  */
    interpolatorOutput = newOutputSignal("InterpolatorOutput", stampsCycle * STAMP_FRAGMENTS, 1, NULL);

    /*  Create buffers.  */

    /*  Reset state.  */
    state = RAST_RESET;

    /*  Set a fake last command.  */
    lastRSCommand = new RasterizerCommand(RSCOM_RESET);
}

/*  Interpolator simulation function.  */
void Interpolator::clock(u64bit cycle)
{
    FragmentInput *frInput;
    QuadFloat *attributes;
    RasterizerCommand *rastCommand;
    bool fragmentReceived;
    bool lastFragment;
    u32bit i;
    u32bit j;

    GPU_DEBUG_BOX(
        printf("Interpolator => clock %lld.\n", cycle);
    )

    /*  Simulate current cycle.  */
    switch(state)
    {
        case RAST_RESET:
            /*  Reset state.  */

            GPU_DEBUG_BOX(
                printf("Interpolator => RESET state.\n");
            )

            /*  Reset interpolator state.  */

            /*  Reset depth buffer precission.  */
            depthBitPrecission = 24;

            /*  Reset fragment attributes interpolation and active flag.  */
            for(i = 0; i < MAX_FRAGMENT_ATTRIBUTES; i++)
            {
                /*  Set all fragment attributes as interpolated.  */
                interpolation[i] = TRUE;

                /*  Set all fragment attributes as not active.  */
                fragmentAttributes[i] = FALSE;
            }

            /*  Change to READY state.  */
            state = RAST_READY;

            break;

        case RAST_READY:
            /*  Ready state.  */

            GPU_DEBUG_BOX(
                printf("Interpolator => READY state.\n");
            )

            /*  Receive and process command from the main Rasterizer box.  */
            if (interpolatorCommand->read(cycle, (DynamicObject *&) rastCommand))
                processCommand(rastCommand);

            break;

        case RAST_DRAWING:
            /*  Draw state.  */

            GPU_DEBUG_BOX(
                printf("Interpolator => DRAWING state.\n");
            )

            /*  Check if last fragment interpolation was finished.  */
            if (remainingCycles > 0)
            {
                GPU_DEBUG_BOX(
                    printf("Interpolator => Interpolating fragment attributes.  Cycles remaining %d.\n",
                        remainingCycles);
                )

                /*  Update cycle counter.  */
                remainingCycles--;
            }
            else
            {
                /*  No fragment received.  */
                fragmentReceived = FALSE;

                /*  Receive stamps from Hierarchical Z.  */
                for(i = 0; i < stampsCycle; i++)
                {
                    /*  No stamp fragment received yet.  */
                    fragmentReceived = TRUE;

                    /*  Receive fragments from a stamp.  */
                    for(j = 0; (j < STAMP_FRAGMENTS) && fragmentReceived; j++)
                    {
                        if (newFragment->read(cycle, (DynamicObject *&) frInput))
                        {
                            GPU_DEBUG_BOX(
                                printf("Interpolator => Received new fragment to interpolate.\n");
                            )

                            /*  Now we have received a fragment.  */
                            fragmentReceived = TRUE;

                            /*  Send fragment through the interpolation signal.  */
                            interpolationStart->write(cycle, frInput,
                                cyclesFragment + INTERPOLATION_LATENCY);

                        }
                        else
                        {
                            /*  Check if only a part of the stamp was received.  */
                            GPU_ASSERT(
                                if (j > 0)
                                {
                                    /*  The number of fragments per cycle is fixed.  */
                                    panic("Interpolator", "clock", "Missing fragments in stamp.");
                                }
                            )

                            /*  No fragment received.  */
                            fragmentReceived = FALSE;
                        }
                    }

                    /*  If fragments have been received.  */
                    if (fragmentReceived)
                    {
                        /*  Set cycles until next interpolation can be started.  */
                        remainingCycles = cyclesFragment - 1;
                    }
                }
            }

            /*  No last fragment processed.  */
            lastFragment = FALSE;

            /*  Interpolate fragments.  */
            while(interpolationEnd->read(cycle, (DynamicObject *&) frInput))
            {

                /*  NOTE: !!! THIS WAY OF COUNTING THE TRIANGLES RECEIVED
                    ONLY WORKS IF NO FRAGMENTS FROM DIFFERENT TRIANGLES
                    ARE ISSUED AT THE SAME TIME!!!
                    THE CORRECT IMPLEMENTATION WOULD USE A MESSAGE FROM
                    PRIMITIVE ASSEMBLY.

                    NOT USED ANY MORE FOR SYNCHRONIZATION.  CAN BE DELETED
                    OR BE USED TO COUNT THE NUMBER OF TRIANGLES PASSING
                    THROUGH THE INTERPOLATOR.

                  */

                /*  Check if it is a fragment from a new triangle.  */
                if(frInput->getTriangleID() != currentTriangle)
                {

                    /*  Change current triangle identifier.  */
                    currentTriangle = frInput->getTriangleID();

                    /*  Set if it is the last triangle.  */
                    lastTriangle = (frInput->getFragment() == NULL);

                    /*  Change first triangle received flag.  */
                    firstTriangle = TRUE;

                    /*  Change current setup triangle identifier.  */
                    currentSetupTriangle = frInput->getSetupTriangle();

                    /*  Update processed triangle counter.  */
                    triangleCounter++;
                }

                /*  Check if a culled fragment was received.  */
                if (frInput->getFragment() != NULL)
                {
                    /*  Allocate an array for the fragment attributes.  */
                    attributes = new QuadFloat[MAX_FRAGMENT_ATTRIBUTES];

                    /*  Interpolate fragment attributes.  */
                    for(i = 0; i < MAX_FRAGMENT_ATTRIBUTES; i++)
                    {
                        /*  Check if the attribute is active.  */
                        if (fragmentAttributes[i])
                        {
                            /*  Check if interpolation is enabled.  */
                            if (interpolation[i])
                            {
                                GPU_DEBUG_BOX(
                                    printf("Interpolator => Interpolating fragment.\n");
                                )

                                /*  Interpolate the fragment attributes.  */
                                //rastEmu.interpolate(frInput->getFragment(), attributes);
                                attributes[i] = rastEmu.interpolate(frInput->getFragment(), i);
                            }
                            else
                            {
                                GPU_DEBUG_BOX(
                                    printf("Interpolator => Copying from vertex attributes.\n");
                                )

                                /*  Copy fragment attribute from the last (third) triangle vertex.  */
                                //rastEmu.copy(frInput->getFragment(), attributes, 0);
                                attributes[i] = rastEmu.copy(frInput->getFragment(), i, 2);
                            }
                        }
                        else
                        {
                            /*  Write default attribute value for non active attribute.  */
                            DEFAULT_FRAGMENT_ATTRIBUTE(attributes[i]);
                        }
                    }

                    /*  Position attribute is a special case.  */
                    attributes[POSITION_ATTRIBUTE][0] = (f32bit) frInput->getFragment()->getX();
                    attributes[POSITION_ATTRIBUTE][1] = (f32bit) frInput->getFragment()->getY();

                    /*  NOTE: !!! POSITION Z AND 1/W MUST BE CALCULATED IN
                        A DIFFERENT WAY !!!
                        NOT IMPLEMENTED YET.
                    */

                    /*  Fragment depth must be scaled between 0 and 1.  */
                    attributes[POSITION_ATTRIBUTE][2] = ((f32bit) frInput->getFragment()->getZ()) /
                        ((f32bit) ((1 << depthBitPrecission) - 1));

                    attributes[POSITION_ATTRIBUTE][3] = 1.0;
                    
                    //  Triangle face/area attribute is another special case.
                    attributes[FACE_ATTRIBUTE][3] = frInput->getFragment()->getTriangle()->getArea();

                    GPU_DEBUG_BOX(
                        printf("Interpolator => Interpolated fragment attributes:\n");
                        for(i = 0; i < MAX_FRAGMENT_ATTRIBUTES; i++)
                            if (fragmentAttributes[i])
                                printf("i[%d] = { %f, %f, %f, %f }\n", i, attributes[i][0], attributes[i][1],
                                    attributes[i][2], attributes[i][3]);
                    )

                    /*  Set fragment attributes.  */
                    frInput->setAttributes(attributes);
                }
                else
                {
                    /*  Empty fragment (last fragment).  No attributes.  */
                    frInput->setAttributes(NULL);
                }


                /*  Send fragment back to Fragment FIFO unit.  */
                interpolatorOutput->write(cycle, frInput);

                GPU_DEBUG_BOX(
                    printf("Interpolator => Sending interpolated fragment to Fragment FIFO.\n");
                )

                /*  Check if it was the last fragment in the triangle.  */
                if (frInput->getFragment() == NULL)
                {
                    /*  Updatet the number of last fragments received.  */
                    lastFragments++;

                    /*  Set if all the last fragments received.  */
                    lastFragment = (lastFragments == (numStampUnits * STAMP_FRAGMENTS));
                }

                /*  Update processed fragments counter.  */
                fragmentCounter++;

            }

            /*  Check end of the batch.  */
            if (lastFragment)
            {
                /*  Change state to end.  */
                state = RAST_END;
            }

            break;

        case RAST_END:
            /*  Draw end state.  */

            GPU_DEBUG_BOX(
                if (state == RAST_END)
                    printf("Interpolator => END state.\n");
            )


            /*  Wait for END command from the Rasterizer main box.  */
            if (interpolatorCommand->read(cycle, (DynamicObject *&) rastCommand))
                processCommand(rastCommand);

            break;

        default:
            panic("Interpolator", "clock", "Unsuported rasterizer state.");
            break;

    }

    /*  Send state to the main rasterizer box.  */
    interpolatorRastState->write(cycle, new RasterizerStateInfo(state));
}

/*  Processes a rasterizer command.  */
void Interpolator::processCommand(RasterizerCommand *command)
{
    u32bit activeAttributes;
    int i;

    /*  Delete last command.  */
    delete lastRSCommand;

    /*  Store current command as last received command.  */
    lastRSCommand = command;

    /*  Process command.  */
    switch(command->getCommand())
    {
        case RSCOM_RESET:
            /*  Reset command from the Rasterizer main box.  */

            GPU_DEBUG_BOX(
                printf("Interpolator => RESET command received.\n");
            )

            /*  Change to reset state.  */
            state = RAST_RESET;

            break;

        case RSCOM_DRAW:
            /*  Draw command from the Rasterizer main box.  */

            GPU_DEBUG_BOX(
                printf("Interpolator => DRAW command received.\n");
            )

            /*  Check current state.  */
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("Interpolator", "processRegisterWrite", "DRAW command can only be received in READY state.");
            )

            /*  Reset triangle counter.  */
            triangleCounter = 0;

            /*  Reset batch fragment counter.  */
            fragmentCounter = 0;

            /*  No fragment to interpolate yet.  */
            remainingCycles = 0;

            /*  Reset previous processed triangle.  */
            currentTriangle = (u32bit) -1;

            /*  Reset current setup triangle identifier.  */
            currentSetupTriangle = (u32bit) -1;

            /*  Reset number of received last fragments.  */
            lastFragments = 0;

            /*  Set as last triangle flag to false.  */
            lastTriangle = FALSE;

            /*  Set first triangle received flag as false.  */
            firstTriangle = FALSE;

            /*  Count the number of active interpolators.  */
            for(i = 0, activeAttributes = 0; i < MAX_FRAGMENT_ATTRIBUTES; i++)
                if (fragmentAttributes[i])
                    activeAttributes++;

            /*  Calculate the number of cycles takes to
                interpolate a fragment with the hardware interpolators
                available.  */
            cyclesFragment = (u32bit) ceil((double) activeAttributes / (double) interpolators);

            /*  Change to drawing state.  */
            state = RAST_DRAWING;

            break;

        case RSCOM_END:
            /*  End command received from Rasterizer main box.  */

            GPU_DEBUG_BOX(
                printf("Interpolator => END command received.\n");
            )

            /*  Check current state.  */
            GPU_ASSERT(
                if (state != RAST_END)
                    panic("Interpolator", "processCommand", "END command can only be received in END state.");
            )

            /*  Change to ready state.  */
            state = RAST_READY;

            break;

        case RSCOM_REG_WRITE:
            /*  Write register command from the Rasterizer main box.  */

            GPU_DEBUG_BOX(
                printf("Interpolator => REG_WRITE command received.\n");
            )


            /*  Check current state.  */
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("Interpolator", "processCommand", "REGISTER WRITE command can only be received in READY state.");
            )

            /*  Process register write.  */
            processRegisterWrite(command->getRegister(),
                command->getSubRegister(), command->getRegisterData());

            break;

        default:
            panic("Interpolator", "proccessCommand", "Unsupported command.");
            break;
    }
}

void Interpolator::processRegisterWrite(GPURegister reg, u32bit subreg,
    GPURegData data)
{
    /*  Process register write.  */
    switch(reg)
    {
        case GPU_Z_BUFFER_BIT_PRECISSION:
            /*  Write depth buffer bit precission.  */
            depthBitPrecission = data.uintVal;

            GPU_DEBUG_BOX(
                printf("Interpolator => Write GPU_Z_BUFFER_BIT_PRECISSION = %d.\n",
                    depthBitPrecission);
            )

            break;

        case GPU_INTERPOLATION:
            /*  Write interpolation flag register.  */

            /*  Check fragment attribute identifier range.  */
            GPU_ASSERT(
                if (subreg >= MAX_FRAGMENT_ATTRIBUTES)
                    panic("Interpolator", "processRegisterWrite", "Fragment attribute identifier out of range.");
            )

            interpolation[subreg] = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("Interpolator => Write GPU_INTERPOLATION = %s.\n",
                    interpolation?"ENABLED":"DISABLED");
            )

            break;

        case GPU_FRAGMENT_INPUT_ATTRIBUTES:
            /*  Write the active fragment attributes register.  */

            /*  Check fragment attribute identifier range.  */
            GPU_ASSERT(
                if (subreg >= MAX_FRAGMENT_ATTRIBUTES)
                    panic("Interpolator", "processRegisterWrite", "Fragment attribute identifier out of range.");
            )

            /*  Write the fragment attribute active flag.  */
            fragmentAttributes[subreg] = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("Interpolator => Write GPU_FRAGMENT_INPUT_ATTRIBUTES[%d] = %s.\n",
                    subreg, data.booleanVal?"ACTIVE":"INACTIVE");
            )

            break;

        default:
            panic("Interpolator", "processRegisterWrite", "Unsupported register.");
            break;
    }

}
