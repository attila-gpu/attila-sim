#include "SignalContent.h"

#include <cstring>
#include <cstdlib>
#include <fstream.h>

SignalContent::SignalSlot::SignalSlot() : cookieList(0), info(0)
{
}

SignalContent::SignalSlot::~SignalSlot()
{
	delete[] cookieList;
	delete[] info;
}


SignalContent::SignalContent( int bw ) : nCookies(0), color(0), index(0), nSlots(bw)
{
	ss = new SignalSlot[bw];
}

SignalContent::~SignalContent()
{
	delete[] ss;
}


void SignalContent::clear()
{
	for ( int i = 0; i < index; i++ ) {
		delete[] ss[i].cookieList;
		delete[] ss[i].info;
		ss[i].cookieList = 0;
		ss[i].info = 0;
	}
	index = 0;
}


int SignalContent::countSignalContents() const
{
	return index;
}

bool SignalContent::addSignalContents( const char* sigContents )
{
	if ( index == nSlots )
		return false;
	int temp[100];
	char buffer[256];
	int itemp = 0;
	
	// parse cookie list
	int i, iPrev = 0;
	for ( i = 0; sigContents[i] != ';' ; i++ ) {		
		if ( sigContents[i] == ':' ) {
			strncpy( buffer, &sigContents[iPrev], i - iPrev );
			buffer[i-iPrev] = 0;			
			iPrev = i + 1;
			temp[itemp++] = atoi( buffer );
		}
	}
	// process last cookie
	strncpy( buffer, &sigContents[iPrev], i - iPrev );
	buffer[i-iPrev] = 0;
	temp[itemp++] = atoi( buffer );
	i++;
	
	// i is pointing to color
	iPrev = i;
	for ( ; sigContents[i] != ';' && sigContents[i] != 0; i++ ) ;
	strncpy( buffer, &sigContents[iPrev], i - iPrev );
	buffer[i-iPrev] = 0;
	color = atoi( buffer );
	// extract info if available
	if ( sigContents[i] == ';' )
		i++;
	iPrev = i;
	for ( ; sigContents[i] != (char)0; i++ ) ;
	
	if ( iPrev != i ) { // Contents available
		ss[index].info = new char[i - iPrev + 1];		
		strcpy( ss[index].info, &sigContents[iPrev] );
	}
	else
		ss[index].info = NULL;
	// Copy cookie list
	nCookies = itemp;
	ss[index].cookieList = new int[itemp];
	for ( i = 0; i < itemp; i++ )
		ss[index].cookieList[i] = temp[i];

	index ++; // slot occupied

	return true;
}

int SignalContent::getColor() const
{
	return color;
}


const int* SignalContent::getCookieList( int slot, int& nCookies ) const
{
	nCookies = this->nCookies;
	return ss[slot].cookieList;
}


const char* SignalContent::getInfo( int slot ) const
{
	return ss[slot].info;
}


void SignalContent::dump() const
{	
	cout << "Signal color: " << color << endl;
	cout << "Cookies per slot: " << nCookies << endl;
	cout << "Signal bw (slots): " << nSlots << endl;
	int i, j;
	for ( i = 0; i < index; i++ ) { // for all slots
		cout << "slot(" << i << ") info: " << endl;
		cout << " cookies: ";
		for ( j = 0; j < nCookies; j++ ) {
			cout << ss[i].cookieList[j];
			if ( j != nCookies - 1 )
				cout << ":";
		}
		if ( ss[i].info )
			cout << "  info: \"" << ss[i].info << "\"";
		cout << endl;

	}
}
