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
 * $RCSfile: StreamerCommand.h,v $
 * $Revision: 1.2 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:29:04 $
 *
 * Streamer Command class definition file. 
 *
 */

#ifndef _STREAMER_COMMAND_

#define _STREAMER_COMMAND_

#include "DynamicObject.h"
#include "support.h"
#include "GPUTypes.h"
#include "GPU.h"

namespace gpu3d
{

/***  Streamer Commands.  */
enum StreamComm
{
    STCOM_RESET,            /**<  Reset Streamer unit command.  */
    STCOM_REG_WRITE,        /**<  Streamer state register write command.  */
    STCOM_REG_READ,         /**<  Streamer state register read command.  */
    STCOM_START,            /**<  Streamer start streaming command.  */
    STCOM_END               /**<  Streamer end streaming command.  */
};

/**
 *
 *  This class defines Streamer Command objects.
 *
 *  A Streamer Command carries dynamically created orders and
 *  state changes from the Command Processor to the Streamer
 *  unit.
 *  This class inherits from the Dynamic Object class that offers
 *  dynamic memory management and trace support.
 *
 */
 
class StreamerCommand : public DynamicObject
{

private:

    StreamComm command;     /**<  The command issued by the Streamer Commad.  */
    GPURegister streamReg;  /**<  Streamer state register to write/read.  */
    u32bit streamSubReg;    /**<  Streamer state register subregister to write/read.  */
    GPURegData data;        /**<  Data to write/read to the streamer state register.  */

public:

    /**
     *
     *  StreamerCommand constructor.
     *  
     *  Creates a new Streamer Command object (for STCOM_RESET, STCOM_START and
     *  STCOM_END).
     *
     *  @param comm Type of Streamer Command.
     *
     *  @return An initialized StreamerCommand object.
     *
     */
     
    StreamerCommand(StreamComm comm);

    /**
     *
     *  Streamer Command constructor.
     *
     *  Creates a new Streamer Command Object (for STCOM_REG_WRITE).
     *
     *  @param reg Streamer register to write.
     *  @param subReg Streamer register subregister to write.
     *  @param data Data to write to the Streamer register.
     *
     *  @return An initialized register write Streamer Command object.
     *
     */
     
    StreamerCommand(GPURegister reg, u32bit subReg, GPURegData data);
    
    /**
     *
     *  Returns the Streamer Command type.
     *
     *  @return The Streamer Command type.
     *
     */
     
    StreamComm getCommand();
    
    /**
     *
     *  Returns the Streamer register from where to read/write.
     *
     *  @return The Streamer register destination of the Streamer Command.
     *
     */
     
    GPURegister getStreamerRegister();
    
    /**
     *
     *  Returns the Streamer register subregister from where to read/write.
     *
     *  @return The Streamer register subregister destination of the 
     *  Streamer Command.
     *
     */
     
    u32bit getStreamerSubRegister();
    
    /**
     *
     *  Returns the data to write to a Streamer register.
     * 
     *  @return Data to write to a Streamer Register.
     *
     */
     
    GPURegData getRegisterData();
    
};

} // namespace gpu3d

#endif
