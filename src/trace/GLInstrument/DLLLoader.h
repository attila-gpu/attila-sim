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


#ifndef DLLLOADER_H
    #define DLLLOADER_H

#include <string>

typedef void *FPointer;

/**
 * Class used to dynamically load functions contained within a dynamic library
 *
 * @code 
 *
 * // Example of use 1
 *
 * DLLLoader dll("myDll.dll");
 * if ( !dll )
 * {
 *     // open was not successful
 * }
 * 
 *
 * @endcode
 */
class DLLLoader
{
public:

    DLLLoader();
    DLLLoader(std::string path);
    ~DLLLoader();

    bool open(std::string path);

    /**
     * Checks if the library could be found (opened)
     */
    bool operator!() const;
    
    FPointer getFunction(std::string functionName);
    
private:

    struct _IRep;
    _IRep* _iRep;

};


#endif // DLLLOADER

