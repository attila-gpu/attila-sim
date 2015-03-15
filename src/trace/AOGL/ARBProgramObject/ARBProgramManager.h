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

#ifndef ARB_PROGRAMMANAGER
    #define ARB_PROGRAMMANAGER

#include "gl.h"
#include "AGLBaseManager.h"
#include "ARBProgramTarget.h"
#include "ARBProgramObject.h"

namespace agl
{

class ARBProgramManager : public BaseManager
{
private:

    ARBProgramManager(const ARBProgramManager&);
    ARBProgramManager& operator=(const ARBProgramManager&);

public:

    ARBProgramManager();

    // Helper function to set the source code of the current program of the specified target
    void programString(GLenum target, GLenum format, GLsizei len, const void* program);

    ARBProgramTarget& target(GLenum target);

};

} // namespace agl

#endif // ARB_PROGRAMMANAGER
