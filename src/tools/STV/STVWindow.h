#ifndef STVWINDOW_H
    #define STVWINDOW_H

// #include <qmainwindow.h>
#include <QT3Support/Q3MainWindow>
// #include <QMainWindow>
#include <QT3Support/Q3Action>
#include <qslider.h>
#include <QT3Support/Q3HBox>
#include <QT3Support/Q3VBox.h>
#include <string>
#include "PreferencesDialog.h"
#include "ui_PreferencesDialog.h"
#include "DrawZone.h"
#include "DataManager.h"
#include "ConfigurationManager.h"

class STVWindow : public Q3MainWindow
{
	Q_OBJECT

public:

    //STVWindow();
    STVWindow( QWidget* parent = 0, const char* name = 0, Qt::WFlags f = Qt::WType_TopLevel );
    
	void keyPressEvent( QKeyEvent* e );

public slots:

	void editPreferences();
	void showAbout();
	void showGoToCycle();
	void showDebug( int value );
	void updateRulers( int newHMax, int newVMax );
	
	void moveRulers(int newHPos, int newVPos );

	void updateDependences( Q3Action* qa );

	void updateHorizontalPageStep(int SquareSize);
	void maintainCurrentCycle();
	void showOpenTraceDialog();
	void showHiddenSignals();

	void saveLayout();
	void loadLayout();
	void saveLayoutAs();
	void loadNewLayout();

	void loadColorMap();

	void reloadConfig_ini();

	/**
	 * Restore standard window title
	 */
	void restoreWindowCaption();

private:
	
	char tracePath[1024];
	std::string currentTraceFile;
	std::string currentLayoutFile;
	std::string currentColorMapFile;

	DataManager* dm;
	DrawZone* dz;
	ConfigurationManager* config;

	Q3Action* showDivisionLinesAction;
	Q3Action* viewCyclesRulerAction;
	Q3Action* viewSignalsRulerAction;
	Q3Action* viewAutoHideRulersAction;
	Q3Action* forwardAction;
	Q3Action* backwardAction;
	Q3Action* backAndForwardAction;
	Q3Action* fullTreeAction;
	
	Q3Action* viewZoomInAction;
	Q3Action* viewZoomOutAction;
	Q3Action* fileQuitAction;
	Q3Action* fileNewAction;
	Q3Action* fileReloadConfigAction;
	Q3Action* fileLoadLayout;
	Q3Action* fileSaveLayout;
	Q3Action* fileLoadNewLayout;
	Q3Action* fileSaveLayoutAs;
	Q3Action* fileLoadColorMap;

	Q3Action* preferencesAction;
	Q3Action* aboutAction;
	Q3Action* goToCycleAction;
	Q3Action* showHiddenSignalsAction;

	Q3PopupMenu* fileMenu;
	Q3PopupMenu* searchMenu;
	Q3PopupMenu* viewMenu;
	Q3PopupMenu* dependenciesMenu;
	Q3PopupMenu* optionsMenu;
	Q3PopupMenu* helpMenu;

	Q3ActionGroup* viewDependencesGroup;

	// PreferencesDialog* d;
    //Ui_PreferencesDialog* d;
    PreferencesDialogImp* d;

	QSlider* qsv;
	QSlider* qsh;

	Q3VBox* vBox;
	Q3HBox* hBox;

	void createMenuOptions();

	void destroyObjects();

	void loadTraceFile(const char* path, int nCycles);
	void showError( const char* title = 0, const char* messageError = 0 );

	void applyConfig_ini();
	

};

#endif //STVWINDOW_H
