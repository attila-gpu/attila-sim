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

#ifndef DATABUFFER_H
    #define DATABUFFER_H

#include <iostream>
#include <stdio.h>
#include "QuadReg.h"
#include "support.h"

/**
 * Template class designed for store data buffers (of any type)
 * The items of this buffer are "litle" vectors of 3 or 4 components
 *
 * @author Carlos González Rodríguez (cgonzale@ac.upc.es)
 * @ver 1.0
 */
template<typename T>
class DataBuffer
{
private:

    unsigned int nc;
    T* ptr; /** contents */
    T* pos; /** position in contents */
    unsigned int size; /** buffer size (in T elements, no bytes) */
    
public:

    
    /**
     * Creates a data buffer with capacity for a given number of vectors
     *
     * A buffer of 100 float vectors can store (if has 'nc' == 4) 400 floats, that is 
     * bytes 400*sizeof(float)
     *
     * @param vects Number of vectors that can be stored in the buffer
     */
    DataBuffer(unsigned int nc = 4, unsigned int vects = 100) :nc(nc), ptr(0), pos(0), size(vects*nc)
    {
        if ( 2 > nc || nc > 4 )
        {
            char temp[256];
            sprintf(temp, "Number of components invalid (%d). Must be 2,3 or 4", nc);
            panic("DataBuffer","DataBuffer()", temp);
        }
        
        if ( vects == 0 )
            size = 100*nc; // default;
    }

    /**
     * Destroy previous contents an set the buffer to store a maximum of 'vects' vectors
     *
     * @param vects initial capacity
     */
    void set(unsigned int vects)
    {  
        if ( ptr != 0 )
        {
            delete[] ptr;
            ptr = 0;
            pos = 0;
        }
        
        if ( vects != 0 )
            size = nc*vects;
        // else -> use previous size if vects is 0
    }
    
    /**
     * Sets initial capacity and length of stored vects
     */
    void set(unsigned int vects, unsigned int nc)
    {          
        if ( 2 > nc || nc > 4 )
        {
            char temp[256];
            sprintf(temp, "Number of components invalid (%d). Must be 2,3 or 4", nc);
            panic("DataBuffer","DataBuffer()", temp);
        }
        if ( vects == 0 )
        {
            if ( size % this->nc != 0 )
                panic("DataBuffer", "set()", "Shouldn't occur ever");
            vects = size / this->nc;
        }
            
        this->nc = nc; // new number of components
        set(vects);
    }

        
    /**
     * Returns a pointer to the internal contents of the buffer that allows to access
     * the contents pointer-like
     *
     * @code
     *
     * // Example
     *
     * DataBuffer<float,4> db;
     * db.add(1,2,3.45,4.5);
     * db.add(0,0,7,8.4);
     *
     * const float* prt = db.pointer();
     *
     * cout << ptr[3]; // prints 4.5
     * cout << *(ptr+6); // prints 7
     *
     * @endcode
     *
     * @warning: After a call to set() the pointer returned previously by pointer() method is invalid
     */
    const T* pointer() const
    { 
        return ptr; 
    }
    
    /**
     * Equivalent to pointer() but the type is casted away for allowing raw access (byte to byte)
     *
     * This method is provided for convenience, avoids casts in the code
     *
     * @code
     * 
     * // Equivalent
     *
     * const unsigned char* ptr1 = (unsigned char *)db.pointer();
     * const unsigned char* ptr2 = db.raw();
     *
     * @endcode
     */
    const unsigned char* raw() const
    {
        return (unsigned char*)ptr;
    }


    /**
     * Returns the number of vectors contained in the buffer
     *
     * @return number of vectors in the buffer (that is number of elems divided by the size of each vector)
     */
    unsigned int vectors() const
    {
        return (unsigned int)(pos-ptr)/nc;
    }

    /**
     * Number of scalars in the buffer
     *
     * @return number of elements (scalars) in the buffer
     */
    unsigned int elems() const
    {
        return (unsigned int)(pos-ptr);
    }
        
    /**
     * Total bytes ocuppied in the buffer
     *
     * @return number of bytes in the buffer
     */
    unsigned int bytes() const
    { 
        return ((unsigned int)(pos-ptr))*sizeof(T);
    }
    
    unsigned int numberOfComponents() const
    {
        return nc;
    }

    /**
     * Return the maximum number of bytes that can hold the buffer
     *
     * @return maximum number of bytes that can hold the container without being resized
     */
    unsigned int realBytes() const
    {
        return size*sizeof(T);
    }
    
    /**
     * Adds a new vector to the buffer
     *
     * @param qval the vector to insert
     * @return a reference to the buffer that allow chaining multiple adds
     *
     * @warning after calling this method the pointers to contents got with raw() or pointer()
     *          can be invalid, so new call to raw() or pointer() is mandatory after add()
     */
    DataBuffer<T>& add(const QuadReg<float>& qval)
    {
        add(qval[0], qval[1], qval[2], qval[3]);
        return *this;
    }
        
    /**
     * Adds a new vector to the buffer
     *
     * @param x first vector component
     * @param y second vector component
     * @param z third vector component
     * @param w fourth vector component
     *
     * @return a reference to the buffer that allow chaining multiple adds
     *
     * @warning after calling this method the pointers to contents got with raw() or pointer()
     *          can be invalid, so new call to raw() or pointer() is mandatory after add()
     */
    DataBuffer<T>& add(T x, T y, T z = 0, T w = 0)
    {   
        if ( ptr == 0 ) // first add allocates space
        {
            ptr = new T[size];
            pos = ptr;
        }
        if ( size <= (unsigned int)(pos - ptr) )
        {
            // resize. Note: pointers to contents previously got, are invalid now
            size = size + nc*(size/10 + 1); // force multiple of nc
            T* temp = new T[size];
            unsigned int occupied = (unsigned int)(pos-ptr);
            memcpy(temp, ptr, occupied*sizeof(T));
            delete[] ptr;
            ptr = temp;
            pos = temp + occupied;
        }

        *pos++ = x;
        *pos++ = y;

        if ( nc == 2 )
            return *this;
        *pos++ = z; 
        
        if ( nc == 3 )
            return *this;
        *pos++ = w;

        return *this;
    }

    /*
     * Delete buffer contents
     */
    void clear() { pos = ptr; }

    /**
     * Destructor
     */
    ~DataBuffer()
    {
        delete[] ptr;
    }

    /**
     * Dump the DataBuffer contents in a formated way
     *
     * @param os ostream where dump will occur
     */
    void dump(std::ostream& os = std::cout) const
    {
        T* p = ptr;
        os << "{";
        while ( p < pos )
        {
            if ( p != ptr )
                os << ",";

            os << "(";
            unsigned int n = nc;
            while ( n-- > 0 )
            {               
                os << *p;
                if ( n != 0 )
                    os << ",";
                p++;
            }
            os << ")";
        }
        os << "}";
    }

    /**
     * Operator equivalent to dump
     */
    friend std::ostream& operator<<(std::ostream& os, const DataBuffer& db)
    {
        db.dump(os);
        return os;
    }
    
   
};

#endif
