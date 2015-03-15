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
 * $RCSfile: ColorWriteV2.cpp,v $
 * $Revision: 1.8 $
 * $Author: vmoya $
 * $Date: 2007-12-17 19:54:58 $
 *
 * Color Write box implementation file.
 *
 */

/**
 *
 *  @file ColorWriteV2.cpp
 *
 *  This file implements the Color Write box.
 *
 *  This class implements the final color blend and color write
 *  stages of the GPU pipeline.
 *
 */

#include "ColorWriteV2.h"
#include "MemoryTransaction.h"
#include "RasterizerStateInfo.h"
#include "ROPStatusInfo.h"
#include "ColorBlockStateInfo.h"
#include "GPUMath.h"
#include <cstdio>
#include "GPU.h" // STAMP_FRAGMENTS
#include <sstream>

using namespace gpu3d;

/*  Color Write box constructor.  */
ColorWriteV2::ColorWriteV2(u32bit numStamps, u32bit overWidth, u32bit overHeight,
    u32bit scanWidth, u32bit scanHeight, u32bit genWidth, u32bit genHeight,
    bool colorComprDisabled, u32bit cacheWays, u32bit cacheLines, u32bit stampsPerLine,
    u32bit portWidth, bool extraReadP, bool extraWriteP, u32bit cacheReqQSz, u32bit inputReqQSz,
    u32bit outputReqQSz, u32bit numStampUnits_, u32bit colorBlocks, u32bit blocksPerCycle,
    u32bit comprCycles, u32bit decomprCycles,
    u32bit cInQSz, u32bit cFetchQSz, u32bit cReadQSz, u32bit cOpQSz, u32bit cWriteQSz,
    u32bit blendStartLat, u32bit blendLat, u32bit blockLatency, u32bit fragMapMode,
    FragmentOpEmulator &fragEmu,
    char *name, char *prefix, Box *parent) :

    disableColorCompr(colorComprDisabled), colorCacheWays(cacheWays), colorCacheLines(cacheLines),
    stampsLine(stampsPerLine), cachePortWidth(portWidth), extraReadPort(extraReadP), extraWritePort(extraWriteP),
    blocksCycle(blocksPerCycle),
    blockStateLatency(blockLatency), fragmentMapMode(fragMapMode), numStampUnits(numStampUnits_),
    GenericROP(numStamps, blendStartLat, blendLat,
        overWidth, overHeight, scanWidth, scanHeight, genWidth, genHeight,
        cInQSz, cFetchQSz, cReadQSz, cOpQSz, cWriteQSz,
        fragEmu,
        "ColorWrite", "CW", name, prefix, parent)

{
    DynamicObject *defaultState[1];
    u32bit i;
    char fullName[64];
    char postfix[32];
    u32bit freeQueueSize;

    //  Check parameters.
    GPU_ASSERT(
        if (colorCacheWays < 1)
            panic("ColorWrite", "ColorWrite", "Color Cache requires at least 1 way.");
        if (colorCacheLines < 1)
            panic("ColorWrite", "ColorWrite", "Color Cache requires at least 1 line.");
        if (stampsLine < 1)
            panic("ColorWrite", "ColorWrite", "Color Cache requires at least one stamp per line.");
        if (stampsLine == 0)
            panic("ColorWrite", "ColorWrite", "Color cache ports must be at least 1 byte wide.");
        if (blocksCycle == 0)
            panic("ColorWrite", "ColorWrite", "At least a color block status register must be cleared per cycle");
        if ((!disableColorCompr) && (stampsLine < 16))
            panic("ColorWrite", "ColorWrite", "Color compression only supported for 8x8 fragment tiles and 256 byte cache lines.");
        if (blockStateLatency < 1)
            panic("ColorWrite", "ColorWrite", "Color block state update signal latency must be 1 cycle or greater.");
    )

    //  Create the full name and postfix for the statistics.
    sprintf(fullName, "%s::%s", prefix, name);
    sprintf(postfix, "CW-%s", prefix);


    //  Create statistics.
    blended = &getSM().getNumericStatistic("BlendedFragments", u32bit(0), fullName, postfix);
    logoped = &getSM().getNumericStatistic("LogicOpFragments", u32bit(0), fullName, postfix);

    //  Create signals.


    //  Create signals with the main Rasterizer box.

    //  Create command signal from the main Rasterizer box.
    rastCommand = newInputSignal("ColorWriteCommand", 1, 1, prefix);

    //  Create state signal to the main Rasterizer box.
    rastState = newOutputSignal("ColorWriteRasterizerState", 1, 1, prefix);

    //  Create default signal value.
    defaultState[0] = new RasterizerStateInfo(RAST_RESET);

    //  Set default signal value.
    rastState->setData(defaultState);


    //  Create signals with fragment producer stage.

    //  Create input fragments signal from the fragment producer stage (Fragment FIFO box).
    inFragmentSignal = newInputSignal("ColorWriteInput", numStamps * STAMP_FRAGMENTS, 1, prefix);

    //  Create state signal to the producer stage (Fragment FIFO box).
    ropStateSignal = newOutputSignal("ColorWriteFFIFOState", 1, 1, prefix);

    /*  Create default signal value.  */
    defaultState[0] = new ROPStatusInfo(ROP_READY);

    /*  Set default signal value.  */
    ropStateSignal->setData(defaultState);

    //  Color Write has no output to a consumer stage.  Set output fragment signal
    //  and consumer state signals to NULL;
    outFragmentSignal = NULL;
    consumerStateSignal = NULL;

    //  Create signals with Memory Controller box.

    //  Create request signal to the Memory Controller.
    memRequest = newOutputSignal("ColorWriteMemoryRequest", 1, 1, prefix);

    //  Create data signal from the Memory Controller.
    memData = newInputSignal("ColorWriteMemoryData", 2, 1, prefix);


    //  Create signals that simulate the blend latency.

    //  Create start signal for blend operation.
    operationStart = newOutputSignal("ColorWriteBlend", 1, blendLat, prefix);

    //  Create end signal for blend operation.
    operationEnd = newInputSignal("ColorWriteBlend", 1, blendLat, prefix);


    //  Create signal with the DAC.

    //  Create block state info signal to the DAC.
    blockStateDAC = newOutputSignal("ColorBlockState", 1, blockStateLatency, prefix);


    //
    //  !!!NOTE: A cache line contains a number of stamps from
    //  the same tile.  The stamps in a line must be in sequential
    //  lines.  !!!
    //
    bytesLine = stampsPerLine * STAMP_FRAGMENTS * 4;

    //  Calculate the line shift and mask.
    lineShift = GPUMath::calculateShift(bytesLine);
    lineMask = GPUMath::buildMask(colorCacheLines);

    //   Calculate stamp unit stride in block (gen tiles).
    u32bit strideUnit = u32bit(ceil((scanWidth * scanHeight) / f32bit (genWidth * genHeight)));
    
    //  Initialize color cache/write buffer.
    colorCache = new ColorCacheV2(
        colorCacheWays,                     /*  Number of ways in the Color Cache.  */
        colorCacheLines,                    /*  Number of lines per way.  */
        bytesLine,                          /*  Bytes per stamp.  */
        1 + (extraReadPort?1:0),            /*  Read ports.  */
        1 + (extraWritePort?1:0),           /*  Write ports.  */
        cachePortWidth,                     /*  Width of the color cache ports (bytes).  */
        cacheReqQSz,                        /*  Size of the memory request queue.  */
        inputReqQSz,                        /*  Size of the input request queue.  */
        outputReqQSz,                       /*  Size of the output request queue.  */
        disableColorCompr,                  /*  Disables color compression.  */
        numStampUnits,                      /*  Number of stamp units in the GPU.  */
        strideUnit,                         /*  Stamp unit stride in blocks.  */
        colorBlocks,                        /*  Number of color blocks in the state memory.  */
        blocksCycle,                        /*  Number of color blocks cleared per cycle.  */
        comprCycles,                        /*  Number of cycles to compress a color block.  */
        decomprCycles,                      /*  Number of cycles to decompress a color block.  */
        postfix                             /*  Postfix for the cache statistics.  */
        );

    //  Set ROP cache.
    ropCache = colorCache;


    //  Calculate the total number of stamps that can be in the Z Stencil Test pipeline
    freeQueueSize = cInQSz + cFetchQSz + cReadQSz + blendLat + cOpQSz + cWriteQSz;
    freeQueue.resize(freeQueueSize);

    //  Initialize free stamp queue entries.
    for(int i = 0; i < freeQueueSize; i++)
    {
        ROPQueue *freeStamp;

        //  Allocate a new stamp queue entry.
        freeStamp = new ROPQueue;

        //  Allocate the buffer for color buffer data.
        freeStamp->data = new u8bit[MAX_RENDER_TARGETS * STAMP_FRAGMENTS * MAX_MSAA_SAMPLES * MAX_BYTES_PER_COLOR];

        //  Allocate the mask for the color buffer data.
        freeStamp->mask = new bool[MAX_RENDER_TARGETS * STAMP_FRAGMENTS * MAX_MSAA_SAMPLES * MAX_BYTES_PER_COLOR];

        //  Add the created stamp containter to the free queue.
        freeQueue.add(freeStamp);
    }

    //  Allocate buffer for the blend output color data.
    outColor = new u8bit[STAMP_FRAGMENTS * MAX_MSAA_SAMPLES * MAX_BYTES_PER_COLOR];

    //  Create a dummy initial rasterizer command.
    lastRSCommand = new RasterizerCommand(RSCOM_RESET);

    //  Debug/log.
    traceFragment = false;
    watchFragmentX = 0;
    watchFragmentY = 0;

    //  Set initial state to reset.
    state = RAST_RESET;
}

//  Function called from the the Generic ROP clock after updating the cache.
void ColorWriteV2::postCacheUpdate(u64bit cycle)
{
    //  Nothing to do here
}

//  Function called from the Generic ROP clock in reset state.
void ColorWriteV2::reset()
{
    //  Reset Color Write registers to default values.
    hRes = 400;
    vRes = 400;
    startX = 0;
    startY = 0;
    width = 400;
    height = 400;
    logicOperation = false;
    logicOpMode = LOGICOP_COPY;
    multisampling = false;
    msaaSamples = 2;
    colorBufferFormat = GPU_RGBA8888;
    compression = true;
    colorSRGBWrite = false;

    //  Reset color clear value.
    clearColor[0] = 0.0f;
    clearColor[1] = 0.0f;
    clearColor[2] = 0.0f;
    clearColor[3] = 1.0f;
    for(u32bit i = 0; i < MAX_BYTES_PER_COLOR; i++)
        clearColorData[i] = 0x00;
    clearColorData[3] = 0xff;
    
    //  By default set front buffer at 1 MB and back buffer at 2 MB.
    frontBuffer = 0x200000;
    backBuffer = 0x400000;

    //  Check if the latenty map is enabled.
    if (fragmentMapMode != DISABLE_MAP)
    {
        //  Allocate space for the latency map.
        latencyMap = new u32bit[((hRes >> 1) + (hRes & 0x01)) * ((vRes >> 1) + (vRes & 0x01))];

        //  Clear latency map.
        memset(latencyMap, 0, ((hRes >> 1) + (hRes & 0x01)) * ((vRes >> 1) + (vRes & 0x01)) * 4);

        //  Clear the latency map before the next batch.
        clearLatencyMap = false;
    }
        
    //  Set the ROP data buffer address.
    bufferAddress[0] = backBuffer;
    stateBufferAddress = 0x00000000;
  
    //  Set render target registers.
    for(u32bit i = 0; i < MAX_RENDER_TARGETS; i++)
    {
        rtEnable[i] = false;
        rtFormat[i] = GPU_R32F;
        rtAddress[i] = 0x600000 + 0x100000 * i;
        blend[i] = false;
        equation[i] = BLEND_FUNC_ADD;
        srcRGB[i] = BLEND_ONE;
        srcAlpha[i] = BLEND_ONE;
        dstRGB[i] = BLEND_ZERO;
        dstAlpha[i] = BLEND_ZERO;
        constantColor[i][0] = 0.0;
        constantColor[i][1] = 0.0;
        constantColor[i][2] = 0.0;
        constantColor[i][3] = 0.0;
        writeR[i] = writeG[i] = writeB[i] = writeA[i] = true;
        bytesPixel[i] = 4;

        //  Set bypass and enable read flags.
        bypassROP[i] = !writeR[i] && !writeG[i] && !writeB[i] && !writeA[i];
        readDataROP[i] = blend[i] || ((i == 0) && logicOperation);

        activeBuffer[i] = false;
    }

    
    //  The backbuffer is aliased to render target 0.
    rtEnable[0] = true;
    rtFormat[0] = colorBufferFormat;
    rtAddress[0] = backBuffer;
    activeBuffer[0] = true;
    numActiveBuffers = 1;
}

void ColorWriteV2::operateStamp(u64bit cycle, ROPQueue *stamp)
{
    Fragment *fr;
    QuadFloat *attrib;
    QuadFloat destColorQF[STAMP_FRAGMENTS * MAX_MSAA_SAMPLES * MAX_RENDER_TARGETS];
    QuadFloat inputColorQF[STAMP_FRAGMENTS * MAX_MSAA_SAMPLES * MAX_RENDER_TARGETS];

    bool traceFragmentMatch = false;

    //  Get input color from fragments
    //  Get the input depth data from the fragments.
    for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
    {
        //  Get fragment attributes.
        attrib = stamp->stamp[f]->getAttributes();

        //  Check if fragment tracing is enabled.
        if (traceFragment)
        {
            //  Detect the fragment to trace.
            traceFragmentMatch = traceFragmentMatch || (stamp->stamp[f]->getFragment()->getX() == watchFragmentX) &&
                                 (stamp->stamp[f]->getFragment()->getY() == watchFragmentY);
                
        }
        
        //  Check if the current fragment matches the fragment to trace.
        if (traceFragmentMatch)
        {
            printf("%s => Start processing pixel (%d, %d) triangle ID %d\n", getName(),
                stamp->stamp[f]->getFragment()->getX(), stamp->stamp[f]->getFragment()->getY(),
                stamp->stamp[f]->getTriangleID());
        }
        
        //  Get fragment.
        fr = stamp->stamp[f]->getFragment();

        GPU_DEBUG_BOX(
            printf("%s (%lld) => Pixel (%d, %d)\n", getName(), cycle, fr->getX(), fr->getY());
        )
        
        //  Check if multisampling is enabled.
        if (!multisampling)
        {
            //  For all render targets.
            for(u32bit rt = 0, activeRT = 0; activeRT < numActiveBuffers; rt++, activeRT++)
            {
                //  Search for the next active render target.
                for(; (!activeBuffer[rt]) && (rt < MAX_RENDER_TARGETS); rt++);
                
                GPU_ASSERT(
                    if (rt == MAX_RENDER_TARGETS)
                        panic("ColorWriteV2", "operateStamp", "Expected an active render target.");
                )
                
                if (traceFragmentMatch)
                {
                    printf("%s => Pixel (%d, %d) triangle ID %d color before clamping {%f, %f, %f, %f}\n", getName(),
                        stamp->stamp[f]->getFragment()->getX(), stamp->stamp[f]->getFragment()->getY(), stamp->stamp[f]->getTriangleID(),
                        attrib[COLOR_ATTRIBUTE + rt][0], attrib[COLOR_ATTRIBUTE + rt][1],
                        attrib[COLOR_ATTRIBUTE + rt][2], attrib[COLOR_ATTRIBUTE + rt][3]);
                }

                //  Get fragment input color from color attribute.
                if (rtFormat[rt] == GPU_RGBA8888)
                {
                    inputColorQF[rt * STAMP_FRAGMENTS + f][0] = GPU_CLAMP(attrib[COLOR_ATTRIBUTE + rt][0], 0.0f, 1.0f);
                    inputColorQF[rt * STAMP_FRAGMENTS + f][1] = GPU_CLAMP(attrib[COLOR_ATTRIBUTE + rt][1], 0.0f, 1.0f);
                    inputColorQF[rt * STAMP_FRAGMENTS + f][2] = GPU_CLAMP(attrib[COLOR_ATTRIBUTE + rt][2], 0.0f, 1.0f);
                    inputColorQF[rt * STAMP_FRAGMENTS + f][3] = GPU_CLAMP(attrib[COLOR_ATTRIBUTE + rt][3], 0.0f, 1.0f);
                }
                else
                {
                    inputColorQF[rt * STAMP_FRAGMENTS + f][0] = attrib[COLOR_ATTRIBUTE + rt][0];
                    inputColorQF[rt * STAMP_FRAGMENTS + f][1] = attrib[COLOR_ATTRIBUTE + rt][1];
                    inputColorQF[rt * STAMP_FRAGMENTS + f][2] = attrib[COLOR_ATTRIBUTE + rt][2];
                    inputColorQF[rt * STAMP_FRAGMENTS + f][3] = attrib[COLOR_ATTRIBUTE + rt][3];
                }

                if (traceFragmentMatch)
                {
                    printf("%s => Pixel (%d, %d) triangle ID %d color after clamping {%f, %f, %f, %f}\n", getName(),
                        stamp->stamp[f]->getFragment()->getX(), stamp->stamp[f]->getFragment()->getY(), stamp->stamp[f]->getTriangleID(),
                        inputColorQF[rt * STAMP_FRAGMENTS + f][0], inputColorQF[rt * STAMP_FRAGMENTS + f][1],
                        inputColorQF[rt * STAMP_FRAGMENTS + f][2], inputColorQF[rt * STAMP_FRAGMENTS + f][3]);
                }

                GPU_DEBUG_BOX(
                    printf("%s => Color {%f, %f, %f, %f}\n", getName(), inputColorQF[rt * STAMP_FRAGMENTS + f][0], 
                        inputColorQF[rt * STAMP_FRAGMENTS + f][1], inputColorQF[rt * STAMP_FRAGMENTS + f][2],
                        inputColorQF[rt * STAMP_FRAGMENTS + f][3]);
                )
                
                //  Build write mask for the stamp.
                switch(rtFormat[rt])
                {
                    case GPU_RGBA8888:
                        stamp->mask[(rt * STAMP_FRAGMENTS + f) * 4 + 0] = writeR[rt] && !stamp->culled[f];
                        stamp->mask[(rt * STAMP_FRAGMENTS + f) * 4 + 1] = writeG[rt] && !stamp->culled[f];
                        stamp->mask[(rt * STAMP_FRAGMENTS + f) * 4 + 2] = writeB[rt] && !stamp->culled[f];
                        stamp->mask[(rt * STAMP_FRAGMENTS + f) * 4 + 3] = writeA[rt] && !stamp->culled[f];
                        
                        GPU_DEBUG_BOX(
                            printf("%s => Stamp write mask : (%d, %d, %d, %d)\n",  getName(),
                                stamp->mask[(rt * STAMP_FRAGMENTS + f) * 4],
                                stamp->mask[(rt * STAMP_FRAGMENTS + f) * 4 + 1],
                                stamp->mask[(rt * STAMP_FRAGMENTS + f) * 4 + 2],
                                stamp->mask[(rt * STAMP_FRAGMENTS + f) * 4 + 3]);
                        )
                        
                        break;
                        
                    case GPU_RG16F:
                        stamp->mask[(rt * STAMP_FRAGMENTS + f) * 4 + 0] =
                        stamp->mask[(rt * STAMP_FRAGMENTS + f) * 4 + 1] = writeR[rt] && !stamp->culled[f];
                        stamp->mask[(rt * STAMP_FRAGMENTS + f) * 4 + 2] =
                        stamp->mask[(rt * STAMP_FRAGMENTS + f) * 4 + 3] = writeG[rt] && !stamp->culled[f];

                        GPU_DEBUG_BOX(
                            printf("%s => Stamp write mask : (%d, %d, %d, %d)\n",  getName(),
                                stamp->mask[(rt * STAMP_FRAGMENTS + f) * 4],
                                stamp->mask[(rt * STAMP_FRAGMENTS + f) * 4 + 1],
                                stamp->mask[(rt * STAMP_FRAGMENTS + f) * 4 + 2],
                                stamp->mask[(rt * STAMP_FRAGMENTS + f) * 4 + 3]);
                        )
                        
                        
                        break;

                    case GPU_R32F:
                        stamp->mask[(rt * STAMP_FRAGMENTS + f) * 4 + 0] =
                        stamp->mask[(rt * STAMP_FRAGMENTS + f) * 4 + 1] = 
                        stamp->mask[(rt * STAMP_FRAGMENTS + f) * 4 + 2] =
                        stamp->mask[(rt * STAMP_FRAGMENTS + f) * 4 + 3] = writeR[rt] && !stamp->culled[f];

                        GPU_DEBUG_BOX(
                            printf("%s => Stamp write mask : (%d, %d, %d, %d)\n",  getName(),
                                stamp->mask[(rt * STAMP_FRAGMENTS + f) * 4],
                                stamp->mask[(rt * STAMP_FRAGMENTS + f) * 4 + 1],
                                stamp->mask[(rt * STAMP_FRAGMENTS + f) * 4 + 2],
                                stamp->mask[(rt * STAMP_FRAGMENTS + f) * 4 + 3]);
                        )
                                                
                        break;                  
                        
                    case GPU_RGBA16:                    
                    case GPU_RGBA16F:                    
                        stamp->mask[(rt * STAMP_FRAGMENTS + f) * 8 + 0] =
                        stamp->mask[(rt * STAMP_FRAGMENTS + f) * 8 + 1] = writeR[rt] && !stamp->culled[f];
                        stamp->mask[(rt * STAMP_FRAGMENTS + f) * 8 + 2] =
                        stamp->mask[(rt * STAMP_FRAGMENTS + f) * 8 + 3] = writeG[rt] && !stamp->culled[f];
                        stamp->mask[(rt * STAMP_FRAGMENTS + f) * 8 + 4] =
                        stamp->mask[(rt * STAMP_FRAGMENTS + f) * 8 + 5] = writeB[rt] && !stamp->culled[f];
                        stamp->mask[(rt * STAMP_FRAGMENTS + f) * 8 + 6] =
                        stamp->mask[(rt * STAMP_FRAGMENTS + f) * 8 + 7] = writeA[rt] && !stamp->culled[f];

                        GPU_DEBUG_BOX(
                            printf("%s => Stamp write mask : (%d, %d, %d, %d, %d, %d, %d, %d)\n", getName(),
                                stamp->mask[(rt * STAMP_FRAGMENTS + f) * 8 + 0], stamp->mask[(rt * STAMP_FRAGMENTS + f) * 8 + 1],
                                stamp->mask[(rt * STAMP_FRAGMENTS + f) * 8 + 2], stamp->mask[(rt * STAMP_FRAGMENTS + f) * 8 + 3],
                                stamp->mask[(rt * STAMP_FRAGMENTS + f) * 8 + 4], stamp->mask[(rt * STAMP_FRAGMENTS + f) * 8 + 5],
                                stamp->mask[(rt * STAMP_FRAGMENTS + f) * 8 + 6], stamp->mask[(rt * STAMP_FRAGMENTS + f) * 8 + 7]);
                        )
                        
                        break;

                    default:
                        panic("ColorWriteV2", "operateStamp", "Unexpected render target format.");
                        break;                        
                }
            }                
        }
        else
        {
            //  Get sample coverage mask for the fragment.
            bool *sampleCoverage = stamp->stamp[f]->getFragment()->getMSAACoverage();
            
            //  For all render targets.
            for(u32bit rt = 0, activeRT = 0; activeRT < numActiveBuffers; rt++, activeRT++)
            {
                //  Search for the next active render target.
                for(; (!activeBuffer[rt]) && (rt < MAX_RENDER_TARGETS); rt++);
                
                GPU_ASSERT(
                    if (rt == MAX_RENDER_TARGETS)
                        panic("ColorWriteV2", "operateStamp", "Expected an active render target.");
                )
                
                //  Copy shaded fragment color to all samples for the fragment.
                for(u32bit s = 0; s < msaaSamples; s++)
                {
                    // Compute the offset inside the buffers for the current sample.
                    u32bit sampleOffset = rt * msaaSamples * STAMP_FRAGMENTS + f * msaaSamples + s;
                    
                    //  Get fragment input color from color attribute.
                    if (rtFormat[rt] == GPU_RGBA8888)
                    {
                        inputColorQF[sampleOffset][0] = GPU_CLAMP(attrib[COLOR_ATTRIBUTE + rt][0], 0.0f, 1.0f);
                        inputColorQF[sampleOffset][1] = GPU_CLAMP(attrib[COLOR_ATTRIBUTE + rt][1], 0.0f, 1.0f);
                        inputColorQF[sampleOffset][2] = GPU_CLAMP(attrib[COLOR_ATTRIBUTE + rt][2], 0.0f, 1.0f);
                        inputColorQF[sampleOffset][3] = GPU_CLAMP(attrib[COLOR_ATTRIBUTE + rt][3], 0.0f, 1.0f);
                    }
                    else
                    {
                        inputColorQF[sampleOffset][0] = attrib[COLOR_ATTRIBUTE + rt][0];
                        inputColorQF[sampleOffset][1] = attrib[COLOR_ATTRIBUTE + rt][1];
                        inputColorQF[sampleOffset][2] = attrib[COLOR_ATTRIBUTE + rt][2];
                        inputColorQF[sampleOffset][3] = attrib[COLOR_ATTRIBUTE + rt][3];
                    }

                    GPU_DEBUG_BOX(
                        printf("ColorWrite => Color {%f, %f, %f, %f}\n", inputColorQF[sampleOffset][0],
                            inputColorQF[sampleOffset][1], inputColorQF[sampleOffset][2],
                            inputColorQF[sampleOffset][3]);
                    )
                    
                    //  Build write mask for the fragment samples.
                    switch(rtFormat[rt])
                    {
                        case GPU_RGBA8888:
                            stamp->mask[sampleOffset * 4 + 0] = writeR[rt] && sampleCoverage[s];
                            stamp->mask[sampleOffset * 4 + 1] = writeG[rt] && sampleCoverage[s];
                            stamp->mask[sampleOffset * 4 + 2] = writeB[rt] && sampleCoverage[s];
                            stamp->mask[sampleOffset * 4 + 3] = writeA[rt] && sampleCoverage[s];
                    
                            GPU_DEBUG_BOX(
                                printf("%s => Stamp write mask for fragment %d sample %d : (%d, %d, %d, %d)\n",
                                    getName(), f, s,
                                    stamp->mask[sampleOffset * 4 + 0], stamp->mask[sampleOffset * 4 + 1],
                                    stamp->mask[sampleOffset * 4 + 2], stamp->mask[sampleOffset * 4 + 3]);
                            )
                            
                            break;
                            
                        case GPU_RG16F:
                            stamp->mask[sampleOffset * 4 + 0] = stamp->mask[sampleOffset * 4 + 1] = writeR[rt] && sampleCoverage[s];
                            stamp->mask[sampleOffset * 4 + 2] = stamp->mask[sampleOffset * 4 + 3] = writeG[rt] && sampleCoverage[s];
                    
                            GPU_DEBUG_BOX(
                                printf("%s => Stamp write mask for fragment %d sample %d : (%d, %d, %d, %d)\n",
                                    getName(), f, s,
                                    stamp->mask[sampleOffset * 4 + 0], stamp->mask[sampleOffset * 4 + 1],
                                    stamp->mask[sampleOffset * 4 + 2], stamp->mask[sampleOffset * 4 + 3]);
                            )
                            
                            break;

                        case GPU_R32F:
                            stamp->mask[sampleOffset * 4 + 0] = stamp->mask[sampleOffset * 4 + 1] = 
                            stamp->mask[sampleOffset * 4 + 2] = stamp->mask[sampleOffset * 4 + 3] = writeR[rt] && sampleCoverage[s];
                    
                            GPU_DEBUG_BOX(
                                printf("%s => Stamp write mask for fragment %d sample %d : (%d, %d, %d, %d)\n",
                                    getName(), f, s,
                                    stamp->mask[sampleOffset * 4 + 0], stamp->mask[sampleOffset * 4 + 1],
                                    stamp->mask[sampleOffset * 4 + 2], stamp->mask[sampleOffset * 4 + 3]);
                            )
                            
                            break;

                        case GPU_RGBA16:
                        case GPU_RGBA16F:
                            stamp->mask[sampleOffset * 8 + 0] = stamp->mask[sampleOffset * 8 + 1] = writeR[rt] && sampleCoverage[s];
                            stamp->mask[sampleOffset * 8 + 2] = stamp->mask[sampleOffset * 8 + 3] = writeG[rt] && sampleCoverage[s];
                            stamp->mask[sampleOffset * 8 + 4] = stamp->mask[sampleOffset * 8 + 5] = writeB[rt] && sampleCoverage[s];
                            stamp->mask[sampleOffset * 8 + 6] = stamp->mask[sampleOffset * 8 + 7] = writeA[rt] && sampleCoverage[s];
                    
                            GPU_DEBUG_BOX(
                                printf("ColorWrite => Stamp write mask for fragment %d sample %d : (%d, %d, %d, %d, %d, %d, %d, %d)\n", f, s,
                                    stamp->mask[sampleOffset * 8 + 0], stamp->mask[sampleOffset * 8 + 1],
                                    stamp->mask[sampleOffset * 8 + 2], stamp->mask[sampleOffset * 8 + 3],
                                    stamp->mask[sampleOffset * 8 + 4], stamp->mask[sampleOffset * 8 + 5],
                                    stamp->mask[sampleOffset * 8 + 6], stamp->mask[sampleOffset * 8 + 7]);
                            )
                            
                            break;
                            
                        default:
                            panic("ColorWriteV2", "operateStamp", "Unexpected render target format.");
                            break;                        
                    }
                }
            }
        }        
    }

    //  Check if the current fragment matches the fragment to trace.
    if (traceFragmentMatch)
    {
        printf("%s => Start reading data for pixel (%d, %d) triangle ID %d.\n", getName(), watchFragmentX,
            watchFragmentY, stamp->stamp[0]->getTriangleID());
    }
    
    //  For all render targets.
    for(u32bit rt = 0, activeRT = 0; activeRT < numActiveBuffers; rt++, activeRT++)
    {
        //  Search for the next active render target.
        for(; (!activeBuffer[rt]) && (rt < MAX_RENDER_TARGETS); rt++);
        
        GPU_ASSERT(
            if (rt == MAX_RENDER_TARGETS)
                panic("ColorWriteV2", "operateStamp", "Expected an active render target.");
        )
        
        //  Check if multisampling is enabled.
        if (!multisampling)
        {
            //  Compute offset inside the buffers for the current samples.
            u32bit dataInOffset = rt * STAMP_FRAGMENTS * bytesPixel[rt];
            u32bit dataOutOffset = rt * STAMP_FRAGMENTS;
            
            //  Convert color buffer data to internal representation (RGBA32F).
            switch(rtFormat[rt])
            {
                case GPU_RGBA8888:
                
                    colorRGBA8ToRGBA32F(&stamp->data[dataInOffset], &destColorQF[dataOutOffset]);
                    break;
                    
                case GPU_RG16F:
                
                    colorRG16FToRGBA32F(&stamp->data[dataInOffset], &destColorQF[dataOutOffset]);
                    break;

                case GPU_R32F:
                
                    colorR32FToRGBA32F(&stamp->data[dataInOffset], &destColorQF[dataOutOffset]);
                    break;

                case GPU_RGBA16:
                    colorRGBA16ToRGBA32F(&stamp->data[dataInOffset], &destColorQF[dataOutOffset]);
                    break;                

                case GPU_RGBA16F:
                
                    colorRGBA16FToRGBA32F(&stamp->data[dataInOffset], &destColorQF[dataOutOffset]);
                    break;

                default:
                    panic("ColorWriteV2", "operateStamp", "Unexpected render target format.");
                    break;                        
            }

            //  Convert from sRGB to linear space if required.
            if (colorSRGBWrite)
                colorSRGBToLinear(&destColorQF[dataOutOffset]);

            //  Check if the current fragment matches the fragment to trace.
            if (traceFragmentMatch)
            {
                printf("%s => Read data (no MSAA) for pixel (%d, %d) triangle ID %d.\n", getName(), watchFragmentX,
                    watchFragmentY, stamp->stamp[0]->getTriangleID());
            }
        }
        else
        {
            //  Convert fixed point color data to float point data for all the samples in the stamp
            for(u32bit s = 0; s < msaaSamples; s++)
            {
                //  Compute offset inside the buffers for the current samples.
                u32bit dataInOffset = rt * STAMP_FRAGMENTS * msaaSamples * bytesPixel[rt] +
                                      s * STAMP_FRAGMENTS * bytesPixel[rt];
                u32bit dataOutOffset = rt * STAMP_FRAGMENTS * msaaSamples + s * STAMP_FRAGMENTS;
            
                //  Convert color buffer data to internal representation (RGBA32F).
                switch(rtFormat[rt])
                {
                    case GPU_RGBA8888:
                    
                        colorRGBA8ToRGBA32F(&stamp->data[dataInOffset], &destColorQF[dataOutOffset]);
                        break;
                        
                    case GPU_RG16F:
                    
                        colorRG16FToRGBA32F(&stamp->data[dataInOffset], &destColorQF[dataOutOffset]);
                        break;
                        
                    case GPU_R32F:
                    
                        colorR32FToRGBA32F(&stamp->data[dataInOffset], &destColorQF[dataOutOffset]);
                        break;
                        
                    case GPU_RGBA16:
                    
                        colorRGBA16ToRGBA32F(&stamp->data[dataInOffset], &destColorQF[dataOutOffset]);
                        break;

                    case GPU_RGBA16F:
                    
                        colorRGBA16FToRGBA32F(&stamp->data[dataInOffset], &destColorQF[dataOutOffset]);
                        break;

                    default:
                        panic("ColorWriteV2", "operateStamp", "Unexpected render target format.");
                        break;                        
                }
            
                //  Convert from sRGB to linear space if required.
                if (colorSRGBWrite)
                    colorSRGBToLinear(&destColorQF[dataOutOffset]);
            }
            
            //  Check if the current fragment matches the fragment to trace.
            if (traceFragmentMatch)
            {
                printf("%s => Read data (MSAA) for pixel (%d, %d) triangle ID %d.\n", getName(), watchFragmentX,
                    watchFragmentY, stamp->stamp[0]->getTriangleID());
            }
        }       
    }
    
    //  Check if the current fragment matches the fragment to trace.
    if (traceFragmentMatch)
    {
        printf("%s => Start blending for pixel (%d, %d) triangle ID %d.\n", getName(), watchFragmentX,
            watchFragmentY, stamp->stamp[0]->getTriangleID());
    }

    //  For all render targets.
    for(u32bit rt = 0, activeRT = 0; activeRT < numActiveBuffers; rt++, activeRT++)
    {
        //  Search for the next active render target.
        for(; (!activeBuffer[rt]) && (rt < MAX_RENDER_TARGETS); rt++);
        
        GPU_ASSERT(
            if (rt == MAX_RENDER_TARGETS)
                panic("ColorWriteV2", "operateStamp", "Expected an active render target.");
        )
        
        FragmentQuadMemoryUpdateInfo quadMemUpdate;

        //  Determine if blend mode is active.
        if (blend[rt])
        {
            GPU_DEBUG_BOX(
                printf("ColorWrite => Blending stamp.\n");
            
                for(u32bit i = 0; i < STAMP_FRAGMENTS; i++)
                    printf(">>  Input Color = { %f, %f, %f %f }\n", inputColorQF[i][0], inputColorQF[i][1],
                        inputColorQF[i][2], inputColorQF[i][3]);
                for(u32bit i = 0; i < STAMP_FRAGMENTS; i++)
                    printf(">>  Dest Color = { %f, %f, %f %f }\n", destColorQF[i][0], destColorQF[i][1],
                        destColorQF[i][2], destColorQF[i][3]);
            )

            //  Check if multisampling is enabled.
            if (!multisampling)
            {                
                //  Compute offset inside the buffers for the current samples.
                u32bit dataOffset = rt * STAMP_FRAGMENTS;
                u32bit dataInOffset = rt * STAMP_FRAGMENTS * bytesPixel[rt];
                
                //  Check if validation mode is enabled.
                if (validationMode)
                {
                    //  Check if there are unculled fragments in the quad.
                    bool anyNotAlreadyCulled = false;            
                    for(u32bit f = 0; (f < STAMP_FRAGMENTS) && !anyNotAlreadyCulled; f++)
                        anyNotAlreadyCulled = anyNotAlreadyCulled || !stamp->culled[f];

                    //  Store the information for the quad if there is a valid fragment.
                    if (anyNotAlreadyCulled)
                    {
                        //  Set the quad identifier to the top left fragment of the quad.
                        quadMemUpdate.fragID.x = stamp->stamp[0]->getFragment()->getX();
                        quadMemUpdate.fragID.y = stamp->stamp[0]->getFragment()->getY();
                        quadMemUpdate.fragID.triangleID = stamp->stamp[0]->getTriangleID();
                        
                        quadMemUpdate.fragID.sample = 0;

                        //  Copy the input (color computed per fragment) and read data (color from the render target) for
                        //  the quad.
                        for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
                        {
                            ((f32bit *) quadMemUpdate.inData)[f * 4 + 0] = inputColorQF[dataOffset + f][0];
                            ((f32bit *) quadMemUpdate.inData)[f * 4 + 1] = inputColorQF[dataOffset + f][1];
                            ((f32bit *) quadMemUpdate.inData)[f * 4 + 2] = inputColorQF[dataOffset + f][2];
                            ((f32bit *) quadMemUpdate.inData)[f * 4 + 3] = inputColorQF[dataOffset + f][3];
                        }
                        for(u32bit qw = 0; qw < ((bytesPixel[rt] * 4) >> 3); qw++)
                            ((u64bit *) quadMemUpdate.readData)[qw] = ((u64bit *) &stamp->data[dataInOffset])[qw];

                        if (traceFragmentMatch)
                        {
                            printf("%s => Read Data from @(%08x, %d, %d) : %08x %08x %08x %08x\n", getName(),
                                stamp->address[rt], stamp->way[rt], stamp->line[rt],
                                ((u32bit *) quadMemUpdate.readData)[0], ((u32bit *) quadMemUpdate.readData)[1],
                                ((u32bit *) quadMemUpdate.readData)[2], ((u32bit *) quadMemUpdate.readData)[3]);
                        }
                    }
                }

                /*  Perform blend operation.  */
                frEmu.blend(rt, &inputColorQF[dataOffset], &inputColorQF[dataOffset], &destColorQF[dataOffset]);

                //  Check if the current fragment matches the fragment to trace.
                if (traceFragmentMatch)
                {
                    printf("%s => Blend (no MSAA) for pixel (%d, %d) triangle ID %d.\n", getName(), watchFragmentX,
                        watchFragmentY, stamp->stamp[0]->getTriangleID());
                }
            }
            else
            {
                //  Perform blending for all the samples in the stamp.
                for(u32bit s = 0; s < msaaSamples; s++)
                {
                    //  Compute offset inside the buffers for the current samples.
                    u32bit dataOffset = rt * msaaSamples * STAMP_FRAGMENTS + STAMP_FRAGMENTS * s;
                    
                    //  Perform blending for a group of samples.
                    frEmu.blend(rt, &inputColorQF[dataOffset], &inputColorQF[dataOffset], &destColorQF[dataOffset]);
                }

                //  Check if the current fragment matches the fragment to trace.
                if (traceFragmentMatch)
                {
                    printf("%s => Blend (MSAA) data for pixel (%d, %d) triangle ID %d.\n", getName(), watchFragmentX,
                        watchFragmentY, stamp->stamp[0]->getTriangleID());
                }
            }
            
            GPU_DEBUG_BOX(        
                for(u32bit i = 0; i < STAMP_FRAGMENTS; i++)
                    printf("<<  Input Color = { %f, %f, %f %f }\n", inputColorQF[i][0], inputColorQF[i][1],
                        inputColorQF[i][2], inputColorQF[i][3]);
            )

            //  Update statistics.
            blended->inc(STAMP_FRAGMENTS);
        }
        else
        {
            //  Compute offset inside the buffers for the current samples.
            u32bit dataOffset = rt * STAMP_FRAGMENTS;
            u32bit dataInOffset = rt * STAMP_FRAGMENTS * bytesPixel[rt];
            
            //  Check if validation mode is enabled.
            if (validationMode)
            {
                //  Check if there are unculled fragments in the quad.
                bool anyNotAlreadyCulled = false;            
                for(u32bit f = 0; (f < STAMP_FRAGMENTS) && !anyNotAlreadyCulled; f++)
                    anyNotAlreadyCulled = anyNotAlreadyCulled || !stamp->culled[f];

                //  Store the information for the quad if there is a valid fragment.
                if (anyNotAlreadyCulled)
                {
                    //  Set the quad identifier to the top left fragment of the quad.
                    quadMemUpdate.fragID.x = stamp->stamp[0]->getFragment()->getX();
                    quadMemUpdate.fragID.y = stamp->stamp[0]->getFragment()->getY();
                    quadMemUpdate.fragID.triangleID = stamp->stamp[0]->getTriangleID();
                    
                    quadMemUpdate.fragID.sample = 0;

                    //  Copy the input (color computed per fragment) and read data (color from the render target) for
                    //  the quad.
                    for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
                    {
                        ((f32bit *) quadMemUpdate.inData)[f * 4 + 0] = inputColorQF[dataOffset + f][0];
                        ((f32bit *) quadMemUpdate.inData)[f * 4 + 1] = inputColorQF[dataOffset + f][1];
                        ((f32bit *) quadMemUpdate.inData)[f * 4 + 2] = inputColorQF[dataOffset + f][2];
                        ((f32bit *) quadMemUpdate.inData)[f * 4 + 3] = inputColorQF[dataOffset + f][3];
                    }
                    
                    for(u32bit qw = 0; qw < ((bytesPixel[rt] * 4) >> 3); qw++)
                        ((u64bit *) quadMemUpdate.readData)[qw] = 0xDEFADA7ADEFADA7AULL;

                    if (traceFragmentMatch)
                    {
                        printf("%s => Read Data from @(%08x, %d, %d) : %08x %08x %08x %08x\n", getName(),
                            stamp->address[rt], stamp->way[rt], stamp->line[rt],
                            ((u32bit *) quadMemUpdate.readData)[0], ((u32bit *) quadMemUpdate.readData)[1],
                            ((u32bit *) quadMemUpdate.readData)[2], ((u32bit *) quadMemUpdate.readData)[3]);
                    }
                }
            }
            
            //  Check if the current fragment matches the fragment to trace.
            if (traceFragmentMatch)
            {
                printf("%s => No blend for pixel (%d, %d) triangle ID %d.\n", getName(), watchFragmentX,
                    watchFragmentY, stamp->stamp[0]->getTriangleID());
            }
        }

        //  Determine if logical operation is active (only for back buffer/render target 0).
        if ((rt == 0) && logicOperation)
        {
            GPU_DEBUG_BOX(
                printf("ColorWrite => LogicOp stamp.\n");
            )

            GPU_ASSERT(
                if (colorBufferFormat != GPU_RGBA8888)
                    panic("ColorWrite", "operateStamp", "Logic operation only supported with RGBA8 color buffer format.");
            )
            
            //  Check if multisampling is enabled
            if (!multisampling)
            {
                //  Convert from linear to sRGB color space on color write if required.
                if (colorSRGBWrite)
                    colorLinearToSRGB(inputColorQF);
                    
                //  Convert the stamp color data to integer format.
                colorRGBA32FToRGBA8(inputColorQF, outColor);

                //  Perform logical operation.
                frEmu.logicOp(outColor, outColor, stamp->data);
            }
            else
            {
                //  Convert to integer format and perform logical op for all the samples in the stamp.
                for(u32bit j = 0; j < msaaSamples; j++)
                {
                    //  Convert from linear to sRGB color space on color write if required.
                    if (colorSRGBWrite)
                        colorLinearToSRGB(&inputColorQF[STAMP_FRAGMENTS * j]);
                        
                    //  Convert the sample color data to integer format.
                    colorRGBA32FToRGBA8(&inputColorQF[STAMP_FRAGMENTS * j], &outColor[STAMP_FRAGMENTS * j * 4]);

                    //  Perform logical operation for a group of samples.
                    frEmu.logicOp(&outColor[STAMP_FRAGMENTS * j * 4], &outColor[STAMP_FRAGMENTS * j * 4], &stamp->data[STAMP_FRAGMENTS * j * 4]);
                }
            }
            
            //  Update statistics.
            logoped->inc(STAMP_FRAGMENTS);

            //  Check if the current fragment matches the fragment to trace.
            if (traceFragmentMatch)
            {
                printf("%s => Performed Logic Op for pixel (%d, %d) triangle ID %d.\n", getName(), watchFragmentX,
                    watchFragmentY, stamp->stamp[0]->getTriangleID());
            }
        }
        else
        {
            //  Check if multisampling is enabled.
            if (!multisampling)
            {
                //  Compute offset inside the buffers for the current samples.
                u32bit dataInOffset = rt * STAMP_FRAGMENTS;
                u32bit dataOutOffset = rt * STAMP_FRAGMENTS * bytesPixel[rt];
                
                //  Convert from linear to sRGB color space on color write if required.
                if (colorSRGBWrite)
                    colorLinearToSRGB(&inputColorQF[dataInOffset]);

                //  Convert the stamp color data in internal format to the color buffer format.
                switch(rtFormat[rt])
                {
                    case GPU_RGBA8888:
                    
                        colorRGBA32FToRGBA8(&inputColorQF[dataInOffset], &stamp->data[dataOutOffset]);
                        
                        GPU_DEBUG_BOX(
                            u32bit *data = (u32bit *) &stamp->data[dataOutOffset];
                            printf("%s => Output data for rt %d : %08x %08x %08x %08x\n", getName(), rt, 
                                data[0], data[1], data[2], data[3]);
                        )
                        
                        break;
                        
                    case GPU_RG16F:
                    
                        colorRGBA32FToRG16F(&inputColorQF[dataInOffset], &stamp->data[dataOutOffset]);

                        GPU_DEBUG_BOX(
                            u32bit *data = (u32bit *) &stamp->data[dataOutOffset];
                            printf("%s => Output data for rt %d : %08x %08x %08x %08x\n", getName(), rt, 
                                data[0], data[1], data[2], data[3]);
                        )
                        break;
                        
                    case GPU_R32F:
                    
                        colorRGBA32FToR32F(&inputColorQF[dataInOffset], &stamp->data[dataOutOffset]);

                        GPU_DEBUG_BOX(
                            u32bit *data = (u32bit *) &stamp->data[dataOutOffset];
                            printf("%s => Output data for rt %d : %08x %08x %08x %08x\n", getName(), rt, 
                                data[0], data[1], data[2], data[3]);
                        )
                        break;
                        
                    case GPU_RGBA16:
                    
                        colorRGBA32FToRGBA16(&inputColorQF[dataInOffset], &stamp->data[dataOutOffset]);

                        GPU_DEBUG_BOX(
                            u32bit *data = (u32bit *) &stamp->data[dataOutOffset];
                            printf("%s => Output data for rt %d : %08x %08x %08x %08x %08x %08x %08x %08x\n", getName(), rt, 
                                data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
                        )
                        break;

                    case GPU_RGBA16F:
                    
                        colorRGBA32FToRGBA16F(&inputColorQF[dataInOffset], &stamp->data[dataOutOffset]);
                        
                        GPU_DEBUG_BOX(
                            u32bit *data = (u32bit *) &stamp->data[dataOutOffset];
                            printf("%s => Output data for rt %d : %08x %08x %08x %08x %08x %08x %08x %08x\n", getName(), rt, 
                                data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
                        )
                        break;

                    default:
                        panic("ColorWriteV2", "operateStamp", "Unexpected render target format.");
                        break;                        
                }

                //  Check if the current fragment matches the fragment to trace.
                if (traceFragmentMatch)
                {
                    printf("%s => Data update (no MSAA) or pixel (%d, %d) triangle ID %d. validationMode = %s\n", getName(), watchFragmentX,
                        watchFragmentY, stamp->stamp[0]->getTriangleID(), validationMode ? "T" : "F");
                }

                if (validationMode)
                {
                    //  Check if a write is performed (fragment not culled).
                    bool anyNotCulled = false;                       
                    for(u32bit f = 0; (f < STAMP_FRAGMENTS) && !anyNotCulled; f++)
                        anyNotCulled = anyNotCulled || !stamp->culled[f];
                                        
                    //  Check if the current fragment matches the fragment to trace.
                    if (traceFragmentMatch)
                    {
                        printf("%s => Checking if pixel (%d, %d) triangle ID %d has to be added to the Memory Update Map.\n", getName(), watchFragmentX,
                            watchFragmentY, stamp->stamp[0]->getTriangleID());
                        printf("%s => anyNotCulled = %s | culled = (%s %s %s %s)\n", getName(),
                            anyNotCulled ? "T" : "F",
                            stamp->culled[0] ? "T" : "F",
                            stamp->culled[1] ? "T" : "F",
                            stamp->culled[2] ? "T" : "F",
                            stamp->culled[3] ? "T" : "F");
                    }
                    
                    //  Store information for the quad and add to the z/stencil memory update map.
                    if (anyNotCulled)
                    {
                        //  Store the result z and stencil results for the quad.
                        for(u32bit qw = 0; qw < ((bytesPixel[rt] * STAMP_FRAGMENTS) >> 3) ; qw++)
                            ((u64bit *) quadMemUpdate.writeData)[qw] = ((u64bit *) &stamp->data[dataOutOffset])[qw];
                            
                        //  Store the write mask for the quad.
                        for(u32bit b = 0; b < (STAMP_FRAGMENTS * bytesPixel[rt]); b++)
                            quadMemUpdate.writeMask[b] = stamp->mask[dataOutOffset + b];
                            
                        //  Store the cull mask for the quad.
                        for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
                            quadMemUpdate.cullMask[f] = stamp->culled[f];
                            
                        //  Store the information for the quad in the z stencil memory update map.
                        colorMemoryUpdateMap[rt].insert(make_pair(quadMemUpdate.fragID, quadMemUpdate));

                        //  Check if the current fragment matches the fragment to trace.
                        if (traceFragmentMatch)
                        {
                            printf("%s => Adding pixel (%d, %d) triangle ID %d to Memory Update Map | rt = %d | FragID = (%d, %d, %d)\n",
                                getName(), watchFragmentX, watchFragmentY, stamp->stamp[0]->getTriangleID(),
                                rt, quadMemUpdate.fragID.x, quadMemUpdate.fragID.y, quadMemUpdate.fragID.triangleID);
                            printf("%s => Out Data : %08x %08x %08x %08x\n", getName(),
                                ((u32bit *) quadMemUpdate.writeData)[0], ((u32bit *) quadMemUpdate.writeData)[1],
                                ((u32bit *) quadMemUpdate.writeData)[2], ((u32bit *) quadMemUpdate.writeData)[3]);
                            printf("%s => Mask : ", getName());
                            for(u32bit b = 0; b < (STAMP_FRAGMENTS * bytesPixel[rt]); b++)
                                printf("%s ", stamp->mask[dataOutOffset + b] ? "T": "F");
                            printf("\n");
                        }
                    }
                }
            }
            else
            {
                //  Convert the sample color data for all the stamp.
                for(u32bit s = 0; s < msaaSamples; s++)
                {
                    //  Compute offset inside the buffers for the current samples.
                    u32bit dataInOffset = rt * msaaSamples * STAMP_FRAGMENTS + s * STAMP_FRAGMENTS;
                    u32bit dataOutOffset = rt * msaaSamples * STAMP_FRAGMENTS * bytesPixel[rt] +
                                           s * STAMP_FRAGMENTS * bytesPixel[rt];

                    //  Convert from linear to sRGB color space on color write if required.
                    if (colorSRGBWrite)
                        colorLinearToSRGB(&inputColorQF[dataInOffset]);
                        
                    //  Convert the stamp color data in internal format to the color buffer format.
                    switch(rtFormat[rt])
                    {
                        case GPU_RGBA8888:
                        
                            colorRGBA32FToRGBA8(&inputColorQF[dataInOffset], &stamp->data[dataOutOffset]);

                            GPU_DEBUG_BOX(
                                u32bit *data = (u32bit *) &stamp->data[dataOutOffset];
                                printf("%s => Output data for sample %d for rt %d : %08x %08x %08x %08x\n",
                                    getName(), s, rt, 
                                    data[0], data[1], data[2], data[3]);
                            )

                            break;
                            
                        case GPU_RG16F:
                        
                            colorRGBA32FToRG16F(&inputColorQF[dataInOffset], &stamp->data[dataOutOffset]);

                            GPU_DEBUG_BOX(
                                u32bit *data = (u32bit *) &stamp->data[dataOutOffset];
                                printf("%s => Output data for sample %d for rt %d : %08x %08x %08x %08x\n",
                                    getName(), s, rt, 
                                    data[0], data[1], data[2], data[3]);
                            )

                            break;
                            
                        case GPU_R32F:
                        
                            colorRGBA32FToR32F(&inputColorQF[dataInOffset], &stamp->data[dataOutOffset]);

                            GPU_DEBUG_BOX(
                                u32bit *data = (u32bit *) &stamp->data[dataOutOffset];
                                printf("%s => Output data for sample %d for rt %d : %08x %08x %08x %08x\n",
                                    getName(), s, rt, 
                                    data[0], data[1], data[2], data[3]);
                            )

                            break;

                        case GPU_RGBA16:
                        
                            colorRGBA32FToRGBA16(&inputColorQF[dataInOffset], &stamp->data[dataOutOffset]);

                            GPU_DEBUG_BOX(
                                u32bit *data = (u32bit *) &stamp->data[dataOutOffset];
                                printf("%s => Output data for sample %d for rt %d : %08x %08x %08x %08x %08x %08x %08x %08x\n",
                                    getName(), s, rt, 
                                    data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
                            )

                            break;

                        case GPU_RGBA16F:
                        
                            colorRGBA32FToRGBA16F(&inputColorQF[dataInOffset], &stamp->data[dataOutOffset]);
                            
                            GPU_DEBUG_BOX(
                                u32bit *data = (u32bit *) &stamp->data[dataOutOffset];
                                printf("%s => Output data for sample %d for rt %d : %08x %08x %08x %08x %08x %08x %08x %08x\n",
                                    getName(), s, rt, 
                                    data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
                            )
                            
                            break;

                    default:
                        panic("ColorWriteV2", "operateStamp", "Unexpected render target format.");
                        break;                        
                    }
                }

                //  Check if the current fragment matches the fragment to trace.
                if (traceFragmentMatch)
                {
                    printf("%s => Data update (MSAA) or pixel (%d, %d) triangle ID %d.\n", getName(), watchFragmentX,
                        watchFragmentY, stamp->stamp[0]->getTriangleID());
                }

            }
        }
    }
}

//  This function is called for stamps
void ColorWriteV2::postWriteProcess(u64bit cycle, ROPQueue *stamp)
{
    Fragment *fr;
    QuadFloat *attrib;
    s32bit x, y;
    u64bit startCycle;
    u32bit shaderLatency;

    //  Get fragment.
    fr = stamp->stamp[0]->getFragment();

    /*  Get fragment coordinates.  */
    //x = (s32bit) attrib[POSITION_ATTRIBUTE][0];
    //y = (s32bit) attrib[POSITION_ATTRIBUTE][1];
    x = fr->getX();
    y = fr->getY();

//if ((x == 0) && (y == 144))
//printf("ColorWriteV2 => Color at (0, 144) is (%02x, %02x, %02x, %02x)\n", stamp->data[0], stamp->data[1], stamp->data[2], stamp->data[3]);
///printf("%s => Color at (%d, %d is (%02x, %02x, %02x, %02x)\n",
//getName(), x, y,
//stamp->data[0], stamp->data[1], stamp->data[2], stamp->data[3]);


    //  Get stamp fragment cycle.
    startCycle = stamp->stamp[0]->getStartCycle();

    //  Set stamp latency in the shader units.  */
    shaderLatency = stamp->stamp[0]->getShaderLatency();

    //  Update the latency map if enabled and destroy the stamp fragments and attributes
    for(int i = 0; i < STAMP_FRAGMENTS; i++)
    {
        //  Get fragment attributes.
        attrib = stamp->stamp[i]->getAttributes();

        //  Get fragment.
        fr = stamp->stamp[i]->getFragment();

        //  Check if color writes are disabled.
        if (writeR[0] || writeG[0] || writeB[0] || writeA[0])
        {
            //  Update latency map.
            switch(fragmentMapMode)
            {
                case COLOR_MAP:

                    GPU_ASSERT(
                        if (colorBufferFormat != GPU_RGBA8888)
                            panic("ColorWrite", "postWriteProcess", "Fragment Map Color Mode not supported for color buffer formats different than GPU_RGBA8888.");
                    )
                    
                    latencyMap[(y >> 1) * ((hRes >> 1) + (hRes & 0x01)) + (x >> 1)] =
                        u32bit(stamp->data[0] + (stamp->data[1] << 8) + (stamp->data[2] << 16));

                    break;

                case OVERDRAW_MAP:

                    latencyMap[(y >> 1) * ((hRes >> 1) + (hRes & 0x01)) + (x >> 1)]++                                ;

                    break;

                case FRAGMENT_LATENCY_MAP:

                    latencyMap[(y >> 1) * ((hRes >> 1) + (hRes & 0x01)) + (x >> 1)] =
                        u32bit(cycle - startCycle);

                    break;

                case SHADER_LATENCY_MAP:

                    latencyMap[(y >> 1) * ((hRes >> 1) + (hRes & 0x01)) + (x >> 1)] =
                        shaderLatency;

                    break;

                case DISABLE_MAP:
                    break;
                    
                default:
                    panic("ColorWrite", "clock", "Unsuported fragment map mode.");
                    break;
            }
        }

        //  Delete fragment attributes.
        delete[] attrib;

        //  Delete the fragment in the emulator.
        delete fr;

        //  Delete fragment input.
        delete stamp->stamp[i];
    }

    //  Delete the stamp container.
    delete[] stamp->stamp;
}

//  This function is called by the generic ROP clock in the flush state.
void ColorWriteV2::flush(u64bit cycle)
{
    u32bit colorBufferScanTiles;
    u32bit colorBufferBlocks;
    ROPBlockState *colorBlockState;
    ColorBlockStateInfo *colorBlockStateInfo;

    //  Check if the flush of the color cache has finished.
    if (!endFlush)
    {
        //  Continue flushing the cache.
        if(!colorCache->flush())
        {
            //  Compute the number of scan tiles that form the frame buffer.
            colorBufferScanTiles = (u32bit)(ceil(vRes / f32bit(genH * scanH * genH * STAMP_HEIGHT))) *
                                   (u32bit)(ceil(hRes / f32bit(genW * scanW * genW * STAMP_WIDTH))) * genH * genW;
                                           
            //  Multiply by the number of blocks in a scan tile based on the surface format and the number 
            //  of MSAA samples.  Then divide by the number of stamp units.
            colorBufferBlocks = u32bit(ceil((colorBufferScanTiles * (bytesPixel[0] >> 2) *
                                            (multisampling ? msaaSamples : 1) * scanH * scanW) / f32bit(numStampUnits)));

            //  Allocate the color block states.
            colorBlockState = new ROPBlockState[colorBufferBlocks];

            //  Check allocation.
            GPU_ASSERT(
                if (colorBlockState == NULL)
                    panic("ColorCache", "flush", "Error allocating color block state memory.");
            )

            //  Get the color block states.
            colorCache->copyBlockStateMemory(colorBlockState, colorBufferBlocks);

            //  Create object for the DAC.
            colorBlockStateInfo = new ColorBlockStateInfo(colorBlockState, colorBufferBlocks);

            //  Copy cookies from the last command from the Command Processor.
            colorBlockStateInfo->copyParentCookies(*lastRSCommand);

            //  Add a cookie level.
            colorBlockStateInfo->addCookie();

            //  Send color buffer block state to the DAC.
            blockStateDAC->write(cycle, colorBlockStateInfo);

            //  Set cycles for copying the color buffer block state to the DAC.
            copyStateCycles = (u32bit) ceil((f32bit) colorBufferBlocks / (f32bit) blocksCycle);

            //  Set flush end.
            endFlush = TRUE;
        }
    }
    else
    {
        //  Check if copying color block state memory to the DAC.
        if (copyStateCycles > 0)
        {
            //  Update cycle counter.
            copyStateCycles--;

            //  Check if end of the copy.
            if (copyStateCycles == 0)
            {
//printf("ColorWrite => End of flush.  Cycle %lld\n", cycle);
                //  End of swap.
                state = RAST_END;
            }
        }
    }
}

//  This function is called by the generic ROP clock in the swap state.
void ColorWriteV2::swap(u64bit cycle)
{
    u32bit colorBufferScanTiles;
    u32bit colorBufferBlocks;
    ROPBlockState *colorBlockState;
    ColorBlockStateInfo *colorBlockStateInfo;

    //  Check if the flush of the color cache has finished.
    if (!endFlush)
    {
        //  Continue flushing the cache.
        if(!colorCache->flush())
        {
            //  Set new color buffer address in the color cache.
            colorCache->swap(backBuffer);

            //  Compute the number of scan tiles that form the frame buffer.
            colorBufferScanTiles = (u32bit)(ceil(vRes / f32bit(genH * scanH * genH * STAMP_HEIGHT))) *
                                   (u32bit)(ceil(hRes / f32bit(genW * scanW * genW * STAMP_WIDTH))) * genH * genW;
                                           
            //  Multiply by the number of blocks in a scan tile based on the surface format and the number 
            //  of MSAA samples.  Then divide by the number of stamp units.
            colorBufferBlocks = u32bit(ceil((colorBufferScanTiles * (bytesPixel[0] >> 2) *
                                            (multisampling ? msaaSamples : 1) * scanH * scanW) / f32bit(numStampUnits)));

            //  Allocate the color block states.
            colorBlockState = new ROPBlockState[colorBufferBlocks];

            //  Check allocation.
            GPU_ASSERT(
                if (colorBlockState == NULL)
                    panic("ColorCache", "swap", "Error allocating color block state memory.");
            )

            //  Get the color block states.
            colorCache->copyBlockStateMemory(colorBlockState, colorBufferBlocks);

            //  Create object for the DAC.
            colorBlockStateInfo = new ColorBlockStateInfo(colorBlockState, colorBufferBlocks);

            //  Copy cookies from the last command from the Command Processor.
            colorBlockStateInfo->copyParentCookies(*lastRSCommand);

            //  Add a cookie level.
            colorBlockStateInfo->addCookie();

            //  Send color buffer block state to the DAC.
            blockStateDAC->write(cycle, colorBlockStateInfo);

            //  Set cycles for copying the color buffer block state to the DAC.
            copyStateCycles = (u32bit) ceil((f32bit) colorBufferBlocks / (f32bit) blocksCycle);

            //  Set flush end.
            endFlush = TRUE;
        }
    }
    else
    {
        //  Check if copying color block state memory to the DAC.
        if (copyStateCycles > 0)
        {
            //  Update cycle counter.
            copyStateCycles--;

            //  Check if end of the copy.
            if (copyStateCycles == 0)
            {
//printf("ColorWrite => End of swap.  Cycle %lld\n", cycle);

                //  End of swap.
                state = RAST_END;
            }
        }
    }
}

//  This function is called by the generic ROP clock in the clear state.
void ColorWriteV2::clear()
{
    //  Check if the color cache has finished the clear.
    if (!colorCache->clear(clearColorData))
    {
        //  Change to end state.
        state = RAST_END;
    }
}

//  This function is called by the Generic ROP clock when a batch processing is finished.
void ColorWriteV2::endBatch(u64bit cycle)
{
    //  Nothing to do.

    //  Delete the last quad fragments
    for(int i = 0; i < STAMP_FRAGMENTS; i++)
        delete lastBatchStamp.stamp[i];

    //  Delete the stamp container.
    delete[] lastBatchStamp.stamp;
}

//  Processes a rasterizer command.
void ColorWriteV2::processCommand(RasterizerCommand *command, u64bit cycle)
{
    u32bit colorBuffer;
    u32bit i;
    u32bit samples;

    //  Delete last command.
    delete lastRSCommand;

    //  Store current command as last received command.
    lastRSCommand = command;

    //  Process command.
    switch(command->getCommand())
    {
        case RSCOM_RESET:
            //  Reset command from the Rasterizer main box.

            GPU_DEBUG_BOX(
                printf("ColorWrite => RESET command received.\n");
            )

            //  Change to reset state.
            state = RAST_RESET;

            break;

        case RSCOM_DRAW:
            //  Draw command from the Rasterizer main box.

            GPU_DEBUG_BOX(
                printf("ColorWrite => DRAW command received.\n");
            )

            //  Check current state.
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("ColorWrite", "processRegisterWrite", "DRAW command can only be received in READY state.");
            )

            //  Configure the blend emulation.
            for(u32bit rt = 0; rt < MAX_RENDER_TARGETS; rt++)
                frEmu.setBlending(rt, equation[rt], srcRGB[rt], srcAlpha[rt], dstRGB[rt], dstAlpha[rt], constantColor[rt]);

            //  Configure logical operation emulation.
            frEmu.setLogicOpMode(logicOpMode);

            //  Compute the number of MSAA samples.
            samples = multisampling ? msaaSamples : 1;
            
            //  Reset the number of active buffers.
            numActiveBuffers = 0;
            
            //  Per render target.
            for(u32bit rt = 0; rt < MAX_RENDER_TARGETS; rt++)
            {            
                //  Set display parameters in the Pixel Mapper.
                pixelMapper[rt].setupDisplay(hRes, vRes, STAMP_WIDTH, STAMP_WIDTH, genW, genH, scanW, scanH, overW, overH,
                    samples, bytesPixel[rt]);
                
                //  Configure the active buffers for Generic ROP.
                activeBuffer[rt] = rtEnable[rt];
                if (rtEnable[rt])
                    numActiveBuffers++;
            }
                        
            //  Reset triangle counter.
            triangleCounter = 0;

            //  Reset batch fragment counter.
            fragmentCounter = 0;

            //  Reset previous processed triangle.
            currentTriangle = (u32bit) -1;

            //  Last batch fragment no received yet.
            lastFragment = FALSE;

            //  Check if the latenty map is enabled.
            if (fragmentMapMode != DISABLE_MAP)
            {
                //  Check if the latency map must be cleared.
                if (clearLatencyMap)
                {
                    //  Clear latency map.
                    memset(latencyMap, 0, ((hRes >> 1) + (hRes & 0x01)) * ((vRes >> 1) + (vRes & 0x01)) * 4);

                    //  Unset clear flag.
                    clearLatencyMap = false;
                }
            }
            
            //  Change to drawing state.
            state = RAST_DRAWING;

            break;

        case RSCOM_END:
            //  End command received from Rasterizer main box.

            GPU_DEBUG_BOX(
                printf("ColorWrite => END command received.\n");
            )

            //  Check current state.
            GPU_ASSERT(
                if (state != RAST_END)
                    panic("ColorWrite", "processCommand", "END command can only be received in END state.");
            )

            //  Change to ready state.
            state = RAST_READY;

            break;

        case RSCOM_REG_WRITE:
            //  Write register command from the Rasterizer main box.

            GPU_DEBUG_BOX(
                printf("ColorWrite => REG_WRITE command received.\n");
            )

            //  Check current state.
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("ColorWrite", "processCommand", "REGISTER WRITE command can only be received in READY state.");
            )

            //  Process register write.
            processRegisterWrite(command->getRegister(),
                command->getSubRegister(), command->getRegisterData());

            break;

        case RSCOM_SWAP:
            //  Swap front and back buffer command.

            GPU_DEBUG_BOX(
                printf("ColorWrite => SWAP command received.\n");
            )

            //  Check current state.
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("ColorWrite", "processCommand", "SWAP command can only be received in READY state.");
            )

            //  Update frame counter.
            frameCounter++;

            //  Swap primary and secondary color buffers.
            colorBuffer = frontBuffer;
            frontBuffer = backBuffer;
            backBuffer = colorBuffer;

            //  Set ROP data buffer address.
            bufferAddress[0] = backBuffer;
            
            //  Backbuffer is aliased as render target 0.
            rtAddress[0] = backBuffer;

            //  Reset color cache flush flag.
            endFlush = false;

            //  Clear the latency map before the next batch.
            clearLatencyMap = true;

            //  Change state to swap state.
            state = RAST_SWAP;

            break;

        case RSCOM_DUMP_COLOR:
        
            //  Dump color buffer command.

            GPU_DEBUG_BOX(
                printf("ColorWrite => DUMPCOLOR command received.\n");
            )

            //  Check current state.
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("ColorWrite", "processCommand", "DUMPCOLOR command can only be received in READY state.");
            )

            //  Set ROP data buffer address.
            bufferAddress[0] = backBuffer;

            //  Reset color cache flush flag.
            endFlush = false;

            //  Change state to swap state.
            state = RAST_SWAP;

            break;

        case RSCOM_FLUSH:
            //  Flush ColorWrite Unit command.

            GPU_DEBUG_BOX(
                printf("ColorWrite => FLUSH command received.\n");
            )

            //  Check current state.
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("ColorWrite", "processCommand", "FLUSH command can only be received in READY state.");
            )

            //  Reset color cache flush flag.
            endFlush = FALSE;

            //  Change state to flush state.
            state = RAST_FLUSH;

            break;

        case RSCOM_SAVE_STATE:
            //  Save state ColorWrite Unit command.

            GPU_DEBUG_BOX(
                printf("ColorWrite => SAVE_STATE command received.\n");
            )

            //  Check current state.
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("ColorWrite", "processCommand", "SAVE_STATE command can only be received in READY state.");
            )

            //  Reset color cache flush flag.
            endFlush = FALSE;

            //  Change state to save state state.
            state = RAST_SAVE_STATE;

            break;

        case RSCOM_RESTORE_STATE:
            //  Restore state ColorWrite Unit command.

            GPU_DEBUG_BOX(
                printf("ColorWrite => RESTORE_STATE command received.\n");
            )

            //  Check current state.
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("ColorWrite", "processCommand", "RESTORE_STATE command can only be received in READY state.");
            )

            //  Reset color cache flush flag.
            endFlush = FALSE;

            //  Change state to restore state state.
            state = RAST_RESTORE_STATE;

            break;

        case RSCOM_CLEAR_COLOR_BUFFER:
            //  Clear color buffer command.

            GPU_DEBUG_BOX(
                printf("ColorWrite => CLEAR_COLOR_BUFFER command received.\n");
            )

            //  Check current state.
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("ColorWrite", "processCommand", "CLEAR command can only be received in READY state.");
            )

            //  Clear the color cache as any info there will be invalid now.
            colorCache->clear(clearColorData);

            //  Change to clear state.
            state = RAST_CLEAR;

            break;
            
        case RSCOM_RESET_STATE:
        
            //  Reset the color buffer block state.
            
            GPU_DEBUG_BOX(
                printf("ColorWrite-%s (%lld) => RESET_STATE command received.\n", getName(), cycle);
            )
            
            //  Check current state.
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("ColorWrite", "processCommand", "RESET_STATE command can only be received in READY state.");
            )
            
            //  Reset color cache flush flag.
            endFlush = false;
            
            //  Change to reset state.
            state = RAST_RESET_STATE;
        
            break;

        default:
            panic("ColorWrite", "proccessCommand", "Unsupported command.");
            break;
    }
}

void ColorWriteV2::processRegisterWrite(GPURegister reg, u32bit subreg,
    GPURegData data)
{
    //  Process register write.
    switch(reg)
    {
        case GPU_DISPLAY_X_RES:
            //  Write display horizontal resolution register.
            hRes = data.uintVal;

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_DISPLAY_X_RES = %d.\n", hRes);
            )

            //  Check if the latenty map is enabled.
            if (fragmentMapMode != DISABLE_MAP)
            {
                //  Delete old latency map.
                delete[] latencyMap;

                //  Reallocate space for the latency map.
                latencyMap = new u32bit[((hRes >> 1) + (hRes & 0x01)) * ((vRes >> 1) + (vRes & 0x01))];

                //  Clear latency map before the next batch.
                clearLatencyMap = true;
                //memset(latencyMap, 0, ((hRes >> 1) + (hRes & 0x01)) * ((vRes >> 1) + (vRes & 0x01)) * 4);
            }
            
            break;

        case GPU_DISPLAY_Y_RES:
            //  Write display vertical resolution register.
            vRes = data.uintVal;

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_DISPLAY_Y_RES = %d.\n", vRes);
            )

            //  Check if the latenty map is enabled.
            if (fragmentMapMode != DISABLE_MAP)
            {
                //  Delete old latency map.
                delete[] latencyMap;

                //  Reallocate space for the latency map.
                latencyMap = new u32bit[((hRes >> 1) + (hRes & 0x01)) * ((vRes >> 1) + (vRes & 0x01))];

                //  Clear latency map before the next batch.
                clearLatencyMap = true;
                //memset(latencyMap, 0, ((hRes >> 1) + (hRes & 0x01)) * ((vRes >> 1) + (vRes & 0x01)) * 4);
            }
            
            break;

        case GPU_VIEWPORT_INI_X:
            //  Write viewport initial x coordinate register.
            startX = data.intVal;

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_VIEWPORT_INI_X = %d.\n", startX);
            )

            break;

        case GPU_VIEWPORT_INI_Y:
            //  Write viewport initial y coordinate register.
            startY = data.intVal;

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_VIEWPORT_INI_Y = %d.\n", startY);
            )

            break;

        case GPU_VIEWPORT_WIDTH:
            //  Write viewport width register.
            width = data.uintVal;

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_VIEWPORT_WIDTH = %d.\n", width);
            )

            break;

        case GPU_VIEWPORT_HEIGHT:
            //  Write viewport height register.
            height = data.uintVal;

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_VIEWPORT_HEIGHT = %d.\n", height);
            )

            break;

        case GPU_COLOR_BUFFER_CLEAR:
        
            //  Write color buffer clear color value.
            clearColor[0] = data.qfVal[0];
            clearColor[1] = data.qfVal[1];
            clearColor[2] = data.qfVal[2];
            clearColor[3] = data.qfVal[3];

            //  Convert the float point clear color to the color buffer format.
            switch(colorBufferFormat)
            {
                case GPU_RGBA8888:
                
                    clearColorData[0] = u8bit(clearColor[0] * 255.0f);
                    clearColorData[1] = u8bit(clearColor[1] * 255.0f);
                    clearColorData[2] = u8bit(clearColor[2] * 255.0f);
                    clearColorData[3] = u8bit(clearColor[3] * 255.0f);
                    
                    break;

                case GPU_RG16F:
                
                    ((f16bit *) clearColorData)[0] = GPUMath::convertFP32ToFP16(clearColor[0]);
                    ((f16bit *) clearColorData)[1] = GPUMath::convertFP32ToFP16(clearColor[1]);
                    break;
                    
                case GPU_R32F:
                
                    ((f32bit *) clearColorData)[0] = clearColor[0];
                    break;

                case GPU_RGBA16:
                
                    ((u16bit *) clearColorData)[0] = u16bit(clearColor[0] * 65535.0f);
                    ((u16bit *) clearColorData)[1] = u16bit(clearColor[1] * 65535.0f);
                    ((u16bit *) clearColorData)[2] = u16bit(clearColor[2] * 65535.0f);
                    ((u16bit *) clearColorData)[3] = u16bit(clearColor[3] * 65535.0f);
                    
                    break;

                case GPU_RGBA16F:
                
                    ((f16bit *) clearColorData)[0] = GPUMath::convertFP32ToFP16(clearColor[0]);
                    ((f16bit *) clearColorData)[1] = GPUMath::convertFP32ToFP16(clearColor[1]);
                    ((f16bit *) clearColorData)[2] = GPUMath::convertFP32ToFP16(clearColor[2]);
                    ((f16bit *) clearColorData)[3] = GPUMath::convertFP32ToFP16(clearColor[3]);
                    
                    break;
                    
                default:
                    panic("ColorWrite", "processRegisterWrite", "Unsupported color buffer format.");
                    break;
            }

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_COLOR_BUFFER_CLEAR = (%f, %f, %f, %f).\n", clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
            )

            break;

        case GPU_FRONTBUFFER_ADDR:
        
            //  Write front color buffer address register.
            frontBuffer = data.uintVal;

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_FRONTBUFFER_ADDR = %x.\n",
                    frontBuffer);
            )

            break;

        case GPU_BACKBUFFER_ADDR:
        
            //  Write back color buffer address register.
            backBuffer = data.uintVal;

            //  Aliased to render target 0.
            rtAddress[0] = backBuffer;
            
            //  Set ROP data buffer address.
            bufferAddress[0] = backBuffer;

            //  Set color buffer address in the color cache.
            colorCache->swap(backBuffer);

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_BACKBUFFER_ADDR = %x.\n",
                    backBuffer);
            )

            break;

        case GPU_COLOR_STATE_BUFFER_MEM_ADDR:

            //  Set ROP block state buffer address.
            stateBufferAddress = data.uintVal;

            //  Set block state buffer address in the Color cache.
            colorCache->setStateAddress(stateBufferAddress);

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_COLOR_STATE_BUFFER_MEM_ADDR = %x\n", stateBufferAddress);
            )

            break;

        case GPU_MULTISAMPLING:
        
            //  Write Multisampling enable flag.
            multisampling = data.booleanVal;
            
            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_MULTISAMPLING = %s\n", multisampling?"TRUE":"FALSE");
            )
                       
            //  Update color cache state.
            if (multisampling)
                colorCache->setMSAASamples(msaaSamples);
            else
                colorCache->setMSAASamples(1);

            break;
            
        case GPU_MSAA_SAMPLES:
        
            //  Write MSAA z samples per fragment register.
            msaaSamples = data.uintVal;
            
            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_MSAA_SAMPLES = %d\n", msaaSamples);
            )

            //  Update color cache state.
            if (multisampling)
                colorCache->setMSAASamples(msaaSamples);
            else
                colorCache->setMSAASamples(1);

            break;            

        case GPU_COLOR_BUFFER_FORMAT:
            
            //  Write Color Buffer format register.
            colorBufferFormat = data.txFormat;           
            
            //  Aliased to render target 0.
            rtFormat[0] = data.txFormat;
            
            //  Set the bytes per pixel for the current color buffer format.
            switch(colorBufferFormat)
            {
                case GPU_RGBA8888:
                case GPU_RG16F:
                case GPU_R32F:
                    bytesPixel[0] = 4;
                    break;
                case GPU_RGBA16:
                case GPU_RGBA16F:
                    bytesPixel[0] = 8;
                    break;
                default:
                    panic("ColorWrite", "processRegisterWrite", "Unsupported color buffer format.");
                    break;
            }
            
            //  Set the bytes per pixel in the color cache.
            colorCache->setBytesPerPixel(bytesPixel[0]);
            
            //  Convert the float point clear color to the color buffer format.
            switch(colorBufferFormat)
            {
                case GPU_RGBA8888:
                
                    clearColorData[0] = u8bit(clearColor[0] * 255.0f);
                    clearColorData[1] = u8bit(clearColor[1] * 255.0f);
                    clearColorData[2] = u8bit(clearColor[2] * 255.0f);
                    clearColorData[3] = u8bit(clearColor[3] * 255.0f);
                    
                    break;

                case GPU_RG16F:
                
                    ((f16bit *) clearColorData)[0] = GPUMath::convertFP32ToFP16(clearColor[0]);
                    ((f16bit *) clearColorData)[1] = GPUMath::convertFP32ToFP16(clearColor[1]);
                    break;
                    
                case GPU_R32F:
                
                    ((f32bit *) clearColorData)[0] = clearColor[0];
                    break;

                case GPU_RGBA16:
                
                    ((u16bit *) clearColorData)[0] = u16bit(clearColor[0] * 65535.0f);
                    ((u16bit *) clearColorData)[1] = u16bit(clearColor[1] * 65535.0f);
                    ((u16bit *) clearColorData)[2] = u16bit(clearColor[2] * 65535.0f);
                    ((u16bit *) clearColorData)[3] = u16bit(clearColor[3] * 65535.0f);                    
                    break;

                case GPU_RGBA16F:
                
                    ((f16bit *) clearColorData)[0] = GPUMath::convertFP32ToFP16(clearColor[0]);
                    ((f16bit *) clearColorData)[1] = GPUMath::convertFP32ToFP16(clearColor[1]);
                    ((f16bit *) clearColorData)[2] = GPUMath::convertFP32ToFP16(clearColor[2]);
                    ((f16bit *) clearColorData)[3] = GPUMath::convertFP32ToFP16(clearColor[3]);                    
                    break;
                    
                default:
                    panic("ColorWrite", "processRegisterWrite", "Unsupported color buffer format.");
                    break;
            }

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_COLOR_BUFFER_FORMAT = ");
                switch(colorBufferFormat)
                {
                    case GPU_RGBA8888:
                        printf(" GPU_RGBA8888.\n");
                        break;
                        
                    case GPU_RG16F:
                        printf(" GPU_RG16F.\n");
                        break;

                    case GPU_R32F:
                        printf(" GPU_R32F.\n");
                        break;
                        
                    case GPU_RGBA16:
                        printf(" GPU_RGBA16.\n");
                        break;
                        
                    case GPU_RGBA16F:
                        printf(" GPU_RGBA16F.\n");
                        break;
                }
            )
            
            break;
                    
        case GPU_COLOR_COMPRESSION:
        
            //  Write color buffer compression enable/disable register.
            compression = data.booleanVal;
            
            //  Set color compression flag in the Color Cache.
            colorCache->setCompression(compression);
            
            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_COLOR_COMPRESSION = %s\n", compression ? "TRUE" : "FALSE");
            )
            
            break;

        case GPU_COLOR_SRGB_WRITE:
            
            //  Write conversion from linear to sRGB on color write enable/disable register.
            colorSRGBWrite = data.booleanVal;
            
            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_COLOR_SRGB_WRITE = %s\n", colorSRGBWrite ? "TRUE" : "FALSE");
            )
            
            break;
            
        case GPU_RENDER_TARGET_ENABLE:
        
            //  Check render target index.
            GPU_ASSERT(
                if (subreg >= MAX_RENDER_TARGETS)
                    panic("ColorWrite", "processRegisterWrite", "Render target index out of range.");
            )
                            
            //  Write render target enable register.
            rtEnable[subreg] = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_RENDER_TARGET_ENABLE[%d] = %s.\n", subreg, data.booleanVal ? "TRUE" : "FALSE");
            )
            
            break;

        case GPU_RENDER_TARGET_FORMAT:
        
            //  Check render target index.
            GPU_ASSERT(
                if (subreg >= MAX_RENDER_TARGETS)
                    panic("ColorWrite", "processRegisterWrite", "Render target index out of range.");
            )
                            
            //  Write render target format register.
            rtFormat[subreg] = data.txFormat;            
            
            //  Aliased back buffer to render target 0.
            if (subreg == 0)
                colorBufferFormat = data.txFormat;
            
            //  Set the bytes per pixel for the current color buffer format.
            switch(data.txFormat)
            {
                case GPU_RGBA8888:
                case GPU_RG16F:
                case GPU_R32F:
                    bytesPixel[subreg] = 4;
                    break;
                case GPU_RGBA16:
                case GPU_RGBA16F:
                    bytesPixel[subreg] = 8;
                    break;
                default:
                    panic("ColorWrite", "processRegisterWrite", "Unsupported render target buffer format.");
                    break;
            }

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_RENDER_TARGET_FORMAT[%d] = ", subreg);
                switch(data.txFormat)
                {
                    case GPU_RGBA8888:
                        printf(" GPU_RGBA8888.\n");
                        break;
                        
                    case GPU_RGBA16:
                        printf(" GPU_RGBA16.\n");
                        break;
                        
                    case GPU_RGBA16F:
                        printf(" GPU_RGBA16F.\n");
                        break;
                                
                    case GPU_RG16F:
                        printf(" GPU_RG16F.\n");
                        break;

                    case GPU_R32F:
                        printf(" GPU_R32F.\n");
                        break;
                }
            )
            
            break;

        case GPU_RENDER_TARGET_ADDRESS:
        
            //  Check render target index.
            GPU_ASSERT(
                if (subreg >= MAX_RENDER_TARGETS)
                    panic("ColorWrite", "processRegisterWrite", "Render target index out of range.");
            )
                            
            //  Write render target base address register.
            rtAddress[subreg] = data.uintVal;
            
            //  Set base buffer address for generic ROP.
            bufferAddress[subreg] = data.uintVal;
            
            //  Backbuffer is aliased as render target 0.
            if (subreg == 0)
                backBuffer = data.uintVal;

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_RENDER_TARGET_ADDRESS[%d] = %d.\n", subreg, data.uintVal);
            )
            
            break;

        case GPU_COLOR_BLEND:
        
            //  Write blend enabled flag register.
            blend[subreg] = data.booleanVal;

            //  Set ROP read data flag.
            readDataROP[subreg] = blend[subreg] || ((subreg == 0) &&  logicOperation);

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_COLOR_BLEND[%d] = %s.\n", subreg, blend?"ENABLED":"DISABLED");
            )

            break;

        case GPU_BLEND_EQUATION:

            //  Write blend equation register.
            equation[subreg] = data.blendEquation;

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_BLEND_EQUATION[%d] = ", subreg);

                switch(equation[subreg])
                {
                    case BLEND_FUNC_ADD:
                        printf("FUNC_ADD.\n");
                        break;

                    case BLEND_FUNC_SUBTRACT:
                        printf("FUNC_SUBTRACT.\n");
                        break;

                    case BLEND_FUNC_REVERSE_SUBTRACT:
                        printf("FUNC_REVERSE_SUBTRACT.\n");
                        break;

                    case BLEND_MIN:
                        printf("MIN.\n");
                        break;

                    case BLEND_MAX:
                        printf("MAX.\n");
                        break;

                    default:
                        panic("ColorWrite", "processRegisterWrite", "Unsupported blend equation mode.");
                        break;
                }
            )

            break;

        case GPU_BLEND_SRC_RGB:

            //  Write source RGB weight function.
            srcRGB[subreg] = data.blendFunction;

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_BLEND_SRC_RGB[%d] = ", subreg);

                switch(srcRGB[subreg])
                {
                    case BLEND_ZERO:
                        printf("ZERO.\n");
                        break;
                    case BLEND_ONE:
                        printf("ONE.\n");
                        break;

                    case BLEND_SRC_COLOR:
                        printf("SRC_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_SRC_COLOR:
                        printf("ONE_MINUS_SRC_COLOR.\n");
                        break;

                    case BLEND_DST_COLOR:
                        printf("DST_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_DST_COLOR:
                        printf("ONE_MINUS_DST_COLOR.\n");
                        break;

                    case BLEND_SRC_ALPHA:
                        printf("SRC_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_SRC_ALPHA:
                        printf("ONE_MINUS_SRC_ALPHA.\n");
                        break;

                    case BLEND_DST_ALPHA:
                        printf("DST_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_DST_ALPHA:
                        printf("ONE_MINUS_DST_ALPHA.\n");
                        break;

                    case BLEND_CONSTANT_COLOR:
                        printf("CONSTANT_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_CONSTANT_COLOR:
                        printf("ONE_MINUS_CONSTANT_COLOR.\n");
                        break;

                    case BLEND_CONSTANT_ALPHA:
                        printf("CONSTANT_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_CONSTANT_ALPHA:
                        printf("ONE_MINUS_CONSTANT_ALPHA.\n");
                        break;

                    case BLEND_SRC_ALPHA_SATURATE:
                        printf("ALPHA_SATURATE.\n");
                        break;

                    default:
                        panic("ColorWrite", "processRegisterWrite", "Unsupported blend weight function.");
                        break;
                }
            )

            break;

        case GPU_BLEND_DST_RGB:
        
            //  Write destination RGB weight function.
            dstRGB[subreg] = data.blendFunction;

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_BLEND_DST_RGB[%d] = ", subreg);

                switch(dstRGB[subreg])
                {
                    case BLEND_ZERO:
                        printf("ZERO.\n");
                        break;
                    case BLEND_ONE:
                        printf("ONE.\n");
                        break;

                    case BLEND_SRC_COLOR:
                        printf("SRC_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_SRC_COLOR:
                        printf("ONE_MINUS_SRC_COLOR.\n");
                        break;

                    case BLEND_DST_COLOR:
                        printf("DST_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_DST_COLOR:
                        printf("ONE_MINUS_DST_COLOR.\n");
                        break;

                    case BLEND_SRC_ALPHA:
                        printf("SRC_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_SRC_ALPHA:
                        printf("ONE_MINUS_SRC_ALPHA.\n");
                        break;

                    case BLEND_DST_ALPHA:
                        printf("DST_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_DST_ALPHA:
                        printf("ONE_MINUS_DST_ALPHA.\n");
                        break;

                    case BLEND_CONSTANT_COLOR:
                        printf("CONSTANT_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_CONSTANT_COLOR:
                        printf("ONE_MINUS_CONSTANT_COLOR.\n");
                        break;

                    case BLEND_CONSTANT_ALPHA:
                        printf("CONSTANT_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_CONSTANT_ALPHA:
                        printf("ONE_MINUS_CONSTANT_ALPHA.\n");
                        break;

                    case BLEND_SRC_ALPHA_SATURATE:
                        printf("ALPHA_SATURATE.\n");
                        break;

                    default:
                        panic("ColorWrite", "processRegisterWrite", "Unsupported blend weight function.");
                        break;
                }

            )
            break;

        case GPU_BLEND_SRC_ALPHA:

            //  Write source alpha weight function.
            srcAlpha[subreg] = data.blendFunction;

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_BLEND_SRC_ALPHA[%d] = ", subreg);

                switch(srcAlpha[subreg])
                {
                    case BLEND_ZERO:
                        printf("ZERO.\n");
                        break;
                    case BLEND_ONE:
                        printf("ONE.\n");
                        break;

                    case BLEND_SRC_COLOR:
                        printf("SRC_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_SRC_COLOR:
                        printf("ONE_MINUS_SRC_COLOR.\n");
                        break;

                    case BLEND_DST_COLOR:
                        printf("DST_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_DST_COLOR:
                        printf("ONE_MINUS_DST_COLOR.\n");
                        break;

                    case BLEND_SRC_ALPHA:
                        printf("SRC_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_SRC_ALPHA:
                        printf("ONE_MINUS_SRC_ALPHA.\n");
                        break;

                    case BLEND_DST_ALPHA:
                        printf("DST_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_DST_ALPHA:
                        printf("ONE_MINUS_DST_ALPHA.\n");
                        break;

                    case BLEND_CONSTANT_COLOR:
                        printf("CONSTANT_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_CONSTANT_COLOR:
                        printf("ONE_MINUS_CONSTANT_COLOR.\n");
                        break;

                    case BLEND_CONSTANT_ALPHA:
                        printf("CONSTANT_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_CONSTANT_ALPHA:
                        printf("ONE_MINUS_CONSTANT_ALPHA.\n");
                        break;

                    case BLEND_SRC_ALPHA_SATURATE:
                        printf("ALPHA_SATURATE.\n");
                        break;

                    default:
                        panic("ColorWrite", "processRegisterWrite", "Unsupported blend weight function.");
                        break;
                }

            )
            break;

        case GPU_BLEND_DST_ALPHA:
        
            //  Write destination ALPHA weight function.
            dstAlpha[subreg] = data.blendFunction;

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_BLEND_DST_ALPHA[%d] = ", subreg);

                switch(dstAlpha[subreg])
                {
                    case BLEND_ZERO:
                        printf("ZERO.\n");
                        break;
                    case BLEND_ONE:
                        printf("ONE.\n");
                        break;

                    case BLEND_SRC_COLOR:
                        printf("SRC_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_SRC_COLOR:
                        printf("ONE_MINUS_SRC_COLOR.\n");
                        break;

                    case BLEND_DST_COLOR:
                        printf("DST_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_DST_COLOR:
                        printf("ONE_MINUS_DST_COLOR.\n");
                        break;

                    case BLEND_SRC_ALPHA:
                        printf("SRC_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_SRC_ALPHA:
                        printf("ONE_MINUS_SRC_ALPHA.\n");
                        break;

                    case BLEND_DST_ALPHA:
                        printf("DST_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_DST_ALPHA:
                        printf("ONE_MINUS_DST_ALPHA.\n");
                        break;

                    case BLEND_CONSTANT_COLOR:
                        printf("CONSTANT_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_CONSTANT_COLOR:
                        printf("ONE_MINUS_CONSTANT_COLOR.\n");
                        break;

                    case BLEND_CONSTANT_ALPHA:
                        printf("CONSTANT_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_CONSTANT_ALPHA:
                        printf("ONE_MINUS_CONSTANT_ALPHA.\n");
                        break;

                    case BLEND_SRC_ALPHA_SATURATE:
                        printf("ALPHA_SATURATE.\n");
                        break;

                    default:
                        panic("ColorWrite", "processRegisterWrite", "Unsupported blend weight function.");
                        break;
                }

            )
            break;

        case GPU_BLEND_COLOR:
        
            //  Write blend constant color register.
            constantColor[subreg][0] = data.qfVal[0];
            constantColor[subreg][1] = data.qfVal[1];
            constantColor[subreg][2] = data.qfVal[2];
            constantColor[subreg][3] = data.qfVal[3];

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_BLEND_COLOR[%d] = {%f, %f, %f, %f}\n",
                    subreg, constantColor[subreg][0], constantColor[subreg][1], constantColor[subreg][2], constantColor[subreg][3]);
            )

            break;

        case GPU_COLOR_MASK_R:
        
            //  Write red component write mask register.
            writeR[subreg] = data.booleanVal;

            //  Set ROP bypass flag.
            bypassROP[subreg] = !writeR[subreg] && !writeG[subreg] && !writeB[subreg] && !writeA[subreg];

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_COLOR_MASK_R[%d] = %s.\n", subreg, writeR[subreg]?"ENABLED":"DISABLED");
            )

            break;

        case GPU_COLOR_MASK_G:
        
            //  Write green component write mask register.
            writeG[subreg] = data.booleanVal;

            //  Set ROP bypass flag.
            bypassROP[subreg] = !writeR[subreg] && !writeG[subreg] && !writeB[subreg] && !writeA[subreg];

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_COLOR_MASK_G[%d] = %s.\n", subreg, writeG[subreg]?"ENABLED":"DISABLED");
            )

            break;

        case GPU_COLOR_MASK_B:

            //  Write blue component write mask register.
            writeB[subreg] = data.booleanVal;

            //  Set ROP bypass flag.
            bypassROP[subreg] = !writeR[subreg] && !writeG[subreg] && !writeB[subreg] && !writeA[subreg];

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_COLOR_MASK_B[%d] = %s.\n", subreg, writeB[subreg]?"ENABLED":"DISABLED");
            )

            break;

        case GPU_COLOR_MASK_A:

            //  Write alpha component write mask register.
            writeA[subreg] = data.booleanVal;

            //  Set ROP bypass flag.
            bypassROP[subreg] = !writeR[subreg] && !writeG[subreg] && !writeB[subreg] && !writeA[subreg];

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_COLOR_MASK_A[%d] = %s.\n", subreg, writeA[subreg]?"ENABLED":"DISABLED");
            )

            break;

        case GPU_LOGICAL_OPERATION:
        
            //  Write logical operation enable flag register.
            logicOperation = data.booleanVal;

            //  Set ROP read data flag.
            readDataROP[0] = blend[0] || logicOperation;

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_LOGICAL_OPERATION = %s.\n", logicOperation?"ENABLED":"DISABLED");
            )

            break;

        case GPU_LOGICOP_FUNCTION:
        
            //  Write logical operation function register.
            logicOpMode = data.logicOp;

            GPU_DEBUG_BOX(
                printf("ColorWrite => Write GPU_LOGICOP_FUNCTION = ");

                switch(logicOpMode)
                {
                    case LOGICOP_CLEAR:
                        printf("CLEAR.\n");
                        break;

                    case LOGICOP_AND:
                        printf("AND.\n");
                        break;

                    case LOGICOP_AND_REVERSE:
                        printf("AND_REVERSE.\n");
                        break;

                    case LOGICOP_COPY:
                        printf("COPY.\n");
                        break;

                    case LOGICOP_AND_INVERTED:
                        printf("AND_INVERTED.\n");
                        break;

                    case LOGICOP_NOOP:
                        printf("NOOP.\n");
                        break;

                    case LOGICOP_XOR:
                        printf("XOR.\n");
                        break;

                    case LOGICOP_OR:
                        printf("OR.\n");
                        break;

                    case LOGICOP_NOR:
                        printf("NOR.\n");
                        break;

                    case LOGICOP_EQUIV:
                        printf("EQUIV.\n");
                        break;

                    case LOGICOP_INVERT:
                        printf("INVERT.\n");
                        break;

                    case LOGICOP_OR_REVERSE:
                        printf("OR_REVERSE.\n");
                        break;

                    case LOGICOP_COPY_INVERTED:
                        printf("COPY_INVERTED.\n");
                        break;

                    case LOGICOP_OR_INVERTED:
                        printf("OR_INVERTED.\n");
                        break;

                    case LOGICOP_NAND:
                        printf("NAND.\n");
                        break;

                    case LOGICOP_SET:
                        printf("SET.\n");
                        break;

                    default:
                        panic("ColorWrite", "processRegisterWrite", "Unsupported logical operation function.");
                        break;
                }

            )

            break;

        default:
            panic("ColorWrite", "processRegisterWrite", "Unsupported register.");
            break;
    }

}

//  Converts color data from RGBA8 format to RGBA32F format.
void ColorWriteV2::colorRGBA8ToRGBA32F(u8bit *in, QuadFloat *out)
{
    u32bit i;

    //  Convert all pixels in the stamp.
    for(i = 0; i < STAMP_FRAGMENTS; i++)
    {
        //  Convert 8 bit normalized color components to 32 bit float point color components.
        out[i][0] = f32bit(in[i * 4]) * (1.0f / 255.0f);
        out[i][1] = f32bit(in[i * 4 + 1]) * (1.0f / 255.0f);
        out[i][2] = f32bit(in[i * 4 + 2]) * (1.0f / 255.0f);
        out[i][3] = f32bit(in[i * 4 + 3]) * (1.0f / 255.0f);
    }

}

//  Converts color data in RGB32F format to RGBA8 format.
void ColorWriteV2::colorRGBA32FToRGBA8(QuadFloat *in, u8bit *out)
{
    u32bit i;

    //  Convert all pixels in the stamp.
    for(i = 0; i < STAMP_FRAGMENTS; i++)
    {
        //  Convert 32bit float point color components to 8 bit normalized color components.
        out[i * 4 + 0] = u32bit(255.0f * GPU_CLAMP(in[i][0], 0.0f, 1.0f));
        out[i * 4 + 1] = u32bit(255.0f * GPU_CLAMP(in[i][1], 0.0f, 1.0f));
        out[i * 4 + 2] = u32bit(255.0f * GPU_CLAMP(in[i][2], 0.0f, 1.0f));
        out[i * 4 + 3] = u32bit(255.0f * GPU_CLAMP(in[i][3], 0.0f, 1.0f));
    }
}

//  Converts color data from RGBA16F format to RGBA32F format.
void ColorWriteV2::colorRGBA16FToRGBA32F(u8bit *in, QuadFloat *out)
{
    u32bit i;

    //  Convert all pixels in the stamp.
    for(i = 0; i < STAMP_FRAGMENTS; i++)
    {
        //  Convert 16 bit float point color components to 32 bit float point color components.
        out[i][0] = GPUMath::convertFP16ToFP32(((u16bit *) in)[i * 4]);
        out[i][1] = GPUMath::convertFP16ToFP32(((u16bit *) in)[i * 4 + 1]);
        out[i][2] = GPUMath::convertFP16ToFP32(((u16bit *) in)[i * 4 + 2]);
        out[i][3] = GPUMath::convertFP16ToFP32(((u16bit *) in)[i * 4 + 3]);
    }

}

//  Converts color data in RGB32F format to RGBA8 format.
void ColorWriteV2::colorRGBA32FToRGBA16F(QuadFloat *in, u8bit *out)
{
    u32bit i;

    //  Convert all pixels in the stamp.
    for(i = 0; i < STAMP_FRAGMENTS; i++)
    {
        //  Convert 32 bit float point color components to 16 bit float point color components.
        ((u16bit *) out)[i * 4 + 0] = GPUMath::convertFP32ToFP16(in[i][0]);
        ((u16bit *) out)[i * 4 + 1] = GPUMath::convertFP32ToFP16(in[i][1]);
        ((u16bit *) out)[i * 4 + 2] = GPUMath::convertFP32ToFP16(in[i][2]);
        ((u16bit *) out)[i * 4 + 3] = GPUMath::convertFP32ToFP16(in[i][3]);
    }
}

//  Converts color data from RGBA16 format to RGBA32F format.
void ColorWriteV2::colorRGBA16ToRGBA32F(u8bit *in, QuadFloat *out)
{
    u32bit i;

    //  Convert all pixels in the stamp.
    for(i = 0; i < STAMP_FRAGMENTS; i++)
    {
        //  Convert 16 bit normalized color components to 32 bit float point color components.
        out[i][0] = f32bit(((u16bit *) in)[i * 4 + 0]) * (1.0f / 65535.0f);
        out[i][1] = f32bit(((u16bit *) in)[i * 4 + 1]) * (1.0f / 65535.0f);
        out[i][2] = f32bit(((u16bit *) in)[i * 4 + 2]) * (1.0f / 65535.0f);
        out[i][3] = f32bit(((u16bit *) in)[i * 4 + 3]) * (1.0f / 65535.0f);
    }
}

//  Converts color data in RGB32F format to RGBA16 format.
void ColorWriteV2::colorRGBA32FToRGBA16(QuadFloat *in, u8bit *out)
{
    u32bit i;

    //  Convert all pixels in the stamp.
    for(i = 0; i < STAMP_FRAGMENTS; i++)
    {
        //  Convert 32bit float point color components to 16 bit normalized color components.
        ((u16bit *) out)[i * 4 + 0] = u16bit(65535.0f * GPU_CLAMP(in[i][0], 0.0f, 1.0f));
        ((u16bit *) out)[i * 4 + 1] = u16bit(65535.0f * GPU_CLAMP(in[i][1], 0.0f, 1.0f));
        ((u16bit *) out)[i * 4 + 2] = u16bit(65535.0f * GPU_CLAMP(in[i][2], 0.0f, 1.0f));
        ((u16bit *) out)[i * 4 + 3] = u16bit(65535.0f * GPU_CLAMP(in[i][3], 0.0f, 1.0f));
    }
}


//  Converts color data from RG16F format to RGBA32F format.
void ColorWriteV2::colorRG16FToRGBA32F(u8bit *in, QuadFloat *out)
{
    u32bit i;

    //  Convert all pixels in the stamp.
    for(i = 0; i < STAMP_FRAGMENTS; i++)
    {
        //  Convert 16 bit float point color components to 32 bit float point color components.
        out[i][0] = GPUMath::convertFP16ToFP32(((u16bit *) in)[i * 2]);
        out[i][1] = GPUMath::convertFP16ToFP32(((u16bit *) in)[i * 2 + 1]);
        out[i][2] = 0.0f;
        out[i][3] = 1.0f;
    }
}

//  Converts color data in RGB32F format to RG16F format.
void ColorWriteV2::colorRGBA32FToRG16F(QuadFloat *in, u8bit *out)
{
    u32bit i;

    //  Convert all pixels in the stamp.
    for(i = 0; i < STAMP_FRAGMENTS; i++)
    {
        //  Convert 32 bit float point color components to 16 bit float point color components.
        ((u16bit *) out)[i * 2]     = GPUMath::convertFP32ToFP16(in[i][0]);
        ((u16bit *) out)[i * 2 + 1] = GPUMath::convertFP32ToFP16(in[i][1]);
    }
}

//  Converts color data from R32F format to RGBA32F format.
void ColorWriteV2::colorR32FToRGBA32F(u8bit *in, QuadFloat *out)
{
    u32bit i;

    //  Convert all pixels in the stamp.
    for(i = 0; i < STAMP_FRAGMENTS; i++)
    {
        //  Convert 16 bit float point color components to 32 bit float point color components.
        out[i][0] = ((f32bit *) in)[i];
        out[i][1] = 0.0f;
        out[i][2] = 0.0f;
        out[i][3] = 1.0f;
    }
}

//  Converts color data in RGB32F format to R32F format.  */
void ColorWriteV2::colorRGBA32FToR32F(QuadFloat *in, u8bit *out)
{
    u32bit i;

    //  Convert all pixels in the stamp.
    for(i = 0; i < STAMP_FRAGMENTS; i++)
    {
        //  Convert 32 bit float point color components to 16 bit float point color components.
        ((f32bit *) out)[i] = in[i][0];
    }
}


#define GAMMA(x) f32bit(GPU_POWER(f64bit(x), f64bit(1.0f / 2.2f)))
#define LINEAR(x) f32bit(GPU_POWER(f64bit(x), f64bit(2.2f)))

//  Converts color data from linear space to sRGB space.
void ColorWriteV2::colorLinearToSRGB(QuadFloat *in)
{
    //  NOTE: Alpha shouldn't be affected by the color space conversion.

    for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
    {
        //  Convert from linear space to sRGB space.
        in[f][0] = GAMMA(in[f][0]);
        in[f][1] = GAMMA(in[f][1]);
        in[f][2] = GAMMA(in[f][2]);
    }
}

//  Converts color data from sRGB space to linear space.
void ColorWriteV2::colorSRGBToLinear(QuadFloat *in)
{
    for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
    {
        //  NOTE: Alpha shouldn't be affected by the color space conversion.

        //  Convert from sRGB space to linear space.
        in[f][0] = LINEAR(in[f][0]);
        in[f][1] = LINEAR(in[f][1]);
        in[f][2] = LINEAR(in[f][2]);
    }
}

/*  Returns the unit fragment latency map.  */
u32bit *ColorWriteV2::getLatencyMap(u32bit &mapWidth, u32bit &mapHeight)
{
    //  Check if the latenty map is enabled.
    if (fragmentMapMode != DISABLE_MAP)
    {
        //  Check if the latency map must be cleared.
        if (clearLatencyMap)
        {
            //  Clear latency map.
            memset(latencyMap, 0, ((hRes >> 1) + (hRes & 0x01)) * ((vRes >> 1) + (vRes & 0x01)) * 4);

            //  Unset clear flag.
            clearLatencyMap = false;
        }

        mapWidth = (hRes >> 1) + (hRes & 0x01);
        mapHeight = (vRes >> 1) + (vRes & 0x01);

        return latencyMap;
    }
    else
        return NULL;
}

void ColorWriteV2::saveBlockStateMemory()
{
    colorCache->saveBlockStateMemory();
}

void ColorWriteV2::loadBlockStateMemory()
{
    colorCache->loadBlockStateMemory();
}

//  List the debug commands supported by the Command Processor
void ColorWriteV2::getCommandList(std::string &commandList)
{
    commandList.append("tracefragment - Traces the execution of the defined fragment.\n");
}

//  Execute a debug command
void ColorWriteV2::execCommand(stringstream &commandStream)
{
    string command;

    commandStream >> ws;
    commandStream >> command;

    if (!command.compare("tracefragment"))
    {
        commandStream >> ws;
        
        if (commandStream.eof())
        {
            cout << "Usage : tracefragment <on | off> <x> <y>" << endl;
        }
        else
        {
            string enableParam;
            bool enableFTrace;
            
            commandStream >> enableParam;
            
            bool error = false;
            if (!enableParam.compare("on"))
                enableFTrace = true;
            else if (!enableParam.compare("off"))
                enableFTrace = false;
            else
            {
                error = true;
                cout << getName() << ">> Usage : tracefragment <on | off> <x> <y>" << endl;
            }
            
            commandStream >> ws;
            
            if (commandStream.eof())
            {
                cout << getName() << " >> Usage : tracefragment <on | off> <x> <y>" << endl;
            }
            else
            {
                commandStream >> watchFragmentX;
                commandStream >> ws;
                
                if (commandStream.eof())
                {
                    cout << getName() << " >> Usage : tracefragment <on | off> <x> <y>" << endl;
                }
                else
                {
                    commandStream >> watchFragmentY;
                    
                    traceFragment = enableFTrace;
                    
                    if (enableFTrace)
                        cout << getName() << " >> Enabling fragment trace for coordinate (" << watchFragmentX << ", " << watchFragmentY << ")." << endl;
                    else
                        cout << getName() << " >> Disabling fragment trace." << endl;
                }
            }
        }
    }    
    else
    {
        cout << "Unsupported box command >> " << command;

        while (!commandStream.eof())
        {
            commandStream >> command;
            cout << " " << command;
        }

        cout << endl;
    }
}


void ColorWriteV2::setValidationMode(bool enable)
{
    validationMode = enable;
}

FragmentQuadMemoryUpdateMap &ColorWriteV2::getColorUpdateMap(u32bit rt)
{
    //  Check the render target identifier range.
    GPU_ASSERT(
        if (rt > MAX_RENDER_TARGETS)
            panic("ColorWriteV2", "getColorUpdateMap", "Render target identifier out of range.");
    )
    
    return colorMemoryUpdateMap[rt];
}

