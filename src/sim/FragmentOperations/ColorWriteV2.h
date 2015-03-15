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
 * $RCSfile: ColorWriteV2.h,v $
 * $Revision: 1.8 $
 * $Author: vmoya $
 * $Date: 2007-12-17 19:54:58 $
 *
 * Color Write box definition file.
 *
 */

/**
 *
 *  @file ColorWriteV2.h
 *
 *  This file defines the Color Write box.
 *
 *  This class implements the final color blend and color write
 *  stages of the GPU pipeline.
 *
 */


#ifndef _COLORWRITEV2_

#define _COLORWRITEV2_

#include "GPUTypes.h"
#include "support.h"
#include "Box.h"
#include "ColorCacheV2.h"
#include "RasterizerCommand.h"
#include "RasterizerState.h"
#include "GPU.h"
#include "FragmentInput.h"
#include "FragmentOpEmulator.h"
#include "GenericROP.h"
#include "ValidationInfo.h"

namespace gpu3d
{

/**
 *
 *  Defines the fragment map modes.
 *
 */
enum FragmentMapMode
{
    COLOR_MAP = 0,
    OVERDRAW_MAP = 1,
    FRAGMENT_LATENCY_MAP = 2,
    SHADER_LATENCY_MAP = 3,
    DISABLE_MAP = 0xff
};

/**
 *
 *  This class implements the simulation of the Color Blend,
 *  Logical Operation and Color Write stages of a GPU pipeline.
 *
 *  This class inherits from the Generic ROP class that implements a generic
 *  ROP pipeline
 *
 */

class ColorWriteV2 : public GenericROP
{
private:

    Signal *blockStateDAC;      /**<  Signal to send the color buffer block state to the DAC.  */

    /*  Color Write registers.  */
    TextureFormat colorBufferFormat;        /**<  Format of the color buffer.  */
    bool colorSRGBWrite;                    /**<  Flag to enable/disable conversion from linear to sRGB on color write.  */
    bool rtEnable[MAX_RENDER_TARGETS];      /**<  Flag to enable/disable a render target.  */
    TextureFormat rtFormat[MAX_RENDER_TARGETS]; /**<  Format of the render target.  */
    u32bit rtAddress[MAX_RENDER_TARGETS];       /**<  Base address of the render target.  */
    
    QuadFloat clearColor;       /**<  Current clear color for the framebuffer.  */
    bool blend[MAX_RENDER_TARGETS];                 /**<  Flag storing if color blending is active.  */
    BlendEquation equation[MAX_RENDER_TARGETS];     /**<  Current blending equation mode.  */
    BlendFunction srcRGB[MAX_RENDER_TARGETS];       /**<  Source RGB weight funtion.  */
    BlendFunction dstRGB[MAX_RENDER_TARGETS];       /**<  Destination RGB weight function.  */
    BlendFunction srcAlpha[MAX_RENDER_TARGETS];     /**<  Source alpha weight function.  */
    BlendFunction dstAlpha[MAX_RENDER_TARGETS];     /**<  Destination alpha weight function.  */
    QuadFloat constantColor[MAX_RENDER_TARGETS];    /**<  Blend constant color.  */
    bool writeR[MAX_RENDER_TARGETS];                /**<  Write mask for red color component.  */
    bool writeG[MAX_RENDER_TARGETS];                /**<  Write mask for green color component.  */
    bool writeB[MAX_RENDER_TARGETS];                /**<  Write mask for blue color component.  */
    bool writeA[MAX_RENDER_TARGETS];                /**<  Write mask for alpha color channel.  */
    bool logicOperation;        /**<  Flag storing if logical operation is active.  */
    LogicOpMode logicOpMode;    /**<  Current logic operation mode.  */
    u32bit frontBuffer;         /**<  Address in the GPU memory of the front (primary) buffer.  */
    u32bit backBuffer;          /**<  Address in the GPU memory of the back (secondary) buffer.  */

    u8bit clearColorData[MAX_BYTES_PER_COLOR];      /**< Color clear value converted to the color buffer format.  */

    /*  Color Write parameters.  */
    bool disableColorCompr;     /**<  Disables color compression.  */
    u32bit colorCacheWays;      /**<  Color cache set associativity.  */
    u32bit colorCacheLines;     /**<  Number of lines in the color cache per way/way.  */
    u32bit stampsLine;          /**<  Number of stamps per color cache line.  */
    u32bit cachePortWidth;      /**<  Width of the color cache ports in bytes.  */
    bool extraReadPort;         /**<  Add an additional read port to the color cache.  */
    bool extraWritePort;        /**<  Add an additional write port to the color cache.  */
    u32bit blocksCycle;         /**<  State of color block copied to the DAC per cycle.  */
    u32bit blockStateLatency;   /**<  Latency of the block state update signal to the DAC.  */
    u32bit fragmentMapMode;     /**<  Fragment map mode.  */
    u32bit numStampUnits;       /**<  Number of stamp units in the GPU pipeline.  */
    
    /*  Color Write state.  */
    RasterizerCommand *lastRSCommand;   /**<  Stores the last Rasterizer Command received (for signal tracing).  */

    /*  Color cache.  */
    ColorCacheV2 *colorCache;   /**<  Pointer to the color cache/write buffer.  */
    u32bit bytesLine;           /**<  Bytes per color cache line.  */
    u32bit lineShift;           /**<  Logical shift for a color cache line.  */
    u32bit lineMask;            /**<  Logical mask for a color cache line.  */

    /*  Color cache blend output color data array.  */
    u8bit *outColor;            /**<  Stores blended color data before logic op & write.  */

    /*  Color Clear state.  */
    u32bit copyStateCycles;     /**<  Number of cycles remaining for the copy of the block state memory.  */

    /*  Statistics.  */
    GPUStatistics::Statistic *inputs;       /**<  Input fragments.  */
    GPUStatistics::Statistic *blended;      /**<  Blended fragments.  */
    GPUStatistics::Statistic *logoped;      /**<  Logical operation fragments.  */

    /*  Latency map.  */
    u32bit *latencyMap;                     /**<  Stores the fragment latency map for the Color Write unit.  */
    bool clearLatencyMap;                   /**<  Clear the latency map.  */

    //  Debug/validation.
    bool validationMode;                /**<  Flag that stores if the validation mode is enabled in the emulator.  */
    FragmentQuadMemoryUpdateMap colorMemoryUpdateMap[MAX_RENDER_TARGETS];   /**<  Map indexed by fragment identifier storing updates to the color buffer.  */

    //  Debug/Log.
    bool traceFragment;     /**<  Flag that enables/disables a trace log for the defined fragment.  */
    u32bit watchFragmentX;  /**<  Defines the fragment x coordinate to trace log.  */
    u32bit watchFragmentY;  /**<  Defines the fragment y coordinate to trace log.  */

    /*  Private functions.  */

    /**
     *
     *  Processes a rasterizer command.
     *
     *  @param command The rasterizer command to process.
     *  @param cycle Current simulation cycle.
     *
     */

    void processCommand(RasterizerCommand *command, u64bit cycle);

    /**
     *
     *  Processes a register write.
     *
     *  @param reg The Interpolator register to write.
     *  @param subreg The register subregister to writo to.
     *  @param data The data to write to the register.
     *
     */

    void processRegisterWrite(GPURegister reg, u32bit subreg, GPURegData data);

    /**
     *
     *  Processes a memory transaction.
     *
     *  @param cycle The current simulation cycle.
     *  @param memTrans The memory transaction to process
     *
     */

    void processMemoryTransaction(u64bit cycle, MemoryTransaction *memTrans);

    /**
     *
     *  Performs any remaining tasks after the stamp data has been written.
     *  The function should read, process and remove the stamp at the head of the
     *  written stamp queue.
     *
     *  @param cycle Current simulation cycle.
     *  @param stamp Pointer to a stamp that has been written to the ROP data buffer
     *  and needs processing.
     *
     */

    void postWriteProcess(u64bit cycle, ROPQueue *stamp);


    /**
     *
     *  To be called after calling the update/clock function of the ROP Cache.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void postCacheUpdate(u64bit cycle);

    /**
     *
     *  This function is called when the ROP stage is in the the reset state.
     *
     */

    void reset();

    /**
     *
     *  This function is called when the ROP stage is in the flush state.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void flush(u64bit cycle);
    
    /**
     *
     *  This function is called when the ROP stage is in the swap state.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void swap(u64bit cycle);

    /**
     *
     *  This function is called when the ROP stage is in the clear state.
     *
     */

    void clear();

    /**
     *
     *  This function is called when a stamp is received at the end of the ROP operation
     *  latency signal and before it is queued in the operated stamp queue.
     *
     *  @param cycle Current simulation cycle.
     *  @param stamp Pointer to the Z queue entry for the stamp that has to be operated.
     *
     */

    void operateStamp(u64bit cycle, ROPQueue *stamp);

    /**
     *
     *  This function is called when all the stamps but the last one have been processed.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void endBatch(u64bit cycle);

    /**
     *
     *  Converts an array of color values in RGBA8 format to RGBA32F format.
     *
     *  @param in The input color array in RGBA8 format.
     *  @param out The output color array in RGBA32F format.
     *
     */

    void colorRGBA8ToRGBA32F(u8bit *in, QuadFloat *out);

    /**
     *
     *  Converts an array of color values in RGBA32F format to RGBA8 format.
     *
     *  @param in The input color array in RGBA32F format.
     *  @param out The output color array in RGBA8 format.
     *
     */

    void colorRGBA32FToRGBA8(QuadFloat *in, u8bit *out);

    /**
     *
     *  Converts an array of color values in RGBA16F format to RGBA32F format.
     *
     *  @param in The input color array in RGBA16F format.
     *  @param out The output color array in RGBA32F format.
     *
     */

    void colorRGBA16FToRGBA32F(u8bit *in, QuadFloat *out);

    /**
     *
     *  Converts an array of color values in RGBA32F format to RGBA16F format.
     *
     *  @param in The input color array in RGBA32F format.
     *  @param out The output color array in RGBA16F format.
     *
     */

    void colorRGBA32FToRGBA16F(QuadFloat *in, u8bit *out);

    /**
     *
     *  Converts an array of color values in RGBA16 format to RGBA32F format.
     *
     *  @param in The input color array in RGBA16 format.
     *  @param out The output color array in RGBA32F format.
     *
     */

    void colorRGBA16ToRGBA32F(u8bit *in, QuadFloat *out);

    /**
     *
     *  Converts an array of color values in RGBA32F format to RGBA16 format.
     *
     *  @param in The input color array in RGBA32F format.
     *  @param out The output color array in RGBA16 format.
     *
     */

    void colorRGBA32FToRGBA16(QuadFloat *in, u8bit *out);

    /**
     *
     *  Converts an array of color values in RG16F format to RGBA32F format.
     *
     *  @param in The input color array in RG16F format.
     *  @param out The output color array in RGBA32F format.
     *
     */

    void colorRG16FToRGBA32F(u8bit *in, QuadFloat *out);

    /**
     *
     *  Converts an array of color values in RGBA32F format to RG16F format.
     *
     *  @param in The input color array in RGBA32F format.
     *  @param out The output color array in RG16F format.
     *
     */

    void colorRGBA32FToRG16F(QuadFloat *in, u8bit *out);

    /**
     *
     *  Converts an array of color values in R32F format to RGBA32F format.
     *
     *  @param in The input color array in R32F format.
     *  @param out The output color array in RGBA32F format.
     *
     */

    void colorR32FToRGBA32F(u8bit *in, QuadFloat *out);

    /**
     *
     *  Converts an array of color values in RGBA32F format to R32F format.
     *
     *  @param in The input color array in RGBA32F format.
     *  @param out The output color array in R32F format.
     *
     */

    void colorRGBA32FToR32F(QuadFloat *in, u8bit *out);

    /**
     *
     *  Converts an array of 32-bit float point color values from linear to sRGB space.
     *
     *  @param in A pointer to the array of color values.
     *
     */
     
    void colorLinearToSRGB(QuadFloat *in);

    /**
     *
     *  Converts an array of 32-bit float point color values from sRGB to linear space.
     *
     *  @param in A pointer to the array of color values.
     *
     */
     
    void colorSRGBToLinear(QuadFloat *in);

public:

    /**
     *
     *  Color Write box constructor.
     *
     *  Creates and initializes a new Color Write box object.
     *
     *  @param stampsCycle Number of stamps per cycle.
     *  @param overW Over scan tile width in scan tiles (may become a register!).
     *  @param overH Over scan tile height in scan tiles (may become a register!).
     *  @param scanW Scan tile width in fragments.
     *  @param scanH Scan tile height in fragments.
     *  @param genW Generation tile width in fragments.
     *  @param genH Generation tile height in fragments.
     *  @param disableColorCompr Disables color compression.
     *  @param cacheWays Color cache set associativity.
     *  @param cacheLines Number of lines in the color cache per way/way.
     *  @param stampsLine Numer of stamps per color cache line (less than a tile!).
     *  @param portWidth Width of the color cache ports in bytes.
     *  @param extraReadPort Adds an extra read port to the color cache.
     *  @param extraWritePort Adds an extra write port to the color cache.
     *  @param cacheReqQueueSize Size of the color cache memory request queue.
     *  @param inputRequests Number of read requests and input buffers in the color cache.
     *  @param outputRequests Number of write requests and output buffers in the color cache.
     *  @param numStampUnits Number of stamp units in the GPU.
     *  @param maxBlocks Maximum number of sopported color blocks in the olor cache state memory.
     *  @param blocksCycle Number of state block entries that can be modified (cleared), read and sent per cycle.
     *  @param compCycles Color block compression cycles.
     *  @param decompCycles Color block decompression cycles.
     *  @param colorInQSz Color input stamp queue size (entries/stamps)
     *  @param colorFetchQSz Color fetched stamp queue size (entries/stamps)
     *  @param colorReadQSz Color read stamp queue size (entries/stamps)
     *  @param colorOpQSz Color operated stamp queue size (entries/stamps)
     *  @param writeOpQSz Color written stamp queue size (entries/stamps)
     *  @param blendRate Rate at which stamps are blended (cycles between stamps).
     *  @param blendLatency Blending latency.
     *  @param blockStateLatency Latency to send the color block state to the DAC.
     *  @param fragMapMode Fragment Map Mode.
     *  @param frEmu Reference to the Fragment Operation Emulator object.
     *  @param name The box name.
     *  @param prefix String used to prefix the box signals names.
     *  @param parent The box parent box.
     *
     *  @return A new Color Write object.
     *
     */

    ColorWriteV2(u32bit stampsCycle, u32bit overW, u32bit overH, u32bit scanW, u32bit scanH,
        u32bit genW, u32bit genH,
        bool disableColorCompr, u32bit cacheWays, u32bit cacheLines, u32bit stampsLine,
        u32bit portWidth, bool extraReadPort, bool extraWritePort, u32bit cacheReqQueueSize,
        u32bit inputRequests, u32bit outputRequests, u32bit numStampUnits, u32bit maxBlocks, u32bit blocksCycle,
        u32bit compCycles, u32bit decompCycles,
        u32bit colorInQSz, u32bit colorFetchQSz, u32bit colorReadQSz, u32bit colorOpQSz,
        u32bit colorWriteQSz,
        u32bit blendRate, u32bit blendLatency, u32bit blockStateLatency,
        u32bit fragMapMode, FragmentOpEmulator &frOp,
        char *name, char *prefix = 0, Box* parent = 0);

    /**
     *
     *  Get pointer to the fragment latency map.
     *
     *  @param width Width of the latency map in stamps.
     *  @param height Height of the latency map in stamps.
     *
     *  @return A pointer to the latency map stored in the Color Write unit.
     *
     */

    u32bit *getLatencyMap(u32bit &width, u32bit &height);

    /**
     *
     *  Saves the block state memory into a file.
     *
     */
     
    void saveBlockStateMemory();
    
    /**
     *
     *  Loads the block state memory from a file.
     *
     */
     
    void loadBlockStateMemory();

    /**
     *
     *  Get the list of debug commands supported by the Color Write box.
     *
     *  @param commandList Reference to a string variable where to store the list debug commands supported
     *  by the Color Write box.
     *
     */
     
    void getCommandList(std::string &commandList);

    /**
     *
     *  Executes a debug command on the Color Write box.
     *
     *  @param commandStream A reference to a stringstream variable from where to read
     *  the debug command and arguments.
     *
     */    
     
    void execCommand(stringstream &commandStream);

    /** 
     *
     *  Set Color Write unit validation mode.
     *
     *  @param enable Boolean value that defines if the validation mode is enabled.
     *
     */
     
    void setValidationMode(bool enable);

    /**
     *
     *  Get the color memory update map of the defined render target for the current draw call.
     *
     *  @param rt Idenfier for the render target for which to obtain the color memory update map.
     *
     *  @return A reference to the color memory update map of the defined render target for the current draw call.
     *
     */
  
    FragmentQuadMemoryUpdateMap &getColorUpdateMap(u32bit rt);

};

} // namespace gpu3d

#endif

