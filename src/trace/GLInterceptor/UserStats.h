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

#ifndef USERSTATS_H
    #define USERSTATS_H

/**
 * About this file
 *
 * This file contains the GLIStat objects pointers used to access directly
 * to each statistic stored within GLIStatisticsManager
 *
 * This GLIStats objects are created in GLInterceptor::init() method
 *
 * @note This file is included by GLInterceptor.cpp and SpecificStats.cpp
 */


/************************************************************
 * Put here the include files declaring the user statistics *
 ************************************************************/
#include "BasicStats.h"
#include "ShaderStats.h"
#include "CheckStats.h"


/***********************************************************************
 * Declare a pointer here (extern) for each user statistic of interest *
 * They must be initialized at GLInterceptor::init() method            *
 *                                                                     *
 * @note definition must be placed in UserStats.cpp                    *  
 ***********************************************************************/
extern VertexStat* vcStat;
extern TriangleStat* tcStat;
extern InstructionCount* vshInstrCountStat;
extern InstructionCount* fshInstrCountStat;
extern TextureLoadsCount* texInstrCountStat;
extern FPAndAlphaStat* fpAndAlphaStat;



#endif // USERSTATS_H
