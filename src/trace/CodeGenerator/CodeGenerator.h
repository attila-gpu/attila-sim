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

#ifndef CODEGENERATOR_H
    #define CODEGENERATOR_H


#include "FuncExtractor.h"
#include "SpecificExtractor.h"

#include <iostream>
#include <fstream>

class CodeGenerator
{

private:

    bool isNull;
    char oDir[256];

protected:

    static void writePreface( std::ostream& f );

    /* Allow creation of outfile files in the current oDir */
    /* It check if file is correctly opened */
    bool openOFile( std::ofstream& f, const char* fileName );

    const char* getOutputDir() const;

    void setOutputDir( const char* path );


public:

    CodeGenerator();
    
    virtual bool generateCode() = 0;

    virtual void dumpInfo() const;
    
    virtual ~CodeGenerator();

};

/**
 * Common interface for function-like generators ( Stubs & Wrappers )
 */
class FuncGenerator : public CodeGenerator
{
private:

    /**
     * Array containing "types" known by code generator
     */
    static const char* knownTypes[];    

protected:
    
    /* Allow to generate a list of function declarations */
    bool generateFuncDeclarations( std::ostream& f, const FuncExtractor& fe );

    bool generateFuncCall( std::ostream& f, const char* fName, const char* postfix, char paramNames[20][64], int nParams );

    bool generateFuncCall( std::ostream& f, std::string fName, std::string postfix, const std::vector<std::string>& paramNames, int nParams);

    /**
     * Check if type is contained in knownTypes array
     *
     * @param type a type
     *
     * @return true if it is a type known by code generator, false otherwise
     */
    static bool isKnownType( const char* type );    

    static void createCast( ParamDescription* pd, char buffer[] );

    ~FuncGenerator()=0;

};

#endif // CODEGENERATOR_H
