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

#include "Matrixf.h"
#include <iostream>
#include <cmath>

using namespace std;

Matrixf Matrixf::identity()
{
    float rep[] = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    return Matrixf(rep);
}

Matrixf Matrixf::operator*(const Matrixf& m) const
{
    float result[16];
    _mult_matrixf(result, transpose().getRows(), m.transpose().getRows());
    return Matrixf(result, true); /* use a representation in columns to create the matrix */
}


Matrixf& Matrixf::operator*=(const Matrixf& m)
{
    _mult_matrixf(rows, transpose().getRows(), m.transpose().getRows());
    _transpose_matrixf(rows);
    return *this;
}

// 64 muls
void Matrixf::_mult_matrixf(float m[], const float a[], const float b[])
{

#define A(row,col) a[(col<<2) + row]
#define B(row,col) b[(col<<2) + row]
#define M(row,col) m[(col<<2) + row]

    for ( int i = 0; i < 4; i++ ) /* current row */
    {
        /* a == m allowed, b == m not allowed */
        const float ai0 = A(i,0);
        const float ai1 = A(i,1);
        const float ai2 = A(i,2);
        const float ai3 = A(i,3);

        M(i,0) = ai0*B(0,0) + ai1*B(1,0) + ai2*B(2,0) + ai3*B(3,0);
        M(i,1) = ai0*B(0,1) + ai1*B(1,1) + ai2*B(2,1) + ai3*B(3,1);
        M(i,2) = ai0*B(0,2) + ai1*B(1,2) + ai2*B(2,2) + ai3*B(3,2);
        M(i,3) = ai0*B(0,3) + ai1*B(1,3) + ai2*B(2,3) + ai3*B(3,3);
    }

#undef A
#undef B
#undef M

}


void Matrixf::_transpose_matrixf(float m[])
{
    float aux[16];

    memcpy(aux, m, 16*sizeof(float));

    m[0] = aux[0];
    m[1] = aux[4];
    m[2] = aux[8];
    m[3] = aux[12];

    m[4] = aux[1];
    m[5] = aux[5];
    m[6] = aux[9];
    m[7] = aux[13];

    m[8] = aux[2];
    m[9] = aux[6];
    m[10] = aux[10];
    m[11] = aux[14];

    m[12] = aux[3];
    m[13] = aux[7];
    m[14] = aux[11];
    m[15] = aux[15];
}

Matrixf Matrixf::inverse() const
{
    float out[16];
    _invert_matrixf(transpose().getRows(), out);
    return Matrixf(out,true);
}

Matrixf& Matrixf::inverseMe()
{
    (*this) = inverse();
    return *this;
}

Matrixf Matrixf::transpose() const
{
    return Matrixf(rows, true); /* force transpose code */
}

Matrixf& Matrixf::transposeMe()
{
    *this = transpose();
    return *this;
}

Matrixf Matrixf::invtrans() const
{
    float out[16];
    _invert_matrixf(transpose().getRows(), out);
    return Matrixf(out); /* already transposed */
}

Matrixf& Matrixf::invtransMe()
{
    *this = invtrans();
    return *this;
}


#define SWAP_ROWS(a, b) { float *_tmp = a; (a)=(b); (b)=_tmp; }
#define MAT(m,r,c) (m)[(c)*4+(r)]

/*
 * Compute inverse of 4x4 transformation matrix.
 * Code contributed by Jacques Leroy jle@star.be
 * Return GL_TRUE for success, GL_FALSE for failure (singular matrix)
 */
bool Matrixf::_invert_matrixf( const float* m, float* out )
{
   float wtmp[4][8];
   float m0, m1, m2, m3, s;
   float *r0, *r1, *r2, *r3;

   r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

   r0[0] = MAT(m,0,0), r0[1] = MAT(m,0,1),
   r0[2] = MAT(m,0,2), r0[3] = MAT(m,0,3),
   r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,

   r1[0] = MAT(m,1,0), r1[1] = MAT(m,1,1),
   r1[2] = MAT(m,1,2), r1[3] = MAT(m,1,3),
   r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,

   r2[0] = MAT(m,2,0), r2[1] = MAT(m,2,1),
   r2[2] = MAT(m,2,2), r2[3] = MAT(m,2,3),
   r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,

   r3[0] = MAT(m,3,0), r3[1] = MAT(m,3,1),
   r3[2] = MAT(m,3,2), r3[3] = MAT(m,3,3),
   r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;

   /* choose pivot - or die */
   if (fabs(r3[0])>fabs(r2[0])) SWAP_ROWS(r3, r2);
   if (fabs(r2[0])>fabs(r1[0])) SWAP_ROWS(r2, r1);
   if (fabs(r1[0])>fabs(r0[0])) SWAP_ROWS(r1, r0);
   if (0.0 == r0[0])  return false;

   /* eliminate first variable     */
   m1 = r1[0]/r0[0]; m2 = r2[0]/r0[0]; m3 = r3[0]/r0[0];
   s = r0[1]; r1[1] -= m1 * s; r2[1] -= m2 * s; r3[1] -= m3 * s;
   s = r0[2]; r1[2] -= m1 * s; r2[2] -= m2 * s; r3[2] -= m3 * s;
   s = r0[3]; r1[3] -= m1 * s; r2[3] -= m2 * s; r3[3] -= m3 * s;
   s = r0[4];
   if (s != 0.0) { r1[4] -= m1 * s; r2[4] -= m2 * s; r3[4] -= m3 * s; }
   s = r0[5];
   if (s != 0.0) { r1[5] -= m1 * s; r2[5] -= m2 * s; r3[5] -= m3 * s; }
   s = r0[6];
   if (s != 0.0) { r1[6] -= m1 * s; r2[6] -= m2 * s; r3[6] -= m3 * s; }
   s = r0[7];
   if (s != 0.0) { r1[7] -= m1 * s; r2[7] -= m2 * s; r3[7] -= m3 * s; }

   /* choose pivot - or die */
   if (fabs(r3[1])>fabs(r2[1])) SWAP_ROWS(r3, r2);
   if (fabs(r2[1])>fabs(r1[1])) SWAP_ROWS(r2, r1);
   if (0.0 == r1[1])  return false;

   /* eliminate second variable */
   m2 = r2[1]/r1[1]; m3 = r3[1]/r1[1];
   r2[2] -= m2 * r1[2]; r3[2] -= m3 * r1[2];
   r2[3] -= m2 * r1[3]; r3[3] -= m3 * r1[3];
   s = r1[4]; if (0.0 != s) { r2[4] -= m2 * s; r3[4] -= m3 * s; }
   s = r1[5]; if (0.0 != s) { r2[5] -= m2 * s; r3[5] -= m3 * s; }
   s = r1[6]; if (0.0 != s) { r2[6] -= m2 * s; r3[6] -= m3 * s; }
   s = r1[7]; if (0.0 != s) { r2[7] -= m2 * s; r3[7] -= m3 * s; }

   /* choose pivot - or die */
   if (fabs(r3[2])>fabs(r2[2])) SWAP_ROWS(r3, r2);
   if (0.0 == r2[2])  return false;

   /* eliminate third variable */
   m3 = r3[2]/r2[2];
   r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
   r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6],
   r3[7] -= m3 * r2[7];

   /* last check */
   if (0.0 == r3[3]) return false;

   s = 1.0F/r3[3];             /* now back substitute row 3 */
   r3[4] *= s; r3[5] *= s; r3[6] *= s; r3[7] *= s;

   m2 = r2[3];                 /* now back substitute row 2 */
   s  = 1.0F/r2[2];
   r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
   r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
   m1 = r1[3];
   r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
   r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
   m0 = r0[3];
   r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
   r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;

   m1 = r1[2];                 /* now back substitute row 1 */
   s  = 1.0F/r1[1];
   r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
   r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
   m0 = r0[2];
   r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
   r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;

   m0 = r0[1];                 /* now back substitute row 0 */
   s  = 1.0F/r0[0];
   r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
   r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);

   MAT(out,0,0) = r0[4]; MAT(out,0,1) = r0[5],
   MAT(out,0,2) = r0[6]; MAT(out,0,3) = r0[7],
   MAT(out,1,0) = r1[4]; MAT(out,1,1) = r1[5],
   MAT(out,1,2) = r1[6]; MAT(out,1,3) = r1[7],
   MAT(out,2,0) = r2[4]; MAT(out,2,1) = r2[5],
   MAT(out,2,2) = r2[6]; MAT(out,2,3) = r2[7],
   MAT(out,3,0) = r3[4]; MAT(out,3,1) = r3[5],
   MAT(out,3,2) = r3[6]; MAT(out,3,3) = r3[7];

   return true;
}
#undef SWAP_ROWS


float* Matrixf::getRows() const
{
    return (float*)rows;
}

Matrixf& Matrixf::identityMe()
{
    (*this) = identity();
    return *this;
}


Matrixf::Matrixf()
{
    MATRIXF_TC(cout << "Using default constructor" << endl;)
}

Matrixf::Matrixf(const Matrixf& m)
{
    MATRIXF_TC(cout << "Using copy constructor" << endl;)
    memcpy(rows, m.rows, sizeof(float)*16);
}


Matrixf::Matrixf(const float* rows_, bool inColumnsRepresentation)
{
    int i,j;
    if ( inColumnsRepresentation == false ) // Matrix given in rows
    {
        MATRIXF_TC(cout << "Using constructor Matrixf(const float*,bool) (memcpy)" << endl;)
        memcpy(rows,rows_,sizeof(float)*16);
    }
    else // Matrix given in columns
    {
        MATRIXF_TC(cout << "Using constructor Matrixf(const float*,bool) (slow copy, columns)" << endl;)
        for ( i = 0; i < 4; i++ )
            for ( j = 0; j < 4; j++ )
                rows[j+4*i] = rows_[4*j+i];
    }
}

Matrixf::Matrixf(const double* rows_, bool inColumnsRepresentation)
{
    int i,j;
    if ( inColumnsRepresentation == false ) // Matrix given in rows
    {
        MATRIXF_TC(cout << "Using constructor Matrixf(const float*,bool) (slow copy_double, rows)" << endl;)
        for ( i = 0; i < 16; i++ )
            rows[i] = float(rows[i]);
    }
    else // Matrix given in columns
    {
        MATRIXF_TC(cout << "Using constructor Matrixf(const float*,bool) (slow copy_double, columns)" << endl;)
        for ( i = 0; i < 4; i++ )
            for ( j = 0; j < 4; j++ )
                rows[j+4*i] = float(rows_[4*j+i]);
    }
}



Matrixf& Matrixf::operator=(const Matrixf& m)
{
    MATRIXF_TC(cout << "Using assignment operator (=)" << endl;)
    memcpy(rows, m.rows, sizeof(float)*16);
    return *this;
}


void Matrixf::dump(ostream& out) const
{
    //long prevState = cout.setf(ios::fixed);
    //long prevPrecision = cout.precision(2);

    for ( int i = 0; i < 16; i++ )
    {
        out << rows[i] << " ";
        if ( (i+1) % 4 == 0 )
            out << "\n";
    }
    out.flush();
    //cout.setf(prevState);
    //cout.precision(prevPrecision);

}

bool Matrixf::operator==(const Matrixf& m) const
{
    if (memcmp(rows,m.rows,sizeof(float)*16) == 0)
        return true;
    else
        return false;
}
