#include <qmainwindow.h>
#include <qaction.h>
#include <qslider.h>

#include "DrawZone.h"
#include "DataManager.h"
#include "ConfigurationManager.h"

class TestWindow : public QMainWindow
{
	Q_OBJECT
public:
	TestWindow( QWidget* parent = 0, const char* name = 0, WFlags f = WType_TopLevel );
	void keyPressEvent( QKeyEvent* e );

public slots:
	void editPreferences();
	void showDebug( int value );
	void updateRulers( int newHMax, int newVMax );
	void updateDependences( QAction* qa );

private:
	
	DataManager* dm;
	DrawZone* dz;
	ConfigurationManager* config;

	QAction* showDivisionLines;
	QAction* viewCyclesRuler;
	QAction* viewSignalsRuler;
	QAction* viewAutoHideRulers;

	QAction* forwardAction;
	QAction* backwardAction;
	QAction* bothAction;

	QSlider* qsv;
	QSlider* qsh;

	void showError( const char* title = 0, const char* messageError = 0 );

};

