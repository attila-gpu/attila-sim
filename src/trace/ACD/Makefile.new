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
#  ACD Makefile
# 

TOPDIR = ../../..

include $(TOPDIR)/Makefile.defs

#########################################################################
 
# All source files below this subdirectories
# will be compiled automatically
SRC_DIRS = \
	Implementation \
	Implementation/ShaderOptimization

# Source files to be excluded from SRC_DIRS
SRC_EXCLUDE = 
	
# Extra source files to be included
SRC_INCLUDE = 

# Directories where compiler will search for includes
INC_DIRS = $(SRC_DIRS) \
	./Interface \
	$(TRACEDIR)/GPUDriver \
	$(SUPPORTDIR) \
	$(TRACEDIR)/utils \
	$(TRACEDIR)/utils/GL \
	$(SIMDIR) \
	$(SIMDIR)/CommandProcessor \
	$(SIMDIR)/MemoryController \
	$(SIMDIR)/Shader \
	$(EMUDIR)
#	ACDX/Interface \

# C++ compilation flags
CXXFLAGS += 

# Library dependences
LIBS += 

# Directories where MAKE will search for target pre-requisites
VPATH = $(SRC_DIRS)

# Archive library to be generated
ARCHIVE = libacd.a

# Include this files in clean
CLEAN_INCLUDE=

#########################################################################

include $(TOPDIR)/Makefile.common

