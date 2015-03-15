#include "ConfigurationManager.h"
#include <fstream>
#include <cstring>
#include <iostream>


#include <windows.h>
#define MSG(title,msg) MessageBox( NULL, msg, title, MB_OK );

//#include <QMessageBox.h>
//#define QMSG(msg) QMessageBox::information(0, "QMessage", msg, QMessageBox::Ok);

using namespace std;

// ConfigurationManager::ConfigurationManager( const char* pathName ) :
ConfigurationManager::ConfigurationManager( const std::string& pathName ) :
cyclesRulerOff(false),
signalsRulerOff(false),
autoHideRulers(false),
divisionLinesOn(true),
loadMaxCycles(0),
sizeSquares(defaultSizeSquares),
cacheSize(defaultCacheSize),
pageStep(defaultPageStep),
colorOptions(defaultColorOptions),
showIndividualInfo(true),
showDefaultInfo(true),
treeDependencyLevel(0),
fileName(pathName)
{
	// fileName = new char[strlen(pathName)+1];
	// strcpy( fileName, pathName );	
}


bool ConfigurationManager::saveConfiguration( bool omitFalseValues )
{
	ofstream f;
	f.open( fileName.c_str() );
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
	f << "CacheSize=" << cacheSize << endl;
	f << "PageStep=" << pageStep << endl;
	f << "ColorOptions=" << colorOptions << endl;
	f << "ShowIndividualInfo=" << ( showIndividualInfo ? '1' : '0' ) << endl;
	f << "ShowDefaultInfo=" << ( showDefaultInfo ? '1' : '0' ) << endl;
	f << "TreeDependencyLevel=" << treeDependencyLevel << endl;
	if ( !layoutFile.empty() )
		f << "LayoutFile=" << layoutFile << endl;
	if ( !colorMapFile.empty() )
		f << "ColorMapFile=" << colorMapFile << endl;
    if ( !signalTraceFile.empty() )
        f << "SignalTraceFile=" << signalTraceFile << endl;
	
	return true;
}

bool ConfigurationManager::loadConfiguration()
{
	ifstream f;
    f.open( fileName.c_str() );
	if ( !f.is_open() )
		return false;	
	
	char line[256];
	while ( !f.eof() ) {
		f.getline( line, sizeof(line), '=' );
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
		else if ( strcmp(line, "CacheSize" ) == 0 )
		{
			f >> cacheSize;
			f.getline(line,sizeof(line));
		}
		else if ( strcmp(line, "PageStep" ) == 0 )
		{
			f >> pageStep;
			f.getline(line,sizeof(line));
		}
		else if ( strcmp(line, "ColorOptions" ) == 0 )
		{
			f >> colorOptions;
			f.getline(line,sizeof(line));
		}

		else if ( strcmp( line, "ShowIndividualInfo" ) == 0 ) {
			f.getline( line, sizeof(line) );
			showIndividualInfo = ( line[0] == '0' ? false : true );
		}
		else if ( strcmp( line, "ShowDefaultInfo" ) == 0 ) {
			f.getline( line, sizeof(line) );
			showDefaultInfo = ( line[0] == '0' ? false : true );
		}
		else if ( strcmp(line, "TreeDependencyLevel") == 0 ) {

			f >> treeDependencyLevel;
			f.getline(line, sizeof(line));
		}
		else if ( strcmp(line, "LayoutFile") == 0 )
		{
			f.getline(line,sizeof(line));
			layoutFile = line;
		}
		else if ( strcmp(line, "ColorMapFile") == 0 )
		{
			f.getline(line,sizeof(line));
			colorMapFile = line;
		}
        else if ( strcmp(line, "SignalTraceFile") == 0 ) {
            f.getline(line, sizeof(line));
            signalTraceFile = line;
            // QMSG(signalTraceFile.c_str());
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
	cout << "CacheSize: " << cacheSize << endl;
	cout << "PageStep: " << pageStep << " cycles" << endl;
	cout << "ColorOptions: " << colorOptions << endl;
	cout << "Show individual info: " << ( showIndividualInfo ? "TRUE" : "FALSE" ) << endl;
	cout << "Show default info: " << ( showDefaultInfo ? "TRUE" : "FALSE" ) << endl;
	cout << "Layout file: " << layoutFile << endl;
	cout << "ColorMap file: " << colorMapFile << endl;
    cout << "Signal Trace File: " << signalTraceFile << endl;
}


int ConfigurationManager::getPageStep() const 
{ 
	return pageStep; 
}

void ConfigurationManager::setPageStep(int ps) 
{ 
	if ( ps <= 0 )
		ps = 1;
	pageStep = ps; 
}

void ConfigurationManager::setTreeDependencyLevel(unsigned int level)
{
	treeDependencyLevel = level;
}

unsigned int ConfigurationManager::getTreeDependencyLevel() const
{
	return treeDependencyLevel;
}

string ConfigurationManager::getLayoutFile() const
{
	return layoutFile;
}

void ConfigurationManager::setLayoutFile(string path)
{
	layoutFile = path;
}

string ConfigurationManager::getColorMapFile() const
{
	return colorMapFile;
}

void ConfigurationManager::setColorMapFile(string path)
{
	colorMapFile = path;
}

string ConfigurationManager::getSignalTraceFile() const
{
    return signalTraceFile;
}

void ConfigurationManager::setSignalTraceFile(const string& path)
{
    signalTraceFile = path;
}

