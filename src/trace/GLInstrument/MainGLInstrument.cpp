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

#include "MainGLInstrument.h"

GLInstrumentImp& gli()
{
    static GLInstrumentImp glInstrumentImp;
    return glInstrumentImp;
}

GLJumpTable& glCalls()
{
    static GLJumpTable& glJT = gli().glCall();
    return glJT;
}

GLJumpTable& prevUserCall()
{
    static GLJumpTable& prevUserJT = gli().registerBeforeFunc();
    return prevUserJT;
}

GLJumpTable& postUserCall()
{
    static GLJumpTable& postUserJT = gli().registerAfterFunc();
    return postUserJT;
}


