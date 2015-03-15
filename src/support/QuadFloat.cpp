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
 * $RCSfile: QuadFloat.cpp,v $
 * $Revision: 1.5 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:55 $
 *
 * Quad Float class implementation file. 
 *
 */


#include "QuadFloat.h"
#include <iostream>

using namespace gpu3d;
using namespace std;

QuadFloat::QuadFloat( f32bit x, f32bit y, f32bit z, f32bit w ) {
    
    component[0] = x;
    component[1] = y;
    component[2] = z;
    component[3] = w;
}

f32bit& QuadFloat::operator[]( u32bit index ) {
    GPU_ERROR_CHECK(
        if ( index < 0 || index > 3 ) {
            cout << "Error. QuadFloat Index out of bounds, index value: " << index << endl; //
            // REPORT_ERROR
        }
    );
    // Returning a reference allow the expression to be "left hand"
    return component[index];
}


void QuadFloat::setComponents( f32bit x, f32bit y, f32bit z,
                                f32bit w ) {

    component[0] = x;
    component[1] = y;
    component[2] = z;
    component[3] = w;    

}

void QuadFloat::getComponents( f32bit& x, f32bit& y, f32bit& z, f32bit& w ) {
    x = component[0];
    y = component[1];
    z = component[2];
    w = component[3];
}

f32bit *QuadFloat::getVector()
{
    return component;
}

QuadFloat& QuadFloat::operator=(f32bit *source)
{
    component[0] = source[0];
    component[1] = source[1];
    component[2] = source[2];
    component[3] = source[3];

    return *this;
}

