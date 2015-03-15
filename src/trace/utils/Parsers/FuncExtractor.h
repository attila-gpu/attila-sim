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

#ifndef FUNCEXTRACTOR_H
    #define FUNCEXTRACTOR_H

#include "FuncDescription.h"
#include "StringToFuncDescription.h"
#include <vector>

/**
 * Extractor and container of FuncDescription objects
 *
 *  - Support multiline function declaration ( common in large declarations )
 *  - All lines or group of lines that do not contain a function declaration
 *    are ignored ( incorrect declarations, #define directives, comments, etc... )
 *
 * The common use of this class is something like this:
 *
 * @code
 *
 *    FuncExtractor fe;
 *
 *    // Do not extract functions containing the pattern "MESA" or "SUN" in its name
 *    fe.addFilter("*MESA");
 *    fe.addFilter("*SUN");
 *    
 *    // Extract functions ( number of extracted functions is returned )
 *    in nFuncs = fe.extractFunctions("gl.h");
 *    if ( nFuncs < 0 )
 *        panic("File not found!");
 *    // After this call all (not filtered) functions in "gl.h" have a FuncDescription stored in
 *    // the FuncExtractor object
 *
 *    // Print all function declarations contained in 'fe'
 *    for ( i = 0; i < nFuncs; i++ )
 *        cout << fe.getFunction(i)->toString() << endl;
 *
 *    fe.clearFilters(); // do not filter anything (remove all filters)
 *
 *    // add more declarations...
 *    fe.extractFunctions("anotherDeclarationFile.h");
 *
 *    // Print all functions declarations in "gl.h" (except filtered) and all in "anotherDeclarationFile.h"
 *    for ( i = 0; i < fe.count(); i++ )
 *        cout << fe.getFunction(i)->ToString() << endl;
 *
 *    // note that functions are sorted lexicographically (by name)
 *
 * @endcode
 *
 * @note This container has an important property:
 *       Given a FuncExtractor fe, fe.getFunction(i) <= fe.getFunction(i+1) with i >= 0
 *       being <= a lexicographic operator.
 *       
 * @version 1.0
 * @date 10/11/2003
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es
 */
class FuncExtractor
{

private:

    std::vector<const char*> filter; ///< list of filters ( std::vector type will be removed soon )

    StringToFuncDescription nameToFunc; ///< Mapping between names and FuncDescription objects

    bool matchWithFilter( const char* name ); ///< Check filter matching

    bool hasReservedWords( const char* stream ); ///< Check reserved words
    
public:

    /**
     * Creates a FuncExtractor object
     */
    FuncExtractor();
    
    /**
     * Extract "all" function declaration in a file
     *
     * All functions are extracted unless a function matches with some filter specified via
     * addFilter() method, if matches it is discarded
     *
     * @param file file name from where declaration will be extracted
     *
     * @return number of extracted functions
     */ 
    int extractFunctions( const char* file );

    /**
     * Adds new filter
     *
     * Filter expresions are simple "[TEXT]" or "*[TEXT]", with [TEXT] being a match text.
     * If * is not specified, then the match must be the full word. If * is specified a
     * partially match is enough to achive a match
     *
     * @code
     *
     *     // "foo(...)" matches but foo1(...) does not match
     *     fe.addFilter("foo"); // filters all functions with the name "foo"
     *
     *     // "bar2(...)", "some_bar(...)" and "onebarFunction(...)" all match
     *     fe.addFilter("*bar"); // filters all functions containing the pattern "bar"
     *
     * @endcode
     *
     * @param aFilter a filter
     * 
     */ 
    void addFilter( const char* aFilter );

    /**
     * Removes a filter
     *
     * @param aFilter filter that will be removed ( if exists )
     */
    void removeFilter( const char* aFilter );

    /** 
     * Remove all filters
     */
    void clearFilters();

    /*
     * Apply current filters to FuncEXtractor object ( filters are not autoapplied )
     *
     * @return the number of functions removed using the current filter
     */
    int applyFilter();    

    /** 
     * Gets the function given its name
     *
     * @param name function
     * @return FuncDescription pointer to FuncDescription object with name 'name'
     */
    const FuncDescription* getFunction( const char* name ) const;

    /*
     * Gets the function in a given position
     *
     * Because of getFunction(i) is lexicographically lower than getFunction(i+1) for i >= 0
     * it is possible to obtain all functions sorted (by name) with the next code:
     * @code
     *     // fe is a FuncExtractor object
     *     // print all function names ( sorted )
     *     for ( int i = 0; i < fe.count(); i++ )
     *         cout << fe.getFunction(i)->getName() << endl;
     * @endcode
     *
     * @param pos FuncDescription position
     * @return FuncDescription pointer to FuncDescription object in position 'pos'
     */
    const FuncDescription* getFunction( int pos ) const;

    /**
     * Number of FuncDescription objects inside the FuncExtractor
     *
     * @return number of FuncDescription objects
     */
    int count() const;

    /**
     * Debug
     */
    void dump() const;    
};

#endif // FUNCEXTRACTOR_H
