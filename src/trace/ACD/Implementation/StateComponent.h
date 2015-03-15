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

#ifndef STATE_COMPONENT
    #define STATE_COMPONENT

#include <string>

namespace acdlib
{

/**
 * Interface for all the subcomponents that owns ACD/GPU state and required synchronization with the GPU
 */
class StateComponent
{
public:

    virtual std::string getInternalState() const = 0;

    virtual void sync() = 0;

    virtual void forceSync() = 0;
};

}

#endif // STATE_COMPONENT
