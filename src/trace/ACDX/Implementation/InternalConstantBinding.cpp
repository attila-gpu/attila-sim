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

#include "InternalConstantBinding.h"
#include "ACDXFixedPipelineStateImp.h"

using namespace acdlib;

InternalConstantBinding::InternalConstantBinding(acd_uint glStateId, acd_bool readGLState, acd_uint constantIndex, const ACDXBindingFunction* function, ACDXFloatVector4 directSource)
:    _glStateId(glStateId), _readGLState(readGLState), _constantIndex(constantIndex), _function(function), _directSource(directSource)
{

}

InternalConstantBinding::~InternalConstantBinding()
{
    //delete _function;
}

void InternalConstantBinding::resolveConstant(const ACDXFixedPipelineState* fpState, ACDXFloatVector4& constant) const
{
    vector<ACDXFloatVector4> resolvedStates;

    // Initialize the output constant to "zeros".
    constant[0] = constant[1] = constant[2] = constant[3] = acd_float(0);

    /* Get the pointer to the ACDXFixedPipelineState implementation class */
    const ACDXFixedPipelineStateImp* fpStateImp = static_cast<const ACDXFixedPipelineStateImp*>(fpState);

    // Check if binding function was initialized
    if (!_function)
        panic("InternalConstantBinding","resolveConstant()","No binding function object specified");

    if (_readGLState)
    {
        if (fpStateImp->getGLState()->isMatrix(_glStateId))
        {
            // It´s matrix state
            const ACDXMatrixf& mat = fpStateImp->getGLState()->getMatrix(_glStateId);

            for (int i=0; i<4; i++)
                resolvedStates.push_back(ACDXFloatVector4(&(mat.getRows()[i*4])));
        }
        else
        {
            // It´s vector state
            const Quadf& vect = fpStateImp->getGLState()->getVector(_glStateId);
            
            ACDXFloatVector4 state;
            state[0] = vect[0]; state[1] = vect[1]; state[2] = vect[2]; state[3] = vect[3]; 

            resolvedStates.push_back(state);
        }
    }
    // Execute the binding function
    _function->function(constant, resolvedStates, _directSource);
}

acd_uint InternalConstantBinding::getConstantIndex() const
{
    return _constantIndex;
}

void InternalConstantBinding::print(std::ostream& os) const
{
    os << "(" << _constantIndex << ") ";

    if (_readGLState)
        os << "ACDXGLState Id = " << _glStateId;

    os << "Direct src: {" << _directSource[0] << "," << _directSource[1] << "," << _directSource[2] << "," << _directSource[3] << "} ";
    os << endl;
}
