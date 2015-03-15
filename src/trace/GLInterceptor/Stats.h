#ifndef STATS_H
    #define STATS_H

#include <iostream>
#include "gl.h"

class AdvancedStats
{
private:

    //int currentFrame;
    
    GLenum currentPrimitive;

	int vshInstrPerBatch; ///< Number of vsh instructions (per vertex) per batch
	int fshInstrPerBatch; ///< Number of fsh instructions (per fragment) per batch
	int fshTexInstrPerBatch; ///< Number of texture instructions (per fragment) per batch
    int vertexPerBatch; ///< Number of vertexes per batch
     
	int avgVshInstrPerFrame; ///< Average number of vsh instructions (per vertex) per frame
	int avgFshInstrPerFrame; ///< Average number of fsh instructions (per fragment) per frame
	int avgFshTexInstrPerFrame; ///< Average number of texture instructions (per fragment) per frame
    int vertexPerFrame; ///< Number of vertexes per frame

    int vertexCount; ///< Total vertex count

    mutable int trianglesPerBatch;

    mutable int trianglesPerFrame;

    mutable int trianglesCount;

    //static AdvancedStats *as;

    AdvancedStats( const AdvancedStats& );

    /* stream for outputing stats */
    //std::ostream* out;

    int computeTriangles( GLenum mode, int nVertices ) const;

public:

	AdvancedStats(); /* create an AdvancedStats object */

    int getVertexPerBatch() const;

    int getVertexPerFrame() const;

    int getVertex() const;

    void resetBatchStats();

    void resetFrameStats();

    void addVertexCount( int vertexCount );

    int getTrianglesPerBatch() const;

    int getTrianglesPerFrame() const;

    int getTriangles() const;

    GLenum getPrimitive() const;

    void setPrimitive( GLenum mode );

    void setFrame( int frame );

    int getFrame() const;

	/*
    void dumpBatchStats( const char* info = 0 );

    void dumpFrameStats( const char* info = 0 );

    void dumpTotalStats( const char* info = 0 );
	*/
};


#endif // STATS_H