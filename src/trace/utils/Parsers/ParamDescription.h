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

#ifndef PARAMDESCRIPTION_H
    #define PARAMDESCRIPTION_H

/**
 * Represents a formal parameter ( or return value ) in a function declaration
 *
 * ParamDescription class models the concept of a Standard C parameter declaration
 * or return value
 *
 * @warning Parsing parameters have a requirement. A declaration is expected to finish
 * with ',', or ')' character, if not, parse will not be possible
 *
 * @code
 *
 *     // parsing a parameter
 *
 *     char buffer[] = " float p1, const   int  *p2 ,"; // comma is mandatory
 *
 *     int i = 10; // skip first parameter ( position before const keyword )
 *
 *     ParamDescription* pd = ParamDescription::parseParam(i, buffer, sizeof(buffer));
 *
 *     // i is updated after calling is performed, here its value is 28, second comma position
 *
 * @endcode
 *
 * @author Carlos González Rodríguez - cgonzale@ac.upc.es
 * @version 0.1
 */
class ParamDescription
{
public:

    /**
     * possible identifiers returned when parseParamType() method is called
     */
    enum PD_TYPE
    {
        NO_TOKEN = 0, ///< any token
        UNKNOWN, ///< Can not be parsed. Unrecognized type
        ENUM, ///< GLenum
        BOOLEAN, ///< bool / GLboolean
        BITFIELD, ///< GLbitfield
        VOID, ///< void / GLvoid
        BYTE, ///< GLbyte 
        UBYTE, ///< GLubyte
        SHORT, ///< short / GLshort
        USHORT, ///< unsigned short / GLushort
        INT, ///< int / GLint / 
        UINT, ///< unsigned int / GLuint
        FLOAT, ///< float / GLfloat
        DOUBLE, ///< double / GLdouble
        CLAMPF, ///< GLclampf
        CLAMPD, ///< GLclampd
        SIZEI, ///< GLsizei
        HDC, ///< HDC Windows specific
        PFD ///< Pixel format description windows Specific
    };
    
private:    

    static char toStringBuffer[]; ///< static shared buffer used in toString method
    
    char* name; ///< Parameter name. It can be NULL

    char* type; ///< Parameter type. It can not be NULL
    
    bool isPointerFlag; ///< Flag indicating if this paramater is a simple pointer

    bool isDoublePointerFlag; ///< Flag indication if this parameter is a double pointer
    
    bool isConstFlag; ///< Flag indicating if this parameter has const modifier

    /**
     * Constructor
     *
     * Private
     *
     * @see parseParam()
     */
    ParamDescription();

    /**
     * Constructor
     *
     * Private
     *
     * @see parseParam()
     */
    ParamDescription(const ParamDescription&);

public:

    /**
     * Destructor
     *
     * Releases all dynamic memory allocated by a ParamDescription
     */
    ~ParamDescription();

    /**
     * Creates a ParamDescription given a line with a parameter and its modifiers
     *
     * Example of use. We are interested in parse second parameter.
     *
     * @code
     * 
     *    char stream[] = "void fooFunction(  uint mode, const void *pointer[16], int size  )";
     *
     *    int i = 29; // position after first comma in the stream
     *
     *    ParamDescription* pd = ParamDescription::parseParam(i, stream, sizeof(stream));
     *
     *    // pd contains the definition of "const void *pointer[16]"
     *    // i value is equal to 53 ( pointing to the second and last comma in the stream )
     *
     * @endcode
     *
     * @param i start position in the input stream containing the text parameter definition
     * @param line input stream with the parameter definition
     * @param lineLen stream size ( in bytes )
     *
     * @return a ParamDescription object representing the input text parameter definition
     * @retval i points just after the parameter parsed
     */
    static ParamDescription* parseParam( int& i, const char* line, int lineLen );

    /**
     * Parses a string representing a type to and PD_TYPE identifier
     *
     * @param i start position in the input stream containing the text parameter definition
     * @param paramString input stream containing the type text
     * @param paramStringLen stream size ( in bytes )
     *
     * @return the corresponding PD_TYPE according to the text parameter. If there is not a token
     *         in the input stream returns NO_TOKEN, if the token is known returns UNKOWN 
     * @retval i points just after the type parsed
     *
     * @note '*' char is considered a delimited ( ignored when parsing )
     */
    static PD_TYPE parseParamType( int& i, const char* paramString, int paramStringLen );

    /**
     * Parses a string representing a type to and PD_TYPE identifier
     *
     * @note Equivalent to parseParamType(0, paramString, paramStringLen)
     *
     * @param paramString input stream containing the type text
     * @param paramStringLen stream size ( in bytes )
     *
     * @return the corresponding PD_TYPE according to the text type. If there is not a token
     *         in the input stream returns NO_TOKEN, if the token is known returns UNKOWN 
     */
    static PD_TYPE parseParamType( const char* paramString, int paramStringLen );

    /**
     * Parses the type of a ParamDescription
     *
     * @return the corresponding PD_TYPE according to the text type in ParamDescription object.
     *         If the token is known returns UNKOWN 
     */
    PD_TYPE parseParamType() const;

    /**
     * Returns paramater name's
     * 
     * @return parameter name's. if there is not a name returns NULL;
     */
    const char* getName() const;

    /**
     * Get parameter's type text
     *
     * @return parameter's type text ( not parsed )
     */
    const char* getType() const;

    /**
     * Check if this parameter is a simple pointer
     *
     * @return true if it is a simple pointer, false otherwise
     *
     * @warning if isDoublePointer() returns true, this function must return false.
     *          isDoublePointer() and isPointer() are exclusive
     */
    bool isPointer() const;

    /**
     * Check if this parameter is a double pointer
     *
     * @return true if it is a double pointer, false otherwise
     *
     * @warning if isSimplePointer() returns true, this function must return false.
     *          isDoublePointer() and isPointer() are exclusive
     */
    bool isDoublePointer() const;

    /**
     * Check if this parameter has the 'const' modifier
     *
     * @return true if is a const parameter, false otherwise
     */
    bool isConst() const;

    const char* toString( char* buffer, int size ) const;
    
    /* 
     * @warning only a temporary string is returned, do not store it,
     * use this function only for inmediate printing or copying
     */
    const char* toString() const;

    /**
     * Dumps ParamDescription information
     *
     * @param debug if enabled prints the parameter in debug format.
     *        Otherwise prints the parameter like text definition ( without returns )
     */
    void dump( bool debug=false ) const;
   
};

#endif // PARAMDESCRIPTION_H
