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



#ifndef _AGPTRACEDRIVER_
    #define _AGPTRACEDRIVER_

#include "GPUTypes.h"
#include "AGPTransaction.h"
#include "TraceDriverInterface.h"
#include "GLExec.h"
#include "zfstream.h"
#include "RegisterWriteBufferAGP.h"
#include <cstring>

/**
 *
 *  AGP Trace Driver class.
 *
 *  Generates AGP transactions to the GPU from an AGP transaction trace file.
 *
 */
 
class AGPTraceDriver : public TraceDriverInterface
{

private:

    int initValue; ///< 0 if creation was ok, < 0 value if not
    u32bit startFrame;
    u32bit traceFirstFrame;
    u32bit currentFrame;
    gzifstream *agpTraceFile;
    
    enum TracePhase
    {
        TP_PREINIT,             //  Preinitialization phase.  Skip draw, blit and swap commands.
        TP_LOAD_REGS,           //  Register load phase.  Load the cached GPU register writes into the GPU.
        TP_LOAD_SHADERS,        //  Shader program load phase.  Load the cached shader programs into the GPU.
        TP_CLEARZSTBUFFER,      //  Clear z and stencil buffer phase.
        TP_CLEARCOLORBUFFER,    //  Clear color buffer phase.
        TP_END_INIT,            //  End of initialization phase.
        TP_SIM_START_EVENT,     //  Send reset event phase.
        TP_SIMULATION           //  Simulation phase.
    };
    
    TracePhase currentPhase;        ///<  Stores the current phase in the processing of the AGP Transaction trace.
    
    u32bit shaderProgramLoadPhase;
    
    RegisterWriteBufferAGP registerCache;

    u32bit startTransaction;
    u32bit agpTransCount;       ///<  Counter for the number of processed agp transactions.
    
    static const u32bit SHADERINSTRUCTIONSIZE = 16;     ///<  Size in bytes of a shader instruction.
    static const u32bit MAXSHADERINSTRUCTIONS = 512;    ///<  Number of shaders instructions that can be stored in shader program memory.
    
    u8bit fragProgramCache[SHADERINSTRUCTIONSIZE * MAXSHADERINSTRUCTIONS];  ///<  Caches the fragment shader program data for skipped frames.
    u8bit vertProgramCache[SHADERINSTRUCTIONSIZE * MAXSHADERINSTRUCTIONS];  ///<  Caches the vertex shader program data for skipped frames.
    
    /**
     *
     *  This structure stores data uploaded to GPU memory that may contain shader program data.
     *
     */
     
    struct ProgramUpload
    {
        u32bit address;
        u32bit size;
        u8bit *data;
        u32bit md;
        u32bit agpTransID;
        
        ///<  Creates a program upload object.
        ProgramUpload(u32bit addr, u32bit sz, u8bit *data_, u32bit md_, u32bit agpID) :
            address(addr), size(sz), md(md_), agpTransID(agpID)
        {
            if ((data_ != NULL) && (size > 0))
            {
                data = new u8bit[size];
                memcpy(data, data_, size);
            }
            else
                data = NULL;
        }
        
        ///<  Updates the data associated with a program upload.
        void updateData(u8bit *newData, u32bit newSize, u32bit agpID)
        {
            delete[] data;
            
            agpTransID = agpID;
            
            if ((newData != NULL) && (size > 0))
            {
                size = newSize;
                data = new u8bit[size];
                memcpy(data, newData, size);
            }
            else
            {
                size = 0;
                data = NULL;
            }            
        }
        
        //  Destroys the program upload.
        ~ProgramUpload()
        {
            delete[] data;
        }
    };
    
    typedef std::map<u32bit, ProgramUpload*>::iterator ProgramUploadsIterator;
    
    std::map<u32bit, ProgramUpload*> programUploads;    ///<  Stores all the shader program uploads. 
    
    ProgramUpload *lastProgramUpload;   ///<  Stores the last data upload that may contain shader program data.
        
    u32bit fragmentProgramPC;           ///<  Stores the fragment program PC GPU register.
    u32bit fragmentProgramAddress;      ///<  Stores the fragment program address GPU register.
    u32bit fragmentProgramSize;         ///<  Stores the fragment program size GPU register.

    u32bit vertexProgramPC;             ///<  Stores the vertex program PC GPU register.
    u32bit vertexProgramAddress;        ///<  Stores the vertex program address GPU register.
    u32bit vertexProgramSize;           ///<  Stores the vertex program size GPU register.
    
    
public:

    /**
     *
     *  AGP Trace Driver Constructor
     *
     *  Creates an AGP Trace Driver object and initializes it.  Trace Driver for AGP transaction trace file.
     *
     *  @param traceFile Pointer to a compressed input stream for the AGP transaction file.
     *  @param startFrame Start simulation frame.  The Trace Driver won't send DRAW or SWAP commands until
     *  the start frame is reached. 
     *  @param traceFirstFrame First frame in the AGP transaction tracefile.
     *
     *  @return  An initialized AGP Trace Driver object.
     *
     */
     
    AGPTraceDriver(gzifstream *traceFile, u32bit startFrame, u32bit traceFirstFrame);
     
    
    /**
     * Starts the trace driver.
     *
     * Verifies if a AGP TraceDriver object is correctly created and available for use
     *
     * @return  0 if all is ok.
     *
     */
     
    int startTrace();

    /**
     *
     *  Generates the next AGP transaction from the API trace file.
     *
     *  @return A pointer to the new AGP transaction, NULL if there are no more AGP transactions.
     *
     */
    gpu3d::AGPTransaction* nextAGPTransaction();

    /**
     *
     *  Returns the current position (agp transaction count) inside the trace file.
     *
     *  @return The current position (agp transaction count) inside the trace file.
     *
     */    
     
    u32bit getTracePosition();

    /**
     *
     *  Saves in a file the current trace position.
     *
     *  @param f Pointer to a file stream object where to save the current trace position.
     *
     */
     
    void saveTracePosition(std::fstream *f);

    /**
     *
     *  Loads from a file a start trace position.
     *
     *  @param f Pointer to a file stream object from where to load a start trace position.
     *
     */
     
    void loadStartTracePosition(std::fstream *f);
};


#endif
