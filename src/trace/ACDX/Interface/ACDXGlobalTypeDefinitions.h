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

#ifndef ACDX_GLOBAL_TYPE_DEFINITIONS_H
    #define ACDX_GLOBAL_TYPE_DEFINITIONS_H

#include "ACDTypes.h"
#include "ACDVector.h"
#include "ACDMatrix.h"

namespace acdlib
{
///////////////////////
//     Vectors       //
///////////////////////

/**
 * Vector of 4 acd_float components defined from the ACDVector class
 */
typedef ACDVector<acd_float,4> ACDXFloatVector4;

/**
 * Vector of 4 acd_float components defined from the ACDVector class
 */
typedef ACDVector<acd_float,3> ACDXFloatVector3;

//////////////////////
//    Matrices      //
//////////////////////
/**
 * Matrix of 4x4 acd_float components defined from the ACDMatrix class
 */
typedef ACDMatrix<acd_float,4,4> ACDXFloatMatrix4x4;

} // namespace acdlib

#endif // ACDX_GLOBAL_TYPE_DEFINITIONS_H
