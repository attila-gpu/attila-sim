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
//////					D3DStats.h					//////
//////												//////
//////////////////////////////////////////////////////////

#ifndef __D3D9STATS
#define __D3D9STATS

#include <d3d9.h>
#include <d3dx9.h>
#include <map>
#include <fstream>

#include "D3DBuffer.h"

#include "D3D9ShaderStats.h"
#include "D3D9TextureStats.h"
#include "D3D9RenderTargetStats.h"

//#define BATCH_STATS

//#define CHANGE_PS

//#define OCCLUSION_STATS
#define START_OCCLUSION 0

#define TITLE_STATS
#define GENERAL_STATS
//#define VERTEX_SHADER_STATS
#define PIXEL_SHADER_STATS
#define TEXTURE_STATS
#define RENDERTARGET_STATS
//#define NVIDIA_STATS
//#define NEW_STATS

#ifdef NVIDIA_STATS
	//#define SETUP_POINT_COUNT
	//#define SETUP_LINE_COUNT
	//#define SETUP_TRIANGLE_COUNT
	//#define SETUP_PRIMITIVE_COUNT
	//#define SETUP_PRIMITIVE_CULLED_COUNT
	//#define ROP_SAMPLES_KILLED_BY_EARLYZ_COUNT
	//#define ROP_SAMPLES_KILLED_BY_LATEZ_COUNT
	//#define TEXTURE_WAITS_FOR_FB
	//#define RASTERIZER_TILES_IN_COUNT
	//#define RASTERIZER_TILES_KILLED_BY_ZCULL_COUNT
	#define SHADED_PIXEL_COUNT
#endif

#ifdef OCCLUSION_STATS
#define OCCLUSION true
#else
#define OCCLUSION false
#endif

#ifdef CHANGE_PS
#define CHPS true
#else
#define CHPS false
#endif


////////////////////////////////////////////////////////////
////////   Defined Functions called by the Player   ////////
////////////////////////////////////////////////////////////

#define D3D9OP_IDIRECT3DDEVICE9_PRESENT_USER_PRE \
	D3D9TraceStats::instance().endFrame();

#define D3D9OP_IDIRECT3DSWAPCHAIN9_PRESENT_USER_PRE \
	D3D9TraceStats::instance().endFrame();

#define D3D9OP_IDIRECT3DDEVICE9_DRAWPRIMITIVE_USER_PRE \
	D3D9TraceStats::instance().beginBatch();\
	/* Occlusion query code */ \
	IDirect3DQuery9* pOcclusionQuery; \
	IDirect3DQuery9* pEvent; \
	DWORD numberOfPixelsDrawn = -5; \
	HRESULT hr; \
    IDirect3DPixelShader9* g_pPSH; \
    if (CHPS) { \
        sip_This->CreatePixelShader((DWORD*)(D3D9TraceStats::instance().getPSBuffer()->GetBufferPointer()), &g_pPSH); \
        sip_This->SetPixelShader(g_pPSH); \
        sip_This->SetRenderState(D3DRS_ALPHATESTENABLE, 0); \
    } \
    if (OCCLUSION && D3D9TraceStats::instance().getNumFrame() >= START_OCCLUSION) { \
		hr = sip_This->CreateQuery(D3DQUERYTYPE_OCCLUSION, &pOcclusionQuery); \
		if (hr == D3D_OK)  \
			pOcclusionQuery->Issue(D3DISSUE_BEGIN);\
	}

#define D3D9OP_IDIRECT3DDEVICE9_DRAWPRIMITIVE_USER_POST \
	if (OCCLUSION && D3D9TraceStats::instance().getNumFrame() >= START_OCCLUSION) { \
		/* Occlusion query code */ \
		if (hr == D3D_OK)  {\
			pOcclusionQuery->Issue(D3DISSUE_END); \
			while(S_FALSE == pOcclusionQuery->GetData( &numberOfPixelsDrawn, \
	                                 sizeof(DWORD), D3DGETDATA_FLUSH )); \
		} \
        sip_This->CreateQuery(D3DQUERYTYPE_EVENT, &pEvent); \
        pEvent->Issue(D3DISSUE_END); \
        while ( S_FALSE == pEvent->GetData(NULL, 0, D3DGETDATA_FLUSH)); \
		D3D9TraceStats::instance().registerDrawPrimitive(ov_PrimitiveType, false, 0, ov_PrimitiveCount, (int)numberOfPixelsDrawn);\
        pOcclusionQuery->Release(); \
        pEvent->Release(); \
	} \
	else { \
        sip_This->CreateQuery(D3DQUERYTYPE_EVENT, &pEvent); \
        pEvent->Issue(D3DISSUE_END); \
        while ( S_FALSE == pEvent->GetData(NULL, 0, D3DGETDATA_FLUSH)); \
		D3D9TraceStats::instance().registerDrawPrimitive(ov_PrimitiveType, false, 0, ov_PrimitiveCount, 1);\
        pEvent->Release(); \
	} \
    if (CHPS) { \
        g_pPSH->Release(); \
    } \
	D3D9TraceStats::instance().endBatch();

#define D3D9OP_IDIRECT3DDEVICE9_DRAWPRIMITIVEUP_USER_PRE \
	D3D9TraceStats::instance().beginBatch();\
	/* Occlusion query code */ \
	IDirect3DQuery9* pOcclusionQuery; \
	IDirect3DQuery9* pEvent; \
	DWORD numberOfPixelsDrawn = -5; \
	HRESULT hr; \
    IDirect3DPixelShader9* g_pPSH; \
    if (CHPS) { \
        sip_This->CreatePixelShader((DWORD*)(D3D9TraceStats::instance().getPSBuffer()->GetBufferPointer()), &g_pPSH); \
        sip_This->SetPixelShader(g_pPSH); \
        sip_This->SetRenderState(D3DRS_ALPHATESTENABLE, 0); \
    } \
	if (OCCLUSION && D3D9TraceStats::instance().getNumFrame() >= START_OCCLUSION) { \
		hr = sip_This->CreateQuery(D3DQUERYTYPE_OCCLUSION, &pOcclusionQuery); \
		if (hr == D3D_OK)\
			pOcclusionQuery->Issue(D3DISSUE_BEGIN); \
	}

#define D3D9OP_IDIRECT3DDEVICE9_DRAWPRIMITIVEUP_USER_POST \
	if (OCCLUSION && D3D9TraceStats::instance().getNumFrame() >= START_OCCLUSION) { \
		/* Occlusion query code */ \
		if (hr == D3D_OK)  {\
			pOcclusionQuery->Issue(D3DISSUE_END); \
			while(S_FALSE == pOcclusionQuery->GetData( &numberOfPixelsDrawn, \
	                                 sizeof(DWORD), D3DGETDATA_FLUSH )); \
		} \
        sip_This->CreateQuery(D3DQUERYTYPE_EVENT, &pEvent); \
        pEvent->Issue(D3DISSUE_END); \
        while ( S_FALSE == pEvent->GetData(NULL, 0, D3DGETDATA_FLUSH)); \
		D3D9TraceStats::instance().registerDrawPrimitive(ov_PrimitiveType, false, 0, ov_PrimitiveCount, (int)numberOfPixelsDrawn);\
        pOcclusionQuery->Release(); \
        pEvent->Release(); \
	} \
	else { \
        sip_This->CreateQuery(D3DQUERYTYPE_EVENT, &pEvent); \
        pEvent->Issue(D3DISSUE_END); \
        while ( S_FALSE == pEvent->GetData(NULL, 0, D3DGETDATA_FLUSH)); \
		D3D9TraceStats::instance().registerDrawPrimitive(ov_PrimitiveType, false, 0, ov_PrimitiveCount, 1);\
        pEvent->Release(); \
	} \
    if (CHPS) { \
        g_pPSH->Release(); \
    } \
	D3D9TraceStats::instance().endBatch();

#define D3D9OP_IDIRECT3DDEVICE9_DRAWINDEXEDPRIMITIVE_USER_PRE \
	D3D9TraceStats::instance().beginBatch(); \
	/* Occlusion query code */ \
	IDirect3DQuery9* pOcclusionQuery; \
	IDirect3DQuery9* pEvent; \
	DWORD numberOfPixelsDrawn = -5; \
	HRESULT hr; \
    IDirect3DPixelShader9* g_pPSH; \
    if (CHPS) { \
        sip_This->CreatePixelShader((DWORD*)(D3D9TraceStats::instance().getPSBuffer()->GetBufferPointer()), &g_pPSH); \
        sip_This->SetPixelShader(g_pPSH); \
        sip_This->SetRenderState(D3DRS_ALPHATESTENABLE, 0); \
    } \
	if (OCCLUSION && D3D9TraceStats::instance().getNumFrame() >= START_OCCLUSION) { \
		hr = sip_This->CreateQuery(D3DQUERYTYPE_OCCLUSION, &pOcclusionQuery); \
		if (hr == D3D_OK)\
			pOcclusionQuery->Issue(D3DISSUE_BEGIN); \
	}
 
#define D3D9OP_IDIRECT3DDEVICE9_DRAWINDEXEDPRIMITIVE_USER_POST \
	if (OCCLUSION && D3D9TraceStats::instance().getNumFrame() >= START_OCCLUSION) { \
		/* Occlusion query code */ \
		if (hr == D3D_OK)  {\
			pOcclusionQuery->Issue(D3DISSUE_END); \
			while(S_FALSE == pOcclusionQuery->GetData( &numberOfPixelsDrawn, \
	                                 sizeof(DWORD), D3DGETDATA_FLUSH )); \
		} \
        sip_This->CreateQuery(D3DQUERYTYPE_EVENT, &pEvent); \
        pEvent->Issue(D3DISSUE_END); \
        while ( S_FALSE == pEvent->GetData(NULL, 0, D3DGETDATA_FLUSH)); \
		D3D9TraceStats::instance().registerDrawPrimitive(ov_Type, true, ov_startIndex, ov_primCount, (int)numberOfPixelsDrawn);\
        pOcclusionQuery->Release(); \
        pEvent->Release(); \
	} \
	else { \
        sip_This->CreateQuery(D3DQUERYTYPE_EVENT, &pEvent); \
        pEvent->Issue(D3DISSUE_END); \
        while ( S_FALSE == pEvent->GetData(NULL, 0, D3DGETDATA_FLUSH)); \
		D3D9TraceStats::instance().registerDrawPrimitive(ov_Type, true, ov_startIndex, ov_primCount, 1);\
        pEvent->Release(); \
	} \
    if (CHPS) { \
        g_pPSH->Release(); \
    } \
	D3D9TraceStats::instance().endBatch();

#define D3D9OP_IDIRECT3DDEVICE9_DRAWINDEXEDPRIMITIVEUP_USER_PRE \
	D3D9TraceStats::instance().beginBatch();\
	/* Occlusion query code */ \
	IDirect3DQuery9* pOcclusionQuery; \
	IDirect3DQuery9* pEvent; \
	DWORD numberOfPixelsDrawn = -5; \
	HRESULT hr; \
    IDirect3DPixelShader9* g_pPSH; \
    if (CHPS) { \
        sip_This->CreatePixelShader((DWORD*)(D3D9TraceStats::instance().getPSBuffer()->GetBufferPointer()), &g_pPSH); \
        sip_This->SetPixelShader(g_pPSH); \
        sip_This->SetRenderState(D3DRS_ALPHATESTENABLE, 0); \
    } \
	if (OCCLUSION && D3D9TraceStats::instance().getNumFrame() >= START_OCCLUSION) { \
		hr = sip_This->CreateQuery(D3DQUERYTYPE_OCCLUSION, &pOcclusionQuery); \
		if (hr == D3D_OK) \
			pOcclusionQuery->Issue(D3DISSUE_BEGIN); \
	}

#define D3D9OP_IDIRECT3DDEVICE9_DRAWINDEXEDPRIMITIVEUP_USER_POST \
	if (OCCLUSION && D3D9TraceStats::instance().getNumFrame() >= START_OCCLUSION) { \
		/* Occlusion query code */ \
		if (hr == D3D_OK)  {\
			pOcclusionQuery->Issue(D3DISSUE_END); \
			while(S_FALSE == pOcclusionQuery->GetData( &numberOfPixelsDrawn, \
	                                 sizeof(DWORD), D3DGETDATA_FLUSH )); \
		} \
        sip_This->CreateQuery(D3DQUERYTYPE_EVENT, &pEvent); \
        pEvent->Issue(D3DISSUE_END); \
        while ( S_FALSE == pEvent->GetData(NULL, 0, D3DGETDATA_FLUSH)); \
        D3D9TraceStats::instance().registerUPIndexVertex(sv_pIndexData, sv_IndexDataFormat, sv_VertexStreamZeroStride); \
		D3D9TraceStats::instance().registerDrawPrimitive(ov_PrimitiveType, true, 0, ov_PrimitiveCount, (int)numberOfPixelsDrawn);\
        pOcclusionQuery->Release(); \
        pEvent->Release(); \
	} \
	else { \
        sip_This->CreateQuery(D3DQUERYTYPE_EVENT, &pEvent); \
        pEvent->Issue(D3DISSUE_END); \
        while ( S_FALSE == pEvent->GetData(NULL, 0, D3DGETDATA_FLUSH)); \
        D3D9TraceStats::instance().registerUPIndexVertex(sv_pIndexData, sv_IndexDataFormat, sv_VertexStreamZeroStride); \
		D3D9TraceStats::instance().registerDrawPrimitive(ov_PrimitiveType, true, 0, ov_PrimitiveCount, 1);\
        pEvent->Release(); \
	} \
    if (CHPS) { \
        g_pPSH->Release(); \
    } \
	D3D9TraceStats::instance().endBatch();

#define D3D9_USER_END \
	D3D9TraceStats::instance().endTrace();


// Shader's calls

#define D3D9OP_IDIRECT3DDEVICE9_CREATEVERTEXSHADER_USER_PRE \
	D3D9TraceStats::instance().registerVertexShaderCreation((IDirect3DVertexShader9 *)oip_ppShader, sv_pFunction);

#define D3D9OP_IDIRECT3DDEVICE9_CREATEPIXELSHADER_USER_PRE \
	D3D9TraceStats::instance().registerPixelShaderCreation((IDirect3DPixelShader9 *)oip_ppShader, sv_pFunction);

#define D3D9OP_IDIRECT3DDEVICE9_SETVERTEXSHADER_USER_PRE \
	D3D9TraceStats::instance().registerVertexShaderSetted((IDirect3DVertexShader9 *)oip_pShader);

#define D3D9OP_IDIRECT3DDEVICE9_SETPIXELSHADER_USER_PRE \
	D3D9TraceStats::instance().registerPixelShaderSetted((IDirect3DPixelShader9 *)oip_pShader);


// Texture calls

#define D3D9OP_IDIRECT3DDEVICE9_CREATETEXTURE_USER_PRE \
	D3D9TraceStats::instance().registerCreateTexture(ov_Width, ov_Height, ov_Levels, ov_Usage, ov_Format, ov_Pool, (IDirect3DTexture9 *)oip_ppTexture, (HANDLE *)ov_pSharedHandle);

#define D3D9OP_IDIRECT3DDEVICE9_CREATEVOLUMETEXTURE_USER_PRE \
	D3D9TraceStats::instance().registerCreateVolumeTexture(ov_Width, ov_Height, ov_Depth, ov_Levels, ov_Usage, ov_Format, ov_Pool, (IDirect3DVolumeTexture9 *)oip_ppVolumeTexture, (HANDLE *)ov_pSharedHandle);

#define D3D9OP_IDIRECT3DDEVICE9_CREATECUBETEXTURE_USER_PRE \
	D3D9TraceStats::instance().registerCreateCubeTexture(ov_EdgeLength, ov_Levels, ov_Usage, ov_Format, ov_Pool, (IDirect3DCubeTexture9 *)oip_ppCubeTexture, (HANDLE *)ov_pSharedHandle);

#define D3D9OP_IDIRECT3DDEVICE9_SETTEXTURE_USER_PRE \
	D3D9TraceStats::instance().registerTextureSetted(ov_Stage, (IDirect3DBaseTexture9 *)oip_pTexture);


#define D3D9OP_IDIRECT3DDEVICE9_SETSAMPLERSTATE_USER_PRE \
	D3D9TraceStats::instance().registerSetSamplerState(ov_Sampler, ov_Type, ov_Value);


// State calls 

#define D3D9OP_IDIRECT3DDEVICE9_BEGINSTATEBLOCK_USER_PRE \
	D3D9TraceStats::instance().registerBeginStateBlock();

#define D3D9OP_IDIRECT3DDEVICE9_ENDSTATEBLOCK_USER_PRE \
	D3D9TraceStats::instance().registerEndStateBlock((IDirect3DStateBlock9 *)oip_ppSB);

#define D3D9OP_IDIRECT3DSTATEBLOCK9_APPLY_USER_PRE \
	D3D9TraceStats::instance().registerApplyStateBlock((IDirect3DStateBlock9 *)oip_This);

#define D3D9OP_IDIRECT3DSTATEBLOCK9_CAPTURE_USER_PRE \
	D3D9TraceStats::instance().registerCaptureStateBlock((IDirect3DStateBlock9 *)oip_This);


// Render Target

#define D3D9OP_IDIRECT3DDEVICE9_CREATERENDERTARGET_USER_POST \
	D3D9TraceStats::instance().registerCreateRenderTarget(ov_Width ,ov_Height ,ov_Format ,ov_MultiSample ,ov_MultisampleQuality ,ov_Lockable , (IDirect3DSurface9*) oip_ppSurface, spip_ppSurface , (HANDLE *)ov_pSharedHandle);;

#define D3D9OP_IDIRECT3DDEVICE9_SETRENDERTARGET_USER_PRE \
	D3D9TraceStats::instance().registerSetRenderTarget(ov_RenderTargetIndex, (IDirect3DSurface9 *)oip_pRenderTarget);

#define D3D9OP_IDIRECT3DDEVICE9_GETRENDERTARGET_USER_POST \
	D3D9TraceStats::instance().registerGetRenderTarget(ov_RenderTargetIndex, (IDirect3DSurface9 *)oip_ppRenderTarget, spip_ppRenderTarget);

#define D3D9OP_IDIRECT3DTEXTURE9_GETSURFACELEVEL_USER_POST \
	D3D9TraceStats::instance().registerGetSurfaceLevel(ov_Level, (IDirect3DSurface9 *)oip_ppSurfaceLevel, /*spip_ppSurfaceLevel,*/ (IDirect3DTexture9 *)oip_This);

// Altres

#define D3D9OP_IDIRECT3DDEVICE9_CREATEVERTEXBUFFER_USER_POST \
	D3D9TraceStats::instance().registerCreateVertexBuffer((IDirect3DVertexBuffer9*)oip_ppVertexBuffer, ov_FVF);

#define D3D9OP_IDIRECT3DDEVICE9_SETSTREAMSOURCE_USER_PRE \
    D3D9TraceStats::instance().registerSetStreamSource(ov_StreamNumber, (IDirect3DVertexBuffer9 *)oip_pStreamData ,ov_Stride);

#define D3D9OP_IDIRECT3DDEVICE9_SETVERTEXDECLARATION_USER_PRE \
    D3D9TraceStats::instance().registerSetVertexDeclaration((IDirect3DVertexDeclaration9 *)sip_pDecl);

// Index buffer

#define D3D9OP_IDIRECT3DDEVICE9_CREATEINDEXBUFFER_USER_PRE \
    D3D9TraceStats::instance().registerCreateIndexBuffer(oip_ppIndexBuffer, ov_Format, ov_Length);

#define D3D9OP_IDIRECT3DDEVICE9_SETINDICES_USER_PRE \
    D3D9TraceStats::instance().registerSetIndices(oip_pIndexData);

#define D3D9OP_IDIRECT3DINDEXBUFFER9_LOCK_USER_PRE \
    D3D9TraceStats::instance().registerLockIndexBuffer(oip_This, ov_OffsetToLock, ov_SizeToLock);

#define D3D9OP_IDIRECT3DINDEXBUFFER9_UNLOCK_USER_PRE \
    D3D9TraceStats::instance().registerUnlockIndexBuffer(oip_This, &buffer);

#define D3D9OP_IDIRECT3DDEVICE9_SETRENDERSTATE_USER_PRE \
    D3D9TraceStats::instance().registerSetRenderState(ov_State ,ov_Value);


#define D3D9OP_IDIRECT3DDEVICE9_CLEAR_USER_PRE \
    D3D9TraceStats::instance().registerClear(ov_Flags, ov_Count);

/*#define D3D9OP_IDIRECT3DDEVICE9_UPDATESURFACE_USER_PRE \
    D3D9TraceStats::instance().registerUpdateSurface(sip_pSourceSurface, sip_pDestinationSurface);*/

#define D3D9OP_IDIRECT3DDEVICE9_STRETCHRECT_USER_PRE \
    D3D9TraceStats::instance().registerStretchRect((IDirect3DSurface9*)oip_pSourceSurface, (IDirect3DSurface9*)oip_pDestSurface, sip_pDestSurface, spv_pDestRect);

#define D3D9OP_IDIRECT3DDEVICE9_GETBACKBUFFER_USER_POST \
    D3D9TraceStats::instance().registerGetBackBuffer((IDirect3DSurface9*)oip_ppBackBuffer, spip_ppBackBuffer);

/*#define D3D9OP_IDIRECT3DDEVICE9_CREATEDEPTHSTENCILSURFACE_USER_PRE \
    oip_ppSurface
	D3D9TraceStats::instance().registerCreateRenderTarget(sv_Width ,sv_Height ,sv_Format ,sv_MultiSample ,sv_MultisampleQuality ,sv_Lockable ,spip_ppSurface ,sv_pSharedHandle);*/

/*#define D3D9OP_IDIRECT3DDEVICE9_CREATEOFFSCREENPLAINSURFACE_USER_PRE \
    oip_ppSurface*/

#define D3D9OP_IDIRECT3DDEVICE9_SETDEPTHSTENCILSURFACE_USER_PRE \
	D3D9TraceStats::instance().registerSetDepthStencilSurface((IDirect3DSurface9*) oip_pNewZStencil);

/*#define D3D9OP_IDIRECT3DDEVICE9_GETDEPTHSTENCILSURFACE_USER_PRE \
	D3D9TraceStats::instance().registerGetDepthStencilSurface(spip_ppZStencilSurface);*/

//#define 

////////////////////////////////////////////////////////////
////////		D3D9Stats Declaration               ////////
////////////////////////////////////////////////////////////

class D3D9Stats;
class D3D9TraceStats;
class D3D9FrameStats;
class D3D9BatchStats;


////////////////////////////////////////////////////////////
////////	  D3D9StatsCollector Declaration        ////////
////////////////////////////////////////////////////////////

class D3D9StatsCollector {

	public:
		D3D9StatsCollector();
		void BatchOpStatsCollector(D3D9StatsCollector* second);
		void FrameOpStatsCollector(D3D9StatsCollector* second);
        void printStats (std::ofstream* f);
		void reset();
		void addDrawPrimitive ();
		void addPrimitiveCount (int primitiveCount);
		void addBeginStateBlock();
		void addEndStateBlock();
		void addApplyStateBlock();
		void addCaptureStateBlock();
		void addDrawPoint(int primitiveCount);
		void addDrawLine(int primitiveCount);
		void addDrawTriangleList(int primitiveCount);
		void addDrawTriangleStript(int primitveCount);
		void addDrawTriangleFan(int primitiveCount);
		void addDrawForceDWORD(int primitiveCount);
		void addCreateRenderTarget();
        void addCreateTexture();
        void addComputedVertices(int numVertex, int numVertex2, int numVertex3, int numVertex4);
		void setActiveSampler(unsigned long long active);
        void setAverageActiveSampler (int active);
        void addAttributes(int numAttrib, int numAttribBytes, int numVertex, int numSVertex);
        void addNumberOfPixelsDrawn(int numberOfPixelsDrawn);
        void addAlphaTest(int alpha);
		int* getFilterVector();
		D3D9ShaderStats* getVertexShaderStats();
		D3D9ShaderStats* getPixelShaderStats();
		D3D9TextureStats* getTextureStats();
		D3D9RenderTargetStats* getRenderTargetStats();

		void collectNvidiaCounters();
        UINT64 getShadedPixel();

	private:
		int numDrawPrimitives;
		int primitiveCount;
		D3D9ShaderStats vertexShaderStats;
		D3D9ShaderStats pixelShaderStats;
		D3D9TextureStats textureStats;
		D3D9RenderTargetStats renderTargetStats;
		int numBeginStateBlock;
		int numEndStateBlock;
		int numApplyStateBlock;
		int numCaptureStateBlock;
		unsigned long long activeSampler;
		double activeSamplerAverage;
		int textureType[14];
		double batchSize;
		int numDrawPoint;
		int numDrawLine;
		int numDrawTriangleList;
		int numDrawTriangleStript;
		int numDrawTriangleFan;
		int numDrawForceDWORD;
		int acumStats;
		int acumSamplerUsage;
		int numCreateRenderTarget;
		int numCreateTexture;
        std::map<DWORD, int> fvfStreams;
		int fvfNumAtributes;
		int fvfBytes;
        int fvfNumAttribAv;
		int fvfBytesAv;
        int fvfNumAttribSV;
		int fvfBytesSV;
        unsigned long long numPixelsDrawn;
        unsigned long long numAlpha;
        unsigned long long numComVertices1;
        unsigned long long numComVertices2;
        unsigned long long numComVertices3;
        unsigned long long numComVertices4;


		UINT64 setup_line_count;
		UINT64 setup_point_count;
		UINT64 setup_triangle_count;
		UINT64 setup_primitive_count;
		UINT64 setup_primitive_culled_count;
		UINT64 setup_line_countC;
		UINT64 setup_point_countC;
		UINT64 setup_triangle_countC;
		UINT64 setup_primitive_countC;
		UINT64 setup_primitive_culled_countC;

		UINT64 rop_samples_killed_by_earlyz_count;
		UINT64 rop_samples_killed_by_latez_count;
		UINT64 rop_samples_killed_by_earlyz_countC;
		UINT64 rop_samples_killed_by_latez_countC;

		UINT64 texture_waits_for_fb;
		UINT64 texture_waits_for_fbC;

		UINT64 rasterizer_tiles_in_count;
		UINT64 rasterizer_tiles_killed_by_zcull_count;
		UINT64 rasterizer_tiles_in_countC;
		UINT64 rasterizer_tiles_killed_by_zcull_countC;

		UINT64 shaded_pixel_count;
		UINT64 shaded_pixel_countC;

};


////////////////////////////////////////////////////////////
////////			D3D9Stats Declaration		    ////////
////////////////////////////////////////////////////////////


class D3D9Stats {

	public:
		D3D9Stats();
		virtual bool isTrace();
		virtual bool isFrame();
		virtual bool isBatch();
		virtual D3D9FrameStats* getBackFrame();
		virtual D3D9BatchStats* getBackBatch();

		D3D9StatsCollector* getStadistics();

		void registerDrawPrimitive(D3DPRIMITIVETYPE type, bool indexed, UINT startIndex, UINT primitiveCount, int numberOfPixelsDrawn);

		void registerVertexShaderCreation(IDirect3DVertexShader9 *sh, DWORD *function);
		void registerPixelShaderCreation(IDirect3DPixelShader9 *sh, DWORD *function);
		void registerCreateTexture(	UINT Width,	UINT Height, UINT Levels, DWORD Usage, 
									D3DFORMAT Format, D3DPOOL Pool,  IDirect3DTexture9* pTexture,
									HANDLE* pSharedHandle);

		void registerCreateVolumeTexture(	UINT Width,	UINT Height, UINT Depth,
											UINT Levels, DWORD Usage, D3DFORMAT Format,
											D3DPOOL Pool, IDirect3DVolumeTexture9* pVolumeTexture,
											HANDLE* pSharedHandle);

		void registerCreateCubeTexture(	UINT EdgeLength, UINT Levels, DWORD Usage,
										D3DFORMAT Format, D3DPOOL Pool,	IDirect3DCubeTexture9 * pCubeTexture,
										HANDLE* pSharedHandle);

		void registerVertexShaderSetted(IDirect3DVertexShader9 *sh);
		void registerPixelShaderSetted(IDirect3DPixelShader9 *sh);
		void registerTextureSetted(DWORD sampler, IDirect3DBaseTexture9 *tex);
		void registerSetSamplerState( DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
		void registerCreateVertexBuffer(IDirect3DVertexBuffer9* sip_ppVertexBuffer, DWORD sv_FVF);
        void registerSetStreamSource(UINT sv_StreamNumber, IDirect3DVertexBuffer9* sip_pStreamData ,UINT sv_Stride);
        void registerSetVertexDeclaration(IDirect3DVertexDeclaration9* sip_pDecl);
		void collectStats();

		// State
		void registerBeginStateBlock();
		void registerEndStateBlock(IDirect3DStateBlock9 *sb);
		void registerApplyStateBlock(IDirect3DStateBlock9 *sb);
		void registerCaptureStateBlock(IDirect3DStateBlock9 *sb);
		void shadersSetted(IDirect3DDevice9 * sip_This);


		void registerCreateRenderTarget(	UINT Width,UINT Height,D3DFORMAT Format,
											D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,
											BOOL Lockable, IDirect3DSurface9* oip_ppSurface, IDirect3DSurface9** ppSurface, 
                                            HANDLE* pSharedHandle);
		void registerSetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9 * pRenderTarget );
		void registerGetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* oip_ppRenderTarget, IDirect3DSurface9** pRenderTarget);
		void registerGetSurfaceLevel(UINT sv_Level, IDirect3DSurface9* oip_ppSurfaceLevel, /*IDirect3DSurface9** spip_ppSurfaceLevel,*/ IDirect3DTexture9 * texture);

        // Index Buffer
        void registerCreateIndexBuffer(unsigned int oip_ppIndexBuffer, D3DFORMAT ov_Format, UINT Length);
        void registerSetIndices(unsigned int oip_pIndexData);
        void registerUnlockIndexBuffer(unsigned int oip_This, D3DBuffer* buffer);
        void registerLockIndexBuffer(unsigned int oip_This, UINT ov_OffsetToLock, UINT ov_SizeToLock);

        void registerUPIndexVertex(void* IndexData, D3DFORMAT Format, UINT Bytes);

        void registerSetRenderState(D3DRENDERSTATETYPE State , DWORD Value);

        void registerClear(DWORD ov_Flags, DWORD ov_Count);

        void registerStretchRect(IDirect3DSurface9* oip_pSourceSurface, IDirect3DSurface9* oip_pDestSurface, IDirect3DSurface9* sip_pDestSurface, RECT* spv_pDestRect);
        void registerGetBackBuffer(IDirect3DSurface9* oip_ppBackBuffer, IDirect3DSurface9** spip_ppBackBuffer);
        
		void registerSetDepthStencilSurface(IDirect3DSurface9 * sip_pNewZStencil);
		//void registerGetDepthStencilSurface(IDirect3DSurface9** spip_ppZStencilSurface);

        int getNumFrame();

        ID3DXBuffer* getPSBuffer();

	protected:
		D3D9StatsCollector stadistics;
		int tmpNumBatch;
        int tmpNumFrame;
	    ID3DXBuffer* i_buffer;

        bool nextIsUP;
        void* UPindexs;
        D3DFORMAT UPformat;
        int UPbytes;

        std::list<D3D9RenderTargetOp*> listRTOp;

	private:
};

////////////////////////////////////////////////////////////
////////		D3D9BatchStats Declaration          ////////
////////////////////////////////////////////////////////////

#ifdef BATCH_STATS
class D3D9BatchStats : public D3D9Stats {

	public:
		D3D9BatchStats();
		bool isBatch();
        void setStadisticsFile(std::ofstream* _fb);
		void writeStadistics();
		void resetNumBatch();
	private:
        std::ofstream* fb;
		int numBatch;
};
#endif

////////////////////////////////////////////////////////////
////////		D3D9FrameStats Declaration          ////////
////////////////////////////////////////////////////////////

class D3D9FrameStats : public D3D9Stats {

	public:
		D3D9FrameStats();
		void beginBatch();
		void endBatch();
		void endFrame();
		bool isFrame();
#ifdef BATCH_STATS
		D3D9BatchStats* getBackBatch();
#endif

#ifdef BATCH_STATS
        void setStadisticsFiles(std::ofstream* _ff, std::ofstream* _fb);
#else
        void setStadisticsFiles(std::ofstream* _ff);
#endif
		void writeStadistics();

	private:
        std::ofstream* ff;

#ifdef BATCH_STATS
        std::ofstream* fb;
		D3D9BatchStats batch;
#endif

		int numFrame;
};

////////////////////////////////////////////////////////////
////////		D3D9TraceStats Declaration          ////////
////////////////////////////////////////////////////////////

class D3D9TraceStats : public D3D9Stats {

	public:
		static D3D9TraceStats &instance();
		void beginBatch();
		void endBatch();
		void endFrame();
		void endTrace();
		bool isTrace();
		D3D9FrameStats* getBackFrame();

	private:
		D3D9TraceStats();
		D3D9FrameStats frame;
        std::ofstream ft;
        std::ofstream ff;

#ifdef BATCH_STATS
        std::ofstream fb;
#endif
};

#endif
