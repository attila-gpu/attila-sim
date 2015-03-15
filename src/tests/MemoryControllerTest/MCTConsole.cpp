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

#include "MCTConsole.h"
#include "MCSplitter.h"
#include "MCSplitter2.h"
#include "MemorySpace.h"
#include <string>
#include <fstream>


using namespace gpu3d;
using namespace std;
using memorycontroller::MemoryRequestSplitter;
using memorycontroller::MCSplitter;
using memorycontroller::MCSplitter2;

static const char* HelpStr = 
"----------------------------------------------------------\n"
" HELP information\n"
"  Syntax: HELP [COMMAND]\n"
"  help command without params to print the standard help\n"
"  showing the avaiable commands with a brief description\n"
"  Use 'HELP COMMAND' to obtain the extended help \n"
"  of a specific COMMAND\n"
"----------------------------------------------------------\n";

static const char* SendStr =
"----------------------------------------------------------\n"
" SEND information\n"
"  Syntax: SEND TYPE UNIT[SUBUNIT] ADDRESS BYTES [RAW_DATA]\n"
"    or:   READ UNIT[SUBUNIT] ADDRESS BYTES\n"
"    or:   WRITE UNIT[SUBUNIT] ADDRESS BYTES [RAW_DATA]\n"
"  Sends a READ or WRITE command to the memory controller\n"
"  Parameters:\n"
"   * TYPE: Possible values = READ or WRITE\n"
"   * UNIT[SUBUNIT]: Encodes the sender of the request.\n"
"       UNIT can be:  CP or COMMANDPROCESSOR\n"
"                     STREAMERFETCH\n"
"                     STREAMERLOADER\n"
"                     ZSTENCIL or ZSTENCILTEST\n"
"                     COLOR or COLORWRITE\n"
"                     TEX or TEXTURE or TEXTUREUNIT\n"
"       SUBUNIT can be ommited or can be a number indicating\n"
"        the instance of the unit in case the unit is\n"
"        replicated (0 if not specified\n"
"     Examples: ZSTENCIL[3], CP, TEXTURE, COLOR[2],...\n"
"  * ADDRESS: indicates the start address of the request\n"
"    It can be:\n"
"      - Decimal address (ex: 0, 128, 1024, etc)\n"
"      - Hexadecimal address ( ex: 0x0, 0xFFFF, etc)\n"
"      - Channel.Bank.Row.Col (ex: 2.0.128.64)\n"
"        ( this last method does not work with multiple memory\n"
"          address  spaces )\n"
"  * BYTES: Number of bytes to be read or written\n"
"  * RAW_DATA: the data to be written (optional parameter\n"
"----------------------------------------------------------\n";


string MCTConsole::parseParams(const vector<string>& params)
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


MCTConsole::MCTConsole( const SimParameters& simP, 
                        const char** tuPrefixes,
                        const char** suPrefixes,
                        const char** slPrefixes,
                        const char* name,
                        const MemoryControllerProxy& mcProxy,
                        Box* parentBox ) : MemoryControllerTestBase(simP, tuPrefixes, suPrefixes, slPrefixes, name, mcProxy, parentBox),
                        currentCycle(0), startCycle(0), quitConsole(false)
{
    memset(&stats, 0, sizeof(Stats));

    lastData[COMMANDPROCESSOR].push_back(string());
    lastData[STREAMERFETCH].push_back(string());
    for ( u32bit i = 0; i < simP.str.streamerLoaderUnits; ++i )
        lastData[STREAMERLOADER].push_back(string());
    for (u32bit i = 0; i < simP.gpu.numStampUnits; i++ )
        lastData[ZSTENCILTEST].push_back(string());
    for (u32bit i = 0; i < simP.gpu.numStampUnits; i++ )
        lastData[COLORWRITE].push_back(string());
    lastData[DACB].push_back(string());
    const u32bit numTexUnits = simP.gpu.numFShaders * simP.fsh.textureUnits;
    for ( u32bit i = 0; i < numTexUnits;i++ )
        lastData[TEXTUREUNIT].push_back(string());
    
    // Init memory controller splitter if using MCV2
    if ( simP.mem.memoryControllerV2 ) {

        // memorySize = #channels * #banks * #rows * rowSize
        // rowSize = 4 * #cols
        // #rows = memorySize / #channels * #banks * rowSize

        const u32bit splitterType = simP.mem.v2SplitterType;
        const u32bit burstLength = simP.mem.burstLength;
        const u32bit memoryChannels = simP.mem.v2MemoryChannels;
        const u32bit banksPerChannel = simP.mem.v2BanksPerMemoryChannel;
        const u32bit memoryRowSize = simP.mem.v2MemoryRowSize;
        const u32bit gpuMemorySize = simP.mem.memSize;

        if ( splitterType == 0 ) {
            splitter = new MCSplitter
                       (
                           burstLength, memoryChannels, banksPerChannel,
                           gpuMemorySize / (memoryChannels * banksPerChannel * memoryRowSize),
                           memoryRowSize / 4, 
                           simP.mem.v2ChannelInterleaving,
                           simP.mem.v2BankInterleaving
                       );
        }
        else if ( splitterType == 1 ) {
            splitter = new MCSplitter2
                       (
                           burstLength, memoryChannels, banksPerChannel,
                           gpuMemorySize / (memoryChannels * banksPerChannel * memoryRowSize),
                           memoryRowSize / 4, 
                           string(simP.mem.v2ChannelInterleavingMask),                    
                           string(simP.mem.v2BankInterleavingMask)
                       );
        }
        else {
            stringstream ss;
            ss << "Splitter type " << splitterType << " unsupported";
            panic("MCTConsole", "ctor", ss.str().c_str());
        }   
    }
}

bool MCTConsole::finished()
{
    return quitConsole;
}

string MCTConsole::getPrompt() const
{
    stringstream ss;
    ss << "MCTConsole (cycle=" << currentCycle << ")> ";
    return ss.str();
}


bool MCTConsole::processScriptCommand(const vector<string>& params)
{
    if ( params.empty() ) {
        cout << " SCRIPT command usage: SCRIPT script_file_path [repeat_count] (repeat_count is 1 by default, no repeat)" << endl;
        return false;
    }

    ifstream f(params[0].c_str());

    if ( !f ) {
        cout << "Error: input file passed to SCRIPT command does not exist or could not be opened" << endl;
        return false;
    }

    u32bit repeatCount = 0;

    if ( params.size() > 1 ) {
        // script accepts a second parameter indicating 
        stringstream ss(params[1]);
        repeatCount = MC_INVALID_INTEGER;
        ss >> repeatCount;
        if ( repeatCount == MC_INVALID_INTEGER || repeatCount == 0 ) 
        {
            cout << "Error. The 'repeat count' parameter for SCRIPT command must be a positive integer";
            f.close();
            return false;
        }
        if ( repeatCount > 500 ) 
        {
            cout << "Error. 'repeat count' parameter for SCRIPT command cannot be higher than 500";
            f.close();
            return false;
        }
        --repeatCount;
    }

    string line;
    while ( !f.eof() ) {
        getline(f, line);
        pendingCommands.push_back(line);
    }

    if ( repeatCount > 0 ) 
    {
        // Inject a new script command with the new repeat count
        stringstream ss;
        ss << "SCRIPT " << params[0] << " " << repeatCount;
        pendingCommands.push_back( ss.str() );
    }

    f.close();

    return true;
}

bool MCTConsole::processQuitCommand(const vector<string>& params)
{
    quitConsole = true;
    cout << "Exiting MCTConsole (wait) ";
    return true;
}

bool MCTConsole::processRunCommand(const vector<string>& params)
{
    if ( params.empty() ) {
        cout << " Error: RUN command requires a parameter telling the number of cycles running the MC" << endl;
        return false;
    }

    stringstream ss(params[0]);
    u32bit cycles = MC_INVALID_INTEGER;
    ss >> cycles;
    if ( cycles == MC_INVALID_INTEGER ) {
        cout << " Error: The parameter of RUN command must be a positive integer" << endl;
        return false;
    }

    startCycle = currentCycle + cycles;
    cout << "Running " << cycles << " cycles..." << endl;

    return true;
}


bool MCTConsole::processSaveMemoryCommand(const std::vector<std::string>& params)
{
    sendMCCommand( gpu3d::MemoryControllerCommand::createSaveMemory() );
    return true;
}

bool MCTConsole::processLoadMemoryCommand(const std::vector<std::string>& params)
{
    sendMCCommand( gpu3d::MemoryControllerCommand::createLoadMemory() );
    return true;
}



bool MCTConsole::processHelpCommand(const vector<string>& params)
{
    static const char* help = 
        "----------------------------------------------------------------\n"
        "  Memory Controller Console Help (commands are case-insensitive)\n"
        "----------------------------------------------------------------\n"
		" 'Help'   : Shows this help or the extended help for one command\n" 
		"            (type \"help help\" for further information)\n"
        " 'Quit'   : Exits the console and finishes the simulation\n"
        " 'Send'   : Sends a read or write request to the MC\n"
        " 'Read'   : Expands to 'SEND READ'\n"
        " 'Write'  : Expands to 'SEND WRITE'\n"
        " 'Run'    : Simulates a given number of cycles\n"
        " 'Loadmem': Loads the GDDR memory's contents from a file\n"
        " 'Savemem': Stores the GDDR memory's contents into a file\n"
        " 'Script' : Executes the commands in an input text file\n"
        "            'repeat' param can be used to execute it N times\n";
        " 'Arch'   : Prints information about the current\n"
        "            memory controller architecture simulated\n"
        " 'Stats'  : Prints basic stats about read/write transactions\n"
        " 'MCDebug': Outputs the current debug information of the MC\n"
        " 'Print'  : Outputs data of the last last MT received by a unit\n"
        "----------------------------------------------------------------\n";
    
	if ( params.size() > 0 ) {
		string cmd = params[0];
		std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
		// (memory is expected to be linear)
		if ( cmd == "HELP" )
            cout << HelpStr;
		else if ( cmd == "SEND" || cmd == "READ" || cmd == "WRITE" )
            cout << SendStr;
        else
            cout << "NO EXTENDED INFORMTATION FOR COMMAND: " << cmd << "\n";
    }
	else
	    cout << help << endl;

    return true;
}

bool MCTConsole::processReadCommand(const std::vector<std::string>& params)
{
    vector<string> newparams;
    newparams.push_back("READ");
    newparams.insert( newparams.end(), params.begin(), params.end() );
    return processSendCommand(newparams);
}

bool MCTConsole::processWriteCommand(const std::vector<std::string>& params)
{
    vector<string> newparams;
    newparams.push_back("WRITE");
    newparams.insert( newparams.end(), params.begin(), params.end() );
    return processSendCommand(newparams);
}


bool MCTConsole::processPrintCommand(const std::vector<std::string>& params)
{
    if ( params.size() != 3 ) {
        cout << " PRINT command requires 3 parameters: unit[subunit] startByte byteCount\n";
        return false;
    }

    string unitName = params[0];

    u32bit unit = MC_INVALID_INTEGER;
    u32bit subunit = MC_INVALID_INTEGER;

    parseUnitName(unitName, unit, subunit);

    if ( unit == MC_INVALID_INTEGER ) {
        cout << " Unknown unit '" << unitName << "'" << endl;
        return false;
    }
    if ( subunit == MC_INVALID_INTEGER ) {
        cout << " Invalid subunit identifier" << endl;
        return false;
    }
    if ( subunit >= lastData[unit].size() ) {
        cout << " Invalid subunit identifier (too high)" << endl;
        return false;
    }
    
    u32bit startByte = MC_INVALID_INTEGER;
    u32bit byteCount = MC_INVALID_INTEGER;

    stringstream ss1(params[1]);
    stringstream ss2(params[2]);

    ss1 >> startByte;
    ss2 >> byteCount;

    if ( startByte == MC_INVALID_INTEGER ) {
        cout << " Start byte must be a non-negative integer" << endl;
        return false;
    }
    if ( byteCount == MC_INVALID_INTEGER ) {
        cout << " Byte count must be a non-negative integer" << endl;
        return false;
    }

    string& data = lastData[unit][subunit];

    if ( data.empty() ) {
        cout << "No data from unit: " << unitName << " has been received yet" << endl;
        return false;
    }
   

    if ( startByte >= data.size() - 1 ) {
        cout << " Start byte must be lower than the last data read" << endl;
        return false;
    }

    if ( startByte + byteCount > data.size() ) {
        cout << " Start byte + amount of data cannot be bigger than last data read" << endl;
        return false;
    }

    cout << "-------------------------------------------------------\n";
    cout << "OUTPUT: " << endl << data.substr(startByte, byteCount) << endl;
    cout << "-------------------------------------------------------\n";

    return true;

}

bool MCTConsole::processSendCommand(const vector<string>& params)
{
    if ( params.size() < 4 ) {
        cout << " SEND command requires at least 4 parameters (type,unit[subunit],@,bytes and optionally raw_data)\n"
                " Type help SEND for detailed info about SEND command" << endl;
        return false;
    }

    string token = params[0]; // Type of transaction
    std::transform(token.begin(), token.end(), token.begin(), ::toupper);
    
    RequestType rt;
    if ( token == "READ" || token == "RD" || token == "R" )
        rt = READ_REQUEST;
    else if ( token == "WRITE" || token == "WR" || token == "W" )
        rt = WRITE_REQUEST;
    else {
        cout << " Unsupported transaction type: '" << params[0] << "'" << endl;
        return false;
    }    

    u32bit unit, subunit;

    string unitName = parseUnitName(params[1], unit, subunit);

    if ( unit == MC_INVALID_INTEGER ) {
        cout << " Unknown unit '" << unitName << "'" << endl;
        return false;
    }
    if ( subunit == MC_INVALID_INTEGER ) {
        cout << " Invalid subunit identifier" << endl;
        return false;
    }

    u32bit address = parseAddress(params[2]);
    if ( address == MC_INVALID_INTEGER )
        // cout << " Address must be a non-negative integer" << endl;
        return false;

    stringstream ss(params[3]);

    u32bit bytes = MC_INVALID_INTEGER;
    ss >> bytes;
    if ( bytes == MC_INVALID_INTEGER ) {
        cout << " Bytes must be a positive integer" << endl;
        return false;
    }

    if ( params.size() > 4 )
        sendRequest(static_cast<GPUUnit>(unit), subunit, rt, bytes, address, (u8bit*)params[4].c_str());
    else
        sendRequest(static_cast<GPUUnit>(unit), subunit, rt, bytes, address, 0);  

    return true;
}

u32bit MCTConsole::parseAddress(string addressStr)
{
    u32bit dots = static_cast<u32bit>(std::count(addressStr.begin(), addressStr.end(), '.'));
    if ( dots == 0 ) {
        u32bit hasx = static_cast<u32bit>(std::count(addressStr.begin(), addressStr.end(), 'x'));
        u32bit hasX = static_cast<u32bit>(std::count(addressStr.begin(), addressStr.end(), 'X'));
        if ( hasx + hasX == 1 ) {
            // hex value expected
            if ( addressStr.length() < 2 ) {
                cout << " Hexadecimal address incomplete" << endl;
                return MC_INVALID_INTEGER;
            }
            if ( addressStr.at(0) != '0' || (addressStr.at(1) != 'x' && addressStr.at(0) != 'X') ) {
                cout << " Hexadecimal format erroneous" << endl;
                return MC_INVALID_INTEGER;
            }
            stringstream ss(addressStr);
            u32bit address = MC_INVALID_INTEGER;
            ss >> std::hex >> address >> std::dec;
            if ( address == MC_INVALID_INTEGER ) {
                cout << " Address must be a non-negative hexadecimal integer" << endl;
                return MC_INVALID_INTEGER;
            }            
            return address;
        }
        else {
            // decimal value expected
            u32bit address = MC_INVALID_INTEGER;
            stringstream ss(addressStr);
            ss >> hex >> address >> dec;
            if ( address == MC_INVALID_INTEGER ) {
                cout << " Address must be a non-negative integer" << endl;
                return MC_INVALID_INTEGER;
            }
            return address;
        }
        
    }
    else if ( dots == 3 ) {

        if ( !SimParams().mem.memoryControllerV2 ) {
            cout << "Error. Address in format Channel.Bank.Row.StartColumn can only be used when using MCV2\n";
            return MC_INVALID_INTEGER;
        }

        if ( SimParams().mem.v2SecondInterleaving ) {
            cout << "Error. Address in format Channel.Bank.Row.StartColumn cannot be used when SecondInterleaving is enabled\n";
            return MC_INVALID_INTEGER;
        }
        
        stringstream ss(addressStr);
        string channelStr, bankStr, rowStr, colStr;
        
        getline(ss, channelStr, '.');
        getline(ss, bankStr, '.');
        getline(ss, rowStr, '.');
        getline(ss, colStr, '.');

        u32bit channel = MC_INVALID_INTEGER;
        u32bit bank = MC_INVALID_INTEGER;
        u32bit row = MC_INVALID_INTEGER;
        u32bit col = MC_INVALID_INTEGER;

        stringstream ss2(channelStr);
        ss2 >> channel;
        ss2.clear();
        ss2.str(bankStr);
        ss2 >> bank;
        ss2.clear();
        ss2.str(rowStr);
        ss2 >> row;
        ss2.clear();
        ss2.str(colStr);
        ss2 >> col;

        MemoryRequestSplitter::AddressInfo ai;
        ai.channel = channel;
        ai.bank = bank;
        ai.row = row;
        ai.startCol = col;

        // make up address
        return splitter->createAddress(ai);
    }
    else {
        cout << "Error parsing address of type \"Channel.Bank.Page.Column\". 3 dots required and " << dots << " dots were found\n";
        return MC_INVALID_INTEGER;
    }

}

bool MCTConsole::processStatsCommand(const vector<string>& params)
{
    cout << "----------------------------------------------------------\n";
    cout << "STATISTICS\n";
    cout << "   GPU local memory\n";
    cout << "       Read requests sent: " << stats.gpuReadRequestsSent << "\n";
    cout << "       Read requests received: " << stats.gpuReadRequestsReceived << "\n";
    cout << "       Write requests sent: " << stats.gpuWriteRequestsSent << "\n";
    cout << "   SYSTEM memory\n";
    cout << "       Read requests sent: " << stats.systemReadRequestsSent << "\n";
    cout << "       Read requests received: " << stats.systemReadRequestsReceived << "\n";
    cout << "       Write requests sent: " << stats.systemWriteRequestsSent << "\n";
    cout << "-----------------------------------------------------------\n";
    return true;
}

bool MCTConsole::processMCDebugCommand(const std::vector<std::string>& params)
{
    cout << "----------------------------------------------------------\n";
    cout << "MEMORY CONTROLLER DEBUG INFO\n";
    cout << getMCProxy().getDebugInfo() << "\n";
    cout << "----------------------------------------------------------\n";
    return true;
}

bool MCTConsole::processArchCommand(const vector<string>& params)
{
    const MemParameters& mem = SimParams().mem;
    cout << "----------------------------------------------------------\n";
    cout << "GPU Global Architecture\n";
    cout << "   GPU clock (gclock): " << SimParams().gpu.gpuClock << " MHz\n";
    cout << "   Memory Controller clock (mclock): " << SimParams().gpu.memoryClock << " MHz\n";
    cout << "   System Address Space (start address): 0x" << hex << SYSTEM_ADDRESS_SPACE << dec << "\n";
    cout << "----------------------------------------------------------\n";
    cout << "GPU to/from MEMORY BUSES\n";
    for ( u32bit i = 0; i < LASTGPUBUS; ++i )
    {
        if ( mem.memoryControllerV2 && i == MEMORYMODULE )
            continue; // Ignore legacy identifier

        cout << "   "
             << MemoryTransaction::getBusName(static_cast<GPUUnit>(i)) << " -> BW: " 
             << MemoryTransaction::getBusWidth(static_cast<GPUUnit>(i)) << " bytes/cycle\n";
    }
    cout << "----------------------------------------------------------\n";
    if ( mem.memoryControllerV2 ) {
        cout << "Memory Controller V2 Architecture\n";
        cout << "   Total local memory: " << mem.memSize << " MB\n";
        cout << "   Total system (mapped) memory: " << mem.mappedMemSize << " MB\n";
        cout << "   Memory Channels: " << mem.v2MemoryChannels << "\n";
        cout << "   Banks per channel: " << mem.v2BanksPerMemoryChannel << "\n";
        cout << "   Page (row) size: " << mem.v2MemoryRowSize << " bytes\n";
        cout << "   Burst Length: " << mem.burstLength << " datums of 32-bit\n";
        cout << "   Bytes per cycle (mclock): " << mem.v2BurstBytesPerCycle;      
        
        if ( mem.v2BurstBytesPerCycle == 8 )
            cout << " (DDR)";
        else if ( mem.v2BurstBytesPerCycle == 16 )
            cout << " (QDR)";

        cout << "\n    (one burst requires " << (4 * mem.burstLength / mem.v2BurstBytesPerCycle) << " cycles)\n";

        if ( mem.v2SplitterType == 0 ) {
            cout << "   Channel interleaving: " << mem.v2ChannelInterleaving;
            if ( mem.v2SecondInterleaving )
                cout << ". [Second channel interleaving: " << mem.v2SecondChannelInterleaving << "]";
            cout << "\n";
            cout << "   Bank interleaving: " << mem.v2BankInterleaving;
            if ( mem.v2SecondInterleaving )
                cout << ". [Second bank interleaving: " << mem.v2SecondBankInterleaving << "]";
            cout << "\n";
        }
        else if ( mem.v2SplitterType == 1 ) {
            cout << "   Channel interleaving: " << string(mem.v2ChannelInterleavingMask);
            if ( mem.v2SecondInterleaving )
                cout << ". [Second channel interleaving: " << mem.v2SecondChannelInterleavingMask << "]";
            cout << "\n";
            cout << "   Bank interleaving: " << string(mem.v2BankInterleavingMask);
            if ( mem.v2SecondInterleaving )
                cout << ". [Second bank interleaving: " << mem.v2SecondBankInterleavingMask << "]";
            cout << "\n";
        }
        else {
            cout << "   Channel interleaving: UNKNOWN\n";
            cout << "   Bank interleaving: UNKNOWN\n";
        }

        cout << "----------------------------------------------------------\n";
    }
    else {
        cout << "Legacy Memory Controller Architecture\n";
        cout << "   No info available (not implemented for legacy MC)\n";
        cout << "----------------------------------------------------------\n";
    }

    return true;
}

string MCTConsole::parseUnitName(string unitName, u32bit& unit, u32bit& subunit)
{    
    size_t openBracket  = unitName.find_last_of("[");
    size_t closeBracket = unitName.find_last_of("]");

    unit = MC_INVALID_INTEGER;
    subunit = MC_INVALID_INTEGER;

    if ( openBracket != string::npos && closeBracket != string::npos && openBracket + 1 < closeBracket ) {
        // subunit properly specified        
        stringstream temp(unitName.substr(openBracket + 1, closeBracket - openBracket - 1));
        unitName = unitName.substr(0, openBracket);
        temp >> subunit;
        if ( subunit == MC_INVALID_INTEGER )
            return unitName;
    }
    else
        subunit = 0;

    std::transform(unitName.begin(), unitName.end(), unitName.begin(), ::toupper);

    if ( unitName == "CP" || unitName == "COMMANDPROCESSOR" )
        unit = COMMANDPROCESSOR;
    else if ( unitName == "STREAMERFETCH" )
        unit = STREAMERFETCH;
    else if ( unitName == "STREAMERLOADER" )
        unit = STREAMERLOADER;
    else if ( unitName == "ZSTENCIL" || unitName == "ZSTENCILTEST" )
        unit = ZSTENCILTEST;
    else if ( unitName == "COLOR" || unitName == "COLORWRITE" )
        unit = COLORWRITE;
    else if ( unitName == "DAC" || unitName == "DACB" )
        unit = DACB;
    else if ( unitName == "TEX" || unitName == "TEXTURE" || unitName == "TEXTUREUNIT" )
        unit = TEXTUREUNIT;
    
    return unitName;
}



void MCTConsole::clock_test(u64bit cycle)
{
    currentCycle = cycle;

    if ( quitConsole )
        return ;

    if ( cycle < startCycle )
        return ; // keep clocking...

    bool loop = true;

    while ( loop ) {

        string inputLine = "";

        if ( !pendingCommands.empty() ) {
            inputLine = pendingCommands.front();
            cout << getPrompt() << inputLine << endl;
            pendingCommands.pop_front();
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
        while ( ss >> token )
            cmdParams.push_back(token);
        
        if ( cmd.empty() )
            continue;

        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

        if ( cmd == "SEND" || cmd == "S" )
            processSendCommand(cmdParams);
        else if ( cmd == "LOADMEM" )
            processLoadMemoryCommand(cmdParams);
        else if ( cmd == "SAVEMEM" )
            processSaveMemoryCommand(cmdParams);
        else if ( cmd == "RUN" )
            loop = !processRunCommand(cmdParams);
        else if ( cmd == "QUIT" || cmd == "Q" )
            loop = !processQuitCommand(cmdParams);
        else if ( cmd == "HELP" || cmd == "H" )
            processHelpCommand(cmdParams);
        else if ( cmd == "SCRIPT" || cmd == "SC" )
            processScriptCommand(cmdParams);
        else if ( cmd == "ARCH" || cmd == "A" )
            processArchCommand(cmdParams);
        else if ( cmd == "STATS" )
            processStatsCommand(cmdParams);
        else if ( cmd == "MCDEBUG" )
            processMCDebugCommand(cmdParams);
        else if ( cmd == "PRINT" )
            processPrintCommand(cmdParams);
        else if ( cmd == "READ" || cmd == "R" )
            processReadCommand(cmdParams);
        else if ( cmd == "WRITE" || cmd == "W" )
            processWriteCommand(cmdParams);
        else
            cout << " Unsupported '" << cmd << "' command (command ignored!)" << endl;
    }
}

void MCTConsole::handler_receiveTransaction(u64bit cycle, MemoryTransaction* mt)
{
    if ( mt->isToSystemMemory() )
        ++stats.systemReadRequestsReceived;
    else
        ++stats.gpuReadRequestsReceived;

    GPUUnit unit = mt->getRequestSource();
    switch ( unit ) 
    {
        case COMMANDPROCESSOR:
        case STREAMERFETCH:
        case STREAMERLOADER:
        case ZSTENCILTEST:
        case COLORWRITE:
        case DACB:
        case TEXTUREUNIT:
            break;
        default:
            panic("MTConsole", "handler_receiveTransaction", "Unknown GPU unit identifier!");
    }

    u32bit subunit = mt->getUnitID();
    if ( subunit >= lastData[unit].size() ) 
        panic("MTConsole", "handler_receiveTransaction", "Sub-unit ID too high");
    

    lastData[unit][subunit] = string(reinterpret_cast<char*>(mt->getData()), mt->getSize());


    MemoryControllerTestBase::handler_receiveTransaction(cycle, mt);
}

void MCTConsole::handler_sendTransaction(u64bit cycle, MemoryTransaction* mt)
{
    if ( mt->isToSystemMemory() ) {
        switch ( mt->getCommand() ) {
            case MT_READ_REQ:
                ++stats.systemReadRequestsSent;
                break;
            case MT_WRITE_DATA:
                ++stats.systemWriteRequestsSent;
                break;
            default:
                panic("MCTConsole", "handler_sendTransaction", "Unnexpected type of request sent!");
        }
    }
    else {
        switch ( mt->getCommand() ) {
            case MT_READ_REQ:
                ++stats.gpuReadRequestsSent;
                break;
            case MT_WRITE_DATA:
                ++stats.gpuWriteRequestsSent;
                break;
            default:
                panic("MCTConsole", "handler_sendTransaction", "Unnexpected type of request sent!");
        }
    }
    MemoryControllerTestBase::handler_sendTransaction(cycle, mt);
}


