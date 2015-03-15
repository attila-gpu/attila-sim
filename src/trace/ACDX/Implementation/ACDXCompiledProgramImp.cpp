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

#include "ACDXCompiledProgramImp.h"

using namespace acdlib;

ACDXCompiledProgramImp::ACDXCompiledProgramImp()
: _compiledBytecode(0), _compiledBytecodeSize(0), _compiledConstantBank(250,"clusterBank"), _killInstructions(false)
{
    for (int tu = 0; tu < MAX_TEXTURE_UNITS_ARB; tu++)
        _textureUnitUsage[tu] = 0;
}

ACDXCompiledProgramImp::~ACDXCompiledProgramImp()
{
    if (_compiledBytecode)
        delete[] _compiledBytecode;
}

void ACDXCompiledProgramImp::setCode(const acd_ubyte* compiledBytecode, acd_uint compiledBytecodeSize)
{
    if (_compiledBytecode)
        delete[] _compiledBytecode;

    _compiledBytecode = new acd_ubyte[compiledBytecodeSize];

    memcpy(_compiledBytecode, compiledBytecode, compiledBytecodeSize);

    _compiledBytecodeSize = compiledBytecodeSize;
}

void ACDXCompiledProgramImp::setTextureUnitsUsage(acd_uint tu, acd_enum usage)
{
    _textureUnitUsage[tu] = usage;
}

void ACDXCompiledProgramImp::setKillInstructions(acd_bool kill)
{
    _killInstructions = kill;
}

acd_enum ACDXCompiledProgramImp::getTextureUnitsUsage(acd_uint tu) const
{
    return _textureUnitUsage[tu];
}

acd_bool ACDXCompiledProgramImp::getKillInstructions() const
{
    return _killInstructions;
}

const acd_ubyte* ACDXCompiledProgramImp::getCode() const
{
    return _compiledBytecode;
}

acd_uint ACDXCompiledProgramImp::getCodeSize() const
{
    return _compiledBytecodeSize;
}

void ACDXCompiledProgramImp::setCompiledConstantBank(const ACDXRBank<float>& ccBank)
{
    _compiledConstantBank = ccBank;
}

const ACDXRBank<float>& ACDXCompiledProgramImp::getCompiledConstantBank() const
{
    return _compiledConstantBank;
}
