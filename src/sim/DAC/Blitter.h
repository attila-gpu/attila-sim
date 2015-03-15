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

/**
  *
  *  @file Blitter.h
  *
  *  This file defines the Blitter class and auxiliary structures. The Blitter class is used to 
  *  perform bit blit operations through a blitter unit present in most of the graphics boards.
  *
  */
 
#ifndef _BLITTER_

#define _BLITTER_

#include "GPUTypes.h"
#include "MemoryTransaction.h"
#include "GPU.h"
#include "ColorCacheV2.h"
#include "RasterizerState.h"
#include "RasterizerCommand.h"
#include "toolsQueue.h"
#include "WakeUpQueue.h"
#include "StatisticsManager.h"
#include "PixelMapper.h"
#include <list>
#include <vector>
#include <utility>
#include <set>

namespace gpu3d
{

/** 
  *  Maximum number of comparisons the wake up queue can perform in a cycle. This "comparison 
  *  capacity" has to be shared between the queue entries and the entry event list size, 
  *  for instance, if MAX_WAKEUPQUEUE_COMPARISONS_PER_CYCLE = 256 then, a wakeup queue allowing
  *  event lists of 16 elements can only have 16 entries, but allowing event lists of 4 elements
  *  the queue can have 64 entries.
  */
const u32bit MAX_WAKEUPQUEUE_COMPARISONS_PER_CYCLE = 256;

/**
  *  The BlitterTransactionInterface is the interface used by the blitter in order to access the memory controller and 
  *  perform memory requests. The Blitter constructor requires an implementation of this interface.
  * 
  *  @note Only a simulator Box directly connected to the memory controller can implement this interface and thus
  *        can host the Blitter component of the GPU.
  *
  *  @note The simulator Box can notify to the Blitter that a read transaction is complete using the 
  *        receivedReadTransaction() public method in of the Blitter class.
  *
 */
 
class BlitterTransactionInterface
{
public:

    /**
      *  Sends a memory read transaction.
      *  
      *  @return return False if the corresponding transaction was not possible this cycle, True otherwise.
      */
    virtual bool sendReadTransaction(
                            u64bit cycle, 
                            u32bit size, 
                            u32bit address, 
                            u8bit* data, 
                            u32bit ticket) = 0;

    /**
      *  Sends a memory write transaction.
      *  
      *  @return return False if the corresponding transaction was not possible this cycle, True otherwise.
      */
    virtual bool sendWriteTransaction(
                            u64bit cycle, 
                            u32bit size, 
                            u32bit address, 
                            u8bit* data, 
                            u32bit ticket) = 0;
                            
    /**
      *  Sends a masked memory write transaction.
      *  
      *  @return return False if the corresponding transaction was not possible this cycle, True otherwise.
      */
    virtual bool sendMaskedWriteTransaction(
                            u64bit cycle, 
                            u32bit size, 
                            u32bit address, 
                            u8bit* data, 
                            u32bit *mask, 
                            u32bit ticket) = 0;

    /**
      *  Gets a reference to StatisticsManager.
      *  
      *  This is needed because only Box classes can have access to the statistics manager.
      */
    virtual GPUStatistics::StatisticsManager& getSM() = 0;
    
    /**
      *  Gets the Box name of this interface implementator.
      *  
      *  @note The Blitter class will use this name for panic and warning messages.
      */
    virtual std::string getBoxName() = 0;
    

}; // class BlitterTransactionInterface


/**
  *  Some auxiliar structures used by the blitter.
  *  
  *  The following are structures to store the processing information of memory blocks. 
  */


typedef std::vector<s32bit> swizzleMask;  /**< Definition of the swizzle mapping. Each position tells the correspondence between pixel and texel bytes.
                                            *  Negative values will be used to specify that the corresponding pixel bytes doesn�t map to any texel byte. */               
/**
  *  Stores all the data and state related to the requested color buffer memory block of a single 
  *  texture block.
  *  It holds the compressed and uncompressed contents of the requested block to memory.
  */
class ColorBufferBlock
{

public:

    u32bit              address;            /**<  The color buffer block start address.  */
    u32bit              block;              /**<  The color buffer address block (address >> log2(colorBufferblockSize)).  */
    ROPBlockState       state;              /**<  State (compression) of the block.  */ 
    swizzleMask         swzMask;            /**<  The swizzling mapping to the texture block texels.  */
    u8bit*              compressedContents; /**<  The compressed contents from memory.  */
    u8bit*              contents;           /**<  The contents once decompressed.  */
    u32bit              requested;          /**<  The texture block bytes already requested.  */
    std::vector<u32bit> tickets;            /**<  The transaction ticket ids. Used as wait events for requested data blocks. */
    
    /**
      *  Color Buffer Block constructor
      *  
      *  @note Initializes pointers to NULL
      */
    ColorBufferBlock();
    
    /**
      *  Color Buffer Block destructor
      *  
      *  @note Deletes non-null pointers
      */
     ~ColorBufferBlock();
     
     /**
       *  Color Buffer Block print method
       */
     void print(std::ostream& os) const;
    
     friend std::ostream& operator<<(std::ostream& os, const ColorBufferBlock& b)
     {
         b.print(os);
         return os;
     }
     
};

        
/**
  *  Stores all the data and state of a texture block inside the texture region to update.
  *  The texture block is the main object that flows through the blitter pipeline.
  *  
  *  It holds a vector of up to as many color buffer blocks as necessary to
  *  fill all the destination texture bytes in the texture block that lay inside 
  *  the destination texture region.
  * 
  */  
class TextureBlock
{
    
public:

    u32bit xBlock;                                         /**<  X "tile" position of the texture region block.  */
    u32bit yBlock;                                         /**<  Y "tile" position of the texture region block.  */
    u32bit address;                                        /**<  The texture block start address.  */
    u32bit block;                                          /**<  The texture address block (address >> log2(TextureblockSize)).   */
    u8bit* contents;                                       /**<  Block contents to write. */
    bool*  writeMask;                                      /**<  Mask telling which bytes correspond to valid data (block bytes corresponding to texels to update in memory). */
    u32bit written;                                        /**<  Bytes already written to memory.  */
    std::vector<ColorBufferBlock*> colorBufferBlocks;      /**<  The related color buffer blocks.  */
    
    /**
      *  Texture Block constructor
      *  
      *  @note Initializes pointers to NULL
      */
    TextureBlock();

    /**
      *  Texture Block destructor
      *  
      *  @note Deletes non-null pointers.
      */
    ~TextureBlock();
    
    /**
      *  Texture Block print method
      */
    void print(std::ostream& os) const;
    
    friend std::ostream& operator<<(std::ostream& os, const TextureBlock& b)
    {
        b.print(os);
        return os;
    }
    
};

/**
  *  The swizzling buffer emulates the functionality of a hardware swizzler working on a entire buffer of bytes.
  *
  *  Two arrays of bytes actually exists, the input buffer and the output buffer, where the result is stored.
  *
  *  The input buffer can be filled using the writeInputData() operation, writing as many bytes as the write port
  *  width. Once performed the operation with the swizzle() op, the results can be obtained, reading read port width bytes
  *  using the readOutputData() operation.
  *
  *  To specify the swizzling mask (correspondence of input bytes to output bytes), other different than the identity 
  *  permutation (used by default if no swizzling mask is speficied after reset()) the setSwzMask() can be used.
  */
class SwizzlingBuffer
{

private:
    
    /*  Swizzling Buffer parameters.  */
    u32bit size;                                     /**<  Permutation buffer size. This Hw piece can perform a swizzling over this amount of bytes. */
    u32bit writePortWidth;                           /**<  Width of the write port. log2(size/width) bits to identify the write position in the buffer.  */
    u32bit readPortWidth;                            /**<  Width of the read port. log2(size/width) bits to identify the write position in the buffer.  */

    /*  Swizzling Buffer registers.  */
    u8bit* inBuffer;                                 /**<  Input Buffer where to store the source of the pixel permutation. */
    u8bit* outBuffer;                                /**<  Output Buffer where to store the result of the pixel permutation. */
    std::vector<s32bit> swzMap;                      /**<  Table mapping the swizzle.  */
    
    /*  Swizzling Buffer structures.  */
    u32bit max_read_position;                        /**<  Last position where to read in the buffer. */
    u32bit max_write_position;                       /**<  Last position where to write in the buffer. */
    
    u32bit highest_written_position;                /**<  The highest written position.  */
    u32bit highest_read_position;                   /**<  The highest read position.  */

public:
    
    /**
      *  Swizzling buffer constructor
      *
      *  @param size            The number of bytes of the swizzling buffer.
      *  @param readPortWidth   The result read port width.
      *  @param writePortWidth  The write port width.
      *  @note                  size must be multiple of readPortWidth and writePortWidth, if not a panic is thrown.
      */
    SwizzlingBuffer(u32bit size = 1024, u32bit readPortWidth = 256, u32bit writePortWidth = 256);
  
    /**
      *  Swizzling buffer destructor
      */
    ~SwizzlingBuffer();
    
    /**
      *  Resets the swizzling buffer. 
      *
      *  @note Basically, puts the swizzling mask to the identity permutation.
      *
      */
    void reset();

    /**
      *  Fills the corresponding portion of the input buffer.
      */      
    void writeInputData(u8bit* inputData, u32bit position);
    
    /**
      *  Reads the corresponding portion of the ouput/result buffer.
      */
    void readOutputData(u8bit* outputData, u32bit position);
    
    /**
      *  Performs the data swizzling.
      */
    void swizzle();
    
    /**
      *  Sets the swizzling mask for the corresponding portion.
      */
    void setSwzMask(std::vector<s32bit> swzMap, u32bit position);
};

/**
  *  The Blitter class implements the functionality of the blitter unit.
  *
  */    
class Blitter
{
private:
    
    /* Blitter state registers */   
    u32bit                     hRes;                            /**<  Display horizontal resolution.  */
    u32bit                     vRes;                            /**<  Display vertical resolution.  */
    s32bit                     startX;                          /**<  Viewport initial x coordinate.  */
    s32bit                     startY;                          /**<  Viewport initial y coordinate.  */
    u32bit                     width;                           /**<  Viewport width.  */
    u32bit                     height;                          /**<  Viewport height.  */
    QuadFloat                  clearColor;                      /**<  The current clear color.  */
    TextureFormat              colorBufferFormat;               /**<  The current color buffer format.  */
    u32bit                     backBufferAddress;               /**<  Address of the back buffer (source blit) in memory.  */
    u32bit                     blitIniX;                        /**<  Bit blit initial (bottom-left) color buffer X coordinate.  */
    u32bit                     blitIniY;                        /**<  Bit blit initial (bottom-left) color buffer Y coordinate.  */
    u32bit                     blitHeight;                      /**<  Bit blit color buffer region height.  */
    u32bit                     blitWidth;                       /**<  Bit blit color buffer region width.  */
    u32bit                     blitXOffset;                     /**<  X coordinate offset in the destination texture. */
    u32bit                     blitYOffset;                     /**<  Y coordinate offset in the destination texture. */
    u32bit                     blitDestinationAddress;          /**<  GPU memory address for blit operation destination. */
    u32bit                     blitTextureWidth2;               /**<  Ceiling log 2 of the destination texture width.  */
    TextureFormat              blitDestinationTextureFormat;    /**<  Texture format for blit destination.  */
    TextureBlocking            blitDestinationBlocking;         /**<  Texture tiling/blocking format used for the destination texture.  */
    std::vector<ROPBlockState> colorBufferState;                /**<  Current state of the color buffer blocks.  */
    u32bit bytesPixel;                                          /**<  Bytes per pixel.  */
    bool multisampling;                                         /**<  Flag that enables or disables multisampling antialiasing.  */
    u32bit msaaSamples;                                         /**<  Number of multisampling samples per pixel.  */
    
    /*  Blitter parameters.  */
    u32bit stampH;                              /**<  Stamp tile height.  */
    u32bit stampW;                              /**<  Stamp tile width.  */
    u32bit genH;                                /**<  Generation tile height in stamps.  */
    u32bit genW;                                /**<  Generation tile width in stamps.  */
    u32bit scanH;                               /**<  Scan tile height in generation tiles.  */
    u32bit scanW;                               /**<  Scan tile width in generation tiles.  */
    u32bit overH;                               /**<  Over scan tile heigh in scan tiles.  */
    u32bit overW;                               /**<  Over scan tile width in scan tiles.  */
    u32bit mortonBlockDim;                      /**<  The morton block size.  */
    u32bit mortonSBlockDim;                     /**<  The morton superblock block size.  */
    u32bit blockStateLatency;                   /**<  Latency of the color block state signal.  */
    u32bit blocksCycle;                         /**<  Number of block states received per cycle.  */
    u32bit colorBufferBlockSize;                /**<  Size of color block in bytes (uncompressed).  Derived from generation tile and pixel data size.  */
    u32bit maxTransactionsPerTextureBlock;      /**<  Maximum transactions for each texture block.  */
    u32bit maxColorBufferBlocksPerTextureBlock; /**<  Maximum color buffer blocks per each texture block.  */
    u32bit textureBlockSize;                    /**<  Size of the texture block in bytes. It should correspond to the size in bytes of a 8x8 texels morton block.  */
    u32bit blockQueueSize;                      /**<  Number of entries in the block request queue.  */
    u32bit newBlockLatency;                     /**<  Number of cycles it takes build a texture block and all the related information.  */
    u32bit decompressionLatency;                /**<  Number of cycles it takes a block to be decompressed.  */
    u32bit swizzlingLatency;                    /**<  Number of cycles it takes the swizzling buffer to swizzle bytes.  */
    bool saveBlitSource;                        /**<  Flag that stores if the blit source data must be saved as a PPM file.  */

    //  Pixel Mapper.
    PixelMapper pixelMapper;                    /**<  Maps pixels to addresses and processing units. */
    
    /*  Blitter queues  */
    tools::Queue<TextureBlock*>       requestPendingQueue;           /**<  Queue with texture blocks whose color buffer corresponding blocks requests are pending.  */
    WakeUpQueue<TextureBlock, u32bit> receiveDataPendingWakeUpQueue; /**<  Queue for blocks waiting to receive the color buffer blocks data.  */
    tools::Queue<TextureBlock*>       decompressionPendingQueue;     /**<  Queue for blocks waiting for color buffer blocks decompression.  */
    tools::Queue<TextureBlock*>       swizzlingPendingQueue;         /**<  Queue with texture blocks waiting for byte swizzling operation.  */
    tools::Queue<TextureBlock*>       writePendingQueue;             /**<  Queue with texture blocks ready to be written to memory.  */
    
    /*  Blitter variables & structures  */
    u32bit colorBufferSize;                      /**<  The current color buffer size.  */
    bool colorBufferStateReady;                  /**<  Flag telling if color buffer state information already available.  */
    u32bit startXBlock;                          /**<  First texture block that includes the texture subarea (X coord).   */
    u32bit startYBlock;                          /**<  First texture block that includes the texture subarea (Y coord).  */
    u32bit lastXBlock;                           /**<  Last texture block that includes the texture subarea (X coord).  */
    u32bit lastYBlock;                           /**<  Last texture block that includes the texture subarea (Y coord).  */
    u32bit currentXBlock;                        /**<  Current texture block being processed (X coord).  */
    u32bit currentYBlock;                        /**<  Current texture block being processed (Y coord).  */
    u32bit totalBlocksToWrite;                   /**<  Total number of blocks to write to memory. */
    u32bit blocksWritten;                        /**<  Blocks already sent to memory.  */
    SwizzlingBuffer  swzBuffer;                  /**<  The swizzling hardware to permute pixel bytes in color buffer blocks to texel bytes in texture blocks.  */
    u32bit swizzlingCycles;                      /**<  The number of remaining cycles to finish the current swizzling operation.  */
    u32bit newBlockCycles;                       /**<  The number of remaining cycles to finish the generation of a new texture block.  */
    u32bit decompressionCycles;                  /**<  The number of remaining cycles to finish a color buffer block decompression.  */
    
    u8bit *colorBuffer;     /**<  Pointer to an array where to store the data read from the color buffer.  */
    
    u32bit nextTicket;                           /**<  Ticket counter to identify read transactions.  */
    
    bool memWriteRequestThisCycle;               /**<  Flag telling if a memory write transaction was sent to the request signal this cycle. 

    /*  DAC Interface */
    BlitterTransactionInterface& transactionInterface;  /**<  Interface to communicate read and write memory transactions through DAC signals. */

    /*  Blitter statistics.  */
    
    GPUStatistics::Statistic &readBlocks;                       /**<  Number of color buffer blocks read from memory.  */
    GPUStatistics::Statistic &writtenBlocks;                    /**<  Number of texture blocks written to memory.  */
                                                                
    GPUStatistics::Statistic &clearColorBlocks;                 /**<  Number of clear color blocks.  */
    GPUStatistics::Statistic &uncompressedBlocks;               /**<  Number of uncompressed color blocks.  */
    GPUStatistics::Statistic &compressedBestBlocks;             /**<  Number of best compressed color blocks.  */
    GPUStatistics::Statistic &compressedNormalBlocks;           /**<  Number of normal compressed color blocks.  */
    GPUStatistics::Statistic &compressedWorstBlocks;             /**<  Number of worst compressed color blocks.  */

    GPUStatistics::Statistic &requestPendingQueueFull;           /**<  Cycles the request pending queue is full.  */
    GPUStatistics::Statistic &receiveDataPendingWakeUpQueueFull; /**<  Cycles the receive pending wakeup queue is full.  */
    GPUStatistics::Statistic &decompressionPendingQueueFull;     /**<  Cycles the decompression pending queue is full.  */
    GPUStatistics::Statistic &swizzlingPendingQueueFull;         /**<  Cycles the swizzling pending queue is full.  */
    GPUStatistics::Statistic &writePendingQueueFull;             /**<  Cycles the write pending queue is full.  */
    
public:

    /**
      *  Blitter constructor method.
      *
      *  @note All the Blitter parameters required in the constructor with any default value.
      */
    Blitter(BlitterTransactionInterface& transactionInterface, 
            u32bit stampH, u32bit stampW, u32bit genH, u32bit genW, u32bit scanH, u32bit scanW, u32bit overH, u32bit overW,
            u32bit mortonBlockDim, u32bit mortonSBlockDim, u32bit textureBlockSize,
            u32bit blockStateLatency, u32bit blocksCycle, u32bit colorBufferBlockSize, u32bit blockQueueSize, u32bit decompressLatency,
            bool saveBlitSource);

    /**
      *  Reset current blitter state registers values.
      */
    void reset();
    
    /**
      *  Register setting functions. 
      */
    void setDisplayXRes(u32bit hRes);
    void setDisplayYRes(u32bit vRes);
    void setClearColor(QuadFloat clearColor);
    void setViewportStartX(u32bit startX);
    void setViewportStartY(u32bit startY);
    void setViewportWidth(u32bit width);
    void setViewportHeight(u32bit height);
    void setColorBufferFormat(TextureFormat format);
    void setBackBufferAddress(u32bit address);
    void setBlitIniX(u32bit iniX);
    void setBlitIniY(u32bit iniY);
    void setBlitWidth(u32bit width);
    void setBlitHeight(u32bit height);
    void setBlitXOffset(u32bit xoffset);
    void setBlitYOffset(u32bit yoffset);
    void setDestAddress(u32bit address);
    void setDestTextWidth2(u32bit width);
    void setDestTextFormat(TextureFormat format);
    void setDestTextBlock(TextureBlocking blocking);    
    void setColorBufferState(ROPBlockState* _colorBufferState, u32bit colorStateBufferBlocks);
    void setMultisampling(bool enabled);
    void setMSAASamples(u32bit samples);
    
    /**
      *
      *  Initiates the bit blit operation.
      *
      *  @note  Initializes all the variables & structures properly to start the bit blit programmed operation.
      */
    void startBitBlit();
    
    /**
      *  Notifies that a memory read transaction has been completed.
      *
      *  @param ticket    Received transaction ticket 
      *  @param size      Received transaction size
      *
      */
    void receivedReadTransaction(u32bit ticket, u32bit size);
    
       
    /**
      *  Notifies that the current bit blit operation has finished.
      *
      *  @note It remains returning true until the startBitBlit() method is
      *        called again.
      */
    bool currentFinished();
    
    /**
     *
     *  Dumps the contents of the portion of the framebuffer accesed by the blit operation to a file.
     *    
     *  @param frameCounter Current frame counter.
     *  @param blitCounter Current blit operation counter.
     *
     */

    void dumpBlitSource(u32bit frameCounter, u32bit blitCounter);
    
    /**
      *
      *  Simulates a cycle of the blitter unit.
      *
      *  @param cycle Current simulation cycle.
      *
      */
    void clock(u64bit cycle);
    

private:

    /***********************************************************
      *  Auxiliar methods used internaly by the Blitter class  *
      ***********************************************************/
         
    /**
      *  Static method used by the blitter constructor to compute the maximum number of necessary color buffer blocks 
      *  to fill a single texture block. If the texture block and the color buffer block don�t overlap in a perfect way,
      *  although blocks of the same size, one texture block would require more than one single color buffer block.
      *
      *  @param TextureBlockSize2      The log2 of the texture block size.
      *  @param ColorBufferBlockSize2  The log2 of the color buffer block size.
      */
    static u32bit computeColorBufferBlocksPerTextureBlock(u32bit TextureBlockSize2, u32bit ColorBufferBlockSize2);
             
    /**
      *  Translates texel coordinates to the morton address offset starting from the texture base address.
      *
      *  @param i         The horizontal coordinate of the texel.
      *  @param j         The vertical coordinate of the texel.
      *  @param blockDim  The morton block dimension.
      *  @param sBlockDim The morton superblock dimension.
      *  @param width     The ceiling log2 width of the destination texture.
      *
      */
    u32bit texel2MortonAddress(u32bit i, u32bit j, u32bit blockDim, u32bit sBlockDim, u32bit width);
    
    /**
      *  Returns the number of bytes of each texel for a texture using this format.
      *
      *  @param format   The texture format.
      *  @return         How many bytes for each texel.
      */
    u32bit getBytesPerTexel(TextureFormat format);
    
    /**
      *
      *  Decompresses a block according to the compression state and using the specified clearColor.
      *
      *  @note                        The output buffer is assumed to have enough space to decompress an entire block.
      *
      *  @param outbuffer             Location where to store the decompressed block.
      *  @param inbuffer              The compressed block buffer.
      *  @param colorBufferBlockSize  The size in bytes of the block
      *  @param state                 The compression state ( CLEAR, UNCOMPRESSED, COMPRESSION_NORMAL and COMPRESSION_BEST)
      *  @param clearColor            The clear color to fill CLEAR blocks.
      *
      */
    void decompressBlock(u8bit* outBuffer, u8bit* inBuffer, u32bit colorBufferBlockSize, ROPBlockState state, QuadFloat clearColor);

    /**
      *  Translates an address to the start of a block into a block number.
      *
      *  @param address     The address of the block.
      *  @param blockSize   The block size.          
      *  @return            The corresponding block number
      */           
    u32bit address2block(u32bit address, u32bit blockSize);
    
    /**
      *  Generates the proper write mask for the memory transaction given the input mask
      *  as a boolean array (one value per byte).
      *
      *  @note inputMask must be indexable in the range [0..size], and outputMask must have
      *        (size >> 2) elements of allocated space.
      *
      *  @param outputMask The output write mask in the memory transaction required format.
      *  @param inputMask  The input write mask as an array of booleans.
      *  @param size       The number of inputMask elements.
      */
    void translateBlockWriteMask(u32bit* outputMask, bool* inputMask, u32bit size);
      

    /***********************************************************
      *  Private Methods for texture block processing.         *
      ***********************************************************/  

    /**
      *  Sends the next write memory transaction for the specified texture block.
      *
      *  @note If a write transaction is not allowed this cycle, then the method returns false.
      *
      *  @param cycle            The simulation cycle.
      *  @param texureBlock      The texture block to write bytes from.
      *
      *  @return False if write transaction fails, otherwise True.
      *
      */
    bool sendNextWriteTransaction(u64bit cycle, TextureBlock* textureBlock);

    /**
      *  Examines ready texture blocks and sends the next transaction of the head block.
      *
      *  @param cycle    The simulation cycle.
      *  @return         False if ready texture blocks is empty or write transaction couldn�t be
      *                  sent.
      */
    bool sendNextReadyTextureBlock(u64bit cycle, tools::Queue<TextureBlock*>& writePendingQueue);
        
    /**
      *  Generates a texture block with all the necessary information.
      *
      *  @note               The texture block pointer memory must be previously allocated.
      *  @param cycle        The simulation cycle.
      *  @param xBlock       The X coordinate of the texture block tile.  
      *  @param yBlock       The Y coordinate of the texture block tile.
      *  @param textureBlock Pointer to the TextureBlock structure to fill.
      *
      */
    void processNewTextureBlock(u64bit cycle, u32bit xBlock, u32bit yBlock, TextureBlock* textureBlock);
   
    /**
      *  Computes the total number of transactions needed to request all the color buffer blocks data
      *  of the texture block.
      *
      *  @param textureBlock    The texture block.
      *  @return                The total number of transactions to request.
      */
    u32bit computeTotalTransactionsToRequest(TextureBlock* textureBlock);
    
    /**
      *  Sends as many memory read requests as allowed in one cycle for the color buffer blocks in the texture block.
      *
      *  @param cycle             The simulation cycle.
      *  @param textureBlock      The texture block.
      *  @param generatedTickets  The output list with all the generated tickets for the sent transactions.
      *
      */
    bool requestColorBufferBlocks(u64bit cycle, TextureBlock* textureBlock, set<u32bit>& generatedTickets);

    /**
      *  This set of functions send as many memory read requests as allowed in one cycle for the color buffer block 
      *  according to the compression format of the block. The result is the list of generated tickets for the
      *  sent transactions.
      */
    bool requestColorBlock(u64bit cycle, ColorBufferBlock *bInfo, u32bit blockRequestSize, set<u32bit>& generatedTickets);

    bool requestUncompressedBlock(u64bit cycle, ColorBufferBlock *cbBlock, std::set<u32bit>& generatedTickets);
    
    bool requestCompressedNormalBlock(u64bit cycle, ColorBufferBlock *cbBlock, std::set<u32bit>& generatedTickets);
    
    bool requestCompressedBestBlock(u64bit cycle, ColorBufferBlock *cbBlock, std::set<u32bit>& generatedTickets);
    
    /**
      *  Decompresses the color buffer blocks data of the texture block.
      *
      *  @param textureBlock    The texture block.
      */
    void decompressColorBufferBlocks(TextureBlock* textureBlock);
    
    /**
      *  Performs swizzling of the texture block contents. To do this sets up and uses the swizzling
      *  hardware emulation class.
      *
      *  @note The corresponding color buffer blocks are assumed to contain the allocated space with 
      *        valid data
      *
      *  @param textureBlock    The texture Block.
      *
      */
    void swizzleTextureBlock(TextureBlock* textureBlock);
};

} // namespace gpu3d

#endif
