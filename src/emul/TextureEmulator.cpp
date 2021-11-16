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
 * $RCSfile: TextureEmulator.cpp,v $
 * $Revision: 1.35 $
 * $Author: vmoya $
 * $Date: 2008-03-02 19:09:16 $
 *
 * Texture Emulator class implementation file.
 *
 */

/**
 *
 *  @file TextureEmulator.cpp
 *
 *  This file implements Texture Emulator Operation class.
 *
 *  This class implements functions that emulate the functionality
 *  of a texture unit in a GPU (address calculation, mipmap selection,
 *  filtering, texture data decompression, texture data conversion).
 *
 */

#include "TextureEmulator.h"
#include "GPUMath.h"
#include <stdio.h>
#
using namespace gpu3d;

//  Texture Emulator constructor.
TextureEmulator::TextureEmulator(u32bit stampFrags, u32bit blockDim, u32bit superBlockDim, u32bit anisoAlg,
                    bool forceAniso, u32bit maxAniso, u32bit triPrecision, u32bit briThreshold,
                    u32bit anisoRoundPrec, u32bit _anisoRoundThreshold, bool _anisoRatioMultOfTwo,
                    u32bit overScanWidth, u32bit overScanHeight, u32bit scanWidth, u32bit scanHeight,
                    u32bit genWidth, u32bit genHeight)
{
    //  Set number of fragments per stamp.
    stampFragments = stampFrags;

    //  Set texture cache and texture tiling/blocking parameters.
    textCacheBlockDim = blockDim;
    textCacheSBlockDim = superBlockDim;
    textCacheBlockSize = (1 << blockDim) * (1 << blockDim);
    textCacheSBlockSize = (1 << superBlockDim) * (1 << superBlockDim);
    
    //  Set framebuffer tiling/blocking 
    overScanTileWidth = overScanWidth;
    overScanTileHeight = overScanHeight;
    scanTileWidth = scanWidth / genWidth;
    scanTileHeight = scanHeight / genHeight;
    genTileWidth = genWidth / STAMP_WIDTH;
    genTileHeight = genHeight / STAMP_HEIGHT;

    //  Set trilinear/brilinear settings.
    trilinearPrecision = triPrecision;
    brilinearThreshold = briThreshold;
        
    GPU_ASSERT(
        if ((trilinearPrecision == 0) || (trilinearPrecision > 32))
            panic("TextureEmulator", "TextureEmulator", "Trilinear precision valid range is [1, 32].");
        if (brilinearThreshold > u32bit(1 << (trilinearPrecision - 1)))
            panic("TextureEmulator", "TextureEmulator", "Brilinear threshold must be less than the 2^(precision - 1).");
        if (maxAniso > 16)
            panic("TextureEmulator", "TextureEmulator", "Maximum anisotropy supported is 16.");            
    )
        
    trilinearRangeMin = f32bit(brilinearThreshold) / f32bit(u64bit(1) << trilinearPrecision );
    trilinearRangeMax = 1.0f - trilinearRangeMin;
            
    //  Set anisotropic settings.
    anisoAlgorithm = anisoAlg;
    forceMaxAnisotropy = forceAniso;
    confMaxAniso = maxAniso;
    anisoRoundPrecision = anisoRoundPrec;
    anisoRoundThreshold = _anisoRoundThreshold;
    anisoRatioMultOfTwo = _anisoRatioMultOfTwo;    

    GPU_ASSERT(
        if ((anisoRoundPrecision == 0) || (anisoRoundPrecision > 32))
            panic("TextureEmulator", "TextureEmulator", "Aniso round precision valid range is [1, 32]");
        if (u64bit(anisoRoundThreshold) > (u64bit(1) << anisoRoundPrecision))
            panic("TextureEmulator", "TextureEmulator", "Aniso round threshold must be less than 2^prec.");
    )
    
    anisoRoundUp = f32bit(anisoRoundThreshold) / f32bit(u64bit(1) << anisoRoundPrecision);
    
    if (anisoRatioMultOfTwo)
        anisoRoundUp = 2.0f * anisoRoundUp;

    //  Set display parameters in the pixel mappers.
    for(u32bit tu = 0; tu < MAX_TEXTURES; tu++)
        texPixelMapper[tu].setupDisplay(1024, 1024, STAMP_WIDTH, STAMP_HEIGHT,
                                        genTileWidth, genTileHeight,
                                        scanTileWidth, scanTileHeight,
                                        overScanTileWidth, overScanTileHeight,
                                        1, 1);

    GPU_ASSERT(
        if (stampFragments != 4)
            panic("TextureEmulator", "TextureEmulator", "Only supported stamps with 4 fragments in a quad.");
    )

}

/*  Calculates the texel address for a stamp.  */
TextureAccess *TextureEmulator::textureOperation(u32bit id, TextureOperation texOp, QuadFloat *stampCoord, f32bit *stampParameter, u32bit textUnit)
{
    TextureAccess *textAccess;
    f32bit dudx, dudy;
    f32bit dvdx, dvdy;
    f32bit dwdx, dwdy;
    f32bit scale;
    f32bit lod;
    u32bit levelBase;
    u32bit q;
    u32bit d1, d2;
    FilterMode filter;
    u32bit frag;
    CubeMapFace cubemap;
    u32bit anisoSamples;
    f32bit dsOffset;
    f32bit dtOffset;
    u32bit i;

/*if (id == 488)
{
printf("TextureEmulator::textureOperation => Texture Operation id = %d texOp = %d textUnit = %d\n", id, texOp, textUnit);
for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
printf("   Pixel %02d -> coord = {%f, %f, %f, %f} param = %f\n",
f, stampCoord[f][0], stampCoord[f][1], stampCoord[f][2], stampCoord[f][3], stampParameter[f]);
}*/

    //  Create a texture access for reading a vertex attribute stream.
    if (texOp == ATTRIBUTE_READ)
    {
        //  Create new texture access object.
        textAccess = new TextureAccess(id, texOp, stampCoord, stampParameter, textUnit);

        //  Copy the saved original coordinates to the texture access object.
        for(i = 0; i < STAMP_FRAGMENTS; i++)
            textAccess->originalCoord[i] = stampCoord[i];

        //  Set anisotropy parameters for the texture access.
        textAccess->anisoSamples = 1;
        textAccess->currentAnisoSample = 1;
        textAccess->anisodsOffset = 0.0f;
        textAccess->anisodtOffset = 0.0f;

        //  Reset number of magnified pixels in the request.
        textAccess->magnified = 0;
        
        //  Reset pointers for the trilinear samples to process.
        textAccess->nextTrilinearFetch = 0;
        textAccess->trilinearFiltered = 0;
        textAccess->trilinearToFilter = 1;
        textAccess->addressCalculated = false;

        //  Allocate trilinear samples when the addresses are calculated to reduce the number of
        //  active Trilinear objects!!
        textAccess->trilinear[0] = NULL;
        
        for(frag = 0; frag < STAMP_FRAGMENTS; frag++)
        {
            //  Clear texture sample.
            textAccess->sample[frag][0] = textAccess->sample[frag][1] =
            textAccess->sample[frag][2] = textAccess->sample[frag][3] = 0.0f;
            textAccess->texelSize[frag] = 0;  // Will be computed.
        }
        
        return textAccess;    
    }
    
    QuadFloat origCoord[STAMP_FRAGMENTS];

    //  Save the original coordinates.
    for(i = 0; i < STAMP_FRAGMENTS; i++)
        origCoord[i] = stampCoord[i];

    //  Get base mipmap level for the texture unit.
    levelBase = textureMinLevel[textUnit];

    //  Calculate scale factor for the stamp.
    switch(textureMode[textUnit])
    {
        case GPU_TEXTURE1D:

            //  Check if the lod is defined as a parameter.
            if (texOp != TEXTURE_READ_WITH_LOD)
            {
                //  Calculate stamp derivatives.
                derivativesXY(stampCoord, textUnit, dudx, dudy, dvdx, dvdy, dwdx, dwdy);

                scale = calculateScale1D(dudx, dudy);
            }
            
            //  Anisotropic filtering not supported for 1D textures.
            anisoSamples = 1;
            
            //  Calculate the maximum lod for the texture.
            q = GPU_MIN(textureWidth2[textUnit] + levelBase, textureMaxLevel[textUnit]);

            break;

        case GPU_TEXTURE2D:

            //  No cubemap.  Use first index.
            cubemap = GPU_CUBEMAP_POSITIVE_X;

            //  Check if the lod is defined as a parameter.
            if (texOp != TEXTURE_READ_WITH_LOD)
            {
                //  Calculate stamp derivatives.
                derivativesXY(stampCoord, textUnit, dudx, dudy, dvdx, dvdy, dwdx, dwdy);

                scale = calculateScale2D(textUnit, dudx, dudy, dvdx, dvdy, maxAnisotropy[textUnit], anisoSamples, dsOffset, dtOffset);
            }
            else
            {
                //  Anisotropic filtering not supported when the lod is defined for the fragment.                
                anisoSamples = 1;
                dsOffset = 0;
                dtOffset = 0;
            }
            
            //  Calculate the maximum lod for the texture.
            q = GPU_MIN(GPU_MAX(textureWidth2[textUnit], textureHeight2[textUnit])
                 + levelBase, textureMaxLevel[textUnit]);

            break;

        case GPU_TEXTURE3D:

            //  Check if the lod is defined as a parameter.
            if (texOp != TEXTURE_READ_WITH_LOD)
            {
                //  Calculate stamp derivatives.
                derivativesXY(stampCoord, textUnit, dudx, dudy, dvdx, dvdy, dwdx, dwdy);

                scale = calculateScale3D(dudx, dudy, dvdx, dvdy, dwdx, dwdy);
            }
            
            //  Anisotropic filtering not supported for 3D textures.
            anisoSamples = 1;

            //  Calculate the maximum lod for the texture.
            q = GPU_MIN(GPU_MAX(textureWidth2[textUnit],
                GPU_MAX(textureHeight2[textUnit], textureDepth2[textUnit]))+ levelBase,
                textureMaxLevel[textUnit]);

            break;

        case GPU_TEXTURECUBEMAP:

            //  Determine cubemap face for the whole stamp and calculate the stamp coordinates for the selected face.
            cubemap = selectCubeMapFace(stampCoord);

            //  Check if the lod is defined as a parameter.
            if (texOp != TEXTURE_READ_WITH_LOD)
            {
                //  Calculate stamp derivatives.
                derivativesXY(stampCoord, textUnit, dudx, dudy, dvdx, dvdy, dwdx, dwdy);

                scale = calculateScale2D(textUnit, dudx, dudy, dvdx, dvdy, maxAnisotropy[textUnit], anisoSamples, dsOffset, dtOffset);
            }
            else
            {
                //  Anisotropic filtering not supported when the lod is defined for the fragment.
                anisoSamples = 1;
            }
            
            //  Calculate the maximum lod for the texture.
            q = GPU_MIN(GPU_MAX(textureWidth2[textUnit], textureHeight2[textUnit])
                 + levelBase, textureMaxLevel[textUnit]);

            break;

        default:
            panic("TextureEmulator", "textureOperation", "Unsupported texture mode.");
            break;
    }

    //  Create new texture access object.
    textAccess = new TextureAccess(id, texOp, stampCoord, stampParameter, textUnit);

    //  Copy the saved original coordinates to the texture access object.
    for(i = 0; i < STAMP_FRAGMENTS; i++)
        textAccess->originalCoord[i] = origCoord[i];

    //  Set anisotropy parameters for the texture access.
    textAccess->anisoSamples = anisoSamples;
    textAccess->currentAnisoSample = 1;
    textAccess->anisodsOffset = dsOffset;
    textAccess->anisodtOffset = dtOffset;

    //  Reset number of magnified pixels in the request.
    textAccess->magnified = 0;
    
    //  Reset pointers for the trilinear samples to process.
    textAccess->nextTrilinearFetch = 0;
    textAccess->trilinearFiltered = 0;
    textAccess->trilinearToFilter = anisoSamples;
    textAccess->addressCalculated = false;

    //  Allocate trilinear samples when the addresses are calculated to reduce the number of
    //  active Trilinear objects!!
    for(i = 0; i < anisoSamples; i++)
        textAccess->trilinear[i] = NULL;

    //  Calculate mipmaps for each fragment in the texture access.
    for(frag = 0; frag < STAMP_FRAGMENTS; frag++)
    {
        //  Check if the lod is defined as a parameter.
        if (texOp == TEXTURE_READ_WITH_LOD)
        {
            //  Calculate the lod for each fragment.
            lod =  calculateLOD(textUnit, stampParameter[frag], 1.0f);
//if (id == 488)
//{
//printf("TXL => lod = %f\n", lod);
//}
        }
        else
        {
            //  Calculate the lod for each fragment.
            lod =  calculateLOD(textUnit, stampParameter[frag], scale);
//if (id == 480)
//{
//printf("TEX => lod = %f\n", lod);            
//}
        }
//printf("frag %d > lod: %f  C1: %f  C2: %f\n", frag, lod, C1, C2);

        //  Calculate mipmap level for each fragment.

        //  Select between magnification and minification.
        if (((lod > C1) && (textureMagFilter[textUnit] == GPU_LINEAR)
            && ((textureMinFilter[textUnit] == GPU_NEAREST_MIPMAP_NEAREST)
            || (textureMinFilter[textUnit] == GPU_NEAREST_MIPMAP_LINEAR)))
            || (lod > C2))
        {
            //  Minification.
            switch(textureMinFilter[textUnit])
            {
                case GPU_NEAREST:
                case GPU_LINEAR:

                    //  Not use mipmap.
                    d1 = levelBase;

                    break;

                case GPU_NEAREST_MIPMAP_NEAREST:
                case GPU_LINEAR_MIPMAP_NEAREST:

                    //  Use only the nearest mipmap level.
                    d1 = (lod <= 0.5f)?levelBase:((f32bit(levelBase) + lod) <= (f32bit(q) + 0.5f))?
                        (u32bit(GPU_CEIL(f32bit(levelBase) + lod + 0.5f)) - 1):q;

//printf("frag %d > lod: %f  d1: %d  levelBase: %d  q: %d\n", frag, lod, d1, levelBase, q);

                    break;

                case GPU_NEAREST_MIPMAP_LINEAR:
                case GPU_LINEAR_MIPMAP_LINEAR:

                    //  Trilinear:  access the two nearest mipmaps.
                    //d1 = ((f32bit(levelBase) + lod) >= f32bit(q))?q:u32bit(GPU_FLOOR(f32bit(levelBase) + lod));
                    d1 = levelBase + u32bit(GPU_FLOOR(lod));
                    //d1 = levelBase + u32bit(lod);
                    d1 = (d1 <= q)?d1:q;

                    //d2 = ((f32bit(levelBase) + lod) >= f32bit(q))?q:d1 + 1;
                    d2 = (d1 < q)?d1 + 1:q;

//printf("frag %d > lod: %f  d1: %d d2: %d levelBase: %d  q: %d\n", frag, lod, d1, d2, levelBase, q);

                    break;

                default:
                    panic("TextureEmulator", "textureOperation", "Unsupported filter mode.");
            }

            //  Set filter mode.
            filter = textureMinFilter[textUnit];
        }
        else
        {
            // Magnification.
            d1 = levelBase;

            //  Set filter mode.
            filter = textureMagFilter[textUnit];
            
            // Update the number of pixels being magnified.
            textAccess->magnified++;
        }

        //  Set fragment lod, mipmap levels accessed and filter kernel to use.
        textAccess->lod[frag] = lod;
        textAccess->level[frag][0] = d1;
        textAccess->level[frag][1] = d2;
        textAccess->filter[frag] = filter;

        //  Clear fragment sample.
        textAccess->sample[frag][0] = textAccess->sample[frag][1] =
        textAccess->sample[frag][2] = textAccess->sample[frag][3] = 0.0f;

        //  Set bytes to read per texel.
        textAccess->texelSize[frag] = GPU_MAX(u32bit(4), u32bit(adjustToFormat(textUnit, 1)));
        
//printf("<< id %d frag %d d1 %d d2 %d filter %d\n", textAccess->accessID, frag, d1, d2, filter);

        //  Set fragment cubemap to use.
        textAccess->cubemap = cubemap;
    }

/*if (id == 488)
{
printf("  anisoSamples = %d\n", textAccess->anisoSamples);
for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
printf("  LOD[%02d] = %f\n", f, textAccess->lod[f]);
}*/

    return textAccess;
}

/*  Calculates the addresses for the texture access.  */
void TextureEmulator::calculateAddress(TextureAccess *textAccess)
{
    QuadFloat *stampCoord;
    u32bit textUnit;
    u32bit d1, d2;
    u32bit i, j, k;
    f32bit a, b, c;
    u32bit frag;
    u32bit width;
    u32bit height;
    u32bit depth;
    u32bit currentAnisoSample;
    u32bit trilinearAccess;
    u32bit anisoSamples;
    f32bit dsOffset;
    f32bit dtOffset;
    f32bit fractionalLOD;
    bool twoMipsSampled;
    bool sampleFirstMip;
    bool sampleSecondMip;
    u32bit nextMIP;

    /*  Get texture access parameters.  */
    stampCoord = textAccess->coordinates;
    textUnit =  textAccess->textUnit;
    currentAnisoSample = textAccess->currentAnisoSample;
    anisoSamples = textAccess->anisoSamples;
    dsOffset = textAccess->anisodsOffset;
    dtOffset = textAccess->anisodtOffset;

    //  Get the next trilinear access for which to calculate the addresses
    trilinearAccess = currentAnisoSample - 1;

    //  Check if the trilinear object was already allocated or the pointer corrupted
    GPU_ASSERT(
        if (textAccess->trilinear[trilinearAccess] != NULL)
            panic("TextureEmulator", "calculateAddress", "Trilinear object should be NULL at this point.");
    )

    //  Allocate the trilinear object the first time it will be used
    textAccess->trilinear[trilinearAccess] = new TextureAccess::Trilinear;

    /*  Calculate the texel coordinates and addresses for all whole texture access.  */
    for(frag = 0; frag < STAMP_FRAGMENTS; frag++)
    {
        /*  Get selected mipmaps for the fragment.  */
        d1 = textAccess->level[frag][0];
        d2 = textAccess->level[frag][1];

//printf(">> id %d frag %d d1 %d d2 %d filter %d\n", textAccess->accessID, frag, d1, d2, textAccess->filter[frag]);

        /*  Reset fetch and read loop counters.  */
        textAccess->trilinear[trilinearAccess]->fetchLoop[frag] = 0;
        textAccess->trilinear[trilinearAccess]->readLoop[frag] = 0;
        textAccess->trilinear[trilinearAccess]->loops[frag] = 0;
        
        //  Reset if two mipmaps must be sampled.  Actual value will be computed below for LINEAR mipmap mode.
        textAccess->trilinear[trilinearAccess]->sampleFromTwoMips[frag] = false;

        /*  Reset texel fetch/read flags.  */
        for(i = 0; i < 16; i++)
        {
            textAccess->trilinear[trilinearAccess]->fetched[frag][i] = FALSE;
            textAccess->trilinear[trilinearAccess]->read[frag][i] = FALSE;
        }

        //  Select the type of texture operation for which to compute the texel addresses.
        switch(textAccess->texOperation)
        {
            case TEXTURE_READ :
            case TEXTURE_READ_WITH_LOD :
           
                //  Texture read operation.
                
                /*  Calculate the texel coordinates for each fragment.  */
                switch(textAccess->filter[frag])
                {
                    case GPU_NEAREST:
                    case GPU_NEAREST_MIPMAP_NEAREST:


                        /*  Get sample texel coordinate.  */
                        texelCoord(textUnit, d1, GPU_NEAREST, currentAnisoSample, anisoSamples, dsOffset, dtOffset,
                                   stampCoord[frag][0], stampCoord[frag][1], stampCoord[frag][2], i, j, k, a, b, c);

//printf("TU > N/N_M_N > d1 %d i %d j %d k %d\n", d1, i, j, k);

                        /*  Get texture size.  */
                        //width = 1 << mipmapSize(textureWidth2[textUnit], d1);
                        //height = 1 << mipmapSize(textureHeight2[textUnit], d1);
                        //depth = 1 << mipmapSize(textureDepth2[textUnit], d1);
                        width  = GPU_MAX(textureWidth[textUnit]  >> d1, u32bit(1));
                        height = GPU_MAX(textureHeight[textUnit] >> d1, u32bit(1));
                        depth  = GPU_MAX(textureDepth[textUnit]  >> d1, u32bit(1));

                        textAccess->trilinear[trilinearAccess]->i[frag][0] = applyWrap(textureWrapS[textUnit], i, width);
                        textAccess->trilinear[trilinearAccess]->j[frag][0] = applyWrap(textureWrapT[textUnit], j, height);
                        textAccess->trilinear[trilinearAccess]->k[frag][0] = applyWrap(textureWrapR[textUnit], k, depth);

                        textAccess->trilinear[trilinearAccess]->a[frag][0] = a;
                        textAccess->trilinear[trilinearAccess]->b[frag][0] = b;
                        textAccess->trilinear[trilinearAccess]->c[frag][0] = c;

                        /*  Convert texel coordinates to memory address.  */
                        textAccess->trilinear[trilinearAccess]->address[frag][0] = texel2address(textUnit, d1, textAccess->cubemap, i, j, k);

                        /*  Configure texture access loop mode.  */
                        textAccess->trilinear[trilinearAccess]->loops[frag] = 1;
                        textAccess->trilinear[trilinearAccess]->texelsLoop[frag] = 1;

                        break;

                    case GPU_LINEAR:
                    case GPU_LINEAR_MIPMAP_NEAREST:


                        /*  Get coordinates of first sample texel.  */
                        texelCoord(textUnit, d1, GPU_LINEAR, currentAnisoSample, anisoSamples, dsOffset, dtOffset,
                                   stampCoord[frag][0], stampCoord[frag][1], stampCoord[frag][2], i, j, k, a, b, c);

//printf("TU > L/L_M_N > d1 %d i %d j %d k %d\n", d1, i, j, k);

                        /*  Generate bilinear sample texel coordinates.  */
                        genBilinearTexels(textUnit, *textAccess, trilinearAccess, frag, d1, i, j, k, 0);

                        textAccess->trilinear[trilinearAccess]->a[frag][0] = a;
                        textAccess->trilinear[trilinearAccess]->b[frag][0] = b;
                        textAccess->trilinear[trilinearAccess]->c[frag][0] = c;

                        break;

                    case GPU_NEAREST_MIPMAP_LINEAR:

                        //  Calculate fractional part of the lod.
                        fractionalLOD = textAccess->lod[frag] - static_cast<f32bit>(GPU_FLOOR(textAccess->lod[frag]));
                        
                        // Check against the trilinear range to determine if one of two mips have to be sampled.
                        twoMipsSampled = (fractionalLOD > trilinearRangeMin) && (fractionalLOD < trilinearRangeMax);
                        
                        //  Compute if the first mip level must be sampled.
                        sampleFirstMip = twoMipsSampled || (fractionalLOD <= trilinearRangeMin);
                        
                        //  Compute if the second mip level must be sampled.
                        sampleSecondMip = twoMipsSampled || (fractionalLOD >= trilinearRangeMax);
                        
                        //  Reset pointer to the first mip level to sample.
                        nextMIP = 0;
                        
                        //  Check if the first mip must be sampled.
                        if (sampleFirstMip)
                        {
                            /*  Get sample texel coordinate at first mipmap.  */
                            texelCoord(textUnit, d1, GPU_NEAREST, currentAnisoSample, anisoSamples, dsOffset, dtOffset,
                                       stampCoord[frag][0], stampCoord[frag][1], stampCoord[frag][2], i, j, k, a, b, c);

//printf("TU > N_M_L > d1 %d i %d j %d k %d\n", d1, i, j, k);
                        
                            /*  Get texture size.  */
                            //width = 1 << mipmapSize(textureWidth2[textUnit], d1);
                            //height = 1 << mipmapSize(textureHeight2[textUnit], d1);
                            //depth = 1 << mipmapSize(textureDepth2[textUnit], d1);
                            width  = GPU_MAX(textureWidth[textUnit]  >> d1, u32bit(1));
                            height = GPU_MAX(textureHeight[textUnit] >> d1, u32bit(1));
                            depth  = GPU_MAX(textureDepth[textUnit]  >> d1, u32bit(1));

                            textAccess->trilinear[trilinearAccess]->i[frag][0] = applyWrap(textureWrapS[textUnit], i, width);
                            textAccess->trilinear[trilinearAccess]->j[frag][0] = applyWrap(textureWrapT[textUnit], j, height);
                            textAccess->trilinear[trilinearAccess]->k[frag][0] = applyWrap(textureWrapR[textUnit], k, depth);

                            textAccess->trilinear[trilinearAccess]->a[frag][0] = a;
                            textAccess->trilinear[trilinearAccess]->b[frag][0] = b;
                            textAccess->trilinear[trilinearAccess]->c[frag][0] = c;

                            /*  Convert texel coordinates to memory address.  */
                            textAccess->trilinear[trilinearAccess]->address[frag][0] = texel2address(textUnit, d1, textAccess->cubemap, i, j, k);
                        
                            //  Update pointer to the next mip level to sample.
                            nextMIP++;
                        }
                        
                        //  Check if the second mip must be sampled.
                        if (sampleSecondMip)
                        {
                            /*  Get sample texel coordinate at first mipmap.  */
                            texelCoord(textUnit, d2, GPU_NEAREST, currentAnisoSample, anisoSamples, dsOffset, dtOffset,
                                       stampCoord[frag][0], stampCoord[frag][1], stampCoord[frag][2], i, j, k, a, b, c);

//printf("TU > N_M_L > d2 %d i %d j %d k %d\n", d2, i, j, k);

                            /*  Get texture size.  */
                            //width = 1 << mipmapSize(textureWidth2[textUnit], d2);
                            //height = 1 << mipmapSize(textureHeight2[textUnit], d2);
                            //depth = 1 << mipmapSize(textureDepth2[textUnit], d2);
                            width  = GPU_MAX(textureWidth[textUnit]  >> d2, u32bit(1));
                            height = GPU_MAX(textureHeight[textUnit] >> d2, u32bit(1));
                            depth  = GPU_MAX(textureDepth[textUnit]  >> d2, u32bit(1));

                            textAccess->trilinear[trilinearAccess]->i[frag][nextMIP] = applyWrap(textureWrapS[textUnit], i, width);
                            textAccess->trilinear[trilinearAccess]->j[frag][nextMIP] = applyWrap(textureWrapT[textUnit], j, height);
                            textAccess->trilinear[trilinearAccess]->k[frag][nextMIP] = applyWrap(textureWrapR[textUnit], k, depth);

                            textAccess->trilinear[trilinearAccess]->a[frag][nextMIP] = a;
                            textAccess->trilinear[trilinearAccess]->b[frag][nextMIP] = b;
                            textAccess->trilinear[trilinearAccess]->c[frag][nextMIP] = c;

                            /*  Convert texel coordinates to memory address.  */
                            textAccess->trilinear[trilinearAccess]->address[frag][nextMIP] = texel2address(textUnit, d2, textAccess->cubemap, i, j, k);
                        }

                        //  Check if two mips were sampled
                        if (twoMipsSampled)
                        {
                            //  Configure texture access loop mode.
                            textAccess->trilinear[trilinearAccess]->loops[frag] = 2;
                            textAccess->trilinear[trilinearAccess]->texelsLoop[frag] = 1;
                        }
                        else
                        {
                            //  Configure texture access loop mode.
                            textAccess->trilinear[trilinearAccess]->loops[frag] = 1;
                            textAccess->trilinear[trilinearAccess]->texelsLoop[frag] = 1;
                        }
                        
                        //  Set if two mipmaps will be sampled.
                        textAccess->trilinear[trilinearAccess]->sampleFromTwoMips[frag] = twoMipsSampled;
                        
                        break;

                    case GPU_LINEAR_MIPMAP_LINEAR:

                        //  Calculate fractional part of the lod.
                        fractionalLOD = textAccess->lod[frag] - static_cast<f32bit>(GPU_FLOOR(textAccess->lod[frag]));
                        
                        // Check against the trilinear range to determine if one of two mips have to be sampled.
                        twoMipsSampled = (fractionalLOD > trilinearRangeMin) && (fractionalLOD < trilinearRangeMax);
                        
                        //  Compute if the first mip level must be sampled.
                        sampleFirstMip = twoMipsSampled || (fractionalLOD <= trilinearRangeMin);
                        
                        //  Compute if the second mip level must be sampled.
                        sampleSecondMip = twoMipsSampled || (fractionalLOD >= trilinearRangeMax);
                        
                        //  Reset pointer to the first mip to sample.
                        nextMIP = 0;
                        
                        //  Check if the first mip must be sampled.
                        if (sampleFirstMip)
                        {
                            /*  Get coordinates of first sample texel at first mipmap level.  */
                            texelCoord(textUnit, d1, GPU_LINEAR, currentAnisoSample, anisoSamples, dsOffset, dtOffset,
                                stampCoord[frag][0], stampCoord[frag][1], stampCoord[frag][2], i, j, k, a, b, c);

//printf("TU > L_M_L > d1 %d i %d j %d k %d\n", d1, i, j, k);

                            /*  Generate bilinear sample texel coordinates.  */
                            genBilinearTexels(textUnit, *textAccess, trilinearAccess, frag, d1, i, j, k, 0);

                            textAccess->trilinear[trilinearAccess]->a[frag][0] = a;
                            textAccess->trilinear[trilinearAccess]->b[frag][0] = b;
                            textAccess->trilinear[trilinearAccess]->c[frag][0] = c;
                        
                            //  Update pointer to the next mip to sample.
                            nextMIP++;
                        }
                        
                        //  Check if second mip must be sampled.
                        if (sampleSecondMip)
                        {
                            /*  Get coordinates of first sample texel at second mipmap level.  */
                            texelCoord(textUnit, d2, GPU_LINEAR, currentAnisoSample, anisoSamples, dsOffset, dtOffset,
                                stampCoord[frag][0], stampCoord[frag][1], stampCoord[frag][2], i, j, k, a, b, c);

//printf("TU > L_M_L > d2 %d i %d j %d k %d\n", d2, i, j, k);

                            /*  Generate bilinear sample texel coordinates.  */
                            genBilinearTexels(textUnit, *textAccess, trilinearAccess, frag, d2, i, j, k, nextMIP);

                            textAccess->trilinear[trilinearAccess]->a[frag][nextMIP] = a;
                            textAccess->trilinear[trilinearAccess]->b[frag][nextMIP] = b;
                            textAccess->trilinear[trilinearAccess]->c[frag][nextMIP] = c;
                        }
                        
                        //  Set if two mipmaps will be sampled.
                        textAccess->trilinear[trilinearAccess]->sampleFromTwoMips[frag] = twoMipsSampled;
                        
                        break;

                    default:
                        panic("TextureEmulator", "calculateAddress", "Unsupported filter mode.");
                }
                
                break;
                
            case ATTRIBUTE_READ:
            
                //  Attribute stream read.
                
                {                
                    //  Get the index from the first component of the parameters.  The index is an integer.
                    f32bit aux = stampCoord[frag][0];
                    u32bit index = *((u32bit *) &aux);
//printf("TxEmu::calculateAddress => ATTRIBUTE_READ -> Reading index = %d\n", index);                    
                    
                    //  Convert index to memory addresses.
                    index2address(textUnit, *textAccess, trilinearAccess, frag, index);
                }
                
                break;
            
            default:
                panic("TextureEmulator", "calculateAddress", "Unsupported texture operation.");
                break;
        }                
    }
}

/*  Expands the texel coordinates for a bilinear.  */
void TextureEmulator::genBilinearTexels(u32bit textUnit, TextureAccess &textAccess, u32bit trilinearAccess,
    u32bit frag, u32bit level, u32bit i, u32bit j, u32bit k, u32bit l)
{
    u32bit i0, j0, k0;
    u32bit i1, j1, k1;
    u32bit width;
    u32bit height;
    u32bit depth;

    /*  Generate the sample points.  */
    switch(textureMode[textUnit])
    {
        case GPU_TEXTURE1D:

            /*  Get size of the texture.  */
            //width = 1 << mipmapSize(textureWidth2[textUnit], level);
            width = GPU_MAX(textureWidth[textUnit] >> level, u32bit(1));
             
            /*  Calculate coordinate components for the bilinear sample points.  */
            i0 = applyWrap(textureWrapS[textUnit], i, width);
            i1 = applyWrap(textureWrapS[textUnit], i + 1, width);

            /*  Two sample points.  */
            textAccess.trilinear[trilinearAccess]->i[frag][2 * l + 0] = i0;

            textAccess.trilinear[trilinearAccess]->i[frag][2 * l + 1] = i1;

            /*  Texel address.  */
            textAccess.trilinear[trilinearAccess]->address[frag][2 * l + 0] = texel2address(textUnit, level, i0);
            textAccess.trilinear[trilinearAccess]->address[frag][2 * l + 1] = texel2address(textUnit, level, i1);

            /*  Configure number of loops for the texture access.  */
            textAccess.trilinear[trilinearAccess]->loops[frag] += 1;

            /*  Configure texture access reads per loop.  */
            textAccess.trilinear[trilinearAccess]->texelsLoop[frag] = 2;

            break;

        case GPU_TEXTURE2D:
        case GPU_TEXTURECUBEMAP:

            /*  Get size of the texture.  */
            //width = 1 << mipmapSize(textureWidth2[textUnit], level);
            //height = 1 << mipmapSize(textureHeight2[textUnit], level);
            width = GPU_MAX(textureWidth[textUnit] >> level, u32bit(1));
            height = GPU_MAX(textureHeight[textUnit] >> level, u32bit(1));

            /*  Calculate coordinate components for the bilinear sample points.  */
            i0 = applyWrap(textureWrapS[textUnit], i, width);
            i1 = applyWrap(textureWrapS[textUnit], i + 1, width);
            j0 = applyWrap(textureWrapT[textUnit], j, height);
            j1 = applyWrap(textureWrapT[textUnit], j + 1, height);

            /*  Three sample points.  */
            textAccess.trilinear[trilinearAccess]->i[frag][4 * l + 0] = i0;
            textAccess.trilinear[trilinearAccess]->j[frag][4 * l + 0] = j0;

            textAccess.trilinear[trilinearAccess]->i[frag][4 * l + 1] = i1;
            textAccess.trilinear[trilinearAccess]->j[frag][4 * l + 1] = j0;

            textAccess.trilinear[trilinearAccess]->i[frag][4 * l + 2] = i0;
            textAccess.trilinear[trilinearAccess]->j[frag][4 * l + 2] = j1;

            textAccess.trilinear[trilinearAccess]->i[frag][4 * l + 3] = i1;
            textAccess.trilinear[trilinearAccess]->j[frag][4 * l + 3] = j1;

            /*  Texel address.  */
            textAccess.trilinear[trilinearAccess]->address[frag][4 * l + 0] = texel2address(textUnit, level, textAccess.cubemap, i0, j0);
            textAccess.trilinear[trilinearAccess]->address[frag][4 * l + 1] = texel2address(textUnit, level, textAccess.cubemap, i1, j0);
            textAccess.trilinear[trilinearAccess]->address[frag][4 * l + 2] = texel2address(textUnit, level, textAccess.cubemap, i0, j1);
            textAccess.trilinear[trilinearAccess]->address[frag][4 * l + 3] = texel2address(textUnit, level, textAccess.cubemap, i1, j1);

            /*  Configure number of loops for the texture access.  */
            textAccess.trilinear[trilinearAccess]->loops[frag] += 1;

            /*  Configure texture access reads per loop.  */
            textAccess.trilinear[trilinearAccess]->texelsLoop[frag] = 4;

            break;

        case GPU_TEXTURE3D:

            /*  Get size of the texture.  */
            //width = 1 << mipmapSize(textureWidth2[textUnit], level);
            //height = 1 << mipmapSize(textureHeight2[textUnit], level);
            //depth = 1 << mipmapSize(textureDepth2[textUnit], level);
            width = GPU_MAX(textureWidth[textUnit] >> level, u32bit(1));
            height = GPU_MAX(textureHeight[textUnit] >> level, u32bit(1));
            depth = GPU_MAX(textureDepth[textUnit] >> level, u32bit(1));

            /*  Calculate coordinate components for the bilinear sample points.  */
            i0 = applyWrap(textureWrapS[textUnit], i, width);
            i1 = applyWrap(textureWrapS[textUnit], i + 1, width);
            j0 = applyWrap(textureWrapT[textUnit], j, height);
            j1 = applyWrap(textureWrapT[textUnit], j + 1, height);
            k0 = applyWrap(textureWrapR[textUnit], k, depth);
            k1 = applyWrap(textureWrapR[textUnit], k + 1, depth);

            /*  Eight sample points.  */
            textAccess.trilinear[trilinearAccess]->i[frag][8 * l + 0] = i0;
            textAccess.trilinear[trilinearAccess]->j[frag][8 * l + 0] = j0;
            textAccess.trilinear[trilinearAccess]->k[frag][8 * l + 0] = k0;

            textAccess.trilinear[trilinearAccess]->i[frag][8 * l + 1] = i1;
            textAccess.trilinear[trilinearAccess]->j[frag][8 * l + 1] = j0;
            textAccess.trilinear[trilinearAccess]->k[frag][8 * l + 1] = k0;

            textAccess.trilinear[trilinearAccess]->i[frag][8 * l + 2] = i0;
            textAccess.trilinear[trilinearAccess]->j[frag][8 * l + 2] = j1;
            textAccess.trilinear[trilinearAccess]->k[frag][8 * l + 2] = k0;

            textAccess.trilinear[trilinearAccess]->i[frag][8 * l + 3] = i1;
            textAccess.trilinear[trilinearAccess]->j[frag][8 * l + 3] = j1;
            textAccess.trilinear[trilinearAccess]->k[frag][8 * l + 3] = k0;

            textAccess.trilinear[trilinearAccess]->i[frag][8 * l + 4] = i0;
            textAccess.trilinear[trilinearAccess]->j[frag][8 * l + 4] = j0;
            textAccess.trilinear[trilinearAccess]->k[frag][8 * l + 4] = k1;

            textAccess.trilinear[trilinearAccess]->i[frag][8 * l + 5] = i1;
            textAccess.trilinear[trilinearAccess]->j[frag][8 * l + 5] = j0;
            textAccess.trilinear[trilinearAccess]->k[frag][8 * l + 5] = k1;

            textAccess.trilinear[trilinearAccess]->i[frag][8 * l + 6] = i0;
            textAccess.trilinear[trilinearAccess]->j[frag][8 * l + 6] = j1;
            textAccess.trilinear[trilinearAccess]->k[frag][8 * l + 6] = k1;

            textAccess.trilinear[trilinearAccess]->i[frag][8 * l + 7] = i1;
            textAccess.trilinear[trilinearAccess]->j[frag][8 * l + 7] = j1;
            textAccess.trilinear[trilinearAccess]->k[frag][8 * l + 7] = k1;

            /*  Texel address.  */
            textAccess.trilinear[trilinearAccess]->address[frag][8 * l + 0] = texel2address(textUnit, level, 0, i0, j0, k0);
            textAccess.trilinear[trilinearAccess]->address[frag][8 * l + 1] = texel2address(textUnit, level, 0, i1, j0, k0);
            textAccess.trilinear[trilinearAccess]->address[frag][8 * l + 2] = texel2address(textUnit, level, 0, i0, j1, k0);
            textAccess.trilinear[trilinearAccess]->address[frag][8 * l + 3] = texel2address(textUnit, level, 0, i1, j1, k0);
            textAccess.trilinear[trilinearAccess]->address[frag][8 * l + 4] = texel2address(textUnit, level, 0, i0, j0, k1);
            textAccess.trilinear[trilinearAccess]->address[frag][8 * l + 5] = texel2address(textUnit, level, 0, i1, j0, k1);
            textAccess.trilinear[trilinearAccess]->address[frag][8 * l + 6] = texel2address(textUnit, level, 0, i0, j1, k1);
            textAccess.trilinear[trilinearAccess]->address[frag][8 * l + 7] = texel2address(textUnit, level, 0, i1, j1, k1);

            /*  Configure number of loops for the texture access.  */
            textAccess.trilinear[trilinearAccess]->loops[frag] += 2;

            /*  Configure texture access reads per loop.  */
            textAccess.trilinear[trilinearAccess]->texelsLoop[frag] = 4;

            break;
    }
}

/*  Selects the cubemap face accessed by the whole stamp and adjust the stamp texture coordinates to the
    selected image.  */
CubeMapFace TextureEmulator::selectCubeMapFace(QuadFloat *coord)
{
    //CubeMapFace face[CUBEMAP_IMAGES];
    u32bit face[CUBEMAP_IMAGES];
    //f32bit sc, tc;
    //f32bit ma;
    u32bit i;
    u32bit selectedFace;
    f32bit rx[STAMP_FRAGMENTS];
    f32bit ry[STAMP_FRAGMENTS];
    f32bit rz[STAMP_FRAGMENTS];
    //u32bit f;


    /*  As an approximation we use the first fragment in the stamp to calculate
    the cube map image to use.  */

    /*  Calculate the absolute value for the first fragment coordinates three axis.  */

    /*  Reset face selection counters.  */
    for(i = 0; i < CUBEMAP_IMAGES; i++)
       face[i] = 0;

    for(i = 0; i < STAMP_FRAGMENTS; i++ )
    {
        rx[i] = GPUMath::ABS(coord[i][0]);
        ry[i] = GPUMath::ABS(coord[i][1]);
        rz[i] = GPUMath::ABS(coord[i][2]);
    }

    /*  Analyze the quad coordinates.  */
    for(i = 0; i < STAMP_FRAGMENTS; i++)
    {
        /*  Select the axis with the largest value.  */
        if (rx[i] >= ry[i])
        {
            /*  Select the axis with the largest value.  */
            if (rx[i] > rz[i])
            {
                /*  X axis is the largest.  */

                /*  Selected positive or negative face.  */
                //face[i] = (coord[i][0] < 0)?GPU_CUBEMAP_NEGATIVE_X:GPU_CUBEMAP_POSITIVE_X;
                face[(coord[i][0] < 0)?GPU_CUBEMAP_NEGATIVE_X:GPU_CUBEMAP_POSITIVE_X]++;
            }
            else
            {
                /*  Z axis is the largest.  */

                /*  Selected positive or negative face.  */
                //face[i] = (coord[i][2] < 0)?GPU_CUBEMAP_NEGATIVE_Z:GPU_CUBEMAP_POSITIVE_Z;
                face[(coord[i][2] < 0)?GPU_CUBEMAP_NEGATIVE_Z:GPU_CUBEMAP_POSITIVE_Z]++;
            }
        }
        else
        {
            /*  Select the axis with the largest value.  */
            if (ry[i] >= rz[i])
            {
                /*  Y axis is the largest.  */

                /*  Selected */
                //face[i] = (coord[i][1] < 0)?GPU_CUBEMAP_NEGATIVE_Y:GPU_CUBEMAP_POSITIVE_Y;
                face[(coord[i][1] < 0)?GPU_CUBEMAP_NEGATIVE_Y:GPU_CUBEMAP_POSITIVE_Y]++;
            }
            else
            {
                /*  Z axis is the largest.  */

                /*  Selected positive or negative face.  */
                //face[i] = (coord[i][2] < 0)?GPU_CUBEMAP_NEGATIVE_Z:GPU_CUBEMAP_POSITIVE_Z;
                face[(coord[i][2] < 0)?GPU_CUBEMAP_NEGATIVE_Z:GPU_CUBEMAP_POSITIVE_Z]++;
            }
        }
    }

    /*  Search for the face selected for most fragments in the quad.  */
    for(i = 1, selectedFace = 0; i < CUBEMAP_IMAGES; i++)
    {
        /*  Choose the face with more hits.  */
        if (face[i] > face[selectedFace])
            selectedFace = i;
    }

    /*if ((face[0] == face[1]) && (face[0] == face[2]))
    {
        f = 0;
    }
    else if ((face[1] == face[0]) && (face[1] == face[3]))
    {
        f = 1;
    }
    else if ((face[2] == face[0]) && (face[0] == face[3]))
    {
        f = 2;
    }
    else if ((face[3] == face[2]) && (face[3] == face[1]))
    {
        f = 3;
    }
    else if ((face[0] == face[2]))
    {
        f = 0;
    }
    else if ((face[0] == face[1]))
    {
        f = 1;
    }
    else if ((face[2] == face[3]))
    {
        f = 2;
    }
    else if ((face[3] == face[1]))
    {
        f = 3;
    }
    else
    {
        f = 0;
    }

    selectedFace = face[f];*/


    /*  Recalculate the stamp texture coordinates as 2D texture coordinates into the
        selected face.  */
    switch(selectedFace)
    {
        case GPU_CUBEMAP_POSITIVE_X:

            for(i = 0; i < STAMP_FRAGMENTS; i++)
            {
                coord[i][0] = ((-coord[i][2] / rx[i]) + 1.0f) * 0.5f;
                coord[i][1] = ((-coord[i][1] / rx[i]) + 1.0f) * 0.5f;
            }

            return GPU_CUBEMAP_POSITIVE_X;

        case GPU_CUBEMAP_NEGATIVE_X:

            for(i = 0; i < STAMP_FRAGMENTS; i++)
            {
                coord[i][0] = ((coord[i][2] / rx[i]) + 1.0f) * 0.5f;
                coord[i][1] = ((-coord[i][1] / rx[i]) + 1.0f) * 0.5f;
            }

            return GPU_CUBEMAP_NEGATIVE_X;

        case GPU_CUBEMAP_POSITIVE_Y:

            for(i = 0; i < STAMP_FRAGMENTS; i++)
            {
                coord[i][0] = ((coord[i][0] / ry[i]) + 1.0f) * 0.5f;
                coord[i][1] = ((coord[i][2] / ry[i]) + 1.0f) * 0.5f;
            }

            return GPU_CUBEMAP_POSITIVE_Y;

        case GPU_CUBEMAP_NEGATIVE_Y:

            for(i = 0; i < STAMP_FRAGMENTS; i++)
            {
                coord[i][0] = ((coord[i][0] / ry[i]) + 1.0f) * 0.5f;
                coord[i][1] = ((-coord[i][2] / ry[i]) + 1.0f) * 0.5f;
            }

            return GPU_CUBEMAP_NEGATIVE_Y;

        case GPU_CUBEMAP_POSITIVE_Z:

            for(i = 0; i < STAMP_FRAGMENTS; i++)
            {
                coord[i][0] = ((coord[i][0] / rz[i]) + 1.0f) * 0.5f;
                coord[i][1] = ((-coord[i][1] / rz[i]) + 1.0f) * 0.5f;
            }

            return GPU_CUBEMAP_POSITIVE_Z;

        case GPU_CUBEMAP_NEGATIVE_Z:

            for(i = 0; i < STAMP_FRAGMENTS; i++)
            {
                coord[i][0] = ((-coord[i][0] / rz[i]) + 1.0f) * 0.5f;
                coord[i][1] = ((-coord[i][1] / rz[i]) + 1.0f) * 0.5f;
            }

            return GPU_CUBEMAP_NEGATIVE_Z;

   }

    panic("TextureEmulator", "selectCubeMapFace", "Undefined cubemap face.");
    return GPU_CUBEMAP_NEGATIVE_Z; // Dummy

   //return selectedFace;
}


/*  Calculates the derivatives in x and y for fragment stamp.  */
void  TextureEmulator::derivativesXY(QuadFloat *coordinates, u32bit textUnit,
    f32bit &dudx, f32bit &dudy, f32bit &dvdx, f32bit &dvdy, f32bit &dwdx, f32bit &dwdy)
{
    f32bit u0, v0, w0;
    f32bit u1, v1, w1;

    /*

       Layout of a fragment quad (or stamp):

                c0 c1
                c2 c3

       WARNING:  IT DOESN'T WORK IF THE STAMP ISN'T A QUAD.

    */


    //  Check if texture non-normalized coordinates are used.
    if (!textureNonNormalized[textUnit])
    {
        //u0 = coordinates[0][0] * f32bit(1 << textureWidth2[textUnit]);
        //v0 = coordinates[0][1] * f32bit(1 << textureHeight2[textUnit]);
        //w0 = coordinates[0][2] * f32bit(1 << textureDepth2[textUnit]);
        u0 = coordinates[0][0] * f32bit(textureWidth[textUnit]);
        v0 = coordinates[0][1] * f32bit(textureHeight[textUnit]);
        w0 = coordinates[0][2] * f32bit(textureDepth[textUnit]);

        //u1 = coordinates[1][0] * f32bit(1 << textureWidth2[textUnit]);
        //v1 = coordinates[1][1] * f32bit(1 << textureHeight2[textUnit]);
        //w1 = coordinates[1][2] * f32bit(1 << textureDepth2[textUnit]);
        u1 = coordinates[1][0] * f32bit(textureWidth[textUnit]);
        v1 = coordinates[1][1] * f32bit(textureHeight[textUnit]);
        w1 = coordinates[1][2] * f32bit(textureDepth[textUnit]);
    }
    
    /*  Calculate derivatives.  */
    dudx = u1 - u0;
    dvdx = v1 - v0;
    dwdx = w1 - w0;

    //  Check if texture non-normalized coordinates are used.
    if (!textureNonNormalized[textUnit])
    {
        //u1 = coordinates[2][0] * f32bit(1 << textureWidth2[textUnit]);
        //v1 = coordinates[2][1] * f32bit(1 << textureHeight2[textUnit]);
        //w1 = coordinates[2][2] * f32bit(1 << textureDepth2[textUnit]);
        u1 = coordinates[2][0] * f32bit(textureWidth[textUnit]);
        v1 = coordinates[2][1] * f32bit(textureHeight[textUnit]);
        w1 = coordinates[2][2] * f32bit(textureDepth[textUnit]);
    }
    
    dudy = u1 - u0;
    dvdy = v1 - v0;
    dwdy = w1 - w0;
}

/*  Calculates level of detail for GPU_TEXTURE1D textures.  */
f32bit TextureEmulator::calculateScale1D(f32bit dudx, f32bit dudy)
{
    f32bit scale;

    /*  Calculate the scale factor.  */
    scale = GPU_MAX(dudx, dudy);

    return scale;
}

/*  Calculates level of detail for GPU_TEXTURE2D textures.  */
f32bit TextureEmulator::calculateScale2D(u32bit textUnit, f32bit dudx, f32bit dudy, f32bit dvdx, f32bit dvdy, u32bit maxAniso,
    u32bit &samples, f32bit &dsOffset, f32bit &dtOffset)
{
    f32bit scale;

    /*  Check maximum anisotropy.  */
    if (maxAniso == 1)
    {
        /*  Just calculate the scale factor as usual.  Ignore all other parameters.  */

        /*  Only take one sample.  */
        samples = 1;
        dsOffset = 0;
        dtOffset = 0;
        
        /*  Calculate the scale factor.  */
        scale = static_cast<f32bit>(GPU_MAX(GPU_SQRT(dudx * dudx + dvdx * dvdx),
            GPU_SQRT(dudy * dudy + dvdy * dvdy)));
    }
    else if (maxAniso > 1)
    {
        //  Call the selected anisotropy algorithm
        switch(anisoAlgorithm)
        {
            case ANISO_TWO_AXIS:
                scale = anisoTwoAxis(dudx, dudy, dvdx, dvdy, maxAniso, samples, dsOffset, dtOffset);
                break;

            case ANISO_FOUR_AXIS:
                scale = anisoFourAxis(dudx, dudy, dvdx, dvdy, maxAniso, samples, dsOffset, dtOffset);
                break;

            case ANISO_RECTANGULAR:
                scale = anisoRectangle(dudx, dudy, dvdx, dvdy, maxAniso, samples, dsOffset, dtOffset);
                break;

            case ANISO_EWA:
                scale = anisoEWA(dudx, dudy, dvdx, dvdy, maxAniso, samples, dsOffset, dtOffset);
                break;

            case ANISO_EXPERIMENTAL:
                scale = anisoExperimentalAngle(dudx, dudy, dvdx, dvdy, maxAniso, samples, dsOffset, dtOffset);
                break;

            default:
                panic("TextureEmulator", "calculateScale2D", "Undefined anisotropy algorithm");
                break;
        }

        /*  Normalize to s,t space : [0..1].  */
        //dsOffset = dsOffset / f32bit(1 << textureWidth2[textUnit]);
        //dtOffset = dtOffset / f32bit(1 << textureHeight2[textUnit]);
        dsOffset = dsOffset / f32bit(textureWidth[textUnit]);
        dtOffset = dtOffset / f32bit(textureHeight[textUnit]);

//printf("Scale2D >> scale %f samples %d dsOffset %f dtOffset %f\n", scale, samples, dsOffset, dtOffset);
    }
    else
    {
        panic("TextureEmulator", "calculateScale2D", "Maximum anisotropy must be at least 1.");
    }

    return scale;
}

//  Computes the number of anisotropic samples based on the anisotropic ratio.
u32bit TextureEmulator::computeAnisoSamples(f32bit anisoRatio)
{
    u32bit samples;
    
    //  Check if the number of aniso samples must be a multiple of two.
    if (anisoRatioMultOfTwo)
    {
        //  Compute the difference to the nearest higher integer multiple of two.
        f32bit diffToMult2 = 2 - (f32bit(((u32bit(GPU_FLOOR(anisoRatio)) & 0xfffffffe) + 2)) - anisoRatio);

        //  Based on the threshold round the nearest lower multiple of two number of samples
        //  or the nearest higher multiple of two number of samples.
        if (diffToMult2 > anisoRoundUp)
            samples = (u32bit(GPU_FLOOR(anisoRatio)) & 0xfffffffe) + 2;
        else
            samples = (u32bit(GPU_FLOOR(anisoRatio)) & 0xfffffffe);
    }
    else
    {
        //  Compute the fractional part of the ratio.
        f32bit fracRatio = anisoRatio - f32bit(GPU_FLOOR(anisoRatio));       
        
        //  Based on the threshold round to the nearest lower integer number of samples
        //  or the nearest higher integer number of samples.
        if (fracRatio > anisoRoundUp)
            samples = u32bit(GPU_CEIL(anisoRatio));
        else
            samples = u32bit(GPU_FLOOR(anisoRatio));        
    }

    return GPU_MAX(samples, u32bit(1));
}

//  Computes the anisotropy based on the screen aligned axis
f32bit TextureEmulator::anisoTwoAxis(f32bit dudx, f32bit dudy, f32bit dvdx, f32bit dvdy,
    u32bit maxAniso, u32bit &samples, f32bit &dsOffset, f32bit &dtOffset)
{
    f32bit scale;
    f32bit px;
    f32bit py;
    f32bit pMin;
    f32bit pMax;
    f32bit N;
    TextureAccess::AnisotropyAxis axis;

    /*  Calculate scales for the two window axes.  Select maximum and calculate samples, axis
        and offsets.  */

    /*  Calculate texture scale in the horizontal and vertical screen axis.  */
    px = (f32bit) GPU_SQRT(dudx * dudx + dvdx * dvdx);
    py = (f32bit) GPU_SQRT(dudy * dudy + dvdy * dvdy);

    /*  Calculate the minimum and maximum for both axis.  */
    pMin = GPU_MIN(px, py);
    pMax = GPU_MAX(px, py);

    /*  Calculate ratio for X/Y axis.  */
    N = GPU_MIN(pMax/pMin, f32bit(maxAniso));

    //  Select the largest axis as the anisotropy axis
    if (pMax == px)
        axis = TextureAccess::X_AXIS;
    else
        axis = TextureAccess::Y_AXIS;

    /*  Calculate the number of samples required.  */
    //samples = u32bit(GPU_CEIL(N));
    samples = computeAnisoSamples(N);

    /*  Calculate the texture scale for each sample.  */
    //scale = pMax / f32bit(samples);
    scale = pMax / N;

    /*  Calculate the per anisotropic sample offsets in s,t space.  */
    switch(axis)
    {
        case TextureAccess::X_AXIS:
            dsOffset = dudx / f32bit(samples + 1);
            dtOffset = dvdx / f32bit(samples + 1);
            break;
        case TextureAccess::Y_AXIS:
            dsOffset = dudy / f32bit(samples + 1);
            dtOffset = dvdy / f32bit(samples + 1);
            break;
    }

    return scale;
}

//  Computes the anisotropy based on the screen aligned axis and the screen aligned axis rotated 45 degrees
f32bit TextureEmulator::anisoFourAxis(f32bit dudx, f32bit dudy, f32bit dvdx, f32bit dvdy,
    u32bit maxAniso, u32bit &samples, f32bit &dsOffset, f32bit &dtOffset)
{
    f32bit scale;
    f32bit px;
    f32bit py;
    f32bit pxy;
    f32bit pyx;
    f32bit pMin;
    f32bit pMax;
    f32bit N;
    f32bit pMin2;
    f32bit pMax2;
    f32bit N2;
    TextureAccess::AnisotropyAxis axis;

    /*  Calculate scales for the two window axes.  Select maximum and calculate samples, axis
        and offsets.  */

    /*  Calculate texture scale in the horizontal and vertical screen axis.  */
    px = (f32bit) GPU_SQRT(dudx * dudx + dvdx * dvdx);
    py = (f32bit) GPU_SQRT(dudy * dudy + dvdy * dvdy);

    /*  Calculate texture scale on the XY/YX axis (axis rotated 45 degrees).  */
    pxy = (f32bit) GPU_SQRT((dudx + dudy) * (dudx + dudy) * 0.5 + (dvdx + dvdy) * (dvdx + dvdy) * 0.5);
    pyx = (f32bit) GPU_SQRT((dudx - dudy) * (dudx - dudy) * 0.5 + (dvdx - dvdy) * (dvdx - dvdy) * 0.5);

//printf("4Axis >> (dudx, dvdx) = (%f, %f) | (dudy, dvdy) = (%f, %f)\n", dudx, dvdx, dudy, dvdy);
//printf("4Axis >> (dX + dY) = (%f, %f) | (dX - dY) = (%f, %f)\n",
//dudx + dudy, dvdx + dvdy, dudx - dudy, dvdx - dvdy);
//printf("4Axis >> px = %f | py = %f | pxy = %f | pyx = %f\n", px, py, pxy, pyx);

    /*  Calculate the minimum and maximum for both axis.  */
    pMin = GPU_MIN(px, py);
    pMax = GPU_MAX(px, py);

    /*  Calculate ratio for X/Y axis.  */
    N = GPU_MIN(pMax/pMin, f32bit(maxAniso));

    /*  Calculate the minimum and maximum for both axis.  */
    pMin2 = GPU_MIN(pxy, pyx);
    pMax2 = GPU_MAX(pxy, pyx);

    /*  Calculate ratio for XY/YX axis.  */
    N2 = GPU_MIN(pMax2/pMin2, f32bit(maxAniso));

//printf("4Axis >>> areaA = %f | areaB = %f | NA = %f | NB = %f\n",
//pMax * pMin, pMax2 * pMin2, N, N2);

    /*  Determine which */
    if (N >= N2)
    {
        if (pMax == px)
            axis = TextureAccess::X_AXIS;
        else
            axis = TextureAccess::Y_AXIS;

        /*  Calculate the number of samples required.  */
        //samples = u32bit(GPU_CEIL(N));
        samples = computeAnisoSamples(N);

        /*  Calculate the texture scale for each sample.  */
        //scale = pMax / f32bit(samples);
        scale = pMax / N;
    }
    else
    {
        /*  Determine the anisotropy axis.  */
        if (pMax2 == pxy)
            axis = TextureAccess::XY_AXIS;
        else
            axis = TextureAccess::YX_AXIS;

        /*  Calculate the number of samples required.  */
        //samples = u32bit(GPU_CEIL(N2));
        samples = computeAnisoSamples(N2);

        /*  Calculate the texture scale for each sample.  */
        //scale = pMax2 / f32bit(samples);
        scale = pMax2 / N2;
    }

//printf("4Axis >> samples = %d | scale = %f | axis = ", samples, scale);

    /*  Calculate the per anisotropic sample offsets in s,t space.  */
    switch(axis)
    {
        case TextureAccess::X_AXIS:
//printf("X ");
            dsOffset = dudx / f32bit(samples + 1);
            dtOffset = dvdx / f32bit(samples + 1);
            break;
        case TextureAccess::Y_AXIS:
//printf("Y ");
            dsOffset = dudy / f32bit(samples + 1);
            dtOffset = dvdy / f32bit(samples + 1);
            break;
        case TextureAccess::XY_AXIS:
//printf("XY ");
            dsOffset = ((dudx + dudy) / (f32bit) GPU_SQRT(2)) / f32bit(samples + 1);
            dtOffset = ((dvdx + dvdy) / (f32bit) GPU_SQRT(2)) / f32bit(samples + 1);
            break;
        case TextureAccess::YX_AXIS:
//printf("YX ");
            dsOffset = ((dudx - dudy) / (f32bit) GPU_SQRT(2)) / f32bit(samples + 1);
            dtOffset = ((dvdx - dvdy) / (f32bit) GPU_SQRT(2)) / f32bit(samples + 1);
            break;
    }
//printf("\n");

    return scale;
}

//  Computes anisotropy based on a rectangular approximation of the projected area of the pixel
f32bit TextureEmulator::anisoRectangle(f32bit dudx, f32bit dudy, f32bit dvdx, f32bit dvdy,
    u32bit maxAniso, u32bit &samples, f32bit &dsOffset, f32bit &dtOffset)
{
    f32bit scale;
    f32bit pMinA;
    f32bit pMaxA;
    f32bit NA;
    f32bit ratioA;
    f32bit pMinB;
    f32bit pMaxB;
    f32bit NB;
    f32bit ratioB;
    f32bit axis1A[2];
    f32bit axis2A[2];
    f32bit axis1B[2];
    f32bit axis2B[2];
    f32bit diag1[2];
    f32bit diag2[2];
    f32bit l1A;
    f32bit l2A;
    f32bit l1B;
    f32bit l2B;
    f32bit lA;
    f32bit lB;
    f32bit N;

    //  Calculate lenghts of the x and y vectors in texture space
    l1A = (f32bit) GPU_SQRT(dudx * dudx + dvdx * dvdx);
    l2A = (f32bit) GPU_SQRT(dudy * dudy + dvdy * dvdy);

    //  Calculate diagonals of the x and y vectors in texture space (45 degree rotation)
    diag1[0] = (dudx + dudy) / (f32bit) GPU_SQRT(2);
    diag1[1] = (dvdx + dvdy) / (f32bit) GPU_SQRT(2);
    diag2[0] = (dudx - dudy) / (f32bit) GPU_SQRT(2);
    diag2[1] = (dvdx - dvdy) / (f32bit) GPU_SQRT(2);

    //  Calculate lengths of the 45 degree rotated x and y vectors in texture space (diagonals)
    l1B = (f32bit) GPU_SQRT(diag1[0] * diag1[0] + diag1[1] * diag1[1]);
    l2B = (f32bit) GPU_SQRT(diag2[0] * diag2[0] + diag2[1] * diag2[1]);

    /*  Select largest vector as major anisotropy axis.  */
    if (l1A >= l2A)
    {
        /*  Calculate major anisotropy axis as the normalized major vector. */
        axis1A[0] = dudx / l1A;
        axis1A[1] = dvdx / l1A;

        /*  Create minor anisotropy axis based on major anisotropy axis.  */
        axis2A[0] = -axis1A[1];
        axis2A[1] = axis1A[0];

        /*  Calculate the ratio between the u and v.  */
        ratioA = l1A/l2A;

        //  Set major axis lenght
        lA = l1A;

        /*  Calculate major axis scale as the addition of the major vector length
            and the projection of the minor vector over the major axis.  */
        pMaxA = l1A + GPU_ABS(dudy * axis1A[0] + dvdy * axis1A[1]);

        /*  Get minor axis scale as the projection of minor vector on the minor anisotropy axis.  */
        pMinA = GPU_ABS(dudy * axis2A[0] + dvdy * axis2A[1]);
    }
    else
    {
        /*  Calculate major anisotropy axis as the normalized major vector. */
        axis1A[0] = dudy / l2A;
        axis1A[1] = dvdy / l2A;

        /*  Create minor anisotropy axis based on major anisotropy axis.  */
        axis2A[0] = -axis1A[1];
        axis2A[1] = axis1A[0];

        /*  Calculate the ratio between the u and v.  */
        ratioA = l2A/l1A;

        //  Set major axis lenght
        lA = l2A;

        /*  Calculate major axis scale as the addition of the major vector length
            and the projection of the minor vector over the major axis.  */
        pMaxA = l2A + GPU_ABS(dudx * axis1A[0] + dvdx * axis1A[1]);;

        /*  Get minor axis scale as the projection of minor vector on the minor anisotropy axis.  */
        pMinA = GPU_ABS(dudx * axis2A[0] + dvdx * axis2A[1]);
    }

    /*  Choose the larger diagonal as the main anisotropy axis.  */
    if (l1B >= l2B)
    {
        /*  Normalize major diagonal to get major anisotropy axis.  */
        axis1B[0] = diag1[0]/l1B;
        axis1B[1] = diag1[1]/l1B;

        /*  Calculate the ratio between major and minor diagonal.  */
        ratioB = l1B/l2B;

        /*  Create perpendicular vector as minor anisotropy axis.  */
        axis2B[0] = -axis1B[1];
        axis2B[1] = axis1B[0];

        //  Set major axis length
        lB = l1B;

        /*  Calculate major axis scale as the addition of the major vector length
            and the projection of the minor vector over the major axis.  */
        pMaxB = l1B + GPU_ABS(diag2[0] * axis1B[0] + diag2[1] * axis1B[1]);

        /*  Calculate minor axis scale as two times the projection of minor diagonal on the minor anisotropy axis.  */
        pMinB = GPU_ABS(diag2[0] * axis2B[0] + diag2[1] * axis2B[1]);
    }
    else
    {
        /*  Normalize major diagonal to get major anisotropy axis.  */
        axis1B[0] = diag2[0]/l2B;
        axis1B[1] = diag2[1]/l2B;

        /*  Create perpendicular vector as minor anisotropy axis.  */
        axis2B[0] = -axis1B[1];
        axis2B[1] = axis1B[0];

        /*  Calculate the ratio between major and minor diagonal.  */
        ratioB = l2B/l1B;

        //  Set major axis lenght
        lB = l2B;

        /*  Calculate major axis scale as the addition of the major vector length
            and the projection of the minor vector over the major axis.  */
        pMaxB = l2B + GPU_ABS(diag1[0] * axis1B[0] + diag1[1] * axis1B[1]);

        /*  Calculate minor axis scale as two times the projection of minor diagonal on the minor anisotropy axis.  */
        pMinB = GPU_ABS(diag1[0] * axis2B[0] + diag1[1] * axis2B[1]);
    }

    /*  Calculate ratio between the two anisotropy axis.  */
    NA = GPU_MIN(pMaxA/pMinA, f32bit(maxAniso));

    /*  Calculate ratio between the two anisotropy axis.  */
    NB = GPU_MIN(pMaxB/pMinB, f32bit(maxAniso));

//printf(" anisoV2 >>> area A = %f | area B = %f | NA = %f | NB = %f | ratioA = %f | ratioB = %f\n",
//pMaxA * pMinA, pMaxB * pMinB, NA, NB, ratioA, ratioB);

    N = GPU_MAX(NA, NB);

    if (ratioA >= ratioB)
    {
       /*  Calculate the number of samples required.  */
        //samples = u32bit(GPU_CEIL(N));
        samples = computeAnisoSamples(N);

        /*  Calculate the texture scale for each sample.  */
        scale = lA / N;

        /*  Calculate the per anisotropic sample offsets in s,t space.  */
        dsOffset = (axis1A[0] * lA) / f32bit(samples + 1);
        dtOffset = (axis1A[1] * lA) / f32bit(samples + 1);
    }
    else
    {
        /*  Calculate the number of samples required.  */
        //samples = u32bit(GPU_CEIL(N));
        samples = computeAnisoSamples(N);

        /*  Calculate the texture scale for each sample.  */
        scale = lB / N;

        /*  Calculate the per anisotropic sample offsets in s,t space.  */
        dsOffset = (axis1B[0] * lB) / f32bit(samples + 1);
        dtOffset = (axis1B[1] * lB) / f32bit(samples + 1);
    }

//printf("V2 >> samples = %d | scale = %f\n", samples, scale);

    //  Check non finite results of the anisotropy algorithm
    if (!(finite(scale) && finite(dsOffset) && finite(dtOffset)))
    {
        samples = 1;

        scale = static_cast<f32bit>(GPU_MAX(GPU_SQRT(dudx * dudx + dvdx * dvdx),
            GPU_SQRT(dudy * dudy + dvdy * dvdy)));

        dsOffset = 0.0f;
        dtOffset = 0.0f;
    }

    return scale;
}

//  Computes anisotropy based on a Heckbert's EWA algorithm
f32bit TextureEmulator::anisoEWA(f32bit dudx, f32bit dudy, f32bit dvdx, f32bit dvdy,
    u32bit maxAniso, u32bit &samples, f32bit &dsOffset, f32bit &dtOffset)
{
    f32bit A;
    f32bit B;
    f32bit C;
    f32bit F;
    f32bit p;
    f32bit t;
    f32bit q;
    f32bit axis1[2];
    f32bit axis2[2];
    f32bit l1;
    f32bit l2;
    f32bit scale;
    f32bit N;

    //  Calculate ellipse equation coefficients from the derivatives
    A = dvdx * dvdx + dvdy * dvdy;
    B = -2.0f * (dudx * dvdx + dudy * dvdy);
    C = dudx * dudx + dudy * dudy;
    F = (dudx * dvdy - dudy * dvdx) * (dudx * dvdy - dudy * dvdx);

    //  What is the purpose of this?
    A = A/F;
    B = B/F;
    C = C/F;
//printf("EWA>> A %f B %f C %f F %f\n", A, B, C, F);
/*
    f32bit cA;
    f32bit cC;
    f32bit major;
    f32bit minor;

    cA = (A + C + GPU_SQRT((A - C) * (A - C) + B * B))/2;
    cC = (A + C - GPU_SQRT((A - C) * (A - C) + B * B))/2;


    if (cA <= cC)
    {
        major = GPU_SQRT(F / cA);
        minor = GPU_SQRT(F / cC);
    }
    else
    {
        major = GPU_SQRT(F / cC);
        minor = GPU_SQRT(F / cA);
    }
printf("EWA >> major %f minor %f\n", major, minor);
*/

#ifndef GPU_SIGN
    #define GPU_SIGN(x) (((x) >= 0)?1:-1)
#endif

    //  Calculate additional factors required to compute the minor and major axis
    //  of the ellipse
    p = A - C;
    q = A + C;
    t = GPU_SIGN(p) * (f32bit) GPU_SQRT(p * p + B * B);

//printf("EWA >> p %f q %f t %f\n", p, q, t);

    //  Check if t is 0
    if (t == 0.0f)
    {
        axis1[0] = 1.0f / (f32bit) GPU_SQRT(A);
        axis1[1] = 0.0f;
        axis2[0] = 0.0f;
        axis2[1] = 1.0f / (f32bit) GPU_SQRT(A);
    }
    else
    {
        //  Calculate the major and minor axis of the ellipse
        axis1[0] = (f32bit) GPU_SQRT((t + p) / (t * (q + t)));
        axis1[1] = GPU_SIGN(B * p) * (f32bit) GPU_SQRT((t - p) / (t * (q + t)));

        axis2[0] = -1.0f * GPU_SIGN(B * p) * (f32bit) GPU_SQRT((t - p) / (t * (q - t)));
        axis2[1] = (f32bit) GPU_SQRT((t + p) / (t * (q - t)));
    }

    /*GPU_ASSERT(
        if (isnan(axis1[0]) || isnan(axis1[1]) || isnan(axis2[0]) || isnan(axis2[1]))
        {
            printf("EWA>> {dudx, dvdx} = {%f, %f} | {dudy, dvdy} = {%f, %f}\n",
                dudx, dvdx, dudy, dvdy);

            printf("EWA>> A %f B %f C %f F %f\n", A, B, C, F);

            printf("EWA >> p %f q %f t %f\n", p, q, t);

            f32bit cA;
            f32bit cC;
            f32bit major;
            f32bit minor;

            cA = (A + C + GPU_SQRT((A - C) * (A - C) + B * B))/2;
            cC = (A + C - GPU_SQRT((A - C) * (A - C) + B * B))/2;


            if (cA <= cC)
            {
                major = GPU_SQRT(F / cA);
                minor = GPU_SQRT(F / cC);
            }
            else
            {
                major = GPU_SQRT(F / cC);
                minor = GPU_SQRT(F / cA);
            }
            printf("EWA >> major %f minor %f\n", major, minor);

            panic("TextureEmulator", "anisoEWA", "Computed axis coordinate is NaN");
        }
    )*/

    //  Compute the lenght of both vectors
    l1 = (f32bit) GPU_SQRT(axis1[0] * axis1[0] + axis1[1] * axis1[1]);
    l2 = (f32bit) GPU_SQRT(axis2[0] * axis2[0] + axis2[1] * axis2[1]);

//printf("EWA => l1 %f l2 %f\n", l1, l2);

    //  Check the major axis
    if (l1 > l2)
    {
//printf("EWA => Major axis (%f, %f)\n", axis1[0], axis1[1]);
//printf("EWA => Minor axis (%f, %f)\n", axis2[0], axis2[1]);

        //  Calculate anisotropy ratio
        N = l1 / l2;

        //  Clamp aniso ratio to max aniso
        N = GPU_MIN(N, f32bit(maxAniso));

        /*  Calculate the number of samples required.  */
        //samples = u32bit(GPU_CEIL(N));
        samples = computeAnisoSamples(N);

        //  Calculate the texture scale for each sample.
        scale = l1 / N;

        //  Calculate the per anisotropic sample offsets in s,t space.
        dsOffset = axis1[0] / f32bit(samples + 1);
        dtOffset = axis1[1] / f32bit(samples + 1);
    }
    else
    {
//printf("EWA => Major axis (%f, %f)\n", axis2[0], axis2[1]);
//printf("EWA => Minor axis (%f, %f)\n", axis1[0], axis1[1]);

        //  Calculate anisotropy ratio
        N = l2 / l1;

        //  Clamp aniso ratio to max aniso
        N = GPU_MIN(N, f32bit(maxAniso));

        /*  Calculate the number of samples required.  */
        //samples = u32bit(GPU_CEIL(N));
        samples = computeAnisoSamples(N);

        //  Calculate the texture scale for each sample.
        scale = l2 / N;

        //  Calculate the per anisotropic sample offsets in s,t space.
        dsOffset = axis2[0] / f32bit(samples + 1);
        dtOffset = axis2[1] / f32bit(samples + 1);
    }

//printf("EWA => ratio %f samples %d scale %f\n", N, samples, scale);

    //  Check for finite results of the anisotropy algoritm
    if (!(finite(scale) && finite(dsOffset) && finite(dtOffset)))
    {
        samples = 1;

        scale = static_cast<f32bit>(GPU_MAX(GPU_SQRT(dudx * dudx + dvdx * dvdx),
            GPU_SQRT(dudy * dudy + dvdy * dvdy)));

        dsOffset = 0.0f;
        dtOffset = 0.0f;
    }

    return scale;
}

f32bit TextureEmulator::anisoExperimentalAngle(f32bit dudx, f32bit dudy, f32bit dvdx, f32bit dvdy, u32bit maxAniso,
    u32bit &samples, f32bit &dsOffset, f32bit &dtOffset)
{
    f32bit scale;
    f32bit scale1;
    f32bit scale2;
    f32bit px;
    f32bit py;
    f32bit pxy;
    f32bit pyx;
    f32bit pMin;
    f32bit pMax;
    f32bit N;
    f32bit pMin2;
    f32bit pMax2;
    f32bit N2;
    f32bit adjustedMajorLength1;
    f32bit adjustedMinorLength1;
    f32bit adjustedMajorLength2;
    f32bit adjustedMinorLength2;
    f32bit newN1;
    f32bit newN2;
    f32bit factor;
    f32bit vectorA[2];
    f32bit vectorB[2];
    TextureAccess::AnisotropyAxis axis;

    //  Calculate scales for the two window axes.  Select maximum and calculate samples, axis
    //  and offsets.

    //  Calculate texture scale in the horizontal and vertical screen axis.
    px = (f32bit) GPU_SQRT(dudx * dudx + dvdx * dvdx);
    py = (f32bit) GPU_SQRT(dudy * dudy + dvdy * dvdy);

    //  Calculate texture scale on the XY/YX axis (axis rotated 45 degrees).
    pxy = (f32bit) GPU_SQRT((dudx + dudy) * (dudx + dudy) * 0.5 + (dvdx + dvdy) * (dvdx + dvdy) * 0.5);
    pyx = (f32bit) GPU_SQRT((dudx - dudy) * (dudx - dudy) * 0.5 + (dvdx - dvdy) * (dvdx - dvdy) * 0.5);

//printf("EXP-Angle >> (dudx, dvdx) = (%f, %f) | (dudy, dvdy) = (%f, %f)\n", dudx, dvdx, dudy, dvdy);
//printf("EXP-Angle >> (dX + dY) = (%f, %f) | (dX - dY) = (%f, %f)\n",
//dudx + dudy, dvdx + dvdy, dudx - dudy, dvdx - dvdy);
//printf("EXP-Angle >> px = %f | py = %f | pxy = %f | pyx = %f\n", px, py, pxy, pyx);

    //  Calculate the minimum and maximum for both axis.  */
    pMin = GPU_MIN(px, py);
    pMax = GPU_MAX(px, py);

    //  Calculate ratio for X/Y axis.
    N = GPU_MIN(pMax/pMin, f32bit(maxAniso));

    //  Calculate the minimum and maximum for both axis.
    pMin2 = GPU_MIN(pxy, pyx);
    pMax2 = GPU_MAX(pxy, pyx);

    //  Calculate ratio for XY/YX axis.
    N2 = GPU_MIN(pMax2/pMin2, f32bit(maxAniso));

//printf("EXP-Angle >>> areaA = %f | areaB = %f | NA = %f | NB = %f\n",
//pMax * pMin, pMax2 * pMin2, N, N2);

    //  Set vectors
    vectorA[0] = dudx;
    vectorA[1] = dvdx;
    vectorB[0] = dudy;
    vectorB[1] = dvdy;

    //  Compute dot product : cos(angle) * pMax * pMin.
    factor = vectorA[0] * vectorB[0] + vectorA[1] * vectorB[1];

    //  Recalculate major and minor length
    adjustedMajorLength1 = GPU_ABS(factor) / pMax + pMax;
    adjustedMinorLength1 = pMin * GPU_ABS( (f32bit) GPU_SQRT(1 - (factor / (pMin * pMax) * (factor / (pMin * pMax)))));

//printf("EXP-Angle >> (X/Y) original ratio %f factor %f angle %f\n", N, factor, acos(factor / (pMax * pMin)));

    //  Recompute anisotropy ratio.
    newN1 = GPU_MIN(adjustedMajorLength1 / adjustedMinorLength1, f32bit(maxAniso));

//printf("EXP-Angle >> (X/Y) adj Major %f adj Minor %f new ratio %f\n", adjustedMajorLength1, adjustedMinorLength1, N);

    //  Set vectors
    vectorA[0] = (dudx + dudy) / (f32bit) GPU_SQRT(2.0f);
    vectorA[1] = (dvdx + dvdy) / (f32bit) GPU_SQRT(2.0f);
    vectorB[0] = (dudx - dudy) / (f32bit) GPU_SQRT(2.0f);
    vectorB[1] = (dvdx - dvdy) / (f32bit) GPU_SQRT(2.0f);

    //  Compute dot product : cos(angle) * pMax2 * pMin2.
    factor = vectorA[0] * vectorB[0] + vectorA[1] * vectorB[1];

    //  Recalculate major and minor length
    adjustedMajorLength2 = GPU_ABS(factor) / pMax2 + pMax2;
    adjustedMinorLength2 = pMin2 * GPU_ABS((f32bit) GPU_SQRT(1 - (factor / (pMin2 * pMax2) * (factor / (pMin2 * pMax2)))));

//printf("EXP-Angle >> (X+Y/X-Y) original ratio %f factor %f angle %f\n", N2, factor, acos(factor / (pMax2 * pMin2)));

    //  Recompute anisotropy ratio.
    newN2 = GPU_MIN(adjustedMajorLength2 / adjustedMinorLength2, f32bit(maxAniso));

//printf("EXP-Angle >> (X+Y/X-Y) adj Major %f adj Minor %f new ratio %f\n", adjustedMajorLength2, adjustedMinorLength2, N2);

    //  Calculate the texture scale for each sample.
    scale1 = adjustedMajorLength1 / N;
    scale2 = adjustedMajorLength2 / N2;

    if (N > N2)
    //if (scale1 < scale2)
    {
        //  Determine the anisotropy axis.
        if (pMax == px)
            axis = TextureAccess::X_AXIS;
        else
            axis = TextureAccess::Y_AXIS;

        //  Calculate the number of samples required.
        //samples = u32bit(GPU_CEIL(N));
        samples = computeAnisoSamples(N);

        //  Calculate the texture scale for each sample.
        //scale = adjustedMajorLength1 / newN1;
        scale = pMax / newN1;
        //scale = (newN1 > N) ? pMax / newN1 : pMax / N;
    }
    else
    {
        //  Determine the anisotropy axis.
        if (pMax2 == pxy)
            axis = TextureAccess::XY_AXIS;
        else
            axis = TextureAccess::YX_AXIS;

        //  Calculate the number of samples required.
        //samples = u32bit(GPU_CEIL(N2));
        samples = computeAnisoSamples(N2);

        //  Calculate the texture scale for each sample.
        //scale = adjustedMajorLength2 / N2;
        scale = pMax2 / newN2;
    }

//printf("EXP-Angle >> samples = %d | scale = %f | axis = ", samples, scale);

    /*  Calculate the per anisotropic sample offsets in s,t space.  */
    switch(axis)
    {
        case TextureAccess::X_AXIS:
//printf("X ");
            dsOffset = dudx / f32bit(samples + 1);
            dtOffset = dvdx / f32bit(samples + 1);
            break;
        case TextureAccess::Y_AXIS:
//printf("Y ");
            dsOffset = dudy / f32bit(samples + 1);
            dtOffset = dvdy / f32bit(samples + 1);
            break;
        case TextureAccess::XY_AXIS:
//printf("X+Y ");
            dsOffset = ((dudx + dudy) / (f32bit) GPU_SQRT(2)) / f32bit(samples + 1);
            dtOffset = ((dvdx + dvdy) / (f32bit) GPU_SQRT(2)) / f32bit(samples + 1);
            break;
        case TextureAccess::YX_AXIS:
//printf("X-Y");
            dsOffset = ((dudx - dudy) / (f32bit) GPU_SQRT(2)) / f32bit(samples + 1);
            dtOffset = ((dvdx - dvdy) / (f32bit) GPU_SQRT(2)) / f32bit(samples + 1);
            break;
    }
//printf("\n");

    //  Check non finite results of the anisotropy algorithm
    if (!(finite(scale) && finite(dsOffset) && finite(dtOffset)))
    {
        samples = 1;

        scale = static_cast<f32bit>(GPU_MAX(GPU_SQRT(dudx * dudx + dvdx * dvdx),
            GPU_SQRT(dudy * dudy + dvdy * dvdy)));

//printf("Exp-Angle >> Non Finite number detected.  Samples %f scale %f\n", samples, scale);
        dsOffset = 0.0f;
        dtOffset = 0.0f;
    }

    return scale;
}

/*  Calculates level of detail for GPU_TEXTURE3D textures.  */
f32bit TextureEmulator::calculateScale3D(f32bit dudx, f32bit dudy, f32bit dvdx, f32bit dvdy, f32bit dwdx, f32bit dwdy)
{
    f32bit scale;

    /*  Calculate the scale factor.  */
    scale = static_cast<f32bit>(GPU_MAX(GPU_SQRT(dudx * dudx + dvdx * dvdx + dwdx * dwdx),
        GPU_SQRT(dudy * dudy + dvdy * dvdy + dwdy * dwdy)));

    return scale;
}

/*  Calculates, biases and clamps the lod for a fragment.  */
f32bit TextureEmulator::calculateLOD(u32bit textUnit, f32bit fragmentBias, f32bit scaleFactor)
{
    f32bit lod;

    /*  Calculate level of detail.  */
    lod = static_cast<f32bit>(GPU_LOG2(scaleFactor) + GPU_CLAMP(textureLODBias[textUnit] + textureUnitLODBias[textUnit]
        + fragmentBias, -MAX_TEXTURE_LOD_BIAS, MAX_TEXTURE_LOD_BIAS));

    /*  Clamp to defined maximum and minimum lods.  */
    lod = GPU_CLAMP(lod, textureMinLOD[textUnit], textureMaxLOD[textUnit]);

    return lod;
}

/*  Calculates texel coordinates and weight factors.  */
void TextureEmulator::texelCoord(u32bit textUnit, u32bit l, FilterMode filter, u32bit currentSample,
    u32bit numSamples, f32bit dsOffset, f32bit dtOffset, f32bit s, f32bit t, f32bit r,
    u32bit &i, u32bit &j, u32bit &k, f32bit &a, f32bit &b, f32bit &c)
{
    switch(textureMode[textUnit])
    {
        case GPU_TEXTURE1D:
            texelCoord1D(textUnit, l, filter, s, i, a);
            break;

        case GPU_TEXTURE2D:
        case GPU_TEXTURECUBEMAP:
            texelCoord2D(textUnit, l, filter, currentSample, numSamples, dsOffset, dtOffset, s, t, i, j, a, b);
            break;

        case GPU_TEXTURE3D:
            texelCoord3D(textUnit, l, filter, s, t, r, i, j, k, a, b, c);
            break;

        default:
            panic("TextureEmulator", "texelCoord", "Unsupported texture mode.");

    }
}

//  Calculates texel cooordinates for a mipmap level of a GPU_TEXTURE1D texture.
void TextureEmulator::texelCoord1D(u32bit textUnit, u32bit level, FilterMode filter, f32bit s, u32bit &i, f32bit &a)
{
    f32bit u;
    u32bit width;
    
    //  Get the size of the mipmap level in texels.
    //width = 1 << mipmapSize(textureWidth2[textUnit], level);
    width = GPU_MAX(textureWidth[textUnit] >> level, u32bit(1));

    //  Check if the texture coordinates are normalized.
    if (!textureNonNormalized[textUnit])
    {
        //  Apply texture coordinate wrap mode to normalized coordinate.
        u = applyWrap(textureWrapS[textUnit], s, width);

        //  Normalize coordinates.
        u = u * f32bit(width);
    }
    else
    {
        //  Just copy the non-normalized coordinate.
        u = s;
    }
    
    //  Coordinates for point sampling.
    if (filter == GPU_NEAREST)
    {
        //  Calculate texel coordinates.
        i = (u32bit) GPU_FLOOR(u);
    }

    //  Coordinates for bilinear filter.
    if (filter == GPU_LINEAR)
    {
        //  WARNING!! Correct behaviour for wrap mode REPEAT may need an additional
        //  module operation with the texture dimension size.

        //  NOTE:  If black texel not implemented i must be clamped to the texture size.

        //  Calculate texel coordinates.
        i = (u32bit) GPU_FLOOR(u - 0.5f);

        //  Calculate weight factors for bilinear filtering.
        a = (u - 0.5f) - static_cast<f32bit>(GPU_FLOOR(u - 0.5f));
    }
}

//  Calculates texel cooordinates for a mipmap level of a GPU_TEXTURE2D texture.
void TextureEmulator::texelCoord2D(u32bit textUnit, u32bit level, FilterMode filter,
    u32bit currentSample, u32bit numSamples, f32bit dsOffset, f32bit dtOffset, f32bit s, f32bit t,
    u32bit &i, u32bit &j, f32bit &a, f32bit &b)
{
    f32bit u;
    f32bit v;
    u32bit width;
    u32bit height;

    /*  Get the size of the mipmap level in texels.  */
    //width = 1 << mipmapSize(textureWidth2[textUnit], level);
    //height = 1 << mipmapSize(textureHeight2[textUnit], level);
    width = GPU_MAX(textureWidth[textUnit] >> level, u32bit(1));
    height = GPU_MAX(textureHeight[textUnit] >> level, u32bit(1));

    //  Check maximum anisotropy for the texture.
    if (maxAnisotropy[textUnit] > 1)
    {
        GPU_ASSERT(
            if (textureNonNormalized[textUnit])
                printf("TextureEmulator, texelCoord, Anistropic filtering not implemented with non-normalized coordinates.");
        )
        
        //  Apply anisotropic sample offset.
        s = s + dsOffset * (currentSample - 0.5f * f32bit(numSamples + 1));
        t = t + dtOffset * (currentSample - 0.5f * f32bit(numSamples + 1));
    }

    //  Check if the texture coordinates are normalized.
    if (!textureNonNormalized[textUnit])
    {
        //  Normalize coordinates.
        u = applyWrap(textureWrapS[textUnit], s, width);
        v = applyWrap(textureWrapT[textUnit], t, height);

        //  Check if the v direction must be inverted for 2D coordinates as defined by D3D9.
        if (textD3D9VInvert[textUnit])
        {
            // Invert coordinate
            v = 1.0f - v;
        }
           
        //  Normalize coordinates.
        u = u * f32bit(width);
        v = v * f32bit(height);                
    }
    else
    {
        //  Just copy the non-normalized coordinate.
        u = s;
        v = t;
    }
    
    switch(filter)
    {
        case GPU_NEAREST:
            //  Coordinates for point sampling.

            //  Calculate texel coordinates.
            i = (u32bit) GPU_FLOOR(u);
            j = (u32bit) GPU_FLOOR(v);

            break;

        case GPU_LINEAR:

            //  WARNING!! Correct behaviour for wrap mode REPEAT may need an additional
            //  module operation with the texture dimension size.

            //  NOTE:  If black texel not implemented i and j must be clamped to the texture size.

            //  Calculate texel coordinates.
            i = (u32bit) GPU_FLOOR(u - 0.5f);
            j = (u32bit) GPU_FLOOR(v - 0.5f);

            //  Calculate weight factors for bilinear filtering.
            a = (u - 0.5f) - static_cast<f32bit>(GPU_FLOOR(u - 0.5f));
            b = (v - 0.5f) - static_cast<f32bit>(GPU_FLOOR(v - 0.5f));

            break;
    }

//printf("TxEm > textunit %d level %d width %d height %d wrap %d filter %d\n", textUnit,
//level, textureWidth2[textUnit], textureHeight2[textUnit], textureWrapS[textUnit], filter);
//printf("TxEm > (%f, %f) -> (%f, %f) -> (%d, %d)\n", s, t, u, v, i, j);

}

//  Calculates texel cooordinates for a mipmap level of a GPU_TEXTURE3D texture.
void TextureEmulator::texelCoord3D(u32bit textUnit, u32bit level, FilterMode filter,
    f32bit s, f32bit t, f32bit r, u32bit &i, u32bit &j, u32bit &k, f32bit &a, f32bit &b, f32bit &c)
{
    f32bit u;
    f32bit v;
    f32bit w;
    u32bit width;
    u32bit height;
    u32bit depth;


    //  Check if the texture coordinates are normalized.
    if (!textureNonNormalized[textUnit])
    {
        //  Get the size of the mipmap level in texels.
        //width = 1 << mipmapSize(textureWidth2[textUnit], level);
        //height = 1 << mipmapSize(textureHeight2[textUnit], level);
        //depth = 1 << mipmapSize(textureDepth2[textUnit], level);
        width = GPU_MAX(textureWidth[textUnit] >> level, u32bit(1));
        height = GPU_MAX(textureHeight[textUnit] >> level, u32bit(1));
        depth = GPU_MAX(textureDepth[textUnit] >> level, u32bit(1));

        //  Apply wrapping.
        u = applyWrap(textureWrapS[textUnit], s, width);
        v = applyWrap(textureWrapT[textUnit], t, height);
        w = applyWrap(textureWrapR[textUnit], r, depth);

        //  Normalize coordinates.
        u = u * f32bit(width);
        v = v * f32bit(height);
        w = w * f32bit(depth);
    }
    else
    {
        //  Just copy the non-normalized coordinates.
        u = s;
        v = t;
        w = r;
    }
    
    //  Coordinates for point sampling.
    if (filter == GPU_NEAREST)
    {
        //  Calculate texel coordinates.
        i = (u32bit) GPU_FLOOR(u);
        j = (u32bit) GPU_FLOOR(v);
        k = (u32bit) GPU_FLOOR(w);
    }

    //  Coordinates for bilinear filter.
    if (filter == GPU_LINEAR)
    {
        //  WARNING!! Correct behaviour for wrap mode REPEAT may need an additional
        //   module operation with the texture dimension size.

        //  NOTE:  If black texel not implemented i, j and k must be clamped to the texture size.

        //  Calculate texel coordinates.
        i = (u32bit) GPU_FLOOR(u - 0.5f);
        j = (u32bit) GPU_FLOOR(v - 0.5f);
        k = (u32bit) GPU_FLOOR(w - 0.5f);

        //  Calculate weight factors for bilinear filtering.
        a = (u - 0.5f) - static_cast<f32bit>(GPU_FLOOR(u - 0.5f));
        b = (v - 0.5f) - static_cast<f32bit>(GPU_FLOOR(v - 0.5f));
        c = (w - 0.5f) - static_cast<f32bit>(GPU_FLOOR(w - 0.5f));
    }
}

//  Applies a texture wrapping mode to a texture coordinate (texture normalized coordinate mode).
f32bit TextureEmulator::applyWrap(ClampMode wrapMode, f32bit a, u32bit size)
{
    f32bit w;
    f32bit min;
    f32bit max;

    switch(wrapMode)
    {
        case GPU_TEXT_REPEAT:

            /*  Get only fractional part of the coordinate.  */
            w = a - floorf(a);
            break;

        case GPU_TEXT_CLAMP:

            /*  Clamp to [0, 1].  */
            w = GPU_CLAMP(a, 0.0f, 1.0f);

            break;

        case GPU_TEXT_CLAMP_TO_EDGE:

            /*  Clamp so the texture filter doesn't samples a border texel.  */
            min = 1.0f / (2.0f * (f32bit) size);
            max = 1.0f - min;
            w = GPU_CLAMP(a, min, max);
            break;

        case GPU_TEXT_CLAMP_TO_BORDER:

            /*  Clamp so that texture filter samples border texels for coordinates
                sufficiently outside the range [0, 1].  */

            min = -1.0f / (2.0f * (f32bit) size);
            max = 1.0f - min;

            w = GPU_CLAMP(a, min, max);
            break;

        case GPU_TEXT_MIRRORED_REPEAT:

            /*  Mirror texture and clamp to the edge (no border texels sampled).  */

            min = 1.0f / (2.0f * (f32bit) size);
            max = 1.0f - min;

            w = (((u32bit) GPU_FLOOR(a)) & 0x01)?(1.0f - (a - (f32bit) GPU_FLOOR(a))):(a - (f32bit) GPU_FLOOR(a));
            w = GPU_CLAMP(w, min, max);
            break;

        default:

            panic("TextureEmulator", "applyWrap(f32bit)", "Unsupported texture wrap mode.");
            break;
    }

    return w;
}

//  Applies a texture wrapping mode to a texture coordinate (texel coordinate).
u32bit TextureEmulator::applyWrap(ClampMode wrapMode, u32bit i, u32bit size)
{
    u32bit w;

    switch(wrapMode)
    {
        case GPU_TEXT_REPEAT:

            //  Get only fractional part of the coordinate.
            w = GPU_PMOD(i, size);
            break;

        case GPU_TEXT_CLAMP:
        
            w = GPU_MIN(i, size);
            break;
            
        case GPU_TEXT_CLAMP_TO_EDGE:
        
            w = GPU_MIN(i, size - 1);
            break;
            
        case GPU_TEXT_CLAMP_TO_BORDER:

            w = GPU_MIN(i, size);        
            break;
            
        case GPU_TEXT_MIRRORED_REPEAT:

            w = GPU_MIN(i, size - 1);
            break;

        default:

            panic("TextureEmulator", "applyWrap(u32bit)", "Unsupported texture wrap mode.");
            break;
    }

    return w;
}

/*  Filters the texels read for a stamp texture access and generates the final sample value.  */
void TextureEmulator::filter(TextureAccess &textAccess, u32bit nextTrilinearFilter)
{
    //  Check the texture operation type.
    if (textAccess.texOperation == ATTRIBUTE_READ)
    {
        //  Perform data conversion.

        //  Get the stream associated with the         
        u32bit stream = attributeMap[textAccess.textUnit];
        
        //  Check for inactive attributes.
        if (stream == ST_INACTIVE_ATTRIBUTE)
        {
            //  Return default value for inactive attributes (may be they shouldn't be allowed).
            for(u32bit frag = 0; frag < stampFragments; frag++)
            {
                textAccess.sample[frag][0] = attrDefValue[textAccess.textUnit][0];
                textAccess.sample[frag][1] = attrDefValue[textAccess.textUnit][1];
                textAccess.sample[frag][2] = attrDefValue[textAccess.textUnit][2];
                textAccess.sample[frag][3] = attrDefValue[textAccess.textUnit][3];
            }
        }
        else
        {
            //  Convert the read data to the attribute final format (SIMD4 float32).
            for(u32bit frag = 0; frag < stampFragments; frag++)
                loadAttribute(textAccess, nextTrilinearFilter, frag);
        }
        
        return;
    }
    
    //  Filter the current trilinear sample.
    filterTrilinear(textAccess, nextTrilinearFilter);

    //  Check for last trilinear sample.
    if (textAccess.trilinearToFilter == 0)
    {
//printf("TU => combining filtered values\n");
        //  Filter each fragment.
        for(u32bit frag = 0; frag < stampFragments; frag++)
        {
            //  Add all the trilinear samples for the texture access.
            for(u32bit subSample = 0; subSample < textAccess.anisoSamples; subSample++)
            {
                textAccess.sample[frag][0] += textAccess.trilinear[subSample]->sample[frag][0];
                textAccess.sample[frag][1] += textAccess.trilinear[subSample]->sample[frag][1];
                textAccess.sample[frag][2] += textAccess.trilinear[subSample]->sample[frag][2];
                textAccess.sample[frag][3] += textAccess.trilinear[subSample]->sample[frag][3];
            }

            //  Apply weight.
            textAccess.sample[frag][0] = textAccess.sample[frag][0] / f32bit(textAccess.anisoSamples);
            textAccess.sample[frag][1] = textAccess.sample[frag][1] / f32bit(textAccess.anisoSamples);
            textAccess.sample[frag][2] = textAccess.sample[frag][2] / f32bit(textAccess.anisoSamples);
            textAccess.sample[frag][3] = textAccess.sample[frag][3] / f32bit(textAccess.anisoSamples);
        }

/*if (textAccess.accessID == 488)
{
printf("TextureEmulator::filter => AnisoSamples %d\n", textAccess.anisoSamples);
for(u32bit f = 0; f < stampFragments; f++)
{
printf("Pixel %02d\n", f);
for(u32bit s = 0; s < textAccess.anisoSamples; s++)
{
printf("  Sample %02d -> {%f, %f, %f, %f}\n", s,
textAccess.trilinear[s]->sample[f][0], textAccess.trilinear[s]->sample[f][1],
textAccess.trilinear[s]->sample[f][2], textAccess.trilinear[s]->sample[f][3]);
}
printf("  Result -> {%f, %f, %f, %f}\n", textAccess.sample[f][0], textAccess.sample[f][1],
textAccess.sample[f][2], textAccess.sample[f][3]);
}
}*/

    }
}

/*  Filters the texels read for a trilinear access.  */
void TextureEmulator::filterTrilinear(TextureAccess &textAccess, u32bit trilinearAccess)
{
    u32bit i;
    QuadFloat t1, t2;
    f32bit w;

    GPU_ASSERT(
        if (textAccess.trilinearToFilter == 0)
            panic("TextureEmulator", "filterTrilinear", "Requesting to filter more trilinear samples than those required.");
    )

    /*  Update number of trilinear samples to filter for the current texture access.  */
    textAccess.trilinearToFilter--;

    /*  Filter each fragment.  */
    for(i = 0; i < stampFragments; i++)
    {
        /*  Select filter mode.  */
        switch(textAccess.filter[i])
        {
            case GPU_NEAREST:
            case GPU_NEAREST_MIPMAP_NEAREST:

                //  Point sampling.  Just copy the read texel.
                textAccess.trilinear[trilinearAccess]->sample[i][0] = textAccess.trilinear[trilinearAccess]->texel[i][0][0];
                textAccess.trilinear[trilinearAccess]->sample[i][1] = textAccess.trilinear[trilinearAccess]->texel[i][0][1];
                textAccess.trilinear[trilinearAccess]->sample[i][2] = textAccess.trilinear[trilinearAccess]->texel[i][0][2];
                textAccess.trilinear[trilinearAccess]->sample[i][3] = textAccess.trilinear[trilinearAccess]->texel[i][0][3];

                break;

            case GPU_LINEAR:
            case GPU_LINEAR_MIPMAP_NEAREST:

                //  Bilinear filtering.
                t1 = bilinearFilter(textAccess, trilinearAccess, 0, i);
                textAccess.trilinear[trilinearAccess]->sample[i][0] = t1[0];
                textAccess.trilinear[trilinearAccess]->sample[i][1] = t1[1];
                textAccess.trilinear[trilinearAccess]->sample[i][2] = t1[2];
                textAccess.trilinear[trilinearAccess]->sample[i][3] = t1[3];

                break;

            case GPU_NEAREST_MIPMAP_LINEAR:

                //  Check if two mipmaps were sampled.
                if (textAccess.trilinear[trilinearAccess]->sampleFromTwoMips[i])
                {
                    //  Calculate weight between mipmaps as fractional part of the lod.
                    w = textAccess.lod[i] - static_cast<f32bit>(GPU_FLOOR(textAccess.lod[i]));
                    
                    //  Point samplig from two mip maps.  Just copy the two read texels.
                    t1 = textAccess.trilinear[trilinearAccess]->texel[i][0];
                    t2 = textAccess.trilinear[trilinearAccess]->texel[i][1];

                    //  Interpolate between two mipmaps.
                    textAccess.trilinear[trilinearAccess]->sample[i][0] = (t1[0] * (1.0f - w) + t2[0] * w);
                    textAccess.trilinear[trilinearAccess]->sample[i][1] = (t1[1] * (1.0f - w) + t2[1] * w);
                    textAccess.trilinear[trilinearAccess]->sample[i][2] = (t1[2] * (1.0f - w) + t2[2] * w);
                    textAccess.trilinear[trilinearAccess]->sample[i][3] = (t1[3] * (1.0f - w) + t2[3] * w);
                }
                else
                {
                    //  Point sampling from single mip map.  Just copy the read texels.
                    textAccess.trilinear[trilinearAccess]->sample[i][0] = textAccess.trilinear[trilinearAccess]->texel[i][0][0];
                    textAccess.trilinear[trilinearAccess]->sample[i][1] = textAccess.trilinear[trilinearAccess]->texel[i][0][1];
                    textAccess.trilinear[trilinearAccess]->sample[i][2] = textAccess.trilinear[trilinearAccess]->texel[i][0][2];
                    textAccess.trilinear[trilinearAccess]->sample[i][3] = textAccess.trilinear[trilinearAccess]->texel[i][0][3];
                }
                
                break;

            case GPU_LINEAR_MIPMAP_LINEAR:
               
                //  Check if two mipmaps were sampled.
                if (textAccess.trilinear[trilinearAccess]->sampleFromTwoMips[i])
                {
                    //  Trilinear filter.

                    //  Calculate weight between mipmaps as fractional part of the lod.
                    w = textAccess.lod[i] - static_cast<f32bit>(GPU_FLOOR(textAccess.lod[i]));

                    //  Compute bilinear samples for two mipmaps.
                    t1 = bilinearFilter(textAccess, trilinearAccess, 0, i);
                    t2 = bilinearFilter(textAccess, trilinearAccess, 1, i);

                    /// Interpolate between two mipmaps.
                    textAccess.trilinear[trilinearAccess]->sample[i][0] = (t1[0] * (1.0f - w) + t2[0] * w);
                    textAccess.trilinear[trilinearAccess]->sample[i][1] = (t1[1] * (1.0f - w) + t2[1] * w);
                    textAccess.trilinear[trilinearAccess]->sample[i][2] = (t1[2] * (1.0f - w) + t2[2] * w);
                    textAccess.trilinear[trilinearAccess]->sample[i][3] = (t1[3] * (1.0f - w) + t2[3] * w);

/*if (textAccess.accessID == 488)
{
printf("TexEmu::filterTrilinear => Pixel %02d -> sample LOD0 = {%f, %f, %f, %f}\n", i, t1[0], t1[1], t1[2], t1[3]);
printf("TexEmu::filterTrilinear => Pixel %02d -> sample LOD1 = {%f, %f, %f, %f}\n", i, t2[0], t2[1], t2[2], t2[3]);
printf("TexEmu::filterTrilinear => Pixel %02d -> w = %f  result = {%f, %f, %f, %f}\n", i, w, 
textAccess.trilinear[trilinearAccess]->sample[i][0], textAccess.trilinear[trilinearAccess]->sample[i][1],
textAccess.trilinear[trilinearAccess]->sample[i][2], textAccess.trilinear[trilinearAccess]->sample[i][3]);
}*/
                }
                else
                {
                    //  Compute bilinear sample for a single mipmap.
                    t1 = bilinearFilter(textAccess, trilinearAccess, 0, i);
                    textAccess.trilinear[trilinearAccess]->sample[i][0] = t1[0];
                    textAccess.trilinear[trilinearAccess]->sample[i][1] = t1[1];
                    textAccess.trilinear[trilinearAccess]->sample[i][2] = t1[2];
                    textAccess.trilinear[trilinearAccess]->sample[i][3] = t1[3];
                }
                
                break;

            default:
                panic("TextureEmulator", "filter", "Unsupported filter mode.");
                break;
        }
    }
}

/*  Performs a bilinear filtering at a mipmap level for a fragment in a stamp texture access. */
QuadFloat TextureEmulator::bilinearFilter(TextureAccess &textAccess, u32bit trilinearAccess, u32bit level, u32bit frag)
{
    QuadFloat sample;
    f32bit a, b, c;

    /*  Select texture mode.  */
    switch(textureMode[textAccess.textUnit])
    {
        case GPU_TEXTURE1D:

            a = textAccess.trilinear[trilinearAccess]->a[frag][level];

            /*  Linear filtering between two points.  */
            sample[0] = textAccess.trilinear[trilinearAccess]->texel[frag][level * 2 + 1][0] * a
                + textAccess.trilinear[trilinearAccess]->texel[frag][level * 2 + 0][0] * (1.0f - a);
            sample[1] = textAccess.trilinear[trilinearAccess]->texel[frag][level * 2 + 1][1] * a
                + textAccess.trilinear[trilinearAccess]->texel[frag][level * 2 + 0][1] * (1.0f - a);
            sample[2] = textAccess.trilinear[trilinearAccess]->texel[frag][level * 2 + 1][2] * a
                + textAccess.trilinear[trilinearAccess]->texel[frag][level * 2 + 0][2] * (1.0f - a);
            sample[3] = textAccess.trilinear[trilinearAccess]->texel[frag][level * 2 + 1][3] * a
                + textAccess.trilinear[trilinearAccess]->texel[frag][level * 2 + 0][3] * (1.0f - a);

            break;

        case GPU_TEXTURE2D:
        case GPU_TEXTURECUBEMAP:

            /*  Get filter weights.  */
            a = textAccess.trilinear[trilinearAccess]->a[frag][level];
            b = textAccess.trilinear[trilinearAccess]->b[frag][level];

            /*  Bilinear filtering between four points.  */
            sample[0] = textAccess.trilinear[trilinearAccess]->texel[frag][level * 4 + 0][0] * (1.0f - a) * (1.0f - b) +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 4 + 1][0] * a * (1.0f - b) +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 4 + 2][0] * (1.0f - a) * b +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 4 + 3][0] * a * b;

            sample[1] = textAccess.trilinear[trilinearAccess]->texel[frag][level * 4 + 0][1] * (1.0f - a) * (1.0f - b) +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 4 + 1][1] * a * (1.0f - b) +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 4 + 2][1] * (1.0f - a) * b +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 4 + 3][1] * a * b;

            sample[2] = textAccess.trilinear[trilinearAccess]->texel[frag][level * 4 + 0][2] * (1.0f - a) * (1.0f - b) +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 4 + 1][2] * a * (1.0f - b) +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 4 + 2][2] * (1.0f - a) * b +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 4 + 3][2] * a * b;

            sample[3] = textAccess.trilinear[trilinearAccess]->texel[frag][level * 4 + 0][3] * (1.0f - a) * (1.0f - b) +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 4 + 1][3] * a * (1.0f - b) +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 4 + 2][3] * (1.0f - a) * b +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 4 + 3][3] * a * b;

/*if (textAccess.accessID == 488)
{
printf("TexEmu::bilinearFilter => Pixel %02d -> a = %f b = %f level = %d\n", frag, a, b, level);
for(u32bit t = 0; t < 4; t++)
printf("   Texel %02d -> {%f, %f, %f, %f}\n", t,
textAccess.trilinear[trilinearAccess]->texel[frag][level * 4 + t][0],
textAccess.trilinear[trilinearAccess]->texel[frag][level * 4 + t][1],
textAccess.trilinear[trilinearAccess]->texel[frag][level * 4 + t][2],
textAccess.trilinear[trilinearAccess]->texel[frag][level * 4 + t][3]);
printf("   Result -> {%f, %f, %f, %f}\n", sample[0], sample[1], sample[2], sample[3]);
}*/
//printf("Filter => %f %f %f %f\n", sample[0],  sample[1],  sample[2],  sample[3]);
            break;

        case GPU_TEXTURE3D:

            /*  Get filter weights.  */
            a = textAccess.trilinear[trilinearAccess]->a[frag][level];
            b = textAccess.trilinear[trilinearAccess]->b[frag][level];
            c = textAccess.trilinear[trilinearAccess]->c[frag][level];

            /*  Bilinear filtering for 8 samples.  */
            sample[0] = textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 0][0] * (1.0f - a) * (1.0f - b) * (1.0f - c) +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 1][0] * a * (1.0f - b) * (1.0f - c) +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 2][0] * (1.0f - a) * b * (1.0f - c) +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 3][0] * a * b * (1.0f - c)+
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 4][0] * (1.0f - a) * (1.0f - b) * c +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 5][0] * a * (1.0f - b) * c +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 6][0] * (1.0f - a) * b * c +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 7][0] * a * b * c;

            sample[1] = textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 0][1] * (1.0f - a) * (1.0f - b) * (1.0f - c) +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 1][1] * a * (1.0f - b) * (1.0f - c) +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 2][1] * (1.0f - a) * b * (1.0f - c) +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 3][1] * a * b * (1.0f - c)+
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 4][1] * (1.0f - a) * (1.0f - b) * c +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 5][1] * a * (1.0f - b) * c +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 6][1] * (1.0f - a) * b * c +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 7][1] * a * b * c;

            sample[2] = textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 0][2] * (1.0f - a) * (1.0f - b) * (1.0f - c) +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 1][2] * a * (1.0f - b) * (1.0f - c) +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 2][2] * (1.0f - a) * b * (1.0f - c) +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 3][2] * a * b * (1.0f - c)+
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 4][2] * (1.0f - a) * (1.0f - b) * c +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 5][2] * a * (1.0f - b) * c +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 6][2] * (1.0f - a) * b * c +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 7][2] * a * b * c;

            sample[3] = textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 0][3] * (1.0f - a) * (1.0f - b) * (1.0f - c) +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 1][3] * a * (1.0f - b) * (1.0f - c) +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 2][3] * (1.0f - a) * b * (1.0f - c) +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 3][3] * a * b * (1.0f - c)+
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 4][3] * (1.0f - a) * (1.0f - b) * c +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 5][3] * a * (1.0f - b) * c +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 6][3] * (1.0f - a) * b * c +
                textAccess.trilinear[trilinearAccess]->texel[frag][level * 8 + 7][3] * a * b * c;

            break;

        default:
            panic("TextureEmulator", "bilinearFilter", "Unsupported texture mode.");
    }

    return sample;
}

/*
    Textures are stored in memory in the following manner:

        1) The texel components (for example four color components) are
           stored in consecutive addresses.
        2) Each texture mipmap is blocked using the 6D method:

            a) A superblock has the size of the texture cache (or smaller).
            b) A block has the size of of a texture cache line.
            c) Blocks in a superblock are in Morton (also called Z) order.
            d) Texels in a block are in Morton (also called Z) order.

    Morton order for 16 elements:

         0  1  4  5
         2  3  6  7
         8  9 12 13
        10 11 14 15

    Both block and superblocks are supossed to be 2^n x 2^n for a given
    parameter n defined by the texture cache configuration.

*/

/*  Translates a texture texel address to a physical texture memory address for 1D textures.  */
u64bit TextureEmulator::texel2address(u32bit textUnit, u32bit level, u32bit i)
{
    u64bit address;

    /*  Set black texel address for texels outside the texture.  */
    //if (i >= u32bit(1) << mipmapSize(textureWidth2[textUnit], level))
    if (i >= u32bit(GPU_MAX(textureWidth[textUnit] >> level, u32bit(1))))
        return BLACK_TEXEL_ADDRESS;

    /*  Calculate texel address in texture memory.  */
    address = textureAddress[textUnit][level][0] + adjustToFormat(textUnit, i);

    return address;
}

/*  Translates a texture texel address to a physical texture memory address for 2D textures.  */
u64bit TextureEmulator::texel2address(u32bit textUnit, u32bit level, u32bit cubemap, u32bit i, u32bit j)
{
    u64bit address;
    u64bit texelAddr;
    u64bit blockAddr;
    u64bit sBlockAddr;
    u32bit sBlocksPerLine;
    u32bit mipmapWidthLog2;
    
    u32bit frameBufferTexelAddr;    
  
    //  Set black texel address for texels outside the texture.
    //if ((i >= u32bit(1) << mipmapSize(textureWidth2[textUnit], level))) ||
    //    (j >= u32bit(1) << mipmapSize(textureHeight2[textUnit], level))))
    if ((i >= u32bit(GPU_MAX(textureWidth[textUnit] >> level, u32bit(1)))) |
        (j >= u32bit(GPU_MAX(textureHeight[textUnit] >> level, u32bit(1)))))
        return BLACK_TEXEL_ADDRESS;

    //  Determine the address space for the texture access.
    switch(textureCompr[textUnit])
    {
        case GPU_NO_TEXTURE_COMPRESSION:

            //  Use uncompressed texture address space.
            address = textureAddress[textUnit][level][cubemap];

            break;

        case GPU_S3TC_DXT1_RGB:

            //  Use compressed texture address space for ratio 1:8.
            address = (u64bit(textureAddress[textUnit][level][cubemap]) << DXT1_SPACE_SHIFT) + COMPRESSED_TEXTURE_SPACE_DXT1_RGB;

            break;
        case GPU_S3TC_DXT1_RGBA:

            //  Use compressed texture address space for ratio 1:8.
            address = (u64bit(textureAddress[textUnit][level][cubemap]) << DXT1_SPACE_SHIFT) + COMPRESSED_TEXTURE_SPACE_DXT1_RGBA;

            break;

        case GPU_S3TC_DXT3_RGBA:

            //  Use compressed texture address space for ratio 1:4.
            address = (u64bit(textureAddress[textUnit][level][cubemap]) << DXT3_DXT5_SPACE_SHIFT) + COMPRESSED_TEXTURE_SPACE_DXT3_RGBA;

            break;

        case GPU_S3TC_DXT5_RGBA:

            //  Use compressed texture address space for ratio 1:4.
            address = (u64bit(textureAddress[textUnit][level][cubemap]) << DXT3_DXT5_SPACE_SHIFT) + COMPRESSED_TEXTURE_SPACE_DXT5_RGBA;

            break;

        case GPU_LATC1:
        
            //  Use compressed texture address space for ratio 1:2.
            address = (u64bit(textureAddress[textUnit][level][cubemap]) << LATC1_LATC2_SPACE_SHIFT) + COMPRESSED_TEXTURE_SPACE_LATC1;
            
            break;            

        case GPU_LATC1_SIGNED:
        
            //  Use compressed texture address space for ratio 1:2.
            address = (u64bit(textureAddress[textUnit][level][cubemap]) << LATC1_LATC2_SPACE_SHIFT) + COMPRESSED_TEXTURE_SPACE_LATC1_SIGNED;
            
            break;            

        case GPU_LATC2:
        
            //  Use compressed texture address space for ratio 1:2.
            address = (u64bit(textureAddress[textUnit][level][cubemap]) << LATC1_LATC2_SPACE_SHIFT) + COMPRESSED_TEXTURE_SPACE_LATC2;
            
            break;            

        case GPU_LATC2_SIGNED:
        
            //  Use compressed texture address space for ratio 1:2.
            address = (u64bit(textureAddress[textUnit][level][cubemap]) << LATC1_LATC2_SPACE_SHIFT) + COMPRESSED_TEXTURE_SPACE_LATC2_SIGNED;
            
            break;            
        
        default:

            panic("TextureEmulator", "texel2address", "Undefined texture compression mode.");
            break;
    }

    //  Select blocking/tiling mode for the texture.
    switch(textureBlocking[textUnit])
    {
        case GPU_TXBLOCK_TEXTURE:
        
            //  Calculate the address of the texel inside the block using Morton order.
            texelAddr = GPUMath::morton(textCacheBlockDim, i, j);
        
            //  Calculate the address of the block inside the superblock using Morton order.
            blockAddr = GPUMath::morton(textCacheSBlockDim, i >> textCacheBlockDim, j >> textCacheBlockDim);
        
            //  Calculate the 2's logarithm of the mipmap width.
            mipmapWidthLog2 = u32bit(GPU_CEIL(GPU_LOG2(GPU_MAX(textureWidth[textUnit] >> level, u32bit(1)))));
            
            //  Calculate the number of 
            sBlocksPerLine = GPU_MAX(s32bit(mipmapWidthLog2) - s32bit((textCacheSBlockDim + textCacheBlockDim)), s32bit(0));

            //  Calculate the address of the superblock inside the cache.
            //sBlockAddr = ((j >> (textCacheSBlockDim + textCacheBlockDim)) *
            //    GPU_MAX(s32bit(GPU_MAX(textureWidth[textUnit] >> level, u32bit(1))) >> (textCacheSBlockDim + textCacheBlockDim), s32bit(0))) +
            //    (i >> (textCacheSBlockDim + textCacheBlockDim));
            sBlockAddr = ((j >> (textCacheSBlockDim + textCacheBlockDim)) << sBlocksPerLine) + 
                         (i >> (textCacheSBlockDim + textCacheBlockDim));
                   
            //  Add the texel address to the base 2D image address.
            address += adjustToFormat(textUnit, (((sBlockAddr << (2 * textCacheSBlockDim)) +
                blockAddr) << (2 * textCacheBlockDim)) + texelAddr);

            break;
            
        case GPU_TXBLOCK_FRAMEBUFFER:
        
            //  Compute the frame buffer texel address.
                                        
            //  Check if the corresponding pixel mapper is already configured.
            if (!pixelMapperConfigured[textUnit])                                        
            {
                //  Update the pixel mapper for framebuffer textures.
                u32bit samples = 1;
                u32bit bytesSample = 1;
                
                switch(textureFormat[textUnit])
                {
                    case GPU_RGBA8888:
                    case GPU_RG16F:
                    case GPU_R32F:
                    case GPU_DEPTH_COMPONENT24:
                        bytesSample = 4;
                        break;
                    case GPU_RGBA16:
                    case GPU_RGBA16F:
                        bytesSample = 8;
                        break;
                    default:
                        printf("T. Unit: %d\n", textUnit);
                        printf("Enabled: %d\n", textureEnabled[textUnit]);
                        printf("Format: %d\n", textureFormat[textUnit]);
                        printf("Width: %d\n", textureWidth[textUnit]);
                        printf("Height: %d\n", textureHeight[textUnit]);
                        printf("Compressed: %d\n", textureCompr[textUnit]);
                        panic("TextureEmulator", "texel2address", "Unsupported format for framebuffer textures.");
                        break;
                }                
                
                texPixelMapper[textUnit].setupDisplay(1024, 1024, STAMP_WIDTH, STAMP_HEIGHT,
                                                    genTileWidth, genTileHeight,
                                                    scanTileWidth, scanTileHeight,
                                                    overScanTileWidth, overScanTileHeight,
                                                    samples, bytesSample);
            
                //  Set as configured.
                pixelMapperConfigured[textUnit] = true;
            }
            
            texPixelMapper[textUnit].changeResolution(GPU_MAX(textureWidth[textUnit] >> level, u32bit(1)),
                                                      GPU_MAX(textureHeight[textUnit] >> level, u32bit(1)));
            frameBufferTexelAddr = texPixelMapper[textUnit].computeAddress(i, j);

            address += u64bit(frameBufferTexelAddr);
        
//printf(" TextureEmulator => witdh = %d height = %d (i, j) = (%d, %d) -> texel address = %08x final address = %016llx\n",
//GPU_MAX(textureWidth[textUnit] >> level, u32bit(1)), GPU_MAX(textureHeight[textUnit] >> level, u32bit(1)), i, j, frameBufferTexelAddr, address);

            break;

        default:
        
            panic("TextureEmulator", "texel2address(2D)", "Undefined texture blocking mode.");
            break;
    }
    
//printf("texture Width %d level %d mipmapSize %d is %d\n",
//textureWidth2[textUnit],
//level,
//mipmapSize(textureWidth2[textUnit], level),
//GPU_MAX(s32bit(mipmapSize(textureWidth2[textUnit], level) - (textCacheSBlockDim + textCacheBlockDim)), s32bit(0)));

//u64bit backupAddress = address;

//printf("TxEm > address surface (unit %d level %d cubemap %d) => %llx\n", textUnit, level, cubemap, address);
//printf("TxEm > i,j %d,%d sblockAddr %lld blockAddr %lld texelAddr %lld\n", i, j, sBlockAddr, blockAddr, texelAddr);

    /*switch(textureCompr[textUnit])
    {
        case GPU_NO_TEXTURE_COMPRESSION:

            if (((u32bit) (address && 0xffff0000)) == 0x37270000)
            {
                printf("TxEm > texture Width %d level %d mipmapSize %d is %d\n",
                textureWidth2[textUnit], level, mipmapSize(textureWidth2[textUnit], level),
                GPU_MAX(s32bit(mipmapSize(textureWidth2[textUnit], level) - (textCacheSBlockDim + textCacheBlockDim)), s32bit(0)));
                printf("TxEm > address surface (unit %d level %d cubemap %d) => %llx\n", textUnit, level, cubemap, backupAddress);
                printf("TxEm > i,j %d,%d sblockAddr %lld blockAddr %lld texelAddr %lld\n", i, j, sBlockAddr, blockAddr, texelAddr);
                printf("TxEm > address texel %llx\n", address);
            }

            break;

        case GPU_S3TC_DXT1_RGB:
        case GPU_S3TC_DXT1_RGBA:

            if (((u32bit) ((address >> DXT1_SPACE_SHIFT) & 0xffff0000)) == 0x37270000)
            {
                printf("TxEm > texture Width %d level %d mipmapSize %d is %d\n",
                textureWidth2[textUnit], level, mipmapSize(textureWidth2[textUnit], level),
                GPU_MAX(s32bit(mipmapSize(textureWidth2[textUnit], level) - (textCacheSBlockDim + textCacheBlockDim)), s32bit(0)));
                printf("TxEm > address surface (unit %d level %d cubemap %d) => %llx\n", textUnit, level, cubemap, backupAddress);
                printf("TxEm > i,j %d,%d sblockAddr %lld blockAddr %lld texelAddr %lld\n", i, j, sBlockAddr, blockAddr, texelAddr);
                printf("TxEm > address texel %llx\n", address);
            }

            break;

        case GPU_S3TC_DXT3_RGBA:
        case GPU_S3TC_DXT5_RGBA:

            if (((u32bit) ((address >> DXT3_DXT5_SPACE_SHIFT) & 0xffff0000)) == 0x37270000)
            {
                printf("TxEm > texture Width %d level %d mipmapSize %d is %d\n",
                textureWidth2[textUnit], level, mipmapSize(textureWidth2[textUnit], level),
                GPU_MAX(s32bit(mipmapSize(textureWidth2[textUnit], level) - (textCacheSBlockDim + textCacheBlockDim)), s32bit(0)));
                printf("TxEm > address surface (unit %d level %d cubemap %d) => %llx\n", textUnit, level, cubemap, backupAddress);
                printf("TxEm > i,j %d,%d sblockAddr %lld blockAddr %lld texelAddr %lld\n", i, j, sBlockAddr, blockAddr, texelAddr);
                printf("TxEm > address texel %llx\n", address);
            }

            break;

    }*/
//printf("TxEm > address texel %llx\n", address);

    return address;
}

/*  Translates a texture texel address to a physical texture memory address for 3D textures.  */
u64bit TextureEmulator::texel2address(u32bit textUnit, u32bit level, u32bit cubemap, u32bit i, u32bit j, u32bit k)
{
    u64bit address;
    u64bit texelAddr;
    u64bit blockAddr;
    u64bit sBlockAddr;
    u32bit sBlocksPerLine;
    u32bit mipmapWidthLog2;

    switch(textureMode[textUnit])
    {
        case GPU_TEXTURE1D:

            address = texel2address(textUnit, level, i);

            break;

        case GPU_TEXTURE2D:
        case GPU_TEXTURECUBEMAP:

            address = texel2address(textUnit, level, cubemap, i, j);

            break;

        case GPU_TEXTURE3D:

            /*  Set black texel address for texels outside the texture.  */
            //if ((i >= (u32bit(1) << mipmapSize(textureWidth2[textUnit], level))) ||
            //    (j >= (u32bit(1) << mipmapSize(textureHeight2[textUnit], level))) ||
            //    (k >= (u32bit(1) << mipmapSize(textureDepth2[textUnit], level))))
            if ((i >= u32bit(GPU_MAX(textureWidth[textUnit] >> level, u32bit(1)))) ||
                (j >= u32bit(GPU_MAX(textureHeight[textUnit] >> level, u32bit(1)))) ||
                (k >= u32bit(GPU_MAX(textureDepth[textUnit] >> level, u32bit(1)))))
                return BLACK_TEXEL_ADDRESS;

            //  Calculate the address of the texel inside the block using Morton order.
            texelAddr = GPUMath::morton(textCacheBlockDim, i, j);
        
            //  Calculate the address of the texel inside the block using Morton order.
            texelAddr = GPUMath::morton(textCacheBlockDim, i, j);

            //  Calculate the address of the block inside the superblock using Morton order.
            blockAddr = GPUMath::morton(textCacheSBlockDim, i >> textCacheBlockDim, j >> textCacheBlockDim);

            /*  Calculate the address of the superblock inside the cache.  */
            //sBlockAddr = ((j >> (textCacheSBlockDim + textCacheBlockDim)) <<
                //GPU_MAX(mipmapSize(textureWidth2[textUnit], level) - (textCacheSBlockDim + textCacheBlockDim), (u32bit)0)) +
            //    GPU_MAX(GPU_MAX(textureWidth[textUnit] >> level, u32bit(1)) >> (textCacheSBlockDim + textCacheBlockDim), (u32bit)0)) +
            //    (i >> (textCacheSBlockDim + textCacheBlockDim));
            
            //  Calculate the 2's logarithm of the mipmap width.
            mipmapWidthLog2 = u32bit(GPU_CEIL(GPU_LOG2(GPU_MAX(textureWidth[textUnit] >> level, u32bit(1)))));
            
            //  Calculate the number of 
            sBlocksPerLine = GPU_MAX(s32bit(mipmapWidthLog2) - s32bit((textCacheSBlockDim + textCacheBlockDim)), s32bit(0));

            //  Calculate the address of the superblock inside the cache.
            sBlockAddr = ((j >> (textCacheSBlockDim + textCacheBlockDim)) << sBlocksPerLine) + 
                         (i >> (textCacheSBlockDim + textCacheBlockDim));

            address = textureAddress[textUnit][level][0] + adjustToFormat(textUnit,
                //(k << ((mipmapSize(textureWidth2[textUnit], level)) + mipmapSize(textureHeight2[textUnit], level))) +
                (k * GPU_MAX(textureWidth[textUnit] >> level, u32bit(1)) *  GPU_MAX(textureHeight[textUnit] >> level, u32bit(1))) +
                (((sBlockAddr << (2 * textCacheSBlockDim)) + blockAddr) << (2 * textCacheBlockDim)) + texelAddr);

            break;
    }

    return address;
}

/*  Calculates the size (as 2^n exponent) of the texture mipmap level dimension.  */
u32bit TextureEmulator::mipmapSize(u32bit topSize, u32bit level)
{
    return GPU_MAX((s32bit) topSize - (s32bit) level, (s32bit) 0);
}

/*  Adjust a texel address to the size and type of the texture format.  */
u64bit TextureEmulator::adjustToFormat(u32bit textUnit, u64bit texelAddress)
{
    u64bit address;

    switch(textureFormat[textUnit])
    {
        case GPU_RGBA32F:
        
            //  Sixteen bytes per texel.
            address = texelAddress << 4;
            
            break;
            
        case GPU_RGBA16:
        case GPU_RGBA16F:
        case GPU_RG32F:
        
            //  Eight bytes per texel.
            address = texelAddress << 3;
            
            break;

        case GPU_DEPTH_COMPONENT24:
        case GPU_DEPTH_COMPONENT32:
        case GPU_LUMINANCE12_ALPHA12:
        case GPU_LUMINANCE16_ALPHA16:
        case GPU_RGB888:
        case GPU_RGB101010:
        case GPU_RGB121212:
        case GPU_RGBA8888:
        case GPU_RGBA1010102:
        case GPU_RG16:
        case GPU_RG16F:
        case GPU_R32F:

            /*  Four bytes per texel.  */
            address = texelAddress << 2;

            break;

        case GPU_ALPHA16:
        case GPU_DEPTH_COMPONENT16:
        case GPU_LUMINANCE12:
        case GPU_LUMINANCE16:
        case GPU_LUMINANCE8_ALPHA8:
        case GPU_LUMINANCE8_ALPHA8_SIGNED:
        case GPU_LUMINANCE12_ALPHA4:
        case GPU_INTENSITY12:
        case GPU_INTENSITY16:
        case GPU_RGB444:
        case GPU_RGB555:
        case GPU_RGB565:
        case GPU_RGBA4444:
        case GPU_RGBA5551:
        case GPU_R16:
        case GPU_R16F:

            /*  Two bytes per texel.  */
            address = texelAddress << 1;

            break;

        case GPU_ALPHA8:
        case GPU_LUMINANCE8:
        case GPU_LUMINANCE8_SIGNED:
        case GPU_LUMINANCE4_ALPHA4:
        case GPU_LUMINANCE6_ALPHA2:
        case GPU_INTENSITY8:
        case GPU_RGB332:
        case GPU_RGBA2222:

            /*  One byte per texel.  */
            address = texelAddress;

            break;

        default:
            panic("TextureEmulator", "adjustToFormat", "Unsupported texture format.");
    }

    return address;
}

#define LINEAR(x) f32bit(GPU_POWER(f64bit(x), f64bit(2.2f)))

/*  Converts texel data from their native source to float point format.  */
void TextureEmulator::convertFormat(TextureAccess &textAccess, u32bit trilinearAccess, u32bit frag, u32bit tex, u8bit *data)
{

    //  Check the type of texture operation.
    if (textAccess.texOperation == ATTRIBUTE_READ)
    {
        //  Check if this is the first cache read.
        if (tex == 0)
        {
            //  Copy the attribute data from the first read.
            u32bit readOffset = textAccess.trilinear[trilinearAccess]->attrFirstOffset[frag];
            u32bit readSize = textAccess.trilinear[trilinearAccess]->attrFirstSize[frag];
//printf("TxEmu::convertFormat => ATTRIBUTE_READ -> (1) copying %d bytes from %d\n", readSize, readOffset);
            for(u32bit b = 0; b < readSize; b++)
                textAccess.trilinear[trilinearAccess]->attrData[frag][b] = data[readOffset + b];                
        }
        else if (tex == 1)
        {
            GPU_ASSERT(
                if (textAccess.trilinear[trilinearAccess]->texelsLoop[frag] != 2)
                {
                    panic("TextureEmulator", "convertFormat", "Unexpected cache read for attribute read.");
                }
            )
            
            //  Copy the attribute data from the second read.
            u32bit dataOffset = textAccess.trilinear[trilinearAccess]->attrFirstSize[frag];
            u32bit readSize = textAccess.trilinear[trilinearAccess]->attrSecondSize[frag];
//printf("TxEmu::convertFormat => ATTRIBUTE_READ -> (2) copying %d bytes to %d\n", readSize, dataOffset);
            for(u32bit b = 0; b < readSize; b++)
                textAccess.trilinear[trilinearAccess]->attrData[frag][b + dataOffset] = data[b];
        }
        else
        {
            panic("TextureEmulator", "convertFormat", "For attribute reads maximum two cache line splits expected.");
        }
        
        //  NOTE:  Data conversion for attributes is performed at filter stage because the Texture Unit
        //  implementation doesn't garantees the order of the two split data reads.
        
        return;
    }
    
    u64bit address;
    
    /*  Get texel address.  */
    address = textAccess.trilinear[trilinearAccess]->address[frag][tex];

    switch(textureFormat[textAccess.textUnit])
    {
        case GPU_ALPHA8:

            /*  Eight bits for alpha component.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = f32bit(data[address & 0x03]) / 255.0f;

            break;

        case GPU_ALPHA12:

            /*  Twelve bits for alpha component.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = f32bit(*((u16bit *) &data[address & 0x02]) & 0x0fff) / 4095.0f;

            break;

        case GPU_ALPHA16:

            /*  Sixteen bits for alpha component.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = f32bit(*((u16bit *) &data[address & 0x02])) / 65535.0f;

            break;

        case GPU_LUMINANCE8:

            /*  Eight bits for luminance component.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = 
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = 
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = f32bit(data[address & 0x03]) / 255.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = 1.0f;

            break;

        case GPU_LUMINANCE8_SIGNED:

            /*  Eight bits for luminance component.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = 
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = 
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = f32bit(((s8bit *) data)[address & 0x03]) / 127.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = 1.0f;

            break;

        case GPU_LUMINANCE12:

            /*  Twelve bits for luminance component.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit(*((u16bit *) &data[address & 0x02]) & 0x0fff) / 4095.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = 0.0f;

            break;

        case GPU_LUMINANCE16:

            /*  Sixteen bits for luminance component.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit(*((u16bit *) &data[address & 0x02])) / 65535.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = 0.0f;

            break;

        case GPU_LUMINANCE4_ALPHA4:

            /*  Four bits for luminance component, four bits for alpha component.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit(data[address & 0x03] >> 4) / 15.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = f32bit(data[address & 0x03] & 0x0f) / 15.0f;

            break;

        case GPU_LUMINANCE6_ALPHA2:

            /*  Sixs bits for luminance component, two bits for alpha component.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit(data[address & 0x03] >> 2) / 63.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = f32bit(data[address & 0x03] & 0x03) / 3.0f;

            break;

        case GPU_LUMINANCE8_ALPHA8:

            /*  Eight bits for luminance component, eight bits for alpha component.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = 
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = 
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = f32bit(data[address & 0x02]) / 255.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = f32bit(data[(address & 0x02) + 1]) / 255.0f;

            break;

        case GPU_LUMINANCE8_ALPHA8_SIGNED:

            /*  Eight bits for luminance component, eight bits for alpha component.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = 
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = 
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = f32bit(((s8bit *) data)[address & 0x02]) / 127.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = f32bit(((s8bit *) data)[(address & 0x02) + 1]) / 127.0f;

            break;

        case GPU_LUMINANCE12_ALPHA4:

            /*  Twelve bits for luminance component, four bits for alpha component.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit(*((u16bit *) &data[address & 0x02]) >> 4) / 4095.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = f32bit(*((u16bit *) &data[address & 0x02]) & 0x0f) / 15.0f;

            break;

        case GPU_LUMINANCE12_ALPHA12:

            /*  Twelve bits for luminance component, twelve bits for alpha component.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit((*((u32bit *) &data[0]) >> 12) & 0x0fff) / 4095.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = f32bit(*((u32bit *) &data[0]) & 0x0fff) / 4095.0f;

            break;

        case GPU_LUMINANCE16_ALPHA16:

            /*  Sixteen bits for luminance component, Sixteen bits for alpha component.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit(*((u16bit *) &data[0])) / 65535.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = f32bit(*((u16bit *) &data[2])) / 65535.0f;

            break;

        case GPU_INTENSITY8:

            /*  Store 8 bit intensity value in red component.  Set all others to default.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit(data[address & 0x03]) / 255.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = f32bit(data[address & 0x03]) / 255.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = f32bit(data[address & 0x03]) / 255.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = f32bit(data[address & 0x03]) / 255.0f;

            break;

        case GPU_INTENSITY12:

            /*  Store 12 bit intensity value in red component.  Set all others to default.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit(*((u16bit *) &data[address & 0x02]) & 0x0fff) / 4095.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = f32bit(*((u16bit *) &data[address & 0x02]) & 0x0fff) / 4095.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = f32bit(*((u16bit *) &data[address & 0x02]) & 0x0fff) / 4095.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = f32bit(*((u16bit *) &data[address & 0x02]) & 0x0fff) / 4095.0f;

            break;

        case GPU_INTENSITY16:

            /*  Store 16 bit intensity value in red component.  Set all others to default.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit(*((u16bit *) &data[address & 0x02])) / 65535.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = f32bit(*((u16bit *) &data[address & 0x02])) / 65535.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = f32bit(*((u16bit *) &data[address & 0x02])) / 65535.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = f32bit(*((u16bit *) &data[address & 0x02])) / 65535.0f;

            break;

        case GPU_RGB332:

            /*  Three bits for read and green color component, two bits for blue component.  Alpha set to default 1.0f.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit(data[address & 0x03] >> 5) / 7.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = f32bit((data[address & 0x03] >> 3) & 0x07) / 7.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = f32bit(data[address & 0x03] & 0x03) / 3.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = 1.0f;

            break;

        case GPU_RGB444:

            /*  Fuor bits for each color component.  Alpha set to default 1.0f.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit((*((u16bit *) &data[address & 0x02]) >> 8) & 0x0f) / 15.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = f32bit((*((u16bit *) &data[address & 0x02]) >> 4) & 0x0f) / 15.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = f32bit(*((u16bit *) &data[address & 0x02]) & 0x0f) / 15.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = 1.0f;

            break;

        case GPU_RGB555:

            /*  Five bits for each color component.  Alpha set to default 1.0f.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit((*((u16bit *) &data[address & 0x02]) >> 10) & 0x1f) / 31.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = f32bit((*((u16bit *) &data[address & 0x02]) >> 5) & 0x1f) / 31.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = f32bit(*((u16bit *) &data[address & 0x02]) & 0x1f) / 31.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = 1.0f;

            break;

        case GPU_RGB565:

            /*  Five bits for each color component.  Alpha set to default 1.0f.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit(*((u16bit *) &data[address & 0x02]) >> 11) / 31.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = f32bit((*((u16bit *) &data[address & 0x02]) >> 5) & 0x3f) / 31.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = f32bit(*((u16bit *) &data[address & 0x02]) & 0x1f) / 31.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = 1.0f;

            break;

        case GPU_RGB888:

            /*  Eight bits for each color component.  Alpha set to default 1.0f.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit(data[0]) / 255.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = f32bit(data[1]) / 255.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = f32bit(data[2]) / 255.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = 1.0f;

            break;

        case GPU_RGB101010:

            /*  Ten bits for each color component.  Alpha set to default 1.0f.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit((*((u32bit *) &data[0]) >> 20) & 0x03ff) / 1023.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = f32bit((*((u32bit *) &data[0]) >> 10) & 0x03ff) / 1023.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = f32bit(*((u32bit *) &data[0]) & 0x03ff) / 1023.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = 1.0f;

            break;

        case GPU_RGB121212:

            /*  Twelve bits for each color component.  Alpha set to default 1.0f.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit(*((u32bit *) &data[0]) >> 24) / 4095.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = f32bit((*((u32bit *) &data[0]) >> 12) & 0xfff) / 4095.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = f32bit(*((u32bit *) &data[0]) & 0x0fff) / 4095.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = 1.0f;

            break;

        case GPU_RGBA2222:

            /*  Two bits per color and alpha component.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit(data[address & 0x03] >> 6) / 3.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = f32bit((data[address & 0x03] >> 4) & 0x03) / 3.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = f32bit((data[address & 0x03] >> 2) & 0x03) / 3.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = f32bit(data[address & 0x03] & 0x03) / 3.0f;

            break;

        case GPU_RGBA4444:

            /*  Four bits per color and alpha component.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit(data[address & 0x02] >> 4) / 15.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = f32bit(data[address & 0x02] & 0x0f) / 15.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = f32bit(data[(address & 0x02) + 1] >> 4) / 15.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = f32bit(data[(address & 0x02) + 1] & 0x0f) / 15.0f;

            break;

        case GPU_RGBA5551:

            /*  Five bits per color component and 1 bit for alpha component.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit(*((u16bit *) &data[address & 0x02]) >> 11) / 31.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = f32bit((*((u16bit *) &data[address & 0x02]) >> 6) & 0x1f) / 31.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = f32bit((*((u16bit *) &data[address & 0x02]) >> 1) & 0x1f) / 31.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = f32bit(*((u16bit *) &data[address & 0x02]) & 0x01) / 1.0f;

            break;

        case GPU_RGBA8888:

            /*  Eight bits per color and alpha component.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit(data[0]) / 255.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = f32bit(data[1]) / 255.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = f32bit(data[2]) / 255.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = f32bit(data[3]) / 255.0f;

            break;

        case GPU_RGBA1010102:

            /*  Ten bits per color component and two bits for alpha component.  */
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit(*((u32bit *) &data[0]) >> 22) / 1023.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = f32bit((*((u32bit *) &data[0]) >> 12) & 0x3ff) / 1023.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = f32bit((*((u32bit *) &data[0]) >> 2) & 0x3ff) / 1023.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = f32bit(*((u32bit *) &data[0]) & 0x03) / 3.0f;

            break;

        case GPU_R16:
            
            //  Sixteen bits for the red color component.  Green and blue set to default 0.0f and alpha set to default 1.0f.
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit(*((u16bit *) &data[address & 0x02])) / 65535.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = 1.0f;
            
            break;
            
        case GPU_RG16:
        
            //  Sixteen bits for the red and green color components.  Blue set to default 0.0f and alpha set to default 1.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit(((u16bit *) &data[address & 0x02])[0]) / 65535.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = f32bit(((u16bit *) &data[address & 0x02])[1]) / 65535.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = 1.0f;
            
            break;
                        
        case GPU_RGBA16:
        
            //  Sixteen bits for the components.
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = f32bit(((u16bit *) &data[address & 0x02])[0]) / 65535.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = f32bit(((u16bit *) &data[address & 0x02])[1]) / 65535.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = f32bit(((u16bit *) &data[address & 0x02])[2]) / 65535.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = f32bit(((u16bit *) &data[address & 0x02])[3]) / 65535.0f;
            
            break;

        case GPU_R16F:
            
            //  Sixteen bits for the red color component.  Green and blue set to default 0.0f and alpha set to default 1.0f.
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = GPUMath::convertFP16ToFP32(*((f16bit *) &data[address & 0x02]));
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = 1.0f;
            
            break;
            
        case GPU_RG16F:
        
            //  Sixteen bits for the red and green color components.  Blue set to default 0.0f and alpha set to default 1.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = GPUMath::convertFP16ToFP32(((f16bit *) data)[0]);
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = GPUMath::convertFP16ToFP32(((f16bit *) data)[1]);
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = 1.0f;
            
            break;
                        
        case GPU_RGBA16F:
        
            //  Sixteen bits for the components.
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = GPUMath::convertFP16ToFP32(((f16bit *) data)[0]);
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = GPUMath::convertFP16ToFP32(((f16bit *) data)[1]);
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = GPUMath::convertFP16ToFP32(((f16bit *) data)[2]);
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = GPUMath::convertFP16ToFP32(((f16bit *) data)[3]);
            /*textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = GPUMath::convertFP16ToFP32(*((f16bit *) data + 0));
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = GPUMath::convertFP16ToFP32(*((f16bit *) data + 2));
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = GPUMath::convertFP16ToFP32(*((f16bit *) data + 4));
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = GPUMath::convertFP16ToFP32(*((f16bit *) data + 6));*/
            /*textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = GPUMath::convertFP16ToFP32(((f16bit *)  &data[address & 0x02])[0]);
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = GPUMath::convertFP16ToFP32(((f16bit *)  &data[address & 0x02])[1]);
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = GPUMath::convertFP16ToFP32(((f16bit *)  &data[address & 0x02])[2]);
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = GPUMath::convertFP16ToFP32(((f16bit *)  &data[address & 0x02])[3]);*/


             /*
             printf("         Float16 = (%04x, %04x, %04x, %04x)\n",
              ((f16bit *) data)[0], ((f16bit *) data)[1],
              ((f16bit *) data)[2], ((f16bit *) data)[3]);
             printf("         Float32 = (%f, %f, %f, %f)\n",
              textAccess.trilinear[trilinearAccess]->texel[frag][tex][0], textAccess.trilinear[trilinearAccess]->texel[frag][tex][1],
              textAccess.trilinear[trilinearAccess]->texel[frag][tex][2], textAccess.trilinear[trilinearAccess]->texel[frag][tex][3]);
             */
             /*printf("         Float32 = (%f, %f, %f, %f) ret\n",
              GPUMath::convertFP16ToFP32(((f16bit *) data)[0]), GPUMath::convertFP16ToFP32(((f16bit *) data)[1]),
              GPUMath::convertFP16ToFP32(((f16bit *) data)[2]), GPUMath::convertFP16ToFP32(((f16bit *) data)[3]));*/
             /*printf("         Float32 = (%f, %f, %f, %f) ret\n",
              GPUMath::convertFP16ToFP32(*((f16bit *) data) + 0), GPUMath::convertFP16ToFP32(*((f16bit *) data) + 2),
              GPUMath::convertFP16ToFP32(*((f16bit *) data) + 4), GPUMath::convertFP16ToFP32(*((f16bit *) data) + 6));*/
             /*printf("         Float32 = (%f, %f, %f, %f) ret\n",
              GPUMath::convertFP16ToFP32(((f16bit *)  &data[address & 0x02])[0]), GPUMath::convertFP16ToFP32(((f16bit *)  &data[address & 0x02])[1]),
              GPUMath::convertFP16ToFP32(((f16bit *)  &data[address & 0x02])[2]), GPUMath::convertFP16ToFP32(((f16bit *)  &data[address & 0x02])[3]));*/
            
            break;
            
        case GPU_R32F:
        
            //  32 bits for the red color component.  Green and blue set to 0.0f and alpha set to 1.0f.
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = *((f32bit *) data);
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = 1.0f;
            break;
            
        case GPU_RG32F:
        
            //  32 bits for the red and green color components.  Blue set to 0.0f and alpha set to 1.0f.
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = ((f32bit *) data)[0];
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = ((f32bit *) data)[1];
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = 0.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = 1.0f;
            break;
            
        case GPU_RGBA32F:
        
            //  32 bits for the color components.
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = ((f32bit *) data)[0];
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = ((f32bit *) data)[1];
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = ((f32bit *) data)[2];
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = ((f32bit *) data)[3];
            break;
            
        case GPU_DEPTH_COMPONENT24:
            
            //  24-bit depth value in the [0, 1] range.
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = 
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = 
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = f32bit(*((u32bit *) data) & 0x00FFFFFF) / 16777215.0f;
            textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = 1.0f;
            break;

        case GPU_DEPTH_COMPONENT32:
            panic("TextureEmulator", "convertFormat", "Format DEPTH_COMPONENT32 not implemented.");
            break;
            
        default:

            panic("TextureEmulator", "convertFormat", "Unsupported texture format.");
            break;
    }

    //  Check if comparison filter (PCF) is enabled.
    if (textureEnableComparison[textAccess.textUnit])
    {
        //  Perform comparison.
        textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = comparisonFilter(textureComparisonFunction[textAccess.textUnit],
                                                                                      textAccess.reference[frag],
                                                                                      textAccess.trilinear[trilinearAccess]->texel[frag][tex][0]);
        textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = comparisonFilter(textureComparisonFunction[textAccess.textUnit],
                                                                                      textAccess.reference[frag],
                                                                                      textAccess.trilinear[trilinearAccess]->texel[frag][tex][1]);
        textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = comparisonFilter(textureComparisonFunction[textAccess.textUnit],
                                                                                      textAccess.reference[frag],
                                                                                      textAccess.trilinear[trilinearAccess]->texel[frag][tex][2]);
        textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = comparisonFilter(textureComparisonFunction[textAccess.textUnit],
                                                                                      textAccess.reference[frag],
                                                                                      textAccess.trilinear[trilinearAccess]->texel[frag][tex][3]);
    }
    
    //  Check if D3D9 color component read order is enabled for the texture unit.
    if (textD3D9ColorConv[textAccess.textUnit])
    {
        //
        //  The D3D9 color formats are stored in little endian order with the alpha in highest byte:
        //
        //  For example:
        //
        //     D3DFMT_A8R8G8B8 is stored as B G R A
        //     D3DFMT_X8R8G8B8 is stored as B G R X
        //
        
        f32bit red   = textAccess.trilinear[trilinearAccess]->texel[frag][tex][2];
        f32bit green = textAccess.trilinear[trilinearAccess]->texel[frag][tex][1];
        f32bit blue  = textAccess.trilinear[trilinearAccess]->texel[frag][tex][0];
        f32bit alpha = textAccess.trilinear[trilinearAccess]->texel[frag][tex][3];
        
        textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = red;
        textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = green;
        textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = blue;
        textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = alpha;
    }
    
    /*  Check if texture data order must be inverted.  */
    if (textureReverse[textAccess.textUnit])
    {
        f32bit aux;
        aux = textAccess.trilinear[trilinearAccess]->texel[frag][tex][0];
        textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = textAccess.trilinear[trilinearAccess]->texel[frag][tex][3];
        textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = aux;
        aux = textAccess.trilinear[trilinearAccess]->texel[frag][tex][1];
        textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = textAccess.trilinear[trilinearAccess]->texel[frag][tex][2];
        textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = aux;

    }
    
    //  Check if the texture data must be converted from sRGB space to linear space.
    if (textureSRGB[textAccess.textUnit])
    {
        f32bit red   = textAccess.trilinear[trilinearAccess]->texel[frag][tex][0];
        f32bit green = textAccess.trilinear[trilinearAccess]->texel[frag][tex][1];
        f32bit blue  = textAccess.trilinear[trilinearAccess]->texel[frag][tex][2];
        //f32bit alpha = textAccess.trilinear[trilinearAccess]->texel[frag][tex][3];
        
        textAccess.trilinear[trilinearAccess]->texel[frag][tex][0] = LINEAR(red);
        textAccess.trilinear[trilinearAccess]->texel[frag][tex][1] = LINEAR(green);
        textAccess.trilinear[trilinearAccess]->texel[frag][tex][2] = LINEAR(blue);
        //textAccess.trilinear[trilinearAccess]->texel[frag][tex][3] = LINEAR(alpha);
    }

//printf("readData => %f %f %f %f\n", textAccess.texel[frag][tex][0], textAccess.texel[frag][tex][1],
//    textAccess.texel[frag][tex][2], textAccess.texel[frag][tex][3]);
}

//  Performs comparison for PCF filter mode.
f32bit TextureEmulator::comparisonFilter(CompareMode function, f32bit reference, f32bit texel)
{
    f32bit result;
    
    switch(function)
    {
        case GPU_NEVER:        
            result = 0.0f;
            break;
            
        case GPU_ALWAYS:
            result = 1.0f;
            break;
            
        case GPU_LESS:
            result = (texel < reference) ? 1.0f : 0.0f;
            break; 
            
        case GPU_LEQUAL:
            result = (texel <= reference) ? 1.0f : 0.0f;
            break; 

        case GPU_EQUAL:
            result = (texel == reference) ? 1.0f : 0.0f;
            break; 

        case GPU_GEQUAL:
            result = (texel >= reference) ? 1.0f : 0.0f;
            break; 

        case GPU_GREATER:
            result = (texel > reference) ? 1.0f : 0.0f;
            break; 

        case GPU_NOTEQUAL:
            result = (texel != reference) ? 1.0f : 0.0f;
            break; 
            
        default:
            panic("TextureEmulator", "comparisonFilter", "Undefined comparison function.");
            break;
    }

    return result;
}

/*  Converts a quadfloat color into a given format.  */
u32bit TextureEmulator::format(TextureFormat format, QuadFloat color)
{
    u32bit output;

    switch(format)
    {
        case GPU_RGBA8888:

            /*  Convert each color component into an unsigned byte in the output 32 bit color.
                Using little endian model (RGBA when stored in memory, ABGR in the 32 bit integer
                variable).  */

            output = u8bit(color[0] * 255.0f) + (u8bit(color[1] * 255.0f) << 8) +
                (u8bit(color[2] * 255.0f) << 16) + (u8bit(color[3] * 255.0f) << 24);

            break;

        case GPU_LUMINANCE8:
            output = u8bit(color[0] * 255.0f);
            break;

        case GPU_LUMINANCE8_SIGNED:
            output = s8bit(color[0] * 127.0f);
            break;

        case GPU_LUMINANCE8_ALPHA8:
            output = u8bit(color[0] * 255.0f) + (u8bit(color[3] * 255.0f) << 8);
            break;            
            
        case GPU_LUMINANCE8_ALPHA8_SIGNED:
            output = ((s8bit(color[0] * 255.0f)) & 0x00FF) + (s8bit(color[3] * 255.0f) << 8);
            break;            

        default:

            panic("TextureEmulator", "format", "Unsupported texture format.");
            break;
    }

    return output;
}

/*  Decode a S3TC compressed texture using DXT1 encoding for RGB format.  */
void TextureEmulator::decodeBlockDXT1RGB(u8bit *inBuffer, u8bit *outBuffer)
{
    u32bit color0, color1;
    QuadFloat RGBA0, RGBA1;
    QuadFloat decodedColor;
    u32bit code;
    u32bit colorbits;
    u32bit i, j;

    /*  Patch to detect use of non initialized memory.  */
    if (*((u32bit *) inBuffer) == 0xDEADCAFE)
    {
        for(i = 0; i < 16; i++)
        {
            ((u32bit *) outBuffer)[i] = 0xFFFFFFFF;
        }

        return;
    }

    /*  Convert first reference color of the compressed block to RGBA.  */
    color0 = (inBuffer[1] << 8) + inBuffer[0];
    RGBA0[0] = f32bit(color0 >> 11) * (1.0f / 31.0f);
    RGBA0[1] = f32bit((color0 >> 5) & 0x3f) * (1.0f / 63.0f);
    RGBA0[2] = f32bit(color0 & 0x1f) * (1.0f / 31.0f);
    //RGBA0[3] = 1.0f;

    /*  Convert second reference color of the compressed block to RGBA.  */
    color1 = (inBuffer[3] << 8) + inBuffer[2];
    RGBA1[0] = f32bit(color1 >> 11) * (1.0f / 31.0f);
    RGBA1[1] = f32bit((color1 >> 5) & 0x3f) * (1.0f / 63.0f);
    RGBA1[2] = f32bit(color1 & 0x1f) * (1.0f / 31.0f);
    //RGBA1[3] = 1.0f;

    /*  Get the code bits for the color components in the block.  */
    colorbits = inBuffer[4] + u32bit(inBuffer[5] << 8) + u32bit(inBuffer[6] << 16) + u32bit(inBuffer[7] << 24);

    /*  Generate the decoded colors for all the texels in the block.  */
    for(j = 0; j < 4; j++)
    {
        for(i = 0; i < 4; i++)
        {
            /*  Get code for the texel.  */
            //code = (inBuffer[4 + j] >> (i * 2)) & 0x03;
            code = colorbits & 0x03;
            colorbits = colorbits >> 2;

            /*  Determine if transparent or non-transparent encoding must be used.  */
            if (color0 > color1)
            {
                /*  Use non transparent encoding.  */
                nonTransparentS3TCRGB(code, RGBA0, RGBA1, decodedColor);
            }
            else
            {
                /*  Use transparent encoding.  */
                transparentS3TCRGB(code, RGBA0, RGBA1, decodedColor);
            }

            /*  Alpha is always non transparent for RGB format.  */
            decodedColor[3] = 1.0f;

            /*  Store block color in Morton order and 32 bit RGBA format.  */
            ((u32bit *) outBuffer)[GPUMath::morton(2, i, j)] = format(GPU_RGBA8888, decodedColor);
        }
    }
}

/*  Decode a S3TC compressed texture using DXT1 encoding for RGBA format.  */
void TextureEmulator::decodeBlockDXT1RGBA(u8bit *inBuffer, u8bit *outBuffer)
{
    u32bit color0, color1;
    QuadFloat RGBA0, RGBA1;
    QuadFloat decodedColor;
    u32bit code;
    u32bit colorbits;
    u32bit i, j;

    /*  Patch to detect use of non initialized memory.  */
    if (*((u32bit *) inBuffer) == 0xDEADCAFE)
    {
        for(i = 0; i < 16; i++)
        {
            ((u32bit *) outBuffer)[i] = 0xFFFFFFFF;
        }

        return;
    }

    /*  Convert first reference color of the compressed block to RGBA.  */
    color0 = (inBuffer[1] << 8) + inBuffer[0];
    RGBA0[0] = ((f32bit) (color0 >> 11)) * (1.0f / 31.0f);
    RGBA0[1] = ((f32bit) ((color0 >> 5) & 0x3f)) * (1.0f / 63.0f);
    RGBA0[2] = ((f32bit) (color0 & 0x1f)) * (1.0f / 31.0f);
    //RGBA0[3] = 1.0f;

    /*  Convert second reference color of the compressed block to RGBA.  */
    color1 = (inBuffer[3] << 8) + inBuffer[2];
    RGBA1[0] = ((f32bit) (color1 >> 11)) * (1.0f / 31.0f);
    RGBA1[1] = ((f32bit) ((color1 >> 5) & 0x3f)) * (1.0f / 63.0f);
    RGBA1[2] = ((f32bit) (color1 & 0x1f)) * (1.0f / 31.0f);
    //RGBA1[3] = 1.0f;

//printf("RGBA0 = {%f, %f, %f, %f}\n", RGBA0[0], RGBA0[1], RGBA0[2], RGBA0[3]);
//printf("RGBA1 = {%f, %f, %f, %f}\n", RGBA1[0], RGBA1[1], RGBA1[2], RGBA1[3]);

    /*  Get the code bits for the color components in the block.  */
    colorbits = inBuffer[4] + u32bit(inBuffer[5] << 8) + u32bit(inBuffer[6] << 16) + u32bit(inBuffer[7] << 24);

    /*  Generate the decoded colors for all the texels in the block.  */
    for(j = 0; j < 4; j++)
    {
        for(i = 0; i < 4; i++)
        {
            /*  Get code for the texel.  */
            //code = (inBuffer[4 + j] >> (i * 2)) & 0x03;
            code = colorbits & 0x03;
            colorbits = colorbits >> 2;

            /*  Determine if transparent or non-transparent encoding must be used.  */
            if (color0 > color1)
            {
                /*  Use non transparent encoding.  */
                nonTransparentS3TCRGB(code, RGBA0, RGBA1, decodedColor);

                /*  Non transparent alpha.  */
                decodedColor[3] = 1.0f;
            }
            else
            {
                /*  Use transparent encoding.  */
                transparentS3TCRGB(code, RGBA0, RGBA1, decodedColor);

                /*  Patch special non transparent case.  */
                if (code != 0x03)
                    decodedColor[3] = 1.0f;
                else
                    decodedColor[3] = 0.0f;
            }

//printf("Decoded texel (%d, %d) stored as %d\n", i, j, GPUMath::morton(2, i, j));
//printf("Decoded color {%f, %f, %f, %f} -> %x\n", decodedColor[0],
//decodedColor[1], decodedColor[2], decodedColor[3], format(GPU_RGBA8888, decodedColor));
            /*  Store block color in Morton order and 32 bit RGBA format.  */
            ((u32bit *) outBuffer)[GPUMath::morton(2, i, j)] = format(GPU_RGBA8888, decodedColor);
        }
    }
}

/*  Decode a S3TC compressed texture using DXT3 encoding for RGBA format.  */
void TextureEmulator::decodeBlockDXT3RGBA(u8bit *inBuffer, u8bit *outBuffer)
{
    u32bit color0, color1;
    QuadFloat RGBA0, RGBA1;
    QuadFloat decodedColor;
    u32bit code;
    u32bit colorbits;
    u64bit alphabits;
    u32bit i, j;

    /*  Patch to detect use of non initialized memory.  */
    if (((u32bit *) inBuffer)[0] == 0xDEADCAFE)
    {
        for(i = 0; i < 16; i++)
        {
            ((u32bit *) outBuffer)[i] = 0xFFFFFFFF;
        }

        return;
    }

//printf("DXT3C Block: ");
//for(i = 0; i < 16; i++) printf("%02x ", inBuffer[i]);
//printf("\n");

    /*  Convert first reference color of the compressed block to RGBA.  */
    color0 = (inBuffer[9] << 8) + inBuffer[8];
    RGBA0[0] = ((f32bit) (color0 >> 11)) * (1.0f / 31.0f);
    RGBA0[1] = ((f32bit) ((color0 >> 5) & 0x3f)) * (1.0f / 63.0f);
    RGBA0[2] = ((f32bit) (color0 & 0x1f)) * (1.0f / 31.0f);
    //RGBA0[3] = 1.0f;

    /*  Convert second reference color of the compressed block to RGBA.  */
    color1 = (inBuffer[11] << 8) + inBuffer[10];
    RGBA1[0] = ((f32bit) (color1 >> 11)) * (1.0f / 31.0f);
    RGBA1[1] = ((f32bit) ((color1 >> 5) & 0x3f)) * (1.0f / 63.0f);
    RGBA1[2] = ((f32bit) (color1 & 0x1f)) * (1.0f / 31.0f);
    //RGBA1[3] = 1.0f;

    /*  Get the code bits for the color components in the block.  */
    colorbits = inBuffer[12] + (inBuffer[13] << 8) + (inBuffer[14] << 16) + (inBuffer[15] << 24);

    /*  Get the data bits for the alpha components in the block.  */
    alphabits = (u64bit) inBuffer[0] + (((u64bit) inBuffer[1]) << 8) + (((u64bit) inBuffer[2]) << 16) +
        (((u64bit) inBuffer[3]) << 24) + (((u64bit) inBuffer[4]) << 32) + (((u64bit) inBuffer[5]) << 40) +
        (((u64bit) inBuffer[6]) << 48) + (((u64bit) inBuffer[7]) << 56);

    /*  Generate the decoded colors for all the texels in the block.  */
    for(j = 0; j < 4; j++)
    {
        for(i = 0; i < 4; i++)
        {
            /*  Get code for the texel.  */
            //code = (inBuffer[12 + j] >> (i * 2)) & 0x03;
            code = colorbits & 0x03;
            colorbits = colorbits >> 2;

            /*  Decode color using non-transparent encoding.  */
            nonTransparentS3TCRGB(code, RGBA0, RGBA1, decodedColor);

            /*  Decode alpha.  */
            decodedColor[3] = f32bit(alphabits & 0x0f) * (1.0f / 15.0f);
            alphabits = alphabits >> 4;

//printf("-> alphabits %llx\n", alphabits);
//printf("DXT3 => {%d, %d}, decoded color { %f, %f, %f, %f } => %x\n", i, j,
//    decodedColor[0], decodedColor[1], decodedColor[2], decodedColor[3], format(GPU_RGBA8888, decodedColor));

            /*  Store block color in Morton order and 32 bit RGBA format.  */
            ((u32bit *) outBuffer)[GPUMath::morton(2, i, j)] = format(GPU_RGBA8888, decodedColor);
        }
    }
}

/*  Decode a S3TC compressed texture using DXT5 encoding for RGBA format.  */
void TextureEmulator::decodeBlockDXT5RGBA(u8bit *inBuffer, u8bit *outBuffer)
{
    u32bit color0, color1;
    QuadFloat RGBA0, RGBA1;
    float alpha0, alpha1;
    QuadFloat decodedColor;
    u32bit code;
    u32bit alphacode;
    u64bit alphabits;
    u32bit colorbits;
    u32bit i, j;

    /*  Patch to detect use of non initialized memory.  */
    if (((u32bit *) inBuffer)[0] == 0xDEADCAFE)
    {
        for(i = 0; i < 16; i++)
        {
            ((u32bit *) outBuffer)[i] = 0xFFFFFFFF;
        }

        return;
    }

    /*  Convert first reference color of the compressed block to RGBA.  */
    color0 = (inBuffer[9] << 8) + inBuffer[8];
    RGBA0[0] = ((f32bit) (color0 >> 11)) * (1.0f / 31.0f);
    RGBA0[1] = ((f32bit) ((color0 >> 5) & 0x3f)) * (1.0f / 63.0f);
    RGBA0[2] = ((f32bit) (color0 & 0x1f)) * (1.0f / 31.0f);
    //RGBA0[3] = 1.0f;

    /*  Convert second reference color of the compressed block to RGBA.  */
    color1 = (inBuffer[11] << 8) + inBuffer[10];
    RGBA1[0] = ((f32bit) (color1 >> 11)) * (1.0f / 31.0f);
    RGBA1[1] = ((f32bit) ((color1 >> 5) & 0x3f)) * (1.0f / 63.0f);
    RGBA1[2] = ((f32bit) (color1 & 0x1f)) * (1.0f / 31.0f);
    //RGBA1[3] = 1.0f;

    /*  Get the code bits for the color components in the block.  */
    colorbits = inBuffer[12] + (inBuffer[13] << 8) + (inBuffer[14] << 16) + (inBuffer[15] << 24);

    /*  Convert first reference alpha from the compressed block.  */
    alpha0 = f32bit(inBuffer[0]) * (1.0f / 255.0f);

    /*  Convert second reference alpha from the compressed block.  */
    alpha1 = f32bit(inBuffer[1]) * (1.0f / 255.0f);

    /*  Get the code bits for the alpha components in the block.  */
    alphabits = ((u64bit) inBuffer[2]) + (((u64bit) inBuffer[3]) << 8) + (((u64bit) inBuffer[4]) << 16) +
        (((u64bit) inBuffer[5]) << 24) + (((u64bit) inBuffer[6]) << 32) + (((u64bit) inBuffer[7]) << 40);

    /*  Generate the decoded colors for all the texels in the block.  */
    for(j = 0; j < 4; j++)
    {
        for(i = 0; i < 4; i++)
        {
            /*  Get code for the texel.  */
            //code = (inBuffer[12 + j] >> (i * 2)) & 0x03;
            code = colorbits & 0x03;
            colorbits = colorbits >> 2;

            /*  Decode color using non-transparent encoding.  */
            nonTransparentS3TCRGB(code, RGBA0, RGBA1, decodedColor);

            /*  Get the three bit alpha code for the texel in the 4x4 block.  */
            //alphacode = (alphaBits >> (3 * (4 * j + i))) & 0x07;
            alphacode = u32bit(alphabits & 0x07);
            alphabits = alphabits >> 3;

            /*  Decode alpha.  */
            decodedColor[3] = decodeS3TCAlpha(alphacode, alpha0, alpha1);

//printf("DXT5 => {%d, %d} decoded color { %f, %f, %f, %f }\n", i, j, decodedColor[0], decodedColor[1], decodedColor[2], decodedColor[3]);

            /*  Store block color in Morton order and 32 bit RGBA format.  */
            ((u32bit *) outBuffer)[GPUMath::morton(2, i, j)] = format(GPU_RGBA8888, decodedColor);
        }
    }
}

//  Decode a LATC1 compressed texture.
void TextureEmulator::decodeBlockLATC1(u8bit *inBuffer, u8bit *outBuffer)
{
    f32bit luminance0, luminance1;
    QuadFloat decodedColor;
    u64bit luminanceCodes;

    //  Convert first reference luminance from the compressed block.
    luminance0 = f32bit(inBuffer[0]) * (1.0f / 255.0f);

    //  Convert second reference luminance from the compressed block.
    luminance1 = f32bit(inBuffer[1]) * (1.0f / 255.0f);

    //  Get the code bits for the luminance elements in the block.
    luminanceCodes = ((u64bit) inBuffer[2]) + (((u64bit) inBuffer[3]) << 8) + (((u64bit) inBuffer[4]) << 16) +
        (((u64bit) inBuffer[5]) << 24) + (((u64bit) inBuffer[6]) << 32) + (((u64bit) inBuffer[7]) << 40);

    //  Generate the decoded values for all the texels in the block.
    for(u32bit j = 0; j < 4; j++)
    {
        for(u32bit i = 0; i < 4; i++)
        {
            //  Get the three bit luminance code for the texels in the 4x4 block.
            u32bit luminanceCode = u32bit(luminanceCodes & 0x07);
            luminanceCodes = luminanceCodes >> 3;

            //  Decode luminance.
            decodedColor[0] =
            decodedColor[1] =
            decodedColor[2] = decodeS3TCAlpha(luminanceCode, luminance0, luminance1);
            decodedColor[3] = 1.0f;

//printf("LATC1 => {%d, %d} decoded color { %f, %f, %f, %f }\n", i, j, decodedColor[0], decodedColor[1], decodedColor[2], decodedColor[3]);

            //  Store block color in Morton order and 8-bit LUMINANCE format.
            outBuffer[GPUMath::morton(2, i, j)] = u8bit(format(GPU_LUMINANCE8, decodedColor) & 0x00FF);
        }
    }
}

//  Decode a LATC1_SIGNED compressed texture.
void TextureEmulator::decodeBlockLATC1Signed(u8bit *inBuffer, u8bit *outBuffer)
{
    f32bit luminance0, luminance1;
    QuadFloat decodedColor;
    u64bit luminanceCodes;

    //  Convert first reference luminance from the compressed block.
    luminance0 = f32bit(s8bit(inBuffer[0])) * (1.0f / 127.0f);

    //  Convert second reference luminance from the compressed block.
    luminance1 = f32bit(s8bit(inBuffer[1])) * (1.0f / 127.0f);

    //  Get the code bits for the luminance elements in the block.
    luminanceCodes = ((u64bit) inBuffer[2]) + (((u64bit) inBuffer[3]) << 8) + (((u64bit) inBuffer[4]) << 16) +
        (((u64bit) inBuffer[5]) << 24) + (((u64bit) inBuffer[6]) << 32) + (((u64bit) inBuffer[7]) << 40);

    //  Generate the decoded values for all the texels in the block.
    for(u32bit j = 0; j < 4; j++)
    {
        for(u32bit i = 0; i < 4; i++)
        {
            //  Get the three bit luminance code for the texels in the 4x4 block.
            u32bit luminanceCode = u32bit(luminanceCodes & 0x07);
            luminanceCodes = luminanceCodes >> 3;

            //  Decode luminance.
            decodedColor[0] =
            decodedColor[1] =
            decodedColor[2] = decodeS3TCAlpha(luminanceCode, luminance0, luminance1);
            decodedColor[3] = 1.0f;

//printf("LATC1_SIGNED => {%d, %d} decoded color { %f, %f, %f, %f }\n", i, j, decodedColor[0], decodedColor[1], decodedColor[2], decodedColor[3]);

            //  Store block color in Morton order and 8-bit LUMINANCE SIGNED format.
            outBuffer[GPUMath::morton(2, i, j)] = u8bit(format(GPU_LUMINANCE8_SIGNED, decodedColor) & 0x00FF);
        }
    }
}

//  Decode a LATC2 compressed texture.
void TextureEmulator::decodeBlockLATC2(u8bit *inBuffer, u8bit *outBuffer)
{
    f32bit luminance0, luminance1;
    f32bit alpha0, alpha1;
    QuadFloat decodedColor;
    u64bit luminanceCodes;
    u64bit alphaCodes;

    //  Convert first reference luminance from the compressed block.
    luminance0 = f32bit(inBuffer[0]) * (1.0f / 255.0f);

    //  Convert second reference luminance from the compressed block.
    luminance1 = f32bit(inBuffer[1]) * (1.0f / 255.0f);

    //  Get the code bits for the luminance elements in the block.
    luminanceCodes = ((u64bit) inBuffer[2]) + (((u64bit) inBuffer[3]) << 8) + (((u64bit) inBuffer[4]) << 16) +
        (((u64bit) inBuffer[5]) << 24) + (((u64bit) inBuffer[6]) << 32) + (((u64bit) inBuffer[7]) << 40);

    //  Convert first reference alpha from the compressed block.
    alpha0 = f32bit(inBuffer[8]) * (1.0f / 255.0f);

    //  Convert second reference alpha from the compressed block.
    alpha1 = f32bit(inBuffer[9]) * (1.0f / 255.0f);

    //  Get the code bits for the luminance elements in the block.
    alphaCodes = ((u64bit) inBuffer[10]) + (((u64bit) inBuffer[11]) << 8) + (((u64bit) inBuffer[12]) << 16) +
        (((u64bit) inBuffer[13]) << 24) + (((u64bit) inBuffer[14]) << 32) + (((u64bit) inBuffer[15]) << 40);

    //  Generate the decoded values for all the texels in the block.
    for(u32bit j = 0; j < 4; j++)
    {
        for(u32bit i = 0; i < 4; i++)
        {
            //  Get the three bit luminance code for the texels in the 4x4 block.
            u32bit luminanceCode = u32bit(luminanceCodes & 0x07);
            luminanceCodes = luminanceCodes >> 3;

            // Get the three bit alpha code for the texels in the 4x4 block.
            u32bit alphaCode = u32bit(alphaCodes & 0x07);
            alphaCodes = alphaCodes >> 3;
            
            //  Decode luminance and alpha.
            decodedColor[0] =
            decodedColor[1] =
            decodedColor[2] = decodeS3TCAlpha(luminanceCode, luminance0, luminance1);
            decodedColor[3] = decodeS3TCAlpha(alphaCode, alpha0, alpha1);;

//printf("LATC2 => {%d, %d} decoded color { %f, %f, %f, %f }\n", i, j, decodedColor[0], decodedColor[1], decodedColor[2], decodedColor[3]);

            //  Store block color in Morton order and 16-bit LUMINANCE_ALPHA format.
            ((u16bit *) outBuffer)[GPUMath::morton(2, i, j)] = u16bit(format(GPU_LUMINANCE8_ALPHA8, decodedColor) & 0x0000FFFF);
        }
    }
}

//  Decode a LATC2_SIGNED compressed texture.
void TextureEmulator::decodeBlockLATC2Signed(u8bit *inBuffer, u8bit *outBuffer)
{
    f32bit luminance0, luminance1;
    f32bit alpha0, alpha1;
    QuadFloat decodedColor;
    u64bit luminanceCodes;
    u64bit alphaCodes;

    //  Convert first reference luminance from the compressed block.
    luminance0 = f32bit(s32bit(inBuffer[0])) * (1.0f / 127.0f);

    //  Convert second reference luminance from the compressed block.
    luminance1 = f32bit(s32bit(inBuffer[1])) * (1.0f / 127.0f);

    //  Get the code bits for the luminance elements in the block.
    luminanceCodes = ((u64bit) inBuffer[2]) + (((u64bit) inBuffer[3]) << 8) + (((u64bit) inBuffer[4]) << 16) +
        (((u64bit) inBuffer[5]) << 24) + (((u64bit) inBuffer[6]) << 32) + (((u64bit) inBuffer[7]) << 40);

    //  Convert first reference alpha from the compressed block.
    alpha0 = f32bit(s32bit(inBuffer[8])) * (1.0f / 127.0f);

    //  Convert second reference alpha from the compressed block.
    alpha1 = f32bit(s32bit(inBuffer[9])) * (1.0f / 127.0f);

    //  Get the code bits for the luminance elements in the block.
    alphaCodes = ((u64bit) inBuffer[10]) + (((u64bit) inBuffer[11]) << 8) + (((u64bit) inBuffer[12]) << 16) +
        (((u64bit) inBuffer[13]) << 24) + (((u64bit) inBuffer[14]) << 32) + (((u64bit) inBuffer[15]) << 40);

    //  Generate the decoded values for all the texels in the block.
    for(u32bit j = 0; j < 4; j++)
    {
        for(u32bit i = 0; i < 4; i++)
        {
            //  Get the three bit luminance code for the texels in the 4x4 block.
            u32bit luminanceCode = u32bit(luminanceCodes & 0x07);
            luminanceCodes = luminanceCodes >> 3;

            // Get the three bit alpha code for the texels in the 4x4 block.
            u32bit alphaCode = u32bit(alphaCodes & 0x07);
            alphaCodes = alphaCodes >> 3;
            
            //  Decode luminance and alpha.
            decodedColor[0] =
            decodedColor[1] =
            decodedColor[2] = decodeS3TCAlpha(luminanceCode, luminance0, luminance1);
            decodedColor[3] = decodeS3TCAlpha(alphaCode, alpha0, alpha1);;

//printf("LATC2_SIGNED => {%d, %d} decoded color { %f, %f, %f, %f }\n", i, j, decodedColor[0], decodedColor[1], decodedColor[2], decodedColor[3]);

            //  Store block color in Morton order and 16-bit LUMINANCE_ALPHA_SIGNED format.
            ((u16bit *) outBuffer)[GPUMath::morton(2, i, j)] = u16bit(format(GPU_LUMINANCE8_ALPHA8_SIGNED, decodedColor) & 0x0000FFFF);
        }
    }
}

/*  Decodes and selects the proper alpha value for S3TC alpha encoding.  */
f32bit TextureEmulator::decodeS3TCAlpha(u32bit code, f32bit alpha0, f32bit alpha1)
{
    f32bit output;

    /*  Select between the two methods of encoding.  */
    if (alpha0 > alpha1)
    {
        /*  Select encoding.  */
        switch(code)
        {
            case 0x00:

                /*  Use alpha0.  */
                output = alpha0;

                break;

            case 0x01:

                /*  Use alpha1.  */
                output = alpha1;

                break;

            case 0x02:

                /*  Use (6 * alpha0 + 1 * alpha1) / 7.  */
                output = (6.0f * alpha0 + 1.0f * alpha1) / 7.0f;
                break;

            case 0x03:

                /*  Use (5 * alpha0 + 2 * alpha1) / 7.  */
                output = (5.0f * alpha0 + 2.0f * alpha1) / 7.0f;
                break;

            case 0x04:

                /*  Use (4 * alpha0 + 3 * alpha1) / 7.  */
                output = (4.0f * alpha0 + 3.0f * alpha1) / 7.0f;
                break;

            case 0x05:

                /*  Use (3 * alpha0 + 4 * alpha1) / 7.  */
                output = (3.0f * alpha0 + 4.0f * alpha1) / 7.0f;
                break;

            case 0x06:

                /*  Use (2 * alpha0 + 5 * alpha1) / 7.  */
                output = (2.0f * alpha0 + 5.0f * alpha1) / 7.0f;
                break;

            case 0x07:

                /*  Use (1 * alpha0 + 6 * alpha1) / 7.  */
                output = (1.0f * alpha0 + 6.0f * alpha1) / 7.0f;
                break;

            default:
                panic("TextureEmulator", "decodeS3TCAlpha", "Unsupported code.");
        }
    }
    else
    {
        /*  Select encoding.  */
        switch(code)
        {
            case 0x00:

                /*  Use alpha0.  */
                output = alpha0;

                break;

            case 0x01:

                /*  Use alpha1.  */
                output = alpha1;

                break;

            case 0x02:

                /*  Use (4 * alpha0 + 1 * alpha1) / 5.  */
                output = (4.0f * alpha0 + 1.0f * alpha1) / 5.0f;
                break;

            case 0x03:

                /*  Use (3 * alpha0 + 2 * alpha1) / 5.  */
                output = (3.0f * alpha0 + 2.0f * alpha1) / 5.0f;
               break;

            case 0x04:

                /*  Use (2 * alpha0 + 3 * alpha1) / 5.  */
                output = (2.0f * alpha0 + 3.0f * alpha1) / 5.0f;
                break;

            case 0x05:

                /*  Use (1 * alpha0 + 4 * alpha1) / 5.  */
                output = (1.0f * alpha0 + 4.0f * alpha1) / 5.0f;
                break;

            case 0x06:

                /*  Use 0.  */
                output = 0.0f;
                break;

            case 0x07:

                /*  Use 1.  */
                output = 1.0f;
                break;

            default:
                panic("TextureEmulator", "decodeS3TCAlpha", "Unsupported code.");
        }
    }

    return output;
}

/*  Decodes and selects the proper color for non-transparent S3TC color encoding.  */
void TextureEmulator::nonTransparentS3TCRGB(u32bit code, QuadFloat RGB0, QuadFloat RGB1, QuadFloat &output)
{
    /*  Select color for the texel and store.  */
    switch(code)
    {
        case 0x00:

            /*  Use RGB0.  */
            output[0] = RGB0[0];
            output[1] = RGB0[1];
            output[2] = RGB0[2];

            break;

        case 0x01:

            /*  Use RGB1.  */
            output[0] = RGB1[0];
            output[1] = RGB1[1];
            output[2] = RGB1[2];

            break;

        case 0x02:

            /*  Use (2 * RGB0 + RGB1) / 3.  */
            output[0] = (2 * RGB0[0] + RGB1[0]) / 3;
            output[1] = (2 * RGB0[1] + RGB1[1]) / 3;
            output[2] = (2 * RGB0[2] + RGB1[2]) / 3;

            break;

        case 0x03:

            /*  Use (RGB0 + 2 * RGB1) / 3.  */
            output[0] = (RGB0[0] + 2 * RGB1[0]) / 3;
            output[1] = (RGB0[1] + 2 * RGB1[1]) / 3;
            output[2] = (RGB0[2] + 2 * RGB1[2]) / 3;

            break;

        default:
            panic("TextureEmulator", "nonTransparentS3TCRGB", "Unsupported code.");
            break;
    }
}

/*  Decodes and selects the proper color for transparent ST3C color encoding.  */
void TextureEmulator::transparentS3TCRGB(u32bit code, QuadFloat RGB0, QuadFloat RGB1, QuadFloat &output)
{
    /*  Select color for the texel and store.  */
    switch(code)
    {
        case 0x00:

            /*  Use RGB0.  */
            output[0] = RGB0[0];
            output[1] = RGB0[1];
            output[2] = RGB0[2];

            break;

        case 0x01:

            /*  Use RGB1.  */
            output[0] = RGB1[0];
            output[1] = RGB1[1];
            output[2] = RGB1[2];

            break;

        case 0x02:

            /*  Use (RGB0 + RGB1) / 2.  */
            output[0] = (RGB0[0] + RGB1[0]) * 0.5f;
            output[1] = (RGB0[1] + RGB1[1]) * 0.5f;
            output[2] = (RGB0[2] + RGB1[2]) * 0.5f;

            break;

        case 0x03:

            /*  Use BLACK.  */
            output[0] = 0.0f;
            output[1] = 0.0f;
            output[2] = 0.0f;

            break;

        default:
            panic("TextureEmulator", "transparentS3TCRGB", "Unsupported code.");
            break;
    }
}

/*  Decompresses a number of DXT1 RGB blocks.  */
void TextureEmulator::decompressDXT1RGB(u8bit *input, u8bit *output, u32bit size)
{
    u32bit i;
    u32bit numBlocks;

    /*  Determine the number of blocks in the input.  */
    numBlocks = size >> S3TC_DXT1_BLOCK_SIZE_SHIFT;

    /*  Decode the DXT1 RGB blocks.  */
    for(i = 0; i < numBlocks; i++)
    {
        /*  Decode next block.  */
        decodeBlockDXT1RGB(&input[i << S3TC_DXT1_BLOCK_SIZE_SHIFT],
            &output[i << (S3TC_DXT1_BLOCK_SIZE_SHIFT + DXT1_SPACE_SHIFT)]);
    }
}

/*  Decompresses a number of DXT1 RGBA blocks.  */
void TextureEmulator::decompressDXT1RGBA(u8bit *input, u8bit *output, u32bit size)
{
    u32bit i;
    u32bit numBlocks;

    /*  Determine the number of blocks in the input.  */
    numBlocks = size >> S3TC_DXT1_BLOCK_SIZE_SHIFT;

//printf("decompressDXT1RGBA => input data : ");
//for(i = 0; i < size; i++)
//printf("%2x ", input[i]);
//printf("\n");

//printf("decompressDXT1RGBA => numBlocks %d size %d\n", numBlocks, size);

//printf("TxEm > decompressing data %p size %d blocks %d\n", input, size, numBlocks);

    /*  Decode the DXT1 RGB blocks.  */
    for(i = 0; i < numBlocks; i++)
    {
        /*  Decode next block.  */
        decodeBlockDXT1RGBA(&input[i << S3TC_DXT1_BLOCK_SIZE_SHIFT],
            &output[i << (S3TC_DXT1_BLOCK_SIZE_SHIFT + DXT1_SPACE_SHIFT)]);
    }


//printf("decompressDXT1RGBA => output data : ");
//for(i = 0; i < (size << DXT1_SPACE_SHIFT); i++)
//printf("%2x ", output[i]);
//printf("\n");

}

/*  Decompresses a number of DXT3 RGBA blocks.  */
void TextureEmulator::decompressDXT3RGBA(u8bit *input, u8bit *output, u32bit size)
{
    u32bit i;
    u32bit numBlocks;

    /*  Determine the number of blocks in the input.  */
    numBlocks = size >> S3TC_DXT3_DXT5_BLOCK_SIZE_SHIFT;

    /*  Decode the DXT1 RGB blocks.  */
    for(i = 0; i < numBlocks; i++)
    {
        /*  Decode next block.  */
        decodeBlockDXT3RGBA(&input[i << S3TC_DXT3_DXT5_BLOCK_SIZE_SHIFT],
            &output[i << (S3TC_DXT3_DXT5_BLOCK_SIZE_SHIFT + DXT3_DXT5_SPACE_SHIFT)]);
    }
}

/*  Decompresses a number of DXT5 RGBA blocks.  */
void TextureEmulator::decompressDXT5RGBA(u8bit *input, u8bit *output, u32bit size)
{
    u32bit i;
    u32bit numBlocks;

    /*  Determine the number of blocks in the input.  */
    numBlocks = size >> S3TC_DXT3_DXT5_BLOCK_SIZE_SHIFT;

    /*  Decode the DXT1 RGB blocks.  */
    for(i = 0; i < numBlocks; i++)
    {
        /*  Decode next block.  */
        decodeBlockDXT5RGBA(&input[i << S3TC_DXT3_DXT5_BLOCK_SIZE_SHIFT],
            &output[i << (S3TC_DXT3_DXT5_BLOCK_SIZE_SHIFT + DXT3_DXT5_SPACE_SHIFT)]);
    }
}

//  Decompresses a number of LATC1 blocks.
void TextureEmulator::decompressLATC1(u8bit *input, u8bit *output, u32bit size)
{
    u32bit i;
    u32bit numBlocks;

    //  Determine the number of blocks in the input.
    numBlocks = size >> LATC1_BLOCK_SIZE_SHIFT;

    //  Decode the LATC1 blocks.
    for(i = 0; i < numBlocks; i++)
    {
        //  Decode next block.
        decodeBlockLATC1(&input[i << LATC1_BLOCK_SIZE_SHIFT], &output[i << (LATC1_BLOCK_SIZE_SHIFT + LATC1_LATC2_SPACE_SHIFT)]);
    }
}

//  Decompresses a number of LATC1_SIGNED blocks.
void TextureEmulator::decompressLATC1Signed(u8bit *input, u8bit *output, u32bit size)
{
    u32bit i;
    u32bit numBlocks;

    //  Determine the number of blocks in the input.
    numBlocks = size >> LATC1_BLOCK_SIZE_SHIFT;

    //  Decode the LATC1_SIGNED blocks.
    for(i = 0; i < numBlocks; i++)
    {
        //  Decode next block.
        decodeBlockLATC1Signed(&input[i << LATC1_BLOCK_SIZE_SHIFT], &output[i << (LATC1_BLOCK_SIZE_SHIFT + LATC1_LATC2_SPACE_SHIFT)]);
    }
}

//  Decompresses a number of LATC2 blocks.
void TextureEmulator::decompressLATC2(u8bit *input, u8bit *output, u32bit size)
{
    u32bit i;
    u32bit numBlocks;

    //  Determine the number of blocks in the input.
    numBlocks = size >> LATC2_BLOCK_SIZE_SHIFT;

    //  Decode the LATC2 blocks.
    for(i = 0; i < numBlocks; i++)
    {
        //  Decode next block.
        decodeBlockLATC2(&input[i << LATC2_BLOCK_SIZE_SHIFT], &output[i << (LATC2_BLOCK_SIZE_SHIFT + LATC1_LATC2_SPACE_SHIFT)]);
    }
}

//  Decompresses a number of LATC2_SIGNED blocks.
void TextureEmulator::decompressLATC2Signed(u8bit *input, u8bit *output, u32bit size)
{
    u32bit i;
    u32bit numBlocks;

    //  Determine the number of blocks in the input.
    numBlocks = size >> LATC2_BLOCK_SIZE_SHIFT;

    //  Decode the LATC2_SIGNED blocks.
    for(i = 0; i < numBlocks; i++)
    {
        //  Decode next block.
        decodeBlockLATC2Signed(&input[i << LATC2_BLOCK_SIZE_SHIFT], &output[i << (LATC2_BLOCK_SIZE_SHIFT + LATC1_LATC2_SPACE_SHIFT)]);
    }
}


/*  Reset the Texture Emulator internal state.  */
void TextureEmulator::reset()
{
    u32bit i;

    /*  Set default register values.  */
    for (i = 0; i < MAX_TEXTURES; i++)
    {
        textureEnabled[i] = FALSE;
        textureMode[i] = GPU_TEXTURE2D;
        textureWidth[i] = 0;
        textureHeight[i] = 0;
        textureDepth[i] = 0;
        textureWidth2[i] = 0;
        textureHeight2[i] = 0;
        textureDepth2[i] = 0;
        textureBorder[i] = 0;
        textureFormat[i] = GPU_RGBA8888;
        textureReverse[i] = false;
        textD3D9ColorConv[i] = false;
        textD3D9VInvert[i] = false;
        textureCompr[i] = GPU_NO_TEXTURE_COMPRESSION;
        textureBlocking[i] = GPU_TXBLOCK_TEXTURE;
        textBorderColor[i][0] = 0.0f;
        textBorderColor[i][1] = 0.0f;
        textBorderColor[i][2] = 0.0f;
        textBorderColor[i][3] = 1.0f;
        textureWrapS[i] = GPU_TEXT_CLAMP;
        textureWrapT[i] = GPU_TEXT_CLAMP;
        textureWrapR[i] = GPU_TEXT_CLAMP;
        textureNonNormalized[i] = false;
        textureMinFilter[i] = GPU_NEAREST;
        textureMagFilter[i] = GPU_NEAREST;
        textureEnableComparison[i] = false;
        textureComparisonFunction[i] = GPU_LEQUAL;
        textureSRGB[i] = false;
        textureMinLOD[i] = 0.0f;
        textureMaxLOD[i] = 12.0f;
        textureLODBias[i] = 0.0f;
        textureMinLevel[i] = 0;
        textureMaxLevel[i] = 12;
        textureUnitLODBias[i] = 0.0f;
        if (forceMaxAnisotropy)
            maxAnisotropy[i] = confMaxAniso;
        else
            maxAnisotropy[i] = GPU_MIN(u32bit(1), confMaxAniso);
    
        pixelMapperConfigured[i] = false;
    }

    //  Set default values to vertex attribute and stream registers.
    for(i = 0; i < MAX_VERTEX_ATTRIBUTES; i++)
    {
        attributeMap[i] = ST_INACTIVE_ATTRIBUTE;
        attrDefValue[i][0] = 0.0f;
        attrDefValue[i][1] = 0.0f;
        attrDefValue[i][2] = 0.0f;
        attrDefValue[i][3] = 1.0f;
    }
    for(i = 0; i < MAX_STREAM_BUFFERS; i++)
    {
        streamAddress[i] = 0;
        streamStride[i] = 0;
        streamData[i] = SD_U32BIT;
        streamElements[i] = 0;
        streamDataSize[i] = 0;
        streamElementSize[i] = 4;
        d3d9ColorStream[i] = false;
    }
}

/*  Writes a texture emulator register.  */
void TextureEmulator::writeRegister(GPURegister reg, u32bit subReg, GPURegData data)
{
    u32bit textUnit;
    u32bit mipmap;
    u32bit cubemap;

    switch(reg)
    {
        case GPU_TEXTURE_ENABLE:
            /*  Write texture enable register.  */
            textureEnabled[subReg] = data.booleanVal;

            break;

        case GPU_TEXTURE_MODE:
            /*  Write texture mode register.  */
            textureMode[subReg] = data.txMode;

            break;

        case GPU_TEXTURE_ADDRESS:

            /*  WARNING:  As the current simulator only support an index per register we must
                decode the texture unit, mipmap and cubemap image.  To do so the cubemap images of
                a mipmap are stored sequentally and then the mipmaps for a texture unit
                are stored sequentially and then the mipmap addresses of the other texture units
                are stored sequentially.  */

            /*  Calculate texture unit and mipmap level.  */
            textUnit = subReg / (MAX_TEXTURE_SIZE * CUBEMAP_IMAGES);
            mipmap = GPU_MOD(subReg / CUBEMAP_IMAGES, MAX_TEXTURE_SIZE);
            cubemap = GPU_MOD(subReg, CUBEMAP_IMAGES);

            /*  Write texture address register (per mipmap and texture unit).  */
            textureAddress[textUnit][mipmap][cubemap] = data.uintVal;

            break;

        case GPU_TEXTURE_WIDTH:

            /*  Write texture width (first mipmap).  */
            textureWidth[subReg] = data.uintVal;

            break;

        case GPU_TEXTURE_HEIGHT:

            /*  Write texture height (first mipmap).  */
            textureHeight[subReg] = data.uintVal;

            break;

        case GPU_TEXTURE_DEPTH:

            /*  Write texture width (first mipmap).  */
            textureDepth[subReg] = data.uintVal;

            break;

        case GPU_TEXTURE_WIDTH2:

            /*  Write texture width (log of 2 of the first mipmap).  */
            textureWidth2[subReg] = data.uintVal;

            break;

        case GPU_TEXTURE_HEIGHT2:

            /*  Write texture height (log 2 of the first mipmap).  */
            textureHeight2[subReg] = data.uintVal;

            break;

        case GPU_TEXTURE_DEPTH2:

            /*  Write texture depth (log of 2 of the first mipmap).  */
            textureDepth2[subReg] = data.uintVal;

            break;

        case GPU_TEXTURE_BORDER:

            /*  Write texture border register.  */
            textureBorder[subReg] = data.uintVal;

            break;


        case GPU_TEXTURE_FORMAT:

            /*  Write texture format register.  */
            textureFormat[subReg] = data.txFormat;

            //  The pixel mapper requires to be configured in the next access.
            if (textureBlocking[subReg] == GPU_TXBLOCK_FRAMEBUFFER)
                pixelMapperConfigured[subReg] = false;

            break;

        case GPU_TEXTURE_REVERSE:

            /*  Write texture reverse register.  */
            textureReverse[subReg] = data.booleanVal;

            break;

        case GPU_TEXTURE_D3D9_COLOR_CONV:

            /*  Write texture D3D9 color component read order register.  */
            textD3D9ColorConv[subReg] = data.booleanVal;

            break;

        case GPU_TEXTURE_D3D9_V_INV:

            /*  Write texture D3D9 v coordinate invert register.  */
            textD3D9VInvert[subReg] = data.booleanVal;

            break;

        case GPU_TEXTURE_COMPRESSION:

            /*  Write texture compression mode register.  */
            textureCompr[subReg] = data.txCompression;

            break;

        case GPU_TEXTURE_BLOCKING:

            //  Write texture blocking mode register.
            textureBlocking[subReg] = data.txBlocking;

            //  The pixel mapper requires to be configured in the next access.
            if (textureBlocking[subReg] == GPU_TXBLOCK_FRAMEBUFFER)
                pixelMapperConfigured[subReg] = false;
            
            break;

        case GPU_TEXTURE_BORDER_COLOR:

            /*  Write texture border color register.  */
            textBorderColor[subReg][0] = data.qfVal[0];
            textBorderColor[subReg][1] = data.qfVal[1];
            textBorderColor[subReg][2] = data.qfVal[2];
            textBorderColor[subReg][3] = data.qfVal[3];

            break;

        case GPU_TEXTURE_WRAP_S:

            /*  Write texture wrap in s dimension register.  */
            textureWrapS[subReg] = data.txClamp;

            break;

        case GPU_TEXTURE_WRAP_T:

            /*  Write texture wrap in t dimension register.  */
            textureWrapT[subReg] = data.txClamp;

            break;

        case GPU_TEXTURE_WRAP_R:

            /*  Write texture wrap in r dimension register.  */
            textureWrapR[subReg] = data.txClamp;

            break;

        case GPU_TEXTURE_NON_NORMALIZED:

            //  Write texture non-normalized coordinates register.
            textureNonNormalized[subReg] = data.booleanVal;

            break;

        case GPU_TEXTURE_MIN_FILTER:

            /*  Write texture minification filter register.  */
            textureMinFilter[subReg] = data.txFilter;

            break;

        case GPU_TEXTURE_MAG_FILTER:

            /*  Write texture magnification filter register.  */
            textureMagFilter[subReg] = data.txFilter;

            break;

        case GPU_TEXTURE_ENABLE_COMPARISON:
            
            //  Write texture enable comparison register.
            textureEnableComparison[subReg] = data.booleanVal;
            
            break;
            
        case GPU_TEXTURE_COMPARISON_FUNCTION:
        
            //  Write texture comparison function.
            textureComparisonFunction[subReg] = data.compare;
            
            break;
            
        case GPU_TEXTURE_SRGB:
        
            //  Write texture sRGB space to linear space conversion.
            textureSRGB[subReg] = data.booleanVal;
            
            break;

        case GPU_TEXTURE_MIN_LOD:

            /*  Write texture minimum lod register.  */
            textureMinLOD[subReg] = data.f32Val;

            break;

        case GPU_TEXTURE_MAX_LOD:

            /*  Write texture maximum lod register.  */
            textureMaxLOD[subReg] = data.f32Val;

            break;

        case GPU_TEXTURE_LOD_BIAS:

            /*  Write texture lod bias register.  */
            textureLODBias[subReg] = data.f32Val;

            break;

        case GPU_TEXTURE_MIN_LEVEL:

            /*  Write texture minimum mipmap level register.  */
            textureMinLevel[subReg] = data.uintVal;

            break;

        case GPU_TEXTURE_MAX_LEVEL:

            /*  Write texture maximum mipmap level register.  */
            textureMaxLevel[subReg] = data.uintVal;

            break;


        case GPU_TEXT_UNIT_LOD_BIAS:

            /*  Write texture unit lod bias register.  */
            textureUnitLODBias[subReg] = data.f32Val;

            break;

        case GPU_TEXTURE_MAX_ANISOTROPY:

            //  Check if the force maximum anisotropy from the configuration file flag is set.
            if (forceMaxAnisotropy)
            {
                //  Set the max anisotropy register to the configuration file max anisotropy.
                maxAnisotropy[subReg] = confMaxAniso;
            }
            else
            {
                //  Set the max anisotropy register limited to the max anisotropy defined in the
                //  configuration file.
                maxAnisotropy[subReg] = GPU_MIN(data.uintVal, confMaxAniso);
            }

            break;

        case GPU_VERTEX_ATTRIBUTE_MAP :

            //  Write vertex attribute to stream buffer map register.
            attributeMap[subReg] = data.uintVal;
            
            break;
            
        case GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE:

            //  Write the vertex attribute default value register.
            attrDefValue[subReg][0] = data.qfVal[0];
            attrDefValue[subReg][1] = data.qfVal[1];
            attrDefValue[subReg][2] = data.qfVal[2];
            attrDefValue[subReg][3] = data.qfVal[3];

            break;
                
        case GPU_STREAM_ADDRESS:

            //  Write the stream buffer address register.
            streamAddress[subReg] = data.uintVal;
            
            break;
            
        case GPU_STREAM_STRIDE:

            //  Write stream buffer stride register.
            streamStride[subReg] = data.uintVal;

            break;
            
        case GPU_STREAM_DATA:

            //  Write stream buffer data type register.
            streamData[subReg] = data.streamData;

            //  Set the data size for an entry in the stream buffer.
            switch(streamData[subReg])
            {
                case SD_UNORM8:
                case SD_SNORM8:
                case SD_UINT8:
                case SD_SINT8:
                    streamDataSize[subReg] = streamElements[subReg];
                    streamElementSize[subReg] = 1;
                    break;
                case SD_UNORM16:
                case SD_SNORM16:
                case SD_UINT16:
                case SD_SINT16:
                case SD_FLOAT16:
                    streamDataSize[subReg] = streamElements[subReg] * 2;
                    streamElementSize[subReg] = 2;
                    break;
                case SD_UNORM32:
                case SD_SNORM32:
                case SD_UINT32:
                case SD_SINT32:
                case SD_FLOAT32:
                    streamDataSize[subReg] = streamElements[subReg] * 4;
                    streamElementSize[subReg] = 4;
                    break;                    
            }
            
            break;

        case GPU_STREAM_ELEMENTS:

            //  Write stream buffer elements register.
            streamElements[subReg] = data.uintVal;

            //  Set the data size for an entry in the stream buffer.
            switch(streamData[subReg])
            {
                case SD_UNORM8:
                case SD_SNORM8:
                case SD_UINT8:
                case SD_SINT8:
                    streamDataSize[subReg] = streamElements[subReg];
                    break;
                case SD_UNORM16:
                case SD_SNORM16:
                case SD_UINT16:
                case SD_SINT16:
                case SD_FLOAT16:
                    streamDataSize[subReg] = streamElements[subReg] * 2;
                    break;
                case SD_UNORM32:
                case SD_SNORM32:
                case SD_UINT32:
                case SD_SINT32:
                case SD_FLOAT32:
                    streamDataSize[subReg] = streamElements[subReg] * 4;
                    break;                    
            }
            
            break;
            
        case GPU_D3D9_COLOR_STREAM:
            
            //  Write D3D9 color stream mode register.
            d3d9ColorStream[subReg] = data.booleanVal;

            break;
            
        default:

            panic("TextureEmulator", "writeRegister", "Unsupported texture emulator register.");
            break;
    }
}


//  Compute the addresses for the attribute read request based on the index and attribute/stream registers.
void TextureEmulator::index2address(u32bit attribute, TextureAccess &texAccess, u32bit trilinearAccess, u32bit frag, u32bit index)
{
    //  Get the stream associated with the attribute.
    u32bit stream = attributeMap[attribute];
    
//printf("TxEmu::index2address => reading attribute with index %d for frag %d associated stream is %d\n", index, frag, stream);
    //  Check if the attribute is inactive.
    if (stream == ST_INACTIVE_ATTRIBUTE)
    {
//printf("TxEmu::index2address => the attribute is inactive setting address to %x\n", BLACK_TEXEL_ADDRESS); 
        //  Set BLACK_TEXEL_ADDRESS as address as there is no data to read.
        texAccess.trilinear[trilinearAccess]->address[frag][0] = BLACK_TEXEL_ADDRESS;

        //  Set offset and bytes to read for first cache read.
        texAccess.trilinear[trilinearAccess]->attrFirstOffset[frag] = 0;
        texAccess.trilinear[trilinearAccess]->attrFirstSize[frag] = streamDataSize[stream];
        texAccess.trilinear[trilinearAccess]->attrSecondSize[frag] = 0;

        //  No filter loops required for attribute reads.
        texAccess.trilinear[trilinearAccess]->loops[frag] = 1;

        //  Configure a single cache read.
        texAccess.trilinear[trilinearAccess]->texelsLoop[frag] = 1;
        
        //  Set amount of data to read.
        texAccess.texelSize[frag] = 4;

//printf("TxEmu::index2address => loops = %d | texels per loop = %d | texelSize = %d\n",
//texAccess.trilinear[trilinearAccess]->loops[frag],
//texAccess.trilinear[trilinearAccess]->texelsLoop[frag],
//texAccess.texelSize[frag]);

        return;
    }
    
    //  Compute the address of the first byte to read for the attribute read.
    u32bit startAddress = streamAddress[stream] + streamStride[stream] * index;
    
    //  Compute the address of the last byte to read.
    u32bit endAddress = startAddress + streamDataSize[stream] - 1;
    
    //  Check if first byte and the last byte are in the same cache line (based on texture cache block!).
    bool sameLine = (startAddress >> (textCacheBlockDim * 2)) == (endAddress >> (textCacheBlockDim * 2));
    
//printf("TxEmu::index2address => startAddress = %x | endAddress = %x | cacheLine(Log2) = %d | sameLine = %s\n",
//startAddress, endAddress, (textCacheBlockDim * 2), sameLine ? "Si" : "No");

    //  Check how many accesses to the texture cache are required.
    if (sameLine)
    {
        //  Only one read from the cache is required.
        
        //  Set address to read.
        texAccess.trilinear[trilinearAccess]->address[frag][0] = u64bit(startAddress);
        
//printf("TxEmu::index2address => address0 = %x\n", texAccess.trilinear[trilinearAccess]->address[frag][0]);

        //  Set offset and bytes to read for first cache read.
        texAccess.trilinear[trilinearAccess]->attrFirstOffset[frag] = 0;
        texAccess.trilinear[trilinearAccess]->attrFirstSize[frag] = streamDataSize[stream];
        texAccess.trilinear[trilinearAccess]->attrSecondSize[frag] = 0;
        
        //  No filter loops.
        texAccess.trilinear[trilinearAccess]->loops[frag] = 1;

        //  Configure a single cache read.
        texAccess.trilinear[trilinearAccess]->texelsLoop[frag] = 1;
        
        //  Set amount of data to read.
        texAccess.texelSize[frag] = streamDataSize[stream];
    }
    else
    {
        //  Two reads from the cache required.
        
        //  Compute the size of the data to read.  Must be the maximum of the splitted sizes.
        //  The line with less data will be padded.
        u32bit firstLineSize = textCacheBlockSize - startAddress & (textCacheBlockSize - 1);
        u32bit secondLineSize = streamDataSize[stream] - firstLineSize;
        u32bit readSize = GPU_MAX(firstLineSize, secondLineSize);
        
        //  Pad address for the first line.
        if (readSize > firstLineSize)
            startAddress = startAddress - (readSize - firstLineSize);
            
        //  Set the address for the first cache line.
        texAccess.trilinear[trilinearAccess]->address[frag][0] = u64bit(startAddress);
        
        //  Set the address for the second cache line (always at the start of the cache line).
        texAccess.trilinear[trilinearAccess]->address[frag][1] = u64bit(((startAddress >> (textCacheBlockDim * 2)) + 1) << (textCacheBlockDim * 2));

//printf("TxEmu::index2address => address0 = %x\n", texAccess.trilinear[trilinearAccess]->address[frag][0]);
//printf("TxEmu::index2address => address1 = %x\n", texAccess.trilinear[trilinearAccess]->address[frag][1]);

        //  Set offset and bytes to read for first cache read.
        texAccess.trilinear[trilinearAccess]->attrFirstOffset[frag] = readSize - firstLineSize;
        texAccess.trilinear[trilinearAccess]->attrFirstSize[frag] = firstLineSize;
        texAccess.trilinear[trilinearAccess]->attrSecondSize[frag] = secondLineSize;

        //  No filter loops.
        texAccess.trilinear[trilinearAccess]->loops[frag] = 1;
        
        //  Configure to reads from the cache.
        texAccess.trilinear[trilinearAccess]->texelsLoop[frag] = 2;
        
        //  Set amount of data to read.
        texAccess.texelSize[frag] = readSize;
    }

//printf("TxEmu::index2address => loops = %d | texels per loop = %d | texelSize = %d\n",
//texAccess.trilinear[trilinearAccess]->loops[frag],
//texAccess.trilinear[trilinearAccess]->texelsLoop[frag],
//texAccess.texelSize[frag]);
    
    return;
}

//  Converts the read data and fills the attribute.
void TextureEmulator::loadAttribute(TextureAccess &texAccess, u32bit trilinearAccess, u32bit frag)
{
    //  Get the attribute identifier for which to convert/fill data.
    u32bit attr = texAccess.textUnit;
    
    //  Get the pointer to the data read for the attribute.
    u8bit *data = texAccess.trilinear[trilinearAccess]->attrData[frag];
    
    //  Get the stream associated with the attribute.
    u32bit stream = attributeMap[attr];

    //  Get attribute data format.
    StreamData format = streamData[stream];

    //  Get the size in bytes of an element.
    u32bit elemSize = streamElementSize[stream];

    //  First element data is at offset 0.
    u32bit offset = 0;

//printf("TxEmu::loadAttribute => converting data for attribute %d from stream %d to format %d elem size %d elements %d\n",
//attr, stream, format, elemSize, streamElements[stream]);

    //  Determine the number of attribute elements that are being loaded.
    switch(streamElements[stream])
    {
        case 0:
            panic("TextureEmulator", "loadAttribute", "Attribute with zero elements can not request loads!.");
            break;

        case 1:
            //  1-element attribute.
            texAccess.sample[frag][0] = attributeDataConvert(format, &data[offset]);

            //  Load the default value in the undefined elements of the attribute.
            texAccess.sample[frag][1] = attrDefValue[attr][1];
            texAccess.sample[frag][2] = attrDefValue[attr][2];
            texAccess.sample[frag][3] = attrDefValue[attr][3];

            break;

        case 2:
            //  2-element attribute.
            texAccess.sample[frag][0] = attributeDataConvert(format, &data[offset]);
            texAccess.sample[frag][1] = attributeDataConvert(format, &data[offset + elemSize]);

            //  Load the default value in the undefined elements of the attribute.
            texAccess.sample[frag][2] = attrDefValue[attr][2];
            texAccess.sample[frag][3] = attrDefValue[attr][3];

            break;

        case 3:
            //  3-element attribute.
            texAccess.sample[frag][0] = attributeDataConvert(format, &data[offset]);
            texAccess.sample[frag][1] = attributeDataConvert(format, &data[offset + elemSize]);
            texAccess.sample[frag][2] = attributeDataConvert(format, &data[offset + elemSize * 2]);

            //  Load the default value in the undefined element of the attribute.
            texAccess.sample[frag][3] = attrDefValue[attr][3];

            break;

        case 4:
            //  4-element attribute.
            texAccess.sample[frag][0] = attributeDataConvert(format, &data[offset]);
            texAccess.sample[frag][1] = attributeDataConvert(format, &data[offset + elemSize]);
            texAccess.sample[frag][2] = attributeDataConvert(format, &data[offset + elemSize * 2]);
            texAccess.sample[frag][3] = attributeDataConvert(format, &data[offset + elemSize * 3]);
            break;
    }

//printf("TxEmu::loadAttribute -> input data =");
//for(u32bit b = 0; b < (elemSize * streamElements[stream]); b++)
//printf(" %02x", data[b]);
//printf("\n");
//printf("TxEmu::loadAttribute -> converted data = {%f, %f, %f, %f}\n",
//texAccess.sample[frag][0],
//texAccess.sample[frag][1],
//texAccess.sample[frag][2],
//texAccess.sample[frag][3]);

    //  Check if the D3D9 color order for the color components must be used.
    if (d3d9ColorStream[stream])
    {
        //
        //  The D3D9 color formats are stored in little endian order with the alpha in highest byte:
        //
        //  For example:
        //
        //     D3DFMT_A8R8G8B8 is stored as B G R A
        //     D3DFMT_X8R8G8B8 is stored as B G R X
        //
        
        f32bit red = texAccess.sample[frag][2];
        f32bit green = texAccess.sample[frag][1];
        f32bit blue= texAccess.sample[frag][0];
        f32bit alpha = texAccess.sample[frag][3];

        texAccess.sample[frag][0] = red;
        texAccess.sample[frag][1] = green;
        texAccess.sample[frag][2] = blue;
        texAccess.sample[frag][3] = alpha;
    }
}

//  Converts data read for an attribute to the attribute final format (float32).
f32bit TextureEmulator::attributeDataConvert(StreamData format, u8bit *data)
{
    f32bit res;

    switch(format)
    {
        case SD_UNORM8:
            res = f32bit(data[0]) * (1.0f / 255.0f);
            break;
        case SD_SNORM8:
            res = f32bit(*((s8bit *) data)) * (1.0f / 127.0f);
            break;
        case SD_UNORM16:
            res = f32bit(*((u16bit *) data)) * (1.0f / 65535.0f);
            break;
        case SD_SNORM16:
            res = f32bit(*((s16bit *) data)) * (1.0f / 32767.0f);
            break;
        case SD_UNORM32:
            res = f32bit(*((u32bit *) data)) * (1.0f / 4294967295.0f);
            break;
        case SD_SNORM32:
            res = f32bit(*((s32bit *) data)) * (1.0f / 2147483647.0f);
            break;
        case SD_FLOAT16:
            res = GPUMath::convertFP16ToFP32(*((f16bit *) data));
            break;                    
        case SD_FLOAT32:
            res = *((f32bit *) data);
            break;
        case SD_UINT8:
            res = f32bit(data[0]);
            break;
        case SD_SINT8:
            res = f32bit(*((s8bit *) data));
            break;
        case SD_UINT16:
            res = f32bit(*((u16bit *) data));            
            break;
        case SD_SINT16:
            res = f32bit(*((s16bit *) data));            
            break;
        case SD_UINT32:
            res = f32bit(*((u32bit *) data));            
            break;
        case SD_SINT32:
            res = f32bit(*((s32bit *) data));            
            break;
        default:
            panic("TextureEmulator", "attributeDataConvert", "Unsupported stream data format.");
            break;
    }

    return res;
}


/*  Texture Access functions.  */

/*  Texture Access constructor.  */
TextureAccess::TextureAccess(u32bit id, TextureOperation texOp, QuadFloat *coord, f32bit *param, u32bit unit)
{
    u32bit i;

    //  Set texture access identifier (entry in the shader emulator queue).
    accessID = id;

    //  Set texture operation.
    texOperation = texOp;
    
    //  Set texture sampler or attribute stream.
    textUnit = unit;

    //  WARNING:  ONLY WORKS FOR STAMPS OF 4 FRAGMENTS.
    //  Set parameters: coordinates/index and lod/bias.
    for(i = 0; i < STAMP_FRAGMENTS; i++)
    {    
        coordinates[i] = coord[i];
        parameter[i] = parameter[i];
        
        //  NOTE:  A temporal hack for PCF (only 2D textures supported).
        reference[i] = coord[i][2];
    }
}

/*   Texture Access destructor.  */
TextureAccess::~TextureAccess()
{
    u32bit i;
    for(i = 0; i < anisoSamples; i++)
        delete trilinear[i];
}

