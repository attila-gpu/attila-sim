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
#  Code generator Makefile
# 

TOPDIR = ../../..

include $(TOPDIR)/Makefile.defs

#########################################################################
 
# All source files below this subdirectories
# will be compiled automatically
SRC_DIRS = . \
	$(TRACEDIR)/utils/Parsers

# Source files to be excluded from SRC_DIRS
SRC_EXCLUDE = 
	
# Extra source files to be included
SRC_INCLUDE = 

# Directories where compiler will search for includes
INC_DIRS = $(SRC_DIRS) $(SUPPORTDIR)
	
# C++ compilation flags
CXXFLAGS += 

# Library dependences
LIBS += -lsupport

# Directories where MAKE will search for target pre-requisites
VPATH = $(SRC_DIRS)

# Program to generate without path processing
PROGRAM = codegenerator

# Internal library dependences
#PROGRAM_LIBS = libsupport.a

# Generation file
GENFILE = $(TRACEDIR)/utils/Gen/APICall.gen

# Include this files in clean
CLEAN_INCLUDE = $(TRACEDIR)/utils/Gen/*.gen

# Target to be processed
TARGETS = $(GENFILE)

#########################################################################
	
include $(TOPDIR)/Makefile.common

$(GENFILE): $(BINDIR)/$(PROGRAM) CGconfig.ini $(TRACEDIR)/utils/GL/OGL_functions_supported_by_bGPU.h
	@echo "  CODEGEN $@"
	@$(BINDIR)/$(PROGRAM)

