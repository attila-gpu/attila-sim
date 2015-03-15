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

#ifndef PROGRAMOBJECT_H
    #define PROGRAMOBJECT_H

#include "gl.h"
#include "RBank.h"
#include "BaseObject.h"
#include "ImplementationLimits.h"
#include <string>

namespace libgl
{

class ProgramTarget;

class ProgramObject : public BaseObject
{

public:

    static const int MaxProgramAttribs = MAX_PROGRAM_ATTRIBS_ARB;
    static const unsigned int MaxTextureUnits = MAX_TEXTURE_UNITS_ARB;


    class ProgramResources
    {
        public:
        
            GLuint numberOfInstructions;
            GLuint numberOfParamRegs;
            GLuint numberOfTempRegs;
            GLuint numberOfAddrRegs;
            GLuint numberOfAttribs;
            bool outputAttribsWritten[MaxProgramAttribs];
            bool inputAttribsRead[MaxProgramAttribs];
            GLuint maxAliveTemps;

            GLenum textureTargets[MaxTextureUnits];

    };
    
    enum { maxSourceLength = 16384*16 };
    
    
    //static const GLsizei maxSourceLength = 16384;

private:
    
    RBank<float> localParams;
    
    RBank<float> clusterBank;

    GLubyte* source;
    GLubyte* binary;
    mutable GLubyte* assembler;
    mutable GLuint assemblerSz;
    GLsizei sourceSize;
    GLsizei binSize;
    GLenum format;
    GLuint gpuAddr;
    ProgramObject::ProgramResources pr; 

    bool driverTranslated;  /**< Stores if the program has been translated/transformed by the driver.  */

    /*  Tables used for writing the instruction.  */

    /*  Shader Opcode to string.  */

    static char *shOpcode2Str[];
    static char *maskMode2Str[];
    static char *ccMode2Str[];
    static char swizzleMode2Str[];
    static char *bank2Str[];

    unsigned int numOperands(unsigned int Opcode) const;

    void _computeASMCode() const;

public:

    /* raw creation */
    //ProgramObject();
    
    GLuint binarySize(GLuint portion = 0);
    const GLubyte* binaryData(GLuint portion = 0);
    
    ProgramObject(GLenum name, GLenum targetName);
                
    void setSource(const GLvoid* program, GLsizei size);

    // return false if the source cannot be returned bacause there is not enough space in the receiver buffer
    // or due to the source has not been set
    bool getSource(GLubyte* program, GLsizei& size) const;
    
    // return the empty string if the source has not been set yet
    std::string getSource() const; // new version

    void setBinary(const GLubyte* binary, GLsizei size);

    GLuint getBinarySize() const;
    
    GLuint getSourceSize() const;
    

    bool getBinary(GLubyte* program, GLsizei& size) const;

    GLenum getFormat() const { return format; }
    
    void setFormat(GLenum format);
    
    void getASMCode(GLubyte *assembler, GLsizei& size) const;

    /**
     * for checking & updating resources
     */
    ProgramResources& getResources();
    
    /**
     * Compiles program only if needed (source code changed)
     */
    bool compile();

    bool isCompiled();
    
    void setAsTranslated();

    bool isTranslated();
     
    /* zero if it is not in GPU local memory */
    GLuint getGPUAddr() const;

    RBank<float>& getLocalParams();

    RBank<float>& getEnvParams();

    RBank<float>& getClusterBank();

    void releaseGPUResources();
    
    void printSource() const;
    
    void printBinary() const;
    
    void printASM() const;
    
    const char* getStringID() const;
        
    ~ProgramObject();
    
};

} // namespace libgl


#endif // PROGRAMOBJECT_H
