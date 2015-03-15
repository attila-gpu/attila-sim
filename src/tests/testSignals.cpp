/*
 * SIGNALS test
 */

#include "Signal.h"
#include "QuadFloat.h" 

Signal s1( "Signal1");//, 3, 2 );


QuadFloat qfin( 1, 2, 3, 4 );
QuadFloat qfin2( 5, 7, 11, 13 );

QuadFloat* qfout = 0;

// Macros for easy use of READ/WRITES tests
#define READ(cycle,data) { cout << "Read: " << (int)s1.read(cycle,data) << endl; }
#define WRITE(cycle,data) { cout << "Write: " << (int)s1.write(cycle,data) << endl; }

// Dumps signal state
#define DUMP() { s1.dump(); }

int main( void ) {
	
	s1.setParameters( 3,2 );
	
	WRITE(2,&qfin);
	WRITE(2,&qfin);
	WRITE(2,&qfin);
	WRITE(3,&qfin2);	

	READ(3,qfout); // No data available yet
	READ(4,qfout);
	DUMP();
	READ(4,qfout);
	READ(4,qfout);
	
	cout << "Value QFOUT: " << (*qfout) << endl;
	
	READ(5,qfout);
	
	cout << "Value QFOUT: " << (*qfout) << endl;

	WRITE(7,&qfin); // READ in  9
	WRITE(8,&qfin); // READ in 10
	
	//DUMP();


	cout << "Name: \"" << s1.getName() << "\"" << endl;
	cout << "Bandwidth: " << s1.getBandwidth() << endl;
	cout << "Latency: " << s1.getLatency() << endl;

	return 0;
}
