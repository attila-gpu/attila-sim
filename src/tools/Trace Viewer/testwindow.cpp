#include "testwindow.h"

#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qapplication.h>

#include "PreferencesDialog.h"
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qmultilineedit.h>
#include "ErrorDialog.h"
#include <qprogressdialog.h>


#define MSG(title,msg) MessageBox( NULL, msg, title, MB_OK );

TestWindow::TestWindow( QWidget* parent, const char* name, WFlags f )
	: QMainWindow( parent, name, f )
{
	setCaption("Signal Trace Visualizer - ver 0.4 (cache)");

	QVBox* vBox = new QVBox( this, "vBox" );	
	QHBox* hBox = new QHBox( vBox, "hBox" );	
	
	qsv = new QSlider( Vertical, hBox, "qsv" );
	qsh = new QSlider( Horizontal, vBox, "qsh" );
	
	config = new ConfigurationManager( "config.ini" );
	if ( !config->loadConfiguration() ) {
		config->saveConfiguration(); // create new config file
		config->loadConfiguration();
	}	

	config->dump();

	dm = new DataManager;
	int nCycles = config->getLoadMaxCycles();
	if ( nCycles == 0 )
		nCycles = -1;	

	if ( dm->loadData( "signaltrace.txt", nCycles ) < 0 ) {
		MSG( "Error", "Trace file not found. No data loaded");
		exit( 0 );
	}

	int sizeSquares = config->getSizeSquares();
	dz = new DrawZone( hBox, *dm, ( sizeSquares > 0 ? sizeSquares : 20 ) );

	dz->showSignalsRuler( config->isSignalsRulerOn() );
	dz->showCyclesRuler( config->isCyclesRulerOn() );	
	dz->showLines( config->isDivisionLinesOn());
	
	qsh -> setMinValue( 0 );
	qsv -> setMinValue( 0 );
	int xMax, yMax;
	dz->getRealPixelsSize( xMax, yMax );

	qsh -> setMaxValue( xMax );
	qsv -> setMaxValue( yMax );

	setCentralWidget( vBox );
    QAction* fileNewAction = new QAction( "Load new trace file", "Load &new trace file", CTRL+Key_N, this, "new" );
    QAction* fileQuitAction = new QAction( "Quit", "&Quit", CTRL+Key_Q, this, "quit" );
	QAction* viewZoomIn = new QAction( "Zoom in", "Zoom &in", CTRL+Key_I, this, "Zoom in" );
	QAction* viewZoomOut = new QAction( "Zoom out", "Zoom &out", CTRL+Key_O, this, "Zoom out" );
	showDivisionLines = new QAction( "Show division lines", "Show division &lines",
		CTRL+Key_L, this, "Show division lines" );
	showDivisionLines->setToggleAction( true );
	showDivisionLines->setOn( config->isDivisionLinesOn() );
	viewCyclesRuler = new QAction( "Show cycle's ruler", "Show cycle's ruler",
		CTRL+Key_C, this, "Show cycles's ruler" );
	viewCyclesRuler->setToggleAction( true );
	viewCyclesRuler->setOn( config->isSignalsRulerOn() );
    viewSignalsRuler = new QAction( "Show signal's ruler", "Show signal's ruler",
		CTRL+Key_S, this, "Show signal's ruler" );
	viewAutoHideRulers = new QAction( "Auto hide rulers", "Auto hide rulers",
		CTRL+Key_H, this, "Auto hide rulers" );
	viewAutoHideRulers->setToggleAction( true );
	viewAutoHideRulers->setOn( config->isAutoHideRulersOn() );
	viewSignalsRuler->setToggleAction( true );
	viewSignalsRuler->setOn( config->isSignalsRulerOn() );

	forwardAction = new QAction( "View forward dependences", "View &forward dependences", 0, this );
	forwardAction->setToggleAction( true );
	backwardAction = new QAction( "View backward dependences", "View &backward dependences", 0, this );
	backwardAction->setToggleAction( true );
	backwardAction->setOn( true );
	bothAction = new QAction( "View all dependences", "View &all dependences", 0, this );
	bothAction->setToggleAction( true );
	QActionGroup* viewDependencesGroup = new QActionGroup( this );
	viewDependencesGroup->insert( forwardAction );
	viewDependencesGroup->insert( backwardAction );
	viewDependencesGroup->insert( bothAction );
	viewDependencesGroup->setEnabled( true );


	QAction* preferences = new QAction( "Preferences", "&Preferences", CTRL+Key_P, this, "Preferences" );

	QPopupMenu* fileMenu = new QPopupMenu( this );
	QPopupMenu* viewMenu = new QPopupMenu( this );
	QPopupMenu* dependencesMenu = new QPopupMenu( this );
	QPopupMenu* optionsMenu = new QPopupMenu( this );
    menuBar()->insertItem( "&File", fileMenu );
	menuBar()->insertItem( "&View", viewMenu );
	menuBar()->insertItem( "&Dependences", dependencesMenu );
	menuBar()->insertItem( "&Options", optionsMenu );
    fileNewAction->addTo( fileMenu );
    fileQuitAction->addTo( fileMenu );
	viewZoomIn->addTo( viewMenu );
	viewZoomOut->addTo( viewMenu );
	viewMenu->insertSeparator();
	showDivisionLines->addTo( viewMenu );
	viewMenu->insertSeparator();
	viewCyclesRuler->addTo( viewMenu );
	viewSignalsRuler->addTo( viewMenu );
	viewAutoHideRulers->addTo( viewMenu );
	viewDependencesGroup->addTo( dependencesMenu );
	preferences->addTo( optionsMenu );
	
	connect( qsh, SIGNAL(valueChanged(int)), dz, SLOT(displaceH(int)) );
	connect( qsv, SIGNAL(valueChanged(int)), dz, SLOT(displaceV(int)) );
	connect( viewZoomIn, SIGNAL(activated()), dz, SLOT( zoomIn() ) );
	connect( viewZoomOut, SIGNAL(activated()), dz, SLOT( zoomOut() ) );
	connect( fileQuitAction, SIGNAL( activated()), qApp, SLOT( quit() ) );
	connect( showDivisionLines, SIGNAL(toggled(bool)), dz, SLOT(showLines(bool) ) );
	connect( viewCyclesRuler, SIGNAL(toggled(bool)), dz, SLOT(showCyclesRuler(bool)) );
	connect( viewSignalsRuler, SIGNAL(toggled(bool)), dz, SLOT(showSignalsRuler(bool)) );
	connect( preferences, SIGNAL(activated()), this, SLOT(editPreferences()) );
	// maintain rulers updated
	connect( dz, SIGNAL(realPixelsChanged(int,int)), this, SLOT(updateRulers(int,int)) );
	connect( viewDependencesGroup, SIGNAL(selected(QAction*)),this,SLOT(updateDependences(QAction*)) );
}

void TestWindow::editPreferences()
{
	PreferencesDialog* d = new PreferencesDialog( this, "Preferences Dialog", true );
	if ( config->isCyclesRulerOn() )
		d->showCyclesRulerCB->setChecked( true );
	if ( config->isSignalsRulerOn() )
		d->showSignalsRulerCB->setChecked( true );
	if ( config->isDivisionLinesOn() )
		d->showDivisionLinesCB->setChecked( true );
	if ( config->isAutoHideRulersOn() )
		d->autoHideRulersCB->setChecked( true );
	
	QIntValidator qiv( 0, 10000000, d->loadMaxCyclesTL );
	QIntValidator validateSS( 2, 100, d->sizeSquareTL );
	char myText[10];
	sprintf( myText, "%d", config->getLoadMaxCycles() );
	d->loadMaxCyclesTL->setText( myText );
	sprintf( myText, "%d", config->getSizeSquares() );
	d->sizeSquareTL->setText( myText );
	while ( d->exec() ) {
		// update config file
		int maxCycles;
		QValidator::State state = qiv.validate( d->loadMaxCyclesTL->text(), maxCycles );
		switch ( state ) {
		case QValidator::Acceptable:
			    maxCycles = atoi( d->loadMaxCyclesTL->text() );
				config->setLoadMaxCycles( maxCycles );
				break;
			case QValidator::Intermediate:
				showError( "Error", "Number out of range. It must be in the range 0 to 10^7" );
				continue;				
			case QValidator::Invalid:
				showError( "Error", "Load max cycles must be an integer in range 0 to 10^7" );
				continue;
		}
		int sizeS;
		QValidator::State state2 = validateSS.validate( d->sizeSquareTL->text(), sizeS );
		switch ( state2 ) {
			case QValidator::Acceptable:
				sizeS = atoi( d->sizeSquareTL->text() );
				config->setSizeSquares( sizeS );
				break;
			case QValidator::Intermediate:
				showError( "Error", "Pixel size out of range" );
				continue;
			case QValidator::Invalid:
				showError( "Error", "Pixel field must contain an integer" );
				continue;
		}
		// undate config in memory and save it to disk
		config->setCyclesRulerOn( d->showCyclesRulerCB->isChecked() );
		config->setSignalsRulerOn( d->showSignalsRulerCB->isChecked() );
		config->setDivisionLinesOn( d->showDivisionLinesCB->isChecked() );
		config->setAutoHideRulersOn( d->autoHideRulersCB->isChecked() );
		config->setLoadMaxCycles( maxCycles );
		config->saveConfiguration();
		
		// update current view with new config file
		dz->showSignalsRuler( config->isSignalsRulerOn() );
		dz->showCyclesRuler( config->isCyclesRulerOn() );	
		dz->showLines( config->isDivisionLinesOn());
		dz->setNewItemSize( config->getSizeSquares() );
		
		// update menu toggle check boxes
		showDivisionLines->setOn( config->isDivisionLinesOn() );
		viewCyclesRuler->setOn( config->isSignalsRulerOn() );
		viewSignalsRuler->setOn( config->isSignalsRulerOn() );
		viewAutoHideRulers->setOn( config->isAutoHideRulersOn() );
		break;

	}
	
	delete d;

}

void TestWindow::keyPressEvent( QKeyEvent* e )
{
	switch ( e->ascii() ) {
		case '+':
			dz->zoomIn( 1 );
			break;
		case '-':
			dz->zoomOut( 1 );
			break;
		// if more key events are needed for parent widgets, should be
		// re-thrown

	}	
}


void TestWindow::showError( const char* title, const char* messageError )
{
	ErrorDialog ed( this, "ed", TRUE );
	if ( title )
		ed.setCaption( title );
	if ( messageError )
		ed.MultiLineEdit1->setText( messageError );	
	ed.exec();
}


void TestWindow::updateRulers( int newHMax, int newVMax )
{
	qsh->setMaxValue( newHMax );
	qsv->setMaxValue( newVMax );
}


void TestWindow::showDebug( int value )
{
	char buffer[256];
	sprintf( buffer, "HPixels: %d\nQSH max: %d\nQSV max: %d", value, qsh->maxValue(), qsv->maxValue() );
	MSG( "showDebug", buffer );
}


void TestWindow::updateDependences( QAction* qa )
{
	if ( qa == forwardAction ) {
		dz->setHighlightMode( false, true );
	}
	else if ( qa == backwardAction ) {
		dz->setHighlightMode( true, false );
	}
	else if ( qa == bothAction ) {
		dz->setHighlightMode( true, true );
	}
}
