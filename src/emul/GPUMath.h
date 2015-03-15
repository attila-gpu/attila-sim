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
 * $RCSfile: GPUMath.h,v $
 * $Revision: 1.32 $
 * $Author: csolis $
 * $Date: 2008-05-31 14:50:41 $
 *
 * GPU Math library.
 *
 */

#ifndef _GPU_MATH_
    #define _GPU_MATH_

#include "GPUTypes.h"
#include <cmath>
#include "GPU.h"
#include "SetupTriangle.h"
#include "FixedPoint.h"


namespace gpu3d
{

/*  Dead to ISOC99!!!! */

#ifndef NAN
#define NAN \
  (__extension__                                                            \
   ((union { unsigned __l __attribute__((__mode__(__SI__))); float __d; })  \
    { __l: 0x7fc00000UL }).__d)
#endif

#undef MAX

#ifndef finite
#define finite(x) (sizeof(x) == sizeof(f32bit))?finite32(x):finite64(x)
#define finite32(x) (((*((u32bit *)&(x))) & u32bit(0x7F800000UL)) != u32bit(0x7F800000UL))
#define finite64(x) (((*((u64bit *)&(x))) & u64bit(0x7FF0000000000000ULL)) != u64bit(0x7FF0000000000000ULL))
#endif

/**
 *
 *  Defines a bias value to create an area around 0 that it is considered as 0 for
 *  rasterization purposes.
 *
 */
static const f64bit MIN_BIAS = 1.0e-12;

/**  Macros for GPU math functions.  (new version without macros) */
f64bit GPU_LOG2(f64bit x);
f64bit GPU_CEIL(f64bit x);
f64bit GPU_CEIL2(f64bit x);
f64bit GPU_FLOOR(f64bit x);

f64bit GPU_SQRT(f64bit x);
f64bit GPU_POWER2OF(f64bit exp);

template<typename T>
T GPU_ABS(T x)
{
    return ((x) < 0 ? -(x) : (x));
}

template<typename T>
T GPU_MAX(T x,T y)
{
    return ((x) > (y) ? (x) : (y));
}

template<typename T>
T GPU_MIN(T x, T y)
{
    return ((x) < (y) ? (x) : (y));
}

template<typename T>
T GPU_CLAMP(T v, T vMin,T vMax)
{
    return ( (vMin)<=(v) && (v)<=(vMax) ? v : ( (v)<(vMin) ? (vMin) : (vMax) ) );
}


static f64bit GPU_POWER(f64bit base, f64bit exp)
{
    return std::pow(base,exp);
}

template<typename T>
T GPU_PMOD(T x, T n)
{
    return (((x) + (n)) % (n));
}

/*  Calculates the 2 power of an unsigned integer exponent.  */
template<typename T>
T GPU_2N(T shift)
{
    return (1 << shift);
}

template<typename T>
bool GPU_IS_POSITIVE(T x) { return x > -MIN_BIAS; }

template<typename T>
bool GPU_IS_ZERO(T x) { return fabs(x) < MIN_BIAS; }

template<typename T>
bool GPU_IS_LESS_EQUAL(T x, T ref) { return (x <= (ref + MIN_BIAS)); }

/*  GPU constants.  */
#define GPU_NAN NAN

/**
 *  @b GPUMath implements the math needed by Simulator.
 *
 *  @b Files: GPUMath.h and GPUMath.cpp
 *
 *  - Implements a collection of methods that represents the differents math operations.
 *  - All the methods are @e stateless .
 *  - All methods are @e static .
 *  - Method invocation example ( DP3 example ):
 *      @code
 *          f32bit in1[] = { 2.0, 2.0, 2.0, 1.0 };
 *          f32bit in2[] = { 3.0, 1.0, 0.0, 7.0 };
 *          f32bit out[4];
 *
 *          GPUMath::DP3( in1, in2, out ); // Using vector version
 *
 *          // out[0] = out[1] = out[2] = out[3] = 8.0
 *      @endcode
 *
 *  - All methods support using parameters as in and out targets
 *       @code
 *          // example of using a vector as in and out target
 *          f32bit v[]= { 2.1, -1.7, 2.3, 1.45 };
 *          GPUMath::FRC( v, v ); // It is safe
 *          // now, v = { 0.1, 0.3, 0.3, 0.45 }
 *          GPUMath::ADD( v, v, v ); // safe, equals to 2*v
 *          // now, v = { 0.2, 0.6, 0.6, 0.9 };
 *          f32bit dp4 = GPUMath::DP4( v, v, v ); // safe
 *          // now, v = { 1.57, 1.57, 1.57, 1.57 } AND dp4 = 1.57
 *       @endcode
 *
 *  - Operations available:
 *        - @a ABS ( 'vector', 'component by component with vector result' and scalar versions )
 *        - @a ADD ( 'vector' and 'component by component with vector result' versions )
 *        - @a ARL ( 'vector' version )
 *        - @a CMP ( 'vector' and 'component by component with scalar return' versions }
 *        - @a DP3 ( 'vector' and 'component by component with scalar return' versions )
 *        - @a DP4 ( 'vector' and 'component by component with scalar return' versions )
 *        - @a DPH ( 'vector' and 'component by component with scalar return' versions )
 *        - @a DST ( 'vector' version )
 *        - @a EX2 ( 'vector' version )
 *        - @a EXP ( 'vector' version )
 *        - @a FRC ( 'vector' version )
 *        - @a LIT ( 'vector' version )
 *        - @a LG2 ( 'vector' version )
 *        - @a LOG ( 'vector' version )
 *        - @a MAD ( 'vector' and 'component by component with vector result' versions )
 *        - @a MAX ( 'vector' and 'component by component with vector result' versions )
 *        - @a MIN ( 'vector' and 'component by component with vector result' versions )
 *        - @a MOV ( 'vector' version )
 *        - @a MUL ( 'vector' and 'component by component with vector result' versions )
 *        - @a RCP ( 'vector' and 'scalar' versions )
 *        - @a RSQ ( 'vector' and 'scalar' versions )
 *        - @a SAT ( 'vector' version )
 *        - @a SGE ( 'vector' version )
 *        - @a SLT ( 'vector' version )
 *
 *  @version 1.1
 *  @date 19/02/2003
 *  @author Carlos Gonz�lez Rodr�guez - cgonzale@ac.upc.es
 */
class GPUMath {

private:
    /// prevents object creation
    GPUMath();

    /// prevents object copy
    GPUMath( const GPUMath& );

    ///  Flag that stores if the morton table is initialized.
    static bool mortonTableInitialized;

    ///  Table used to perform morton swizzling.
    static u8bit mortonTable[];

    /**
     *
     *  Initialized the table used to compute morton swizzling.
     *
     */

    static void buildMortonTable();

public:

    /**
     *  ABS (absolute value)
     *
     *  @param in A 32bit integer.
     *  @return The absolute value of the in parameter.
     */
    static u32bit ABS(const s32bit in);

    /**
     *  ABS (absolute value)
     *
     *  @param in A 32bit float point value.
     *  @return The absolute value of the in parameter.
     */
    static f32bit ABS(const f32bit in);

    /**
     * ABS ( absolute value )
     *
     * The ABS instruction performs a component-wise absolute value operation on
     * the single operand to yield a result vector.
     *
     * @param vin 4-component vector
     * @param result same as vin but with ABS computed for each component
     *
     * @remark Special-case rules
     *    - 1. ABS(NaN) = NaN
     *    - 2. ABS(-INF) = ABS(+INF) = +INF
     *    - 3. ABS(-0.0) = ABS(+0.0) = 0.0
     *
     * @warning Current implementation don't use this rules, simply does ABS operation
     */
    static void ABS( const f32bit* vin, f32bit* result );

    /**
     * ABS ( absolute value )
     *
     * The ABS instruction performs a component-wise absolute value operation on
     * the single operand to yield a result vector.
     *
     * @param vinout 4-component vector, it is an in/out variable
     *
     * @remark Special-case rules
     *    - 1. ABS(NaN) = NaN
     *    - 2. ABS(-INF) = ABS(+INF) = +INF
     *    - 3. ABS(-0.0) = ABS(+0.0) = 0.0
     *
     * @note ABS(v) has exacly the same behaviour than ABS(v,v)
     * @warning Current implementation don't use this rules, simply does ABS operation
     */
    static void ABS( f32bit* vinout );

    /**
     * ADD ( component by component version, result as a vector )
     *
     * @param x1 first component of first vector
     * @param y1 second component of first vector
     * @param z1 third component of first vector
     * @param w1 fourth component of first vector
     * @param x2 first component of second vector
     * @param y2 second component of second vector
     * @param z2 third component of second vector
     * @param w2 fourth component of second vector
     *
     * @retval result 4-component result vector
     *
     * @remark Special-case rules:
     *    - 1. "A+B" is always equivalent to "B+A".
     *    - 2. NaN + \<x> = NaN, for all \<x>.
     *    - 3. +INF + \<x> = +INF, for all \<x> except NaN and -INF.
     *    - 4. -INF + \<x> = -INF, for all \<x> except NaN and +INF.
     *    - 5. +INF + -INF = NaN.
     *    - 6. -0.0 + \<x> = \<x>, for all \<x>.
     *    - 7. +0.0 + \<x> = \<x>, for all \<x> except -0.0.
     *
     * @warning Current implementation don't use this rules, simply does ADD operation
     */
    static void ADD( const f32bit x1, const f32bit y1, const f32bit z1, const f32bit w1,
        const f32bit x2, const f32bit y2, const f32bit z2, const f32bit w2,
        f32bit* result );


    /**
     * ADD ( vector version )
     *
     * The ADD instruction performs a component-wise add of the two operands ( vectors ) to
     * yield a result vector.
     *
     * @param v1 first 4-component vector
     * @param v2 second 4-component vector
     *
     * @retval result 4-component result vector
     *
     * @remark Special-case rules:
     *    - 1. "A+B" is always equivalent to "B+A".
     *    - 2. NaN + \<x> = NaN, for all \<x>.
     *    - 3. +INF + \<x> = +INF, for all \<x> except NaN and -INF.
     *    - 4. -INF + \<x> = -INF, for all \<x> except NaN and +INF.
     *    - 5. +INF + -INF = NaN.
     *    - 6. -0.0 + \<x> = \<x>, for all \<x>.
     *    - 7. +0.0 + \<x> = \<x>, for all \<x> except -0.0.
     *
     * @warning Current implementation don't use this rules, simply does ADD operation
     */
    static void ADD( const f32bit* v1, const f32bit* v2, f32bit* result );


    /**
     * CMP ( vector version )
     *
     * The CMP performs a component wise comparision of the first operand against zero, and
     * copies the values of the second or third operands based on the results of the compare.
     *
     * @param v1 first 4-component vector
     * @param v2 second 4-component vector
     * @param v3 third 4-component vector
     *
     * @retval result 4-component result vector
     * @return CMP computed
     */
    static void CMP( const f32bit* v1, const f32bit* v2, const f32bit* v3, f32bit* result );

    /**
     * DP3 ( vector version )
     *
     * The DP3 instruction computes a three component dot product of the two
     * operands (using the x, y, and z components) and replicates the dot product
     * to all four components of the result vector.
     *
     * @param v1 first 4-component vector ( or 3-component vector )
     * @param v2 second 4-component vector ( or 3-component vector )
     *
     * @retval result 4-component result vector with DP3 value replicated in all 4 components
     * @return DP3 computed
     */
    static f32bit DP3( const f32bit* v1, const f32bit* v2, f32bit* result );


    /**
     * DP3 ( component by component version )
     *
     * @param x1 first component of first vector
     * @param y1 second component of first vector
     * @param z1 third component of first vector
     * @param x2 first component of second vector
     * @param y2 second component of second vector
     * @param z2 third component of second vector
     *
     * @return DP3 value computed
     */
    static f32bit DP3( const f32bit x1, const f32bit y1, const f32bit z1,
        const f32bit x2, const f32bit y2, const f32bit z2 );


    /**
     * ARL
     *
     * Loads the floor(in) into the address register, in its first component.
     *
     * @param in a 32-bit float point value that is going to be loaded into the integer register
     *
     * @retval addressRegister 4-component vector where we are going to write its first component
     * @return value of component of in specified by component parameter
     */
    static void ARL( s32bit* addressRegister, f32bit* in );

    /**
     * DP4 ( vector version )
     *
     * The DP4 instruction computes a four component dot product of the two
     * operands and replicates the dot product to all four components of the
     * result vector
     *
     * @param v1 first 4-component vector
     * @param v2 second 4-component vector
     *
     * @retval result 4-component result vector with DP4 value replicated in all 4 components
     * @return DP4 result
     */
    static f32bit DP4( const f32bit* v1, const f32bit* v2, f32bit* result );


    /**
     * DP4 ( component by component version )
     *
     * @param x1 first component of first vector
     * @param y1 second component of first vector
     * @param z1 third component of first vector
     * @param w1 fourth component of first vector
     * @param x2 first component of second vector
     * @param y2 second component of second vector
     * @param z2 third component of second vector
     * @param w2 fourth component of second vector
     *
     * @return DP4 result
     */
    static f32bit DP4( const f32bit x1, const f32bit y1, const f32bit z1, const f32bit w1,
        const f32bit x2, const f32bit y2, const f32bit z2, const f32bit w2 );


    /**
     * DPH ( homogeneous DP4, vector version )
     *
     * The DPH instruction computes a four-component dot product of the two
     * operands, except that the W component of the first operand is assumed to
     * be 1.0. The instruction replicates the dot product to all four components
     * of the result vector.
     *
     * @param v1 first 4-component vector
     * @param v2 second 4-component vector
     *
     * @retval result 4-component result, DPH result replicated in all 4 components
     * @return DPH result
     */
    static f32bit DPH( const f32bit* v1, const f32bit* v2, f32bit* result );


    /**
     * DPH ( homogeneous DP4, component by component version )
     *
     * @param x1 first component of first vector
     * @param y1 second component of first vector
     * @param z1 third component of first vector
     * @param w1 fourth component of first vector
     * @param x2 first component of second vector
     * @param y2 second component of second vector
     * @param z2 third component of second vector
     * @param w2 fourth component of second vector
     *
     * @return DPH result
     */
    static f32bit DPH( const f32bit x1, const f32bit y1, const f32bit z1, const f32bit w1,
        const f32bit x2, const f32bit y2, const f32bit z2, const f32bit w2 );


    /**
     * DST ( Distant vector )
     *
     * The DST instruction computes a distance vector from two specially-
     * formatted operands. The first operand should be of the form [NA, d^2,
     * d^2, NA] and the second operand should be of the form [NA, 1/d, NA, 1/d],
     * where NA values are not relevant to the calculation and d is a vector
     * length. If both vectors satisfy these conditions, the result vector will
     * be of the form [1.0, d, d^2, 1/d].
     *
     * @param v1 first 4-component vector
     * @param v2 second 4-component vector
     *
     * @retval result 4-component result
     */
    static void DST( const f32bit* v1, const f32bit* v2, f32bit* result );


    /**
     * MAD ( Multiply and Add ) ( vector version )
     *
     * The MAD instruction performs a component-wise multiply of the first two
     * operands, and then does a component-wise add of the product to the third
     * operand to yield a result vector
     *
     * @param v1 first 4-component vector
     * @param v2 second 4-component vector
     * @param v3 third 4-component vector
     *
     * @retval result 4-component result with MAD vector computed
     */
    static void MAD( const f32bit* v1, const f32bit* v2, const f32bit* v3, f32bit* result );


    /**
     * MAD ( Multiply and Add ) ( component by component version with vector result )
     *
     * @param x1 first component of first vector
     * @param y1 second component of first vector
     * @param z1 third component of first vector
     * @param w1 fourth component of first vector
     * @param x2 first component of second vector
     * @param y2 second component of second vector
     * @param z2 third component of second vector
     * @param w2 fourth component of second vector
     * @param x3 first component of third vector
     * @param y3 second component of third vector
     * @param z3 third component of third vector
     * @param w3 fourth component of third vector
     *
     * @retval result 4-component result with MAD vector computed
     */
    static void MAD( const f32bit x1, const f32bit y1, const f32bit z1, const f32bit w1,
        const f32bit x2, const f32bit y2, const f32bit z2, const f32bit w2,
        const f32bit x3, const f32bit y3, const f32bit z3, const f32bit w3,
        f32bit* result );


    /**
     * MAX ( Maximum ) ( vector version )
     *
     * The MAX instruction computes component-wise maximums of the values in the
     * two operands to yield a result vector
     *
     * @param v1 first 4-component vector
     * @param v2 second 4-component vector
     *
     * @retval result 4-component result with MAX vector computed
     *
     * @remark Special-case rules:
     *    - 1. MAX(A,B) is always equivalent to MAX(B,A).
     *    - 2. MAX(NaN,\<x>) = NaN, for all \<x>
     *
     * @warning Current implementation doesn't use this rules, simply does MAX operation
     */
    static void MAX( const f32bit* v1, const f32bit* v2, f32bit* result );


    /**
     * MAX ( Maximum ) ( component by component version  with vector result )
     *
     * @param x1 first component of first vector
     * @param y1 second component of first vector
     * @param z1 third component of first vector
     * @param w1 fourth component of first vector
     * @param x2 first component of second vector
     * @param y2 second component of second vector
     * @param z2 third component of second vector
     * @param w2 fourth component of second vector
     *
     * @retval result 4-component result with MAX vector computed
     *
     * @remark Special-case rules:
     *    - 1. MAX(A,B) is always equivalent to MAX(B,A).
     *    - 2. MAX(NaN,\<x>) = NaN, for all \<x>
     *
     * @warning Current implementation doesn't use this rules, simply does MAX operation
     *
     */
    static void MAX( const f32bit x1, const f32bit y1, const f32bit z1, const f32bit w1,
        const f32bit x2, const f32bit y2, const f32bit z2, const f32bit w2,
        f32bit* result );

    /**
     * MIN ( Minimum ) ( vector version )
     *
     * The MIN instruction computes component-wise minimums of the values in the
     * two operands to yield a result vector.
     *
     * @param v1 first 4-component vector
     * @param v2 second 4-component vector
     *
     * @retval result 4-component result with MIN vector computed
     *
     * @remark Special-case rules:
     *    - 1. MIN(A,B) is always equivalent to MIN(B,A).
     *    - 2. MIN(NaN,\<x>) = NaN, for all \<x>
     *
     * @warning Current implementation doesn't use this rules, simply does MIN operation
     */
    static void MIN( const f32bit* v1, const f32bit* v2, f32bit* result );



    /**
     * MIN ( Minimum ) ( component by component version  with vector result )
     *
     * @param x1 first component of first vector
     * @param y1 second component of first vector
     * @param z1 third component of first vector
     * @param w1 fourth component of first vector
     * @param x2 first component of second vector
     * @param y2 second component of second vector
     * @param z2 third component of second vector
     * @param w2 fourth component of second vector
     *
     * @retval result 4-component result with MIN vector computed
     *
     * @remark Special-case rules:
     *    - 1. MIN(A,B) is always equivalent to MIN(B,A).
     *    - 2. MIN(NaN,\<x>) = NaN, for all \<x>
     *
     * @warning Current implementation doesn't use this rules, simply does MIN operation
     */
    static void MIN( const f32bit x1, const f32bit y1, const f32bit z1, const f32bit w1,
        const f32bit x2, const f32bit y2, const f32bit z2, const f32bit w2,
        f32bit* result );

    /**
     * MOV ( Move )
     *
     * Moves ( copy ) the value of the source vector into the destination register
     *
     * @param source source register
     * @retval destination destination register
     */
    static void MOV( f32bit* source, f32bit* destination );


    /**
     * MUL ( vector version )
     *
     * The MUL instruction performs a component-wise multiply of the two operands
     * to yield a result vector
     *
     * @param v1 first 4-component vector
     * @param v2 second 4-component vector
     *
     * @retval result 4-component result with MUL vector computed
     *
     * @remark Special-case rules:
     *    - 1. "A*B" is always equivalent to "B*A".
     *    - 2. NaN * \<x> = NaN, for all \<x>.
     *    - 3. +/-0.0 * +/-INF = NaN.
     *    - 4. +/-0.0 * \<x> = +/-0.0, for all \<x> except -INF, +INF, and NaN. The
     *       sign of the result is positive if the signs of the two operands match
     *       and negative otherwise.
     *    - 5. +/-INF * \<x> = +/-INF, for all \<x> except -0.0, +0.0, and NaN. The
     *       sign of the result is positive if the signs of the two operands match
     *       and negative otherwise.
     *    - 6. +1.0 * \<x> = \<x>, for all \<x>.
     *
     * @warning Current implementation don't use this rules, simply do MUL operation
     */
    static void MUL( const f32bit* v1, const f32bit* v2, f32bit* result );


    /**
     * MUL ( component by component version, result as a vector )
     *
     * @param x1 first component of first vector
     * @param y1 second component of first vector
     * @param z1 third component of first vector
     * @param w1 fourth component of first vector
     * @param x2 first component of second vector
     * @param y2 second component of second vector
     * @param z2 third component of second vector
     * @param w2 fourth component of second vector
     *
     * @retval result 4-component result with MIN vector computed
     *
     * @remark Special-case rules:
     *    - 1. "A*B" is always equivalent to "B*A".
     *    - 2. NaN * \<x> = NaN, for all \<x>.
     *    - 3. +/-0.0 * +/-INF = NaN.
     *    - 4. +/-0.0 * \<x> = +/-0.0, for all \<x> except -INF, +INF, and NaN. The
     *       sign of the result is positive if the signs of the two operands match
     *       and negative otherwise.
     *    - 5. +/-INF * \<x> = +/-INF, for all \<x> except -0.0, +0.0, and NaN. The
     *       sign of the result is positive if the signs of the two operands match
     *       and negative otherwise.
     *    - 6. +1.0 * \<x> = \<x>, for all \<x>.
     *
     * @warning Current implementation don't use this rules, simply do MUL operation
     */
    static void MUL( const f32bit x1, const f32bit y1, const f32bit z1, const f32bit w1,
        const f32bit x2, const f32bit y2, const f32bit z2, const f32bit w2,
        f32bit* result );

    /**
     * RCP ( Reciprocal )
     *
     * The RCP instruction approximates the reciprocal of the scalar operand and
     * replicates it to all four components of the result vector.
     *
     * @param vin  32 bit float input value
     *
     * @retval vout 4-component vector with all four components containing reciprocal value
     * @return reciprocal value of specified component
     *
     * @remark Special-case rules:
     *    - 1. ApproxReciprocal(NaN) = NaN.
     *    - 2. ApproxReciprocal(+INF) = +0.0.
     *    - 3. ApproxReciprocal(-INF) = -0.0.
     *    - 4. ApproxReciprocal(+0.0) = +INF.
     *    - 5. ApproxReciprocal(-0.0) = -INF.
     *
     * @code
     *    // Example of RCP
     *
     *    f32bit vin[4] = { 3.0, -2.0, 7.75, 9.5 };
     *    f32bit vout[4] = { 0, 0, 0, 0 };
     *    f32bit result;
     *
     *    result = GPUMath::RCP( vin, vout, 1 ); // second component selected ( y component )
     *
     *    // vout = { -0.5, -0.5, -0.5, -0.5 };
     *    // result = -0.5
     * @endcode
     */
    static f32bit RCP( const f32bit vin, f32bit* vout );

    /**
     * RCP ( Reciprocal scalar version )
     *
     * The RCP instruction approximates the reciprocal of the scalar operand.
     *
     * @param value value we are going to calculate reciprocal
     * @return reciprocal of value
     *
     * @remark Special-case rules:
     *    - 1. ApproxReciprocal(NaN) = NaN.
     *    - 2. ApproxReciprocal(+INF) = +0.0.
     *    - 3. ApproxReciprocal(-INF) = -0.0.
     *    - 4. ApproxReciprocal(+0.0) = +INF.
     *    - 5. ApproxReciprocal(-0.0) = -INF.
     *
     * @code
     *   // Example of RCP ( scalar version )
     *   f32bit rcp;
     *   rcp = GPUMath::RCP( 2.0 );
     *   // rcp = 0.5
     * @endcode
     */
    static f32bit RCP( f32bit value );

    /**
     * RSQ ( Reciprocal Square Root )
     *
     * Computes the inverse square root of the absolute value of the vout[component]
     * and replicates the result across the destination register
     *
     * @param vin 32 bit float input
     * @retval vout 4-component vector with vin[component] inverse square root replicated
     *         in all components
     */
    static f32bit RSQ( const f32bit vin, f32bit* vout );

    /**
     * RSQ ( Reciprocal Square Root scalar version )
     *
     * Computes the inverse square root of the absolute value of the source scalar
     *
     * @param value value we are going to calculate reciprocal square root
     * @return reciprocal square root of 'value'
     */
    static f32bit RSQ( f32bit value );

    /**
     * FRC ( fraction )
     *
     * The FRC instruction extracts the fractional portion of each component of
     * the operand to generate a result vector. The fractional portion of a
     * component is defined as the result after subtracting off the floor of the
     * component and is always in the range [0.00, 1.00).
     *
     * @note
     *    For negative values, the fractional portion is NOT the number written to
     *    the right of the decimal point -- the fractional portion of -1.7 is not
     *    0.7 -- it is 0.3. 0.3 is produced by subtracting the floor of -1.7 (-2.0)
     *    from -1.7.
     *
     * @param vin 4-component in vector
     * @retval vout 4-component vector containing the computation of FRC
     */
    static void FRC( f32bit* vin, f32bit* vout );

    /**
     * SAT ( Saturate )
     *
     * Clamps to the [0.0f, 1.0f] range the componets of the input 4-component vector.
     *
     * @param vin1 4-component vector that is first source
     * @retval vout 4-component vector with the clamped components of the first source vector.
     */
    static void SAT( f32bit* vin1, f32bit* vout );

    /**
     * SLT ( Set On Less Than )
     *
     * Performs a component-wise assignment of either 1.0 or 0.0. 1.0 is assigned if
     * the value of the first source is less than the value of the second.
     * Otherwise, 0.0 is assigned.
     *
     * @param vin1 4-component vector that is first source
     * @param vin2 4-component vector that is second source
     * @retval vout 4-component vector with the result of component-wise assigment
     */
    static void SLT( f32bit* vin1, f32bit* vin2, f32bit* vout );


    /**
     * SGE ( Set On Greater Than or Equal Than )
     *
     * Performs a component-wise assignment of either 1.0 or 0.0. 1.0 is assigned if
     * the value of the first source is greater than or equal the value of the second.
     * Otherwise, 0.0 is assigned.
     *
     * @param vin1 4-component vector that is first source
     * @param vin2 4-component vector that is second source
     * @retval vout 4-component vector with the result of component-wise assigment
     */
    static void SGE( f32bit* vin1, f32bit* vin2, f32bit* vout );

    /**
     * EXP ( Exponential Base 2 )
     *
     * Generates an approximation of 2^vin for vin.
     * Also generates intermediate terms that can be used to compute a more accurate
     * result using additional instructions.
     *
     * @param vin float with source value
     * @retval vout 4-component vector with some values computed. vout[0] contains
     *         2^floor(vin[component]), vout[1] contains vin[component] - floor(vin[component]),
     *         vout[2] contains a rough approximation of 2^vin[component] and vout[3] contains 1
     *
     */
    static void EXP( f32bit vin, f32bit* vout );

    /**
     * EX2 ( Exponential Base 2 )
     *
     * Generates an approximation of 2^vin for vin.

     *
     * @param vin float with source value
     * @retval vout 4-component vector with replicated rough approximation of 2^vin
     * in all its components.
     *
     */
    static void EX2( f32bit vin, f32bit* vout );

    /**
     * LOG ( Logarithm Base 2 )
     *
     * Generates an approximation of log2(|vin|) for vin.
     * Also generates intermediate terms that can be used to compute a more accurate
     * result using additional instructions.
     *
     * @param vin 32 bit float with source value
     * @retval vout 4-component vector with some values computed. vout[0] contains Exponent(vin[0])
     *         in range [-126,127], vout[1] contains Mantisa(vin[0]) in range [1.0..2.0),
     *         vout[2] contains a rough approximation of log2(|s|) and vout[3] contains 1
     */
    static void LOG( f32bit vin, f32bit* vout );

    /**
     * LG2 ( Logarithm Base 2 )
     *
     * Generates an approximation of log2(|vin|) for vin.
     *
     *
     * @param vin float with source value
     * @retval vout 4-component vector with replicated rough approximation of 2^vin
     * in all its components.
     *
     */
    static void LG2( f32bit vin, f32bit* vout );

    /**
     * LIT
     *
     * Computes ambient, diffuse, and specular lighting coefficients from a diffuse dot product,
     * a specular dot product, and a specular power
     *
     * @param vin 4-component vector, it is assumed that vin[0] contains a difuse dot product (N*L),
     *        vin[1] contains a specular dot product (N*H) and vin[3] contains a power (m)
     * @retval vout 4-component vector containing ambient coefficient in vout[0] ( 1.0 ),
     *         diffuse coefficient in vout[1], specular coefficient in vout[2] and 1.0 in vout[3]
     */
    static void LIT( f32bit* vin, f32bit* vout );

    /**
     *
     *  SETPEQ
     *
     *  Computes the equal comparison between two input 32-bit float values.
     *
     *          pred = (a == b)
     *
     *  @param a First value to compare
     *  @param b Second value to compare
     *  @param out Reference to a boolean variable where to store the result of the comparison.
     *
     */
     
    static void SETPEQ(f32bit a, f32bit b, bool &out);
    
    /**
     *
     *  SETPGT
     *
     *  Computes the greater than comparison between two input 32-bit float values.
     *
     *          pred = (a > b)
     *
     *  @param a First value to compare
     *  @param b Second value to compare
     *  @param out Reference to a boolean variable where to store the result of the comparison.
     *
     */
     
    static void SETPGT(f32bit a, f32bit b, bool &out);

    /**
     *
     *  SETPLT
     *
     *  Computes the less than comparison between two input 32-bit float values.
     *
     *          pred = (a < b)
     *
     *  @param a First value to compare
     *  @param b Second value to compare
     *  @param out Reference to a boolean variable where to store the result of the comparison.
     *
     */
     
    static void SETPLT(f32bit a, f32bit b, bool &out);
     
    /**
     *
     *  ANDP
     *
     *  Predicate operator.  Computes the AND between the two input predicates.
     *
     *          predOut = predA && predB
     *
     *  @param a First value to compare
     *  @param b Second value to compare
     *  @param out Reference to a boolean variable where to store the result of the comparison.
     *
     */
     
    static void ANDP(bool a, bool b, bool &out);

    /**
     * COS ( Cosinus )
     *
     * Computes the cosinus of vin.

     *
     * @param vin float with source value
     * @retval vout 4-component vector with replicated cosinus of vin
     *
     */
    static void COS( f32bit vin, f32bit* vout );
     
    /**
     * SIN ( Sinus )
     *
     * Computes the sinus of vin.

     *
     * @param vin float with source value
     * @retval vout 4-component vector with replicated sinus of vin
     *
     */
    static void SIN( f32bit vin, f32bit* vout );


    /**
     *
     *  Computes the derivatives in x for 4 fragments/samples arranged as a 2x2 quad in the defined order:
     *
     *        0 1 
     *        2 3
     *
     *  Derivative for fragment/sample 0 is computed as input[1] - input[0]
     *  Derivative for fragment/sample 1 is computed as input[1] - input[0]
     *  Derivative for fragment/sample 2 is computed as input[3] - input[2]
     *  Derivative for fragment/sample 3 is computed as input[3] - input[2]
     *
     *  @param input Pointer to an array of QuadFloat variables storing the input values for the derivation
     *  @param derivates Pointer to an array of QuadFloat variables where to store the deriveated values.
     *
     */

    static void derivX(QuadFloat *input, QuadFloat *derivatives);    
     
    /**
     *
     *  Computes the derivatives in y for 4 fragments/samples arranged as a 2x2 quad in the defined order:
     *
     *        0 1 
     *        2 3
     *
     *  Derivative for fragment/sample 0 is computed as input[2] - input[0]
     *  Derivative for fragment/sample 1 is computed as input[3] - input[1]
     *  Derivative for fragment/sample 2 is computed as input[2] - input[0]
     *  Derivative for fragment/sample 3 is computed as input[3] - input[1]
     *
     *  @param input Pointer to an array of QuadFloat variables storing the input values for the derivation
     *  @param derivates Pointer to an array of QuadFloat variables where to store the deriveated values.
     *
     */

    static void derivY(QuadFloat *input, QuadFloat *derivatives);    

    /**
     *  ADDI ( vector version )
     *
     *  The ADDI instruction performs a component-wise integer add of the two operands (vectors) to
     *  yield a result vector.
     *
     *  @param v1 first 4-component vector
     *  @param v2 second 4-component vector
     *
     *  @retval result 4-component result vector
     *
     */
     
    static void ADDI(const s32bit* v1, const s32bit* v2, s32bit* result);

    /**
     *  MULI ( vector version )
     *
     *  The MULI instruction performs a component-wise integer multiplication of the two operands (vectors) to
     *  yield a result vector.
     *
     *  @param v1 first 4-component vector
     *  @param v2 second 4-component vector
     *
     *  @retval result 4-component result vector
     *
     */
     
    static void MULI(const s32bit* v1, const s32bit* v2, s32bit* result);

    /**
     *
     *  STPEQI
     *
     *  Computes the equal comparison between two input 32-bit integer values.
     *
     *          pred = (a == b)
     *
     *  @param a First value to compare
     *  @param b Second value to compare
     *  @param out Reference to a boolean variable where to store the result of the comparison.
     *
     */
     
    static void STPEQI(s32bit a, s32bit b, bool &out);
    
    /**
     *
     *  STPGTI
     *
     *  Computes the greater than comparison between two input 32-bit integer values.
     *
     *          pred = (a > b)
     *
     *  @param a First value to compare
     *  @param b Second value to compare
     *  @param out Reference to a boolean variable where to store the result of the comparison.
     *
     */
     
    static void STPGTI(s32bit a, s32bit b, bool &out);

    /**
     *
     *  STPLTI
     *
     *  Computes the less than comparison between two input 32-bit integer values.
     *
     *          pred = (a < b)
     *
     *  @param a First value to compare
     *  @param b Second value to compare
     *  @param out Reference to a boolean variable where to store the result of the comparison.
     *
     */
     
    static void STPLTI(s32bit a, s32bit b, bool &out);

    /**
     *
     *  Calculates the adjoint matrix from the three vertices in a triangle
     *  to create the three edge equations for the triangle.
     *
     *  @param vertex1 First triangle vertex.
     *  @param vertex2 Second triangle vertex.
     *  @param vertex3 Third triangle vertex.
     *  @param edge1 Pointer to the f64bit array were the first edge
     *  equation will be stored.
     *  @param edge2 Pointer to the f64bit array were the second edge
     *  equation will be stored.
     *  @param edge3 Pointer to the f64bit array were the third edge
     *  equation will be stored.
     *
     */

    static void setupMatrix(QuadFloat vertex1, QuadFloat vertex2,
        QuadFloat vertex3, f64bit *edge1, f64bit *edge2, f64bit *edge3);

    /**
     *
     *  Adjusts the triangle edge and z interpolation equations to the viewport.
     *
     *  @param vertex1 First triangle vertex.
     *  @param vertex2 Second triangle vertex.
     *  @param vertex3 Third triangle vertex.
     *  @param edge1 Pointer to the first edge equation.
     *  @param edge2 Pointer to the second edge equation.
     *  @param edge3 Pointer to the third edge equation.
     *  @param zeq Pointer to the z interpolation equation.
     *  @param x0 Viewport start x coordinate.
     *  @param y0 Viewport start y coordinate.
     *  @param w Viewport width.
     *  @param h Viewport height.
     *
     *  @return The vertex matrix determinant (signed triangle area
     *  approximation).
     *
     */

    static f64bit viewport(QuadFloat vertex1, QuadFloat vertex2,
        QuadFloat vertex3, f64bit *edge1, f64bit *edge2, f64bit *edge3,
        f64bit *zeq, s32bit x0, s32bit y0, u32bit w, u32bit h);


    /**
     *
     *  Calculates the bounding box for a triangle defined by three
     *  vertices in homogeneous coordinates.
     *
     *  @param v1 Triangle first vertex.
     *  @param v2 Triangle second vertex.
     *  @param v3 Triangle third vertex.
     *  @param x0 Viewport start x coordinate.
     *  @param y0 Viewport start y coordinate.
     *  @param w Viewport width.
     *  @param h Viewport height.
     *  @param xMin Reference to the variable where to store
     *  the minimun horizontal value of the bounding box.
     *  @param xMax Reference to the variable where to store
     *  the maximum horizontal value of the bounding box.
     *  @param yMin Reference to the variable where to store
     *  the minimun vertical value of the bounding box.
     *  @param yMax Reference to the variable where to store
     *  the maximum vertical value of the bounding box.
     *  @param zMin Reference to the variable where to store
     *  the minimun depth value of the bounding box.
     *  @param zMax Reference to the variable where to store
     *  the maximum depth value of the bounding box.
     *
     */

    static void boundingBox(QuadFloat v1, QuadFloat v2, QuadFloat v3,
        s32bit x0, s32bit y0, u32bit w, u32bit h, s32bit &xMin, s32bit &xMax,
        s32bit &yMin, s32bit &yMax, s32bit &zMin, s32bit &zMax);

    /**
     *
     *  Calculates the subpixel bounding box for a triangle defined by three
     *  vertices in homogeneous coordinates using the fixed-point defined precision.
     *
     *  @param v1 Triangle first vertex.
     *  @param v2 Triangle second vertex.
     *  @param v3 Triangle third vertex.
     *  @param x0 Viewport start x coordinate.
     *  @param y0 Viewport start y coordinate.
     *  @param w Viewport width.
     *  @param h Viewport height.
     *  @param xMin Reference to the fixed point variable where to store
     *  the minimun horizontal subpixel value of the bounding box.
     *  @param xMax Reference to the fixed point variable where to store
     *  the maximum horizontal subpixel value of the bounding box.
     *  @param yMin Reference to the fixed point variable where to store
     *  the minimun vertical subpixel value of the bounding box.
     *  @param yMax Reference to the fixed point variable where to store
     *  the maximum vertical subpixel value of the bounding box.
     *  @param zMin Reference to the variable where to store
     *  the minimun depth value of the bounding box.
     *  @param zMax Reference to the variable where to store
     *  the maximum depth value of the bounding box.
     *  @param subPixelPrecision The number of bits to represent
     *  the decimal part of this fixed-point output variables.
     *
     */
    static void subPixelBoundingBox(QuadFloat v1, QuadFloat v2, QuadFloat v3,
        s32bit x0, s32bit y0, u32bit width, u32bit height, FixedPoint &xMin, FixedPoint &xMax,
        FixedPoint &yMin, FixedPoint &yMax, s32bit &zMin, s32bit &zMax, u32bit subPixelPrecision);

    /**
     *
     *  Calculates the start position in rasterization window coordinates
     *  for the triangle defined by the three edge equations.
     *
     *  @param edge1 Pointer to the first edge equation.
     *  @param edge2 Pointer to the second edge equation.
     *  @param edge3 Pointer to the third edge equation.
     *  @param x0 Viewport start x coordinate.
     *  @param y0 Viewport start y coordinate.
     *  @param w Viewport width.
     *  @param h Viewport height.
     *  @param scissorX0 Scissor box start x coordinate.
     *  @param scissorY0 Scissor box start y coordinate.
     *  @param scissorW Scissor box width.
     *  @param scissorH Scissor box height.
     *  @param boxMinX Triangle bounding box minimum x coordinate.
     *  @param boxMinY Triangle bounding box minimum y coordinate.
     *  @param boxMaxX Triangle bounding box maximum x coordinate.
     *  @param boxMaxY Triangle bounding box maximum y coordinate.
     *  @param startX Reference to the variable where to store
     *  the start rasterization horizontal position for the triangle.
     *  @param startY Reference to the variable where to store
     *  the start vertical rasterization vertical position for the triangle.
     *  @param dir Reference to the variable where to store the
     *  start rasterization direction.
     *
     */

    static void startPosition(QuadFloat v1, QuadFloat v2, QuadFloat v3,
    f64bit *edge1, f64bit *edge2, f64bit *edge3, s32bit x0, s32bit y0, u32bit w, u32bit h,
    s32bit scissorX0, s32bit scissorY0, u32bit scissorW, u32bit scissorH,
    s32bit boxMinX, s32bit boxMinY, s32bit boxMaxX, s32bit boxMaxY, s32bit &startX, s32bit &startY,
    RasterDirection &dir);

    /**
     *
     *  Evaluates a 2D point against a edge equations.
     *
     *  @param e Pointer to a triangle edge equation.
     *  @param p Pointer to a f64bit array with the point coordinates.
     *
     *  return The value of the edge equation at the given point.
     *
     */

    static f64bit evaluateEquation(f64bit *e, f64bit *p);

    /**
     *
     *  Tests if a 2D point is inside a triangle (front facing
     *  only) using its edge equations.
     *
     *  @param e1 Pointer to a triangle edge equation.
     *  @param e1 Pointer to a triangle edge equation.
     *  @param e1 Pointer to a triangle edge equation.
     *  @param p Pointer to a f64bit array with the point coordinates.
     *
     *  @return TRUE if the point is inside the triangle, FALSE otherwise.
     *
     */

    static bool isInside(f64bit *e1, f64bit *e2, f64bit *e3, f64bit *p);

    /**
     *  Creates the interpolation equation for a triangle per vertex
     *  parameter.
     *
     *  @param edge1 Pointer to the first triangle edge equation (aka
     *  a rasterization matrix row).
     *  @param edge2 Pointer to the second triangle edge equation (aka
     *  a rasterization matrix row).
     *  @param edge3 Pointer to the third triangle edge equation (aka
     *  a rasterization matrix row).
     *  @param u0 Parameter value in first triangle vertex.
     *  @param u1 Parameter value in second triangle vertex.
     *  @param u2 Parameter value in third triangle vertex.
     *  @param intEq Pointer to the array where to store the calculated
     *  interpolation equation coefficients for the parameter.
     *
     */

    static void interpolationEquation(f64bit *edge1, f64bit *edge2, f64bit *edge3,
        f32bit u0, f32bit u1, f32bit u2, f64bit *intEq);

    /**
     *
     *  Computes the triangle size in percentage of covered screen area. The result of multiplying
     *  this number by the screen area in pixels is the triangle area in pixels.
     *
     *  In addition, the non-homogeneous position coordinates of the three vertices used
     *  for the computation are returned. The forth component is also the 1/w computed value 
     *  for each vertex.
     *
     *  @param vPos1 Pointer to the vertex1 position attribute coordinates.
     *  @param vPos2 Pointer to the vertex2 position attribute coordinates.
     *  @param vPos3 Pointer to the vertex3 position attribute coordinates.
     *  @param nH_vPos1 returns the non-homogeneous screen position of vertex 1 (fourth component has 1/vtx1.w).
     *  @param nH_vPos2 returns the non-homogeneous screen position of vertex 2 (fourth component has 1/vtx2.w).
     *  @param nH_vPos3 returns the non-homogeneous screen position of vertex 3 (fourth component has 1/vtx3.w).
     *  @return The triangle size in percentage of screen area.
     *
     */

    static f64bit computeTriangleScreenArea(QuadFloat vPos1, QuadFloat vPos2, QuadFloat vPos3, QuadFloat& nH_vPos1, QuadFloat& nH_vPos2, QuadFloat& nH_vPos3);

    /**
     *
     *  Vector addition of 2 4-component 64bit float point vectors.
     *
     *  @param res Pointer to the 4-component 64bit float point vector where
     *  to store the result of the addition.
     *  @param a Pointer to the first 4-component 64bit float point input
     *  vector.
     *  @param b Pointer to the second 4-component 64bit float point input
     *  vector.
     *
     */

    static void ADD64(f64bit *res, f64bit *a, f64bit *b);

    /**
     *
     *  Evaluates a rasterization sample point against the triangle
     *  three edge equations.
     *
     *  @param sample Pointer to a three component 64bit float point
     *  vector with the values of the triangle three edge equations
     *  at the sample point.
     *
     *  @return The sample outcode against the three edge equation.
     *  Bits 0, 1 and 2 of the outcode will be '1' if the sample point
     *  is in the negative half plane of one of the respective three
     *  edge equations.
     *
     *
     */

    static u32bit evaluateSample(f64bit *sample);


    /**
     *
     *  Evaluates a tile defined by four rasterization sample points
     *  against the triangle edge equations.
     *
     *  @param s0 Pointer to a three component 64bit float point vector
     *  with the values for the three edge equations at the tile start
     *  sample point.
     *  @param s1 Pointer to a three component 64bit float point vector
     *  with the values for the three edge equations at the tile second
     *  sample point.
     *  @param s2 Pointer to a three component 64bit float point vector
     *  with the values for the three edge equations at the tile third
     *  sample point.
     *  @param s3 Pointer to a three component 64bit float point vector
     *  with the values for the three edge equations at the tile fourth
     *  sample point.
     *
     *  @return True if there may be triangle fragments inside the evaluated
     *  tile.  False otherwise.
     *
     */

    static bool evaluateTile(f64bit *s0, f64bit *s1, f64bit *s2, f64bit *s3);

    /**
     *
     *  Converts an input 32 bit unsigned integer value into a bit shift
     *  value that covers/removes/adds at least as many values as the
     *  input value.
     *
     *  @param n Input unsigned 32 bit value.
     *
     *  @return The shift for the input 32 bit value.
     *
     */

    static u32bit calculateShift(u32bit n);


    /**
     *
     *  Converts an input 32 bit unsigned integer value into a mask
     *  that covers at least as many values as the input value.
     *
     *  @param n Input unsigned 32 bit value.
     *
     *  @return The bit mask for the input 32 bit value.
     *
     */

    static u32bit buildMask(u32bit n);

    /**
     *  Translates a pixel address to a memory address in the framebuffer.
     *  Supports a tiled organization and pixel stamps.
     *
     *  @param x The pixel x in viewport coordinates.
     *  @param y The pixel y in viewport coordinates.
     *  @param x0 The viewport initial x coordinate.
     *  @param y0 The viewport initial y coordinate.
     *  @param w The viewport width (not used).
     *  @param h The viewport height (not used).
     *  @param xRes The device horizontal resolution.
     *  @param yRes The device vertical resolution.
     *  @param stampW The width in pixels of a stamp.
     *  @param stampH The height in pixels of a stamp.
     *  @param genTileW The generation tile width in stamps.
     *  @param genTileH The generation tile height in stamps.
     *  @param scanTileW The scan tile width in generation tiles.
     *  @param scanTileH The scan tile height in generation tiles.
     *  @param overTileW The over scan tile width in scan tiles.
     *  @param overTileH The over scan tile height in scan tiles.
     *  @param samples Number of samples per pixel.
     *  @param bytesSample Bytes per sample.
     *
     *  @return The address in the framebuffer of the pixel.
     *
     */

    static u32bit pixel2Memory(s32bit x, s32bit y, s32bit x0, s32bit y0, u32bit w, u32bit h,
        u32bit xRes, u32bit yRes, u32bit stampW, u32bit stampH, u32bit genW, u32bit genH,
        u32bit scanW, u32bit scanH, u32bit overW, u32bit overH, u32bit samples, u32bit bytesSample, bool log = false);

    /**
     *
     *  Calculates the address of an element in 2^size x 2^size matrix using
     *  Morton (Z) order.
     *
     *  @param size Logarithm of 2 of the size of the square matrix were
     *  the element is located.
     *  @param i The horizontal coordinate of the element in the square matrix.
     *  @param j The vertical coordinate of the element in the square matrix.
     *
     *  @return The linear address in Moron order of the element in the defined
     *  square matrix.
     *
     */

    static u32bit morton(u32bit size, u32bit i, u32bit j);

    /**
     *
     *  Converts a 16-bit float point value to a 32-bit float point value.
     *
     *  Denormalized float16 values are converted to normalized float32 values.
     *
     *  @param Input 16-bit float point value.
     *
     *  @return A 32-bit float point value representing the input 16-bit float point value.
     *
     */

    static f32bit convertFP16ToFP32(f16bit input);

    /**
     *
     *  Converts a 32-bit float point value to a 16-bit float point value.
     *
     *  Denormalized float32 values are flushed to 0.  Numbers with exponents that are too
     *  large for float16 are converted to infinites.  The rounding mode used is truncation.
     *
     *  @param Input 32-bit float point value.
     *
     *  @return A 16-bit float point value representing the input 32-bit float point value.
     *
     */

    static f16bit convertFP32ToFP16(f32bit input);


}; // class GPUMath

} // namespace gpu3d

#endif
