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

#ifndef PROGRAMMANAGER_H
    #define PROGRAMMANAGER_H

#include "gl.h"
#include "BaseManager.h"
#include "ProgramTarget.h"

namespace libgl
{

class ProgramManager : public BaseManager
{

private:

    ProgramManager();
    ProgramManager(const ProgramManager&);

    static ProgramManager* pm;

public:

    static ProgramManager& instance();

    bool programString(GLenum target, GLenum format, GLsizei len, const void* program);

    // Wraps cast
    ProgramTarget& getTarget(GLenum target) const
    {
        return static_cast<ProgramTarget&>(BaseManager::getTarget(target));
    }
};

} // namespace libgl

#endif // PROGRAMMANAGER_H

