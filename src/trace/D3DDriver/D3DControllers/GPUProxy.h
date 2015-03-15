#ifndef GPUPROXY_H
#define GPUPROXY_H

enum GPUProxyRegisterType {
        RT_U32BIT,
        RT_S32BIT,
        RT_F32BIT,
        RT_U8BIT,
        RT_BOOL,
        RT_QUADFLOAT,
        RT_GPU_STATUS,
        RT_FACE_MODE,
        RT_CULLING_MODE,
        RT_PRIMITIVE_MODE,
        RT_STREAM_DATA,
        RT_COMPARE_MODE,
        RT_STENCIL_UPDATE_FUNCTION,
        RT_BLEND_EQUATION,
        RT_BLEND_FUNCTION,
        RT_LOGIC_OP_MODE,
        RT_TEXTURE_MODE,
        RT_TEXTURE_FORMAT,
        RT_TEXTURE_COMPRESSION,
        RT_TEXTURE_BLOCKING,
        RT_CLAMP_MODE,
        RT_FILTER_MODE,
        RT_ADDRESS
};

struct GPUProxyRegId {
    gpu3d::GPURegister gpu_reg;
    u32bit index;
    bool operator()(const GPUProxyRegId &a, const GPUProxyRegId &b) const;
    friend bool operator<(const GPUProxyRegId &a, const GPUProxyRegId &b)
    {
        if((unsigned int)(a.gpu_reg) < (unsigned int)(b.gpu_reg))
            return true;
        else if((unsigned int)(a.gpu_reg) > (unsigned int)(b.gpu_reg))
            return false;
        else {
            return (a.index < b.index);
        }
    }

    friend bool operator==(const GPUProxyRegId &a, const GPUProxyRegId &b)
    {
        return ((a.gpu_reg == b.gpu_reg) && (a.index == b.index));
    }

    GPUProxyRegId();
    GPUProxyRegId(gpu3d::GPURegister _gpu_reg, u32bit _index = 0);
};

struct GPUProxyAddress {
    u32bit md;
    u32bit offset;
    GPUProxyAddress();
    GPUProxyAddress(u32bit md, u32bit offset = 0);
};


/** @note You must read/write address registers with read/writeGPUAddrRegister except for setting them to 0.
		  Simetrically address registers must be read/written with read/writeGPURegister.
		  This ensures the values cached for reading registers are consistent */
class GPUProxy {
    public:
        void getGPUParameters(u32bit& gpuMemSz, u32bit& sysMemSz, u32bit& blocksz, u32bit& sblocksz,
        u32bit& scanWidth, u32bit& scanHeight, u32bit& overScanWidth, u32bit& overScanHeight,
        bool& doubleBuffer, u32bit& fetchRate) const;
        void writeGPURegister( gpu3d::GPURegister regId, gpu3d::GPURegData data);
        void writeGPURegister( gpu3d::GPURegister regId, u32bit index, gpu3d::GPURegData data);
        void writeGPUAddrRegister( gpu3d::GPURegister regId, u32bit index, u32bit md, u32bit offset = 0);
        void readGPURegister( gpu3d::GPURegister regId, gpu3d::GPURegData* data);
        void readGPURegister( gpu3d::GPURegister regId, u32bit index, gpu3d::GPURegData* data);
        void readGPUAddrRegister( gpu3d::GPURegister regId, u32bit index, u32bit* md, u32bit* offset);
        void sendCommand(gpu3d::GPUCommand com);
        void initBuffers(u32bit* mdFront = 0, u32bit* mdBack = 0, u32bit* mdZS = 0);
        void setResolution(u32bit width, u32bit height);
        /**@todo Add parameter MemoryRequestPolicy memRequestPolicy = GPUMemoryFirst,
                 now is ommited because it gives a build error. */
        u32bit obtainMemory( u32bit sizeBytes);
        void releaseMemory( u32bit md );
        bool writeMemory( u32bit md, const u8bit* data, u32bit dataSize, bool isLocked = false);
        bool writeMemory( u32bit md, u32bit offset, const u8bit* data, u32bit dataSize, bool isLocked = false );

        bool commitVertexProgram( u32bit memDesc, u32bit programSize, u32bit startPC );
        bool commitFragmentProgram( u32bit memDesc, u32bit programSize, u32bit startPC );

        void debug_print_registers();

        static GPUProxy* get_instance();
    private:
        map<GPUProxyRegId, GPURegData, GPUProxyRegId> values;
        map<GPUProxyRegId, GPUProxyAddress, GPUProxyRegId> addresses;
        void debug_print_register(gpu3d::GPURegister reg, u32bit index = 0);

        map<GPUProxyRegId, GPUProxyRegisterType, GPUProxyRegId> register_types;

        map<gpu3d::GPURegister, string> register_names;

        GPUProxy();
        ~GPUProxy();
};



#endif // GPUPROXY_H
