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
 * $RCSfile: Parser.h,v $
 * $Revision: 1.4 $
 * $Author: christip $
 * $Date: 2007-10-07 21:32:47 $
 *
 * Parser definition file.
 *
 */

/**
 *
 *  @file Parser.h
 *
 *
 *  This file defines the Parser class.  The Parser class
 *  offers services for parsing strings and text files.
 *
 */

#ifndef _PARSER_

#define _PARSER_

#include "support.h"
#include "GPUTypes.h"
#include <limits.h>

#define MAX_STR_LEN 256     /*  Max size of a string.  */

#define MAX_BYTE CHAR_MAX   /*  Maximum value for a 8 bit integer.  */
#define MIN_BYTE CHAR_MIN   /*  Minimum value for a 8 bit integer.  */
#define MAX_UBYTE UCHAR_MAX /*  Maximum value for a 8 bit unsigned integer.  */

#define MAX_SHORT SHRT_MAX  /*  Maximum value for a 16 bit integer.  */
#define MIN_SHORT SHRT_MIN  /*  Minimum value for a 16 bit integer.  */
#define MAX_USHORT USHRT_MAX    /*  Maximum value for a 16 bit unsigned integer.  */

#define MAX_INT INT_MAX     /*  Maximum value for a 32 bit integer.  */
#define MIN_INT INT_MIN     /*  Minimum value for a 32 bit integer.  */
#define MAX_UINT UINT_MAX   /*  Maximum value for a 32 bit unsigned integer.  */

#define MAX_HEX MAX_UINT    /*  Maximum hexadecimal value for a 32 bit unsigned integer.  */

#define MAX_FP_EXP 127      /*  Maximum FP exponent for a 32 bit float. */
#define MIN_FP_EXP -127     /*  Minimum FP exponent for a 32 bit float. */

/*  End of string test macro.  */
#define checkEndOfString    \
{                           \
    if (pos >= len)         \
    {                       \
        error = PARSER_EOS; \
        return FALSE;       \
    }                       \
}


namespace gpu3d
{

/**
 *
 *  Parser error codes.
 *
 *
 */

enum ParseError
{
    PARSER_OK,
    PARSER_ERROR,
    PARSER_EOS,
    PARSER_NO_ALPHANUM,

};



/**
 *
 *  Parser class.
 *
 *  This class offers parsing functions for string and files.
 *
 */

class Parser
{

protected:

    char *str;          /**<  String being parsed.  */
    size_t len;         /**<  String length.  */
    u32bit pos;         /**<  Current parsing position.  */
    ParseError error;   /**<  The last parsing error.  */

    /**
     *
     *  Skips spaces and tabulator characters.  Stops when
     *  the next non space/tabulator character is found in
     *  the input string/file or until the string/file end.
     *
     */

    void skipSpaces();

    /**
     *
     *  Copies from the source string/file to the destination
     *  string alphanumerical characters.
     *
     *  Stops when one of the following characters is found:
     *
     *   - space, \t, \n
     *   - ( ) [ ] { } = - +  < > , ; |
     *
     *  @param destStr Pointer to the destination string where
     *  the characters are going to be copied.
     *
     */

    void copyString(char *destStr);

public:

    /**
     *
     *  Parser constructor.
     *
     *  Initialized the Parser to parse given a string.  The
     *  string must en in a 0 character.
     *
     *  @param str  The string to parse.
     *
     *  @return An initialized Parser object.
     *
     */

    Parser(char *str);

    /**
     *  Parser constructor.
     *
     *  Initializes the Parser to parse a given string.  The string
     *  doesn't need to end in a 0 character.
     *
     *  @param str The string to parse.
     *  @param length The number of characters in the string to parse.
     *
     *  @return An initialized Parser object.
     *
     */

    Parser(char *str, u32bit length);

    /**
     *
     *  Tries to parse an identifier (alphanumeric name) in the
     *  current parsing position.
     *
     *  @param id Pointer to the string where to copy the parsed
     *  identifier.
     *
     *  @return If a correct identifier could be parsed returns TRUE,
     *  otherwise returns FALSE.
     *
     */

    bool parseId(char *id);

    /**
     *
     *  Tries to parse a decimal number in the current parsing position.
     *
     *  @param val Reference to the signed 32 bit integer where to copy
     *  the parsed decimal number.
     *
     *  @return If a correct decimal number could be parsed returns TRUE,
     *  otherwise returns FALSE.
     *
     */

    bool parseDecimal(s32bit &val);

    /**
     *
     *  Tries to parse a decimal number in the current parsing position.
     *
     *  @param val Reference to the signed 64 bit integer where to copy
     *  the parsed decimal number.
     *
     *  @return If a correct decimal number could be parsed returns TRUE,
     *  otherwise returns FALSE.
     *
     */

    bool parseDecimal(s64bit &val);

    /**
     *
     *  Tries to parse an hexadecimal number in the current parsing position.
     *
     *  @param val Reference to the unsigned 32 bit integer where to copy
     *  the parsed hexadecimal number.
     *
     *  @return If a correct hexadecimal number could be parsed returns TRUE,
     *  otherwise returns FALSE.
     *
     */

    bool parseHex(u32bit &val);

    /**
     *
     *  Tries to parse a float point number in the current parsing position.
     *
     *  @param val Reference to the 32 bit float point where to copy
     *  the parsed float point number.
     *
     *  @return If a correct float point number could be parsed returns TRUE,
     *  otherwise returns FALSE.
     *
     */

    bool parseFP(f32bit &val);

    /**
     *
     *  Tries to parse a decimal byte in the current parsing position.
     *
     *  @param val Reference to the signed 8 bit integer where to copy
     *  the parsed decimal byte.
     *
     *  @return If a correct decimal byte could be parsed returns TRUE,
     *  otherwise returns FALSE.
     *
     */

    bool parseByte(s8bit &val);

    /**
     *
     *  Tries to parse a string limited by quote characters.
     *
     *  @param val Reference to a character array where to store
     *  the parsed string.
     *
     *  @return If the a correct string was parsed returns TRUE,
     *  otherwise returns FALSE.
     *
     */

    bool parseString(char *&strOut);

    /**
     *
     *  Tries to parse a boolean value.  Valid boolean values parsed by this function are:
     *
     *      - for TRUE : true, TRUE, t, T, 1
     *      - for FALSE : false, FALSE, t, T, 0
     *
     *  @param val Reference to a boolean variable where to store the parsed boolean
     *  value.
     *
     *  @return If a correct boolean value was parsed the function returns TRUE, otherwise
     *  returns FALSE.
     *
     */

    bool parseBoolean(bool &val);

};

} // namespace gpu3d

#endif
