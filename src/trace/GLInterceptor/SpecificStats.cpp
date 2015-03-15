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

#include "SpecificStats.h"
#include "GLInterceptor.h"
#include "GLResolver.h"
#include "Stats.h"
#include "ShProgramInfo.h"
#include "UserStats.h"
#include <sstream>

using namespace std;

#define SM (GLInterceptor::statManager)

/* Avoid update repeatedly using addVertexCount call */ 
/* Accumulates all vertex counting from glVertex__() functions */
static int vc = 0;

/**
 * Maintains the current primitive (must be set in glBegin call)
 */
static unsigned int currentPrimitive = GL_LINES;

/* count vertex alias */
//#define CV(howmany) SM.getAS().addVertexCount(howmany)
#define CV(howmany) vc+=howmany

void GLAPIENTRY glProgramStringARB_STAT(GLenum target, GLenum format, GLsizei len, const GLvoid *str)
{
    ShProgramInfo* shpi = ShProgramManager::instance().getCurrent(target);
    shpi->setSource((const char*)str);
}

void GLAPIENTRY glBindProgramARB_STAT(GLenum target, GLuint name)
{

    ShProgramManager::instance().bindProgram(name, target);
}

void GLAPIENTRY glBegin_STAT( GLenum mode )
{
    currentPrimitive = mode;
    /* call not included in batch counting. first increase call counting, afterwards begin batch  */
    SM.addCallBeginBatch();
}

void GLAPIENTRY glVertex2d_STAT(GLdouble x, GLdouble y)
{ CV(1); } 

void GLAPIENTRY glVertex2dv_STAT(const GLdouble *v)
{ CV(1); }

void GLAPIENTRY glVertex2f_STAT(GLfloat x, GLfloat y) 
{ CV(1); }

void GLAPIENTRY glVertex2fv_STAT(const GLfloat *v)
{ CV(1); }

void GLAPIENTRY glVertex2i_STAT(GLint x, GLint y)
{ CV(1); }

void GLAPIENTRY glVertex2iv_STAT(const GLint *v)
{ CV(1); }

void GLAPIENTRY glVertex2s_STAT(GLshort x, GLshort y)
{ CV(1); }

void GLAPIENTRY glVertex2sv_STAT(const GLshort *v)
{ CV(1); }

void GLAPIENTRY glVertex3d_STAT(GLdouble x, GLdouble y, GLdouble z)
{ CV(1); }

void GLAPIENTRY glVertex3dv_STAT(const GLdouble *v)
{ CV(1); }

void GLAPIENTRY glVertex3f_STAT(GLfloat c, GLfloat y, GLfloat z) 
{ CV(1); }

void GLAPIENTRY glVertex3fv_STAT(const GLfloat *v)
{ CV(1); }

void GLAPIENTRY glVertex3i_STAT(GLint x, GLint y, GLint z)
{ CV(1); }

void GLAPIENTRY glVertex3iv_STAT(const GLint *v)
{ CV(1); }

void GLAPIENTRY glVertex3s_STAT(GLshort x, GLshort y, GLshort z)
{ CV(1); }

void GLAPIENTRY glVertex3sv_STAT(const GLshort *v)
{ CV(1); }

void GLAPIENTRY glVertex4d_STAT(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{ CV(1); }

void GLAPIENTRY glVertex4dv_STAT( const GLdouble *v )
{ CV(1); }

void GLAPIENTRY glVertex4f_STAT( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{ CV(1); }

void GLAPIENTRY glVertex4fv_STAT( const GLfloat *v )
{ CV(1); }

void GLAPIENTRY glVertex4i_STAT( GLint x, GLint y, GLint z, GLint w )
{ CV(1); }

void GLAPIENTRY glVertex4iv_STAT( const GLint *v )
{ CV(1); }

void GLAPIENTRY glVertex4s_STAT( GLshort x, GLshort y, GLshort z, GLshort w )
{ CV(1); }

void GLAPIENTRY glVertex4sv_STAT( const GLshort *v )
{ CV(1); }

void GLAPIENTRY glDrawArrays_STAT(GLenum mode, GLint first, GLsizei count)
{
    SM.beginBatch(); /* This call is included in batch counting */

    vcStat->addVertexes(count);
    tcStat->addTriangles(mode, count);

    SM.addCallEndBatch(); /* single call in this batch */
}

void GLAPIENTRY glMultiDrawArrays_STAT(GLenum mode, GLint *first, GLsizei *count, GLsizei primcount)
{
    SM.beginBatch(); /* This call is included in batch counting */
    
    int totalCount = 0;
    for ( int i = 0; i < primcount; i++ )
        totalCount += count[i];

    vcStat->addVertexes(totalCount);
    tcStat->addTriangles(mode, totalCount);

    SM.addCallEndBatch(); /* Single call in this batch */
}

void GLAPIENTRY glDrawElements_STAT(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    SM.beginBatch(); /* this call is included in batch counting */
    vcStat->addVertexes(count);
    tcStat->addTriangles(mode, count);
    SM.addCallEndBatch(); /* single call in this batch */
}

void GLAPIENTRY glDrawRangeElements_STAT(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)
{
    SM.beginBatch(); /* this call is included in batch counting */
    vcStat->addVertexes(count);
    tcStat->addTriangles(mode, count);
    SM.addCallEndBatch(); /* single call in this batch */
}

void GLAPIENTRY glDrawRangeElementArrayATI_STAT(GLenum mode, GLuint start, GLuint end, GLsizei count)
{
    SM.beginBatch(); /* this call is included in batch counting */
    vcStat->addVertexes(count);
    tcStat->addTriangles(mode, count);
    SM.addCallEndBatch(); /* single call in this batch */
}

void GLAPIENTRY glEnd_STAT()
{
    /* batch finished, this call won't be counted in current batch stats */
    vcStat->addVertexes(vc);
    tcStat->addTriangles(currentPrimitive, vc);
    SM.endBatch();
    vc = 0;
}

int GLAPIENTRY wglSwapBuffers_STAT(HDC hdc)
{
    //SM.getAS().addVertexCount(vc);
    if ( vc != 0 )
        panic("SpecificStats", "wglSwapBuffers_STAT", "Vertex count greater than 0, shouldn't happen ever");
    SM.addCallEndFrame();   
    return 0; /* dummy */
}


void GLAPIENTRY glEnable_STAT( GLenum cap )
{
    if ( cap == GL_VERTEX_PROGRAM_ARB )
        vshInstrCountStat->setEnabled(true);
    else if ( cap == GL_FRAGMENT_PROGRAM_ARB )
    {
        texInstrCountStat->setEnabled(true);
        fshInstrCountStat->setEnabled(true);
        fpAndAlphaStat->setFPFlag(true);
    }
    else if ( cap == GL_ALPHA_TEST )
        fpAndAlphaStat->setAlphaFlag(true);
}

void GLAPIENTRY glDisable_STAT( GLenum cap )
{
    if ( cap == GL_VERTEX_PROGRAM_ARB )
        vshInstrCountStat->setEnabled(false);
    else if ( cap == GL_FRAGMENT_PROGRAM_ARB )
    {
        fpAndAlphaStat->setFPFlag(false);
        texInstrCountStat->setEnabled(false);
        fshInstrCountStat->setEnabled(false);
    }
    else if ( cap == GL_ALPHA_TEST )
        fpAndAlphaStat->setAlphaFlag(false);    
}
