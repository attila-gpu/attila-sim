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
#  ACDX Makefile
# 

TOPDIR = ../../..

include $(TOPDIR)/Makefile.defs

#########################################################################
 
# All source files below this subdirectories
# will be compiled automatically
SRC_DIRS = \
	Implementation \
	Implementation/FPEmulation \
	Implementation/ARBCompilers \
	Implementation/ARBCompilers/VertexProgram \
	Implementation/ARBCompilers/FragmentProgram

# Source files to be excluded from SRC_DIRS
SRC_EXCLUDE = 
	
# Extra source files to be included
SRC_INCLUDE = 

# Directories where compiler will search for includes
INC_DIRS = $(SRC_DIRS) \
	$(TRACEDIR)/ACD/Interface \
	$(TRACEDIR)/ACD/Implementation \
	Interface \
	$(TRACEDIR)/GPUDriver \
	$(SUPPORTDIR) \
	$(TRACEDIR)/utils \
	$(TRACEDIR)/utils/GL \
	$(SIMDIR) \
	$(SIMDIR)/CommandProcessor \
	$(SIMDIR)/MemoryController \
	$(EMUDIR)

# C++ compilation flags
CXXFLAGS += 

# Library dependences
LIBS += 

# Directories where MAKE will search for target pre-requisites
VPATH = $(SRC_DIRS)

# Archive library to be generated
ARCHIVE = libacdx.a

# Include this files in clean
CLEAN_INCLUDE= \
	Implementation/ARBCompilers/VertexProgram/*.gen \
	Implementation/ARBCompilers/FragmentProgram/*.gen

#########################################################################

include $(TOPDIR)/Makefile.common

#########################################################################

Implementation/ARBCompilers/VertexProgram/ACDXVP1ExecEnvironment.o: \
	Implementation/ARBCompilers/VertexProgram/ACDXVP1Flex.gen \
	Implementation/ARBCompilers/VertexProgram/ACDXVP1Bison.gen

Implementation/ARBCompilers/FragmentProgram/ACDXFP1ExecEnvironment.o: \
	Implementation/ARBCompilers/FragmentProgram/ACDXFP1Flex.gen \
	Implementation/ARBCompilers/FragmentProgram/ACDXFP1Bison.gen

Implementation/ARBCompilers/VertexProgram/ACDXVP1Flex.gen: \
	Implementation/ARBCompilers/VertexProgram/ACDXVP1Lexic.l

Implementation/ARBCompilers/VertexProgram/ACDXVP1Bison.gen: \
	Implementation/ARBCompilers/VertexProgram/ACDXVP1Grammar.y

Implementation/ARBCompilers/FragmentProgram/ACDXFP1Flex.gen: \
	Implementation/ARBCompilers/FragmentProgram/ACDXFP1Lexic.l

Implementation/ARBCompilers/FragmentProgram/ACDXFP1Bison.gen: \
	Implementation/ARBCompilers/FragmentProgram/ACDXFP1Grammar.y


%Flex.gen: %Lexic.l
	@echo "  FLEX $< --> $@"
	@$(FLEX) -P$(shell echo $(notdir $*) | tr "[:upper:]" "[:lower:]") -t -8 $< > $@
	
%Bison.gen: %Grammar.y
	@echo "  BISON $< --> $@"
	@$(BISON) -p $(shell echo $(notdir $*) | tr "[:upper:]" "[:lower:]") -t $< -o $@

