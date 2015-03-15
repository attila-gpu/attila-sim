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
SRC_DIRS = .

# Source files to be excluded from SRC_DIRS
SRC_EXCLUDE = ./GPUDriver.vmoya.cpp 

# Extra source files to be included
SRC_INCLUDE = \
	$(TRACEDIR)/utils/log/LogObject.cpp \
	$(TRACEDIR)/utils/log/IncludeLog.cpp

# Directories where compiler will search for includes
INC_DIRS = $(SRC_DIRS) \
	$(TRACEDIR)/utils \
	$(SUPPORTDIR) \
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
ARCHIVE = libgpudriver.a

# Include this files in clean
CLEAN_INCLUDE=

#########################################################################

include $(TOPDIR)/Makefile.common

