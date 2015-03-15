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

#ifndef ACD_MACROS
    #define ACD_MACROS

#define ACD_ASSERT_ON

#ifdef ACD_ASSERT_ON
    #define ACD_ASSERT(x) { x }
#else
    #define ACD_ASSERT(x) {}
#endif

#endif // ACD_MACROS
