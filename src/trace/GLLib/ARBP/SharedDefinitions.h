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

#ifndef SHARED_DEFINITIONS_H
    #define SHARED_DEFINITIONS_H    
/**
 * This file contains the shared definitions and structs used by semantic parser 
 * and generation code phase.
 */

#include "ImplementationLimits.h"
#include <string>

namespace libgl
{

namespace GenerationCode
{

enum IdentType { VERTEX_ATTRIB_REG, 
                 FRAGMENT_ATTRIB_REG, 
                 VERTEX_RESULT_REG, 
                 FRAGMENT_RESULT_REG, 
                 TEMP_REGISTER, 
                 PROGRAM_PARAMETER, 
                 PROGRAM_ENV_PARAMETER, 
                 PROGRAM_LOCAL_PARAMETER, 
                 PROGRAM_STATE_PARAMETER, 
                 PROGRAM_CONST_PARAMETER, 
                 ADDRESS_REG, 
                 ALIAS_REF };

typedef struct {
    enum IdentType type;             ///< The type of the identifier.If type 
                                     ///< is PROGRAM_PARAMETER then the 
                                     ///< specific type is in 
                                     ///< paramType["position"].
    bool implicit_binding;  
    bool parameter_array;            ///< Is a parameter array variable.
    unsigned int array_size;         ///< Sense only if parameter array.
    bool relative_access_in_program; ///< Sense only if parameter array.
    std::string alias_reference;     ///< Pointer to the referenced name.
                                     ///< Sense only if alias reference
    
    unsigned int regId;              ///< The number in the related register
                                     ///< bank when is not a program parameter 
                                     ///< variable
    
    /**
     * Each position of array is the number (order) of the param position in the param 
     * register bank. For vector params (size = 1) only paramRegIds[0] has useful Id
     */
     
    unsigned int paramRegId[MAX_PROGRAM_PARAMETERS_ARB];
    
    /**
     * Type of each vector in the param array. For vector params (size = 1) only 
     * paramType[0] has the useful type
     */
     
    enum IdentType paramType[MAX_PROGRAM_PARAMETERS_ARB];
    
} IdentInfo;



} // namespace CodeGeneration

} // namespace libgl

#endif // SHARED_DEFINITIONS_H
