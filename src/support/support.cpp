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
 * $RCSfile: support.cpp,v $
 * $Revision: 1.14 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:56 $
 *
 * Support functions file. 
 *
 */


#include <iostream>
#include "support.h"
//#include "Log.h"
#ifdef WIN32
    #include <windows.h>
    #include <sstream>
    #include <direct.h>
#else
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
#endif
#include <cstdlib>
#include <cerrno>

using namespace std;

void (*panicCallback)() = 0;

void panic(const char* className, const char* fnName, const char* message)
{
    #ifdef WIN32_MESSAGES
        std::stringstream ss;
        ss << className << ":" << fnName << " => " << message;
        MessageBox(0, ss.str().c_str(), "panic", MB_OK);
    #else        
        cout << endl;
        cout << className << ":" << fnName << " => " << message << endl;
    #endif
    
    if (panicCallback != NULL)
    {    
        void (*definedPanicCallback)() = panicCallback;
        
        //  Disable panic call back inside a panic.
        panicCallback = NULL;
        
        (*definedPanicCallback)();
    }
    
    exit(-1);
}

/*void panic_func(const char *className, const char *fnName, const char *message, const char* file, int line)
//{
    //LOG(0, Log::log() << "PANIC: " << "class: " << className << " method: " << fnName << " Msg: " << message; )
    cout << "File: " << file << "  Line: " << line << "\n ";
    cout << className << ":" << fnName << " => " << message << endl;
    exit(-1);
}*/

void popup( const char* title, const char* msg )
{
    //#define SHOW_POPUPS
    #ifdef WIN32_MESSAGES
        MessageBox(0, msg, title, MB_OK);
    #else
        #ifdef SHOW_POPUPS    
            cout << title << ": " << msg << endl;
        #endif
    #endif    
}

int createDirectory(char *dirName)
{
#ifdef WIN32
    if (_mkdir(dirName) == 0)
        return 0;
    else
    {
        if (errno == EEXIST)
            return DIRECTORY_ALREADY_EXISTS;
        else
            printf("createDirectory => errno = %d\n", errno);
            return -1;
    }
#else
    if (mkdir(dirName, S_IRWXU) == 0)
        return 0;
    else
        if (errno == EEXIST)
            return DIRECTORY_ALREADY_EXISTS;
        else
            return -1;
#endif
}

int changeDirectory(char *dirName)
{
#ifdef WIN32
    return _chdir(dirName);
#else
    return chdir(dirName);
#endif
}

int getCurrentDirectory(char *dirName, int arraySize)
{
#ifdef WIN32
    if (_getcwd(dirName, arraySize) != NULL)
        return 0;
    else
        return -1;
#else
    if(getcwd(dirName, arraySize) != NULL)
        return 0;
    else
        return -1;
#endif
}


unsigned int DebugTracker::refs = 0;


DebugTracker::DebugTracker(const char* name) : name(name) 
{
    cout << "Create DEBUGTRACKER address: " << reinterpret_cast<unsigned int*>(this) << endl;
    ++refs;
}

DebugTracker::~DebugTracker() 
{ 
    --refs; 
}

void DebugTracker::__showInfo() const
{
    std::cout << "Name: " << name << ". References => " << refs << std::endl;
}


DebugTracker::DebugTracker(const DebugTracker& dt) 
{ 
    ++refs; 
}

