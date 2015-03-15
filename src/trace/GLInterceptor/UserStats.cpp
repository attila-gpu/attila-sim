/**************************************************************************
 *
 * Copyright (c) 2002 - 2011 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 */

#include "UserStats.h"

/**
 * Definitions must be placed here*
 *
 * The objects pointed by this pointers must be created in GLInterceptor::init() method
 */

VertexStat* vcStat = 0;
TriangleStat* tcStat = 0;
InstructionCount* vshInstrCountStat = 0;
InstructionCount* fshInstrCountStat = 0;
TextureLoadsCount* texInstrCountStat = 0;
FPAndAlphaStat* fpAndAlphaStat = 0;
