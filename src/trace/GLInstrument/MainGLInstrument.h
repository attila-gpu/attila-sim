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

#ifndef MAINGLINSTRUMENT_H
    #define MAINGLINSTRUMENT_H

#ifdef _MSC_VER
#pragma warning (disable:4201)
#pragma warning (disable:4214)
#pragma warning (disable:4115)
#pragma warning (disable:4514)
#pragma warning (disable:4127)
#pragma warning (disable:4273)   /* No complaints about DLL linkage... */
#endif /* _MSC_VER */

#include "GLInstrumentImp.h"

#define PREV_USER_CALL(callName, funcCall)\
    if ( prevUserCall().callName != 0 ) prevUserCall().funcCall;

#define POST_USER_CALL(callName, funcCall)\
    if ( postUserCall().callName != 0 ) postUserCall().funcCall;


GLInstrumentImp& gli();
GLJumpTable& glCalls();
GLJumpTable& prevUserCall();
GLJumpTable& postUserCall();


#endif // MAINGLINSTRUMENT_H

