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

#include "DDRBank.h"
#include <iostream>
#include <iomanip>

using namespace std;
using gpu3d::memorycontroller::DDRBurst;
using gpu3d::memorycontroller::DDRBank;

//u32bit DDRBank::defRows = 4096;
//u32bit DDRBank::defColumns = 512; // values of 32 bit
/*
void DDRBank::setDefaultDimensions(u32bit rows, u32bit columns)
{
    GPU_ASSERT
    (
        if ( rows == 0 || columns == 0 )
            panic("DDRBank", "setDefaultDimensions", "rows and columns must be defined greater than 0");
    )
    defRows = rows;
    defColumns = columns;
}

void DDRBank::getDefaultDimensions(u32bit& rows, u32bit& columns)
{
    rows = defRows;
    columns = defColumns;
}
*/
/*
DDRBank::DDRBank()
{
    nRows = defRows;
    nColumns = defColumns;
    activeRow = NoActiveRow;
    
    data = new u32bit*[nRows];
    for ( u32bit i = 0; i < nRows; i++ )
    {
        data[i] = new u32bit[nColumns];
        for ( u32bit j = 0; j < nColumns; j++ )
            data[i][j] = 0xDEADCAFE;
        //memset(data[i], 0xDEADCAFE, nColumns*sizeof(u32bit));
    }
}

*/

DDRBank::DDRBank(u32bit rows, u32bit cols) : 
    nRows(rows), nColumns(cols), activeRow(NoActiveRow)
{
    //cout << "ctor -> bank with " << rows << " rows and " << cols << " columns\n";
    data = new u32bit*[rows];
    for ( u32bit i = 0; i < rows; i++ )
    {
        data[i] = new u32bit[cols];
        for ( u32bit j = 0; j < cols; j++ )
            data[i][j] = 0xDEADCAFE; // Initialize memory
    }
}

DDRBank::DDRBank(const DDRBank& aBank)
{
    //cout << "ctor (cpy) -> bank with " << aBank.nRows << " rows and " << aBank.nColumns << " columns\n";
    nRows = aBank.nRows;
    nColumns = aBank.nColumns;
    activeRow = aBank.activeRow;

    data = new u32bit*[nRows];
    for ( u32bit i = 0; i < nRows; i++ )
    {
        data[i] = new u32bit[nColumns];
        for ( u32bit j = 0; j < nColumns; j++ )
            data[i][j] = aBank.data[i][j]; // Copy memory contents
    }
}


DDRBank& DDRBank::operator=(const DDRBank& aBank)
{
    //cout << "Bank Assign operator\n";
    if ( this == &aBank )
        return *this; // protect self-copying

    delete[] data; // delete previous memory array

    nRows = aBank.nRows;
    nColumns = aBank.nColumns;
    activeRow = aBank.activeRow;

    data = new u32bit*[nRows];
    for ( u32bit i = 0; i < nRows; i++ )
    {
        data[i] = new u32bit[nColumns];
        for ( u32bit j = 0; j < nColumns; j++ )
            data[i][j] = aBank.data[i][j]; // Copy memory contents
    }
    
    return *this;
}

u32bit DDRBank::getActive() const
{
    return activeRow;
}

void DDRBank::activate(u32bit row)
{
    GPU_ASSERT
    (
        if ( row >= nRows && row != NoActiveRow )
            panic("DDRBank", "activate", "row out of bound");
    )
    activeRow = row;
}

void DDRBank::deactivate()
{
    activeRow = NoActiveRow;
}

DDRBurst* DDRBank::read(u32bit column, u32bit burstSize) const
{
    /*
    cout << "DDRBank::read(" << column << "," << burstSize << ")" << endl;
    cout << "Bank row contents: ";
    dumpRow(getActive(), column, burstSize*4);
    cout << endl;
    */

    GPU_ASSERT
    (
        if ( column >= nColumns )
            panic("DDRBank", "read", "Starting column out of bounds");
        if ( activeRow == NoActiveRow )
            panic("DDRBank", "write", "A row must be active");
    )

    DDRBurst* burst = new DDRBurst(burstSize);

    GPU_ASSERT
    (
        /*
        if ( column % burstSize != 0 )
        {
            cout << "Column = " << column << "  Current burst size = " << burstSize << "\n";
            panic("DDRBank", "read", "Starting column must be aligned to burst size");
        }
        */
        if ( burstSize + column > nColumns )
        {
cout << "DDRBank::read => column = " << column << " burstSize = " << burstSize << " nColumns = " << nColumns << endl;
            panic("DDRBank", "read", "Burst access out of row bounds");
        }
    )
    
    for ( u32bit i = 0; i < burstSize; i++ )
        burst->setData(i, data[activeRow][i+column]);

    /*
    cout << "Burst contents: ";
    burst->dump();
    cout << endl;
    */

    return burst;
}

void DDRBank::write(u32bit column, const DDRBurst* burst)
{
    GPU_ASSERT
    (   
        if ( column >= nColumns )
        {
            cout << "Column = " << column << " Total columns = " << nColumns << "\n";
            panic("DDRBank", "write", "Starting column out of bounds");
        }
        if ( activeRow == NoActiveRow )
            panic("DDRBank", "write", "A row must be active");
    )
    
    u32bit burstSize = burst->getSize();
    
    GPU_ASSERT
    (
        /*
        if ( column % burstSize != 0 )
        {
            cout << "Column = " << column << "  Current burst size = " << burstSize << "\n";
            panic("DDRBank", "write", "Starting column must be aligned to burst size");
        }
        */
        if ( burstSize + column > nColumns )
        {
cout << "DDRBank::write => column = " << column << " burstSize = " << burstSize << " nColumns = " << nColumns << endl;
            panic("DDRBank", "write", "Burst access out of row bounds");         
        }
    )
    
    for ( u32bit i = 0; i < burstSize; i++ )
        burst->write(i, data[activeRow][i+column]); // applies mask if required
}

u32bit DDRBank::rows() const
{
    return nRows;
}

u32bit DDRBank::columns() const
{
    return nColumns;
}

void DDRBank::dump(BankDumpFormat format) const
{
    cout << "Rows: " << nRows << " Columns: " << nColumns << "\n";
    if ( activeRow == NoActiveRow )
        cout << "[None active row]\n";
    cout << std::hex;
    for ( u32bit i = 0; i < nRows; i++ )
    {
        cout << i << ": ";
        for ( u32bit j = 0; j < nColumns; j++ )
        {
            const u8bit* ptr = (const u8bit*)&data[i][j];
            if ( format == DDRBank::hex )
            {
                cout << setfill('0') << setw(2) << u32bit(ptr[0]) 
                     << setfill('0') << setw(2) << u32bit(ptr[1]) 
                     << setfill('0') << setw(2) << u32bit(ptr[2]) 
                     << setfill('0') << setw(2) << u32bit(ptr[3]);
            }
            else // format == DDRBank::txt
            {
                cout << static_cast<char>(*ptr);
                cout << static_cast<char>(*(ptr+1));
                cout << static_cast<char>(*(ptr+2));
                cout << static_cast<char>(*(ptr+3));
            }
            if ( j < nColumns - 1 )
                cout << ".";
        }
        if ( activeRow == i )
            cout << " <--";
        cout << "\n";
    }
    cout << std::dec << endl;
}

void DDRBank::setBytes(u8bit byte)
{
    for ( u32bit i = 0; i < nRows; i++ )
    {
        for ( u32bit j = 0; j < nColumns; j++ )
        {
            u8bit* p = (u8bit *)&data[i][j];
            *p = byte;
            *(p+1) = byte;
            *(p+2) = byte;
            *(p+3) = byte;
        }
    }
}

void DDRBank::dumpRow(u32bit row, u32bit startCol, u32bit bytes) const
{
    if ( bytes == 0 )
        bytes = 4*(nColumns - startCol); // Whole row

    if ( row > nRows )
        panic("DDRBank", "dumpRow", "Row specified too high");
    if ( bytes % 4 != 0 )
        panic("DDRBank", "dumpRow", "bytes parameter must be multiple of 4");
    if ( startCol + bytes/4 > nColumns )
        panic("DDRBank", "dumpRow", "accessing out of row bounds");

    //cout << "row=" << row << " startCol=" << startCol << " bytes=" << bytes << " Data: ";

    const u8bit* r = (const u8bit*)data[row];
    r += (startCol*4);

    for ( u32bit i = 0; i < bytes; i++ )
        cout << std::hex << (u32bit)(r[i]) << std::dec << ".";
}



void DDRBank::readData(u32bit row, u32bit startCol, u32bit bytes, ostream& outStream) const
{
    GPU_ASSERT(
        if ( bytes % 4 != 0 )
            panic("DDRBank", "readData", "Number of bytes to be read has to be multiple of 4 (column size)");
        if ( row > nRows )
            panic("DDRBank", "readData", "Row specified too high");
        if ( bytes % 4 != 0 )
            panic("DDRBank", "readData", "bytes parameter must be multiple of 4");
        if ( startCol + bytes/4 > nColumns )
            panic("DDRBank", "readData", "accessing out of row bounds");
    )

    // write data from the bank into the output stream
    outStream.write((const char*)&data[row][startCol], bytes);
}

void DDRBank::writeData(u32bit row, u32bit startCol, u32bit bytes, std::istream& inStream)
{
    GPU_ASSERT(
        if ( bytes % 4 != 0 )
            panic("DDRBank", "writeData", "Number of bytes to be read has to be multiple of 4 (column size)");
        if ( row > nRows )
            panic("DDRBank", "writeData", "Row specified too high");
        if ( bytes % 4 != 0 )
            panic("DDRBank", "writeData", "bytes parameter must be multiple of 4");
        if ( startCol + bytes/4 > nColumns )
            panic("DDRBank", "writeData", "accessing out of row bounds");
    )

    // read data from stream and write it into the bank
    inStream.read((char*)&data[row][startCol], bytes);
}

