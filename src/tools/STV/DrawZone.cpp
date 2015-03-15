#include "DrawZone.h"
#include <windows.h>
#include <cstdio>
#include <QDialog.h>
#include <QPushButton.h>
#include <list>
#include <qstatusbar.h>
#include "ui_CycleDialog.h"
#include <qlistview.h>
#include <fstream>
#include <QMessageBox.h>
#include "stv.h"
#include <qregexp.h>
#include <sstream>

#include "CycleDialog.h" // includes class CycleDialogImp

/*
#define BLACK 0
#define WHITE 1
#define RED 2
#define GREEN 3
#define BLUE 4
#define UNDEF_COLOR 23
#define HIGHLIGHTED_COLOR UNDEF_COLOR
*/

/**
 * Macros for easy access using the intermediate maps
 */
#define GET_DATA(dm,i,j) dm.getData(signalState[i].map,j)
#define GET_SIMPLE_SIGNAL_NAME(dm,x) dm.getSignalSlotName(signalState[x].map)
#define GET_SIGNAL_NAME(dm,x) dm.getSignalName(signalState[x].map)
#define GET_SIMPLE_SIGNALS(dm,x) dm.getSignalSlots(GET_SIGNAL_NAME(dm,x))
#define GET_COLORMAP(i_) colorMap[signalState[i_].map]
#define IS_SIGNAL_SEPARATOR(x) (signalState[x].map == -1)

#define QMSG(msg) QMessageBox::information(0, "QMessage", msg, QMessageBox::Ok);

using namespace std;

/*
QColor COLOR_TABLE[] =
{
	QColor(0,0,0), // black 0
	QColor(255,255,255), // white 1
	QColor(255,0,0), // red 2
	QColor(0,255,0), // green 3 
	QColor(0,0,255), // blue 4
	QColor(255,128,0), // Orange 5
	QColor(0,255,255), // brilliant blue 6
	QColor(255,128,192), // 7
	QColor(64,128,128), // 8
	QColor(255,230,254), // 9
	QColor(128,255,255), // 10
	QColor(255,128,255), // 11
	QColor(255,255,128), // 12
	QColor(0,255,192), // 13
	QColor(0,192,192), // 14
	QColor(0,0,192), // 15
	QColor(128,128,192), // 16
	QColor(130,180,140), // 17
	QColor(200,180,200), //18
	QColor(192,92,0), // 19
	QColor(192,128,128), //20
	QColor(255,0,128), // 21
	QColor(0,255,128), // 22
	QColor(255,255,0) // HIGHLIGHT COLOR // 23
};
*/

//#define MSG(title,msg) MessageBoxA( NULL, msg, title, MB_OK );

#ifndef NULL
	#define NULL 0
#endif

using namespace std;

void DrawZone::setHiglightColor(const QColor& newHighlightColor) 
{
	COLOR_TABLE.back() = newHighlightColor;
}

QColor DrawZone::getHighlightColor() const
{
	return COLOR_TABLE.back();
}


DrawZone::DrawZone( Q3MainWindow* app, QWidget* parent, DataManager& dm, int sizeItems  ) : 
	QWidget(parent), dm(dm), app(app), offsetV(0), offsetH(0), S_SIZE(sizeItems),
	linesEnabled(true), defaultOriginX(200), defaultOriginY(30),
	backward(true), forward(false), fullTree(false), fullTreeParentLevel(0), lastSelected(-1),
	colorOptions(0), showDefaultInfo(false), showIndividualInfo(false), firstCycle(0)
{	
	COLOR_TABLE.push_back(QColor(0,0,0)); // black 0
	COLOR_TABLE.push_back(QColor(255,255,255)); // white 1
	COLOR_TABLE.push_back(QColor(255,0,0)); // red 2
	COLOR_TABLE.push_back(QColor(0,255,0)); // green 3 
	COLOR_TABLE.push_back(QColor(0,0,255)); // blue 4
	COLOR_TABLE.push_back(QColor(255,128,0)); // Orange 5
	COLOR_TABLE.push_back(QColor(0,255,255)); // brilliant blue 6
	COLOR_TABLE.push_back(QColor(255,128,192)); // 7
	COLOR_TABLE.push_back(QColor(64,128,128)); // 8
	COLOR_TABLE.push_back(QColor(255,230,254)); // 9
	COLOR_TABLE.push_back(QColor(128,255,255)); // 10
	COLOR_TABLE.push_back(QColor(255,128,255)); // 11
	COLOR_TABLE.push_back(QColor(255,255,128)); // 12
	COLOR_TABLE.push_back(QColor(0,255,192)); // 13
	COLOR_TABLE.push_back(QColor(0,192,192)); // 14
	COLOR_TABLE.push_back(QColor(0,0,192)); // 15
	COLOR_TABLE.push_back(QColor(128,128,192)); // 16
	COLOR_TABLE.push_back(QColor(130,180,140)); // 17
	COLOR_TABLE.push_back(QColor(200,180,200)); //18
	COLOR_TABLE.push_back(QColor(192,92,0)); // 19
	COLOR_TABLE.push_back(QColor(192,128,128)); //20
	COLOR_TABLE.push_back(QColor(255,0,128)); // 21
	COLOR_TABLE.push_back(QColor(0,255,128)); // 22
	UNDEF_COLOR = COLOR_TABLE.size();
	HIGHLIGHTED_COLOR = UNDEF_COLOR;
	COLOR_TABLE.push_back(QColor(255,255,0)); // HIGHLIGHT COLOR // 23

	signalPopup = new Q3PopupMenu(this, "Signal options");
	
	moveRowsAction = new Q3Action("Move selected rows here", "Move selected rows here", 0, this, "Move selected rows here");	
	moveRowsAction->addTo(signalPopup);
	connect(moveRowsAction, SIGNAL(activated()), this, SLOT(doMoveRows()) );

	removeRowsAction = new Q3Action("Remove selected rows", "Remove selected rows", 0, this, "Remove selected rows");	
	removeRowsAction->addTo(signalPopup);
	connect(removeRowsAction, SIGNAL(activated()), this, SLOT(doRemoveRows()) );

	addSepRowAction = new Q3Action("Add separator row", "Add separator row", 0, this, "Add separator row");
	addSepRowAction->addTo(signalPopup);
	connect(addSepRowAction, SIGNAL(activated()), this, SLOT(doAddSepRow()));

	unselecRowsAction = new Q3Action("Unselect selected rows", "Unselect selected rows", 0,this, "Unselect selected rows");
	unselecRowsAction->addTo(signalPopup);
	connect(unselecRowsAction, SIGNAL(activated()), this, SLOT(doUnselecRows()));

	signalPopup->insertSeparator();

	collapseSignal = new Q3Action("Collapse signal", "Collapse signal", 0, this, "Collapse signal");
	collapseSignal->addTo(signalPopup);
	connect(collapseSignal, SIGNAL(activated()), this, SLOT(doCollapseRow()));

	cyclePopup = new Q3PopupMenu(this, "Cycle row options");
	/* Actions are added in setupCyclePopup */

	removeOneRowAction = new Q3Action("Remove one row", "Remove one row", 0, this, "Remove one row");
	connect(removeOneRowAction, SIGNAL(activated()), this, SLOT(doRemoveOneRow()) );

	showDependenciesAction = new Q3Action("Show dependencies", "Show dependencies", 0, this, "Show dependencies");
	connect(showDependenciesAction, SIGNAL(activated()), this, SLOT(doShowDependencies()));

	showExtendedInfoAction = new Q3Action("Show extended info", "Show extended info", 0, this, "Show extended info");
	connect(showExtendedInfoAction, SIGNAL(activated()), this, SLOT(doShowExtendedInfo()));

	goToCycleAction = new Q3Action("Go to cycle", "Go to cycle", 0, this, "Go to cycle");
	connect(goToCycleAction, SIGNAL(activated()), this, SLOT(doGoToCycle()));
	
	setMouseTracking(true);

	ORIGIN_X = defaultOriginX;
	ORIGIN_Y = defaultOriginY;
	itemsH = dm.getCycles(firstCycle);
	int itemsV = dm.getSignalSlots();
    setBackgroundMode(Qt::NoBackground); // mandatory  for efficient Double buffer

	int i;
	signalState.resize(itemsV);
	//signalMap.resize(itemsV);
	for ( i = 0; i < itemsV; i++ )
	{
		signalState[i].map = i;
		signalState[i].selected = false;
		signalState[i].collapsed = false;
		//signalMap[i] = 	i;
	}
/*
	selectedSignals.resize(itemsV);
	for ( i = 0; i < itemsV; i++ )
	selectedSignals[i] = false;
*/

	colorMap.resize(itemsV,0);

	//colorOptions = 2;
	//showDefaultInfo = true;
	//showIndividualInfo = true;
}


int DrawZone::getSquareSize() const
{
	return S_SIZE;
}

int DrawZone::getCurrentCycle() const
{
	return (offsetH / S_SIZE);
}


void DrawZone::zoomIn( int incPixels )
{
	oldCycle = getCurrentCycle();
	if ( S_SIZE >= 100 )
		return ;
	S_SIZE = S_SIZE + incPixels;
	if ( S_SIZE > 100 )
		S_SIZE = 100;
	if ( S_SIZE < 2 )
		S_SIZE = 2;
	repaint();
	// emit signal
	emit realPixelsChanged( S_SIZE*itemsH, S_SIZE*signalState.size());
	emit squareSizeChanged( S_SIZE );
}

void DrawZone::zoomIn()
{
	zoomIn( 1 );
}

void DrawZone::zoomOut( int decPixels )
{
	oldCycle = getCurrentCycle();
	if ( S_SIZE <= 2 )
		return;
	S_SIZE = S_SIZE - decPixels;
	if ( S_SIZE < 2 )
		S_SIZE = 2;
	if ( S_SIZE > 100 )
		S_SIZE = 100;
	repaint();
	// emit signal
	emit squareSizeChanged( S_SIZE );
	emit realPixelsChanged( S_SIZE*itemsH, S_SIZE*signalState.size() );
}

void DrawZone::zoomOut()
{
	zoomOut( 1 );
}

void DrawZone::setNewItemSize( int newSize )
{
	oldCycle = getCurrentCycle();
	S_SIZE = newSize;
	repaint();
	// emit signal
	emit realPixelsChanged( S_SIZE*itemsH, S_SIZE*signalState.size() );
	emit squareSizeChanged( S_SIZE );
	
}


void DrawZone::getRealPixelsSize( int& hPixels, int& vPixels)
{
	hPixels = itemsH*S_SIZE;
	vPixels = signalState.size()*S_SIZE;
}


void DrawZone::dump() const 
{
	dm.dump();
}



void DrawZone::pixelToCoord( int xPixel, int yPixel, int& xOI, int& yOI )
{
	xOI = ( offsetH + xPixel - ORIGIN_X ) / S_SIZE;
	yOI = ( offsetV + yPixel - ORIGIN_Y ) / S_SIZE;
}

// Coordenadas de pixel absoluto ( no la visibilidad actual )
void DrawZone::coordToPixel( int xOI, int yOI, int& xPixel, int& yPixel )
{
	xPixel = xOI*S_SIZE + ORIGIN_X;
	yPixel = yOI*S_SIZE + ORIGIN_Y;
}


// Use absolute coords
void DrawZone::drawTextInBox( QPainter& p, int x, int y, int w, int h, 
							 const char* text, QColor& textColor, QColor& boxColor,
							 bool boxVisible )
{
	if ( boxVisible ) {
		p.setPen( boxColor );
		p.drawRect( x, y, w, h );
	}
	p.setPen( textColor );
    p.drawText( x, y, w, h, Qt::AlignHCenter, text );
}


void DrawZone::drawCyclesRuler( QPainter& p ) 
{	
	p.setPen( RED );
	char buffer[10];
	int x; 
	
	int dummy;
	int i_first, i_last;	
	pixelToCoord( ORIGIN_X, 0, i_first, dummy );
	pixelToCoord( ORIGIN_X + width(),0, i_last, dummy );
	
	for ( int i = i_first; i <= i_last && i < itemsH; i++ ) {
		// computer x, y and sizes given offsetH and ORIGIN_X
		x = i*S_SIZE; // pixel lógico ( en la view )
		if ( x < offsetH ) { // perhaps partially viewed
			if ( offsetH - x < S_SIZE ) {// definitely -> partially visible
				sprintf( buffer, "%d", i );
				p.drawRect( ORIGIN_X, 0, S_SIZE - ( offsetH - x ), ORIGIN_Y );
                p.drawText( ORIGIN_X, 0, S_SIZE - ( offsetH - x ), ORIGIN_Y, Qt::AlignHCenter, buffer );
			}
		}
		else { // fully viewed
			sprintf( buffer, "%d", i );
			p.drawRect( x + ORIGIN_X - offsetH, 0, S_SIZE, ORIGIN_Y );
            p.drawText( x + ORIGIN_X - offsetH, 0, S_SIZE, ORIGIN_Y, Qt::AlignHCenter, buffer );
		}
	}
}

void DrawZone::drawSignalsRuler( QPainter& p )
{
	p.setPen( RED );
	int x;
	
	int i_first, i_last, dummy;
	pixelToCoord( 0, ORIGIN_Y, dummy, i_first );
	pixelToCoord( 0, ORIGIN_Y + height(), dummy, i_last );

	p.setBackgroundColor(COLOR_TABLE[5]);
	
	for ( int i = i_first; i <= i_last && i < (int)signalState.size(); i++ ) {
		
		if ( signalState[i].selected )
			p.setBackgroundMode(Qt::OpaqueMode);			
		else
			p.setBackgroundMode(Qt::TransparentMode);

		x = i*S_SIZE;
		if ( x < offsetV ) {
			if ( offsetV - x < S_SIZE ) {
				if ( IS_SIGNAL_SEPARATOR(i) )
				{
					p.drawText( 0, ORIGIN_Y, ORIGIN_X, S_SIZE - ( offsetV - x ), 
						Qt::AlignLeft, 
						"                                                ");
				}
				else
				{					
					string signalName;
					if ( signalState[i].collapsed )
						// sprintf(signalName, " + %s", GET_SIMPLE_SIGNAL_NAME(dm,i));
                        signalName = string(" + ") + GET_SIMPLE_SIGNAL_NAME(dm,i);
                    else
                        signalName = GET_SIMPLE_SIGNAL_NAME(dm,i);
						// strcpy(signalName, GET_SIMPLE_SIGNAL_NAME(dm,i));
                    

					p.drawRect( 0, ORIGIN_Y, ORIGIN_X, S_SIZE - ( offsetV - x ) );
					p.drawText( 0, ORIGIN_Y, ORIGIN_X, S_SIZE - ( offsetV - x ), 
						Qt::AlignLeft, 
                        signalName.c_str());
				}
			}
		}
		else {

			if ( IS_SIGNAL_SEPARATOR(i) )
			{
				p.drawText( 0, x + ORIGIN_Y - offsetV, ORIGIN_X, S_SIZE, 
                    Qt::AlignLeft, 
					"                                                ");
			}
			else
			{
				string signalName;
				if ( signalState[i].collapsed )
					// sprintf(signalName, " + %s", GET_SIMPLE_SIGNAL_NAME(dm,i));
                    signalName = string(" + ") + GET_SIMPLE_SIGNAL_NAME(dm,i);
				else
					// strcpy(signalName, GET_SIMPLE_SIGNAL_NAME(dm,i));
                    signalName = GET_SIMPLE_SIGNAL_NAME(dm,i);

				p.drawRect( 0, x + ORIGIN_Y - offsetV, ORIGIN_X, S_SIZE );
				p.drawText( 0, x + ORIGIN_Y - offsetV, ORIGIN_X, S_SIZE, 
                    Qt::AlignLeft, signalName.c_str());
			}
			
		}
	}
}


// drawItem an item
void DrawZone::drawItem( QPainter& p, int x, int y, const QColor& color, bool solid )
{
	p.setPen( color );	
	
	int xPixel, yPixel;
	int xSize, ySize;
	
	coordToPixel( x, y, xPixel, yPixel );
	// gets the pixel coords from the item is going to be painted
	
	// normalize subtracting ORIGIN_* and displace substracting offset_*
	xSize = ( xPixel - ORIGIN_X ) - offsetH;
 	ySize = yPixel - ORIGIN_Y - offsetV;

	// Compute new pixel coords and pixel sizes
	if ( xSize < 0 && ySize < 0 ) { // maybe partially visible
		//MSG( "info", "Partially x, y" );
		xSize += S_SIZE;
		ySize += S_SIZE;
		if ( xSize < 0 || ySize < 0 ) // definitely not visible
			return ;		
		xPixel = ORIGIN_X;
		yPixel = ORIGIN_Y;
	}
	else if ( xSize < 0 ) { // maybe partially visible
		xSize += S_SIZE;
		ySize = S_SIZE;
		if ( xSize < 0 ) // definitely not visible
			return ;
		xPixel = ORIGIN_X;
		yPixel = yPixel - offsetV;
	}
	else if ( ySize < 0 ) { // maybe partially visible
		xSize = S_SIZE;
		ySize += S_SIZE;
		if ( ySize < 0 ) // definitely not visible
			return ;
		xPixel = xPixel - offsetH;
		yPixel = ORIGIN_Y;
	}
	else {
		xSize = S_SIZE;
		ySize = S_SIZE;
		xPixel = xPixel - offsetH;
		yPixel = yPixel - offsetV;
	}
	
	// draw with the pixel coords ans new sizes already calculated
	if ( !solid ) {
		if ( linesEnabled )
			p.drawRect( xPixel-1, yPixel-1, xSize-1, ySize-1 );
	}
	else {
		p.fillRect( xPixel-1, yPixel-1, xSize-1, ySize-1, color );
		p.setPen( COLOR_TABLE[BLACK] );
		if ( linesEnabled )
			p.drawRect( xPixel-1, yPixel-1, xSize-1, ySize-1 );
	}
}


void DrawZone::showLines( bool visible )
{	
	linesEnabled = visible;
	repaint();
}


bool DrawZone::mustBeHighlighted( const SignalSlotData* ssi )
{
	if ( cookies.empty() )
		return false;

	if ( ssi == 0 )
		QMessageBox::critical(0, "Panic", "DrawZone::mustBeHighlighted() - crash", 
		QMessageBox::Ok, QMessageBox::NoButton);
		

	int i;
	const vector<int> cooks = ssi->getCookies();
    int n = cooks.size();

	if ( fullTree )
	{
		if ( n == 0 )
			return false;

		if ( fullTreeParentLevel == 0 || cookies.size() <= fullTreeParentLevel + 1 ) 
			return cookies.front() == cooks[0]; // full tree
		
		// check if the cookies matches with the partial dependency tree
		unsigned int limit = cookies.size() - fullTreeParentLevel;	
		for ( unsigned int i = 0; i < limit; i++ )
		{
			if ( cookies[i] != cooks[i] )
				return false;
		}
		return true;
	}

	if ( backward )
	{
		if ( n <= (int)cookies.size() ) // check backward dependencies
		{
			for ( i = 0; i < n; i++ )
			{
				if ( cookies[i] != cooks[i] )
					break;
			}
			if ( i == n )
				return true;				
		}
	}

	// no backward dependencies here (or backward == false)
	// check forward (if forward == true )
	if ( forward )
	{
		if ( n >= (int)cookies.size() )
		{
			for ( i = 0; i < (int)cookies.size(); i++ )
			{
				if ( cookies[i] != cooks[i] )
					return false;
			}
			return true;
		}
	}

	return false; // no dependencies
}


void DrawZone::paintEvent( QPaintEvent* /* event */)
{	
	QPixmap pm( size() );
	QPainter p;
	pm.fill( backgroundColor() );
	p.begin( &pm, this );
	
	// Draw cycles numbers...
	if ( ORIGIN_Y != 0 )
		drawCyclesRuler( p );
	if ( ORIGIN_X != 0 )		
		drawSignalsRuler( p );

	int i_first, i_last;
	int j_first, j_last;
	
	pixelToCoord( ORIGIN_X, ORIGIN_Y, j_first, i_first );
	pixelToCoord( width(), height(), j_last, i_last );
	
	const SignalSlotData* ssi;
	
	// i: Rows (simple signals)
	// j: Columns (cycles)
	for ( int i = i_first; i <= i_last && i < (int)signalState.size(); i++ ) {
		for ( int j = j_first; j <= j_last && j < itemsH; j++ ) 
		{
			if ( signalState[i].map == -1 )
			{
				// Separator...
				drawItem(p, j, i, COLOR_TABLE[GREEN]);
				continue;
			}

			if ( ( ssi = GET_DATA(dm,i,j) ) != NULL ) {
				// check if it should be highlighted
				if ( mustBeHighlighted( ssi ) ) {					
					drawItem( p, j, i, COLOR_TABLE[HIGHLIGHTED_COLOR], true );
				}
				else {

					const CInfo* ci = getColorInfo(signalState[i].map, ssi->getColor());
					if ( ci )
						drawItem( p, j, i, ci->color, true );
					else
					{
						int color = ssi->getColor() % UNDEF_COLOR;
						drawItem( p, j, i, COLOR_TABLE[color], true );
					}
					

				}
			}
			else 
				drawItem( p, j, i, COLOR_TABLE[BLACK] );			

		}
	}
	bitBlt( this, 0, 0, &pm );

}

void DrawZone::mouseMoveEvent(QMouseEvent* mEvent)
{
	int xPixels = mEvent->x();
	int y = mEvent->y();
	int x;
	
	if ( y < ORIGIN_Y )	
	{
		app->statusBar()->clear();
		return ;
	}

	pixelToCoord(xPixels, y, x, y); // y contains the simple signal target
	if ( y >= (int)signalState.size() )
	{
		app->statusBar()->clear();
		return ;
	}

	if ( IS_SIGNAL_SEPARATOR(y) )
	{
		app->statusBar()->message("No data. This row is merely a separator");
		return ;
	}

	if ( xPixels < ORIGIN_X )
		app->statusBar()->message("Mouse over simple signal name");
	else	
	{
		if ( y >= (int)signalState.size() || x >= itemsH ) // control out of range
		{
			app->statusBar()->message("Trace finishes here :-)", 2000);
			return;
		}

		const SignalSlotData* ssi = GET_DATA(dm,y,x);

		char cycleStr[16];
		if ( ssi )
			sprintf(cycleStr, ", %d)", ssi->getCycle());
		else 
			sprintf(cycleStr, ", %d)", x + firstCycle);
		
		char msg[1024];
        sprintf(msg, "(Signal, Cycle) : (%s", GET_SIMPLE_SIGNAL_NAME(dm,y).c_str());
		strcat(msg, cycleStr);
		
		if ( ssi )
		{
			if ( showIndividualInfo )
			{
				const string& info = ssi->getInfo();
				if ( !info.empty() )
				{
					strcat(msg, " --> info: ");
                    strcat(msg, info.c_str());
				}
			}
			if ( showDefaultInfo )
			{
				const CInfo* ci = getColorInfo(signalState[y].map, ssi->getColor());
				if ( ci ) // check for default info
				{
					strcat(msg, " --> def.info: ");
					strcat(msg, ci->info.c_str());
				}
			}

			char temp[256];
			int i = 0;
			switch ( colorOptions )
			{
				case 1: // just show color
					sprintf(temp, " --> color: %d ", ssi->getColor());
					strcat(msg, temp);
					break;
				case 2: // Cookies  & color					
                    {
					    strcat(msg, " --> Cookies;color: ");
					    const vector<int>& c = ssi->getCookies();
                        int nc = c.size();
					    for ( i = 0; i < nc; i++ )
					    {
						    if ( i < nc - 1)
							    sprintf(temp, "%d:", c[i]);
						    else
							    sprintf(temp, "%d", c[i]);
						    strcat(msg, temp);
					    }
					    sprintf(temp, ";%d", ssi->getColor());
					    strcat(msg, temp);
                    }
					break;
				default:
					; // nothing :-)
			}
		}
		else
			strcat(msg, " -> NO DATA");


		app->statusBar()->message(msg);

	}
}



void DrawZone::mouseDoubleClickEvent (QMouseEvent* mEvent)
{	
	
	int xPixels = mEvent->x();
	int y = mEvent->y();
	int x;
	
	if ( y < ORIGIN_Y )	
		return ;

	pixelToCoord(xPixels, y, x, y); // y contains the simple signal target
	if ( y >= (int)signalState.size() )
		return ;

	if ( xPixels < ORIGIN_X )
	{
		//selectedSignals[y] = !selectedSignals[y]; // undo changes done by previous SimpleClick event
		addSeparatorRow(y);
	}

	// else (ignore)
}



void DrawZone::mousePressEvent ( QMouseEvent* mEvent ) 
{
	int x = mEvent->x(); // x-cartesian axis
	int y = mEvent->y(); // y-cartesian axis

	int xPixels = x;
	int yPixels = y;

	if ( y < ORIGIN_Y )
		return;

	if ( x < ORIGIN_X )
	{
		pixelToCoord(x, y, x, y); // y contains the simple signal target

		if ( y >= (int)signalState.size() )
			return ;

        if ( mEvent->button() == Qt::LeftButton )
		{	
			if ( lastSelected != -1 && signalState[lastSelected].selected && (mEvent->state() & Qt::ShiftButton))
			{
				// multiple selection
				int i;
				for ( i = 0; i < (int)signalState.size(); i++ )
					signalState[i].selected = false;
				
				vector<int> v1 = findSiblings(lastSelected);
				vector<int> v2 = findSiblings(y);
				if ( y >= lastSelected ) // v2 elements are equal or greater than elements in v1
				{
					for ( int i = v1.front(); i <= v2.back(); i++ )
						signalState[i].selected = true;
				}
				else  // v1 elements are greater than elements en v2
				{
					for ( int i = v2.front(); i <= v1.back(); i++ )
						signalState[i].selected = true;
				}
				

				/*
				if ( y >= lastSelected )
				{
					for ( i = lastSelected; i <= y; i++ )
						selectedSignals[i] = true;
				}
				else // y < lastSelected
				{
					for ( i = y; i <= lastSelected; i++ )
						selectedSignals[i] = true;
				}
				*/
				
				
				repaint();
				// lastSelected is not updated (allows enlarging selection)
				return ;
			}

			// Simple selection
			vector<int> ss = findSiblings(y);
			for ( unsigned int j = 0; j < ss.size(); j++ )
				signalState[ss[j]].selected = !signalState[ss[j]].selected; 
			lastSelected = ss.back();
			/*
			selectedSignals[y] = !selectedSignals[y];
			lastSelected = y;
			*/
						
			repaint();
		}
		else
		{ 
			QPoint p = mapToGlobal(QPoint(xPixels,yPixels));
			
			currentCycle = x;
			currentRow = y;

			setupSignalPopup();
			
			signalPopup->exec(p);

			// right click (move selected signals)
			//moveSignalSlots(y);
			repaint();
		}

		return ;
		

	}
	pixelToCoord( x, y, y, x ); // Convert pixels coordinates in OI coordinates
	if ( x >= (int)signalState.size() || y >= itemsH ) // control out of range
		return;
	

	if ( mEvent->button() == Qt::RightButton ) 
	{		
		QPoint p = mapToGlobal(QPoint(xPixels,yPixels));
		
		currentCycle = y;
		currentRow = x;

		setupCyclePopup();

		cyclePopup->exec(p);
	}	
		
	else if ( mEvent->button() == Qt::LeftButton )
	{		

		if ( mEvent->state() & Qt::ShiftButton )
		{
			currentCycle = y;
			doGoToCycle();
			return ;
		}
		
		if ( IS_SIGNAL_SEPARATOR(x) ) // Left click in a separator row
			return ;

		currentRow = x;
		currentCycle = y;

		//doShowExtendedInfo();
		doShowDependencies();
	}
}


int DrawZone::displaceH( int pixels )
{	
	app->statusBar()->message("Seeking cycle. Wait please...");
	offsetH = pixels;
	repaint();
	app->statusBar()->message("Search finished");
	return offsetH;
}

int DrawZone::displaceV( int pixels )
{

	offsetV = pixels;
	repaint();
	return offsetV;

}

void DrawZone::showCyclesRuler( bool visible )
{
	if ( !visible )
		ORIGIN_Y = 0;
	else
		ORIGIN_Y = defaultOriginY;
	repaint();
}

void DrawZone::showSignalsRuler( bool visible )
{
	if ( !visible )
		ORIGIN_X = 0;
	else
		ORIGIN_X = defaultOriginX;
	repaint();
}


void DrawZone::highlightCycles( const vector<int> cooks, bool backward, bool forward )
{
	if ( !cookies.empty() )
		cookies.clear();

    int nCooks = cooks.size();
	for ( int i = 0; i < nCooks; i++ )
		cookies.push_back(cooks[i]);

	this->backward = backward;
	this->forward = forward;
}

void DrawZone::setHighlightMode( bool backward, bool forward )
{
	this->backward = backward;
	this->forward = forward;
	repaint();
}

void DrawZone::setHiglighModeFullTree(bool enable, unsigned int plevel)
{
	fullTree  = enable;
	fullTreeParentLevel = plevel;
	repaint();
}


void DrawZone::unhighlightCycles()
{
	cookies.clear();
	//backward = false;
	//forward = false;	
}

std::string DrawZone::getStateString() const
{
	using namespace std;

	char buf[1024];
	sprintf(buf, "offsetH = %d\n"
				 "offsetV = %d\n"
				 "ORIGIN_X = %d\n"
				 "ORIGIN_Y = %d\n",
				 offsetH, offsetV, ORIGIN_X, ORIGIN_Y);

	return string(buf);
}



void DrawZone::moveSignalSlots(int destination)
{
	int i;
	// avoid repainting if there are not selected signals
	for ( i = 0; i < (int)signalState.size(); i++ )
	{
		if (signalState[i].selected)
			break ;
	}
	if ( i == signalState.size() )
		return ;

	vector<int> siblings = findSiblings(destination);
	destination = siblings.front();

	if ( !signalState[destination].selected )
	{
		list<int> l;
		list<int> selected;
		for ( i = 0; i < (int)signalState.size(); i++ )
			l.push_back(i);

		list<int>::iterator it = l.begin();
		list<int>::iterator itPos;

		i = 0;
		while ( it != l.end() )
		{
			if ( i == destination )
				itPos = it; // store pos

			if ( signalState[i].selected )
			{
				selected.push_back(*it);
				it = l.erase(it);
			}
			else 
				it++;

			i++;
		}
		
		// l -> contains all unselected simple signals (included de destination, in itPos)
		// selected -> contains all selected signals

		l.insert(itPos, selected.begin(), selected.end());


		vector<SignalState> aux(signalState.size());
						
		it = l.begin();

		for ( i = 0; i < (int)signalState.size(); i++, it++ )
			aux[i] = signalState[*it];
		

		signalState = aux;


	}
	else
	{
		// clear selection
		for ( i = 0; i < (int)signalState.size(); i++ )
			signalState[i].selected = false;
	}
}

void DrawZone::collapseRow(int position)
{
	if ( 0 > position || position >= (int)signalState.size() )
		return ;

	vector<int> v = findSiblings(position);
	if ( v.size() == 1 )
	{
		// if it is already collapsed, expand it if possible.
		if ( signalState[position].collapsed )
		{
			//QMSG("Expand not yet implemented");
			// if the signal is selected, when is expanded all must be selected
			bool selected = signalState[position].selected;
			signalState[position].collapsed = false;

			const string& signalName = GET_SIGNAL_NAME(dm,position);
			int count = dm.countSignalSlots(signalName);

			// simple signals are inserted in reversed order
			int last = signalState[position].map + count - 1; // high (last) identifier first
			
			while ( last != signalState[position].map )
			{
				SignalState ss(last, selected, false);
				signalState.insert(signalState.begin() + position + 1, ss);
				last--;
			}
		}
		return ;
	}

	signalState[v.front()].collapsed = true;
	v.erase(v.begin());
	while ( !v.empty() )
	{
		signalState.erase(signalState.begin() + v.back());
		v.pop_back();
	}
}


void DrawZone::addSeparatorRow(int position)
{			
	if ( 0 > position || position >= (int)signalState.size() )
		return ;

	vector<int> aux = findSiblings(position);
	position = aux.front();
	vector<SignalState>::iterator it = signalState.begin();	
	for ( int i = 0; i < position; i++ )
		it++;

	SignalState ss(-1, false, false);
	signalState.insert(it, ss);

	repaint();
}


void DrawZone::doRemoveRows()
{
	vector<SignalState>::iterator it = signalState.begin();
	
	while ( it != signalState.end() )
	{
		if ( it->selected )
			it = signalState.erase(it);
		else
			it++;
	}

	repaint();
}



void DrawZone::doAddSepRow()
{
	addSeparatorRow(currentRow);
}

void DrawZone::doCollapseRow()
{
	collapseRow(currentRow);
}


void DrawZone::doMoveRows()
{
	moveSignalSlots(currentRow);
}


void DrawZone::doUnselecRows()
{
	int i;
	for ( i = 0; i < (int)signalState.size(); i++ )
		signalState[i].selected = false;
	repaint();
}

void DrawZone::setupSignalPopup()
{
	if ( signalState[currentRow].selected )
	{
		moveRowsAction->setEnabled(false);
		removeRowsAction->setEnabled(true);
		unselecRowsAction->setEnabled(true);
	}
	else
	{		
	int i;

		for ( i = 0; i < (int)signalState.size(); i++ )
		{
			if ( signalState[i].selected )
				break;
		}
		if ( i != signalState.size() )
			moveRowsAction->setEnabled(true);
		else 
			moveRowsAction->setEnabled(false);

		removeRowsAction->setEnabled(false);
		unselecRowsAction->setEnabled(false);
	}

	int count = 0;
	if ( !IS_SIGNAL_SEPARATOR(currentRow) )
	{
		count = dm.countSignalSlots( GET_SIGNAL_NAME(dm,currentRow) );
	}

	if ( count <= 1 )
		collapseSignal->setEnabled(false);
	else
	{
		collapseSignal->setEnabled(true);
		if ( signalState[currentRow].collapsed )
		{			
			collapseSignal->setMenuText("Expand signal");
			
		}
		else
			collapseSignal->setMenuText("Collapse signal");
	}	
}

void DrawZone::setupCyclePopup()
{
	bool isSeparator = IS_SIGNAL_SEPARATOR(currentRow);

	const char* sName = 0;
	if ( isSeparator )
        sName = GET_SIMPLE_SIGNAL_NAME(dm, currentRow).c_str();
	
	char buf[256];

	cyclePopup->clear(); // remove previous options

	removeOneRowAction->addTo(cyclePopup);
	if ( !isSeparator)
		sprintf(buf, "Remove row: %s", sName);
	else
		sprintf(buf, "Remove separator row");
	removeOneRowAction->setText(buf);
	
	addSepRowAction->addTo(cyclePopup);	

	if ( !isSeparator && (GET_DATA(dm,currentRow, currentCycle) != NULL) )
	{
		showDependenciesAction->addTo(cyclePopup);
		sprintf(buf, "Show dependencies for (%s, %d)", sName, currentCycle + firstCycle);
		showDependenciesAction->setText(buf);
	}

	if ( !isSeparator && (GET_DATA(dm, currentRow, currentCycle) != NULL) )
	{
		showExtendedInfoAction->addTo(cyclePopup);
		sprintf(buf, "Show extended information of (%s,%d)", sName, currentCycle + firstCycle);
		showExtendedInfoAction->setText(buf);
	}

	goToCycleAction->addTo(cyclePopup);
	sprintf(buf, "Go to cycle %d", currentCycle + firstCycle);
	goToCycleAction->setText(buf);
}

void DrawZone::doRemoveOneRow()
{	
	vector<SignalState>::iterator it = signalState.begin();

	vector<int> siblings = findSiblings(currentRow);
	int pos = siblings.front();
	int howManyToDelete = siblings.back() - pos + 1;

	for ( int i = 0; i < pos; i++ )
		it++;

	for ( int i = 0; i < howManyToDelete; i++ )
		it = signalState.erase(it);

	repaint();
}

void DrawZone::doShowDependencies()
{
	const SignalSlotData* ssi;
	if ( ( ssi = GET_DATA(dm,currentRow,currentCycle) ) != NULL ) {
		const vector<int> cks = ssi->getCookies();
        int nCks = cks.size();
		if ( nCks != cookies.size() ) 
		{
			unhighlightCycles();
			highlightCycles( cks, backward, forward );
		}
		else 
		{ // Check if it is the same 
			int i;
			for ( i = 0; i < nCks; i++ ) {
				if ( cookies[i] != cks[i] ) {
					unhighlightCycles();
					highlightCycles( cks, backward, forward );						
					break;
				}	
			}
			if ( i == nCks ) 
				unhighlightCycles();
		}
		
		repaint();
	}
}

void DrawZone::doShowExtendedInfo()
{
		int x = currentRow;
		int y = currentCycle;
		const SignalSlotData* ssi;
		if ( ( ssi = GET_DATA(dm,x,y) ) != NULL ) {
            CycleDialogImp* cd = new CycleDialogImp( this, "cdlg", true, 0 );
			cd->ListView1->setSorting( -1 );
			Q3ListViewItem* item = cd->ListView1->firstChild();
			
			char buffer[256];
			
			item->setText(0, tr("Cycle"));
			sprintf( buffer, "%d", ssi->getCycle());			
			item->setText( 1, tr(buffer) ); // cycle 
			
			item = item->nextSibling(); 
			item->setText(0, tr("Signal"));
            item->setText( 1, tr(GET_SIMPLE_SIGNAL_NAME(dm,x).c_str()) ); // signal name
							
			const vector<int> cookies = ssi->getCookies();
            int nCookies = cookies.size();
			const string& info = ssi->getInfo();
			char aux[10];		
			buffer[0] = 0;
			for ( int i = 0; i < nCookies; i++ ) {
				if ( i == nCookies - 1 )
					sprintf( aux, "%d", cookies[i] );
				else
					sprintf( aux, "%d:", cookies[i] );
				
				strcat( buffer, aux );
			}
			item = item->nextSibling();
			item->setText(0, tr("Cookies"));
			item->setText( 1, tr(buffer) ); // cookies
							
			sprintf( buffer, "%d", ssi->getColor() );
			item = item->nextSibling();
			item->setText( 0, tr("Color ID"));
			item->setText( 1, tr( buffer ) ); // color
			
			item = item->nextSibling();
			item->setText(0, tr("Info"));
			if ( !info.empty() )
                item->setText( 1, tr( info.c_str() ) );
			else
				item->setText( 1, tr( "NO info" ) );

			cd->exec(); // it doesn't care if accept or cancel
			delete cd;
		}	
}

void DrawZone::doGoToCycle()
{
	offsetH = S_SIZE * currentCycle;
	repaint();
	emit squarePosition(currentCycle*S_SIZE, currentRow*S_SIZE);
}


void DrawZone::saveLayout(const std::string& path)
{	
	ofstream f(path.c_str(), ios::trunc);

	if ( f )
	{
		string lastSignalName = "";
		for ( int i = 0; i < (int)signalState.size(); i++ )
		{
			if ( IS_SIGNAL_SEPARATOR(i) )
				f << "\n";
			else
			{
				string signalName = GET_SIGNAL_NAME(dm, i);
				if ( signalName != lastSignalName )
				{
					if ( signalState[i].collapsed )
						f << "+ ";
					f << signalName << "\n";
					lastSignalName = signalName;
				}
			}
		}
	}
	else
		QMessageBox::warning(0, "Warning", "Writing layout file failed", 
							 QMessageBox::Ok, QMessageBox::NoButton);

}


bool DrawZone::loadLayout(const std::string& path)
{
	ifstream f(path.c_str());

	if ( f )
	{
		signalState.clear();
		char buffer[1024];
		while ( !f.eof() )
		{		
			f.getline(buffer, sizeof(buffer));
			string signalPattern(buffer);
			if ( signalPattern.find_first_not_of(" ") != string::npos)
			{
				// Check if signal is collapsed
				int pos = signalPattern.find_first_not_of(" ");
				bool collapsed = false;
				if ( signalPattern.at(pos) == '+' )
				{
					pos = signalPattern.find_first_not_of(" ", pos+1);
					if ( pos == string::npos )
					{
						// just a separator with '+' in front
						SignalState separator(-1);
						signalState.push_back(separator);
						continue;
					}
					collapsed = true;
				}
				int pos2 = signalPattern.find_last_not_of(" ");
				signalPattern = signalPattern.substr(pos, pos2 - pos + 1); // trim leading whitespaces
				vector<string> signalNames = dm.getMatchingSignals(signalPattern.c_str());
				for ( int i = 0; i < signalNames.size(); i++ )
				{
					string signalName = signalNames[i];
					// not empty line (contains a signal name), insert all slots if the signal exists
					int simpleSignalCount = dm.countSignalSlots(signalName.c_str());
					if ( simpleSignalCount > 0 )
					{
						// signal name is present
						int pos = dm.getSignalSlotPosition(signalName.c_str());
						if ( pos < 0 )
							QMessageBox::critical(0, "Panic", 
						                      "LoadLayout. DataManager inconsistency",
											  QMessageBox::Ok, QMessageBox::NoButton);
						if ( collapsed )
						{
							SignalState ss(pos, false, true);
							signalState.push_back(ss);
						}
						else
						{
							for ( int j = 0; j < simpleSignalCount; j++ )
							{
								SignalState ss(pos+j, false, false);
								signalState.push_back(ss);
							}
						}
					}
				}
			}
			else
			{
				// add separator
				SignalState separator(-1);
				signalState.push_back(separator);
			}
			repaint();
		}
		return true;
	}
	else
	{
		QMessageBox::warning(0, "Warning", "Layout file not found", 
							 QMessageBox::Ok, QMessageBox::NoButton);
		return false;
	}
	
}

bool DrawZone::isNumber(const string& numberStr)
{
    unsigned int i = 0;
    const unsigned int Length = numberStr.length();
    for ( ; i < Length; ++i ) {
        if ( '0' > numberStr[i] || numberStr[i] > '9' )
            return false;
    }
    return (i != 0); // to handle the case of [empty string => not a number]
}


bool DrawZone::loadColorMap(const string& path)
{
    std::ifstream f(path.c_str());
    if ( !f )
        return false;

    // Clear previous info
    colorMap.clear();
    colorMap.resize(dm.getSignalSlots(), 0);

    unsigned int lineNumber = 0;

    // parse color map file lines

    map<string,QColor> colorDeclarations;
    string line;
    while ( getline(f,line) ) {
        ++lineNumber;
        stringstream ss(line);
        string token;
        ss >> token;
        if ( token == "declare" ) {
            string colorName;
            ss >> colorName;
            if ( colorName.empty() ) {
                stringstream errmsg;
                errmsg << "Color declaration name not found in '" << line << "' in line " << lineNumber << " (color map file processing stopped!)";
                QMessageBox::warning(this, "Warning", errmsg.str().c_str(), QMessageBox::Ok, QMessageBox::NoButton);
                return false;
            }
            if ( '0' <= colorName.at(0) && colorName.at(0) <= '9' ) {
                stringstream errmsg;
                errmsg << "Color declaration '" << line << "' in line " << lineNumber << " cannot start with a number" 
                       << " (color map file processing stopped!)";
                QMessageBox::warning(this, "Warning", errmsg.str().c_str(), QMessageBox::Ok, QMessageBox::NoButton);
                return false;
            }


            string rStr, gStr, bStr;
            ss >> rStr;
            if ( rStr.empty() ) {
                stringstream errmsg;
                errmsg << "Color declaration is empty, line: '" << line << "' line number: " << lineNumber << " (color map file processing stopped!)";
                QMessageBox::warning(this, "Warning", errmsg.str().c_str(), QMessageBox::Ok, QMessageBox::NoButton);
                return false;
            }

            if ( '0' <= rStr.at(0) && rStr.at(0) <= '9' ) {        
                ss >> gStr >> bStr;
                if ( !isNumber(rStr) || !isNumber(gStr) || !isNumber(bStr) ) {
                    stringstream errmsg;
                    errmsg << "Color declaration '" << line << "' in line " << lineNumber << " is erroneous (color map file processing stopped!)";
                    QMessageBox::warning(this, "Warning", errmsg.str().c_str(), QMessageBox::Ok, QMessageBox::NoButton);
                    return false;
                }
                else {
                    stringstream conversor(rStr + " " + gStr + " " + bStr);
                    unsigned int r, g, b;
                    conversor >> r >> g >> b;
                    colorDeclarations[colorName] = QColor(r,g,b);
                }
            }
            else {
                // try to search for a declaration
                map<string,QColor>::iterator it = colorDeclarations.find(rStr);
                if ( it == colorDeclarations.end() ) {
                    stringstream errmsg;
                    errmsg << "Previous declaration '" << rStr << "' to define the new declaration '" << colorName << "' not found"
                        " (Color map file processing stopped!)";
                    QMessageBox::warning(this, "Warning", errmsg.str().c_str(), QMessageBox::Ok, QMessageBox::NoButton);
                    return false;
                }
                else {
                    colorDeclarations[colorName] = it->second;
                }
            }
        }
        else if ( token == "define" ) {

            CMap* cm = new CMap; // Color map for these signals

            string pattern, aux;
            // parse pattern
            ss >> pattern;
            if ( pattern.empty() ) {
                stringstream errmsg;
                errmsg << "Color definition '" << line << "' in line " << lineNumber
                       << " requires a token defining a pattern with wilcards"
                       << " (color map file processing stopped!)";
                QMessageBox::warning(this, "Warning", errmsg.str().c_str(), QMessageBox::Ok, QMessageBox::NoButton);
                return false;
            }
            ss >> aux;
            if ( !aux.empty() ) {
                stringstream errmsg;
                errmsg << "Color definition '" << line << "' in line " << lineNumber
                       << " requires only a token defining a pattern with wilcards"
                       << " (color map file processing stopped!)";
                QMessageBox::warning(0, "Warning", errmsg.str().c_str(), QMessageBox::Ok, QMessageBox::NoButton);
                return false;
            }
            // pattern parsed

            // parse color definitions for this pattern

            streampos prevPos = f.tellg();
            while ( getline(f,line) ) {
                stringstream ss2(line);
                string enumStr;
                unsigned int enumColor, enumColorSecond;

                // parse single enum or range enum
                ss2 >> enumStr;

                if ( enumStr.empty() || enumStr.at(0) == '#' )
                    continue; // skip line

                if ( enumStr == "define" || enumStr == "declare" ) 
                {
                    f.seekg(prevPos); // restore previous pos
                    break;
                }
                prevPos = f.tellg(); // Save position

                ++lineNumber;

                if ( enumStr.empty() ) {
                    stringstream errmsg;
                    errmsg << "Color enumeration value not found in '" << line << "' in line " << lineNumber
                           << " (color map file processing stopped!)";
                    QMessageBox::warning(this, "Warning", errmsg.str().c_str(), QMessageBox::Ok, QMessageBox::NoButton);
                    return false;
                }
                if ( isNumber(enumStr) ) {
                    stringstream conversor(enumStr);
                    conversor >> enumColor;
                    enumColorSecond = enumColor;
                }
                else {
                    string::size_type index = enumStr.find_first_of("-") ;
                    if ( index == string::npos ) {
                        stringstream errmsg;
                        errmsg << "Color enumeration range hyphen not found '" << line << "' in line " << lineNumber
                               << " (color map file processing stopped!)";
                        QMessageBox::warning(this, "Warning", errmsg.str().c_str(), QMessageBox::Ok, QMessageBox::NoButton);
                        return false;
                    }
                    string lowerStr = enumStr.substr(0, index);
                    string upperStr = enumStr.substr(index + 1, string::npos);
                    if ( !isNumber(lowerStr) || !isNumber(upperStr) ) {
                        stringstream errmsg;
                        errmsg << "Color enumeration range bad defined [" << lowerStr << "," << upperStr << "] at line " << lineNumber
                               << " (color map file processing stopped!)";
                        QMessageBox::warning(this, "Warning", errmsg.str().c_str(), QMessageBox::Ok, QMessageBox::NoButton);
                        return false;
                    }
                    stringstream conversor(lowerStr + " " + upperStr);
                    conversor >> enumColor >> enumColorSecond;
                }

                // parse color definition or color declaration
                string colorDef;
                ss2 >> colorDef;
                if ( colorDef.empty() ) {
                    stringstream errmsg;
                    errmsg << "Color declaration or RGB not found '" << line << "' in line " << lineNumber
                           << " (color map file processing stopped!)";
                    QMessageBox::warning(this, "Warning", errmsg.str().c_str(), QMessageBox::Ok, QMessageBox::NoButton);
                    return false;
                }

                QColor color;
                if ( !isNumber(colorDef) ) { // It is a color declaration
                    map<string,QColor>::iterator it = colorDeclarations.find(colorDef);
                    if ( it == colorDeclarations.end() ) {
                        stringstream errmsg;
                        errmsg << "Color definition '" << colorDef << "' not previously declared in line " << lineNumber
                               << " (color map file processing stopped!)";
                        QMessageBox::warning(this, "Warning", errmsg.str().c_str(), QMessageBox::Ok, QMessageBox::NoButton);
                        return false;
                    }
                    color = it->second;
                }
                else {
                    string rStr, gStr, bStr;
                    rStr = colorDef;
                    ss2 >> gStr >> bStr;
                    if ( !isNumber(gStr) || !isNumber(bStr) ) {
                        stringstream errmsg;
                        errmsg << "Color RGB definition '" << colorDef << "' bad defined" << lineNumber
                               << " (color map file processing stopped!)";
                        QMessageBox::warning(this, "Warning", errmsg.str().c_str(), QMessageBox::Ok, QMessageBox::NoButton);
                        return false;
                    }
                    stringstream conversor(rStr + " " + gStr + " " + bStr);
                    unsigned int r, g, b;
                    conversor >> r >> g >> b;
                    color = QColor(r, g, b);
                }

                string info;
                ss2 >> info;

				CInfo ci;
				ci.color = color;
				ci.info = info;	

				for ( unsigned int i = enumColor; i <= enumColorSecond; i++ ) {
					(*cm)[i] = ci;
				}

            } // end while
            // end of enum/color definitions
			QRegExp regexp(pattern.c_str());
			regexp.setWildcard(true);

			vector<SignalState>::iterator it = signalState.begin();
			while ( it != signalState.end() )
			{
			    if ( it->map != -1 )
				{
				    string name(dm.getSignalSlotName(it->map)); 
					int pos = name.find('.', 0);
					name = name.substr(0, pos);
        					
					if ( regexp.exactMatch(name.c_str()) )
                        colorMap[it->map] = cm; // Matching
				}
			    it++;
            }
        }
        else {
            if ( line.empty() || line.at(0) == '#' )
                continue; // ignore line
            stringstream errmsg;
            errmsg << "Unexpected colormap file line: '" << line << "' line number " << lineNumber
                   << " (color map file processing stopped!)";
            QMessageBox::warning(this, "Warning", errmsg.str().c_str(), QMessageBox::Ok, QMessageBox::NoButton);
            return false;
        }
    }
    repaint();
    return true;
}

bool DrawZone::loadColorMapOld(const string& path)
{
	std::ifstream f(path.c_str());    
	if ( f )
	{
		colorMap.clear(); // Requires deleting all previous contents
		colorMap.resize(dm.getSignalSlots(), 0);		
		
		string signamePattern;
		int nColors;
		
		while ( !f.eof() )
		{
			signamePattern = ""; // reset
			f >> signamePattern >> nColors;

			if ( signamePattern == "" )
			{
				repaint();
				return true; // no more signals
			}

			if ( signamePattern == "DEPENDENCY_COLOR" )
			{
				// A highlight color is specified
				int r = nColors;
				int g = 0;
				int b = 0;
				f >> g >> b;
				setHiglightColor(QColor(r,g,b));
				continue;
			}


			CMap* cm = new CMap; // Color map for these signals
			
			while ( nColors-- > 0 )
			{
				int enumColor;
				int enumColorSecond;
				int r, g, b;
				string token;
				f >> enumColor; // Parse enumColor
				enumColorSecond = enumColor; // by default (no range)
				f >> token;
				if ( '0' <= token.at(0) && token.at(0) <= '9' )
				{
					// Specifyng a RGB value
					r = atoi(token.c_str());
					f >> g >> b;
				}
				else 
				{
					// Specifying a range or a token color
					if ( token.at(0) == '-' )
					{
						// Range specification
						enumColorSecond = -atoi(token.c_str());
						f >> token; // read new value
					} 
					if ( '0' <= token.at(0) && token.at(0) <= '9')
					{
						// Specifying a RGB value
						r = atoi(token.c_str());
						f >> g >> b;
					}
					else
					{
						// Assume token color
						if ( token == "BLACK" ) { r = 0; g = 0; b = 0; }
						else if ( token == "WHITE" ) { r = 255; g = 255; b = 255; }
						else if ( token == "RED" ) { r = 255; g = 0; b = 0; }
						else if ( token == "RED_0" ) { r = 255; g = 0; b = 0; }
						else if ( token == "RED_1" ) { r = 213; g = 0; b = 0; }
						else if ( token == "RED_2" ) { r = 171; g = 0; b = 0; }
						else if ( token == "RED_3" ) { r = 129; g = 0; b = 0; }
						else if ( token == "RED_4" ) { r = 87; g = 0; b = 0; }
						else if ( token == "RED_5" ) { r = 45; g = 0; b = 0; }
						else if ( token == "GREEN" ) { r = 0; g = 255; b = 0; }
						else if ( token == "GREEN_0" ) { r = 192; g = 255; b = 192; }
						else if ( token == "GREEN_1" ) { r = 128; g = 255; b = 128; }
						else if ( token == "GREEN_2" ) { r = 64; g = 255; b = 64; }
						else if ( token == "GREEN_3" ) { r = 0; g = 255; b = 0; }
						else if ( token == "GREEN_4" ) { r = 0; g = 200; b = 0; }
						else if ( token == "GREEN_5" ) { r = 0; g = 160; b = 0; }
						else if ( token == "BLUE" ) { r = 0; g = 0; b = 255; }
						else if ( token == "BLUE_0" ) { r = 0; g = 0; b = 255; }
						else if ( token == "BLUE_1" ) { r = 0; g = 0; b = 213; }
						else if ( token == "BLUE_2" ) { r = 0; g = 0; b = 171; }
						else if ( token == "BLUE_3" ) { r = 0; g = 0; b = 129; }
						else if ( token == "BLUE_4" ) { r = 0; g = 0; b = 87; }
						else if ( token == "BLUE_5" ) { r = 0; g = 0; b = 45; }
						else { r = 0; g = 0; b = 0; } // unknown token
					}
				}
				char buf[256];
				f.getline(buf, sizeof(buf)); // info
				
				CInfo ci;
				ci.color = QColor(r,g,b);
				ci.info = buf;	

				for ( int i = enumColor; i <= enumColorSecond; i++ )
				{
					(*cm)[i] = ci;
				}
			}
			
			QRegExp regexp(signamePattern.c_str());
			regexp.setWildcard(true);

			vector<SignalState>::iterator it = signalState.begin();
			while ( it != signalState.end() )
			{
				if ( it->map != -1 )
				{
					string name(dm.getSignalSlotName(it->map)); 
					int pos = name.find('.', 0);
					name = name.substr(0, pos);
					
					if ( regexp.exactMatch(name.c_str()) )
					//if ( name == signame )
						colorMap[it->map] = cm; // Matching
				}
				it++;
			}
		}
		repaint();
		return true;
	}
	else
	{
		QMessageBox::warning(0, "Warning", "ColorMap file not found", 
							 QMessageBox::Ok, QMessageBox::NoButton);
		return false;
	}
		
}

void DrawZone::showCurrentColorMap()
{
	string info = "";

	//stringstream ss;
	vector<CMap*>::iterator it = colorMap.begin();
	int i = 0;
	for ( ; it != colorMap.end(); it++, i++)
	{
		if ( *it != 0 ) // SignalSlotData with color map information
		{
			char temp[256];
			CMap* cm = *it;
			info = info + dm.getSignalSlotName(i) + "  ";// + cm->size() + "\n";
			sprintf(temp, "%d\n", cm->size());
			info += temp;
			map<int,CInfo>::iterator it2 = cm->begin();
			for ( ; it2 != cm->end(); it2++ )
			{
				QColor col = it2->second.color;
				string str = it2->second.info;
				sprintf(temp, " %d %d %d %d  '%s'\n", it2->first, col.red(), 
						col.green(), col.blue(), str.c_str());
				
				info += temp;
			}
		}
	}
    QMessageBox::information(0, "Info", info.c_str(), QMessageBox::Ok, QMessageBox::NoButton);

	// MessageBoxA(NULL, info.c_str(), "Info", MB_OK);
}


void DrawZone::showCycleMessage(int cycle)
{
	char msg[256];

	sprintf(msg, "Going to cycle: %d", cycle/S_SIZE + firstCycle);
	app->statusBar()->message(msg);
}

vector<int> DrawZone::findSiblings(int simpleSignal)
{
	vector<int> matches;
	if ( IS_SIGNAL_SEPARATOR(simpleSignal) )
	{
		matches.push_back(simpleSignal);
		return matches;
	}

	string signalName(GET_SIGNAL_NAME(dm, simpleSignal));
	for ( int i = 0; i < (int)signalState.size(); i++ )
	{
		if ( !IS_SIGNAL_SEPARATOR(i) )
		{
			string signalName2(GET_SIGNAL_NAME(dm, i));
			if ( signalName == signalName2 )
				matches.push_back(i);
		}
	}
	return matches;
}

bool DrawZone::isSignalVisible(const string& signalName)
{
	for ( int i = 0; i < (int)signalState.size(); i++ )
	{
		if ( !IS_SIGNAL_SEPARATOR(i) )
		{
			if ( GET_SIGNAL_NAME(dm,i) == signalName )
				return true;
		}
	}
	return false;
}

bool DrawZone::setSignalVisible(const char* signalName, bool visible)
{
	vector<SignalState>::iterator it = signalState.begin();
	int i = 0;
	for ( ; it != signalState.end(); it++, i++ )
	{
		if ( it->map != -1 )
		{
			if ( GET_SIGNAL_NAME(dm,i) == signalName ) // signal is currently visible
			{
				if ( !visible )
				{
					signalState.erase(it);
					return true;
				}
				return false;
			}
		}
	}
	// signal not found in the current view
	int pos = dm.getSignalSlotPosition(signalName);
	if ( pos >= 0 )
	{
		int count = dm.countSignalSlots(signalName);
		for ( int i = 0; i < count; i++ )
		{
			SignalState ss(pos+i);
			signalState.push_back(ss);
		}
		return true;
	}
	return false;
}
