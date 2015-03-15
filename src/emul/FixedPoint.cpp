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
 *  @file FixedPoint.cpp
 *
 *  This file implements the Fixed Point class supporting fixed point data types and basic
 *  fixed point arithmetic operations.
 *
 */

#include "FixedPoint.h"
#include <iostream>
#include <cmath>

using namespace std;

//  Constructor:  32-bit float point version.
FixedPoint::FixedPoint(f32bit f, u32bit intBits_, u32bit fracBits_) :
    intBits(intBits_), fracBits(fracBits_)
{
    u8bit exponent;
    u32bit mantissa;

    //  Check representation limits.
    if (intBits > MAX_INTEGER_BITS)
    {
        panic("FixedPoint", "FixedPoint(float32)", "Integral representation exceded");
    }

    //  Check representation limits.
    if (fracBits > MAX_FRACTIONAL_BITS)
    {
        panic("FixedPoint", "FixedPoint(float32)", "Fractional representation exceded");
    }

//cout << "----  Starting conversion (source float32) --- " << endl << endl;
//cout << "float point value = " << f << endl;

    //  Get components of the 32-bit float point value; sign, mantissa and exponent.
    sign = (((*((u32bit *) &f) & 0x80000000) >> 31) == 0) ? 1 : -1;
    mantissa = (*((u32bit *) &f) & 0x007fffff);
    exponent = (*((u32bit *) &f) & 0x7f800000) >> 23;

//cout << hex;
//cout << "sign = " << s32bit(sign) << " | mantissa = " << mantissa << " | exponent = " << u32bit(exponent) << endl;

    //  Check for NaN or infinite.
    nan = (mantissa != 0) && (exponent == 0xff);
    infinite = (mantissa == 0) && (exponent == 0xff);

    //  Check for denormalized 32-bit float point values.
    bool denormal = (mantissa != 0) && (exponent == 0);

    //  Set exponent to the denormalized exponent.
    if (denormal)
    {
//cout << "denormal" << endl;
        exponent = 1;
    }

    //  Check for zero.
    bool zero = (mantissa == 0) && (exponent == 0);

    //  Add to the mantissa the implicit most significative bit if not a denormalized number.
    if (!denormal && !zero)
    {
        mantissa = mantissa | 0x00800000;
    }

    //  Compute the position of the fractional point inside the mantissa.
    s32bit pointPosition = 23 - (exponent - 127);

//cout << dec;
//cout << "pointPosition = " << pointPosition << endl;

    //  Set overflow flag.
    overflow = (pointPosition < 0);

    //  Create the mask covering the integral bits of the fixed point value.
    u64bit intMask = (u64bit(1) << intBits) - 1;

//cout << hex;
//cout << "intMask = " << intMask << endl;

    //  Get the integral bits of the value.
    if (std::abs(f32bit(pointPosition)) > 31)
        intVal = 0;     //  Shifted to zero due to shift value overflow.
    else
        intVal = ((pointPosition >= 0) ? (mantissa >> pointPosition) : (u64bit(mantissa) << s32bit(std::abs(f32bit(pointPosition))))) & intMask;

//cout << "shifted mantissa (+) = " << (mantissa >> pointPosition) << endl;
//cout << "shifted mantissa (-) = " << (mantissa << std::abs(f32bit(pointPosition))) << endl;

//cout << "intVal = " << intVal << endl;

    //  Create the mask covering the fractional bits of the fixed point value.
    u64bit fracMask = u64bit((u64bit(1) << fracBits) - 1) << (64 - fracBits);

//cout << "fracBits = " << fracBits << " t = " << (1 << fracBits) << " s = " << ((1 << fracBits) - 1) << " r = " << (64 - fracBits) << endl;
//cout << "fracMask = " << fracMask << endl;

    //  Get the fractional bits of the value.
    s32bit fractionalShift = 64 - pointPosition;
    fracVal = (pointPosition <= 0) ? 0 : ((fractionalShift >= 0) ? (u64bit(mantissa) << fractionalShift) : (u64bit(mantissa) >> (-fractionalShift)));
    fracVal = fracVal & fracMask;

//cout << "shifted Mantissa = " << (mantissa << (64 - pointPosition)) << endl;
//cout << "fracVal = " << fracVal << endl;

    //  Check for underflow.
    underflow = !overflow && (mantissa != 0) && (intVal == 0) && (fracVal == 0);

    if (overflow)
    {
//cout << "Overflow" << endl;
        intVal = (1 << (intBits -1)) - 1;
        fracVal = (1 << fracBits) - 1;
    }

    if (underflow)
    {
//cout << "Underflow" << endl;
        intVal = 0;
        fracVal = 0;
    }

    if (nan)
    {
//cout << "NaN" << endl;
        intVal  = 0;
        fracVal = 0;
        sign = 0;
    }

    if (infinite)
    {
//cout << "infinite" << endl;
        intVal  = 0xffffffffffffffffULL;
        fracVal = 0xffffffffffffffffULL;
    }

//cout << dec;
//cout << "---- End of conversion ---- " << endl << endl;
}

//  Constructor:  64-bit float point version.
FixedPoint::FixedPoint(f64bit f, u32bit intBits_, u32bit fracBits_) :
    intBits(intBits_), fracBits(fracBits_)
{
    u16bit exponent;
    u64bit mantissa;

    //  Check representation limits.
    if (intBits > MAX_INTEGER_BITS)
    {
        panic("FixedPoint", "FixedPoint(float64)", "Integral representation exceded");
    }

    //  Check representation limits.
    if (fracBits > MAX_FRACTIONAL_BITS)
    {
        panic("FixedPoint", "FixedPoint(float64)", "Fractional representation exceded");
    }

//cout << "----  Starting conversion (source float64) --- " << endl << endl;
//cout << "float point value = " << f << endl;

    //  Get components of the 64-bit float point value; sign, mantissa and exponent.
    sign = ((*((u64bit *) &f) & 0x8000000000000000ULL) >> 63) == 0 ? 1 : -1;
    mantissa = (*((u64bit *) &f) & 0x000fffffffffffffULL);
    exponent = (*((u64bit *) &f) & 0x7ff0000000000000ULL) >> 52;

//cout << hex;
//cout << "sign = " << s64bit(sign) << " | mantissa = " << mantissa << " | exponent = " << u64bit(exponent) << endl;

    //  Check for NaN or infinite.
    nan = (mantissa != 0) && (exponent == 0x7ff);
    infinite = (mantissa == 0) && (exponent == 0x7ff);

    //  Check for denormalized 64-bit float point values.
    bool denormal = (mantissa != 0) && (exponent == 0);

    //  Set exponent to the denormalized exponent.
    if (denormal)
    {
//cout << "denormal" << endl;
        exponent = 1;
    }

    //  Add to the mantissa the implicit most significative bit if not a denormalized number.
    if (!denormal)
    {
        mantissa = mantissa | 0x0010000000000000ULL;
    }

    //  Compute the position of the fractional point inside the mantissa.
    s32bit pointPosition = 52 - (exponent - 1023);

//cout << dec;
//cout << "pointPosition = " << pointPosition << endl;

    //  Set overflow flag.
    overflow = (pointPosition < 0);

    //  Create the mask covering the integral bits of the fixed point value.
    u64bit intMask = (u64bit(1) << intBits) - 1;

//cout << hex;
//cout << "intMask = " << intMask << endl;

    //  Get the integral bits of the value.
    if (std::abs(f32bit(pointPosition)) > 63)
        intVal = 0;     //  Shifted to zero due to shift value overflow.
    else
        intVal = ((pointPosition >= 0) ? (mantissa >> pointPosition) : (mantissa << s32bit(std::abs(f32bit(pointPosition))))) & intMask;

//cout << "shifted mantissa (+) = " << (mantissa >> pointPosition) << endl;
//cout << "shifted mantissa (-) = " << (mantissa << std::abs(f32bit(pointPosition))) << endl;

//cout << "intVal = " << intVal << endl;

    //  Create the mask covering the fractional bits of the fixed point value.
    u64bit fracMask = u64bit((u64bit(1) << fracBits) - 1) << (64 - fracBits);

//cout << "fracBits = " << fracBits << " t = " << (1 << fracBits) << " s = " << ((1 << fracBits) - 1) << " r = " << (64 - fracBits) << endl;
//cout << "fracMask = " << fracMask << endl;

    //  Get the fractional bits of the value.
    s32bit fractionalShift = 64 - pointPosition;
    fracVal = (pointPosition <= 0) ? 0 : ((fractionalShift >= 0) ? (u64bit(mantissa) << fractionalShift) : (u64bit(mantissa) >> (-fractionalShift)));
    fracVal = fracVal & fracMask;

//cout << "shifted Mantissa = " << (mantissa << (64 - pointPosition)) << endl;
//cout << "fracVal = " << fracVal << endl;

    //  Check for underflow.
    underflow = !overflow && (mantissa != 0) && (intVal == 0) && (fracVal == 0);

    if (overflow)
    {
//cout << "Overflow" << endl;
        intVal = (1 << (intBits -1)) - 1;
        fracVal = (1 << fracBits) - 1;
    }

    if (underflow)
    {
//cout << "Underflow" << endl;
        intVal = 0;
        fracVal = 0;
    }

    if (nan)
    {
//cout << "NaN" << endl;
        intVal = 0;
        fracVal = 0;
        sign = 0;
    }

    if (infinite)
    {
//cout << "infinite" << endl;
        intVal  = 0xffffffffffffffffULL;
        fracVal = 0xffffffffffffffffULL;
    }

//cout << dec;
//cout << "---- End of conversion ---- " << endl << endl;

}

//  Constructor:  32-bit integer point version.
FixedPoint::FixedPoint(s8bit signP, u32bit intValP, u32bit fracValP, u32bit intBitsP, u32bit fracBitsP)
{
    sign = signP;
    intVal = intValP;
    fracVal = u64bit(fracValP) << 32;
    intBits = intBitsP;
    fracBits = fracBitsP;

    nan = false;
    infinite = false;
    overflow = false;
    underflow = false;
}

//  Constructor:  64-bit integer point version.
FixedPoint::FixedPoint(s8bit signP, u64bit intValP, u64bit fracValP, u32bit intBitsP, u32bit fracBitsP)
{
    sign = signP;
    intVal = intValP;
    fracVal = fracValP;
    intBits = intBitsP;
    fracBits = fracBitsP;

    nan = false;
    infinite = false;
    overflow = false;
    underflow = false;
}

FixedPoint FixedPoint::operator+(const FixedPoint &b)
{
    u32bit resIntBits = (intBits > b.intBits) ? intBits : b.intBits;
    u32bit resFracBits = (fracBits > b.fracBits) ? fracBits : b.fracBits;

    u64bit intMask = (u64bit(1) << resIntBits) - 1;
    u64bit fracMask = (u64bit(u64bit(1) << resFracBits) - 1) << (64 - resFracBits);

    s8bit resSign;
    u64bit resFracVal;
    u64bit resIntVal;

    if (sign == b.sign)
    {
        //  Shift fractional portion to the right by one bit to compute carry bit for
        //  the integral portion and then add the two fractional portions.
        resFracVal = (fracVal >> 1) + (b.fracVal >> 1);

        //  Add the integral portions with the carry from the fractional portions.
        resIntVal = (intVal + b.intVal + (resFracVal >> 63)) & intMask;

        //  Shift fractional portion to the left and bit apply mask.
        resFracVal = (resFracVal << 1) & fracMask;

        //  Both signs are the same.
        resSign = sign;
    }
    else
    {
        //  Determine the largest operand.
        if ((intVal > b.intVal) || ((intVal == b.intVal) && (fracVal >= b.fracVal)))
        {
            //  Shift fractional portion to the right by one bit to compute borrow bit for
            //  the integral portion and then substract the two fractional portions.
            resFracVal = (fracVal >> 1) - (b.fracVal >> 1);

            //  Substract the integral portions with the borrow from the fractional portions.
            resIntVal = (intVal -  b.intVal - (resFracVal >> 63)) & intMask;

            //  Shift fractional portion to the left and bit apply mask.
            resFracVal = (resFracVal << 1) & fracMask;

            //  The result has the sign of the largest operand.
            resSign = sign;
        }
        else
        {
            //  Shift fractional portion to the right by one bit to compute borrow bit for
            //  the integral portion and then substract the two fractional portions.
            resFracVal = (b.fracVal >> 1) - (fracVal >> 1);

            //  Substract the integral portions with the borrow from the fractional portions.
            resIntVal = (b.intVal -  intVal - (resFracVal >> 63)) & intMask;

            //  Shift fractional portion to the left and bit apply mask.
            resFracVal = (resFracVal << 1) & fracMask;

            //  The result has the sign of the largest operand.
            resSign = b.sign;
        }
    }

    return FixedPoint(resSign, resIntVal, resFracVal, resIntBits, resFracBits);
}

FixedPoint FixedPoint::operator-(const FixedPoint &b)
{
    u32bit resIntBits = (intBits > b.intBits) ? intBits : b.intBits;
    u32bit resFracBits = (fracBits > b.fracBits) ? fracBits : b.fracBits;

    s64bit intMask = (u64bit(1) << resIntBits) - 1;
    u64bit fracMask = (u64bit(u64bit(1) << resFracBits) - 1) << (64 - resFracBits);

    s8bit resSign;
    u64bit resFracVal;
    u64bit resIntVal;

    if (sign != b.sign)
    {
        //  Shift fractional portion to the right by one bit to compute carry bit for
        //  the integral portion and then add the two fractional portions.
        resFracVal = (fracVal >> 1) + (b.fracVal >> 1);

        //  Add the integral portions with the carry from the fractional portions.
        resIntVal = (intVal + b.intVal + (resFracVal >> 63)) & intMask;

        //  Shift fractional portion to the left and bit apply mask.
        resFracVal = (resFracVal << 1) & fracMask;

        //  Both signs are the same.
        resSign = sign;
    }
    else
    {
        //  Determine the largest operand.
        if ((intVal > b.intVal) || ((intVal == b.intVal) && (fracVal >= b.fracVal)))
        {
            //  Shift fractional portion to the right by one bit to compute borrow bit for
            //  the integral portion and then substract the two fractional portions.
            u64bit resFracVal = (fracVal >> 1) - (b.fracVal >> 1);

            //  Substract the integral portions with the borrow from the fractional portions.
            s64bit resIntVal = (intVal - b.intVal - (resFracVal >> 63)) & intMask;

            //  Shift fractional portion to the left and bit apply mask.
            resFracVal = (resFracVal << 1) & fracMask;

            //  The result has the sign of the largest operand.
            resSign = sign;
        }
        else
        {
            //  Shift fractional portion to the right by one bit to compute borrow bit for
            //  the integral portion and then substract the two fractional portions.
            resFracVal = (b.fracVal >> 1) - (fracVal >> 1);

            //  Substract the integral portions with the borrow from the fractional portions.
            resIntVal = (b.intVal -  intVal - (resFracVal >> 63)) & intMask;

            //  Shift fractional portion to the left and bit apply mask.
            resFracVal = (resFracVal << 1) & fracMask;

            //  The result has the inverted sign of the largest operand.
            resSign = -b.sign;
        }
    }

    return FixedPoint(resSign, resIntVal, resFracVal, resIntBits, resFracBits);
}

FixedPoint FixedPoint::operator-()
{
    return FixedPoint(-sign, intVal, fracVal, intBits, fracBits);
}

FixedPoint FixedPoint::operator*(const FixedPoint &b)
{
//cout << "Mult A = ";
//this->dump();
//cout << " x B = ";
//b.dump();
//cout << endl;

//cout << hex;

//cout << "this.IntBits = " << intBits << " | this.fracBits = " << fracBits << endl;
//cout << "b.IntBits = " << b.intBits << " | b.fracBits = " << b.fracBits << endl;

    u32bit resIntBits = (intBits > b.intBits) ? intBits : b.intBits;
    u32bit resFracBits = (fracBits > b.fracBits) ? fracBits : b.fracBits;

//cout << "resIntBits = " << resIntBits << " | resFracBits = " << resFracBits << endl;

    s64bit intMask = (u64bit(1) << resIntBits) - 1;
    u64bit fracMask = (u64bit((u64bit(1) << resFracBits) - 1)) << (64 - resFracBits);

//cout << "intMask = " << intMask << " | fracMask = " << fracMask << endl;

    u64bit multFrac0;
    u64bit multFrac1;
    u64bit multFrac2;
    u64bit multFrac3;
    u64bit multFrac4;
    u64bit multInt1;
    u64bit multInt2;
    u64bit multInt3;
    u64bit multInt4;

    mult64bit(fracVal, b.fracVal, multFrac1, multFrac0);
    mult64bit(fracVal, b.intVal, multInt2, multFrac2);
    mult64bit(intVal, b.fracVal, multInt3, multFrac3);
    mult64bit(intVal, b.intVal, multInt4, multInt1);

    multFrac4 = (multFrac1 >> 2) + (multFrac2 >> 2) + (multFrac3 >> 2);
    u64bit multFracFinal = multFrac4 << 2;
    s64bit multIntFinal = multInt1 + multInt2 + multInt3 + (multFrac4 >> 62);

//cout << "multFrac1 = " << multFrac1 << " | multFrac2 = " << multFrac2 << " | multFrac3 = " << multFrac3 << " | multFrac4 = " << multFrac4 << endl;
//cout << "multInt1 = " << multInt1 << " | multInt2 = " << multInt3 << " | multInt3 = " << multFrac3 << endl;
//cout << "multFrac = " << multFracFinal << " | multInt = " << multIntFinal << endl;

    u64bit resFracVal = multFracFinal & fracMask;
    s64bit resIntVal = multIntFinal & intMask;

    //  Multiply signs.
    s8bit resSign = sign * b.sign;

//cout << "resFracVal = " << resFracVal << " | resIntVal = " << resIntVal << endl;

//cout << dec;

    return FixedPoint(resSign, resIntVal, resFracVal, resIntBits, resFracBits);
}

bool FixedPoint::operator>(const FixedPoint &b)
{
    //  Get the largest number of bits for integer and fractional values in both operands.
    u32bit cmpFracBits = (fracBits > b.fracBits) ? fracBits : b.fracBits;
    u32bit cmpIntBits = (intBits > b.intBits) ? intBits : b.intBits;

    //  Compare integer values (sign matters).
    s64bit intMask = (u64bit(1) << cmpIntBits) - 1;

    if ( (s64bit(sign) * (s64bit(intVal) & intMask)) > (s64bit(b.sign) * (s64bit(b.intVal) & intMask)) )
        return true;

    //  Check if equal integer parts.
    if ( (sign == b.sign) && (s64bit(intVal) == s64bit(b.intVal)) )
    {
        //  Compare fractional values.

        //  Required shiftings for fractional values;
        u32bit shiftFracA = cmpFracBits - fracBits;
        u32bit shiftFracB = cmpFracBits - b.fracBits;

        if ( (u64bit(fracVal) << shiftFracA) > (u64bit(b.fracVal) << shiftFracB) )
            return true;
        else
            return false;
    }

    return false;
}

#define FP_FINFINITE 0x7F800000
#define FP_FNAN 0x7FD00000

f32bit FixedPoint::toFloat32()
{
    f32bit val;

    /*if (overflow)
        cout << "FixedPoint => toFloat32 -> overflow found" << endl;
    if (underflow)
        cout << "FixedPoint => toFloat32 -> underflow found" << endl;
    if (infinite)
        cout << "FixedPoint => toFloat32 -> infinite found" << endl;
    if (nan)
        cout << "FixedPoint => toFloat32 -> nan found" << endl;*/

    if ((overflow || underflow) || infinite)
        *((u32bit *) &val) = FP_FINFINITE;
    else if (nan)
        *((u32bit *) &val) = FP_FNAN;

    val = f32bit(sign) * f32bit(intVal) + f32bit(sign) * (f32bit(fracVal) / std::pow(2.0f, 64.0f));

    return val;
}

f64bit FixedPoint::toFloat64()
{
    f64bit val;

    val = f64bit(sign) * f64bit(intVal) + f64bit(sign) * (f64bit(fracVal) / std::pow(2.0f, 64.0f));

    return val;
}

FixedPoint FixedPoint::integer() const
{
    return FixedPoint(sign, intVal, 0, intBits, fracBits);
}

FixedPoint FixedPoint::fractional() const
{
    return FixedPoint(1, 0, fracVal, intBits, fracBits);
}

void FixedPoint::print() const
{
    cout << hex << intVal << "." << fracVal << dec;
}

void FixedPoint::dump() const
{
    cout << "-------" << endl;
    cout << hex << "intVal = " << intVal << endl;
    cout << "sign = " << s32bit(sign) << endl;
    cout << "fracVal = " << fracVal << endl;
    cout << "intBits = " << intBits << endl;
    cout << "fracBits = " << fracBits << endl;
    cout << "nan = " << nan << endl;
    cout << "infinite = " << infinite << endl;
    cout << "overflow = " << overflow << endl;
    cout << "underflow = " << underflow << endl;
    cout << "-------" << endl;
}

void FixedPoint::mult64bit(const u64bit a, const u64bit b, u64bit &resHI, u64bit &resLO)
{

//cout << " mult64bit -> input A = " << a << " input B = " << b << endl;

    u64bit multRes1 = (a & 0x00000000FFFFFFFFULL) * (b & 0x00000000FFFFFFFFULL);
    u64bit multRes2 = (a & 0x00000000FFFFFFFFULL) * (b >> 32);
    u64bit multRes3 = (a >> 32) * (b & 0x00000000FFFFFFFFULL);
    u64bit multRes4 = (a >> 32) * (b >> 32);

    u64bit resLOWithCarry = (multRes1 >> 2) + ((multRes2 << 32) >> 2) + ((multRes3 << 32) >> 2);
    resLO = (resLOWithCarry << 2) + (multRes1 & 0x03);

    resHI = multRes4 + (multRes2 >> 32) + (multRes3 >> 32) + (resLOWithCarry >> 62);

//cout << "mult64bit -> output HI = " << resHI << " output LO = " << resLO << endl;
}



