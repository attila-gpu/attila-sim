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

//////////////////////////////////////////////////////////
//////												//////	
//////					D3DStats.cpp				//////
//////												//////
//////////////////////////////////////////////////////////

#include "D3D9Stats.h"
#include "D3D9State.h"
#include "OptimizationDataStructures.h"


#ifdef NVIDIA_STATS
	#include <NVPerfSDK.h>
#endif

using namespace std;

////////////////////////////////////////////////////////////
////////		D3D9Stats Definitions               ////////
////////////////////////////////////////////////////////////

D3D9Stats::D3D9Stats() {}

bool D3D9Stats::isTrace() {
	return false;
}

bool D3D9Stats::isFrame() {
	return false;
}

bool D3D9Stats::isBatch() {
	return false;
}

void D3D9TraceStats::beginBatch() {}

D3D9FrameStats* D3D9Stats::getBackFrame() {
	return NULL;
}

D3D9BatchStats* D3D9Stats::getBackBatch() {
	return NULL;
}

D3D9StatsCollector* D3D9Stats::getStadistics() {
	return &stadistics;
}


////////////////////////////////////////////////////////////
////////	D3D9Stats Stadistics Functions          ////////
////////////////////////////////////////////////////////////

void D3D9Stats::registerDrawPrimitive(D3DPRIMITIVETYPE type, bool indexed, UINT startIndex, UINT primitiveCount, int numberOfPixelsDrawn) {

    ofstream f;

    if (isTrace()) {

            f.open("rtLog.txt", ios::app);
            f << tmpNumFrame << ": draw\n";
            f.close();

        D3D9State::instance().addTexSetToRTOp();
		getBackFrame()->registerDrawPrimitive(type, indexed, startIndex, primitiveCount, numberOfPixelsDrawn);
        D3D9State::instance().addNumberOfPixelsDrawn(getBackFrame()->getStadistics()->getShadedPixel());

    }

	if (isFrame()) {
		#ifdef BATCH_STATS
			getBackBatch()->registerDrawPrimitive(type, indexed, startIndex, primitiveCount, numberOfPixelsDrawn);
		#else
			#ifdef GENERAL_STATS
				stadistics.addDrawPrimitive();
				stadistics.addPrimitiveCount(primitiveCount);

				int numVertex;

				switch (type) {

					case 1: stadistics.addDrawPoint(primitiveCount); numVertex = primitiveCount; break;
					case 2: stadistics.addDrawLine(primitiveCount); numVertex = primitiveCount * 2; break;
					case 4: stadistics.addDrawTriangleList(primitiveCount); numVertex = primitiveCount * 3; break;
					case 5: stadistics.addDrawTriangleStript(primitiveCount); numVertex = primitiveCount + 2; break;
					case 6: stadistics.addDrawTriangleFan(primitiveCount); numVertex = primitiveCount + 2; break;
				}
			#endif

			#ifdef NVIDIA_STATS
				stadistics.collectNvidiaCounters();
			#endif

		    #ifdef VERTEX_SHADER_STATS
                switch (type) {
                    case 1:
                    case 2:
                    case 4:
                        if (indexed) {
                            //int numIndices = D3D9State::instance().getNumIndices(startIndex, numVertex);
                            int numIndices1, numIndices2, numIndices3, numIndices4;
                            D3D9State::instance().getNumIndices(startIndex, numVertex, &numIndices1, &numIndices2, &numIndices3, &numIndices4);
                            stadistics.getVertexShaderStats()->addShaderStats(D3D9State::instance().getActualVshStats(), numIndices4);
                            stadistics.addComputedVertices(numIndices1, numIndices2, numIndices3, numIndices4);
                            stadistics.addAttributes(numVertex, numIndices4);
                        }
                        else {
                            stadistics.getVertexShaderStats()->addShaderStats(D3D9State::instance().getActualVshStats(), numVertex);
                            stadistics.addComputedVertices(numVertex, numVertex, numVertex, numVertex);
                            stadistics.addAttributes(numVertex, numVertex);
                        }
                        break;
                    case 5:
                    case 6:
			           stadistics.getVertexShaderStats()->addShaderStats(D3D9State::instance().getActualVshStats(), numVertex);
                       stadistics.addComputedVertices(numVertex, numVertex, numVertex, numVertex);
                       stadistics.addAttributes(numVertex, numVertex);
                       break;
                }
            #endif

            #ifdef PIXEL_SHADER_STATS
                stadistics.getPixelShaderStats()->addShaderStats(D3D9State::instance().getActualPshStats(), stadistics.getShadedPixel());
		    #endif

			#ifdef TEXTURE_STATS
				stadistics.getTextureStats()->addTextureStats(D3D9State::instance().getActualTextureStats());
			    stadistics.setAverageActiveSampler(D3D9State::instance().getSamplerUsage());
				stadistics.setActiveSampler(D3D9State::instance().getSamplerUsage() * stadistics.getShadedPixel());
				D3D9State::instance().getSamplingCategory(stadistics.getFilterVector(), stadistics.getShadedPixel());
			#endif

			#ifdef RENDERTARGET_STATS
				stadistics.getRenderTargetStats()->addRenderTargetStats(D3D9State::instance().getActualRenderTargetStats());
			#endif

            #ifdef NEW_STATS
                //stadistics.addAttributes();
                stadistics.addNumberOfPixelsDrawn(numberOfPixelsDrawn);
            #endif
		#endif
	}

	if (isBatch()) {

		#ifdef GENERAL_STATS
			stadistics.addDrawPrimitive();
			stadistics.addPrimitiveCount(primitiveCount);

			int numVertex;

			switch (type) {
				case 1: stadistics.addDrawPoint(primitiveCount); numVertex = primitiveCount; break;
				case 2: stadistics.addDrawLine(primitiveCount); numVertex = primitiveCount * 2; break;
				case 4: stadistics.addDrawTriangleList(primitiveCount); numVertex = primitiveCount * 3; break;
				case 5: stadistics.addDrawTriangleStript(primitiveCount); numVertex = primitiveCount + 2; break;
				case 6: stadistics.addDrawTriangleFan(primitiveCount); numVertex = primitiveCount + 2; break;
			}
		#endif 

		#ifdef NVIDIA_STATS
			stadistics.collectNvidiaCounters();
		#endif

		#ifdef VERTEX_SHADER_STATS
            switch (type) {
                case 1:
                case 2:
                case 4:
                    if (indexed) {
                        //int numIndices = D3D9State::instance().getNumIndices(startIndex, numVertex);
                        int numIndices1, numIndices2, numIndices3, numIndices4;
                        int numAttrib, numAttribBytes;
                        numIndices1 = 0;
                        numIndices2 = 0;
                        numIndices3 = 0;
                        numIndices4 = 0;
                        numAttrib = 0;
                        numAttribBytes = 0;
                        /*ofstream fl;
                        fl.open("cacheLog.txt", ios::app);
                        fl << "*c16: " << numIndices1 << " c32: " << numIndices2 << " c48: " << numIndices3 << " cInf: " << numIndices4 << "\n";
                        fl.close();*/
                        if (nextIsUP) {
                            D3D9State::instance().getNumIndicesUP(numVertex, &numIndices1, &numIndices2, &numIndices3, &numIndices4, UPindexs, UPformat);
                            numAttrib = D3D9State::instance().getNumAttribUP();
                            numAttribBytes = UPbytes;
                            nextIsUP = false;
                            /*ofstream fup;
                            fup.open("UPLog.txt", ios::app);
                            fup << "IsUP:\n";
                            fup << "  numIndices1: " << numIndices1 << "\n";
                            fup << "  numIndices2: " << numIndices2 << "\n";
                            fup << "  numIndices3: " << numIndices3 << "\n";
                            fup << "  numIndices4: " << numIndices4 << "\n";
                            fup << "  numAttrib: " << numAttrib << "\n";
                            fup << "  numAttribBytes: " << numAttribBytes << "\n";
                            fup.close();*/
                        }
                        else {
                            D3D9State::instance().getNumIndices(startIndex, numVertex, &numIndices1, &numIndices2, &numIndices3, &numIndices4);
                            numAttrib = D3D9State::instance().getNumAttrib();
                            numAttribBytes = D3D9State::instance().getNumAttribBytes();
                        }
                        /*ofstream fl;
                        fl.open("cacheLog.txt", ios::app);
                        fl << "+c16: " << numIndices1 << " c32: " << numIndices2 << " c48: " << numIndices3 << " cInf: " << numIndices4 << "\n";
                        fl.close();*/
                        stadistics.getVertexShaderStats()->addShaderStats(D3D9State::instance().getActualVshStats(), numIndices4);
                        stadistics.addComputedVertices(numIndices1, numIndices2, numIndices3, numIndices4);
                        stadistics.addAttributes(numAttrib, numAttribBytes, numVertex, numIndices4);
                    }
                    else {
                        stadistics.getVertexShaderStats()->addShaderStats(D3D9State::instance().getActualVshStats(), numVertex);
                        stadistics.addComputedVertices(numVertex, numVertex, numVertex, numVertex);
                        stadistics.addAttributes(D3D9State::instance().getNumAttrib(), D3D9State::instance().getNumAttribBytes(), numVertex, numVertex);
                    }
                    break;
                case 5:
                case 6:
			       stadistics.getVertexShaderStats()->addShaderStats(D3D9State::instance().getActualVshStats(), numVertex);
                   stadistics.addComputedVertices(numVertex, numVertex, numVertex, numVertex);
                   stadistics.addAttributes(D3D9State::instance().getNumAttrib(), D3D9State::instance().getNumAttribBytes(), numVertex, numVertex);
                   break;
            }
        #endif

            /*f.open("shLog.txt", ios::app);
            f << "++1\n";
            f.close();*/

        #ifdef PIXEL_SHADER_STATS
            stadistics.getPixelShaderStats()->addShaderStats(D3D9State::instance().getActualPshStats(), stadistics.getShadedPixel());
		#endif

		#ifdef TEXTURE_STATS
			stadistics.getTextureStats()->addTextureStats(D3D9State::instance().getActualTextureStats());
			stadistics.setAverageActiveSampler(D3D9State::instance().getSamplerUsage());
			stadistics.setActiveSampler(D3D9State::instance().getSamplerUsage() * stadistics.getShadedPixel());
			D3D9State::instance().getSamplingCategory(stadistics.getFilterVector(), stadistics.getShadedPixel());
		#endif

		#ifdef RENDERTARGET_STATS
			stadistics.getRenderTargetStats()->addRenderTargetStats(D3D9State::instance().getActualRenderTargetStats());
		#endif

        #ifdef NEW_STATS
            //stadistics.addAttributes(numVertex, numIndices);
            if (D3D9State::instance().getAlphaTest())
                stadistics.addAlphaTest(stadistics.getShadedPixel());
        #endif

        #ifdef OCCLUSION_STATS
            stadistics.addNumberOfPixelsDrawn(numberOfPixelsDrawn);
        #endif
	}
}

void D3D9Stats::registerVertexShaderCreation(IDirect3DVertexShader9 *sh, DWORD *function) {
	D3D9State::instance().addShader(sh, function);
}	

void D3D9Stats::registerPixelShaderCreation(IDirect3DPixelShader9 *sh, DWORD *function) {
	D3D9State::instance().addShader(sh, function);
}

void D3D9Stats::registerVertexShaderSetted(IDirect3DVertexShader9 *sh) {
	D3D9State::instance().setVertexShader(sh);
}

void D3D9Stats::registerPixelShaderSetted(IDirect3DPixelShader9 *sh) {
	D3D9State::instance().setPixelShader(sh);
}

void D3D9Stats::registerCreateTexture(	UINT Width,	UINT Height, UINT Levels, DWORD Usage, 
										D3DFORMAT Format, D3DPOOL Pool,  IDirect3DTexture9* pTexture,
										HANDLE* pSharedHandle){
	
	if (isTrace()) {
		getBackFrame()->registerCreateTexture(Width, Height, Levels, Usage, Format, Pool, pTexture, pSharedHandle);
	}
	if (isFrame()) {
		#ifdef BATCH_STATS
			getBackBatch()->registerCreateTexture(Width, Height, Levels, Usage, Format, Pool, pTexture, pSharedHandle);
		#else
	        D3D9State::instance().addTexture(Width, Height, Levels, Usage, Format, Pool, pTexture, pSharedHandle);
			stadistics.addCreateTexture();
		#endif
	}
	if (isBatch()) {
		D3D9State::instance().addTexture(Width, Height, Levels, Usage, Format, Pool, pTexture, pSharedHandle);
		stadistics.addCreateTexture();
    }
}

void D3D9Stats::registerCreateVolumeTexture(	UINT Width,	UINT Height, UINT Depth,
												UINT Levels, DWORD Usage, D3DFORMAT Format,
												D3DPOOL Pool, IDirect3DVolumeTexture9* pVolumeTexture,
												HANDLE* pSharedHandle){
	

	if (isTrace()) {
		getBackFrame()->registerCreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, pVolumeTexture, pSharedHandle);
	}
	if (isFrame()) {
		#ifdef BATCH_STATS
			getBackBatch()->registerCreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, pVolumeTexture, pSharedHandle);
		#else
	        D3D9State::instance().addVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, pVolumeTexture, pSharedHandle);
			stadistics.addCreateTexture();
		#endif
	}
	if (isBatch()) {
		D3D9State::instance().addVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, pVolumeTexture, pSharedHandle);
		stadistics.addCreateTexture();
    }
}

void D3D9Stats::registerCreateCubeTexture(	UINT EdgeLength, UINT Levels, DWORD Usage,
											D3DFORMAT Format, D3DPOOL Pool,	IDirect3DCubeTexture9 * pCubeTexture,
											HANDLE* pSharedHandle) {

	if (isTrace()) {
		getBackFrame()->registerCreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, pCubeTexture, pSharedHandle);
	}
	if (isFrame()) {
		#ifdef BATCH_STATS
			getBackBatch()->registerCreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, pCubeTexture, pSharedHandle);
		#else
	        D3D9State::instance().addCubeTexture(EdgeLength, Levels, Usage, Format, Pool, pCubeTexture, pSharedHandle);
			stadistics.addCreateTexture();
		#endif
	}
	if (isBatch()) {
		D3D9State::instance().addCubeTexture(EdgeLength, Levels, Usage, Format, Pool, pCubeTexture, pSharedHandle);
		stadistics.addCreateTexture();
    }
}

void D3D9Stats::registerTextureSetted(DWORD sampler, IDirect3DBaseTexture9 *tex) {

	D3D9State::instance().setTexture(sampler, tex);
}

void D3D9Stats::registerSetSamplerState( DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) {

	D3D9State::instance().setSamplerState(Sampler, Type, Value);
}

void D3D9Stats::registerBeginStateBlock() {

	if (isTrace()) {
		getBackFrame()->registerBeginStateBlock();
	}
	if (isFrame()) {
		#ifdef BATCH_STATS
			getBackBatch()->registerBeginStateBlock();
		#else
			stadistics.addBeginStateBlock();
			D3D9State::instance().beginStateBlock();
		#endif
	}
	if (isBatch()) {
		stadistics.addBeginStateBlock();
		D3D9State::instance().beginStateBlock();
	}
}

void D3D9Stats::registerEndStateBlock(IDirect3DStateBlock9 *sb) {
	if (isTrace()) {
		getBackFrame()->registerEndStateBlock(sb);
	}
	if (isFrame()) {
		#ifdef BATCH_STATS
			getBackBatch()->registerEndStateBlock(sb);
		#else
			stadistics.addEndStateBlock();
			D3D9State::instance().endStateBlock(sb);
		#endif
	}
	if (isBatch()) {
		stadistics.addEndStateBlock();
		D3D9State::instance().endStateBlock(sb);
	}
}

void D3D9Stats::registerApplyStateBlock(IDirect3DStateBlock9 *sb) {
	if (isTrace()) {
		getBackFrame()->registerApplyStateBlock(sb);
	}
	if (isFrame()) {
		#ifdef BATCH_STATS
			getBackBatch()->registerApplyStateBlock(sb);
		#else
			stadistics.addApplyStateBlock();
			D3D9State::instance().applyStateBlock(sb);
		#endif
	}
	if (isBatch()) {
		stadistics.addApplyStateBlock();
		D3D9State::instance().applyStateBlock(sb);
	}
}

void D3D9Stats::registerCaptureStateBlock(IDirect3DStateBlock9 *sb) {
	if (isTrace()) {
		getBackFrame()->registerCaptureStateBlock(sb);
	}
	if (isFrame()) {
		#ifdef BATCH_STATS
			getBackBatch()->registerCaptureStateBlock(sb);
		#else
			stadistics.addCaptureStateBlock();
			D3D9State::instance().captureStateBlock(sb);
		#endif
	}
	if (isBatch()) {
		stadistics.addCaptureStateBlock();
		D3D9State::instance().captureStateBlock(sb);
	}
}

void D3D9Stats::registerCreateRenderTarget(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,
										  BOOL Lockable, IDirect3DSurface9* oip_ppSurface, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle){

	//D3D9State::instance().addRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, oip_ppSurface, ppSurface, pSharedHandle);

	if (isTrace()) {
		getBackFrame()->registerCreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, oip_ppSurface, ppSurface, pSharedHandle);
	}
	if (isFrame()) {
		#ifdef BATCH_STATS
			getBackBatch()->registerCreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, oip_ppSurface, ppSurface, pSharedHandle);
		#else
			stadistics.addCreateRenderTarget();
			D3D9State::instance().addRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, oip_ppSurface, ppSurface, pSharedHandle);
		#endif
	}
	if (isBatch()) {
		stadistics.addCreateRenderTarget();
		D3D9State::instance().addRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, oip_ppSurface, ppSurface, pSharedHandle);
	}

}

void D3D9Stats::registerSetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9 * pRenderTarget ) {

    ofstream f;

            f.open("rtLog.txt", ios::app);
            f << tmpNumFrame << ": setRenderTarget\n";
            f.close();

    if(!(D3D9State::instance().isRTSet(pRenderTarget)) && (RenderTargetIndex == 0)) {

        D3D9RenderTargetOp* rt = D3D9State::instance().getRTOp();

        if (rt != NULL) {
            if (rt->getClears() > 0) {
                D3D9RenderTargetOp* clear = new D3D9RenderTargetOp(rt->getRT());
                clear->setClearNode(true);
                listRTOp.push_back(clear);
                D3D9RenderTargetStats* rtStats = D3D9State::instance().getRenderTargetStatsIdent(rt->getRT());
                if (rtStats == NULL)
                    rt->addTextOp(0);
                else
                    rt->addTextOp(rtStats->getTextureIdent());
            }

            listRTOp.push_back(rt);
        }

	    D3D9State::instance().setRenderTarget(RenderTargetIndex, pRenderTarget);

        D3D9State::instance().resetRTOp();

    }

}

void D3D9Stats::registerGetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* oip_ppRenderTarget, IDirect3DSurface9** pRenderTarget){
	D3D9State::instance().getRenderTarget(RenderTargetIndex, oip_ppRenderTarget, pRenderTarget);
}

        
void D3D9Stats::registerSetDepthStencilSurface(IDirect3DSurface9 * sip_pNewZStencil) {

    D3D9RenderTargetOp* rtOp = D3D9State::instance().getDSOp();

    if (D3D9State::instance().getIdentifier(sip_pNewZStencil) != NULL &&
        D3D9State::instance().getTexIformRT(D3D9State::instance().getIdentifier(sip_pNewZStencil)) != 0) {

            if (rtOp != NULL) {

        if (rtOp->getClears() > 0) {
            D3D9RenderTargetOp* clear = new D3D9RenderTargetOp(rtOp->getRT());
            clear->setClearNode(true);
            clear->depthRT(true);
            listRTOp.push_back(clear);
            D3D9RenderTargetStats* rtStats = D3D9State::instance().getRenderTargetStatsIdent(rtOp->getRT());
            if (rtStats == NULL)
                rtOp->addTextOp(0);
            else
                rtOp->addTextOp(rtStats->getTextureIdent());
        }

                listRTOp.push_back(rtOp);
            }

            D3D9State::instance().setDepthStencilSurface(sip_pNewZStencil);
            D3D9State::instance().resetDSOp();

        /*D3D9RenderTargetOp* rtOp = new D3D9RenderTargetOp(D3D9State::instance().getIdentifier(sip_pNewZStencil));
        rtOp->depthRT(true);
        listRTOp.push_back(rtOp);*/
    }
    else {

        if (rtOp != NULL) {

        if (rtOp->getClears() > 0) {
            D3D9RenderTargetOp* clear = new D3D9RenderTargetOp(rtOp->getRT());
            clear->setClearNode(true);
            clear->depthRT(true);
            listRTOp.push_back(clear);
            D3D9RenderTargetStats* rtStats = D3D9State::instance().getRenderTargetStatsIdent(rtOp->getRT());
            if (rtStats == NULL)
                rtOp->addTextOp(0);
            else
                rtOp->addTextOp(rtStats->getTextureIdent());
        }
            listRTOp.push_back(rtOp);

            D3D9State::instance().setDepthStencilSurface(NULL);
            D3D9State::instance().resetDSOp();
        }

    }

}

/*void D3D9Stats::registerGetDepthStencilSurface(IDirect3DSurface9** spip_ppZStencilSurface) {
}*/

void D3D9Stats::registerGetSurfaceLevel(UINT sv_Level, IDirect3DSurface9* oip_ppSurfaceLevel, /*IDirect3DSurface9** spip_ppSurfaceLevel,*/ IDirect3DTexture9 * texture) {
	D3D9State::instance().getSurfaceLevel(sv_Level, oip_ppSurfaceLevel, /*spip_ppSurfaceLevel,*/ texture);
}

void D3D9Stats::registerCreateVertexBuffer(IDirect3DVertexBuffer9* sip_ppVertexBuffer, DWORD sv_FVF) {

    D3D9State::instance().addCreateVertexBuffer(sip_ppVertexBuffer, sv_FVF);

}

void D3D9Stats::registerSetStreamSource(UINT sv_StreamNumber, IDirect3DVertexBuffer9* sip_pStreamData ,UINT sv_Stride) {

    D3D9State::instance().addSetStreamSource(sv_StreamNumber, sip_pStreamData, sv_Stride);

}

void D3D9Stats::registerSetVertexDeclaration(IDirect3DVertexDeclaration9* sip_pDecl) {
    D3D9State::instance().addVertexDeclaration(sip_pDecl);
}

void D3D9Stats::registerCreateIndexBuffer(unsigned int oip_ppIndexBuffer, D3DFORMAT ov_Format, UINT Length) {
    #ifdef VERTEX_SHADER_STATS
        D3D9State::instance().addCreateIndexBuffer((void*)oip_ppIndexBuffer, ov_Format, Length);
    #endif
}

void D3D9Stats::registerSetIndices(unsigned int oip_pIndexData) {
    #ifdef VERTEX_SHADER_STATS
        D3D9State::instance().setIndexBuffer((void*)oip_pIndexData);
    #endif
}

void D3D9Stats::registerLockIndexBuffer(unsigned int oip_This, UINT ov_OffsetToLock, UINT ov_SizeToLock) {
    #ifdef VERTEX_SHADER_STATS
        D3D9State::instance().setLockIndexBuffer((void*)oip_This, ov_OffsetToLock, ov_SizeToLock);
    #endif
}

void D3D9Stats::registerUnlockIndexBuffer(unsigned int oip_This, D3DBuffer* buffer) {
    #ifdef VERTEX_SHADER_STATS
        D3D9State::instance().setUnlockIndexBuffer((void*)oip_This, buffer);
    #endif
}

void D3D9Stats::registerUPIndexVertex(void* IndexData, D3DFORMAT Format, UINT Bytes) {
    nextIsUP = true;
    UPindexs = IndexData;
    UPformat = Format;
    UPbytes = Bytes;
}

void D3D9Stats::registerSetRenderState(D3DRENDERSTATETYPE State , DWORD Value) {

    if (State == D3DRS_ALPHATESTENABLE) {
        if (Value == TRUE) {
            D3D9State::instance().setAlphaTest(true);
        }
        else {
            D3D9State::instance().setAlphaTest(false);
        }
    }
    else if (State == D3DRS_COLORWRITEENABLE) {
        D3D9State::instance().setColorWriteEnable(Value);
    }
    else if (State == D3DRS_ZWRITEENABLE) {
        if (Value == TRUE) {
            D3D9State::instance().setZWriteEnable(true);
        }
        else {
            D3D9State::instance().setZWriteEnable(false);
        }
    }

}

void D3D9Stats::registerClear(DWORD ov_Flags, DWORD ov_Count) {

    D3D9State::instance().clearRTOp(ov_Flags, ov_Count);

}

void D3D9Stats::registerStretchRect(IDirect3DSurface9* oip_pSourceSurface, IDirect3DSurface9* oip_pDestSurface, IDirect3DSurface9* sip_pDestSurface, RECT* spv_pDestRect) {

    ofstream f;

            f.open("rtLog.txt", ios::app);
            f << tmpNumFrame << ": StretchRect\n";
            f.close();

    D3D9RenderTargetOp* tmpRtOp;
    //D3D9RenderTargetOp* tmpSR;

    if (D3D9State::instance().isRTSet(oip_pSourceSurface)) {

        tmpRtOp = D3D9State::instance().getRTOp();

        if (tmpRtOp != NULL)
            listRTOp.push_back(tmpRtOp);

        tmpRtOp = D3D9State::instance().addTexToRTOp(oip_pSourceSurface, oip_pDestSurface/*, &tmpSR*/);

        D3D9State::instance().resetRTOp();

    }
    else {

        tmpRtOp = D3D9State::instance().addTexToRTOp(oip_pSourceSurface, oip_pDestSurface/*, &tmpSR*/);

    }

    /*if (tmpSR != NULL) {

        listRTOp.push_back(tmpSR);

    }*/

    if (tmpRtOp != NULL) {

        UINT64 spc;
        D3DSURFACE_DESC surfDesc;
        sip_pDestSurface->GetDesc(&surfDesc);

        if (spv_pDestRect == NULL) {
            spc = surfDesc.Width * surfDesc.Height;
            tmpRtOp->fullSR(true);
        }
        else {
            spc = (spv_pDestRect->right - spv_pDestRect->left) * (spv_pDestRect->bottom - spv_pDestRect->top);

            if ((spv_pDestRect->left == 0) && (spv_pDestRect->top == 0) && (spv_pDestRect->right == surfDesc.Width) && (spv_pDestRect->bottom == surfDesc.Height)) {
                tmpRtOp->fullSR(true);
            }
        }

        tmpRtOp->addNumPixelsDrawn(spc);
        listRTOp.push_back(tmpRtOp);
    }

}

void D3D9Stats::registerGetBackBuffer(IDirect3DSurface9* oip_ppBackBuffer, IDirect3DSurface9** spip_ppBackBuffer) {
    D3D9State::instance().getRenderTarget(0, oip_ppBackBuffer, spip_ppBackBuffer);
}

int D3D9Stats::getNumFrame() {
    return tmpNumFrame;
}

////////////////////////////////////////////////////////////
////////		D3D9TraceStats Definitions          ////////
////////////////////////////////////////////////////////////

D3D9TraceStats &D3D9TraceStats::instance() {
	static D3D9TraceStats inst;
	return inst;
}

#ifdef NVIDIA_STATS
int MyEnumFunc(UINT unCounterIndex, char *pcCounterName)
{
	char zString[200], zLine[400];
	unsigned int unLen;
	float fValue;
    FILE* f;

	unLen = 200;
	if(NVPMGetCounterDescription(unCounterIndex, zString, &unLen) == NVPM_OK) {
		sprintf(zLine, "Counter %d [%s] : ", unCounterIndex, zString);
	
		unLen = 200;
		if(NVPMGetCounterName(unCounterIndex, zString, &unLen) == NVPM_OK)
			strcat(zLine, zString); // Append the short name
		else
			strcat(zLine, "Error retrieving short name");

		NVPMGetCounterClockRate(zString, &fValue);
		sprintf(zString, " %.2f\n", fValue);
		strcat(zLine, zString);

        f = fopen("counters.txt", "a");
        fputs(zLine, f);
        fclose(f);
	}

	return(NVPM_OK);
}
#endif

D3D9TraceStats::D3D9TraceStats() : D3D9Stats() {

    i_buffer = 0;

    nextIsUP = false;
    UPindexs = 0;
    UPformat = D3DFMT_UNKNOWN;
    UPbytes = 0;
	
	#ifdef NVIDIA_STATS

		// Nvidia PerfAPI
		NVPMInit();

        //NVPMEnumCounters(MyEnumFunc);

		#ifdef SETUP_POINT_COUNT
			NVPMAddCounterByName("setup_point_count");
		#endif

		#ifdef SETUP_LINE_COUNT
			NVPMAddCounterByName("setup_line_count");
		#endif

		#ifdef SETUP_TRIANGLE_COUNT
			NVPMAddCounterByName("setup_triangle_count");
		#endif

		#ifdef SETUP_PRIMITIVE_COUNT
			NVPMAddCounterByName("setup_primitive_count");
		#endif

		#ifdef SETUP_PRIMITIVE_CULLED_COUNT
			NVPMAddCounterByName("setup_primitive_culled_count");
		#endif

		#ifdef ROP_SAMPLES_KILLED_BY_EARLYZ_COUNT
			NVPMAddCounterByName("rop_samples_killed_by_earlyz_count");
		#endif

		#ifdef ROP_SAMPLES_KILLED_BY_LATEZ_COUNT
			NVPMAddCounterByName("rop_samples_killed_by_latez_count");
		#endif

		#ifdef TEXTURE_WAITS_FOR_FB
			NVPMAddCounterByName("texture_waits_for_fb");
		#endif

		#ifdef RASTERIZER_TILES_IN_COUNT
			NVPMAddCounterByName("rasterizer_tiles_in_count");
		#endif

		#ifdef RASTERIZER_TILES_KILLED_BY_ZCULL_COUNT
			NVPMAddCounterByName("rasterizer_tiles_killed_by_zcull_count");
		#endif

		#ifdef SHADED_PIXEL_COUNT
			NVPMAddCounterByName("shaded_pixel_count");
		#endif

	#endif

	ff.open("frame.csv");

	#ifdef TITLE_STATS
		ff << "Frames;";
	#endif

	#ifdef GENERAL_STATS
		ff << "DrawPrimitives;primitiveCount;DrawPoint;DrawLine;DrawTriangleList;DrawTriangleStrip;DrawTriangleFan;Average Active Samplers;Active Samplers;AverageBatchSize;VerticesPerFrame;ApplyStateBlocks;";
	#endif

    #ifdef TEXTURE_STATS
		ff << "NEAREST;NEAREST_MIPMAP_LINEAR;BILINEAR;BILINEAR_ANISO 2x;BILINEAR_ANISO 4x;BILINEAR_ANISO 8x;BILINEAR_ANISO 16x;TRILINEAR;TRILINEAR_ANISO 2x;TRILINEAR_ANISO 4x;TRILINEAR_ANISO 8x;TRILINEAR_ANISO 16x;UNKNOWN;";
	#endif

	/*#ifdef TEXTURE_STATS
		ff << "NEAREST;NEAREST_MIPMAP_LINEAR;BILINEAR;BILINEAR_ANISO;TRILINEAR;TRILINEAR_ANISO;UNKNOWN;";
	#endif*/

	#ifdef VERTEX_SHADER_STATS
		ff << "DEF;DEFI;DEFB;DCL;PS;LABEL;NOP;PHASE;CALL;CALLNZ;ELSE;ENDIF;ENDLOOP;ENDREP;IF;LOOP;REP;RET;ADD;BEM;CMP;CND;DP3;DP4;LRP;MAD;MOV;MUL;SUB;TEX;TEXBEM;TEXBEML;TEXCOORD;TEXCRD;TEXDEPTH;TEXDP3;TEXDP3TEX;TEXKILL;TEXLD;TEXM3X2DEPTH;TEXM3X2PAD;TEXM3X2TEX;TEXM3X3;TEXM3X3PAD;TEXM3X3TEX;TEXM3X3VSPEC;TEXREG2AR;TEXREG2GB;TEXREG2RGB;ABS;EXP;FRC;LOG;RCP;RSQ;TEXLDB;TEXLDP;TEXLDD;TEXLDL;TEXM3X3SPEC;DST;LIT;LOGP;M3x2;M3x3;M3x4;M4x3;M4x4;MAX;MIN;MOVA;SGE;SLT;CRS;NRM;POW;SGN;SINCOS;DP2ADD;BREAK;BREAKP;SETP;DSX;DSY;OTHER;";
		ff << "shaderSlots;pixelShaderSlots;vertexShaderSlots;executed slots;ps_1_0;ps_1_1;ps_1_2;ps_1_3;ps_1_4;ps_2_0;ps_2_x;ps_2_sw;ps_3_0;ps_3_sw;vs_1_1;vs_2_0;vs_2_x;vs_2_sw;vs_3_0;vs_3_sw;textureInstructionsAv;differentShader;";
        ff << "vectorInstructions;scalarInstructions;specialInstructions;specialD3D9Instructions;textureInstructions;Depth changed in pixel shader;";
        ff << "Shaded Vertices Cache 16;Shaded Vertices Cache 32;Shaded Vertices Cache 48;Shaded Vertices Cache Inf.;Attribute number; Attibute size; Attribute number per vertex; Attibute size per vertex;Attribute number per shaded vertex; Attibute size per shaded vertex;";
    #endif

    #ifdef PIXEL_SHADER_STATS
		ff << "DEF;DEFI;DEFB;DCL;PS;LABEL;NOP;PHASE;CALL;CALLNZ;ELSE;ENDIF;ENDLOOP;ENDREP;IF;LOOP;REP;RET;ADD;BEM;CMP;CND;DP3;DP4;LRP;MAD;MOV;MUL;SUB;TEX;TEXBEM;TEXBEML;TEXCOORD;TEXCRD;TEXDEPTH;TEXDP3;TEXDP3TEX;TEXKILL;TEXLD;TEXM3X2DEPTH;TEXM3X2PAD;TEXM3X2TEX;TEXM3X3;TEXM3X3PAD;TEXM3X3TEX;TEXM3X3VSPEC;TEXREG2AR;TEXREG2GB;TEXREG2RGB;ABS;EXP;FRC;LOG;RCP;RSQ;TEXLDB;TEXLDP;TEXLDD;TEXLDL;TEXM3X3SPEC;DST;LIT;LOGP;M3x2;M3x3;M3x4;M4x3;M4x4;MAX;MIN;MOVA;SGE;SLT;CRS;NRM;POW;SGN;SINCOS;DP2ADD;BREAK;BREAKP;SETP;DSX;DSY;OTHER;";
		ff << "shaderSlots;pixelShaderSlots;vertexShaderSlots;executed slots;ps_1_0;ps_1_1;ps_1_2;ps_1_3;ps_1_4;ps_2_0;ps_2_x;ps_2_sw;ps_3_0;ps_3_sw;vs_1_1;vs_2_0;vs_2_x;vs_2_sw;vs_3_0;vs_3_sw;textureInstructionsAv;differentShader;";
        ff << "vectorInstructions;scalarInstructions;specialInstructions;specialD3D9Instructions;textureInstructions;Depth changed in pixel shader;";
	#endif

	#ifdef TEXTURE_STATS
		ff << "differentTexture;TextureReuse;Size;noPot2U;Compressed Texture;8 bits Texture;16 bits Texture;24 bits Texture;32 bits Texture;64 bits Texture;128 bits Texture;Vertex Data Texture;NULL Texture;DEFAULT;MANAGED;SYSTEMMEM;SCRATCH;";
        ff << "T S_D3DFMT_R8G8B8;T S_D3DFMT_A8R8G8B8;T S_D3DFMT_X8R8G8B8;T S_D3DFMT_R5G6B5;T S_D3DFMT_X1R5G5B5;T S_D3DFMT_A1R5G5B5;T S_D3DFMT_A4R4G4B4;T S_D3DFMT_R3G3B2;T S_D3DFMT_A8;T S_D3DFMT_A8R3G3B2;";
        ff << "T S_D3DFMT_X4R4G4B4;T S_D3DFMT_A2B10G10R10;T S_D3DFMT_A8B8G8R8;T S_D3DFMT_X8B8G8R8;T S_D3DFMT_G16R16;T S_D3DFMT_A2R10G10B10;T S_D3DFMT_A16B16G16R16;T S_D3DFMT_A8P8;T S_D3DFMT_P8;T S_D3DFMT_L8;";
        ff << "T S_D3DFMT_A8L8;T S_D3DFMT_A4L4;T S_D3DFMT_V8U8;T S_D3DFMT_L6V5U5;T S_D3DFMT_X8L8V8U8;T S_D3DFMT_Q8W8V8U8;T S_D3DFMT_V16U16;T S_D3DFMT_A2W10V10U10;T S_D3DFMT_UYVY;T S_D3DFMT_R8G8_B8G8;";
        ff << "T S_D3DFMT_YUY2;T S_D3DFMT_G8R8_G8B8;T S_D3DFMT_DXT1;T S_D3DFMT_DXT2;T S_D3DFMT_DXT3;T S_D3DFMT_DXT4;T S_D3DFMT_DXT5;T S_D3DFMT_D16_LOCKABLE;T S_D3DFMT_D32;T S_D3DFMT_D15S1;T S_D3DFMT_D24S8;";
        ff << "T S_D3DFMT_D24X8;T S_D3DFMT_D24X4S4;T S_D3DFMT_D16;T S_D3DFMT_D32F_LOCKABLE;T S_D3DFMT_D24FS8;T S_D3DFMT_L16;T S_D3DFMT_VERTEXDATA;T S_D3DFMT_INDEX16;T S_D3DFMT_INDEX32;T S_D3DFMT_Q16W16V16U16;";
        ff << "T S_D3DFMT_MULTI2_ARGB8;T S_D3DFMT_R16F;T S_D3DFMT_G16R16F;T S_D3DFMT_A16B16G16R16F;T S_D3DFMT_R32F;T S_D3DFMT_G32R32F;T S_D3DFMT_A32B32G32R32F;T S_D3DFMT_CxV8U8;T S_D3DFMT_ATI2;T S_D3DFMT_NULL;";
	#endif

	#ifdef RENDERTARGET_STATS
		ff << "Render Target setted;Render Target width;Render Target height;Render Target size;numRenderTargetTexture;numDiffRenderTargetTexture;Compressed Render Target;8 bits Render Target;16 bits Render Target;24 bits Render Target;32 bits Render Target;64 bits Render Target;128 bits Render Target;Vertex Data Render Target;NULL Render Target;";
        ff << "RT S_D3DFMT_R8G8B8;RT S_D3DFMT_A8R8G8B8;RT S_D3DFMT_X8R8G8B8;RT S_D3DFMT_R5G6B5;RT S_D3DFMT_X1R5G5B5;RT S_D3DFMT_A1R5G5B5;RT S_D3DFMT_A4R4G4B4;RT S_D3DFMT_R3G3B2;RT S_D3DFMT_A8;RT S_D3DFMT_A8R3G3B2;";
        ff << "RT S_D3DFMT_X4R4G4B4;RT S_D3DFMT_A2B10G10R10;RT S_D3DFMT_A8B8G8R8;RT S_D3DFMT_X8B8G8R8;RT S_D3DFMT_G16R16;RT S_D3DFMT_A2R10G10B10;RT S_D3DFMT_A16B16G16R16;RT S_D3DFMT_A8P8;RT S_D3DFMT_P8;RT S_D3DFMT_L8;";
        ff << "RT S_D3DFMT_A8L8;RT S_D3DFMT_A4L4;RT S_D3DFMT_V8U8;RT S_D3DFMT_L6V5U5;RT S_D3DFMT_X8L8V8U8;RT S_D3DFMT_Q8W8V8U8;RT S_D3DFMT_V16U16;RT S_D3DFMT_A2W10V10U10;RT S_D3DFMT_UYVY;RT S_D3DFMT_R8G8_B8G8;";
        ff << "RT S_D3DFMT_YUY2;RT S_D3DFMT_G8R8_G8B8;RT S_D3DFMT_DXT1;RT S_D3DFMT_DXT2;RT S_D3DFMT_DXT3;RT S_D3DFMT_DXT4;RT S_D3DFMT_DXT5;RT S_D3DFMT_D16_LOCKABLE;RT S_D3DFMT_D32;RT S_D3DFMT_D15S1;RT S_D3DFMT_D24S8;";
        ff << "RT S_D3DFMT_D24X8;RT S_D3DFMT_D24X4S4;RT S_D3DFMT_D16;RT S_D3DFMT_D32F_LOCKABLE;RT S_D3DFMT_D24FS8;RT S_D3DFMT_L16;RT S_D3DFMT_VERTEXDATA;RT S_D3DFMT_INDEX16;RT S_D3DFMT_INDEX32;RT S_D3DFMT_Q16W16V16U16;";
        ff << "RT S_D3DFMT_MULTI2_ARGB8;RT S_D3DFMT_R16F;RT S_D3DFMT_G16R16F;RT S_D3DFMT_A16B16G16R16F;RT S_D3DFMT_R32F;RT S_D3DFMT_G32R32F;RT S_D3DFMT_A32B32G32R32F;RT S_D3DFMT_CxV8U8;RT S_D3DFMT_ATI2;RT S_D3DFMT_NULL;";
	#endif

	#ifdef NVIDIA_STATS

		#ifdef SETUP_POINT_COUNT
			ff << "setup_point_count;";
		#endif

		#ifdef SETUP_LINE_COUNT
			ff << "setup_line_count;";
		#endif

		#ifdef SETUP_TRIANGLE_COUNT
			ff << "setup_triangle_count;";
		#endif

		#ifdef SETUP_PRIMITIVE_COUNT
			ff << "setup_primitive_count;";
		#endif

		#ifdef SETUP_PRIMITIVE_CULLED_COUNT
			ff << "setup_primitive_culled_count;";
		#endif

		#ifdef ROP_SAMPLES_KILLED_BY_EARLYZ_COUNT
			ff << "rop_samples_killed_by_earlyz_count;";
		#endif

		#ifdef ROP_SAMPLES_KILLED_BY_LATEZ_COUNT
			ff << "rop_samples_killed_by_latez_count;";
		#endif

		#ifdef TEXTURE_WAITS_FOR_FB
			ff << "texture_waits_for_fb;";
		#endif

		#ifdef RASTERIZER_TILES_IN_COUNT
			ff << "rasterizer_tiles_in_count;";
		#endif

		#ifdef RASTERIZER_TILES_KILLED_BY_ZCULL_COUNT
			ff << "rasterizer_tiles_killed_by_zcull_count;";
		#endif
       
		#ifdef SHADED_PIXEL_COUNT
			ff << "shaded_pixel_count;";
		#endif

	#endif

	#ifdef NEW_STATS
		ff << "numAlpha;" ;
	#endif

    #ifdef OCCLUSION_STATS
        ff << "numPixelsDrawn;";
    #endif

    #ifdef TEXTURE_STATS
        ff << "numCreateTexture;";
    #endif

	ff << "\n";



	#ifdef BATCH_STATS
		fb.open("batch.csv");

		#ifdef TITLE_STATS
			fb << "Frames;Batch;";
		#endif

		#ifdef GENERAL_STATS
			fb << "DrawPrimitives;primitiveCount;DrawPoint;DrawLine;DrawTriangleList;DrawTriangleStrip;DrawTriangleFan;Average Active Samplers;Active Samplers;AverageBatchSize;VerticesPerFrame;ApplyStateBlocks;";
		#endif

        #ifdef TEXTURE_STATS
			fb << "NEAREST;NEAREST_MIPMAP_LINEAR;BILINEAR;BILINEAR_ANISO 2x;BILINEAR_ANISO 4x;BILINEAR_ANISO 8x;BILINEAR_ANISO 16x;TRILINEAR;TRILINEAR_ANISO 2x;TRILINEAR_ANISO 4x;TRILINEAR_ANISO 8x;TRILINEAR_ANISO 16x;UNKNOWN;";
		#endif

		/*#ifdef TEXTURE_STATS
			fb << "NEAREST;NEAREST_MIPMAP_LINEAR;BILINEAR;BILINEAR_ANISO;TRILINEAR;TRILINEAR_ANISO;UNKNOWN;";
		#endif*/

		#ifdef VERTEX_SHADER_STATS
			fb << "DEF;DEFI;DEFB;DCL;PS;LABEL;NOP;PHASE;CALL;CALLNZ;ELSE;ENDIF;ENDLOOP;ENDREP;IF;LOOP;REP;RET;ADD;BEM;CMP;CND;DP3;DP4;LRP;MAD;MOV;MUL;SUB;TEX;TEXBEM;TEXBEML;TEXCOORD;TEXCRD;TEXDEPTH;TEXDP3;TEXDP3TEX;TEXKILL;TEXLD;TEXM3X2DEPTH;TEXM3X2PAD;TEXM3X2TEX;TEXM3X3;TEXM3X3PAD;TEXM3X3TEX;TEXM3X3VSPEC;TEXREG2AR;TEXREG2GB;TEXREG2RGB;ABS;EXP;FRC;LOG;RCP;RSQ;TEXLDB;TEXLDP;TEXLDD;TEXLDL;TEXM3X3SPEC;DST;LIT;LOGP;M3x2;M3x3;M3x4;M4x3;M4x4;MAX;MIN;MOVA;SGE;SLT;CRS;NRM;POW;SGN;SINCOS;DP2ADD;BREAK;BREAKP;SETP;DSX;DSY;OTHER;";
			fb << "shaderSlots;pixelShaderSlots;vertexShaderSlots;executed slots;ps_1_0;ps_1_1;ps_1_2;ps_1_3;ps_1_4;ps_2_0;ps_2_x;ps_2_sw;ps_3_0;ps_3_sw;vs_1_1;vs_2_0;vs_2_x;vs_2_sw;vs_3_0;vs_3_sw;textureInstructions;differentShader;";   
            fb << "vectorInstructions;scalarInstructions;specialInstructions;specialD3D9Instructions;textureInstructions;Depth changed in pixel shader;";
            fb << "Shaded Vertices Cache 16;Shaded Vertices Cache 32;Shaded Vertices Cache 48;Shaded Vertices Cache Inf.;Attribute number; Attibute size; Attribute number per vertex; Attibute size per vertex;Attribute number per shaded vertex; Attibute size per shaded vertex;";
       #endif

        #ifdef PIXEL_SHADER_STATS
			fb << "DEF;DEFI;DEFB;DCL;PS;LABEL;NOP;PHASE;CALL;CALLNZ;ELSE;ENDIF;ENDLOOP;ENDREP;IF;LOOP;REP;RET;ADD;BEM;CMP;CND;DP3;DP4;LRP;MAD;MOV;MUL;SUB;TEX;TEXBEM;TEXBEML;TEXCOORD;TEXCRD;TEXDEPTH;TEXDP3;TEXDP3TEX;TEXKILL;TEXLD;TEXM3X2DEPTH;TEXM3X2PAD;TEXM3X2TEX;TEXM3X3;TEXM3X3PAD;TEXM3X3TEX;TEXM3X3VSPEC;TEXREG2AR;TEXREG2GB;TEXREG2RGB;ABS;EXP;FRC;LOG;RCP;RSQ;TEXLDB;TEXLDP;TEXLDD;TEXLDL;TEXM3X3SPEC;DST;LIT;LOGP;M3x2;M3x3;M3x4;M4x3;M4x4;MAX;MIN;MOVA;SGE;SLT;CRS;NRM;POW;SGN;SINCOS;DP2ADD;BREAK;BREAKP;SETP;DSX;DSY;OTHER;";
			fb << "shaderSlots;pixelShaderSlots;vertexShaderSlots;executed slots;ps_1_0;ps_1_1;ps_1_2;ps_1_3;ps_1_4;ps_2_0;ps_2_x;ps_2_sw;ps_3_0;ps_3_sw;vs_1_1;vs_2_0;vs_2_x;vs_2_sw;vs_3_0;vs_3_sw;textureInstructions;differentShader;";   
            fb << "vectorInstructions;scalarInstructions;specialInstructions;specialD3D9Instructions;textureInstructions;Depth changed in pixel shader;";
		#endif

		#ifdef TEXTURE_STATS
			fb << "differentTexture;TextureReuse;Size;noPot2U;Compressed Texture;8 bits Texture;16 bits Texture;24 bits Texture;32 bits Texture;64 bits Texture;128 bits Texture;Vertex Data Texture;DEFAULT;MANAGED;SYSTEMMEM;SCRATCH;";
            fb << "T S_D3DFMT_R8G8B8;T S_D3DFMT_A8R8G8B8;T S_D3DFMT_X8R8G8B8;T S_D3DFMT_R5G6B5;T S_D3DFMT_X1R5G5B5;T S_D3DFMT_A1R5G5B5;T S_D3DFMT_A4R4G4B4;T S_D3DFMT_R3G3B2;T S_D3DFMT_A8;T S_D3DFMT_A8R3G3B2;";
            fb << "T S_D3DFMT_X4R4G4B4;T S_D3DFMT_A2B10G10R10;T S_D3DFMT_A8B8G8R8;T S_D3DFMT_X8B8G8R8;T S_D3DFMT_G16R16;T S_D3DFMT_A2R10G10B10;T S_D3DFMT_A16B16G16R16;T S_D3DFMT_A8P8;T S_D3DFMT_P8;T S_D3DFMT_L8;";
            fb << "T S_D3DFMT_A8L8;T S_D3DFMT_A4L4;T S_D3DFMT_V8U8;T S_D3DFMT_L6V5U5;T S_D3DFMT_X8L8V8U8;T S_D3DFMT_Q8W8V8U8;T S_D3DFMT_V16U16;T S_D3DFMT_A2W10V10U10;T S_D3DFMT_UYVY;T S_D3DFMT_R8G8_B8G8;";
            fb << "T S_D3DFMT_YUY2;T S_D3DFMT_G8R8_G8B8;T S_D3DFMT_DXT1;T S_D3DFMT_DXT2;T S_D3DFMT_DXT3;T S_D3DFMT_DXT4;T S_D3DFMT_DXT5;T S_D3DFMT_D16_LOCKABLE;T S_D3DFMT_D32;T S_D3DFMT_D15S1;T S_D3DFMT_D24S8;";
            fb << "T S_D3DFMT_D24X8;T S_D3DFMT_D24X4S4;T S_D3DFMT_D16;T S_D3DFMT_D32F_LOCKABLE;T S_D3DFMT_D24FS8;T S_D3DFMT_L16;T S_D3DFMT_VERTEXDATA;T S_D3DFMT_INDEX16;T S_D3DFMT_INDEX32;T S_D3DFMT_Q16W16V16U16;";
            fb << "T S_D3DFMT_MULTI2_ARGB8;T S_D3DFMT_R16F;T S_D3DFMT_G16R16F;T S_D3DFMT_A16B16G16R16F;T S_D3DFMT_R32F;T S_D3DFMT_G32R32F;T S_D3DFMT_A32B32G32R32F;T S_D3DFMT_CxV8U8;T S_D3DFMT_ATI2;T S_D3DFMT_NULL;";
        #endif

		#ifdef RENDERTARGET_STATS
			fb << "Render Target setted;Render Target width; Render Target height;Render Target size;numRenderTargetTexture;numDiffRenderTargetTexture;Compressed Render Target;8 bits Render Target;16 bits Render Target;24 bits Render Target;32 bits Render Target;64 bits Render Target;128 bits Render Target;Vertex Data Render Target;";
            fb << "RT S_D3DFMT_R8G8B8;RT S_D3DFMT_A8R8G8B8;RT S_D3DFMT_X8R8G8B8;RT S_D3DFMT_R5G6B5;RT S_D3DFMT_X1R5G5B5;RT S_D3DFMT_A1R5G5B5;RT S_D3DFMT_A4R4G4B4;RT S_D3DFMT_R3G3B2;RT S_D3DFMT_A8;RT S_D3DFMT_A8R3G3B2;";
            fb << "RT S_D3DFMT_X4R4G4B4;RT S_D3DFMT_A2B10G10R10;RT S_D3DFMT_A8B8G8R8;RT S_D3DFMT_X8B8G8R8;RT S_D3DFMT_G16R16;RT S_D3DFMT_A2R10G10B10;RT S_D3DFMT_A16B16G16R16;RT S_D3DFMT_A8P8;RT S_D3DFMT_P8;RT S_D3DFMT_L8;";
            fb << "RT S_D3DFMT_A8L8;RT S_D3DFMT_A4L4;RT S_D3DFMT_V8U8;RT S_D3DFMT_L6V5U5;RT S_D3DFMT_X8L8V8U8;RT S_D3DFMT_Q8W8V8U8;RT S_D3DFMT_V16U16;RT S_D3DFMT_A2W10V10U10;RT S_D3DFMT_UYVY;RT S_D3DFMT_R8G8_B8G8;";
            fb << "RT S_D3DFMT_YUY2;RT S_D3DFMT_G8R8_G8B8;RT S_D3DFMT_DXT1;RT S_D3DFMT_DXT2;RT S_D3DFMT_DXT3;RT S_D3DFMT_DXT4;RT S_D3DFMT_DXT5;RT S_D3DFMT_D16_LOCKABLE;RT S_D3DFMT_D32;RT S_D3DFMT_D15S1;RT S_D3DFMT_D24S8;";
            fb << "RT S_D3DFMT_D24X8;RT S_D3DFMT_D24X4S4;RT S_D3DFMT_D16;RT S_D3DFMT_D32F_LOCKABLE;RT S_D3DFMT_D24FS8;RT S_D3DFMT_L16;RT S_D3DFMT_VERTEXDATA;RT S_D3DFMT_INDEX16;RT S_D3DFMT_INDEX32;RT S_D3DFMT_Q16W16V16U16;";
            fb << "RT S_D3DFMT_MULTI2_ARGB8;RT S_D3DFMT_R16F;RT S_D3DFMT_G16R16F;RT S_D3DFMT_A16B16G16R16F;RT S_D3DFMT_R32F;RT S_D3DFMT_G32R32F;RT S_D3DFMT_A32B32G32R32F;RT S_D3DFMT_CxV8U8;RT S_D3DFMT_ATI2;RT S_D3DFMT_NULL;";
        #endif

		#ifdef NVIDIA_STATS

			#ifdef SETUP_POINT_COUNT
				fb << "setup_point_count;";
			#endif

			#ifdef SETUP_LINE_COUNT
				fb << "setup_line_count;";
			#endif

			#ifdef SETUP_TRIANGLE_COUNT
				fb << "setup_triangle_count;";
			#endif

			#ifdef SETUP_PRIMITIVE_COUNT
				fb << "setup_primitive_count;";
			#endif

			#ifdef SETUP_PRIMITIVE_CULLED_COUNT
				fb << "setup_primitive_culled_count;";
			#endif

			#ifdef ROP_SAMPLES_KILLED_BY_EARLYZ_COUNT
				fb << "rop_samples_killed_by_earlyz_count;";
			#endif

			#ifdef ROP_SAMPLES_KILLED_BY_LATEZ_COUNT
				fb << "rop_samples_killed_by_latez_count;";
			#endif

			#ifdef TEXTURE_WAITS_FOR_FB
				fb << "texture_waits_for_fb;";
			#endif

			#ifdef RASTERIZER_TILES_IN_COUNT
				fb << "rasterizer_tiles_in_count;";
			#endif

			#ifdef RASTERIZER_TILES_KILLED_BY_ZCULL_COUNT
				fb << "rasterizer_tiles_killed_by_zcull_count;";
			#endif

	    	#ifdef SHADED_PIXEL_COUNT
	    		fb << "shaded_pixel_count;";
	    	#endif

		#endif

		#ifdef NEW_STATS
			fb << "numAlpha;" ;
		#endif

        #ifdef OCCLUSION_STATS
            fb << "numPixelsDrawn;";
        #endif

        #ifdef TEXTURE_STATS
            fb << "numCreateTexture;";
        #endif

		fb << "\n";

		frame.setStadisticsFiles(&ff, &fb);

	#else
		frame.setStadisticsFiles(&ff);
	#endif
    tmpNumFrame = 0;

    //------------------------------------------------------------
    char ps_source[] =
        "ps_2_0\n"
        "def c0, 0, 0, 0, 0.5\n"
        "dcl t0\n"
        "add r0, t0, c0\n"
        "mov oC0, r0\n";

    /*char ps_source[] =
        "ps_2_0\n"
        "dcl t0\n"
        "mov oC0, v0\n";*/

    ID3DXBuffer* i_errors = 0;

    D3DXAssembleShader(ps_source, strlen(ps_source), 0, 0, 0, &i_buffer, &i_errors);
    if(i_errors != 0) {
        i_errors->Release();
    }

	/*if(g_pd3dDevice->CreateVertexShader((DWORD*)i_buffer->GetBufferPointer(), &g_pVSH) != D3D_OK) {
			MessageBox(0,"Error creating vs", 0, 0);
			exit(-1);
	}*/
	//i_buffer->Release();
}

ID3DXBuffer* D3D9Stats::getPSBuffer() {
    return i_buffer;
}

void D3D9FrameStats::beginBatch() {}

void D3D9TraceStats::endBatch () {
	frame.endBatch();
}

void D3D9TraceStats::endFrame() {

    D3D9RenderTargetOp* rt = D3D9State::instance().getRTOp();

    if (rt != NULL) 
        listRTOp.push_back(rt);

    D3D9State::instance().resetRTOp();

	ofstream frt;
    char filename[50];
    sprintf(filename, "rt-%u.txt", tmpNumFrame);
	frt.open(filename);

    std::list<D3D9RenderTargetOp*>::iterator itRTOp;
    std::list<D3D9RenderTargetOp*>::iterator itRTOp2;
    std::list<unsigned int>::iterator itOpList;

    for (itRTOp = listRTOp.begin(); itRTOp != listRTOp.end(); itRTOp++) {
        (*itRTOp)->save(&frt);
    }

    frt.close();

    acdlib::acdlib_opt::DependencyGraph* g = new acdlib::acdlib_opt::DependencyGraph(listRTOp.size());

    unsigned int i = 0;
    for (itRTOp = listRTOp.begin(); itRTOp != listRTOp.end(); itRTOp++) {

        /*if (D3D9State::instance().isRTBackBuffer((*itRTOp)->getRT()))
            g->insertInstructionInfoRectangle(i, (*itRTOp)->getString(), 1);
        else 
            g->insertInstructionInfo(i, (*itRTOp)->getString(), 1);

        if ((*itRTOp)->isStretchRect())
            g->setStretchRect(i);*/

    
        g->insertInstructionInfo(i, (*itRTOp)->getString(), /*(*itRTOp)->getDraws()*/1);
        g->extraProperties(i, D3D9State::instance().isRTBackBuffer((*itRTOp)->getRT()), 
            (*itRTOp)->isStretchRect(), (*itRTOp)->getPixelsDrawn(), (*itRTOp)->getExtraDescription(), 
            (*itRTOp)->getColorWriteEnable(), (*itRTOp)->isZST(), (*itRTOp)->getSize(), (*itRTOp)->getClearNode());


        i++;
    }
    
    unsigned int nodeW = 0;
    unsigned int nodeR = 0;

    for (itRTOp = listRTOp.begin(); itRTOp != listRTOp.end(); itRTOp++) {

        unsigned int w = D3D9State::instance().getTexIformRT((*itRTOp)->getRT());
        unsigned int wRT = (*itRTOp)->getRT();
        nodeR = 0;
        bool srFound = false;
        bool lastSR = false;

        for (itRTOp2 = listRTOp.begin(); itRTOp2 != listRTOp.end(); itRTOp2++) {

            if ((((*itRTOp2)->getRT()) == wRT) && (nodeR > nodeW)) {
                
                if ((*itRTOp)->getClearNode()) {
                        //g->insertTrueDependency(nodeW, nodeR);
                }
                /*else if ((*itRTOp2)->isStretchRect()) {

                    //if (nodeW - 1 == nodeR) {
                        g->insertTrueDependency(nodeW, nodeR);
                        srFound = true;
                    //}

                }*/
                else {

                    if (/*(!(*itRTOp)->isStretchRect()) && */(!(*itRTOp)->getClearNode()) && (!(*itRTOp2)->getClearNode()) && (!(*itRTOp2)->getFullSR()))
                        g->insertOutputDependency(nodeW, nodeR);

                    break;

                }

            }

            if (!srFound){

                std::list<unsigned int> opList;
                opList = (*itRTOp2)->getOpList();

                for (itOpList = opList.begin(); itOpList != opList.end(); itOpList++) {

                    if ((nodeR > nodeW) && ((*itOpList) == w)) {

                        if (/*(*itRTOp)->isStretchRect() ||*/ (*itRTOp)->getClearNode()) {

                            if (nodeW + 1 == nodeR)
                                g->insertTrueDependency(nodeW, nodeR);

                        }
                        else {
                            //if (!lastSR)
                                g->insertTrueDependency(nodeW, nodeR);
                        }

                    }

                }

            }

            /*if ((*itRTOp2)->isStretchRect())
                lastSR = true;
            else
                lastSR = false;*/


            nodeR++;
        }

        nodeW++;
    }

    sprintf(filename, "rt-%u.dot", tmpNumFrame);
	frt.open(filename);

    g->groupNodes(true);
    g->clusterGraph(false);
    g->printDOTFormat(frt);
    frt.close();

    sprintf(filename, "rt-cluster-%u.dot", tmpNumFrame);
	frt.open(filename);

    g->groupNodes(true);
    g->clusterGraph(true);
    g->printDOTFormat(frt);
    frt.close();


    for (itRTOp = listRTOp.begin(); itRTOp != listRTOp.end(); itRTOp++)
        delete (*itRTOp);

    listRTOp.clear();

    delete g;


	stadistics.FrameOpStatsCollector(frame.getStadistics());
	frame.writeStadistics();
	frame.endFrame();
    tmpNumFrame++;

}

bool D3D9TraceStats::isTrace() {
	return true;
}

D3D9FrameStats* D3D9TraceStats::getBackFrame() {
	return &frame;
}

#ifdef BATCH_STATS
	D3D9BatchStats* D3D9FrameStats::getBackBatch() {
		return &batch;
	}
#endif

void D3D9TraceStats::endTrace() {

	ofstream ft;
	ft.open("trace.csv");

	#ifdef GENERAL_STATS
		ft << "DrawPrimitives;primitiveCount;DrawPoint;DrawLine;DrawTriangleList;DrawTriangleStrip;DrawTriangleFan;Average Active Samplers;Active Samplers;AverageBatchSize;VerticesPerFrame;ApplyStateBlocks;";
	#endif

	#ifdef TEXTURE_STATS
		ft << "NEAREST;NEAREST_MIPMAP_LINEAR;BILINEAR;BILINEAR_ANISO 2x;BILINEAR_ANISO 4x;BILINEAR_ANISO 8x;BILINEAR_ANISO 16x;TRILINEAR;TRILINEAR_ANISO 2x;TRILINEAR_ANISO 4x;TRILINEAR_ANISO 8x;TRILINEAR_ANISO 16x;UNKNOWN;";
	#endif
        
	/*#ifdef TEXTURE_STATS
		ft << "NEAREST;NEAREST_MIPMAP_LINEAR;BILINEAR;BILINEAR_ANISO;TRILINEAR;TRILINEAR_ANISO;UNKNOWN;";
	#endif*/

	#ifdef VERTEX_SHADER_STATS
		ft << "DEF;DEFI;DEFB;DCL;PS;LABEL;NOP;PHASE;CALL;CALLNZ;ELSE;ENDIF;ENDLOOP;ENDREP;IF;LOOP;REP;RET;ADD;BEM;CMP;CND;DP3;DP4;LRP;MAD;MOV;MUL;SUB;TEX;TEXBEM;TEXBEML;TEXCOORD;TEXCRD;TEXDEPTH;TEXDP3;TEXDP3TEX;TEXKILL;TEXLD;TEXM3X2DEPTH;TEXM3X2PAD;TEXM3X2TEX;TEXM3X3;TEXM3X3PAD;TEXM3X3TEX;TEXM3X3VSPEC;TEXREG2AR;TEXREG2GB;TEXREG2RGB;ABS;EXP;FRC;LOG;RCP;RSQ;TEXLDB;TEXLDP;TEXLDD;TEXLDL;TEXM3X3SPEC;DST;LIT;LOGP;M3x2;M3x3;M3x4;M4x3;M4x4;MAX;MIN;MOVA;SGE;SLT;CRS;NRM;POW;SGN;SINCOS;DP2ADD;BREAK;BREAKP;SETP;DSX;DSY;OTHER;";
		ft << "shaderSlots;pixelShaderSlots;vertexShaderSlots;executed slots;ps_1_0;ps_1_1;ps_1_2;ps_1_3;ps_1_4;ps_2_0;ps_2_x;ps_2_sw;ps_3_0;ps_3_sw;vs_1_1;vs_2_0;vs_2_x;vs_2_sw;vs_3_0;vs_3_sw;textureInstructions;differentShader;";   
        ft << "vectorInstructions;scalarInstructions;specialInstructions;specialD3D9Instructions;textureInstructions;Depth changed in pixel shader;";
        ft << "Shaded Vertices Cache 16;Shaded Vertices Cache 32;Shaded Vertices Cache 48;Shaded Vertices Cache Inf.;Attribute number; Attibute size; Attribute number per vertex; Attibute size per vertex;Attribute number per shaded vertex; Attibute size per shaded vertex;";
    #endif

    #ifdef PIXEL_SHADER_STATS
		ft << "DEF;DEFI;DEFB;DCL;PS;LABEL;NOP;PHASE;CALL;CALLNZ;ELSE;ENDIF;ENDLOOP;ENDREP;IF;LOOP;REP;RET;ADD;BEM;CMP;CND;DP3;DP4;LRP;MAD;MOV;MUL;SUB;TEX;TEXBEM;TEXBEML;TEXCOORD;TEXCRD;TEXDEPTH;TEXDP3;TEXDP3TEX;TEXKILL;TEXLD;TEXM3X2DEPTH;TEXM3X2PAD;TEXM3X2TEX;TEXM3X3;TEXM3X3PAD;TEXM3X3TEX;TEXM3X3VSPEC;TEXREG2AR;TEXREG2GB;TEXREG2RGB;ABS;EXP;FRC;LOG;RCP;RSQ;TEXLDB;TEXLDP;TEXLDD;TEXLDL;TEXM3X3SPEC;DST;LIT;LOGP;M3x2;M3x3;M3x4;M4x3;M4x4;MAX;MIN;MOVA;SGE;SLT;CRS;NRM;POW;SGN;SINCOS;DP2ADD;BREAK;BREAKP;SETP;DSX;DSY;OTHER;";
		ft << "shaderSlots;pixelShaderSlots;vertexShaderSlots;executed slots;ps_1_0;ps_1_1;ps_1_2;ps_1_3;ps_1_4;ps_2_0;ps_2_x;ps_2_sw;ps_3_0;ps_3_sw;vs_1_1;vs_2_0;vs_2_x;vs_2_sw;vs_3_0;vs_3_sw;textureInstructions;differentShader;";   
        ft << "vectorInstructions;scalarInstructions;specialInstructions;specialD3D9Instructions;textureInstructions;Depth changed in pixel shader;";
	#endif

	#ifdef TEXTURE_STATS
		ft << "differentTexture;TextureReuse;Size;noPot2U;Compressed Texture;8 bits Texture;16 bits Texture;24 bits Texture;32 bits Texture;64 bits Texture;128 bits Texture;Vertex Data Texture;DEFAULT;MANAGED;SYSTEMMEM;SCRATCH;";
        ft << "T S_D3DFMT_R8G8B8;T S_D3DFMT_A8R8G8B8;T S_D3DFMT_X8R8G8B8;T S_D3DFMT_R5G6B5;T S_D3DFMT_X1R5G5B5;T S_D3DFMT_A1R5G5B5;T S_D3DFMT_A4R4G4B4;T S_D3DFMT_R3G3B2;T S_D3DFMT_A8;T S_D3DFMT_A8R3G3B2;";
        ft << "T S_D3DFMT_X4R4G4B4;T S_D3DFMT_A2B10G10R10;T S_D3DFMT_A8B8G8R8;T S_D3DFMT_X8B8G8R8;T S_D3DFMT_G16R16;T S_D3DFMT_A2R10G10B10;T S_D3DFMT_A16B16G16R16;T S_D3DFMT_A8P8;T S_D3DFMT_P8;T S_D3DFMT_L8;";
        ft << "T S_D3DFMT_A8L8;T S_D3DFMT_A4L4;T S_D3DFMT_V8U8;T S_D3DFMT_L6V5U5;T S_D3DFMT_X8L8V8U8;T S_D3DFMT_Q8W8V8U8;T S_D3DFMT_V16U16;T S_D3DFMT_A2W10V10U10;T S_D3DFMT_UYVY;T S_D3DFMT_R8G8_B8G8;";
        ft << "T S_D3DFMT_YUY2;T S_D3DFMT_G8R8_G8B8;T S_D3DFMT_DXT1;T S_D3DFMT_DXT2;T S_D3DFMT_DXT3;T S_D3DFMT_DXT4;T S_D3DFMT_DXT5;T S_D3DFMT_D16_LOCKABLE;T S_D3DFMT_D32;T S_D3DFMT_D15S1;T S_D3DFMT_D24S8;";
        ft << "T S_D3DFMT_D24X8;T S_D3DFMT_D24X4S4;T S_D3DFMT_D16;T S_D3DFMT_D32F_LOCKABLE;T S_D3DFMT_D24FS8;T S_D3DFMT_L16;T S_D3DFMT_VERTEXDATA;T S_D3DFMT_INDEX16;T S_D3DFMT_INDEX32;T S_D3DFMT_Q16W16V16U16;";
        ft << "T S_D3DFMT_MULTI2_ARGB8;T S_D3DFMT_R16F;T S_D3DFMT_G16R16F;T S_D3DFMT_A16B16G16R16F;T S_D3DFMT_R32F;T S_D3DFMT_G32R32F;T S_D3DFMT_A32B32G32R32F;T S_D3DFMT_CxV8U8;T S_D3DFMT_ATI2;T S_D3DFMT_NULL;";
    #endif

	#ifdef RENDERTARGET_STATS
		ft << "Render Target setted;Render Target width; Render Target height;Render Target size;numRenderTargetTexture;numDiffRenderTargetTexture;Compressed Render Target;8 bits Render Target;16 bits Render Target;24 bits Render Target;32 bits Render Target;64 bits Render Target;128 bits Render Target;Vertex Data Render Target;";
        ft << "RT S_D3DFMT_R8G8B8;RT S_D3DFMT_A8R8G8B8;RT S_D3DFMT_X8R8G8B8;RT S_D3DFMT_R5G6B5;RT S_D3DFMT_X1R5G5B5;RT S_D3DFMT_A1R5G5B5;RT S_D3DFMT_A4R4G4B4;RT S_D3DFMT_R3G3B2;RT S_D3DFMT_A8;RT S_D3DFMT_A8R3G3B2;";
        ft << "RT S_D3DFMT_X4R4G4B4;RT S_D3DFMT_A2B10G10R10;RT S_D3DFMT_A8B8G8R8;RT S_D3DFMT_X8B8G8R8;RT S_D3DFMT_G16R16;RT S_D3DFMT_A2R10G10B10;RT S_D3DFMT_A16B16G16R16;RT S_D3DFMT_A8P8;RT S_D3DFMT_P8;RT S_D3DFMT_L8;";
        ft << "RT S_D3DFMT_A8L8;RT S_D3DFMT_A4L4;RT S_D3DFMT_V8U8;RT S_D3DFMT_L6V5U5;RT S_D3DFMT_X8L8V8U8;RT S_D3DFMT_Q8W8V8U8;RT S_D3DFMT_V16U16;RT S_D3DFMT_A2W10V10U10;RT S_D3DFMT_UYVY;RT S_D3DFMT_R8G8_B8G8;";
        ft << "RT S_D3DFMT_YUY2;RT S_D3DFMT_G8R8_G8B8;RT S_D3DFMT_DXT1;RT S_D3DFMT_DXT2;RT S_D3DFMT_DXT3;RT S_D3DFMT_DXT4;RT S_D3DFMT_DXT5;RT S_D3DFMT_D16_LOCKABLE;RT S_D3DFMT_D32;RT S_D3DFMT_D15S1;RT S_D3DFMT_D24S8;";
        ft << "RT S_D3DFMT_D24X8;RT S_D3DFMT_D24X4S4;RT S_D3DFMT_D16;RT S_D3DFMT_D32F_LOCKABLE;RT S_D3DFMT_D24FS8;RT S_D3DFMT_L16;RT S_D3DFMT_VERTEXDATA;RT S_D3DFMT_INDEX16;RT S_D3DFMT_INDEX32;RT S_D3DFMT_Q16W16V16U16;";
        ft << "RT S_D3DFMT_MULTI2_ARGB8;RT S_D3DFMT_R16F;RT S_D3DFMT_G16R16F;RT S_D3DFMT_A16B16G16R16F;RT S_D3DFMT_R32F;RT S_D3DFMT_G32R32F;RT S_D3DFMT_A32B32G32R32F;RT S_D3DFMT_CxV8U8;RT S_D3DFMT_ATI2;RT S_D3DFMT_NULL;";
    #endif

	#ifdef NVIDIA_STATS

		#ifdef SETUP_POINT_COUNT
			ft << "setup_point_count;";
		#endif

		#ifdef SETUP_LINE_COUNT
			ft << "setup_line_count;";
		#endif

		#ifdef SETUP_TRIANGLE_COUNT
			ft << "setup_triangle_count;";
		#endif

		#ifdef SETUP_PRIMITIVE_COUNT
			ft << "setup_primitive_count;";
		#endif

		#ifdef SETUP_PRIMITIVE_CULLED_COUNT
			ft << "setup_primitive_culled_count;";
		#endif

		#ifdef ROP_SAMPLES_KILLED_BY_EARLYZ_COUNT
			ft << "rop_samples_killed_by_earlyz_count;";
		#endif

		#ifdef ROP_SAMPLES_KILLED_BY_LATEZ_COUNT
			ft << "rop_samples_killed_by_latez_count;";
		#endif

		#ifdef TEXTURE_WAITS_FOR_FB
			ft << "texture_waits_for_fb;";
		#endif

		#ifdef RASTERIZER_TILES_IN_COUNT
			ft << "rasterizer_tiles_in_count;";
		#endif

		#ifdef RASTERIZER_TILES_KILLED_BY_ZCULL_COUNT
			ft << "rasterizer_tiles_killed_by_zcull_count;";
		#endif

		#ifdef SHADED_PIXEL_COUNT
			ft << "shaded_pixel_count;";
		#endif

	#endif

	#ifdef NEW_STATS
		ft << "numAlpha;" ;
	#endif

#ifdef OCCLUSION_STATS
        ft << "numPixelsDrawn;";
#endif

    #ifdef TEXTURE_STATS
        ft << "numCreateTexture;";
    #endif

	ft << "\n";


	stadistics.printStats(&ft);

	#ifdef BATCH_STATS
		fb.close();
	#endif

	ff.close();
	ft.close();

	#ifdef NVIDIA_STATS
		//Nvidia PerfAPI
		NVPMShutdown();
	#endif
}

////////////////////////////////////////////////////////////
////////		D3D9FrameStats Definitions          ////////
////////////////////////////////////////////////////////////

D3D9FrameStats::D3D9FrameStats() : D3D9Stats() {
	numFrame = 1;
}

void D3D9FrameStats::endFrame() {
	#ifdef BATCH_STATS
		batch.resetNumBatch();
	#endif
}

void D3D9FrameStats::endBatch () {
	#ifdef BATCH_STATS
		stadistics.BatchOpStatsCollector(batch.getStadistics());
		batch.writeStadistics();
	#endif
}


bool D3D9FrameStats::isFrame() {
	return true;
}

void D3D9FrameStats::writeStadistics() {
	#ifdef TITLE_STATS
		*ff << numFrame << ";";
	#endif
	stadistics.printStats(ff);
	stadistics.reset();
	numFrame++;

	#ifdef BATCH_STATS
		#ifdef TITLE_STATS
			*fb << numFrame;
		#endif
	#endif
}


#ifdef BATCH_STATS
	void D3D9FrameStats::setStadisticsFiles(ofstream* _ff,ofstream* _fb) {
#else
	void D3D9FrameStats::setStadisticsFiles(ofstream* _ff) {
#endif

		ff = _ff;

	#ifdef BATCH_STATS
		fb = _fb;

		#ifdef TITLE_STATS
			*fb << numFrame;
		#endif

		batch.setStadisticsFile(fb);
	#endif
}



////////////////////////////////////////////////////////////
////////		D3D9BatchStats Definitions          ////////
////////////////////////////////////////////////////////////

#ifdef BATCH_STATS
	D3D9BatchStats::D3D9BatchStats() : D3D9Stats() {
		numBatch = 1;
		tmpNumBatch = 1;
	}

	bool D3D9BatchStats::isBatch() {
		return true;
	}


	void D3D9BatchStats::resetNumBatch() {
		numBatch = 1;
		tmpNumBatch = 1;
	}

	void D3D9BatchStats::setStadisticsFile(ofstream* _fb) {
		fb = _fb;
	}

	void D3D9BatchStats::writeStadistics() {

		#ifdef TITLE_STATS
			*fb << ";" << numBatch << ";";
		#endif

		stadistics.printStats(fb);
		stadistics.reset();
		numBatch++;
		tmpNumBatch++;

	}
#endif

////////////////////////////////////////////////////////////
////////	   D3D9StatsCollector Definitions       ////////
////////////////////////////////////////////////////////////

D3D9StatsCollector::D3D9StatsCollector() {
	numDrawPrimitives = 0;
	primitiveCount = 0;
	numDrawPoint = 0;
	numDrawLine = 0;
	numDrawTriangleList = 0;
	numDrawTriangleStript = 0;
	numDrawTriangleFan = 0;
	numDrawForceDWORD = 0;
	activeSampler = 0;
	activeSamplerAverage = 0;
	acumStats = 0;
	batchSize = 0;
	numApplyStateBlock = 0;
	numCreateRenderTarget = 0;
	acumSamplerUsage = 0;
	textureType [NEAREST] = 0;
	textureType [NEAREST_MIPMAP_LINEAR] = 0;
	textureType [BILINEAR] = 0;
	textureType [BILINEAR_ANISO_2] = 0;
	textureType [BILINEAR_ANISO_4] = 0;
	textureType [BILINEAR_ANISO_8] = 0;
	textureType [BILINEAR_ANISO_16] = 0;
	textureType [TRILINEAR] = 0;
	textureType [TRILINEAR_ANISO_2] = 0;
	textureType [TRILINEAR_ANISO_4] = 0;
	textureType [TRILINEAR_ANISO_8] = 0;
	textureType [TRILINEAR_ANISO_16] = 0;
	textureType [UNKNOWN] = 0;
    numCreateTexture = 0;

	setup_line_count = 0;
	setup_point_count = 0;
	setup_triangle_count = 0;
	setup_primitive_count = 0;
	setup_primitive_culled_count = 0;
	setup_line_countC = 0;
	setup_point_countC = 0;
	setup_triangle_countC = 0;
	setup_primitive_countC = 0;
	setup_primitive_culled_countC = 0;

	rop_samples_killed_by_earlyz_count = 0;
	rop_samples_killed_by_latez_count = 0;
	rop_samples_killed_by_earlyz_countC = 0;
	rop_samples_killed_by_latez_countC = 0;

	texture_waits_for_fb = 0;
	texture_waits_for_fbC = 0;

	rasterizer_tiles_in_count = 0;
	rasterizer_tiles_killed_by_zcull_count = 0;
	rasterizer_tiles_in_countC = 0;
	rasterizer_tiles_killed_by_zcull_countC = 0;

	shaded_pixel_count = 0;
	shaded_pixel_countC = 0;

	fvfNumAtributes = 0;
	fvfBytes = 0;
    numPixelsDrawn = 0;

    numAlpha = 0;

    numComVertices1 = 0;
    numComVertices2 = 0;
    numComVertices3 = 0;
    numComVertices4 = 0;

}

void D3D9StatsCollector::reset(){
	numDrawPrimitives = 0;
	primitiveCount = 0;
	numDrawPoint = 0;
	numDrawLine = 0;
	numDrawTriangleList = 0;
	numDrawTriangleStript = 0;
	numDrawTriangleFan = 0;
	numDrawForceDWORD = 0;
	activeSampler = 0;
	activeSamplerAverage = 0;
	acumStats = 0;
	batchSize = 0;
	numApplyStateBlock = 0;
	numCreateRenderTarget = 0;
	acumSamplerUsage = 0;
	textureType [NEAREST] = 0;
	textureType [NEAREST_MIPMAP_LINEAR] = 0;
	textureType [BILINEAR] = 0;
	textureType [BILINEAR_ANISO_2] = 0;
	textureType [BILINEAR_ANISO_4] = 0;
	textureType [BILINEAR_ANISO_8] = 0;
	textureType [BILINEAR_ANISO_16] = 0;
	textureType [TRILINEAR] = 0;
	textureType [TRILINEAR_ANISO_2] = 0;
	textureType [TRILINEAR_ANISO_4] = 0;
	textureType [TRILINEAR_ANISO_8] = 0;
	textureType [TRILINEAR_ANISO_16] = 0;
	textureType [UNKNOWN] = 0;
    numCreateTexture = 0;
	
	setup_line_count = 0;
	setup_point_count = 0;
	setup_triangle_count = 0;
	setup_primitive_count = 0;
	setup_primitive_culled_count = 0;
	setup_line_countC = 0;
	setup_point_countC = 0;
	setup_triangle_countC = 0;
	setup_primitive_countC = 0;
	setup_primitive_culled_countC = 0;

	rop_samples_killed_by_earlyz_count = 0;
	rop_samples_killed_by_latez_count = 0;
	rop_samples_killed_by_earlyz_countC = 0;
	rop_samples_killed_by_latez_countC = 0;

	texture_waits_for_fb = 0;
	texture_waits_for_fbC = 0;

	rasterizer_tiles_in_count = 0;
	rasterizer_tiles_killed_by_zcull_count = 0;
	rasterizer_tiles_in_countC = 0;
	rasterizer_tiles_killed_by_zcull_countC = 0;

	shaded_pixel_count = 0;
	shaded_pixel_countC = 0;

	vertexShaderStats.reset();
    pixelShaderStats.reset();

	textureStats.reset();
	renderTargetStats.reset();

	fvfNumAtributes = 0;
	fvfBytes = 0;
    fvfNumAttribAv = 0;
    fvfBytesAv = 0;
    fvfNumAttribSV = 0;
    fvfBytesSV = 0;

    numPixelsDrawn = 0;

    numAlpha = 0;

    numComVertices1 = 0;
    numComVertices2 = 0;
    numComVertices3 = 0;
    numComVertices4 = 0;
}

void D3D9StatsCollector::BatchOpStatsCollector(D3D9StatsCollector* second){

    //ofstream f;
		
	#ifdef GENERAL_STATS
		numDrawPrimitives += second->numDrawPrimitives;
		primitiveCount += second->primitiveCount;
		numDrawPoint += second->numDrawPoint;
		numDrawLine += second->numDrawLine;
		numDrawTriangleList += second->numDrawTriangleList;
		numDrawTriangleStript += second->numDrawTriangleStript;
		numDrawTriangleFan += second->numDrawTriangleFan;
		numDrawForceDWORD+= second->numDrawForceDWORD;
		activeSampler = activeSampler + second->activeSampler;
		activeSamplerAverage = activeSamplerAverage + second->activeSamplerAverage;
		numApplyStateBlock += second->numApplyStateBlock;
		numCreateRenderTarget += second->numCreateRenderTarget;
		batchSize = batchSize + second->batchSize;
        numComVertices1 = numComVertices1 + second->numComVertices1;
        numComVertices2 = numComVertices2 + second->numComVertices2;
        numComVertices3 = numComVertices3 + second->numComVertices3;
        numComVertices4 = numComVertices4 + second->numComVertices4;
		acumStats += second->acumStats;
	#endif

	#ifdef VERTEX_SHADER_STATS
		vertexShaderStats.addShaderStats(second->getVertexShaderStats(), 1);
		fvfNumAtributes += second->fvfNumAtributes;
		fvfBytes += second->fvfBytes;
        fvfNumAttribAv += second->fvfNumAttribAv;
        fvfBytesAv += second->fvfBytesAv;
        fvfNumAttribSV += second->fvfNumAttribSV;
        fvfBytesSV += second->fvfBytesSV;
    #endif

            /*f.open("shLog.txt", ios::app);
            f << "++batchop\n";
            f.close();*/

    #ifdef PIXEL_SHADER_STATS
		pixelShaderStats.addShaderStats(second->getPixelShaderStats(), 1);
	#endif

	#ifdef TEXTURE_STATS
		textureStats.addTextureStats(second->getTextureStats());
        numCreateTexture += second->numCreateTexture;
	#endif

	#ifdef RENDERTARGET_STATS
		renderTargetStats.addRenderTargetStats(second->getRenderTargetStats());
	#endif

	#ifdef TEXTURE_STATS
		textureType[NEAREST] += (second->textureType[NEAREST]/* * second->shaded_pixel_count*/);
		textureType[NEAREST_MIPMAP_LINEAR] += (second->textureType[NEAREST_MIPMAP_LINEAR]/* * second->shaded_pixel_count*/);
		textureType[BILINEAR] += (second->textureType[BILINEAR]/* * second->shaded_pixel_count*/);
		textureType[BILINEAR_ANISO_2] += (second->textureType[BILINEAR_ANISO_2]/* * second->shaded_pixel_count*/);
		textureType[BILINEAR_ANISO_4] += (second->textureType[BILINEAR_ANISO_4]/* * second->shaded_pixel_count*/);
		textureType[BILINEAR_ANISO_8] += (second->textureType[BILINEAR_ANISO_8]/* * second->shaded_pixel_count*/);
		textureType[BILINEAR_ANISO_16] += (second->textureType[BILINEAR_ANISO_16]/* * second->shaded_pixel_count*/);
		textureType[TRILINEAR] += (second->textureType[TRILINEAR]/* * second->shaded_pixel_count*/);
		textureType[TRILINEAR_ANISO_2] += (second->textureType[TRILINEAR_ANISO_2]/* * second->shaded_pixel_count*/);
		textureType[TRILINEAR_ANISO_4] += (second->textureType[TRILINEAR_ANISO_4]/* * second->shaded_pixel_count*/);
		textureType[TRILINEAR_ANISO_8] += (second->textureType[TRILINEAR_ANISO_8]/* * second->shaded_pixel_count*/);
		textureType[TRILINEAR_ANISO_16] += (second->textureType[TRILINEAR_ANISO_16]/* * second->shaded_pixel_count*/);
		textureType[UNKNOWN] += (second->textureType[UNKNOWN]/* * second->shaded_pixel_count*/);
	#endif

	#ifdef NVIDIA_STATS
		setup_line_count += second->setup_line_count;
		setup_point_count += second->setup_point_count;
		setup_triangle_count += second->setup_triangle_count;
		setup_primitive_count += second->setup_primitive_count;
		setup_primitive_culled_count += second->setup_primitive_culled_count;

		rop_samples_killed_by_earlyz_count += second->rop_samples_killed_by_earlyz_count;
		rop_samples_killed_by_latez_count += second->rop_samples_killed_by_latez_count;
		rop_samples_killed_by_earlyz_countC += second->rop_samples_killed_by_earlyz_countC;
		rop_samples_killed_by_latez_countC += second->rop_samples_killed_by_latez_countC;

		texture_waits_for_fb += second->texture_waits_for_fb;
		texture_waits_for_fbC += second->texture_waits_for_fbC;

		rasterizer_tiles_in_count += second->rasterizer_tiles_in_count;
		rasterizer_tiles_killed_by_zcull_count += second->rasterizer_tiles_killed_by_zcull_count;
		rasterizer_tiles_in_countC += second->rasterizer_tiles_in_countC;
		rasterizer_tiles_killed_by_zcull_countC += second->rasterizer_tiles_killed_by_zcull_countC;

		shaded_pixel_count += second->shaded_pixel_count;
		shaded_pixel_countC += second->shaded_pixel_countC;
	#endif NVIDIA_STATS

	#ifdef NEW_STATS

        /*if ((primitiveCount + second->primitiveCount) != 0) {
            fvfNumAttribAv = fvfNumAtributes * ((double)primitiveCount / (double)(primitiveCount + second->primitiveCount)) + second->fvfNumAtributes * ((double)second->primitiveCount / (double)(primitiveCount + second->primitiveCount));
            fvfBytesAv = fvfBytes * ((double)primitiveCount / (double)(primitiveCount + second->primitiveCount)) + second->fvfBytes * ((double)second->primitiveCount / (double)(primitiveCount + second->primitiveCount));
        }
        else {
            fvfNumAttribAv = 0;
            fvfBytesAv = 0;
       }*/
        numAlpha += second->numAlpha;
	#endif

#ifdef OCCLUSION_STATS
        numPixelsDrawn += second->numPixelsDrawn;
#endif

	acumSamplerUsage += second->acumSamplerUsage;
}

void D3D9StatsCollector::FrameOpStatsCollector(D3D9StatsCollector* second){

    //ofstream f;

	numDrawPrimitives += second->numDrawPrimitives;
	primitiveCount += second->primitiveCount;

	#ifdef VERTEX_SHADER_STATS
		vertexShaderStats.addShaderStats(second->getVertexShaderStats(), 1);
		fvfNumAtributes += second->fvfNumAtributes;
		fvfBytes += second->fvfBytes;
        fvfNumAttribAv += second->fvfNumAttribAv;
        fvfBytesAv += second->fvfBytesAv;
        fvfNumAttribSV += second->fvfNumAttribSV;
        fvfBytesSV += second->fvfBytesSV;
    #endif

            /*f.open("shLog.txt", ios::app);
            f << "++frameop\n";
            f.close();*/

    #ifdef PIXEL_SHADER_STATS
		pixelShaderStats.addShaderStats(second->getPixelShaderStats(), 1);
	#endif

	#ifdef TEXTURE_STATS
		textureStats.addTextureStats(second->getTextureStats());
        numCreateTexture += second->numCreateTexture;
	#endif

	numDrawPoint += second->numDrawPoint;
	numDrawLine += second->numDrawLine;
	numDrawTriangleList += second->numDrawTriangleList;
	numDrawTriangleStript += second->numDrawTriangleStript;
	numDrawTriangleFan += second->numDrawTriangleFan;
	numDrawForceDWORD+= second->numDrawForceDWORD;
	activeSampler = activeSampler + second->activeSampler;
	activeSamplerAverage = activeSamplerAverage + second->activeSamplerAverage;
	numApplyStateBlock += second->numApplyStateBlock;
	numCreateRenderTarget += second->numCreateRenderTarget;
	batchSize = batchSize + second->batchSize;
    numComVertices1 = numComVertices1 + second->numComVertices1;
    numComVertices2 = numComVertices2 + second->numComVertices2;
    numComVertices3 = numComVertices3 + second->numComVertices3;
    numComVertices4 = numComVertices4 + second->numComVertices4;
	acumStats += second->acumStats;

	#ifdef TEXTURE_STATS
		textureType[NEAREST] += second->textureType[NEAREST];
		textureType[NEAREST_MIPMAP_LINEAR] += second->textureType[NEAREST_MIPMAP_LINEAR];
		textureType[BILINEAR] += second->textureType[BILINEAR];
		textureType[BILINEAR_ANISO_2] += second->textureType[BILINEAR_ANISO_2];
		textureType[BILINEAR_ANISO_4] += second->textureType[BILINEAR_ANISO_4];
		textureType[BILINEAR_ANISO_8] += second->textureType[BILINEAR_ANISO_8];
		textureType[BILINEAR_ANISO_16] += second->textureType[BILINEAR_ANISO_16];
		textureType[TRILINEAR] += second->textureType[TRILINEAR];
		textureType[TRILINEAR_ANISO_2] += second->textureType[TRILINEAR_ANISO_2];
		textureType[TRILINEAR_ANISO_4] += second->textureType[TRILINEAR_ANISO_4];
		textureType[TRILINEAR_ANISO_8] += second->textureType[TRILINEAR_ANISO_8];
		textureType[TRILINEAR_ANISO_16] += second->textureType[TRILINEAR_ANISO_16];
		textureType[UNKNOWN] += second->textureType[UNKNOWN];
	#endif

	#ifdef NVIDIA_STATS
		setup_line_count += second->setup_line_count;
		setup_point_count += second->setup_point_count;
		setup_triangle_count += second->setup_triangle_count;
		setup_primitive_count += second->setup_primitive_count;
		setup_primitive_culled_count += second->setup_primitive_culled_count;

		rop_samples_killed_by_earlyz_count += second->rop_samples_killed_by_earlyz_count;
		rop_samples_killed_by_latez_count += second->rop_samples_killed_by_latez_count;
		rop_samples_killed_by_earlyz_countC += second->rop_samples_killed_by_earlyz_countC;
		rop_samples_killed_by_latez_countC += second->rop_samples_killed_by_latez_countC;

		texture_waits_for_fb += second->texture_waits_for_fb;
		texture_waits_for_fbC += second->texture_waits_for_fbC;

		rasterizer_tiles_in_count += second->rasterizer_tiles_in_count;
		rasterizer_tiles_killed_by_zcull_count += second->rasterizer_tiles_killed_by_zcull_count;
		rasterizer_tiles_in_countC += second->rasterizer_tiles_in_countC;
		rasterizer_tiles_killed_by_zcull_countC += second->rasterizer_tiles_killed_by_zcull_countC;

		shaded_pixel_count += second->shaded_pixel_count;
		shaded_pixel_countC += second->shaded_pixel_countC;
	#endif

	#ifdef NEW_STATS

        /*if ((primitiveCount + second->primitiveCount) != 0) {
            fvfNumAttribAv = fvfNumAtributes * ((double)primitiveCount / (double)(primitiveCount + second->primitiveCount)) + second->fvfNumAtributes * ((double)second->primitiveCount / (double)(primitiveCount + second->primitiveCount));
            fvfBytesAv = fvfBytes * ((double)primitiveCount / (double)(primitiveCount + second->primitiveCount)) + second->fvfBytes * ((double)second->primitiveCount / (double)(primitiveCount + second->primitiveCount));
        }
        else {
            fvfNumAttribAv = 0;
            fvfBytesAv = 0;
       }*/
        numAlpha += second->numAlpha;
	#endif

#ifdef OCCLUSION_STATS
        numPixelsDrawn += second->numPixelsDrawn;
#endif

	acumSamplerUsage += second->acumSamplerUsage;
}	

void D3D9StatsCollector::printStats (ofstream* f) {

	#ifdef GENERAL_STATS
		*f << numDrawPrimitives << ";";
		*f << primitiveCount << ";";
		*f << numDrawPoint<< ";";
		*f << numDrawLine<< ";";
		*f << numDrawTriangleList<< ";";
		*f << numDrawTriangleStript<< ";";
		*f << numDrawTriangleFan<< ";";
		*f << ((acumSamplerUsage == 0) ? 0 : ((double)activeSamplerAverage /(double)acumSamplerUsage)) << ";";
		*f << activeSampler << ";";
		*f << ((acumStats == 0) ? 0 : (batchSize/(double)acumStats)) << ";";
		*f << batchSize << ";";
		*f << numApplyStateBlock<< ";";
	#endif

#ifdef TEXTURE_STATS
		*f << (unsigned long long)textureType[NEAREST] << ";";
		*f << (unsigned long long)textureType[NEAREST_MIPMAP_LINEAR] << ";";
		*f << (unsigned long long)textureType[BILINEAR] << ";";
		*f << (unsigned long long)textureType[BILINEAR_ANISO_2] << ";";
		*f << (unsigned long long)textureType[BILINEAR_ANISO_4] << ";";
		*f << (unsigned long long)textureType[BILINEAR_ANISO_8] << ";";
		*f << (unsigned long long)textureType[BILINEAR_ANISO_16] << ";";
		*f << (unsigned long long)textureType[TRILINEAR] << ";";
		*f << (unsigned long long)textureType[TRILINEAR_ANISO_2] << ";";
		*f << (unsigned long long)textureType[TRILINEAR_ANISO_4] << ";";
		*f << (unsigned long long)textureType[TRILINEAR_ANISO_8] << ";";
		*f << (unsigned long long)textureType[TRILINEAR_ANISO_16] << ";";
		*f << (unsigned long long)textureType[UNKNOWN] << ";";
#endif

	/*#ifdef TEXTURE_STATS
		*f << ((acumStats == 0) ? 0 : ((double)textureType[NEAREST]/(double)acumStats)) << ";";
		*f << ((acumStats == 0) ? 0 : ((double)textureType[NEAREST_MIPMAP_LINEAR]/(double)acumStats)) << ";";
		*f << ((acumStats == 0) ? 0 : ((double)textureType[BILINEAR]/(double)acumStats)) << ";";
		*f << ((acumStats == 0) ? 0 : ((double)textureType[BILINEAR_ANISO]/(double)acumStats)) << ";";
		*f << ((acumStats == 0) ? 0 : ((double)textureType[TRILINEAR]/(double)acumStats)) << ";";
		*f << ((acumStats == 0) ? 0 : ((double)textureType[TRILINEAR_ANISO]/(double)acumStats)) << ";";
		*f << ((acumStats == 0) ? 0 : ((double)textureType[UNKNOWN]/(double)acumStats)) << ";";
	#endif*/
        

	#ifdef VERTEX_SHADER_STATS
		vertexShaderStats.printShaderStats(f);
        *f << numComVertices1 << ";";
        *f << numComVertices2 << ";";
        *f << numComVertices3 << ";";
        *f << numComVertices4 << ";";
		*f << ((fvfNumAtributes == 0) ? 0 : ((double)fvfNumAtributes/(double)acumStats)) << ";";
		*f << ((fvfBytes == 0) ? 0 : ((double)fvfBytes/(double)acumStats)) << ";";
		*f << ((fvfNumAttribAv == 0) ? 0 : ((double)fvfNumAttribAv/(double)batchSize)) << ";";
		*f << ((fvfBytesAv == 0) ? 0 : ((double)fvfBytesAv/(double)batchSize)) << ";";
		*f << ((fvfNumAttribSV == 0) ? 0 : ((double)fvfNumAttribSV/(double)numComVertices4)) << ";";
		*f << ((fvfBytesSV == 0) ? 0 : ((double)fvfBytesSV/(double)numComVertices4)) << ";";
    #endif

    #ifdef PIXEL_SHADER_STATS
		pixelShaderStats.printShaderStats(f);
	#endif

	#ifdef TEXTURE_STATS
		textureStats.printTextureStats(f);
	#endif

	#ifdef RENDERTARGET_STATS
		renderTargetStats.printRenderTargetStats(f);
	#endif

	#ifdef NVIDIA_STATS

		#ifdef SETUP_POINT_COUNT
			*f << setup_point_count << ";";
		#endif

		#ifdef SETUP_LINE_COUNT
			*f << setup_line_count << ";";
		#endif

		#ifdef SETUP_TRIANGLE_COUNT
			*f << setup_triangle_count << ";";
		#endif

		#ifdef SETUP_PRIMITIVE_COUNT
			*f << setup_primitive_count << ";";
		#endif

		#ifdef SETUP_PRIMITIVE_CULLED_COUNT
			*f << setup_primitive_culled_count << ";";
		#endif

		#ifdef ROP_SAMPLES_KILLED_BY_EARLYZ_COUNT
			*f << rop_samples_killed_by_earlyz_count << ";";
		#endif

		#ifdef ROP_SAMPLES_KILLED_BY_LATEZ_COUNT
			*f << rop_samples_killed_by_latez_count << ";";
		#endif

		#ifdef TEXTURE_WAITS_FOR_FB
			*f << texture_waits_for_fb << ";";
		#endif

		#ifdef RASTERIZER_TILES_IN_COUNT
			*f << rasterizer_tiles_in_count << ";";
		#endif

		#ifdef RASTERIZER_TILES_KILLED_BY_ZCULL_COUNT
			*f << rasterizer_tiles_killed_by_zcull_count << ";";
		#endif

		#ifdef SHADED_PIXEL_COUNT
			*f << shaded_pixel_count << ";";
		#endif

	#endif

	#ifdef NEW_STATS
        *f << numAlpha << ";";
	#endif

    #ifdef OCCLUSION_STATS
        *f << numPixelsDrawn << ";";
    #endif

    #ifdef TEXTURE_STATS
        *f << numCreateTexture << ";";
    #endif
	
	*f << "\n";

}

void D3D9StatsCollector::addDrawPrimitive () {
	numDrawPrimitives++;
	acumStats++;
}

void D3D9StatsCollector::addPrimitiveCount(int _primitiveCount) {
	primitiveCount += _primitiveCount;
}

void D3D9StatsCollector::addDrawPoint(int primitiveCount){
	batchSize += primitiveCount;
	numDrawPoint++;
}

void D3D9StatsCollector::addDrawLine(int primitiveCount){
	batchSize +=(primitiveCount*2);
	numDrawLine++;
}

void D3D9StatsCollector::addDrawTriangleList(int primitiveCount){
	batchSize +=(primitiveCount*3);
	numDrawTriangleList++;
}

void D3D9StatsCollector::addDrawTriangleStript(int primitiveCount){
	batchSize +=(primitiveCount+2);
	numDrawTriangleStript++;
}

void D3D9StatsCollector::addDrawTriangleFan(int primitiveCount){
	batchSize +=(primitiveCount+2);
	numDrawTriangleFan++;
}

void D3D9StatsCollector::addDrawForceDWORD(int primitiveCount){
	batchSize += primitiveCount;
	numDrawForceDWORD++;
}

void D3D9StatsCollector::addBeginStateBlock() {
	numBeginStateBlock++;
}

void D3D9StatsCollector::addEndStateBlock() {
	numEndStateBlock++;
}

void D3D9StatsCollector::addApplyStateBlock() {
	numApplyStateBlock++;
}

void D3D9StatsCollector::addCreateRenderTarget() {
	numCreateRenderTarget++;
}

void D3D9StatsCollector::addCaptureStateBlock() {
	numCaptureStateBlock++;
}

void D3D9StatsCollector::addCreateTexture() {
	numCreateTexture++;
}

void D3D9StatsCollector::addComputedVertices(int numVertex1, int numVertex2, int numVertex3, int numVertex4) {
    numComVertices1 = numComVertices1 + numVertex1;
    numComVertices2 = numComVertices2 + numVertex2;
    numComVertices3 = numComVertices3 + numVertex3;
    numComVertices4 = numComVertices4 + numVertex4;
}

void D3D9StatsCollector::addAttributes(int numAttrib, int numAttribBytes, int numVertex, int numSVertex) {
    fvfNumAtributes = numAttrib;
    fvfBytes = numAttribBytes;
    fvfNumAttribAv = fvfNumAtributes * numVertex;
    fvfBytesAv = fvfBytes * numVertex;
    fvfNumAttribSV = fvfNumAtributes * numSVertex;
    fvfBytesSV = fvfBytes * numSVertex;
}

void D3D9StatsCollector::addNumberOfPixelsDrawn(int numberOfPixelsDrawn) {
    numPixelsDrawn += numberOfPixelsDrawn;
}

void D3D9StatsCollector::addAlphaTest(int alpha){
    numAlpha += alpha;
}

D3D9ShaderStats* D3D9StatsCollector::getVertexShaderStats() {
	return &vertexShaderStats;
}

D3D9ShaderStats* D3D9StatsCollector::getPixelShaderStats() {
	return &pixelShaderStats;
}

D3D9TextureStats* D3D9StatsCollector::getTextureStats() {
	return &textureStats;
}

D3D9RenderTargetStats* D3D9StatsCollector::getRenderTargetStats() {
	return &renderTargetStats;
}

void D3D9StatsCollector::setAverageActiveSampler (int active) {
	activeSamplerAverage += active;

	if (active != 0) acumSamplerUsage++;
}

void D3D9StatsCollector::setActiveSampler (unsigned long long active) {
	activeSampler += active;

	//if (active != 0) acumSamplerUsage++;
}

int* D3D9StatsCollector::getFilterVector() {
	return textureType;
}

UINT64 D3D9StatsCollector::getShadedPixel() {
    return shaded_pixel_count;
}

#ifdef NVIDIA_STATS
void D3D9StatsCollector::collectNvidiaCounters() {

	UINT nNumSamples;

	NVPMSample(NULL, &nNumSamples);

	#ifdef SETUP_POINT_COUNT
		NVPMGetCounterValueByName("setup_point_count", 0, &setup_point_count, &setup_point_countC);
	#endif

	#ifdef SETUP_LINE_COUNT
		NVPMGetCounterValueByName("setup_line_count", 0, &setup_line_count, &setup_line_countC); 
	#endif

	#ifdef SETUP_TRIANGLE_COUNT
		NVPMGetCounterValueByName("setup_triangle_count", 0, &setup_triangle_count, &setup_triangle_countC);
	#endif

	#ifdef SETUP_PRIMITIVE_COUNT
		NVPMGetCounterValueByName("setup_primitive_count", 0, &setup_primitive_count, &setup_primitive_countC);
	#endif

	#ifdef SETUP_PRIMITIVE_CULLED_COUNT
		NVPMGetCounterValueByName("setup_primitive_culled_count", 0, &setup_primitive_culled_count, &setup_primitive_culled_countC);
	#endif	

	#ifdef ROP_SAMPLES_KILLED_BY_EARLYZ_COUNT
		NVPMGetCounterValueByName("rop_samples_killed_by_earlyz_count", 0, &rop_samples_killed_by_earlyz_count, &rop_samples_killed_by_earlyz_countC);
	#endif

	#ifdef ROP_SAMPLES_KILLED_BY_LATEZ_COUNT
		NVPMGetCounterValueByName("rop_samples_killed_by_latez_count", 0, &rop_samples_killed_by_latez_count, &rop_samples_killed_by_latez_countC);
	#endif

	#ifdef TEXTURE_WAITS_FOR_FB
		NVPMGetCounterValueByName("texture_waits_for_fb", 0, &texture_waits_for_fb, &texture_waits_for_fbC);
	#endif

	#ifdef RASTERIZER_TILES_IN_COUNT
		NVPMGetCounterValueByName("rasterizer_tiles_in_count", 0, &rasterizer_tiles_in_count, &rasterizer_tiles_in_countC);
	#endif

	#ifdef RASTERIZER_TILES_KILLED_BY_ZCULL_COUNT
		NVPMGetCounterValueByName("rasterizer_tiles_killed_by_zcull_count", 0, &rasterizer_tiles_killed_by_zcull_count, &rasterizer_tiles_killed_by_zcull_countC);
	#endif

	#ifdef SHADED_PIXEL_COUNT
		NVPMGetCounterValueByName("shaded_pixel_count", 0, &shaded_pixel_count, &shaded_pixel_countC);
	#endif
}
#endif
