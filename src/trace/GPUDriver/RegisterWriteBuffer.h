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

#ifndef REGISTERWRITEBUFFER_H
    #define REGISTERWRITEBUFFER_H

#include <map>
#include "GPU.h"

class GPUDriver;

class RegisterWriteBuffer
{
public:

    enum WritePolicy
    {
        WaitUntilFlush, // Register writes are stored until calling 'flush()' [DEFAULT]
        Inmediate // Do not use the store buffer (forward data instantaneously)  
    };

    RegisterWriteBuffer(GPUDriver* driver, WritePolicy wp = WaitUntilFlush);
    void writeRegister(gpu3d::GPURegister reg, u32bit index, const gpu3d::GPURegData& data, u32bit md = 0);

    void readRegister(gpu3d::GPURegister reg, u32bit index, gpu3d::GPURegData& data);
    
    // Direct write (inmediate write, not buffered)
    void unbufferedWriteRegister(gpu3d::GPURegister reg, u32bit index, const gpu3d::GPURegData& data, u32bit md = 0);

    void flush();

    void setWritePolicy(WritePolicy wp);
    WritePolicy getWritePolicy() const;

    void initRegisterStatus (gpu3d::GPURegister reg, u32bit index, const gpu3d::GPURegData& data, u32bit md);

    void initAllRegisterStatus ();

    void dumpRegisterStatus (int frame, int batch);

    void dumpRegisterInfo(std::ofstream& out, gpu3d::GPURegister reg, u32bit index, const gpu3d::GPURegData& data, u32bit md);


private:

    u32bit registerWritesCount; // Statistic

    WritePolicy writePolicy;

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
    enum registerType{
        uintVal,
        intVal,
        qfVal,
        f32Val,
        booleanVal,
        status,
        faceMode,
        culling,
        primitive,
        streamData,
        compare,
        stencilUpdate,
        blendEquation,
        blendFunction,
        logicOp,
        txMode,
        txFormat,
        txCompression,
        txBlocking,
        txClamp,
        txFilter
    };

    struct RegisterData
    {
        gpu3d::GPURegData data;
        u32bit md;
        registerType dataType = {};

        RegisterData(gpu3d::GPURegData data_, u32bit md_ ) :
            data(data_), md(md_)
        {}
    };

    GPUDriver* driver;

    // id, value
    typedef std::map<RegisterIdentifier, RegisterData> WriteBuffer;
    typedef WriteBuffer::iterator WriteBufferIt;
    typedef WriteBuffer::const_iterator WriteBufferConstIt;
    
    WriteBuffer writeBuffer;

    WriteBuffer registerStatus;
};

#endif // REGISTERWRITEBUFFER_H
