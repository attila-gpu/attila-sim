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
#  $Revision: 1.6 $
#  $Author: christip $
#  $Date: 2008-02-25 22:32:18 $
# 
#  GPU3D framework Makefile. 
# 
# 

TOPDIR = ..

include $(TOPDIR)/Makefile.defs

#########################################################################

TARGETS = bGPU gl2atila extractTraceRegion

SUBDIR_TARGETS = \
	support \
	trace \
	emul \
	gpu \
	sim \
	bgpu \
	$(TRACEDIR)/gl2atila \
	$(TRACEDIR)/extractTraceRegion \
	tests

CLEAN_DIRS = $(SUBDIR_TARGETS)

SIMCLEAN_DIRS = $(BGPUDIR) $(EMUDIR) $(GPUDIR) $(SIMDIR)

TRACECLEAN_DIRS = $(TRACEDIR)

#########################################################################

.PHONY: usage $(SUBDIR_TARGETS) $(TARGETS) clean simclean traceclean

usage:
	@echo "Usage: make { clean | simclean | traceclean | <target> } <options>"
	@echo ""
	@echo "clean      - Delete all OBJs and binary files."
	@echo "simclean   - Delete only simulator related OBJs, and binary files."
	@echo "traceclean - Delete only OBJS generated from trace directory (includes lib & driver)"
	@echo ""
	@echo "Available <target> are:"
	@echo ""
	@echo "       all                - All GPU programs"
	@echo "       bGPU               - bGPU"
	@echo "       gl2atila           - Tool for translating OpenGL traces into AGP traces"
	@echo "       extractTraceRegion - Tool for extracting frames from AGP traces"
	@echo ""
	@echo "Available <options> are:"
	@echo ""
	@echo "CONFIG={ debug  | profiling | optimized | verbose } "
	@echo ""
	@echo "       debug     - Compiler debug information"
	@echo "       profiling - Compiler profiling information"
	@echo "       optimized - Maximum optimization (default)"
	@echo ""
#	@echo "UNIFIED={ yes | no }"
#	@echo ""
#	@echo "       yes       - Use Unified shaders (default)"
#	@echo "       no        - Don't use Unified shaders"
	@echo ""
	@echo "CPU=<cpu>"
	@echo ""
	@echo "       <cpu>   - Target cpu for compilation. This lets apply some optimizations."
	@echo "                 Possible values: x86, x86_64, pentium4, athlon, core2"
	@echo ""
	@echo "VERBOSE={ yes | no }"
	@echo ""
	@echo "       yes       - Activate debug messages"
	@echo "       no        - Deactive debug messages (default)"
	@echo ""
	@echo "PLATFORM=<platf>"
	@echo ""
	@echo "       <platf> - Target platform for compilation."
	@echo "                 Possible values: linux (default), cygwin"	
#########################################################################

all: $(TARGETS)

bgpu: support emul gpu sim trace

bGPU: bgpu
	@$(MAKE) -C bgpu -f Makefile.new build-all

$(TRACEDIR)/gl2atila: bgpu

gl2atila: $(TRACEDIR)/gl2atila

$(TRACEDIR)/extractTraceRegion: bgpu

extractTraceRegion: $(TRACEDIR)/extractTraceRegion

$(SUBDIR_TARGETS):
	@$(MAKE) -C $@ -f Makefile.new

simclean:
	@for d in $(SIMCLEAN_DIRS); do ($(MAKE) -C $$d -f Makefile.new clean); done
	@$(MAKE) -C bgpu -f Makefile.new clean-build

traceclean:
	@for d in $(TRACECLEAN_DIRS); do ($(MAKE) -C $$d -f Makefile.new clean); done
	
clean:
	@for d in $(CLEAN_DIRS); do ($(MAKE) -C $$d -f Makefile.new clean); done
	@$(MAKE) -C bgpu -f Makefile.new clean-build

