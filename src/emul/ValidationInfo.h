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
 * Validation Info definition file.
 *
 */

#ifndef _VALIDATION_INFO_

#define _VALIDATION_INFO_

#include "GPUTypes.h"
#include "GPU.h"
#include <map>

namespace gpu3d
{

/**
 *
 *  Defines the identifier for a vertex.
 *
 */

struct VertexID
{
    u32bit instance;
    u32bit index;
    
    VertexID() : instance(0), index(0) {}
    VertexID(u32bit instance, u32bit index) : instance(instance), index(index) {}
    VertexID(u32bit &index) : instance(0), index(index) {}
    
    bool operator==(const VertexID &in) const
    {
        return ((instance == in.instance) && (index == in.index));
    }
    
    bool operator<(const VertexID &in) const
    {
        return ((instance < in.instance) || ((instance == in.instance) && (index < in.index)));
    }
};

/**
 *
 *  Defines information associated with a vertex input.
 *
 */

struct VertexInputInfo
{
    VertexID vertexID;
    u32bit timesRead;
    bool differencesBetweenReads;
    QuadFloat attributes[MAX_VERTEX_ATTRIBUTES];
};

typedef std::map<VertexID, VertexInputInfo> VertexInputMap;

/**
 *
 *  Defines information associated with shaded vertices.
 *
 */

struct ShadedVertexInfo
{
    VertexID vertexID;
    u32bit timesShaded;
    bool differencesBetweenShading;
    QuadFloat attributes[MAX_VERTEX_ATTRIBUTES];
};

typedef std::map<VertexID, ShadedVertexInfo> ShadedVertexMap;

/**
 *
 *  Defines a fragment identifier and the operations required to
 *  use the class as a map key.
 *
 */
 
struct FragmentID
{
    u32bit x;
    u32bit y;
    u32bit sample;
    u32bit triangleID;
    
    FragmentID(u32bit triangleID, u32bit x, u32bit y, u32bit sample) : triangleID(triangleID), x(x), y(y), sample(sample) {}
    FragmentID() : triangleID(0), x(0), y(0), sample(0) {}
    
    bool operator==(const FragmentID &in) const
    {
        return ((triangleID == in.triangleID) && (x == in.x) && (y == in.y) && (sample == in.sample));
    };
    
    bool operator<(const FragmentID &in) const
    {
        return ((triangleID < in.triangleID) ||
                ((triangleID == in.triangleID) && ((x < in.x) ||
                                                   ((x == in.x) && ((y < in.y) || ((y == in.y) && (sample < in.sample)))))));
    }   
};
 

/**
 *
 *  Defines information associated with a fragment quad (2x2 fragments) that is updating memory.
 *
 */

struct FragmentQuadMemoryUpdateInfo
{
    static const u32bit MAX_BYTES_PER_PIXEL = 16;
    
    FragmentID fragID;
    u8bit inData[4 * MAX_BYTES_PER_PIXEL];
    u8bit readData[4 * MAX_BYTES_PER_PIXEL];
    u8bit writeData[4 * MAX_BYTES_PER_PIXEL];
    bool writeMask[4 * MAX_BYTES_PER_PIXEL];
    bool cullMask[4];
};

typedef std::map<FragmentID, FragmentQuadMemoryUpdateInfo> FragmentQuadMemoryUpdateMap;

};  // namespace gpu3d

#endif