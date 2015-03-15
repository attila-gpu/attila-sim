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

#ifndef SPECIFIC_H
    #define SPECIFIC_H

#include "glAll.h"

/************************
 * SPECIFIC DEFINITIONS *
 ************************/
// Put specific definitions here ( header functions with postfix _SPECIFIC )
// This file will be used by Code Generator

/********************************
 * ARB Vertex Program extension *
 ********************************/
//<A=3>
//<NL=4>
void GLAPIENTRY glProgramStringARB_SPECIFIC(GLenum target, GLenum format, GLsizei len, const GLvoid *str);

//<A=1>
//<NL=2>
void GLAPIENTRY glGenProgramsARB_SPECIFIC(GLsizei, GLuint *);

//<A=2>
//<NL=3>
void GLAPIENTRY glGetProgramivARB_SPECIFIC(GLenum, GLenum, GLint *);

//<A=3>
//<NL=4>
void GLAPIENTRY glProgramEnvParameters4fvEXT_SPECIFIC(GLenum target, GLuint index, GLsizei count, const GLfloat *params); 

//<A=3>
//<NL=4>
void GLAPIENTRY glProgramLocalParameters4fvEXT_SPECIFIC(GLenum target, GLuint index, GLsizei count, const GLfloat *params); 

//<A=1>
//<NL=2>
void GLAPIENTRY glGetIntegerv_SPECIFIC( GLenum pname, GLint *params );

//<A=1>
//<NL=2>
void GLAPIENTRY glGetFloatv_SPECIFIC( GLenum pname, GLfloat *params );

//<A=6>
//<NL=7>
void GLAPIENTRY glReadPixels_SPECIFIC( GLint x, GLint y, GLsizei width, GLsizei height,
                                    GLenum format, GLenum type, GLvoid *pixels );

/*********************
 * GLSLANG functions *
 *********************/
//<A=2>
//<NL=3,4>
void GLAPIENTRY glShaderSourceARB_SPECIFIC(GLhandleARB shaderObj, GLsizei count, const GLcharARB **str, const GLint *length);

/****************************************
 * VERTEX BUFFER OBJECT EXTENSION CALLS *
 ****************************************/
// To support vertex buffer object extension in vertex arrays
//<A=0>
void GLAPIENTRY glBindBufferARB_SPECIFIC(GLenum _p0, GLuint _p1);

//<A=1>
//<NL=2>
void GLAPIENTRY glGenBuffersARB_SPECIFIC(GLsizei _p0, GLuint *_p1);

//<A=1>
//<NL=2>
void GLAPIENTRY glDeleteBuffersARB_SPECIFIC( GLsizei n, const GLuint *buffers);

//<A=0,1>
//<NL=2,3>
void GLAPIENTRY glBufferDataARB_SPECIFIC(GLenum _p0, GLsizeiptrARB _p1, const GLvoid *_p2, GLenum _p3);

//<A=3>
//<NL=4>
void GLAPIENTRY glBufferSubDataARB_SPECIFIC(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid * data);


//<R> call this function instead of call original function ( i will call original version )
GLvoid * GLAPIENTRY glMapBufferARB_SPECIFIC(GLenum target, GLenum access);

//<A=0>
GLboolean GLAPIENTRY glUnmapBufferARB_SPECIFIC(GLenum _p0);

//<A=7>
//<NL=8>
void GLAPIENTRY glCompressedTexImage2DARB_SPECIFIC(GLenum target, GLint level, GLenum iFmt, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid * data);

//<A=7>
//<NL=8>
void GLAPIENTRY glCompressedTexImage2D_SPECIFIC(GLenum target, GLint level, GLenum iFmt, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid * data);

//<A=8>
//<NL=9>
void GLAPIENTRY glCompressedTexSubImage2DARB_SPECIFIC(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);

//<A=3>
//<NL=4>
void GLAPIENTRY glGetTexLevelParameteriv_SPECIFIC( GLenum target, GLint level, GLenum pname, GLint *params );

//<A=6>
//<NL=7>
void GLAPIENTRY glReadPixels_SPECIFIC( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels );


//<A=2>
//<NL=3,4>
void GLAPIENTRY glGetInfoLogARB_SPECIFIC(GLhandleARB shader, GLsizei bufSize, GLsizei *length, GLcharARB *infoLog);

//<A=2>
//<NL=3>
void GLAPIENTRY glGetObjectParameterivARB_SPECIFIC(GLhandleARB object, GLenum pname, GLint *params);

//<A=1>
//<NL=2>
GLint APIENTRY glGetUniformLocationARB_SPECIFIC(GLhandleARB program, const GLcharARB *name);

//<A=1>
//<NL=2>
void GLAPIENTRY glSetFragmentShaderConstantATI_SPECIFIC(GLuint dst, const GLfloat * value);

//<A=0>
void GLAPIENTRY glEnableVertexAttribArrayARB_SPECIFIC(GLuint index);

//<A=0>
void GLAPIENTRY glDisableVertexAttribArrayARB_SPECIFIC(GLuint index);

//<A=0>
void GLAPIENTRY glEnable_SPECIFIC(GLenum cap);

//<A=0>
void GLAPIENTRY glDisable_SPECIFIC(GLenum cap);

//<A=0>
void GLAPIENTRY glEnableClientState_SPECIFIC(GLenum cap);

//<A=0>
void GLAPIENTRY glDisableClientState_SPECIFIC(GLenum cap);

//<NL=4> do not log param 4 ( we are using and identifier of buffer )
//<A=3> call this function once, after log parameter 3
void GLAPIENTRY glVertexPointer_SPECIFIC( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr );

//<NL=4>
//<A=3>
void GLAPIENTRY glColorPointer_SPECIFIC( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr );

//<NL=3>
//<A=2>
void GLAPIENTRY glNormalPointer_SPECIFIC(GLenum type, GLsizei stride, const GLvoid *ptr);

//<NL=4>
//<A=3>
void GLAPIENTRY glTexCoordPointer_SPECIFIC(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);

//<A=0>
void GLAPIENTRY glClientActiveTextureARB_SPECIFIC( GLenum texture );

//<NL=3>
//<A=2>
void GLAPIENTRY glInterleavedArrays_SPECIFIC( GLenum format, GLsizei stride,const GLvoid *pointer );

//<NL=4>
//<A=0,3>
void GLAPIENTRY glDrawElements_SPECIFIC( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices );

//<NL=6>
//<A=0,5>
void GLAPIENTRY glDrawRangeElements_SPECIFIC(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);

// Specific must be called to resolve buffers...
//<A=0>
GLAPI void GLAPIENTRY glDrawArrays_SPECIFIC( GLenum mode, GLint first, GLsizei count );

//<NL=1>
//<A=1>
void GLAPIENTRY glMultMatrixd_SPECIFIC( const GLdouble *m );

//<NL=1>
//<A=1>
void GLAPIENTRY glMultMatrixf_SPECIFIC( const GLfloat *m );

//<NL=1>
//<A=1>
void GLAPIENTRY glLoadMatrixf_SPECIFIC(const GLfloat *m);

//<NL=1>
//<A=1>
void GLAPIENTRY glLoadMatrixd_SPECIFIC(const GLdouble *m);


//<A=8> call after log parameter 8
//<NL=9> i will store buffer manually
void GLAPIENTRY glTexImage2D_SPECIFIC( GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels );

//<A=9> call after log parameter 8
//<NL=10> i will store buffer manually
void GLAPIENTRY glTexImage3D_SPECIFIC( GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels );

//<A=1>
//<NL=2>
void GLAPIENTRY glFogfv_SPECIFIC(GLenum pname, const GLfloat *params);

//<A=1>
//<NL=2>
GLAPI void GLAPIENTRY glFogiv_SPECIFIC( GLenum pname, const GLint *params );


//<A=2>
//<NL=3>
void GLAPIENTRY glLightfv_SPECIFIC(GLenum light, GLenum pname, const GLfloat *params);

//<A=2>
//<NL=3>
GLAPI void GLAPIENTRY glMaterialfv_SPECIFIC( GLenum face, GLenum pname, const GLfloat *params );

//<A=1>
//<NL=2>
GLAPI void GLAPIENTRY glLightModelfv_SPECIFIC( GLenum pname, const GLfloat *params );

//<A=1> call specific after log parameter 1
//<NL=2> i will log parameter 2 ( remeber that is an output parameter)
void GLAPIENTRY glGenTextures_SPECIFIC(GLsizei n, GLuint *textures);

//<A=1>
//<NL=2>
GLAPI void GLAPIENTRY glDeleteTextures_SPECIFIC( GLsizei n, const GLuint *textures);

//<OCS> i will write all code for logging and execute this function
const GLubyte * GLAPIENTRY glGetString_SPECIFIC( GLenum name );


//<A=0,r> call specific after return ( after count this call )
int GLAPIENTRY wglSwapBuffers_SPECIFIC(HDC hdc);

//<A=2>
//<NL=3>
void GLAPIENTRY glTexEnvfv_SPECIFIC( GLenum target, GLenum pname, const GLfloat *params );

//<A=2>
//<NL=3>
void GLAPIENTRY glTexGenfv_SPECIFIC( GLenum coord, GLenum pname, const GLfloat *params );

//<A=2>
//<NL=3>
void GLAPIENTRY glTexParameterfv_SPECIFIC( GLenum target, GLenum pname, const GLfloat *params );

//<A=8>
//<NL=9>
void GLAPIENTRY glTexSubImage2D_SPECIFIC( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels );

//<A=5>
//<NL=6>
void GLAPIENTRY glVertexAttribPointerARB_SPECIFIC(GLuint index, GLint size, GLenum type,  GLboolean normalized, GLsizei stride,  const GLvoid *pointer);

/********************
 * NVDIA EXTENSIONS *
 ********************/
//<NL=4>
//<A=3>
void GLAPIENTRY glLoadProgramNV_SPECIFIC(GLenum _p0, GLuint _p1, GLsizei _p2, const GLubyte *_p3);

/********************
 * OTHER EXTENSIONS *
 ********************/

//<A=5>
//<NL=6>
void GLAPIENTRY glColorTableEXT_SPECIFIC(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *data);

/*****************
 * WGL FUNCTIONS *
 *****************/

//<R> call this function instead of call original function ( i will call original version )
//<A=1,r> call after parameter 1 and return
//<NL=1,r> i will log the parameter ( like a string )
PROC GLAPIENTRY wglGetProcAddress_SPECIFIC(const char* funcName);


//<OCS> do not generate any code for logging function name or parameters ( i will do all the work )
const char * GLAPIENTRY wglGetExtensionsStringARB_SPECIFIC(HDC _p0);

//<A=1>
//<NL=2>
int GLAPIENTRY wglChoosePixelFormat_SPECIFIC(HDC hdc, const PIXELFORMATDESCRIPTOR *ppfd);

//<A=2>
//<NL=3>
int GLAPIENTRY wglSetPixelFormat_SPECIFIC(HDC hdc, int iPixelFormat, const PIXELFORMATDESCRIPTOR *ppfd);


//<A=1>
//<NL=2>
void GLAPIENTRY glGenQueriesARB_SPECIFIC(GLsizei n,  GLuint * ids);

//<A=2>
//<NL=3>
void GLAPIENTRY glGetQueryObjectivARB_SPECIFIC(GLuint id, GLenum pname, GLint * params);


//////////////////////////////////////////////////////////
// DISABLE TRACING OF SOME FUNCTIONS using OCS TAG hack //
//////////////////////////////////////////////////////////

//<OCS>
HDC GLAPIENTRY wglGetCurrentDC_SPECIFIC();

//<OCS>
HGLRC GLAPIENTRY wglGetCurrentContext_SPECIFIC();

//<R>
int GLAPIENTRY wglMakeCurrent_SPECIFIC(HDC hdc, HGLRC hglrc);

#endif
