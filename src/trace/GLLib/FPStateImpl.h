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
 
#ifndef FPSTATEIMPL_H
    #define FPSTATEIMPL_H

#include "FPState.h"

namespace libgl
{

class FPStateImpl : public FPState
{

public:
    FPStateImpl();
    virtual ~FPStateImpl();

    bool fogEnabled();
    bool separateSpecular();
    bool anyTextureUnitEnabled();
    bool isTextureUnitEnabled(GLuint unit);
    TextureUnit getTextureUnit(GLuint unit);
    int maxTextureUnits();

};

} // namespace libgl

#endif // FPSTATEIMPL_H

