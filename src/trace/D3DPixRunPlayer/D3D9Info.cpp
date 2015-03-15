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

#include "stdafx.h"

#include "D3D9Info.h"

using namespace std;

/************************************************************************************/

int D3D9Info::getSize(D3DFORMAT f) 
{
    map<D3DFORMAT, unsigned long>::iterator it;
    
    it = formatSizes.find(f);
    if (it != formatSizes.end())
        return it->second;
    else
    {
        char buffer[256];
        sprintf(buffer, "Size of D3D9FORMAT %x has not been defined.", f);
        panic("D3D9Info", "getSize", buffer);
        return 0;
    }
}

bool D3D9Info::isCompressed(D3DFORMAT f)
{
    map<D3DFORMAT, bool>::iterator it;
    
    it = compressedFormats.find(f);
    
    if (it != compressedFormats.end())    
        return it->second;
    else
        return false;
}

D3D9Info &D3D9Info::instance() {
    static D3D9Info me;
    return me;
}

D3D9Info::D3D9Info() {

    // Compressed formats hold 16 pixels per texel
    compressedFormats[D3DFMT_DXT1] = true;
    compressedFormats[D3DFMT_DXT2] = true;
    compressedFormats[D3DFMT_DXT3] = true;
    compressedFormats[D3DFMT_DXT4] = true;
    compressedFormats[D3DFMT_DXT5] = true;
    compressedFormats[D3DFMT_ATI2] = true;

    // Hardcode info about sizes of different formats

    formatSizes[D3DFMT_A2R10G10B10] = 4;
    formatSizes[D3DFMT_A8R8G8B8] = 4;
    formatSizes[D3DFMT_X8R8G8B8] = 4;
    formatSizes[D3DFMT_A1R5G5B5] = 2;
    formatSizes[D3DFMT_X1R5G5B5] = 2;
    formatSizes[D3DFMT_R5G6B5] = 2;
    formatSizes[D3DFMT_D16_LOCKABLE] = 2;
    formatSizes[D3DFMT_D32] = 4;
    formatSizes[D3DFMT_D15S1] = 2;
    formatSizes[D3DFMT_D24S8] = 4;
    formatSizes[D3DFMT_D24X8] = 4;
    formatSizes[D3DFMT_D24X4S4] = 4;
    formatSizes[D3DFMT_D32F_LOCKABLE] = 4;
    formatSizes[D3DFMT_D24FS8] = 4;
    formatSizes[D3DFMT_D16] = 2;

    formatSizes[D3DFMT_DXT1] = 8;
    formatSizes[D3DFMT_DXT2] = 16;
    formatSizes[D3DFMT_DXT3] = 16;
    formatSizes[D3DFMT_DXT4] = 16;
    formatSizes[D3DFMT_DXT5] = 16;
    formatSizes[D3DFMT_ATI2] = 16;
    
    formatSizes[D3DFMT_NULL] = 0;

    formatSizes[D3DFMT_R16F] = 2;
    formatSizes[D3DFMT_G16R16F] = 4;
    formatSizes[D3DFMT_A16B16G16R16F] = 8;

    formatSizes[D3DFMT_MULTI2_ARGB8] = 4; // correct????
    formatSizes[D3DFMT_R8G8_B8G8]    = 2;
    formatSizes[D3DFMT_UYVY] = 2;
    formatSizes[D3DFMT_YUY2] = 2;

    formatSizes[D3DFMT_INDEX32] = 4;
    formatSizes[D3DFMT_INDEX16] = 2;
    formatSizes[D3DFMT_VERTEXDATA] = 0;

    formatSizes[D3DFMT_R32F] = 4;
    formatSizes[D3DFMT_G32R32F] = 8;
    formatSizes[D3DFMT_A32B32G32R32F] = 16;

    formatSizes[D3DFMT_L6V5U5] = 2;
    formatSizes[D3DFMT_X8L8V8U8] = 4;
    formatSizes[D3DFMT_A2W10V10U10] = 4;

    formatSizes[D3DFMT_V8U8] = 2;
    formatSizes[D3DFMT_Q8W8V8U8] = 4;
    formatSizes[D3DFMT_V16U16] = 4;
    formatSizes[D3DFMT_Q16W16V16U16] = 8;
    formatSizes[D3DFMT_CxV8U8] = 2;
  
    formatSizes[D3DFMT_R8G8B8] = 3;
    formatSizes[D3DFMT_A8R8G8B8] = 4;
    formatSizes[D3DFMT_X8R8G8B8] = 4;
    formatSizes[D3DFMT_R5G6B5] = 2;
    formatSizes[D3DFMT_X1R5G5B5] = 2;
    formatSizes[D3DFMT_A1R5G5B5] = 2;
    formatSizes[D3DFMT_A4R4G4B4] = 2;
    formatSizes[D3DFMT_R3G3B2] = 1;
    formatSizes[D3DFMT_A8] = 1;
    formatSizes[D3DFMT_A8R3G3B2] = 2;
    formatSizes[D3DFMT_X4R4G4B4] = 2;
    formatSizes[D3DFMT_A2B10G10R10] = 4;
    formatSizes[D3DFMT_A8B8G8R8] = 4;
    formatSizes[D3DFMT_X8B8G8R8] = 4;
    formatSizes[D3DFMT_G16R16] = 4;
    formatSizes[D3DFMT_A2R10G10B10] = 4;
    formatSizes[D3DFMT_A16B16G16R16] = 8;
    formatSizes[D3DFMT_A8P8] = 2; // ??????
    formatSizes[D3DFMT_P8] = 1;
    formatSizes[D3DFMT_L8] = 1;
    formatSizes[D3DFMT_L16] = 2;
    formatSizes[D3DFMT_A8L8] = 2;
    formatSizes[D3DFMT_A4L4] = 1;
}

bool D3D9Info::ispower2(unsigned int x) {
// Divide x by 2 checking for a remainder
// if it's found, then x isn't power of 2
bool foundRemainder = false;
    while((x != 1) & !foundRemainder) {
        if ((x % 2) != 0)
            foundRemainder = true;
        else
            x /= 2;
    }
    return !foundRemainder;
}

unsigned int D3D9Info::ilog2(unsigned int x) {
    unsigned int result = 0;
    while(x != 1) {
        result ++;
        x /= 2;
    }
    return result;
}

unsigned int D3D9Info::ipow(unsigned int x, unsigned int y) {
    unsigned int result = 1;
    for(unsigned int i = 0; i < y; i ++)
        result *= x;
    return result;
}

unsigned int D3D9Info::ceiledLog2(unsigned int x) {
    unsigned int result = 0;
    result = ilog2(x);
    // Ceil result if its not an exact power of 2
    if (!ispower2(x))
        result ++;
    return result;
}

unsigned int D3D9Info::get1DMipLength(UINT Length, UINT Level, UINT MaxLevel, bool Compressed) {
    // CeiledLog2length is the theoretical last level.
    UINT ceiledLog2Length = ceiledLog2(Length);
    // Ceiled length is the next power-of-2 greater or equal to length.
    UINT ceiledLength = ipow(2, ceiledLog2Length);

    // If level demanded is greater than maxlevel clamp it
    if((Level > (MaxLevel - 1)) & (MaxLevel != 0))
        Level = MaxLevel - 1;

    // The last two levels of a compressed mimpap have length 4
    if(Compressed & (Level > (ceiledLog2Length - 2))) {
        return 4;
    }
    // Check if demanded level is greater than the theoretical limit
    else if(Level > ceiledLog2Length) {
        return 1;
    }
    // Apply formula for calculate regular mipmap length
    else
        return ceiledLength / ipow(2, Level);
}

unsigned int D3D9Info::get2DMipArea(UINT Width, UINT Height, UINT Level, UINT MaxLevel, bool Compressed) {
    // Simply consider dimensions by separate.
    return get1DMipLength(Width, Level, MaxLevel, Compressed) *
           get1DMipLength(Height, Level, MaxLevel, Compressed);
}

unsigned int D3D9Info::get3DMipVolume(UINT Width, UINT Height, UINT Depth, UINT Level, UINT MaxLevel, bool Compressed) {
    // Consider dimensions by separate. The slices (width & height) are compressed but depth is not.
    return get1DMipLength(Width , Level, MaxLevel, Compressed) *
           get1DMipLength(Height, Level, MaxLevel, Compressed) *
           get1DMipLength(Depth , Level, MaxLevel, false);
}

unsigned int D3D9Info::get2DMipsTotalArea(UINT Width, UINT Height, UINT MaxLevel, bool Compressed) {
    // Calculate the highest level that can be reached
    unsigned int MaxCeiledLog2Length = max(ceiledLog2(Width), ceiledLog2(Height));
    unsigned int TopLevel;
    // 0 Maxlevel means all levels
    if(MaxLevel == 0)
        TopLevel = MaxCeiledLog2Length;
    else {
        TopLevel = MaxLevel - 1;
        // Limit to the highest if a greater is demanded
        if (TopLevel > MaxCeiledLog2Length)
            TopLevel = MaxCeiledLog2Length;
    }
    
    // Iterate accumulating area.
    unsigned int totalArea = 0;
    for(unsigned int i = 0; i <= TopLevel; i ++) 
        totalArea += get2DMipArea(Width, Height, i, MaxLevel, Compressed);
    return totalArea;
}

unsigned int D3D9Info::get3DMipsTotalVolume(UINT Width, UINT Height, UINT Depth, UINT MaxLevel,  bool Compressed) {
    // Calculate the highest level that can be reached
    unsigned int MaxCeiledLog2Length =
        max(max(ceiledLog2(Width), ceiledLog2(Height)), ceiledLog2(Depth));
    unsigned int TopLevel;
    // 0 Maxlevel means all levels
    if(MaxLevel == 0)
        TopLevel = MaxCeiledLog2Length;
    else {
        TopLevel = MaxLevel - 1;
        // Limit to the highest if a greater is demanded
        if (TopLevel > MaxCeiledLog2Length)
            TopLevel = MaxCeiledLog2Length;
    }

    // Iterate accumulating volume.
    unsigned int totalVolume = 0;
    for(unsigned int i = 0; i <= TopLevel; i ++) 
        totalVolume += get3DMipVolume(Width, Height, Depth, i, MaxLevel, Compressed );
    return totalVolume;
}

unsigned int D3D9Info::getTextureSize(
UINT Width, UINT Height, UINT Levels, D3DFORMAT Format) {
    unsigned int TexelSize = getSize(Format);
    unsigned int result = 0;
    if(!isCompressed(Format)) {
        return get2DMipsTotalArea(Width, Height, Levels) * TexelSize;
    }
    else {
        // Compressed textures always are formed of blocks of 4 * 4
        unsigned int blocksWidth = ((Width % 4) == 0) ? Width / 4 : Width / 4 + 1;
        unsigned int blocksHeight = ((Height % 4) == 0) ? Height / 4 : Height / 4 + 1;
        return get2DMipsTotalArea(blocksWidth * 4, blocksHeight * 4, Levels, true) / 16 * TexelSize;
        }
}

unsigned int D3D9Info::getCubeTextureSize(
    UINT EdgeLength, UINT Levels, D3DFORMAT Format) {
    unsigned int TexelSize = getSize(Format);

    if(!isCompressed(Format)) {
        return get2DMipsTotalArea(EdgeLength, EdgeLength, Levels) * TexelSize * 6;
    }
    else
    {
        // Compressed textures always are formed of blocks of 4 * 4
        unsigned int blocksEdgeLength = ((EdgeLength % 4) == 0) ? EdgeLength / 4 : EdgeLength / 4 + 1;
        return get2DMipsTotalArea(blocksEdgeLength * 4, blocksEdgeLength * 4, Levels, true) / 16 * TexelSize * 6;
    }
}

unsigned int D3D9Info::getVolumeTextureSize(
    UINT Width, UINT Height, UINT Depth, UINT Levels, D3DFORMAT Format) {
    D3D9Info &info = D3D9Info::instance();
    unsigned int TexelSize = info.getSize(Format);
    if(!info.isCompressed(Format)) {
        return get3DMipsTotalVolume(Width, Height, Depth, Levels) * TexelSize;
    }
    else {
        // Compressed textures always are formed of blocks of 4 * 4
        // Note depth is not considered in compression.
        unsigned int blocksWidth = ((Width % 4) == 0) ? Width / 4 : Width / 4 + 1;
        unsigned int blocksHeight = ((Height % 4) == 0) ? Height / 4 : Height / 4 + 1;
        return get3DMipsTotalVolume(blocksWidth * 4, blocksHeight * 4, Depth, Levels, true) / 16 * TexelSize;
    }
}
