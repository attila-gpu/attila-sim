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

#ifndef SCREENUTILS_H
    #define SCREENUTILS_H

#include <windows.h>

// From msdn

PBITMAPINFO createBitmapInfoStruct(HBITMAP hBmp);

void createBMPFile(LPTSTR pszFile, PBITMAPINFO pbi, 
                  HBITMAP hBMP, HDC hDC);


#endif // SCREENUTILS_H