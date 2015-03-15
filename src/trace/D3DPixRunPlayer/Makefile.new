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
#  $Revision: 1.1 $
#  $Author: christip $
#  $Date: 2007-09-03 14:57:47 $
# 
#  Shader Simulator classes Makefile. 
# 
# 

TOPDIR = ../../..

include $(TOPDIR)/Makefile.defs

# All source files below this subdirectories
# will be included automatically
SRC_DIRS = . 

# Source files to be excluded from SRCDIRS
SRC_EXCLUDE = 
	
# Extra source files to be included
SRC_INCLUDE = $(TRACEDIR)/utils/StringUtils.cpp

# Directories where compiler will search for includes
INC_DIRS = \
	$(SRCDIRS) \
	$(SRCDIR)/D3D \
	$(SUPPORTDIR) \
	$(TRACEDIR) \
	$(TRACEDIR)/utils \
	$(TRACEDIR)/utils/log

# C++ compilation flags
CXXFLAGS += 

# Library dependences
LIBS += 

# Directories where MAKE will search for target pre-requisites
VPATH = $(SRC_DIRS)

# Archive library to be generated
ARCHIVE = libd3dpixrunplayer.a

# Include this files in clean
CLEAN_INCLUDE =

#########################################################################

include $(TOPDIR)/Makefile.common

