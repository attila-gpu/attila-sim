#include "Stats.h"
#include "GLResolver.h"
#include <fstream>
#include "support.h"
#include "gl.h"

using namespace std;

AdvancedStats::AdvancedStats()
: 
    /* reset counters */
    vertexPerBatch(0),
    vertexPerFrame(0),
    vertexCount(0),
    trianglesPerBatch(0),
    trianglesPerFrame(0),
    trianglesCount(0),
    //currentFrame(1),
    currentPrimitive(666) /* undefined */
    //out(0) /* not initialized */
{
}

int AdvancedStats::computeTriangles( GLenum mode, int nVertices ) const
{
    int t = 0;
    switch ( mode )
    {
        case GL_POINTS:
        case GL_LINES:
        case GL_LINE_STRIP:
        case GL_LINE_LOOP:
            break;
        case GL_TRIANGLES:
            t = nVertices / 3;
            break;
        case GL_TRIANGLE_STRIP:
        case GL_TRIANGLE_FAN:
        case GL_POLYGON:
            t = ( nVertices >= 3 ? (1 + nVertices - 3) : 0);
            break;
        case GL_QUADS:
            t = (nVertices / 4) * 2;
            break;
        case GL_QUAD_STRIP:
            t = ( nVertices >= 4 ? (2 + ((nVertices-4)/2)*2) : 0);
            break;
        default:
            panic("AdvancedStats","computeTriangles()", "Unknown primitive");
    }
    return t;
}


int AdvancedStats::getVertexPerBatch() const
{
    return vertexPerBatch;
}

int AdvancedStats::getVertexPerFrame() const
{
    return vertexPerFrame;
}

int AdvancedStats::getVertex() const
{
    return vertexCount;
}

void AdvancedStats::resetBatchStats()
{
    vertexPerFrame += vertexPerBatch;
    trianglesPerFrame += trianglesPerBatch;
    vertexPerBatch = 0;
	/*
	char myBuf[256];
	sprintf(myBuf, "vertexPerFrame = %d\ntrianglesPerFrame =%d\nvertexPerBatch = %d", 
		vertexPerFrame, trianglesPerFrame, vertexPerBatch);
	popup("resetBatchStats()",myBuf);
	*/
}

void AdvancedStats::resetFrameStats()
{
    vertexCount += vertexPerFrame;
    trianglesCount += trianglesPerFrame;

    vertexPerBatch = 0;
    vertexPerFrame = 0;
    trianglesPerFrame = 0;
}

void AdvancedStats::addVertexCount( int vertexCount )
{
   vertexPerBatch += vertexCount;
}


int AdvancedStats::getTrianglesPerBatch() const
{
    /* compute triangles in this batch */
    return (trianglesPerBatch = computeTriangles(currentPrimitive, vertexPerBatch));
}

int AdvancedStats::getTrianglesPerFrame() const
{
    /* compute triangles in this frame */
    return trianglesPerFrame;
}


int AdvancedStats::getTriangles() const
{
    /* compute all triangles in this execution */
    return trianglesCount;
}


void AdvancedStats::setPrimitive( GLenum mode )
{
    currentPrimitive = mode;
}

GLenum AdvancedStats::getPrimitive() const
{
    return currentPrimitive;
}

