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
#  $Date: 2008-02-22 18:32:44 $
# 
#  GPU simulation basic classes Makefile. 
# 
# 

include ../Makefile.defs

INCLUDE = -I ./ -I $(SUPPORTDIR)/  
CXFLAGS = $(HOWFLAGS) $(WHEREFLAGS)
LIBS = 

OBJECTS = $(OBJDIR)/GPUSignal.o $(OBJDIR)/SignalBinder.o \
          $(OBJDIR)/StatisticsManager.o $(OBJDIR)/Box.o \
          $(OBJDIR)/Statistic.o

all: $(OBJECTS)

test: $(OBJDIR)/StatisticsManager.o $(OBJDIR)/Statistic.o $(OBJDIR)/Box.o $(OBJDIR)/GPUSignal.o $(OBJDIR)/SignalBinder.o
	$(CX) $(CXFLAGS) -o  $(OBJDIR)/StatisticsManager.o $(OBJDIR)/Statistic.o $(OBJDIR)/Box.o $(OBJDIR)/GPUSignal.o $(OBJDIR)/SignalBinder.o 

$(OBJECTS): $(OBJDIR)/%.o : %.cpp
	$(CX) $(CXFLAGS) -c -o $@ $< $(INCLUDE) $(LIBS)
clean:
	for o in $(OBJECTS); do (rm $$o); done

