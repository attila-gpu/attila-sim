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
  * @file Blitter.cpp
  *
  * Implements the Blitter class. This class is used to perform BitBlit operations
  *
  */

#include "Blitter.h"
#include "GPUMath.h"
#include "FragmentOpEmulator.h"
#include <algorithm> // STL find() function
#include <map> 
#include <utility> // For pair<> data type


using namespace gpu3d;
using namespace std;
using namespace gpu3d::GPUStatistics;

/******************************************************************************************
 *                       Color Buffer Block class implementation                          *
 ******************************************************************************************/

ColorBufferBlock::ColorBufferBlock()
    :   address(0), block(0), state(ROPBlockState::UNCOMPRESSED), 
        compressedContents(0), contents(0), requested(0)
{
}

ColorBufferBlock::~ColorBufferBlock()
{
    if (compressedContents) delete[] compressedContents;
    if (contents) delete[] contents;
}

void ColorBufferBlock::print(std::ostream& os) const
{
    os << block;
    switch(state.state)
    {
       case ROPBlockState::CLEAR: os << "(CLEAR_COLOR)"; break;
       case ROPBlockState::UNCOMPRESSED: os << "(UNCOMPRESSED)"; break;
       case ROPBlockState::COMPRESSED: os << "(COMP_LEVEL_)" << state.comprLevel; break;
       default:
          panic("Blitter::ColorBufferBlock","print","Color buffer block state not expected");
    }

    if (!tickets.empty())
    {
        vector<u32bit>::const_iterator iter2 = tickets.begin();
 
        os << " Ticket list: {";
        
        while ( iter2 != tickets.end() )
        {
            os << (*iter2);
            iter2++;           
            if (iter2 != tickets.end()) os << ",";
        }    
        
        os << "}";
    }
}

/******************************************************************************************
 *                       Texture Block class implementation                               *
 ******************************************************************************************/
 
TextureBlock::TextureBlock() 
    :   xBlock(0), yBlock(0), address(0), block(0), contents(0), writeMask(0), written(0)
{
}

TextureBlock::~TextureBlock()
{
    if (contents) delete[] contents;
    if (writeMask) delete[] writeMask;

    vector<ColorBufferBlock*>::iterator iter = colorBufferBlocks.begin();
              
    while ( iter != colorBufferBlocks.end() )
    {
       delete (*iter);
       iter++;
    }
}

void TextureBlock::print(ostream& os) const
{
    os << "Texture Block: (" << xBlock << "," << yBlock << ")" << endl;
    os << "\tBlock id = " << block << ", address = " << address;
    
    if (colorBufferBlocks.empty())

        os << ", no colorBuffer blocks defined. " << endl;

    else    // color buffer blocks defined
    {   
        os << ", colorBuffer blocks: {";
              
        vector<ColorBufferBlock*>::const_iterator iter = colorBufferBlocks.begin();
              
        while ( iter != colorBufferBlocks.end() )
        {
           os << (*(*iter));
           iter++;
           if (iter != colorBufferBlocks.end()) os << ",";
        }
        os << "}" << endl;
    }
    
    if (!contents) os << "\tNo contents defined. " << endl;
    if (!writeMask) os << "\tNo write mask defined. " << endl;
    os << "\tWritten bytes = " << written << endl;
}


/******************************************************************************************
 *                       Swizzling Buffer class implementation                            *
 ******************************************************************************************/

/*  Swizzling buffer constructor.  */
SwizzlingBuffer::SwizzlingBuffer(u32bit size, u32bit readPortWidth, u32bit writePortWidth):
    size(size), readPortWidth(readPortWidth), writePortWidth(writePortWidth), 
    highest_written_position(0), highest_read_position(0)
{
    if ( size % readPortWidth != 0 )
        panic("SwizzlingBuffer", "Constructor", "The buffer size must be multiple of the read port width");
        
    if ( size % writePortWidth != 0 )
        panic("SwizzlingBuffer", "Constructor", "The buffer size must be multiple of the write port width");
        
    max_read_position = (u32bit)((f32bit) size / (f32bit) readPortWidth) - 1;
    max_write_position = (u32bit)((f32bit) size / (f32bit) writePortWidth) - 1;
    
    inBuffer = new u8bit[size];
    outBuffer = new u8bit[size];
    
    for (int i = 0; i < size; i++)
        swzMap.push_back(i);
}

/*  Swizzling buffer destructor.  */
SwizzlingBuffer::~SwizzlingBuffer()
{
    delete[] inBuffer;
    delete[] outBuffer;
}
 
/*  Resets the swizzling buffer.  */
void SwizzlingBuffer::reset()
{
    for (int i = 0; i < size; i++)
        swzMap[i] = i;
        
    highest_written_position = highest_read_position = 0;
}

/*  Fills the corresponding portion of the input buffer.  */
void SwizzlingBuffer::writeInputData(u8bit* inputData, u32bit position)
{
    if (position > max_write_position)
        panic("SwizzlingBuffer", "writeInputData", "Write position out of range");
    
    if (position > highest_written_position)
        highest_written_position = position;
            
    u32bit bufferOffset = position * writePortWidth;
    
    memcpy(&inBuffer[bufferOffset], inputData, writePortWidth);
}
    
/*  Reads the corresponding portion of the ouput/result buffer.  */
void SwizzlingBuffer::readOutputData(u8bit* outputData, u32bit position)
{
    if (position > max_read_position)
        panic("SwizzlingBuffer", "readOutputData", "Read position out of range");

    if (position > highest_read_position)
        highest_read_position = position;
        
    u32bit bufferOffset = position * readPortWidth;
    
    memcpy(outputData, &outBuffer[bufferOffset], readPortWidth);
}

/*  Performs the data swizzling.  */
void SwizzlingBuffer::swizzle()
{
    for (int i = 0; i < (highest_written_position + 1) * writePortWidth; i++)
    {
        u32bit idx = i;
        s32bit dest = swzMap[i];

//printf("Swizzling Buffer  => Moving input byte %d to output byte position %d.\n", idx, dest);
        if (dest >= size && dest != -1)
            panic("SwizzlingBuffer", "swizzle", "a swizzle map element points to a buffer position out of range");
        
        if (dest != -1)
            outBuffer[dest] = inBuffer[idx];
    }    
}

/*  Sets the swizzling mask for the corresponding portion.  */
void SwizzlingBuffer::setSwzMask(std::vector<s32bit> _swzMap, u32bit position)
{
    if (position > max_write_position)
        panic("SwizzlingBuffer", "setSwzMask", "Swz Map position out of range");
    
    u32bit maskOffset = position * writePortWidth;
    
    for (int i = maskOffset; i < maskOffset + writePortWidth; i++)
        swzMap[i] = _swzMap[i - maskOffset];
}


/******************************************************************************************
 *                          Blitter class implementation                                  *
 ******************************************************************************************/

/*  Blitter constructor method.  */
Blitter::Blitter(BlitterTransactionInterface& transactionInterface,
                 u32bit stampH, u32bit stampW, u32bit genH, u32bit genW, u32bit scanH, u32bit scanW, u32bit overH, u32bit overW, 
                 u32bit mortonBlockDim, u32bit mortonSBlockDim, u32bit textureBlockSize,
                 u32bit blockStateLatency, u32bit blocksCycle, u32bit colorBufferBlockSize, u32bit blockQueueSize, u32bit decompressLatency,
                 bool saveBlitSource) :

    readBlocks(transactionInterface.getSM().getNumericStatistic("ReadBlocks", u32bit(0), "Blitter", "Blitter")),
    writtenBlocks(transactionInterface.getSM().getNumericStatistic("WrittenBlocks", u32bit(0), "Blitter", "Blitter")),
    
    requestPendingQueueFull(transactionInterface.getSM().getNumericStatistic("RequestPendingQueueFull", u32bit(0), "Blitter", "Blitter")),
    receiveDataPendingWakeUpQueueFull(transactionInterface.getSM().getNumericStatistic("ReceiveDataPendingWakeUpQueueFull", u32bit(0), "Blitter", "Blitter")),
    decompressionPendingQueueFull(transactionInterface.getSM().getNumericStatistic("DecompressionPendingQueueFull", u32bit(0), "Blitter", "Blitter")),
    swizzlingPendingQueueFull(transactionInterface.getSM().getNumericStatistic("SwizzlingPendingQueueFull", u32bit(0), "Blitter", "Blitter")),
    writePendingQueueFull(transactionInterface.getSM().getNumericStatistic("WritePendingQueueFull", u32bit(0), "Blitter", "Blitter")),
    
    clearColorBlocks(transactionInterface.getSM().getNumericStatistic("ColorStateBlocksClear", u32bit(0), "Blitter", "Blitter")),
    uncompressedBlocks(transactionInterface.getSM().getNumericStatistic("ColorStateBlocksUncompressed", u32bit(0), "Blitter", "Blitter")),
    compressedBestBlocks(transactionInterface.getSM().getNumericStatistic("ColorStateBlocksCompressedBest", u32bit(0), "Blitter", "Blitter")),
    compressedNormalBlocks(transactionInterface.getSM().getNumericStatistic("ColorStateBlocksCompressedNormal", u32bit(0), "Blitter", "Blitter")),
    compressedWorstBlocks(transactionInterface.getSM().getNumericStatistic("ColorStateBlocksCompressedWorst", u32bit(0), "Blitter", "Blitter")),

    transactionInterface(transactionInterface),
    stampH(stampH), stampW(stampW), genH(genH), genW(genW), scanH(scanH), scanW(scanW), overH(overH), overW(overW),
    mortonBlockDim(mortonBlockDim), mortonSBlockDim(mortonSBlockDim), textureBlockSize(textureBlockSize), blockStateLatency(blockStateLatency), 
    blocksCycle(blocksCycle), colorBufferBlockSize(colorBufferBlockSize), blockQueueSize(blockQueueSize), saveBlitSource(saveBlitSource),
    
    maxColorBufferBlocksPerTextureBlock(computeColorBufferBlocksPerTextureBlock( (u32bit) GPU_LOG2((f64bit)textureBlockSize), 
                                                                                 (u32bit) GPU_LOG2((f64bit)colorBufferBlockSize))),

    maxTransactionsPerTextureBlock(computeColorBufferBlocksPerTextureBlock( (u32bit) GPU_LOG2((f64bit)textureBlockSize), 
                                                                                 (u32bit) GPU_LOG2((f64bit)colorBufferBlockSize)) *
                                   (u32bit)(ceil((f32bit) colorBufferBlockSize / (f32bit) MAX_TRANSACTION_SIZE)) ),
                                                                                 
    decompressionLatency(decompressLatency), 
    colorBufferStateReady(false), blitIniX(0), blitIniY(0), blitXOffset(0), blitYOffset(0), blitHeight(0), blitWidth(0), 
    blitDestinationAddress(0x800000), blitDestinationBlocking(GPU_TXBLOCK_TEXTURE), blitTextureWidth2(0), blitDestinationTextureFormat(GPU_RGBA8888), 
    hRes(400), vRes(400), startX(0), startY(0), width(400), height(400),

    receiveDataPendingWakeUpQueue(0, maxTransactionsPerTextureBlock),
    
    swzBuffer(colorBufferBlockSize * maxColorBufferBlocksPerTextureBlock, textureBlockSize, colorBufferBlockSize),
    
    /* The hardware specific latencies.  */
    newBlockLatency(maxColorBufferBlocksPerTextureBlock),
    swizzlingLatency(maxColorBufferBlocksPerTextureBlock)
{
    string queueName;

    queueName.clear();
    queueName.append("Blitter::requestPendingQueue");
    requestPendingQueue.setName(queueName);
    requestPendingQueue.resize(4);

    queueName.clear();
    queueName.append("Blitter::receiveDataPendingWakeUpQueue");
    receiveDataPendingWakeUpQueue.setName(queueName);
    receiveDataPendingWakeUpQueue.resize((u32bit)((f32bit)MAX_WAKEUPQUEUE_COMPARISONS_PER_CYCLE/(f32bit)maxTransactionsPerTextureBlock));
    
    queueName.clear();
    queueName.append("Blitter::decompressionPendingQueue");
    decompressionPendingQueue.setName(queueName);
    decompressionPendingQueue.resize(4);
    
    queueName.clear();
    queueName.append("Blitter::swizzlingPendingQueue");
    swizzlingPendingQueue.setName(queueName);
    swizzlingPendingQueue.resize(4);

    queueName.clear();
    queueName.append("Blitter::writePendingQueue");
    writePendingQueue.setName(queueName);
    writePendingQueue.resize(4);
    
    /*colorBufferSize = ((u32bit) ceil(vRes / (f32bit) (overH * scanH * genH * stampH))) *
                      ((u32bit) ceil(hRes / (f32bit) (overW * scanW * genW * stampW))) *
                      overW * overH * scanW * scanH * genW * genH * stampW * stampH * bytesPixel;*/

    // Setup display in the Pixel Mapper.
    pixelMapper.setupDisplay(hRes, vRes, stampW, stampH, genW, genH, scanW, scanH, overW, overH, 1, 4);
    colorBufferSize = pixelMapper.computeFrameBufferSize();                      

    colorBuffer = NULL;
                          
    clearColor[0] = 0.0f;
    clearColor[1] = 0.0f;
    clearColor[2] = 0.0f;
    clearColor[3] = 1.0f;
    
    colorBufferFormat = GPU_RGBA8888;
}

/*  Reset current blitter state registers values.  */
void Blitter::reset()
{
    hRes = 400;
    vRes = 400;
    startX = 0;
    startY = 0;
    width = 400; 
    height = 400;
    blitIniX = 0;
    blitIniY = 0;
    blitHeight = 400;
    blitWidth = 400; 
    blitXOffset = 0;
    blitYOffset = 0;
    blitDestinationAddress = 0x8000000;
    blitTextureWidth2 = 9; // 2^9 = 512 (> 400)
    blitDestinationTextureFormat = GPU_RGBA8888;
    blitDestinationBlocking = GPU_TXBLOCK_TEXTURE;

    clearColor[0] = 0.0f;
    clearColor[1] = 0.0f;
    clearColor[2] = 0.0f;
    clearColor[3] = 1.0f;
    colorBufferFormat = GPU_RGBA8888;
    bytesPixel = 4;
    multisampling = false;
    msaaSamples = 2;
        
    /*colorBufferSize = ((u32bit) ceil(vRes / (f32bit) (overH * scanH * genH * stampH))) *
                      ((u32bit) ceil(hRes / (f32bit) (overW * scanW * genW * stampW))) *
                      overW * overH * scanW * scanH * genW * genH * stampW * stampH * bytesPixel;*/

    // Setup display in the Pixel Mapper.
    u32bit samples = multisampling ? msaaSamples : 1;
    pixelMapper.setupDisplay(hRes, vRes, stampW, stampH, genW, genH, scanW, scanH, overW, overH, samples, bytesPixel);
    colorBufferSize = pixelMapper.computeFrameBufferSize();                      

    if (colorBuffer != NULL)
        delete [] colorBuffer;
        
    colorBuffer = NULL;
                    
    colorBufferState.clear();
}

/**
 *  Register setting functions. 
 */
void Blitter::setDisplayXRes(u32bit _hRes)
{
    hRes = _hRes;
    
    /*colorBufferSize = ((u32bit) ceil(vRes / (f32bit) (overH * scanH * genH * stampH))) *
                      ((u32bit) ceil(hRes / (f32bit) (overW * scanW * genW * stampW))) *
                      overW * overH * scanW * scanH * genW * genH * stampW * stampH * bytesPixel;*/
}

void Blitter::setDisplayYRes(u32bit _vRes)
{
    vRes = _vRes;
    
    /*colorBufferSize = ((u32bit) ceil(vRes / (f32bit) (overH * scanH * genH * stampH))) *
                      ((u32bit) ceil(hRes / (f32bit) (overW * scanW * genW * stampW))) *
                      overW * overH * scanW * scanH * genW * genH * stampW * stampH * bytesPixel;*/
}

void Blitter::setClearColor(QuadFloat _clearColor)
{
    clearColor = _clearColor;
}

void Blitter::setColorBufferFormat(TextureFormat format)
{
    colorBufferFormat = format;
    
    switch(colorBufferFormat)
    {
        case GPU_RGBA8888:
        case GPU_RG16F:
        case GPU_R32F:
            bytesPixel = 4;
            break;
        
        case GPU_RGBA16F:
        case GPU_RGBA16:
            bytesPixel = 8;
            break;
        default:
            panic("Blitter", "setColorBufferFormat", "Unsupported color buffer format.");
            break;
    }
}

void Blitter::setBackBufferAddress(u32bit address)
{
    backBufferAddress = address;
}

void Blitter::setMultisampling(bool enable)
{
    multisampling = enable;
}

void Blitter::setMSAASamples(u32bit samples)
{
    msaaSamples = samples;
}

void Blitter::setViewportStartX(u32bit _startX)
{
    startX = _startX;
}
    
void Blitter::setViewportStartY(u32bit _startY)
{
    startY = _startY;
}
    
void Blitter::setViewportWidth(u32bit _width)
{
    width = _width;
}
    
void Blitter::setViewportHeight(u32bit _height)
{
    height = _height;
}
   
void Blitter::setBlitIniX(u32bit iniX)
{
    blitIniX = iniX;
}

void Blitter::setBlitIniY(u32bit iniY)
{
    blitIniY = iniY;
}

void Blitter::setBlitXOffset(u32bit xoffset)
{
    blitXOffset = xoffset;
}
    
void Blitter::setBlitYOffset(u32bit yoffset)
{
    blitYOffset = yoffset;
}
    
void Blitter::setBlitWidth(u32bit width)
{
    blitWidth = width;
}

void Blitter::setBlitHeight(u32bit height)
{
    blitHeight = height;
}

void Blitter::setDestAddress(u32bit address)
{
    blitDestinationAddress = address;
}

void Blitter::setDestTextWidth2(u32bit width)
{
    blitTextureWidth2 = width;
}

void Blitter::setDestTextFormat(TextureFormat format)
{
    blitDestinationTextureFormat = format;
}

void Blitter::setDestTextBlock(TextureBlocking blocking)
{
    blitDestinationBlocking = blocking;
}

void Blitter::setColorBufferState(ROPBlockState* _colorBufferState, u32bit colorStateBufferBlocks)
{
    if ( (colorBufferSize >> GPUMath::calculateShift(colorBufferBlockSize)) != colorStateBufferBlocks )
        panic("Blitter","setColorBufferState","A different number of color buffer state blocks is specified");

    //  Clear old color buffer state.
    colorBufferState.clear();
            
    //  Set new color buffer state.
    for (int i = 0; i < colorStateBufferBlocks; i++)
        colorBufferState.push_back(_colorBufferState[i]);

    //printf("Blitter => Setting color buffer state | num blocks = %d\n", colorStateBufferBlocks);
    //printf("Blitter => Color block 153 (param) -> state = %d level = %d\n", _colorBufferState[153].getState(), _colorBufferState[153].getComprLevel());
    //printf("Blitter => Color block 153 (buffer) -> state = %d level = %d\n", colorBufferState[153].getState(), colorBufferState[153].getComprLevel());
            
    colorBufferStateReady = true;
}

/*  Initiates the bit blit operation.  */
void Blitter::startBitBlit()
{
    /*  Initialize the coordinates for the first texture block to update.  */
    startXBlock = blitXOffset >> mortonBlockDim;
    startYBlock = blitYOffset >> mortonBlockDim;
    
    /*  Compute last texture block.  */
    lastXBlock = (blitXOffset + blitWidth - 1) >> mortonBlockDim;
    lastYBlock = (blitYOffset + blitHeight - 1) >> mortonBlockDim;
    
    /*  Current texture bloks begins as the first.  */
    currentXBlock = startXBlock;
    currentYBlock = startYBlock;
    
    totalBlocksToWrite = (lastXBlock - startXBlock + 1) * (lastYBlock - startYBlock + 1);

    GPU_DEBUG(
        printf("%s::Blitter => Bit blit operation initiated. %d texture blocks to write starting from address = %X.\n", 
                transactionInterface.getBoxName().c_str(), totalBlocksToWrite, blitDestinationAddress);
    )
        
    // Setup display in the Pixel Mapper.
    u32bit samples = multisampling ? msaaSamples : 1;
    pixelMapper.setupDisplay(hRes, vRes, stampW, stampH, genW, genH, scanW, scanH, overW, overH, samples, bytesPixel);
    colorBufferSize = pixelMapper.computeFrameBufferSize();
        
    /*  Initialize other counters.  */        

    swizzlingCycles = swizzlingLatency;
    newBlockCycles = newBlockLatency;
    decompressionCycles = decompressionLatency;

    blocksWritten = 0;
    nextTicket = 0;

    //  Check if the blit source data must be saved into a file.
    if (saveBlitSource)
    {
        //  Allocate space for the color buffer.
        colorBuffer = new u8bit[colorBufferSize];
        
        GPU_ASSERT(
            if (colorBuffer == NULL)
                panic("Blitter", "startBitBlit", "Error allocating memory for the source blit color buffer data.");
        )
        
    }
    
//printf("Blitter::startBitBlit => created color buffer %p with size %d\n", colorBuffer, colorBufferSize);
}

/*  Notifies that a memory read transaction has been completed.  */
void Blitter::receivedReadTransaction(u32bit ticket, u32bit size)
{
    GPU_DEBUG(
        printf("%s::Blitter => Received transaction with ticket id = %d and size = %d.\n", transactionInterface.getBoxName().c_str(), ticket, size);
    )

        /*  Wake up all the texture blocks waiting for this transaction completeness. Texture Blocks waiting
         *  more than one transaction will remain asleep until all transactions wake them up. */
    receiveDataPendingWakeUpQueue.wakeup(ticket);
}

/*  Notifies that the current bit blit operation has finished.  */
bool Blitter::currentFinished()
{
    return (blocksWritten == totalBlocksToWrite);
}

/**
 *  Simulates a cycle of the blitter unit.
 *  
 *  The blitter pipeline is implemented as follows:
 *
 *      Given the destination texture image partitioned in texture blocks of texture block size bytes:
 *
 *      1. The First stage generates texture block structures for the tiles (X,Y) that cover the 
 *         texture region to update. This structure is filled with all the necessary information, such as:
 *    
 *         1.1 The color buffer blocks to request to memory to fill the texture block with data,
 *         1.2 The write mask of the valid bytes of the texture block that will be finally written
 *             to memory (the texture region can partially cover the generated texture block.
 *         1.3 The start address in memory of the texture block.
 *         1.4 The mapping or correspondence between pixel byte positions in the color buffer blocks and
 *             the texel bytes position in the texture block. This is necessary when the destination texture
 *             is arranged in a different blocking format (such as Morton) or when the color buffer block
 *             doesn�t fit exactly in the texture block shape.
 *
 *         The operation is performed each newBlockLatency cycles. When finished the texture block is 
 *         enqueued in the "pendingRequestQueue".
 *
 *      2. The Second stage generates memory read transactions for the Texture Blocks in the "requestPendingQueue". 
 *         A memory read request is only possible if there is any write request the same cycle, avoiding
 *         the situation that only reads are performed and no writes make progress, and consecuently, the different 
 *         pipeline queues get full and the pipeline gets stalled. The first time we get the head texture block of the
 *         pending request queue, the block is enqueued on the "receivedDataPendingWakeUpQueue" with the corresponding
 *         transaction tickets (events) to wait for. Next cycles, the event list can be completed adding new transactions 
 *         tickets as new memory read transactions for the texture block are generated.
 *         
 *      3. The Third stage tries to pick-up/select those Texture blocks whose corresponding memory transactions ticket
 *         have arrived (transactions are completed) and hence, the corresponding memory blocks are available for
 *         further processing. The selected Texture Block is enqueued in the "decompressionPendingQueue".
 *
 *      4. The Fourth stage decompress the read color buffer blocks of the head Texture Block in the decompressionPendingQueue.
 *         The operation is performed each decompressionLatency cycles. When finished the texture block is enqueued in the
 *         "swizzlingPendingQueue".
 *  
 *      5. The Fifth stage performs the swizzling operation over the head of the "swizzlingPendingQueue". It takes all the color
 *         buffer blocks data and swizzling masks and performs the operation using a Swizzling Buffer hardware. The corresponding
 *         operation latency is applied and, when finished, the texture block is enqueued in the "writePendingQueue".
 *      
 *      6. Finally, the ready texture blocks of the "writePendingQueue" are written to memory sending the related memory write
 *         transactions.
 *       
 *    As can be seen in the clock() code, the stages actually are processed in inversed order. This is because
 *    the work performed in a stage is only available for the next stage on the next cycle (following a
 *    pipelined model).
 */

void Blitter::clock(u64bit cycle)
{
    /* Sixth stage: Send a new write transacion for the next ready texture block in the write pending queue. 
     *              If a write is performed this cycle then a block read request will not be allowed.
     */
    memWriteRequestThisCycle = sendNextReadyTextureBlock(cycle, writePendingQueue);
        
    /** 
     * Fifth stage: Look up the swizzling pending queue for texture blocks and apply swizzling operation.
     *
     */
    if (!swizzlingPendingQueue.empty())
    {
        /*  Decrement the swizzling remaining cycles.  */
        if (swizzlingCycles > 0) swizzlingCycles--;

        GPU_DEBUG(
           printf("%s::Blitter %lld => Swizzling remaining cycles = %d.\n", transactionInterface.getBoxName().c_str(), cycle, swizzlingCycles);
        )
        
        if (swizzlingCycles == 0 && !writePendingQueue.full())
        {
            GPU_DEBUG(
               printf("%s::Blitter %lld => Swizzling finished.\n", transactionInterface.getBoxName().c_str(), cycle);
            )
        
            TextureBlock* textureBlock = swizzlingPendingQueue.head();
            
            /*  Perform swizzling.  */
            swizzleTextureBlock(textureBlock);
            
            /*  Enqueue it to the texture block queue for blocks ready to be written.  */
            writePendingQueue.add(textureBlock);
            
            /*  Remove from the swizzling pending queue.  */           
            swizzlingPendingQueue.remove();
            
            /*  Reset swizzling operation remaining cycles.  */
            swizzlingCycles =  swizzlingLatency;
        }
        else if (writePendingQueue.full()) /*  Update statistic.  */
            writePendingQueueFull++;
    }

    /**
     * Fourth stage: Select next texture block whose color buffer blocks must be decompressed.
     *
     */
    if (!decompressionPendingQueue.empty())
    {
        /*  Decrement the decompression remaining cycles.  */
        if (decompressionCycles > 0) decompressionCycles--;

        GPU_DEBUG(
           printf("%s::Blitter %lld => Decompression remaining cycles = %d.\n", transactionInterface.getBoxName().c_str(), cycle, decompressionCycles);
        )
        
        /*  Check if decompression operation finished.  */
        if (decompressionCycles == 0 && !swizzlingPendingQueue.full())
        {
            GPU_DEBUG(
                printf("%s::Blitter %lld => Decompression finished.\n", transactionInterface.getBoxName().c_str(), cycle);
            ) 
        
            TextureBlock* textureBlock = decompressionPendingQueue.head();
            
            /*  Perform decompression.  */    
            decompressColorBufferBlocks(textureBlock);

            /*  Enqueue to the swizzling operation queue.  */
            swizzlingPendingQueue.add(textureBlock);

            /*  Remove from decompression queue.  */            
            decompressionPendingQueue.remove();
            
            /*  Reset decompression remainder cycles.  */
            decompressionCycles = decompressionLatency;
        }
        else if (swizzlingPendingQueue.full()) /*  Update statistic.  */
            swizzlingPendingQueueFull++;

    }

    /**
     * Third stage: Select completely received texture blocks. These texture blocks have all the 
     *              color buffer block necessary data already available.
     */
    if (!receiveDataPendingWakeUpQueue.empty())
    {
        /*  Select the next ready/selectable texture block.  */
        TextureBlock* textureBlock = receiveDataPendingWakeUpQueue.select();

        if (textureBlock && !decompressionPendingQueue.full())
        {
            GPU_DEBUG(
               printf("%s::Blitter %lld => Block (%d,%d) Fully received.\n", transactionInterface.getBoxName().c_str(), cycle, textureBlock->xBlock, textureBlock->yBlock);
            )
            
            /*  Update statistic.  */
            for (int i = 0; i < textureBlock->colorBufferBlocks.size(); i++)
                readBlocks++;

            /*  Send Color Buffer Blocks to the decompression queue.  */
            decompressionPendingQueue.add(textureBlock);
            
            /*  Remove from wakeup queue.  */
            receiveDataPendingWakeUpQueue.remove(textureBlock);
        }
        else if (decompressionPendingQueue.full()) /*  Update statistic.  */
            decompressionPendingQueueFull++;
    }
    
    /**
     * Second stage: Request the color buffer blocks sending 
     *              the corresponding memory read transactions  
     */
    if (!requestPendingQueue.empty() && !memWriteRequestThisCycle)
    {
        /*  There�s an entry of request pending (either already in the Wakeup queue or not) and 
         *  the read requests are allowed (no write request this cycle).  */
        
        /*  Tells when we have finished with a block.  */
        bool allBlocksRequested = false; 
        
        TextureBlock* textureBlock = requestPendingQueue.head();
        
        /*  Check if already exists in the Wakeup queue. If not generate transactions and enqueue it 
         *  for the first time.  */
        if (!receiveDataPendingWakeUpQueue.exists(textureBlock) && !receiveDataPendingWakeUpQueue.full())
        {
             set<u32bit> generatedTickets;
             
             /*  For each block send as many read request as possible.  */ 
             allBlocksRequested = requestColorBufferBlocks(cycle, textureBlock, generatedTickets);
             
             /*  Enqueue at the Wakeup queue with the generated tickets as the events to wait for.  */
             receiveDataPendingWakeUpQueue.add(textureBlock, generatedTickets, computeTotalTransactionsToRequest(textureBlock));
        }
        else if (receiveDataPendingWakeUpQueue.exists(textureBlock)) 
        { 
             /* Already enqueued in the Wakeup queue.  */

             set<u32bit> generatedTickets;
             
             /*  For each block send as many read request as possible.  */ 
             allBlocksRequested = requestColorBufferBlocks(cycle, textureBlock, generatedTickets);
             
             /*  Update the Wakeup queue with the new generated tickets to wait for.  */
             receiveDataPendingWakeUpQueue.addEventsToEntry(textureBlock, generatedTickets);
             
             if (receiveDataPendingWakeUpQueue.full()) /*  Update statistic.  */
                 receiveDataPendingWakeUpQueueFull++;
        }
        else if (receiveDataPendingWakeUpQueue.full()) /*  Update statistic.  */
                 receiveDataPendingWakeUpQueueFull++;
        
        if ( allBlocksRequested )
        {
            GPU_DEBUG(
               printf("%s::Blitter %lld => Block (%d,%d) Fully requested.\n", transactionInterface.getBoxName().c_str(), cycle, textureBlock->xBlock, textureBlock->yBlock);
            )
            
            /*  If all read transactions generated remove head texture block.  */
            requestPendingQueue.remove();
        }
    }

    /**  
     *  First stage: Every texture block creation latency cycles, 
     *               fill the request pending queue with a new texture block.
     */
    if (colorBufferStateReady && !requestPendingQueue.full())
    {
        if (currentYBlock <= lastYBlock && currentXBlock <= lastXBlock)
        {
              /*  If no remaining cycles perform new texture block generation.  */
             if (newBlockCycles == 0)
             {   
                    /*  The texture block allocated space will be deleted after going out of the last queue.  */             
                 TextureBlock* textureBlock = new TextureBlock;
                 
                 textureBlock->xBlock = currentXBlock;
                 textureBlock->yBlock = currentYBlock;
     
                 /*  The texture block information such as the texture block address in memory,
                  *  the write mask, the color buffer blocks to request and mapping of pixels
                  *  to texels for each requested block, is generated using this function.
                  */
                 processNewTextureBlock(cycle, currentXBlock, currentYBlock, textureBlock);
 
                 GPU_DEBUG(
                    printf("%s::Blitter %lld => New block (%d,%d) generated.\n", transactionInterface.getBoxName().c_str(), cycle, textureBlock->xBlock, textureBlock->yBlock);
                    //cout << (*textureBlock) << endl;
                 )
                 
                 /*  Enqueue the texture block for the next stage processing.  */
                 requestPendingQueue.add(textureBlock);
                                      
                 if (currentXBlock == lastXBlock)
                 {
                     currentXBlock = startXBlock;
                     currentYBlock++;
                 }
                 else
                     currentXBlock++;
                 
                 /*  Reset new block generation remaining cycles.  */
                 newBlockCycles = newBlockLatency;
             }
             else 
             {
                 GPU_DEBUG(
                    printf("%s::Blitter %lld => New block generation remaining cycles = %d.\n", transactionInterface.getBoxName().c_str(), cycle, newBlockCycles);
                 )
 
                 /*  Decrement new block generation remaining cycles.  */
                 newBlockCycles--;
             }
         }        
    }
    if (requestPendingQueue.full()) /*  Update statistic.  */
        requestPendingQueueFull++;
}

/****************************************************************************************
 *                          Auxiliar functions implementation                           *
 ****************************************************************************************/
 
/*  Computes the maximum number of necessary color buffer blocks to fill a single texture block.  */
u32bit Blitter::computeColorBufferBlocksPerTextureBlock(u32bit TextureBlockSize2, u32bit ColorBufferBlockSize2)
{
    const u32bit& t = TextureBlockSize2;
    const u32bit& c = ColorBufferBlockSize2;
    
    u32bit base = (u32bit) ceil( (f32bit)(t - c) / 2.0f ) + 2;
    
    return base * base; // base^2 x 
}
 

/*  Translates texel coordinates to the morton address offset starting from the texture base address.  */
u32bit Blitter::texel2MortonAddress(u32bit i, u32bit j, u32bit blockDim, u32bit sBlockDim, u32bit width)
{
    u32bit address;
    u32bit texelAddr;
    u32bit blockAddr;
    u32bit sBlockAddr;

    /*  Compute the address of the texel inside the block using Morton order.  */
    texelAddr = GPUMath::morton(blockDim, i, j);

    /*  Compute the address of the block inside the superblock using Morton order.  */
    blockAddr = GPUMath::morton(sBlockDim, i >> blockDim, j >> blockDim);

    /*  Compute the address of the superblock inside the cache.  */
    sBlockAddr = ((j >> (sBlockDim + blockDim)) << GPU_MAX(s32bit(width - (sBlockDim + blockDim)), s32bit(0))) + (i >> (sBlockDim + blockDim));

    /*  Compute the final address.  */
    address = (((sBlockAddr << (2 * sBlockDim)) + blockAddr) << (2 * blockDim)) + texelAddr;

    return address;
}

/*  Returns the number of bytes of each texel for a texture using this format.  */
u32bit Blitter::getBytesPerTexel(TextureFormat format)
{
    /*  Only one format supported at the moment.  */
    switch(format)
    {
        case GPU_RGBA8888: return 4; 
            break;
        default:
            panic("Blitter", "getBytesPerTexel", "Unsupported blit destination texture format.");

            //  Remove VS2005 warning.
            return 0;
            break;
    }
}

/*  Decompresses a block according to the compression state and using the specified clearColor.  */
void Blitter::decompressBlock(u8bit* outBuffer, u8bit* inBuffer, u32bit colorBufferBlockSize, ROPBlockState state, QuadFloat clearColor)
{
    int i;
    
    u8bit clearColorData[MAX_BYTES_PER_COLOR];
    
    //  Convert the float point clear color to the color buffer format.
    switch(colorBufferFormat)
    {
        case GPU_RGBA8888:
        
            clearColorData[0] = u8bit(clearColor[0] * 255.0f);
            clearColorData[1] = u8bit(clearColor[1] * 255.0f);
            clearColorData[2] = u8bit(clearColor[2] * 255.0f);
            clearColorData[3] = u8bit(clearColor[3] * 255.0f);
            
            break;

        case GPU_RG16F:
        
            ((f16bit *) clearColorData)[0] = GPUMath::convertFP32ToFP16(clearColor[0]);
            ((f16bit *) clearColorData)[1] = GPUMath::convertFP32ToFP16(clearColor[1]);

            break;
            
        case GPU_R32F:
        
            ((f32bit *) clearColorData)[0] = clearColor[0];

            break;

        case GPU_RGBA16F:
        
            ((f16bit *) clearColorData)[0] = GPUMath::convertFP32ToFP16(clearColor[0]);
            ((f16bit *) clearColorData)[1] = GPUMath::convertFP32ToFP16(clearColor[1]);
            ((f16bit *) clearColorData)[2] = GPUMath::convertFP32ToFP16(clearColor[2]);
            ((f16bit *) clearColorData)[3] = GPUMath::convertFP32ToFP16(clearColor[3]);
            
            break;
            
        default:
            panic("Blitter", "decompressBlock", "Unsupported color buffer format.");
            break;
    }
    
    /*  Select compression method.  */
    switch (state.state)
    {
        case ROPBlockState::CLEAR:

            GPU_DEBUG(
                printf("%s::Blitter => Decompressing CLEAR block.\n", transactionInterface.getBoxName().c_str());
            )

            /*  Fill with the clear color.  */
            for (i = 0; i < colorBufferBlockSize ; i += bytesPixel)
                for(u32bit j = 0; j < (bytesPixel >> 2); j++)
                    *((u32bit *) &outBuffer[i + j * 4]) = ((u32bit *) clearColorData)[j];
            
            /*  Update stat.  */
            clearColorBlocks++;
                
            break;

        case ROPBlockState::UNCOMPRESSED:

            GPU_DEBUG(
                printf("%s::Blitter => Decompressing UNCOMPRESSED block.\n", transactionInterface.getBoxName().c_str());
            )
            
            /*  Fill with the read data.  */
            for (i = 0; i < colorBufferBlockSize ; i+= sizeof(u32bit))
                *((u32bit *) &outBuffer[i]) = *((u32bit *) &inBuffer[i]);

            /*  Update stat.  */
            uncompressedBlocks++;
                            
            break;

        case ROPBlockState::COMPRESSED:

            GPU_DEBUG(
                printf("%s::Blitter => Decompressing COMPRESSED LEVEL %i block.\n", 
                        transactionInterface.getBoxName().c_str(), state.comprLevel);
            )

            /*  Uncompress the block.  */
            ColorCompressorEmulator::getInstance().uncompress(
                    inBuffer, 
                    (u32bit *) outBuffer, 
                    colorBufferBlockSize >> 2, 
                    state.getComprLevel());

            /*  Update stat.  */
            switch (state.comprLevel) {
            case 0: compressedBestBlocks++; break;
            case 1: compressedNormalBlocks++; break;
            case 2: compressedWorstBlocks++; break;
            default:
                GPU_DEBUG(
                        printf("Warning: Blitter: There isn't statistic for compression level 2."); 
                )
                break;
            }
                            
            break;

        /*case ROP_BLOCK_COMPRESSED_BEST:

            GPU_DEBUG(
                printf("%s::Blitter => Decompressing COMPRESSED BEST block.\n", transactionInterface.getBoxName().c_str());
            )

              Uncompress the block.  
            FragmentOpEmulator::hiloUncompress(
                inBuffer,
                (u32bit *) outBuffer,
                colorBufferBlockSize >> 2,
                FragmentOpEmulator::COMPR_L1,
                ColorCacheV2::COMPRESSION_HIMASK_NORMAL,
                ColorCacheV2::COMPRESSION_HIMASK_BEST,
                ColorCacheV2::COMPRESSION_LOSHIFT_NORMAL,
                ColorCacheV2::COMPRESSION_LOSHIFT_BEST);
                
              Update stat.  
            compressedBestBlocks++;
            
            break;*/

        default:
            panic("Blitter", "decompressBlock", "Undefined compression method.");
            break;
    }
}

/*  Translates an address to the start of a block into a block number.  */
u32bit Blitter::address2block(u32bit address, u32bit blockSize)
{
    u32bit block;

    /*  Calculate the shift bits for block addresses.  */
    u32bit blockShift = GPUMath::calculateShift(blockSize);

    /*  Translate start block/line address to a block address.  */
    block = address >> blockShift;

    return block;
}

/*  Generates the proper write mask for the memory transaction.  */
void Blitter::translateBlockWriteMask(u32bit* outputMask, bool* inputMask, u32bit size)
{
    for (int i = 0; i < size; i++)
    {
        if (inputMask[i])
            ((u8bit *) outputMask)[i] = 0xff;
        else
            ((u8bit *) outputMask)[i] = 0x00;
    }
}
    
/*  Sends the next write memory transaction for the specified texture block.  */
bool Blitter::sendNextWriteTransaction(u64bit cycle, TextureBlock* textureBlock)
{
    /*  Compute the transaction size.  */
    u32bit size = GPU_MIN(MAX_TRANSACTION_SIZE, textureBlockSize - textureBlock->written);

    u32bit blockOffset = textureBlock->written;
    
    /*  Get the write mask in the memory transaction format.  */
    u32bit* writeMask = new u32bit[(u32bit) ((f32bit) textureBlockSize / (f32bit) sizeof(u32bit))];

    translateBlockWriteMask(writeMask, &textureBlock->writeMask[blockOffset], size);

    /*  Send the memory transaction using the related interface.  */    
    
    if (transactionInterface.sendMaskedWriteTransaction(
        cycle, 
        size, 
        textureBlock->address + blockOffset, 
        &textureBlock->contents[blockOffset],
        writeMask,
        0))
    {
        GPU_DEBUG(
              printf("%s::Blitter %lld => Writing memory at %x %d bytes for block %d.\n",
                     transactionInterface.getBoxName().c_str(), cycle, textureBlock->address + blockOffset, size, textureBlock->block);
        )
                    
        /*  Update requested block bytes counter.  */
        textureBlock->written += size;
     
        delete[] writeMask;            
        return true;
    }
    else
    {
        /*  No memory transaction generated.  */

        delete[] writeMask;
        return false;
    }
}

/*  Examines ready texture blocks and sends the next transaction of the head block.  */
bool Blitter::sendNextReadyTextureBlock(u64bit cycle, tools::Queue<TextureBlock*>& writePendingQueue)
{
    if (writePendingQueue.empty())
    {
        /*  No memory write transaction generated.  */
        return false;
    }
    else
    {
        TextureBlock* textureBlock;
        
        textureBlock = writePendingQueue.head();
        
        if (textureBlock->written == textureBlockSize)
        {
            /*  Remove head block and pick up the next one. */        
            writePendingQueue.remove();
            
            /*  Update block counter for finishing condition.  */
            blocksWritten++;
            
            /*  Update statistic.  */
            writtenBlocks++;

            GPU_DEBUG(
                    printf("%s::Blitter => Block (%d,%d) fully written to memory (%d of %d written).\n", transactionInterface.getBoxName().c_str(), textureBlock->xBlock, textureBlock->yBlock, blocksWritten, totalBlocksToWrite);
            )
            
            /*  Delete the allocated space for this object.  */
            delete textureBlock;
            
            if (writePendingQueue.empty())
            {
                /*  No memory write transaction generated.  */
                return false;
            }
            else
            {
                textureBlock = writePendingQueue.head();
                return sendNextWriteTransaction(cycle, textureBlock);
            }
        }
        else
        {
             return sendNextWriteTransaction(cycle, textureBlock);
        }
    }
}

/*  Generates a texture block with all the necessary information.  */
void Blitter::processNewTextureBlock(u64bit cycle, u32bit xBlock, u32bit yBlock, TextureBlock* textureBlock)
{

    u32bit texelsPerDim;
    u32bit texelsPerBlock;
    
    switch(blitDestinationBlocking)
    {
        case GPU_TXBLOCK_TEXTURE:
        
            //  Compute the number of horizontal or vertical texels in the morton block.
            texelsPerDim = (u32bit) GPU_POWER2OF((f64bit)mortonBlockDim);
            
            //  Compute total number of texels in the morton block.
            texelsPerBlock = (u32bit) GPU_POWER2OF ((f64bit) (mortonBlockDim << 1));
        
            break;

        case GPU_TXBLOCK_FRAMEBUFFER:
            
            panic("Blitter", "processNewTextureBlock", "Blits to textures in framebuffer format are not working.");
            
            texelsPerDim = STAMP_WIDTH;
            texelsPerBlock = STAMP_WIDTH * STAMP_HEIGHT;
            
            break;
        
        default:
        
            panic("Blitter", "processNewTextureBlock", "Unknown destination tile blocking format.");
        
            break;
    }
    
     
    //  Compute the texel coordinates of the bottom-left corner of the texture block.
    u32bit startBlockTexelX = xBlock * texelsPerDim;
    u32bit startBlockTexelY = yBlock * texelsPerDim;
    
    //  Compute the texel coordinate of the top-right corner of the texture block.
    u32bit endBlockTexelX = startBlockTexelX + texelsPerDim - 1;
    u32bit endBlockTexelY = startBlockTexelY + texelsPerDim - 1;
    

    //  Compute the bytes per texel according to the texture format.
    u32bit bytesTexel = getBytesPerTexel(blitDestinationTextureFormat);
    
    //  Bytes in a texture morton block.
    u32bit bytesPerBlock = texelsPerBlock * bytesTexel;
    
    //  Allocate the block mask space. Allocating one boolean for each block byte.
    bool* mask = new bool[bytesPerBlock];
    
    //  Initialize mask all to false.
    for (int i = 0; i < bytesPerBlock; i++)
        mask[i] = false;
    
    u32bit startBlockAddress;

    //  Compute the address (in fact, the offset from texture base address) 
    //  of the first texel inside the texture block. It depends on the destination
    //  texture blocking/tiling order. 
    //
    switch(blitDestinationBlocking)
    {
        case GPU_TXBLOCK_TEXTURE:
            
            //  Compute the morton address for the first corner texel of the block.
            startBlockAddress = texel2MortonAddress(startBlockTexelX, 
                                                    startBlockTexelY, 
                                                    mortonBlockDim, 
                                                    mortonSBlockDim, 
                                                    blitTextureWidth2) * bytesTexel;
                                                    
            break;

        case GPU_TXBLOCK_FRAMEBUFFER:

            //  Compute the framebuffer tiling/blocking address for the first corner texel of the block.
            startBlockAddress = pixelMapper.computeAddress(startBlockTexelX, startBlockTexelY);
            break;            
        
        default:
            panic("Blitter", "processNewTextureBlock", "Unknown destination tile blocking format.");
            break;
    }

//printf("%s::Blitter => Block texels: (%d,%d)->(%d,%d). Start address: %d\n", transactionInterface.getBoxName().c_str(), startBlockTexelX, startBlockTexelY, endBlockTexelX, endBlockTexelY, startBlockAddress);

    //  Fill the texture block address information.  */
    textureBlock->address = blitDestinationAddress + startBlockAddress;
    textureBlock->block = address2block(textureBlock->address, bytesPerBlock);
    
    //  Iterate over all the texels in the texture block, masking all the texels inside the covered texture region. 
    //  At the same time, the corresponding color buffer data blocks are stored in a set, and the correspondece/mapping between
    //  pixels and texels is stored.  
    //
    set<u32bit> colorBufferBlocks;
    
    map<u32bit, list<pair<u32bit,u32bit> > > correspondenceMap;
    
    for (int j = startBlockTexelY; j <= endBlockTexelY; j++)
    {        
        for (int i = startBlockTexelX; i <= endBlockTexelX; i++)
        {
            u32bit texelAddress;

            switch(blitDestinationBlocking)
            {
                case GPU_TXBLOCK_TEXTURE:

                    //  Compute the morton address for the texel.
                    texelAddress = texel2MortonAddress(i, j, mortonBlockDim, mortonSBlockDim, blitTextureWidth2) * bytesTexel;
                    break;

                case GPU_TXBLOCK_FRAMEBUFFER:

                    //  Compute the framebuffer tiling/blocking address for the first corner texel of the block */
                    texelAddress = pixelMapper.computeAddress(i, j);                    
                    break;

                default:                    
                    panic("Blitter", "processNewTextureBlock", "Unknown destination tile blocking format.");
                    break;
            }
              
            GPU_ASSERT(
                u32bit maxTexelAddress = bytesTexel * ((u32bit)GPU_POWER2OF((f64bit)blitTextureWidth2)) * ((u32bit)GPU_POWER2OF((f64bit)blitTextureWidth2));

                if (texelAddress > maxTexelAddress)
                {
                    printf("%s::Blitter => Texels: (%d,%d) inside the texture region. Texel address: %d \n", transactionInterface.getBoxName().c_str(), i, j, texelAddress);
                    panic("Blitter", "processNewTextureBlock", "Generated texture block address not in texture memory region.");
                }
            )
              
            // Ask if texel inside the texture region.
            if ( blitXOffset <= i && i < blitXOffset + blitWidth && blitYOffset <= j && j < blitYOffset + blitHeight)
            {
                int k;

                //  Compute the texture block offset of the texel.
                u32bit texelBlockOffset = texelAddress - startBlockAddress;
                  
// printf("%s::Blitter => Texels: (%d,%d) inside the texture region. Offset: %d \n", transactionInterface.getBoxName().c_str(), i, j, texelBlockOffset);

if ((texelBlockOffset + bytesTexel) > (bytesPerBlock))
{
u32bit a = 0;
}

                //  Set the mask for all the texel bytes.
                for (k = 0; k < bytesTexel; k++)
                    mask[texelBlockOffset + k] =  true;
                  
                //  Compute the color buffer data blocks to request.
                u32bit pixelX = (i - blitXOffset) + blitIniX;
                u32bit pixelY = (j - blitYOffset) + blitIniY;

                //  Compute the address of the corresponding texel in the source sourface.
                u32bit colorBufferAddress = pixelMapper.computeAddress(pixelX, pixelY);

                //  Compute the block and offset in the source surface.
                u32bit colorBufferBlock = colorBufferAddress >> GPUMath::calculateShift(colorBufferBlockSize);
                u32bit colorBufferBlockOffset = colorBufferAddress % colorBufferBlockSize;

                //  Insert color buffer block in the set (the set avoids repetitions).
                colorBufferBlocks.insert(colorBufferBlock);

                //  Insert the correspondences in the map (for all the texel bytes).
                for (k = 0; k < bytesTexel; k++)
                   correspondenceMap[colorBufferBlock].push_back(make_pair(colorBufferBlockOffset+k, texelBlockOffset+k));
             }
        }
    }
    
    //  Store the mask.
    textureBlock->writeMask = mask;
    
    //  Allocate space for the final contents of the texture block. This could be done somewhere else, 
    //  for instance, before performing swizzling.  
    //
    textureBlock->contents = new u8bit[bytesPerBlock];

    //  Initialize the color buffer blocks structures.
    for (set<u32bit>::const_iterator iter = colorBufferBlocks.begin(); iter != colorBufferBlocks.end(); iter++)
    {
        ColorBufferBlock* cbBlock = new ColorBufferBlock;
        
        cbBlock->address = (*iter) * colorBufferBlockSize;
        cbBlock->block = (*iter);
        cbBlock->state = colorBufferState[cbBlock->block];
        
        u32bit blockSize;
        
        //  Allocate space for the compressed color buffer block accordling to the block state.
        switch(cbBlock->state.state)
        {
            case ROPBlockState::CLEAR: blockSize = 0; break;
            case ROPBlockState::UNCOMPRESSED: blockSize = colorBufferBlockSize; break;
            case ROPBlockState::COMPRESSED: 
                blockSize = ColorCompressorEmulator::getInstance()
                                                        .getLevelBlockSize(cbBlock->state.comprLevel);
                break;
            default:
                panic("Blitter", "processNewTextureBlock", "Undefined compression method.");
                break;
        }

        if (blockSize > 0)
            cbBlock->compressedContents = new u8bit[blockSize];
        else
            cbBlock->compressedContents = NULL;
        
        //  Fill the correspoding information to request the 
        //  corresponding color buffer data in the next stages.  
        //
        for (int i = 0; i < colorBufferBlockSize; i++)
            cbBlock->swzMask.push_back(-1);
            
        list<pair<u32bit,u32bit> >::const_iterator iter2 = correspondenceMap[cbBlock->block].begin();
        
        while ( iter2 != correspondenceMap[cbBlock->block].end() )
        {
            cbBlock->swzMask[iter2->first] = iter2->second;
            iter2++;
        }

        //  Push the block into the texture block information.
        textureBlock->colorBufferBlocks.push_back(cbBlock);
    }
}

/*  Computes the total number of transactions needed to request all the color buffer blocks data of the texture block.  */
u32bit Blitter::computeTotalTransactionsToRequest(TextureBlock* textureBlock)
{
    vector<ColorBufferBlock*>::const_iterator iter = textureBlock->colorBufferBlocks.begin();
    
    u32bit totalTransactions = 0;
    
    while ( iter != textureBlock->colorBufferBlocks.end())
    {
        const ROPBlockState& block = (*iter)->state;
        
        switch(block.state)
        {
            case ROPBlockState::CLEAR: 

                /*  No request needed. */
                break;
                        
            case ROPBlockState::UNCOMPRESSED: 

                totalTransactions += (u32bit) ceil((f32bit) colorBufferBlockSize / (f32bit) MAX_TRANSACTION_SIZE );
                break;
               
            case ROPBlockState::COMPRESSED: 

                totalTransactions += (u32bit) ceil((f32bit) ColorCompressorEmulator::getInstance()
                       .getLevelBlockSize(block.comprLevel) / (f32bit) MAX_TRANSACTION_SIZE );
                break;

            default:
                panic("Blitter::TextureBlock", "computeTotalTransactionsToRequest", "Color buffer block state not expected");
        }
             
        iter++;
    }

    return totalTransactions;
}


/*  Sends as many memory read requests as allowed in one cycle for the color buffer blocks in the texture block.  */
bool Blitter::requestColorBufferBlocks(u64bit cycle, TextureBlock* textureBlock, set<u32bit>& generatedTickets)
{
    vector<ColorBufferBlock*>::const_iterator iter = textureBlock->colorBufferBlocks.begin();
    
    bool transactionSent = false;
    
    bool blockFinished = true;
    
    while ( iter != textureBlock->colorBufferBlocks.end() && blockFinished )
    {
        const ROPBlockState& block = (*iter)->state;
        
        switch(block.state)
        {
            case ROPBlockState::CLEAR: 

                /*  No request needed. */
                break;
                        
            case ROPBlockState::UNCOMPRESSED: 

                blockFinished = requestColorBlock(cycle,(*iter), colorBufferBlockSize, generatedTickets);
                
                //blockFinished = requestUncompressedBlock(cycle,(*iter), generatedTickets);
                
                break;
               
            case ROPBlockState::COMPRESSED: 

                blockFinished = requestColorBlock(cycle, (*iter), 
                                                  ColorCompressorEmulator::getInstance().getLevelBlockSize(block.comprLevel),
                                                  generatedTickets);
                                                  
                //switch (block.comprLevel) {
                //case 0: blockFinished = requestCompressedBestBlock(cycle,(*iter), generatedTickets); break;
                //case 1: blockFinished = requestCompressedNormalBlock(cycle,(*iter), generatedTickets); break;
                //case 2: // TODO: Implementar requestCompressedBlock para level 2
                    //break;
                //default: 
                //    panic("Blitter", "requestColorBufferBlocks", "Compression level not supported !!!");
                //}
                
                break;
                
            default:
                panic("Blitter::TextureBlock","print","Color buffer block state not expected");
        }
             
        iter++;
    }

    return blockFinished;
}

bool Blitter::requestColorBlock(u64bit cycle, ColorBufferBlock *bInfo, u32bit blockRequestSize, set<u32bit>& generatedTickets)
{
    if (bInfo->requested >= blockRequestSize)
        return true;

    /*  Compute transaction size.  */
    u32bit size = GPU_MIN(MAX_TRANSACTION_SIZE, blockRequestSize - bInfo->requested);
    
    u32bit blockOffset = bInfo->requested;
    
    u32bit requestAddress = bInfo->address + blockOffset + backBufferAddress;
    
    if (transactionInterface.sendReadTransaction(cycle, size, requestAddress, bInfo->compressedContents + blockOffset , nextTicket))
    {
        GPU_DEBUG(
              printf("%s::Blitter %lld => Requesting %d bytes at %x for color buffer block %d (ticket %d).\n",
                     transactionInterface.getBoxName().c_str(), cycle, size, requestAddress,
                     bInfo->block, nextTicket);
        )
                    
        bInfo->tickets.push_back(nextTicket);
        generatedTickets.insert(nextTicket);
        
        nextTicket++;
                    
        //  Update requested block bytes counter.
        bInfo->requested += size;
    }
    
    return false;
};

bool Blitter::requestUncompressedBlock(u64bit cycle, ColorBufferBlock *bInfo, set<u32bit>& generatedTickets)
{
    if (bInfo->requested >= colorBufferBlockSize)
        return true;

    /*  Compute transaction size.  */
    u32bit size = GPU_MIN(MAX_TRANSACTION_SIZE, colorBufferBlockSize);
    
    u32bit blockOffset = bInfo->requested;
    
    if (transactionInterface.sendReadTransaction(cycle, size, bInfo->address + blockOffset, bInfo->compressedContents + blockOffset , nextTicket))
    {
        GPU_DEBUG(
              printf("%s::Blitter %lld => Requesting %d bytes at %x for color buffer block %d (ticket %d).\n",
                     transactionInterface.getBoxName().c_str(), cycle, size, bInfo->address + blockOffset,
                     bInfo->block, nextTicket);
        )
                    
        bInfo->tickets.push_back(nextTicket);
        generatedTickets.insert(nextTicket);
        
        nextTicket++;
                    
        //  Update requested block bytes counter.
        bInfo->requested += size;
    }
    
    return false;
};

bool Blitter::requestCompressedNormalBlock(u64bit cycle, ColorBufferBlock *bInfo, set<u32bit>& generatedTickets)
{
    if (bInfo->requested >= ColorCacheV2::COMPRESSED_BLOCK_SIZE_NORMAL)
        return true;

    /*  Compute transaction size.  */
    u32bit size = GPU_MIN(MAX_TRANSACTION_SIZE, ColorCacheV2::COMPRESSED_BLOCK_SIZE_NORMAL);
    
    u32bit blockOffset = bInfo->requested;
    
    if (transactionInterface.sendReadTransaction(cycle, size, bInfo->address + blockOffset, bInfo->compressedContents + blockOffset , nextTicket))
    {
        GPU_DEBUG(
              printf("%s::Blitter %lld => Requesting %d bytes at %x for color buffer block %d (ticket %d).\n",
                     transactionInterface.getBoxName().c_str(), cycle, size, bInfo->address + blockOffset,
                     bInfo->block, nextTicket);
        )
                    
        bInfo->tickets.push_back(nextTicket);
        generatedTickets.insert(nextTicket);
        
        nextTicket++;
                    
        //  Update requested block bytes counter.
        bInfo->requested += size;
    }
    
    return false;
};

bool Blitter::requestCompressedBestBlock(u64bit cycle, ColorBufferBlock *bInfo, set<u32bit>& generatedTickets)
{
    if (bInfo->requested >= ColorCacheV2::COMPRESSED_BLOCK_SIZE_BEST)
        return true;

    /*  Compute transaction size.  */
    u32bit size = GPU_MIN(MAX_TRANSACTION_SIZE, ColorCacheV2::COMPRESSED_BLOCK_SIZE_BEST);
    
    u32bit blockOffset = bInfo->requested;
    
    if (transactionInterface.sendReadTransaction(cycle, size, bInfo->address + blockOffset, bInfo->compressedContents + blockOffset , nextTicket))
    {
        GPU_DEBUG(
              printf("%s::Blitter %lld => Requesting %d bytes at %x for color buffer block %d (ticket %d).\n",
                     transactionInterface.getBoxName().c_str(), cycle, size, bInfo->address + blockOffset,
                     bInfo->block, nextTicket);
        )
                    
        bInfo->tickets.push_back(nextTicket);
        generatedTickets.insert(nextTicket);
        
        nextTicket++;
                    
        //  Update requested block bytes counter.
        bInfo->requested += size;
    }
    
    return false;
};

/*  Decompresses the color buffer blocks data of the texture block.  */
void Blitter::decompressColorBufferBlocks(TextureBlock* textureBlock)
{
    std::vector<ColorBufferBlock*>::iterator iter = textureBlock->colorBufferBlocks.begin();
          
    while (iter != textureBlock->colorBufferBlocks.end())
    {
        (*iter)->contents = new u8bit[colorBufferBlockSize];

//printf("Blitter::decompressColorBufferBlocks => block %d address %x size %d | state = %d | level = %d\n",
//    (*iter)->block , (*iter)->address, (*iter)->requested, (*iter)->state.getState(), (*iter)->state.getComprLevel());
//printf("Compressed data (%d bytes) : \n    ", (*iter)->requested);
//for(u32bit b = 0; b < (*iter)->requested; b++)
//printf("%02x ", (*iter)->compressedContents[b]);
//printf("\n");
        
        decompressBlock((*iter)->contents, (*iter)->compressedContents, colorBufferBlockSize, (*iter)->state, clearColor);
        
        //  Check if saving the blit source data to a file is enabled.
        if (saveBlitSource)
        {
            //  Copy decompressed data to color buffer array.
            for(u32bit b = 0; b < colorBufferBlockSize; b += 4)
                *((u32bit *) &colorBuffer[(*iter)->address + b]) = *((u32bit *) &(*iter)->contents[b]);        
        }
        
//printf("(1) Uncompressed data (%d bytes) : \n    ", colorBufferBlockSize);
//for(u32bit b = 0; b < colorBufferBlockSize; b++)
//printf("%02x ", (*iter)->contents[b]);
//printf("\n");

//printf("(2) Uncompressed data (%d bytes) : \n    ", colorBufferBlockSize);
//for(u32bit b = 0; b < colorBufferBlockSize; b++)
//printf("%02x ", colorBuffer[(*iter)->address + b]);
//printf("\n");
        
        iter++;
    }
}

/*  Performs swizzling of the texture block contents.  */
void Blitter::swizzleTextureBlock(TextureBlock* textureBlock)
{
    std::vector<ColorBufferBlock*>::const_iterator iter = textureBlock->colorBufferBlocks.begin();
    
    u32bit position = 0;
    
    /*  Reset for new swizzling operation.  */
    swzBuffer.reset();
    
    /*   Write each color buffer block data and swizzling mask in one swizzling Buffer position.   */
    while ( iter != textureBlock->colorBufferBlocks.end() )
    {
        swzBuffer.writeInputData((*iter)->contents, position);
        swzBuffer.setSwzMask((*iter)->swzMask, position);
        position++;
        iter++;
    }

    /*  Perform swizzling.  */
    swzBuffer.swizzle();
    
    /*  Collect swizzled data.  */
    swzBuffer.readOutputData(textureBlock->contents, 0);
}

#define GAMMA(x) f32bit(GPU_POWER(f64bit(x), f64bit(1.0f / 2.2f)))
#define LINEAR(x) f32bit(GPU_POWER(f64bit(x), f64bit(2.2f)))
//#define GAMMA(x) (x)

//  Dump source data for the blit operation to a file.
void Blitter::dumpBlitSource(u32bit frameCounter, u32bit blitCounter)
{

    //  Check if saving the blit source data to a ppm file is enabled.
    if (!saveBlitSource)
        return;
        
    FILE *fout;
    char filename[30];
    u32bit address;
    s32bit x,y;

    /*  Create current frame filename.  */
    sprintf(filename, "blitop-f%04d-%04d.ppm", frameCounter, blitCounter);

    /*  Open/Create the file for the current frame.  */
    fout = fopen(filename, "wb");

    /*  Check if the file was correctly created.  */
    GPU_ASSERT(
        if (fout == NULL)
            panic("Blitter", "dumpBlitSource", "Error creating frame color output file.");
    )

    /*  Write file header.  */

    /*  Write magic number.  */
    fprintf(fout, "P6\n");

    /*  Write frame size.  */
    fprintf(fout, "%d %d\n", blitWidth, blitHeight);

    /*  Write color component maximum value.  */
    fprintf(fout, "255\n");

    u8bit red;
    u8bit green;
    u8bit blue;

    //  Check if multisampling is enabled.
    if (!multisampling)
    {
        /* Do this for the whole picture now */
        for(y = s32bit(blitIniY + blitHeight) - 1; y >= s32bit(blitIniY); y--)
        {
            for(x = s32bit(blitIniX); x < s32bit(blitIniX + blitWidth); x++)
            {
                /**  NOTE THERE SURE ARE FASTER WAYS TO DO THIS ...  **/
                /*  Calculate address from pixel position.  */
                //address = GPUMath::pixel2Memory(x, y, startX, startY, width, height, hRes, vRes,
                //    STAMP_WIDTH, STAMP_HEIGHT, genW, genH, scanW, scanH, overW, overH, 1, bytesPixel);
                address = pixelMapper.computeAddress(x, y);

                //  Convert data from the color color buffer to 8-bit PPM format.
                switch(colorBufferFormat)
                {
                    case GPU_RGBA8888:
                        red   = colorBuffer[address];
                        green = colorBuffer[address + 1];
                        blue  = colorBuffer[address + 2];
                        break;
                        
                    case GPU_RG16F:
                        red   = u8bit(GPUMath::convertFP16ToFP32(*((u16bit *) &colorBuffer[address + 0])) * 255.0f);
                        green = u8bit(GPUMath::convertFP16ToFP32(*((u16bit *) &colorBuffer[address + 2])) * 255.0f);
                        blue  = 0;
                        break;

                    case GPU_R32F:
                        red   = u8bit(*((f32bit *) &colorBuffer[address]) * 255.0f);
                        green = 0;
                        blue  = 0;
                        break;
                                                
                    case GPU_RGBA16F:
                        red   = u8bit(GPUMath::convertFP16ToFP32(*((u16bit *) &colorBuffer[address + 0])) * 255.0f);
                        green = u8bit(GPUMath::convertFP16ToFP32(*((u16bit *) &colorBuffer[address + 2])) * 255.0f);
                        blue  = u8bit(GPUMath::convertFP16ToFP32(*((u16bit *) &colorBuffer[address + 4])) * 255.0f);
                        break;
                }
                
                //  Write color to the file.
                fputc((char) red, fout);
                fputc((char) green, fout);
                fputc((char) blue, fout);
            }
        }
    }
    else
    {
        QuadFloat sampleColors[MAX_MSAA_SAMPLES];
        QuadFloat resolvedColor;
        QuadFloat referenceColor;
        QuadFloat currentColor;

        //  Resolve the whole framebuffer.
        for(y = (blitIniY + blitHeight) - 1; y >= blitIniY; y--)
        {
            for(x = blitIniX; x < s32bit(blitWidth); x++)
            {
                u32bit sampleX;
                u32bit sampleY;
                
                sampleX = x - GPU_MOD(x, STAMP_WIDTH);
                sampleY = y - GPU_MOD(y, STAMP_HEIGHT);
                //u32bit msaaBytesPixel = bytesPixel * msaaSamples;
                
                //  Calculate address for the first sample in the stamp.
                //address = GPUMath::pixel2Memory(sampleX, sampleY, startX, startY, width, height,
                //    hRes, vRes, STAMP_WIDTH, STAMP_HEIGHT, genW, genH,
                //    scanW, scanH, overW, overH, msaaSamples, bytesPixel);
                address = pixelMapper.computeAddress(sampleX, sampleY);
                    
                //  Calculate address for the first sample in the pixel.                    
                address = address + msaaSamples * (GPU_MOD(y, STAMP_HEIGHT) * STAMP_WIDTH + GPU_MOD(x, STAMP_WIDTH)) * bytesPixel;
                
                //  Zero resolved color.
                resolvedColor[0] = 0.0f;
                resolvedColor[1] = 0.0f;
                resolvedColor[2] = 0.0f;
                
                //  Convert data from the color color buffer 32 bit fp internal format.
                switch(colorBufferFormat)
                {
                    case GPU_RGBA8888:
                        referenceColor[0] = f32bit(colorBuffer[address + 0]) / 255.0f;
                        referenceColor[1] = f32bit(colorBuffer[address + 1]) / 255.0f;
                        referenceColor[2] = f32bit(colorBuffer[address + 2]) / 255.0f;
                        break;
                        
                    case GPU_RG16F:
                        referenceColor[0] = GPUMath::convertFP16ToFP32(*((u16bit *) &colorBuffer[address + 0]));
                        referenceColor[1] = GPUMath::convertFP16ToFP32(*((u16bit *) &colorBuffer[address + 2]));
                        referenceColor[2] = 0.0f;
                        break;
                        
                    case GPU_R32F:
                        referenceColor[0] = *((f32bit *) &colorBuffer[address]);
                        referenceColor[1] = 0.0f;
                        referenceColor[2] = 0.0f;
                        break;                        

                    case GPU_RGBA16F:
                        referenceColor[0] = GPUMath::convertFP16ToFP32(*((u16bit *) &colorBuffer[address + 0]));
                        referenceColor[1] = GPUMath::convertFP16ToFP32(*((u16bit *) &colorBuffer[address + 2]));
                        referenceColor[2] = GPUMath::convertFP16ToFP32(*((u16bit *) &colorBuffer[address + 4]));
                        break;
                }
                
                bool fullCoverage = true;

                //  Accumulate the color for all the samples in the pixel
                for(u32bit i = 0; i < msaaSamples; i++)
                {
                    //  Convert data from the color color buffer 32 bit fp internal format.
                    switch(colorBufferFormat)
                    {
                        case GPU_RGBA8888:
                            currentColor[0] = f32bit(colorBuffer[address + i * 4 + 0]) / 255.0f;
                            currentColor[1] = f32bit(colorBuffer[address + i * 4 + 1]) / 255.0f;
                            currentColor[2] = f32bit(colorBuffer[address + i * 4 + 2]) / 255.0f;
                            break;
                            
                        case GPU_RG16F:
                            currentColor[0] = GPUMath::convertFP16ToFP32(*((u16bit *) &colorBuffer[address + i * 4 + 0]));
                            currentColor[1] = GPUMath::convertFP16ToFP32(*((u16bit *) &colorBuffer[address + i * 4 + 2]));
                            currentColor[2] = 0.0f;
                            break;
                            
                        case GPU_R32F:
                            currentColor[0] = *((f32bit *) &colorBuffer[address + i * 4]);
                            currentColor[1] = 0.0f;
                            currentColor[2] = 0.0f;
                            break;                        
                                    
                        case GPU_RGBA16F:
                            currentColor[0] = GPUMath::convertFP16ToFP32(*((u16bit *) &colorBuffer[address + i * 8 + 0]));
                            currentColor[1] = GPUMath::convertFP16ToFP32(*((u16bit *) &colorBuffer[address + i * 8 + 2]));
                            currentColor[2] = GPUMath::convertFP16ToFP32(*((u16bit *) &colorBuffer[address + i * 8 + 4]));
                            break;
                    }

                    sampleColors[i][0] = currentColor[0];
                    sampleColors[i][1] = currentColor[1];
                    sampleColors[i][2] = currentColor[2];
                    
                    resolvedColor[0] += LINEAR(currentColor[0]);
                    resolvedColor[1] += LINEAR(currentColor[1]);
                    resolvedColor[2] += LINEAR(currentColor[2]);
                    
                    fullCoverage = fullCoverage && (referenceColor[0] == currentColor[0])
                                                && (referenceColor[1] == currentColor[1])
                                                && (referenceColor[2] == currentColor[2]);
                }
                
                //  Check if there is a single sample for the pixel
                if (fullCoverage)
                {
                    //  Convert from RGBA32F (internal format) to RGBA8 (PPM format)
                    red   = u8bit(referenceColor[0] * 255.0f);
                    green = u8bit(referenceColor[1] * 255.0f);
                    blue  = u8bit(referenceColor[2] * 255.0f);
                    
                    //  Write pixel color into the output file.
                    fputc(char(red)  , fout);
                    fputc(char(green), fout);
                    fputc(char(blue) , fout);
                }
                else
                {
                    //  Resolve color as the average of all the sample colors.
                    resolvedColor[0] = GAMMA(resolvedColor[0] / f32bit(msaaSamples));
                    resolvedColor[1] = GAMMA(resolvedColor[1] / f32bit(msaaSamples));
                    resolvedColor[2] = GAMMA(resolvedColor[2] / f32bit(msaaSamples));

                    //  Write resolved color into the output file.
                    fputc(char(resolvedColor[0] * 255.0f), fout);
                    fputc(char(resolvedColor[1] * 255.0f), fout);
                    fputc(char(resolvedColor[2] * 255.0f), fout);
                }
            }                    
        }                
    }
    
    fclose(fout);

    //  Remove the buffer where the data has been stored.
    delete [] colorBuffer;
}
