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

#include "MathLib.h"
#include <cmath>
#include <cstring>

using namespace mathlib;


/*  NOTE: WHY????!!!! ASK WHY ceil of 0x4014000000000000 (double for 5) is 5 but whatever stupid
    optimization is doing for ceil(log(32)/log(2)) is 6.  log(32)/log(2) seems to be
    0x4014000000000000 or I'm doing the printf's really wrong.  If you make it inline will
    be 'optimized' as the other version.  */
double mathlib::ceil2(double x) { return (x - std::floor(x) > 0)?std::floor(x) + 1:std::floor(x); }

// 64 muls
void mathlib::_mult_matrixf( f32bit m[], const f32bit a[], const f32bit b[] )
{

#define A(row,col) a[(col<<2) + row]
#define B(row,col) b[(col<<2) + row]
#define M(row,col) m[(col<<2) + row]

    for ( int i = 0; i < 4; i++ ) /* current row */
    {
        /* a == m allowed, b == m not allowed */
        const f32bit ai0 = A(i,0);
        const f32bit ai1 = A(i,1);
        const f32bit ai2 = A(i,2);
        const f32bit ai3 = A(i,3);

        M(i,0) = ai0*B(0,0) + ai1*B(1,0) + ai2*B(2,0) + ai3*B(3,0);
        M(i,1) = ai0*B(0,1) + ai1*B(1,1) + ai2*B(2,1) + ai3*B(3,1);
        M(i,2) = ai0*B(0,2) + ai1*B(1,2) + ai2*B(2,2) + ai3*B(3,2);
        M(i,3) = ai0*B(0,3) + ai1*B(1,3) + ai2*B(2,3) + ai3*B(3,3);
    }

#undef A
#undef B
#undef M

}

/**
 * Extracted from The Red Book. The Official Guide to Learning OpenGL. pag 76.
 * Caution: If you're programming in C and you declare a matrix as m[4][4], then the element m[i][j] is in
 * the ith column and jth row of the OpenGL transformation matrix. This is the reverse of the standard C
 * convention in which m[i][j] is in row i and column j. To avoid confusion, you should declare your
 * matrices as m[16].
 *
 * Example: Declaring M as TYPE m[16] = { m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,m10,m11,m12,m13,m14,m15 };
 *      _             _
 *     | m0 m4  m8 m12 |
 * M = | m1 m5  m9 m13 |
 *     | m2 m6 m10 m14 |
 *     | m3 m7 m11 m15 |
 *      -             -
 */

void mathlib::_transpose_matrixf( f32bit m[] )
{
    f32bit aux[16];

    memcpy(aux, m, 16*sizeof(f32bit));

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


// Based on Mesa 5.0 version _math_matrix_translate in file m_matrix.c
void mathlib::_translate_matrix( f32bit m[], f32bit x, f32bit y, f32bit z )
{

   m[12] = m[0] * x + m[4] * y + m[8]  * z + m[12];
   m[13] = m[1] * x + m[5] * y + m[9]  * z + m[13];
   m[14] = m[2] * x + m[6] * y + m[10] * z + m[14];
   m[15] = m[3] * x + m[7] * y + m[11] * z + m[15];

}

void mathlib::_scale_matrix(f32bit mat[], f32bit x, f32bit y, f32bit z)
{
     f32bit m[16]; /* scale matrix */

     memcpy(m, IDENTITY, sizeof(f32bit)*16);

     m[0] *= x;
     m[5] *= y;
     m[10] *= z;

     _mult_matrixf(mat,mat, m);
}


/*
 * Generate a 4x4 transformation matrix from glRotate parameters, and
 * postmultiply the input matrix by it.
 * This function contributed by Erich Boleyn (erich@uruk.org).
 * Optimizatios contributed by Rudolf Opalla (rudi@khm.de).
 */
void mathlib::_rotate_matrix( f32bit mat[], f32bit angle, f32bit x, f32bit y, f32bit z )
{
   f32bit xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c, s, c;
   f32bit m[16]; /* rotation matrix */
   bool optimized;

   s = (f32bit) sin( angle * DEG2RAD );
   c = (f32bit) cos( angle * DEG2RAD );

   memcpy(m, IDENTITY, sizeof(f32bit)*16);
   optimized = false;

#define M(row,col)  m[col*4+row]

   if (x == 0.0F) {
      if (y == 0.0F) {
         if (z != 0.0F) {
            optimized = true;
            /* rotate only around z-axis */
            M(0,0) = c;
            M(1,1) = c;
            if (z < 0.0F) {
               M(0,1) = s;
               M(1,0) = -s;
            }
            else {
               M(0,1) = -s;
               M(1,0) = s;
            }
         }
      }
      else if (z == 0.0F) {
         optimized = true;
         /* rotate only around y-axis */
         M(0,0) = c;
         M(2,2) = c;
         if (y < 0.0F) {
            M(0,2) = -s;
            M(2,0) = s;
         }
         else {
            M(0,2) = s;
            M(2,0) = -s;
         }
      }
   }
   else if (y == 0.0F) {
      if (z == 0.0F) {
         optimized = true;
         /* rotate only around x-axis */
         M(1,1) = c;
         M(2,2) = c;
         if (y < 0.0F) {
            M(1,2) = s;
            M(2,1) = -s;
         }
         else {
            M(1,2) = -s;
            M(2,1) = s;
         }
      }
   }

   if (!optimized) {
      const f32bit mag = (f32bit) mathlib::sqrt(x * x + y * y + z * z);

      if (mag <= 1.0e-4) {
         /* no rotation, leave mat as-is */
         return;
      }

      x /= mag;
      y /= mag;
      z /= mag;


      /*
       * Arbitrary axis rotation matrix.
       *
       * Version extracted from Mesa 5.0
       * File: src\math\m_matrix.c
       * Function: _math_matrix_rotate()
       */

      xx = x * x;
      yy = y * y;
      zz = z * z;
      xy = x * y;
      yz = y * z;
      zx = z * x;
      xs = x * s;
      ys = y * s;
      zs = z * s;
      one_c = 1.0F - c;

      /* We already hold the identity-matrix so we can skip some statements */
      M(0,0) = (one_c * xx) + c;
      M(0,1) = (one_c * xy) - zs;
      M(0,2) = (one_c * zx) + ys;
/*    M(0,3) = 0.0F; */

      M(1,0) = (one_c * xy) + zs;
      M(1,1) = (one_c * yy) + c;
      M(1,2) = (one_c * yz) - xs;
/*    M(1,3) = 0.0F; */

      M(2,0) = (one_c * zx) - ys;
      M(2,1) = (one_c * yz) + xs;
      M(2,2) = (one_c * zz) + c;
/*    M(2,3) = 0.0F; */

/*
      M(3,0) = 0.0F;
      M(3,1) = 0.0F;
      M(3,2) = 0.0F;
      M(3,3) = 1.0F;
*/
   }
#undef M

    _mult_matrixf(mat, mat, m);
}

#define SWAP_ROWS(a, b) { f32bit *_tmp = a; (a)=(b); (b)=_tmp; }
#define MAT(m,r,c) (m)[(c)*4+(r)]

/*
 * Compute inverse of 4x4 transformation matrix.
 * Code contributed by Jacques Leroy jle@star.be
 * Return GL_TRUE for success, GL_FALSE for failure (singular matrix)
 */
bool mathlib::_invert_matrixf( const f32bit* m, f32bit* out )
{
   f32bit wtmp[4][8];
   f32bit m0, m1, m2, m3, s;
   f32bit *r0, *r1, *r2, *r3;

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


void mathlib::_mult_mat_vect( const f32bit* mat, f32bit* vect)
{
    f32bit x = vect[0];
    f32bit y = vect[1];
    f32bit z = vect[2];
    f32bit w = vect[3];

    #define __DP4(i) vect[i] = x*mat[i] + y*mat[i+4] + z*mat[i+8] + w*mat[i+12];

    __DP4(0);
    __DP4(1);
    __DP4(2);
    __DP4(3);

    #undef __DP4
}


float mathlib::_cos(float angleInDegrees)
{
    return float(std::cos(angleInDegrees*DEG2RAD));
}


double mathlib::log2(double x) { return std::log(x)/std::log(2.0); }


double mathlib::floor(double x) { return std::floor(x); }
double mathlib::ceil(double x) { return std::ceil(x); }
