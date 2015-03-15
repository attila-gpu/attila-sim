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
 */

#include "Tools.h"

EventHandler* wakeupHandler;
EventHandler* infoHandler;

void FrameHandlerInfo::action(GLInstrument& gli)
{
    gli.registerFrameHandler(gli.After, *wakeupHandler);
    int frame = gli.currentFrame();
    int batches = gli.currentBatch();
    int vertexCount = gli.frameVertexCount();
    int totalVertexCount = gli.totalVertexCount();
    char buffer[1024];
    sprintf(buffer, "Frame: %d\nBatches: %d\nVertexes in this frame: %d\n"
                    "Total vertexes in this application: %d",
                    frame, batches, vertexCount, totalVertexCount);

    MessageBox(NULL, buffer, "Every 50 frames InfoTool", MB_OK);

}

void FrameHandlerWakeup::action(GLInstrument& gli)
{
    if ( gli.currentFrame() % 50 == 0)
    {
        gli.unregisterFrameHandler(gli.After);
        gli.registerFrameHandler(gli.After, *infoHandler);
    }
}

EveryNInfoTool::EveryNInfoTool(int every) : every(every)
{}


bool EveryNInfoTool::init(GLInstrument& gli)
{
    wakeupHandler = new FrameHandlerWakeup;
    infoHandler = new FrameHandlerInfo;

    //gli.registerAfterFunc().glDrawElements = glDrawElementsMsg;
    gli.registerFrameHandler(GLInstrument::After, *wakeupHandler);
    return true;
}

