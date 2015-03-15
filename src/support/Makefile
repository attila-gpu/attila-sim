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
#  $Revision: 1.9 $
#  $Author: vmoya $
#  $Date: 2008-02-22 18:32:55 $
# 
#  Support files and classes Makefile. 
# 
# 


include ../Makefile.defs

INCLUDE = -I . 
CXFLAGS = $(HOWFLAGS) $(WHEREFLAGS)
LIBS = 

OBJECTS = $(OBJDIR)/QuadFloat.o $(OBJDIR)/QuadInt.o $(OBJDIR)/support.o \
	  $(OBJDIR)/OptimizedDynamicMemory.o $(OBJDIR)/DynamicObject.o \
	  $(OBJDIR)/Parser.o 

all: $(OBJECTS)

$(OBJECTS): $(OBJDIR)/%.o : %.cpp
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS)
clean:
	@for o in $(OBJECTS); do (rm $$o); done

