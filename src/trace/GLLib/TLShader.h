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

#ifndef TLSHADER_H
    #define TLSHADER_H

#include <string>
#include <vector>
#include "gl.h"
#include "glext.h"
#include "ProgramObject.h"
#include "GLContext.h"
#include <iostream>

namespace libgl
{

class InitLocal
{
    /* Only TLFactory & FPFactory can create this kind of objects */
    friend class TLFactory;
    friend class FPFactory;

protected:
    InitLocal() {}

private:

    /* Copy not allowed */
    InitLocal(const InitLocal& );
    InitLocal& operator=(const InitLocal& );

public:

    virtual void init(ProgramObject& po, const GLContext& ctx)=0;

    virtual void dump() const {}
};


/**
 * Interface for initilizing a ProgramObject
 *
 * The implementor of this class is assumed to provided 
 */
class TLShader
{
    std::string code;
    GLenum format;
    std::vector<InitLocal *> inits;

    friend class TLFactory; // can call private methods
    friend class FPFactory;

    void setCode(const std::string& code_, GLenum format_ = GL_PROGRAM_FORMAT_ASCII_ARB)
    {
        code = code_;
        format = format_;
    }

    void addLocal(InitLocal* il) { inits.push_back(il); }

public:

    std::string getCode() { return code; }

    void setup(ProgramObject& po, const GLContext& ctx, bool cached)
    {
        // Victor:  Trying to Fix fixed function shader caches!!! /// po.setSource(code.c_str(), code.length());
        if (!cached)
            po.setSource(code.c_str(), code.length());
        po.setFormat(format);
        std::vector<InitLocal*>::iterator it = inits.begin();
        for ( ; it != inits.end(); it++ )
            (*it)->init(po, ctx);
    }

    void dump() const
    {
        std::cout << code << std::endl;
        std::cout << "Locals:\n";
        std::vector<InitLocal*>::const_iterator it = inits.begin();
        for ( ; it != inits.end(); it++ )
            (*it)->dump();
    }

    
    /**
     * TLShader owns its InitLocal objects
     * so TLShader deletes them
     */
    ~TLShader() 
    {
        std::vector<InitLocal*>::iterator it = inits.begin();
        for ( ; it != inits.end(); it++ )
            delete (*it);
    }

};

} // namespace libgl

#endif
