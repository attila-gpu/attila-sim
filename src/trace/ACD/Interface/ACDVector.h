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

#ifndef ACD_VECTOR
#define ACD_VECTOR

#include "ACDTypes.h"
#include <ostream>
#include <iostream>
#include <cmath>
#include <cstring>

namespace acdlib
{

/**
 * Templatized standard ACD vector class
 *
 * @author Carlos Gonz�lez Rodr�guez (cgonzale@ac.upc.edu)
 * @date 03/13/2007
 */
template<class T, acd_uint D> class ACDVector
{
public:

    /**
     * Creates an uninitialized vector of dimension D
     */
    inline ACDVector();

    /**
     * Creates a vector of dimension D with all all its components initialized to the same value
     *
     * @param initValue Initial value of every vector compoent
     */
    ACDVector(T initValue);

    /**
     * Creates a vector of dimension D with the "first" components initialized to a value
     * and the remaining components initialized to a specific default value
     *
     * @param initValue Value of the "first" components
     * @param count Number of components starting from the first one that use the 'initialValue'
     * @param def Default value for the components after the first ones
     */
    ACDVector(T initValue, acd_uint count, T def = 0);

    /**
     * Creates a vector of dimension D using an input array to initialize all the vector components
     *
     * @param values An array of D values used to initialize the D components of the vector
     *
     * @note It is assumed that values contains at least D values, otherwise the expected behaviour is
     *       undefined
     */
    ACDVector(const T* values);

    /**
     * Creates a vector of dimension D using a number of values from an input array and setting
     * the remaining components (if exists), with a default value
     *
     * @param values An array used to initialize the "first" components of the vector
     * @param count Number of values to use from the array
     * @param def Default value used to initialize the remaining components not initialized
     *        with values coming from the array
     */
    ACDVector(const T* values, acd_uint count, T def = 0);

    /**
     * Returns a read/write reference of a vector component
     *
     * @param index component index
     * @returns a read/write reference to the vector component
     *
     * @note Bounds checking is not performed (like c-like arrays)
     * @note This operator is automatically selected in "non-const contexts"
     */
    inline T& operator[](acd_uint index);

    /**
     * Returns a read-only reference of a vector component
     *
     * @param index component index
     * @returns a read-only reference to the vector component
     *
     * @note Bounds checking is not performed (like c-like arrays)
     * @note This operator is automatically selected in "const contexts"
     */
    inline const T& operator[](acd_uint index) const;

    /**
     * Assigns the contents from a vector of dimension D to this vector
     *
     * @param vect A vector of dimension D and type T
     */
    inline ACDVector& operator=(const ACDVector& vect);

    /**
     * Assigns the contents from an c-like array of dimension D to this vector
     *
     * @param vect A c-like array containing at least D values
     *
     * @note It is assumed that vect contains at least D values, otherwise the expected behaviour is
     *       undefined
     */
    inline ACDVector& operator=(const T* vect);

    /**
     * Multiplies each vector component by one scalar value
     *
     * @param scalar The scalar value used to multiply each vector component
     *
     * @returns A new vector with each component equals to the same component in the original vector
     *          multiplied by the scalar value
     */
    ACDVector operator*(const T& scalar) const;

    /**
     * Computes the DOT product between the vector and another input vector
     *
     * @param vect The second vector used to compute the DOT product
     *
     * @returns The DOT product of the two input vectors
     */
    T operator*(const ACDVector& vect) const;

    /**
     * Perform the addition of the vector with another input vector
     *
     * @param vect The second vector used in the addition operation
     *
     * @returns A vector which is the addition of the two input vectors
     */
    ACDVector operator+(const ACDVector& vect) const;

    /**
     * Perform the subtract if the vector with a second vector
     *
     * @param the second vector used in the subtrac operation
     *
     * @returns A vector which is the subtract between the two vectors
     */
    ACDVector operator-(const ACDVector& vect) const;

    /**
     * Applies the possitive sign of type T to each vector component
     *
     * @returns A vector with the possitive sign of type T applied to each vector component
     */
    ACDVector operator+() const;

    /**
     * Applies the negative sign of type T to each vector component
     *
     * @returns A vector with the negative sign of type T applied to each vector component
     */
    ACDVector operator-() const;

    /**
     * Check if two vectors are equal (all components have the same value)
     *
     * @param v The second vector of the comparison
     *
     * @returns True if the two vector are equal, false otherwise
     */
    inline acd_bool operator==(const ACDVector<T,D>& v) const;

    /**
     * Check if two vectors are different (if they have any component different)
     *
     * @param v The second vector of the comparison
     *
     * @returns True if the two vectors are different, false otherwise
     */
    inline acd_bool operator!=(const ACDVector<T,D>& v) const;
    /**
     * Returns the vector dimension (D)
     *
     * @returns the vector dimension
     */
    inline acd_uint dim() const;

    /**
     * Returns the internal data managed by this vector
     *
     * @returns the internal data through a pointer to non-const data (used automatically in non-const contexts)
     */
    inline T* c_array();

    /**
     * Returns the internal data managed by this vector
     *
     * @returns the internal data through a pointer to const data (used automatically in const contexts)
     */
    inline const T* c_array() const;

    ///////////////////
    // PRINT METHODS //
    ///////////////////

    friend std::ostream& operator<<(std::ostream& os, const ACDVector<T,D>& v)
    {
        v.print(os);
        return os;
    }

    void print(std::ostream& out = std::cout) const;

private:

    T _data[D];
};

/**
 * Convenient matrix definitions frequently used
 */
typedef ACDVector<acd_float,4> ACDFloatVector4;
typedef ACDVector<acd_float,3> ACDFloatVector3;

typedef ACDVector<acd_uint,4> ACDUintVector4;
typedef ACDVector<acd_uint,3> ACDUintVector3;

typedef ACDVector<acd_int,4> ACDIntVector4;
typedef ACDVector<acd_int,3> ACDIntVector3;


/**
 * Helper function to implement simetric scalar product behaviour
 * i.e: 4 * v == v * 4
 */
template<class T, acdlib::acd_uint D>
inline ACDVector<T,D> operator*(const T& scalar, const ACDVector<T,D>& vect);

////////////////////////////////////////////////////
// EUCLIDIAN OPERATIONS DEFINED FOR FLOAT VECTORS // 
////////////////////////////////////////////////////

/**
 * Returns the length (euclidian norm) of a ACDVector<acd_float,D> with D
 * taking an arbitrary number.
 *
 * This is computed as the root square of the sum of the squared components).
 *
 * @param    vect    The input ACDVector.
 * @returns The vector length
 */
template<acdlib::acd_uint D>
acdlib::acd_float _length(const ACDVector<acd_float,D>& vect);

/**
 * Returns a copy of the equivalent normalized ACDVector<acd_float,D> with D
 * taking an arbitrary number.
 *
 * The normalization is computed by dividing all the components by the length.
 *
 * @param    vect    The input ACDVector.
 * @returns The normalized vector copy
 */
template<acdlib::acd_uint D>
acdlib::ACDVector<acd_float,D> _normalize(const ACDVector<acd_float,D>& vect);

} // namespace acdlib


//////////////////////////////////////
//// ACD Template code definition ////
//////////////////////////////////////

template<class T, acdlib::acd_uint D>
acdlib::ACDVector<T,D>::ACDVector(T initValue)
{
    for ( acd_uint i = 0; i < D; ++i )
        _data[i] = initValue;
}

template<class T, acdlib::acd_uint D>
acdlib::ACDVector<T,D>::ACDVector(const T* values, acd_uint count, T def)
{
    acd_uint i = 0;
    for ( ; i < D && i < count; ++i )
        _data[i] = values[i];
    for ( ; i < D; ++i )
        _data[i] = def;
}

template<class T, acdlib::acd_uint D>
acdlib::ACDVector<T,D>::ACDVector(T initValue, acd_uint count, T def)
{
    acd_uint i = 0;
    for ( ; i < D && i < count; ++i )
        _data[i] = initValue;
    for ( ; i < D; ++i )
        _data[i] = def;
}

template<class T, acdlib::acd_uint D>
acdlib::ACDVector<T,D>::ACDVector(const T* vect) { memcpy(_data, vect, D*sizeof(T)); }

template<class T, acdlib::acd_uint D>
acdlib::ACDVector<T,D>::ACDVector() {}

template<class T, acdlib::acd_uint D>
acdlib::acd_uint acdlib::ACDVector<T,D>::dim() const { return D; }

template<class T, acdlib::acd_uint D>
acdlib::ACDVector<T,D>& acdlib::ACDVector<T,D>::operator=(const ACDVector<T,D>& vect)
{
    memcpy(_data, vect._data, D*sizeof(T));
    return *this;
}

template<class T, acdlib::acd_uint D>
acdlib::ACDVector<T,D>& acdlib::ACDVector<T,D>::operator=(const T* vect)
{
    memcpy(_data, vect, D*sizeof(T));
    return *this;
}

template<class T, acdlib::acd_uint D>
acdlib::ACDVector<T,D> acdlib::ACDVector<T,D>::operator*(const T& scalar) const
{
    ACDVector<T,D> result;
    const T* in = _data;
    T* out = result._data;
    for ( acd_uint i = 0; i < D; ++i )
        *out++ = *in++ * scalar;

    return result;
}

template<class T, acdlib::acd_uint D>
T acdlib::ACDVector<T,D>::operator*(const acdlib::ACDVector<T,D>& vect) const
{
    T accum(0);

    const T* a = _data;
    const T* b = vect._data;
    for ( acd_uint i = 0; i < D; ++i )
        accum += *a++ * *b++;
    
    return accum;
}

template<class T, acdlib::acd_uint D>
acdlib::ACDVector<T,D> acdlib::ACDVector<T,D>::operator+(const ACDVector& vect) const
{
    ACDVector<T,D> result;
    const T* a = _data;
    const T* b = vect._data;
    T* out = result._data;
    for ( acd_uint i = 0; i < D; ++i )
        *out++ = *a++ + *b++;
    return result;
}

template<class T, acdlib::acd_uint D>
const T& acdlib::ACDVector<T,D>::operator[](acdlib::acd_uint index) const 
{
    // no bounds checking performed!
    return _data[index]; 
}

template<class T, acdlib::acd_uint D>
T& acdlib::ACDVector<T,D>::operator[](acdlib::acd_uint index)
{
    // no bounds checking performed!
    return _data[index];
}

template<class T, acdlib::acd_uint D>
acdlib::ACDVector<T,D> acdlib::ACDVector<T,D>::operator+() const
{
    return *this;
}

template<class T, acdlib::acd_uint D>
acdlib::ACDVector<T,D> acdlib::ACDVector<T,D>::operator-() const
{
    ACDVector<T,D> result;
    const T* in = _data;
    T* out = result._data;
    for ( acd_uint i = 0; i < D; ++i )
        *out++ = - *in++;
    return result;
}

template<class T, acdlib::acd_uint D>
acdlib::acd_bool acdlib::ACDVector<T,D>::operator==(const acdlib::ACDVector<T,D>& v) const
{
    return (memcmp(_data, v._data, D * sizeof(T)) == 0);
}

template<class T, acdlib::acd_uint D>
acdlib::acd_bool acdlib::ACDVector<T,D>::operator!=(const acdlib::ACDVector<T,D>& v) const
{
    return (memcmp(_data, v._data, D * sizeof(T)) != 0);
}



template<class T, acdlib::acd_uint D>
T* acdlib::ACDVector<T,D>::c_array() { return _data; }

template<class T, acdlib::acd_uint D>
const T* acdlib::ACDVector<T,D>::c_array() const { return _data; }


template<class T, acdlib::acd_uint D>
acdlib::ACDVector<T,D> acdlib::operator*(const T& scalar, const acdlib::ACDVector<T,D>& vect)
{
    return vect * scalar;
}

template<acdlib::acd_uint D>
acdlib::acd_float acdlib::_length(const acdlib::ACDVector<acd_float,D>& v)
{
    acd_float accum = acd_float(0);

    for (acd_uint i=0; i<D; i++)
        accum += v[i] * v[i];

    return acd_float(sqrt(accum));
}

template<acdlib::acd_uint D>
acdlib::ACDVector<acdlib::acd_float,D> acdlib::_normalize(const acdlib::ACDVector<acdlib::acd_float,D>& v)
{
    ACDVector<acd_float,D> ret;
    
    for (acd_uint i=0; i<D; i++)
    {
        ret[i] = v[i] / _length(v);
    }
    
    return ret;
}

template<class T, acdlib::acd_uint D>
void acdlib::ACDVector<T,D>::print(std::ostream& out) const
{
    //const T* v = _data;

    out << "{";

    for ( acd_uint i = 0; i < D; i++ )
    {
        out << _data[i];
        if ( i != D - 1 ) out << ",";
    }

    out << "}";

    out.flush();
}


#endif // ACD_VECTOR
