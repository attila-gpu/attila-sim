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
#  GPU3D framework top Makefile. 
# 
# 

TOPDIR = .

include $(TOPDIR)/Makefile.defs

#########################################################################

# "bGPU" and "bgpu" are the same target, but the last is easier to type.

TARGETS = usage all bGPU bgpu gl2atila extractTraceRegion tests clean simclean traceclean

.PHONY: $(TARGETS)

$(TARGETS):
	@$(TIME) $(MAKE) -C $(SRCDIR) -f Makefile.new $@
