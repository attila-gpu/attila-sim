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

#ifndef ACDX_COMPILED_PROGRAM_IMP_H
    #define ACDX_COMPILED_PROGRAM_IMP_H

#include "ACDX.h"
#include "ACDXRBank.h"
#include "ACDXImplementationLimits.h"

namespace acdlib
{

class ACDXCompiledProgramImp: public ACDXCompiledProgram
{
//////////////////////////
//  interface extension //
//////////////////////////
private:
    
    acd_ubyte* _compiledBytecode;
    acd_uint _compiledBytecodeSize;
    ACDXRBank<float> _compiledConstantBank;
    acd_enum _textureUnitUsage[MAX_TEXTURE_UNITS_ARB];
    acd_bool _killInstructions;

public:
    
    ACDXCompiledProgramImp();

    void setCode(const acd_ubyte* compiledBytecode, acd_uint compiledBytecodeSize);

    void setTextureUnitsUsage(acd_uint tu, acd_enum usage);

    void setKillInstructions(acd_bool kill);

    const acd_ubyte* getCode() const;

    acd_uint getCodeSize() const;

    acd_enum getTextureUnitsUsage(acd_uint tu) const;

    acd_bool getKillInstructions() const;

    void setCompiledConstantBank(const ACDXRBank<float>& ccBank);

    const ACDXRBank<float>& getCompiledConstantBank() const;

    ~ACDXCompiledProgramImp();
};

} // namespace acdlib

#endif // ACDX_COMPILED_PROGRAM_IMP_H
