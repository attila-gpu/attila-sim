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

#ifndef PROGRAMEXECUTIONENVIRONMENT_H
    #define PROGRAMEXECUTIONENVIRONMENT_H

#include "RBank.h"
#include "ProgramObject.h"
#include "ImplementationLimits.h"
#include "GPUTypes.h"
#include <list>
#include <bitset>
#include "SemanticTraverser.h"

namespace libgl
{
    

// interface for semantic checkers
/*
class ShaderSemanticCheck
{
public:

    typedef std::bitset<MAX_PROGRAM_ATTRIBS_ARB> AttribMask; 
    
    // Methods for retrieving results
    virtual u32bit getNumberOfInstructions() const = 0;
    virtual u32bit getNumberOfParamRegs() const = 0;
    virtual u32bit getNumberOfTempRegs() const = 0;
    virtual u32bit getNumberOfAddrRegs() const = 0;
    virtual u32bit getNumberOfAttribs() const = 0;
    
    virtual AttribMask getAttribsWritten() const = 0; // returns a mask
    virtual AttribMask getAttribsRead() const = 0; // returns a mask

    virtual std::string getErrorString() const = 0;
    virtual bool foundErrors() const = 0;
};
*/

// interface for code generators
class ShaderCodeGeneration
{
public:
    
    /**
     * Methods for retrieving results
     */
    virtual RBank<f32bit> getParametersBank() = 0;
    virtual void returnGPUCode(u8bit* bin, u32bit& size) = 0;
    virtual u32bit getMaxAliveTemps() = 0;
    
    virtual ~ShaderCodeGeneration() {}
};

class ProgramExecutionEnvironment
{
protected:
        
    /**
     * @param po Input Program Object
     * @retval ssc Semantic Check object returned after perform this method
     * @retval scg Code Generator object returned after perform this method
     */     
    virtual void dependantCompilation(const u8bit* code, u32bit codeSize, GenerationCode::SemanticTraverser*& ssc, ShaderCodeGeneration*& scg)=0;
    
public:

    /* this method implements independent type program compilation */
    // Fetch rate indicates how many instructions can fetch the underlying architecture
    void compile(ProgramObject& po);

};

class VP1ExecEnvironment : public ProgramExecutionEnvironment
{
    void dependantCompilation(const u8bit* code, u32bit codeSize, GenerationCode::SemanticTraverser*& ssc, ShaderCodeGeneration*& scg);
};

class FP1ExecEnvironment : public ProgramExecutionEnvironment
{
    void dependantCompilation(const u8bit* code, u32bit codeSize, GenerationCode::SemanticTraverser*& ssc, ShaderCodeGeneration*& scg);
};

} // namespace libgl

#endif // PROGRAMEXECUTIONENVIRONMENT_H
