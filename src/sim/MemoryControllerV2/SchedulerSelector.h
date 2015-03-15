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

#ifndef SCHEDULERSELECTOR_H
    #define SCHEDULERSELECTOR_H

#include "ChannelScheduler.h"
#include "MemoryControllerV2.h"

namespace gpu3d
{
namespace memorycontroller
{

ChannelScheduler* createChannelScheduler(const char* name, 
                                         const char* prefix, 
                                         const MemoryControllerParameters& params,
                                         Box* parent);

} // namespace memorycontroller
} // namespace gpu3d


#endif // SCHEDULERSELECTOR_H
