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

#ifndef ARB_REGISTERBANK
    #define ARB_REGISTERBANK

#include "gl.h"

#include <vector>
#include <set>

namespace agl
{

class ARBRegisterBank
{
public:

    ARBRegisterBank(GLuint registerCount);

    void restartTracking();

    std::vector<GLuint> getModified() const;

    void set(GLuint reg, GLfloat coord1, GLfloat coord2, GLfloat coord3, GLfloat coord4, GLubyte mask = 0xF);
    void set(GLuint reg, const GLfloat* coords, GLubyte mask = 0xF);

    void get(GLuint reg, GLfloat& coord1, GLfloat& coord2, GLfloat& coord3, GLfloat& coord4) const;
    void get(GLuint reg, GLfloat* coords) const;
    const GLfloat* get(GLuint reg) const;

private:

    const GLuint _RegisterCount;

    typedef GLfloat _Register[4];

    _Register* _registers; // register bank

    std::set<GLuint> _modified; // register tracking
};

} // namespace agl

#endif // ARB_REGISTERBANK
