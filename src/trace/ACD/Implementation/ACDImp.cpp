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

#include "ACD.h"
#include "ACDDeviceImp.h"
#include "GPUDriver.h"

using namespace acdlib;

acdlib::ACDDevice* acdlib::createDevice(GPUDriver* driver)
{
    if ( driver == 0 ) // direct access to the single driver
        return new ACDDeviceImp(GPUDriver::getGPUDriver());

    return new ACDDeviceImp(driver);
}

void acdlib::destroyDevice(ACDDevice* acddev)
{
    delete static_cast<ACDDeviceImp*>(acddev);
}
