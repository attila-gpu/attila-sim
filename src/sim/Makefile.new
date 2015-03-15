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
#  Shader Simulator classes Makefile. 
# 
# 

TOPDIR = ../..

include $(TOPDIR)/Makefile.defs

#########################################################################

# All source files below this subdirectories
# will be compiled automatically
SRC_DIRS = . \
	Cache \
	Clipper \
	CommandProcessor \
	DAC \
	FragmentOperations \
	MemoryController \
	MemoryControllerV2 \
	PrimitiveAssembly \
	Rasterizer \
        MicroPolygonRasterizer \
	Shader \
	VectorShader \
	Streamer

# Source files to be excluded from SRC_DIRS
SRC_EXCLUDE = \
	Cache/ColorCache.cpp Cache/ZCache.cpp Cache/ColorCache.old.cpp \
	FragmentOperations/*.cpp \
	Rasterizer/RecursiveDescent.cpp Rasterizer/RecursiveRasterizer.cpp \
	Rasterizer/TileEvaluator.cpp Rasterizer/TileEvaluatorStateInfo.cpp Rasterizer/TileInput.cpp \
	Rasterizer/FragmentFIFO-nonUnified-legacy.cpp Rasterizer/Rasterizer-nonUnified-legacy.cpp \
	Shader/ShaderDecodeExecute-nonUnified-legacy.cpp Shader/ShaderDecodeExecute-perGroupDecode.cpp \
	Shader/ShaderFetch-nonUnified-legacy.cpp Shader/TextureUnit-in-order.cpp \
	Shader/TextureUnit-out-of-order.cpp

# Extra source files to be included
SRC_INCLUDE = \
	FragmentOperations/ColorBlockStateInfo.cpp \
	FragmentOperations/GenericROP.cpp \
	FragmentOperations/ROPOperation.cpp \
	FragmentOperations/ROPStatusInfo.cpp \
	FragmentOperations/ZStencilTestV2.cpp \
	FragmentOperations/ColorWriteV2.cpp

# Directories where compiler will search for includes
INC_DIRS = \
	$(SRC_DIRS) \
	$(SUPPORTDIR) $(EMUDIR) $(GPUDIR) $(TRACEDIR) \
	$(TRACEDIR)/GLLib $(TRACEDIR)/GPUDriver \
	$(TRACEDIR)/GLLib/GLObject \
	$(TRACEDIR)/utils $(TRACEDIR)/utils/TraceReader \
	$(TRACEDIR)/utils/log \
	$(TRACEDIR)/utils/Gen $(GLDIR) \
	$(BGPUDIR) 

# C++ compilation flags
CXXFLAGS +=

# Library dependences
LIBS +=

# Directories where MAKE will search for target pre-requisites
VPATH = $(SRC_DIRS)

# Archive library to be generated
ARCHIVE = libsim.a

#########################################################################

include $(TOPDIR)/Makefile.common

