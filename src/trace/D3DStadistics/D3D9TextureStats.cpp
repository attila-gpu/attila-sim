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

#include "D3D9TextureStats.h"
#include "D3D9State.h"

using namespace std;

D3D9TextureStats::D3D9TextureStats() {

	Width = 0;
	Height = 0;
	Levels = 0;
	Usage = 0;
	noPot2 = 0;
	RectTexture = 0;
	Level13 = 0;
	size = 0;
	poolType[DEFAULT] = 0;
	poolType[MANAGED] = 0;
	poolType[SYSTEMMEM] = 0;
	poolType[SCRATCH] = 0;
	poolType[FORCE_DWORD] = 0;

	for (int i = 0; i<9; i++) 
		textureType[i] = 0;
	
	acumTextures = 0;
	reuseTexture = 0;

	noPot2U = 0;

	poolTypeU[DEFAULT] = 0;
	poolTypeU[MANAGED] = 0;
	poolTypeU[SYSTEMMEM] = 0;
	poolTypeU[SCRATCH] = 0;
	poolTypeU[FORCE_DWORD] = 0;

	for (int i = 0; i < 9; i++) 
		textureTypeU[i] = 0;

    for (int i = 0; i < 61; i++)
        textureType2U[i] = 0;

}

void D3D9TextureStats::reset(){

	Width = 0;
	Height = 0;
	Levels = 0;
	Usage = 0;
	noPot2 = 0;
	RectTexture = 0;
	Level13 = 0;
	size = 0;
	poolType[DEFAULT] = 0;
	poolType[MANAGED] = 0;
	poolType[SYSTEMMEM] = 0;
	poolType[SCRATCH] = 0;
	poolType[FORCE_DWORD] = 0;

	for(int i = 0; i<9; i++) 
		textureType[i] = 0;
	
	acumTextures = 0;
	differentTexture.clear();

	noPot2U = 0;

	poolTypeU[DEFAULT] = 0;
	poolTypeU[MANAGED] = 0;
	poolTypeU[SYSTEMMEM] = 0;
	poolTypeU[SCRATCH] = 0;
	poolTypeU[FORCE_DWORD] = 0;

	for(int i = 0; i<9; i++) 
		textureTypeU[i] = 0;

    for (int i = 0; i < 61; i++)
        textureType2U[i] = 0;

}

void D3D9TextureStats::setSize(int _size) {
	size = _size;
}

void D3D9TextureStats::addTexture(UINT _Width,	UINT _Height, UINT _Levels, DWORD _Usage, 
										D3DFORMAT _Format, D3DPOOL _Pool, void* pTexture) {

	Width = _Width;
	Height = _Height;
	Levels = _Levels;
	Usage = _Usage;
	Format = _Format;
	Pool = _Pool;

	switch (_Pool) {
		case D3DPOOL_DEFAULT: poolType[DEFAULT]++; break;
		case D3DPOOL_MANAGED: poolType[MANAGED]++; break;
		case D3DPOOL_SYSTEMMEM: poolType[SYSTEMMEM]++; break;
		case D3DPOOL_SCRATCH: poolType[SCRATCH]++; break;
		case D3DPOOL_FORCE_DWORD: poolType[FORCE_DWORD]++; break;
	}

	differentTexture.push_back(pTexture);

	int pot;
	if (_Width != _Height) RectTexture++;

	for (pot = _Width; pot > 1; pot = pot / 2)
		if (pot % 2 != 0) break;

	if (pot == 1) {
		for (pot = _Height; pot > 1; pot = pot / 2)
			if (pot % 2 != 0) break;
	}

	if (pot != 1) noPot2++;

	if (_Levels == 13) Level13++;

	textureType[getTexelSize(_Format)]++;

	textureType2U[getTextureFormat(_Format)]++;

	acumTextures = 1;

}

S_TexelSize D3D9TextureStats::getTexelSize(D3DFORMAT format) {

	switch (format) {
		case D3DFMT_D16_LOCKABLE:
			return T16B;
			break;
		
		case D3DFMT_D32:
			return T32B;
			break;
		
		case D3DFMT_D15S1:
			return T16B;
			break;
		
		case D3DFMT_D24S8:
			return T32B;
			break;
		
		case D3DFMT_D24X8:
			return T32B;
			break;
		
		case D3DFMT_D24X4S4:
			return T32B;
			break;
		
		case D3DFMT_D32F_LOCKABLE:
			return T32B;
			break;
		
		case D3DFMT_D24FS8:
			return T32B;
			break;
		
		case D3DFMT_D16:
			return T16B;
			break;
		
		case D3DFMT_VERTEXDATA:
			return TVD;
			break;
		
		case D3DFMT_INDEX16:
			return T16B;
			break;
		
		case D3DFMT_INDEX32:
			return T32B;
			break;
		
		case D3DFMT_DXT1:
			return TCOMP;
			break;
		
		case D3DFMT_DXT2:
			return TCOMP;
			break;
		
		case D3DFMT_DXT3:
			return TCOMP;
			break;
		
		case D3DFMT_DXT4:
			return TCOMP;
			break;
		
		case D3DFMT_DXT5:
			return TCOMP;
			break;
		
		case D3DFMT_R16F:
			return T16B;
			break;
		
		case D3DFMT_G16R16F:
			return T32B;
			break;
		
		case D3DFMT_A16B16G16R16F:
			return T64B;
			break;
		
		case D3DFMT_R32F:
			return T32B;
			break;
		
		case D3DFMT_G32R32F:
			return T64B;
			break;
		
		case D3DFMT_A32B32G32R32F:
			return T128B;
			break;
		
		case D3DFMT_L6V5U5:
			return T16B;
			break;
		
		case D3DFMT_X8L8V8U8:
			return T32B;
			break;
		
		case D3DFMT_A2W10V10U10:
			return T32B;
			break;
		
		case D3DFMT_V8U8:
			return T16B;
			break;
		
		case D3DFMT_Q8W8V8U8:
			return T32B;
			break;
		
		case D3DFMT_V16U16:
			return T32B;
			break;
		
		case D3DFMT_Q16W16V16U16:
			return T64B;
			break;
		
		case D3DFMT_CxV8U8:
			return T16B;
			break;
		
		case D3DFMT_R8G8B8:
			return T24B;
			break;
		
		case D3DFMT_A8R8G8B8:
			return T32B;
			break;
		
		case D3DFMT_X8R8G8B8:
			return T32B;
			break;
		
		case D3DFMT_R5G6B5:
			return T16B;
			break;
		
		case D3DFMT_X1R5G5B5:
			return T16B;
			break;
		
		case D3DFMT_A1R5G5B5:
			return T16B;
			break;
		
		case D3DFMT_A4R4G4B4:
			return T16B;
			break;
		
		case D3DFMT_R3G3B2:
			return T8B;
			break;
		
		case D3DFMT_A8:
			return T8B;
			break;
		
		case D3DFMT_A8R3G3B2:
			return T16B;
			break;
		
		case D3DFMT_X4R4G4B4:
			return T16B;
			break;
		
		case D3DFMT_A2B10G10R10:
			return T32B;
			break;
		
		case D3DFMT_A8B8G8R8:
			return T32B;
			break;
		
		case D3DFMT_X8B8G8R8:
			return T32B;
			break;
		
		case D3DFMT_G16R16:
			return T32B;
			break;
		
		case D3DFMT_A2R10G10B10:
			return T32B;
			break;
		
		case D3DFMT_A16B16G16R16:
			return T64B;
			break;
		
		case D3DFMT_A8P8:
			return T16B;
			break;
		
		case D3DFMT_P8:
			return T8B;
			break;
		
		case D3DFMT_L8:
			return T8B;
			break;
		
		case D3DFMT_L16:
			return T16B;
			break;
		
		case D3DFMT_A8L8:
			return T16B;
			break;
		
		case D3DFMT_A4L4:
			return T8B;
			break;

        case MAKEFOURCC('U', 'Y', 'V', 'Y'):
            return T32B;
            break;

        case MAKEFOURCC('R', 'G', 'B', 'G'):
            return T32B;
            break;

        case MAKEFOURCC('Y', 'U', 'Y', '2'):
            return T32B;
            break;

        case MAKEFOURCC('G', 'R', 'G', 'B'):
            return T32B;
            break;

        case MAKEFOURCC('M', 'E',' T', '1'):
            return T32B;
            break;

        case MAKEFOURCC('A', 'T', 'I', '2'):
            return TCOMP;
            break;

        case MAKEFOURCC('N', 'U', 'L', 'L'):
            return TNULL;
            break;
	}

	return TUNKW;

}

S_Format D3D9TextureStats::getTextureFormat(D3DFORMAT format) {
    
	switch (format) {
		case D3DFMT_D16_LOCKABLE:
			return S_D3DFMT_D16_LOCKABLE;
			break;
		
		case D3DFMT_D32:
			return S_D3DFMT_D32;
			break;
		
		case D3DFMT_D15S1:
			return S_D3DFMT_D15S1;
			break;
		
		case D3DFMT_D24S8:
			return S_D3DFMT_D24S8;
			break;
		
		case D3DFMT_D24X8:
			return S_D3DFMT_D24X8;
			break;
		
		case D3DFMT_D24X4S4:
			return S_D3DFMT_D24X4S4;
			break;
		
		case D3DFMT_D32F_LOCKABLE:
			return S_D3DFMT_D32F_LOCKABLE;
			break;
		
		case D3DFMT_D24FS8:
			return S_D3DFMT_D24FS8;
			break;
		
		case D3DFMT_D16:
			return S_D3DFMT_D16;
			break;
		
		case D3DFMT_VERTEXDATA:
			return S_D3DFMT_VERTEXDATA;
			break;
		
		case D3DFMT_INDEX16:
			return S_D3DFMT_INDEX16;
			break;
		
		case D3DFMT_INDEX32:
			return S_D3DFMT_INDEX32;
			break;
		
		case D3DFMT_DXT1:
			return S_D3DFMT_DXT1;
			break;
		
		case D3DFMT_DXT2:
			return S_D3DFMT_DXT2;
			break;
		
		case D3DFMT_DXT3:
			return S_D3DFMT_DXT3;
			break;
		
		case D3DFMT_DXT4:
			return S_D3DFMT_DXT4;
			break;
		
		case D3DFMT_DXT5:
			return S_D3DFMT_DXT5;
			break;
		
		case D3DFMT_R16F:
			return S_D3DFMT_R16F;
			break;
		
		case D3DFMT_G16R16F:
			return S_D3DFMT_G16R16F;
			break;
		
		case D3DFMT_A16B16G16R16F:
			return S_D3DFMT_A16B16G16R16F;
			break;
		
		case D3DFMT_R32F:
			return S_D3DFMT_R32F;
			break;
		
		case D3DFMT_G32R32F:
			return S_D3DFMT_G32R32F;
			break;
		
		case D3DFMT_A32B32G32R32F:
			return S_D3DFMT_A32B32G32R32F;
			break;
		
		case D3DFMT_L6V5U5:
			return S_D3DFMT_L6V5U5;
			break;
		
		case D3DFMT_X8L8V8U8:
			return S_D3DFMT_X8L8V8U8;
			break;
		
		case D3DFMT_A2W10V10U10:
			return S_D3DFMT_A2W10V10U10;
			break;
		
		case D3DFMT_V8U8:
			return S_D3DFMT_V8U8;
			break;
		
		case D3DFMT_Q8W8V8U8:
			return S_D3DFMT_Q8W8V8U8;
			break;
		
		case D3DFMT_V16U16:
			return S_D3DFMT_V16U16;
			break;
		
		case D3DFMT_Q16W16V16U16:
			return S_D3DFMT_Q16W16V16U16;
			break;
		
		case D3DFMT_CxV8U8:
			return S_D3DFMT_CxV8U8;
			break;
		
		case D3DFMT_R8G8B8:
			return S_D3DFMT_R8G8B8;
			break;
		
		case D3DFMT_A8R8G8B8:
			return S_D3DFMT_A8R8G8B8;
			break;
		
		case D3DFMT_X8R8G8B8:
			return S_D3DFMT_X8R8G8B8;
			break;
		
		case D3DFMT_R5G6B5:
			return S_D3DFMT_R5G6B5;
			break;
		
		case D3DFMT_X1R5G5B5:
			return S_D3DFMT_X1R5G5B5;
			break;
		
		case D3DFMT_A1R5G5B5:
			return S_D3DFMT_A1R5G5B5;
			break;
		
		case D3DFMT_A4R4G4B4:
			return S_D3DFMT_A4R4G4B4;
			break;
		
		case D3DFMT_R3G3B2:
			return S_D3DFMT_R3G3B2;
			break;
		
		case D3DFMT_A8:
			return S_D3DFMT_A8;
			break;
		
		case D3DFMT_A8R3G3B2:
			return S_D3DFMT_A8R3G3B2;
			break;
		
		case D3DFMT_X4R4G4B4:
			return S_D3DFMT_X4R4G4B4;
			break;
		
		case D3DFMT_A2B10G10R10:
			return S_D3DFMT_A2B10G10R10;
			break;
		
		case D3DFMT_A8B8G8R8:
			return S_D3DFMT_A8B8G8R8;
			break;
		
		case D3DFMT_X8B8G8R8:
			return S_D3DFMT_X8B8G8R8;
			break;
		
		case D3DFMT_G16R16:
			return S_D3DFMT_G16R16;
			break;
		
		case D3DFMT_A2R10G10B10:
			return S_D3DFMT_A2R10G10B10;
			break;
		
		case D3DFMT_A16B16G16R16:
			return S_D3DFMT_A16B16G16R16;
			break;
		
		case D3DFMT_A8P8:
			return S_D3DFMT_A8P8;
			break;
		
		case D3DFMT_P8:
			return S_D3DFMT_P8;
			break;
		
		case D3DFMT_L8:
			return S_D3DFMT_L8;
			break;
		
		case D3DFMT_L16:
			return S_D3DFMT_L16;
			break;
		
		case D3DFMT_A8L8:
			return S_D3DFMT_A8L8;
			break;
		
		case D3DFMT_A4L4:
			return S_D3DFMT_A4L4;
			break;

        case MAKEFOURCC('U', 'Y', 'V', 'Y'):
            return S_D3DFMT_UYVY;
            break;

        case MAKEFOURCC('R', 'G', 'B', 'G'):
            return S_D3DFMT_R8G8_B8G8;
            break;

        case MAKEFOURCC('Y', 'U', 'Y', '2'):
            return S_D3DFMT_YUY2;
            break;

        case MAKEFOURCC('G', 'R', 'G', 'B'):
            return S_D3DFMT_G8R8_G8B8;
            break;

        case MAKEFOURCC('M', 'E', 'T', '1'):
            return S_D3DFMT_MULTI2_ARGB8;
            break;

        case MAKEFOURCC('A', 'T', 'I', '2'):
            return S_D3DFMT_ATI2;
            break;

        case MAKEFOURCC('N', 'U', 'L', 'L'):
            return S_D3DFMT_NULL;
            break;
	}

	return S_D3DFMT_UNKNOWN;

}

int D3D9TextureStats::getSize() {
	return size;
}

void D3D9TextureStats::addTextureStats (D3D9TextureStats* second) {

	bool found = false;
	
	if((acumTextures + second->acumTextures)!=0) {
		Width = (double)((Width * acumTextures) + (second->Width * second->acumTextures)) / (double)(acumTextures + second->acumTextures);
		Height = (double)((Height * acumTextures) + (second->Height * second->acumTextures)) / (double)(acumTextures + second->acumTextures);
	
		textureType[0] += second->textureType[0];
		textureType[1] += second->textureType[1];
		textureType[2] += second->textureType[2];
		textureType[3] += second->textureType[3];
		textureType[4] += second->textureType[4];
		textureType[5] += second->textureType[5];
		textureType[6] += second->textureType[6];
		textureType[7] += second->textureType[7];
		textureType[8] += second->textureType[8];

		noPot2 += second->noPot2;
		RectTexture += second->RectTexture;
		Level13 += second->Level13;
		
		
		poolType[DEFAULT] += second->poolType[DEFAULT];
		poolType[MANAGED] += second->poolType[MANAGED];
		poolType[SYSTEMMEM] += second->poolType[SYSTEMMEM];
		poolType[SCRATCH] += second->poolType[SCRATCH];
		poolType[FORCE_DWORD] += second->poolType[FORCE_DWORD];

		list<void *>::iterator it;

		list <void *> aux;
		for ( it=(second->differentTexture).begin() ; it != (second->differentTexture).end(); it++ )
			aux.push_back(*it);

		int auxT, auxT2;
		if(acumTextures == 0) reuseTexture = 0;
		differentTexture.merge(aux);
		auxT = differentTexture.size();
		differentTexture.unique();
		auxT2 = differentTexture.size();
		reuseTexture = reuseTexture + auxT - auxT2;
		acumTextures += second->acumTextures;

		list<D3D9TextureStats *> auxo = D3D9State::instance().getDifferentTextureStats(differentTexture);
		list<D3D9TextureStats *> :: iterator ito;

		size = 0;
        noPot2U = 0;

        for (int i = 0; i < 9; i++)
            textureTypeU[i] = 0;

        for (int i = 0; i < 5; i++)
            poolTypeU[i] = 0;

        for (int i = 0; i < 61; i++)
            textureType2U[i] = 0;

        for (ito = auxo.begin(); ito != auxo.end(); ito++) {
			size += (*ito)->getSize();
            noPot2U += (*ito)->getNoPot2();
            for (int i = 0; i < 9; i++)
                textureTypeU[i] += (*ito)->getTextureType(i);
            for (int i = 0; i < 5; i++)
                poolTypeU[i] += (*ito)->getPoolType(i);
            for (int i = 0; i < 61; i++)
                textureType2U[i] += (*ito)->getTextureType2(i);
        }
	}
}

void D3D9TextureStats::printTextureStats (ofstream* f) {

	/**f << acumTextures << ";";
	*f << Width << ";";
	*f << Height << ";";
	*f << noPot2 << ";";
	*f << RectTexture << ";";
	*f << Level13 << ";";
	*f << textureType[0] << ";";
	*f << textureType[1] << ";";
	*f << textureType[2] << ";";
	*f << textureType[3] << ";";
	*f << textureType[4] << ";";
	*f << textureType[5] << ";";
	*f << textureType[6] << ";";
	*f << textureType[7] << ";";*/
	*f << differentTexture.size() << ";";
	/**f << poolType[DEFAULT] << ";";
	*f << poolType[MANAGED] << ";";
	*f << poolType[SYSTEMMEM] << ";";
	*f << poolType[SCRATCH] << ";";
	*f << poolType[FORCE_DWORD] << ";";*/
	*f << reuseTexture << ";";
    //-----------------------------
	*f << size << ";";
    *f << noPot2U << ";";
	*f << textureTypeU[0] << ";";
	*f << textureTypeU[1] << ";";
	*f << textureTypeU[2] << ";";
	*f << textureTypeU[3] << ";";
	*f << textureTypeU[4] << ";";
	*f << textureTypeU[5] << ";";
	*f << textureTypeU[6] << ";";
	*f << textureTypeU[7] << ";";
	*f << textureTypeU[8] << ";";
	*f << poolTypeU[DEFAULT] << ";";
	*f << poolTypeU[MANAGED] << ";";
	*f << poolTypeU[SYSTEMMEM] << ";";
	*f << poolTypeU[SCRATCH] << ";";
	//*f << poolTypeU[FORCE_DWORD] << ";";
    for (int i = 0; i < 61; i++)
        *f << textureType2U[i] << ";";
}

double D3D9TextureStats::getWidth() {
	return Width;
}
double D3D9TextureStats::getHeight() {
	return Height;
}

int D3D9TextureStats::getTextureType(int type) {
	return textureType[type];
}

int D3D9TextureStats::getTextureType2(int type) {
	return textureType2U[type];
}

D3DFORMAT D3D9TextureStats::getFormat () {
	return Format;
}

int D3D9TextureStats::getNoPot2() {
    return noPot2;
}

int D3D9TextureStats::getPoolType(int type) {
    return poolType[type];
}
