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

#ifndef STRINGTOKENIZER_H
    #define STRINGTOKENIZER_H

/**
 * Class for manage C strings
 *
 * This class allow process C strings easily. It has operations to:
 *   - Extract substrings (tokens)
 *   - Iterate over chars
 *   - Skip spaces or other chars
 *   - Search for a given char
 *   - Random access by position
 *   - etc
 *
 * @note Do not modify the buffer used to create this object as the standard C
 *       library function strtok does
 * 
 * Examples of use
 * @code
 *     StringTokenizer st("Hi my name's Peter");
 *     // default delimiters are ' ' and '\t'
 *     
 *     char token[256];
 *
 *     while ( st.getToken(token,sizeof(token)) )
 *        cout << token << endl;
 *
 *     // prints (quotes not printed)
 *     //   "Hi"
 *     //   "my"
 *     //   "name's"
 *     //   "Peter"
 *
 *     st.setPosition(0); // restart position
 *     st.setDefaultDelimiters("'");
 *     // now the only char considered a delimiter is '''
 *
 *     while ( !st.getToken(token,sizeof(token)) )
 *        cout << token << endl;
 *
 *     // prints (quotes not printed)
 *     //   "Hi my name"
 *     //   "s Peter"
 *
 * #@endcode
 *           
 * @version 1.0
 * @date 10/11/2003
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es 
 */ 
class StringTokenizer
{
private:
    
    char* str; ///< internal buffer where StringTokenizer will perform operations

    int strLen; ///< buffer length

    int pos; ///< current position in the internal bufefr

    char* delimiters; ///< characters that will be considered delimiters

    /**
     * Simple function that checks if a given char is a delimiter in a
     * delimiter list
     *
     * @param delimiters delimiter list
     * @param character to be tested
     *
     * @return true if the character is in the delimiter list, false otherwise
     */
    static bool isDelimiter( const char* delimiters, char c );

public:

    /**
     * Destructor
     *
     * Release internal buffer
     */
    ~StringTokenizer();

    /**
     * Creates a new StringTokenizer given a Standard C string (null-terminated)
     *
     * @note The internal buffer is created making a real copy of this buffer.
     *       Therefore all operations to this object do not modify the buffer
     *       used to create the object
     * @param str a Standard C string (null-terminated)
     */
    StringTokenizer( const char* str );

    /**
     * Creates a new StringTokenizer given a buffer of chars and its size
     *
     * It is not required that the buffer is null-terminated
     *
     * @note The internal buffer is created making a real copy. Therefore
     *       all operations in this StringTokenizer object do not modify
     *       the buffer used to create the object
     *
     * @param str buffer of chars
     * @param size buffer size
     */
    StringTokenizer( const char* str, int size );

    /**
     * Returns next token
     *
     * @param buffer buffer where next token will be stored
     * @param buffer size
     *
     * @return pointer to 'buffer' if there is a next token, null otherwise
     */
    const char* getToken( char* buffer, int size );

    /**
     * Returns next token
     *
     * @param buffer buffer where next token will be stored
     * @param buffer size
     * @delimiters specific delimiters used instead default delimiters
     *
     * @return pointer to 'buffer' if there is a next token, null otherwise
     */
    const char* getToken( char* buffer, int size, const char* delimiters );

    /**
     * Gets current position in the internal buffer
     *
     * @return pos current position in the internal buffer
     */ 
    int getPos() const;

    /**
     * Gets the char pointed by the current position in the internal buffer
     *
     * @return char in the current position of the internal buffer 
     */
    char getCurrentChar() const;

    /**
     * Increase the position into the internal buffer until a char is found
     *
     * The position is increased until a char from a given list of chars
     * is found ( is pointed by the internal buffer current position )
     *
     * @code
     *
     *     // Internal buffer of 'st' contains = "This is and example"
     *     // Current position is 2 ( pointing to i of 'This' word )
     * 
     *     // skip chars until an 'a' or 'd' is found
     *     int newPos = st.skipUntilChar("ad"); 
     *
     *     // newPos has the value 8 ( pointing to 'a' in 'and' word )
     *
     * @endcode
     *
     * @param cList list of characters we are looking for
     *
     * @return position with the first occurrence of a char in the list 'cList',
     *         length() if no char is found ( for compatibility )
     *
     * @see skipUntilCharRev, skipWhileChar, skipWhileCharRev
     */
    int skipUntilChar( const char* cList );

    char getChar( int absolutePos );

    int skipUntilCharRev( const char* cList );

    int skipWhileChar( const char* cList );

    int skipWhileCharRev( const char* cList );

    bool isCurrentChar( const char* cList ) const;

    // negative for skip chars in reverse
    int skipChars( int howMany );    

    int setPos( int newPos );

    int length() const;

    void setDefaultDelimiters( const char* delimiters );

    void dump() const;

    //void resetToDefaultDelimiters();

};

#endif // STRINGTOKENIZER_H
