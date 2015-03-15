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

#ifndef _LINEREADER_

#define _LINEREADER_

#include "GPUTypes.h"
#include <iostream>
#include <vector>

// Avoid <termios.h> for now (win/unix interface compatibility)
struct termios;

/**
 *
 *  The LineReader class provides a basic command line reader features.
 *
 *  The LineReader classes implement a basic command line reader with a
 *  prompt, backspace and direction keys supports and an history buffer.
 *
 */

class LineReader
{

private:

    static const int MAX_CHARS = 256;       /**<  Defines the number of characters.  */

#ifdef WIN32
    static const int ESCAPE_SEQUENCE_LENGTH = 1;    /**<  Number of characters in a escape sequence.  */
    static const unsigned char BACKSPACE = 0x08;    /**<  Backspace character.  */
    static const unsigned char NEWLINE   = 0x0D;    /**<  New line character.  */
    static const unsigned char ESCAPE    = 0xE0;    /**<  Escape character.  */
#else
    static const int ESCAPE_SEQUENCE_LENGTH = 2;    /**<  Number of characters in a escape sequence.  */
    static const unsigned char BACKSPACE = 0x08;    /**<  Backspace character.  */
    static const unsigned char NEWLINE   = 0x0A;    /**<  New line character.  */
    static const unsigned char ESCAPE    = 0x1B;    /**<  Escape character.  */
#endif

    static const char UP_KEY[];                 /**<  Up key escape string.  */
    static const char DOWN_KEY[];               /**<  Down key escape string.  */
    static const char RIGHT_KEY[];              /**<  Right key escape string.  */
    static const char LEFT_KEY[] ;              /**<  Left key escape string.  */


    // declared as a pointer to avoid <termios.h> in .h file
    termios* savedTermInfo;                     /**<  Stores the original termios structure.  */

    static LineReader* lineReaderInst;          /**<  Singleton instance.  */

    LineReader();                               /**<  Constructors are private.  */
    LineReader(const LineReader&);              /**<  Copy constructor is private.  */
    LineReader& operator=(const LineReader&);   /**<  Copy constructor is private.  */

    std::vector<std::string> lineBuffer;        /**<  Stores the current line.  */
    u32bit linePos;                             /**<  Current line position.  */
    u32bit currentLine;                         /**<  Current line in the line buffer.  */

    bool endLine;                               /**<  Flag that is set when a line is finished.  */

    bool escapeMode;                            /**<  Flag that is set when escape character mode is enabled.  */
    std::string escapeString;                   /**<  Stores the escape characters.  */

    void specialChar(unsigned char c);          /**<  Callback function for special characters.  */
    void printableChar(unsigned char c);        /**<  Callback function for printable characters.  */
    void escapeChar(std::string s);             /**<  Callback function for escape characters.  */

#ifdef WIN32

    void *inputHandle;                          /**<  Stores the input handle for the console.  */
    u64bit focusEvents;                         /**<  Focus events counter.  */
    u64bit menuEvents;                          /**<  Menu events counter.  */
    u64bit mouseEvents;                         /**<  Mouse events counter.  */
    u64bit windowBufferSizeEvents;              /**<  Window buffer size events counter.  */   
    u64bit keyEvents;                           /**<  Key events counter.  */

    char pendingEscapeChar;                     /**<  Stores the escape sequence character for special keys.  */
    
    bool useConsoleAPI;                         /**<  Defines if using the Win32 console API or standard C++ input.  */
    
    void setInputProcessing(bool enable);       /**<  Enables/disables input processing.  */
    char getChar();                             /**<  Gets a character from the console input.  */

#endif

public:

    /**
     *
     *  Returns the single instance of the Line Reader class.
     *
     *  @return A reference to the single instance of the Line Reader class.
     *
     */

    static LineReader &instance();

    /**
     *
     *  Initializes the Line Reader.
     *
     *  Saves and configures the input stream terminal.
     *
     */

    void initialize();

    /**
     *
     *  Restores the input stream terminal state.
     *
     */

    void restore();

    /**
     *
     *  Reads a line from the input stream displaying a prompt.
     *
     *  @param prompt The string containing the prompt.
     *  @param line Reference to a string where to store the read line.
     *
     */

    void readLine(std::string prompt, std::string &line);

    /**
     * Deallocates dynamic memory allocated by LineReader object
     */
    ~LineReader();
};

#endif
