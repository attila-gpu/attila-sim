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

#ifndef INTERNAL_CONSTANT_BINDING_H
    #define INTERNAL_CONSTANT_BINDING_H

#include "ACDXConstantBinding.h"

namespace acdlib
{

class InternalConstantBinding: public ACDXConstantBinding
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

    acd_bool _readGLState;                  ///< Tells if ACDXGLState read is required (thus _glStateId is valid).
    acd_uint _glStateId;                  ///< The ACDXGLState state Id
    
    acd_uint _constantIndex;              ///< The final position in the constant bank.
    ACDXFloatVector4 _directSource;          ///< The direct input source for the binding function
    const ACDXBindingFunction* _function; ///< The binding function

public:

    InternalConstantBinding(acd_uint glStateId, acd_bool readGLState,
                            acd_uint constantIndex, 
                            const ACDXBindingFunction* function,
                            ACDXFloatVector4 directSource = ACDXFloatVector4(acd_float(0)));    

    
    ~InternalConstantBinding();
    
};

} // namespace acdlib

#endif // INTERNAL_CONSTANT_BINDING_H
