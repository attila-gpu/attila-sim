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

/**************************************************
Simple Buffer, D3D Style.

Dependencies: none

Author: José Manuel Solís
**************************************************/

#ifndef __D3DBUFFER
#define __D3DBUFFER

/************************************************
A convenient unknown size buffer D3D style.
************************************************/
class D3DBuffer {
public:
    /**************************
    Returns a pointer to a raw data region
    of setted size.

    Precondition: size is set.
    **************************/
    virtual void lock(void **p);

    /*************************
    The locked pointer is invalid
    after this call.

    Precondition: lock has been called
    **************************/
    virtual void unlock();

    /**************************
    Sets capacity for size bytes
    ***************************/
    virtual void setSize(size_t size);

    /**************************
    Returns current size

    Precondition: size is set
    **************************/
    virtual size_t getSize();

    D3DBuffer();
    ~D3DBuffer();
private:
    // Internal buffer
    size_t size; 
    void *data;
};


#endif
