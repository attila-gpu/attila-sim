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
 * $RCSfile: RasterizerState.h,v $
 * $Revision: 1.5 $
 * $Author: jroca $
 * $Date: 2007-01-19 19:33:59 $
 *
 * Rasterizer state definition file.
 *
 */


/**
 *
 *  @file RasterizerState.h
 *
 *  This file just defines the rasterizer state.
 *
 */

#ifndef _RASTERIZERSTATE_

#define _RASTERIZERSTATE_

namespace gpu3d
{

/***  Rasterizer states.  */
enum RasterizerState
{
    RAST_RESET,         /**<  The Rasterizer is resetting.  */
    RAST_READY,         /**<  The Rasterizer is ready.  */
    RAST_DRAWING,       /**<  The Rasterizer is displaying a batch of vertexes.  */
    RAST_BUSY,          /**<  The Rasterizer is busy.  */
    RAST_END,           /**<  The Rasterizer has finished displaying a batch of vertexes.  */
    RAST_SWAP,          /**<  Fragment Operations swap buffers state.  */
    RAST_DUMP_BUFFER,   /**<  Dump buffer (debug only!).  */
    RAST_FLUSH,         /**<  Fragment Operations flush state. */
    RAST_SAVE_STATE,    /**<  Fragment Operations save block state info state.  */
    RAST_RESTORE_STATE, /**<  Fragment Operations restore block state info state.  */
    RAST_RESET_STATE,   /**<  Fragment Operations buffer block state reset state.  */
    RAST_BLIT,          /**<  Fragment Operations blit state. */
    RAST_CLEAR,         /**<  Fragment Operations clear buffers state.  */
    RAST_CLEAR_END      /**<  End of clear buffer state.  */
};

} // namespace gpu3d

#endif
