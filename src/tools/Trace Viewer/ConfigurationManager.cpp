#include "ConfigurationManager.h"
#include <fstream.h>
#include <cstring>

#include <windows.h>
#define MSG(title,msg) MessageBox( NULL, msg, title, MB_OK );

ConfigurationManager::ConfigurationManager( const char* pathName ) :
cyclesRulerOff(false),
signalsRulerOff(false),
autoHideRulers(false),
divisionLinesOn(true),
loadMaxCycles(0),
sizeSquares(10)

{
	fileName = new char[strlen(pathName)+1];
	strcpy( fileName, pathName );	
}


bool ConfigurationManager::saveConfiguration( bool omitFalseValues )
{
	ofstream f;
	f.open( fileName );
	if ( !f.is_open() )
		return false;
	if ( !omitFalseValues || cyclesRulerOff )
		f << "CyclesRulerOff=" << ( cyclesRulerOff ? '1' : '0' ) << endl;
	if ( !omitFalseValues || signalsRulerOff )
		f << "SignalsRulerOff=" << ( signalsRulerOff ? '1' : '0' ) << endl;
	if ( !omitFalseValues || autoHideRulers )
		f << "AutoHideRulers=" << ( autoHideRulers ? '1' : '0' ) << endl;
	if ( !omitFalseValues || divisionLinesOn )
		f << "DivisionLinesOn=" << ( divisionLinesOn ? '1' : '0' ) << endl;
	if ( !omitFalseValues || loadMaxCycles != 0 )
		f << "LoadMaxCycles=" << loadMaxCycles << endl;
	if ( !omitFalseValues || sizeSquares != 0 )
		f << "SizeSquares=" << sizeSquares << endl;
	return true;
}

bool ConfigurationManager::loadConfiguration()
{
	ifstream f;
	f.open( fileName, ios::nocreate );
	if ( !f.is_open() )
		return false;	
	
	char line[256];
	while ( !f.eof() ) {
		f.getline( line, sizeof(line), '=' );
		cout << "Line read(1): " << "\"" << line << "\"" << endl;
		if ( strcmp( line, "CyclesRulerOff" ) == 0 ) {			
			f.getline( line, sizeof(line) );
			cyclesRulerOff = ( line[0] == '0' ? false : true );
		}
		else if ( strcmp( line, "SignalsRulerOff" ) == 0 ) {
			f.getline( line, sizeof(line) );
			signalsRulerOff = ( line[0] == '0' ? false : true );
		}
		else if ( strcmp( line, "AutoHideRulers" ) == 0 ) {
			f.getline( line, sizeof(line) );
			autoHideRulers = ( line[0] == '0' ? false : true );
		}
		else if ( strcmp( line, "DivisionLinesOn" ) == 0 ) {
			f.getline( line, sizeof(line) );
			divisionLinesOn = ( line[0] == '0' ? false : true );
		}
		else if ( strcmp( line, "LoadMaxCycles" ) == 0 ) {
			f >> loadMaxCycles;
			f.getline( line, sizeof(line) ); // consume line
		}
		else if ( strcmp( line, "SizeSquares" ) == 0 ) {
			f >> sizeSquares;
			f.getline( line, sizeof(line) ); // consume line
		}
		else {			
			// ignore line ( unknown line )
		}
	}
	f.close();
	return true;
}


bool ConfigurationManager::isAutoHideRulersOn()
{
	return autoHideRulers;
}

bool ConfigurationManager::isCyclesRulerOn()
{
	return !cyclesRulerOff;
}

bool ConfigurationManager::isSignalsRulerOn()
{
	return !signalsRulerOff;
}

bool ConfigurationManager::isDivisionLinesOn()
{
	return divisionLinesOn;
}

int ConfigurationManager::getLoadMaxCycles()
{
	return loadMaxCycles;
}

int ConfigurationManager::getSizeSquares()
{
	return sizeSquares;
}


void ConfigurationManager::setAutoHideRulersOn( bool on )
{
	autoHideRulers = on;
}

void ConfigurationManager::setCyclesRulerOn( bool on )
{
	cyclesRulerOff = !on;
}

void ConfigurationManager::setSignalsRulerOn( bool on )
{
	signalsRulerOff = !on;
}

void ConfigurationManager::setDivisionLinesOn( bool on )
{
	divisionLinesOn = on;
}


void ConfigurationManager::setLoadMaxCycles( int cycles )
{
	loadMaxCycles = cycles;
}

void ConfigurationManager::setSizeSquares( int sizeS )
{
	sizeSquares = sizeS;
}


void ConfigurationManager::dump()
{
	cout << "Configuration:" << endl;
	cout << "Cycles ruler off: " << ( cyclesRulerOff ? "TRUE" : "FALSE" ) << endl;
	cout << "Signals ruler off: " << ( signalsRulerOff ? "TRUE" : "FALSE" ) << endl;
	cout << "Autohide rulers: " << ( autoHideRulers ? "TRUE" : "FALSE" ) << endl;
	cout << "Division Lines on: " << ( divisionLinesOn ? "TRUE" : "FALSE" ) << endl;
	cout << "Load max cycles ( 0 means infinite ): " << loadMaxCycles << endl;
	cout << "SizeSquares ( <= 0 means not valid ): " << sizeSquares << endl;
}

