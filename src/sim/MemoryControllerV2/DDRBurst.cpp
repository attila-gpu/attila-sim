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

#include <iostream>
#include <sstream>
#include <cstring>
#include "DDRBurst.h"

using namespace std;
using gpu3d::memorycontroller::DDRBurst;

u32bit DDRBurst::instances = 0;

DDRBurst::DDRBurst(u32bit size) : size(size)
{
    ++instances;

    GPU_ASSERT
    (
        if ( size > MAX_BURST_SIZE )
        {
            cout << "Burst length = " << size << " Max. Burst length accepted = " 
                 << MAX_BURST_SIZE << "\n";
            panic("DDRBurst", "ctor", "Max. burst length exceed");
        }
    )

    for ( u32bit i = 0; i < size; i++ )
        masks[i] = 0xF;
}

DDRBurst::~DDRBurst()
{
    --instances;
}

u32bit DDRBurst::countInstances()
{
    return instances;
}

void DDRBurst::setData(u32bit position, u32bit datum)
{
    GPU_ASSERT
    (
        if ( position >= size )
            panic("DDRBurst", "setData", "Position selected in the burst out of bounds");
    )
    values[position] = datum;
    masks[position] = 0xF; // no mask
}

void DDRBurst::setData(u32bit position, u32bit datum, u8bit mask)
{
    GPU_ASSERT
    (
        if ( position >= size )
            panic("DDRBurst", "setData", "Position selected in the burst out of bounds");
        if ( mask > 0xF )
            panic("DDRBurst", "setData", "Mask out of bounds, must be in the range [0..15]");
    )
    values[position] = datum;
    masks[position] = mask;
}

void DDRBurst::setData(const u8bit* dataBytes, u32bit dataSize)
{
    if ( dataSize > 4 * size )
        panic("DDRBurst", "setData", "Too many data supplied");

    if ( dataSize == 0 )
        panic("DDRBurst", "setData", "Amount of data supplied must be at least 1 byte");

    memcpy(values, dataBytes, dataSize); // copy data

    if ( dataSize < 4 * size) // partial burst ( 4*size > dataSize )
    {
        u32bit i = dataSize / 4; // Compute the first 32-bit burst item to be masked
        if ( dataSize % 4 == 0 )
            ; // frequent case (do nothing)
        else
        {
            //if ( dataSize % 4 == 1 )
            //    masks[i] = 0x8;
            //else if ( dataSize % 4 == 2 )
            //    masks[i] = 0xC;
            //else // ( dataSize % 4 == 3 )
            //    masks[i] = 0xE;
            if ( dataSize % 4 == 1 )
                masks[i] = 0x1;
            else if ( dataSize % 4 == 2 )
                masks[i] = 0x3;
            else // ( dataSize % 4 == 3 )
                masks[i] = 0x7;
            i++; // next 32-bit item to be masked
        }
        // mask not set bytes
        for ( ; i < size; i++ )
            masks[i] = 0x0; // mask the four bytes of the 32-bit datum
    }
}

void DDRBurst::setMask(const u32bit* writeMask, u32bit maskSize)
{
    GPU_ASSERT(
        if ( maskSize > size )
            panic("DDRBurst", "setMask", "Mask size cannot be greater than burst length");
    )
    if ( maskSize == 0 ) 
        maskSize = size;
    
    // Mask conversion
    /*
    for ( u32bit i = 0; i < maskSize; i++ )
    {
        masks[i] = 0x0;
        if ( writeMask[i] & 0xFF000000 == 0xFF000000)
            masks[i] = 0x1;
        if ( writeMask[i] & 0x00FF0000 == 0x00FF0000)
            masks[i] |= 0x2;
        if ( writeMask[i] & 0x0000FF00 == 0x0000FF00)
            masks[i] |= 0x4;
        if ( writeMask[i] & 0x000000FF == 0x000000FF)
            masks[i] |= 0x8;
    }
    */

    for ( u32bit i = 0; i < maskSize; i++ )
    {
        masks[i] = 0x0;
        if ( (writeMask[i] & 0x000000FF) == 0x000000FF)
            masks[i] = 0x1;
        if ( (writeMask[i] & 0x0000FF00) == 0x0000FF00)
            masks[i] |= 0x2;
        if ( (writeMask[i] & 0x00FF0000) == 0x00FF0000)
            masks[i] |= 0x4;
        if ( (writeMask[i] & 0xFF000000) == 0xFF000000)
            masks[i] |= 0x8;
    }
}

void DDRBurst::write(u32bit position, u32bit& destination) const
{
    GPU_ASSERT
    (
        if ( position >= size )
            panic("DDRBurst", "write", "Position selected in the burst out of bounds");
    )

    u8bit mask = masks[position]; // mask for this 32-bit value

    if ( mask == 0xF ) // optimize frequent case (write without mask)
    {
        destination = values[position];
        return ;
    }
    else if ( mask == 0x0 ) // all bytes masked
        return ;

    // Obtain byte address pointers to source and destination
    const u8bit* source = (const u8bit *)&values[position];
    u8bit* dest = (u8bit *)&destination;

    //if ( mask & 0x8 ) *dest = *source;
    //if ( mask & 0x4 ) *(dest+1) = *(source+1);
    //if ( mask & 0x2 ) *(dest+2) = *(source+2);
    //if ( mask & 0x1 ) *(dest+3) = *(source+3);
    if ( mask & 0x1 ) *dest = *source;
    if ( mask & 0x2 ) *(dest+1) = *(source+1);
    if ( mask & 0x4 ) *(dest+2) = *(source+2);
    if ( mask & 0x8 ) *(dest+3) = *(source+3);
}

void DDRBurst::dump() const
{
    cout << "Burst size: " << size
         << " Data: ";

    const u8bit* val = getBytes();
    cout << hex;
    for ( u32bit i = 0; i < size*4; i++ )
        cout << u32bit(val[i]) << ".";

    cout << " Mask: ";
    for ( u32bit i = 0; i < size; i++ )
    {
        u8bit m = masks[i];
        if ( m & 0x8 ) cout << "1."; else cout << "0.";
        if ( m & 0x4 ) cout << "1."; else cout << "0.";
        if ( m & 0x2 ) cout << "1."; else cout << "0.";
        if ( m & 0x1 ) cout << "1."; else cout << "0.";
    }
    cout << dec;
}

const u8bit* DDRBurst::getBytes() const
{
    return (const u8bit*)values;
}

u32bit DDRBurst::getSize() const
{
    return size;
}
