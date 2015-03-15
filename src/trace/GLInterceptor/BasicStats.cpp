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

#include "BasicStats.h"
#include "gl.h"
#include "support.h"

using namespace std;

VertexStat::VertexStat(const string& name) : GLIStat(name), batchCount(0), frameCount(0)
{}

int VertexStat::getBatchCount()
{
    frameCount += batchCount; // add batch count to frame count
    int temp = batchCount; // get the current batch count
    batchCount = 0;    // reset the current batch count
    return temp; // return the previous batch count
}

int VertexStat::getFrameCount()
{
    int temp = frameCount;
    frameCount = 0;
    return temp;
}

void VertexStat::addVertexes(int vertexCount)
{
    batchCount += vertexCount;
}


TriangleStat::TriangleStat(const string& name) : GLIStat(name), batchCount(0), frameCount(0)
{}

int TriangleStat::getBatchCount()
{
    frameCount += batchCount; // add batch count to frame count
    int temp = batchCount; // get the current batch count
    batchCount = 0;    // reset the current batch count
    return temp; // return the previous batch count
}

int TriangleStat::getFrameCount()
{
    int temp = frameCount;
    frameCount = 0;
    return temp;
}

void TriangleStat::addTriangles(unsigned int primitive, int vertexCount)
{
    int t = 0;
    switch ( primitive )
    {
        case GL_POINTS:
        case GL_LINES:
        case GL_LINE_STRIP:
        case GL_LINE_LOOP:
            break;
        case GL_TRIANGLES:
            t = vertexCount / 3;
            break;
        case GL_TRIANGLE_STRIP:
        case GL_TRIANGLE_FAN:
        case GL_POLYGON:
            t = (vertexCount >= 3 ? (1 + vertexCount - 3) : 0);
            break;
        case GL_QUADS:
            t = (vertexCount / 4) * 2;
            break;
        case GL_QUAD_STRIP:
            t = ( vertexCount >= 4 ? (2 + ((vertexCount-4)/2)*2) : 0);
            break;
        default:
            panic("TriangleStat", "addTriangleCount()", "Unknown OpenGL primitive");

    }

    batchCount += t;
}

