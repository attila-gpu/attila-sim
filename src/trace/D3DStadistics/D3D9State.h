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

#ifndef __D3D9STATE
#define __D3D9STATE

#include <d3d9.h>
#include <map>
#include <list>

#include "D3DBuffer.h"

#include "D3D9ShaderStats.h"
#include "D3D9TextureStats.h"
#include "D3D9RenderTargetStats.h"


////////////////////////////////////////////////////////////
////////			D3D9State Declaration			////////
////////////////////////////////////////////////////////////

// Types of filters that can be set.
// Only most common aniso levels are considered.
enum samplingCategory {
	NEAREST,
	NEAREST_MIPMAP_LINEAR,
	BILINEAR,
	BILINEAR_ANISO_2,
	BILINEAR_ANISO_4,
	BILINEAR_ANISO_8,
	BILINEAR_ANISO_16,
	TRILINEAR,
	TRILINEAR_ANISO_2,
	TRILINEAR_ANISO_4,
	TRILINEAR_ANISO_8,
	TRILINEAR_ANISO_16,
	UNKNOWN
};

class D3D9State {
	public:
		static D3D9State &instance();

        // Adds a new vertex shader to the state.
		void addShader(IDirect3DVertexShader9 *sh, DWORD *function);

        // Adds a new pixel shader to the state.
		void addShader(IDirect3DPixelShader9 *sh, DWORD *function);

        // Adds a new 2D texture to the state.
		void addTexture(	UINT Width,	UINT Height, UINT Levels, DWORD Usage, 
							D3DFORMAT Format, D3DPOOL Pool,  IDirect3DTexture9* pTexture,
							HANDLE* pSharedHandle);

        // Adds a new volume texture to the state.
		void addVolumeTexture(	UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, 
								D3DFORMAT Format, D3DPOOL Pool,  IDirect3DVolumeTexture9* pTexture,
								HANDLE* pSharedHandle);

        // Adds a new cube map texture to the state.
		void addCubeTexture(	UINT EdgeLength, UINT Levels, DWORD Usage,
								D3DFORMAT Format, D3DPOOL Pool,	IDirect3DCubeTexture9 * pCubeTexture,
								HANDLE* pSharedHandle);

        // Adds a new surface used as render target to the state.
		void addRenderTarget(	UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,
								DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9* oip_ppSurface, 
                                IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle);

        // Adds a new vertex buffer to the state.
        void addCreateVertexBuffer(IDirect3DVertexBuffer9* sip_ppVertexBuffer, DWORD sv_FVF);


        // Sets a texture to use to the specified sampler.
		void setTexture(DWORD sampler, IDirect3DBaseTexture9 *tex);

        // Configures a parameter of the specified sampler.
		void setSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);

        // Sets the vertex shader to use.
		void setVertexShader(IDirect3DVertexShader9 *sh);

        // Sets the pixel shader to use.
		void setPixelShader(IDirect3DPixelShader9 *sh);


        // Sets if alpha test is enabled or disabled.
        void setAlphaTest(bool e);

        // Sets if z write is enabled or disabled.
        void setZWriteEnable(bool z);


        // Sets a render target to use to the specified index.
		void setRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9 * pRenderTarget );

        // Sets a render target to use to the specified index.
		void setDepthStencilSurface(IDirect3DSurface9 * pRenderTarget );

        // Gets the render target from the specified index.
		void getRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget, IDirect3DSurface9** ppRenderTarget);

        // Gets a surface from the specified texture.
		void getSurfaceLevel(UINT sv_Level, IDirect3DSurface9* oip_ppSurfaceLevel, /*IDirect3DSurface9** spip_ppSurfaceLevel,*/ IDirect3DTexture9 * texture);


        // Returns statistics of a specified vertex shader.
		D3D9ShaderStats* getShaderStats (IDirect3DVertexShader9 *sh);
        
        // Returns statistics of a specified pixel shader.
		D3D9ShaderStats* getShaderStats (IDirect3DPixelShader9 *sh);
        
        // Returns statistics of a specified texture.
        D3D9TextureStats* getTextureStats(void *tex);

        // Returns statistics of a specified render target.
		D3D9RenderTargetStats* getRenderTargetStats(void *rt);

        // Returns statistics of a specified render target with its identifier.
		D3D9RenderTargetStats* getRenderTargetStatsIdent(unsigned int rt);

        // Returns statistics of the current vertex shader.
		D3D9ShaderStats* getActualVshStats();

        // Returns statistics of the current pixel shader.
		D3D9ShaderStats* getActualPshStats();

        // Returns statistics of the current textures.
		D3D9TextureStats* getActualTextureStats();

        // Returns statistics of the current render target.
		D3D9RenderTargetStats* getActualRenderTargetStats();


        // Returns the internal identifier that will be asigned to the next object added.
		int getNextId();

        // ????
		int getSamplerUsage ();

        // Starts to record state calls to save in an state block.
		void beginStateBlock();

        // Saves the actual state in an state block.
		void endStateBlock(IDirect3DStateBlock9 *sb);

        // Applies the state saved in the state block.
		void applyStateBlock(IDirect3DStateBlock9 *sb);

        // ????
		void captureStateBlock(IDirect3DStateBlock9 *sb);
		unsigned long long getSamplingCategory (int* samplingType, unsigned long long exec); 
        int getTotalSize(std::list <void *> differentTexture);
        int getTotalSize(std::list <IDirect3DSurface9 *> differentRenderTarget);
        int getStreamerNumAttrib(IDirect3DVertexBuffer9* StreamData);
		bool isATexture(void* pTexture);
        std::list <D3D9TextureStats*> getDifferentTextureStats(std::list <void *> differentTexture);
        std::list <D3D9RenderTargetStats*> getDifferentRenderTargetStats(std::list <IDirect3DSurface9 *> differentRenderTarget);
		IDirect3DVertexShader9 * getActualVertexShader();
		IDirect3DPixelShader9 * getActualPixelShader();
        bool getAlphaTest();
		unsigned int getLastApplyStateBlock();
        int getNumAttrib();
        int getNumAttribBytes();
        int getNumAttribUP();
        void addSetStreamSource(UINT sv_StreamNumber, IDirect3DVertexBuffer9* sip_pStreamData ,UINT sv_Stride);
        void addVertexDeclaration(IDirect3DVertexDeclaration9* sip_pDecl);
        
        void addCreateIndexBuffer(void* oip_ppIndexBuffer, D3DFORMAT ov_Format, UINT Length);
        void setIndexBuffer(void* oip_pIndexData);
        void setLockIndexBuffer(void* oip_This, UINT OffsetToLock, UINT SizeToLock);
        void setUnlockIndexBuffer(void* oip_This, D3DBuffer* buffer);
        void getNumIndices(UINT StartIndex, UINT numVertex, int* numVertices1, int* numVertices2, int* numVertices3, int* numVertices4);
        void getNumIndicesUP(UINT numVertex, int* numVertices1, int* numVertices2, int* numVertices3, int* numVertices4, void* indexBuffer, D3DFORMAT format);

        void setColorWriteEnable(DWORD color);

        // Calls related with the render target dependences graphs.
        //
        void clearRTOp(DWORD ov_Flags, DWORD ov_Count);
        void resetRTOp();
        void resetDSOp();
        D3D9RenderTargetOp* getRTOp();
        D3D9RenderTargetOp* getDSOp();
        void addTexSetToRTOp();
        D3D9RenderTargetOp* addTexToRTOp(IDirect3DSurface9* sip_pSourceSurface, IDirect3DSurface9* sip_pDestSurface/*, D3D9RenderTargetOp** tmpSR*/);
        bool isRTSet(IDirect3DSurface9* surface);
        void addNumberOfPixelsDrawn(UINT64 num);

        unsigned int getTexIformRT(unsigned int t);
        unsigned int getIdentifier(void* p);
        bool isRTBackBuffer(unsigned int t);

	private:
		D3D9State();

		struct SamplerStatusRecord {

			bool textureSetted;
			IDirect3DBaseTexture9 *texture;

			bool magFilterSetted;
			D3DTEXTUREFILTERTYPE magFilter;

			bool minFilterSetted;
			D3DTEXTUREFILTERTYPE minFilter;

			bool mipFilterSetted;
			D3DTEXTUREFILTERTYPE mipFilter;

			bool maxAnisotropySetted;
			DWORD maxAnisotropy;

			SamplerStatusRecord():
				textureSetted(false), magFilterSetted(false),
				minFilterSetted(false), mipFilterSetted(false),
				texture(0),
				magFilter(D3DTEXF_POINT),
				minFilter(D3DTEXF_POINT),
				mipFilter(D3DTEXF_NONE),
				maxAnisotropy(1) {}

		};

        // This struct have the information necessary used to save and restore state blocks.
		struct StateBlockRecord {

            std::map<DWORD, SamplerStatusRecord> samplerStatus;

			bool vertexShaderSetted;
			IDirect3DVertexShader9 *vertexShader;

			bool pixelShaderSetted;
			IDirect3DPixelShader9 *pixelShader;

            bool numAttribSetted;
            unsigned int numAttrib;

            bool alphaTestSetted;
            bool alphaTestEnabled;

            bool colorWriteEnableSetted;
            bool rEnable;
            bool gEnable;
            bool bEnable;
            bool aEnable;

            bool zWriteEnableSetted;
            bool zWriteEnable;

			StateBlockRecord():
				vertexShaderSetted(false),
				pixelShaderSetted(false),
                numAttribSetted(false),
                alphaTestSetted(false),
                colorWriteEnableSetted(false),
                alphaTestEnabled(false),
                zWriteEnableSetted(false),
                zWriteEnable(true),
                rEnable(true),
                gEnable(true),
                bEnable(true),
                aEnable(true),
				vertexShader(0),
				pixelShader(0),
                numAttrib(0) {}

		};
		
        struct StreamerStats {
            int numAttrib;
            int size;
        };

        struct IndexBufferStats {
            D3DBuffer buffer;
            D3DFORMAT format;
            UINT OffLock;
            UINT SizeLock;
        };

        // This maps contain statistics of each object created.
        std::map<unsigned int, D3D9TextureStats*> textureStats;
        std::map<unsigned int, D3D9ShaderStats*> shaderStats;
        std::map<unsigned int, D3D9RenderTargetStats*> renderTargetStats;
        std::map<unsigned int, StreamerStats> streamerStats;
        std::map<unsigned int, IndexBufferStats> indexBufferStats;

        std::map<unsigned int, void *> settedStreamers;

        // This maps 
        std::map<void *, unsigned int> addressesToIdentifiers;
        std::map<unsigned int, void *> identifiersToAddresses;

        std::map<unsigned int, StateBlockRecord> stateBlockRecords;

        // Internat identifier to asign to the next object created.
		unsigned int nextId;

		bool insideBeginEnd;
		StateBlockRecord current;
		StateBlockRecord actual;

        std::map<unsigned int, unsigned int> settedRenderTargets;

        unsigned int setDepthStencil;

		void unsetCurrentStateBlock();
		void saveStateBlockList();

		unsigned int lastApplyStateBlock;

        unsigned int settedIndexBuffer;

        std::map <unsigned int, void *> surfaceStats;
        //std::map <unsigned int, unsigned int> rtToTextures;
        
        D3D9RenderTargetOp* rtOp;
        D3D9RenderTargetOp* dsOp;
        unsigned int lastRTorigSR;

};

#endif
