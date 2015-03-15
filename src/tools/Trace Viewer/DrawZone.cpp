#include "DrawZone.h"
#include <windows.h>
#include <cstdio>
#include <QDialog.h>
#include <QPushButton.h>

#include "CycleDialog.h"
#include <qlistview.h>

#define BLACK 0
#define WHITE 1
#define RED 2
#define GREEN 3
#define BLUE 4
#define UNDEF_COLOR 23
#define HIGHLIGHTED_COLOR UNDEF_COLOR

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

#define MSG(title,msg) MessageBox( NULL, msg, title, MB_OK );

#ifndef NULL
	#define NULL 0
#endif



DrawZone::DrawZone( QWidget* parent, DataManager& dm, int sizeItems  ) : 
	QWidget(parent), dm(dm), offsetV(0), offsetH(0), S_SIZE(sizeItems),
	linesEnabled(true), defaultOriginX(200), defaultOriginY(30),
	backward(true), forward(false)
{	
	ORIGIN_X = defaultOriginX;
	ORIGIN_Y = defaultOriginY;
	this -> itemsH = dm.getCycles();
	this -> itemsV = dm.getSimpleSignals();
}


void DrawZone::zoomIn( int incPixels )
{
	if ( S_SIZE >= 100 )
		return ;
	S_SIZE = S_SIZE + incPixels;
	if ( S_SIZE > 100 )
		S_SIZE = 100;
	if ( S_SIZE < 2 )
		S_SIZE = 2;
	repaint();
	// emit signal
	emit realPixelsChanged( S_SIZE*itemsH, S_SIZE*itemsV );
}

void DrawZone::zoomIn()
{
	zoomIn( 1 );
}

void DrawZone::zoomOut( int decPixels )
{
	if ( S_SIZE <= 2 )
		return;
	S_SIZE = S_SIZE - decPixels;
	if ( S_SIZE < 2 )
		S_SIZE = 2;
	if ( S_SIZE > 100 )
		S_SIZE = 100;
	repaint();
	// emit signal
	emit realPixelsChanged( S_SIZE*itemsH, S_SIZE*itemsV );
}

void DrawZone::zoomOut()
{
	zoomOut( 1 );
}

void DrawZone::setNewItemSize( int newSize )
{
	S_SIZE = newSize;
	repaint();
	// emit signal
	emit realPixelsChanged( S_SIZE*itemsH, S_SIZE*itemsV );
	
}


void DrawZone::getRealPixelsSize( int& hPixels, int& vPixels)
{
	hPixels = itemsH*S_SIZE;
	vPixels = itemsV*S_SIZE;
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
	p.drawText( x, y, w, h, AlignHCenter, text );
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
				p.drawText( ORIGIN_X, 0, S_SIZE - ( offsetH - x ), ORIGIN_Y, AlignHCenter, buffer );
			}
		}
		else { // fully viewed
			sprintf( buffer, "%d", i );
			p.drawRect( x + ORIGIN_X - offsetH, 0, S_SIZE, ORIGIN_Y );
			p.drawText( x + ORIGIN_X - offsetH, 0, S_SIZE, ORIGIN_Y, AlignHCenter, buffer );
		}
	}
}

void DrawZone::drawSignalsRuler( QPainter& p )
{
	p.setPen( RED );
	//char buffer[256];
	int x;
	
	int i_first, i_last, dummy;
	pixelToCoord( 0, ORIGIN_Y, dummy, i_first );
	pixelToCoord( 0, ORIGIN_Y + height(), dummy, i_last );
	
	for ( int i = i_first; i <= i_last && i < itemsV; i++ ) {
		x = i*S_SIZE;		
		if ( x < offsetV ) {
			if ( offsetV - x < S_SIZE ) {
				p.drawRect( 0, ORIGIN_Y, ORIGIN_X, S_SIZE - ( offsetV - x ) );
				p.drawText( 0, ORIGIN_Y, ORIGIN_X, S_SIZE - ( offsetV - x ), 
					AlignLeft, dm.getSimpleSignalName( i ) );
			}
		}
		else {
			p.drawRect( 0, x + ORIGIN_Y - offsetV, ORIGIN_X, S_SIZE );
			p.drawText( 0, x + ORIGIN_Y - offsetV, ORIGIN_X, S_SIZE, 
				AlignLeft, dm.getSimpleSignalName( i ) );
			
		}
	}
}


// drawItem an item
void DrawZone::drawItem( QPainter& p, int x, int y, QColor& color, bool solid )
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


bool DrawZone::mustBeHighlighted( const SimpleSignalInfo* ssi )
{
	if ( cookies == NULL )
		return false;
	int nC, i;
	const int* cooks = ssi->getCookies( nC );
	if ( backward && forward ) {
		if ( nC <= nCookies ) { // check backward
			for ( i = 0; i < nC; i++ ) {
				if ( cookies[i] != cooks[i] )
					return false;
			}
			return true;
		}
		else {
			for ( i = 0; i < nCookies; i++ ) {
				if ( cookies[i] != cooks[i] )
					return false;
			}
			return true;
		}
	}
	else if ( backward ) {
		for ( i = 0; i < nC; i++ ) {
			if ( cookies[i] != cooks[i] )
				return false;
		}
		return true;
	}
	else if ( forward ) {
		for ( i = 0; i < nCookies; i++ ) {
			if ( cookies[i] != cooks[i] ) 
				return false;
		}
		return true;
	}
	else
		return false;
}

/*
// only backwards implemented for the time being
bool DrawZone::mustBeHighlighted( const SimpleSignalInfo* ssi )
{
	if ( cookies == NULL )
		return false;
	int nC, i;
	const int* cooks = ssi->getCookies( nC );	
	if ( backward && nC <= nCookies ) { // check backward
		for ( i = 0; i < nC; i++ ) {
			if ( cooks[i] != cookies[i] ) { // no matching for backward
				if ( forward && nC >= nCookies ) { // check forward
					for ( i = 0; i < nCookies; i++ ) {
						if ( cooks[i] != cookies[i] )
							return false;
					}		
					return true; // no backward matching but forward matching
				}
				// else ( not interested in forward
				return false; // no matching backward and forward
			}
		}
		return true; // backward matching
	}
	else { // not interested in backward, only check forward
		if ( forward && nC >= nCookies ) { // check forward
			for ( i = 0; i < nCookies; i++ ) {
				if ( cooks[i] != cookies[i] )
					return false;
			}		
			return true; // forward matching
		}
	}
	return false;
}
*/

void DrawZone::paintEvent( QPaintEvent* event )
{	
	QPixmap pm( size() );	
	QPainter p;
	pm.fill( backgroundColor() );
	p.begin( &pm, this );
	//QPainter p(this);
	
	// Draw cycles numbers...
	if ( ORIGIN_Y != 0 )
		drawCyclesRuler( p );
	if ( ORIGIN_X != 0 )		
		drawSignalsRuler( p );

	int i_first, i_last;
	int j_first, j_last;
	
	pixelToCoord( ORIGIN_X, ORIGIN_Y, j_first, i_first );
	pixelToCoord( ORIGIN_X + width(), ORIGIN_Y + height(), j_last, i_last );
	
	const SimpleSignalInfo* ssi;
	//const SimpleSignalInfo*** ssArray = dm.getArrayData();
	for ( int i = i_first; i <= i_last && i < itemsV; i++ ) {
		for ( int j = j_first; j <= j_last && j < itemsH; j++ ) {			
			if ( ( ssi = dm.getData(i,j) ) != NULL ) {
				// check if it should be highlighted
				if ( mustBeHighlighted( ssi ) ) {					
					drawItem( p, j, i, COLOR_TABLE[HIGHLIGHTED_COLOR], true );
				}
				else {
					int color = ssi->getColor() % UNDEF_COLOR;						
					drawItem( p, j, i, COLOR_TABLE[color], true );
				}
			}
			else 
				drawItem( p, j, i, COLOR_TABLE[BLACK] );			
		}
	}
	//p.end();
	bitBlt( this, 0, 0, &pm );

}


/*
void DrawZone::mouseDoubleClickEvent( QMouseEvent* mEvent )
{
	//MSG( "info", "double click" );
}
*/


void DrawZone::mousePressEvent ( QMouseEvent* mEvent ) 
{
	int x = mEvent->x();
	int y = mEvent->y();

	if ( y < ORIGIN_Y || x < ORIGIN_Y )
		return ;
	pixelToCoord( x, y, y, x ); // Convert pixels coordinates in OI coordinates
	if ( x >= itemsV || y >= itemsH ) // control out of range
		return;
	const SimpleSignalInfo* ssi;

	if ( mEvent->button() == ButtonState::RightButton ) {
		if ( ( ssi = dm.getData(x,y) ) != NULL ) {						
			int nCks;
			const int* cks = ssi->getCookies( nCks );
			//char buf_[256];
			//sprintf( buf_, "nCks: %d    nCookies: %d", nCks, nCookies );
			//MSG ("info", buf_ );
			if ( nCks != nCookies ) {
				unhighlightCycles();
				highlightCycles( cks, nCks, backward, forward );
			}
			else { // same number of cookies				
				int i;
				for ( i = 0; i < nCks; i++ ) {
					if ( cookies[i] != cks[i] ) {
						unhighlightCycles();
						highlightCycles( cks, nCks, backward, forward );						
						break;
					}	
				}
				if ( i == nCks ) 
					unhighlightCycles();
			}
			repaint();
		}
	}
		
	if ( mEvent->button() == ButtonState::LeftButton ) {		
		if ( ( ssi = dm.getData(x,y) ) != NULL ) {
			CycleDialog* cd = new CycleDialog( this, "cdlg", true, 0 );
			//cd->ListView1->
			cd->ListView1->setSorting( -1 );
			QListViewItem* item = cd->ListView1->firstChild();
			
			char buffer[256];
			
			sprintf( buffer, "%d", y );
			item->setText( 1, tr(buffer) ); // cycle 

			item = item->nextSibling(); 
			item->setText( 1, tr(dm.getSimpleSignalName(x)) ); // signal name
							
			int nCookies;
			const int* cookies = ssi->getCookies( nCookies );
			const char* info = ssi->getInfo();
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
			item->setText( 1, tr(buffer) ); // cookies
							
			sprintf( buffer, "%d", ssi->getColor() );
			item = item->nextSibling();
			item->setText( 1, tr( buffer ) ); // color
			
			item = item->nextSibling();
			if ( info )
				item->setText( 1, tr( info ) );
			else
				item->setText( 1, tr( "NO info" ) );
			
			cd->exec(); // it doesn't care if accept or cancel
			delete cd;
		}
	}
}

int DrawZone::displaceH( int pixels )
{	
	offsetH = pixels;
	repaint();
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


void DrawZone::highlightCycles( const int* cookies, int nCookies, bool backward, bool forward )
{
	this->nCookies = nCookies;
	this->backward = backward;
	this->forward = forward;
	this->cookies = new int[nCookies];
	for ( int i = 0; i < nCookies; i++ )
		this->cookies[i] = cookies[i];
}

void DrawZone::setHighlightMode( bool backward, bool forward )
{
	this->backward = backward;
	this->forward = forward;
	repaint();
}

void DrawZone::unhighlightCycles()
{
	if ( cookies == NULL )
		return ;	
	delete[] cookies;
	cookies = NULL;
	nCookies = 0;
	//backward = false;
	//forward = false;	
}

