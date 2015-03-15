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
 * $RCSfile: TriangleTraversal.cpp,v $
 * $Revision: 1.18 $
 * $Author: vmoya $
 * $Date: 2007-09-21 14:56:34 $
 *
 * Triangle Traversal box implementation file.
 *
 */

 /**
 *
 *  @file TriangleTraversal.cpp
 *
 *  This file implements the Triangle Traversal box.
 *
 *  This class traverses the triangle generating fragments.
 *
 */

#include "TriangleTraversal.h"
#include "HZStateInfo.h"
#include "TriangleSetupRequest.h"
#include "FragmentInput.h"
#include "RasterizerStateInfo.h"
#include <stdio.h>
#include <sstream>

using namespace gpu3d;


/*  Triangle Traversal box constructor.  */
TriangleTraversal::TriangleTraversal(RasterizerEmulator &rasEmu, u32bit cycleTriangles, u32bit setupLat,
    u32bit stampsPerCycle, u32bit zSamplesCycle, u32bit triBatch, u32bit trQSz,
    bool recursive, bool microTrisAsFrag, u32bit nStampUnits,
    u32bit overWidth, u32bit overHeight, u32bit scanWidth, u32bit scanHeight,
    u32bit genWidth, u32bit genHeight, char *name, Box *parent) :

    rastEmu(rasEmu), trianglesCycle(cycleTriangles), setupLatency(setupLat), stampsCycle(stampsPerCycle),
    samplesCycle(zSamplesCycle), triangleBatch(triBatch), triangleQSize(trQSz), recursiveMode(recursive),
    numStampUnits(nStampUnits),
    overH(overHeight), overW(overWidth),
    scanH((u32bit) ceil(scanHeight / (f32bit) genHeight)), scanW((u32bit) ceil(scanWidth / (f32bit) genWidth)),
    genH(genHeight / STAMP_HEIGHT), genW(genWidth / STAMP_WIDTH),
    Box(name, parent)

{
    DynamicObject *defaultState[1];

    /*  Create statistics.  */
    inputs = &getSM().getNumericStatistic("InputTriangles", u32bit(0), "TriangleTraversal", "TT");
    requests = &getSM().getNumericStatistic("RequestedTriangles", u32bit(0), "TriangleTraversal", "TT");
    outputs = &getSM().getNumericStatistic("GeneratedFragments", u32bit(0), "TriangleTraversal", "TT");
    utilizationCycles = &getSM().getNumericStatistic("UtilizationCycles", u32bit(0), "TriangleTraversal", "TT");
    stamps0frag = &getSM().getNumericStatistic("Stamps0Fragment", u32bit(0), "TriangleTraversal", "TT");
    stamps1frag = &getSM().getNumericStatistic("Stamps1Fragment", u32bit(0), "TriangleTraversal", "TT");
    stamps2frag = &getSM().getNumericStatistic("Stamps2Fragment", u32bit(0), "TriangleTraversal", "TT");
    stamps3frag = &getSM().getNumericStatistic("Stamps3Fragment", u32bit(0), "TriangleTraversal", "TT");
    stamps4frag = &getSM().getNumericStatistic("Stamps4Fragment", u32bit(0), "TriangleTraversal", "TT");

    /*  Check there are enough entries in the triangle queue.  */
    GPU_ASSERT(
        if ((samplesCycle != 2) && (samplesCycle != 4) && (samplesCycle != 8))
            panic("TriangleTraversal", "TriangleTraversal", "Unsupported number of samples per cycle.  Currently supported values are 2, 4 and 8.");
        
        if (triangleQSize < trianglesCycle)
            panic("TriangleTraversal", "TriangleTraversal", "Triangle Queue must have at least triangles per cycle entries.");
    )

    /*  Create signals.  */

    /*  Create command signal from the main rasterizer box.  */
    traversalCommand = newInputSignal("TriangleTraversalCommand", 1, 1, NULL);

    /*  Create state signal to the main rasterizer box.  */
    traversalState = newOutputSignal("TriangleTraversalState", 1, 1, NULL);

    /*  Create default signal value.  */
    defaultState[0] = new RasterizerStateInfo(RAST_RESET);

    /*  Set default signal value.  */
    traversalState->setData(defaultState);

    /*  Create setup triangle input from the Triangle Setup box.  */
    setupTriangle = newInputSignal("TriangleSetupOutput", trianglesCycle, setupLatency, NULL);

    /*  Create request signal to Triangle Setup.  */
    setupRequest = newOutputSignal("TriangleSetupRequest", 1, 1, NULL);

    /*  Create new fragment signal to the Hierarchical Z early test.  */
    newFragment = newOutputSignal("HZInputFragment", stampsCycle * STAMP_FRAGMENTS, 1, NULL);

    /*  Create state signal from the Hierarchical Z early test.  */
    hzState = newInputSignal("HZTestState", 1, 1, NULL);

    /*  Check that the triangle queue is large enough for the given batch size.  */
    GPU_ASSERT(
        if (recursiveMode && (triangleQSize < (triangleBatch * 2)))
            panic("TriangleTraversal", "TriangleTraversal", "Triangle queue too small for given rasterization batch size.");
    )

    /*  Allocate space for the triangles to be traversed.  */
    triangleQueue = new TriangleSetupOutput*[triangleQSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (triangleQueue == NULL)
            panic("TriangleTraversal", "TriangleTraversal", "Error allocating triangle queue.");
    )

    /*  If scanline rasterization mode is used only one triangle supported per batch.  */
    if (!recursiveMode)
        triangleBatch = 1;

    //  Set the number of ROP units in the Pixel Mapper.
    pixelMapper.setupUnit(numStampUnits, 0);
    
    /*  Reset state.  */
    state = RAST_RESET;

    /*  Create a fake last rasterizer command.  */
    lastRSCommand = new RasterizerCommand(RSCOM_RESET);

    /*  Set orthogonal vertex position transformation.  */
    perspectTransform = false;
}

/*  Triangle Traversal simulation function.  */
void TriangleTraversal::clock(u64bit cycle)
{
    RasterizerCommand *rastCommand;
    HZStateInfo *hzStateInfo;
    HZState hzCurrentState;
    TriangleSetupOutput *tsOutput;
    TriangleSetupRequest *tsRequest;
    Fragment **stamp;
    DynamicObject *stampCookies;
    FragmentInput *frInput;
    u32bit *triangleList;
    u32bit triangleID;
    u32bit stampUnit;
    TileIdentifier stampTileID;
    u32bit i, j;
    bool rasterizationWorking;

    GPU_DEBUG_BOX(
        printf("TriangleTraversal => clock %lld.\n", cycle);
    )

    /*  Receive state from the Hierarchical Z unit.  */
    if (hzState->read(cycle, (DynamicObject *&) hzStateInfo))
    {
        /*  Get HZ state.  */
        hzCurrentState = hzStateInfo->getState();

        /*  Delete HZ state info object.  */
        delete hzStateInfo;
    }
    else
    {
        panic("TriangleTraversal", "clock", "Missing state signal from the Hierarchical Z box.");
    }

/*if ((cycle > 5024880) && (GPU_MOD(cycle, 1000) == 0))
{
printf("TT %lld => state ", cycle);
switch(state)
{
    case RAST_RESET:    printf("RESET\n"); break;
    case RAST_READY:    printf("READY\n"); break;
    case RAST_DRAWING:  printf("DRAWING\n"); break;
    case RAST_END:      printf("END\n"); break;
}
printf("TT => stored triangles %d requested triangles %d last fragment %s\n",
storedTriangles, requestedTriangles, lastFragment?"T":"F");
}*/

    /*  Simulate current cycle.  */
    switch(state)
    {
        case RAST_RESET:
            /*  Reset state.  */

            GPU_DEBUG_BOX(
                printf("TriangleTraversal => RESET state.\n");
            )

            /*  Reset Triangle Traversal state.  */
            
            //  MSAA is disabled by default.
            multisampling = false;
            
            //  The default number of MSAA cycles is 2.
            msaaSamples = 2;
            
            //  Precalculate the number of cycles it takes for all the samples in a fragment to be generated.
            msaaCycles = u32bit(ceil(f32bit(msaaSamples) / f32bit(samplesCycle)));
            
            /*  Change to READY state.  */
            state = RAST_READY;

            break;

        case RAST_READY:
            /*  Ready state.  */

            GPU_DEBUG_BOX(
                printf("TriangleTraversal => READY state.\n");
            )

            /*  Receive and process command from the main Rasterizer box.  */
            if (traversalCommand->read(cycle, (DynamicObject *&) rastCommand))
                processCommand(rastCommand, cycle);

            break;

        case RAST_DRAWING:

            // If any rastEmu function is performed this variable will change to true to indicate that the rasterizer is actively working
            rasterizationWorking = false; 


            /*  Draw state.  */

            GPU_DEBUG_BOX(
                printf("TriangleTraversal => DRAWING state.\n");
            )

            //  Update counter for the cycles until the next group of quads can be processed by the MSAA sample generator.
            if (nextMSAAQuadCycles > 0)
            {
                //  Update counter for next MSAA generation round.
                nextMSAAQuadCycles--;
            }
            
            /*  Check if the HZ can receive a new set of fragments.  */
            if ((hzCurrentState == HZST_READY) && (!traversalFinished) && (nextMSAAQuadCycles == 0))
            {
                {
                    /*  Check if is last triangle.  */
                    if (!triangleQueue[nextTriangle]->isLast())
                    {
                        /*  Update recursive rasterization of the current triangle batch.  */
                        if (recursiveMode)
                        {
                            rastEmu.updateRecursiveMultiv2(batchID);
                            rasterizationWorking = true;
                        }
                    }

                    /*  Generate fragments.  */
                    for(i = 0; (i < stampsCycle) && (!lastFragment); i++)
                    {
                        /*  Check if is the last triangle in the batch.  */
                        if (triangleQueue[nextTriangle]->isLast())
                        {
                            /*  Allocate a stamp.  */
                            stamp = new Fragment*[STAMP_FRAGMENTS];

                            /*  Generate an empty stamp for the last triangle.  */
                            for(j = 0; j < STAMP_FRAGMENTS; j++)
                                stamp[j] = NULL;

                            GPU_DEBUG_BOX(
                                printf("TriangleTraversal => Generating an empty fragment (last triangle).\n");
                            )

                            /*  Set triangle identifier to the relative position of the last triangle in the queue.  */
                            triangleID = 0;

                            /*  Set last fragment flag.  */
                            lastFragment = TRUE;

                            /*  The last stamp must be sent to all units, so we don't care about the identifier.  */
                            stampUnit = 0;
                            stampTileID = TileIdentifier(0, 0);
                        }
                        else
                        {
                            /*  Determine rasterization method.  */
                            if (recursiveMode)
                            {
                                /*  Generate a new stamp.  */
                                stamp = rastEmu.nextStampRecursiveMulti(batchID, triangleID);
                            }
                            else
                            {
                                /*  Generate a new stamp.  */
                                stamp = rastEmu.nextScanlineStampTiled(triangleQueue[nextTriangle]->getSetupTriangleID());
                            }
                            
                            rasterizationWorking = true;

                            GPU_DEBUG_BOX(
                                printf("TriangleTraversal => Generating new stamp.\n");
                            )

                            /*  Check if a stamp was received.  */
                            if (stamp != NULL)
                            {
                                /*  Calculate the tile identifier for the first fragment (should be the same for the whole stamp).  */
                                stampTileID = rastEmu.calculateTileID(stamp[0]);

                                /*  Calculate the stamp unit assigned to the stamp  (use first fragment in the stamp).  */
                                //stampUnit = stampTileID.assignOnX(numStampUnits);
                                //stampUnit = stampTileID.assignMorton(numStampUnits);
                                stampUnit = pixelMapper.mapToUnit(stamp[0]->getX(), stamp[0]->getY());

                                /*  Set if last fragment was received.  */
                                lastFragment = stamp[STAMP_FRAGMENTS - 1]->isLastFragment();
                            }
                        }

                        /*  Check if a stamp was generated.  */
                        if (stamp != NULL)
                        {
                            /*  Allocate the container for the stamp cookies.  */
                            stampCookies = new DynamicObject();

                            /*  Generate stamp cookies from triangle cookies.  */
                            if (recursiveMode)
                            {
                                /*  Copy cookies from parent Setup Triangle.  */
                                stampCookies->copyParentCookies(*triangleQueue[GPU_MOD(nextTriangle + triangleID,triangleQSize)]);
                            }
                            else
                            {
                                /*  Copy cookies from parent Setup Triangle.  */
                                stampCookies->copyParentCookies(*triangleQueue[nextTriangle]);
                            }

                            /*  Add a new cookie for the whole stamp.  */
                            stampCookies->addCookie();

                            // Update the number of generated fragments.
                            fragmentCounter += STAMP_FRAGMENTS;

                            // Compute stamp coverage and update related stats.
                            u32bit count = 0;
                            u32bit f;

                            for (f = 0; f < STAMP_FRAGMENTS; f++)
                            {
                                    if (stamp[f] != NULL)
                                            count += (stamp[f]->isInsideTriangle()? 1 : 0);
                            }
                            switch(count)
                            {
                                    case 0: stamps0frag->inc(); break;
                                    case 1: stamps1frag->inc(); break;
                                    case 2: stamps2frag->inc(); break;
                                    case 3: stamps3frag->inc(); break;
                                    case 4: stamps4frag->inc(); break;
                            }
                        }

                        /*  Send the fragments in the stamp down the pipeline.  */
                        for(j = 0; (j < STAMP_FRAGMENTS) && (stamp != NULL); j++)
                        {
                            //  Compute MSAA samples if multisampling is enabled.
                            if (multisampling)
                            {
                                //  Check it isn't a NULL fragment from the last quad
                                if (stamp[j] != NULL)
                                {
                                    //  Compute the requested depth samples for the fragment.
                                    rastEmu.computeMSAASamples(stamp[j], msaaSamples);
                                }
                            }
                            
                            /*  Determine rasterization method.  */
                            if (recursiveMode)
                            {
                                /*  Create fragment input for the Hierarchical Z Box.  */
                                frInput = new FragmentInput(
                                    triangleQueue[GPU_MOD(nextTriangle + triangleID, triangleQSize)]->getTriangleID(),
                                    triangleQueue[GPU_MOD(nextTriangle + triangleID, triangleQSize)]->getSetupTriangleID(),
                                    stamp[j],
                                    stampTileID,
                                    stampUnit);

                                /*  Set fragment input start cycle.  */
                                frInput->setStartCycle(cycle);

                                /*  Copy cookies from parent Setup Triangle.  */
                                //frInput->copyParentCookies(*triangleQueue[GPU_MOD(nextTriangle + triangleID,triangleQSize)]);
                            }
                            else
                            {
                                /*  Create fragment input for the Hierarchical Z Box.  */
                                frInput = new FragmentInput(
                                    triangleQueue[nextTriangle]->getTriangleID(),
                                    triangleQueue[nextTriangle]->getSetupTriangleID(),
                                    stamp[j],
                                    stampTileID,
                                    stampUnit);

                                /*  Set fragment input start cycle.  */
                                frInput->setStartCycle(cycle);

                                /*  Copy cookies from parent Setup Triangle.  */
                                //frInput->copyParentCookies(*triangleQueue[nextTriangle]);
                            }

                            /*  Keep a common cookie for the whole stamp.  */
                            frInput->copyParentCookies(*stampCookies);

                            /*  Add a new cookie for each fragment.  */
                            frInput->addCookie();

                            /*  Update statistics.  */
                            outputs->inc();

                            /*  Send fragment to Interpolator.  */
                            newFragment->write(cycle, frInput);

                            GPU_DEBUG_BOX(
                                printf("TriangleTraversal => Sending Fragment to Hierarchical Z.\n");
                            )
                        }

                        /*  Check if a stamp was generated.  */
                        if (stamp != NULL)
                        {
                            /*  Delete stamp container.  */
                            delete stampCookies;
                        }

                        /*  Delete the stamp (array of pointers to fragment).  */
                        delete[] stamp;
                    }
                }
            }
//if (lastFragment)
//printf("E");

            /*  Check if the last fragment of the triangle was generated.  */
            if (!traversalFinished && !triangleQueue[nextTriangle]->isLast() && 
                rastEmu.lastFragment(triangleQueue[nextTriangle]->getSetupTriangleID()))
            {
                /*  Set current triangle as finished.  */
                traversalFinished = TRUE;

                /*  Free the setup triangles in the batch from the rasterizer emulator.  */
                for(i = 0; i < batchTriangles; i++)
                {
//printf("T");
                    /*  Free the setup triangle from the rasterizer emulator.  */
                    rastEmu.destroyTriangle(triangleQueue[GPU_MOD(nextTriangle + i, triangleQSize)]->getSetupTriangleID());

                    /*  Delete triangle output objects for the finished batch.  */
                    delete triangleQueue[GPU_MOD(nextTriangle + i, triangleQSize)];

                    /*  Set current triangle pointer to NULL.  */
                    triangleQueue[GPU_MOD(nextTriangle + i, triangleQSize)] = NULL;
                }

                /*  Update pointer to the next triangle to process.  */
                nextTriangle = GPU_MOD(nextTriangle + batchTriangles, triangleQSize);

                /*  Update number of stored triangles.  */
                storedTriangles -= batchTriangles;

                /*  Update processed triangles counter.  */
                triangleCounter += batchTriangles;

                /*  Reset last fragment flag.  */
                lastFragment = FALSE;
            }
            else if (!traversalFinished && lastFragment)
            {
printf("B");
                /*  Set current triangle as finished.  */
                traversalFinished = TRUE;

                /*  Delete current triangle.  */
                delete triangleQueue[nextTriangle];

                /*  Set current triangle pointer to NULL.  */
                triangleQueue[nextTriangle] = NULL;

                /*  Update processed triangles counter.  */
                triangleCounter++;

                /*  Update number of stored triangles.  */
                storedTriangles--;

                /*  End of batch, change to END state.  */
                state = RAST_END;
            }

            /*  Check if start rasterization of a new triangle batch.  */
            if (traversalFinished &&
                ((storedTriangles >= triangleBatch) ||
                ((storedTriangles > 0) && (triangleQueue[GPU_PMOD(nextStore - 1, triangleQSize)]->isLast()))))
            {
                /*  Set current triangle traversal as not finished.  */
                traversalFinished = FALSE;
 
                /*  Calculates the number of triangles to batch into the rasterizer.  */
                batchTriangles = GPU_MIN(storedTriangles, triangleBatch);
 
                /*  Check if the last in the batch is the last triangle.  */
                if (triangleQueue[GPU_MOD(nextTriangle + batchTriangles - 1, triangleQSize)]->isLast())
                {
                    /*  Leave the last triangle for the end.  */
                    batchTriangles--;
                }
 
                if (!triangleQueue[nextTriangle]->isLast()) /*  Check if it is the last triangle.  */
                {
                    /*  Initialize the rasterization of the current batch.  */
 
                    /*  Determine rasterization method.  */
                    if (recursiveMode)
                    {
                        /*  Create a list with the triangles in batch.  */
                        triangleList = new u32bit[batchTriangles];
                        for(i = 0; i < batchTriangles; i++)
                            triangleList[i] = triangleQueue[GPU_MOD(nextTriangle + i, triangleQSize)]->getSetupTriangleID();
 
                        /*  Start the recursive rasterization of the batch.  */
                        batchID = rastEmu.startRecursiveMulti(triangleList, batchTriangles, multisampling);
 
                        /*  Delete triangle list.  */
                        delete[] triangleList;
                    }
                    else
                    {
                        /*  Calculate start position for scanline rasterization.  */
                        rastEmu.startPosition(triangleQueue[nextTriangle]->getSetupTriangleID(), multisampling);
                    }
                    
                    rasterizationWorking = true;
                }
                else
                {
                    GPU_DEBUG_BOX(
                        printf("TriangleTraversal => Last triangle (ID %d) received.\n",
                            triangleQueue[nextTriangle]->getTriangleID());
                    )
                }
            }

            /*  Receive setup triangles from Triangle Setup.  */
            while (setupTriangle->read(cycle, (DynamicObject *&) tsOutput))
            {
                GPU_DEBUG_BOX(
                    printf("TriangleTraversal => Received new setup triangle ID %d from Triangle Setup.\n",
                        tsOutput->getTriangleID());
                )
 
                GPU_ASSERT(
                    if (storedTriangles == triangleQSize)
                        panic("TriangleTraversal", "clock", "Triangle buffer full.");
                )
 
                /*  Store as current triangle to traverse.  */
                triangleQueue[nextStore] = tsOutput;
 
                /*  Update pointer to next store position.  */
                nextStore = GPU_MOD(nextStore + 1, triangleQSize);
 
                /*  Update number of stored triangles.  */
                storedTriangles++;
 
                /*  Update requested triangles counter.  */
                requestedTriangles--;
 
                /*  Update statistics.  */
                inputs->inc();
            }

            /*  Check if triangles can be requested to setup.  */
            if ((storedTriangles + requestedTriangles + trianglesCycle) <= triangleQSize)
            {
                GPU_DEBUG_BOX(
                    printf("TriangleTraversal => Requested triangle to Triangle Setup.\n");
                )
 
                /*  Create a request to Triangle Setup.  */
                tsRequest = new TriangleSetupRequest(trianglesCycle);
 
                /*  Copy cookies from the last DRAW command.  */
                tsRequest->copyParentCookies(*lastRSCommand);
 
                /*  Add a new cookie.  */
                tsRequest->addCookie();
 
                /*  Send request to Triangle Setup.  */
                setupRequest->write(cycle, tsRequest);
 
                /*  Update number of requested triangles.  */
                requestedTriangles += trianglesCycle;
 
                /*  Update statistics.  */
                requests->inc();
            }

            // Update utilization statistic
            if ( rasterizationWorking )
                utilizationCycles->inc();

            break;

        case RAST_END:
            /*  Draw end state.  */

            GPU_DEBUG_BOX(
                printf("TriangleTraversal => END state.\n");
            )

            /*  Wait for END command from the Rasterizer main box.  */
            if (traversalCommand->read(cycle, (DynamicObject *&) rastCommand))
                processCommand(rastCommand, cycle);

            break;

        default:
            panic("TriangleTraversal", "clock", "Unsupported state.");
            break;
    }

    /*  Send current state to the main Rasterizer box.  */
    traversalState->write(cycle, new RasterizerStateInfo(state));
}


/*  Processes a rasterizer command.  */
void TriangleTraversal::processCommand(RasterizerCommand *command, u64bit cycle)
{
    u32bit samples;

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
                printf("TriangleTraversal => RESET command received.\n");
            )

            /*  Change to reset state.  */
            state = RAST_RESET;

            break;

        case RSCOM_DRAW:
            /*  Draw command from the Rasterizer main box.  */

            GPU_DEBUG_BOX(
                printf("TriangleTraversal => DRAW command received.\n");
            )


            /*  Check current state.  */
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("TriangleTraversal", "processCommand", "DRAW command can only be received in READY state.");
            )

            /*  Reset number of stored triangles.  */
            storedTriangles = 0;

            /*  Reset pointer to the next triangle to process.  */
            nextTriangle = 0;

            /*  Reset pointer to the next store position in the triangle queue.  */
            nextStore = 0;

            /*  Reset number of requested triangles.  */
            requestedTriangles = 0;

            /*  Reset triangle counter.  */
            triangleCounter = 0;

            /*  Reset batch fragment counter.  */
            fragmentCounter = 0;

            //  Reset the number of cycles remaining until the next group of quads can be processed by the MSAA sample generator.
            nextMSAAQuadCycles = 0;
            
            /*  No triangle to traverse yet.  */
            traversalFinished = TRUE;

            /*  Reset last fragment flag.  */
            lastFragment = FALSE;

            //  Set display in the pixel mapper.
            samples = multisampling ? msaaSamples : 1;
            pixelMapper.setupDisplay(hRes, vRes, STAMP_WIDTH, STAMP_WIDTH, genW, genH, scanW, scanH, overW, overH, samples, 1);

            /*  Change to drawing state.  */
            state = RAST_DRAWING;

            break;

        case RSCOM_END:
            /*  End command received from Rasterizer main box.  */

            GPU_DEBUG_BOX(
                printf("TriangleTraversal => END command received.\n");
            )


            /*  Check current state.  */
            GPU_ASSERT(
                if (state != RAST_END)
                    panic("TriangleTraversal", "processCommand", "END command can only be received in END state.");
            )

            /*  Change to ready state.  */
            state = RAST_READY;

            break;

        case RSCOM_REG_WRITE:
            /*  Write register command from the Rasterizer main box.  */

            GPU_DEBUG_BOX(
                printf("TriangleTraversal => REG_WRITE command received.\n");
            )

            /*  Check current state.  */
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("TriangleTraversal", "processCommand", "REGISTER WRITE command can only be received in READY state.");
            )

            /*  Process register write.  */
            processRegisterWrite(command->getRegister(),
                command->getSubRegister(), command->getRegisterData());

            break;

        default:
            panic("TriangleTraversal", "proccessCommand", "Unsupported command.");
            break;
    }
}

void TriangleTraversal::processRegisterWrite(GPURegister reg, u32bit subreg,
    GPURegData data)
{
    /*  Process register write.  */
    switch(reg)
    {
        case GPU_DISPLAY_X_RES:
            /*  Write display horizontal resolution register.  */
            hRes = data.uintVal;

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Write GPU_DISPLAY_X_RES = %d.\n", hRes);
            )

            break;

        case GPU_DISPLAY_Y_RES:
            /*  Write display vertical resolution register.  */
            vRes = data.uintVal;

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Write GPU_DISPLAY_Y_RES = %d.\n", vRes);
            )

            break;

        case GPU_VIEWPORT_INI_X:
            /*  Write viewport initial x coordinate register.  */
            startX = data.intVal;

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Write GPU_VIEWPORT_INI_X = %d.\n", startX);
            )

            break;

        case GPU_VIEWPORT_INI_Y:
            /*  Write viewport initial y coordinate register.  */
            startY = data.intVal;

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Write GPU_VIEWPORT_INI_Y = %d.\n", startY);
            )

            break;

        case GPU_VIEWPORT_WIDTH:
            /*  Write viewport width register.  */
            width = data.uintVal;

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Write GPU_VIEWPORT_WIDTH = %d.\n", width);
            )

            break;

        case GPU_VIEWPORT_HEIGHT:
            /*  Write viewport height register.  */
            height = data.uintVal;

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Write GPU_VIEWPORT_HEIGHT = %d.\n", height);
            )

            break;
    
        case GPU_MULTISAMPLING:
        
            //  Write Multisampling enable flag.
            multisampling = data.booleanVal;
            
            GPU_DEBUG_BOX(
                printf("TriangleTraversal => Write GPU_MULTISAMPLING = %s\n", multisampling?"TRUE":"FALSE");
            )
                       
            break;
            
        case GPU_MSAA_SAMPLES:
        
            //  Write MSAA z samples per fragment register.
            msaaSamples = data.uintVal;
            
            //  Precalculate the number of cycles it takes for all the samples in a fragment to be generated.
            msaaCycles = u32bit(ceil(f32bit(msaaSamples) / f32bit(samplesCycle)));

            GPU_DEBUG_BOX(
                printf("TriangleTraversal => Write GPU_MSAA_SAMPLES = %d\n", msaaSamples);
            )

            break;            

        default:
            panic("TriangleTraversal", "processRegisterWrite", "Unsupported register.");
            break;
    }

}

void TriangleTraversal::getState(string &stateString)
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

    stateStream << " | Stored Triangles = " << storedTriangles;
    stateStream << " | Requested Triangles = " << requestedTriangles;
    stateStream << " | Triangle Counter = " << triangleCounter;
    stateStream << " | Traversal Finished = " << traversalFinished;
    stateStream << " | Fragment Counter = " << fragmentCounter;

    stateString.assign(stateStream.str());
}
