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

#include "ACDSupport.h"

using namespace acdlib;

acd_float acdsupport::clamp(acd_float v)
{
    return ((v)>1.0f?1.0f:((v)<0.0f?0.0f:(v)));
}

void acdsupport::clamp_vect4(acd_float* vect4)
{
    vect4[0] = acdlib::acdsupport::clamp(vect4[0]);
    vect4[1] = acdlib::acdsupport::clamp(vect4[1]);
    vect4[2] = acdlib::acdsupport::clamp(vect4[2]);
    vect4[3] = acdlib::acdsupport::clamp(vect4[3]);
}
