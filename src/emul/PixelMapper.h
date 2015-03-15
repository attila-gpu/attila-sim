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
 * Map pixels to addresses and processing units.
 *
 */

/**
 *
 * @file PixelMapper.h
 *
 * Defines a class used to map pixels to memory addresses or processing units 
 * applying tiling and different distribution algorithms.
 *
 */

#include "GPUTypes.h"
#include "GPU.h"

#ifndef _PIXELMAPPER_

#define _PIXELMAPPER_

namespace gpu3d
{

/**
 *
 *  Defines a class to map pixels to memory addresses and processing units.
 *
 */
class PixelMapper
{
private:

    //  Display parameters.
    
    u32bit hRes;            /**<  Display horizontal resolution.  */
    u32bit vRes;            /**<  Display vertical resolution.  */
    u32bit samples;         /**<  Samples per pixel.  */
    u32bit bytesSample;     /**<  Bytes per sample.  */ 
    
    //  Tiling parameters.
    u32bit overTileWidth;   /**<  Over scan tile width in scan tiles.  */
    u32bit overTileHeight;  /**<  Over scan tile height in scan tiles.  */
    u32bit scanTileWidth;   /**<  Scan tile width in generation tiles.  */
    u32bit scanTileHeight;  /**<  Scan tile height in generation tiles.  */
    u32bit genTileWidth;    /**<  Generation tile width in stamp tiles.  */
    u32bit genTileHeight;   /**<  Generation tile height in stamp tiles.  */
    u32bit stampTileWidth;  /**<  Stamp tile width in pixels.  */
    u32bit stampTileHeight; /**<  Stamp tile height in pixels.  */
    
    //  Processing units parameters.
    
    u32bit numUnits;            /**<  Number of processing units to which pixels are mapped.  */
    u32bit memUnitInterleaving; /**<  Memory interleaving between processing units.  */
    
    //  Precomputed parameters.
    u32bit overTilePixelWidth;  /**<  Width of the over scan tile in pixels.  */
    u32bit overTilePixelHeight; /**<  Height of the over scan tile in pixels.  */
    u32bit overTileRowWidth;    /**<  Number of over scan tiles per over scan tile row.  */ 
    u32bit overTileRows;        /**<  Number of over scan tile rows.  */
    u32bit overTileSize;        /**<  Size of the over scan tile in scan tiles.  */
    u32bit overTilePixelSize;   /**<  Size of the over scan tile in pixels.  */
    u32bit scanTilePixelWidth;  /**<  Width of the scan tile in pixels.  */
    u32bit scanTilePixelHeight; /**<  Height of the scan tile in pixels.  */    
    u32bit scanTileSize;        /**<  Size of the scan tile in generation tiles.  */
    u32bit scanTilePixelSize;   /**<  Size of the scan tile in pixels.  */
    u32bit scanSubTileWidth;    /**<  Scan sub tile width in adjusted generation tiles.  */
    u32bit scanSubTileHeight;   /**<  Scan sub tile height in adjusted generation tiles.  */
    u32bit scanSubTileSize;     /**<  Size of the scan sub tile in generation tiles.  */
    u32bit scanSubTilePixelWidth;   /**<  Scan sub tile width in pixels.  */
    u32bit scanSubTilePixelHeight;  /**<  Scan sub tile heigh in pixels.  */
    u32bit scanSubTileBytes;        /**<  Bytes per scan sub tile.  */
    u32bit genAdjTileWidth;         /**<  Adjusted generation tile width in stamp tiles.  */
    u32bit genAdjTileHeight;        /**<  Adjusted generation tile height in stamp tiles.  */
    u32bit genAdjTileSize;          /**<  Adjusted generation tile size in stamp tiles.  */
    u32bit genAdjTilePixelWidth;    /**<  Adjusted generation tile width in pixels.  */
    u32bit genAdjTilePixelHeight;   /**<  Adjusted generation tile height in pixels.  */
    u32bit genAdjTilePixelSize;     /**<  Adjusted generation tile pixel size.  */
    u32bit stampTileSize;           /**<  Stamp tile size in pixels.  */
    
    //  Precomputed Morton table.
    u8bit mortonTable[256];         /**<  Precomputed table to be used for Morton order mapping.  */
    
    
    //  Private functions.
    
    
    /**
     *
     *  Build the precomputed Morton table to be used for fast Morton order mapping.
     *
     */
     
    void buildMortonTable();
    
    /**
     *
     *  Maps an element inside a square power of two tile (2^size x 2^size) using Morton order.
     *  This function implements a fast version of the Morton mapping algorithm using the
     *  precomputed Morton table.
     *
     *  @param size Log2(dimension of tile).
     *  @param i Horizontal position of the element inside the tile.
     *  @param j Vertical position of the element inside the tile.
     *
     *  @return The position of the element in Morton order.
     *
     */
     
    u32bit fastMapMorton(u32bit size, u32bit i, u32bit j);
     
    /**
     *
     *  Maps an element inside a square power of two tile (2^size x 2^size) using Morton order.
     *
     *  @param size Log2(dimension of tile).
     *  @param i Horizontal position of the element inside the tile.
     *  @param j Vertical position of the element inside the tile.
     *
     *  @return The position of the element in Morton order.
     *
     */
     
    u32bit mapMorton(u32bit size, u32bit i, u32bit j);
    
    
public:

    /**
     *
     *  Class constructor.
     *
     */
     
    PixelMapper();
    
    /**
     *
     *  Sets the display parameters for the mapping.
     *
     *  @param xRes Display horizontal resolution.
     *  @param yRes Display vertical resolution.
     *  @param stampW The width in pixels of a stamp.
     *  @param stampH The height in pixels of a stamp.
     *  @param genTileW The generation tile width in stamps.
     *  @param genTileH The generation tile height in stamps.
     *  @param scanTileW The scan tile width in generation tiles.
     *  @param scanTileH The scan tile height in generation tiles.
     *  @param overTileW The over scan tile width in scan tiles.
     *  @param overTileH The over scan tile height in scan tiles.
     *  @param samples Number of samples per pixel.
     *  @param bytesSample Bytes per sample.
     *
     */
     
    void setupDisplay(u32bit xRes, u32bit yRes, u32bit stampW, u32bit stampH,
                      u32bit genW, u32bit genH, u32bit scanW, u32bit scanH,
                      u32bit overW, u32bit overH,
                      u32bit samples, u32bit bytesSample);
                      
    /**
     *
     *  Sets the processing unit parameters for the mapping.
     *
     *  @param numUnits Number of processing units between which the pixels are distributed.
     *  @param memInterleaving Memory interleaving between the processing units.
     *
     */
  
    void setupUnit(u32bit numUnits, u32bit memInterleaving);
    
    /**
     *
     *  Changes the display resolution (used in TextureEmulator).
     *
     *  @param hRes Horizontal display resolution.
     *  @param vRes Vertical display resolution.
     *
     */
     
    void changeResolution(u32bit hRes, u32bit vRes);
         
    /**
     *
     *  Computes the memory address corresponding with the given pixel position.
     *
     *  @param x Pixel horizontal position.
     *  @param y Pixel vertical position.     
     *
     *  @return The memory address corresponding with the defined pixel position.
     *
     */
    
    u32bit computeAddress(u32bit x, u32bit y);
        
    /**
     *
     *  Maps the defined pixel position to a processing unit.
     *
     *  @param x Pixel horizontal position.
     *  @param y Pixel vertical position.
     *
     *  @return The identifier of the processing unit to which the pixel is mapped.
     *
     */
          
    u32bit mapToUnit(u32bit x, u32bit y);
    
    /**
     *
     *  Maps the defined pixel address to a processing unit.
     *
     *  @param address Pixel address.
     *
     *  @return The identifier of the processing unit to which the pixel is mapped.
     *
     */
          
    u32bit mapToUnit(u32bit address);

    /**
     *
     *  Compute framebuffer size.
     *     
     */
     
    u32bit computeFrameBufferSize();
    
    
}; // class PixelMapper

} // namespace gpu3d

#endif
