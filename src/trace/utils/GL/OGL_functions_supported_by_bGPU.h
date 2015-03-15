/**
 * OpenGL functions supported by OpenGL bGPU library
 *
 * This file is parsed by CodeGenerator
 *
 * @date 28/02/2005
 */

GLAPI void GLAPIENTRY glActiveTextureARB(GLenum texture);

GLAPI void GLAPIENTRY glAlphaFunc( GLenum func, GLclampf ref );

GLAPI void GLAPIENTRY glArrayElement( GLint i );

GLAPI void GLAPIENTRY glBegin( GLenum mode );

GLAPI void GLAPIENTRY glBindBufferARB(GLenum target, GLuint buffer);

GLAPI void GLAPIENTRY glBindProgramARB (GLenum target, GLuint pid);

GLAPI void GLAPIENTRY glBindTexture( GLenum target, GLuint texture );

GLAPI void GLAPIENTRY glBlendColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha );

GLAPI void GLAPIENTRY glBlendEquation( GLenum mode );

GLAPI void GLAPIENTRY glBlendFunc( GLenum sfactor, GLenum dfactor );

GLAPI void GLAPIENTRY glBufferDataARB(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);

GLAPI void GLAPIENTRY glBufferSubDataARB (GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data);

GLAPI void GLAPIENTRY glClear( GLbitfield mask );

GLAPI void GLAPIENTRY glClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha );

GLAPI void GLAPIENTRY glClearDepth( GLclampd depth );

GLAPI void GLAPIENTRY glClearStencil( GLint s );

GLAPI void GLAPIENTRY glClientActiveTextureARB(GLenum texture);

GLAPI void GLAPIENTRY glColor3f( GLfloat red, GLfloat green, GLfloat blue );

GLAPI void GLAPIENTRY glColor4f( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha );

GLAPI void GLAPIENTRY glColor4fv( const GLfloat *v );

GLAPI void GLAPIENTRY glColor4ub( GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha );

GLAPI void GLAPIENTRY glColorMask( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha );

GLAPI void GLAPIENTRY glColorMaterial( GLenum face, GLenum mode );

GLAPI void GLAPIENTRY glColorPointer( GLint size, GLenum type,
                                      GLsizei stride, const GLvoid *ptr );

GLAPI void GLAPIENTRY glCompressedTexImage2DARB(GLenum target, GLint level,
                                             GLenum internalFormat, GLsizei width, GLsizei height,
                                             GLint border, GLsizei imageSize, const GLvoid* data);

GLAPI void GLAPIENTRY glCompressedTexImage2D(GLenum target, GLint level,
                                             GLenum internalFormat, GLsizei width, GLsizei height,
                                             GLint border, GLsizei imageSize, const GLvoid* data);

GLAPI void GLAPIENTRY glCopyTexImage2D( GLenum target, GLint level, GLenum internalformat, GLint x, GLint y,
                                        GLsizei width, GLsizei height, GLint border );

GLAPI void GLAPIENTRY glCopyTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y,
                                           GLsizei width, GLsizei height );

GLAPI void GLAPIENTRY glCullFace( GLenum mode );

GLAPI void GLAPIENTRY glDeleteBuffersARB(GLsizei n, const GLuint *buffers);

GLAPI void GLAPIENTRY glDeleteTextures(GLsizei n, const GLuint *textures);

GLAPI void GLAPIENTRY glDepthFunc( GLenum func );

GLAPI void GLAPIENTRY glDepthMask( GLboolean flag );

GLAPI void GLAPIENTRY glDepthRange( GLclampd near_val, GLclampd far_val );

GLAPI void GLAPIENTRY glDisable( GLenum cap );

GLAPI void GLAPIENTRY glDisableClientState( GLenum cap );

GLAPI void APIENTRY glDisableVertexAttribArrayARB (GLuint);

GLAPI void GLAPIENTRY glDrawArrays( GLenum mode, GLint first, GLsizei count );

GLAPI void GLAPIENTRY glDrawElements( GLenum mode, GLsizei count,
                                      GLenum type, const GLvoid *indices );

GLAPI void GLAPIENTRY glDrawRangeElements( GLenum mode, GLuint start, GLuint end, GLsizei count,
                                           GLenum type, const GLvoid *indices );

GLAPI void GLAPIENTRY glEdgeFlagPointer( GLsizei stride, const GLvoid *ptr );

GLAPI void GLAPIENTRY glEnable( GLenum cap );

GLAPI void GLAPIENTRY glEnableClientState( GLenum cap );

GLAPI void APIENTRY glEnableVertexAttribArrayARB (GLuint);

GLAPI void GLAPIENTRY glEnd( void );

GLAPI void GLAPIENTRY glFlush( void );

GLAPI void GLAPIENTRY glFogf( GLenum pname, GLfloat param );

GLAPI void GLAPIENTRY glFogfv( GLenum pname, const GLfloat *params );

GLAPI void GLAPIENTRY glFogi( GLenum pname, GLint param );

GLAPI void GLAPIENTRY glFogiv( GLenum pname, const GLint *params );

GLAPI void GLAPIENTRY glFrontFace( GLenum mode );

GLAPI void GLAPIENTRY glFrustum( GLdouble left, GLdouble right,
                                   GLdouble bottom, GLdouble top,
                                   GLdouble near_, GLdouble far_ );

GLAPI const GLubyte* GLAPIENTRY glGetString( GLenum name );

GLAPI void GLAPIENTRY glIndexPointer( GLenum type, GLsizei stride,
                                      const GLvoid *ptr );

GLAPI void GLAPIENTRY glInterleavedArrays( GLenum format, GLsizei stride,
                                           const GLvoid *pointer );

GLAPI void GLAPIENTRY glLightf( GLenum light, GLenum pname, GLfloat param );

GLAPI void GLAPIENTRY glLightfv( GLenum light, GLenum pname, const GLfloat *params );

GLAPI void GLAPIENTRY glLightModelf( GLenum pname, GLfloat param );

GLAPI void GLAPIENTRY glLightModelfv( GLenum pname, const GLfloat *params );

GLAPI void GLAPIENTRY glLightModeli( GLenum pname, GLint param );

GLAPI void GLAPIENTRY glLoadIdentity( void );

GLAPI void GLAPIENTRY glLoadMatrixf( const GLfloat *m );

GLAPI void GLAPIENTRY glMaterialf( GLenum face, GLenum pname, GLfloat param );

GLAPI void GLAPIENTRY glMaterialfv( GLenum face, GLenum pname, const GLfloat *params );

GLAPI void GLAPIENTRY glMatrixMode( GLenum mode );

GLAPI void GLAPIENTRY glMultiTexCoord2f( GLenum texture, GLfloat s, GLfloat t );

GLAPI void GLAPIENTRY glMultiTexCoord2fv( GLenum texture, const GLfloat *v );

GLAPI void GLAPIENTRY glMultMatrixd( const GLdouble *m );

GLAPI void GLAPIENTRY glMultMatrixf( const GLfloat *m );

GLAPI void GLAPIENTRY glNormal3f( GLfloat nx, GLfloat ny, GLfloat nz );

GLAPI void GLAPIENTRY glNormal3fv( const GLfloat *v );

GLAPI void GLAPIENTRY glNormalPointer( GLenum type, GLsizei stride,
                                       const GLvoid *ptr );

GLAPI void GLAPIENTRY glOrtho( GLdouble left, GLdouble right,
                                 GLdouble bottom, GLdouble top,
                                 GLdouble near_val, GLdouble far_val );

GLAPI void GLAPIENTRY glPixelStorei( GLenum pname, GLint param );

GLAPI void GLAPIENTRY glPolygonOffset( GLfloat factor, GLfloat units );

GLAPI void GLAPIENTRY glPopMatrix( void );

GLAPI void GLAPIENTRY glProgramEnvParameter4fvARB (GLenum target, GLuint index, const GLfloat *params);

GLAPI void GLAPIENTRY glProgramLocalParameter4fARB (GLenum target, GLuint index,
                                                  GLfloat x, GLfloat y, GLfloat z, GLfloat w);

GLAPI void GLAPIENTRY glProgramLocalParameter4fvARB (GLenum, GLuint, const GLfloat *v);

GLAPI void GLAPIENTRY glProgramStringARB (GLenum target, GLenum format,
                                        GLsizei len, const GLvoid * str);

GLAPI void GLAPIENTRY glPopAttrib( void );

GLAPI void GLAPIENTRY glPushAttrib( GLbitfield mask );

GLAPI void GLAPIENTRY glPushMatrix( void );

GLAPI void GLAPIENTRY glRotated( GLdouble angle, GLdouble x, GLdouble y, GLdouble z );

GLAPI void GLAPIENTRY glRotatef( GLfloat angle, GLfloat x, GLfloat y, GLfloat z );

GLAPI void GLAPIENTRY glScalef( GLfloat x, GLfloat y, GLfloat z );

GLAPI void GLAPIENTRY glScissor( GLint x, GLint y, GLsizei width, GLsizei height);

GLAPI void GLAPIENTRY glShadeModel( GLenum mode );

GLAPI void GLAPIENTRY glStencilFunc( GLenum func, GLint ref, GLuint mask );

GLAPI void GLAPIENTRY glStencilMask( GLuint mask );

GLAPI void GLAPIENTRY glStencilOp( GLenum fail, GLenum zfail, GLenum zpass );

GLAPI void GLAPIENTRY glTexCoord2f( GLfloat s, GLfloat t );

GLAPI void GLAPIENTRY glTexCoordPointer( GLint size, GLenum type,
                                         GLsizei stride, const GLvoid *ptr );

GLAPI void GLAPIENTRY glTexEnvf( GLenum target, GLenum pname, GLfloat param );

GLAPI void GLAPIENTRY glTexEnvfv( GLenum target, GLenum pname, const GLfloat *params );

GLAPI void GLAPIENTRY glTexEnvi( GLenum target, GLenum pname, GLint param );

GLAPI void GLAPIENTRY glTexGenf( GLenum coord, GLenum pname, GLfloat param );

GLAPI void GLAPIENTRY glTexGenfv(GLenum coord, GLenum pname, const GLfloat* param);

GLAPI void GLAPIENTRY glTexGeni(GLenum coord, GLenum pname, GLint param);

GLAPI void GLAPIENTRY glTexImage2D( GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height,
                                    GLint border, GLenum format, GLenum type, const GLvoid *pixels );

GLAPI void GLAPIENTRY glTexParameterf( GLenum target, GLenum pname, GLfloat param );

GLAPI void GLAPIENTRY glTexParameteri( GLenum target, GLenum pname, GLint param );

GLAPI void GLAPIENTRY glTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
                                       GLsizei height, GLenum format, GLenum type, const GLvoid *pixels );

GLAPI void GLAPIENTRY glTranslated( GLdouble x, GLdouble y, GLdouble z );

GLAPI void GLAPIENTRY glTranslatef( GLfloat x, GLfloat y, GLfloat z );

GLAPI void GLAPIENTRY glVertex2f( GLfloat x, GLfloat y );

GLAPI void GLAPIENTRY glVertex3f( GLfloat x, GLfloat y, GLfloat z );

GLAPI void GLAPIENTRY glVertex3fv( const GLfloat *v );

GLAPI void GLAPIENTRY glVertex2i( GLint x, GLint y );

GLAPI void APIENTRY glVertexAttribPointerARB (GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *);

GLAPI void GLAPIENTRY glVertexPointer( GLint size, GLenum type,
                                       GLsizei stride, const GLvoid *ptr );

GLAPI void GLAPIENTRY glViewport( GLint x, GLint y, GLsizei width, GLsizei height );

