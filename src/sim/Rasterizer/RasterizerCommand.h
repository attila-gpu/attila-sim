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
 * $RCSfile: RasterizerCommand.h,v $
 * $Revision: 1.7 $
 * $Author: vmoya $
 * $Date: 2007-01-21 18:57:45 $
 *
 * Rasterizer Command class definition file.
 *
 */

#ifndef _RASTERIZERCOMMAND_

#define _RASTERIZERCOMMAND_

#include "DynamicObject.h"
#include "support.h"
#include "GPUTypes.h"
#include "GPU.h"

namespace gpu3d
{

/***  Rasterizer Command types.  */
enum RastComm
{
    RSCOM_RESET,                    /**<  Reset the Rasterizer.  */
    RSCOM_REG_WRITE,                /**<  Write a Rasterizer register.  */
    RSCOM_REG_READ,                 /**<  Read a Rasterizer register.  */
    RSCOM_DRAW,                     /**<  Start drawing a batch of transformed triangles.  */
    RSCOM_END,                      /**<  End drawing of a vertex batch.  */
    RSCOM_CLEAR_COLOR_BUFFER,       /**<  Clear the color buffer.  */
    RSCOM_CLEAR_ZSTENCIL_BUFFER,    /**<  Clear the depth and stencil buffer.  */
    RSCOM_SWAP,                     /**<  End the scene and swap front and back buffers.  */
    RSCOM_FRAME_CHANGE,             /**<  Fake swap sent to the DAC when the simulator is skipping draw calls.  */
    RSCOM_DUMP_COLOR,               /**<  Dump the color buffer (debug only!).  */
    RSCOM_DUMP_DEPTH,               /**<  Dump the depth buffer (debug only!).  */
    RSCOM_DUMP_STENCIL,             /**<  Dump the stencil buffer (debug only!).  */
    RSCOM_FLUSH,                    /**<  Flush caches to memory (z and stencil or color).  */
    RSCOM_SAVE_STATE,               /**<  Save (color or zstencil) block state info into memory.  */
    RSCOM_RESTORE_STATE,            /**<  Restore (color or zstencil) block state info from memory.  */
    RSCOM_RESET_STATE,              /**<  Reset buffer block state.  */
    RSCOM_BLIT                      /**<  Wait Framebuffer update and bit blit operation end.  */
};

/**
 *
 *  This class defines Rasterizer Command objects.
 *
 *  Rasterizer commad objects are created in the Command Processor
 *  and sent to the Rasterizer (fake) to change the Rasterizer
 *  state and issue commands to the Rasterizer.
 *  Inherits from DynamicObject class that implements dynamic
 *  memory management and tracing.
 *
 */

class RasterizerCommand : public DynamicObject
{

private:

    RastComm command;       /**<  The rasterizer command issued.  */
    GPURegister reg;        /**<  The Rasterizer register to write or read.  */
    u32bit subReg;          /**<  Rasterizer register subregister to write or read.  */
    GPURegData data;        /**<  Data to write or read from the Rasterizer register.  */


public:

    /**
     *
     *  Rasterizer Command constructor.
     *
     *  This function creates and initializes a new Rasterizer Command object.
     *  This function can be used to create RSCOM_RESET and RSCOM_SWAP
     *  commands only.
     *
     *  @param comm The type of Rasterizer Command to create.
     *  @return An initialized Rasterizer Command object of the requested
     *  type.
     *
     */

    RasterizerCommand(RastComm comm);

    /**
     *
     *  Rasterizer Command constructor.
     *
     *  This function crates and initializes a new Rasterizer Command object
     *  of type RSCOM_REG_WRITE.
     *
     *  @param reg Rasterizer register to write to.
     *  @param subReg Rasterizer register subregister to write to.
     *  @param data Data to write to the Rasterizer register.
     *  @return An initialized Rasterizer Command object for a write
     *  register request.
     *
     */

    RasterizerCommand(GPURegister reg, u32bit subReg, GPURegData data);

    /**
     *
     *  Gets the type of this Rasterizer Command.
     *
     *  @return The type of command for this Rasterizer Command object.
     *
     */

    RastComm getCommand();


    /**
     *
     *  Gets the Rasterizer register identifier to write to or read from
     *  for this Rasterizer Command object.
     *
     *  @return The destination/source Rasterizer register identifier.
     *
     */

    GPURegister getRegister();

    /**
     *
     *  Gets the Rasterizer register subregister to write or to read
     *  from for this Rasterizer Command object.
     *
     *  @return The destination/source Rasterizer register subregister
     *  number.
     *
     */

    u32bit getSubRegister();

    /**
     *
     *  Gets the data to write to the Rasterizer register.
     *
     *  @return Data to write to the Rasterizer register.
     *
     */

    GPURegData getRegisterData();

};

} // namespace gpu3d

#endif
