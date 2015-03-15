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
#  gl2atila Makefile. 
# 
# 

TOPDIR = ../../..

include $(TOPDIR)/Makefile.defs

#########################################################################

# All source files below this subdirectories
# will be compiled automatically
SRC_DIRS = .

# Source files to be excluded from SRC_DIRS
SRC_EXCLUDE = 
	
# Extra source files to be included
SRC_INCLUDE = \
	$(BGPUDIR)/ConfigLoader.cpp

# Directories where compiler will search for includes
INC_DIRS = $(SRC_DIRS) \
	$(BGPUDIR) \
	$(SUPPORTDIR) $(EMUDIR) $(SIMDIR) \
	$(SIMDIR)/CommandProcessor $(SIMDIR)/MemoryController \
	$(SIMDIR)/MemoryControllerV2 $(SIMDIR)/Shader $(SIMDIR)/Streamer \
	$(SIMDIR)/PrimitiveAssembly $(SIMDIR)/Clipper \
	$(SIMDIR)/Rasterizer $(SIMDIR)/FragmentOperations \
	$(SIMDIR)/DAC $(SIMDIR)/Cache $(GPUDIR) $(TRACEDIR) \
	$(TRACEDIR)/GLLib $(TRACEDIR)/GPUDriver \
	$(TRACEDIR)/utils $(TRACEDIR)/utils/TraceReader \
	$(TRACEDIR)/utils/Gen $(TRACEDIR)/GLLib/ARBP \
	$(TRACEDIR)/utils/log $(TRACEDIR)/GLLib/ARBP/VertexProgram \
	$(TRACEDIR)/GLLib/ARBP/FragmentProgram $(GLDIR) \
	$(TRACEDIR)/GLLib/GLObject $(TRACEDIR)/GLLib/Texture \
	$(TRACEDIR)/GLLib/BufferObject
	
# C++ compilation flags
ifeq ($(UNIFIED),yes)
	CXXFLAGS += -D UNIFIEDSHADER
else
	CXXFLAGS += 
endif

# Library dependences
LIBS += -ltracedriver -lgl -laogl -lgpudriver -lacdx -lacd \
	-ld3dpixrunplayer -ld3ddriver -lsim -lgpu -lemul -lsupport -lz

# PROGRAM dependences
PROGRAM_DEPS = $(LIBDIR)/libtracedriver.a $(LIBDIR)/libgl.a $(LIBDIR)/libaogl.a \
               $(LIBDIR)/libgpudriver.a $(LIBDIR)/libacdx.a $(LIBDIR)/libacd.a \
               $(LIBDIR)/libd3dpixrunplayer.a $(LIBDIR)/libd3ddriver.a \
               $(LIBDIR)/libsim.a $(LIBDIR)/libgpu.a $(LIBDIR)/libemul.a \
               $(LIBDIR)/libsupport.a 

# Directories where MAKE will search for target pre-requisites
VPATH = $(SRC_DIRS)

# Generated binary
PROGRAM = gl2atila

#########################################################################

include $(TOPDIR)/Makefile.common

