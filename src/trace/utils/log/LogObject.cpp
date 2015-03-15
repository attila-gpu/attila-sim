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

#include "LogObject.h"
#include <ios>
#include <sstream>
#include "support.h"

using std::string;
using std::size_t;
using std::ios_base;

LogObject::LogObject() : levels(0), writeTime(false), autoflush(true), enabled(false)
{}

void LogObject::enable() { enabled = true; }
void LogObject::disable() { enabled = false; }

void LogObject::pushInfo(string curFile, string curFunc, Line line)
{
    if ( enabled ) posInfo.push(PosInfo(curFile, curFunc, line));
}

void LogObject::popInfo()
{
    if ( enabled ) {
        if ( !posInfo.empty() )
            posInfo.pop();
    }
}

bool LogObject::open(string path)
{
    if ( enabled ) {
        if ( o.is_open() )
            o.close(); // close previous file attached to this log object
        o.clear(); // reset previous I/O errors in the stream
        o.open(path.c_str(), ios_base::out | ios_base::trunc | ios_base::binary );
        if ( o.is_open() ) {
            opath = path;
            return true;
        }
        return false;
    }
    return false;
}

bool LogObject::close()
{
    if ( enabled )
    {
        o.close();
        return !o.is_open();
    }
    return false;
}

void LogObject::writeConfig()
{
    if ( enabled && o.is_open() )
    {
        o << "log = " << opath << "\n";
        o << "logtime = " << (writeTime ? "TRUE" : "FALSE") << "\n";
        o << "logfile = " << (writeFile ? "TRUE" : "FALSE") << "\n";
        o << "logfunction = " << (writeFunction ? "TRUE" : "FALSE") << "\n";
        o << "logline = " << (writeLine ? "TRUE" : "FALSE") << "\n";
        //o << "logmask = " << std::hex << levels << std::dec << "\n";
        o << "logmask = " << levels << "\n";
        if ( autoflush )
            o.flush();
    }
}

bool LogObject::config(string path)
{
    if ( !enabled )
        return false;

    string tmp_log;
    bool tmp_log_flag = false;
    bool tmp_logtime = false;
    bool tmp_logfile = false;
    bool tmp_logfunction = false;
    bool tmp_logline = false;
    LevelMask tmp_logmask = 0;

    std::ifstream conf;
    conf.open(path.c_str(), ios_base::in);
    if ( !conf.is_open() )
        return false;

    // line buffer
    char buffer[1024];
    
    while ( !conf.eof() )
    {
        conf.getline(buffer, sizeof(buffer));
        string line(buffer);
        if ( line.empty() )
            continue;
        line = line.substr(line.find_first_not_of(" \t")); // remove initial whites
        if ( line.empty() )
            continue;
        if ( *line.begin() == '#' )
            continue; // skip comment line
        size_t pos = line.find_first_of(" \t=");
        if ( pos == string::npos ) continue;
        string param = line.substr(0, pos);
        if ( param != "log" && param != "logtime" && param != "logfile"
          && param != "logfunction" && param != "logline" && param != "logmask" )
          continue; // next loop plz

        // Valid param found (if any param value is wrong, ignore config [return false])
        line = line.substr(pos);
        pos = line.find_first_not_of(" \t");
        if ( pos == string::npos || line[pos] != '=' ) return false;
        line = line.substr(pos+1); // chop blanks and '='
        if ( line.empty() ) return false;
        pos = line.find_first_not_of(" \t");
        if ( pos == string::npos ) return false;
        line = line.substr(pos); // trim whites preceding param value
        pos = line.find_last_not_of(" \t");
        if ( pos == string::npos ) return false;
        string value = line.substr(0, pos+1);
        if ( value.empty() ) return false;

        if ( param == "log" ) {
            tmp_log = value;
            tmp_log_flag = true; // log destination file found (mark tmp_log as valid)
        }
        else if ( param == "logtime" ) { tmp_logtime = (value == "0" ? false : true); }
        else if ( param == "logfile" ) { tmp_logfile = (value == "0" ? false : true); }
        else if ( param == "logfunction" ) { tmp_logfunction = (value == "0" ? false : true); }
        else if ( param == "logline" ) { tmp_logline = (value == "0" ? false : true); }
        else if ( param == "logmask" ) {
            if ( value.length() < 2 ) return false;
            if ( value[0] != '0' || (value[1] != 'x' && value[1] != 'X') ) return false;
            std::stringstream ss(value.substr(2));
            ss >> std::hex >> tmp_logmask;
        }
        else return false; // should not happen        
    }

    if ( tmp_log_flag && !LogObject::open(tmp_log) )
        return false;
    
    writeTime = tmp_logtime;
    writeFile = tmp_logfile;
    writeFunction = tmp_logfunction;
    writeLine = tmp_logline;
    levels = tmp_logmask;

    return true;
}

void LogObject::enableLevels(LevelMask lMask)
{
    if ( enabled ) {
        levels = lMask;
    }
}


bool LogObject::write(LevelMask l, const char* message, bool dumpLogExtras)
{
    if ( enabled )
        return write(l, string(message), dumpLogExtras);
    return false;
}

bool LogObject::write(LevelMask l, const string& message, bool dumpLogExtras )
{
    if ( enabled )
    {
        if ( (l & levels) != 0 ) {
            if ( dumpLogExtras ) {
                if (!posInfo.empty())
                {
                    PosInfo& pInf = posInfo.back();
                    if ( writeTime )
                        o << "NO_TIME_AVAIL:";
                    if ( writeFile )
                        o << pInf.file << ":";
                    if ( writeFunction )
                        o << pInf.func << ":";
                    if ( writeLine )
                        o << pInf.line << ":";
                }
            }
            o << message;
            if ( autoflush ) { o.flush(); }
        }
        return o.is_open();
    }
    return false;
}

void LogObject::logTime(bool yes) 
{ 
    if ( enabled ) writeTime = yes; 
}

void LogObject::logFile(bool yes) 
{ 
    if ( enabled ) writeFile = yes; 
}

void LogObject::logFunction(bool yes) 
{ 
    if ( enabled ) writeFunction = yes; 
}

void LogObject::logLine(bool yes) 
{ 
    if ( enabled ) writeLine = yes; 
}
