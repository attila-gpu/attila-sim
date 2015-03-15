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

#ifndef GLJUMPTABLE_H
    #define GLJUMPTABLE_H

/*
 * Include all gl api headers
 */
#include "glAll.h"
//#include "glext.h"

#ifndef WIN32
    /* hack for linux/unix */

typedef unsigned int HPBUFFERARB ;
typedef unsigned int HPBUFFEREXT;
typedef unsigned int HDC;
typedef unsigned int HGLRC;
typedef unsigned short GLhalfNV;
typedef unsigned int GLhandleARB;
typedef void PIXELFORMATDESCRIPTOR;
typedef void *LPPIXELFORMATDESCRIPTOR;
typedef void *LPLAYERPLANEDESCRIPTOR;
typedef void *COLORREF;
typedef void *LPGLYPHMETRICSFLOAT;
typedef char GLcharARB;
/*
typedef GLsizei *GLsizeiptr;
typedef GLsizei *GLsizeiptrARB;
typedef GLint *GLintptr;
typedef GLint *GLintptrARB;
*/

typedef GLuint HANDLE; 
typedef void* LPVOID;
typedef GLuint DWORD;
typedef GLuint UINT;
//typedef bool BOOL;
#define BOOL bool
typedef GLfloat FLOAT;
typedef GLushort USHORT;
typedef int INT32;
typedef int INT64;
typedef void *PROC;


#define WINAPI /* ignore */

#endif

/*
 * Include automatically generated code ( jump table fields )
 * @see file "GLJumpTableFields.gen"
 */
struct GLJumpTable
{
    #include "GLJumpTableFields.gen"
};


bool loadGLJumpTable( GLJumpTable& jt, const char* dllPath );


#endif // GLJUMPTABLE_H
