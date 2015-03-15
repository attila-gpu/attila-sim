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

#ifndef MEMORYCONTROLLERCOMMAND_H
    #define MEMORYCONTROLLERCOMMAND_H

#include "DynamicObject.h"
#include "GPU.h"

namespace gpu3d
{

/**
 * Memory Controller command types
 */
enum MCCommand
{
    MCCOM_REG_WRITE,
    MCCOM_LOAD_MEMORY,
    MCCOM_SAVE_MEMORY
};

/**
 * Memory controller commands are created in the Command Processor unit
 * and sent to the MemoryController to change the MemoryController state
 */
class MemoryControllerCommand : public DynamicObject
{
private:

    MCCommand command; ///< The memory controller command issued.
    GPURegister reg; ///< The memory controller register to write or read.
    GPURegData data; ///< Data to write or read to/from the memory controller register.

    MemoryControllerCommand();
    MemoryControllerCommand(MCCommand cmd, GPURegister reg, GPURegData data);
    MemoryControllerCommand(MCCommand cmd);
    MemoryControllerCommand(const MemoryControllerCommand&);
    MemoryControllerCommand& operator=(const MemoryControllerCommand&);

public:

    /**
     * Creates a write register memory controller command
     */
    static MemoryControllerCommand* createRegWrite(GPURegister reg, GPURegData data);
    static MemoryControllerCommand* createLoadMemory();
    static MemoryControllerCommand* createSaveMemory();

    /**
     * Gets the type of this Memory Controller Command.
     *
     * @return The type of command for this MemoryControllerCommand object.
     */
    MCCommand getCommand() const;

    /**
     * Returns the target register to read/write
     *
     * @return The destination/source memory controller register identifier.
     */
    GPURegister getRegister() const;

    /**
     * Gets the data to write to the memory controller register.
     *
     * @return Data to write to the memory controller register.
     */
    GPURegData getRegisterData() const;
};

} // namespace gpu3d

#endif
