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

#include "ProgramTarget.h"
#include "support.h"
#include "glext.h"

using namespace std;
using namespace libgl;

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


ProgramObject* ProgramTarget::createObject(GLenum name)
{
    ProgramObject* po = new ProgramObject(name, getName());
    po->setTarget(*this);
    return po;
}

ProgramTarget::ProgramTarget(GLenum target) : BaseTarget(target), envParams(96), fetchRate(1)
{   
    ProgramObject* df = new ProgramObject(0, target);
    if ( target == GL_VERTEX_PROGRAM_ARB )
        df->setSource(vprogram, sizeof(vprogram));
    else if ( target == GL_FRAGMENT_PROGRAM_ARB )
        df->setSource(fprogram, sizeof(fprogram));
    else
        panic("ProgramTarget", "ProgramTarget", "Unsupported target");
    df->setFormat(GL_PROGRAM_FORMAT_ASCII_ARB);
    
    df->setTarget(*this);
    setCurrent(*df);
    setDefault(df);
}

RBank<float>& ProgramTarget::getEnvironmentParameters()
{
    return envParams;
}

