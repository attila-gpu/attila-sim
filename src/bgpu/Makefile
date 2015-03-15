###########################################################################
# 
#  Copyright (c) 2002, 2003 by Computer Architecture Department,
#  Universitat Politecnica de Catalunya.
#  All rights reserved.
# 
#  The contents of this file may not be disclosed to third parties,
#  copied or duplicated in any form, in whole or in part, without the
#  prior permission of the authors, Computer Architecture Department
#  and Universitat Politecnica de Catalunya.
# 
#  $RCSfile: Makefile,v $
#  $Revision: 1.55 $
#  $Author: cgonzale $
#  $Date: 2008-02-29 15:56:39 $
# 
#  Shader Simulator Makefile. 
# 
# 


include ../Makefile.defs

INCLUDE = -I . -I $(SUPPORTDIR) -I $(EMUDIR) -I $(SIMDIR) \
          -I $(SIMDIR)/CommandProcessor -I $(SIMDIR)/MemoryController \
          -I $(SIMDIR)/MemoryControllerV2 -I $(SIMDIR)/Shader -I $(SIMDIR)/Streamer \
          -I $(SIMDIR)/PrimitiveAssembly -I $(SIMDIR)/Clipper \
	  -I $(SIMDIR)/Rasterizer -I $(SIMDIR)/MicroPolygonRasterizer \
	  -I $(SIMDIR)/FragmentOperations -I $(SIMDIR)/DAC \
	  -I $(SIMDIR)/Cache -I $(GPUDIR) -I $(TRACEDIR) \
  	  -I $(TRACEDIR)/GLLib -I $(TRACEDIR)/GPUDriver\
  	  -I $(TRACEDIR)/utils -I $(TRACEDIR)/utils/TraceReader \
  	  -I $(TRACEDIR)/utils/Gen -I $(TRACEDIR)/GLLib/ARBP \
          -I $(TRACEDIR)/utils/log/ \
	  -I $(TRACEDIR)/GLLib/ARBP/VertexProgram/ \
	  -I $(TRACEDIR)/GLLib/ARBP/FragmentProgram/ \
	  -I $(GLDIR) -I $(TRACEDIR)/GLLib/GLObject -I $(TRACEDIR)/GLLib/Texture \
          -I $(TRACEDIR)/GLLib/BufferObject

CXFLAGS = $(HOWFLAGS) $(WHEREFLAGS)
LDFLAGS = 
LIBS = -lz

TARGETS = $(BINDIR)/bGPU $(BINDIR)/bGPU-Uni

ARBPOBJS = $(OBJDIR)/GenericInstruction.o \
	$(OBJDIR)/ProgramManager.o $(OBJDIR)/ProgramObject.o\
	$(OBJDIR)/ProgramTarget.o\
	$(OBJDIR)/ProgramExecutionEnvironment.o\
	$(OBJDIR)/ShaderInstructionTranslator.o \
	$(OBJDIR)/Scheduler.o $(OBJDIR)/DependencyGraph.o \
	$(OBJDIR)/CompilerSteps.o $(OBJDIR)/VP1ExecEnvironment.o\
	$(OBJDIR)/IRNode.o $(OBJDIR)/CodeGenTraverser.o \
	$(OBJDIR)/SemanticTraverser.o\
	$(OBJDIR)/OutputTraverser.o $(OBJDIR)/FP1ExecEnvironment.o

GLOBJECT = $(OBJDIR)/BaseManager.o $(OBJDIR)/BaseTarget.o $(OBJDIR)/BaseObject.o

TEXTUREOBJS = $(OBJDIR)/TextureManager.o $(OBJDIR)/TextureTarget.o \
	$(OBJDIR)/TextureObject.o 

BUFFEROBJS = $(OBJDIR)/BufferManager.o $(OBJDIR)/BufferTarget.o \
	$(OBJDIR)/BufferObject.o

GLLIBOBJS = $(OBJDIR)/GLContext.o $(OBJDIR)/GPULib.o $(OBJDIR)/GPULibInternals.o \
	$(OBJDIR)/GLState.o $(OBJDIR)/VSLoader.o $(OBJDIR)/AuxFuncsLib.o \
	$(OBJDIR)/Matrixf.o $(OBJDIR)/MathLib.o $(OBJDIR)/TextureUnitLib.o \
	$(OBJDIR)/CodeSnip.o $(OBJDIR)/TLFactory.o \
	$(OBJDIR)/FPFactory.o $(OBJDIR)/Transformations.o \
        $(OBJDIR)/ProgramObjectCache.o $(OBJDIR)/PushState.o \
	$(OBJDIR)/ContextStateAdapter.o
	
AOGLOBJS = $(OBJDIR)/AGL.o $(OBJDIR)/AGLContext.o $(OBJDIR)/AGLEntryPoints.o $(OBJDIR)/MatrixStackImp.o \
	   $(OBJDIR)/AGLBaseManager.o $(OBJDIR)/AGLBaseObject.o $(OBJDIR)/AGLBaseTarget.o \
	   $(OBJDIR)/ARBProgramManager.o $(OBJDIR)/ARBProgramObject.o $(OBJDIR)/ARBProgramTarget.o \
	   $(OBJDIR)/ARBRegisterBank.o $(OBJDIR)/AGLTextureManager.o $(OBJDIR)/AGLTextureObject.o \
	   $(OBJDIR)/AGLTextureTarget.o
	   
ACDOBJS = $(OBJDIR)/ACDBlendingStageImp.o $(OBJDIR)/ACDBufferImp.o $(OBJDIR)/ACDDeviceImp.o $(OBJDIR)/ACDImp.o $(OBJDIR)/ACDMath.o \
	  	  $(OBJDIR)/ACDMatrixImp.o $(OBJDIR)/ACDRasterizationStageImp.o $(OBJDIR)/ACDRenderTargetImp.o $(OBJDIR)/ACDSamplerImp.o \
	  $(OBJDIR)/ACDShaderProgramImp.o $(OBJDIR)/ACDStoredStateImp.o $(OBJDIR)/ACDStreamImp.o $(OBJDIR)/ACDSupport.o \
	  $(OBJDIR)/ACDTexture2DImp.o $(OBJDIR)/ACDTextureCubeMapImp.o $(OBJDIR)/ACDZStencilStageImp.o $(OBJDIR)/MemoryObject.o \
	  $(OBJDIR)/MemoryObjectAllocator.o $(OBJDIR)/TextureMipmap.o $(OBJDIR)/TextureMipmapChain.o \
	  $(OBJDIR)/OptimizationDataStructures.o $(OBJDIR)/OptimizationSteps.o $(OBJDIR)/ShaderOptimizer.o \
	  $(OBJDIR)/ACDX.o $(OBJDIR)/ACDXCompiledProgramImp.o $(OBJDIR)/ACDXConstantBindingImp.o \
	  $(OBJDIR)/ACDXFixedPipelineStateImp.o $(OBJDIR)/ACDXLightImp.o $(OBJDIR)/ACDXMaterialImp.o $(OBJDIR)/ACDXStoredFPStateImp.o \
	  $(OBJDIR)/ClusterBankAdapter.o $(OBJDIR)/ACDXCodeGenTraverser.o $(OBJDIR)/ACDXGenericInstruction.o $(OBJDIR)/ACDXIRNode.o \
	  $(OBJDIR)/ACDXProgramExecutionEnvironment.o $(OBJDIR)/ACDXSemanticTraverser.o $(OBJDIR)/ACDXOutputTraverser.o \
	  $(OBJDIR)/ACDXShaderInstructionTranslator.o $(OBJDIR)/ACDXFP1ExecEnvironment.o $(OBJDIR)/ACDXVP1ExecEnvironment.o \
	  $(OBJDIR)/ACDXCodeSnip.o $(OBJDIR)/ACDXFPFactory.o $(OBJDIR)/ACDXGLState.o $(OBJDIR)/ACDXMatrixf.o $(OBJDIR)/ACDXTLFactory.o \
	  $(OBJDIR)/ACDXTransformations.o $(OBJDIR)/InternalConstantBinding.o $(OBJDIR)/SettingsAdapter.o $(OBJDIR)/StoredStateItem.o

D3DDRIVEROBJS = $(OBJDIR)/D3DDriver.o \
	$(OBJDIR)/ToString.o \
	$(OBJDIR)/CZStencilTest_9.o \
	$(OBJDIR)/CConstants_9.o \
	$(OBJDIR)/CDebugGeometry_9.o \
	$(OBJDIR)/CIndexVertexBuffers_9.o \
	$(OBJDIR)/CLockBox_9.o \
	$(OBJDIR)/CLockRect_9.o \
	$(OBJDIR)/CLock_9.o \
	$(OBJDIR)/CSamplers_9.o \
	$(OBJDIR)/CShaders_9.o \
	$(OBJDIR)/CStreams_9.o \
	$(OBJDIR)/CSurfaces_9.o \
	$(OBJDIR)/CVolumes_9.o \
	$(OBJDIR)/CTextures_9.o \
	$(OBJDIR)/CVSInput_9.o \
	$(OBJDIR)/Matrices.o \
	$(OBJDIR)/ShaderGenerate.o \
	$(OBJDIR)/CRenderTargetZStencil_9.o \
	$(OBJDIR)/CDebugState.o \
	$(OBJDIR)/CDevice_9.o \
	$(OBJDIR)/CIndexStream_9.o \
	$(OBJDIR)/CAlphaBlending_9.o \
	$(OBJDIR)/CRasterization_9.o \
	$(OBJDIR)/CRoot.o \
	$(OBJDIR)/CSemanticMapping_9.o \
	$(OBJDIR)/Utils.o \
	$(OBJDIR)/D3DControllers.o \
	$(OBJDIR)/GPUProxy.o \
	$(OBJDIR)/ResourceAssignationTable.o \
	$(OBJDIR)/IR.o \
	$(OBJDIR)/IRBuilder.o \
	$(OBJDIR)/IRPrinter.o \
	$(OBJDIR)/IRTranslator.o \
	$(OBJDIR)/ShaderTranslator.o \
	$(OBJDIR)/D3DState.o \
	$(OBJDIR)/StateDataTree.o \
	$(OBJDIR)/CoverBuffer_9.o \
	$(OBJDIR)/D3DInterface.o \
	$(OBJDIR)/IBaseTextureImp_9.o \
	$(OBJDIR)/ICubeTextureImp_9.o \
	$(OBJDIR)/IDeviceImp_9.o \
	$(OBJDIR)/IDirect3DImp_9.o \
	$(OBJDIR)/IFunctionsImp_9.o \
	$(OBJDIR)/IIndexBufferImp_9.o \
	$(OBJDIR)/IPixelShaderImp_9.o \
	$(OBJDIR)/IQueryImp_9.o \
	$(OBJDIR)/IResourceImp_9.o \
	$(OBJDIR)/IRoot_9.o \
	$(OBJDIR)/IStateBlockImp_9.o \
	$(OBJDIR)/ISurfaceImp_9.o \
	$(OBJDIR)/ISwapChainImp_9.o \
	$(OBJDIR)/ITextureImp_9.o \
	$(OBJDIR)/IVertexBufferImp_9.o \
	$(OBJDIR)/IVertexDeclarationImp_9.o \
	$(OBJDIR)/IVertexShaderImp_9.o \
	$(OBJDIR)/IVolumeImp_9.o \
	$(OBJDIR)/IVolumeTextureImp_9.o



D3DTRACEOBJS = $(OBJDIR)/D3DTraceDriver.o $(OBJDIR)/D3D9PixRunPlayer.o $(OBJDIR)/D3DPixRunReader.o $(OBJDIR)/D3D9Status.o $(OBJDIR)/D3DBuffer.o $(OBJDIR)/D3D9Info.o $(OBJDIR)/D3DConfiguration.o $(OBJDIR)/IniFile.o $(OBJDIR)/StringUtils.o 


MEMORYCONTROLLERV2OBJS = $(OBJDIR)/ChannelScheduler.o $(OBJDIR)/ChannelTransaction.o \
	$(OBJDIR)/DDRBank.o $(OBJDIR)/DDRBurst.o $(OBJDIR)/DDRCommand.o \
	$(OBJDIR)/DDRModule.o $(OBJDIR)/DDRModuleState.o\
	$(OBJDIR)/MemoryRequestSplitter.o \
	$(OBJDIR)/MCSplitter.o $(OBJDIR)/MCSplitter2.o $(OBJDIR)/MemoryControllerV2.o\
	$(OBJDIR)/MemoryRequest.o $(OBJDIR)/DependencyQueue.o \
	$(OBJDIR)/SchedulerSelector.o $(OBJDIR)/FifoSchedulerBase.o \
	$(OBJDIR)/FifoScheduler.o $(OBJDIR)/RWFifoScheduler.o \
	$(OBJDIR)/BankQueueScheduler.o $(OBJDIR)/BankRWQueueScheduler.o \
	$(OBJDIR)/MemoryTraceRecorder.o

SHARED_OBJECTS = $(OBJDIR)/ShaderInstruction.o $(OBJDIR)/ShaderEmulator.o \
	  $(OBJDIR)/GPUMath.o $(OBJDIR)/SetupTriangle.o \
          $(OBJDIR)/Fragment.o $(OBJDIR)/RasterizerEmulator.o \
	  $(OBJDIR)/Tile.o $(OBJDIR)/ClipperEmulator.o \
	  $(OBJDIR)/FragmentOpEmulator.o $(OBJDIR)/Cache.o \
          $(OBJDIR)/TextureEmulator.o $(OBJDIR)/Cache64.o \
	  $(OBJDIR)/CacheReplacement.o \
	  $(OBJDIR)/Clipper.o \
	  $(OBJDIR)/ClipperCommand.o $(OBJDIR)/ClipperStateInfo.o \
          $(OBJDIR)/ClipperStatusInfo.o \
          $(OBJDIR)/Box.o $(OBJDIR)/GPUSignal.o $(OBJDIR)/Statistic.o \
          $(OBJDIR)/SignalBinder.o $(OBJDIR)/StatisticsManager.o   \
          $(OBJDIR)/ShaderCommand.o $(OBJDIR)/ShaderExecInstruction.o \
          $(OBJDIR)/ShaderDecodeCommand.o $(OBJDIR)/AGPTransaction.o \
	  $(OBJDIR)/CommandProcessor.o $(OBJDIR)/MemoryController.o \
	  $(OBJDIR)/MemoryControllerCommand.o \
	  $(OBJDIR)/MemoryTransaction.o $(OBJDIR)/Streamer.o \
	  $(OBJDIR)/StreamerFetch.o $(OBJDIR)/StreamerOutputCache.o \
	  $(OBJDIR)/StreamerLoader.o $(OBJDIR)/StreamerCommit.o \
	  $(OBJDIR)/StreamerCommand.o $(OBJDIR)/StreamerControlCommand.o \
          $(OBJDIR)/InputCache.o $(OBJDIR)/RasterizerCommand.o \
	  $(OBJDIR)/ShaderInput.o $(OBJDIR)/ShaderStateInfo.o\
	  $(OBJDIR)/ShaderDecodeStateInfo.o $(OBJDIR)/StreamerStateInfo.o \
	  $(OBJDIR)/ConsumerStateInfo.o $(OBJDIR)/RasterizerStateInfo.o \
	  $(OBJDIR)/PrimitiveAssembly.o $(OBJDIR)/PrimitiveAssemblyInput.o \
	  $(OBJDIR)/PrimitiveAssemblyCommand.o \
	  $(OBJDIR)/PrimitiveAssemblyRequest.o \
	  $(OBJDIR)/PrimitiveAssemblyStateInfo.o \
	  $(OBJDIR)/TriangleSetup.o $(OBJDIR)/TriangleSetupInput.o \
	  $(OBJDIR)/TriangleSetupOutput.o $(OBJDIR)/TriangleSetupStateInfo.o \
	  $(OBJDIR)/TriangleTraversal.o $(OBJDIR)/TriangleSetupRequest.o \
	  $(OBJDIR)/Interpolator.o $(OBJDIR)/FFIFOStateInfo.o \
	  $(OBJDIR)/FragmentInput.o \
	  $(OBJDIR)/HierarchicalZ.o $(OBJDIR)/HZStateInfo.o \
	  $(OBJDIR)/HZAccess.o $(OBJDIR)/HZUpdate.o \
	  $(OBJDIR)/TextureUnit.o $(OBJDIR)/FilterOperation.o \
	  $(OBJDIR)/TextureRequest.o $(OBJDIR)/TextureResult.o \
	  $(OBJDIR)/TextureUnitStateInfo.o $(OBJDIR)/TextureCache.o \
	  $(OBJDIR)/TextureCacheL2.o $(OBJDIR)/TextureCacheGen.o \
	  $(OBJDIR)/ZCacheV2.o $(OBJDIR)/GenericROP.o \
	  $(OBJDIR)/ROPOperation.o $(OBJDIR)/ROPStatusInfo.o \
	  $(OBJDIR)/ROPCache.o $(OBJDIR)/ZStencilTestV2.o \
	  $(OBJDIR)/ColorCacheV2.o $(OBJDIR)/FetchCache.o \
	  $(OBJDIR)/FetchCache64.o $(OBJDIR)/ColorWriteV2.o \
	  $(OBJDIR)/ColorBlockStateInfo.o $(OBJDIR)/DAC.o $(OBJDIR)/Blitter.o\
	  $(OBJDIR)/AGPTraceDriver.o\
	  $(OBJDIR)/GLTraceDriver.o $(OBJDIR)/RegisterWriteBufferAGP.o \
	  $(OBJDIR)/TraceReader.o $(OBJDIR)/GPUDriver.o \
	  $(D3DTRACEOBJS) \
	  $(OBJDIR)/LogObject.o $(OBJDIR)/IncludeLog.o \
	  $(OBJDIR)/ShaderProgramSched.o $(OBJDIR)/RegisterWriteBuffer.o \
	  $(OBJDIR)/GPUMemory.o $(OBJDIR)/zfstream.o \
	  $(OBJDIR)/BufferDescriptor.o $(OBJDIR)/MemoryRegion.o \
	  $(OBJDIR)/DArray.o $(OBJDIR)/GLExec.o $(OBJDIR)/GLExecStats.o $(OBJDIR)/GLJumpTable.o \
	  $(OBJDIR)/GLResolver.o $(OBJDIR)/StubApiCalls.o \
	  $(OBJDIR)/support.o $(OBJDIR)/QuadFloat.o $(OBJDIR)/QuadInt.o \
	  $(OBJDIR)/OptimizedDynamicMemory.o $(OBJDIR)/DynamicObject.o \
	  $(OBJDIR)/Parser.o $(ARBPOBJS) $(GLLIBOBJS) \
          $(GLOBJECT) $(TEXTUREOBJS) $(BUFFEROBJS) $(MEMORYCONTROLLERV2OBJS) \
          $(AOGLOBJS) $(ACDOBJS) $(D3DDRIVEROBJS)

UNI_OBJECTS = $(OBJDIR)/Rasterizer.o $(OBJDIR)/FragmentFIFO.o \
	      $(OBJDIR)/ShaderFetch.o $(OBJDIR)/ShaderDecodeExecute.o

OBJECTS = $(OBJDIR)/Rasterizer.o $(OBJDIR)/FragmentFIFO.o $(OBJDIR)/ShaderFetch.o \
	  $(OBJDIR)/ShaderDecodeExecute.o

LOCALOBJS = $(OBJDIR)/ConfigLoader.o $(OBJDIR)/LineReader.o $(OBJDIR)/MemoryControllerSelector.o

BGPU = $(OBJDIR)/bGPU.o

BGPU_UNI = $(OBJDIR)/bGPU-Unified.o

DELETEOBJS = $(LOCALOBJS) $(BGPU) $(BGPU_UNI) $(TARGETS)

BGPUBIN = $(BINDIR)/bGPU

BGPU_UNIBIN = $(BINDIR)/bGPU-Uni

all: $(TARGETS)

bgpu: $(BGPUBIN)

bgpu-uni: $(BGPU_UNIBIN)

$(LOCALOBJS): $(OBJDIR)/%.o : %.cpp
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)

$(BGPU): bGPU-Unified.cpp bGPU-Unified.h
	$(CX) $(CXFLAGS) -c -o $(OBJDIR)/bGPU.o bGPU-Unified.cpp $(INCLUDE) $(LIBS) $(LDFLAGS)

$(BGPU_UNI): bGPU-Unified.cpp bGPU-Unified.h
	$(CX) $(CXFLAGS) -c -o $(OBJDIR)/bGPU-Unified.o bGPU-Unified.cpp -D UNIFIEDSHADER $(INCLUDE) $(LIBS) $(LDFLAGS)
	
$(BGPUBIN): $(SHARED_OBJECTS) $(OBJECTS) $(BGPU) $(LOCALOBJS)
	$(CX) $(CXFLAGS) -o $@ $(BGPU) $(SHARED_OBJECTS) $(OBJECTS) $(LOCALOBJS) $(INCLUDE) $(LIBS) $(LDFLAGS)
	
$(BGPU_UNIBIN): $(SHARED_OBJECTS) $(UNI_OBJECTS) $(BGPU_UNI) $(LOCALOBJS)
	$(CX) $(CXFLAGS) -o $@ $(BGPU_UNI) $(SHARED_OBJECTS) $(UNI_OBJECTS) $(LOCALOBJS) $(INCLUDE) $(LIBS) $(LDFLAGS)

clean:
	for o in $(DELETEOBJS); do (rm $$o); done
