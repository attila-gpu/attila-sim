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

#ifndef USERCALLTABLE_H
    #define USERCALLTABLE_H

#include "glAll.h"

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
typedef GLsizei *GLsizeiptr;
typedef GLsizei *GLsizeiptrARB;
typedef GLint *GLintptr;
typedef GLint *GLintptrARB;

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

/**
 * Macro used to add callbacks
 */
#define UCT_ADD_CALLBACK(uct,call,callback) uct.call = callback;

struct UserCallTable
{
    #include "GLJumpTableFields.gen"
};

/**
 * Must be called before starting to use a UserCallTable or any UCT_ADD_CALLBACK macro use.
 */
void initUserCallTable(UserCallTable& uct);


#endif
