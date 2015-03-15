#include "CyclePage.h"
#include <ostream.h>

#ifndef NULL
	#define NULL 0
#endif

#include <windows.h>
#include <cstdio>
#define MSG(title,msg) MessageBox( NULL, msg, title, MB_OK );

const int CyclePage::PAGE_SIZE = 1024;
int CyclePage::rows = 0;
unsigned long CyclePage::globalTimeStamping = 0;

void CyclePage::setRows( int _rows )
{
	rows = _rows;
}

int CyclePage::getRows()
{
	return rows;
}

void CyclePage::setPageNumber( int idPage )
{
	pageNumber = idPage;
}

int CyclePage::getPageNumber()
{
	return pageNumber;
}

CyclePage::CyclePage( int pageNumber ) : pageNumber(pageNumber), myTimeStamping(0)
{
	ssiArray = new SimpleSignalInfo**[rows];
	for ( int i = 0; i < rows; i++ ) {
		ssiArray[i] = new SimpleSignalInfo*[PAGE_SIZE];
		for ( int j = 0; j < PAGE_SIZE; j++ )
			ssiArray[i][j]= NULL;
	}	
}

int CyclePage::getPageCapacity()
{
	return PAGE_SIZE;
}


SimpleSignalInfo* CyclePage::getData( int absoluteX, int relativeY )
{
	myTimeStamping = globalTimeStamping++;
	return ssiArray[absoluteX][relativeY];
}


void CyclePage::setData( int absoluteX, int relativeY, SimpleSignalInfo* ssi )
{	
	ssiArray[absoluteX][relativeY] = ssi;
}


CyclePage::~CyclePage()
{
	for ( int i = 0; i < rows; i++ ) {
		for ( int j = 0; j < PAGE_SIZE; j++ )
			delete ssiArray[i][j];
		delete[] ssiArray[i];
	}
	delete[] ssiArray;
}


void CyclePage::clearPageData()
{
	for ( int i = 0; i < rows; i++ ) {
		for ( int j = 0; j < PAGE_SIZE; j++ ) {
			delete ssiArray[i][j]; // delete null pointers is not bad
			ssiArray[i][j] = NULL;
		}		
	}	
}


int CyclePage::getTimeStamping()
{
	return myTimeStamping;
}


void CyclePage::touch()
{
	myTimeStamping = globalTimeStamping++;
}



void CyclePage::dump()
{
	
	int i, j;
	cout << "----------- Page " << pageNumber << " -----------------------" << endl;
	for ( j = 0; j < PAGE_SIZE; j++ ) {
		cout << "Cycle " << ( j + PAGE_SIZE*pageNumber ) << " infomation:" << endl;
		for ( i = 0; i < rows; i++ ) {			
			if ( ssiArray[i][j] != NULL ) {
				cout << "RowID ( signalID ): " << i ;
				cout << "  Cookies: ";
				int nCookies;
				const int* cookies = ssiArray[i][j]->getCookies( nCookies );
				for ( int k = 0; k < nCookies; k++ ) {
					cout << cookies[k];
					if ( k != nCookies - 1 ) cout << ":";
				}
				cout << "  Color: " << ssiArray[i][j]->getColor();
				const char* info = ssiArray[i][j]->getInfo();
				if ( info ) 					
					cout << "  Info: " << info << endl;
				else
					cout << endl;
				//cout << endl;				
			}
		}
		cout << "-----------------------------------------" << endl;
	}	
}

