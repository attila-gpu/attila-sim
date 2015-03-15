#include <qapplication.h>
#include "testwindow.h"
#include "SignalTraceReader.h"


int main( int argc, char** argv )
{

	
	QApplication app( argc, argv );
	TestWindow window;
	app.setMainWidget(&window);
	//app.setMainWidget( dz );
	window.resize( 640, 400 );
	window.show();
	return app.exec();
	/*
	SignalTraceReader str;
	str.open( "signaltrace.txt" );	
	str.skipLines( 4 );
	
	SignalInfoList sil;
	
	str.readSignalInfo( sil );

	sil.dump();
	*/
}





