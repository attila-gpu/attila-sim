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

#include "GPU.h"
#include "zfstream.h"

#include <string>

using namespace std;
using namespace gpu3d;

namespace gpu3d
{
struct SimParameters;
class AGPTransaction;
}

#ifndef GL_VERTEX_PROGRAM_ARB
    #define GL_VERTEX_PROGRAM_ARB 0x8620
#endif

#ifndef GL_FRAGMENT_PROGRAM_ARB
    #define GL_FRAGMENT_PROGRAM_ARB 0x8804
#endif

class ALLAInterface
{
private:

    gzofstream outFile;
    SimParameters *simP;

    bool verbose;
    
    void writeAGPTransaction(AGPTransaction *agpt);

public:

    ALLAInterface();

    void initialize(bool verbose = false);
    void createTraceFile(char *traceName);
    void closeTraceFile();

    void writeGPURegister(GPURegister reg, u32bit subreg, bool data);
    void writeGPURegister(GPURegister reg, u32bit subreg, u32bit data);
    void writeGPURegister(GPURegister reg, u32bit subreg, s32bit data);
    void writeGPURegister(GPURegister reg, u32bit subreg, f32bit data);
    void writeGPURegister(GPURegister reg, u32bit subreg, f32bit *data);
    void writeGPURegister(GPURegister reg, u32bit subreg, TextureFormat data);
    void writeGPUAddrRegister(GPURegister reg, u32bit subreg, u32bit data, u32bit mdID);

    void writeBuffer(u32bit address, u32bit size, u8bit *data, u32bit mdID);

    void compileARBProgram(u32bit target, char *program, u32bit size, u8bit *code, u32bit &codeSize);
    bool assembleProgramFromText(char *program, u8bit *code, u32bit &codeSize, string &errorString);
    bool assembleProgramFromFile(char *filename, u8bit *code, u32bit &codeSize, string &errorString);
    void loadVertexProgram(u32bit address, u32bit pc, u32bit size, u8bit *code, u32bit resources, u32bit mdID);
    void loadFragmentProgram(u32bit address, u32bit pc, u32bit size, u8bit *code, u32bit resources, u32bit mdID);

    
    void resetVertexOutputAttributes();
    void resetFragmentInputAttributes();

    void setResolution(u32bit w, u32bit h);
    void setViewport(s32bit x, s32bit y, u32bit w, u32bit h);
    void setScissor(s32bit x, s32bit y, u32bit w, u32bit h);

    void resetStreamer();
    void setAttributeStream(u32bit stream, u32bit attr, u32bit address, u32bit stride, StreamData format, u32bit elements);

    void miscReset();

    void resetTextures();
    void enableTexture(u32bit textureID);
    void disableTexture(u32bit textureID);
    void setTexture(u32bit texID, TextureMode texMode, u32bit address, u32bit width, u32bit height,
                    u32bit widthlog2, u32bit heightlog2, TextureFormat texFormat, TextureBlocking texBlockMode,
                    ClampMode texClampS, ClampMode texClampT, FilterMode texMinFilter, FilterMode texMagFilter,
                    bool invertV);

    void draw(PrimitiveMode primitive, u32bit start, u32bit count);

    void swap();

    void reset();

    void clearColorBuffer();
    void clearZStencilBuffer();

    void flushColorBuffer();
    void flushZStencilBuffer();

    void saveColorState();
    void restoreColorState();
    void saveZStencilState();
    void restoreZStencilState();
    void resetColorState();
    void resetZStencilState();

};
