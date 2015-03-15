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
 * $RCSfile: ClipperEmulator.cpp,v $
 * $Revision: 1.8 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:40 $
 *
 * Clipper Emulator implementation file.
 *
 */

/**
 *
 *  @file ClipperEmulator.cpp
 *
 *  Implements the Clipper Emulator class.
 *
 *  This class implements triangle clipping functions for the
 *  clipper simulator.
 *
 *
 */

#include "ClipperEmulator.h"
#include "support.h"
#include <cstdio>

using namespace gpu3d;

/*  Clipper emulator constructor.  */
ClipperEmulator::ClipperEmulator()
{
    u32bit i;

    /*  Initialize number of clip vertices.  */
    numClipVertex = 0;

    /*  Initialize defined user clip planes table.  */
    for(i = 0; i < MAX_USER_CLIP_PLANES; i++)
    {
        userPlanes[i] = FALSE;
    }

    /*  Set to null the generated clip vertex list.  */
    for(i = 0; i < MAX_CLIP_VERTICES; i++)
        clipVertex[i] = NULL;
}

/*  Performs a trivial reject test against the frustum volume.  */
bool ClipperEmulator::trivialReject(QuadFloat v1, QuadFloat v2, QuadFloat v3, bool d3d9DepthRange)
{
    u32bit outcode1, outcode2, outcode3;
    bool reject;

    /*  Create Sutherland and Cohen Outcodes:

            0 inside the positive half-plane defined by the clip plane
            1 outside the positive half-plane defined by the clip plane

            lrbtnf

            In homogenous clip coordinates
            
              OpenGL => (x, y) -> [-1, 1], z -> [-1, 1]
              D3D9   => (x, y) -> [-1, 1], z -> [ 0, 1]

            l -> left clip plane    : w * (w + x) >= 0  (0x20)
            r -> rigth clip plane   : w * (w - x) >= 0  (0x10)
            b -> bottom clip plane  : w * (w + y) >= 0  (0x08)
            t -> top clip plane     : w * (w - y) >= 0  (0x04)

            (OpenGL)
                        
            n -> near clip plane    : w * (w + z) >= 0  (0x02)
            f -> far clip plane     : w * (w - z) >= 0  (0x01)


            n -> near clip plane    : z >= 0  (0x02)
            f -> far clip plane     : w * (w - z) >= 0  (0x01)

    */

//printf("Vertex1: (%f, %f, %f, %f)\n", v1[0], v1[1], v1[2], v1[3]);
//printf("Vertex2: (%f, %f, %f, %f)\n", v2[0], v2[1], v2[2], v2[3]);
//printf("Vertex3: (%f, %f, %f, %f)\n", v3[0], v3[1], v3[2], v3[3]);

    /*  Left plane test.  */
    //outcode1 = (v1[3] * (v1[3] + v1[0]) >= 0) ? 0x00 : 0x01;
    //outcode2 = (v2[3] * (v2[3] + v2[0]) >= 0) ? 0x00 : 0x01;
    //outcode3 = (v3[3] * (v3[3] + v3[0]) >= 0) ? 0x00 : 0x01;
    outcode1 = ((v1[3] + v1[0]) >= 0) ? 0x00 : 0x01;
    outcode2 = ((v2[3] + v2[0]) >= 0) ? 0x00 : 0x01;
    outcode3 = ((v3[3] + v3[0]) >= 0) ? 0x00 : 0x01;

//printf("Outcode1 %02x Outcode2 %02x Outcode3 %02x\n",
//    outcode1, outcode2, outcode3);

    /*  Right plane test.  */
    //outcode1 |= (v1[3] * (v1[3] - v1[0]) >= 0) ? 0x00 : 0x02;
    //outcode2 |= (v2[3] * (v2[3] - v2[0]) >= 0) ? 0x00 : 0x02;
    //outcode3 |= (v3[3] * (v3[3] - v3[0]) >= 0) ? 0x00 : 0x02;
    outcode1 |= ((v1[3] - v1[0]) >= 0) ? 0x00 : 0x02;
    outcode2 |= ((v2[3] - v2[0]) >= 0) ? 0x00 : 0x02;
    outcode3 |= ((v3[3] - v3[0]) >= 0) ? 0x00 : 0x02;

//printf("Outcode1 %02x Outcode2 %02x Outcode3 %02x\n",
//   outcode1, outcode2, outcode3);

    /*  Bottom plane test.  */
    //outcode1 |= (v1[3] * (v1[3] + v1[1]) >= 0) ? 0x00 : 0x04;
    //outcode2 |= (v2[3] * (v2[3] + v2[1]) >= 0) ? 0x00 : 0x04;
    //outcode3 |= (v3[3] * (v3[3] + v3[1]) >= 0) ? 0x00 : 0x04;
    outcode1 |= ((v1[3] + v1[1]) >= 0) ? 0x00 : 0x04;
    outcode2 |= ((v2[3] + v2[1]) >= 0) ? 0x00 : 0x04;
    outcode3 |= ((v3[3] + v3[1]) >= 0) ? 0x00 : 0x04;

//printf("Outcode1 %02x Outcode2 %02x Outcode3 %02x\n",
//    outcode1, outcode2, outcode3);

    /*  Top plane test.  */
    //outcode1 |= (v1[3] * (v1[3] - v1[1]) >= 0) ? 0x00 : 0x08;
    //outcode2 |= (v2[3] * (v2[3] - v2[1]) >= 0) ? 0x00 : 0x08;
    //outcode3 |= (v3[3] * (v3[3] - v3[1]) >= 0) ? 0x00 : 0x08;
    outcode1 |= ((v1[3] - v1[1]) >= 0) ? 0x00 : 0x08;
    outcode2 |= ((v2[3] - v2[1]) >= 0) ? 0x00 : 0x08;
    outcode3 |= ((v3[3] - v3[1]) >= 0) ? 0x00 : 0x08;

//printf("Outcode1 %02x Outcode2 %02x Outcode3 %02x\n",
//    outcode1, outcode2, outcode3);

    /*  Near plane test.  */
    //outcode1 |= (v1[3] * (v1[3] + v1[2]) >= 0) ? 0x00 : 0x10;
    //outcode2 |= (v2[3] * (v2[3] + v2[2]) >= 0) ? 0x00 : 0x10;
    //outcode3 |= (v3[3] * (v3[3] + v3[2]) >= 0) ? 0x00 : 0x10;

    if (d3d9DepthRange)
    {
        outcode1 |= (v1[2] >= 0) ? 0x00 : 0x10;
        outcode2 |= (v2[2] >= 0) ? 0x00 : 0x10;
        outcode3 |= (v3[2] >= 0) ? 0x00 : 0x10;
    }
    else
    {
        outcode1 |= ((v1[3] + v1[2]) >= 0) ? 0x00 : 0x10;
        outcode2 |= ((v2[3] + v2[2]) >= 0) ? 0x00 : 0x10;
        outcode3 |= ((v3[3] + v3[2]) >= 0) ? 0x00 : 0x10;
    }
    
//printf("Outcode1 %02x Outcode2 %02x Outcode3 %02x\n",
//    outcode1, outcode2, outcode3);

    /*  Far plane test.  */
    //outcode1 |= (v1[3] * (v1[3] - v1[2]) >= 0) ? 0x00 : 0x20;
    //outcode2 |= (v2[3] * (v2[3] - v2[2]) >= 0) ? 0x00 : 0x20;
    //outcode3 |= (v3[3] * (v3[3] - v3[2]) >= 0) ? 0x00 : 0x20;
    outcode1 |= ((v1[3] - v1[2]) >= 0) ? 0x00 : 0x20;
    outcode2 |= ((v2[3] - v2[2]) >= 0) ? 0x00 : 0x20;
    outcode3 |= ((v3[3] - v3[2]) >= 0) ? 0x00 : 0x20;

    /*  Sutherland-Cohen clipping test:

        if all three vertices outcodes are 000000
            trivially accept
        else
            if logical and of the three outcodes is not zero
                trivially reject
            else
                intersection detected => accept
            endif
        endif

     */

//printf("Outcode1 %02x Outcode2 %02x Outcode3 %02x\n",
//    outcode1, outcode2, outcode3);
//printf("AND %02x Reject? %s\n", outcode1 & outcode2 & outcode3,
//    ((outcode1 & outcode2 & outcode3) != 0)?"YES":"NO");

    reject = ((outcode1 & outcode2 & outcode3) != 0);

    return reject;
}


/*  Clips the triangle against the frustum clip volume.  */
bool ClipperEmulator::frustumClip(QuadFloat *v1, QuadFloat *v2, QuadFloat *v3)
{
    panic("ClipperEmulator", "frustumClip", "Not implemented.");
    return false;
}

/*  Clips the triangle against the defined user clip planes.  */
bool ClipperEmulator::userClip(QuadFloat *v1, QuadFloat *v2, QuadFloat *v3)
{
    panic("ClipperEmulator", "userClip", "Not implemented.");
    return false;
}

/*  Gets the next clip vertex produced by the last clip operation.  */
QuadFloat *ClipperEmulator::getNextClipVertex()
{
    panic("ClipperEmulator", "getNextClipVertex", "Not implemented.");
    return 0;
}


/*  Defines a new user clip plane.  */
void ClipperEmulator::defineClipPlane(u32bit id, QuadFloat plane)
{
    panic("ClipperEmulator", "defineClipPlane", "Not implemented.");
}

/*  Undefines an user clip plane.  */
void ClipperEmulator::undefineClipPlane(u32bit id)
{
    panic("ClipperEmulator", "undefineClipPlane", "Not implemented.");
}


