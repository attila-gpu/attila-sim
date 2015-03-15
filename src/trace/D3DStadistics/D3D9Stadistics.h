/*****************************************************************************
Several classes for register and calculate stadistics in Direct3D9.

Author: José Manuel Solís
*****************************************************************************/

#ifndef __D3D9STADISTICS
#define __D3D9STADISTICS

#include <support.h>
#include <UserStats.h>
#include <StatsUtils.h>
#include <D3D9ShaderAnalyzer.h>

/******************************************************
Definitions of macros for integrate with player
******************************************************/

#define D3D9OP_IDIRECT3DDEVICE9_PRESENT_USER_PRE \
	D3D9Stadistics::instance().endFrame();

#define D3D9OP_IDIRECT3DDEVICE9_DRAWPRIMITIVE_USER_PRE \
	D3D9Stadistics::instance().beginBatch();\
	D3D9Stadistics::instance().registerDrawPrimitive(ov_PrimitiveType, ov_PrimitiveCount);\
	D3D9Stadistics::instance().endBatch();

#define D3D9OP_IDIRECT3D9_CREATEDEVICE_USER_POST \
	D3D9Stadistics::instance().setMainSubstituteDevice(sip_ppReturnedDeviceInterface);

#define D3D9OP_IDIRECT3DDEVICE9_DRAWPRIMITIVEUP_USER_PRE \
	D3D9Stadistics::instance().beginBatch();\
	D3D9Stadistics::instance().registerDrawPrimitive(ov_PrimitiveType, ov_PrimitiveCount);\
	D3D9Stadistics::instance().endBatch();

#define D3D9OP_IDIRECT3DDEVICE9_DRAWINDEXEDPRIMITIVE_USER_PRE \
	D3D9Stadistics::instance().beginBatch(); \
	D3D9Stadistics::instance().registerDrawPrimitive(ov_Type, ov_primCount);\
	D3D9Stadistics::instance().endBatch();

#define D3D9OP_IDIRECT3DDEVICE9_DRAWINDEXEDPRIMITIVEUP_USER_PRE \
	D3D9Stadistics::instance().beginBatch();\
	D3D9Stadistics::instance().registerDrawPrimitive(ov_PrimitiveType, ov_PrimitiveCount);\
	D3D9Stadistics::instance().endBatch();

#define D3D9OP_IDIRECT3DDEVICE9_CREATEVERTEXSHADER_USER_PRE \
	D3D9Stadistics::instance().registerVertexShaderCreation(oip_ppShader, sv_pFunction);

#define D3D9OP_IDIRECT3DDEVICE9_CREATEPIXELSHADER_USER_PRE \
	D3D9Stadistics::instance().registerPixelShaderCreation(oip_ppShader, sv_pFunction);

#define D3D9OP_IDIRECT3DDEVICE9_SETVERTEXSHADER_USER_PRE \
	D3D9Stadistics::instance().registerVertexShaderSetted(oip_pShader);

#define D3D9OP_IDIRECT3DDEVICE9_SETPIXELSHADER_USER_PRE \
	D3D9Stadistics::instance().registerPixelShaderSetted(oip_pShader);

#define D3D9OP_IDIRECT3DDEVICE9_SETTEXTURE_USER_PRE \
	D3D9Stadistics::instance().registerTextureSetted(ov_Stage, oip_pTexture);

#define D3D9OP_IDIRECT3DDEVICE9_CREATETEXTURE_USER_PRE \
	D3D9Stadistics::instance().registerCreateTexture(ov_Width, ov_Height, ov_Levels, ov_Usage, ov_Format, ov_Pool, oip_ppTexture, ov_pSharedHandle);

#define D3D9OP_IDIRECT3DDEVICE9_CREATEVOLUMETEXTURE_USER_PRE \
	D3D9Stadistics::instance().registerCreateVolumeTexture(ov_Width, ov_Height, ov_Depth, ov_Levels, ov_Usage, ov_Format, ov_Pool, oip_ppVolumeTexture, ov_pSharedHandle);

#define D3D9OP_IDIRECT3DDEVICE9_CREATECUBETEXTURE_USER_PRE \
	D3D9Stadistics::instance().registerCreateCubeTexture(ov_EdgeLength, ov_Levels, ov_Usage, ov_Format, ov_Pool, oip_ppCubeTexture, ov_pSharedHandle);

#define D3D9OP_IDIRECT3DDEVICE9_SETSAMPLERSTATE_USER_PRE \
	D3D9Stadistics::instance().registerSetSamplerState(ov_Sampler, ov_Type, ov_Value);

#define D3D9OP_IDIRECT3DDEVICE9_BEGINSTATEBLOCK_USER_PRE \
	D3D9Stadistics::instance().registerBeginStateBlock();

#define D3D9OP_IDIRECT3DDEVICE9_ENDSTATEBLOCK_USER_PRE \
	D3D9Stadistics::instance().registerEndStateBlock(oip_ppSB);

#define D3D9OP_IDIRECT3DSTATEBLOCK9_APPLY_USER_PRE \
	D3D9Stadistics::instance().registerApplyStateBlock(oip_This);

#define D3D9OP_IDIRECT3DSTATEBLOCK9_CAPTURE_USER_PRE \
	D3D9Stadistics::instance().registerCaptureStateBlock(oip_This);

#define D3D9_USER_BEGIN \
	D3D9Stadistics::instance().setPlayer(this);

#define D3D9_USER_END \
	D3D9Stadistics::instance().endTrace();

/****************************************************************/

// Forward declarations
class D3D9Stadistics;

class D3D9VerticesPerFrame: public workloadStats::UserStat {
	public:
		D3D9VerticesPerFrame();
		D3D9VerticesPerFrame(const std::string &name);
		void endBatch();
		void endFrame();
		void endTrace();
		void printTraceValue(std::ostream& os) const;
		void printFrameValue(std::ostream& os, unsigned int frameIndex) const;
		void printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const;
		void registerVertices(int nvertexs);
	private:
		workloadStats::SumPerFrameAverageTotal<int> collector;
		int currentBatchVertices;
};

class D3D9PrimitivesPerFrame: public workloadStats::UserStat {
	public:
		D3D9PrimitivesPerFrame();
		D3D9PrimitivesPerFrame(const std::string name);
		void endBatch();
		void endFrame();
		void endTrace();
		void printTraceValue(std::ostream& os) const;
		void printFrameValue(std::ostream& os, unsigned int frameIndex) const;
		void printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const;
		void registerPrimitives(int nprimitives);
	private:
		workloadStats::SumPerFrameAverageTotal<int> collector;
		int currentBatchPrimitives;
};

class D3D9BatchesPerFrame: public workloadStats::UserStat {
	public:
		D3D9BatchesPerFrame();
		D3D9BatchesPerFrame(const std::string name);
		void endBatch();
		void endFrame();
		void endTrace();
		void printTraceValue(std::ostream& os) const;
		void printFrameValue(std::ostream& os, unsigned int frameIndex) const;
		void printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const;
	private:
		workloadStats::SumPerFrameAverageTotal<unsigned int> collector;
};

class D3D9PrimitiveUsage: public workloadStats::UserStat {
	public:
		D3D9PrimitiveUsage();
		D3D9PrimitiveUsage(const std::string name);
		void setPrimitive(D3DPRIMITIVETYPE prim);
		void endBatch();
		void endFrame();
		void endTrace();
		void printTraceValue(std::ostream& os) const;
		void printFrameValue(std::ostream& os, unsigned int frameIndex) const;
		void printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const;
		void registerPrimitiveUsage(D3DPRIMITIVETYPE prim);
	private:
		workloadStats::SumPerFrameSumTotal<int> totalPrimitivesCollector;
		workloadStats::SumPerFrameSumTotal<int> thisPrimitiveCollector;
		int totalPrimitivesCount;
		int thisPrimitiveCount;
		D3DPRIMITIVETYPE primitive;
};

class D3D9VertexShaderInstructionSlots: public workloadStats::UserStat {
	public:
		D3D9VertexShaderInstructionSlots();
		D3D9VertexShaderInstructionSlots(const std::string name);
		void endBatch();
		void endFrame();
		void endTrace();
		void printTraceValue(std::ostream& os) const;
		void printFrameValue(std::ostream& os, unsigned int frameIndex) const;
		void printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const;

		/**
			* It's possible that D3D9 implementation reuse interfaces addresses, so to
			* we must associate a identifier to each one in it's creation.
			**/
		void registerVertexShaderCreation(IDirect3DVertexShader9 *sh, DWORD *function);
		void registerVertexShaderSetted(IDirect3DVertexShader9 *sh);
	private:
		workloadStats::PredicatedAveragePerFrameAverageTotal< unsigned int >
			collector;
		std::map<void *, unsigned int> addressesToIdentifiers;
		std::map<unsigned int, void *> identifiersToAddresses;
		std::map<unsigned int, unsigned int> instructionCount;
		IDirect3DVertexShader9 * settedVertexShader;
		unsigned int nextId;
};

class D3D9PixelShaderInstructionSlots: public workloadStats::UserStat {
	public:
		D3D9PixelShaderInstructionSlots();
		D3D9PixelShaderInstructionSlots(const std::string name);
		void endBatch();
		void endFrame();
		void endTrace();
		void printTraceValue(std::ostream& os) const;
		void printFrameValue(std::ostream& os, unsigned int frameIndex) const;
		void printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const;

		/**
			* It's possible that D3D9 implementation reuse interfaces addresses, so to
			* we must associate a identifier to each one in it's creation.
			**/
		void registerPixelShaderCreation(IDirect3DPixelShader9 *sh, DWORD *function);
		void registerPixelShaderSetted(IDirect3DPixelShader9 *sh);
	private:
		workloadStats::PredicatedAveragePerFrameAverageTotal< unsigned int >
			collector;
		std::map<void *, unsigned int> addressesToIdentifiers;
		std::map<unsigned int, void *> identifiersToAddresses;
		std::map<unsigned int, unsigned int> instructionCount;
		IDirect3DPixelShader9 * settedPixelShader;
		unsigned int nextId;
};

class D3D9DifferentShaders: public workloadStats::UserStat {
	public:
		D3D9DifferentShaders();
		D3D9DifferentShaders(const std::string name);
		void endBatch();
		void endFrame();
		void endTrace();
		void printTraceValue(std::ostream& os) const;
		void printFrameValue(std::ostream& os, unsigned int frameIndex) const;
		void printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const;

		/**
			* It's possible that D3D9 implementation reuse interfaces addresses, so to
			* count correctly the shader usage we must associate a identifier to each one
			* in it's creation.
			**/
		void registerVertexShaderCreation(IDirect3DVertexShader9 *sh);
		void registerPixelShaderCreation(IDirect3DPixelShader9 *sh);

		void registerVertexShaderSetted(IDirect3DVertexShader9 *sh);
		void registerPixelShaderSetted(IDirect3DPixelShader9 *sh);
	private:
		workloadStats::ListDifferentPerFrameListDifferentTotal< unsigned int >
			collector;
		std::map<void *, unsigned int> addressesToIdentifiers;
		std::map<unsigned int, void *> identifiersToAddresses;
		IDirect3DVertexShader9 * settedVertexShader;
		IDirect3DPixelShader9 * settedPixelShader;
		unsigned int nextId;
};

class D3D9BatchSize : public workloadStats::UserStat {
public:
		D3D9BatchSize();
		D3D9BatchSize(const std::string name);
		void endBatch();
		void endFrame();
		void endTrace();
		void printTraceValue(std::ostream& os) const;
		void printFrameValue(std::ostream& os, unsigned int frameIndex) const;
		void printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const;
		void registerVertices(int nverts);
private:
		workloadStats::AveragePerFrameAverageTotal<int> collector;
		int currentBatchVertices;
};

class D3D9SampleUnits : public workloadStats::UserStat {
public:
		D3D9SampleUnits();
		D3D9SampleUnits(const std::string name);
		void endBatch();
		void endFrame();
		void endTrace();
		void printTraceValue(std::ostream& os) const;
		void printFrameValue(std::ostream& os, unsigned int frameIndex) const;
		void printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const;
		void registerTextureSetted(DWORD sampler, IDirect3DBaseTexture9 *pTexture);
private:
		workloadStats::AveragePerFrameAverageTotal< int > collector;
		std::set< int > activeSampleUnits;
};

class D3D9SamplingStadistics;

class D3D9TextureStadistics : public workloadStats::UserStat {
public:
		D3D9TextureStadistics();
		void setSamplingStadistics(D3D9SamplingStadistics *ss);
		void endBatch();
		void endFrame();
		void endTrace();
		void printTraceValue(std::ostream& os) const;
		void printFrameValue(std::ostream& os, unsigned int frameIndex) const;
		void printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const;

		void registerCreateTexture(
			UINT Width,
			UINT Height,
			UINT Levels, 
			DWORD Usage, 
			D3DFORMAT Format,
			D3DPOOL Pool, 
			IDirect3DTexture9* ppTexture,
			HANDLE* pSharedHandle);

		void registerCreateVolumeTexture(
			UINT Width,
			UINT Height,
			UINT Depth,
			UINT Levels,
			DWORD Usage,
			D3DFORMAT Format,
			D3DPOOL Pool,
			IDirect3DVolumeTexture9* ppVolumeTexture,
			HANDLE* pSharedHandle);

		void registerCreateCubeTexture(
			UINT EdgeLength,
			UINT Levels,
			DWORD Usage,
			D3DFORMAT Format,
			D3DPOOL Pool,
			IDirect3DCubeTexture9 * ppCubeTexture,
			HANDLE* pSharedHandle);

		void registerSetTexture(
			DWORD Sampler,
			IDirect3DBaseTexture9 * pTexture
			);

		
private:
		struct TextureInfo {
			unsigned int size;
			bool isCompressed;
			TextureInfo();
		};
		workloadStats::ListDifferentPerFrameListDifferentTotal< unsigned int > collector;
		/**
			* It's possible that D3D9 implementation reuse interfaces addresses, so to
			* count correctly the texture reuse we must associate a identifier to each one
			* in it's creation.
			**/
		std::map<void *, unsigned int> addressesToIdentifiers;
		std::map<unsigned int, void *> identifiersToAddresses;
		int nextId;
		
		std::map<unsigned int, unsigned int> settedSamplers;
		std::map<unsigned int, TextureInfo > texturesInfo;

		D3D9SamplingStadistics *samplingStats;
};


class D3D9SamplingStadistics : public workloadStats::UserStat {
public:
	D3D9SamplingStadistics();
	bool isSamplerReferenced(DWORD Sampler);
	void endBatch();
	void endFrame();
	void endTrace();
	void printTraceValue(std::ostream& os) const;
	void printFrameValue(std::ostream& os, unsigned int frameIndex) const;
	void printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const;
	void registerTextureSetted(DWORD sampler, IDirect3DBaseTexture9 *pTexture);
	void registerVertexShaderCreation(IDirect3DVertexShader9 *sh,  DWORD *function);
	void registerPixelShaderCreation(IDirect3DPixelShader9 *sh,  DWORD *function);
	void registerVertexShaderSetted(IDirect3DVertexShader9 *sh);
	void registerPixelShaderSetted(IDirect3DPixelShader9 *sh);
	void registerSetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
private:

	enum SamplingCategory {
		NEAREST,
		NEAREST_MIPMAP_LINEAR,
		BILINEAR,
		BILINEAR_ANISO,
		TRILINEAR,
		TRILINEAR_ANISO,
		UNKNOWN
	};

	struct SamplerStatus {
		IDirect3DBaseTexture9 *texture;
		D3DTEXTUREFILTERTYPE magFilter;
		D3DTEXTUREFILTERTYPE minFilter;
		D3DTEXTUREFILTERTYPE mipFilter;
		DWORD maxAnisotropy;

		SamplerStatus():
		texture(0),
		magFilter(D3DTEXF_POINT),
		minFilter(D3DTEXF_POINT),
		mipFilter(D3DTEXF_NONE),
		maxAnisotropy(1) {}

		SamplingCategory getCategory() {
			if((minFilter == D3DTEXF_POINT) &
				((mipFilter == D3DTEXF_POINT) |
				(mipFilter == D3DTEXF_NONE)))
				return NEAREST;
			else if((minFilter == D3DTEXF_POINT) &
				(mipFilter == D3DTEXF_LINEAR))
				return NEAREST_MIPMAP_LINEAR;
			else if((minFilter == D3DTEXF_LINEAR) &
				((mipFilter == D3DTEXF_POINT) |
				(mipFilter == D3DTEXF_NONE)))
				return BILINEAR;
			else if((minFilter == D3DTEXF_ANISOTROPIC) &
				((mipFilter == D3DTEXF_POINT) |
				(mipFilter == D3DTEXF_NONE)) &
				(maxAnisotropy > 1))
				return BILINEAR_ANISO;
			else if((minFilter == D3DTEXF_LINEAR) &
				(mipFilter == D3DTEXF_LINEAR))
				return TRILINEAR;
			else if((minFilter == D3DTEXF_ANISOTROPIC) &
				(mipFilter == D3DTEXF_LINEAR) &
				(maxAnisotropy > 1))
				return TRILINEAR_ANISO;
			else return UNKNOWN;
		}
	};
	
	std::map< DWORD, SamplerStatus > samplers;
	std::map<void *, unsigned int> addressesToIdentifiers;
	std::map<unsigned int, void *> identifiersToAddresses;
	unsigned int nextId;

	std::map< unsigned int, D3D9ShaderAnalyzer::D3D9ShaderAnalysis > shaderAnalysis;
	IDirect3DVertexShader9 *vsh;
	IDirect3DPixelShader9 *psh;

	workloadStats::PredicatedAveragePerFrameAverageTotal<int> textureInstructionsCollector;
	workloadStats::PredicatedAveragePerFrameAverageTotal<int> activeSamplersCollector;
	workloadStats::PredicatedAveragePerFrameAverageTotal<float> nearestCollector;
	workloadStats::PredicatedAveragePerFrameAverageTotal<float> nearestMipMapCollector;

	workloadStats::PredicatedAveragePerFrameAverageTotal<float> bilinearCollector;
	workloadStats::PredicatedAveragePerFrameAverageTotal<float> bilinearAnisoCollector;

	workloadStats::PredicatedAveragePerFrameAverageTotal<float> trilinearCollector;
	workloadStats::PredicatedAveragePerFrameAverageTotal<float> trilinearAnisoCollector;
};

class D3D9TextureReuse : public workloadStats::UserStat {
public:
		D3D9TextureReuse();
		D3D9TextureReuse(const std::string name);
		void endBatch();
		void endFrame();
		void endTrace();
		void printTraceValue(std::ostream& os) const;
		void printFrameValue(std::ostream& os, unsigned int frameIndex) const;
		void printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const;
		void registerTextureCreation(IDirect3DBaseTexture9 *pTexture);
		void registerTextureSetted(DWORD sampler, IDirect3DBaseTexture9 *pTexture);
private:
		workloadStats::ListDifferentPerFrameListDifferentTotal< unsigned int > collector;
		/**
			* It's possible that D3D9 implementation reuse interfaces addresses, so to
			* count correctly the texture reuse we must associate a identifier to each one
			* in it's creation.
			**/
		std::map<void *, unsigned int> addressesToIdentifiers;
		std::map<unsigned int, void *> identifiersToAddresses;
		std::map<unsigned int , unsigned int > activeSampleUnits;
		int nextId;
};

class D3D9StateChangesPerFrame : public workloadStats::UserStat {
public:
		D3D9StateChangesPerFrame();
		D3D9StateChangesPerFrame(const std::string name);
		void endBatch();
		void endFrame();
		void endTrace();
		void printTraceValue(std::ostream& os) const;
		void printFrameValue(std::ostream& os, unsigned int frameIndex) const;
		void printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const;
		void setD3D9Stadistics(D3D9Stadistics *s);
private:
		workloadStats::AveragePerFrameAverageTotal< int > collector;
		D3D9Stadistics *stats;
		unsigned long lastCallOffset;
};

/**
	To take stadistics more accurately state blocks should be
	monitorized. This class updates D3D9Stadistics object when
	a state block is applied. Currently only supports state blocks
	recorded using the begin/end stateblock pair:

	device->BeginStateBlock();
	...
	device->EndStateBlock(&sb);
	sb->Apply();

	Not supports CreateStateBlock method.
*/
class D3D9StateBlockHandler {
public:
	D3D9StateBlockHandler();
	void setD3D9Stadistics(D3D9Stadistics *st);
	void beginStateBlock();
	void endStateBlock(IDirect3DStateBlock9 *sb);
	void applyStateBlock(IDirect3DStateBlock9 *sb);
	void captureStateBlock(IDirect3DStateBlock9 *sb);
	void setSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
	void setTexture( DWORD Sampler, IDirect3DBaseTexture9 * pTexture);
	void setVertexShader( IDirect3DVertexShader9* pShader);
	void setPixelShader( IDirect3DPixelShader9* pShader);
private:
	struct SamplerStatusRecord {
		bool textureSetted;
		bool magFilterSetted;
		bool minFilterSetted;
		bool mipFilterSetted;
		bool maxAnisotropySetted;
		IDirect3DBaseTexture9 *texture;
		D3DTEXTUREFILTERTYPE magFilter;
		D3DTEXTUREFILTERTYPE minFilter;
		D3DTEXTUREFILTERTYPE mipFilter;
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

	struct StateBlockRecord {
		std::map<DWORD, SamplerStatusRecord> samplerStatus;
		bool vertexShaderSetted;
		bool pixelShaderSetted;
		IDirect3DVertexShader9 *vertexShader;
		IDirect3DPixelShader9 *pixelShader;
		StateBlockRecord():
			vertexShaderSetted(false),
			pixelShaderSetted(false),
			vertexShader(0),
			pixelShader(0) {}
	};

	/**
	* It's possible that D3D9 implementation reuse interfaces addresses, so
	* we must associate a identifier to each state block.
	**/
	std::map<void *, unsigned int> addressesToIdentifiers;
	std::map<unsigned int, void *> identifiersToAddresses;
	std::map<unsigned int, StateBlockRecord> stateBlockRecords;
	unsigned int nextId;

	bool insideBeginEnd;
	StateBlockRecord current;
	void unsetCurrentStateBlock();
	D3D9Stadistics *stats;
};

class D3D9Queries : public workloadStats::UserStat {
public:
	D3D9Queries();
	void setMainSubstituteDevice(IDirect3DDevice9 *dev);
	void endBatch();
	void endFrame();
	void endTrace();
	void printTraceValue(std::ostream& os) const;
	void printFrameValue(std::ostream& os, unsigned int frameIndex) const;
	void printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const;
private:
	IDirect3DDevice9 *mainDevice;
	IDirect3DQuery9 *vertexStatsQuery;
	workloadStats::SumPerFrameAverageTotal< unsigned int > renderedCollector;
	workloadStats::SumPerFrameAverageTotal< unsigned int > extraCollector;
};

class D3D9Stadistics {
	public:
		static D3D9Stadistics &instance();

		void setPlayer(D3DPlayer *p);
		void setMainSubstituteDevice(IDirect3DDevice9 *dev);

		void registerDrawPrimitive(D3DPRIMITIVETYPE type, int primitiveCount);
		void registerVertexShaderCreation(IDirect3DVertexShader9 *sh, DWORD *function);
		void registerPixelShaderCreation(IDirect3DPixelShader9 *sh, DWORD *function);
		void registerVertexShaderSetted(IDirect3DVertexShader9 *sh);
		void registerPixelShaderSetted(IDirect3DPixelShader9 *sh);
		void registerTextureSetted(DWORD sampler, IDirect3DBaseTexture9 *tex);

		void registerCreateTexture(
			UINT Width,	UINT Height, UINT Levels, DWORD Usage, 
			D3DFORMAT Format, D3DPOOL Pool,  IDirect3DTexture9* pTexture,
			HANDLE* pSharedHandle);

		void registerCreateVolumeTexture(
			UINT Width,	UINT Height, UINT Depth,
			UINT Levels, DWORD Usage, D3DFORMAT Format,
			D3DPOOL Pool, IDirect3DVolumeTexture9* pVolumeTexture,
			HANDLE* pSharedHandle);

		void registerCreateCubeTexture(
			UINT EdgeLength, UINT Levels, DWORD Usage,
			D3DFORMAT Format, D3DPOOL Pool,	IDirect3DCubeTexture9 * pCubeTexture,
			HANDLE* pSharedHandle);

		void registerSetSamplerState( DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
		void registerBeginStateBlock();
		void registerEndStateBlock(IDirect3DStateBlock9 *sb);
		void registerApplyStateBlock(IDirect3DStateBlock9 *sb);
		void registerCaptureStateBlock(IDirect3DStateBlock9 *sb);

		void beginBatch();
		void endBatch();
		void endFrame();
		void endTrace();

		unsigned long getCallOffset();
	private:
		D3D9Stadistics();

		D3DPlayer *player;
		D3D9StateBlockHandler sbh;

		D3D9VerticesPerFrame vpf;
		D3D9PrimitivesPerFrame ppf;
		D3D9BatchesPerFrame bpf;
		D3D9PrimitiveUsage pu[6];
		D3D9DifferentShaders dsh;
		D3D9BatchSize bs;
		D3D9SampleUnits su;
		D3D9TextureReuse tr;
		D3D9StateChangesPerFrame sc;
		D3D9VertexShaderInstructionSlots vshis;
		D3D9PixelShaderInstructionSlots pshis;
		D3D9SamplingStadistics ss;
		D3D9TextureStadistics ts;
		D3D9Queries que;
};

#endif
