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

#include "support.h"
#include "GLJumpTable.h"
#include "GLResolver.h"

// by default uses system "opengl32.dll" library for loading the jump table
// Define LOAD_JUMPTABLE_STATICALLY to force static load
// see JumpTableSL.gen file


#if defined WIN32 || defined WIN64 // MS-Windows systems
    
    #include <windows.h>

    void *loadDynamicLibrary( const char* dllPath )
    {
        HMODULE handle = LoadLibrary(dllPath);
        return (void *)handle;
    }

    void *getFunction( void* handle, const char* name )
    {
        return (void *)GetProcAddress((HMODULE)handle, name);
    }

#else // Assume LINUX/UNIX/Others

    //#include <dlfcn.h>

    //void *loadDynamicLibrary( const char* name )
    //{
    //    void *handle;
    //    handle = dlopen(name, RTLD_LAZY | RTLD_LOCAL);
    //    return handle;
    //}

    //void *getFunction( void* handle, const char* name )
    //{
    //    return dlsym(handle, name);
    //}

#endif


bool loadGLJumpTable( GLJumpTable& jt, const char* dllPath )
{

#ifndef LOAD_JUMPTABLE_STATICALLY
    void *handle = loadDynamicLibrary(dllPath);
    
    if ( !handle ) /* Error loading library */
        panic("", "loadGLJumpTable()","Error getting handle to Dinamic Library");

    int i;
    unsigned int func;    

    unsigned int* ptr = (unsigned int*)&jt;

    for ( i = 0; i < GLResolver::countFunctions(); i++ )
    {        
        func = (unsigned int)getFunction(handle, GLResolver::getFunctionName((APICall)i));
        if ( func ) /* function found */
            ptr[i] = func;
    }

    
    return true;

#else

    // STATICALLY LOAD

    //_this text generate an error _if #_else is selected.

    
#define _JT jt // rename names

    /**
     * Code added (example)
     *
     * ...
     * _JT.glBegin = glBegin;
     *
     * ...
     *
     * _JT.glVertex3f = glVertex3f;
     * ...
     *
     */
    #include "GLJumpTableSL.gen" // SL means: Static Link
    
    return true;

#endif

}
