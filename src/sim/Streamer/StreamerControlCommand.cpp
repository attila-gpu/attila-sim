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
 * $RCSfile: StreamerControlCommand.cpp,v $
 * $Revision: 1.4 $
 * $Author: vmoya $
 * $Date: 2005-11-11 15:41:49 $
 *
 * Streamer Control Command class implementation file.
 *
 */

/**
 *
 *  @file StreamerControlCommand.cpp
 *
 *  Implements the Streamer Control Command class.
 *
 *
 */

#include "StreamerControlCommand.h"
#include "support.h"
#include <stdio.h>

using namespace gpu3d;

/*  Constructor for STRC_DEALLOC_X.  */
StreamerControlCommand::StreamerControlCommand(StreamControl comm, u32bit entry)
{
    /*  Check command.  */
    GPU_ASSERT(
        if ((comm != STRC_DEALLOC_IRQ) && (comm != STRC_DEALLOC_OFIFO)
            && (comm != STRC_DEALLOC_OM) && (comm != STRC_DEALLOC_OM_CONFIRM)
            && (comm != STRC_OM_ALREADY_ALLOC))
            panic("StreamerControlCommand", "StreamerControlCommand", "No deallocation command.");
    )

    /*  Set command.  */
    command = comm;

    /*  Set entry to deallocate.  */
    switch(command)
    {
        case STRC_DEALLOC_IRQ:
            IRQEntry = entry;
            setUnitID(0);
            sprintf((char *) getInfo(), "DEALLOC_IRQ entry = %d", getIRQEntry());
            break;

        case STRC_DEALLOC_OFIFO:
            OFIFOEntry = entry;
            sprintf((char *) getInfo(), "DEALLOC_OFIFO entry = %d", getOFIFOEntry());
            break;

        case STRC_DEALLOC_OM:
            OMLine = entry;
            sprintf((char *) getInfo(), "DEALLOC_OM entry = %d", getOMLine());
            break;

        case STRC_DEALLOC_OM_CONFIRM:
            OMLine = entry;
            sprintf((char *) getInfo(), "DEALLOC_OM_CONFIRM entry = %d", getOMLine());
            break;

        case STRC_OM_ALREADY_ALLOC:
            OMLine = entry;
            sprintf((char *) getInfo(), "OM_ALREADY_ALLOC entry = %d", getOMLine());
            break;

        default:
            panic("StreamerControlCommand", "StreamerControlCommand", "Wrong deallocation command.");
    }

    /*  Set color for signal tracing.  */
    setColor(command);

    setTag("stCCom");
}

/*  Constructor for STRC_NEW_INDEX and STRC_UPDATE.  */
StreamerControlCommand::StreamerControlCommand(StreamControl com, u32bit idx, u32bit instanceIdx, u32bit entry)
{
    /*  Set the command.  */
    command = com;

    /*  Set the index.  */
    index = idx;

    //  Set the instance index.
    instanceIndex = instanceIdx;
    
    switch(com)
    {
        case STRC_NEW_INDEX:

            /*  Set Output FIFO entry.  */
            OFIFOEntry = entry;
            sprintf((char *) getInfo(), "NEW_INDEX idx = %d inst = %d OFIFOentry = %d", index, instanceIndex, OFIFOEntry);
            break;

        case STRC_UPDATE_OC:

            /*  Set Output Memory line.  */
            OMLine = entry;
            sprintf((char *) getInfo(), "UPDATE_OC idx = %d inst = %d OMLine = %d", index, instanceIndex, OMLine);
            break;

        default:
            panic("StreamerControlCommand", "StreamerControlCommand", "Wrong control command.");
            break;
    }

    /*  Set color for signal tracing.  */
    setColor(command);

    setTag("stCCom");
}

/*  Returns the StreamerControlCommand command.  */
StreamControl StreamerControlCommand::getCommand()
{
    return command;
}

/*  Return the input index.  */
u32bit StreamerControlCommand::getIndex()
{
    return index;
}

//  Return the instance index.
u32bit StreamerControlCommand::getInstanceIndex()
{
    return instanceIndex;
}

/*  Return the IRQ entry.  */
u32bit StreamerControlCommand::getIRQEntry()
{
    return IRQEntry;
}

/*  Return the output FIFO entry.  */
u32bit StreamerControlCommand::getOFIFOEntry()
{
    return OFIFOEntry;
}

/*  Sets the output memory line found in the output cache.  */
void StreamerControlCommand::setOMLine(u32bit omLine)
{
    OMLine = omLine;
}

/*  Return the output memory line.  */
u32bit StreamerControlCommand::getOMLine()
{
    return OMLine;
}

/*  Sets the output cache hit attribute.  */
void StreamerControlCommand::setHit(bool hitOrMiss)
{
    hit = hitOrMiss;
}

/*  Return if it was a hit in the output cache.  */
bool StreamerControlCommand::isAHit()
{
    return hit;
}

/*  Set last index in batch flag.  */
void StreamerControlCommand::setLast(bool lastInBatch)
{
    last = lastInBatch;
}

/*  Return if the index is the last in the batch.  */
bool StreamerControlCommand::isLast()
{
    return last;
}

/*  Sets the streamer loader unit id (for STRC_DEALLOC_IRQ streamer commands).  */
void StreamerControlCommand::setUnitID(u32bit unit)
{
    unitID = unit;
}

/*  Gets the streamer loader unit id (for STRC_DEALLOC_IRQ streamer commands).  */
u32bit StreamerControlCommand::getUnitID()
{
    return unitID;
}

