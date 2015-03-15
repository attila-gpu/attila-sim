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

#ifndef ACD_MATRIX
    #define ACD_MATRIX

#include "ACDTypes.h"
#include "ACDVector.h"
#include <iostream>
#include <cmath>

using namespace std;

namespace acdlib
{

template<class T, acd_uint R, acd_uint C>
class ACDMatrix
{
public:

    ACDMatrix() {};
    ACDMatrix(const T& initValue);

    ACDMatrix(const T* values, acd_bool rowMajorOrder);

    T& operator()(acd_uint i, acd_uint j);

    ACDVector<T,C>& operator[](acd_uint row);
    const ACDVector<T,C>& operator[](acd_uint row) const;
    
    const T& operator()(acd_uint i, acd_uint j) const;

    acd_bool operator==(const ACDMatrix<T,R,C>& m) const
    {
        for ( acd_uint i = 0; i < R; ++i ) {
            if ( _rows[i] != m._rows[i] )
                return false;
        }
        return true;
    }

    acd_bool operator!=(const ACDMatrix<T,R,C>& m) const
    {
        for ( acd_uint i = 0; i < R; ++i ) {
            if ( _rows[i] != m._rows[i] )
                return true;
        }
        return false;
    }

    ACDVector<T,R> multVector(ACDVector<T,C>& v) const
    {
        ACDVector<T,R> result;
        for ( acd_uint i = 0; i < R; ++i ) {
            for ( acd_uint j = 0; j < C; ++j )
                result[i] = v[j] * _rows[i][j];
        }
        return result;
    }


    ACDVector<T,C> row(acd_uint r) const;

    ACDVector<T,R> col(acd_uint c) const;

    ACDMatrix<T,C,R> transpose() const;

    template<acd_uint C2>
    ACDMatrix<T,R,C2> operator*(const ACDMatrix<T,C,C2>& mat) const
    {
        ACDMatrix<T,R,C2> result;
        for ( acd_uint i = 0; i < R; ++i ) {
            for ( acd_uint j = 0; j < C2; ++j ) {
                T accum = 0;
                for ( acd_uint k = 0; k < C; ++k )
                    accum += (_rows[i][k] * mat(k,j));
                result(i,j) = accum;
            }
        }
        return result;
    }

    ///////////////////
    // PRINT METHODS //
    ///////////////////

    friend std::ostream& operator<<(std::ostream& os, const ACDMatrix<T,R,C>& m)
    {
        m.print(os);
        return os;
    }

    void print(std::ostream& out = cout) const;


private:

    ACDVector<T,C> _rows[R];

};

////////////////////////////////////////////////////
// INVERSE OPERATION DEFINED FOR 4x4 FLOAT MATRIX // 
////////////////////////////////////////////////////

template<class T,acd_uint RC>
void identityMatrix(ACDMatrix<T,RC,RC>& m)
{
    for ( acd_uint i = 0; i < RC; ++i ) {
        for ( acd_uint j = 0; j < RC; ++j )
            m(i,j) = static_cast<T>(0);
    }

    for ( acd_uint i = 0; i < RC; ++i )
        m(i,i) = static_cast<T>(1);
}

/**
 * Computes the inverse of the input 4x4 matrix
 *
 * Code contributed by Jacques Leroy jle@star.be. 
 *
 * @param    outMat    The output 4x4 acd_float inverse matrix.
 * @param    inMat    The input 4x4 acd_float matrix.
 * @returns True for success, false for failure (singular matrix).
 */
acdlib::acd_bool _inverse(acdlib::ACDMatrix<acdlib::acd_float,4,4>& outMat, const acdlib::ACDMatrix<acdlib::acd_float,4,4>& inMat);

acdlib::acd_bool _translate(acdlib::ACDMatrix<acdlib::acd_float,4,4>& inOutMat, 
                            acd_float x, acd_float y, acd_float z);

void _scale(acdlib::ACDMatrix<acdlib::acd_float,4,4>& inOutMat,
                    acd_float x, acd_float y, acd_float z);

void _rotate(acdlib::ACDMatrix<acdlib::acd_float,4,4>& inOutMat, 
                            acd_float angle, acd_float x, acd_float y, acd_float z);

void _mult_mat_vect( acdlib::ACDVector<acdlib::acd_float,4>& vect, const acdlib::ACDMatrix<acdlib::acd_float,4,4>& mat);
// Convenient matrix definition
typedef ACDMatrix<acd_float,4,4> ACDFloatMatrix4x4;


} // namespace acdlib

template<class T, acdlib::acd_uint R, acdlib::acd_uint C>
acdlib::ACDMatrix<T,R,C>::ACDMatrix(const T& initValue)
{
    for ( acd_uint i = 0; i < R; ++i ) {
        for ( acd_uint j = 0; j < C; ++j ) {
            _rows[i][j] = initValue;
        }
    }
}

template<class T, acdlib::acd_uint R, acdlib::acd_uint C>
acdlib::ACDMatrix<T,R,C>::ACDMatrix(const T* values, acdlib::acd_bool rows) 
{
    acd_uint k = 0;
    if ( rows ) {
        for ( acd_uint i = 0; i < R; i++ ) {
            for ( acd_uint j = 0; j < C; ++j ) {
                _rows[i][j] = values[k++];
            }
        }
    }
    else {
        for ( acd_uint i = 0; i < C; i++ ) {
            for ( acd_uint j = 0; j < R; ++j ) {
                _rows[j][i] = values[k++];
            }
        }
    }
}

template<class T, acdlib::acd_uint R, acdlib::acd_uint C>
acdlib::ACDVector<T,C>& acdlib::ACDMatrix<T,R,C>::operator[](acdlib::acd_uint row)
{
    return _rows[row];
}

template<class T, acdlib::acd_uint R, acdlib::acd_uint C>
const acdlib::ACDVector<T,C>& acdlib::ACDMatrix<T,R,C>::operator[](acdlib::acd_uint row) const
{
    return _rows[row];
}


template<class T, acdlib::acd_uint R, acdlib::acd_uint C>
T& acdlib::ACDMatrix<T,R,C>::operator()(acdlib::acd_uint i, acdlib::acd_uint j) { return _rows[i][j]; }

template<class T, acdlib::acd_uint R, acdlib::acd_uint C>
const T& acdlib::ACDMatrix<T,R,C>::operator()(acdlib::acd_uint i, acdlib::acd_uint j) const { return _rows[i][j]; }

template<class T, acdlib::acd_uint R, acdlib::acd_uint C>
acdlib::ACDVector<T,C> acdlib::ACDMatrix<T,R,C>::row(acdlib::acd_uint r) const { return _rows[r]; }

template<class T, acdlib::acd_uint R, acdlib::acd_uint C>
acdlib::ACDVector<T,R> acdlib::ACDMatrix<T,R,C>::col(acdlib::acd_uint c) const
{
    ACDVector<T,R> column;
    for ( acd_uint i = 0; i < R; ++i )
        column[i] = _rows[i][c];
    return column;
}

template<class T, acdlib::acd_uint R, acdlib::acd_uint C>
acdlib::ACDMatrix<T,C,R> acdlib::ACDMatrix<T,R,C>::transpose() const
{
    ACDMatrix<T,C,R> m;
    for ( acd_uint i = 0; i < R; ++i ) {
        for ( acd_uint j = 0; j < C; ++j )
            m(j,i) = _rows[i][j];
    }
    return m;
}

template<class T, acdlib::acd_uint R, acdlib::acd_uint C>
void acdlib::ACDMatrix<T,R,C>::print(std::ostream& out) const
{
    for ( acdlib::acd_uint i = 0; i < R; ++i ) 
    {
        out << "{";

        for ( acdlib::acd_uint j = 0; j < C; ++j )
        {
            out << _rows[i][j];
            if ( j != C - 1 ) out << ",";
        }

        out << "}";

        if ( i != R - 1 ) out << ",";
    }

    out.flush();
}



#endif // ACD_MATRIX
