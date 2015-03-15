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

/**************************************************
Code for write a bitmap

Author: MSDN
**************************************************/

#ifndef __WRITE_BMP
#define __WRITE_BMP

PBITMAPINFO createBitmapInfoStruct(HBITMAP hBmp);

void createBMPFile(LPTSTR pszFile, PBITMAPINFO pbi, 
                  HBITMAP hBMP, HDC hDC);

#endif