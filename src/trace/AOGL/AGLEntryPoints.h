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

#ifndef AGLENTRYPOINTS
    #define AGLENTRYPOINTS

#include "gl.h"
#include "glext.h"
#include "AGLContext.h"

/*************************************
 * OpenGL on AGL supported functions *
 *************************************/

GLAPI void GLAPIENTRY AGL_glBegin( GLenum );
GLAPI void GLAPIENTRY AGL_glBindBufferARB(GLenum target, GLuint buffer);
GLAPI void GLAPIENTRY AGL_glDeleteBuffersARB(GLsizei n, const GLuint *buffers);
GLAPI void GLAPIENTRY AGL_glBufferDataARB(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);

GLAPI void GLAPIENTRY AGL_glBufferSubDataARB (GLenum target, GLintptrARB offset,
                                          GLsizeiptrARB size, const GLvoid *data);

GLAPI void GLAPIENTRY AGL_glClear( GLbitfield mask );

GLAPI void GLAPIENTRY AGL_glClearColor( GLclampf red, GLclampf green,
                                   GLclampf blue, GLclampf alpha );

GLAPI void GLAPIENTRY AGL_glColor3f( GLfloat red, GLfloat green, GLfloat blue );


GLAPI void GLAPIENTRY AGL_glLoadIdentity( void );

GLAPI void GLAPIENTRY AGL_glMatrixMode( GLenum mode );

GLAPI void GLAPIENTRY AGL_glOrtho( GLdouble left, GLdouble right,
                                 GLdouble bottom, GLdouble top,
                                 GLdouble near_, GLdouble far_ );

GLAPI void GLAPIENTRY AGL_glFrustum( GLdouble left, GLdouble right,
                                   GLdouble bottom, GLdouble top,
                                   GLdouble near_, GLdouble far_ );

GLAPI void GLAPIENTRY AGL_glShadeModel( GLenum mode );

GLAPI void GLAPIENTRY AGL_glVertex2i( GLint x, GLint y );

GLAPI void GLAPIENTRY AGL_glVertex2f( GLfloat x, GLfloat y );

GLAPI void GLAPIENTRY AGL_glVertex3f( GLfloat x, GLfloat y, GLfloat z );

GLAPI void GLAPIENTRY AGL_glViewport( GLint x, GLint y, GLsizei width, GLsizei height );

GLAPI void GLAPIENTRY AGL_glNormal3f( GLfloat nx, GLfloat ny, GLfloat nz );

GLAPI void GLAPIENTRY AGL_glEnd( void );

GLAPI void GLAPIENTRY AGL_glFlush( void );

GLAPI void GLAPIENTRY AGL_glEnableClientState( GLenum cap );

GLAPI void GLAPIENTRY AGL_glDrawArrays( GLenum mode, GLint first, GLsizei count );

GLAPI void GLAPIENTRY AGL_glDisableClientState( GLenum cap );

GLAPI void GLAPIENTRY AGL_glEnableVertexAttribArrayARB (GLuint index);

GLAPI void GLAPIENTRY AGL_glDisableVertexAttribArrayARB (GLuint index);

GLAPI void GLAPIENTRY AGL_glVertexAttribPointerARB(GLuint index, GLint size, GLenum type,
                                                GLboolean normalized, GLsizei stride, const GLvoid *pointer);

GLAPI void GLAPIENTRY AGL_glVertexPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr );

GLAPI void GLAPIENTRY AGL_glColorPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr );

GLAPI void GLAPIENTRY AGL_glNormalPointer( GLenum type, GLsizei stride, const GLvoid *ptr );

GLAPI void GLAPIENTRY AGL_glIndexPointer( GLenum type, GLsizei stride, const GLvoid *ptr );

GLAPI void GLAPIENTRY AGL_glTexCoordPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr );

GLAPI void GLAPIENTRY AGL_glEdgeFlagPointer( GLsizei stride, const GLvoid *ptr );

GLAPI void GLAPIENTRY AGL_glArrayElement( GLint i );

GLAPI void GLAPIENTRY AGL_glInterleavedArrays( GLenum format, GLsizei stride, const GLvoid *pointer );

GLAPI void GLAPIENTRY AGL_glDrawRangeElements( GLenum mode, GLuint start, GLuint end, GLsizei count,
                                           GLenum type, const GLvoid *indices );

GLAPI void GLAPIENTRY AGL_glDrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices );

GLAPI void GLAPIENTRY AGL_glLoadMatrixf( const GLfloat *m );

GLAPI void GLAPIENTRY AGL_glMultMatrixd( const GLdouble *m );

GLAPI void GLAPIENTRY AGL_glMultMatrixf( const GLfloat *m );

GLAPI void GLAPIENTRY AGL_glTranslated( GLdouble x, GLdouble y, GLdouble z );

GLAPI void GLAPIENTRY AGL_glTranslatef( GLfloat x, GLfloat y, GLfloat z );

GLAPI void GLAPIENTRY AGL_glRotated( GLdouble angle, GLdouble x, GLdouble y, GLdouble z );

GLAPI void GLAPIENTRY AGL_glRotatef( GLfloat angle, GLfloat x, GLfloat y, GLfloat z );

GLAPI void GLAPIENTRY AGL_glScalef( GLfloat x, GLfloat y, GLfloat z );

GLAPI void GLAPIENTRY AGL_glEnable( GLenum cap );

GLAPI void GLAPIENTRY AGL_glDisable( GLenum cap );

GLAPI void GLAPIENTRY AGL_glBindProgramARB (GLenum target, GLuint pid);

GLAPI const GLubyte* GLAPIENTRY AGL_glGetString( GLenum name );

GLAPI void GLAPIENTRY AGL_glProgramEnvParameter4fvARB (GLenum target, GLuint index, const GLfloat *params);

GLAPI void GLAPIENTRY AGL_glProgramEnvParameters4fvEXT (GLenum a,GLuint b,GLsizei c,const GLfloat* d);

GLAPI void GLAPIENTRY AGL_glProgramLocalParameter4fvARB (GLenum target, GLuint index, const GLfloat *v);

GLAPI void GLAPIENTRY AGL_glProgramLocalParameter4fARB (GLenum target, GLuint index,
                                                  GLfloat x, GLfloat y, GLfloat z, GLfloat w);

GLAPI void GLAPIENTRY AGL_glProgramLocalParameters4fvEXT (GLenum a,GLuint b,GLsizei c,const GLfloat* d);

GLAPI void GLAPIENTRY AGL_glProgramStringARB (GLenum target, GLenum format,
                                        GLsizei len, const GLvoid * str);

GLAPI void GLAPIENTRY AGL_glLightfv( GLenum light, GLenum pname, const GLfloat *params );

GLAPI void GLAPIENTRY AGL_glMaterialfv( GLenum face, GLenum pname, const GLfloat *params );

GLAPI void GLAPIENTRY AGL_glLightModelfv( GLenum pname, const GLfloat *params );

GLAPI void GLAPIENTRY AGL_glColor4ub( GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha );

GLAPI void GLAPIENTRY AGL_glColor4f( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha );

GLAPI void GLAPIENTRY AGL_glColor4fv( const GLfloat *v );

GLAPI void GLAPIENTRY AGL_glMaterialf( GLenum face, GLenum pname, GLfloat param );

GLAPI void GLAPIENTRY AGL_glLightModelf( GLenum pname, GLfloat param );

GLAPI void GLAPIENTRY AGL_glLightModeli( GLenum pname, GLint param );

GLAPI void GLAPIENTRY AGL_glLightf( GLenum light, GLenum pname, GLfloat param );

GLAPI void GLAPIENTRY AGL_glNormal3fv( const GLfloat *v );

GLAPI void GLAPIENTRY AGL_glVertex3fv( const GLfloat *v );

GLAPI void GLAPIENTRY AGL_glDepthFunc( GLenum func );

GLAPI void GLAPIENTRY AGL_glDepthMask( GLboolean flag );

GLAPI void GLAPIENTRY AGL_glClearDepth( GLclampd depth );

GLAPI void GLAPIENTRY AGL_glCullFace( GLenum mode );

GLAPI void GLAPIENTRY AGL_glFrontFace( GLenum mode );

GLAPI void GLAPIENTRY AGL_glPushMatrix( void );

GLAPI void GLAPIENTRY AGL_glPopMatrix( void );

GLAPI void GLAPIENTRY AGL_glActiveTextureARB(GLenum texture);

GLAPI void GLAPIENTRY AGL_glBindTexture( GLenum target, GLuint texture );

GLAPI void GLAPIENTRY AGL_glDeleteTextures(GLsizei n, const GLuint *textures);

GLAPI void GLAPIENTRY AGL_glTexParameteri( GLenum target, GLenum pname, GLint param );

GLAPI void GLAPIENTRY AGL_glTexParameterf( GLenum target, GLenum pname, GLfloat param );

GLAPI void GLAPIENTRY AGL_glPixelStorei( GLenum pname, GLint param );

GLAPI void GLAPIENTRY AGL_glCompressedTexImage2DARB( GLenum target, GLint level,
                                                 GLenum internalFormat,
                                                 GLsizei width, GLsizei height,
                                                 GLint border, GLsizei imageSize,
                                                 const GLvoid *data );

GLAPI void GLAPIENTRY AGL_glCompressedTexImage2D( GLenum target, GLint level,
                                              GLenum internalFormat,
                                              GLsizei width, GLsizei height,
                                              GLint border, GLsizei imageSize,
                                              const GLvoid *data );

GLAPI void GLAPIENTRY AGL_glCompressedTexSubImage2DARB (    GLenum target,
                                                            GLint level,
                                                            GLint xoffset,
                                                            GLint yoffset,
                                                            GLsizei width,
                                                            GLsizei height,
                                                            GLenum format,
                                                            GLsizei imageSize,
                                                            const GLvoid *data);

GLAPI void GLAPIENTRY AGL_glTexImage2D( GLenum target, GLint level,
                                    GLint internalFormat,
                                    GLsizei width, GLsizei height,
                                    GLint border, GLenum format, GLenum type,
                                    const GLvoid *pixels );

GLAPI void GLAPIENTRY AGL_glTexImage3D( GLenum target, GLint level,
                                        GLint internalFormat,
                                        GLsizei width, GLsizei height,
                                        GLsizei depth, GLint border,
                                        GLenum format, GLenum type,
                                        const GLvoid *pixels );


GLAPI void GLAPIENTRY AGL_glTexSubImage2D( GLenum target, GLint level,
                                       GLint xoffset, GLint yoffset,
                                       GLsizei width, GLsizei height,
                                       GLenum format, GLenum type,
                                       const GLvoid *pixels );

GLAPI void GLAPIENTRY AGL_glCopyTexImage2D( GLenum target, GLint level,
                                        GLenum internalformat,
                                        GLint x, GLint y,
                                        GLsizei width, GLsizei height,
                                        GLint border );


GLAPI void GLAPIENTRY AGL_glCopyTexSubImage2D( GLenum target, GLint level,
                                           GLint xoffset, GLint yoffset,
                                           GLint x, GLint y,
                                           GLsizei width, GLsizei height );

GLAPI void GLAPIENTRY AGL_glTexCoord2f( GLfloat s, GLfloat t );

GLAPI void GLAPIENTRY AGL_glMultiTexCoord2f( GLenum texture, GLfloat s, GLfloat t );

GLAPI void GLAPIENTRY AGL_glMultiTexCoord2fv( GLenum texture, const GLfloat *v );

GLAPI void GLAPIENTRY AGL_glTexGeni(GLenum coord, GLenum pname, GLint param);

GLAPI void GLAPIENTRY AGL_glTexGenfv(GLenum coord, GLenum pname, const GLfloat* param);

GLAPI void GLAPIENTRY AGL_glTexGenf( GLenum coord, GLenum pname, GLfloat param );

GLAPI void GLAPIENTRY AGL_glClientActiveTextureARB(GLenum texture);

GLAPI void GLAPIENTRY AGL_glTexEnvf( GLenum target, GLenum pname, GLfloat param );

GLAPI void GLAPIENTRY AGL_glTexEnvi( GLenum target, GLenum pname, GLint param );

GLAPI void GLAPIENTRY AGL_glTexEnvfv( GLenum target, GLenum pname, const GLfloat *params );

GLAPI void GLAPIENTRY AGL_glFogf( GLenum pname, GLfloat param );

GLAPI void GLAPIENTRY AGL_glFogi( GLenum pname, GLint param );

GLAPI void GLAPIENTRY AGL_glFogfv( GLenum pname, const GLfloat *params );

GLAPI void GLAPIENTRY AGL_glFogiv( GLenum pname, const GLint *params );

GLAPI void GLAPIENTRY AGL_glStencilFunc( GLenum func, GLint ref, GLuint mask );

GLAPI void GLAPIENTRY AGL_glStencilOp( GLenum fail, GLenum zfail, GLenum zpass );

GLAPI void GLAPIENTRY AGL_glStencilOpSeparateATI(GLenum face, GLenum fail, GLenum zfail, GLenum zpass );

GLAPI void GLAPIENTRY AGL_glClearStencil( GLint s );

GLAPI void GLAPIENTRY AGL_glStencilMask( GLuint mask );

GLAPI void GLAPIENTRY AGL_glColorMask( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha );

GLAPI void GLAPIENTRY AGL_glAlphaFunc( GLenum func, GLclampf ref );

GLAPI void GLAPIENTRY AGL_glBlendFunc( GLenum sfactor, GLenum dfactor );

GLAPI void GLAPIENTRY AGL_glBlendEquation( GLenum mode );

GLAPI void GLAPIENTRY AGL_glBlendColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha );

GLAPI void GLAPIENTRY AGL_glColorMaterial( GLenum face, GLenum mode );

GLAPI void GLAPIENTRY AGL_glScissor( GLint x, GLint y, GLsizei width, GLsizei height);

GLAPI void GLAPIENTRY AGL_glDepthRange( GLclampd near_val, GLclampd far_val );

GLAPI void GLAPIENTRY AGL_glPolygonOffset( GLfloat factor, GLfloat units );

GLAPI void GLAPIENTRY AGL_glPushAttrib( GLbitfield mask );

GLAPI void GLAPIENTRY AGL_glPopAttrib( void );

GLAPI void GLAPIENTRY AGL_glPolygonMode (GLenum face, GLenum mode);

GLAPI void GLAPIENTRY AGL_glBlendEquationEXT (GLenum mode);

#endif // AGLENTRYPOINTS
