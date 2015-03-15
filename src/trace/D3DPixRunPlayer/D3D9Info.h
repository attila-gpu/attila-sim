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

/**************************************************
Singleton that provides info about Direct3D9.

Author: José Manuel Solís
**************************************************/

#ifndef __D3D9INFO
#define __D3D9INFO

#include <map>
#include <stack>

/************************************************
Singleton that provides info about D3D9
************************************************/
class D3D9Info {

public:
    /************************
    @return size in bytes of an element    with this format.
    ************************/
    int getSize(D3DFORMAT f);

    /************************
    @return true for DXTn formats
    ************************/
    bool isCompressed(D3DFORMAT f);

    /****************************
    @param Levels to generate, 0 means all (until 1 x 1).
    @return Size in bytes of the texture.
    *****************************/
    unsigned int getTextureSize(UINT Width, UINT Height, UINT Levels, D3DFORMAT Format);

    /****************************
    @param Levels to generate, 0 means all (until 1 x 1).
    @return Size in bytes of the texture.
    *****************************/
    unsigned int getCubeTextureSize(UINT EdgeLength, UINT Levels, D3DFORMAT Format);

    /****************************
    @param Levels to generate, 0 means all (until 1 x 1).
    @return Size in bytes of the texture.
    *****************************/
    unsigned int getVolumeTextureSize(UINT Width, UINT Height, UINT Depth, UINT Levels, D3DFORMAT Format);

    static D3D9Info &instance();

private:
    D3D9Info();
    std::map< D3DFORMAT, unsigned long > formatSizes;
    std::map< D3DFORMAT, bool > compressedFormats;

    /****************************
    @return true if x is a power of 2 (i. e. 256)
    *****************************/
    bool ispower2(unsigned int x);
    /****************************
    @return integer part of log2(x)
    *****************************/
    unsigned int ilog2(unsigned int x);
    /****************************
    @return x^y
    *****************************/
    unsigned int ipow(unsigned int x, unsigned int y);
    /****************************
    @return log2(x) ceiled if it has fractional part.
    *****************************/
    unsigned int ceiledLog2(unsigned int x);
    /****************************
    @param MaxLevel Maximum level allowed, 0 means all.
    @return length in texels of the level of the mipmap.
    *****************************/
    unsigned int get1DMipLength(UINT Length, UINT Level, UINT MaxLevel, bool Compressed = false);
    /****************************
    @param MaxLevel Maximum level allowed, 0 means all.
    @return area in texels of the level of the mipmap.
    *****************************/
    unsigned int get2DMipArea(UINT Width, UINT Height, UINT Level, UINT MaxLevel, bool Compressed = false);
    /****************************
    @param MaxLevel Maximum level allowed, 0 means all.
    @return volume in texels of the level of the mipmap.
    *****************************/
    unsigned int get3DMipVolume(UINT Width, UINT Height, UINT Depth, UINT Level, UINT MaxLevel, bool Compressed = false);

    /****************************
    @param MaxLevel Maximum level allowed, 0 means all.
    @return sum of area in texels of all mipmaps.
    *****************************/
    unsigned int get2DMipsTotalArea(UINT Width, UINT Height, UINT MaxLevel, bool Compressed = false);

    /****************************
    @param MaxLevel Maximum level allowed, 0 means all.
    @return sum of volume in texels of all mipmaps.
    *****************************/
    unsigned int get3DMipsTotalVolume(UINT Width, UINT Height, UINT Depth, UINT MaxLevel,  bool Compressed = false);

};

#endif
