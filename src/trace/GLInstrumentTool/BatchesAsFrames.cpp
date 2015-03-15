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

#include "BatchesAsFrames.h"

void DoSwap::action(GLInstrument& gli)
{
    char msg[512];
    //sprintf(msg, "Frame: %d  Batch: %d", gli.currentFrame(), gli.currentBatch());
    //MessageBox(NULL, msg, "BatchesAsFrames message", MB_OK);
    gli.doSwapBuffers();
}

bool BatchesAsFrames::init(GLInstrument& gli)
{
    gliPtr = &gli;
    doSwap = new DoSwap;
    gli.registerBatchHandler(gli.After, *doSwap);
    return true;
}

BatchesAsFrames::~BatchesAsFrames()
{
    gliPtr->unregisterBatchHandler(GLInstrument::After);
    delete doSwap;
}
