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
 * $RCSfile: Parser.cpp,v $
 * $Revision: 1.5 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:55 $
 *
 * Parser implementation file.
 *
 */


/**
 *
 *  @file Parser.cpp
 *
 *
 *  This file implements the Parser class.  The Parser class
 *  offers services for parsing strings and text files.
 *
 */

#include <cmath>
#include <cstring>
#include "Parser.h"

using namespace gpu3d;

/* Minimum and maximum values a `signed long long int' can hold.  */
#ifndef LLONG_MAX
    #define LLONG_MAX    9223372036854775807LL
#endif
#ifndef LLONG_MIN
    #define LLONG_MIN    (-LLONG_MAX - 1LL)
#endif

/* Maximum value an `unsigned long long int' can hold.  (Minimum is 0.)  */

#ifndef ULLONG_MAX
    #define ULLONG_MAX   18446744073709551615ULL
#endif

/*  Parser constructor.  */
Parser::Parser(char *string)
{
    /*  Set parse string.  */
    str = string;

    /*  Get string length.  */
    len = strlen(str);

    /*  Reset parse position.  */
    pos = 0;

    /*  Clear error.  */
    error = PARSER_OK;
}

Parser::Parser(char *string, u32bit length)
{
    /*  Set parse string.  */
    str = string;

    /*  Set the parse string length.  */
    len = length;

    /*  Reset parse position.  */
    pos = 0;

    /*  Clear error.  */
    error = PARSER_OK;
}

bool Parser::parseId(char *id)
{
    /*  Skip spaces.  */
    skipSpaces();

    checkEndOfString

    /*  Check if it is the first character of an identifier (numbers
        excluded).  */

    if (!(((str[pos] >= 'a') && (str[pos] <= 'z')) ||
        ((str[pos] >='A') && (str[pos] <='Z')) ||
        (str[pos] == '#') || (str[pos] == '_') || (str[pos] == '$')))
    {
        error = PARSER_NO_ALPHANUM;
        return FALSE;
    }

    /*  Copy characters until an space/tab/newline is found or the end
        of the line is reached.  */

    copyString(id);

    return TRUE;
}

/*  Parses a decimal integer.  */
bool Parser::parseDecimal(s32bit &val)
{
    u8bit digit;

    /*  Skip spaces.  */
    skipSpaces();

    checkEndOfString

    /*  Check if it is a correct decimal value.  */
    if ((str[pos] != '-') && !((str[pos] >= '0') && (str[pos] <='9')))
        return FALSE;

    /*  Is a negative number.  */
    if (str[pos] == '-')
    {
        /*  Skip negative sign.  */
        pos++;

        checkEndOfString

        /*  Check decimal digit.  */
        if (!((str[pos] >= '0') && (str[pos] <= '9')))
            return FALSE;

        /*  Initialice as a negative value.  */
        val = - (str[pos] - '0');

        /*  Skip first digit.  */
        pos++;
    }
    else
        val = 0;

    checkEndOfString

    /*  Check and copy decimal number digits.  */
    for (; (pos < len) && ((str[pos] >= '0') && (str[pos] <='9')); pos++)
    {
        digit = str[pos] - '0';

        /*  Check if is a negative or a positive number.  */
        if (val < 0)
        {
            /*  Check overflow.  */
            if ((-(MIN_INT - val)) < digit)
                return FALSE;

            /*  Update value.  */
            val = val * 10 - digit;
        }
        else    /*  Positive Number.  */
        {
            /*  Check overflow.  */
            if ((MAX_INT - val) < digit)
                return FALSE;

            /*  Update value.  */
            val = val * 10 + digit;
        }
    }

    checkEndOfString

    /*  Check correct end of the decimal number.  */
    if ((str[pos] != ' ') && (str[pos] != '\t') && (str[pos] != '\n') &&
        (str[pos] != '=') && (str[pos] != '-') && (str[pos] != '+') &&
        (str[pos] != '<') && (str[pos] != '>') && (str[pos] != ',') &&
        (str[pos] != ';') && (str[pos] != '(') && (str[pos] != ')') &&
        (str[pos] != '[') && (str[pos] != ']') && (str[pos] != '{') &&
        (str[pos] != '}') && (str[pos] != '|') && (str[pos] != ':'))
        return FALSE;

    return TRUE;

}

/*  Parses a decimal integer.  */
bool Parser::parseDecimal(s64bit &val)
{
    u8bit digit;

    /*  Skip spaces.  */
    skipSpaces();

    checkEndOfString

    /*  Check if it is a correct decimal value.  */
    if ((str[pos] != '-') && !((str[pos] >= '0') && (str[pos] <='9')))
        return FALSE;

    /*  Is a negative number.  */
    if (str[pos] == '-')
    {
        /*  Skip negative sign.  */
        pos++;

        checkEndOfString

        /*  Check decimal digit.  */
        if (!((str[pos] >= '0') && (str[pos] <= '9')))
            return FALSE;

        /*  Initialice as a negative value.  */
        val = - (str[pos] - '0');

        /*  Skip first digit.  */
        pos++;
    }
    else
        val = 0;

    checkEndOfString

    /*  Check and copy decimal number digits.  */
    for (; (pos < len) && ((str[pos] >= '0') && (str[pos] <='9')); pos++)
    {
        digit = str[pos] - '0';

        /*  Check if is a negative or a positive number.  */
        if (val < 0)
        {
            /*  Check overflow.  */
            if ((-s32bit(LLONG_MIN - val)) < digit)
                return FALSE;

            /*  Update value.  */
            val = val * 10 - digit;
        }
        else    /*  Positive Number.  */
        {
            /*  Check overflow.  */
            if ((LLONG_MAX - val) < digit)
                return FALSE;

            /*  Update value.  */
            val = val * 10 + digit;
        }
    }

    checkEndOfString

    /*  Check correct end of the decimal number.  */
    if ((str[pos] != ' ') && (str[pos] != '\t') && (str[pos] != '\n') &&
        (str[pos] != '=') && (str[pos] != '-') && (str[pos] != '+') &&
        (str[pos] != '<') && (str[pos] != '>') && (str[pos] != ',') &&
        (str[pos] != ';') && (str[pos] != '(') && (str[pos] != ')') &&
        (str[pos] != '[') && (str[pos] != ']') && (str[pos] != '{') &&
        (str[pos] != '}') && (str[pos] != '|') && (str[pos] != ':'))
        return FALSE;

    return TRUE;

}

/*  Parses an hexadecimal integer.  */
bool Parser::parseHex(u32bit &val)
{
    u8bit digit;

    /*  Skip spaces.  */
    skipSpaces();

    checkEndOfString

    /*  Check and skip '0x'/'0X'.  */
    if (((pos + 1) < len) && (str[pos] == '0') &&
        ((str[pos+1] == 'x') || (str[pos + 1] == 'X')))
    {
        /*  Skip '0x'/'0X'  */
        pos += 2;
    }

    checkEndOfString

    /*  Check fist charater is a correct hexadecimal digit.  */
    if (!(((str[pos] >= '0') && (str[pos] <= '9')) ||
        ((str[pos] >= 'a') && (str[pos] <= 'f')) ||
        ((str[pos] >= 'A') && (str[pos] <= 'F'))))
        return FALSE;

    /*  Initialize value.  */
    val = 0;

    /*  Check and copy hexadecimal number digits.  */
    for (; (pos < len) && (((str[pos] >= '0') && (str[pos] <='9')) ||
        ((str[pos] >='A') && (str[pos] <= 'F')) ||
        ((str[pos] >='a') && (str[pos] <= 'f'))); pos++)
    {
        /*  Decode the hexadecimal digit.  */
        digit = ((str[pos] >= '0') && (str[pos] <= '9'))?str[pos] - '0': 1;
        digit *= ((str[pos] >= 'a') && (str[pos] <= 'f'))?10 + str[pos] - 'a': 1;
        digit *= ((str[pos] >= 'A') && (str[pos] <= 'F'))?10 + str[pos] - 'A': 1;

        /*  Check overflow.  */
        if ((MAX_HEX - val) <= digit)
            return FALSE;

        /*  Update value.  */
        val = val * 16 + digit;
    }

    checkEndOfString

    /*  Check correct end of the hexadecimal number.  */
    if ((str[pos] != ' ') && (str[pos] != '\t') && (str[pos] != '\n') &&
        (str[pos] != '=') && (str[pos] != '-') && (str[pos] != '+') &&
        (str[pos] != '<') && (str[pos] != '>') && (str[pos] != ',') &&
        (str[pos] != ';') && (str[pos] != '(') && (str[pos] != ')') &&
        (str[pos] != '[') && (str[pos] != ']') && (str[pos] != '{') &&
        (str[pos] != '}') && (str[pos] != '|'))
        return FALSE;

    return TRUE;
}

/*  Parses a FP value.   */
bool Parser::parseFP(f32bit &val)
{
    u8bit digit;
    f32bit fraction;
    s32bit exponent;
    s32bit sign;

    /*  Skip spaces.  */
    skipSpaces();

    checkEndOfString

    /*  Check if it is a correct decimal value.  */
    if ((str[pos] != '-') && !((str[pos] >= '0') && (str[pos] <='9')))
        return FALSE;

    /*  Is a negative number.  */
    if (str[pos] == '-')
    {
        /*  Skip negative sign.  */
        pos++;

        checkEndOfString

        /*  Set negative sign.  */
        sign = -1;
    }
    else
        /*  Set positive sign.  */
        sign = 1;


    /*  Check and copy decimal number digits.  */
    for (val = 0; (pos < len) && ((str[pos] >= '0') && (str[pos] <='9')); pos++)
    {
        /*  Get digit value.  */
        digit = str[pos] - '0';

        /*  Check if it is a negative number.  */
        if (sign < 0)
        {
            val = val * 10 - digit;
        }
        else    /*  Positive number.  */
        {
            /*  Update value.  */
            val = val * 10 + digit;
        }
    }

    checkEndOfString

    /*  Check fraction point.  */
    if (str[pos] == '.')
    {
        /*  Skip '.'.  */
        pos++;

        /*  Obtain fractional part.  */
        fraction = 1;

        /*  Check and copy decimal number digits.  */
        for (; (pos < len) && ((str[pos] >= '0') && (str[pos] <='9')); pos++)
        {
            /*  Read digit.  */
            digit = str[pos] - '0';

            /*  Add digit.  */
            fraction = fraction/10;

            /*  Check if it is a negative number.  */
            if (sign < 0)
            {
                /*  Update value.  */
                val -= fraction * digit;
            }
            else    /*  Positive number.  */
            {
                /*  Update value.  */
                val += fraction * digit;
            }
        }

    }

    checkEndOfString

    /*  Check exponent.  */
    if ((str[pos] == 'E') || (str[pos] == 'e'))
    {
        /*  Skip 'E'/'e'.  */
        pos++;

        /*  Read the exponent value.  */
        parseDecimal(exponent);

        /*  Check exponent overflow.  */
        if ((exponent > MAX_FP_EXP) || (exponent < MIN_FP_EXP))
            return FALSE;

        /*  Update value.  */
        val *= pow(10.0f, exponent);
    }

    checkEndOfString

    /*  Check correct end of the decimal number.  */
    if ((str[pos] != ' ') && (str[pos] != '\t') && (str[pos] != '\n') &&
        (str[pos] != '=') && (str[pos] != '-') && (str[pos] != '+') &&
        (str[pos] != '<') && (str[pos] != '>') && (str[pos] != ',') &&
        (str[pos] != ';') && (str[pos] != '(') && (str[pos] != ')') &&
        (str[pos] != '[') && (str[pos] != ']') && (str[pos] != '{') &&
        (str[pos] != '}') && (str[pos] != '|'))
        return FALSE;

    return TRUE;

}

/*  Skips spaces and tabs in a string.  */
void Parser::skipSpaces()
{
    /*  Skip spaces and tabs.  */

    while((pos < len) && ((str[pos] == ' ') || (str[pos] == '\t')))
        pos++;
}

/*  Copies a string from pos until a space/tab/newline/end of line.  */
void Parser::copyString(char *strDest)
{
    int i;

    /*  Copy until a non allowed character is found or the end of the
        string.  */

    for(i = 0; (pos < len) &&
        (str[pos] != ' ') && (str[pos] != '\t') && (str[pos] != '\n') &&
        (str[pos] != '=') && (str[pos] != '-') && (str[pos] != '+') &&
        (str[pos] != '<') && (str[pos] != '>') && (str[pos] != ',') &&
        (str[pos] != ';') && (str[pos] != '(') && (str[pos] != ')') &&
        (str[pos] != '[') && (str[pos] != ']') && (str[pos] != '{') &&
        (str[pos] != '}') && (str[pos] != '|') &&
        (i < (MAX_STR_LEN - 1)); i++, pos++)
        strDest[i] = str[pos];

    /*  Mark end of string.  */
    strDest[i] = '\0';
}

/*  Parse a byte value.  */
bool Parser::parseByte(s8bit &val)
{
    s32bit auxVal;

    parseDecimal(auxVal);

    if ((auxVal < MIN_BYTE) || (auxVal > MAX_BYTE))
        return FALSE;

    val = auxVal;

    return TRUE;
}


/*  Parse string. */
bool Parser::parseString(char *&strOut)
{
    u32bit savedPos;
    u32bit strLen;
    u32bit i;

    /*  Check start string character.  */
    if (str[pos] != '"')
        return FALSE;

    /*  Update position.  */
    pos++;

    checkEndOfString

    /*  Save parse position.  */
    savedPos = pos;

    /*  Reset string length.  */
    strLen = 0;

    /*  Count characters in the line until the string end character.  */
    for(;(pos < len) && (str[pos] != '"'); pos++, strLen++);

    checkEndOfString

    /*  Allocate the string.  */
    strOut = new char[strLen + 1];

    /*  Restore position.  */
    pos = savedPos;

    /*  Copy string.  */
    for(i = 0; i < strLen; i++, pos++)
        strOut[i] = str[pos];

    /*  Write end of string character (zero).  */
    strOut[strLen] = '\0';

    /*  Skip end of string character.  */
    pos++;

    return TRUE;
}

/*  Parses a boolean value.  */
bool Parser::parseBoolean(bool &val)
{
    //u8bit digit;

    /*  Skip spaces.  */
    skipSpaces();

    checkEndOfString

    /*  Check first character of the boolean value.  */
    if (str[pos] == 't')
    {
        pos++;

        checkEndOfString

        /*  Check for large version.  */
        if (str[pos] == 'r')
        {
            pos++;

            checkEndOfString;

            /*  Check lenght.  */
            if (((pos + 2) >= len) || (str[pos] != 'u') || (str[pos + 1] != 'e'))
                return FALSE;

            pos += 2;
        }

        val = TRUE;
    }
    else if (str[pos] == 'T')
    {
        pos++;

        checkEndOfString

        /*  Check for large version.  */
        if (str[pos] == 'R')
        {
            pos++;

            checkEndOfString;

            /*  Check lenght.  */
            if (((pos + 2) >= len) || (str[pos] != 'U') || (str[pos + 1] != 'E'))
                return FALSE;

            pos += 2;
        }

        val = TRUE;
    }
    else if (str[pos] == 'f')
    {
        pos++;

        checkEndOfString

        /*  Check for large version.  */
        if (str[pos] == 'a')
        {
            pos++;

            checkEndOfString;

            /*  Check lenght.  */
            if (((pos + 3) >= len) && (str[pos] != 'l') && (str[pos + 1] != 's') && (str[pos + 2] != 'e'))
                return FALSE;

            pos += 3;
        }

        val = FALSE;
    }
    else if (str[pos] == 'F')
    {
        pos++;

        checkEndOfString

        /*  Check for large version.  */
        if (str[pos] == 'A')
        {
            pos++;

            checkEndOfString;

            /*  Check lenght.  */
            if (((pos + 3) >= len) && (str[pos] != 'L') && (str[pos + 1] != 'S') && (str[pos + 2] != 'E'))
                return FALSE;

            pos += 3;
        }

        val = FALSE;
    }
    else if (str[pos] == '1')
    {
        pos++;

        val = TRUE;
    }
    else if (str[pos] == '0')
    {
        pos++;

        val = FALSE;
    }
    else
        return FALSE;

    checkEndOfString

    /*  Check correct end of the boolean value.  */
    if ((str[pos] != ' ') && (str[pos] != '\t') && (str[pos] != '\n') &&
        (str[pos] != '=') && (str[pos] != '-') && (str[pos] != '+') &&
        (str[pos] != '<') && (str[pos] != '>') && (str[pos] != ',') &&
        (str[pos] != ';') && (str[pos] != '(') && (str[pos] != ')') &&
        (str[pos] != '[') && (str[pos] != ']') && (str[pos] != '{') &&
        (str[pos] != '}') && (str[pos] != '|') && (str[pos] != ':'))
        return FALSE;

    return TRUE;
}
