#include "STVWindow.h"
// #include <Q3PopupMenu.h>
#include <QT3Support/Q3PopupMenu>
#include <qmenubar.h>
#include <qstatusbar.h>
#include <qapplication.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qvalidator.h>
//#include <qmultilineedit.h>
#include <QT3Support/Q3MultiLineEdit>
#include "ErrorDialog.h"
#include "CycleGo.h"
// #include "HiddenSignalsDialog.h"
#include <ui_HiddenSignalsDialog.h>
#include <qprogressdialog.h>
#include <qfiledialog.h>
// #include <QListBox.h>
#include <QT3Support/Q3ListBox.h>
#include "stv.h"
#include <QMessageBox.h>
#include "PreferencesDialog.h"
#include "HiddenSignalsDialog.h"

#include <string>

#include <windows.h>

using std::string;

#define QMSG(msg) QMessageBox::information(0, "QMessage", msg, QMessageBox::Ok);


void STVWindow::loadTraceFile(const char* path, int nCycles)
{	
	statusBar()->message("Loading tracefile...");

	if ( dz )
	{
		dz->disconnect();
		qsh->disconnect();
		qsv->disconnect();
		viewZoomInAction->disconnect();
		viewZoomOutAction->disconnect();
		showDivisionLinesAction->disconnect();
		viewCyclesRulerAction->disconnect();
		viewSignalsRulerAction->disconnect();

		delete dz;
		delete dm;

		dz = 0;
		dm = 0;

		qsh->setHidden(true);
		qsv->setHidden(true);
	}
	
	dm = new DataManager(config->getCacheSize());

	int errorCode = dm->loadData(path,nCycles);
	if ( errorCode < 0 ) 
	{
		char temp[256];
		if ( errorCode == -1 )
		{		
			sprintf(temp, "File: \"%s\" not found", path);
			showError("File not found", temp);
		}
		else if ( errorCode == - 2 )
		{
			sprintf(temp, "Trace file: \"%s\" with  invalid format", path);
			showError("Trace file format error", temp);			
		}
		delete dm;
		dm = 0;
		return ;
	}	

	int sizeSquares = config->getSizeSquares();
	
	hBox->hide();
	
	dz = new DrawZone(this, hBox, *dm, ( sizeSquares > 0 ? sizeSquares : 20 ) );

	statusBar()->message("Tracefile loaded!");

	currentLayoutFile = ""; // No layout file selected
	fileReloadConfigAction->setEnabled(true);
	fileLoadLayout->setEnabled(true);
	fileLoadNewLayout->setEnabled(true);
	fileSaveLayout->setEnabled(true);
	fileSaveLayoutAs->setEnabled(true);	
	fileLoadColorMap->setEnabled(true);
	
	connect( qsh, SIGNAL(valueChanged(int)), dz, SLOT(displaceH(int)) );
	connect( qsv, SIGNAL(valueChanged(int)), dz, SLOT(displaceV(int)) );
	connect( qsh, SIGNAL(sliderMoved(int)), dz, SLOT(showCycleMessage(int)) );

	connect( viewZoomInAction, SIGNAL(activated()), dz, SLOT( zoomIn() ) );
	connect( viewZoomOutAction, SIGNAL(activated()), dz, SLOT( zoomOut() ) );
	connect( showDivisionLinesAction, SIGNAL(toggled(bool)), dz, SLOT(showLines(bool) ) );
	connect( viewCyclesRulerAction, SIGNAL(toggled(bool)), dz, SLOT(showCyclesRuler(bool)) );
	connect( viewSignalsRulerAction, SIGNAL(toggled(bool)), dz, SLOT(showSignalsRuler(bool)) );
	connect( dz, SIGNAL(realPixelsChanged(int,int)), this, SLOT(updateRulers(int,int)) );
	connect( dz, SIGNAL(squareSizeChanged(int)), this, SLOT(updateHorizontalPageStep(int)) );
	connect( dz, SIGNAL(squareSizeChanged(int)), this, SLOT(maintainCurrentCycle()) );
	connect( dz, SIGNAL(squarePosition(int,int)), this, SLOT(moveRulers(int,int)) );

	//dz->showSignalsRuler( config->isSignalsRulerOn() );
	//dz->showCyclesRuler( config->isCyclesRulerOn() );	
	//dz->showLines( config->isDivisionLinesOn());

	int xMax, yMax;
	dz->getRealPixelsSize( xMax, yMax );

	qsh -> setMaxValue( xMax );
	qsv -> setMaxValue( yMax );	

	qsh -> setValue(0);
	qsv -> setValue(0);

	//qsh->setPageStep(config->getPageStep()*config->getSizeSquares());
	//qsh->setLineStep(config->getSizeSquares());
	//qsv->setLineStep(config->getSizeSquares());

	qsh->show();
	qsv->show();
	qsh->setEnabled(true);
	qsv->setEnabled(true);
	goToCycleAction->setEnabled(true);
	showHiddenSignalsAction->setEnabled(true);

	viewZoomInAction->setEnabled(true);
	viewZoomOutAction->setEnabled(true);
	viewDependencesGroup->setEnabled(true);

	if ( forwardAction->isOn() )
		dz->setHighlightMode(false, true);
	else if ( backwardAction->isOn() )
		dz->setHighlightMode( true, false );
	else if ( backAndForwardAction->isOn() )
		dz->setHighlightMode( true, true );
	else
		dz->setHiglighModeFullTree(true);

	//dz->setColorOptions( config->getColorOptions() );
	//dz->setShowIndividualInfo( config->isShowIndividualInfo() );
	//dz->setShowDefaultInfo( config->isShowDefaultInfo() );

		
	hBox->show();

	sprintf(tracePath, path);

    applyConfig_ini();

	restoreWindowCaption();

	// Try to load defaults layout and colormap
/*
	std::string auxStrPath = config->getLayoutFile();
	if ( !auxStrPath.empty() )
	{
		if ( dz->loadLayout(auxStrPath) )
			currentLayoutFile = auxStrPath;
	}
	auxStrPath = config->getColorMapFile();
	if ( !auxStrPath.empty() )
		dz->loadColorMap(auxStrPath);	
*/
}

//STVWindow::STVWindow() : 
//Q3MainWindow(),
STVWindow::STVWindow( QWidget* parent, const char* name, Qt::WFlags f )
	: Q3MainWindow( parent, name, f ), 
      dz(0), dm(0),
      currentTraceFile(""),
	  currentLayoutFile("")
{
	restoreWindowCaption();

	vBox = new Q3VBox( this, "vBox" );	
	hBox = new Q3HBox( vBox, "hBox" );	
	
    qsv = new QSlider( Qt::Vertical, hBox, "qsv" );
    qsh = new QSlider( Qt::Horizontal, vBox, "qsh" );
    qsv->setFocusPolicy(Qt::NoFocus);
    qsh->setFocusPolicy(Qt::NoFocus);
	qsh->setTracking(false);
	
	config = new ConfigurationManager( "config.ini" );
	if ( !config->loadConfiguration() ) {
		config->saveConfiguration(); // create new config file
		config->loadConfiguration();
	}	


	qsh -> setMinValue( 0 );
	qsv -> setMinValue( 0 );

	qsh->hide();
	qsv->hide();
	qsh->setEnabled(false);
	qsv->setEnabled(false);

	setCentralWidget( vBox );

	createMenuOptions();

	d = new PreferencesDialogImp( this, "Preferences Dialog", true );

	/* Permanent connetions */
	connect( fileNewAction, SIGNAL(activated()), this, SLOT(showOpenTraceDialog()));
	connect( fileReloadConfigAction, SIGNAL(activated()), this, SLOT(reloadConfig_ini()));
	connect( fileQuitAction, SIGNAL( activated()), qApp, SLOT( quit() ) );
	connect( fileLoadLayout, SIGNAL( activated()), this, SLOT( loadLayout()));
	connect( fileLoadNewLayout, SIGNAL(activated()), this, SLOT( loadNewLayout()));
	connect( fileSaveLayout, SIGNAL( activated()), this, SLOT( saveLayout()));
	connect( fileSaveLayoutAs, SIGNAL(activated()), this, SLOT( saveLayoutAs()));
	connect( fileLoadColorMap, SIGNAL(activated()), this, SLOT( loadColorMap()));

	connect( preferencesAction, SIGNAL(activated()), this, SLOT(editPreferences()) );
	connect( aboutAction, SIGNAL(activated()), this, SLOT(showAbout()) );
	connect( goToCycleAction, SIGNAL(activated()), this, SLOT(showGoToCycle()) );	
	connect( showHiddenSignalsAction, SIGNAL(activated()), this, SLOT(showHiddenSignals()) );

	// maintain rulers updated
	connect( viewDependencesGroup, SIGNAL(selected(Q3Action*)),this,SLOT(updateDependences(Q3Action*)) );

	statusBar(); // force status bar creation

    string stf = config->getSignalTraceFile();
    if ( !stf.empty() ) // Force loading the path registered in SignalTraceFile field
        loadTraceFile(stf.c_str(),0);
}



void STVWindow::editPreferences()
{	
	if ( config->isCyclesRulerOn() )
		d->showCyclesRulerCB->setChecked( true );
	if ( config->isSignalsRulerOn() )
		d->showSignalsRulerCB->setChecked( true );
	if ( config->isDivisionLinesOn() )
		d->showDivisionLinesCB->setChecked( true );
	if ( config->isAutoHideRulersOn() )
		d->autoHideRulersCB->setChecked( true );

	d->colorOptionsBG->setExclusive(true);

	switch ( config->getColorOptions() )
	{
		case 0: // None
			d->colNoneOptionRB->setChecked(true);
			break;
		case 1: // Only color
			d->colEnumOptionRB->setChecked(true);
			break;
		case 2: // Cookies & Color
			d->colCookiesOptionRB->setChecked(true);
			break;
	}

	if ( config->isShowIndividualInfo() )
		d->showIndividualInfoCB->setChecked(true);

	if ( config->isShowDefaultInfo() )
		d->showDefaultInfoCB->setChecked(true);

	
	QIntValidator qiv( 0, 10000000,0);
	QIntValidator validateSS( 2, 100,0);
	QIntValidator valCacheSize(1, 100000, 0);
	QIntValidator valPageStep(1,1000,0);

	char myText[10];
	
	sprintf( myText, "%d", config->getLoadMaxCycles() );
	d->loadMaxCyclesTL->setText( myText );

	sprintf( myText, "%d", config->getSizeSquares() );
	d->sizeSquareTL->setText( myText );

	sprintf( myText, "%d", config->getCacheSize() );
	d->cacheSizeTL->setText(myText);

	sprintf( myText, "%d", config->getPageStep() );
	d->pageStepTL->setText(myText);

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
		
		int cacheSize;
		QValidator::State state3 = valCacheSize.validate( d->cacheSizeTL->text(), cacheSize);
		switch ( state3 )
		{
			case QValidator::Acceptable:
				cacheSize = atoi(d->cacheSizeTL->text());
				if ( config->getCacheSize() !=  cacheSize )
				{
					config->setCacheSize(cacheSize);
                    QMessageBox::information(0, "Info", "Effects of resizing cache will take effect after restart the visualizer", 
                                             QMessageBox::Ok, QMessageBox::NoButton);
				}
				break;
			case QValidator::Intermediate:
				showError( "Error", "Cache size not allowed (min value=1)" );
				continue;
			case QValidator::Invalid:
				showError( "Error", "Cache size must be a positive integer" );
				continue;
		}
		int pageStep;
		
		QValidator::State state4 = valPageStep.validate( d->pageStepTL->text(), pageStep);
		switch ( state4 )
		{
			case QValidator::Acceptable:
				pageStep = atoi(d->pageStepTL->text());
				config->setPageStep(pageStep);
				if ( dz )
					updateHorizontalPageStep(dz->getSquareSize());
				break;
			case QValidator::Intermediate:
				showError( "Error", "Page Step size not allowed (1..1000)" );
				continue;
			case QValidator::Invalid:
				showError( "Error", "Page Step must be a positive integer" );
				continue;
		}
		

		// update config in memory and save it to disk
		config->setCyclesRulerOn( d->showCyclesRulerCB->isChecked() );
		config->setSignalsRulerOn( d->showSignalsRulerCB->isChecked() );
		config->setDivisionLinesOn( d->showDivisionLinesCB->isChecked() );
		config->setAutoHideRulersOn( d->autoHideRulersCB->isChecked() );
		//config->setLoadMaxCycles( maxCycles );

		if ( d->colNoneOptionRB->isChecked()  )
			config->setColorOptions(0);
		else if ( d->colEnumOptionRB->isChecked() )
			config->setColorOptions(1);
		else if ( d->colCookiesOptionRB->isChecked() )
			config->setColorOptions(2);

		config->setShowIndividualInfo(d->showIndividualInfoCB->isChecked());
		config->setShowDefaultInfo(d->showDefaultInfoCB->isChecked());

		config->saveConfiguration();
		
				
		// update current view with new config file
		if ( dz )
		{
			dz->setColorOptions( config->getColorOptions() );
			dz->setShowIndividualInfo( config->isShowIndividualInfo() );
			dz->setShowDefaultInfo( config->isShowDefaultInfo() );
			
			dz->showSignalsRuler( config->isSignalsRulerOn() );
			
			dz->showCyclesRuler( config->isCyclesRulerOn() );	
			
			dz->showLines( config->isDivisionLinesOn());
			
			// Mueve al final :-(
			dz->setNewItemSize( config->getSizeSquares() );
			
		}
		
		
		// update menu toggle check boxes
		showDivisionLinesAction->setOn( config->isDivisionLinesOn() );
		viewCyclesRulerAction->setOn( config->isSignalsRulerOn() );
		viewSignalsRulerAction->setOn( config->isSignalsRulerOn() );
		viewAutoHideRulersAction->setOn( config->isAutoHideRulersOn() );
		

		break;
	}

}

void STVWindow::keyPressEvent( QKeyEvent* e )
{
	switch ( e->key() )
	{
    case Qt::Key_Plus:
			if ( dz )
				dz->zoomIn( 1 );
			break;
		case Qt::Key_Minus:
			if ( dz )
				dz->zoomOut( 1 );
			break;
		case Qt::Key_Up:
			qsv->subtractLine();
			break;
		case Qt::Key_Down:
			qsv->addLine();
			break;
		case Qt::Key_Left:
			qsh->subtractLine();
			break;
		case Qt::Key_Right:
			qsh->addLine();
			break;
		case Qt::Key_Prior: // Re Pag
			qsh->subtractStep();
			break;
		case Qt::Key_Next: // Av Pag
			qsh->addStep();
			break;
		case Qt::Key_Home: // Inicio
			qsh->setValue(qsh->minValue());
			break;			
		case Qt::Key_End: //fin
			qsh->setValue(qsh->maxValue());
			break;
	}

		// if more key events are needed for parent widgets, should be
		// re-thrown

}


void STVWindow::showError( const char* title, const char* messageError )
{
	ErrorDialogImp ed( this, "ed", TRUE );
	if ( title )
		ed.setCaption( title );
	if ( messageError )
		ed.MultiLineEdit1->setText( messageError );	
	ed.exec();
}


void STVWindow::updateRulers( int newHMax, int newVMax )
{
	qsh->setMaxValue( newHMax );
	qsv->setMaxValue( newVMax );
}


void STVWindow::showDebug( int value )
{
	char buffer[256];
	sprintf( buffer, "HPixels: %d\nQSH max: %d\nQSV max: %d", value, qsh->maxValue(), qsv->maxValue() );
    
    QMessageBox::information(0, "showDebug", buffer, QMessageBox::Ok, QMessageBox::NoButton);
	// MessageBoxA(NULL, buffer, "showDebug", MB_OK);
	//MSG( "showDebug", buffer );
}


void STVWindow::updateDependences( Q3Action* qa )
{
	if ( qa == fullTreeAction )
	{
		dz->setHiglighModeFullTree(true,  config->getTreeDependencyLevel());
		return ;
	}
	else
		dz->setHiglighModeFullTree(false);

	if ( qa == forwardAction ) {
		dz->setHighlightMode( false, true );
	}
	else if ( qa == backwardAction ) {
		dz->setHighlightMode( true, false );
	}
	else if ( qa == backAndForwardAction )
		dz->setHighlightMode( true, true );
}


void STVWindow::showHiddenSignals()
{
	int simpleSignals = dm->getSignalSlots();
	HiddenSignalsDialogImp hs(this, "Hidden Signals", true);
	string previous = "";
	for ( int i = 0; i < simpleSignals; i++ )
	{
		const string& signalName = dm->getSignalName(i);
		if ( signalName == previous )
			continue;
		previous = signalName;
		if ( !dz->isSignalVisible(signalName) )
            hs.HiddenSignalsList->insertItem(signalName.c_str());

		hs.HiddenSignalsList->sort();
	}

	if ( hs.exec() == QDialog::Accepted )
	{
		// extract selected signals and make them visible
		unsigned int count = hs.HiddenSignalsList->count();
		for ( unsigned int i = 0; i < count; i++ )
		{
			Q3ListBoxItem* item = hs.HiddenSignalsList->item(i);
			if ( item->selected() )
				dz->setSignalVisible(item->text(), true);
		}
		dz->repaint(); // force paint updated contents
	}
}


void STVWindow::showGoToCycle()
{
	int firstCycle;
	dm->getCycles(firstCycle);
	int cycleTarget;
	CycleGoImp cg( this, "ed", TRUE );
	char currentCycle[256];
	sprintf(currentCycle, "%d", dz->getCurrentCycle());
	cg.cycleGoLineEdit->setText(currentCycle);
	char msg[128];
	sprintf(msg, "Offset is added to first cycle (%d) in the current trace file", firstCycle); 
	cg.goToCycleMsgInfo->setText(msg);
	cg.cycleGoLineEdit->setFocus();
	cg.cycleGoLineEdit->setSelection(0,strlen(currentCycle));
	QIntValidator qiv( 0, dm->getCycles(), this );
	while ( cg.exec() )
	{
		QValidator::State state = qiv.validate( cg.cycleGoLineEdit->text(), cycleTarget);
		switch ( state )
		{
			case QValidator::Acceptable:
				cycleTarget = atoi( cg.cycleGoLineEdit->text() );
				sprintf(msg, "Seeking cycle %d. Wait please...", cycleTarget);
				setCaption(msg);
				statusBar()->message(msg);
				qsh->setValue(dz->getSquareSize()*cycleTarget);
				statusBar()->message("Search finished");
				restoreWindowCaption();
				return ;
			case QValidator::Intermediate:
				char msg[64];
				sprintf(msg, "Cycle offset out of range. Must be within the range [0 .. %d]", dm->getCycles());
				showError( "Error", msg );
				continue;
			case QValidator::Invalid:
				showError( "Error", "Go to cycle text must contain an integer" );
				continue;
		}
	}
}

void STVWindow::updateHorizontalPageStep(int squareSize)
{
	qsh->setPageStep(config->getPageStep()*squareSize);
	qsh->setLineStep(squareSize);
	qsv->setLineStep(squareSize);
}

void STVWindow::maintainCurrentCycle()
{
	qsh->setValue(dz->getSquareSize()*dz->getPreviousCycle());
}

void STVWindow::showOpenTraceDialog()
{
	QString fileName(QFileDialog::getOpenFileName(QString::null, "Tracefiles (*.txt *.dat);;All (*.*)", this));
	if ( !fileName.isEmpty() )
	{
		currentTraceFile = fileName;
		loadTraceFile(currentTraceFile.c_str(),0);
	}
}


void STVWindow::createMenuOptions()
{
    fileNewAction = new Q3Action( "Load New Signal Trace File", "Load &New Signal Trace File", Qt::CTRL+Qt::Key_N, this, "new" );
    fileReloadConfigAction = new Q3Action( "Reload config.ini", "&Reload config.ini", Qt::CTRL+Qt::Key_R, this, "Reload config.ini");
	fileReloadConfigAction->setEnabled(false);
    fileQuitAction = new Q3Action( "Quit", "&Quit", Qt::CTRL+Qt::Key_Q, this, "quit" );
	fileLoadLayout = new Q3Action( "Load Signal Layout", "Load signals layout" , 0, this, "load layout");
	fileLoadLayout->setEnabled(false);
	fileLoadNewLayout = new Q3Action("Load New Signal Layout", "Load New Signal Layout", 0, this, "load new layout");
	fileLoadNewLayout->setEnabled(false);	

	fileSaveLayout = new Q3Action( "Save Signals Layout","Save signals Layout", 0, this, "save layout");
	fileSaveLayout->setEnabled(false);
	fileSaveLayoutAs = new Q3Action("Save Signal Layout As...", "Save Signal Layout As...", 0, this, "save layout as");
	fileSaveLayoutAs->setEnabled(false);		

	fileLoadColorMap = new Q3Action("Load ColorMap", "Load ColorMap", 0, this, "load color map");
	fileLoadColorMap->setEnabled(false);

	viewZoomInAction = new Q3Action( "Zoom in", "Zoom &in", Qt::CTRL+Qt::Key_I, this, "Zoom in" );
	viewZoomInAction->setEnabled(false);
	viewZoomOutAction = new Q3Action( "Zoom out", "Zoom &out", Qt::CTRL+Qt::Key_O, this, "Zoom out" );
	viewZoomOutAction->setEnabled(false);

	showDivisionLinesAction = new Q3Action( "Show division lines", "Show division &lines",
		Qt::CTRL+Qt::Key_L, this, "Show division lines" );
	showDivisionLinesAction->setToggleAction( true );
	showDivisionLinesAction->setOn( config->isDivisionLinesOn() );

	viewCyclesRulerAction = new Q3Action( "Show cycle's ruler", "Show cycle's ruler",
		Qt::CTRL+Qt::Key_C, this, "Show cycles's ruler" );
	viewCyclesRulerAction->setToggleAction( true );
	viewCyclesRulerAction->setOn( config->isSignalsRulerOn() );

    viewSignalsRulerAction = new Q3Action( "Show signal's ruler", "Show signal's ruler",
		Qt::CTRL+Qt::Key_S, this, "Show signal's ruler" );
	viewAutoHideRulersAction = new Q3Action( "Auto hide rulers", "Auto hide rulers",
		Qt::CTRL+Qt::Key_H, this, "Auto hide rulers" );
	viewAutoHideRulersAction->setToggleAction( true );

	viewAutoHideRulersAction->setOn( config->isAutoHideRulersOn() );
	viewSignalsRulerAction->setToggleAction( true );
	viewSignalsRulerAction->setOn( config->isSignalsRulerOn() );

	forwardAction = new Q3Action( "View forward dependences", "View &forward dependences", 0, this );
	forwardAction->setToggleAction( true );
	backwardAction = new Q3Action( "View backward dependences", "View &backward dependences", 0, this );
	backwardAction->setToggleAction( true );
	backAndForwardAction = new Q3Action( "View back and forward dependences", "View back and forward dependences", 0, this );
	backAndForwardAction->setToggleAction( true );
	fullTreeAction = new Q3Action("View full tree of dependences", "View full &tree of dependences", 0, this);
	fullTreeAction->setToggleAction(true);
	backAndForwardAction->setOn(true);
	
	viewDependencesGroup = new Q3ActionGroup( this );
	viewDependencesGroup->insert( forwardAction );
	viewDependencesGroup->insert( backwardAction );
	viewDependencesGroup->insert( backAndForwardAction );
	viewDependencesGroup->insert( fullTreeAction );
	viewDependencesGroup->setEnabled( false);

	preferencesAction = new Q3Action( "Preferences", "&Preferences", Qt::CTRL+Qt::Key_P, this, "Preferences" );
	aboutAction = new Q3Action( "About", "&About", 0, this, "About visualizer");
	goToCycleAction = new Q3Action( "Go to cycle", "&Go to cycle...", Qt::CTRL+Qt::Key_G, this, "Go to cycle");
	goToCycleAction->setEnabled(false);

	showHiddenSignalsAction = new Q3Action("Show hidden signals", "&Show hidden signals", Qt::CTRL+Qt::Key_H, this, "Show hidden signals");
	showHiddenSignalsAction->setEnabled(false);

	fileMenu = new Q3PopupMenu( this );
	searchMenu = new Q3PopupMenu( this );
	viewMenu = new Q3PopupMenu( this );
	dependenciesMenu = new Q3PopupMenu( this );
	optionsMenu = new Q3PopupMenu( this );
	helpMenu = new Q3PopupMenu( this );

    menuBar()->insertItem( "&File", fileMenu );
	menuBar()->insertItem( "&Search", searchMenu );
	menuBar()->insertItem( "&View", viewMenu );
	menuBar()->insertItem( "&Dependences", dependenciesMenu );
	menuBar()->insertItem( "&Options", optionsMenu );
	menuBar()->insertItem( "&Help", helpMenu );

    fileNewAction->addTo( fileMenu );
	fileReloadConfigAction->addTo( fileMenu );
	fileMenu->insertSeparator();
	fileLoadLayout->addTo( fileMenu );
	fileLoadNewLayout->addTo(fileMenu);
	fileMenu->insertSeparator();
	fileSaveLayout->addTo( fileMenu );
	fileSaveLayoutAs->addTo(fileMenu );
	fileMenu->insertSeparator();
	fileLoadColorMap->addTo(fileMenu);
	fileMenu->insertSeparator();
    fileQuitAction->addTo( fileMenu );
	goToCycleAction->addTo( searchMenu );
	showHiddenSignalsAction->addTo( searchMenu );
	viewZoomInAction->addTo( viewMenu );
	viewZoomOutAction->addTo( viewMenu );
	viewMenu->insertSeparator();
	showDivisionLinesAction->addTo( viewMenu );
	viewMenu->insertSeparator();
	viewCyclesRulerAction->addTo( viewMenu );
	viewSignalsRulerAction->addTo( viewMenu );
	viewAutoHideRulersAction->addTo( viewMenu );
	viewDependencesGroup->addTo( dependenciesMenu );
	preferencesAction->addTo( optionsMenu );
	aboutAction->addTo( helpMenu);
}

void STVWindow::destroyObjects()
{
	delete dz;
	delete dm;
}

// ignored newVPos
void STVWindow::moveRulers(int newHPos, int /*newVPos*/ )
{
	qsh->setValue(newHPos);
}


void STVWindow::showAbout()
{
	static const char* info =
		"Signal Trace Visualizer " STV_VER "\n"
		"Built: " __DATE__ " " __TIME__ "\n\n"
		"Author: " STV_AUTHOR "\n"
		"eMail: " STV_AUTHOR_EMAIL "\n"
		"\n"
		" What's new:\n"
		"   - 1.1 Implemented using QT4.4 & VS2005.NET\n"
        "   - 1.1 New fexible and robust format to define color map (not compatible with old ones)\n"
        "   - 1.2 Internal code renamings & upgraded to use C++ code instead of C-like code\n"
        "   - 1.2 Support to work with corrupted traces (not properly flushed, completed, etc)\n"
        "   - 1.2.1 Default SignalTraceFile option supported in config.ini\n"
		;

	QMessageBox::information(this, "About", info, "Ok");
}


void STVWindow::saveLayoutAs()
{
	QString fileName(QFileDialog::getSaveFileName(QString(currentLayoutFile.c_str()), "Tracefiles (*.layout *.dat);;All (*.*)", this));
	if ( !fileName.isEmpty() )
	{
		QFileInfo pathInfo(fileName);
		if ( pathInfo.extension(false) == "" )
			currentLayoutFile = fileName + ".layout";
		else
			currentLayoutFile = fileName;
		
		pathInfo.setFile(currentLayoutFile.c_str());
		QString name = pathInfo.fileName();
		
		std::string newLoadMenuText = std::string("Load Signals Layout (") + name.latin1() + ")";
		std::string newSaveMenuText = std::string("Save Signals Layout (") + name.latin1() + ")";
		
		saveLayout();

		fileSaveLayout->setMenuText(newSaveMenuText.c_str());
		fileLoadLayout->setMenuText(newLoadMenuText.c_str());
		fileLoadLayout->setEnabled(true);
		fileSaveLayout->setEnabled(true);
	}
}

void STVWindow::loadNewLayout()
{
	QString fileName(QFileDialog::getOpenFileName(QString::null, "Layout files (*.layout);;All (*.*)", this));
	if ( !fileName.isEmpty() )
	{
		currentLayoutFile = fileName;

		QFileInfo pathInfo(fileName);
		QString name = pathInfo.fileName();

		std::string newLoadMenuText = std::string("Load Signals Layout (") + name.latin1() + ")";
		std::string newSaveMenuText = std::string("Save Signals Layout (") + name.latin1() + ")";

		loadLayout();

		fileSaveLayout->setMenuText(newSaveMenuText.c_str());
		fileLoadLayout->setMenuText(newLoadMenuText.c_str());
		fileLoadLayout->setEnabled(true);
		fileSaveLayout->setEnabled(true);
	}
}

void STVWindow::saveLayout()
{	
	if ( currentLayoutFile == "" )
		saveLayoutAs();
	else
		dz->saveLayout(currentLayoutFile);
}


void STVWindow::loadLayout()
{
	if ( currentLayoutFile == "" )
		loadNewLayout();
	else
		dz->loadLayout(currentLayoutFile);

}

void STVWindow::loadColorMap()
{
	QString fileName(QFileDialog::getOpenFileName(QString::null, "ColorMap files (*.txt);;All (*.*)", this));
	if ( !fileName.isEmpty() )
		dz->loadColorMap(fileName.latin1());		

}

void STVWindow::restoreWindowCaption()
{
	if ( dz )
	{
		char msg[1024];
		sprintf(msg, "%s - %s", STV_TITLE_TXT, tracePath);
		setCaption(msg);
	}
	else
		setCaption(STV_TITLE_TXT);
}

// convenient slot to force config.ini reloading
void STVWindow::reloadConfig_ini()
{
	config->loadConfiguration();
	applyConfig_ini();
}

void STVWindow::applyConfig_ini()
{
	dz->showSignalsRuler( config->isSignalsRulerOn() );
	dz->showCyclesRuler( config->isCyclesRulerOn() );	
	dz->showLines( config->isDivisionLinesOn());

	qsh->setPageStep(config->getPageStep()*config->getSizeSquares());
	qsh->setLineStep(config->getSizeSquares());
	qsv->setLineStep(config->getSizeSquares());

	dz->setColorOptions( config->getColorOptions() );
	dz->setShowIndividualInfo( config->isShowIndividualInfo() );
	dz->setShowDefaultInfo( config->isShowDefaultInfo() );

	// Try to load defaults layout and colormap
	std::string auxStrPath = config->getLayoutFile();
	if ( !auxStrPath.empty() )
	{
		if ( dz->loadLayout(auxStrPath) )
			currentLayoutFile = auxStrPath;
	}
	auxStrPath = config->getColorMapFile();
	if ( !auxStrPath.empty() )
		dz->loadColorMap(auxStrPath);		

	if ( fullTreeAction->isOn() )
		dz->setHiglighModeFullTree(true, config->getTreeDependencyLevel());

    /*
    auxStrPath = config->getSignalTraceFile();
    QMSG(auxStrPath.c_str());
    if ( !auxStrPath.empty() )
        loadTraceFile(auxStrPath.c_str(),0);
    */

}
