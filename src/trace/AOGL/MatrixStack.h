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

#ifndef MATRIX_STACK
    #define MATRIX_STACK

#include "ACDXGlobalTypeDefinitions.h"

namespace agl
{

class MatrixStack
{
public:

    virtual void multiply(const acdlib::ACDXFloatMatrix4x4& mat) = 0;

    virtual void set(const acdlib::ACDXFloatMatrix4x4& mat) = 0;

    virtual acdlib::ACDXFloatMatrix4x4 get() const = 0;

    virtual void push() = 0;

    virtual void pop() = 0;
};

}

#endif // MATRIX_STACK
