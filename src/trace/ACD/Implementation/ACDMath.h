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

#ifndef ACD_MATH
    #define ACD_MATH

#include "ACDTypes.h"

namespace acdlib
{
    template<class T> T max(T a, T b) { return (a>b?a:b); } 
    template<class T> T min(T a, T b) { return (a<b?a:b); }
    acd_double ceil(acd_double a);
    acd_double logTwo(acd_double a);
    acd_int abs(acd_int a);
}

#endif // ACD_MATH
