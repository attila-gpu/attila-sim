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

#include "LineReader.h"
#include "support.h"

#include <cstdlib>
#include <cstring>
#include <stdio.h>

using namespace std;

/*  LineReader single instance.  */
LineReader *LineReader::lineReaderInst = 0;


// This method is fully multiplatform
/*  Returns a reference to the single LineReader instance.  */
LineReader &LineReader::instance()
{
    /*  Create the LineReader instance if it doesn't exist yet.  */
    if (lineReaderInst == 0)
    {
        lineReaderInst = new LineReader();
    }

    /*  Return the single LineReader instance.  */
    return *lineReaderInst;
}


//#ifdef WIN32

/*
 * WIN32 Stub-Dummy implementation (simply panics)
 */

/*void specialChar(unsigned char c) {}
void printableChar(unsigned char c) {}
void escapeChar(std::string s) {}

LineReader::LineReader() 
{
    panic("LineReader", "ctor", "Line reader not supported under windows for now");
}

void LineReader::initialize() {}
void LineReader::restore() {}
void LineReader::readLine(string prompt, string& line) {}
LineReader::~LineReader() {}
*/
   
//#else // End WIN32 Dummy implementation

#ifdef WIN32
    #include <conio.h>
    #include <windows.h>
#else
    #include <termios.h>
#endif

#ifdef WIN32

#define ENABLE_INSERT_MODE 0x0020
#define ENABLE_QUICK_EDIT_MODE 0x0040
#define ENABLE_EXTENDED_FLAGS 0x0080
#define ENABLE_AUTO_POSITION 0x0100

const char LineReader::UP_KEY[]       = "H";     /**<  Up key escape string.  */
const char LineReader::DOWN_KEY[]     = "P";     /**<  Down key escape string.  */
const char LineReader::RIGHT_KEY[]    = "M";     /**<  Right key escape string.  */
const char LineReader::LEFT_KEY[]     = "K";     /**<  Left key escape string.  */
#else
const char LineReader::UP_KEY[]       = "[A";     /**<  Up key escape string.  */
const char LineReader::DOWN_KEY[]     = "[B";     /**<  Down key escape string.  */
const char LineReader::RIGHT_KEY[]    = "[C";     /**<  Right key escape string.  */
const char LineReader::LEFT_KEY[]     = "[D";     /**<  Left key escape string.  */
#endif

/*  LineReader constructor.  */
LineReader::LineReader() :
    escapeMode(false), endLine(false), linePos(0), currentLine(0)
{
    escapeString.clear();
    lineBuffer.clear();
    
#ifdef WIN32

    //  Get the console input handle.
    inputHandle = GetStdHandle(STD_INPUT_HANDLE);

    if (inputHandle == INVALID_HANDLE_VALUE)
        panic("LineReader", "LineReader", "Error obtaining the console input handle.");

    //  Check if the handler is usable.
    DWORD mode = 0;
    BOOL result = GetConsoleMode(inputHandle, &mode);

    //  Check if the console API can be used.
    useConsoleAPI = (result != 0);
    
    //  Reset event counters.
    focusEvents = 0;
    menuEvents = 0;
    mouseEvents = 0;
    windowBufferSizeEvents = 0;
    keyEvents = 0;
    
    //  Set escape sequence character to 0 (no escape sequence character pending).
    pendingEscapeChar = 0;
   
#else
    savedTermInfo = new termios;
#endif
}

LineReader::~LineReader()
{
#ifndef WIN32
    delete savedTermInfo;
#endif    
}


#ifdef WIN32

void LineReader::setInputProcessing(bool enable)
{
    return;
    
    //  Get the console input mode.
    DWORD mode = 0;

    //  Get the console input handle.
    BOOL result = GetConsoleMode(inputHandle, &mode);
    
    if (!result)
    {
        DWORD error = GetLastError();
        panic("LineReader", "setInputProcessing", "Error getting the console input mode.");
    }
       
    //  Enable/disable input processing.
    mode = enable ? (mode | ENABLE_PROCESSED_INPUT) :  (mode & ~ENABLE_PROCESSED_INPUT);
    result = SetConsoleMode(inputHandle, mode);
    
    if (!result)
    {
        DWORD error = GetLastError();
        panic("LineReader", "setInputProcessing", "Error setting the console input mode.");   
    }
}

char LineReader::getChar()
{
    char resultChar;
    
    //  Check if there is escape sequence character pending.
    if (pendingEscapeChar != 0)
    {
        //  Return the escape sequence character and reset.
        resultChar = pendingEscapeChar;
        pendingEscapeChar = 0;
        return resultChar;
    }
    
    bool keyPressed = false;
    
    //  Wait until a key is pressed.
    while (!keyPressed)
    {
        INPUT_RECORD inputRecord;
        DWORD numInputRecords = 0;
        BOOL result = ReadConsoleInput(inputHandle, &inputRecord, 1, &numInputRecords);
        
        if (!result)
            panic("LineReader", "getChar", "Error reading console input.");
     
        switch(inputRecord.EventType)
        {
            case FOCUS_EVENT:
                focusEvents++;
                break;
                
            case KEY_EVENT:
            
                keyEvents++;
                
                //  Check for key press, not release.
                if (inputRecord.Event.KeyEvent.bKeyDown)
                {
                    keyPressed = true;
                    
                    switch(inputRecord.Event.KeyEvent.wVirtualKeyCode)
                    {
                        //case VK_BACK:
                        //case VK_RETURN:
                        //case VK_ESCAPE:
                        
                        case VK_UP:
                        
                            pendingEscapeChar = UP_KEY[0];
                            resultChar = ESCAPE;
                            break;
                            
                        case VK_DOWN:
                        
                            pendingEscapeChar = DOWN_KEY[0];
                            resultChar = ESCAPE;
                            break;

                        case VK_LEFT:
                        
                            pendingEscapeChar = LEFT_KEY[0];
                            resultChar = ESCAPE;
                            break;

                        case VK_RIGHT:
                        
                            pendingEscapeChar = RIGHT_KEY[0];
                            resultChar = ESCAPE;
                            break;
                       
                        default:
                        
                            resultChar = inputRecord.Event.KeyEvent.uChar.AsciiChar;
                            break;
                    }
                }
                                
                break;
            
            case MENU_EVENT:
                menuEvents++;
                break;
                
            case MOUSE_EVENT:
                mouseEvents++;
                break;
                
            case WINDOW_BUFFER_SIZE_EVENT:
                windowBufferSizeEvents++;
                break;
                
            default:
                panic("LineReader", "getChar", "Undefined console input event.");
                break;
        }        
    }

    return resultChar;
}

#endif

//  Initializes the Line Reader
void LineReader::initialize()
{
#ifndef WIN32
    struct termios newTermInfo;

    //  Get the terminal attribute structure.
    tcgetattr(fileno(stdin), savedTermInfo);

    //  Make a clean terminal attribute structure.
    //cfmakeraw(&newTermInfo);
    memcpy(&newTermInfo, savedTermInfo, sizeof(newTermInfo));
    //newTermInfo.c_lflag = newTermInfo.c_lflag & ~ECHOCTL & ~ICANON;
    newTermInfo.c_lflag = ISIG;

    //  Set the clean terminal attribute structure.
    tcsetattr(fileno(stdin), TCSANOW, &newTermInfo);

    // Initialize the character callback table.
    //for(int i = 0; i < 0x20; i++)
    //    callBackTable[i] = &specialChar;
    //for(int i = 0x20; i < 0x100 < i++)
    //    callBackTable[i] = &printableChar;
    //callBackTable[0x7F] = &specialChar;
#endif
};

void LineReader::restore()
{
#ifndef WIN32
    //  Restore the original terminal attribute structure.
    tcsetattr(fileno(stdin), TCSANOW, savedTermInfo);
#endif
}

void LineReader::readLine(string prompt, string &line)
{
    unsigned char c;
    string s;

    endLine = false;

    s.clear();

    currentLine = lineBuffer.size();

    lineBuffer.push_back(s);

    linePos = 0;

    cout << prompt.c_str();
    cout.flush();

#ifdef WIN32
    //  Disable input processing.
    if (useConsoleAPI)
        setInputProcessing(false);
    else
    {
        char buffer[256];
        cin.getline(buffer, 256 - 1);
        endLine = true;
        lineBuffer.at(currentLine).append(buffer);
    }
#endif

    while(!endLine)
    {
#ifdef WIN32
        //c = _getch();
        c = getChar();
#else
        c = cin.get();
#endif
        //callBackTable[c](c);
        if (!escapeMode)
        {
            if ((c > 0x1F) && (c != 0x7F) && (c != ESCAPE))
                printableChar(c);
            else
                specialChar(c);
        }
        else
        {
            escapeString.push_back(c);
            if (escapeString.length() == ESCAPE_SEQUENCE_LENGTH)
            {
                escapeChar(escapeString);
                escapeMode = false;
            }
        }
    }

#ifdef WIN32
    //  Enable input processing.
    if (useConsoleAPI)
        setInputProcessing(true);
#endif

    line = lineBuffer.at(currentLine);

    if (currentLine != (lineBuffer.size() - 1))
    {
        lineBuffer.at(lineBuffer.size() - 1) = lineBuffer.at(currentLine);
        currentLine = lineBuffer.size();
    }

    currentLine++;
}

void LineReader::escapeChar(string s)
{
    if (!s.compare(UP_KEY))
    {
        if (currentLine > 0)
        {
            for(; linePos > 0; linePos--)
                cout << '\b';

            for(linePos = 0; linePos < lineBuffer.at(currentLine - 1).length(); linePos++)
                cout << lineBuffer.at(currentLine - 1)[linePos];

            for(int i = linePos; i < lineBuffer.at(currentLine).length(); i++)
                cout << ' ';

            for(int i = linePos; i < lineBuffer.at(currentLine).length(); i++)
                cout << '\b';

            currentLine--;
        }
    }
    else if (!s.compare(DOWN_KEY))
    {
        if (currentLine < (lineBuffer.size() - 1))
        {
            for(; linePos > 0; linePos--)
                cout << '\b';

            for(linePos = 0; linePos < lineBuffer.at(currentLine + 1).length(); linePos++)
                cout << lineBuffer.at(currentLine + 1)[linePos];

            for(int i = linePos; i < lineBuffer.at(currentLine).length(); i++)
                cout << ' ';

            for(int i = linePos; i < lineBuffer.at(currentLine).length(); i++)
                cout << '\b';

            currentLine++;
        }
    }
    else if (!s.compare(RIGHT_KEY))
    {
        if (linePos < lineBuffer.at(currentLine).length())
        {
            cout << lineBuffer.at(currentLine)[linePos];
            linePos++;
        }
    }
    else if (!s.compare(LEFT_KEY))
    {
        if (linePos > 0)
        {
            cout << '\b';
            linePos--;
        }
    }
}

void LineReader::specialChar(unsigned char c)
{
    if (c == BACKSPACE)
    {
        if (lineBuffer.at(currentLine).length() > 0)
        {
            if (linePos == lineBuffer.at(currentLine).length())
            {
                cout << '\b' << ' ' << '\b';
                lineBuffer.at(currentLine).erase(linePos - 1, 1);
                linePos--;
            }
            else if (linePos > 0)
            {
                lineBuffer.at(currentLine).erase(linePos - 1, 1);
                linePos--;

                cout << '\b';

                for(int i = linePos; i < lineBuffer.at(currentLine).length(); i++)
                    cout << lineBuffer.at(currentLine)[i];

                cout << ' ';

                for(int i = linePos; i < lineBuffer.at(currentLine).length() + 1; i++)
                    cout << '\b';
            }
        }
    }
    else if (c == NEWLINE)
    {
        //lineBuffer.at(currentLine).push_back(c);
        endLine = true;
        cout << endl;
        linePos++;
    }
    else if (c == ESCAPE)
    {
        escapeMode = true;
        escapeString.clear();
    }
    //else
    //{
    //    cout << "UNKNOWN " << c << "(0x" << hex << unsigned int(c) << dec << ")" << endl;
    //}
}

void LineReader::printableChar(unsigned char c)
{
    if (lineBuffer.at(currentLine).length() == linePos)
    {
        lineBuffer.at(currentLine).push_back(c);
        cout << c;
        linePos++;
    }
    else if (lineBuffer.at(currentLine).length() > linePos)
    {
        lineBuffer.at(currentLine).insert(linePos, 1, c);
        cout << c;
        linePos++;

        for(int i = linePos; i < lineBuffer.at(currentLine).length(); i++)
            cout << lineBuffer.at(currentLine)[i];
        for(int i = linePos; i < lineBuffer.at(currentLine).length(); i++)
            cout << '\b';

    }
    else
    {
        cout << "eror => LineReader::printableChar(), Line position is out of bonds." << endl;
        exit(-1);
    }
}

/*void (*LineReader::callBackTable[])(char) =
{
    &specialChar,       // 0x00 -> NUL (null)
    &specialChar,       // 0x01 -> SOH (start of heading)
    &specialChar,       // 0x02 -> STX (start of text)
    &specialChar,       // 0x03 -> ETX (end of text)
    &specialChar,       // 0x04 -> EOT (end of transmission)
    &specialChar,       // 0x05 -> ENQ (enquiry)
    &specialChar,       // 0x06 -> ACK (acknowledge)
    &specialChar,       // 0x07 -> BEL (bell)
    &specialChar,       // 0x08 -> BS (backspace)
    &specialChar,       // 0x09 -> TAB (horizontal tab)
    &specialChar,       // 0x0A -> LF (NL line feed, new line)
    &specialChar,       // 0x0B -> VT (vertical tab)
    &specialChar,       // 0x0C -> FF (NP form feed, new page)
    &specialChar,       // 0x0D -> CR (carriage return)
    &specialChar,       // 0x0E -> SO (shift out)
    &specialChar,       // 0x0F -> SI (shift in)
    &specialChar,       // 0x10 -> DLE (data link escape)
    &specialChar,       // 0x11 -> DC1 (device control 1)
    &specialChar,       // 0x12 -> DC2 (device control 2)
    &specialChar,       // 0x13 -> DC3 (device control 3)
    &specialChar,       // 0x14 -> DC4 (device control 4)
    &specialChar,       // 0x15 -> NAK (negative acknowledge)
    &specialChar,       // 0x16 -> SYN (synchronous idle)
    &specialChar,       // 0x17 -> ETB (end of trans. block)
    &specialChar,       // 0x18 -> CA (cancel)
    &specialChar,       // 0x19 -> EM (end of medium)
    &specialChar,       // 0x1A -> SUB (substitute)
    &specialChar,       // 0x1B -> ESC (escape)
    &specialChar,       // 0x1C -> FS (file separator)
    &specialChar,       // 0x1D -> GS (group separator)
    &specialChar,       // 0x1E -> RS (record separator)
    &specialChar,       // 0x1F -> US (unit separator)

    // 0x20 - 0x2F -> ' !"#$%&'()*+´-./'
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,

    // 0x30 - 0x3F -> '0123456789:;<=>?'
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,

    // 0x40 - 0x4F -> '@ABCDEFGHIJKLMNO'
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,

    // 0x50 - 0x5F -> 'PQRSTUVWXYZ[\]^_'
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,

    // 0x60 - 0x6F -> '`abcdefghijklmno'
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,

    // 0x70 - 0x7E -> 'pqrstuvwxyz{|}~'
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar,

    &specialChar,       // 0x7F -> DEL

    // 0x80 - 0x8F  -> extended characters
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,

    // 0x90 - 0x9F  -> extended characters
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,

    // 0xA0 - 0xAF  -> extended characters
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,

    // 0xB0 - 0xBF  -> extended characters
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,

    // 0xC0 - 0xCF  -> extended characters
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,

    // 0xD0 - 0xDF  -> extended characters
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,

    // 0xE0 - 0xEF  -> extended characters
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,

    // 0xF0 - 0xFF  -> extended characters
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar,
    &printableChar, &printableChar, &printableChar, &printableChar
};*/

//#endif // _WIN32 not defined

