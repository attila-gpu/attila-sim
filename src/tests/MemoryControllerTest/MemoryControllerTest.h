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

#ifndef MEMORYCONTROLLERTEST_H
    #define MEMORYCONTROLLERTEST_H

/// Define globals ///
// #define MEMORY_CONTROLLER_TEST_DEBUG

#ifdef MEMORY_CONTROLLER_TEST_DEBUG
    #define MCT_DEBUG(expr) { expr }
#else
#define MCT_DEBUG(expr) {}
#endif

#endif MEMORYCONTROLLERTEST_H
