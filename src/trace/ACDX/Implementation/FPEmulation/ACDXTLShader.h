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

#ifndef ACDX_TLSHADER_H
    #define ACDX_TLSHADER_H

#include <string>
#include <list>
#include "gl.h"
#include "glext.h"
#include "ACDX.h"
#include <iostream>

namespace acdlib
{

/**
 * Interface for getting the program generation results.
 *
 * The implementor of this class is assumed to provided 
 */
class ACDXTLShader
{
    std::string code;
    GLenum format;

    std::list<ACDXConstantBinding*> constantBindings;

public:

    void setCode(const std::string& code_, GLenum format_ = GL_PROGRAM_FORMAT_ASCII_ARB)
    {
        code = code_;
        format = format_;
    }

    std::string getCode() { return code; }

    void addConstantBinding(acdlib::ACDXConstantBinding* cb)    { constantBindings.push_back(cb); }

    std::list<ACDXConstantBinding*> getConstantBindingList() {    return constantBindings;    }

    ~ACDXTLShader()
    {
        std::list<ACDXConstantBinding*>::iterator iter = constantBindings.begin();
    
        while ( iter != constantBindings.end() )
        {
            ACDXDestroyConstantBinding(*iter);
            iter++;
        }
    }
};

} // namespace acdlib

#endif // ACDX_TLSHADER_H
