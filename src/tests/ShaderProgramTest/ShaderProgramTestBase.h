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

#ifndef SHADERPROGRAMTESTBASE_H
    #define SHADERPROGRAMTESTBASE_H

#include <vector>
#include <string>
#include "QuadFloat.h"

namespace acdlib 
{
    //  class prototype (ACDXProgramExecutionEnvironment.h included in implementation file)
    class ACDXVP1ExecEnvironment;
    class ACDXFP1ExecEnvironment;

    namespace acdlib_opt
    {
        //  class prototype (ShaderOptimizer.h included in implementation file)
        class ShaderOptimizer;
    }
}

namespace gpu3d {

//  class prototype (ShaderInstruction.h included in implementation file)
class ShaderInstruction;

// From ConfigLoader.h (included in implementation .cpp file)
struct SimParameters;

//  class prototype (ShaderEmulator.h included in implementation file)
class ShaderEmulator;

//  class prototype (TextureEmulator.h included in implementation file)
class TextureEmulator;

enum compileModel
{   
    OGL_ARB_1_0,
    D3D_SHADER_MODEL_2_0,
    D3D_SHADER_MODEL_3_0,
};

enum compileTarget
{
    SPT_VERTEX_TARGET,
    SPT_FRAGMENT_TARGET
};

struct ProgramInput
{
    float components[4];

    ProgramInput()
    {
        components[0] = components[1] = components[2] = components[3] = 0.0f;
    };
};

class ShaderProgramTestBase
{
private:
    
    acdlib::ACDXVP1ExecEnvironment* arbvp10comp;
    acdlib::ACDXFP1ExecEnvironment* arbfp10comp;

    acdlib::acdlib_opt::ShaderOptimizer* shOptimizer;

    // Current shader program state.
    bool loadedProgram;
    compileTarget target;
    compileModel shaderModel;
    QuadFloat* programInputs;
    QuadFloat* programConstants;

    // Program binary code storage.
    unsigned char* compiledCode;
    unsigned int codeSize;

    ShaderEmulator* shEmu;
    TextureEmulator* texEmu;

    //  Auxiliary functions.
    void printCompiledProgram(std::vector<ShaderInstruction*> program, bool showBinary);

public:

    virtual std::string parseParams(const std::vector<std::string>& params)
    {
        return std::string(); // OK
    }

    ShaderProgramTestBase( const SimParameters& simP );

    virtual bool finished() = 0;

    //  Console commands implementation

    void compileProgram(const std::string& code, bool optimize, bool transform, compileModel compModel);
    void defineProgramInput(unsigned int index, float* value);
    void defineProgramConstant(unsigned int index, float* value);
    void executeProgram(int stopPC);
    void dumpProgram(std::ostream& out);
};

} // namespace gpu3d

#endif // SHADERPROGRAMTESTBASE_H
