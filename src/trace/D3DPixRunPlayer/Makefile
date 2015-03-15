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
#  $RCSfile: Makefile,v $
#  $Revision: 1.1 $
#  $Author: csolis $
#  $Date: 2007-08-02 17:22:04 $
# 
#  Shader Simulator classes Makefile. 
# 
# 

include ../../Makefile.defs

# All source files below this subdirectories
# will be included automatically
SRCDIRS = . 

# Source files to be excluded from SRCDIRS
SRC_EXCLUDE = 
	
# Extra source files to be included
SRC_INCLUDE = \
	$(TRACEDIR)/utils/StringUtils.cpp


# Directories where MAKE will search for target pre-requisites
VPATH = $(SRCDIRS)

# Directories where compiler will search for includes
INCDIRS = \
	$(SRCDIRS) \
	$(SRCDIR)/D3D \
	$(SUPPORTDIR) \
	$(TRACEDIR) \
	$(TRACEDIR)/utils

CPPFLAGS = $(CONFIG_FLAGS) $(WHEREFLAGS)

LIBS =  

src_found=$(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.cpp))
src_exc=$(wildcard $(SRC_EXCLUDE))
src_inc=$(wildcard $(SRC_INCLUDE))
SOURCES=$(filter-out $(src_exc),$(src_found)) $(src_inc)
obj_temp=$(notdir $(SOURCES))
OBJECTS=$(obj_temp:%.cpp=$(OBJDIR)/%.o)
INCLUDES=$(INCDIRS:%=-I %)

.PHONY: all clean
	
all: $(OBJECTS)

$(OBJDIR)/%.o : $(notdir %).cpp $(notdir %).h
	@echo "  CC $< --> $(@F)"
	@$(CX) $(CPPFLAGS) $(INCLUDES) -o $@ -c $< $(LIBS)
	
clean:
	@echo "  CLEAN $(notdir $(OBJECTS))"
	@rm -f $(OBJECTS)
