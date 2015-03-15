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
 * $RCSfile: QuadInt.cpp,v $
 * $Revision: 1.5 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:55 $
 *
 * Quad Int class implementation file.
 *
 */


#include "QuadInt.h"
#include <iostream>

using namespace gpu3d;
using namespace std;

QuadInt::QuadInt( s32bit x, s32bit y, s32bit z, s32bit w ) {

    component[0] = x;
    component[1] = y;
    component[2] = z;
    component[3] = w;
}

s32bit& QuadInt::operator[]( u32bit index ) {
    GPU_ERROR_CHECK(
        if ( index < 0 || index > 3 ) {
            cout << "Error. QuadFloat Index out of bounds, index value: " << index << endl; //
            // REPORT_ERROR
        }
    );
    // Returning a reference allow the expression to be "left hand"
    return component[index];
}


void QuadInt::setComponents( s32bit x, s32bit y, s32bit z,
                                s32bit w ) {

    component[0] = x;
    component[1] = y;
    component[2] = z;
    component[3] = w;

}

void QuadInt::getComponents( s32bit& x, s32bit& y, s32bit& z, s32bit& w ) {
    x = component[0];
    y = component[1];
    z = component[2];
    w = component[3];
}

s32bit *QuadInt::getVector()
{
    return component;
}

QuadInt& QuadInt::operator=(s32bit *source)
{
    component[0] = source[0];
    component[1] = source[1];
    component[2] = source[2];
    component[3] = source[3];

    return *this;
}


