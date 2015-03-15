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

#ifndef MATHLIB_H
    #define MATHLIB_H

#include "GPUTypes.h"

#include <cmath>

#ifdef max
    #undef max
#endif
#ifdef min
    #undef min
#endif

namespace mathlib
{    
    
/**
 * MathLib constants
 */
static const double PI = 3.1415926536;
static const double DEG2RAD = (PI/180.0f);

template<typename T>
f32bit sqrtf(T x) { return (f32bit)std::sqrt(x); }

template<typename T>
double sqrt(T x) { return std::sqrt(x); }

template<typename T>
T max(T a, T b) { return (a > b ? a : b); }

template<typename T>
T min(T a, T b) { return (a < b ? a : b); }

#ifdef log2
    #undef log2
#endif

double log2(double x);
double floor(double x);
double ceil(double x);

/*  NOTE: WHY????!!!! ASK WHY ceil of 0x4014000000000000 (double for 5) is 5 but whatever stupid
    optimization is doing for ceil(log(32)/log(2)) is 6.  log(32)/log(2) seems to be
    0x4014000000000000 or I'm doing the printf's really wrong.  If you make it inline will
    be 'optimized' as the other version.  */
double ceil2(double x);

/*
 * Identity 4x4 Matrix
 */
static f32bit IDENTITY[] =
{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};


 /* a == m allowed, b == m not allowed */
void _mult_matrixf( f32bit m[], const f32bit a[], const f32bit b[] );

/* transpose a matrix */
void _transpose_matrixf( f32bit m[] );

/*
 * Generate a 4x4 transformation matrix from glRotate parameters, and
 * postmultiply the input matrix by it.
 * Based on Mesa 5.0, function _math_matrix_rotate in file m_matrix.c
 */
void _rotate_matrix( f32bit mat[], f32bit angle, f32bit x, f32bit y, f32bit z );

/* Scale function */
void _scale_matrix(f32bit mat[], f32bit x, f32bit y, f32bit z);

/* Based on Mesa 5.0, function _math_matrix_translate in file m_matrix.c */
void _translate_matrix( f32bit mat[], f32bit x, f32bit y, f32bit z );

/*
 * Compute inverse of 4x4 transformation matrix.
 * Code contributed by Jacques Leroy jle@star.be
 * Mesa 5.0
 */
bool _invert_matrixf( const f32bit* m, f32bit* out );

/**
 * vect = Matrix * vect;
 *
 * @note Assumes matrix placed in columns
 */
void _mult_mat_vect( const f32bit* mat, f32bit* vect);

/**
 * computes cos function given an angle in degrees
 */
f32bit _cos(float angleInDegrees);

} // namespace math

#endif
