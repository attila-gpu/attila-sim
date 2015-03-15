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
#  $Revision: 1.11 $
#  $Author: vmoya $
#  $Date: 2008-02-22 18:32:41 $
# 
#  GPU Emulation classes Makefile. 
# 
# 

include ../Makefile.defs

INCLUDE = -I . -I $(SIMDIR) -I $(SUPPORTDIR)
CXFLAGS = $(HOWFLAGS) $(WHEREFLAGS)
LIBS = 

OBJECTS = $(OBJDIR)/GPUMath.o $(OBJDIR)/ShaderInstruction.o \
          $(OBJDIR)/ShaderEmulator.o $(OBJDIR)/SetupTriangle.o \
          $(OBJDIR)/Fragment.o $(OBJDIR)/RasterizerEmulator.o \
          $(OBJDIR)/ClipperEmulator.o $(OBJDIR)/Tile.o \
          $(OBJDIR)/FragmentOpEmulator.o $(OBJDIR)/Cache.o \
          $(OBJDIR)/Cache64.o $(OBJDIR)/CacheReplacement.o \
          $(OBJDIR)/TextureEmulator.o

all: $(OBJECTS)

$(OBJECTS): $(OBJDIR)/%.o : %.cpp
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS)
clean:
	for o in $(OBJECTS); do (rm $$o); done

