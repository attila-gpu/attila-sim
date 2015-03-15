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

#ifndef ACDX_PROGRAMEXECUTIONENVIRONMENT_H
    #define ACDX_PROGRAMEXECUTIONENVIRONMENT_H

#include "ACDXRBank.h"
#include "ACDXImplementationLimits.h"
#include "GPUTypes.h"
#include <list>
#include <bitset>
#include <string>

namespace acdlib
{

namespace GenerationCode
{
    class ACDXSemanticTraverser ;
}

struct ResourceUsage
{
    u32bit numberOfInstructions;
    u32bit numberOfParamRegs;
    u32bit numberOfTempRegs;
    u32bit numberOfAddrRegs;
    u32bit numberOfAttribs;
    int textureUnitUsage [MAX_TEXTURE_UNITS_ARB];
    bool killInstructions;
};

// interface for code generators
class ACDXShaderCodeGeneration
{
public:
    
    /**
     * Methods for retrieving results
     */
    virtual acdlib::ACDXRBank<f32bit> getParametersBank() = 0;
    virtual void returnGPUCode(u8bit* bin, u32bit& size) = 0;
    
    virtual ~ACDXShaderCodeGeneration() {}
};

class ACDXProgramExecutionEnvironment
{
protected:
        
    /**
     * @param po Input Program Object
     * @retval ssc Semantic Check object returned after perform this method
     * @retval scg Code Generator object returned after perform this method
     */     
    virtual void dependantCompilation(const u8bit* code, u32bit codeSize, GenerationCode::ACDXSemanticTraverser *& ssc, ACDXShaderCodeGeneration*& scg)=0;
    
public:

    /* this method implements independent type program compilation */
    // Fetch rate indicates how many instructions can fetch the underlying architecture
    //void compile(ProgramObject& po);

    u8bit* compile(const std::string& code, u32bit& size_binary, acdlib::ACDXRBank<float>& clusterBank, ResourceUsage& resourceUsage);

};

class ACDXVP1ExecEnvironment : public ACDXProgramExecutionEnvironment
{
    void dependantCompilation(const u8bit* code, u32bit codeSize, GenerationCode::ACDXSemanticTraverser *& ssc, ACDXShaderCodeGeneration*& scg);
};

class ACDXFP1ExecEnvironment : public ACDXProgramExecutionEnvironment
{
    void dependantCompilation(const u8bit* code, u32bit codeSize, GenerationCode::ACDXSemanticTraverser *& ssc, ACDXShaderCodeGeneration*& scg);
};

} // namespace acdlib

#endif // ACDX_PROGRAMEXECUTIONENVIRONMENT_H
