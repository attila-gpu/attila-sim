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

#ifndef ACDX_CONSTANT_BINDING_IMP_H
    #define ACDX_CONSTANT_BINDING_IMP_H

#include "ACDXConstantBinding.h"
#include "ACDXFixedPipelineStateImp.h"

namespace acdlib
{

class ACDXConstantBindingImp: public ACDXConstantBinding
{
public:

    /**
     * The resolveConstant() method executes the binding function and resolves
     * the constant value using the current FP state.
     *
     * @param    fpStateImp The ACDXFixedPipelineImp object.
     * @retval    constant   The resolved constant value.        
     */
    virtual void resolveConstant(const ACDXFixedPipelineState* fpState, ACDXFloatVector4& constant) const;

    /**
     * Returns the constant binding index
     *
     * @returns The constant binding index
     */
    virtual acd_uint getConstantIndex() const;

    /**
     * Prints the ACDXConstantBinding object internal data state.
     *
     * @retval    os    The output stream to print to.
     */
    virtual void print(std::ostream& os) const;

//////////////////////////
//  interface extension //
//////////////////////////
private:
    
    ACDX_BINDING_TARGET _target;    ///< Binding target used by the program to reference the target (environment or local parameter)
    acd_uint _constantIndex;        ///< Binding index used by the program
    std::vector<ACDX_STORED_FP_ITEM_ID> _vStateIds;    ///< Id of the requested binding state
    ACDXFloatVector4 _directSource;        ///< The direct input source for the binding function
    const ACDXBindingFunction* _function; ///< The binding function

    void resolveGLSVectorState(const ACDXFixedPipelineStateImp* fpStateImp, VectorId vId, acd_uint componentMask, ACDXFloatVector4& vState) const;

    void resolveGLSMatrixState(const ACDXFixedPipelineStateImp* fpStateImp, MatrixId mId, MatrixType type, acd_uint unit, std::vector<ACDXFloatVector4>& mState) const;

public:
    
    ACDXConstantBindingImp(ACDX_BINDING_TARGET target, 
                           acd_uint constantIndex, 
                           std::vector<ACDX_STORED_FP_ITEM_ID> vStateIds, 
                           const ACDXBindingFunction* function,
                           ACDXFloatVector4 directSource = ACDXFloatVector4(acd_float(0)));    
                           
    ~ACDXConstantBindingImp();

    /**
     * Returns the constant binding target
     *
     * @returns The constant target
     */
    virtual ACDX_BINDING_TARGET getTarget() const;

    /**
     * Returns the state id
     *
     * @returns The state id
     */
    virtual std::vector<ACDX_STORED_FP_ITEM_ID> getStateIds() const;

    /**
     * Returns a pointer to the binding function.
     *
     * @returns The pointer to the binding function.
     */
    virtual const ACDXBindingFunction* getBindingFunction() const;

    /**
     * Returns a copy of the direct source value
     *
     * @returns The copy of the direct source value
     */
    virtual ACDXFloatVector4 getDirectSource() const;


    static void printTargetEnum(std::ostream& os, ACDX_BINDING_TARGET target);

    friend std::ostream& operator<<(std::ostream& os, const ACDXConstantBindingImp& cbi)
    {
        cbi.print(os);
        return os;
    }
};

} // namespace acdlib

#endif // ACDX_CONSTANT_BINDING_IMP_H
