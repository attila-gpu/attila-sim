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

#include "D3D9State.h"
#include "D3D9Info.h"
#include <vector>

#define VERTEX_CACHE_1_SIZE 16
#define VERTEX_CACHE_2_SIZE 32
#define VERTEX_CACHE_3_SIZE 48
//#define VERTEX_CACHE_4_SIZE 65536

//#define SAVE_SHADERS

using namespace std;

D3D9State &D3D9State::instance() {
	static D3D9State inst;
	return inst;
}

D3D9State::D3D9State() {
	nextId = 1;
	lastApplyStateBlock = 0;
    rtOp = new D3D9RenderTargetOp(settedRenderTargets[0]);
    dsOp = NULL;
    lastRTorigSR = 0;
}

int D3D9State::getNextId() {
	return nextId;
}

void D3D9State::addShader(IDirect3DVertexShader9 *sh, DWORD *function) {

	D3D9ShaderStats* aux = new D3D9ShaderStats;
#ifdef SAVE_SHADERS
    aux->saveShader(function, nextId);
#endif
	aux->analyze(sh, function);
	addressesToIdentifiers[sh] = nextId;
	identifiersToAddresses[nextId] = sh;
	shaderStats[nextId] = aux;
	nextId ++;
}

void D3D9State::addShader(IDirect3DPixelShader9 *sh, DWORD *function) {

	D3D9ShaderStats* aux = new D3D9ShaderStats;
#ifdef SAVE_SHADERS
    aux->saveShader(function, nextId);
#endif
	aux->analyze(sh, function);
	addressesToIdentifiers[sh] = nextId;
	identifiersToAddresses[nextId] = sh;
	shaderStats[nextId] = aux;
	nextId ++;

}


void D3D9State::setVertexShader(IDirect3DVertexShader9 *sh) {

	if (insideBeginEnd) {
		current.vertexShaderSetted = true;
		current.vertexShader = sh;
	}
	else {
		actual.vertexShaderSetted = true;
		actual.vertexShader = sh;
	}

}

void D3D9State::setPixelShader(IDirect3DPixelShader9 *sh) {

	if (insideBeginEnd) {
		current.pixelShaderSetted = true;
		current.pixelShader = sh;
	}
	else {
		actual.pixelShaderSetted = true;
		actual.pixelShader = sh;
	}

}

void D3D9State::setAlphaTest(bool e) {

	if (insideBeginEnd) {
		current.alphaTestSetted = true;
		current.alphaTestEnabled = e;
	}
	else {
		actual.alphaTestSetted = true;
		actual.alphaTestEnabled = e;
	}

}

void D3D9State::setZWriteEnable(bool z) {

	if (insideBeginEnd) {
		current.zWriteEnableSetted = true;
		current.zWriteEnable = z;
	}
	else {
		actual.zWriteEnableSetted = true;
		actual.zWriteEnable = z;
	}
}

void D3D9State::setColorWriteEnable(DWORD color) {

	if (insideBeginEnd) {
		current.colorWriteEnableSetted = true;
		current.rEnable = ((color & D3DCOLORWRITEENABLE_RED)   != 0);
		current.gEnable = ((color & D3DCOLORWRITEENABLE_GREEN) != 0);
		current.bEnable = ((color & D3DCOLORWRITEENABLE_BLUE)  != 0);
		current.aEnable = ((color & D3DCOLORWRITEENABLE_ALPHA) != 0);
	}
	else {
		actual.colorWriteEnableSetted = true;
		actual.rEnable = ((color & D3DCOLORWRITEENABLE_RED)   != 0);
		actual.gEnable = ((color & D3DCOLORWRITEENABLE_GREEN) != 0);
		actual.bEnable = ((color & D3DCOLORWRITEENABLE_BLUE)  != 0);
		actual.aEnable = ((color & D3DCOLORWRITEENABLE_ALPHA) != 0);
	}

}

void D3D9State::addTexture(UINT Width,	UINT Height, UINT Levels, DWORD Usage, 
										D3DFORMAT Format, D3DPOOL Pool,  IDirect3DTexture9* pTexture,
										HANDLE* pSharedHandle){

	D3D9TextureStats* aux = new D3D9TextureStats ();

	aux->addTexture(Width, Height, Levels, Usage, Format, Pool, pTexture);
	aux->setSize(D3D9Info::instance().getTextureSize(Width, Height, Levels, Format));

	addressesToIdentifiers[pTexture] = nextId;
	identifiersToAddresses[nextId] = pTexture;
	textureStats[nextId] = aux;
	nextId ++;
}

void D3D9State::addVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, 
										D3DFORMAT Format, D3DPOOL Pool,  IDirect3DVolumeTexture9* pTexture,
										HANDLE* pSharedHandle){

	D3D9TextureStats* aux = new D3D9TextureStats ();

	aux->addTexture(Width, Height, Levels, Usage, Format, Pool, pTexture);
	aux->setSize(D3D9Info::instance().getVolumeTextureSize(Width, Height, Depth, Levels, Format));

	addressesToIdentifiers[pTexture] = nextId;
	identifiersToAddresses[nextId] = pTexture;
	textureStats[nextId] = aux;
	nextId ++;
}

void D3D9State::addCubeTexture (UINT EdgeLength, UINT Levels, DWORD Usage,
											D3DFORMAT Format, D3DPOOL Pool,	IDirect3DCubeTexture9 * pCubeTexture,
											HANDLE* pSharedHandle){

	D3D9TextureStats* aux = new D3D9TextureStats ();

	aux->addTexture(EdgeLength, EdgeLength, Levels, Usage, Format, Pool, pCubeTexture);
	aux->setSize(D3D9Info::instance().getCubeTextureSize(EdgeLength, Levels, Format));

	addressesToIdentifiers[pCubeTexture] = nextId;
	identifiersToAddresses[nextId] = pCubeTexture;
	textureStats[nextId] = aux;
	nextId ++;
}

void D3D9State::setTexture(DWORD sampler, IDirect3DBaseTexture9 *tex) {

	if (insideBeginEnd) {
		current.samplerStatus[sampler].texture = tex;
		current.samplerStatus[sampler].textureSetted = true;
	} else {
		actual.samplerStatus[sampler].texture = tex;
		actual.samplerStatus[sampler].textureSetted = true;
	}
}

void D3D9State::addRenderTarget(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,
								BOOL Lockable, IDirect3DSurface9* oip_ppSurface, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {

	D3D9RenderTargetStats* aux = new D3D9RenderTargetStats ();

	aux->addRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, oip_ppSurface, /*ppSurface,*/ pSharedHandle);
	aux->setSize(D3D9Info::instance().getTextureSize(Width, Height, 1, Format));

	addressesToIdentifiers[oip_ppSurface] = nextId;
	identifiersToAddresses[nextId] = oip_ppSurface;
	renderTargetStats[nextId] = aux;
	nextId++;
}

void D3D9State::addVertexDeclaration(IDirect3DVertexDeclaration9* sip_pDecl) {

    if (insideBeginEnd) {
        current.numAttrib = 0;

        if (sip_pDecl != NULL)
            sip_pDecl->GetDeclaration( NULL, &(current.numAttrib));

        current.numAttrib--;
        current.numAttribSetted = true;
    }
    else {
        actual.numAttrib = 0;

        if (sip_pDecl != NULL)
            sip_pDecl->GetDeclaration( NULL, &(actual.numAttrib));

        actual.numAttrib--;
        actual.numAttribSetted = true;
    }

}

void D3D9State::addCreateVertexBuffer(IDirect3DVertexBuffer9* sip_ppVertexBuffer, DWORD sv_FVF) {

	int tmpfvfNumAtributes = 0;


    if (sv_FVF != 0) {

	    if ((sv_FVF & D3DFVF_DIFFUSE) != 0) {
		    tmpfvfNumAtributes++;
	    }

    	
	    if ((sv_FVF & D3DFVF_NORMAL) != 0) {
		    tmpfvfNumAtributes++;
	    }

    	
	    if ((sv_FVF & D3DFVF_PSIZE) != 0) {
		    tmpfvfNumAtributes++;
	    }

    	
	    if ((sv_FVF & D3DFVF_SPECULAR) != 0) {
		    tmpfvfNumAtributes++;
	    }

    	
	    if ((sv_FVF & D3DFVF_XYZ) != 0) {
		    tmpfvfNumAtributes++;
	    }

    	
	    if ((sv_FVF & D3DFVF_XYZRHW) != 0) {
		    tmpfvfNumAtributes++;
	    }

    	
	    if ((sv_FVF & D3DFVF_XYZW) != 0) {
		    tmpfvfNumAtributes++;
	    }

	    if ((sv_FVF & D3DFVF_XYZB1) != 0) {
		    tmpfvfNumAtributes++;
	    }

	    if ((sv_FVF & D3DFVF_XYZB2) != 0) {
		    tmpfvfNumAtributes++;
	    }

	    if ((sv_FVF & D3DFVF_XYZB3) != 0) {
		    tmpfvfNumAtributes++;
	    }

	    if ((sv_FVF & D3DFVF_XYZB4) != 0) {
		    tmpfvfNumAtributes++;
	    }

	    if ((sv_FVF & D3DFVF_XYZB5) != 0) {
		    tmpfvfNumAtributes++;
	    }

	    if ((sv_FVF & D3DFVF_TEX0) != 0) {
		    tmpfvfNumAtributes++;
	    }

	    if ((sv_FVF & D3DFVF_TEX1) != 0) {
		    tmpfvfNumAtributes++;
	    }

	    if ((sv_FVF & D3DFVF_TEX2) != 0) {
		    tmpfvfNumAtributes++;
	    }

	    if ((sv_FVF & D3DFVF_TEX3) != 0) {
		    tmpfvfNumAtributes++;
	    }

	    if ((sv_FVF & D3DFVF_TEX4) != 0) {
		    tmpfvfNumAtributes++;
	    }

	    if ((sv_FVF & D3DFVF_TEX5) != 0) {
		    tmpfvfNumAtributes++;
	    }

	    if ((sv_FVF & D3DFVF_TEX6) != 0) {
		    tmpfvfNumAtributes++;
	    }

	    if ((sv_FVF & D3DFVF_TEX7) != 0) {
		    tmpfvfNumAtributes++;
	    }

	    if ((sv_FVF & D3DFVF_TEX8) != 0) {
		    tmpfvfNumAtributes++;
	    }

    }
    
	addressesToIdentifiers[sip_ppVertexBuffer] = nextId;
	identifiersToAddresses[nextId] = sip_ppVertexBuffer;
    streamerStats[nextId].numAttrib = tmpfvfNumAtributes;
	nextId++;
}

void D3D9State::addSetStreamSource(UINT sv_StreamNumber, IDirect3DVertexBuffer9* sip_pStreamData ,UINT sv_Stride) {
    settedStreamers[sv_StreamNumber] = sip_pStreamData;
    streamerStats[addressesToIdentifiers[sip_pStreamData]].size = sv_Stride;

    if (insideBeginEnd) {
        ofstream f;
        f.open("ibeLog.txt", ios::app);
        f << "!! Stream: " << sv_StreamNumber << " Inside Begin End\n";
        f.close();
    }
}


void D3D9State::addCreateIndexBuffer(void* oip_ppIndexBuffer, D3DFORMAT ov_Format, UINT Length) {

	addressesToIdentifiers[oip_ppIndexBuffer] = nextId;
	identifiersToAddresses[nextId] = oip_ppIndexBuffer;
    indexBufferStats[nextId].format = ov_Format;
    indexBufferStats[nextId].buffer.setSize(Length);
    indexBufferStats[nextId].OffLock = 0;
    indexBufferStats[nextId].SizeLock = 0;
	nextId++;

}

void D3D9State::setIndexBuffer(void* oip_pIndexData) {
    settedIndexBuffer = addressesToIdentifiers[oip_pIndexData];

    if (insideBeginEnd) {
        ofstream f;
        f.open("ibeLog.txt", ios::app);
        f << "!! Index Buffer Inside Begin End\n";
        f.close();
    }
}

void D3D9State::setLockIndexBuffer(void* oip_This, UINT OffsetToLock, UINT SizeToLock) {
    indexBufferStats[addressesToIdentifiers[oip_This]].OffLock = OffsetToLock;
    indexBufferStats[addressesToIdentifiers[oip_This]].SizeLock = SizeToLock;
}

void D3D9State::setUnlockIndexBuffer(void* oip_This, D3DBuffer* buffer) {

    unsigned char* dataSrc;
    unsigned char* dataDest;

    indexBufferStats[addressesToIdentifiers[oip_This]].buffer.lock((void**)&dataDest);
    buffer->lock((void**)&dataSrc);

    dataDest += indexBufferStats[addressesToIdentifiers[oip_This]].OffLock;

    if (indexBufferStats[addressesToIdentifiers[oip_This]].OffLock == 0 &&
        indexBufferStats[addressesToIdentifiers[oip_This]].SizeLock == 0) {
        for (int i = 0; i < buffer->getSize(); i++) {
            *dataDest = *dataSrc;
            dataSrc++;
            dataDest++;
        }
    }
    else {
        for (int i = 0; i < indexBufferStats[addressesToIdentifiers[oip_This]].SizeLock; i++) {
            *dataDest = *dataSrc;
            dataSrc++;
            dataDest++;
        }
    }

    buffer->unlock();
    indexBufferStats[addressesToIdentifiers[oip_This]].buffer.unlock();

}

void D3D9State::getNumIndicesUP(UINT numVertex, int* numVertices1, int* numVertices2, int* numVertices3, int* numVertices4, void* indexBuffer, D3DFORMAT format) {

    if (format == D3DFMT_INDEX16) {

        UINT16* pdata;
        UINT16* enddata;

        *numVertices1 = 0;
        std::vector<bool> indexes1(65536, false);
        std::list<UINT16> cachedIndexes1;
        *numVertices2 = 0;
        std::vector<bool> indexes2(65536, false);
        std::list<UINT16> cachedIndexes2;
        *numVertices3 = 0;
        std::vector<bool> indexes3(65536, false);
        std::list<UINT16> cachedIndexes3;
        *numVertices4 = 0;
        std::vector<bool> indexes4(65536, false);
        std::list<UINT16> cachedIndexes4;

        pdata = (UINT16*)indexBuffer;
        enddata = pdata;
        enddata += numVertex;

        while (pdata != enddata) {

            // Cache 1
            if (indexes1[*pdata] == false) {
                // Miss
                if (cachedIndexes1.size() == VERTEX_CACHE_1_SIZE) {
                    // The vertex cache is full
                    indexes1[cachedIndexes1.front()] = false;
                    cachedIndexes1.pop_front();
                }

                (*numVertices1)++;
                indexes1[*pdata] = true;
                cachedIndexes1.push_back(*pdata);
            }
            else {
                // Hit
                cachedIndexes1.remove(*pdata);
                cachedIndexes1.push_back(*pdata);
            }

            // Cache 2
            if (indexes2[*pdata] == false) {
                // Miss
                if (cachedIndexes2.size() == VERTEX_CACHE_2_SIZE) {
                    // The vertex cache is full
                    indexes2[cachedIndexes2.front()] = false;
                    cachedIndexes2.pop_front();
                }

                (*numVertices2)++;
                indexes2[*pdata] = true;
                cachedIndexes2.push_back(*pdata);
            }
            else {
                // Hit
                cachedIndexes2.remove(*pdata);
                cachedIndexes2.push_back(*pdata);
            }

            // Cache 3
            if (indexes3[*pdata] == false) {
                // Miss
                if (cachedIndexes3.size() == VERTEX_CACHE_3_SIZE) {
                    // The vertex cache is full
                    indexes3[cachedIndexes3.front()] = false;
                    cachedIndexes3.pop_front();
                }

                (*numVertices3)++;
                indexes3[*pdata] = true;
                cachedIndexes3.push_back(*pdata);
            }
            else {
                // Hit
                cachedIndexes3.remove(*pdata);
                cachedIndexes3.push_back(*pdata);
            }

            // Cache 4 (Inf. Cache)
            if (indexes4[*pdata] == false) {
                // Miss
                /*if (cachedIndexes4.size() == VERTEX_CACHE_4_SIZE) {
                    // The vertex cache is full
                    indexes4[cachedIndexes4.front()] = false;
                    cachedIndexes4.pop_front();
                }*/

                (*numVertices4)++;
                indexes4[*pdata] = true;
                //cachedIndexes4.push_back(*pdata);
            }
            /*else {
                // Hit
                cachedIndexes4.remove(*pdata);
                cachedIndexes4.push_back(*pdata);
            }*/

            pdata++;
        }

    }
    else if (format == D3DFMT_INDEX32) {
        std::list <UINT32> indexList;
        UINT32* pdata;
        UINT32* enddata;

        *numVertices1 = 0;
        std::map<UINT32, bool> indexes1;
        std::list<UINT32> cachedIndexes1;
        *numVertices2 = 0;
        std::map<UINT32, bool> indexes2;
        std::list<UINT32> cachedIndexes2;
        *numVertices3 = 0;
        std::map<UINT32, bool> indexes3;
        std::list<UINT32> cachedIndexes3;
        *numVertices4 = 0;
        std::map<UINT32, bool> indexes4;
        std::list<UINT32> cachedIndexes4;

        /*ofstream f;
        f.open("i32Log.txt", ios::app);
        f << "!! 32 bits index\n";
        f.close();*/

        pdata = (UINT32*)indexBuffer;
        enddata = pdata;
        enddata += numVertex;

        while (pdata != enddata) {

            // Cache 1
            if (indexes1[*pdata] == false) {
                // Miss
                if (cachedIndexes1.size() == VERTEX_CACHE_1_SIZE) {
                    // The vertex cache is full
                    indexes1[cachedIndexes1.front()] = false;
                    cachedIndexes1.pop_front();
                }

                (*numVertices1)++;
                indexes1[*pdata] = true;
                cachedIndexes1.push_back(*pdata);
            }
            else {
                // Hit
                cachedIndexes1.remove(*pdata);
                cachedIndexes1.push_back(*pdata);
            }

            // Cache 2
            if (indexes2[*pdata] == false) {
                // Miss
                if (cachedIndexes2.size() == VERTEX_CACHE_2_SIZE) {
                    // The vertex cache is full
                    indexes2[cachedIndexes2.front()] = false;
                    cachedIndexes2.pop_front();
                }

                (*numVertices2)++;
                indexes2[*pdata] = true;
                cachedIndexes2.push_back(*pdata);
            }
            else {
                // Hit
                cachedIndexes2.remove(*pdata);
                cachedIndexes2.push_back(*pdata);
            }

            // Cache 3
            if (indexes3[*pdata] == false) {
                // Miss
                if (cachedIndexes3.size() == VERTEX_CACHE_3_SIZE) {
                    // The vertex cache is full
                    indexes3[cachedIndexes3.front()] = false;
                    cachedIndexes3.pop_front();
                }

                (*numVertices3)++;
                indexes3[*pdata] = true;
                cachedIndexes3.push_back(*pdata);
            }
            else {
                // Hit
                cachedIndexes3.remove(*pdata);
                cachedIndexes3.push_back(*pdata);
            }

            // Cache 4 (Inf. Cache)
            if (indexes4[*pdata] == false) {
                // Miss
                /*if (cachedIndexes4.size() == VERTEX_CACHE_4_SIZE) {
                    // The vertex cache is full
                    indexes4[cachedIndexes4.front()] = false;
                    cachedIndexes4.pop_front();
                }*/

                (*numVertices4)++;
                indexes4[*pdata] = true;
                //cachedIndexes4.push_back(*pdata);
            }
            /*else {
                // Hit
                cachedIndexes4.remove(*pdata);
                cachedIndexes4.push_back(*pdata);
            }*/

            pdata++;
        }

        /*while (pdata != enddata) {
            indexList.push_back(*pdata);
            pdata++;
        }

        indexList.sort();
        indexList.unique();
        //return indexList.size();
        *numVertices4 = indexList.size();*/
    }

    //return 0;
}

void D3D9State::getNumIndices(UINT StartIndex, UINT numVertex, int* numVertices1, int* numVertices2, int* numVertices3, int* numVertices4) {

    if (indexBufferStats[settedIndexBuffer].format == D3DFMT_INDEX16) {
        /*ofstream fl;
        fl.open("cacheLog.txt", ios::app);
        fl << "-c16: " << *numVertices1 << " c32: " << *numVertices2 << " c48: " << *numVertices3 << " cInf: " << *numVertices4 << "\n";
        fl.close();*/

        UINT16* pdata;
        UINT16* enddata;

        *numVertices1 = 0;
        std::vector<bool> indexes1(65536, false);
        std::list<UINT16> cachedIndexes1;
        *numVertices2 = 0;
        std::vector<bool> indexes2(65536, false);
        std::list<UINT16> cachedIndexes2;
        *numVertices3 = 0;
        std::vector<bool> indexes3(65536, false);
        std::list<UINT16> cachedIndexes3;
        *numVertices4 = 0;
        std::vector<bool> indexes4(65536, false);
        std::list<UINT16> cachedIndexes4;

        indexBufferStats[settedIndexBuffer].buffer.lock((void**)&pdata);
        pdata += StartIndex;
        enddata = pdata;
        enddata += numVertex;

        while (pdata != enddata) {

            // Cache 1
            if (indexes1[*pdata] == false) {
                // Miss
                if (cachedIndexes1.size() == VERTEX_CACHE_1_SIZE) {
                    // The vertex cache is full
                    indexes1[cachedIndexes1.front()] = false;
                    cachedIndexes1.pop_front();
                }

                (*numVertices1)++;
                indexes1[*pdata] = true;
                cachedIndexes1.push_back(*pdata);
            }
            else {
                // Hit
                cachedIndexes1.remove(*pdata);
                cachedIndexes1.push_back(*pdata);
            }

            // Cache 2
            if (indexes2[*pdata] == false) {
                // Miss
                if (cachedIndexes2.size() == VERTEX_CACHE_2_SIZE) {
                    // The vertex cache is full
                    indexes2[cachedIndexes2.front()] = false;
                    cachedIndexes2.pop_front();
                }

                (*numVertices2)++;
                indexes2[*pdata] = true;
                cachedIndexes2.push_back(*pdata);
            }
            else {
                // Hit
                cachedIndexes2.remove(*pdata);
                cachedIndexes2.push_back(*pdata);
            }

            // Cache 3
            if (indexes3[*pdata] == false) {
                // Miss
                if (cachedIndexes3.size() == VERTEX_CACHE_3_SIZE) {
                    // The vertex cache is full
                    indexes3[cachedIndexes3.front()] = false;
                    cachedIndexes3.pop_front();
                }

                (*numVertices3)++;
                indexes3[*pdata] = true;
                cachedIndexes3.push_back(*pdata);
            }
            else {
                // Hit
                cachedIndexes3.remove(*pdata);
                cachedIndexes3.push_back(*pdata);
            }

            // Cache 4 (Inf. Cache)
            if (indexes4[*pdata] == false) {
                // Miss
                /*if (cachedIndexes4.size() == VERTEX_CACHE_4_SIZE) {
                    // The vertex cache is full
                    indexes4[cachedIndexes4.front()] = false;
                    cachedIndexes4.pop_front();
                }*/

                (*numVertices4)++;
                indexes4[*pdata] = true;
                //cachedIndexes4.push_back(*pdata);
            }
            /*else {
                // Hit
                cachedIndexes4.remove(*pdata);
                cachedIndexes4.push_back(*pdata);
            }*/

            pdata++;
        }

        indexBufferStats[settedIndexBuffer].buffer.unlock();
        //return numVertices;

        /*ofstream fl;
        fl.open("cacheLog.txt", ios::app);
        fl << "-c16: " << *numVertices1 << " c32: " << *numVertices2 << " c48: " << *numVertices3 << " cInf: " << *numVertices4 << "\n";
        fl.close();*/
    }
    else if (indexBufferStats[settedIndexBuffer].format == D3DFMT_INDEX32) {
        std::list <UINT32> indexList;
        UINT32* pdata;
        UINT32* enddata;

        *numVertices1 = 0;
        std::map<UINT32, bool> indexes1;
        std::list<UINT32> cachedIndexes1;
        *numVertices2 = 0;
        std::map<UINT32, bool> indexes2;
        std::list<UINT32> cachedIndexes2;
        *numVertices3 = 0;
        std::map<UINT32, bool> indexes3;
        std::list<UINT32> cachedIndexes3;
        *numVertices4 = 0;
        std::map<UINT32, bool> indexes4;
        std::list<UINT32> cachedIndexes4;

        /*ofstream f;
        f.open("i32Log.txt", ios::app);
        f << "!! 32 bits index\n";
        f.close();*/

        indexBufferStats[settedIndexBuffer].buffer.lock((void**)&pdata);
        pdata += StartIndex;
        enddata = pdata;
        enddata += numVertex;

        while (pdata != enddata) {

            // Cache 1
            if (indexes1[*pdata] == false) {
                // Miss
                if (cachedIndexes1.size() == VERTEX_CACHE_1_SIZE) {
                    // The vertex cache is full
                    indexes1[cachedIndexes1.front()] = false;
                    cachedIndexes1.pop_front();
                }

                (*numVertices1)++;
                indexes1[*pdata] = true;
                cachedIndexes1.push_back(*pdata);
            }
            else {
                // Hit
                cachedIndexes1.remove(*pdata);
                cachedIndexes1.push_back(*pdata);
            }

            // Cache 2
            if (indexes2[*pdata] == false) {
                // Miss
                if (cachedIndexes2.size() == VERTEX_CACHE_2_SIZE) {
                    // The vertex cache is full
                    indexes2[cachedIndexes2.front()] = false;
                    cachedIndexes2.pop_front();
                }

                (*numVertices2)++;
                indexes2[*pdata] = true;
                cachedIndexes2.push_back(*pdata);
            }
            else {
                // Hit
                cachedIndexes2.remove(*pdata);
                cachedIndexes2.push_back(*pdata);
            }

            // Cache 3
            if (indexes3[*pdata] == false) {
                // Miss
                if (cachedIndexes3.size() == VERTEX_CACHE_3_SIZE) {
                    // The vertex cache is full
                    indexes3[cachedIndexes3.front()] = false;
                    cachedIndexes3.pop_front();
                }

                (*numVertices3)++;
                indexes3[*pdata] = true;
                cachedIndexes3.push_back(*pdata);
            }
            else {
                // Hit
                cachedIndexes3.remove(*pdata);
                cachedIndexes3.push_back(*pdata);
            }

            // Cache 4 (Inf. Cache)
            if (indexes4[*pdata] == false) {
                // Miss
                /*if (cachedIndexes4.size() == VERTEX_CACHE_4_SIZE) {
                    // The vertex cache is full
                    indexes4[cachedIndexes4.front()] = false;
                    cachedIndexes4.pop_front();
                }*/

                (*numVertices4)++;
                indexes4[*pdata] = true;
                //cachedIndexes4.push_back(*pdata);
            }
            /*else {
                // Hit
                cachedIndexes4.remove(*pdata);
                cachedIndexes4.push_back(*pdata);
            }*/

            pdata++;
        }

        indexBufferStats[settedIndexBuffer].buffer.unlock();

        /*while (pdata != enddata) {
            indexList.push_back(*pdata);
            pdata++;
        }

        indexBufferStats[settedIndexBuffer].buffer.unlock();
        indexList.sort();
        indexList.unique();
        //return indexList.size();
        *numVertices4 = indexList.size();*/
    }

    //return 0;
}

void D3D9State::setRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9 * pRenderTarget ) {

	if (pRenderTarget == NULL)
		settedRenderTargets.erase(RenderTargetIndex);
	else
		settedRenderTargets[RenderTargetIndex] = addressesToIdentifiers[pRenderTarget];
}


void D3D9State::setDepthStencilSurface(IDirect3DSurface9 * pRenderTarget ) {

	if (pRenderTarget == NULL)
		setDepthStencil = 0;
	else
		setDepthStencil = addressesToIdentifiers[pRenderTarget];
}


void D3D9State::getRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget, IDirect3DSurface9** ppRenderTarget) {

	if (renderTargetStats[addressesToIdentifiers[pRenderTarget]] == NULL) {

        // Render target dosn't exist... so is Back Buffer
		D3D9RenderTargetStats* aux = new D3D9RenderTargetStats ();

		D3DSURFACE_DESC pDesc;
		(*ppRenderTarget)->GetDesc(&pDesc);
		aux->setDesc(&pDesc, pRenderTarget);
		aux->setSize(D3D9Info::instance().getTextureSize(pDesc.Width, pDesc.Height, 1, pDesc.Format));
        aux->setBackBuffer(true);

		addressesToIdentifiers[pRenderTarget] = nextId;
		identifiersToAddresses[nextId] = pRenderTarget;
		renderTargetStats[nextId] = aux;
        //rtToTextures[nextId] = 0; //...
		nextId++;
	}
}

void D3D9State::getSurfaceLevel(UINT sv_Level, IDirect3DSurface9* oip_ppSurfaceLevel, /*IDirect3DSurface9** spip_ppSurfaceLevel,*/ IDirect3DTexture9 * texture) {

	if (textureStats[addressesToIdentifiers[texture]] != NULL && renderTargetStats[addressesToIdentifiers[oip_ppSurfaceLevel]] == NULL) {
		D3D9RenderTargetStats* aux = new D3D9RenderTargetStats ();

		aux->copy((D3D9TextureStats*)textureStats[addressesToIdentifiers[texture]], oip_ppSurfaceLevel);
		aux->setSize(D3D9Info::instance().getTextureSize(((D3D9TextureStats*)textureStats[addressesToIdentifiers[texture]])->getWidth(), ((D3D9TextureStats*)textureStats[addressesToIdentifiers[texture]])->getHeight(), 1, ((D3D9TextureStats*)textureStats[addressesToIdentifiers[texture]])->getFormat()));
        aux->setTextureIdent(addressesToIdentifiers[texture]);

		addressesToIdentifiers[oip_ppSurfaceLevel] = nextId;
		identifiersToAddresses[nextId] = oip_ppSurfaceLevel;
		renderTargetStats[nextId] = aux;
        //rtToTextures[nextId] = addressesToIdentifiers[texture];
		nextId++;
	}
}

void D3D9State::setSamplerState( DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) {

	if (insideBeginEnd) {
		switch(Type) {
			case	D3DSAMP_MAGFILTER:
					current.samplerStatus[Sampler].magFilter = (D3DTEXTUREFILTERTYPE)Value;
					current.samplerStatus[Sampler].magFilterSetted = true;
					break;
			case	D3DSAMP_MINFILTER:
					current.samplerStatus[Sampler].minFilter = (D3DTEXTUREFILTERTYPE)Value;
					current.samplerStatus[Sampler].minFilterSetted = true;
					break;
			case	D3DSAMP_MIPFILTER:
					current.samplerStatus[Sampler].mipFilter = (D3DTEXTUREFILTERTYPE)Value;
					current.samplerStatus[Sampler].mipFilterSetted = true;
					break;
			case	D3DSAMP_MAXANISOTROPY:
					current.samplerStatus[Sampler].maxAnisotropy = Value;
					current.samplerStatus[Sampler].maxAnisotropySetted = true;
					break;
		}	
	}
	else {
		switch(Type) {
			case	D3DSAMP_MAGFILTER:
					actual.samplerStatus[Sampler].magFilter = (D3DTEXTUREFILTERTYPE)Value;
					actual.samplerStatus[Sampler].magFilterSetted = true;
					break;
			case	D3DSAMP_MINFILTER:
					actual.samplerStatus[Sampler].minFilter = (D3DTEXTUREFILTERTYPE)Value;
					actual.samplerStatus[Sampler].minFilterSetted = true;
					break;
			case	D3DSAMP_MIPFILTER:
					actual.samplerStatus[Sampler].mipFilter = (D3DTEXTUREFILTERTYPE)Value;
					actual.samplerStatus[Sampler].mipFilterSetted = true;
					break;
			case	D3DSAMP_MAXANISOTROPY:
					actual.samplerStatus[Sampler].maxAnisotropy = Value;
					actual.samplerStatus[Sampler].maxAnisotropySetted = true;
					break;
		}
	}
}

D3D9ShaderStats* D3D9State::getShaderStats (IDirect3DVertexShader9 *sh) {
	return shaderStats[addressesToIdentifiers[sh]];
}

D3D9ShaderStats* D3D9State::getShaderStats (IDirect3DPixelShader9 *sh) {
	return shaderStats[addressesToIdentifiers[sh]];
}

D3D9TextureStats* D3D9State::getTextureStats(void *tex) {
    return textureStats[addressesToIdentifiers[tex]];
}

D3D9RenderTargetStats* D3D9State::getRenderTargetStats(void *rt) {
    return renderTargetStats[addressesToIdentifiers[rt]];
}

D3D9RenderTargetStats* D3D9State::getRenderTargetStatsIdent(unsigned int rt) {
    return renderTargetStats[rt];
}

D3D9ShaderStats* D3D9State::getActualVshStats() {
    
    /*ofstream f;
    f.open("shLog.txt", ios::app);
    f << " -Actual Vertex Shader: " << actual.vertexShader << "\n";
    f.close();*/

	return shaderStats[addressesToIdentifiers[actual.vertexShader]];

}
D3D9ShaderStats* D3D9State::getActualPshStats() {
    
    /*ofstream f;
    f.open("shLog.txt", ios::app);
    f << " -Actual Pixel Shader: " << actual.pixelShader << "\n";
    f.close();*/

    /*f.open("shLog.txt", ios::app);
    f << " - " << addressesToIdentifiers[actual.pixelShader] << "\n";
    f.close();

    f.open("shLog.txt", ios::app);
    f << " - " << shaderStats[addressesToIdentifiers[actual.pixelShader]] << "\n";
    f.close();*/

		/*list<void *>::iterator it;

            f.open("shLog.txt", ios::app);
            f << "//ActualDifferentShadersSize: " << shaderStats[addressesToIdentifiers[actual.pixelShader]]->differentShaders.size() << "\n";
            f.close();

        for ( it=(shaderStats[addressesToIdentifiers[actual.pixelShader]]->differentShaders).begin() ; it != (shaderStats[addressesToIdentifiers[actual.pixelShader]]->differentShaders).end(); it++ ) {
            f.open("shLog.txt", ios::app);
            f << "//Actualfor:\n";
            f.close();
            f.open("shLog.txt", ios::app);
            f << "  // " << &(*it) << "\n";
            f.close();
        }*/

	return shaderStats[addressesToIdentifiers[actual.pixelShader]];

}

list <D3D9TextureStats*> D3D9State::getDifferentTextureStats(list <void *> differentTexture) {
	list<void *>::iterator it;
	list <D3D9TextureStats*> tsl;

	for (it = differentTexture.begin(); it != differentTexture.end(); it++ )
		tsl.push_back(textureStats[addressesToIdentifiers[(*it)]]);

	return tsl;
}

list <D3D9RenderTargetStats*> D3D9State::getDifferentRenderTargetStats(list <IDirect3DSurface9 *> differentRenderTarget) {
	list<IDirect3DSurface9 *>::iterator it;
	list <D3D9RenderTargetStats*> rtsl;

	for (it = differentRenderTarget.begin(); it != differentRenderTarget.end(); it++ ) 
		rtsl.push_back(renderTargetStats[addressesToIdentifiers[(*it)]]);

	return rtsl;
}

D3D9TextureStats* D3D9State::getActualTextureStats() {

	D3D9TextureStats* aux = new D3D9TextureStats();

	map< DWORD, SamplerStatusRecord >::iterator itSM;
	list< DWORD > samplersList;

	for( itSM = actual.samplerStatus.begin(); itSM != actual.samplerStatus.end(); itSM ++)
		samplersList.push_back((*itSM).first);


	list< DWORD >::iterator itSL;
	for(itSL = samplersList.begin(); itSL != samplersList.end(); itSL ++) {

		SamplerStatusRecord &status = actual.samplerStatus[*itSL];
		if (actual.pixelShader != NULL && (shaderStats[addressesToIdentifiers[actual.pixelShader]]->isUse(*itSL) != 0)) {

			if (textureStats[addressesToIdentifiers[status.texture]] != NULL)
				aux->addTextureStats(textureStats[addressesToIdentifiers[status.texture]]);
		}
	}

	return aux;
}

D3D9RenderTargetStats* D3D9State::getActualRenderTargetStats() {

	D3D9RenderTargetStats* aux = new D3D9RenderTargetStats();

	map<unsigned int, unsigned int>::iterator itRT;
	list< unsigned int > renderTargetList;

	for( itRT = settedRenderTargets.begin(); itRT != settedRenderTargets.end(); itRT ++)
		renderTargetList.push_back((*itRT).first);
	
	list< unsigned int >::iterator itRTL;

	for (itRTL = renderTargetList.begin(); itRTL != renderTargetList.end(); itRTL++) {
		if (renderTargetStats[settedRenderTargets[(*itRTL)]] != NULL) {
			aux->addRenderTargetStats(renderTargetStats[settedRenderTargets[(*itRTL)]]);
		}
	}

	return aux;
}

bool D3D9State::isATexture(void* pTexture) {
	if(addressesToIdentifiers[pTexture] == 0) return false;
	else return true;
}


int D3D9State::getNumAttrib() {
    int sum = 0;

	map<unsigned int, void *>::iterator itSM;


    for( itSM = settedStreamers.begin(); itSM != settedStreamers.end(); itSM ++) {
        if ((*itSM).second != 0)
            if (streamerStats[addressesToIdentifiers[(*itSM).second]].numAttrib != 0)
                sum += streamerStats[addressesToIdentifiers[(*itSM).second]].numAttrib;
    }

    if (sum == 0) 
        sum = actual.numAttrib;

    return sum;
}

int D3D9State::getNumAttribBytes() {
    int sum = 0;

	map<unsigned int, void *>::iterator itSM;

    for( itSM = settedStreamers.begin(); itSM != settedStreamers.end(); itSM ++) {
        if ((*itSM).second != 0)
            sum += streamerStats[addressesToIdentifiers[(*itSM).second]].size;
    }

    return sum;
}

int D3D9State::getNumAttribUP() {
    return actual.numAttrib;
}

int D3D9State::getSamplerUsage () {

	int samplerUsage = 0;

	map< DWORD, SamplerStatusRecord >::iterator itSM;
	list< DWORD > samplersList;

	for( itSM = actual.samplerStatus.begin(); itSM != actual.samplerStatus.end(); itSM ++)
		samplersList.push_back((*itSM).first);

	list< DWORD >::iterator itSL;
	for(itSL = samplersList.begin(); itSL != samplersList.end(); itSL ++) {

		SamplerStatusRecord &status = actual.samplerStatus[*itSL];
		if (actual.pixelShader != NULL && (shaderStats[addressesToIdentifiers[actual.pixelShader]]->isUse(*itSL) != 0)) {

			if (textureStats[addressesToIdentifiers[status.texture]] != NULL) samplerUsage++;
		}
	}

	return samplerUsage;
}

IDirect3DVertexShader9 * D3D9State::getActualVertexShader() {
	return actual.vertexShader;
}

IDirect3DPixelShader9 * D3D9State::getActualPixelShader() {
	return actual.pixelShader;
}

bool D3D9State::getAlphaTest() {
    return actual.alphaTestEnabled;
}

unsigned int D3D9State::getLastApplyStateBlock() {
	return lastApplyStateBlock;
}

void D3D9State:: unsetCurrentStateBlock() {

	current.pixelShaderSetted = false;
	current.vertexShaderSetted = false;
    current.alphaTestSetted = false;
    current.numAttribSetted = false;
	current.colorWriteEnableSetted = false;

	map< DWORD, SamplerStatusRecord >::iterator itSM;
	list< DWORD > samplersList;

	for( itSM = current.samplerStatus.begin(); itSM != current.samplerStatus.end(); itSM ++)
		samplersList.push_back((*itSM).first);

	list< DWORD >::iterator itSL;
	for(itSL = samplersList.begin(); itSL != samplersList.end(); itSL ++) {
		SamplerStatusRecord &status = current.samplerStatus[*itSL];
		status.magFilterSetted = false;
		status.minFilterSetted = false;
		status.mipFilterSetted = false;
		status.maxAnisotropySetted = false;
		status.textureSetted = false;
	}

}

void D3D9State:: beginStateBlock() {

	insideBeginEnd = true;
	unsetCurrentStateBlock();

}

void D3D9State:: endStateBlock(IDirect3DStateBlock9 *sb) {

	addressesToIdentifiers[sb] = nextId;
	identifiersToAddresses[nextId] = sb;
	stateBlockRecords[nextId] = current;
	nextId ++;

	insideBeginEnd = false;
	unsetCurrentStateBlock();

}


void D3D9State:: applyStateBlock(IDirect3DStateBlock9 *sb) {

	int idSb = addressesToIdentifiers[sb];
	StateBlockRecord record = stateBlockRecords[idSb];

	lastApplyStateBlock = idSb;

	if (record.pixelShaderSetted) {
		actual.pixelShaderSetted = record.pixelShaderSetted;
		actual.pixelShader = record.pixelShader;
	}

	if (record.vertexShaderSetted) {
		actual.vertexShaderSetted = record.vertexShaderSetted;
		actual.vertexShader = record.vertexShader;
	}

    if (record.alphaTestSetted) {
        actual.alphaTestSetted = record.alphaTestSetted;
        actual.alphaTestEnabled = record.alphaTestEnabled;
    }

    if (record.numAttribSetted) {
        actual.numAttribSetted = record.numAttribSetted;
        actual.numAttrib = record.numAttrib;
    }

    if (record.colorWriteEnableSetted) {
		actual.colorWriteEnableSetted = record.colorWriteEnableSetted;
		actual.rEnable = record.rEnable;
		actual.gEnable = record.gEnable;
		actual.bEnable = record.bEnable;
		actual.aEnable = record.aEnable;
    }

	map< DWORD, SamplerStatusRecord > ::iterator itS;

	for(itS = record.samplerStatus.begin(); itS != record.samplerStatus.end(); itS ++) {
		DWORD currentS = (*itS).first;
		SamplerStatusRecord currentSSR = (*itS).second;
		
		SamplerStatusRecord &samplerStatus =  actual.samplerStatus[currentS];

		if(currentSSR.magFilterSetted)
			samplerStatus.magFilter = currentSSR.magFilter;
		if(currentSSR.minFilterSetted)
			samplerStatus.minFilter = currentSSR.minFilter;
		if(currentSSR.mipFilterSetted)
			samplerStatus.mipFilter = currentSSR.mipFilter;
		if(currentSSR.maxAnisotropySetted)
			samplerStatus.maxAnisotropy = currentSSR.maxAnisotropy;
		if(currentSSR.textureSetted)
			samplerStatus.texture = currentSSR.texture;
	}

}

void D3D9State:: captureStateBlock(IDirect3DStateBlock9 *sb) {

	int idSb = addressesToIdentifiers[sb];
	StateBlockRecord &stateBlock = stateBlockRecords[idSb];

	if (stateBlock.pixelShaderSetted)
		stateBlock.pixelShader = actual.pixelShader;

	if (stateBlock.vertexShaderSetted)
		stateBlock.vertexShader = actual.vertexShader;

    if (stateBlock.alphaTestSetted)
        stateBlock.alphaTestEnabled = actual.alphaTestEnabled;

    if (stateBlock.numAttribSetted)
        stateBlock.numAttrib = actual.numAttrib;

    if (stateBlock.colorWriteEnableSetted) {
		stateBlock.rEnable = actual.rEnable;
		stateBlock.gEnable = actual.gEnable;
		stateBlock.bEnable = actual.bEnable;
		stateBlock.aEnable = actual.aEnable;
    }

	map< DWORD, SamplerStatusRecord > ::iterator itS;

	for(itS = stateBlock.samplerStatus.begin(); itS != stateBlock.samplerStatus.end(); itS ++) {
		DWORD currentS = (*itS).first;
		SamplerStatusRecord currentSSR = (*itS).second;
		
		SamplerStatusRecord &samplerStatus =  actual.samplerStatus[currentS];

		if(currentSSR.magFilterSetted)
			stateBlock.samplerStatus[(*itS).first].magFilter = samplerStatus.magFilter;

		if(currentSSR.minFilterSetted)
			stateBlock.samplerStatus[(*itS).first].minFilter = samplerStatus.minFilter;

		if(currentSSR.mipFilterSetted)
			stateBlock.samplerStatus[(*itS).first].mipFilter = samplerStatus.mipFilter;

		if(currentSSR.maxAnisotropySetted)
			stateBlock.samplerStatus[(*itS).first].maxAnisotropy = samplerStatus.maxAnisotropy;

		if(currentSSR.textureSetted)
			stateBlock.samplerStatus[(*itS).first].texture = samplerStatus.texture;

	}

}

unsigned long long D3D9State::getSamplingCategory(int* samplingType, unsigned long long exec) {

	map< DWORD, SamplerStatusRecord > ::iterator itS;

    unsigned long long acumbw = 0;

   /* ofstream f;
    f.open("samplersLog.txt", ios::app);
    f << "begin\n" ;
    f << &actual << "\n";
    f << "  - " << actual.pixelShader << "\n";
    f.close();*/

	for(itS = actual.samplerStatus.begin(); itS != actual.samplerStatus.end(); itS ++) {
		DWORD currentS = (*itS).first;
		SamplerStatusRecord currentSSR = (*itS).second;
		
        /*f.open("samplersLog.txt", ios::app);
        f << currentS << ": " << "\n" ;
        f.close();
        f.open("samplersLog.txt", ios::app);
        f << "  - " << actual.pixelShader << "\n";
        f.close();
        f.open("samplersLog.txt", ios::app);
        f << "  - " << shaderStats[addressesToIdentifiers[actual.pixelShader]]->isUse(currentS) << "\n" ;
        f.close();
        f.open("samplersLog.txt", ios::app);
        f << "  - " << actual.samplerStatus[currentS].minFilter << "\n";
        f.close();
        f.open("samplersLog.txt", ios::app);
        f << "  - " << actual.samplerStatus[currentS].mipFilter << "\n";
        f.close();
        f.open("samplersLog.txt", ios::app);
        f << "  - " << actual.samplerStatus[currentS].maxAnisotropy << "\n";
        f.close();*/

		SamplerStatusRecord &samplerStatus =  actual.samplerStatus[currentS];

        int use;
        if (actual.pixelShader != NULL)
            use = shaderStats[addressesToIdentifiers[actual.pixelShader]]->isUse(currentS);
        else
            use = 0;

		if (actual.pixelShader != NULL && (use != 0)) {
			if((samplerStatus.minFilter == D3DTEXF_POINT) &
				((samplerStatus.mipFilter == D3DTEXF_POINT) |
                (samplerStatus.mipFilter == D3DTEXF_NONE))){
				samplingType[NEAREST] += (use * exec);
                acumbw = acumbw + use * exec;
            }
			else if((samplerStatus.minFilter == D3DTEXF_POINT) &
                (samplerStatus.mipFilter == D3DTEXF_LINEAR)){
				samplingType[NEAREST_MIPMAP_LINEAR] += (use * exec);
                acumbw = acumbw + use * exec * 2;
            }
			else if((samplerStatus.minFilter == D3DTEXF_LINEAR) &
				((samplerStatus.mipFilter == D3DTEXF_POINT) |
                (samplerStatus.mipFilter == D3DTEXF_NONE))){
				samplingType[BILINEAR] += (use * exec);
                acumbw = acumbw + use * exec * 4;
            }
			else if((samplerStatus.minFilter == D3DTEXF_ANISOTROPIC) &
				((samplerStatus.mipFilter == D3DTEXF_POINT) |
				(samplerStatus.mipFilter == D3DTEXF_NONE)) &
                (samplerStatus.maxAnisotropy > 1)) {
                    if (samplerStatus.maxAnisotropy == 2) {
				        samplingType[BILINEAR_ANISO_2] += (use * exec);
                        acumbw = acumbw + use * exec * 8;
                    }
                    else if (samplerStatus.maxAnisotropy == 4) {
				        samplingType[BILINEAR_ANISO_4] += (use * exec);
                        acumbw = acumbw + use * exec * 16;
                    }
                    else if (samplerStatus.maxAnisotropy == 8) {
				        samplingType[BILINEAR_ANISO_8] += (use * exec);
                        acumbw = acumbw + use * exec * 32;
                    }
                    else if (samplerStatus.maxAnisotropy == 16) {
				        samplingType[BILINEAR_ANISO_16] += (use * exec);
                        acumbw = acumbw + use * exec * 64;
                    }
                    else {
				        samplingType[UNKNOWN] += (use * exec);
                    }
            }

			else if((samplerStatus.minFilter == D3DTEXF_LINEAR) &
                (samplerStatus.mipFilter == D3DTEXF_LINEAR)){
				samplingType[TRILINEAR] += (use * exec);
                acumbw = acumbw + use * exec * 8;
            }
			else if((samplerStatus.minFilter == D3DTEXF_ANISOTROPIC) &
				(samplerStatus.mipFilter == D3DTEXF_LINEAR) &
                (samplerStatus.maxAnisotropy > 1)) {
                    if (samplerStatus.maxAnisotropy == 2) {
				        samplingType[TRILINEAR_ANISO_2] += (use * exec);
                        acumbw = acumbw + use * exec * 16;
                    }
                    else if (samplerStatus.maxAnisotropy == 4) {
				        samplingType[TRILINEAR_ANISO_4] += (use * exec);
                        acumbw = acumbw + use * exec * 32;
                    }
                    else if (samplerStatus.maxAnisotropy == 8) {
				        samplingType[TRILINEAR_ANISO_8] += (use * exec);
                        acumbw = acumbw + use * exec * 64;
                    }
                    else if (samplerStatus.maxAnisotropy == 16) {
 				        samplingType[TRILINEAR_ANISO_16] += (use * exec);
                        acumbw = acumbw + use * exec * 128;
                    }
                    else {
				        samplingType[UNKNOWN] += (use * exec);
                    }
            }
			else samplingType[UNKNOWN] += (use * exec);


        }
	}
    /*f.open("samplersLog.txt", ios::app);
    f << "end\n" ;
    f.close();*/

    return acumbw;

}

void D3D9State::clearRTOp(DWORD ov_Flags, DWORD ov_Count) {

    /*ofstream f;

    if (settedRenderTargets[0] != NULL) {
            f.open("rtLog.txt", ios::app);
            f << "--" << renderTargetStats[settedRenderTargets[0]]->getTextureIdent() << ": clear\n";
            f.close();
    }

    if (rtOp != NULL)
        delete rtOp;

    rtOp = new D3D9RenderTargetOp(settedRenderTargets[0]);*/
    if (ov_Count == 0) {

        if ((ov_Flags & D3DCLEAR_TARGET) != 0)
            rtOp->clear();

        if ((dsOp != NULL) && ((ov_Flags & D3DCLEAR_ZBUFFER) != 0))
            dsOp->clear();

    }
    else {

        if ((ov_Flags & D3DCLEAR_TARGET) != 0)
            rtOp->partialClear();

        if ((dsOp != NULL) && ((ov_Flags & D3DCLEAR_ZBUFFER) != 0))
            dsOp->partialClear();

    }

}

void D3D9State::resetDSOp() {

    ofstream f;

    if (setDepthStencil != NULL) {
            f.open("rtLog.txt", ios::app);
            f << "--DS " << renderTargetStats[setDepthStencil]->getTextureIdent() << ": new\n";
            f.close();
    }

    if (setDepthStencil != NULL) {
        dsOp = new D3D9RenderTargetOp(setDepthStencil);
        dsOp->depthRT(true);
        dsOp->setColorWriteEnable(true);
    }
    else 
        dsOp = NULL;

}

void D3D9State::resetRTOp() {

    ofstream f;

    if (settedRenderTargets[0] != NULL) {
            f.open("rtLog.txt", ios::app);
            f << "--" << renderTargetStats[settedRenderTargets[0]]->getTextureIdent() << ": new\n";
            f.close();
    }

    rtOp = new D3D9RenderTargetOp(settedRenderTargets[0]);

}

D3D9RenderTargetOp* D3D9State::getRTOp() {

    if ((lastRTorigSR == rtOp->getRT()) && (rtOp->getPixelsDrawn() == 0)) {
        lastRTorigSR = 0;
        return NULL;
    }

    lastRTorigSR = 0;
    return rtOp;

}

D3D9RenderTargetOp* D3D9State::getDSOp() {

    return dsOp;

}

void D3D9State::addTexSetToRTOp() {

    ofstream f;
    

            f.open("rtLog.txt", ios::app);
            f << "--";

    map< DWORD, SamplerStatusRecord >::iterator itSM;
	list< DWORD > samplersList;

	for( itSM = actual.samplerStatus.begin(); itSM != actual.samplerStatus.end(); itSM ++)
		samplersList.push_back((*itSM).first);

	list< DWORD >::iterator itSL;
	for(itSL = samplersList.begin(); itSL != samplersList.end(); itSL ++) {

		SamplerStatusRecord &status = actual.samplerStatus[*itSL];
		if (actual.pixelShader != NULL && (shaderStats[addressesToIdentifiers[actual.pixelShader]]->isUse(*itSL) != 0)) {

            // if t = 0 means that set a non registered texture...
            // this happens in NFSU. Have to see why...
            if (addressesToIdentifiers[status.texture] != 0)
                rtOp->addTextOp(addressesToIdentifiers[status.texture]);

            f << addressesToIdentifiers[status.texture] << " ";

		}
	}

    rtOp->setColorWriteEnable(actual.rEnable || actual.gEnable || actual.bEnable || actual.aEnable);
    rtOp->addDraw(1);

    if (dsOp != NULL)
        dsOp->addDraw(1);

    f << ": add\n";
            f.close();

}

D3D9RenderTargetOp* D3D9State::addTexToRTOp(IDirect3DSurface9* sip_pSourceSurface, IDirect3DSurface9* sip_pDestSurface/*, D3D9RenderTargetOp** tmpSR*/) {
    //rtOp->addTextOp(renderTargetStats[t]->getTextureIdent());

    ofstream f;
    

            f.open("rtLog.txt", ios::app);

    /*if (rtOp->isRT(addressesToIdentifiers[sip_pDestSurface])) {
        if (addressesToIdentifiers[sip_pSourceSurface] != NULL) {
        f << "--" << "dst is setted, orig dif NULL: " << renderTargetStats[addressesToIdentifiers[sip_pSourceSurface]]->getTextureIdent() << "\n";
            rtOp->addTextOp(renderTargetStats[addressesToIdentifiers[sip_pSourceSurface]]->getTextureIdent());
        }
        else {
        f << "--" << "dst is setted, orig eq NULL\n";
            rtOp->addTextOp(0);
        }

            f.close();
        return NULL;
    }
    else {*/

        // Create a new graph node with the destination surface.
        D3D9RenderTargetOp* tmpRtOp = new D3D9RenderTargetOp(addressesToIdentifiers[sip_pDestSurface]);

        // Create a new graph node to represent stretch rect operation.
        /*(*tmpSR) = new D3D9RenderTargetOp(addressesToIdentifiers[sip_pSourceSurface]);
        (*tmpSR)->setStretchRect(true);
        //(*tmpSR)->addTextOp(renderTargetStats[addressesToIdentifiers[sip_pSourceSurface]]->getTextureIdent());
        (*tmpSR)->addTextOp(addressesToIdentifiers[sip_pSourceSurface]);
        (*tmpSR)->setColorWriteEnable(true);*/

        // If exist source surface...
        if (addressesToIdentifiers[sip_pSourceSurface] != NULL) {
            /*if (rtOp->isRT(addressesToIdentifiers[sip_pSourceSurface])) {

            }
            else {*/
                f << "--" << "dst not setted:" << renderTargetStats[addressesToIdentifiers[sip_pDestSurface]]->getTextureIdent() << ", orig: " << renderTargetStats[addressesToIdentifiers[sip_pSourceSurface]]->getTextureIdent() << "\n";
                tmpRtOp->addTextOp(renderTargetStats[addressesToIdentifiers[sip_pSourceSurface]]->getTextureIdent());
            //}
        }
        else {
            f << "--" << "dst not setted:" << renderTargetStats[addressesToIdentifiers[sip_pDestSurface]]->getTextureIdent() << ", orig eq NULL\n";
            tmpRtOp->addTextOp(0);
        }
        tmpRtOp->setColorWriteEnable(true);
        tmpRtOp->setStretchRect(true);
        lastRTorigSR = addressesToIdentifiers[sip_pSourceSurface];

            f.close();

        return tmpRtOp;
    //}
}

unsigned int D3D9State::getTexIformRT(unsigned int t) {

    if (renderTargetStats[t] != NULL)
        return renderTargetStats[t]->getTextureIdent();
    else
        return 0;

}


bool D3D9State::isRTBackBuffer(unsigned int t) {

    if (renderTargetStats[t] != NULL)
        return renderTargetStats[t]->isBackBuffer();
    else
        return false;

}

bool D3D9State::isRTSet(IDirect3DSurface9* surface) {

    return rtOp->isRT(addressesToIdentifiers[surface]);

}

unsigned int D3D9State::getIdentifier(void* p) {

    return addressesToIdentifiers[p];

}

void D3D9State::addNumberOfPixelsDrawn(UINT64 num) {
    rtOp->addNumPixelsDrawn(num);

    if (dsOp != NULL)
        dsOp->addNumPixelsDrawn(num);

}
