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
#  $Revision: 1.10 $
#  $Author: vmoya $
#  $Date: 2006-12-20 18:13:11 $
# 
#  GPU3D framework Makefile. 
# 
# 

include ./Makefile.defs

WHATS = all bGPU bGPU-Uni gl2atila extractTraceRegion

HOWS = debug profiling optimized verbose

TARGETS = mk-bGPU mk-bGPU-Uni mk-gl2atila mk-extractTraceRegion

ALLDIR = $(BGPUDIR) $(SUPPORTDIR) $(EMUDIR) $(GPUDIR) $(SIMDIR) \
	 $(TRACEDIR)
	 
SIMDIRS = $(BGPUDIR) $(EMUDIR) $(GPUDIR) $(SIMDIR)

TRACEDIRS = $(TRACEDIR)


default: usage

usage:
	@echo "Usage: make [ clean | simclean | traceclean ]"
	@echo "       make [what how]"
	@echo ""
	@echo "clean      - Delete all OBJs and binary files."
	@echo "simclean   - Delete only simulator related OBJs, and binary files."
	@echo "traceclean - Delete only OBJS generated from trace directory (includes lib & driver)"
	@echo ""
	@echo "what:"
	@echo "       all      - All GPU programs"
	@echo "       bGPU     - bGPU"
	@echo "       bGPU-Uni - bGPU unified shaders"
	@echo "       gl2atila - Tool for translating OpenGL traces into AGP traces"
	@echo "       extractTraceRegion - Tool for extracting frames from AGP traces"
	@echo ""
	@echo "how: "
	@echo "       debug     - Compiler debug information"
	@echo "       profiling - Compiler profiling information"
	@echo "       optimized - Maximum optimization"
	@echo "       verbose   - Full simulation trace."

$(WHATS): FORCE ; echo $@ > .what
$(HOWS): FORCE ; echo $@ > .how
	$(MAKE) mk-`cat .what` WHAT=`cat .what` HOW=`cat .how`
	rm -f .what .how
	
FORCE: ;

#######################################################################

ifeq ($(HOW),debug)
    export HOWFLAGS = $(DEBUGFLG)
endif 
ifeq ($(HOW),profiling)
    export HOWFLAGS = $(PROFFLG)
endif
ifeq ($(HOW),optimized)
    export HOWFLAGS = $(OPTFLG)
endif
ifeq ($(HOW),verbose)
    export HOWFLAGS = $(VERBFLG)
endif

#########################################################################			
		
mk-all: $(TARGETS)

mk-bGPU: mk-support mk-trace mk-emul mk-gpu mk-sim
	cd $(BGPUDIR); $(MAKE) bgpu

mk-bGPU-Uni: mk-support mk-trace mk-emul mk-gpu mk-sim-uni
	cd $(BGPUDIR); $(MAKE) bgpu-uni

mk-gl2atila: mk-support mk-trace mk-emul mk-gpu mk-sim-uni
	cd $(TRACEDIR); $(MAKE) gl2atila

mk-extractTraceRegion: mk-support mk-trace mk-emul mk-gpu mk-sim-uni
	cd $(TRACEDIR); $(MAKE) extractTraceRegion
	
mk-support:
	cd $(SUPPORTDIR); $(MAKE) all
	
mk-trace:
	cd $(TRACEDIR); $(MAKE) all

mk-emul:
	cd $(EMUDIR); $(MAKE) all
	
mk-gpu:
	cd $(GPUDIR); $(MAKE) all
	
mk-sim:
	cd $(SIMDIR); $(MAKE) sim

mk-sim-uni:
	cd $(SIMDIR); $(MAKE) sim-uni

simclean:
	for d in $(SIMDIRS); do (cd $$d; $(MAKE) clean);done

traceclean:
	for d in $(TRACEDIRS); do (cd $$d; $(MAKE) clean);done
	
clean:
	for d in $(ALLDIR); do (cd $$d; $(MAKE) $@);done
	
