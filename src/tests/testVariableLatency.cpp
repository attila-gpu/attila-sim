#include "Signal.h"

// Macros for easy use

#define WRITE(cycle,data,latency) { cout << "WRITE(" << #cycle << "," << #data << ","\
#latency << "): " << ( s1.writeGen(cycle,data,latency) == 0 ? "FALSE" : "TRUE" ) << endl; }

#define READ(cycle,data) { cout << "READ(" << #cycle << "," << #data << "): ";\
( s1.readGen(cycle,data) == 0 ? cout << "FALSE" << endl : cout << "TRUE, " <<\
"READ DATA: " << "\"" << data << "\"" << endl ); }


#define DUMP() {\
cout << "***********************" << endl;\
s1.dump();\
cout << "***********************" << endl; }


int main( void ) {


	char* A = "A";
	char* B = "B";
	char* C = "C";

	char* dataOut = 0;

	Signal s1( "Signal Testing", 3, 4 );


	WRITE(2,A,3);	
	WRITE(2,B,4);
	READ(4,dataOut); // False ( not data available yet )
	WRITE(4,C,1);
	
	//DUMP();	
	
	READ(5,dataOut);

	//DUMP();

	READ(5,dataOut);

	//DUMP();
	READ(6,dataOut);

	WRITE(22,A,2);

	READ(24,dataOut)
	
	DUMP();


	return 0;
}