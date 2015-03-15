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
 * $RCSfile: ShaderDecodeCommand.h,v $
 * $Revision: 1.3 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:59 $
 *
 * Shader Decode Command.
 *
 */

/**
 *
 *  @file ShaderDecodeCommand.h
 *
 *  Defines ShaderDecodeCommand class.   This class stores commands
 *  from the decode/execute box to the Fetch box.
 *
 */

#include "GPUTypes.h"
#include "DynamicObject.h"

#ifndef _SHADERDECODECOMMAND_

#define _SHADERDECODECOMMAND_

namespace gpu3d
{

/**  Commands that can be received from Shader.  */
enum DecodeCommand
{
    UNBLOCK_THREAD,     /**<  Unblock the thread because the blocking instruction has finished.  */
    BLOCK_THREAD,       /**<  Block the thread because of the execution of long latency instruction.  */
    END_THREAD,         /**<  The thread has executed the last instruction.  */
    REPEAT_LAST,        /**<  Ask fetch to resend the last instruction of the thread.  */
    NEW_PC,             /**<  Send a new PC for the thread (branch, call, ret).  */
    ZEXPORT_THREAD      /**<  The thread has executed a z export instruction.  */
};

/**
 *
 *  Defines a Shader Decode Command from the Decode/Execute box to the Fetch box.
 *
 *  This class stores information about commands sent to the Fetch box
 *  from the decode/execute box.
 *  This class inherits from the DynamicObject class which offers dynamic
 *  memory management and tracing support.
 *
 */

class ShaderDecodeCommand : public DynamicObject
{

private:

    DecodeCommand command;      /**<  Type of the decode command.  */
    u32bit numThread;           /**<  Thread Number for the command.  */
    u32bit PC;                  /**<  New PC for the thread.  */

public:

    /**
     *
     *  ShaderDecodeCommand constructor.
     *
     *  Creates a ShaderDecodeCommand without any parameters.
     *
     *  @param comm  Command type for the ShaderDecodeCommand.
     *  @param threadID  Thread identifier for which the command is issued.
     *  @param PC  New PC for thread branches, calls, rets.
     *  @return A ShaderDecodeCommand object.
     *
     */

    ShaderDecodeCommand(DecodeCommand comm, u32bit threadID, u32bit PC);

    /**
     *
     *  Returns the type of the command.
     *
     *  @return  Type of the command.
     *
     */

    DecodeCommand getCommandType();

    /**
     *
     *  Returns the new PC for the thread.
     *
     *  @return New PC for the thread.
     *
     */

    u32bit getNewPC();

    /**
     *
     *  Returns the thread identifier.
     *
     *  @return The thread identifier for which the command is issued.
     *
     */

    u32bit getNumThread();

};

} // namespace gpu3d

#endif
