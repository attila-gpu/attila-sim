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

#ifndef GLEXECSTATS_H
    #define GLEXECSTATS_H

#include "gl.h"
#include "GLJumpTable.h"

class GLExecStats
{
private:

    int vc; // vertex count
    int bc; // batch count
    
    GLExecStats() : vc(0), bc(0){}
    GLExecStats(const GLExecStats&);
    GLExecStats& operator=(const GLExecStats&);

    static GLExecStats stats;

public:

    static GLExecStats& instance() { return stats; }

    void incVertexCount(int inc) { vc += inc; }
    void resetVertexCount() { vc = 0; }
    int getVertexCount() const { return vc; }

    void incBatchCount(int inc) { bc += inc; }
    void resetBatchCount() { bc = 0; }
    int getBatchCount() const { return bc; }

};

void GLAPIENTRY drawArrays_USER( GLenum mode, GLint first, GLsizei count );
// used to count vertices generated with glDrawElements & drawRangeElements
void GLAPIENTRY drawElements_USER( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices );
void GLAPIENTRY drawRangeElements_USER( GLenum mode, GLuint start, GLuint end, GLsizei count, 
                        GLenum type, const GLvoid *indices );

void GLAPIENTRY end_USER();

#endif // GLEXECSTATS_H
