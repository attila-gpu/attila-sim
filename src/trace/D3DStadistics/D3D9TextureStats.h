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

#ifndef __D3D9TEXTURESTATS
#define __D3D9TEXTURESTATS

#include <d3d9.h>
#include <list>
#include <fstream>

////////////////////////////////////////////////////////////
////////		D3D9TextureStats Declaration		////////
////////////////////////////////////////////////////////////

typedef enum {
    TUNKW = -1,
	TCOMP = 0,
	T8B   = 1,
	T16B  = 2,
	T24B  = 3,
	T32B  = 4,
	T64B  = 5,
	T128B = 6,
	TVD   = 7,
    TNULL = 8,
} S_TexelSize;

enum {
	DEFAULT = 0,
	MANAGED = 1,
	SYSTEMMEM = 2,
	SCRATCH = 3,
	FORCE_DWORD = 4
};

typedef enum {
    S_D3DFMT_UNKNOWN = -1,

    S_D3DFMT_R8G8B8,
    S_D3DFMT_A8R8G8B8,
    S_D3DFMT_X8R8G8B8,
    S_D3DFMT_R5G6B5,
    S_D3DFMT_X1R5G5B5,
    S_D3DFMT_A1R5G5B5,
    S_D3DFMT_A4R4G4B4,
    S_D3DFMT_R3G3B2,
    S_D3DFMT_A8,
    S_D3DFMT_A8R3G3B2,
    S_D3DFMT_X4R4G4B4,
    S_D3DFMT_A2B10G10R10,
    S_D3DFMT_A8B8G8R8,
    S_D3DFMT_X8B8G8R8,
    S_D3DFMT_G16R16,
    S_D3DFMT_A2R10G10B10,
    S_D3DFMT_A16B16G16R16,

    S_D3DFMT_A8P8,
    S_D3DFMT_P8,

    S_D3DFMT_L8,
    S_D3DFMT_A8L8,
    S_D3DFMT_A4L4,

    S_D3DFMT_V8U8,
    S_D3DFMT_L6V5U5,
    S_D3DFMT_X8L8V8U8,
    S_D3DFMT_Q8W8V8U8,
    S_D3DFMT_V16U16,
    S_D3DFMT_A2W10V10U10,

    S_D3DFMT_UYVY,
    S_D3DFMT_R8G8_B8G8,
    S_D3DFMT_YUY2,
    S_D3DFMT_G8R8_G8B8,
    S_D3DFMT_DXT1,
    S_D3DFMT_DXT2,
    S_D3DFMT_DXT3,
    S_D3DFMT_DXT4,
    S_D3DFMT_DXT5,

    S_D3DFMT_D16_LOCKABLE,
    S_D3DFMT_D32,
    S_D3DFMT_D15S1,
    S_D3DFMT_D24S8,
    S_D3DFMT_D24X8,
    S_D3DFMT_D24X4S4,
    S_D3DFMT_D16,

    S_D3DFMT_D32F_LOCKABLE,
    S_D3DFMT_D24FS8,


    S_D3DFMT_L16,

    S_D3DFMT_VERTEXDATA,
    S_D3DFMT_INDEX16,
    S_D3DFMT_INDEX32,

    S_D3DFMT_Q16W16V16U16,

    S_D3DFMT_MULTI2_ARGB8,

    // Floating point surface formats

    // s10e5 formats (16-bits per channel)
    S_D3DFMT_R16F,
    S_D3DFMT_G16R16F,
    S_D3DFMT_A16B16G16R16F,

    // IEEE s23e8 formats (32-bits per channel)
    S_D3DFMT_R32F,
    S_D3DFMT_G32R32F,
    S_D3DFMT_A32B32G32R32F,

    S_D3DFMT_CxV8U8,


    //  Propietary Formats (added by hand)
    
    S_D3DFMT_ATI2,
    S_D3DFMT_NULL

} S_Format;


class D3D9TextureStats {
	public:
		D3D9TextureStats();
		void addTexture(	UINT Width,	UINT Hetight, UINT Levels, DWORD Usage, 
							D3DFORMAT Format, D3DPOOL Pool, void* pTexture);
		void addTextureStats (D3D9TextureStats* second);
		void setSize(int _size);
		int getSize();
        void printTextureStats (std::ofstream* f);
		void reset();
		double getWidth();
		double getHeight();
		int getTextureType(int type);
		int getTextureType2(int type);
		D3DFORMAT getFormat ();
        int getNoPot2();
        int getPoolType(int type);

		static S_TexelSize getTexelSize(D3DFORMAT format);
		static S_Format getTextureFormat(D3DFORMAT format);
		
	private:
		double Width;
		double Height; 
		int size;
		UINT Levels;
		DWORD Usage; 
		D3DFORMAT Format; 
		D3DPOOL Pool;
		int noPot2;
		int textureType [9];
		int poolType [5];
        std::list<void*> differentTexture;
		int RectTexture;
		int Level13;
		int reuseTexture;
		int acumTextures;
        int noPot2U;
		int textureTypeU[9];
		int poolTypeU[5];
        int textureType2U[61];
};

#endif