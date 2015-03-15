#include "GPUTypes.h"
#include "Signal.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fstream.h>

void SETPARAMETERS( Signal& s, u32bit bw, u32bit lat )
{
	cout << "SETPARAMETERS(" << bw << "," << lat << ") - Signal's State: " <<
		( s.setParameters( bw, lat ) == 0 ? "NOT WELL-DEFINED" : "WELL-DEFINED" ) << endl;
}

void SETLATENCY( Signal& s, u32bit lat ) 
{
	cout << "SETLATENCY(" << lat << ") - Signal's State: " <<
		( s.setLatency( lat ) == 0 ? "NOT WELL-DEFINED" : "WELL-DEFINED" ) << endl;
}

void SETBANDWIDTH( Signal& s, u32bit bw ) 
{
	cout << "SETBANDWIDTH(" << bw << ") - Signal's State: " <<
		( s.setBandwidth( bw ) == 0 ? "NOT WELL-DEFINED" : "WELL-DEFINED" ) << endl;
}


void WRITE( Signal& s, u32bit cycle, char* data, u32bit latency )
{
	cout << "WRITE(" << cycle << "," << "\"" << data << "\"," << latency << "): ";
	if ( !s.writeGen( cycle, data, latency ) )
		cout << "FALSE" << endl;
	else
		cout << "TRUE" << endl;
}

void WRITE( Signal& s, u32bit cycle, char* data )
{
	cout << "WRITE(" << cycle << "," << "\"" << data << "\"): ";	
	if ( !s.writeGen( cycle, data ) )
		cout << "FALSE" << endl;
	else
		cout << "TRUE" << endl;
}


void READ( Signal& s, u32bit cycle, char* data )
{
	
	cout << "READ(" << cycle << ",dataRead): ";
	if ( !s.readGen( cycle, data ) )
		cout << "FALSE" << endl;
	else
		cout << "TRUE, DATA READ: " << data << endl;
}



void DUMP( Signal& s )
{
	cout << "******* DUMP *******" << endl;
	s.dump();
	cout << "***** END DUMP *****" << endl;
}


void executeOperation( Signal& s, char* buffer, u32bit size )
{
	

	u32bit latency, cycle, i, j, ini;
	char* data = 0;

	if ( buffer[0] == 'F' ) {
		cout << "FINISH" << endl;
		exit( 0 );
	}
	if ( buffer[0] == 'D' ) {
		DUMP(s);
		return;

	}
		
	i = 2; // gets cycle operation for read/writes op or first param in the other methods
	while ( buffer[i] >= '0' && buffer[i] <= '9' )
		i++;
	buffer[i] = 0;
	
	cycle = atoi( &buffer[2] );		
	
		
	switch ( buffer[0] ) {
		case 'R':
			READ(s,cycle,data);
			delete[] data; // Consumimos el dato
			break;
		case 'W':
			data = new char[50];
			i += 2;
			j = 0;
			while ( buffer[i] != '"' )
				data[j++] = buffer[i++];
			data[j] = 0;
			if ( buffer[i+1] == ',' ) { // latency specified
				i = i + 2;
				ini = i;
				while ( buffer[i] >= '0' && buffer[i] <= '9' )
					i++;
				buffer[i] = 0;
				latency = atoi( &buffer[ini] );				
				WRITE(s,cycle,data,latency);
			}
			else {				
				WRITE(s,cycle,data); // Use MAX latency available
			}
			break;
		case 'P': // setParameters
			// cycle represents bw
			i++;
			ini = i;
			while ( buffer[i] >= '0' && buffer[i] <= '9'  )
				i++;
			buffer[i] = 0;
			latency = atoi( &buffer[ini] );
			SETPARAMETERS(s,cycle,latency);
			break;
		case 'L': // new Latency
			// cycle represents latency
			SETLATENCY(s,cycle);
			break;
		case 'B': // new Bandwidth
			SETBANDWIDTH(s,cycle);
			// cycle represents bw
		default:
			cout << "Unknown command" << endl;
	}
}


int main( void ) 
{

	char buffer[50];

	u32bit latency, bandwidth;

	ifstream driverFile( "SignalDriverFile.txt" );

	driverFile >> bandwidth;
	driverFile >> latency;
	
	driverFile.getline( buffer, sizeof(buffer) ); // Saltamos el resto de la linea

	cout << "BANDWIDTH: " << bandwidth << endl;
	cout << "MAX LATENCY: " << latency << endl;

	Signal s( "Signal Test", bandwidth, latency );
	
	while ( !driverFile.eof() ) {		
		driverFile.getline( buffer, sizeof(buffer) );		
		executeOperation( s, buffer, sizeof(buffer) );		
	}
	
	driverFile.close();
		
	return 0;

}
