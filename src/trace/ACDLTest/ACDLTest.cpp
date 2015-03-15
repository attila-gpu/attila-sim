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

#include "ACD.h"
#include "ACDBuffer.h"
#include "ACDVector.h"
#include "ACDMatrix.h"
#include <iostream>

using namespace acdlib;
using namespace std;

template<class T,acd_uint D>
ostream& operator<<(ostream& os, const ACDVector<T,D>& vect)
{
    acd_uint i = 0;
    os << "{";
    for ( ; i < vect.dim() - 1; ++i )
        os << vect[i] << ",";
    os << vect[i] << "}";
    return os;
}

template<class T,acd_uint R, acd_uint C>
ostream& operator<<(ostream& os, const ACDMatrix<T,R,C>& mat)
{
    os << "{";
    for ( acd_uint i = 0; i < R; ++i ) {
        os << "{";
        for ( acd_uint j = 0; j < C; ++j ) {
            os << mat(i,j);
            if ( j < C - 1)
                os << ",";
            else
                os << "}\n";
        }
        if ( i < R - 1 ) os << ",";
        else os << "}\n";
    }
    return os;
}

/*
void matrixTest()
{
    acd_uint data[] = {
        1,2,3,4,
        5,6,7,8,
        9,10,11,12 };

    ACDMatrix<acd_float,3,4> a(0);

    a(0,1) = 1;

    ACDMatrix<acd_float,4,3> b;
    
    b = a.transpose();

    ACDMatrix<acd_int,3,4> A(data, true);    
    ACDMatrix<acd_int,4,3> B(data, false);
    B(0,2) = 10;
    B(1,2) = 11;
    B(2,2) = 12;
    B(3,2) = 13;
    ACDMatrix<acd_int,3,3> C;

    C = A * B;

    cout << "Matrix A = \n" << A << "\n";
    cout << "Matrix B = \n" << B << "\n";
    cout << "Matrix C = \n" << C << "\n";
}
*/

void main()
{
    using namespace acdlib;
    ACDDevice* acddev = acdlib::createDevice(0);
    cout << "Available streams: " << acddev->availableStreams() << "\n";
    /*
    MortonTransformation mt;

    mt.setTilingParameters(3,3);
    */

/*
    acd_int a_data[] = {1,2,1,2};
    acd_int b_data[] = {10,-10, 10, -10};

    ACDVector<acd_int, 4> a(-1);
    ACDVector<acd_int,4> b(b_data);
    acd_int s;

    cout << "a: " << a << "\n";
    cout << "b: " << b << "\n";
    cout << "result: " << s << "\n";



    ACDDevice* acddev = acdlib::createDevice();

    ACDBuffer* buffer = acddev->createBuffer();

    buffer->resize(1, false);

    cout << "ACDTest finished!" << endl;

    ACD_RT_DESC rtDesc = { ACD_FORMAT_UNKNOWN, ACD_RT_DIMENSION_BUFFER, { 48, 24 } };

    ACDRenderTarget* rt = acddev->createRenderTarget(buffer, &rtDesc);

    acddev->setRenderTargets(1, &rt, 0);
    
*/

}
