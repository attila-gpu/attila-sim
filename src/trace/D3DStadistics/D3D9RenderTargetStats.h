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

#ifndef __D3D9RENDERTARGETSTATS
#define __D3D9RENDERTARGETSTATS

#include <d3d9.h>
#include <list>
#include <fstream>

#include "D3D9TextureStats.h"

class D3D9RenderTargetStats {

	public:
		D3D9RenderTargetStats ();
		void addRenderTarget(	UINT _Width,UINT _Height,D3DFORMAT _Format,D3DMULTISAMPLE_TYPE _MultiSample,
								DWORD _MultisampleQuality, BOOL _Lockable, IDirect3DSurface9* oip_ppSurface, /*IDirect3DSurface9** _ppSurface, */
								HANDLE* _pSharedHandle);
		void addRenderTargetStats(D3D9RenderTargetStats* second);
        void printRenderTargetStats (std::ofstream* f);
		void setSize(int _size);
		int getSize();
		int getNumDiffRenderTargetTexture();
        void setTextureIdent(unsigned int ti);
        unsigned int getTextureIdent();
        int getRenderTargetType(int i);
		void reset();
		void copy (D3D9TextureStats* second, IDirect3DSurface9* surface);
		void setDesc(D3DSURFACE_DESC* pDesc, IDirect3DSurface9* surface);

        void setBackBuffer(bool bb);
        bool isBackBuffer();

        unsigned int getWidth();
        unsigned int getHeight();

	private:
		UINT width;
		UINT height;
		D3DFORMAT format;
		D3DMULTISAMPLE_TYPE multiSample;
		DWORD multisampleQuality;
		BOOL lockable;
		int renderTargetType[9];
        int renderTargetType2U[61];
		int numRenderTargetTexture;
		int numDiffRenderTargetTexture;
		int size;
		int acumRenderTargets;
        std::list<IDirect3DSurface9*> differentRenderTarget;
        bool backBuffer;

        unsigned int textureIdent;
};

class D3D9RenderTargetOp {

public:
    D3D9RenderTargetOp(unsigned int i);
    void addTextOp(unsigned int t);
    void clear();
    void partialClear();
    void save(std::ofstream* f);
    std::string getString();
    std::string getExtraDescription();
    unsigned int getRT();
    unsigned int getDraws();
    std::list<unsigned int> getOpList();
    bool isRT(unsigned int rt);
    void depthRT(bool b);
    void setStretchRect(bool b);
    bool isStretchRect();
    void addDraw(unsigned int d);
    void addNumPixelsDrawn(UINT64 num);
    UINT64 getPixelsDrawn();
    UINT64 getSize();
    void setColorWriteEnable(bool e);
    bool getColorWriteEnable();
    unsigned int getClears();
    bool isZST();
    void setClearNode(bool c);
    bool getClearNode();
    void fullSR(bool fsr);
    bool getFullSR();

private:
    unsigned int ident;
    unsigned int draws;
    unsigned int numClears;
    unsigned int numPartialClears;
    UINT64 numPixelsDrawn;
    bool dRT;
    bool stretchrect;
    bool colorWriteEnable;
    bool clearNode;
    bool fullStretchRect;
    std::list<unsigned int> textIdentOp;

};

#endif