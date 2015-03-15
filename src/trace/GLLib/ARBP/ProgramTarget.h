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

#ifndef PROGRAMTARGET_H
    #define PROGRAMTARGET_H

#include "BaseTarget.h"
#include "ProgramObject.h"
#include <map>
#include <string>

namespace libgl
{

class ProgramTarget : public BaseTarget
{
    
private:
    
    u32bit fetchRate;
    
    // provides a default (0)
    ProgramObject* def;

    /**
     * Environment parameters
     */
    RBank<float> envParams;

    const char* programErrorString;

    GLint programErrorPosition; /* -1 -> no errors */
    
protected:

    ProgramObject* createObject(GLenum name);

public:

    ProgramTarget(GLenum target);

    RBank<float>& getEnvironmentParameters();
    
    ProgramObject& getCurrent() const
    {
        return static_cast<ProgramObject&>(BaseTarget::getCurrent());
    }
        
};


} // namespace libgl

#endif // PROGRAMTARGET_H
