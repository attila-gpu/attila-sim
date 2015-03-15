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

#ifndef IMPLEMENTED_CONSTANT_BINDING_FUNCTIONS_H
    #define IMPLEMENTED_CONSTANT_BINDINGS_FUNCTIONS_H

#include "ACDXConstantBinding.h"
#include <cmath>

namespace acdlib
{

/**
 * Function for rescaling factor computing
 */
class ScaleFactorFunction: public acdlib::ACDXBindingFunction
{
public:

    virtual void function(acdlib::ACDXFloatVector4& constant, 
                          std::vector<acdlib::ACDXFloatVector4> vState,
                          const acdlib::ACDXFloatVector4&) const
    {
        acdlib::ACDXFloatMatrix4x4 mat, invMat;
        
        mat[0] = vState[0];    mat[1] = vState[1];    mat[2] = vState[2];    mat[3] = vState[3];

        acdlib::_inverse(invMat,mat);
        
        constant = 1 / acdlib::_length(invMat[2]); // Without indexing assigns the value to all the components
    };
};

/**
 * Function for light position vector normalization
 */
class LightPosNormalizeFunction: public acdlib::ACDXBindingFunction
{
public:

    virtual void function(acdlib::ACDXFloatVector4& constant, 
                          std::vector<acdlib::ACDXFloatVector4> vState,
                          const acdlib::ACDXFloatVector4&) const
    {
        vState[0][3] = 0.0f;
        constant = acdlib::_normalize(vState[0]);
        constant[3] = 0.0f;
    };

};

/**
 * Function for linear fog parameters computation
 *
 * @note The constant vector needed c = {-1/(END-START), END/(END-START), NOT USED, NOT USED};
 */
class LinearFogParamsFunction: public acdlib::ACDXBindingFunction
{
public:
    
    virtual void function(acdlib::ACDXFloatVector4& constant, 
                          std::vector<acdlib::ACDXFloatVector4> vState,
                          const acdlib::ACDXFloatVector4&) const
    {
        constant[0] = acdlib::acd_float(-1) / (vState[1][0] - vState[0][0]);
        constant[1] = vState[1][0]/(vState[1][0] - vState[0][0]); //constant[0] * vState[1][0];
    };
};

/**
 * Function for exponential fog parameters computation
 *
 * @note The constant vector needed c = {DENSITY/LN(2), NOT USED, NOT USED, NOT USED};
 */
class ExponentialFogParamsFunction: public acdlib::ACDXBindingFunction
{
public:
    virtual void function(acdlib::ACDXFloatVector4& constant, 
                          std::vector<acdlib::ACDXFloatVector4> vState,
                          const acdlib::ACDXFloatVector4&) const
    {
        constant[0] =  vState[0][0] / acdlib::acd_float(std::log(2.0));
    };
};

/**
 * Function for second order exponential fog parameters computation
 *
 * @note The constant vector needed c = {DENSITY/SQRT(LN(2)), NOT USED, NOT USED, NOT USED};
 */
class SecondOrderExponentialFogParamsFunction: public acdlib::ACDXBindingFunction
{
public:
    virtual void function(acdlib::ACDXFloatVector4& constant, 
                          std::vector<acdlib::ACDXFloatVector4> vState,
                          const acdlib::ACDXFloatVector4&) const
    {
        constant[0] =  vState[0][0] / acdlib::acd_float(std::sqrt(std::log(2.0)));
    };
};

/**
 * Function to copy single matrix rows to 
 */
class CopyMatrixRowFunction: public acdlib::ACDXBindingFunction
{
private:

    acdlib::acd_uint _row;

public:

    CopyMatrixRowFunction(acdlib::acd_uint row): _row(row) {};

    virtual void function(acdlib::ACDXFloatVector4& constant, 
                          std::vector<acdlib::ACDXFloatVector4> vState,
                          const acdlib::ACDXFloatVector4&) const
    {
        constant = vState[_row];
    };
};

/**
 * Function to replicate the Alpha test reference value to all the
 * vector components.
 */
class AlphaRefValueFunction: public acdlib::ACDXBindingFunction
{
public:

    virtual void function(acdlib::ACDXFloatVector4& constant, 
                          std::vector<acdlib::ACDXFloatVector4> vState,
                          const acdlib::ACDXFloatVector4&) const
    {
        constant[0] = constant[1] = constant[2] = constant[3] = vState[0][0];
    };

};

} // namespace acdlib

#endif // IMPLEMENTED_CONSTANT_BINDINGS_FUNCTIONS_H
