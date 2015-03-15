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
# 
#  GL trace Makefile
# 

TOPDIR = ../../..

include $(TOPDIR)/Makefile.defs

#########################################################################
 
# All source files below this subdirectories
# will be compiled automatically
SRC_DIRS = . \
	GLObject \
	Texture \
	BufferObject \
	ARBP \
	ARBP/VertexProgram \
	ARBP/FragmentProgram

# Source files to be excluded from SRC_DIRS
SRC_EXCLUDE = 

# Extra source files to be included
SRC_INCLUDE = 

# Directories where compiler will search for includes
INC_DIRS = $(SRC_DIRS) \
	$(TRACEDIR)/GPUDriver \
	$(TRACEDIR)/utils \
	$(TRACEDIR)/utils/GL \
	$(TRACEDIR)/utils/Gen \
	$(SUPPORTDIR) \
	$(SIMDIR) \
	$(SIMDIR)/CommandProcessor \
	$(SIMDIR)/MemoryController \
	$(SIMDIR)/Shader \
	$(EMUDIR)	
#	AOGL/ARBProgramObject \
#	ACD/Interface ACDX/Interface
	
# C++ compilation flags
CXXFLAGS += 

# Library dependences
LIBS += 

# Directories where MAKE will search for target pre-requisites
VPATH = $(SRC_DIRS)

# Archive library to be generated
ARCHIVE = libgl.a

# Include this files in clean
CLEAN_INCLUDE=ARBP/VertexProgram/*.gen ARBP/FragmentProgram/*.gen

#########################################################################

include $(TOPDIR)/Makefile.common

#########################################################################

ARBP/VertexProgram/VP1ExecEnvironment.o: \
	ARBP/VertexProgram/VP1Flex.gen ARBP/VertexProgram/VP1Bison.gen

ARBP/FragmentProgram/FP1ExecEnvironment.o: \
	ARBP/FragmentProgram/FP1Flex.gen ARBP/FragmentProgram/FP1Bison.gen

ARBP/VertexProgram/VP1Flex.gen: ARBP/VertexProgram/VP1Lexic.l

ARBP/VertexProgram/VP1Bison.gen: ARBP/VertexProgram/VP1Grammar.y

ARBP/FragmentProgram/FP1Flex.gen: ARBP/FragmentProgram/FP1Lexic.l

ARBP/FragmentProgram/FP1Bison.gen: ARBP/FragmentProgram/FP1Grammar.y


%Flex.gen: %Lexic.l
	@echo "  FLEX $< --> $@"
	@$(FLEX) -P$(shell echo $(notdir $*) | tr "[:upper:]" "[:lower:]") -t -8 $< > $@
	
%Bison.gen: %Grammar.y
	@echo "  BISON $< --> $@"
	@$(BISON) -p $(shell echo $(notdir $*) | tr "[:upper:]" "[:lower:]") -t $< -o $@

