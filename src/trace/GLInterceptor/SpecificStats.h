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

#ifndef SPECIFICSTATS_H
    #define SPECIFICSTATS_H

#include "glAll.h"


/***********************************
 * Functions requiring "stat code" *
 ***********************************/

void GLAPIENTRY glEnable_STAT( GLenum cap );

void GLAPIENTRY glDisable_STAT( GLenum cap );

void GLAPIENTRY glBindProgramARB_STAT(GLenum target, GLuint name);

void GLAPIENTRY glProgramStringARB_STAT(GLenum target, GLenum format, GLsizei len, const GLvoid *str);

/*
 * how to  use this file:
 *
 * use glXXX_STAT() for adding stat code to glXXX() wrapper call
 */

void GLAPIENTRY glBegin_STAT( GLenum mode );

void GLAPIENTRY glVertex2d_STAT(GLdouble x, GLdouble y);

void GLAPIENTRY glVertex2dv_STAT(const GLdouble *v);

void GLAPIENTRY glVertex2f_STAT(GLfloat x, GLfloat y);

void GLAPIENTRY glVertex2fv_STAT(const GLfloat *v);

void GLAPIENTRY glVertex2i_STAT(GLint x, GLint y);

void GLAPIENTRY glVertex2iv_STAT(const GLint *v);

void GLAPIENTRY glVertex2s_STAT(GLshort x, GLshort y);

void GLAPIENTRY glVertex2sv_STAT(const GLshort *v);

void GLAPIENTRY glVertex3d_STAT(GLdouble x, GLdouble y, GLdouble z);

void GLAPIENTRY glVertex3dv_STAT(const GLdouble *v);

void GLAPIENTRY glVertex3f_STAT(GLfloat c, GLfloat y, GLfloat z);

void GLAPIENTRY glVertex3fv_STAT(const GLfloat *v);

void GLAPIENTRY glVertex3i_STAT(GLint x, GLint y, GLint z);

void GLAPIENTRY glVertex3iv_STAT(const GLint *v);

void GLAPIENTRY glVertex3s_STAT(GLshort x, GLshort y, GLshort z);

void GLAPIENTRY glVertex3sv_STAT(const GLshort *v);

void GLAPIENTRY glVertex4d_STAT( GLdouble x, GLdouble y, GLdouble z, GLdouble w );

void GLAPIENTRY glVertex4dv_STAT( const GLdouble *v );

void GLAPIENTRY glVertex4f_STAT( GLfloat x, GLfloat y, GLfloat z, GLfloat w );

void GLAPIENTRY glVertex4fv_STAT( const GLfloat *v );

void GLAPIENTRY glVertex4i_STAT( GLint x, GLint y, GLint z, GLint w );

void GLAPIENTRY glVertex4iv_STAT( const GLint *v );

void GLAPIENTRY glVertex4s_STAT( GLshort x, GLshort y, GLshort z, GLshort w );

void GLAPIENTRY glVertex4sv_STAT( const GLshort *v );

void GLAPIENTRY glDrawArrays_STAT(GLenum mode, GLint first, GLsizei count);

void GLAPIENTRY glMultiDrawArrays_STAT(GLenum _p0, GLint *_p1, GLsizei *_p2, GLsizei _p3);

void GLAPIENTRY glDrawElements_STAT(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);

void GLAPIENTRY glDrawRangeElements_STAT(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);

/*
 * ATI EXTENSION
 */
void GLAPIENTRY glDrawRangeElementArrayATI_STAT(GLenum _p0, GLuint _p1, GLuint _p2, GLsizei _p3);

void GLAPIENTRY glEnd_STAT();

int GLAPIENTRY wglSwapBuffers_STAT(HDC hdc);

#endif // SPECIFICSTATS_H