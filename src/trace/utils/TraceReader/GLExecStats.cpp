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

#include "GLExecStats.h"

GLExecStats GLExecStats::stats;

void GLAPIENTRY end_USER()
{
    GLExecStats::instance().incBatchCount(1);
}

void GLAPIENTRY drawArrays_USER( GLenum mode, GLint first, GLsizei count )
{
    GLExecStats::instance().incBatchCount(1);
    GLExecStats::instance().incVertexCount(count);
}

void GLAPIENTRY drawElements_USER( GLenum, GLsizei count, GLenum , const GLvoid *)
{
    GLExecStats::instance().incBatchCount(1);
    GLExecStats::instance().incVertexCount(count);
}

void GLAPIENTRY drawRangeElements_USER( GLenum, GLuint, GLuint, GLsizei count, GLenum , const GLvoid *)
{
    GLExecStats::instance().incBatchCount(1);
    GLExecStats::instance().incVertexCount(count);
}
