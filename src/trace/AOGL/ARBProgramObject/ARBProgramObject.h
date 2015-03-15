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

#ifndef ARB_PROGRAMOBJECT
    #define ARB_PROGRAMOBJECT


#include "gl.h"

#include "AGLBaseObject.h"
#include "ARBRegisterBank.h"
#include "ACDShaderProgram.h"
#include "ARBImplementationLimits.h"

#include "ACDDevice.h"
#include "ACDX.h"

#include <string>
#include <set>

namespace agl
{

class ARBProgramTarget;

class ARBProgramObject : public BaseObject
{

public:

    static const GLuint MaxLocalRegisters = 23;//MAX_PROGRAM_LOCAL_PARAMETERS_ARB;

private:

    // Program Object info
    std::string _source; ///< The program string defined through the glProgramString() call.
    GLenum _format; ///< The program string format. GL_PROGRAM_FORMAT_ASCII_ARB only supported.
    
    // The binary attila program
    acdlib::ACDShaderProgram* _shader;

    // The compiled arb program exposed by the ACDX
    acdlib::ACDXCompiledProgram* _arbCompiledProgram;

    acdlib::ACDDevice* _acddev;

    bool _sourceCompiled;

    ARBRegisterBank _locals;

    friend class ARBProgramTarget;

    ARBProgramObject(GLenum name, GLenum targetName);

public:

    void attachDevice(acdlib::ACDDevice* device);

    void restartLocalTracking();
    //const std::vector<GLuint>& getLocalsChanged() const; 
                
    //void setSource(const GLvoid* arbProgram, GLuint arbProgramSize, GLenum format);
    //bool getSource(GLubyte* arbProgram, GLuint& arbProgramSize) const;
    void setSource(const std::string& arbCode, GLenum format);
    const std::string& getSource() const;

    // Access to local params register bank
    ARBRegisterBank& getLocals();
    const ARBRegisterBank& getLocals() const;

    // Force program compilation if the program is not compiled
    acdlib::ACDShaderProgram* compileAndResolve(acdlib::ACDXFixedPipelineState* fpState);

    // Check if real compilation is required
    bool isCompiled();

    const char* getStringID() const;
        
    ~ARBProgramObject();
};

} // namespace libgl


#endif // ARB_PROGRAMOBJECT
