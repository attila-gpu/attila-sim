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
#  $Revision: 1.44 $
#  $Author: vmoya $
#  $Date: 2008-02-22 18:32:57 $
# 
#  Trace tools and support files Makefile. 
# 
# 

include ../Makefile.defs

INCLUDE = -I ./ -I ./GPUDriver -I ./GLLib -I ./utils/ -I ./utils/log/ \
	  -I ./utils/Gen -I ./utils/TraceReader -I $(SUPPORTDIR)/ \
	  -I ./utils/Parsers/ -I ./CodeGenerator \
	  -I $(SIMDIR)/CommandProcessor -I $(SIMDIR)/MemoryController \
          -I $(SIMDIR)/Shader -I $(SIMDIR)/ \
	  -I $(EMUDIR)/ -I ./GLLib/ARBP/ -I ./GLLib/ARBP/VertexProgram/ \
	  -I ./GLLib/ARBP/FragmentProgram/ -I $(GLDIR)\
	  -I ./GLLib/Texture/ -I ./GLLib/GLObject/ \
	  -I ./GLLib/BufferObject/ -I ./extractTraceRegion/ -I ./gl2atila/ -I ../bgpu/ \
	  -I ./AOGL/ -I ./AOGL/ARBProgramObject/ -I ./AOGL/AGLBaseObject/ -I ./AOGL/AGLTexture/ \
	  -I ./ACD/Interface -I ./ACDX/Interface -I ./ACD/Implementation \
	  -I ./ACD/Implementation/ShaderOptimization -I ./ACDX/Implementation -I ./ACDX/Implementation/FPEmulation \
	  -I ./ACDX/Implementation/ARBCompilers

LIBS = -lz

CODEGENDIR = $(TRACEDIR)/CodeGenerator
UTILSDIR = $(TRACEDIR)/utils

CXFLAGS = $(HOWFLAGS) $(WHEREFLAGS) -D LOAD_JUMPTABLE_STATICALLY -D GL_GLEXT_LEGACY

TARGETS = codegen d3ddriverbuild tracedriver tools gllib acd

PARSERS = $(OBJDIR)/ConstDescription.o $(OBJDIR)/ConstExtractor.o \
	  $(OBJDIR)/FuncDescription.o $(OBJDIR)/FuncExtractor.o \
	  $(OBJDIR)/NameModifiers.o $(OBJDIR)/ParamDescription.o \
	  $(OBJDIR)/SpecificExtractor.o $(OBJDIR)/StringTokenizer.o \
	  $(OBJDIR)/StringToFuncDescription.o

CODEGENERATOR = $(OBJDIR)/CodeGenerator.o $(OBJDIR)/JTGenerator.o \
	  $(OBJDIR)/GLNamesGenerator.o $(OBJDIR)/StubGenerator.o \
	  $(OBJDIR)/SwitchBranchesGenerator.o \
	  $(OBJDIR)/WrapperGenerator.o $(OBJDIR)/main.o \
	  $(OBJDIR)/GLIEntryPointsGenerator.o \

GLOBJECT = $(OBJDIR)/BaseManager.o $(OBJDIR)/BaseTarget.o $(OBJDIR)/BaseObject.o 

VP1_ARBP = $(OBJDIR)/VP1ExecEnvironment.o

FP1_ARBP = $(OBJDIR)/FP1ExecEnvironment.o

ARBP =  $(OBJDIR)/IRNode.o $(OBJDIR)/CodeGenTraverser.o\
	$(OBJDIR)/SemanticTraverser.o $(OBJDIR)/OutputTraverser.o\
	$(OBJDIR)/GenericInstruction.o $(OBJDIR)/ProgramManager.o\
        $(OBJDIR)/ProgramObject.o $(OBJDIR)/ProgramTarget.o\
        $(OBJDIR)/ProgramExecutionEnvironment.o\
        $(OBJDIR)/ShaderInstructionTranslator.o\
	$(OBJDIR)/CompilerSteps.o $(OBJDIR)/DependencyGraph.o \
	$(OBJDIR)/Scheduler.o

	

TEXTURE = $(OBJDIR)/TextureManager.o $(OBJDIR)/TextureTarget.o \
	$(OBJDIR)/TextureObject.o

BUFFEROBJECTS = $(OBJDIR)/BufferManager.o $(OBJDIR)/BufferTarget.o \
                $(OBJDIR)/BufferObject.o


CODEGENBIN = $(CODEGENDIR)/codegenerator

# All files requiring codegeneration must include CGEN_FILES as pre-requisite
CGEN_FILES = $(CODEGENBIN) $(CODEGENDIR)/CGconfig.ini \
             $(UTILSDIR)/Gen/APICall.gen 

SUPPORT = $(OBJDIR)/support.o

TRACEDRIVER = $(OBJDIR)/AGPTraceDriver.o $(OBJDIR)/GLTraceDriver.o $(OBJDIR)/RegisterWriteBufferAGP.o $(OBJDIR)/D3DTraceDriver.o

TRACEREADER = $(OBJDIR)/TraceReader.o $(OBJDIR)/StubApiCalls.o \
	      $(OBJDIR)/GLExec.o $(OBJDIR)/GLExecStats.o

TRACEUTILS = $(OBJDIR)/BufferDescriptor.o $(OBJDIR)/MemoryRegion.o \
	     $(OBJDIR)/DArray.o $(OBJDIR)/GLJumpTable.o $(OBJDIR)/GLResolver.o \
	     $(OBJDIR)/zfstream.o

TRACELOGDIR = $(OBJDIR)/LogObject.o $(OBJDIR)/IncludeLog.o

GPUDRIVER = $(OBJDIR)/GPUDriver.o $(OBJDIR)/RegisterWriteBuffer.o \
	    $(OBJDIR)/ShaderProgramSched.o

GLLIB = $(OBJDIR)/GPULib.o $(OBJDIR)/GPULibInternals.o $(OBJDIR)/MathLib.o \
	$(OBJDIR)/GLState.o $(OBJDIR)/VSLoader.o $(OBJDIR)/Matrixf.o \
	$(OBJDIR)/AuxFuncsLib.o $(OBJDIR)/GLContext.o \
	$(OBJDIR)/CodeSnip.o $(OBJDIR)/TLFactory.o $(OBJDIR)/FPFactory.o \
	$(OBJDIR)/Transformations.o $(OBJDIR)/TextureUnitLib.o \
	$(OBJDIR)/GPUMemory.o $(OBJDIR)/ProgramObjectCache.o \
	$(OBJDIR)/PushState.o $(OBJDIR)/ContextStateAdapter.o

D3DDRIVER = $(OBJDIR)/D3DDriver.o \
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
	$(OBJDIR)/IVolumeTextureImp_9.o \
	$(OBJDIR)/AIBaseTextureImp_9.o \
	$(OBJDIR)/AICubeTextureImp_9.o \
	$(OBJDIR)/AIDeviceImp_9.o \
	$(OBJDIR)/AIDirect3DImp_9.o \
	$(OBJDIR)/AIIndexBufferImp_9.o \
	$(OBJDIR)/AIPixelShaderImp_9.o \
	$(OBJDIR)/AIQueryImp_9.o \
	$(OBJDIR)/AIResourceImp_9.o \
	$(OBJDIR)/AIRoot_9.o \
	$(OBJDIR)/AIStateBlockImp_9.o \
	$(OBJDIR)/AISurfaceImp_9.o \
	$(OBJDIR)/AISwapChainImp_9.o \
	$(OBJDIR)/AITextureImp_9.o \
	$(OBJDIR)/AIVertexBufferImp_9.o \
	$(OBJDIR)/AIVertexDeclarationImp_9.o \
	$(OBJDIR)/AIVertexShaderImp_9.o \
	$(OBJDIR)/AIVolumeImp_9.o \
	$(OBJDIR)/AIVolumeTextureImp_9.o \
	$(OBJDIR)/D3D9PixRunPlayer.o \
	$(OBJDIR)/D3DBuffer.o \
        $(OBJDIR)/D3D9Status.o \
	$(OBJDIR)/D3D9Info.o \
        $(OBJDIR)/D3DConfiguration.o \
	$(OBJDIR)/D3DPixRunReader.o \
	$(OBJDIR)/IniFile.o \
	$(OBJDIR)/StringUtils.o


AOGL = $(OBJDIR)/AGL.o $(OBJDIR)/AGLContext.o $(OBJDIR)/AGLEntryPoints.o $(OBJDIR)/MatrixStackImp.o

AGLOBJECT = $(OBJDIR)/AGLBaseManager.o $(OBJDIR)/AGLBaseObject.o $(OBJDIR)/AGLBaseTarget.o

ARBPROGRAM = $(OBJDIR)/ARBProgramManager.o $(OBJDIR)/ARBProgramObject.o $(OBJDIR)/ARBProgramTarget.o $(OBJDIR)/ARBRegisterBank.o

AGLTEXTURE = $(OBJDIR)/AGLTextureManager.o $(OBJDIR)/AGLTextureObject.o $(OBJDIR)/AGLTextureTarget.o

ACD = $(OBJDIR)/ACDBlendingStageImp.o $(OBJDIR)/ACDBufferImp.o $(OBJDIR)/ACDDeviceImp.o $(OBJDIR)/ACDImp.o $(OBJDIR)/ACDMath.o \
	  $(OBJDIR)/ACDMatrixImp.o $(OBJDIR)/ACDRasterizationStageImp.o $(OBJDIR)/ACDRenderTargetImp.o $(OBJDIR)/ACDSamplerImp.o \
	  $(OBJDIR)/ACDShaderProgramImp.o $(OBJDIR)/ACDStoredStateImp.o $(OBJDIR)/ACDStreamImp.o $(OBJDIR)/ACDSupport.o \
	  $(OBJDIR)/ACDTexture2DImp.o $(OBJDIR)/ACDTextureCubeMapImp.o $(OBJDIR)/ACDZStencilStageImp.o $(OBJDIR)/MemoryObject.o \
	  $(OBJDIR)/MemoryObjectAllocator.o $(OBJDIR)/TextureMipmap.o $(OBJDIR)/TextureMipmapChain.o $(OBJDIR)/StoredStateItem.o
	  
ACDSHOPTIMIZE = $(OBJDIR)/OptimizationDataStructures.o $(OBJDIR)/OptimizationSteps.o $(OBJDIR)/ShaderOptimizer.o

ACDX = $(OBJDIR)/ACDX.o $(OBJDIR)/ACDXCompiledProgramImp.o $(OBJDIR)/ACDXConstantBindingImp.o \
	  $(OBJDIR)/ACDXFixedPipelineStateImp.o $(OBJDIR)/ACDXLightImp.o $(OBJDIR)/ACDXMaterialImp.o $(OBJDIR)/ACDXStoredFPStateImp.o \
	  $(OBJDIR)/ClusterBankAdapter.o $(OBJDIR)/InternalConstantBinding.o $(OBJDIR)/SettingsAdapter.o
	  
ACDXARBCOMPILERS = $(OBJDIR)/ACDXCodeGenTraverser.o $(OBJDIR)/ACDXGenericInstruction.o $(OBJDIR)/ACDXIRNode.o \
	  $(OBJDIR)/ACDXProgramExecutionEnvironment.o $(OBJDIR)/ACDXSemanticTraverser.o $(OBJDIR)/ACDXShaderInstructionTranslator.o \
	  $(OBJDIR)/ACDXOutputTraverser.o
	   
ACDXVERTEXPROGRAM = $(OBJDIR)/ACDXVP1ExecEnvironment.o 

ACDXFRAGMENTPROGRAM = $(OBJDIR)/ACDXFP1ExecEnvironment.o

ACDXFPEMULATION = $(OBJDIR)/ACDXCodeSnip.o $(OBJDIR)/ACDXFPFactory.o $(OBJDIR)/ACDXGLState.o $(OBJDIR)/ACDXMatrixf.o \
	  $(OBJDIR)/ACDXTLFactory.o $(OBJDIR)/ACDXTransformations.o 

GL2ATILA_EXT = $(GLLIB) $(GPUDRIVER) $(ARBP) $(VP1_ARBP) $(FP1_ARBP) $(TRACEDRIVER) \
               $(TRACEREADER) $(TRACEUTILS) $(GLOBJECT) $(TEXTURE) $(BUFFEROBJECTS) \
               $(TRACELOGDIR) $(OBJDIR)/ConfigLoader.o $(OBJDIR)/ShaderInstruction.o \
               $(OBJDIR)/support.o $(OBJDIR)/QuadFloat.o $(OBJDIR)/AGPTransaction.o \
               $(OBJDIR)/OptimizedDynamicMemory.o $(OBJDIR)/DynamicObject.o \
               $(OBJDIR)/Parser.o $(D3DDRIVER) $(AOGL) $(AGLOBJECT) $(ARBPROGRAM) \
	       $(ACD) $(ACDX) $(ACDXARBCOMPILERS) $(ACDXVERTEXPROGRAM) \
               $(ACDXFRAGMENTPROGRAM) $(ACDXFPEMULATION) $(AGLTEXTURE) $(ACDSHOPTIMIZE) \
               $(OBJDIR)/GPUMath.o

GL2ATILA = $(OBJDIR)/gl2atila.o

GL2ATILA_BIN = $(BINDIR)/gl2atila

EXTRACTTRACEREGION_EXT = $(OBJDIR)/QuadFloat.o $(OBJDIR)/AGPTransaction.o \
                         $(OBJDIR)/OptimizedDynamicMemory.o $(OBJDIR)/DynamicObject.o \
                         $(OBJDIR)/zfstream.o $(OBJDIR)/support.o $(OBJDIR)/RegisterWriteBufferAGP.o

EXTRACTTRACEREGION = $(OBJDIR)/extractTraceRegion.o

EXTRACTTRACEREGION_BIN = $(BINDIR)/extractTraceRegion
  
OBJECTS = $(PARSERS) $(CODEGENERATOR) $(CODEGENBIN) \
	  $(TRACEDRIVER) $(TRACEREADER) $(TRACEUTILS) $(GPUDRIVER) $(GLLIB) $(D3DDRIVER) \
	  $(ARBP) $(VP1_ARBP) $(FP1_ARBP) $(TEXTURE) $(GLOBJECT) \
	  $(BUFFEROBJECTS) $(TRACELOGDIR) $(GL2ATILA) $(EXTRACTRACEREGION) \
	  $(AOGL) $(AGLOBJECT) $(ARBPROGRAM) $(AGLTEXTURE) $(ACD) $(ACDSHOPTIMIZE) $(ACDX) \
	  $(ACDXFPEMULATION) $(ACDXARBCOMPILERS) $(ACDXVERTEXPROGRAM) $(ACDXFRAGMENTPROGRAM)
	  

TOOLS = 
#TOOLS = glplay gltrace

all: $(TARGETS)

tracedriver: $(GPUDRIVER) $(GLLIB) $(TRACEDRIVER) $(TRACEREADER) \
             $(TRACEUTILS) $(GLOBJECT) $(ARBP) $(VP1_ARBP) $(FP1_ARBP) \
             $(TEXTURE) $(BUFFEROBJECTS) $(TRACELOGDIR)
	@cd D3DPixRunPlayer ; $(MAKE)

codegen: $(CODEGENBIN)

gllib: $(GLLIB) $(GPUDRIVER) $(ARBP) $(VP1_ARBP) $(FP1_ARBP) \
		$(AOGL) $(AGLOBJECT) $(AGLTEXTURE) $(ARBPROGRAM) 

d3ddriverbuild: 
	cd D3DDriver ; $(MAKE)

acd: $(ACD) $(ACDSHOPTIMIZE) $(ACDX) $(ACDXFPEMULATION) $(ACDXARBCOMPILERS) $(ACDXVERTEXPROGRAM) $(ACDXFRAGMENTPROGRAM)

gl2atila: gllib $(GL2ATILA) $(GL2ATILA_EXT) $(GL2ATILA_BIN)

extractTraceRegion: $(EXTRACTTRACEREGION) $(EXTRACTTRACEREGION_EXT) $(EXTRACTTRACEREGION_BIN)

# esta regla genera al generador de código y lo invoca...
$(CODEGENBIN): $(PARSERS) $(CODEGENERATOR) $(SUPPORT)
	echo ############################
	echo # Linking CodeGenerator... #
	echo ############################
	$(CX) $(CXFLAGS) -o $@ $(PARSERS) $(CODEGENERATOR) $(SUPPORT) $(LIBS) $(LDFLAGS)
	echo CodeGenerator linked...
        
# genera codigo .gen si el generador de codigo ha cambiado o el config ha sido modificado
$(UTILSDIR)/Gen/APICall.gen : $(CODEGENBIN) $(CODEGENDIR)/CGconfig.ini \
	                      $(UTILSDIR)/GL/OGL_functions_supported_by_bGPU.h 
	echo ##############################
	echo # Executing CodeGenerator... #
	echo ##############################
	cd $(CODEGENDIR); ./codegenerator


$(SUPPORT): $(OBJDIR)/%.o : $(SUPPORTDIR)/%.cpp
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)

$(PARSERS): $(OBJDIR)/%.o : $(UTILSDIR)/Parsers/%.cpp
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)

$(CODEGENERATOR): $(OBJDIR)/%.o : $(CODEGENDIR)/%.cpp
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)

$(TRACEDRIVER): $(OBJDIR)/%.o : %.cpp $(CGEN_FILES)
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)

$(TRACEREADER): $(OBJDIR)/%.o : $(UTILSDIR)/TraceReader/%.cpp $(UTILSDIR)/TraceReader/%.h $(CGEN_FILES)
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)

$(TRACEUTILS):  $(OBJDIR)/%.o : $(UTILSDIR)/%.cpp $(CGEN_FILES)
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)

$(TRACELOGDIR): $(OBJDIR)/%.o : $(UTILSDIR)/log/%.cpp
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)

$(GPUDRIVER): $(OBJDIR)/%.o : GPUDriver/%.cpp
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)
	
$(GLLIB): $(OBJDIR)/%.o : GLLib/%.cpp $(OBJDIR)/GLResolver.o
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)

$(GLOBJECT): $(OBJDIR)/%.o : GLLib/GLObject/%.cpp
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)

$(ARBP): $(OBJDIR)/%.o : GLLib/ARBP/%.cpp
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)

$(VP1_ARBP): $(OBJDIR)/%.o : GLLib/ARBP/VertexProgram/%.cpp GLLib/ARBP/VertexProgram/VP1Bison.gen
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)

$(FP1_ARBP): $(OBJDIR)/%.o : GLLib/ARBP/FragmentProgram/%.cpp GLLib/ARBP/FragmentProgram/FP1Bison.gen
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)

$(TEXTURE): $(OBJDIR)/%.o : GLLib/Texture/%.cpp
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)

$(BUFFEROBJECTS) : $(OBJDIR)/%.o : GLLib/BufferObject/%.cpp
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)

GLLib/ARBP/VertexProgram/VP1Flex.gen: GLLib/ARBP/VertexProgram/VP1Lexic.l
	flex -Pvp1 -t -8 GLLib/ARBP/VertexProgram/VP1Lexic.l > GLLib/ARBP/VertexProgram/VP1Flex.gen

GLLib/ARBP/VertexProgram/VP1Bison.gen: GLLib/ARBP/VertexProgram/VP1Grammar.y GLLib/ARBP/VertexProgram/VP1Flex.gen
	bison -p vp1 -t GLLib/ARBP/VertexProgram/VP1Grammar.y -o GLLib/ARBP/VertexProgram/VP1Bison.gen

GLLib/ARBP/FragmentProgram/FP1Flex.gen: GLLib/ARBP/FragmentProgram/FP1Lexic.l
	flex -Pfp1 -t -8 GLLib/ARBP/FragmentProgram/FP1Lexic.l > GLLib/ARBP/FragmentProgram/FP1Flex.gen

GLLib/ARBP/FragmentProgram/FP1Bison.gen: GLLib/ARBP/FragmentProgram/FP1Grammar.y GLLib/ARBP/FragmentProgram/FP1Flex.gen
	bison -p fp1 -t GLLib/ARBP/FragmentProgram/FP1Grammar.y -o GLLib/ARBP/FragmentProgram/FP1Bison.gen

$(GL2ATILA): $(OBJDIR)/%.o : gl2atila/%.cpp
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)

$(EXTRACTTRACEREGION): $(OBJDIR)/%.o : extractTraceRegion/%.cpp
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)

$(BINDIR)/gl2atila: $(GL2ATILA)
	$(CX) $(CXFLAGS) -o $@ $(GL2ATILA) $(GL2ATILA_EXT) $(INCLUDE) $(LIBS) $(LDFLAGS)

$(BINDIR)/extractTraceRegion: $(EXTRACTTRACEREGION)
	$(CX) $(CXFLAGS) -o $@ $(EXTRACTTRACEREGION) $(EXTRACTTRACEREGION_EXT) $(INCLUDE) $(LIBS) $(LDFLAGS)

$(AOGL): $(OBJDIR)/%.o : AOGL/%.cpp
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)
	
$(AGLOBJECT): $(OBJDIR)/%.o : AOGL/AGLBaseObject/%.cpp
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)
	
$(AGLTEXTURE): $(OBJDIR)/%.o : AOGL/AGLTextureObject/%.cpp
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)
	
$(ARBPROGRAM): $(OBJDIR)/%.o : AOGL/ARBProgramObject/%.cpp
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)
	
$(ACD): $(OBJDIR)/%.o : ACD/Implementation/%.cpp
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)
	
$(ACDSHOPTIMIZE): $(OBJDIR)/%.o : ACD/Implementation/ShaderOptimization/%.cpp
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)
	
$(ACDX): $(OBJDIR)/%.o : ACDX/Implementation/%.cpp
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)
	
$(ACDXFPEMULATION): $(OBJDIR)/%.o : ACDX/Implementation/FPEmulation/%.cpp
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)
	
$(ACDXARBCOMPILERS): $(OBJDIR)/%.o : ACDX/Implementation/ARBCompilers/%.cpp
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)
	
$(ACDXVERTEXPROGRAM): $(OBJDIR)/%.o : ACDX/Implementation/ARBCompilers/VertexProgram/%.cpp ACDX/Implementation/ARBCompilers/VertexProgram/ACDXVP1Bison.gen
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)

$(ACDXFRAGMENTPROGRAM): $(OBJDIR)/%.o : ACDX/Implementation/ARBCompilers/FragmentProgram/%.cpp ACDX/Implementation/ARBCompilers/FragmentProgram/ACDXFP1Bison.gen
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS) $(LDFLAGS)

ACDX/Implementation/ARBCompilers/VertexProgram/ACDXVP1Flex.gen: ACDX/Implementation/ARBCompilers/VertexProgram/ACDXVP1Lexic.l
	flex -Pacdxvp1 -t -8 ACDX/Implementation/ARBCompilers/VertexProgram/ACDXVP1Lexic.l > ACDX/Implementation/ARBCompilers/VertexProgram/ACDXVP1Flex.gen

ACDX/Implementation/ARBCompilers/VertexProgram/ACDXVP1Bison.gen: ACDX/Implementation/ARBCompilers/VertexProgram/ACDXVP1Grammar.y ACDX/Implementation/ARBCompilers/VertexProgram/ACDXVP1Flex.gen
	bison -p acdxvp1 -t ACDX/Implementation/ARBCompilers/VertexProgram/ACDXVP1Grammar.y -o ACDX/Implementation/ARBCompilers/VertexProgram/ACDXVP1Bison.gen

ACDX/Implementation/ARBCompilers/FragmentProgram/ACDXFP1Flex.gen: ACDX/Implementation/ARBCompilers/FragmentProgram/ACDXFP1Lexic.l
	flex -Pacdxfp1 -t -8 ACDX/Implementation/ARBCompilers/FragmentProgram/ACDXFP1Lexic.l > ACDX/Implementation/ARBCompilers/FragmentProgram/ACDXFP1Flex.gen

ACDX/Implementation/ARBCompilers/FragmentProgram/ACDXFP1Bison.gen: ACDX/Implementation/ARBCompilers/FragmentProgram/ACDXFP1Grammar.y ACDX/Implementation/ARBCompilers/FragmentProgram/ACDXFP1Flex.gen
	bison -p acdxfp1 -t ACDX/Implementation/ARBCompilers/FragmentProgram/ACDXFP1Grammar.y -o ACDX/Implementation/ARBCompilers/FragmentProgram/ACDXFP1Bison.gen

tools: $(TOOLS)

glplay:
	cd GLPlay/; $(MAKE) all

gltrace:
	cd GLTrace/; $(MAKE)

clean:
	for o in $(OBJECTS); do (rm $$o); done
	cd $(UTILSDIR)/Gen; rm *.gen
	cd $(UTILSDIR)/TraceReader; rm StubApiCalls.*
	cd $(TRACEDIR)/GLLib/ARBP/VertexProgram; rm *.gen 
	cd $(TRACEDIR)/GLLib/ARBP/FragmentProgram; rm *.gen 
	rm $(BINDIR)/gl2atila
	rm $(BINDIR)/extractTraceRegion

#	cd GLTrace/; $(MAKE) clean
#	cd GLPlay/; $(MAKE) clean
