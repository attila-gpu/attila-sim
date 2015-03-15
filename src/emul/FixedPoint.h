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
 * $RCSfile:$
 * $Revision:$
 * $Author:$
 * $Date:$
 *
 * The Fixed Point class implements a fixed point type and basic arithmetic operations.
 *
 */

/**
 *
 *  @file FixedPoint.h
 *
 *  This file defines the Fixed Point class supporting fixed point data types and basic
 *  fixed point arithmetic operations.
 *
 */



#ifndef _FIXEDPOINT_

#define _FIXEDPOINT_

#include "support.h"
#include "GPUTypes.h"


/**
 *
 *  The Fixed Point class defines fixed point data types and supports basic arithmetic
 *  operations.
 *
 */

class FixedPoint
{
private:

    u32bit intBits;         /**<  Precission of the fixed point value: number of bits for the integral part of the value.  */
    u32bit fracBits;        /**<  Precission of the fixed point value: number of bits for the fractional part of the value.  */
    s8bit sign;             /**<  Sign of the fixed point value.  Stores -1 or 1 depending on the sign of the value.  */
    u64bit intVal;          /**<  Value for the integral part of the fixed point value.  Bits are stored from lower to to higher bits.
                                  Unused bits are set to zero.  */
    u64bit fracVal;         /**<  Value for the fractional part of the fixed point value.  Bits are stored from higher to lower bits.
                                  Unused bits are set to zero.  */

    bool nan;               /**<  The fixed point object was initialized with a NaN float point value or was computed based on a fixed
                                  point object with the NaN flag set.  */
    bool infinite;          /**<  The fixed point object was initialized with a NaN float point value or was computed based on a fixed
                                  point object with the infinite flag set.  */
    bool overflow;          /**<  The fixed point object was initialized from a float point value that exceded the precission of the
                                  fixed point object or was the result of a computation of two fixed point objects that exceded the
                                  precission of the fixed point object.  */
    bool underflow;         /**<  The fixed point object was initialized from a float point value that exceded the precission of the
                                  fixed point object or was the result of a computation of two fixed point objects that exceded the
                                  precission of the fixed point object.  */

public:

    /**
     *
     *  Defines the maximum number of bits that can be used to represent the integral part of
     *  fixed point values.
     *
     */

    static const u32bit MAX_INTEGER_BITS = 64;

    /**
     *
     *  Defines the maximum number of bits that can be used to represent the fractional part of
     *  fixed point values.
     *
     */

    static const u32bit MAX_FRACTIONAL_BITS = 63;

    /**
     *
     *  Fixed point constructor.
     *
     *  Creates a new fixed point object based on a 32-bit float point value.
     *
     *  @param value A 32-bit float point value that defines the initial value of
     *               the fixed point object.
     *  @param intBits Precission of the fixed point object: number of integer bits.
     *  @param fracBits Precission of the fixed point object: number of fractional bits.
     *
     *  @return A new initialized fixed point object.
     *
     */

    FixedPoint(f32bit val = 0, u32bit intBits = MAX_INTEGER_BITS, u32bit fracBits = MAX_FRACTIONAL_BITS);

    /**
     *
     *  Fixed point constructor.
     *
     *  Creates a new fixed point object based on a 64-bit float point value.
     *
     *  @param value A 64-bit float point value that defines the initial value of
     *               the fixed point object.
     *  @param intBits Precission of the fixed point object: number of integer bits.
     *  @param fracBits Precission of the fixed point object: number of fractional bits.
     *
     *  @return A new initialized fixed point object.
     *
     */

    FixedPoint(f64bit val, u32bit intBits = MAX_INTEGER_BITS, u32bit fracBits = MAX_FRACTIONAL_BITS);

    /**
     *
     *  Fixed point constructor.
     *
     *  Creates a new fixed point object based on two 64-bit integer values.
     *
     *  @param sing Stores the sign of the fixed point value: 1 -> positive, -1 -> negative.
     *  @param intVal A 64-bit integer value that defines the initial value of
     *               the integer part of the fixed point object.  The significative bits
     *               are stored in the lower bits of the 64-bit value.  The not used bits
     *               in the upper bits of the 64-bit word must be set to zero.
     *  @param fracVal A 64-bit integer value that defines the initial value of
     *               the fractional part of the fixed point object.  The significative bits
     *               are stored in the upper bits of the 64-bit value.  The not used bits
     *               in the lower bits of the 64-bit word must be set to zero.
     *  @param intBits Precission of the fixed point object: number of integer bits.
     *  @param fracBits Precission of the fixed point object: number of fractional bits.
     *
     *  @return A new initialized fixed point object.
     *
     */

    FixedPoint(s8bit sign, u64bit intVal, u64bit fracVal, u32bit intBits = MAX_INTEGER_BITS, u32bit fracBits = MAX_FRACTIONAL_BITS);

    /**
     *
     *  Fixed point constructor.
     *
     *  Creates a new fixed point object based on two 32-bit integer values.
     *
     *  @param sing Stores the sign of the fixed point value: 1 -> positive, -1 -> negative.
     *  @param intVal A 32-bit integer value that defines the initial value of
     *               the integer part of the fixed point object.  The significative bits
     *               are stored in the lower bits of the 32-bit value.  The not used bits
     *               in the upper bits of the 32-bit word must be set to zero.
     *  @param fracVal A 32-bit integer value that defines the initial value of
     *               the fractional part of the fixed point object.  The significative bits
     *               are stored in the upper bits of the 32-bit value.  The not used bits
     *               in the lower bits of the 32-bit word must be set to zero.
     *  @param intBits Precission of the fixed point object: number of integer bits.
     *  @param fracBits Precission of the fixed point object: number of fractional bits.
     *
     *  @return A new initialized fixed point object.
     *
     */

    FixedPoint(s8bit sign, u32bit intVal, u32bit fracVal, u32bit intBits = MAX_INTEGER_BITS, u32bit fracBits = MAX_FRACTIONAL_BITS);

    /**
     *
     *  Operator + for fixed point objects.
     *
     *  Creates a fixed point object that is the result of the addition of the two input fixed point objects.
     *
     *  @param a A reference to a fixed point object that acts as the first operand of the addition.
     *  @param b A reference to a fixed point object that acts as the second operand of the addition.
     *
     *  @return A fixed point object that stores the result of the addition operation.
     *
     */

    FixedPoint operator+ (const FixedPoint &b);

    /**
     *
     *  Operator - for fixed point objects.
     *
     *  Creates a fixed point object that is the result of substraction of the two input fixed point objects.
     *
     *  @param a A reference to a fixed point object that acts as the first operand of the substraction.
     *  @param b A reference to a fixed point object that acts as the second operand of the substraction.
     *
     *  @return A fixed point object that stores the result of the substraction operation.
     *
     */

    FixedPoint operator- (const FixedPoint &b);

    /**
     *
     *  Operator - for fixed point objects.
     *
     *  Creates a fixed point object that is the result of changing the sign of the input fixed point.
     *
     *  @param a A reference to a fixed point object that acts as the first operand of the negation.
     *
     *  @return A fixed point object that stores the result of the negation operation.
     *
     */

    FixedPoint operator- ();

    /**
     *
     *  Operator * for fixed point objects.
     *
     *  Creates a fixed point object that is the result of the multiplication of the two input fixed point objects.
     *
     *  @param a A reference to a fixed point object that acts as the first operand of the multiplication.
     *  @param b A reference to a fixed point object that acts as the second operand of the multiplication.
     *
     *  @return A fixed point object that stores the result of the multiplication operation.
     *
     */

    FixedPoint operator* (const FixedPoint &b);

    /**
     *
     *  Greater than comparator for fixed point objects.
     *
     *  Returns if the left fixed point value is greater than the right value.
     *
     *  @param fxp A reference to a fixed point object that acts as the left operand of the comparison.
     *
     *  @return True if the left fixed point operand is greater than the right operand.
     *
     */

    bool operator> (const FixedPoint &fxp);

    /**
     *
     *  Converts the value of the fixed point object into a 32-bit float point value.
     *
     *  @return The 32-bit float point value equivalent to the value of the fixed point object.
     *
     */

    f32bit toFloat32();

    /**
     *
     *  Converts the value of the fixed point object into a 64-bit float point value.
     *
     *  @return The 64-bit float point value equivalent to the value of the fixed point object.
     *
     */

    f64bit toFloat64();

    /**
     *
     *  Gets a fixed point object representing the signed integer part of the fixed point value
     *  (the integer part is zero).
     *
     *  @return A fixed point object that stores the signed integer part of the fixed point value.
     *
     */

    FixedPoint integer() const;

    /**
     *
     *  Gets a fixed point object representing only the fractional part of the fixed point value
     *  (the integer part is zero).
     *
     *  @return A fixed point object that stores the fractional part of the fixed point value.
     *
     */

    FixedPoint fractional() const;

    /**
     *
     *  Outputs the fixed point value to the stdout.
     *
     */

    void print() const;

    /**
     *
     *  Dumps all the attributes of the fixed point object.
     *
     */

    void dump() const;

    /**
     *
     *  Multiplies two 64 bit integers and returns the resulting 128 bit integer as two 64-bit integers.
     *
     *  @param a Multiplication first operand.
     *  @param b Multiplication second operand.
     *  @param resHI Reference to a 64-bit integer where to store the higher 64-bit bits of the multiplication result.
     *  @param resLO Reference to a 64-bit integer where to store the lower 64-bit  bits of the multiplication result.
     *
     */

    void mult64bit(const u64bit a, const u64bit b, u64bit &resHI, u64bit &resLO);
};


#endif
