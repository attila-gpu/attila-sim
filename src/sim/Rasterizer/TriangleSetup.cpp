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
 * $RCSfile: TriangleSetup.cpp,v $
 * $Revision: 1.24 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:53 $
 *
 * Triangle Setup box implementation file.
 *
 */

 /**
 *
 *  @file TriangleSetup.cpp
 *
 *  This file implements the Triangle Setup box.
 *
 *
 */

#include "TriangleSetup.h"
#include "RasterizerStateInfo.h"
#include "PrimitiveAssemblyRequest.h"
#include "TriangleSetupRequest.h"
#include "ShaderInput.h"
#include "ShaderStateInfo.h"
#include "support.h"
#include <cstdio>
#include <cmath>
#include <sstream>

using namespace gpu3d;
using namespace std;

/*  Triangle Setup box constructor.  */
TriangleSetup::TriangleSetup(RasterizerEmulator &rasEmu, u32bit cycleTriangles, u32bit tsFIFOSize,
    u32bit setupUnits, u32bit latency, u32bit startLat, u32bit paLat, u32bit tbLat, u32bit fgLat, 
    bool shadedSetup, bool preTriangleBound, bool microTrisAsFrag, u32bit triShQSz, char *name, 
    Box *parent) :

    trianglesCycle(cycleTriangles), numSetupUnits(setupUnits), setupLatency(latency),
    setupStartLatency(startLat), setupFIFOSize(tsFIFOSize), paLatency(paLat), tbLatency(tbLat), 
    fgLatency(fgLat), shaderSetup(shadedSetup), preTriangleBound(preTriangleBound),  
    triangleShaderQSz(triShQSz), rastEmu(rasEmu), Box(name, parent)

{
    DynamicObject *defaultState[1];

    /*  Check parameters. */
    GPU_ASSERT(
        if (trianglesCycle != numSetupUnits)
            panic("TriangleSetup", "TriangleSetup", "Number of setup units must be the same that the triangles per cycle in current implementation.");
        if (setupFIFOSize <= ((paLatency + 1) * trianglesCycle))
            panic("TriangleSetup", "TriangleSetup", "Triangle setup fifo too small for triangles input througput and latency.");
        if (shaderSetup && (triangleShaderQSz == 0))
            panic("TriangleSetup", "TriangleSetup", "Triangle shader queue requires at least 1 entry.");
    )

    /*  Create statistics.  */
    inputs = &getSM().getNumericStatistic("InputTriangles", u32bit(0), "TriangleSetup", "TS");
    outputs = &getSM().getNumericStatistic("OutputTriangles", u32bit(0), "TriangleSetup", "TS");
    requests = &getSM().getNumericStatistic("RequestedTriangles", u32bit(0), "TriangleSetup", "TS");
    culled = &getSM().getNumericStatistic("CulledTriangles", u32bit(0), "TriangleSetup", "TS");
    front = &getSM().getNumericStatistic("FrontFacingTriangles", u32bit(0), "TriangleSetup", "TS");
    back = &getSM().getNumericStatistic("BackFacingTriangles", u32bit(0), "TriangleSetup", "TS");

    /*  Create triangle size related statistics.  */
    count_less_1 = &getSM().getNumericStatistic("TrianglesLess_1", u32bit(0), "TriangleSetup", "TS");
    count_1_to_4 = &getSM().getNumericStatistic("Triangles1_to_4", u32bit(0), "TriangleSetup", "TS");
    count_4_to_16 = &getSM().getNumericStatistic("Triangles4_to_16", u32bit(0), "TriangleSetup", "TS");
    count_16_to_100 = &getSM().getNumericStatistic("Triangles16_to_100", u32bit(0), "TriangleSetup", "TS");
    count_100_to_1000 = &getSM().getNumericStatistic("Triangles100_to_1000", u32bit(0), "TriangleSetup", "TS");
    count_1000_to_screen = &getSM().getNumericStatistic("Triangles1000_to_Screen", u32bit(0), "TriangleSetup", "TS");
    count_greater_screen = &getSM().getNumericStatistic("TrianglesGreaterScreen", u32bit(0), "TriangleSetup", "TS");
    count_overlap_1x1 = &getSM().getNumericStatistic("TrianglesOverlap_1x1", u32bit(0), "TriangleSetup", "TS");
    count_overlap_2x2 = &getSM().getNumericStatistic("TrianglesOverlap_2x2", u32bit(0), "TriangleSetup", "TS");

    /*  Create command signal from the main Rasterizer box.  */
    setupCommand = newInputSignal("TriangleSetupCommand", 1, 1, NULL);

    /*  Create state signal to the main Rasterizer state.  */
    rastSetupState = newOutputSignal("TriangleSetupRasterizerState", 1, 1, NULL);

    /*  Create default signal value.  */
    defaultState[0] = new RasterizerStateInfo(RAST_RESET);

    /*  Set default signal value.  */
    rastSetupState->setData(defaultState);

    if (!preTriangleBound)
    {
        /*  Create new triangle signal from Primitive Assembly/Clipper.  */
        inputTriangle = newInputSignal("RasterizerNewTriangle", trianglesCycle, paLatency, NULL);

        /*  Create request signal to the Clipper.  */
        triangleRequest = newOutputSignal("ClipperRasterizerRequest", 1, 1, NULL);
    }
    else
    {
        /*  Create new triangle signal from Triangle Bound Unit.  */
        inputTriangle = newInputSignal("TriangleBoundNewTriangle", trianglesCycle, tbLatency, NULL);

        /*  Create request signal to the TriangleBound Unit.  */
        triangleRequest = newOutputSignal("TriangleBoundRequest", 1, 1, NULL);
    }

    /*  Create output signal to the Triangle Traversal/Fragment Generator.  */
    setupOutput = newOutputSignal("TriangleSetupOutput", trianglesCycle, fgLatency, NULL);

    /*  Create request signal from the Triangle Traversal/Fragment Generator.  */
    setupRequest = newInputSignal("TriangleSetupRequest", 1, 1, NULL);

    /*  Create triangle setup signals.  */

    /*  Check if setup is performed in the shader or in a specific unit.  */
    if (shaderSetup)
    {
        /*  Create triangle input signal with the unified shaders (FragmentFIFO).  */
        setupShaderInput = newOutputSignal("TriangleShaderInput", numSetupUnits, setupLatency, NULL);

        /*  Create triangle output signal from the unified shaders (FragmentFIFO).  */
        setupShaderOutput = newInputSignal("TriangleShaderOutput", numSetupUnits, setupLatency, NULL);

        /*  Create state signal from the unified shaders (FragmentFIFO).  */
        setupShaderState = newInputSignal("TriangleShaderState", 1, 1, NULL);
    }
    else
    {
        /*  Create setup start signal to the Triangle Setup box.  */
        setupStart = newOutputSignal("TriangleSetup", numSetupUnits, setupLatency, NULL);

        /*  Create setup ed signal from the Triangle Setup box.  */
        setupEnd = newInputSignal("TriangleSetup", numSetupUnits, setupLatency, NULL);
    }


    /*  Check if triangle setup is performed in the unified shaders.  */
    if (shaderSetup)
    {
        /*  Allocate reorder queue for triangle shader results.  */
        triangleShaderQueue = new TriangleShaderInput[triangleShaderQSz];

        /*  Check memory allocation.  */
        GPU_ASSERT(
            if (triangleShaderQueue == NULL)
                panic("TriangleSetup", "TriangleSetup", "Error allocating triangle shader reorder queue.");
        )
    }

    /*  Allocate memory for the setup FIFO.  */
    setupFIFO = new TriangleSetupOutput*[setupFIFOSize];

    /*  Check memory allocation.  */
    GPU_ASSERT(
        if(setupFIFO == NULL)
            panic("TriangleSetup", "TriangleSetup", "Error allocating memory for the setup FIFO.");
    )

    /*  Create a dump last rasterizer command.  */
    lastRSCommand = new RasterizerCommand(RSCOM_RESET);

    /*  Set initial state to reset.  */
    state = RAST_RESET;
}

/*  Triangle setup simulation rutine.  */
void TriangleSetup::clock(u64bit cycle)
{
    RasterizerCommand *rastCommand;
    TriangleSetupRequest *tsRequest;
    TriangleSetupInput *tsInput;
    ShaderInput *shInput;
    ShaderStateInfo *shStateInfo;
    ShaderState shState;
    QuadFloat *shAttributes;
    QuadFloat *vertAttr1;
    QuadFloat *vertAttr2;
    QuadFloat *vertAttr3;
    u32bit triangleID;
    u32bit triRequest;
    u32bit shOutputTriangle;
    u32bit i;

    GPU_DEBUG_BOX(
        printf("TriangleSetup => clock %lld\n", cycle);
    )

    /*  Receive setup triangle request.  */
    if (setupRequest->read(cycle, (DynamicObject *&) tsRequest))
    {
        /*  A new setup triangle has been requested.  */
        trianglesRequested += tsRequest->getRequest();

        GPU_DEBUG_BOX( printf("TriangleSetup => Requested setup triangle.\n"); )

        /*  Delete Setup Triangle Request object.  */
        delete tsRequest;

        /*  Update statistics.  */
        requests->inc();
    }

    /*  Check if triangle setup in the shaders is enabled.  */
    if (shaderSetup)
    {
        /*  Receive state from the shaders (FragmentFIFO).  */
        if (setupShaderState->read(cycle, (DynamicObject *&) shStateInfo))
        {
            /*  Get current shaders state (FragmentFIFO).  */
            shState = shStateInfo->getState();

            /*  Delete shader state info object.  */
            delete shStateInfo;
        }
    }

    /*  Process current state.  */
    switch(state)
    {
        case RAST_RESET:
            /*  Reset state.  */

            GPU_DEBUG_BOX(
                printf("TriangleSetup => RESET state.\n");
            )

            /*  Reset the triangle setup internal state.  */

            /*  Reset Triangle setup register.  */
            hRes = 400;
            vRes = 400;
            d3d9PixelCoordinates = false;
            iniX = 0;
            iniY = 0;
            width = 400;
            height = 400;
            scissorTest = false;
            scissorIniX = 0;
            scissorIniY = 0;
            scissorWidth = 400;
            scissorHeight = 400;
            near = 0.0;
            far = 1.0;
            d3d9DepthRange = false;
            slopeFactor = 0.0f;
            unitOffset = 0.0f;
            zBitPrecission = 24;
            faceMode = GPU_CCW;
            d3d9RasterizationRules = false;
            twoSidedLighting = FALSE;
            //culling = BACK;
            culling = NONE;

            /*  Calculate minimun triangle area (pixel area?) from
                the viewport.  Viewport in clip coordinates sistem
                has 2.0 x 2.0 size.

                NOTE: !!! SHOULD BE ADJUSTED TO TAKE INTO ACCOUNT
                THE APPROXIMATION TO THE TRIANGLE AREA USED !!!

                (Precalculated).
            */

            /* !!! NOTE: AREA CULLING DISABLED AS IT DOESN'T TAKE INTO
               SMALL LARGE TRIANGLES.!!!  */

            //minTriangleArea = 0.000025;

            /*  Reset next setup wait cycles counter.  */
            setupWait = 0;

            /*  Change to ready state.  */
            state = RAST_READY;

            break;

        case RAST_READY:
            /*  Ready state.  */

            GPU_DEBUG_BOX(
                printf("TriangleSetup => READY state.\n");
            )

            /*  Receive and process command from the main Rasterizer box.  */
            if (setupCommand->read(cycle, (DynamicObject *&) rastCommand))
                processCommand(rastCommand);

            break;

        case RAST_DRAWING:
            /*  Draw state.  */

            GPU_DEBUG_BOX(
                printf("TriangleSetup => DRAWING state.\n");
            )

            /*  Check if triangle setup in the shader is enabled.  */
            if (!shaderSetup)
            {
                /*  Check if there is an empty setup FIFO entry.  */
                if (!lastTriangle && ((setupTriangles + setupReserves) < setupFIFOSize))
                {
                    /*  Calculate number of triangles to request to the Clipper/Triangle Bound unit.  */
                    triRequest = GPU_MIN(setupFIFOSize - setupTriangles - setupReserves, trianglesCycle);

                    /*  Send triangle request to the Clipper/Triangle Bound unit.  */
                    triangleRequest->write(cycle, new PrimitiveAssemblyRequest(triRequest));

                    /*  Update number of reserved setup FIFO entries.  */
                    setupReserves += triRequest;
                }
            }
            else
            {
                /*  Check if there is an empty setup FIFO entry and triangle shader queue entry.  */
                if (!lastTriangle && ((setupTriangles + setupReserves) < setupFIFOSize) &&
                    ((shTriangleReserves + sendShTriangles + shaderTriangles) < triangleShaderQSz))
                {
                    /*  Calculate number of triangles to request to the Clipper/Triangle Bound unit.  */
                    triRequest = GPU_MIN(GPU_MIN(setupFIFOSize - setupTriangles - setupReserves, trianglesCycle), freeShTriangles);

                    /*  Send triangle request to the Clipper/Triangle Bound unit.  */
                    triangleRequest->write(cycle, new PrimitiveAssemblyRequest(triRequest));

                    /*  Update number of reserved setup FIFO entries.  */
                    setupReserves += triRequest;

                    /*  Update number or reserved triangle shader queue entries.  */
                    shTriangleReserves++;
                }
            }


            /*  Check if setup is blocked processing a triangle.  */
            if (setupWait > 0)
            {
                GPU_DEBUG_BOX( printf("TriangleSetup => Setting triangle.\n"); )

                /*  Update next setup wait cycle counter.  */
                setupWait--;
            }
            else
            {
                /*  We can start the setup of a new triangle.  */
                /*  Read new triangle input from Clipper/Triangle Bound unit.  */
                while (inputTriangle->read(cycle, (DynamicObject *&) tsInput))
                {
                    /*  Check if there is an empty entry in the setup FIFO.  */
                    GPU_ASSERT(
                        if (setupTriangles == setupFIFOSize)
                            panic("TriangleSetup", "clock", "No free setup FIFO entries.");
                    )

                    GPU_DEBUG_BOX(
                        printf("TriangleSetup => Start setup of a new %s triangle ID %d.\n",
                            tsInput->isLast()?"last":"", tsInput->getTriangleID());
                    )

                    /*  Set if last triangle has been received.  */
                    lastTriangle = tsInput->isLast();

                    /*  Check if the unified shaders perform the setup.  */
                    if (shaderSetup)
                    {
                        /*  Check there are free entries in the triangle shader queue.  */
                        GPU_ASSERT(
                            if (freeShTriangles == 0)
                                panic("TriangleSetup", "clock", "No free entries in the triangle shader queue.");
                        )

                        /*  Store triangle setup input.  */
                        triangleShaderQueue[nextFreeShaderTriangle].tsInput = tsInput;

                        /*  Set as not yet shaded.  */
                        triangleShaderQueue[nextFreeShaderTriangle].shaded = false;

                        /*  Update triangles waiting to be sent to the shaders.  */
                        sendShTriangles++;

                        /*  Update number of free triangle shader queue entries.  */
                        freeShTriangles--;

                        /*  Update number or reserved triangle shader queue entries.  */
                        shTriangleReserves--;

                        /*  Update pointer to the next free triangle shader queue entry.  */
                        nextFreeShaderTriangle = GPU_MOD(nextFreeShaderTriangle + 1, triangleShaderQSz);
                    }
                    else
                    {
                        /*  Send triangle through the setup signal.  */
                        setupStart->write(cycle, tsInput);
                    }

                    /*  Set next setup wait cycles counter.  */
                    setupWait = setupStartLatency - 1;

                    /*  Update statistics.  */
                    inputs->inc();
                }
            }


            /*  Check if setup is performed in the unified shaders.  */
            if (shaderSetup)
            {
                /*  Send triangle inputs to the shaders.  */
                for(i = 0; (i < numSetupUnits) && (sendShTriangles > 0) && (shState != SH_BUSY); i++)
                {
                    /*  Get triangle input from the triangle shader queue.  */
                    tsInput = triangleShaderQueue[nextSendShTriangle].tsInput;

                    /*  Build shader input.  */

                    /*  Allocate triangle attributes.  */
                    shAttributes = new QuadFloat[MAX_TRIANGLE_ATTRIBUTES];

                    /*  Check allocation.  */
                    GPU_ASSERT(
                        if (shAttributes == NULL)
                            panic("TriangleSetup", "clock", "Error allocating triangle attributes.");
                    )

                    /*  Check if the triangle is the last in the batch.  The last
                        triangle has no attributes so the triangle shader attributes
                        must be ignored.  */
                    if (!tsInput->isLast())
                    {
                        /*  Pack triangle attributes.  */
                        vertAttr1 = tsInput->getVertexAttributes(0);
                        vertAttr2 = tsInput->getVertexAttributes(1);
                        vertAttr3 = tsInput->getVertexAttributes(2);

                        /*  Set X triangle input attribute.  */
                        shAttributes[X_ATTRIBUTE][0] = vertAttr1[POSITION_ATTRIBUTE][0];
                        shAttributes[X_ATTRIBUTE][1] = vertAttr2[POSITION_ATTRIBUTE][0];
                        shAttributes[X_ATTRIBUTE][2] = vertAttr3[POSITION_ATTRIBUTE][0];
                        shAttributes[X_ATTRIBUTE][3] = 0.0f;

                        /*  Set Y triangle input attribute.  */
                        shAttributes[Y_ATTRIBUTE][0] = vertAttr1[POSITION_ATTRIBUTE][1];
                        shAttributes[Y_ATTRIBUTE][1] = vertAttr2[POSITION_ATTRIBUTE][1];
                        shAttributes[Y_ATTRIBUTE][2] = vertAttr3[POSITION_ATTRIBUTE][1];
                        shAttributes[Y_ATTRIBUTE][3] = 0.0f;

                        /*  Set Z triangle input attribute.  */
                        shAttributes[Z_ATTRIBUTE][0] = vertAttr1[POSITION_ATTRIBUTE][2];
                        shAttributes[Z_ATTRIBUTE][1] = vertAttr2[POSITION_ATTRIBUTE][2];
                        shAttributes[Z_ATTRIBUTE][2] = vertAttr3[POSITION_ATTRIBUTE][2];
                        shAttributes[Z_ATTRIBUTE][3] = 0.0f;

                        /*  Set W triangle input attribute.  */
                        shAttributes[W_ATTRIBUTE][0] = vertAttr1[POSITION_ATTRIBUTE][3];
                        shAttributes[W_ATTRIBUTE][1] = vertAttr2[POSITION_ATTRIBUTE][3];
                        shAttributes[W_ATTRIBUTE][2] = vertAttr3[POSITION_ATTRIBUTE][3];
                        shAttributes[W_ATTRIBUTE][3] = 0.0f;
                    }

                    GPU_DEBUG_BOX(
                        printf("TriangleSetup => Sending triangle input to shader (FragmentFIFO) from %d\n",
                            nextSendShTriangle);
                    )

                    /*  Create shader input.  */
                    shInput = new ShaderInput(0, 0, nextSendShTriangle, shAttributes, SHIM_TRIANGLE);

                    /*  Set last triangle flag.  */
                    shInput->setLast(tsInput->isLast());

                    /*  Send triangle shader input to the unified shaders (FragmentFIFO).  */
                    setupShaderInput->write(cycle, shInput);

                    /*  Update number of triangles waiting for result.  */
                    shaderTriangles++;

                    /*  Update number of triangles waiting to be sent to the shaders.  */
                    sendShTriangles--;

                    /*  Update pointer to the next triangle to send.  */
                    nextSendShTriangle = GPU_MOD(nextSendShTriangle + 1, triangleShaderQSz);
                }

                /*  Process setup triangles from the shader.  */
                for(i = 0; (i < trianglesCycle) && (shaderTriangles > 0); i++)
                {
                    /*  Check if shader result has been received.  */
                    if (triangleShaderQueue[nextShaderTriangle].shaded)
                    {
                        GPU_DEBUG_BOX(
                            printf("TriangleSetup => Commiting setup triangle from the shader at %d.\n", nextShaderTriangle);
                        )

                        /*  Check if the triangle is the last in the batch.  Last triangle is just a dummy
                            triangle without valid data so ignore and pass it down the pipeline.  */
                        if (!triangleShaderQueue[nextShaderTriangle].tsInput->isLast())
                        {
                            /*  Get the triangle vertex attributes.  */
                            vertAttr1 = triangleShaderQueue[nextShaderTriangle].tsInput->getVertexAttributes(0);
                            vertAttr2 = triangleShaderQueue[nextShaderTriangle].tsInput->getVertexAttributes(1);
                            vertAttr3 = triangleShaderQueue[nextShaderTriangle].tsInput->getVertexAttributes(2);

                            if (preTriangleBound)
                            {
                                GPU_ASSERT(
                                    if (!triangleShaderQueue[nextShaderTriangle].tsInput->isPreBound())
                                        panic("TriangleSetup", "clock", "Triangle processed at shader was not pre-bound.");
                                )

                                /*  Get the Rasterizer emulator ID of a previously bound triangle.  */
                                triangleID = triangleShaderQueue[nextShaderTriangle].tsInput->getSetupID();
 
                                /*  Ask the rasterizer emulator to setup a triangle previously bound.  */
                                rastEmu.setupEdgeEquations(triangleID,
                                    triangleShaderQueue[nextShaderTriangle].A,
                                    triangleShaderQueue[nextShaderTriangle].B,
                                    triangleShaderQueue[nextShaderTriangle].C,
                                    triangleShaderQueue[nextShaderTriangle].tArea);
                            }
                            else
                            {
                                /*  Ask the rasterizer emulator to setup the triangle.  */
                                triangleID = rastEmu.setup(vertAttr1, vertAttr2, vertAttr3,
                                    triangleShaderQueue[nextShaderTriangle].A,
                                    triangleShaderQueue[nextShaderTriangle].B,
                                    triangleShaderQueue[nextShaderTriangle].C,
                                    triangleShaderQueue[nextShaderTriangle].tArea);
                            }
                        }
                        
                        /*  Process the setup triangle. */
                        processSetupTriangle(triangleShaderQueue[nextShaderTriangle].tsInput, triangleID);

                        /*  Update number of free triangle shader queue entries.  */
                        freeShTriangles++;

                        /*  Update number of triangles waiting for commit.  */
                        shaderTriangles--;

                        /*  Update pointer to the next shader triangle to commit.  */
                        nextShaderTriangle = GPU_MOD(nextShaderTriangle + 1, triangleShaderQSz);
                    }
                }

                /*  Receive setup triangles from the shader.  */
                while (setupShaderOutput->read(cycle, (DynamicObject *&) shInput))
                {
                    /*  Get the triangle shader queue entry for the triangle shader output.  */
                    shOutputTriangle = shInput->getEntry();

                    /*  Set triangle as shaded (setup).  */
                    triangleShaderQueue[shOutputTriangle].shaded = true;

                    /*  Get triangle shader attributes.  */
                    shAttributes = shInput->getAttributes();

                    /*  Set triangle setup parameters.  */
                    triangleShaderQueue[shOutputTriangle].A[0] = shAttributes[A_ATTRIBUTE][0];
                    triangleShaderQueue[shOutputTriangle].A[1] = shAttributes[A_ATTRIBUTE][1];
                    triangleShaderQueue[shOutputTriangle].A[2] = shAttributes[A_ATTRIBUTE][2];
                    triangleShaderQueue[shOutputTriangle].A[3] = shAttributes[A_ATTRIBUTE][3];

                    triangleShaderQueue[shOutputTriangle].B[0] = shAttributes[B_ATTRIBUTE][0];
                    triangleShaderQueue[shOutputTriangle].B[1] = shAttributes[B_ATTRIBUTE][1];
                    triangleShaderQueue[shOutputTriangle].B[2] = shAttributes[B_ATTRIBUTE][2];
                    triangleShaderQueue[shOutputTriangle].B[3] = shAttributes[B_ATTRIBUTE][3];

                    triangleShaderQueue[shOutputTriangle].C[0] = shAttributes[C_ATTRIBUTE][0];
                    triangleShaderQueue[shOutputTriangle].C[1] = shAttributes[C_ATTRIBUTE][1];
                    triangleShaderQueue[shOutputTriangle].C[2] = shAttributes[C_ATTRIBUTE][2];
                    triangleShaderQueue[shOutputTriangle].C[3] = shAttributes[C_ATTRIBUTE][3];

                    triangleShaderQueue[shOutputTriangle].tArea = shAttributes[AREA_ATTRIBUTE][0];

                    GPU_DEBUG_BOX(
                        printf("TriangleSetup => Received setup triangle from shader (FragmentFIFO) for entry %d\n",
                            shOutputTriangle);
                    )

                    /*  Delete attribute array.  */
                    delete[] shAttributes;

                    /*  Delete shader input object.  */
                    delete shInput;
                }
            }
            else
            {
                /*  Read triangle from setup end signal.  */
                while (setupEnd->read(cycle, (DynamicObject *&) tsInput))
                {
                    GPU_DEBUG_BOX( printf("TriangleSetup => End of setup.\n"); )

                    /*  Check if the triangle is the last in the batch.  Last triangle is just a dummy
                        triangle without valid data so ignore and pass it down the pipeline.  */
                    if (!tsInput->isLast())
                    {
                        /*  Get the triangle vertex attributes.  */
                        vertAttr1 = tsInput->getVertexAttributes(0);
                        vertAttr2 = tsInput->getVertexAttributes(1);
                        vertAttr3 = tsInput->getVertexAttributes(2);

                        if (preTriangleBound)
                        {
                            GPU_ASSERT(
                                if (!tsInput->isPreBound())
                                    panic("TriangleSetup", "clock", "Triangle was not pre-bound.");
                            )

                            /*  Get the Rasterizer emulator ID of a previously bound triangle.  */
                            triangleID = tsInput->getSetupID();

                            /*  Ask the rasterizer emulator to setup a triangle previously bound.  */
                            rastEmu.setupEdgeEquations(triangleID);
                        }
                        else
                        {
                            /*  Ask the rasterizer emulator to setup the triangle.  */
                            triangleID = rastEmu.setup(vertAttr1, vertAttr2, vertAttr3);
                        }
                    }
                    else
                    {
                        //  Avoid exceptions.
                        triangleID = -1;
                    }

                    /*  Process the setup triangle. */
                    processSetupTriangle(tsInput, triangleID);
                }
            }


            /*  Check if it has been requested a triangle.  */
            if (trianglesRequested > 0)
            {
                /*  Sends requested triangles to Fragment Generation.  */
                sendTriangles(cycle);
            }

            break;

        case RAST_END:
            /*  Draw end state.  */

            GPU_DEBUG_BOX(
                printf("TriangleSetup => END state.\n");
            )

            /*  Wait for END command from the Rasterizer main box.  */
            if (setupCommand->read(cycle, (DynamicObject *&) rastCommand))
                processCommand(rastCommand);

            break;

        default:
            panic("TriangleSetup", "clock", "Unsupported state.");
            break;
    }

    /*  Send current state to the rasterizer main box.  */
    rastSetupState->write(cycle, new RasterizerStateInfo(state));
}

/*  Processes a rasterizer command.  */
void TriangleSetup::processCommand(RasterizerCommand *command)
{
    u32bit i;

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
                printf("TriangleSetup => RESET command received.\n");
            )

            /*  Change to reset state.  */
            state = RAST_RESET;

            break;

        case RSCOM_DRAW:
            /*  Draw command from the Rasterizer main box.  */

            GPU_DEBUG_BOX(
                printf("TriangleSetup => DRAW command received.\n");
            )

            /*  Check current state.  */
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("TriangleSetup", "processCommand", "DRAW command can only be received in READY state.");
            )

            /*  Start drawing state.  */
            setupWait = 0;

            /*  Set rasterizer emulator viewport.  */
            rastEmu.setViewport(d3d9PixelCoordinates, iniX, iniY, width, height);

            /*  Set rasterizer emulator scissor rectangle.  */
            rastEmu.setScissor(hRes, vRes, scissorTest, scissorIniX, scissorIniY, scissorWidth, scissorHeight);

            /*  Set raterizer emulator depth range.  */
            rastEmu.setDepthRange(d3d9DepthRange, near, far);

            /*  Set rasterizer emulator polygon offset.  */
            rastEmu.setPolygonOffset(slopeFactor, unitOffset);

            /*  Set rasterizer emulator front face selection mode.  */
            rastEmu.setFaceMode(faceMode);
            
            //  Set rasterizer emulator d3d9 rasterization rules.
            rastEmu.setD3D9RasterizationRules(d3d9RasterizationRules);

            /*
                NOTE:  !!! DEPTH BUFFER PRECISSION SHOULD ONLY CHANGE
                ONCE PER FRAME!!!

                !!! NOT CHECKED !!!

             */

            /*  Set rasterizar depth buffer precission.  */
            rastEmu.setDepthPrecission(zBitPrecission);

            /*  Calculate minimun triangle area (pixel area?) from
                the viewport.  Viewport in clip coordinates sistem
                has 2.0 x 2.0 size.

                NOTE: !!! SHOULD BE ADJUSTED TO TAKE INTO ACCOUNT
                THE APPROXIMATION TO THE TRIANGLE AREA USED !!!

                (maybe should be calculated by the rasterizer
                 emulator).
            */

            /* !!! NOTE: AREA CULLING DISABLED AS IT DOESN'T TAKE INTO
                SMALL LARGE TRIANGLES.!!!  */

            //minTriangleArea = (f64bit) .04 / ((f64bit) width * height);

            /*  Reset triangle counter.  */
            triangleCounter = 0;

            /*  Reset pointer to the next free setup FIFO entry.  */
            nextFreeSTFIFO = 0;

            /*  Reset pointer to the first triangle in the Setup FIFO. */
            firstSetupTriangle = 0;

            /*  Reset the counter for stored triangles in the setup FIFO.  */
            setupTriangles = 0;

            /*  Reset number of reserved entries in the Setup FIFO.  */
            setupReserves = 0;

            /*  Reset Setup FIFO entries.  */
            for(i = 0; i < setupFIFOSize; i++)
                setupFIFO[i] = NULL;

            /*  Reset requested triangles counter..  */
            trianglesRequested = 0;

            /*  Reset last triangle flag.  */
            lastTriangle = FALSE;

            /*  Check if setup in unified shader is enabled.  */
            if (shaderSetup)
            {
                /*  Reset pointer to the next free triangle shader queue entry.  */
                nextFreeShaderTriangle = 0;

                /*  Reset pointer to the next input triangle to send to the shaders.  */
                nextSendShTriangle = 0;

                /*  Reset pointer to the next shader triangle to commit.  */
                nextShaderTriangle = 0;

                /*  Reset free triangle shader queue entries counter.  */
                freeShTriangles = triangleShaderQSz;

                /*  Teset triangles waiting to be sent to the shader counter.  */
                sendShTriangles = 0;

                /*  Reset triangles waiting for the shader result counter.  */
                shaderTriangles = 0;

                /*  Reset number or reserved triangle shader queue entries.  */
                shTriangleReserves = 0;

                /*  Reset triangle shader reorder queue.  */
                for(i = 0; i < triangleShaderQSz; i++)
                {
                    triangleShaderQueue[i].tsInput = NULL;
                    triangleShaderQueue[i].shaded = false;
                }
            }

            /*  Change to drawing state.  */
            state = RAST_DRAWING;

            break;

        case RSCOM_END:
            /*  End command received from Rasterizer main box.  */

            GPU_DEBUG_BOX(
                printf("TriangleSetup => END command received.\n");
            )

            /*  Check current state.  */
            GPU_ASSERT(
                if (state != RAST_END)
                    panic("TriangleSetup", "processCommand", "END command can only be received in END state.");
            )

            /*  Change to ready state.  */
            state = RAST_READY;

            break;

        case RSCOM_REG_WRITE:
            /*  Write register command from the Rasterizer main box.  */

            GPU_DEBUG_BOX(
                printf("TriangleSetup => REG_WRITE command received.\n");
            )


            /*  Check current state.  */
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("TriangleSetup", "processCommand", "REGISTER WRITE command can only be received in READY state.");
            )

            /*  Process register write.  */
            processRegisterWrite(command->getRegister(),
                command->getSubRegister(), command->getRegisterData());

            break;

        default:
            panic("TriangleSetup", "processCommand", "Unsupported command.");
            break;
    }
}

void TriangleSetup::processRegisterWrite(GPURegister reg, u32bit subreg,
    GPURegData data)
{
    /*  Process register write.  */
    switch(reg)
    {
        case GPU_DISPLAY_X_RES:
            /*  Write viewport width regsiter.  */
            hRes = data.uintVal;

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Write GPU_DISPLAY_X_RES = %d.\n", hRes);
            )

            break;

        case GPU_DISPLAY_Y_RES:
            /*  Write viewport height register.  */
            vRes = data.uintVal;

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Write GPU_DISPLAY_Y_RES = %d.\n", vRes);
            )

            break;

        case GPU_D3D9_PIXEL_COORDINATES:
            
            //  Write use D3D9 pixel coordinates convention register.
            d3d9PixelCoordinates = data.booleanVal;
            
            GPU_DEBUG_BOX(
                printf("TriangleSetup => Write GPU_D3D9_PIXEL_COORDINATES = %s\n", d3d9PixelCoordinates ? "TRUE" : "FALSE");
            )
            
            break;
            
        case GPU_VIEWPORT_INI_X:
            /*  Write viewport left most X coordinate.  */
            iniX = data.intVal;

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Write GPU_VIEWPORT_INI_X = %d.\n", iniX);
            )

            break;

        case GPU_VIEWPORT_INI_Y:
            /*  Write viewport bottom most Y coordinate.  */
            iniY = data.intVal;

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Write GPU_VIEWPORT_INI_Y = %d.\n", iniY);
            )

            break;

        case GPU_VIEWPORT_WIDTH:
            /*  Write viewport width regsiter.  */
            width = data.uintVal;

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Write GPU_VIEWPORT_WIDTH = %d.\n", width);
            )

            break;

        case GPU_VIEWPORT_HEIGHT:
            /*  Write viewport height register.  */
            height = data.uintVal;

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Write GPU_VIEWPORT_HEIGHT = %d.\n", height);
            )

            break;

        case GPU_SCISSOR_TEST:
            /*  Write scissor test enable flag.  */
            scissorTest = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Write GPU_SCISSOR_TEST = %s.\n", scissorTest?"TRUE":"FALSE");
            )

            break;

        case GPU_SCISSOR_INI_X:
            /*  Write scissor left most X coordinate.  */
            scissorIniX = data.intVal;

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Write GPU_SCISSOR_INI_X = %d.\n", scissorIniX);
            )

            break;

        case GPU_SCISSOR_INI_Y:
            /*  Write scissor bottom most Y coordinate.  */
            scissorIniY = data.intVal;

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Write GPU_SCISSOR_INI_Y = %d.\n", scissorIniY);
            )

            break;

        case GPU_SCISSOR_WIDTH:
            /*  Write scissor width regsiter.  */
            scissorWidth = data.uintVal;

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Write GPU_SCISSOR_WIDTH = %d.\n", scissorWidth);
            )

            break;

        case GPU_SCISSOR_HEIGHT:
            /*  Write scissor height register.  */
            scissorHeight = data.uintVal;

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Write GPU_SCISSOR_HEIGHT = %d.\n", scissorHeight);
            )

            break;

        case GPU_DEPTH_RANGE_NEAR:
            /*  Write near depth range register.  */
            near = data.f32Val;

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Write GPU_DEPTH_RANGE_NEAR = %f.\n", near);
            )

            break;

        case GPU_DEPTH_RANGE_FAR:
            /*  Write far depth range register.  */
            far = data.f32Val;

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Write GPU_DEPTH_RANGE_FAR = %f.\n", far);
            )

            break;

        case GPU_DEPTH_SLOPE_FACTOR:
            /*  Write depth slope factor register.  */
            slopeFactor = data.f32Val;

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Write GPU_DEPTH_SLOPE_FACTOR = %f.\n", slopeFactor);
            )

            break;

        case GPU_DEPTH_UNIT_OFFSET:
            /*  Write depth unit offset register.  */
            unitOffset = data.f32Val;

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Write GPU_DEPTH_UNIT_OFFSET = %f.\n", unitOffset);
            )

            break;

        case GPU_Z_BUFFER_BIT_PRECISSION:
            /*  Write Z Buffer bit precission register.  */
            zBitPrecission = data.uintVal;

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Write GPU_Z_BUFFER_BIT_PRECISSION = %d.\n",
                    zBitPrecission);
            )

            break;

        case GPU_D3D9_DEPTH_RANGE:
        
            //  Write use D3D9 depth range in clip space register.
            d3d9DepthRange = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Writing register GPU_D3D9_DEPTH_RANGE = %s.\n", d3d9DepthRange?"TRUE":"FALSE");
            )

            break;

        case GPU_FACEMODE:
            /*  Write face mode register.  */
            faceMode = data.faceMode;

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Write GPU_FACEMODE = ");
                switch(faceMode)
                {
                    case GPU_CW:
                        printf("CW.\n");
                        break;
                    case GPU_CCW:
                        printf("CCW.\n");
                        break;
                }
            )

            break;

        case GPU_CULLING:
            /*  Write face cull mode register.  */
            culling = data.culling;

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Write GPU_CULLING = ");
                switch(culling)
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

            break;

        case GPU_D3D9_RASTERIZATION_RULES:
        
            //  Write use D3D9 rasterization rules register.
            d3d9RasterizationRules = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Writing register GPU_D3D9_RASTERIZATION_RULES = %s.\n", d3d9RasterizationRules?"TRUE":"FALSE");
            )

            break;

        case GPU_TWOSIDED_LIGHTING:
            /* Write Two sided lighting register. */
            twoSidedLighting = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Writing register GPU_TWOSIDED_LIGHTING = %s.\n", twoSidedLighting?"TRUE":"FALSE");
            )

            break;

        default:
            panic("TriangleSetup", "processRegisterWrite", "Unsupported register.");
            break;
    }

}

/*  Performs the cull face test for the triangle.  */
bool TriangleSetup::cullFaceTest(u32bit triangleID, u32bit setupID, f64bit &tArea)
{
    bool dropTriangle;
    s32bit minX, minY, maxX, maxY;

    /*  Ask for the triangle signed area.  */
    tArea = rastEmu.triangleArea(setupID);

    GPU_DEBUG_BOX(
        printf("TriangleSetup => Triangle ID %d with area %f\n", triangleID, tArea);
    )

    /*  Check triangle area.  */
    if (tArea < 0)
    {
        /*  Update statistics.  */
        back->inc();
    }
    else
    {
        /*  Update statistics.  */
        front->inc();
    }

    /*  Check face culling mode.  */
    switch(culling)
    {
        case NONE:
            /*  If area is negative.  */
            if (tArea < 0)
            {
                /*  Change the triangle edge equation signs so
                all the triangles can be rasterized.  */
                rastEmu.invertTriangleFacing(setupID);

                GPU_DEBUG_BOX(
                    printf("TriangleSetup => Cull NONE.  Inverting back facing triangle ID %d.\n", triangleID);
                )

            }

            /*  Do not drop any triangle because of face.  */
            dropTriangle = FALSE;

            break;

        case FRONT:

            /*  If area is positive and front face culling.  */
            if (tArea > 0)
            {
                /*  Drop front facing triangle.  */
                dropTriangle = TRUE;

                /*  Destroy the triangle in the rasterizer emulator.  */
                rastEmu.destroyTriangle(setupID);

                GPU_DEBUG_BOX(
                    printf("TriangleSetup => Cull FRONT.  Culling front facing triangle ID %d.\n", triangleID);
                )
            }
            else
            {
                /*  Change the triangle edge equation signs
                    so all the triangles can be rasterized.  */
                rastEmu.invertTriangleFacing(setupID);

                /*  Do not drop back facing triangles.  */
                dropTriangle = FALSE;

                GPU_DEBUG_BOX(
                    printf("TriangleSetup => Cull FRONT.  Inverting back facing triangle ID %d.\n", triangleID);
                )
            }

            break;

        case BACK:

            /*  If area is negative and back face culling is
                selected.  */
            if (tArea < 0)
            {
                /*  Drop back facing triangles.  */
                dropTriangle = TRUE;

                /*  Destroy the triangle in the rasterizer emulator.  */
                rastEmu.destroyTriangle(setupID);

                GPU_DEBUG_BOX(
                    printf("TriangleSetup => Cull BACK.  Culling back facing triangle ID %d.\n", triangleID);
                )
            }
            else
            {
                /*  Do not drop front facing triangles.  */
                dropTriangle = FALSE;
            }

            break;

        case FRONT_AND_BACK:

            /*  Drop all triangles.  */
            dropTriangle = TRUE;

            /*  Destroy the triangle in the rasterizer emulator.  */
            rastEmu.destroyTriangle(setupID);

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Cull FRONT_AND_BACK.  Culling triangle ID %d\n", triangleID);
            )

            break;

        default:

            panic("TriangleSetup", "clock", "Unknown culling mode.");
            break;
    }

    /*  Update triangle size breakdown stats.  */
    if (!dropTriangle)
    {
        double screen_size = width * height;
        double area = abs(rastEmu.triScreenPercent(setupID)) * screen_size;

        //printf("Area: %lf of {%d,%d}{%d,%d}\n",area,iniX,iniY,width,height);

        if (area < 1.0)
           count_less_1->inc();
        else if (area < 4.0)
           count_1_to_4->inc();
        else if (area < 16.0)
           count_4_to_16->inc();
        else if (area < 100.0)
           count_16_to_100->inc();
        else if (area < 1000.0)
           count_100_to_1000->inc();
        else if (area < screen_size)
           count_1000_to_screen->inc();
        else if (area >= screen_size)
           count_greater_screen->inc();

        u32bit overlapsX, overlapsY;

        /*  Get the triangle Bounding Box computed at setup.  */
        rastEmu.triangleBoundingBox(setupID, minX, minY, maxX, maxY);

        /*  Since the triangle BB is exterior to the triangle (BB borders do not include 
            the triangle vertexes), the border pixels must be subtracted to get x and y
            overlaps.  */
        overlapsX = (maxX - minX) - 1;
        overlapsY = (maxY - minY) - 1;

        if ((overlapsX == 1) && (overlapsY == 1))
           count_overlap_1x1->inc();

        if ((overlapsX <= 2) && (overlapsY <= 2))
           count_overlap_2x2->inc();
    }

    /*  Check if two sided lighting color selection is enabled.  */
    if (twoSidedLighting && !dropTriangle)
    {
        rastEmu.selectTwoSidedColor(setupID);
    }

    return dropTriangle;
}

/*  Sends triangles to triangle traversal/fragment generation.  */
void TriangleSetup::sendTriangles(u64bit cycle)
{
    TriangleSetupOutput *tsOutput;
    u32bit i;

    /*  Send triangles to Fragment Generation.  */
    for(i = 0; (i < trianglesCycle) && (trianglesRequested > 0) && (setupTriangles > 0); i++)
    {
        /*  Get next setup triangle.  */
        tsOutput = setupFIFO[firstSetupTriangle];

        GPU_DEBUG_BOX(
            printf("TriangleSetup => Sending setup triangle ID %d to Fragment Generation.\n",
                tsOutput->getTriangleID());
        )

        /*  Send setup triangle to the Fragment Generator.  */
        setupOutput->write(cycle, tsOutput);

        /*  Update counter of stored setup triangles.  */
        setupTriangles--;

        /*  Check if it is the last triangle.  */
        if (tsOutput->isLast())
        {
            /*  Check there are no other triangles in the FIFO.  */
            GPU_ASSERT(
                if (setupTriangles > 0)
                {
printf("TSetup %lld > setupTriangles %d setupReserves %d triangleCounter %d trianglesRequested %d trianglesCycle %d i %d\n",
cycle, setupTriangles, setupReserves, triangleCounter, trianglesRequested, trianglesCycle, i);

                    panic("TriangleSetup", "sendTriangles", "Setup triangles received after last triangle mark.");
                }
            )

            /*  Batch end, change to END state.  */
            state = RAST_END;
        }

        /*  Free the setup FIFO entry.  */
        setupFIFO[firstSetupTriangle] = NULL;

        /*  Update pointer to the next setup triangle.  */
        firstSetupTriangle = GPU_MOD(firstSetupTriangle + 1, setupFIFOSize);

        /*  Update triangles requested counter.  */
        trianglesRequested--;

        /*  Update statistics.  */
        outputs->inc();
    }
}

/*  Processes a setup triangle.  */
void TriangleSetup::processSetupTriangle(TriangleSetupInput *tsInput, u32bit triangleID)
{
    TriangleSetupOutput *tsOutput;
    f64bit tArea;
    bool dropTriangle;

    /*  Check if the triangle was already rejected.  */
    if (!tsInput->isLast())
    {
        /*  Perform cull face test.  */
        dropTriangle = cullFaceTest(tsInput->getTriangleID(), triangleID, tArea);

        /* !!! NOTE: AREA CULLING DISABLED AS IT DOESN'T TAKE INTO
           SMALL LARGE TRIANGLES.!!!

           ONLY FOR AREA 0.

         */

        /*  Check the triangle area.  */
        if (!dropTriangle && (tArea == 0.0f))
        {
            /*  Drop the triangle.  */
            dropTriangle = TRUE;

            /*  Destroy the triangle in the rasterizer emulator.  */
            rastEmu.destroyTriangle(triangleID);

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Cull by area.  Culling triangle ID %d with area %f.\n",
                    tsInput->getTriangleID(), tArea);
            )
        }
    }
    else
    {
        /*  Do no drop the triangle!!.  */
        dropTriangle = FALSE;

        GPU_DEBUG_BOX(
            printf("TriangleSetup => Last triangle ID %d\n", tsInput->getTriangleID());
        )
    }

    /*  Check if the triangle must be dropped either because
        of face culling or because minimun area culling.  */
    if (dropTriangle)
    {
        /*  Dropped triangles are not sent down the pipeline.  */

        /*  Updated setup reserves counter.  */
        setupReserves--;

        /*  Update statistics.  */
        culled->inc();
    }
    else
    {
        /*  Store triangle in the setup queue.  */

        /*  Check if it was last triangle.  */
        if (tsInput->isLast())
        {
            /*  Send the last triangle down the pipeline.  */
            tsOutput = new TriangleSetupOutput(tsInput->getTriangleID(), 0, TRUE);

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Triangle ID %d marked as last triangle.\n",
                    tsInput->getTriangleID());
            )

        }
        else
        {
            /*  Create Triangle Setup Output object for the triangle.
                The output triangle has two identifiers: the
                identifier inside the current batch and the
                identifier of the setup triangle in the rasterizer
                emulator.  */
            tsOutput = new TriangleSetupOutput(tsInput->getTriangleID(), triangleID, FALSE);

            GPU_DEBUG_BOX(
                printf("TriangleSetup => Triangle ID %d stored in setup FIFO.\n",
                    tsInput->getTriangleID());
            )
        }

        /*  Copy cookies from the triangle input.  */
        tsOutput->copyParentCookies(*tsInput);

        /*  Update setup reserves counter.  */
        setupReserves--;

        /*  Update setup triangles counter.  */
        setupTriangles++;

        /*  Store setup triangle in the Setup FIFO.  */
        setupFIFO[nextFreeSTFIFO] = tsOutput;

        /*  Update next free setup FIFO entry pointer.  */
        nextFreeSTFIFO = GPU_MOD(nextFreeSTFIFO + 1, setupFIFOSize);
    }

    /*  Update processed triangle counter.  */
    triangleCounter++;

    /*  Delete input triangle.  */
    delete tsInput;
}

void TriangleSetup::getState(string &stateString)
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

    stateStream << " | Triangle Counter = " << triangleCounter;
    stateStream << " | Setup Triangles = " << setupTriangles;
    stateStream << " | Setup Reserves = " << setupReserves;
    stateStream << " | Triangles Requested = " << trianglesRequested;

    stateString.assign(stateStream.str());
}


