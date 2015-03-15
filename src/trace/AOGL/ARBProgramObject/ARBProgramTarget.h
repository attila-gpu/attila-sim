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

#ifndef ARB_PROGRAMTARGET
    #define ARB_PROGRAMTARGET

#include "AGLBaseTarget.h"
#include "ARBProgramObject.h"

#include <map>
#include <string>

namespace agl
{

class ARBProgramTarget : public BaseTarget
{
public:

    static const GLuint MaxEnvRegisters = MAX_PROGRAM_ENV_PARAMETERS_ARB;

private:
        
    // provides a default (0)
    ARBProgramObject* _def;

    ARBRegisterBank _envs;

    friend class ARBProgramManager; // Only program managers can create program targets
    ARBProgramTarget(GLenum target);

protected:

    ARBProgramObject* createObject(GLenum name);

public:



    ARBRegisterBank& getEnv();
    const ARBRegisterBank& getEnv() const;

    ARBProgramObject& getCurrent() const { return static_cast<ARBProgramObject&>(BaseTarget::getCurrent()); }
        
};


} // namespace libgl

#endif // ARB_PROGRAMTARGET
