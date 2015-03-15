#include "stdafx.h"
#include "D3DPlayer.h"
#include "D3D9Info.h"
#include "support.h"
#include "UserStats.h"
#include "StatsUtils.h"
#include "StatsManager.h"
#include "D3D9ShaderAnalyzer.h"
#include "D3D9Stadistics.h"
#include <map>
#include <list>


// Enable log of samplers status
// #define SAMPLERSTATUSLOG

// Enable log of state blocks
// #define STATEBLOCKLOG

#ifdef SAMPLERSTATUSLOG
#include <fstream>
#endif

using namespace std;
using namespace workloadStats;

// Enable similarity matrices generation 
// #define SIMILARITYMATRICES


D3D9VerticesPerFrame::D3D9VerticesPerFrame(): UserStat(),
currentBatchVertices(0) {}

D3D9VerticesPerFrame::D3D9VerticesPerFrame(const std::string &name): UserStat(name),
currentBatchVertices(0) {}

void D3D9VerticesPerFrame::endBatch() {
	collector.endBatch(currentBatchVertices);
	currentBatchVertices = 0;
}

void D3D9VerticesPerFrame::endFrame() {
	collector.endFrame();
}

void D3D9VerticesPerFrame::endTrace() {
	collector.endTrace();
}

void D3D9VerticesPerFrame::printTraceValue(std::ostream& os) const {
	os << collector.getTraceValue();
}

void D3D9VerticesPerFrame::printFrameValue(std::ostream& os, unsigned int frameIndex) const {
	os << collector.getFrameValue(frameIndex);
}

void D3D9VerticesPerFrame::printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const {
	os << collector.getBatchValue(frameIndex, batchIndex);
}

void D3D9VerticesPerFrame::registerVertices(int nvertexs) {
	currentBatchVertices += nvertexs;
}

D3D9PrimitivesPerFrame::D3D9PrimitivesPerFrame(): UserStat(),
currentBatchPrimitives(0) {}

D3D9PrimitivesPerFrame::D3D9PrimitivesPerFrame(const std::string name): UserStat(name),
currentBatchPrimitives(0) {}

void D3D9PrimitivesPerFrame::endBatch() {
	collector.endBatch(currentBatchPrimitives);
	currentBatchPrimitives = 0;
}

void D3D9PrimitivesPerFrame::endFrame() {
	collector.endFrame();
}

void D3D9PrimitivesPerFrame::endTrace() {
	collector.endTrace();
}

void D3D9PrimitivesPerFrame::printTraceValue(std::ostream& os) const {
	os << collector.getTraceValue();
}

void D3D9PrimitivesPerFrame::printFrameValue(std::ostream& os, unsigned int frameIndex) const {
	os << collector.getFrameValue(frameIndex);
}

void D3D9PrimitivesPerFrame::printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const {
	os << collector.getBatchValue(frameIndex, batchIndex);
}

void D3D9PrimitivesPerFrame::registerPrimitives(int nprimitives) {
	currentBatchPrimitives += nprimitives;
}


D3D9BatchesPerFrame::D3D9BatchesPerFrame(): UserStat() {}

D3D9BatchesPerFrame::D3D9BatchesPerFrame(const std::string name): UserStat(name) {}

void D3D9BatchesPerFrame::endBatch() {
	collector.endBatch(1);
}

void D3D9BatchesPerFrame::endFrame() {
	collector.endFrame();
}

void D3D9BatchesPerFrame::endTrace() {
	collector.endTrace();
}

void D3D9BatchesPerFrame::printTraceValue(std::ostream& os) const {
	os << collector.getTraceValue();
}

void D3D9BatchesPerFrame::printFrameValue(std::ostream& os, unsigned int frameIndex) const {
	os << collector.getFrameValue(frameIndex);
}

void D3D9BatchesPerFrame::printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const {
	os << collector.getBatchValue(frameIndex, batchIndex);
}

D3D9PrimitiveUsage::D3D9PrimitiveUsage(): UserStat(),
primitive(D3DPT_TRIANGLELIST),
totalPrimitivesCount(0),
thisPrimitiveCount(0)
{}

D3D9PrimitiveUsage::D3D9PrimitiveUsage(const std::string name): UserStat(name),
primitive(D3DPT_TRIANGLELIST),
totalPrimitivesCount(0),
thisPrimitiveCount(0)
{}

void D3D9PrimitiveUsage::endBatch() {
	thisPrimitiveCollector.endBatch(thisPrimitiveCount);
	totalPrimitivesCollector.endBatch(totalPrimitivesCount);
	thisPrimitiveCount = 0;
	totalPrimitivesCount = 0;
}

void D3D9PrimitiveUsage::endFrame() {
	thisPrimitiveCollector.endFrame();
	totalPrimitivesCollector.endFrame();
}

void D3D9PrimitiveUsage::endTrace() {
	thisPrimitiveCollector.endTrace();
	totalPrimitivesCollector.endTrace();
}

void D3D9PrimitiveUsage::printTraceValue(std::ostream& os) const {
	int parcial = thisPrimitiveCollector.getTraceValue();
	int total = totalPrimitivesCollector.getTraceValue();
	float percentUsage =
		float(parcial) /
		float((total <= 0)? 1: total);
	os << percentUsage;
}

void D3D9PrimitiveUsage::printFrameValue(std::ostream& os, unsigned int frameIndex) const {
	int parcial = thisPrimitiveCollector.getFrameValue(frameIndex);
	int total = totalPrimitivesCollector.getFrameValue(frameIndex);
	float percentUsage =
		float(parcial) /
		float((total <= 0)? 1: total);
	os << percentUsage;
}

void D3D9PrimitiveUsage::printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const {
	int parcial = thisPrimitiveCollector.getBatchValue(frameIndex, batchIndex);
	int total = totalPrimitivesCollector.getBatchValue(frameIndex, batchIndex);
	float percentUsage =
		float(parcial) /
		float((total <= 0)? 1: total);
	os << percentUsage;
}

void D3D9PrimitiveUsage::registerPrimitiveUsage(D3DPRIMITIVETYPE prim) {
	if(prim == primitive)
		thisPrimitiveCount ++;
	totalPrimitivesCount ++;
}

void D3D9PrimitiveUsage::setPrimitive(D3DPRIMITIVETYPE prim) {
	primitive = prim;
}

D3D9VertexShaderInstructionSlots::D3D9VertexShaderInstructionSlots():
settedVertexShader(0),
collector(0),
nextId(1) {
}

D3D9VertexShaderInstructionSlots::D3D9VertexShaderInstructionSlots(const std::string name):
UserStat(name),
settedVertexShader(0),
collector(0),
nextId(1){
}

void D3D9VertexShaderInstructionSlots::endBatch() {

	if(settedVertexShader != 0) {
		unsigned int id = addressesToIdentifiers[settedVertexShader];
		collector.endBatch(instructionCount[id]);
	}
	else
		collector.endBatch(0);
}

void D3D9VertexShaderInstructionSlots::endFrame() {
	collector.endFrame();
}

void D3D9VertexShaderInstructionSlots::endTrace() {
	collector.endTrace();
}

void D3D9VertexShaderInstructionSlots::printTraceValue(std::ostream& os) const{
	os << collector.getTraceValue();
}

void D3D9VertexShaderInstructionSlots::printFrameValue(std::ostream& os, unsigned int frameIndex) const {
	os << collector.getFrameValue(frameIndex);
}

void D3D9VertexShaderInstructionSlots::printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const{
	os << collector.getBatchValue(frameIndex, batchIndex);
}

void D3D9VertexShaderInstructionSlots::registerVertexShaderCreation(IDirect3DVertexShader9 *sh, DWORD *function) {
		addressesToIdentifiers[sh] = nextId;
		identifiersToAddresses[nextId] = sh;

		// Determine instruction count
		D3D9ShaderAnalyzer shAn;
		D3D9ShaderAnalyzer::D3D9ShaderAnalysis an;
		if(shAn.analyze(function, &an))
			instructionCount[nextId] = an.instructionSlots;
		else
			instructionCount[nextId] = 0;

		nextId++;
}

void D3D9VertexShaderInstructionSlots::registerVertexShaderSetted(IDirect3DVertexShader9 *sh) {
	settedVertexShader = sh;
}

D3D9PixelShaderInstructionSlots::D3D9PixelShaderInstructionSlots():
settedPixelShader(0),
collector(0),
nextId(1) {
}

D3D9PixelShaderInstructionSlots::D3D9PixelShaderInstructionSlots(const std::string name):
UserStat(name),
settedPixelShader(0),
collector(0),
nextId(1){
}

void D3D9PixelShaderInstructionSlots::endBatch() {

	if(settedPixelShader != 0) {
		unsigned int id = addressesToIdentifiers[settedPixelShader];
		collector.endBatch(instructionCount[id]);
	}
	else
		collector.endBatch(0);
}

void D3D9PixelShaderInstructionSlots::endFrame() {
	collector.endFrame();
}

void D3D9PixelShaderInstructionSlots::endTrace() {
	collector.endTrace();
}

void D3D9PixelShaderInstructionSlots::printTraceValue(std::ostream& os) const{
	os << collector.getTraceValue();
}

void D3D9PixelShaderInstructionSlots::printFrameValue(std::ostream& os, unsigned int frameIndex) const {
	os << collector.getFrameValue(frameIndex);
}

void D3D9PixelShaderInstructionSlots::printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const{
	os << collector.getBatchValue(frameIndex, batchIndex);
}

void D3D9PixelShaderInstructionSlots::registerPixelShaderCreation(IDirect3DPixelShader9 *sh, DWORD *function) {
		addressesToIdentifiers[sh] = nextId;
		identifiersToAddresses[nextId] = sh;

		// Determine instruction count
		D3D9ShaderAnalyzer shAn;
		D3D9ShaderAnalyzer::D3D9ShaderAnalysis an;
		if(shAn.analyze(function, &an))
			instructionCount[nextId] = an.instructionSlots;
		else
			instructionCount[nextId] = 0;

		nextId++;
}

void D3D9PixelShaderInstructionSlots::registerPixelShaderSetted(IDirect3DPixelShader9 *sh) {
	settedPixelShader = sh;
}

D3D9DifferentShaders::D3D9DifferentShaders():
settedVertexShader(0),
settedPixelShader(0),
nextId(1) {
}

D3D9DifferentShaders::D3D9DifferentShaders(const std::string name):
UserStat(name),
settedVertexShader(0),
settedPixelShader(0),
nextId(1){
}

void D3D9DifferentShaders::endBatch() {
	set<unsigned int> s;

	if(settedVertexShader != 0)
		s.insert(addressesToIdentifiers[settedVertexShader]);
	if(settedPixelShader != 0)
		s.insert( addressesToIdentifiers[settedPixelShader] );

	collector.endBatch(s);
}

void D3D9DifferentShaders::endFrame() {
	collector.endFrame();
}

void D3D9DifferentShaders::endTrace() {
	collector.endTrace();
}

void D3D9DifferentShaders::printTraceValue(std::ostream& os) const{
	set<unsigned int> s = collector.getTraceValue();
	os << static_cast<unsigned long>(s.size());

#ifdef SIMILARITYMATRICES
		
	/** Generate similarity matrix for compare batches from the frame with
	 *  more different shaders.
	 **/

	// Find frame with more different shaders

	unsigned int maxShadersCount = 0;
	unsigned int maxShadersFrameIndex = 0;

	unsigned int currentShadersCount = 0;
	ListDifferentCountAggregator<unsigned int> count;
	unsigned int frameCount = collector.getFrameCount();

	for(unsigned int i = 0; i < frameCount; i ++) {

		currentShadersCount  = collector.getFrameValue(i).size();
		if(currentShadersCount > maxShadersCount) {
			maxShadersCount = currentShadersCount;
			maxShadersFrameIndex = i;
		}
	}

	// Compute similarity matrix for this frame

	if(frameCount == 0) return;

	vector< set< unsigned int > > batchesValues;
	batchesValues = collector.getFrameBatchesValues(maxShadersFrameIndex);

	unsigned int batchesCount = batchesValues.size();

	float *reuseMatrix = new float[batchesCount * batchesCount];
	workloadStats::computeValueSimilarityMatrix(batchesValues, reuseMatrix);
	char filename[256];
	sprintf(filename, "Shader reuse for frame %d.ppm", maxShadersFrameIndex);
	workloadStats::writeSquareMatrixToPPM(filename, reuseMatrix, batchesCount);

	delete[] reuseMatrix;

#endif
}

void D3D9DifferentShaders::printFrameValue(std::ostream& os, unsigned int frameIndex) const {
	set<unsigned int> s = collector.getFrameValue(frameIndex);
	os << static_cast<unsigned long>(s.size());
}

void D3D9DifferentShaders::printBatchValue(std::ostream& os,
	unsigned int frameIndex, unsigned int batchIndex) const{
	set<unsigned int> s = collector.getBatchValue(frameIndex, batchIndex);
	os << static_cast<unsigned long>(s.size());
}

void D3D9DifferentShaders::registerPixelShaderCreation(IDirect3DPixelShader9 *sh) {
		addressesToIdentifiers[sh] = nextId;
		identifiersToAddresses[nextId] = sh;
		nextId++;

}

void D3D9DifferentShaders::registerPixelShaderSetted(IDirect3DPixelShader9 *sh) {
	settedPixelShader = sh;
}

void D3D9DifferentShaders::registerVertexShaderCreation(IDirect3DVertexShader9 *sh) {
		addressesToIdentifiers[sh] = nextId;
		identifiersToAddresses[nextId] = sh;
		nextId++;
}

void D3D9DifferentShaders::registerVertexShaderSetted(IDirect3DVertexShader9 *sh) {
	settedVertexShader = sh;
}

D3D9BatchSize::D3D9BatchSize(): currentBatchVertices(0) {
}

D3D9BatchSize::D3D9BatchSize(const std::string name):currentBatchVertices(0), UserStat(name) {
}

void D3D9BatchSize::endBatch() {
	collector.endBatch(currentBatchVertices);
	currentBatchVertices = 0;
}

void D3D9BatchSize::endFrame() {
	collector.endFrame();
}

void D3D9BatchSize::endTrace() {
	collector.endTrace();
}

void D3D9BatchSize::printTraceValue(std::ostream& os) const {
	os << collector.getTraceValue();
}

void D3D9BatchSize::printFrameValue(std::ostream& os, unsigned int frameIndex) const {
	os << collector.getFrameValue(frameIndex);
}

void D3D9BatchSize::printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const {
	os << collector.getBatchValue(frameIndex, batchIndex);
}

void D3D9BatchSize::registerVertices(int nverts) {
	currentBatchVertices = nverts;
}

D3D9SampleUnits::D3D9SampleUnits() {
}

D3D9SampleUnits::D3D9SampleUnits(const std::string name) : UserStat(name) {
}

void D3D9SampleUnits::endBatch() {
	collector.endBatch(static_cast<int>(activeSampleUnits.size()));
}

void D3D9SampleUnits::endFrame() {
	collector.endFrame();
}

void D3D9SampleUnits::endTrace() {
	collector.endTrace();
}

void D3D9SampleUnits::printTraceValue(std::ostream& os) const {
	os << collector.getTraceValue();
}

void D3D9SampleUnits::printFrameValue(std::ostream& os, unsigned int frameIndex) const {
	os << collector.getFrameValue(frameIndex);
}

void D3D9SampleUnits::printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const {
	os << collector.getBatchValue(frameIndex, batchIndex);
}

void D3D9SampleUnits::registerTextureSetted(DWORD sampler, IDirect3DBaseTexture9 *pTexture) {
	if(pTexture!= 0)
		activeSampleUnits.insert(sampler);
	else
		activeSampleUnits.erase(sampler);
}

D3D9TextureReuse::D3D9TextureReuse(): nextId(1) {
}

D3D9TextureReuse::D3D9TextureReuse(const std::string name)  : UserStat(name), nextId(1) {
}

void D3D9TextureReuse::endBatch() {
	set<unsigned int> texts;
	map<unsigned int, unsigned int>::iterator itS;
	for( itS = activeSampleUnits.begin(); itS != activeSampleUnits.end(); itS ++)
		texts.insert((*itS).second);
	collector.endBatch(texts);
}

void D3D9TextureReuse::endFrame() {
	collector.endFrame();
}

void D3D9TextureReuse::endTrace() {
	collector.endTrace();
}

void D3D9TextureReuse::printTraceValue(std::ostream& os) const{
	set<unsigned int> s = collector.getTraceValue();
	os << static_cast<unsigned long>(s.size());

#ifdef SIMILARITYMATRICES

	/** Generate similarity matrix **/

	unsigned int frameCount = collector.getFrameCount();
	vector< set<unsigned int> > diffTextsPerFrame;
	for(unsigned int i = 0; i < frameCount; i ++) {
		diffTextsPerFrame.push_back(collector.getFrameValue(i));
	}

	float *reuseMatrix = new float[frameCount *frameCount];
	workloadStats::computeValueSimilarityMatrix(diffTextsPerFrame, reuseMatrix);
	workloadStats::writeSquareMatrixToPPM("Texture reuse.ppm", reuseMatrix, frameCount);

	delete[] reuseMatrix;

#endif
}

void D3D9TextureReuse::printFrameValue(std::ostream& os, unsigned int frameIndex) const {
	set<unsigned int> s = collector.getFrameValue(frameIndex);
	os << static_cast<unsigned long>(s.size());
}

void D3D9TextureReuse::printBatchValue(std::ostream& os,
	unsigned int frameIndex, unsigned int batchIndex) const{
	set<unsigned int> s = collector.getBatchValue(frameIndex, batchIndex);
	os << static_cast<unsigned long>(s.size());
}

void D3D9TextureReuse::registerTextureCreation(IDirect3DBaseTexture9 *pTexture) {
		addressesToIdentifiers[pTexture] = nextId;
		identifiersToAddresses[nextId] = pTexture;
		nextId++;
}

void D3D9TextureReuse::registerTextureSetted(DWORD sampler, IDirect3DBaseTexture9 *pTexture) {
	if(pTexture != 0)
		activeSampleUnits[sampler] = addressesToIdentifiers[pTexture];
	else
		activeSampleUnits.erase(sampler);
}

D3D9StateChangesPerFrame::D3D9StateChangesPerFrame(): lastCallOffset(0) {
}

D3D9StateChangesPerFrame::D3D9StateChangesPerFrame(const std::string name) :
UserStat(name), lastCallOffset(0) {
}

void D3D9StateChangesPerFrame::endBatch() {
	// Stats returns us the offset in calls from the beginning of trace
	// of the "batch" call.
	unsigned long callOffset = stats->getCallOffset();
	collector.endBatch(callOffset - lastCallOffset);

	// We put the "batch" call in the offset
	lastCallOffset = callOffset + 1;
}

void D3D9StateChangesPerFrame::endFrame() {
	collector.endFrame();
}

void D3D9StateChangesPerFrame::endTrace() {
	collector.endTrace();
}

void D3D9StateChangesPerFrame::printTraceValue(std::ostream& os) const {
	os << collector.getTraceValue();
}

void D3D9StateChangesPerFrame::printFrameValue(std::ostream& os, unsigned int frameIndex) const {
	os << collector.getFrameValue(frameIndex);
}

void D3D9StateChangesPerFrame::printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const {
	os << collector.getBatchValue(frameIndex, batchIndex);
}

void D3D9StateChangesPerFrame::setD3D9Stadistics(D3D9Stadistics *s) {
	stats = s;
}

D3D9TextureStadistics::D3D9TextureStadistics(): UserStat("Total texture resources; Uncompressed texture resources; Compressed texture resources"), nextId(1) {
}

void D3D9TextureStadistics::setSamplingStadistics(D3D9SamplingStadistics *ss) {
	samplingStats = ss;
}


void D3D9TextureStadistics::endBatch() {
	set<unsigned int> texts;
	map<unsigned int, unsigned int>::iterator itS;
	for( itS = settedSamplers.begin(); itS != settedSamplers.end(); itS ++) {
		if(samplingStats->isSamplerReferenced((*itS).first))
			texts.insert((*itS).second);
	}
	collector.endBatch(texts);
}

void D3D9TextureStadistics::endFrame() {
	collector.endFrame();
}

void D3D9TextureStadistics::endTrace() {
	collector.endTrace();
}

void D3D9TextureStadistics::printTraceValue(std::ostream& os) const{
	set<unsigned int> s = collector.getTraceValue();

	set<unsigned int>::iterator itS;
	unsigned int total = 0;
	unsigned int uncompressed = 0;
	unsigned int compressed = 0;

	for(itS = s.begin(); itS != s.end(); itS ++) {
		if(texturesInfo.find(*itS) != texturesInfo.end()) {
			const TextureInfo &info = (*texturesInfo.find(*itS)).second;
			total+= info.size;
			if(info.isCompressed)
				compressed += info.size;
			else
				uncompressed += info.size;
		}
	}

	os << total << ";" << uncompressed << ";" << compressed;

}

void D3D9TextureStadistics::printFrameValue(std::ostream& os, unsigned int frameIndex) const {
	set<unsigned int> s = collector.getFrameValue(frameIndex);

	set<unsigned int>::iterator itS;
	unsigned int total = 0;
	unsigned int uncompressed = 0;
	unsigned int compressed = 0;
	for(itS = s.begin(); itS != s.end(); itS ++) {
		if(texturesInfo.find(*itS) != texturesInfo.end()) {
			const TextureInfo &info = (*texturesInfo.find(*itS)).second;
			total+= info.size;
			if(info.isCompressed)
				compressed += info.size;
			else
				uncompressed += info.size;
		}
	}

	os << total << ";" << uncompressed << ";" << compressed;
}

void D3D9TextureStadistics::printBatchValue(std::ostream& os,
	unsigned int frameIndex, unsigned int batchIndex) const{

	set<unsigned int> s = collector.getBatchValue(frameIndex, batchIndex);

	set<unsigned int>::iterator itS;
	unsigned int total = 0;
	unsigned int uncompressed = 0;
	unsigned int compressed = 0;
	for(itS = s.begin(); itS != s.end(); itS ++) {
		if(texturesInfo.find(*itS) != texturesInfo.end()) {
			const TextureInfo &info = (*texturesInfo.find(*itS)).second;
			total+= info.size;
			if(info.isCompressed)
				compressed += info.size;
			else
				uncompressed += info.size;
		}
	}

	os << total << ";" << uncompressed << ";" << compressed;
}

void D3D9TextureStadistics::registerCreateTexture(
	UINT Width,	UINT Height, UINT Levels, DWORD Usage, 
	D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9* pTexture,
	HANDLE* pSharedHandle) {
		addressesToIdentifiers[pTexture] = nextId;
		identifiersToAddresses[nextId] = pTexture;
		TextureInfo info;
		info.size = D3D9Info::instance().getTextureSize(Width, Height, Levels, Format);
		info.isCompressed = D3D9Info::instance().isCompressed(Format);
		texturesInfo[nextId] = info;
		nextId++;
	}

void D3D9TextureStadistics::registerCreateVolumeTexture(
	UINT Width,	UINT Height, UINT Depth, UINT Levels,
	DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
	IDirect3DVolumeTexture9* pVolumeTexture,
	HANDLE* pSharedHandle) {
		addressesToIdentifiers[pVolumeTexture] = nextId;
		identifiersToAddresses[nextId] = pVolumeTexture;
		TextureInfo info;
		info.size = D3D9Info::instance().getVolumeTextureSize(Width, Height, Depth, Levels, Format);
		info.isCompressed = D3D9Info::instance().isCompressed(Format);
		texturesInfo[nextId] = info;
		nextId++;
	}

void D3D9TextureStadistics::registerCreateCubeTexture(
	UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format,
	D3DPOOL Pool, IDirect3DCubeTexture9 * pCubeTexture,
	HANDLE* pSharedHandle) {
		addressesToIdentifiers[pCubeTexture] = nextId;
		identifiersToAddresses[nextId] = pCubeTexture;
		TextureInfo info;
		info.size = D3D9Info::instance().getCubeTextureSize(EdgeLength, Levels, Format);
		info.isCompressed = D3D9Info::instance().isCompressed(Format);
		texturesInfo[nextId] = info;
		nextId++;
	}

void D3D9TextureStadistics::registerSetTexture(
	DWORD Sampler, IDirect3DBaseTexture9 * pTexture) {
		if(pTexture == 0)
			settedSamplers.erase(Sampler);
		else
			settedSamplers[Sampler] = addressesToIdentifiers[pTexture];
	}

D3D9TextureStadistics::TextureInfo::TextureInfo():
isCompressed(false), size(0) {
}


D3D9SamplingStadistics::D3D9SamplingStadistics():
UserStat("NEAREST %; NEAREST_MIPMAP_LINEAR %; BILINEAR %; BILINEAR_ANISO %; TRILINEAR %; TRILINEAR_ANISO %; Active samplers; Texture instructions"),
nearestCollector(-1.0f),
nearestMipMapCollector(-1.0f),
bilinearCollector(-1.0f),
bilinearAnisoCollector(-1.0f),
trilinearCollector(-1.0f),
trilinearAnisoCollector(-1.0f),
activeSamplersCollector(-1.0),
textureInstructionsCollector(-1)
{}

bool D3D9SamplingStadistics::isSamplerReferenced(DWORD Sampler) {

	// Is referenced by vertex shader?
	if(vsh != 0) {
		unsigned int vshId = addressesToIdentifiers[vsh];
		D3D9ShaderAnalyzer::D3D9ShaderAnalysis vshAnalysis = shaderAnalysis[vshId];
		if(vshAnalysis.samplerUsage.find(Sampler) != vshAnalysis.samplerUsage.end())
			return true;
	}

	// Is referenced by pixel shader?
	if(psh != 0) {
		unsigned int pshId = addressesToIdentifiers[psh];
		D3D9ShaderAnalyzer::D3D9ShaderAnalysis pshAnalysis = shaderAnalysis[pshId];
		if(pshAnalysis.samplerUsage.find(Sampler) != pshAnalysis.samplerUsage.end())
			return true;
	}
	// Assume fixed function is using all setted samplers
	else {
		map<DWORD, SamplerStatus>::iterator itS;
		itS = samplers.find(Sampler);
		if(itS != samplers.end())
			if((*itS).second.texture != 0)
				return true;
	}

	return false;
}

#ifdef SAMPLERSTATUSLOG
	string filter2str(D3DTEXTUREFILTERTYPE t) {
		switch(t) {
			case D3DTEXF_NONE:
				return "NONE";
			case D3DTEXF_POINT:
				return "POINT";
			case D3DTEXF_LINEAR:
				return "LINEAR";
			case D3DTEXF_ANISOTROPIC:
				return "ANISOTROPIC";
			case D3DTEXF_PYRAMIDALQUAD:
				return "PYRAMIDALQUAD";
			case D3DTEXF_GAUSSIANQUAD:
				return "GAUSSIANQUAD";
			default:
				return "OPAVIASEUNCORRA";
		}

	}

	ofstream sampling_log("samplerStatus.txt");
	unsigned int frame = 1;
	unsigned int batch = 1;
#endif

void D3D9SamplingStadistics::endBatch() {

	map< UINT, UINT >::iterator itS;
	D3D9ShaderAnalyzer::D3D9ShaderAnalysis totalAnalysis;

	bool foundSamplerUsage = false;

#ifdef SAMPLERSTATUSLOG
	bool isFixedFunction = false;
#endif

	if(vsh != 0) {
		unsigned int vshId = addressesToIdentifiers[vsh];
		D3D9ShaderAnalyzer::D3D9ShaderAnalysis vshAnalysis = shaderAnalysis[vshId];
		totalAnalysis.samples += vshAnalysis.samples;
		for(itS = vshAnalysis.samplerUsage.begin(); itS != vshAnalysis.samplerUsage.end(); itS ++) {
			foundSamplerUsage = true;
			if(totalAnalysis.samplerUsage.find((*itS).first) == totalAnalysis.samplerUsage.end())
				totalAnalysis.samplerUsage[(*itS).first] = 0;
			totalAnalysis.samplerUsage[(*itS).first ] += (*itS).second;
		}
	}

	if(psh != 0) {
		unsigned int pshId = addressesToIdentifiers[psh];
		D3D9ShaderAnalyzer::D3D9ShaderAnalysis pshAnalysis = shaderAnalysis[pshId];
		totalAnalysis.samples += pshAnalysis.samples;
		for(itS = pshAnalysis.samplerUsage.begin(); itS != pshAnalysis.samplerUsage.end(); itS ++) {
			foundSamplerUsage = true;
			if(totalAnalysis.samplerUsage.find((*itS).first) == totalAnalysis.samplerUsage.end())
				totalAnalysis.samplerUsage[(*itS).first] = 0;
			totalAnalysis.samplerUsage[(*itS).first ] += (*itS).second;
		}
	}
	else {

#ifdef SAMPLERSTATUSLOG
		isFixedFunction = true;
#endif
		// Assume fixed function is using all samplers bounded to a texture
		map< DWORD, SamplerStatus> ::iterator itFS;
		for(itFS = samplers.begin(); itFS != samplers.end(); itFS ++) {
			if((*itFS).second.texture != 0) {
				foundSamplerUsage = true;
				if(totalAnalysis.samplerUsage.find((*itFS).first) == totalAnalysis.samplerUsage.end())
					totalAnalysis.samplerUsage[(*itFS).first] = 0;
				totalAnalysis.samplerUsage[(*itFS).first ] ++;
				totalAnalysis.samples ++;
			}
		}
	}

	if(totalAnalysis.samples != 0) {

		float nearest = 0.0f;
		float nearestMipMapLinear = 0.0f;
		float bilinear = 0.0f;
		float bilinearAniso = 0.0f;
		float trilinear = 0.0f;
		float trilinearAniso = 0.0f;

		for(itS = totalAnalysis.samplerUsage.begin(); itS != totalAnalysis.samplerUsage.end(); itS ++) {
			UINT sampler = (*itS).first;
			UINT samplerCount = (*itS).second;

			switch(samplers[sampler].getCategory()) {
				case NEAREST:
					nearest += samplerCount;
					break;
				case NEAREST_MIPMAP_LINEAR:
					nearestMipMapLinear += samplerCount;
					break;
				case BILINEAR:
					bilinear += samplerCount;
					break;
				case BILINEAR_ANISO:
					bilinearAniso += samplerCount;
					break;
				case TRILINEAR:
					trilinear += samplerCount;
					break;
				case TRILINEAR_ANISO:
					trilinearAniso += samplerCount;
					break;
				case UNKNOWN:
					nearest += samplerCount;
			}
		}
		
		nearest /= totalAnalysis.samples;
		nearestMipMapLinear /= totalAnalysis.samples;
		bilinear /= totalAnalysis.samples;
		bilinearAniso /= totalAnalysis.samples;
		trilinear /= totalAnalysis.samples;
		trilinearAniso /= totalAnalysis.samples;

		nearestCollector.endBatch(nearest);
		nearestMipMapCollector.endBatch(nearestMipMapLinear );
		bilinearCollector.endBatch(bilinear);
		bilinearAnisoCollector.endBatch(bilinearAniso);
		trilinearCollector.endBatch(trilinear);
		trilinearAnisoCollector.endBatch(trilinearAniso);
	}
	else {
		nearestCollector.endBatch(-1.0f);
		nearestMipMapCollector.endBatch(-1.0f);
		bilinearCollector.endBatch(-1.0f);
		bilinearAnisoCollector.endBatch(-1.0f);
		trilinearCollector.endBatch(-1.0f);
		trilinearAnisoCollector.endBatch(-1.0f);
	}

	if (foundSamplerUsage) {
		activeSamplersCollector.endBatch(totalAnalysis.samplerUsage.size());
		textureInstructionsCollector.endBatch(totalAnalysis.samples);
	}
	else  {
		activeSamplersCollector.endBatch(-1);
		textureInstructionsCollector.endBatch(-1);
	}


#ifdef SAMPLERSTATUSLOG
	sampling_log << frame << " : " << batch << endl;
	sampling_log << "------------------------------------" << endl;

	sampling_log << totalAnalysis.samples << " samples" << endl;
	if(isFixedFunction) {
		sampling_log << "Fixed function " << endl;
	}
	else {
		sampling_log << "VS<" << this->vsh << "> PS<" << this->psh << ">" << endl;
	}
	
	sampling_log << endl;

	map< DWORD, SamplerStatus >::iterator itSS;
	for(itSS = samplers.begin(); itSS != samplers.end(); itSS ++) {
		SamplerStatus &ss = (*itSS).second;
		sampling_log << "Sampler " << (*itSS).first << endl;
		bool used = (totalAnalysis.samplerUsage.find((*itSS).first) != totalAnalysis.samplerUsage.end());
		if (used)
			sampling_log << "Used " << totalAnalysis.samplerUsage[(*itSS).first] << " times" << endl;
		else
			sampling_log << "Unused" << endl;
		sampling_log << "----------" << endl;
		sampling_log << "Texture     : " << ss.texture << endl;
		sampling_log << "Min|Mag|Mip : " << filter2str(ss.minFilter) << " | " << filter2str(ss.magFilter) << " | " << filter2str(ss.mipFilter) << endl;
		sampling_log << "MaxAniso    : " << ss.maxAnisotropy << endl;
		switch(ss.getCategory()) {
			case NEAREST:
				sampling_log << "OGL equiv   : NEAREST" << endl;
				break;
			case NEAREST_MIPMAP_LINEAR:
				sampling_log << "OGL equiv   : NEAREST_MIPMAP_LINEAR" << endl;
				break;
			case BILINEAR:
				sampling_log << "OGL equiv   : BILINEAR" << endl;
				break;
			case BILINEAR_ANISO:
				sampling_log << "OGL equiv   : BILINEAR_ANISO" << endl;
				break;
			case TRILINEAR:
				sampling_log << "OGL equiv   : TRILINEAR" << endl;
				break;
			case TRILINEAR_ANISO:
				sampling_log << "OGL equiv   : TRILINEAR_ANISO" << endl;
				break;
			case UNKNOWN:
				sampling_log << "OGL equiv   : UNKNOWN" << endl;
				break;

		}

		sampling_log << endl;
	}

	sampling_log << "------------------------------------" << endl;
	batch ++;
#endif

}

void D3D9SamplingStadistics::endFrame() {
	nearestCollector.endFrame();
	nearestMipMapCollector.endFrame();
	bilinearCollector.endFrame();
	bilinearAnisoCollector.endFrame();
	trilinearCollector.endFrame();
	trilinearAnisoCollector.endFrame();
	activeSamplersCollector.endFrame();
	textureInstructionsCollector.endFrame();

#ifdef SAMPLERSTATUSLOG
	frame ++;
	batch = 1;
#endif
}

void D3D9SamplingStadistics::endTrace() {
	nearestCollector.endTrace();
	nearestMipMapCollector.endTrace();
	bilinearCollector.endTrace();
	bilinearAnisoCollector.endTrace();
	trilinearCollector.endTrace();
	trilinearAnisoCollector.endTrace();
	activeSamplersCollector.endTrace();
	textureInstructionsCollector.endTrace();
}

void D3D9SamplingStadistics::printTraceValue(std::ostream& os) const {
	os << nearestCollector.getTraceValue() << "; ";
	os << nearestMipMapCollector.getTraceValue() << "; ";
	os << bilinearCollector.getTraceValue() << "; ";
	os << bilinearAnisoCollector.getTraceValue() << "; ";
	os << trilinearCollector.getTraceValue() << "; ";
	os << trilinearAnisoCollector.getTraceValue() << "; ";
	os << activeSamplersCollector.getTraceValue() << "; ";
	os << textureInstructionsCollector.getTraceValue();
}

void D3D9SamplingStadistics::printFrameValue(std::ostream& os, unsigned int frameIndex) const {
	os << nearestCollector.getFrameValue(frameIndex) << "; ";
	os << nearestMipMapCollector.getFrameValue(frameIndex) << "; ";
	os << bilinearCollector.getFrameValue(frameIndex) << "; ";
	os << bilinearAnisoCollector.getFrameValue(frameIndex) << "; ";
	os << trilinearCollector.getFrameValue(frameIndex) << "; ";
	os << trilinearAnisoCollector.getFrameValue(frameIndex) << "; ";
	os << activeSamplersCollector.getFrameValue(frameIndex) << "; ";
	os << textureInstructionsCollector.getFrameValue(frameIndex);

}

void D3D9SamplingStadistics::printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const {
	os << nearestCollector.getBatchValue(frameIndex, batchIndex) << "; ";
	os << nearestMipMapCollector.getBatchValue(frameIndex, batchIndex) << "; ";
	os << bilinearCollector.getBatchValue(frameIndex, batchIndex) << "; ";
	os << bilinearAnisoCollector.getBatchValue(frameIndex, batchIndex) << "; ";
	os << trilinearCollector.getBatchValue(frameIndex, batchIndex) << "; ";
	os << trilinearAnisoCollector.getBatchValue(frameIndex, batchIndex) << "; ";
	os << activeSamplersCollector.getBatchValue(frameIndex, batchIndex) << "; ";
	os << textureInstructionsCollector.getBatchValue(frameIndex, batchIndex);
}

void D3D9SamplingStadistics::registerTextureSetted(DWORD sampler, IDirect3DBaseTexture9 *pTexture) {
	samplers[sampler].texture = pTexture;
}

void D3D9SamplingStadistics::registerVertexShaderCreation(IDirect3DVertexShader9 *sh, DWORD *function) {
		addressesToIdentifiers[sh] = nextId;
		identifiersToAddresses[nextId] = sh;

		// Analyze
		D3D9ShaderAnalyzer shAn;
		D3D9ShaderAnalyzer::D3D9ShaderAnalysis an;
		if(shAn.analyze(function, &an))
			shaderAnalysis[nextId] = an;
		else
			shaderAnalysis[nextId];

		nextId++;
}

void D3D9SamplingStadistics::registerPixelShaderCreation(IDirect3DPixelShader9 *sh,  DWORD *function) {
		addressesToIdentifiers[sh] = nextId;
		identifiersToAddresses[nextId] = sh;

		// Analyze
		D3D9ShaderAnalyzer shAn;
		D3D9ShaderAnalyzer::D3D9ShaderAnalysis an;
		if(shAn.analyze(function, &an))
			shaderAnalysis[nextId] = an;
		else
			shaderAnalysis[nextId];

		nextId++;
}

void D3D9SamplingStadistics::registerVertexShaderSetted(IDirect3DVertexShader9 *sh) {
	vsh = sh;
}

void D3D9SamplingStadistics::registerPixelShaderSetted(IDirect3DPixelShader9 *sh) {
	psh = sh;
}

void D3D9SamplingStadistics::registerSetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) {
	switch (Type) {
		case D3DSAMP_MAGFILTER:
			samplers[Sampler].magFilter = (D3DTEXTUREFILTERTYPE)Value;
			break;
		case D3DSAMP_MINFILTER:
			samplers[Sampler].minFilter = (D3DTEXTUREFILTERTYPE)Value;
			break;
		case D3DSAMP_MIPFILTER:
			samplers[Sampler].mipFilter = (D3DTEXTUREFILTERTYPE)Value;
			break;
		case D3DSAMP_MAXANISOTROPY:
			samplers[Sampler].maxAnisotropy = Value;
			break;
	}
}


D3D9StateBlockHandler::D3D9StateBlockHandler():
insideBeginEnd(false), nextId(1) {}

void D3D9StateBlockHandler:: setD3D9Stadistics(D3D9Stadistics *st) {
	stats = st;
}

void D3D9StateBlockHandler:: unsetCurrentStateBlock() {
	current.pixelShaderSetted = false;
	current.vertexShaderSetted = false;
	
	map< DWORD, SamplerStatusRecord >::iterator itSM;
	list< DWORD > samplersList;
	for( itSM = current.samplerStatus.begin(); itSM != current.samplerStatus.end(); itSM ++) {
		samplersList.push_back((*itSM).first);
	}

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

void D3D9StateBlockHandler:: beginStateBlock() {
	insideBeginEnd = true;
	unsetCurrentStateBlock();
}

void D3D9StateBlockHandler:: endStateBlock(IDirect3DStateBlock9 *sb) {
	addressesToIdentifiers[sb] = nextId;
	identifiersToAddresses[nextId] = sb;
	stateBlockRecords[nextId] = current;
	nextId ++;

	insideBeginEnd = false;
	unsetCurrentStateBlock();
}

#ifdef STATEBLOCKLOG
ofstream stateblock_log("stateblockstatus.txt");
#endif

void D3D9StateBlockHandler:: applyStateBlock(IDirect3DStateBlock9 *sb) {

#ifdef STATEBLOCKLOG
	stateblock_log << frame << " : " << batch << endl;
	stateblock_log << "----------------" << endl;
	stateblock_log << "Applied <" << sb << ">" << endl;
#endif

	int idSb = addressesToIdentifiers[sb];
	StateBlockRecord record = stateBlockRecords[idSb];
	if(record.pixelShaderSetted) {
		stats->registerPixelShaderSetted(record.pixelShader);
#ifdef STATEBLOCKLOG
		stateblock_log << "Pixel shader <" << record.pixelShader << ">" << endl;
#endif
	}
	if(record.vertexShaderSetted) {
		stats->registerVertexShaderSetted(record.vertexShader);
#ifdef STATEBLOCKLOG
		stateblock_log << "Vertex shader <" << record.pixelShader << ">" << endl;
#endif
	}

	map< DWORD, SamplerStatusRecord >::iterator itS; 
	for(itS = record.samplerStatus.begin(); itS != record.samplerStatus.end(); itS ++) {
		DWORD sampler = (*itS).first;
		SamplerStatusRecord status = (*itS).second;

#ifdef STATEBLOCKLOG
		stateblock_log << "Sampler " << sampler << ":" << endl;
#endif

		if (status.magFilterSetted) {
			stats->registerSetSamplerState(sampler, D3DSAMP_MAGFILTER, status.magFilter);
#ifdef STATEBLOCKLOG
			stateblock_log << "Mag      : " << filter2str(status.magFilter) << endl;
#endif
		}
		if (status.minFilterSetted) {
			stats->registerSetSamplerState(sampler, D3DSAMP_MINFILTER, status.minFilter);
#ifdef STATEBLOCKLOG
			stateblock_log << "Min      : " << filter2str(status.minFilter) << endl;
#endif
		}
		if (status.mipFilterSetted) {
			stats->registerSetSamplerState(sampler, D3DSAMP_MIPFILTER, status.mipFilter);
#ifdef STATEBLOCKLOG
			stateblock_log << "Mip      : " << filter2str(status.mipFilter) << endl;
#endif
		}
		if (status.maxAnisotropySetted) {
			stats->registerSetSamplerState(sampler, D3DSAMP_MAXANISOTROPY, status.maxAnisotropy);
#ifdef STATEBLOCKLOG
			stateblock_log << "MaxAniso : " << status.maxAnisotropy << endl;
#endif
		}
		if (status.textureSetted) {
			stats->registerTextureSetted(sampler, status.texture);
#ifdef STATEBLOCKLOG
			stateblock_log << "Texture : " << status.texture << endl;
#endif
		}

#ifdef STATEBLOCKLOG
		stateblock_log << endl;
#endif
	}

#ifdef STATEBLOCKLOG
	stateblock_log << endl;
#endif
}

void D3D9StateBlockHandler:: captureStateBlock(IDirect3DStateBlock9 *sb) {

	int idSb = addressesToIdentifiers[sb];
	StateBlockRecord &stateBlock = stateBlockRecords[idSb];

	if(stateBlock.pixelShaderSetted)
		stateBlock.pixelShader = current.pixelShader;
	if(stateBlock.vertexShaderSetted)
		stateBlock.vertexShader = current.vertexShader;

	map< DWORD, SamplerStatusRecord > ::iterator itS;
	for(itS = current.samplerStatus.begin(); itS != current.samplerStatus.end(); itS ++) {
		DWORD currentS = (*itS).first;
		SamplerStatusRecord currentSSR = (*itS).second;
		
		SamplerStatusRecord &samplerStatus =  stateBlock.samplerStatus[currentS];

		if(samplerStatus.magFilterSetted)
			samplerStatus.magFilter = currentSSR.magFilter;
		if(samplerStatus.minFilterSetted)
			samplerStatus.minFilter = currentSSR.minFilter;
		if(samplerStatus.mipFilterSetted)
			samplerStatus.mipFilter = currentSSR.mipFilter;
		if(samplerStatus.maxAnisotropySetted)
			samplerStatus.maxAnisotropy = currentSSR.maxAnisotropy;
		if(samplerStatus.textureSetted)
			samplerStatus.texture = currentSSR.texture;
	}
}

void D3D9StateBlockHandler:: setSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) {

	switch (Type) {
		case D3DSAMP_MAGFILTER:
			if (insideBeginEnd)
				current.samplerStatus[Sampler].magFilterSetted = true;
			current.samplerStatus[Sampler].magFilter = (D3DTEXTUREFILTERTYPE)Value;
			break;
		case D3DSAMP_MINFILTER:
			if (insideBeginEnd)
				current.samplerStatus[Sampler].minFilterSetted = true;
			current.samplerStatus[Sampler].minFilter = (D3DTEXTUREFILTERTYPE)Value;
			break;
		case D3DSAMP_MIPFILTER:
			if (insideBeginEnd)
				current.samplerStatus[Sampler].mipFilterSetted = true;
			current.samplerStatus[Sampler].mipFilter = (D3DTEXTUREFILTERTYPE)Value;
			break;
		case D3DSAMP_MAXANISOTROPY:
			if (insideBeginEnd)
				current.samplerStatus[Sampler].maxAnisotropySetted = true;
			current.samplerStatus[Sampler].maxAnisotropy = Value;
	}

}

void D3D9StateBlockHandler:: setTexture( DWORD Sampler, IDirect3DBaseTexture9 * pTexture) {
	if (insideBeginEnd)
		current.samplerStatus[Sampler].textureSetted = true;
	current.samplerStatus[Sampler].texture = pTexture;
}

void D3D9StateBlockHandler:: setVertexShader( IDirect3DVertexShader9* pShader) {
	if (insideBeginEnd)
		current.vertexShaderSetted = true;
	current.vertexShader = pShader;
}

void D3D9StateBlockHandler:: setPixelShader( IDirect3DPixelShader9* pShader) {
	if (insideBeginEnd)
		current.pixelShaderSetted = true;
	current.pixelShader = pShader;
}

D3D9Queries::D3D9Queries(): UserStat("Rendered Triangles; Extra Triangles"),
vertexStatsQuery(0), mainDevice(0) {
}

void D3D9Queries::setMainSubstituteDevice(IDirect3DDevice9 *dev) {
	// Release previous query if there is one created and
	// mainDevice is a new one.
	if((vertexStatsQuery != 0) & (dev != mainDevice))
		vertexStatsQuery->Release();

	mainDevice = dev;
	
	// Check query is supported
	if(S_OK != mainDevice->CreateQuery(D3DQUERYTYPE_VERTEXSTATS, 0)) {
		vertexStatsQuery = 0;
		return;
	}

	mainDevice->CreateQuery(D3DQUERYTYPE_VERTEXSTATS, &vertexStatsQuery);
}

void D3D9Queries::endBatch() {
	renderedCollector.endBatch(0);
	extraCollector.endBatch(0);	
}

void D3D9Queries::endFrame() {
	if(vertexStatsQuery != 0) {
		// Issue
		vertexStatsQuery->Issue(D3DISSUE_END);

		// Wait for query data to be ready
		// This kind of loop is explicitly discouraged
		// in MSDN docs so "don't do this at home kids!"
		while(vertexStatsQuery->GetData(0, 0, 0) != S_OK);;

		D3DDEVINFO_D3DVERTEXSTATS data;
		vertexStatsQuery->GetData((void *)&data, sizeof(data), 0);

		renderedCollector.endBatch(data.NumRenderedTriangles);
		extraCollector.endBatch(data.NumExtraClippingTriangles);	
	}
	else {
		renderedCollector.endBatch(0);
		extraCollector.endBatch(0);	
	}

	renderedCollector.endFrame();
	extraCollector.endFrame();
}

void D3D9Queries::endTrace() {
	renderedCollector.endTrace();
	extraCollector.endTrace();
}

void D3D9Queries::printTraceValue(std::ostream& os) const {
	os << renderedCollector.getTraceValue() << "; " << extraCollector.getTraceValue();
}

void D3D9Queries::printFrameValue(std::ostream& os, unsigned int frameIndex) const {
	os << renderedCollector.getFrameValue(frameIndex) <<
		"; " << extraCollector.getFrameValue(frameIndex);
}

void D3D9Queries::printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const {
	os << renderedCollector.getBatchValue(frameIndex, batchIndex) <<
		"; " << extraCollector.getBatchValue(frameIndex, batchIndex);
}

D3D9Stadistics &D3D9Stadistics::instance() {
	static D3D9Stadistics inst;
	return inst;
}

D3D9Stadistics::D3D9Stadistics():
	player(0),
	bpf("Batches per frame"),
	vpf("Vertices per frame"),
	ppf("Primitives per frame"),
	dsh("Total different shaders per frame"),
	bs("Average batch size"),
	su("Average active sample units per frame"),
	tr("Texture reuse"),
	sc("State changes"),
	vshis("Vertex Shader instructions slots"),
	pshis("Pixel Shader instructions slots") {

	sbh.setD3D9Stadistics(this);
	ts.setSamplingStadistics(&ss);

	StatsManager &sm = StatsManager::instance();
	sm.setPerBatchStats(true);
	sm.setPerFrameStats(true);
	sm.setPerTraceStats(true);

	pu[0].setName("D3DPT_POINTLIST %");
	pu[0].setPrimitive(D3DPT_POINTLIST);
	pu[1].setName("D3DPT_LINELIST %");
	pu[1].setPrimitive(D3DPT_LINELIST);
	pu[2].setName("D3DPT_LINESTRIP %");
	pu[2].setPrimitive(D3DPT_LINESTRIP);
	pu[3].setName("D3DPT_TRIANGLELIST %");
	pu[3].setPrimitive(D3DPT_TRIANGLELIST);
	pu[4].setName("D3DPT_TRIANGLESTRIP %");
	pu[4].setPrimitive(D3DPT_TRIANGLESTRIP);
	pu[5].setName("D3DPT_TRIANGLEFAN %");
	pu[5].setPrimitive(D3DPT_TRIANGLEFAN);

	sc.setD3D9Stadistics(this);

	for(int i = 0;i < 6; i++)
		sm.addUserStat(&pu[i]);

	sm.addUserStat(&bpf);
	sm.addUserStat(&vpf);
	sm.addUserStat(&ppf);
	sm.addUserStat(&dsh);
	sm.addUserStat(&bs);
	sm.addUserStat(&su);
	sm.addUserStat(&tr);
	sm.addUserStat(&sc);
	sm.addUserStat(&vshis);
	sm.addUserStat(&pshis);
	sm.addUserStat(&ss);
	sm.addUserStat(&ts);
	sm.addUserStat(&que);
}

void D3D9Stadistics::setPlayer(D3DPlayer *e) {
	player = e;
}

void D3D9Stadistics::setMainSubstituteDevice(IDirect3DDevice9 *dev) {
	que.setMainSubstituteDevice(dev);
}


void D3D9Stadistics::beginBatch() {
	StatsManager &sm = StatsManager::instance();
	sm.beginBatch();
}

void D3D9Stadistics::endBatch() {
	StatsManager &sm = StatsManager::instance();
	sm.endBatch();
}

void D3D9Stadistics::endFrame() {
	StatsManager &sm = StatsManager::instance();
	sm.endFrame();
}

void D3D9Stadistics::endTrace() {
	StatsManager &sm = StatsManager::instance();
	sm.endTrace();
}


void D3D9Stadistics::registerDrawPrimitive(D3DPRIMITIVETYPE type, int primitiveCount) {

	int verticesCount = 0;

	switch(type) {
		case D3DPT_POINTLIST:
			verticesCount = primitiveCount;
			break;			
		case D3DPT_LINELIST:
			verticesCount = primitiveCount * 2;
			break;			
		case D3DPT_LINESTRIP:
			verticesCount = primitiveCount + 1;
			break;
		case D3DPT_TRIANGLELIST:
			verticesCount = primitiveCount * 3;
			break;
		case D3DPT_TRIANGLESTRIP:
			verticesCount = primitiveCount + 2;
			break;
		case D3DPT_TRIANGLEFAN:
			verticesCount = primitiveCount + 2;
			break;
	}
	
	vpf.registerVertices(verticesCount);
	bs.registerVertices(verticesCount);
	ppf.registerPrimitives(primitiveCount);

	for(int i = 0; i < 6; i ++) {
		pu[i].registerPrimitiveUsage(type);
	}
}

void D3D9Stadistics::registerVertexShaderCreation(IDirect3DVertexShader9 *sh, DWORD *function) {
	ss.registerVertexShaderCreation(sh, function);
	vshis.registerVertexShaderCreation(sh, function);
	dsh.registerVertexShaderCreation(sh);
	
}

void D3D9Stadistics::registerPixelShaderCreation(IDirect3DPixelShader9 *sh, DWORD *function) {
	ss.registerPixelShaderCreation(sh, function);
	pshis.registerPixelShaderCreation(sh, function);
	dsh.registerPixelShaderCreation(sh);
}

void D3D9Stadistics::registerVertexShaderSetted(IDirect3DVertexShader9 *sh) {
	ss.registerVertexShaderSetted(sh);
	vshis.registerVertexShaderSetted(sh);
	dsh.registerVertexShaderSetted(sh);
	sbh.setVertexShader(sh);
}

void D3D9Stadistics::registerPixelShaderSetted(IDirect3DPixelShader9 *sh) {
	ss.registerPixelShaderSetted(sh);
	pshis.registerPixelShaderSetted(sh);
	dsh.registerPixelShaderSetted(sh);
	sbh.setPixelShader(sh);
}

void D3D9Stadistics::registerTextureSetted(DWORD sampler, IDirect3DBaseTexture9 *tex) {
	ss.registerTextureSetted(sampler, tex);
	su.registerTextureSetted(sampler, tex);
	tr.registerTextureSetted(sampler, tex);
	sbh.setTexture(sampler, tex);
	ts.registerSetTexture(sampler, tex);
}

void D3D9Stadistics::registerCreateTexture(
	UINT Width,	UINT Height, UINT Levels, DWORD Usage, 
	D3DFORMAT Format, D3DPOOL Pool,  IDirect3DTexture9* pTexture,
	HANDLE* pSharedHandle) {

	if(pTexture != 0) {
		tr.registerTextureCreation(pTexture);
		ts.registerCreateTexture(Width, Height, Levels, Usage,
			Format, Pool, pTexture, pSharedHandle);
	}
}

void D3D9Stadistics::registerCreateVolumeTexture(
	UINT Width,	UINT Height, UINT Depth,
	UINT Levels, DWORD Usage, D3DFORMAT Format,
	D3DPOOL Pool, IDirect3DVolumeTexture9* pVolumeTexture,
	HANDLE* pSharedHandle) {

	if(pVolumeTexture != 0) {
		tr.registerTextureCreation(pVolumeTexture);
		ts.registerCreateVolumeTexture(Width, Height, Depth, Levels,
			Usage, Format, Pool, pVolumeTexture, pSharedHandle);

	}
}

void D3D9Stadistics::registerCreateCubeTexture(
	UINT EdgeLength, UINT Levels, DWORD Usage,
	D3DFORMAT Format, D3DPOOL Pool,	IDirect3DCubeTexture9 * pCubeTexture,
	HANDLE* pSharedHandle) {

	if(pCubeTexture != 0) {
		tr.registerTextureCreation(pCubeTexture);
		ts.registerCreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool,
			pCubeTexture, pSharedHandle);
	}
}


void D3D9Stadistics::registerSetSamplerState( DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) {
	ss.registerSetSamplerState(Sampler, Type, Value);
	sbh.setSamplerState(Sampler, Type, Value);
}

void D3D9Stadistics::registerBeginStateBlock() {
	sbh.beginStateBlock();
}

void D3D9Stadistics::registerEndStateBlock(IDirect3DStateBlock9 *sb) {
	sbh.endStateBlock(sb);
}

void D3D9Stadistics::registerApplyStateBlock(IDirect3DStateBlock9 *sb) {
	sbh.applyStateBlock(sb);
}

void D3D9Stadistics::registerCaptureStateBlock(IDirect3DStateBlock9 *sb) {
	sbh.captureStateBlock(sb);
}

unsigned long D3D9Stadistics::getCallOffset() {
	return player->getCallOffset();
}



