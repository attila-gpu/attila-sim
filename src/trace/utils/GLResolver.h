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

#ifndef GLRESOLVER_H
    #define GLRESOLVER_H

#include <vector>
#include <map>
#include <string>
#include <fstream>

enum APICall
{
   /*
    * Include all possible function indentifiers
    *
    * example:
    *
    *    APICall_glVertex2i
    *      ...
    */
    #include "APICall.gen"
    , APICall_UNDECLARED
    , APICall_SPECIAL
};

/**
 * Simple class to manage function names & ids associated to them
 */
class GLResolver
{
public:
    
    /**
     * Unknown name returns APICall_UNDECLARED
     */
    static APICall getFunctionID( const char* name );

    /**
     * Get all APICall ids of functions containing the pattern 
     *
     * 3 patterns supported, being 'token':
     * 
     * 1) *token* -> name containing the sub-token token
     * 2) token* -> name with prefix token
     * 3) *token -> name with postfix token
     *
     *
     * examples :
     *
     * 1) "wglGetProcAddress" will match with "tProc" but no with "tProc*"
     * 2) "wglGetProcAddress" will match with "wgl*" but no with "glGet*" 
     * 3) "wglGetProcAddress" will match with "*dress" but no with "*wgl"
     *
     * @note: vector returned must be destroyed by the user of this function
     */
    static std::vector<APICall>* getFunctionNameMatching( const char* match );

    /**
     * Unkown id returns NULL
     */
    static const char* getFunctionName( APICall id );

    

    /**
     * -1 if the constant does not exist
     */
    static int getConstantValue( const char* name );

    /**
     * Null if there isn't any constant with value 'value'
     */
    static const char* getConstantName( int value );

    static int countFunctions();

    static int countConstants();

private:

    struct NameToValue
    {
        const char* name;
        int value;
    };

    static NameToValue constants[];

    static const char* fnames[];

    static bool constantTableCreated;

    static std::map<std::string, int> constantTable;

    static void createConstantTable();

    //  Avoid copying
    //GLResolver();
    //GLResolver(const GLResolver&);
    //GLResolver& operator=(const GLResolver&);
};


#endif // GLRESOLVER_H
