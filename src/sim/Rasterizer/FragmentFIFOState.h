/**************************************************************************
 *
 * Copyright (c) 2002 - 2004 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 * $RCSfile: FragmentFIFOState.h,v $
 * $Revision: 1.1 $
 * $Author: vmoya $
 * $Date: 2005-04-06 11:04:17 $
 *
 * Fragment FIFO state definition file.
 *
 */

/**
 *
 *  @file FragmentFIFOState.h
 *
 *  This file defines the Fragment FIFO states.
 *
 */


#ifndef _FRAGMENTFIFOSTATE_

#define _FRAGMENTFIFOSTATE_


/**
 *
 *  Defines the Fragment FIFO states for the Hierarchical/Early Z box.
 *
 */

enum FFIFOState
{
    FFIFO_READY,        /**<  The Fragment FIFO test can receive new stamps.  */
    FFIFO_BUSY          /**<  The Fragment FIFO can not receive new stamps.  */
};

#endif
