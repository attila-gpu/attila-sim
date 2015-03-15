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
#  $Revision: 1.33 $
#  $Author: cgonzale $
#  $Date: 2008-02-29 15:56:39 $
# 
#  Shader Simulator classes Makefile. 
# 
# 

include ../Makefile.defs

INCLUDE = -I . -I $(SUPPORTDIR) -I $(EMUDIR) -I $(GPUDIR) -I $(TRACEDIR) \
	  -I $(TRACEDIR)/GLLib -I $(TRACEDIR)/GPUDriver \
	  -I $(TRACEDIR)/GLLib/GLObject \
	  -I $(TRACEDIR)/utils -I $(TRACEDIR)/utils/TraceReader \
          -I $(TRACEDIR)/utils/log \
	  -I $(TRACEDIR)/utils/Gen -I $(GLDIR) \
	  -I $(BGPUDIR) -I ./CommandProcessor -I ./MemoryController \
	  -I ./MemoryControllerV2 \
	  -I ./Streamer -I ./Shader -I ./PrimitiveAssembly -I ./Rasterizer \
	  -I ./Clipper -I ./FragmentOperations -I ./DAC -I ./Cache
CXFLAGS = $(HOWFLAGS) $(WHEREFLAGS)
LIBS = 

COMMANDPROCESSOR = $(OBJDIR)/CommandProcessor.o $(OBJDIR)/AGPTransaction.o

MEMORYCONTROLLER = $(OBJDIR)/MemoryController.o $(OBJDIR)/MemoryTransaction.o \
                   $(OBJDIR)/MemoryControllerCommand.o

MEMORYCONTROLLER_V2 = $(OBJDIR)/MemoryControllerV2.o $(OBJDIR)/ChannelScheduler.o \
	   $(OBJDIR)/ChannelTransaction.o $(OBJDIR)/DDRBank.o $(OBJDIR)/DDRBurst.o \
	   $(OBJDIR)/DDRCommand.o $(OBJDIR)/DDRModule.o $(OBJDIR)/DDRModuleState.o \
	   $(OBJDIR)/MemoryRequest.o $(OBJDIR)/MemoryRequestSplitter.o \
	   $(OBJDIR)/MCSplitter.o $(OBJDIR)/MCSplitter2.o \
	   $(OBJDIR)/DependencyQueue.o $(OBJDIR)/SchedulerSelector.o \
	   $(OBJDIR)/FifoSchedulerBase.o \
	   $(OBJDIR)/FifoScheduler.o $(OBJDIR)/RWFifoScheduler.o \
	   $(OBJDIR)/BankQueueScheduler.o $(OBJDIR)/MemoryTraceRecorder.o

STREAMER = $(OBJDIR)/Streamer.o $(OBJDIR)/StreamerFetch.o \
	   $(OBJDIR)/StreamerOutputCache.o $(OBJDIR)/StreamerLoader.o \
	   $(OBJDIR)/StreamerCommit.o $(OBJDIR)/StreamerCommand.o \
	   $(OBJDIR)/StreamerControlCommand.o $(OBJDIR)/StreamerStateInfo.o

SHADER =  $(OBJDIR)/ShaderCommand.o $(OBJDIR)/ShaderDecodeCommand.o \
	  $(OBJDIR)/ShaderExecInstruction.o $(OBJDIR)/ShaderInput.o \
	  $(OBJDIR)/ShaderStateInfo.o $(OBJDIR)/ShaderDecodeStateInfo.o \
	  $(OBJDIR)/ConsumerStateInfo.o $(OBJDIR)/TextureUnit.o \
	  $(OBJDIR)/FilterOperation.o $(OBJDIR)/TextureRequest.o \
 	  $(OBJDIR)/TextureResult.o $(OBJDIR)/TextureUnitStateInfo.o \
	  $(OBJDIR)/ShaderFetch.o $(OBJDIR)/ShaderDecodeExecute.o
						      
PRIMITIVEASSEMBLY = $(OBJDIR)/PrimitiveAssembly.o \
		    $(OBJDIR)/PrimitiveAssemblyInput.o \
		    $(OBJDIR)/PrimitiveAssemblyCommand.o \
		    $(OBJDIR)/PrimitiveAssemblyStateInfo.o \
		    $(OBJDIR)/PrimitiveAssemblyRequest.o

CLIPPER = $(OBJDIR)/Clipper.o $(OBJDIR)/ClipperCommand.o \
	  $(OBJDIR)/ClipperStateInfo.o $(OBJDIR)/ClipperStatusInfo.o

RASTERIZER = $(OBJDIR)/RasterizerStateInfo.o $(OBJDIR)/RasterizerCommand.o \
	     $(OBJDIR)/TriangleSetup.o \
	     $(OBJDIR)/TriangleSetupInput.o $(OBJDIR)/TriangleSetupOutput.o \
	     $(OBJDIR)/TriangleSetupStateInfo.o $(OBJDIR)/TriangleTraversal.o \
	     $(OBJDIR)/TriangleSetupRequest.o $(OBJDIR)/Interpolator.o \
	     $(OBJDIR)/FragmentInput.o \
	     $(OBJDIR)/HierarchicalZ.o $(OBJDIR)/HZAccess.o \
	     $(OBJDIR)/HZUpdate.o $(OBJDIR)/HZStateInfo.o \
	     $(OBJDIR)/FFIFOStateInfo.o \
	     $(OBJDIR)/Rasterizer.o $(OBJDIR)/FragmentFIFO.o

FRAGMENTOPS = $(OBJDIR)/ColorBlockStateInfo.o $(OBJDIR)/GenericROP.o \
	      $(OBJDIR)/ROPOperation.o $(OBJDIR)/ROPStatusInfo.o \
	      $(OBJDIR)/ZStencilTestV2.o $(OBJDIR)/ColorWriteV2.o

DAC = $(OBJDIR)/DAC.o $(OBJDIR)/Blitter.o

CACHE = $(OBJDIR)/FetchCache.o $(OBJDIR)/InputCache.o \
	$(OBJDIR)/TextureCache.o $(OBJDIR)/TextureCacheL2.o \
	$(OBJDIR)/TextureCacheGen.o $(OBJDIR)/FetchCache64.o $(OBJDIR)/ROPCache.o \
	$(OBJDIR)/ColorCacheV2.o $(OBJDIR)/ZCacheV2.o

OBJECTS = $(COMMANDPROCESSOR) $(MEMORYCONTROLLER) $(MEMORYCONTROLLER_V2) $(STREAMER) \
	  $(SHADER) $(PRIMITIVEASSEMBLY) $(CLIPPER) $(RASTERIZER) $(FRAGMENTOPS) \
	  $(DAC) $(CACHE)

all : $(OBJECTS)

sim: $(OBJECTS)

sim-uni: $(OBJECTS)

$(COMMANDPROCESSOR): $(OBJDIR)/%.o : CommandProcessor/%.cpp CommandProcessor/%.h
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS)

$(MEMORYCONTROLLER): $(OBJDIR)/%.o : MemoryController/%.cpp MemoryController/%.h
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS)

$(MEMORYCONTROLLER_V2): $(OBJDIR)/%.o : MemoryControllerV2/%.cpp MemoryControllerV2/%.h
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS)

$(STREAMER): $(OBJDIR)/%.o : Streamer/%.cpp Streamer/%.h
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS)

$(SHADER): $(OBJDIR)/%.o : Shader/%.cpp Shader/%.h
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS)

$(PRIMITIVEASSEMBLY): $(OBJDIR)/%.o : PrimitiveAssembly/%.cpp PrimitiveAssembly/%.h
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS)

$(CLIPPER): $(OBJDIR)/%.o : Clipper/%.cpp Clipper/%.h
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS)

$(RASTERIZER): $(OBJDIR)/%.o : Rasterizer/%.cpp Rasterizer/%.h
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS)

$(FRAGMENTOPS): $(OBJDIR)/%.o : FragmentOperations/%.cpp FragmentOperations/%.h
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS)

$(DAC): $(OBJDIR)/%.o : DAC/%.cpp DAC/%.h
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS)
	
$(CACHE): $(OBJDIR)/%.o : Cache/%.cpp Cache/%.h
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS)

clean:
	for o in $(OBJECTS); do (rm $$o); done
