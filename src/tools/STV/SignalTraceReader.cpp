#include "SignalTraceReader.h"
#include <windows.h>
#include <iostream>
#include <sstream>
#include <QMessageBox.h>

// #define ENABLE_STR_DEBUG
#ifdef ENABLE_STR_DEBUG
    #define STR_DEBUG(x) { x }
#else
    #define STR_DEBUG(x) {}
#endif

using namespace std;

// #define MSG(title,msg) (cout << (title) << " -> " << (msg));// MessageBoxA( NULL, msg, title, MB_OK );
#define STR_MSG(title,msg) QMessageBox::information(0, title, msg, QMessageBox::Ok);

using namespace std;

SignalTraceReader::SignalTraceReader() : fileName(0) 
{ 
	// empty
}

bool SignalTraceReader::open( const char* filePath )
{
    STR_DEBUG
    (
        stringstream ss;
        ss << "Performing open(\"" << filePath <<"\")";
        STR_MSG("STR_DEBUG: SignalTraceReader::open()", ss.str().c_str());
    )

	if ( fileName )
		delete[] fileName;
	fileName = new char[strlen(filePath) + 1];
	strcpy( fileName, filePath );
    f.open( fileName, ios::binary | ios::in );
	//f.open( fileName, ios::in | ios::binary | ios::nocreate );

    if ( f.is_open() ) {
        STR_DEBUG
        (
            stringstream ss;
            ss << "Input file: '" << filePath << "' correctly opened";
            STR_MSG("STR_DEBUG: SignalTraceReader::open()", ss.str().c_str());
        )
		return true;
    }
    STR_DEBUG
    (
        stringstream ss;
        ss << "Input file: '" << filePath << "' couldn't be opened";
        STR_MSG("STR_DEBUG: SignalTraceReader::open()", ss.str().c_str());
    )
	return false;
}


bool SignalTraceReader::skipLines( int nLines )
{
    STR_DEBUG
    (
        stringstream ss;
        ss << "Performing skipLines(" << nLines <<")";
        STR_MSG("STR_DEBUG: SignalTraceReader::skipLines()", ss.str().c_str());
    )

    if ( !f.is_open() ) {
        STR_DEBUG
        (
            stringstream ss;
            ss << "Completed (file not open!)";
            STR_MSG("STR_DEBUG: SignalTraceReader::skipLines()", ss.str().c_str());
        )
		return false;
    }
	
	char buffer[1024];
	while ( !f.eof() && nLines-- != 0 )
		f.getline( buffer, sizeof(buffer) );
    if ( f.eof() ) {
        STR_DEBUG
        (
            stringstream ss;
            ss << "Completed (file end reached!)";
            STR_MSG("STR_DEBUG: SignalTraceReader::skipLines()", ss.str().c_str());
        )
		return false;
    }
    STR_DEBUG
    (
        stringstream ss;
        ss << "Completed (OK)";
        STR_MSG("STR_DEBUG: SignalTraceReader::skipLines()", ss.str().c_str());
    )

	return true;
}


bool SignalTraceReader::checkTrace( const char* file )
{
    STR_DEBUG
    (
        stringstream ss;
        ss << "Performing checkTrace(\"" << file << "\")";
        STR_MSG("STR_DEBUG: SignalTraceReader::checkTrace()", ss.str().c_str());
    )

	ifstream f;

    f.open(file, ios::binary | ios::in);
	if ( !f )
		return false;

	char buffer[64];
	f.getline(buffer, sizeof(buffer));
	f.close();

    if ( strcmp(buffer, "Signal Trace File v. 1.0") == 0 ) {
        STR_DEBUG
        (
            stringstream ss;
            ss << "Input trace format '" << buffer << "' VALID (exiting checkTrace())";
            STR_MSG("STR_DEBUG: SignalTraceReader::checkTrace()", ss.str().c_str());
        )
		return true;
    }
    STR_DEBUG
    (
        stringstream ss;
        ss << "Input trace format '" << buffer << "' NOT VALID (exiting checkTrace())";
        STR_MSG("STR_DEBUG: SignalTraceReader::checkTrace()", ss.str().c_str());
    )

	return false;
	
}
/**
 * Important. Only work with files containing cycles from [N..M], with no gaps in the range 
 */ 
int SignalTraceReader::countCyclesInfo( const char* filePath, int& firstCycle )
{	

    STR_DEBUG
    (
        stringstream ss;
        ss << "Performing countCyclesInfo(" << filePath << "," << firstCycle << ")";
        STR_MSG("STR_DEBUG: SignalTraceReader::countCyclesInfo()", ss.str().c_str());
    )

	ifstream f;

    f.open(filePath, ios::binary | ios::in);

	if ( !f )
		return -1;
	
	firstCycle = 0;
	char c;

	if ( !f.eof() )
		f.get(c);
    else {
        STR_DEBUG
        (
            stringstream ss;
            ss << "Error finding the first cycle in the selected Signal Trace file";
            STR_MSG("STR_DEBUG: SignalTraceReader::countCyclesInfo()", ss.str().c_str());
        )
		return -1; // error finding first cycle
    }
	
	/*
	 * Search first cycle
	 */
	while ( !f.eof() ) {
		
		if ( c == 'C' )
		{
			f.get(c);
			if ( f.eof() )
				return -1;
			if ( c == ' ' )
			{
				c = (char)f.peek();
				if ( '0' <= c && c <= '9' ) // pattern found
				{
					f >> firstCycle;
					break; // finish loop
				}
			}
			

		}
		f.get(c);
	}

    STR_DEBUG
    (
        stringstream ss;
        ss << "First cycle found: " << firstCycle << ". Position aprox. in file: " << f.tellg();
        STR_MSG("STR_DEBUG: SignalTraceReader::countCyclesInfo()", ss.str().c_str());
    )

	/*
	 * Search last cycle
	 */ 
    f.seekg(-1, ios_base::end);

	int cycle = -1;

    STR_DEBUG
    (
        stringstream ss;
        ss << "Before tellg()";
        STR_MSG("STR_DEBUG: SignalTraceReader::countCyclesInfo()", ss.str().c_str());
    )
    STR_DEBUG
    (
        stringstream ss;
        ss << "tellg() returns: " << f.tellg();
        STR_MSG("STR_DEBUG: SignalTraceReader::countCyclesInfo()", ss.str().c_str());
    )


	while ( (int)f.tellg() != 0)
	{		
		int prevPos = f.tellg();
		f.get(c);
        f.seekg(prevPos, ios::beg);
		f.seekg(-1, ios::cur);
		if ( c == 'C' ) // Cycle detected
		{
			prevPos = f.tellg();
			f.get(c);
			if ( c == '\n' )
			{
				f >> c; // Skip 'C'
				f >> cycle;
                STR_DEBUG
                (
                    stringstream ss;
                    ss << "Cycle: " << cycle << "  detected in filepos: " << prevPos << ". Look for \"End of Trace\" token";
                    STR_MSG("STR_DEBUG: SignalTraceReader::countCyclesInfo()", ss.str().c_str());
                )
                // Check if this last cycle can be included in the trace or if it must be discarded because it is corrupted
                string line;
                while ( !f.eof() ) {
                    getline(f, line);
                    // MSG("Line: ", (string("'") + line + "'").c_str());
                    if ( line == "End of Trace" ) {
                        /*
                        stringstream ss;
                        ss << "First cycle found " << firstCycle << "\nLast cycle found " << cycle 
                           << "\nThe total amount of cycles visible using STV is " << (cycle - firstCycle + 1);
                        MSG("Debug info - trace CORRECT", ss.str().c_str());
                        */
                        STR_DEBUG
                        (
                            stringstream ss;
                            ss << "Last cycle found: " << cycle << ". Number of cycles found: " << (cycle - firstCycle + 1);
                            STR_MSG("STR_DEBUG: SignalTraceReader::countCyclesInfo()", ss.str().c_str());
                        )
                        return cycle - firstCycle + 1;
                    }
                }
                
                stringstream ss;
                ss << "First cycle found " << firstCycle << "\nLast cycle found " << cycle << " (ignored since the trace is corrupted)"
                   << "\nThe total amount of cycles visible using STV is " << (cycle - firstCycle);
                STR_MSG("Debug info - trace CORRUPTED", ss.str().c_str());
                return cycle - firstCycle;
				return cycle - firstCycle;
			}
			else
			{
                f.seekg(prevPos, ios::beg);
				f.seekg(-1, ios::cur);
			}
		}
	}

    STR_DEBUG
    (
        stringstream ss;
        ss << "Cycle tag not found. The Signal Trace file format is not correct";
        STR_MSG("STR_DEBUG: SignalTraceReader::countCyclesInfo()", ss.str().c_str());
    )

	// MessageBoxA(NULL, "Cycle tag not found. Maybe this is not a correct tracefile", "Error", MB_OK);
    STR_MSG("Error", "MB_OK");
	return -1; 
}


int SignalTraceReader::readSignalDescription( SignalDescriptionList& sil )
{	
    STR_DEBUG
    (
        stringstream ss;
        ss << "Performing readSignalDescription()";
        STR_MSG("STR_DEBUG: SignalTraceReader::readSignalDescription()", ss.str().c_str());
    )

	if ( !f.is_open() )
		return false;

	int reads = 0;

    while ( !f.eof() ) {

		fstream::pos_type prevPos = f.tellg(); // Save previous position

        string buffer;
        getline(f, buffer);

        stringstream ss(buffer);
        vector<string> tokens;
        while ( ss ) {
            string temp;
            ss >> temp;
            if ( temp == "" )
                continue;
            tokens.push_back(temp);
        }

        if ( tokens.size() == 1 ) {
            STR_MSG("SignalTraceReader::readSignalDescription", "Error loading signal info [line with only one token found when processing file header]");
            return -1;
        }

        if ( tokens.size() == 0 )
            continue; // blank line, process next line

        unsigned int cycleTest = static_cast<unsigned int>(-1);
        string cycleStr = "X";
        stringstream auxSS(buffer);
        auxSS >> cycleStr >> cycleTest;        

        if ( cycleStr == "C" && cycleTest >= 0 )  {
            // restore position in the input stream
            // first cycle info found
            f.seekg(prevPos, ios::beg);
            return reads;
        }

        if ( tokens.size() != 4 ) {
            stringstream msge;
            msge << "(tokens=" << tokens.size() << "  tokens = ";
            for ( int i = 0; i < tokens.size(); ++i )
                msge << tokens[i] << ",";
            STR_MSG("SignalTraceReader::readSignalDescription (FATAL ERROR)", msge.str().c_str());
            return -1;
        }

        ss.clear();
        ss.str(buffer);

        string signalName;
        int id = -1;
        int bw = -1;
        int latency = -1;

        ss >> signalName;
        ss >> id >> bw >> latency;

        if ( id < 0 || bw < 0 || latency < 0 ) {
            stringstream msge;
            msge << "Error. Inconsistent signal definition. Line: " << buffer;
            STR_MSG("SignalTraceReader::readSignalDescription", msge.str().c_str());
            return -1;
        }

        ++reads;

		if ( buffer[0] == 'C' && buffer[1] == ' ' )
			return reads;
		reads++;

        sil.add( signalName.c_str(), bw, latency );
	}
	return reads;
}


bool SignalTraceReader::skipCycleData()
{	
	char tag;
	char buffer[1024];
	f >> tag;
	if ( tag != 'C' )
		f.putback( tag );
	tag = 'X';
	while ( tag != 'C' ) {
		f.getline( buffer, sizeof( buffer ) ); // consume line
		f >> tag;
		f.putback( tag );
	}
	return true;
}


bool SignalTraceReader::readCycleData( CycleData& ci )
{	
	char tag;
	f >> tag;
	if ( tag != 'C' ) {
		//MSG( "Warning!", "EOF or ParseError" );
		f.putback( tag );
		return false;
	}
	
	ci.clear(); // remove all previous information
	
	int cycle;
	f >> cycle;
	ci.setCycle( cycle );
	
	char buffer[256];
	f >> tag;
	while ( tag == 'S' ) {
		// process new signal info		
		int sigId;
		f >> sigId;		
		f.getline( buffer, sizeof(buffer) ); // skip line		
		SignalData* sc = ci.getSignalData( sigId );		
		f >> tag;
		while ( tag != 'S' ) { // read all available info from this signal
			if ( tag == 'C' || tag == 'E' )
				break;
			f.putback( tag );
			f.getline( buffer, sizeof(buffer) );
			sc->addSignalDatas( buffer );
			f >> tag;
		}
	}
	f.putback( tag );
	return true;
}


long SignalTraceReader::getPosition()
{
	return f.tellg();
}

void SignalTraceReader::setPosition( long newPosition )
{	
	f.seekg( newPosition, ios::beg );
}

const char* SignalTraceReader::getFileName()
{
	return fileName;
}