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

#ifndef ACD
    #define ACD

#include "ACDTypes.h"
#include "ACDDevice.h"


class GPUDriver;

namespace acdlib
{

    /**
     * Creayes the Atila Common Driver object
     *
     * @retuns A ready to use ACDDevice interface
     */
    ACDDevice* createDevice(GPUDriver* driver);


    void destroyDevice(ACDDevice* acddev);

}

#endif // ACD
