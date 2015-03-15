#include <qapplication.h>
#include "STVWindow.h"


int main( int argc, char ** argv )
{
    QApplication app( argc, argv );    

	STVWindow window;

	app.setMainWidget(&window);

	window.resize( 640, 400 );
	window.show();
    return app.exec();
}
