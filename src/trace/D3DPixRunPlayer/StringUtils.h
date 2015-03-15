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

/*****************************************************************************
String parsing utilities

Author: Jos� Manuel Sol�s
*****************************************************************************/

#ifndef __STRING_UTILS
#define __STRING_UTILS

std::vector< std::string > explode(std::string s, const std::string& by);
std::vector< std::string > explode_by_any(std::string s, const std::string& characters);
std::string trim(std::string& s,const std::string& drop = " \t");

#endif
