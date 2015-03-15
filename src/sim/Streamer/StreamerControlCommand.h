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
 * $RCSfile: StreamerControlCommand.h,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2005-05-02 10:26:32 $
 *
 * Streamer Control Command class definition file.
 *
 */

/**
 *  @file StreamerControlCommand.h
 *
 *  This file defines the Streamer Control Command class.
 *
 */

#ifndef _STREAMERCONTROLCOMMAND_

#define _STREAMERCONTROLCOMMAND_

#include "GPUTypes.h"
#include "DynamicObject.h"

namespace gpu3d
{

/*  Streamer Control Command.  */
enum StreamControl
{
    STRC_NEW_INDEX,
    STRC_DEALLOC_IRQ,
    STRC_DEALLOC_OFIFO,
    STRC_DEALLOC_OM,
    STRC_DEALLOC_OM_CONFIRM,
    STRC_OM_ALREADY_ALLOC,
    STRC_UPDATE_OC
};


/**
 *
 *  This class defines objects that carry control information
 *  between the different Streamer simulation boxes.
 *  The class inherits from the DynamicObject class that
 *  offers dynamic memory management and signal tracing
 * support.
 *
 */

class StreamerControlCommand : public DynamicObject
{

private:

    StreamControl command;  /**<  The streamer control command.  */
    u32bit index;           /**<  A new vertex/input index to search/add.  */
    u32bit instanceIndex;   /**<  Instance index.  */
    u32bit unitID;          /**<  The streamer loader unit processing the index.  */
    u32bit IRQEntry;        /**<  A vertex request queue entry identifier.  */
    u32bit OFIFOEntry;      /**<  An output FIFO entry identifier.  */
    u32bit OMLine ;         /**<  An output memory line identifier.  */
    bool hit;               /**<  The index was found in the output cache.  */
    bool last;              /**<  Flag marking the a vertex index as the last for the batch.  */

public:

    /**
     *
     *  Streamer control command constructor (for STRC_DEALLOC_X).
     *
     *  Creates a new Streamer Control Command object to deallocate
     *  Streamer resources.
     *
     *  @param command The streamer control command that is
     *  going to be created.
     *  @param An entry in one of the Streamer structures
     *  that must be deallocated.
     *
     *  @return A new initialized StreamerControlCommand.
     *
     */

    StreamerControlCommand(StreamControl command, u32bit entry);

    /**
     *
     *  Streamer control command constructor (for STRC_NEW_INDEX
     *  and STRC_UPDATE_OC).
     *
     *  Creates a new Streamer Control Command object to search
     *  a new index in the output cache and to add the new index
     *  in the VRQ and output FIFO.
     *  Or creates a new Streamer Control Command object to ask
     *  for an update of the output cache.
     *
     *  @param command The streamer control command to create.
     *  @param index The new input index to add/update/search/ for.
     *  @param instanceIndex The instance index corresponding to the current index.
     *  @param entry Output FIFO for the new index or the output memory line for the output cache update.
     *
     *  @return A new initialized StreamerControlCommand object.
     *
     */

    StreamerControlCommand(StreamControl command, u32bit index, u32bit instanceIndex, u32bit entry);

    /**
     *
     *  Gets the Streamer Control Command.
     *
     *  @return The streamer control command for this object.
     *
     */

    StreamControl getCommand();

    /**
     *
     *  Gets the input index.
     *
     *  @return The input index.
     *
     */

    u32bit getIndex();

    /**
     *
     *  Gets the instance index associated with the current index.
     *
     *  @return The instance index for the current index.
     *
     */
     
    u32bit getInstanceIndex();
    
    /**
     *
     *  Gets the IRQ entry.
     *
     *  @return The IRQ entry.
     *
     */

    u32bit getIRQEntry();

    /**
     *
     *  Gets the output FIFO entry.
     *
     *  @return The output FIFO entry.
     *
     */

    u32bit getOFIFOEntry();

    /**
     *
     *  Sets the output memory line for the streamer control command.
     *
     *  @param omEntry The output memory line.
     *
     */

    void setOMLine(u32bit omLine);

    /**
     *
     *  Gets the Output Memory line.
     *
     *  @return The output memory line.
     *
     */

    u32bit getOMLine();

    /**
     *
     *  Sets output cache hit attribute.
     *
     *  @param hit If there was a hit in the output cache
     *  for the input index.
     *
     */

    void setHit(bool hit);

    /**
     *
     *  Gets if there was a hit in the output cache for the input index.
     *
     *  @return If there was a hit in the output cache.
     *
     */

    bool isAHit();

    /**
     *
     *  Sets the index as the last one in the batch.
     *
     *  @param last Boolean storing if the index is the last in the batch.
     *
     */

    void setLast(bool last);

    /**
     *
     *  Gets the last in batch mark for the index.
     *
     *  @return A boolean value representing if the index as the last in the batch.
     *
     */

    bool isLast();

    /**
     *
     *  Sets the streamer loader unit id (for STRC_DEALLOC_IRQ streamer commands).
     *
     *  @param unit The streamer loader unit id.
     *
     */

    void setUnitID(u32bit unit);

    /**
     *
     *  Gets the streamer loader unit id (for STRC_DEALLOC_IRQ streamer commands).
     *
     *  @return The streamer loader unit id.
     *
     */

    u32bit getUnitID();

};

} // namespace gpu3d

#endif
