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

#ifndef ACDX_MATRIXF_H
    #define ACDX_MATRIXF_H

#include <cstring>
#include <iostream>

namespace acdlib
{
//#define MATRIX_TRACE_CODE 

#ifdef MATRIX_TRACE_CODE
    #define MATRIXF_TC(x) x
#else
    #define MATRIXF_TC(x)
#endif


class ACDXMatrixf
{

private:
    
    float rows[16];    

    /* a == m allowed, b == m not allowed */
    static void _mult_matrixf(float m[], const float a[], const float b[]);
    static void _transpose_matrixf(float m[]);
    static bool _invert_matrixf(const float* m, float* out);

public:

    ACDXMatrixf();
    ACDXMatrixf(const ACDXMatrixf& m);    
    
    /**
     * Creates a ACDXMatrixf object using a row or column representation array
     *
     * colRep: true means 'rows' pointer points to a matrix stored in columns
     *         false means matrix stored in rows
     */
    ACDXMatrixf(const float* rows, bool colRep = false);    
    ACDXMatrixf(const double* rows, bool colRep = false); /* truncates doubles to floats */



    ACDXMatrixf& operator=(const ACDXMatrixf& m);
    
    ACDXMatrixf& identityMe();

    /* gets the identity matrix */
    static ACDXMatrixf identity();

    /* if u want to get the columns of a matrix 'm' do: m.transpose().getRows() */
    /* @note: Allow direct access to internal representation in rows */
    float* getRows() const;

    ACDXMatrixf operator*(const ACDXMatrixf& m) const;

    /* A *= B is equivalent to A = A * B (not B*A) but more efficient; */
    ACDXMatrixf& operator*=(const ACDXMatrixf& m);

    /* gets the inverse of this matrix */
    ACDXMatrixf inverse() const;        
    ACDXMatrixf& inverseMe();

    /* gets the transpose of this matrix */
    ACDXMatrixf transpose() const;
    ACDXMatrixf& transposeMe();

    /* 
     * gets the transpose inverse of this matrix 
     * equivalent to: m.inverse().transpose()
     */
    ACDXMatrixf invtrans() const;
    ACDXMatrixf& invtransMe();

    /* allows indexing in a matrix like in a two dimensional array. Example: m[3][2] = 12.3; */
    float* operator[](int row)
    {
        return &(rows[row*4]);
    }

    /* 
     * Returns a 16 floats array containing the matrix representation in rows 
     * equivalent to getRows().
     */
    const float* operator()()
    {
        return rows;
    }

    /*
     * Compare two matrices
     */
    bool operator==(const ACDXMatrixf& m) const;
    
    friend std::ostream& operator<<(std::ostream& os, const ACDXMatrixf& m)
    {
        m.dump(os);
        return os;
    }


    void dump(std::ostream& out = std::cout) const;
    
};

} // namespace acdlib

#endif // ACDX_MATRIXF_H
