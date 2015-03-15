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

#include "ARBProgramTarget.h"
#include "support.h"
#include "glext.h"

using namespace std;
using namespace agl;

static const GLubyte vprogram[] =
    "!!ARBvp1.0\
    #\n\
    # This is the Default vertex program\n\
    #\n\
    ATTRIB iPos        = vertex.position;\
    ATTRIB iColor    = vertex.color;\
    ATTRIB iTex0    = vertex.texcoord[0];\
    PARAM mvp[4]    = { state.matrix.mvp };\
    OUTPUT oPos        = result.position;\
    OUTPUT oColor    = result.color;\
    OUTPUT oTex0    = result.texcoord[0];\
    # This just does out vertex transform by the modelview projection matrix\n\
    DP4 oPos.x, mvp[0], iPos;\
    DP4 oPos.y, mvp[1], iPos;\
    DP4 oPos.z, mvp[2], iPos;\
    DP4 oPos.w, mvp[3], iPos;\
    # Write our color out directly\n\
    MOV oColor, iColor;\
    END";

static const GLubyte fprogram[] =
    "!!ARBfp1.0\n\
    # This is the Default fragment program\n\
    MOV result.color, fragment.color;\n\
    END";


ARBProgramObject* ARBProgramTarget::createObject(GLenum name)
{
    ARBProgramObject* po = new ARBProgramObject(name, getName());
    po->setTarget(*this);
    return po;
}

ARBProgramTarget::ARBProgramTarget(GLenum target) : BaseTarget(target), _envs(MaxEnvRegisters)
{   
    ARBProgramObject* df = new ARBProgramObject(0, target);
    if ( target == GL_VERTEX_PROGRAM_ARB )
        df->setSource(string((const char*)vprogram, sizeof(vprogram)), GL_PROGRAM_FORMAT_ASCII_ARB);
    else if ( target == GL_FRAGMENT_PROGRAM_ARB )
        df->setSource(string((const char*)fprogram, sizeof(fprogram)), GL_PROGRAM_FORMAT_ASCII_ARB); 
    else
        panic("ARBProgramTarget", "ARBProgramTarget", "Unsupported target");
    
    df->setTarget(*this);
    setCurrent(*df);
    setDefault(df);
}

ARBRegisterBank& ARBProgramTarget::getEnv()
{
    return _envs;
}

const ARBRegisterBank& ARBProgramTarget::getEnv() const
{
    return _envs;
}
