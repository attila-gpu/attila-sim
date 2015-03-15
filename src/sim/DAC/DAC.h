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
 * $RCSfile: DAC.h,v $
 * $Revision: 1.19 $
 * $Author: vmoya $
 * $Date: 2008-03-02 19:09:17 $
 *
 * DAC box definition file.
 *
 */


/**
 *
 *  @file DAC.h
 *
 *  This file defines the DAC box.
 *
 *  This class implements the DAC unit in a GPU unit.
 *  Currently just consumes memory and outputs a file.
 *
 */

#include "GPUTypes.h"
#include "Box.h"
#include "GPU.h"
#include "RasterizerState.h"
#include "RasterizerCommand.h"
//#include "MemoryController.h"
#include "ColorCacheV2.h"
#include "Blitter.h"
#include "PixelMapper.h"
#include "CompressorEmulator.h"


#ifndef _DAC_

#define _DAC_

namespace gpu3d
{

/**
 *
 *  Defines a block request to memory.
 *
 */

struct BlockRequest
{
    u32bit address;         /**<  Address in memory of the block to request.  */
    u32bit block;           /**<  Block identifier.  */
    ROPBlockState state;    /**<  State (compression) of the block.  */
    u32bit size;            /**<  Size of the block in memory.  */
    u32bit requested;       /**<  Block bytes requested to memory.  */
    u32bit received;        /**<  Block bytes received from memory.  */
};

/**
 *
 *  This class implements a DAC box.
 *
 *  The DAC converts the color buffer to signals for the display
 *  device (LCD, CRT, ...) in a real hardware.  The simulation box
 *  just consumes GPU memory bandwidth and stores the current
 *  colorbuffer in a file when the buffers are swapped.
 *
 *  Inherits from the Box class that offers basic simulation
 *  support.
 *
 */

class DAC : public Box
{
private:

    /*  DAC signals.  */
    Signal *dacCommand;         /**<  Command signal from the Command Processor.  */
    Signal *dacState;           /**<  State signal to the Command Processor.  */
    Signal *memoryRequest;      /**<  Request signal to the Memory Controller.  */
    Signal *memoryData;         /**<  Data signal from the Memory Controller.  */
    Signal **blockStateCW;      /**<  Array of signal from the Color Write units with the information about the color buffer block states (compressed, cleared, ...).  */
    Signal **blockStateZST;     /**<  Array of signal from the Z Stencil Test units with the information about the color buffer block states (compressed, cleared, ...).  */

    /*  DAC registers.  */
    u32bit hRes;                /**<  Display horizontal resolution.  */
    u32bit vRes;                /**<  Display vertical resolution.  */
    s32bit startX;              /**<  Viewport initial x coordinate.  */
    s32bit startY;              /**<  Viewport initial y coordinate.  */
    bool d3d9PixelCoordinates;  /**<  Use D3D9 pixel coordinates convention -> top left is pixel (0, 0).  */
    u32bit width;               /**<  Viewport width.  */
    u32bit height;              /**<  Viewport height.  */
    u32bit frontBuffer;         /**<  Address in the GPU memory of the front (primary) buffer.  */
    u32bit backBuffer;          /**<  Address in the GPU memory of the back (secondary) buffer.  */
    u32bit zStencilBuffer;      /**<  Address in the GPU memory of the z stencil buffer.  */
    TextureFormat colorBufferFormat;    /**<  Format of the color buffer.  */
    QuadFloat clearColor;       /**<  The current clear color.  */
    bool multisampling;         /**<  Flag that stores if MSAA is enabled.  */
    u32bit msaaSamples;         /**<  Number of MSAA Z samples generated per fragment when MSAA is enabled.  */
    u32bit bytesPixel;          /**<  Bytes per pixel.  */
    u32bit clearDepth;          /**<  Current clear depth value.  */
    u32bit depthPrecission;     /**<  Depth bit precission.  */
    u8bit clearStencil;         /**<  Current clear stencil value.  */
    bool zStencilCompression;   /**<  Flag that stores if if z/stencil compression is enabled.  */
    bool colorCompression;      /**<  Flag that stores if color compression is enabled.  */
    bool rtEnable[MAX_RENDER_TARGETS];              /**<  Render target enable.  */
    TextureFormat rtFormat[MAX_RENDER_TARGETS];     /**<  Render target format.  */
    u32bit rtAddress[MAX_RENDER_TARGETS];           /**<  Render target address.  */

    u8bit clearColorData[MAX_BYTES_PER_COLOR];      /**< Color clear value converted to the color buffer format.  */
                            
    /*  DAC parameters.  */
    u32bit genH;                /**<  Generation tile height in stamps.  */
    u32bit genW;                /**<  Generation tile width in stamps.  */
    u32bit scanH;               /**<  Scan tile height in generation tiles.  */
    u32bit scanW;               /**<  Scan tile width in generation tiles.  */
    u32bit overH;               /**<  Over scan tile heigh in scan tiles.  */
    u32bit overW;               /**<  Over scan tile width in scan tiles.  */
    u32bit blockStateLatency;   /**<  Latency of the color block state signal.  */
    u32bit blocksCycle;         /**<  Number of block states received per cycle.  */
    u32bit blockSize;           /**<  Size of color block in bytes (uncompressed).  Derived from generation tile and pixel data size.  */
    u32bit blockShift;          /**<  Bits to shift to retrieve the block number from a color buffer address.  */
    u32bit blockQueueSize;      /**<  Number of entries in the block request queue.  */
    u32bit decompressLatency;   /**<  Number of cycles it takes a block to be decompressed.  */
    u32bit numStampUnits;       /**<  Number of Color Write units attached to the DAC.  */
    u32bit startFrame;          /**<  Number of the first frame (non simulation related, first frame number for framebuffer dumping).  */
    u64bit refreshRate;         /**<  Number of cycles between the dumping/refresh of the color buffer.  */
    bool synchedRefresh;        /**<  Flag storing if the swap of the color buffer must be synchronized with the refresh.  */
    bool refreshFrame;          /**<  Flag that stores if the color buffer must be refreshed/dumped to a file.  */
    bool saveBlitSourceData;    /**<  Save the source data of blit operations to a PPM file.  */

    /*  DAC state.  */
    RasterizerState state;              /**<  Current DAC state.  */
    RasterizerCommand *lastRSCommand;   /**<  Stores the last Rasterizer Command received (for signal tracing).  */
    u8bit *colorBuffer;                 /**<  Buffer where to store the full color buffer.  */
    ROPBlockState *colorBufferState;    /**<  Current state of the color buffer blocks.  */
    u32bit colorStateBufferBlocks;      /**<  Number of blocks in the current color buffer state.  */
    u32bit frameCounter;                /**<  Counts the number of rendered frames.  */
    u32bit batchCounter;                /**<  Counts the number of draw calls in the current frame.  */
    u32bit blitCounter;                 /**<  Counts the number of blit ops in the current frame.  */
    u32bit requested;                   /**<  Requested color buffer bytes (blocks).  */
    u32bit colorBufferSize;             /**<  Size in bytes of the color buffer.  */
    u32bit stateUpdateCycles;           /**<  Number of cycles remaining for updating the color buffer state memory.  */
    u32bit decompressCycles;            /**<  Number of cycles remaining for block decompression.  */
    u8bit *zstBuffer;                   /**<  Buffer where to store the full z stencil buffer.  */
    ROPBlockState *zStencilBufferState; /**<  Current state of the z stencil buffer blocks.  */
    u32bit zStencilStateBufferBlocks;   /**<  Number of blocks in the current z stencil buffer state.  */
    u32bit zStencilBufferSize;          /**<  Size in bytes of the z stencil buffer.  */
    u32bit zStencilBlockSize;           /**<  Size in bytes of a z stencill buffer block.  */
    u32bit zStencilBytesPixel;          /**<  Bytes per z stencil pixel.  */
    u32bit clearZStencilData;           /**<  Clear value for the z stencil buffer.  */

    /*  DAC block queue.  */
    BlockRequest *blockQueue;   /**<  Queue for the blocks being processed by the DAC.  */
    u8bit **blockBuffer;        /**<  Buffer where to store block data.  */
    u32bit ticket2queue[MAX_MEMORY_TICKETS];    /**<  Associates memory tickets with block queue entries.  */
    u32bit nextFree;            /**<  Next free block queue entry.  */
    u32bit nextRequest;         /**<  Next block in the queue to request to memory.  */
    u32bit nextDecompress;      /**<  Next block in the queue to decompress.  */
    u32bit numFree;             /**<  Number of free queue entries.  */
    u32bit numToRequest;        /**<  Number of blocks to request in the queue.  */
    u32bit numToDecompress;     /**<  Number of blocks to decompress in the queue.  */

    /*  DAC memory state.  */
    MemState memState;                  /**<  Current memory state.  */
    tools::Queue<u32bit> ticketList;    /**<  List with the memory tickets available to generate requests to memory.  */
    u32bit freeTickets;                 /**<  Number of free memory tickets.  */
    u32bit busCycles;                   /**<  Counts the number of cycles remaining for the end of the current memory transmission.  */
    u32bit lastSize;                    /**<  Size of the last memory transaction received.  */
    u32bit lastTicket;                  /**<  Ticket of the last memory transaction received.  */

    //  Pixel mapper.
    PixelMapper colorPixelMapper;       /**<  Maps pixels to addresses and processing units.  */
    PixelMapper zstPixelMapper;         /**<  Maps pixels to addresses and processing units.  */
    
    /*  DAC Blitter component */
    Blitter* blt;                     /**< Blitter unit in DAC Box */
    bool bufferStateUpdatedAtBlitter; /**<  Flag telling the buffer state block structure has been sent to the blitter.  */
    
    /*  DAC statistics.  */
    GPUStatistics::Statistic &stateBlocks;                  /**<  Total number of state blocks read from CW units.  */
    GPUStatistics::Statistic &stateBlocksClear;             /**<  Number of clear color blocks.  */
    GPUStatistics::Statistic &stateBlocksUncompressed;      /**<  Number of uncompressed color blocks.  */
    GPUStatistics::Statistic &stateBlocksCompressedBest;     /**<  Number of best compressed color blocks.  */
    GPUStatistics::Statistic &stateBlocksCompressedNormal;   /**<  Number of normal compressed color blocks.  */
    GPUStatistics::Statistic &stateBlocksCompressedWorst;     /**<  Number of worst compressed color blocks.  */
    
    /*  Private functions.  */

    /**
     *
     *  Processes a rasterizer command.
     *
     *  @param cycle Current simulation cycle.
     *  @param command The rasterizer command to process.
     *
     */

    void processCommand(u64bit cycle, RasterizerCommand *command);

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
     *  @param memTrans Pointer to the memory transaction to process.
     *
     */

    void processMemoryTransaction(MemoryTransaction *memTrans);

    /**
     *
     *  Outputs the current color buffer as a PPM file.
     *
     */

    void writeColorBuffer();

    /**
     *
     *  Outputs the current depth buffer as a PPM file.
     *
     */

    void writeDepthBuffer();

    /**
     *
     *  Outputs the current stencil buffer as a PPM file.
     *
     */

    void writeStencilBuffer();

    /**
     *
     *  Translates an address to the current color buffer into
     *  a block index in the color buffer state memory.
     *
     *  @param address The address to translate.
     *
     *  @return The block index for that address.
     *
     */

    u32bit address2block(u32bit address);


    /**
     *
     *  Calculates the block assigned stamp unit.
     *
     *  @param block Block number/address.
     *  @param pixelMapper The pixel mapper used to obtain the unit corresponding to the block.
     *
     *  @return The stamp unit to which the block of fragments was assigned.
     *
     */

    u32bit blockUnit(PixelMapper &pixelMapper, u32bit block);

    /**
     *
     *  Initializes the refresh/dumping of the color buffer.
     *
     */

    void startRefresh();

    /**
     *
     *  Update memory transmission.
     *
     *  @param cycle The current simulation cycle. 
     *
     */
     
    void updateMemory(u64bit cycle);
    
    /**
     *
     *  Update the block decompressor state.
     *
     *  @param cycle The current simulation cycle.
     *  @param compressorEmulator A pointer to the compress emulator used to decompress blocks.
     *  @param dumpBuffer A pointer to the buffer where to store the block decompressed data.
     *  @param clearValue A pointer to a byte array with the clear value data.
     *  @param bytesPerPixel Bytes per pixel for the current format of the buffer to decompress.
     *  @param baseBufferAddress Address in GPU memory of the buffer to decompress.
     *
     */
     
    void updateDecompressor(u64bit cycle, CompressorEmulator *compressorEmulator, u8bit *dumpBuffer, 
                             u8bit *clearValue, u32bit bytesPerPixel, u32bit baseBufferAddress);
    
    
    /**
     *
     *  Update the block request stage.
     *
     *  @param cycle Current simulation cycle.
     *
     */
     
    void updateBlockRequest(u64bit cycle);
 
    /**
     *
     *  Update request queue stage.
     *
     *  @param cycle Current simulation cycle.
     *  @param compressorEmulator A pointer to the compress emulator used to decompress blocks.
     *  @param baseBufferAddress Address in GPU memory of the buffer to request.
     *  @param dumpBufferSize Size in bytes of the buffer to request.
     *  @param bufferBlockState Pointer to an array with per block state information for the buffer to request.
     *  @param bufferBlockSize Size in bytes of a block in the buffer to request.
     *
     */

    void updateRequestQueue(u64bit cycle, CompressorEmulator *compressorEmulator, u32bit baseBufferAddress,
                            u32bit dumpBufferSize, ROPBlockState *bufferBlockState, u32bit bufferBlockSize);

    /**
     *
     *  Reset the state of the DAC unit.
     *
     */     
     
    void reset();
    
    class BlitterTransactionInterfaceImpl;
    
    friend class BlitterTransactionInterfaceImpl; // Allow internal access to parent class (DAC) 
    
    class BlitterTransactionInterfaceImpl: public BlitterTransactionInterface
    {
        
    private:
        
        DAC& dac;

    public:
        
        BlitterTransactionInterfaceImpl(DAC& dac);

        /**
         *  Callback methods available for the Blitter unit to send memory transactions
         *
         */
        bool sendWriteTransaction(u64bit cycle, u32bit size, u32bit address, u8bit* data, u32bit id);
        bool sendMaskedWriteTransaction(u64bit cycle, u32bit size, u32bit address, u8bit* data, u32bit *mask, u32bit id);
        bool sendReadTransaction(u64bit cycle, u32bit size, u32bit address, u8bit* data, u32bit ticket);
    
        GPUStatistics::StatisticsManager& getSM();
        
        std::string getBoxName();
        
    } bltTransImpl; // The BlitterTransactionInterfaceImpl implementor object
    
    
public:

    /**
     *
     *  DAC box constructor.
     *
     *  Creates and initializes a DAC box.
     *
     *  @param overW Over scan tile width in scan tiles.
     *  @param overH Over scan tile height in scan tiles.
     *  @param scanW Scan tile width in pixels.
     *  @param scanH Scan tile height in pixels.
     *  @param genW Generation tile width in pixels.
     *  @param genH Generation tile height in pixels.
     *  @param mortonBlockDim Dimension of a texture morton block.
     *  @param mortonSBlockDim Dimension of a texture morton superblock.
     *  @param blockSz Size of an uncompressed block in bytes.
     *  @param blockStateLatency Color block state signal latency.
     *  @param blocksCycle Blocks updated per cycle in the state memory.
     *  @param blockQueueSize Number of entries in the block request queue.
     *  @param decompLatency Number of cycles it takes to decompress a block.
     *  @param nStampUnits Number of stamp units attached to HZ.
     *  @param suPrefixes Array of stamp unit prefixes.
     *  @param startFrame Number of the first frame to be dumped.
     *  @param refresh Refresh rate (cycles between generation/dumping of the color buffer).
     *  @param synched Framebuffer dumping/refresh syncronized with refresh rate.
     *  @param refreshFrame Enables or disables the dumping/refresh of the frame buffer.
     *  @param name The box name.
     *  @param box The parent box.
     *
     *  @return An initialized DAC box.
     *
     */

    DAC(u32bit overW, u32bit overH, u32bit scanW, u32bit scanH, u32bit genW, u32bit genH,
        u32bit mortonBlockDim, u32bit mortonSBlockDim,
        u32bit blockSz, u32bit blockStateLatency, u32bit blocksCycle, u32bit blockQueueSize,
        u32bit decompLatency, u32bit nStampUnits, char **suPrefixes, 
        u32bit startFrame, u64bit refresh, bool synched, bool refreshFrame,
        bool saveBlitSourceData, char *name, Box *parent);

    /**
     *
     *  DAC simulation function.
     *
     *  Simulates a cycle of the DAC box.
     *
     *  @param cycle The current simulation cycle.
     *
     */

    void clock(u64bit cycle);
};

} // namespace gpu3d

#endif
