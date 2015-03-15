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
#  $RCSfile: Makefile.new,v $
#  $Revision: 1.5 $
#  $Author: csolis $
#  $Date: 2007-11-28 17:18:28 $
# 
#  D3D Driver Makefile. 
# 
# 

TOPDIR = ../../..

include $(TOPDIR)/Makefile.defs

# All source files below this subdirectories
# will be included automatically
SRC_DIRS = . \
	   AD3D9 \
	   AD3D9/ShaderTranslator \
	   AD3D9/FFGenerator

# Source files to be excluded from SRCDIRS
SRC_EXCLUDE = ToString.cpp
	
# Extra source files to be included
SRC_INCLUDE = 

# Directories where compiler will search for includes
INC_DIRS = \
	$(SRCDIRS) \
	$(SRCDIR)/D3D \
	$(SRCDIR)/support \
        $(SRCDIR)/trace \
	$(SRCDIR)/trace/GPUDriver \
	$(SRCDIR)/trace/utils \
	$(SRCDIR)/sim \
	$(SRCDIR)/emul \
	$(SRCDIR)/sim/CommandProcessor \
	$(SRCDIR)/sim/MemoryController \
	$(SRCDIR)/trace/ACD/Interface \
	$(SRCDIR)/trace/D3DDriver \
	$(SRCDIR)/trace/D3DDriver/AD3D9 \
	$(SRCDIR)/trace/D3DDriver/AD3D9/ShaderTranslator \
	$(SRCDIR)/trace/D3DDriver/AD3D9/FFGenerator

# C++ compilation flags
CXXFLAGS += 

# Library dependences
LIBS += 

# Directories where MAKE will search for target pre-requisites
VPATH = $(SRC_DIRS)

# Archive library to be generated
ARCHIVE = libd3ddriver.a

# Include this files in clean
CLEAN_INCLUDE =

#########################################################################

include $(TOPDIR)/Makefile.common

