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

#ifndef BASICSTATS_H
    #define BASICSTATS_H

#include "GLIStat.h"

class VertexStat : public GLIStat
{
private:

    int batchCount;
    int frameCount;

public:

    VertexStat(const std::string& name);

    /**
     * Returns the count in the vertex-per-batch counter, add the vertex-per-batch 
     * counter value to the vertex-per-frame counter and finally resets the 
     * vertex-per-batch counter
     */
    virtual int getBatchCount();

    /**
     * Returns the count in the vertex-per-frame counter and afterwards resets this
     * internal counter
     */
    virtual int getFrameCount();

    /**
     * Increase the current vertex count in the vertex-per-batch counter
     */
    void addVertexes(int vertexCount);

};

class TriangleStat : public GLIStat
{
private:

    int batchCount;
    int frameCount;

public:

    TriangleStat(const std::string& name);

    virtual int getBatchCount();

    virtual int getFrameCount();

    void addTriangles(unsigned int primitive, int vertexCount);

};


#endif