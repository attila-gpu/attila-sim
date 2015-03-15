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

#include "ACDXConstantBindingImp.h"
#include "support.h"
#include <ostream>

using namespace acdlib;
using namespace std;

ACDXConstantBindingImp::ACDXConstantBindingImp(ACDX_BINDING_TARGET target, acd_uint constantIndex, std::vector<ACDX_STORED_FP_ITEM_ID> vStateIds, const ACDXBindingFunction* function, ACDXFloatVector4 directSource)
: 
_target(target), 
_constantIndex(constantIndex), 
_vStateIds(vStateIds), 
_directSource(directSource), 
_function(function)
{
};

ACDXConstantBindingImp::~ACDXConstantBindingImp()
{
    _vStateIds.clear();
    //delete _function;
}

void ACDXConstantBindingImp::resolveGLSMatrixState(const ACDXFixedPipelineStateImp* fpStateImp, MatrixId mId, MatrixType type, acd_uint unit, std::vector<ACDXFloatVector4>& mState) const
{
    // Read the current matrix state value.
    
    ACDXMatrixf mat;

    mat = fpStateImp->getGLState()->getMatrix(mId,type,unit);

    for (int i=0; i<4; i++)
        mState.push_back(ACDXFloatVector4(&(mat.getRows()[i*4])));
}

void ACDXConstantBindingImp::resolveGLSVectorState(const ACDXFixedPipelineStateImp* fpStateImp, VectorId vId, acd_uint componentMask, ACDXFloatVector4& vState) const
{
    Quadf vect;
    
    vect = fpStateImp->getGLState()->getVector(vId);

    switch (componentMask)
    {
        case 0x08: {
                       vState[0] = vect[0];
                       vState[1] = acd_float(0);
                       vState[2] = acd_float(0);
                       vState[3] = acd_float(1);
                   }
                   break;
        case 0x04: {
                       vState[0] = vect[1];
                         vState[1] = acd_float(0);
                       vState[2] = acd_float(0);
                       vState[3] = acd_float(1);
                   }
                   break;
        case 0x02: {
                       vState[0] = vect[2];
                          vState[1] = acd_float(0);
                       vState[2] = acd_float(0);
                       vState[3] = acd_float(1);
                   }
                   break;
        case 0x01: {
                       vState[0] = vect[3];
                          vState[1] = acd_float(0);
                       vState[2] = acd_float(0);
                       vState[3] = acd_float(1);
                   }
                   break;
        case 0x07: {
                       vState[0] = vect[1]; 
                       vState[1] = vect[2]; 
                       vState[2] = vect[3];
                       vState[3] = acd_float(1);
                   }
                   break;
        case 0x0B: {
                       vState[0] = vect[0]; 
                       vState[1] = vect[2]; 
                       vState[2] = vect[3]; 
                       vState[3] = acd_float(1);
                   }
                   break;
        case 0x0D: {
                       vState[0] = vect[0]; 
                       vState[1] = vect[1]; 
                       vState[2] = vect[3];
                       vState[3] = acd_float(1);
                   }
                   break;
        case 0x0E: {
                       vState[0] = vect[0]; 
                       vState[1] = vect[1]; 
                       vState[2] = vect[2];
                       vState[3] = acd_float(1);
                   }
                   break;
        case 0x0F: {
                       vState[0] = vect[0];
                       vState[1] = vect[1];
                       vState[2] = vect[2];
                       vState[3] = vect[3];
                   }
                   break;
        default:
            panic("ACDXConstantBindingImp","resolveGLSVectorState","Unexpected component mask");
    }

}

void ACDXConstantBindingImp::resolveConstant(const ACDXFixedPipelineState* fpState, ACDXFloatVector4& constant) const
{
    /* ACDXGLState indexing parameters */
    acd_bool isMatrix;
    VectorId vId;
    acd_uint componentMask = 0x0F;
    MatrixId mId;
    MatrixType type;
    acd_uint unit;

    vector<ACDXFloatVector4> resolvedStates;

    // Initialize the output constant to "zeros".
    constant[0] = constant[1] = constant[2] = constant[3] = acd_float(0);

    /* Get the pointer to the ACDXFixedPipelineState implementation class */
    const ACDXFixedPipelineStateImp* fpStateImp = static_cast<const ACDXFixedPipelineStateImp*>(fpState);

    // Check if binding function was initialized
    if (!_function)
        panic("ACDXConstantBindingImp","resolveConstant()","No binding function object specified");

    // Iterate over all requested states and push them back in the resolved states
    // (resolvedStates) vector.

    std::vector<ACDX_STORED_FP_ITEM_ID>::const_iterator iter = _vStateIds.begin();

    while ( iter != _vStateIds.end() )
    {
        ACDXFloatVector4 vState(acd_float(0));
        std::vector<ACDXFloatVector4> mState(4, ACDXFloatVector4(acd_float(0)));

        switch ((*iter))
        {
            //
            // Put here all the Fixed Pipeline states not present in ACDXGLState
            // The original state value in the ACDXFixedPipelineState will be forwarded instead
            ///
            case ACDX_ALPHA_TEST_REF_VALUE: 
                // Get value from fpStateImp

                vState[0] = fpStateImp->getSingleState(ACDX_ALPHA_TEST_REF_VALUE);
                vState[1] = acd_float(0);
                vState[2] = acd_float(0);
                vState[3] = acd_float(1);
                
                resolvedStates.push_back(vState);

                break;

            default: 
                
                // The remainder states need to be read from ACDXGLState.
                
                fpStateImp->getGLStateId((*iter), vId, componentMask, mId, unit, type, isMatrix);  

                if (isMatrix)
                {
                    resolveGLSMatrixState(fpStateImp, mId, type, unit, mState);
                    
                    resolvedStates.push_back(mState[0]);
                    resolvedStates.push_back(mState[1]);
                    resolvedStates.push_back(mState[2]);
                    resolvedStates.push_back(mState[3]);
                }
                else
                {    // It´s vector or single state
                    
                    resolveGLSVectorState(fpStateImp, vId, componentMask, vState);

                    resolvedStates.push_back(vState);
                }
                break;
        }
        iter++;
    }
    
    // Execute the binding function
    _function->function(constant, resolvedStates, _directSource);
}

acd_uint ACDXConstantBindingImp::getConstantIndex() const
{
    return _constantIndex;
}

ACDXFloatVector4 ACDXConstantBindingImp::getDirectSource() const
{
    return _directSource;
}

ACDX_BINDING_TARGET ACDXConstantBindingImp::getTarget() const
{
    return _target;
}

std::vector<ACDX_STORED_FP_ITEM_ID> ACDXConstantBindingImp::getStateIds() const
{
    return _vStateIds;
}

const ACDXBindingFunction* ACDXConstantBindingImp::getBindingFunction() const
{
    return _function;
}

void ACDXConstantBindingImp::print(std::ostream& os) const
{
    printTargetEnum(os, _target);
    os << "(" << _constantIndex << ") ";

    std::vector<ACDX_STORED_FP_ITEM_ID>::const_iterator iter = _vStateIds.begin();

    os << "StoredFPIds = ";

    while ( iter != _vStateIds.end() )
    {
        ACDXFixedPipelineStateImp::printStateEnum(os, (*iter));

        iter++;

        if ( iter != _vStateIds.end() ) os << ", ";
    }

    os << "Direct src: {" << _directSource[0] << "," << _directSource[1] << "," << _directSource[2] << "," << _directSource[3] << "} ";

    os << endl;
}

void ACDXConstantBindingImp::printTargetEnum(std::ostream& os, ACDX_BINDING_TARGET target)
{
    switch(target)
    {
        case ACDX_BINDING_TARGET_ENVIRONMENT: os << "ENVIRONMENT"; break;
        case ACDX_BINDING_TARGET_LOCAL: os << "LOCAL"; break;
        case ACDX_BINDING_TARGET_FINAL: os << "FINAL"; break;
    }
}
