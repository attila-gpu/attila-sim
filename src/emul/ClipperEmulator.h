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
 * $RCSfile: ClipperEmulator.h,v $
 * $Revision: 1.2 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:36 $
 *
 * Clipper Emulator definition file. 
 *
 */
 
/**
 * 
 *  @file ClipperEmulator.h
 *
 *  Defines the Clipper Emulator class.
 *
 *  This class provides triangle clipping functions to the
 *  clipper simulator.
 * 
 *
 */
 
#ifndef _CLIPPEREMULATOR_

#define _CLIPPEREMULATOR_ 

#include "GPUTypes.h"
#include "GPU.h"
 
namespace gpu3d
{

/**
 *
 *  Maximum number of user clip planes supported.
 *  @note (carlos) it was defined previously in GPU.h
 */
//static const u32bit MAX_USER_CLIP_PLANES = 6;

/**
 * 
 *  Maximum number of clip vertices that can be stored in the
 *  clipper emulator.
 *
 */
static const u32bit MAX_CLIP_VERTICES = 12;


/**
 *
 *  Implements triangle clipping functions. 
 *
 *  This class implements clipping functions for the clipper unit
 *  in the GPU simulator.
 *
 */
 
class ClipperEmulator
{

private:

    QuadFloat userClipPlanes[MAX_USER_CLIP_PLANES]; /**<  User clip planes.  */
    bool userPlanes[MAX_USER_CLIP_PLANES];          /**<  Active user clip planes.  */
    QuadFloat *clipVertex[MAX_CLIP_VERTICES];       /**<  Table with the generated clip vertices.  */
    u32bit numClipVertex;                           /**<  Number of clip vertices produced by the last clip operation. */
    
public:

    /**
     *
     *  Clipper emulator constructor.
     *
     *  Creates and initializes a new clipper emulator object.
     *
     *  @return A new initializec clipper emulator object.
     *
     */
     
    ClipperEmulator();

    /**
     *
     *  Tests the triangle three vertices against the frustum clip
     *  volume and performs a trivial reject test
     *
     *  @param v1 The triangle first vertex coordinates in homogeneous space.
     *  @param v2 The triangle second vertex coordinates in homogeneous space.
     *  @param v3 The triangle third vertex coordinates in homogeneous space.
     *  @param d3d9DepthRange A boolean value that defines the range in clip space
     *  for the depth component, if TRUE uses [0, 1] to emulate D3D9, if FALSE
     *  used [-1, 1] to emulate OpenGL.
     *     
     *  @return If the triangle can be trivially rejected (completely
     *  outside of the frustum clip volume).
     *
     */
     
    static bool trivialReject(QuadFloat v1, QuadFloat v2, QuadFloat v3, bool d3d9DepthRange);
    
    /**
     *
     *  Clips a triangle against the frustum clip volume.
     *
     *  @param v1 The triangle first vertex attributes.
     *  @param v2 The triangle second vertex attributes.
     *  @param v3 The triangle third vertex attributes.
     *
     *  @return If the clip operation has produced new vertices.
     *
     */
     
    bool frustumClip(QuadFloat *v1, QuadFloat *v2, QuadFloat *v3);
    
    /**
     *
     *  Clips a triangle against the user defined clip planes.
     *
     *  @param v1 The triangle first vertex attributes.
     *  @param v2 The triangle second vertex attriutes.
     *  @param v3 The triangle third vertex attributes.
     *
     *  @return If the clip operation has produced new vertices.
     *
     */
     
    bool userClip(QuadFloat *v1, QuadFloat *v2, QuadFloat *v3);
    
    /**
     *
     *  Returns the next clipped vertex produced 
     *
     *  @return The next clip vertex and attributes produced by
     *  the previous clip operation.
     *
     */
     
    QuadFloat *getNextClipVertex();

    /**
     *
     *  Defines a new user clip plane.
     *
     *  @param planeID The user clip plane to be defined.
     *  @param plane The user clip plane.
     *
     */
     
    void defineClipPlane(u32bit planeID, QuadFloat plane);
    
    /**
     *
     *  Undefines an user clip plane.
     *
     *  @param planeID The user clip plane to undefine.
     *
     */
    
    void undefineClipPlane(u32bit planeID);
    
        
};

} // namespace gpu3d

#endif
