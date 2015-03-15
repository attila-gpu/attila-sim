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

#ifndef SPECIFICEXTRACTOR_H
    #define SPECIFICEXTRACTOR_H

#include "FuncDescription.h"

#define SPECIFIC_SUFIX "_SPECIFIC"

/**
 * This class implements a container for storing specific function options
 *
 * @version 1.0
 * @date 21/01/2004
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es 
 */
class SpecificItem
{
private:

    enum // internal constants
    { 
        MAX_PARAMS = 16, ///< Max parameters in this specific function
        MAX_CALLS = 16  ///< Max number of calls for this specific function
    };

    const FuncDescription* fd; ///< Specific Function description (contains all relevant information)
    
    /**
     * bitmap containing if a parameter must be logged or not (true means do not log)
     *
     * 0 position is associated to parameter1, 1 to parameter 2, etc
     */
    bool disabledParams[MAX_PARAMS]; 

    /**
     * List of calls to specific function
     *
     * listOfCall[i] with {0<=i<howManyCalls} indicates the positions of the diferent calls that
     * will be done ( position means after a parameter, position 0 means before all parameters ).
     * Parameters are numbered from 1 to total number of formal parameters
     *
     * This array is always sorted and cannot contain repeated values or negative ones
     */
    int listOfCalls[MAX_CALLS]; // sorted    

    /**
     * Indicates that a specific call must be done after log return ( if exist return )
     */
    bool callAfterReturn;

    /**
     * Indicates if return must be logged or not
     */
    bool logReturnFlag;

    int howManyCalls; ///< how many items in listOfCalls

    /**
     * If true indicates that the original call to openGl function must be replaced by
     * the specific version. By default is false
     */ 
    bool replaceFlag;

    /**
     * //<OCS> only call specific
     * 
     * True indicates : do not generate code for logging function, parameters are return values
     * generate only code for calling specific and return the value ( if required )
     *
     * false: do normal behaviour
     */
    bool ocs;

public:

    /**
     * Creates an empty Specific item without any information
     */
    SpecificItem();

    /**
     * Sets "only call specific"
     *
     * @param ocsFlag new value ( true / false )
     */
    void setOCSFlag( bool ocsFlag );

    /**
     * Gets the current value of OCS flag
     *
     * @return true if the "only call specific" flag is enabled, false otherwise
     */
    bool getOCSFlag() const;

    /**
     * Sets the replace flag with a new value
     *
     * @param replaceFlag true enables the replaceFlag, false disables it
     */
    void setReplaceFlag( bool replaceFlag );

    /**
     * Gets the current Replace Flag
     *
     * @return the current Replace Flag value
     */
    bool getReplaceFlag() const;

    /**
     * Sets the FuncDescriptor object containing all relevant information about the specific
     * function: name, parameters, types of parameters, etc...
     *
     * @param fd specific function information
     */
    void setFunction( const FuncDescription* fd );

    /**
     * Gets the specific function information ( the associated FuncDescription object )
     *
     * @return a pointer to specific function information
     */
    const FuncDescription* getFunction() const;
    
    /**
     * Sets if a parameter must be logged or not
     *
     * @param pos parameter position
     * @param disabled flag indicationg if the parameter will be disabled ( true ) or not ( false )
     */
    void setParam( int pos, bool disabled );
    
    /**
     * Gets a parameter state ( if it must be logged or not )
     *
     * @param pos parameter position
     * @return true if must not be logged, false otherwise
     */ 
    bool getParam( int pos ) const;
    
    /**
     * Returns the number of specific calls that has been added until now
     *
     * @return number of specific calls added
     */
    int howManySpecificCalls() const;

    /**
     * Adds the information describing that a new specific call must be done in a given position
     *
     * @param afterParam position where the specific call must be done
     *
     * 0 means after any parameter, other numbers mean after a parameter, for example,
     * if 'afterParam' holds the value 2 the specific call must be placed after logging 
     * parameter 2
     *
     * @see isSpecificCallRequired
     */
    void addSpecificCall( int afterParam );
    
    /**
     * Checks if a position must have a specific function call
     *
     * @param afterParam position that is being tested
     *
     * @see addSpecificCall
     */
    bool isSpecificCallRequired( int afterParam ) const;

    /**
     * Adds the information describing that a new specific call must be done after log
     * return value
     */
    void addSpecificCallAfterReturn();

    /**
     * Cheks if a specific function call must be called after log return value
     */
    bool isSpecificCallAfterReturnRequired() const;

    /**
     * Checks if return is logged or not
     *
     * @return true if return will be logged, false otherwise
     */
    bool logReturn() const;

    /**
     * Sets if return will be logged or not
     *
     * @param value true if return will be logged, false if we do not want return to be logged
     * 
     */
    void setLogReturn( bool value );

    /**
     * Debug
     */ 
    void dump() const;
};

/**
 *
 * This class parses a file containing specific function declarations plus a set of
 * options and store all the information
 *
 * A specific function is a function that can be called by a code generator in a given 
 * position and any number of times and implements a specific behaviour implemented
 * by the programmer
 *
 * This class can be used for specifying options in a code generator using the mecanism
 * of call a function in a specific place and disabling some parameter logging
 *
 * @see CodeGenerator ( not yet full implemented )
 *
 * The sintax for options is quite simple, function options are expressed before the function
 * is declared
 *
 * Example of a specific function with options
 *
 * @code
 *     //<NL=1,2> do not log 1st and 2nd parameter ( do not log any parameter )
 *     //<A=0> call specific function once, before log any param
 *     int ChoosePixelFormat_SPECIFIC( HDC _p0 ,const PIXELFORMATDESCRIPTOR *_p1 );
 * @endcode
 *
 * @note The options are associated to the first function declared after options
 *
 * The possible options (using tags) in this current version are:
 * 
 *   - Tag NOT LOG ( do not log the specified parameters )
 *       - Examples:
 *           - //<NL=1,3,7> do not log parameters 1, 3 and 7 ( first parameter is 1, not 0 )
 *           - //<NL=2> do not log parameter 2
 *   - Tag AFTER ( indicates that a call must be placed after a parameter is logged )
 *       - Examples:
 *           - //<A=2> make a call after log parameter 2
 *           - //<A=0,4> make a call before log any parameter and another after log parameter 4
 *
 * @note text after '>' is not processed ( it is ignored )
 *
 * @version 1.0
 * @date 21/01/2004
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es 
 */
class SpecificExtractor
{
private:

    enum { MAX_ITEMS = 1000 }; ///< Max Specific items supported

    SpecificItem* items[MAX_ITEMS]; ///< Specific items extracted

    int iItem; ///< Number o specific items extracted

    /**
     * Auxiliar function for parsing a list of integers
     *
     * The expected list has the form of: "v1,v2,...,vn>"
     *
     * @param buffer stream containing the list in text format
     * @param values output array with the list values parsed to integers
     * @maxValues maximum number of values that 'values' array can hold
     *
     * @return the number of values parsed if the list could be parsed, -1 otherwise ( 'r' not included )
     * @retval ret true if return is specified in list of values
     * @note Return is expressed using 'R' or 'r' char
     */
    int parseValues( const char* buffer, int values[], int maxValues, bool& ret);

public:

    /**
     * Creates a SpecificExtractor object
     */
    SpecificExtractor();

    /**
     * Extract all declarations and options from a given file
     *
     * @param file name
     * @return number of declarations ( declarations with its options ) extracted,
     *         if some problem occurs returns -1
     */
    int extract( const char* specificFileName );

    /**
     * Gets a specific function given its corresponding original name
     *
     *
     * @code
     *     // Example finding the specific function for a given function name
     *
     *     FuncDescription* sfd = se.getSpecificFunction("glVertex3f");
     *
     *     if ( sfd != NULL )
     *     {
     *         // specific function exist
     *         cout << sfd->getName() << endl;
     *         // it should be printed: "glVertex3f_SPECIFIC"
     *     }
     *     else
     *     {
     *         cout << "NO specific function found" << endl;
     *     }
     * @endcode
     *
     * @param originalFunctionName original function name
     * @return the FuncDescription describing the specific function associated with
     *         the function with name originalFunctionName
     *
     */
    SpecificItem* getSpecificFunction( const char* originalFunctionName ) const;

    /**
     * Number of specific functions in the extractor
     *
     * @return number of specific functions
     */
    int count() const;

    /**
     * debug
     */
    void dump() const;
};

#endif // SPECIFICEXTRACTOR_H
