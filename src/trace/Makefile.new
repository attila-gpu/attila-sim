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
#  Trace Makefile
# 

TOPDIR = ../..

include $(TOPDIR)/Makefile.defs

#########################################################################

# All source files below this subdirectories
# will be compiled automatically
SRC_DIRS = . \
	utils/TraceReader \
	utils

# Source files to be excluded from SRC_DIRS
SRC_EXCLUDE = ./*.cpp \
	utils/UserCallTable.cpp \
	utils/StringUtils.cpp

# Extra source files to be included
SRC_INCLUDE = \
	AGPTraceDriver.cpp \
	GLTraceDriver.cpp \
	D3DTraceDriver.cpp \
	RegisterWriteBufferAGP.cpp 

# Directories where compiler will search for includes
INC_DIRS = $(SRC_DIRS) \
	$(SUPPORTDIR) utils/Gen utils/GL utils/log \
	$(SIMDIR) $(SIMDIR)/CommandProcessor $(SIMDIR)/MemoryController \
	GPUDriver GLLib GLLib/ARBP GLLib/GLObject GLLib/Texture GLLib/BufferObject \
	AOGL AOGL/ARBProgramObject AOGL/AGLBaseObject AOGL/AGLBufferObject AOGL/AGLTextureObject\
	ACD/Interface ACDX/Interface

# C++ compilation flags
CXXFLAGS += -D LOAD_JUMPTABLE_STATICALLY -D GL_GLEXT_LEGACY

# Library dependences
LIBS += 

# Directories where MAKE will search for target pre-requisites
VPATH = $(SRC_DIRS)

# Archive library to be generated
ARCHIVE = libtracedriver.a

# Include this files in clean
CLEAN_INCLUDE =

SUBDIR_TARGETS = CodeGenerator GPUDriver GLLib AOGL ACD ACDX D3DDriver D3DPixRunPlayer

TARGETS = bGPU

.PHONY: $(TARGETS) $(SUBDIR_TARGETS) gl2atila extractTraceRegion tracedriver GL D3D

CODEGEN_TRIGGERS = \
	$(TRACEDIR)/utils/Gen/*.gen \
	CodeGenerator/CGconfig.ini

#########################################################################

include $(TOPDIR)/Makefile.common

#########################################################################

$(CODEGEN_TRIGGERS) : CodeGenerator

$(OBJECTS): $(CODEGEN_TRIGGERS)

tracedriver: $(LIBDIR)/$(ARCHIVE)

GL: GPUDriver GLLib AOGL

D3D: D3DDriver D3DPixRunPlayer

bGPU: GL ACD ACDX D3D tracedriver

gl2atila: GL ACD ACDX D3D tracedriver

extractTraceRegion: GL tracedriver
