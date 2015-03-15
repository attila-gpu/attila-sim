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

#include "SPTConsole.h"
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <limits>

using namespace gpu3d;
using namespace std;

string SPTConsole::parseParams(const vector<string>& params)
{
    if ( params.empty() )
        return string();
        
    vector<string>::const_iterator it = params.begin();
    string cmd = *it;
    ++it;
    for ( ; it != params.end(); ++it )
        cmd = cmd + " " + *it;

    pendingCommands.push_back(cmd);
}

SPTConsole::SPTConsole( const SimParameters& simP, const char* name) 
    : ShaderProgramTestBase(simP), quitConsole(false)
{}

bool SPTConsole::finished()
{
    return quitConsole;
}

bool SPTConsole::processCompileProgramCommand(const vector<string>& params)
{
    vector<string>::const_iterator it = params.begin();
    string programString = string("");
    bool optimize = false;
    bool transform = false;
    compileModel compModel = OGL_ARB_1_0;
    bool definedCompileTarget = false;
    bool definedProgram = false;

    if (params.empty())
    {
        cout << "Insufficient parameters found" << endl;
        return false;
    }

    while (it != params.end())
    {
        if (!(*it).compare(0, 2, "-f")) // filename
        {   
            // Extract filename
            it++;

            if (it == params.end())
            {
                cout << "Error: Missing program filename" << endl;
                return false;
            }

            // Open filename
            ifstream f((*it).c_str());

            if ( !f ) {
                cout << "Error: Program filename does not exist or could not be opened" << endl;
                return false;
            }

            //  Read file contents
            string line;
            while ( !f.eof() ) 
            {
                getline(f, line);
                line.append("\n");
                programString.append(line);
            }
            
            definedProgram = true;
        }
        else if (!(*it).compare(0, 4, "-opt")) // optimization flag
        {
            optimize = true;
        }
        else if (!(*it).compare(0, 7, "-target")) // compilation target
        {
            definedCompileTarget = true;
            // Extract compilation target.
            it++;

            if (it == params.end())
            {
                cout << "Error: Compilation target not defined" << endl;
                return false;
            }

            if (!(*it).compare(0, 7, "ARB_1_0"))
            {
                compModel = OGL_ARB_1_0;
            }
            else if (!(*it).compare(0, 6, "SM_2_0"))
            {  
                compModel = D3D_SHADER_MODEL_2_0;
            }
            else if (!(*it).compare(0, 6, "SM_3_0"))
            {
                compModel = D3D_SHADER_MODEL_3_0;
            }
            else
            {
                cout << "Error: Unknown compilation target" << endl;
                return false;
            }
        }
        else // program string
        {
            programString.assign(*it);
            definedProgram = true;
        }

        it++;
    }

    if (!definedCompileTarget) 
    {
        cout << "Error: Compilation target not defined" << endl;
        return false;
    }

    if (!definedProgram)
    {
        cout << "Error: Program not defined" << endl;
        return false;
    }

    compileProgram(programString, optimize, transform, compModel);

    return true;  
}

bool SPTConsole::processDefineProgramInputCommand(const vector<string>& params)
{
    unsigned int inputIndex = SPT_INVALID_INTEGER;
    float inputValue[4];

    for (unsigned int i = 0; i < 4; i++)
        inputValue[i] = numeric_limits<float>::quiet_NaN();
    
    if (params.empty())
    {
        cout << "Error: Input index not defined" << endl;
        return false;
    }

    stringstream is(params.front());
    is >> inputIndex;

    if ( inputIndex == SPT_INVALID_INTEGER ) 
    {
        cout << "Error: Input index must be a non-negative integer" << endl;
        return false;
    }

    if ( inputIndex > 32 )
    {
        cout << "Error: Input index range is [0..31]" << endl;
        return false;
    }

    if (params.size() < 5)
    {
        cout << "Error: A input value is defined by four 32-bit floats" << endl;
        return false;
    }

    for (unsigned int i = 0; i < 4; i++)
    {
        stringstream vs(params[i+1]);
        vs >> inputValue[i];

        if (inputValue[i] == numeric_limits<float>::quiet_NaN())
        {
            cout << "Error: Not a float number defined in component " << i << endl;
            return false;
        }
    }

    defineProgramInput(inputIndex, inputValue);

    return true;
}

bool SPTConsole::processDefineProgramConstantCommand(const vector<string>& params)
{
    unsigned int constantIndex = SPT_INVALID_INTEGER;
    float constantValue[4];

    for (unsigned int i = 0; i < 4; i++)
        constantValue[i] = numeric_limits<float>::quiet_NaN();
    
    if (params.empty())
    {
        cout << "Error: Constant index not defined" << endl;
        return false;
    }

    stringstream idx(params.front());
    idx >> constantIndex;

    if ( constantIndex == SPT_INVALID_INTEGER ) 
    {
        cout << "Error: Constant index must be a non-negative integer" << endl;
        return false;
    }

    if ( constantIndex > 255 )
    {
        cout << "Error: Constant index range is [0..255]" << endl;
        return false;
    }

    if (params.size() < 5)
    {
        cout << "Error: A constant value is defined by four 32-bit floats" << endl;
        return false;
    }

    for (unsigned int i = 0; i < 4; i++)
    {
        stringstream value(params[i+1]);
        value >> constantValue[i];

        if (constantValue[i] == numeric_limits<float>::quiet_NaN())
        {
            cout << "Error: Not a float number defined in component " << i << endl;
            return false;
        }
    }

    defineProgramConstant(constantIndex, constantValue);

    return true;
}

bool SPTConsole::processExecuteProgramCommand(const vector<string>& params)
{
    if (params.empty())
    {
        //  Execute program until the end.
        executeProgram(-1);
    }
    else
    {
        unsigned int stopPC = SPT_INVALID_INTEGER;
        stringstream ss(params.front());
        ss >> stopPC;

        if ( stopPC == SPT_INVALID_INTEGER ) {
            cout << " Stop PC must be a non-negative integer" << endl;
            return false;
        }

        executeProgram((int)stopPC);
    }
    return true;
}

bool SPTConsole::processQuitCommand(const vector<string>& params)
{
    quitConsole = true;
    cout << "Bye!" << endl;
    return true;
}

bool SPTConsole::processHelpCommand(const vector<string>& params)
{
    static const char* help = 
        "Available commands\n"
        "   help: shows this help\n"
        "   quit: exits the console\n"
        "   compile {\"program string\"|-f file} [-opt] -target {ARB_1_0, SM_2_0, SM_3_0}':" 
        "      compiles and loads a shader program (-opt: applies optimizations)\n"
        "   input {i0..i32} float_value: defines input attribute for shader execution\n"
        "   execute [instruction]: entirely or partially executes the shader program\n"
        "   script file: executes the commands from the specified input file\n"
        "   dump filename: dumps currently loaded program into an output file\n"
        "   transform filename: applies the microtriangle shader transformation\n\ton the program of the specified file";

    cout << help << endl;
    return true;
}

bool SPTConsole::processScriptCommand(const vector<string>& params)
{
    if ( params.empty() ) {
        cout << " Error: SCRIPT command requires a path specifying the script to be run" << endl;
        return false;
    }

    ifstream f(params[0].c_str());

    if ( !f ) {
        cout << "Error: input file passed to SCRIPT command does not exist or could not be opened" << endl;
        return false;
    }

    string line;
    while ( f ) {
        getline(f, line);
        pendingCommands.push_back(line);
    }


    return true;
}

bool SPTConsole::processDumpProgramCommand(const vector<string>& params)
{
    if ( params.empty() ) {
        cout << " Error: dump command requires a output filename" << endl;
        return false;
    }

    ofstream f(params[0].c_str());

    if ( !f ) {
        cout << "Error: could not open output file to write" << endl;
        return false;
    }

    dumpProgram(f);

    f.close();

    return true;
}

void SPTConsole::printWelcomeMessage() const
{
    cout << "ShaderProgramTest Console.\n Enter Command.\n HELP for list of available commands." << endl;
}

string SPTConsole::getPrompt() const
{
    stringstream ss;
    ss << "SPTConsole>";
    return ss.str();
}

void SPTConsole::execute() {
    
    if ( quitConsole )
        return ;

    bool loop = true;

    while ( loop ) 
    {
        string inputLine;

        if ( !pendingCommands.empty() ) {
            inputLine = pendingCommands.front();
            pendingCommands.pop_front();
            cout << getPrompt() << inputLine << endl;
        }
        else {
            cout << getPrompt();
            getline(cin, inputLine);
        }

        vector<string> cmdParams;
        string token; // aux string

        // Tokenize input line
        stringstream ss(inputLine);
        string cmd;
        ss >> cmd;
        while ( ss >> token ) {
            cmdParams.push_back(token);
        }
        
        if ( cmd.empty() )
            continue;

        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

        if ( cmd == "COMPILE" || cmd == "P" )
            processCompileProgramCommand(cmdParams);
        else if ( cmd == "INPUT" || cmd == "I" )
            processDefineProgramInputCommand(cmdParams);
        else if ( cmd == "CONSTANT" || cmd == "C" )
            processDefineProgramConstantCommand(cmdParams);
        else if ( cmd == "EXECUTE" || cmd == "E" )
            processExecuteProgramCommand(cmdParams);
        else if ( cmd == "QUIT" || cmd == "Q" )
            loop = !processQuitCommand(cmdParams);
        else if ( cmd == "HELP" || cmd == "H" )
            processHelpCommand(cmdParams);
        else if ( cmd == "SCRIPT" || cmd == "S" )
            processScriptCommand(cmdParams);
        else if ( cmd == "DUMP" || cmd == "D" )
            processDumpProgramCommand(cmdParams);
        else
            cout << " Unsupported '" << cmd << "' command (command ignored!)" << endl;
    }
}