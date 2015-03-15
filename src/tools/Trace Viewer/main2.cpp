#include "SignalTraceReader.h"
#include "CycleInfo.h"


int main( void )
{
	SignalTraceReader str;
	str.open( "signaltrace.txt" );	
	str.skipLines( 4 );
	
	SignalInfoList sil;	
	str.readSignalInfo( sil );
	//sil.dump();

	// mandatory
	CycleInfo::setSignalInfoList( &sil );
	
	CycleInfo ci;
	//while ( str.hasCycleInfo() ) {
	while ( str.readCycleInfo( ci ) )
		ci.dump();


	
	//ci.dump();
	//str.readCycleInfo( ci );
	//ci.dump();
	//cout << "hasCycleInfo: " << str.hasCycleInfo() << endl;
		//ci.dump();
	//}
	//cout << "Fin" << endl << "Contenido del último ciclo leido: " << endl;
	//ci.dump();
	


	//}
	
	
	//ci.dump();

	return 0;
}
