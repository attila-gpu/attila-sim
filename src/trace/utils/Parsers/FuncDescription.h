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

#ifndef FUNCDESCRIPTION_H
    #define FUNCDESCRIPTION_H

#include "ParamDescription.h"

/**
 * Object representing a function declaration
 *
 * FuncDescription class models the concept of a Standard C function declaration,
 * It also parses function declarations included in the standard OpenGL header 
 * files (i.e gl.h). Those functions contain MACROS like GLAPI or APIENTRY.
 *
 * The expected format for a "parseable" function is:
 *
 *    // OpenGL declaration
 *    GLAPI returningType GLAPYENTRY funcName ( listOfParameters );
 *    or
 *    // Another type of OpenGL declaration
 *    GLAPI returningType APIENTRY funcName( listOfParamaters );
 *
 *    Also supported ( standard C ):
 *
 *    // Standard C declaration ( without MACROS )
 *    returningType funcName( listOfParameters );
 *
 * Where listOfParameters can contain only list of types or names and types
 * for the parameters
 *
 * Definition examples:
 *
 * @code
 * // Both correct definitions ( parse correctly )
 *
 *    // definition with names and types
 *    GLAPI void GLAPIENTRY glVertex3d( GLdouble x, GLdouble y, GLdouble z );
 *
 *    // definition with only the list of types
 *    GLAPI void APIENTRY glDeleteProgramsARB (GLsizei, const GLuint *);
 * @endcode
 *
 * The common use of this class is something like this:
 *
 * @code
 *    // Example: How to create a FuncDescription...
 *    const char buffer[] = "GLAPI void APIENTRY glDeleteProgramsARB (GLsizei, const GLuint *);"
 *
 *    FuncDescription* func = FuncDescription::parseFunc(buffer, sizeof(buffer));
 *
 *    if ( func != NULL )
 *        cout << func->toString() << endl;
 *    else
 *        cout << "Parse error!" << endl;
 *
 * @endcode
 *       
 * @version 1.0
 * @date 10/11/2003
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es
 */
class FuncDescription
{

private:

    /**
     * static shared buffer used in toString() method
     */
    static char toStringBuffer[]; 

    enum 
    { 
        MAX_PARAMS = 32 ///< Max. number of params in a function description
    };
    
    char* name; ///< Function name

    /**
     * Return type
     */
    ParamDescription* returnType; 

    /**
     * ParamDescription objects representing formal function parameters
     */
    ParamDescription* param[MAX_PARAMS]; 
    
    /**
     * Number of formal function parameters 
     */
    int nParams;

    /**
     * any macro before return type
     *
     * examples: GLAPI or WGLAPI
     */ 
    char* macro1;

    /**
     * any macro after return type
     *
     * examples: GLAPIENTRY or APIENTRY
     */
    char* macro2;


    /**
     * Constructor
     *
     * Private
     *
     * @see parseFunc()
     */
    FuncDescription();

    /**
     * Constructor
     *
     * Private
     *
     * @see parseFunc()
     */
    FuncDescription(const FuncDescription&);

    static bool isMacro1( const char* candidate );
    static bool isMacro2( const char* candidate );
    static bool isVoid( const char* candidate );

public:

    /**
     * Destructor
     */
    ~FuncDescription();

    /**
     * Returns the macro before return type function
     *
     * @return the macro string if exist ( example: "GLAPI" ), null otherwise
     */
    const char* getMacro1() const;

    /**
     * Returns the macro text after return type function
     *
     * @return the macro string if exist ( example: "APIENTRY" ), null otherwise
     */
    const char* getMacro2() const;
    
    /**
     * Parses a standard C function declaratiom (including OpenGL functions using GLAPI, GlAPIENTRY, etc)
     *
     * Parses a OpenGL function given in a single line, the expected definition must
     * be as a definition in a standard C function or OpenGL function ( using GLAPI, APIENTRY, etc )
     *
     * @param i start position in the stream ( once parsed it is updated to the last position read )
     * @param lineFunctionDefinition stream containing a text function definition
     * @param len stream size
     *
     * @return a FuncDescription representing the definition of the input function,
     *         if the text definition could not be parsed returns NULL
     *
     * @note This function is the only that allow creating FuncDescription objects
     */    
    static FuncDescription* parseFunc( int& i, const char* line, int lineLen );
        
    /**
     * Parses a standard C function declaratiom (including OpenGL functions using GLAPI, GlAPIENTRY, etc)
     *
     * Parses a OpenGL function given in a single line, the expected definition must
     * be as a definition in a standard C function or OpenGL function ( using GLAPI, APIENTRY, etc )
     *
     * @param lineFunctionDefinition stream containing a text function definition
     * @param len stream size
     *
     * @return a FuncDescription representing the definition of the input function,
     *         if the text definition could not be parsed returns NULL
     *
     * @note This function is the only that allow creating FuncDescription objects
     * @note Equivalent to: parseFunc(0, line, lineLen)
     */    
    static FuncDescription* parseFunc( const char* line, int lineLen );

    /**
     * Returns function's name
     *
     * @return function's name
     */
    const char* getName() const;
    
    /**
     * Returns return type
     *
     * @return return type unless the function described returns 'void', then null is returned
     */
    ParamDescription* getReturn() const;
    
    /**
     * Returns parameter type in a given position ( first is 0, last is f.countParams()-1 )
     *
     * @param i Parameter position
     */
    ParamDescription* getParam( int i ) const;

    /**
     * Returns the number of parameters
     *
     * @return number of parameters
     *
     * @warning single 'void' parameter type, like in void glEnd( void ) is not considered a parameter.
     *          Therefore this function declaration contains 0 parameters
     */
    int countParams() const;

    /**
     * Obtains a text representation of the function
     *
     * The text representation returned is semantically identical to text representation from where
     * the funcDescription object was parsed
     *
     * @param buffer output buffer where text representation will be dumped
     * @param size buffer size
     *
     * @return pointer to 'buffer'( first parameter )
     *
     * @note final ';' is not returned
     */
    const char* toString( char* buffer, int size ) const;

    /**
     * Obtains a text representation of the function
     *
     * The text representation returned is semantically identical to text representation from where
     * the funcDescription object was parsed
     *
     * @return pointer to buffer where text representation is placed ( internal buffer )
     *
     * @warning Uses an internal static shared buffer. Use strcpy or similar if you need to store
     *          the text contents, store the pointer is not secure
     *         
     * @note final ';' is not returned
     * @note this toString() version is very useful if a quick print is required
     * @code
     *    // f is a funcDescription
     *    cout << f->toString() << endl;
     * @endcode
     */
    const char* toString() const;

    /**
     * Prints debug information
     *
     * @param debug if true prints real debug information, otherwise prints in 
     *        a similar way than toString()
     */ 
    void dump( bool debug=false ) const;

};

#endif // FUNCDESCRIPTION_H
