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

#ifndef ACD_SUPPORT
    #define ACD_SUPPORT

#include "ACDTypes.h"

namespace acdlib
{

namespace acdsupport
{
    acd_float clamp(acd_float value);

    void clamp_vect4(acd_float* vect4);
}

}

#endif // ACD_SUPPORT
