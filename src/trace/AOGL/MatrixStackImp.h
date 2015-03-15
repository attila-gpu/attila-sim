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

#ifndef MATRIX_STACK_IMP
    #define MATRIX_STACK_IMP

#include "MatrixStack.h"
#include "ACDXFixedPipelineState.h"
#include "ACDXTextCoordGenerationStage.h"
#include <stack>

namespace agl
{

class MatrixStackProjection : public MatrixStack
{
public:

    MatrixStackProjection(acdlib::ACDXTransformAndLightingStage* tl);

    void multiply(const acdlib::ACDXFloatMatrix4x4& mat);

    void set(const acdlib::ACDXFloatMatrix4x4& mat);

    acdlib::ACDXFloatMatrix4x4 get() const;

    void push();

    void pop();

private:

    acdlib::ACDXTransformAndLightingStage* _tl;
    std::stack<acdlib::ACDXFloatMatrix4x4> _stack;
};
    
class MatrixStackModelview : public MatrixStack
{
public:

    MatrixStackModelview(acdlib::ACDXTransformAndLightingStage* tl, acdlib::acd_uint unit);

    void multiply(const acdlib::ACDXFloatMatrix4x4& mat);

    void set(const acdlib::ACDXFloatMatrix4x4& mat);

    acdlib::ACDXFloatMatrix4x4 get() const;

    void push();

    void pop();

private:

    acdlib::ACDXTransformAndLightingStage* _tl;
    const acdlib::acd_uint _UNIT;
    std::stack<acdlib::ACDXFloatMatrix4x4> _stack;
};


class MatrixStackTextureCoord : public MatrixStack
{
public:

    MatrixStackTextureCoord(acdlib::ACDXTextCoordGenerationStage* tl, acdlib::acd_uint textureStage);

    void multiply(const acdlib::ACDXFloatMatrix4x4& mat);

    void set(const acdlib::ACDXFloatMatrix4x4& mat);

    acdlib::ACDXFloatMatrix4x4 get() const;

    void push();

    void pop();

private:

    acdlib::ACDXTextCoordGenerationStage* _tl;
    const acdlib::acd_uint _UNIT;
    std::stack<acdlib::ACDXFloatMatrix4x4> _stack;
};

} // namespace agl


#endif // MATRIX_STACK_IMP
