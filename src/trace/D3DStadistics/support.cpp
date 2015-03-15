/**************************************************************************
 *
 * Copyright (c) 2002, 2003 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 * $RCSfile: support.cpp,v $
 * $Revision: 1.1 $
 * $Author: csolis $
 * $Date: 2006-12-06 18:31:50 $
 *
 * Support functions file. 
 *
 */


#include <iostream>
#include "support.h"

using namespace std;

void panic(const char* className, const char* fName, const char* message)
{
    panic_func(className,fName,message,__FILE__,__LINE__);
}

void panic_func(const char *className, const char *fnName, const char *message, const char* file, int line)
{
    cout << "File: " << file << "  Line: " << line << "\n ";
	cout << className << ":" << fnName << " => " << message << endl;
	exit(-1);
}

void popup( const char* title, const char* msg )
{
    //#define SHOW_POPUPS
    #ifdef SHOW_POPUPS    
    cout << title << ": " << msg << endl;
    #endif
}
