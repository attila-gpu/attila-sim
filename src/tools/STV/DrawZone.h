#ifndef DRAWZONE_H
	#define DRAWZONE_H

#include <vector>
#include <qwidget.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qevent.h>
#include <cstdio>
#include <windows.h>
#include "DataManager.h"
#include <string>
#include <utility>
#include <qmainwindow.h>
#include <QT3Support/Q3PopupMenu>
#include <QT3Support/Q3Action>
#include <QT3Support/Q3MainWindow>
//#include <QMainWindow>
#include <vector>

/**
 * DrawZone class implements a QT Widget for visualizing all the contents provided by a DataManager
 *
 * It is a user interface and has all behaviours implemented for an easy navigation
 *
 * The comunication between DataManager and DrawZone is done via SignalSlotData objects
 *
 * @version 2.0
 * @date 24/10/2008
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es
 */
class DrawZone : public QWidget
{
	Q_OBJECT
public:
	
	/**
	 * Defines a "drawable" area associated with a concrete DataManager
	 *
	 * @param parent parent QWidget ( for example QWindow )
	 * @param dm Associated DataManager ( provides the tracefile information via SignalSlotData objects )
	 * @param sizeItems size for squares representing a SignalSlotData data
	 */
	DrawZone( Q3MainWindow* app, QWidget* parent, DataManager& dm, int sizeItems = 20 );
	
	/**
	 * Obtains the current square's size
	 *
	 * @return current square's size
	 */ 
	int getSquareSize() const;

	int getCurrentCycle() const;

	int getPreviousCycle() const { return oldCycle; }

	/**
	 * Returns if a given signal (identified by its name) is visible currently
	 */
    bool isSignalVisible(const std::string& signalName);

	/**
	 * Sets if a signal is visible or not
	 *
	 * If a signal is vis
	 */
	bool setSignalVisible(const char* signalName, bool visible);

	/**
	 * Obtains the real dimension in pixels ( not the part shown in the screen ) that
	 * should have the screen to show all visual data
	 *
	 * @param hPixels out parameter real horizontal pixels
	 * @param vPixels out parameter real vertical pixels
	 */
	void getRealPixelsSize( int& hPixels, int& vPixels);

	/**
	 * Adds an empty row in a given position
	 *
	 * Used for spacing other rows
	 */
	void addSeparatorRow(int position);

	void collapseRow(int position);


	void saveLayout(const std::string& path);
	bool loadLayout(const std::string& path);

	bool loadColorMapOld(const std::string& path);	
    bool loadColorMap(const std::string& path);	
    static bool isNumber(const std::string& numberStr);


	void showCurrentColorMap();

	void setShowDefaultInfo(bool di) { showDefaultInfo = di; }
	void setShowIndividualInfo(bool ii) { showIndividualInfo = ii; }
	void setColorOptions(int co) { colorOptions = co; }
	
public slots:

	void showCycleMessage(int cycle);

	void doMoveRows();
	void doRemoveRows();
	void doAddSepRow();
	void doUnselecRows();

	void doCollapseRow();

	void doRemoveOneRow();
	void doShowDependencies();
	void doShowExtendedInfo();
	void doGoToCycle();

	/**
	 * Perform a 'zoom in' if possible
	 */
	void zoomIn();

	/**
	 * Perform a 'zoom out' if possible
	 */
	void zoomOut();

	/**
	 * Perform a 'zoom in' with specified pixels increasement ( if possible )
	 *
	 * @param incPixels pixels each square will be increased
	 */
	void zoomIn( int incPixels );
	
	/**
	 * Perform a 'zoom out' with specified pixels increasement ( if possible )
	 *
	 * @param decPixels pixels each square will be decreased
	 */
	void zoomOut( int decPixels );

	/**
	 * Resize the square's size
	 *
	 * @param newSize new square's size
	 */
	void setNewItemSize( int newSize );
		
	/**
	 * Displace the current view an arbitrary number of pixels vertically, up or down ( if possible )
	 *
	 * positive pixel values means displacing in down-direction, negative means up-direction
	 *
	 * @param pixels number of pixels we want to displace
	 */
	int displaceV( int pixels );
	
	/**
	 * Displace the current view an arbitrary number of pixels horizontally, left or rigth ( if possible )
	 *
	 * positive pixel values means displacing in rigth-direction, negative means left-direction
	 *
	 * @param pixels number of pixels we want to displace
	 */
	int displaceH( int pixels );


	/**
	 * Set the drawing flag for help lines
	 *
	 * @param visible true if we want to show lines, false otherwise
	 */
	void showLines( bool visible );

	/**
	 * Set the drawing flag for Signals ruler
	 *
	 * @param visible true if we want to show the ruler, false otherwise
	 */
	void showSignalsRuler( bool visible );
	
	/**
	 * Set the drawing flag for Cycles ruler
	 *
	 * @param visible true if we want to show the ruler, false otherwise
	 */
	void showCyclesRuler( bool visible );		
	
	/**
	 * Sets the highlighting of some squares given a list of cookies and two flags
	 *
	 * @param cookies cookies
	 * @param nCookies number of cookies
	 * @param backward indicates if we want to highlight ancestor squares
	 * @param forward indicates if we want to highlight successor squares
	 */
    void highlightCycles( const std::vector<int> cooks, bool backward, bool forward );
	
	/**
	 * Sets the mode of highlighting
	 *
	 * @param backward indicates if we want to highlight ancestor squares
	 * @param forward indicates if we want to highlight successor squares
	 */
	void setHighlightMode( bool backward, bool forward ); // use previous cookies

	/**
	 * Sets the special higlighting mode to show the full tree of dependences
	 * for a single item
	 */
	void setHiglighModeFullTree(bool enable, unsigned int plevel = 1);
	
	/**
	 * Disable highlighting
	 */
	void unhighlightCycles();

	std::string getStateString() const;

	/**
	 * Dumps some internal state for debug purposes
	 */
	void dump() const;

signals:

	/**
	 * Emit this signal when real pixel size changes
	 *
	 * @param newHPixelsSize out parameter, new real horizontal size in pixels
	 * @param newVPixelsSize out parameter, new real vertical size in pixels
	 */
	void realPixelsChanged( int newHPixelsSize, int newVPixelsSize );

	/**
	 * Emit this signal when widget Square size changes
	 */	
	void squareSizeChanged(int newSquareSize);


	/**
	 * Emit this cycle when widget contents are displaced via doGoToCycle(X)
	 */ 
	void squarePosition(int row, int column);


protected:

	/**
	 * Called automatically when a repaint is required ( can be forced calling repaint method )
	 */
	void paintEvent( QPaintEvent* );

	/**
	 * Called automatically when a mouse single click event has been received
	 */
	void mousePressEvent ( QMouseEvent* );


	/**
	 * Track mouse...
	 */
	void mouseMoveEvent(QMouseEvent* );
	
	void mouseDoubleClickEvent (QMouseEvent* );


private:	

	enum // predefinde color
	{ 
		BLACK = 0,
		WHITE = 1,
		RED = 2,
		GREEN = 3,
		BLUE = 4,
	};

	// This names are maintained for compatibility with old code
	unsigned int UNDEF_COLOR; // = COLOR_TABLE.size() - 1
	unsigned int HIGHLIGHTED_COLOR; // = UNDEF_COLOR

	std::vector<QColor> COLOR_TABLE;

	int firstCycle;
	int colorOptions;
	bool showDefaultInfo;
	bool showIndividualInfo;

	struct CInfo
	{
		QColor color;
		std::string info;
	};

	// A map color for a unique signal (one shared for all simple signals from a given simple signal)
	typedef std::map<int, CInfo> CMap;
	
	// All simpleSignals from a Signal share its CMAP;
	std::vector<CMap*> colorMap;


	// simpleSig -> simple Signal identificator
	// color -> original simple signal color
	const CInfo* getColorInfo(int simpleSig, int color)
	{
		CMap* cm = colorMap[simpleSig];
		if ( !cm )
			return 0;
		CMap::iterator it = cm->find(color);
		if ( it == cm->end() )
			return 0;
		return &(it->second);
	}

	/**
	 * Returns all 'siblings' of the simple signal identified by 'simpleSignal'
	 * sibling simple signals are all simple signals that are generated from the same
	 * original signal
	 *
	 * @note Returns all siblings included the simpleSignal passes as parameter
	 *
	 * @param simpleSignal Simple signal identifier in the current visual layout
	 */
	std::vector<int> findSiblings(int simpleSignal);

	void setupSignalPopup();
	void setupCyclePopup();

	int lastSelected; // Last signal selected

	int currentRow;
	int currentCycle;

	Q3PopupMenu* signalPopup;

	Q3Action* moveRowsAction;
	Q3Action* removeRowsAction;
	Q3Action* addSepRowAction; // shared in both signal & cycle popup menus
	Q3Action* unselecRowsAction;
	Q3Action* collapseSignal;

	Q3PopupMenu* cyclePopup;

	Q3Action* removeOneRowAction;
	Q3Action* showDependenciesAction;
	//Q3Action* addSepRowAction; // shared in both signal & cycle popup menus
	Q3Action* showExtendedInfoAction;
	Q3Action* goToCycleAction;


	// Associated main application
	Q3MainWindow* app;

	struct SignalState
	{
		int map;
		bool selected;
		bool collapsed;

		SignalState(int map_= -1) : map(map_), selected(false), collapsed(false) {}
		
		SignalState(int map_, bool selected_, bool collapsed_) :
			map(map_), selected(selected_), collapsed(collapsed_)
		{}
	};

	std::vector<SignalState> signalState;

	//std::vector<int> signalMap;
	//std::vector<bool> selectedSignals;

	int oldCycle;

	bool linesEnabled; ///< flag that indicates if help lines must be drawn or not
	
	bool backward; ///< backward flag for highlighting mode
	bool forward; ///< forward flag for highlighting mode
	bool fullTree; ///< Full three dependecies
	unsigned int fullTreeParentLevel; ///< 0 means full tree from the root
	
	std::vector<int> cookies; // Changed since 0.81
	//int* cookies; ///< cookies for highlighting
	//int nCookies; ///< number of cookies

	int S_SIZE; ///< Current square's size

	int ORIGIN_X; ///< current Initial X position for drawing squares
	int ORIGIN_Y; ///< current Initial Y position for drawing squares

	int defaultOriginX; ///< default Initial X position for drawing squares
	int defaultOriginY; ///< default Initial Y position for drawing squares

	DataManager& dm; ///< Data that can be show
		
	int itemsH; ///< horizontal squares ( total, not shown currently )
	//int itemsV; ///< verticall squares ( total, not shown currently )

	int offsetV; ///< current vertical offset in pixels ( initially 0 )
	int offsetH; ///< current horizontal offset in pixels ( initially 0 )

	/**
	 * Draw a box with a specific message in an absolute position of current view
	 *
	 * @param p QPainter object needed to draw in a QWidget
	 * @param x vertical position in pixels ( starting at 0 )
	 * @param y horizontal position in pixels ( starting at 0 )
	 * @param w width for the box
	 * @param h height for the box
	 * @param text message to be shown inside the box
	 * @param textColor Color for the text
	 * @param boxColor Color for the bounding box
	 * @param boxVisible flag to indicate if the bounding box is visible or not
	 */
	void drawTextInBox( QPainter& p, int x, int y, int w, int h, 
							 const char* text, QColor& textColor, QColor& boxColor = QColor(255,0,0),
							 bool boxVisible = true );	

	/**
	 * Obtains the coordinates that identify a Square in a DrawZone specific region given
	 * the pixel (x,y) in the DrawZone
	 *
	 * @param xPixel pixel's coordinate x
	 * @param yPixel pixel's coordinate y
	 * @param xOI out parameter, x coordinate for the corresponding object in the data Manager at x row
	 * @param yOI out parameter, y coordinate for the corresponding object in the data manager at y column
	 */
	void pixelToCoord( int xPixel, int yPixel, int& xOI, int& yOI );
	
	/**
	 * Obtains the pixel(x,y) in which an object defined by its x, y components is starting to be painted
	 *
	 * @param xOI object x coordinate ( row )
	 * @param yOI object y coordinate ( column )
	 * @param xPixel first x pixel to be painted
	 * @param yPixel first y pixel to be painted
	 */
	void coordToPixel( int xOI, int yOI, int& xPixel, int& yPixel );

	/**
	 * Draws a square given an x and y object coordinates
	 *
	 * @param p QPainter object required to paint in a QWidget
	 * @param x row coordinate
	 * @param y column coordinate
	 * @param color QT Color object used to draw the wired square
	 * @param solid if true, the square is drawn solid, wired otherwise
	 */
	void drawItem( QPainter& p, int x, int y, const QColor& color, bool solid = false );
	
	/**
	 * Draws the cycles ruler
	 *
	 * @param p QPainter object required to paint in a QWidget
	 */
	void drawCyclesRuler( QPainter& p );
	
	/**
	 * Draws the signals ruler
	 *
	 * @param p QPainter object required to paint in a QWidget
	 */
	void drawSignalsRuler( QPainter& p );

	/**
	 * Check if a SignalSlotData object must be highlighted or not
	 *
	 * @return true if must be highlightd, false otherwise
	 */
	bool mustBeHighlighted( const SignalSlotData* ssi );

	void setHiglightColor(const QColor& newHighlighColor);

	QColor getHighlightColor() const;


	/**
	 * Called when some simple signals must to be moved
	 */ 
	void moveSignalSlots(int destination);
	
};


#endif // DRAWZONE_H
