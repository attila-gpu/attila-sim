#include "SignalTraceReader.h"
#include <windows.h>

#define MSG(title,msg) MessageBox( NULL, msg, title, MB_OK );

SignalTraceReader::SignalTraceReader() : fileName(0) 
{ 
	// empty
}


int SignalTraceReader::readAndPutback( char* buffer, int sizeBuffer ) 
{
	f.read( buffer, sizeBuffer );
	for ( int i = sizeBuffer-1; i >= 0; i-- )
		f.putback( buffer[i] );
	return sizeBuffer;
}

bool SignalTraceReader::open( const char* filePath )
{
	if ( fileName )
		delete[] fileName;
	fileName = new char[strlen(filePath) + 1];
	strcpy( fileName, filePath );
	f.open( fileName, ios::in | ios::binary | ios::nocreate );
	if ( f.is_open() )
		return true;
	return false;
}


bool SignalTraceReader::skipLines( int nLines )
{
	if ( !f.is_open() )
		return false;
	
	char buffer[1024];
	while ( !f.eof() && nLines-- != 0 )
		f.getline( buffer, sizeof(buffer) );
	if ( f.eof() )
		return false;
	return true;
}



int SignalTraceReader::countCyclesInfo( const char* filePath )
{
	int nCycles = 0;
	ifstream file;
	file.open( filePath );
	if ( !file.is_open() )
		return -1;
	// file opened
	
	char buffer[1024];

	// try to find explicit cycles counting in second line
	if ( !file.eof() )
		file.getline( buffer, sizeof(buffer) ); // discard first line
	else
		return -1;
	if ( !file.eof() ) {
		file.getline( buffer, sizeof(buffer) );
		if ( buffer[0] == 'C' || buffer[0] == 'c' ) {
			int cycleCount = atoi( &buffer[2] ); // parse cycles
			/*
			char buf[256];
			sprintf( buf, "ciclos explicitados en el fichero: %d", cycleCount );
			MSG( "informacion", buf );
			*/
			return cycleCount;
		}
	}
	else
		return -1;
	
	// Count cycles ( expensive )
	while ( !file.eof() ) {
		(*(int *)buffer) = 0; // zeroing first 4 bytes
		file.getline( buffer, sizeof( buffer ) );
		// buscamos el patrón "C *" siendo * un digito entre 0..9
		if ( buffer[0] == 'C' && buffer[1] == ' ' &&
			'0' <= buffer[2] && buffer[2] <= '9' )			
			nCycles++;

	}
	file.close();
	return nCycles;
}

int SignalTraceReader::readSignalInfo( SignalInfoList& sil )
{
	if ( !f.is_open() )
		return false;
	int reads = 0;
	char buffer[1024];	
	int id, bw, latency;
	while ( !f.eof() ) {
		f.read( buffer, 2 );
		f.putback( buffer[1] );
		f.putback( buffer[0] );
		if ( buffer[0] == 'C' && buffer[1] == ' ' )
			return reads;
		reads++;
		// Read a new SignalInfo
		f.getline( buffer, sizeof(buffer), '\t' );		
		f >> id;
		f >> bw;
		f >> latency;
		sil.add( buffer, bw, latency );
		f.getline( buffer, sizeof( buffer ) ); // consume this line
	}
	return reads;
}


bool SignalTraceReader::skipCycleInfo()
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


bool SignalTraceReader::readCycleInfo( CycleInfo& ci )
{	
	char tag;
	f >> tag;
	if ( tag != 'C' ) {
		MSG( "Warning!", "EOF or ParseError" );
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
		SignalContent* sc = ci.getSignalContent( sigId );		
		f >> tag;
		while ( tag != 'S' ) { // read all available info from this signal
			if ( tag == 'C' || tag == 'E' )
				break;
			f.putback( tag );
			f.getline( buffer, sizeof(buffer) );
			sc->addSignalContents( buffer );
			f >> tag;
		}
	}
	f.putback( tag );
	return true;
}


long SignalTraceReader::getPosition()
{
	/*
	char buf[256];
	sprintf( buf, "getPosition -> %d", f.tellg() );
	MSG( "getPosition", buf );
	*/
	return f.tellg();
}

void SignalTraceReader::setPosition( long newPosition )
{	
	/*
	char buf[256];
	sprintf( buf, "setPosition <- %d", newPosition );
	MSG( "setPosition", buf );
	*/
	f.seekg( newPosition, ios::beg );
}


const char* SignalTraceReader::getFileName()
{
	return fileName;
}