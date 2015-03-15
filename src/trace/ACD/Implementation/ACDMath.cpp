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

#include "ACDMath.h"
#include <cmath>

using namespace acdlib; // to use types acd_*

acd_double acdlib::ceil(acd_double x)
{
    return (x - std::floor(x) > 0)?std::floor(x) + 1:std::floor(x);
}

acd_double acdlib::logTwo(acd_double x)
{
    return std::log(x)/std::log(2.0);
}

acd_int acdlib::abs(acd_int a)
{
    return (acd_int) std::abs((acd_float) a);
}

