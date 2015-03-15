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
 * $RCSfile: RegisterWriteBufferAGP.h,v $
 * $Revision: 1.2 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:58 $
 *
 * RegisterWriteBufferAGP definition file
 *
 */
 
 
/**
 *
 *  @file RegisterWriteBufferAGP.h
 *
 *  This file contains the definitions of a GPU register write buffer class used by the extractTraceRegion tool to
 *  store register writes until the start of the region to extract from the original AGP transaction trace.
 *
 */

#ifndef _REGISTERWRITEBUFFERAGP_
    #define _REGISTERWRITEBUFFERAGP_

#include <map>
#include "GPU.h"

class RegisterWriteBufferAGP
{
public:

    RegisterWriteBufferAGP();

    void writeRegister(gpu3d::GPURegister reg, u32bit index, const gpu3d::GPURegData& data, u32bit md = 0);

    bool readRegister(gpu3d::GPURegister reg, u32bit index, gpu3d::GPURegData &data, u32bit &md);
    
    bool flushNextRegister(gpu3d::GPURegister &reg, u32bit &index, gpu3d::GPURegData &data, u32bit &md);

private:

    u32bit registerWritesCount; // Statistic

    struct RegisterIdentifier
    {
        gpu3d::GPURegister reg;
        u32bit index;

        RegisterIdentifier(gpu3d::GPURegister reg_, u32bit index_) :
            reg(reg_), index(index_)
        {}
        
        friend bool operator<(const RegisterIdentifier& lv,
                              const RegisterIdentifier& rv)
        {
            if ( lv.reg == rv.reg )
                return lv.index < rv.index;
                
            return lv.reg < rv.reg;
        }

        friend bool operator==(const RegisterIdentifier& lv, 
                               const RegisterIdentifier& rv)
        {
            return (lv.reg == rv.reg && lv.index == rv.index );
        }
    };
    
    struct RegisterData
    {
        gpu3d::GPURegData data;
        u32bit md;

        RegisterData(gpu3d::GPURegData data_, u32bit md_) :
            data(data_), md(md_)
        {}
        
    };

    // id, value
    typedef std::map<RegisterIdentifier, RegisterData> WriteBuffer;
    typedef WriteBuffer::iterator WriteBufferIt;
    typedef WriteBuffer::const_iterator WriteBufferConstIt;
    
    WriteBuffer writeBuffer;
};

#endif // REGISTERWRITEBUFFER_H
