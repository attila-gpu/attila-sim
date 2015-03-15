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

#include "MatrixStackImp.h"
#include "support.h"

using namespace agl;
using namespace acdlib;

MatrixStackProjection::MatrixStackProjection(ACDXTransformAndLightingStage* tl) : _tl(tl)
{}

void MatrixStackProjection::multiply(const acdlib::ACDXFloatMatrix4x4& mat)
{
    ACDXFloatMatrix4x4 newMatrix = _tl->getProjectionMatrix() * mat;
    _tl->setProjectionMatrix(newMatrix);
}

void MatrixStackProjection::set(const acdlib::ACDXFloatMatrix4x4& mat)
{
    _tl->setProjectionMatrix(mat);
}

ACDXFloatMatrix4x4 MatrixStackProjection::get() const
{
    return _tl->getProjectionMatrix();
}

void MatrixStackProjection::push()
{
    _stack.push(_tl->getProjectionMatrix());
}

void MatrixStackProjection::pop()
{
    if ( _stack.empty() )
        panic("agl::MatrixStackProjection", "pop", "Stack pop method failed. Stack is empty");

    _tl->setProjectionMatrix(_stack.top());
    _stack.pop(); // discard top
}


MatrixStackModelview::MatrixStackModelview(ACDXTransformAndLightingStage* tl, acd_uint unit) : _tl(tl), _UNIT(unit)
{}

void MatrixStackModelview::multiply(const acdlib::ACDXFloatMatrix4x4& mat)
{
    ACDXFloatMatrix4x4 newMatrix = _tl->getModelviewMatrix(_UNIT) * mat;
    _tl->setModelviewMatrix(_UNIT, newMatrix);
}

void MatrixStackModelview::set(const acdlib::ACDXFloatMatrix4x4& mat)
{
    _tl->setModelviewMatrix(_UNIT, mat);
}

ACDXFloatMatrix4x4 MatrixStackModelview::get() const
{
    return _tl->getModelviewMatrix(_UNIT);
}

void MatrixStackModelview::push()
{
    _stack.push(_tl->getModelviewMatrix(_UNIT));
}

void MatrixStackModelview::pop()
{
    if ( _stack.empty() )
        panic("agl::MatrixStackModelview", "pop", "Stack pop method failed. Stack is empty");

    _tl->setModelviewMatrix(_UNIT, _stack.top());
    _stack.pop(); // discard top
}


MatrixStackTextureCoord::MatrixStackTextureCoord(ACDXTextCoordGenerationStage* tl, acd_uint unit) : _tl(tl), _UNIT(unit)
{}

void MatrixStackTextureCoord::multiply(const acdlib::ACDXFloatMatrix4x4& mat)
{
    ACDXFloatMatrix4x4 newMatrix = _tl->getTextureCoordMatrix(_UNIT) * mat;
    _tl->setTextureCoordMatrix(_UNIT, newMatrix);
}

void MatrixStackTextureCoord::set(const acdlib::ACDXFloatMatrix4x4& mat)
{
    _tl->setTextureCoordMatrix(_UNIT, mat);
}

ACDXFloatMatrix4x4 MatrixStackTextureCoord::get() const
{
    return _tl->getTextureCoordMatrix(_UNIT);
}

void MatrixStackTextureCoord::push() 
{ 
    _stack.push(_tl->getTextureCoordMatrix(_UNIT));
}

void MatrixStackTextureCoord::pop()
{
    if ( _stack.empty() )
        panic("agl::MatrixStackTextureCoord", "pop", "Stack pop method failed. Stack is empty");

    _tl->setTextureCoordMatrix(_UNIT, _stack.top());
    _stack.pop(); // discard top
}
