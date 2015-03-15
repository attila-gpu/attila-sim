#include "CyclePage.h"

#include <iostream>

#ifndef NULL
	#define NULL 0
#endif

#include <windows.h>
#include <cstdio>
#define MSG(title,msg) MessageBoxA( NULL, msg, title, MB_OK );

using namespace std;

const int CyclePage::PAGE_SIZE = 1024;
int CyclePage::rows = 0;
unsigned long CyclePage::globalTimeStamping = 0;


CyclePage::CyclePage( int pageNumber ) : pageNumber(pageNumber), myTimeStamping(0)
{
	ssiArray = new SignalSlotData**[rows];
	for ( int i = 0; i < rows; i++ ) {
		ssiArray[i] = new SignalSlotData*[PAGE_SIZE];
		for ( int j = 0; j < PAGE_SIZE; j++ )
			ssiArray[i][j]= NULL;
	}	
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
				//int nCookies;
				//const int* cookies = ssiArray[i][j]->getCookies( nCookies );
                const vector<int>& cookies = ssiArray[i][j]->getCookies();
                int nCookies = cookies.size();
				for ( int k = 0; k < nCookies; k++ ) {
					cout << cookies[k];
					if ( k != nCookies - 1 ) cout << ":";
				}
				cout << "  Color: " << ssiArray[i][j]->getColor();
				const string& info = ssiArray[i][j]->getInfo();
                if ( !info.empty() ) 					
					cout << "  Info: " << info << endl;
				else
					cout << endl;
				//cout << endl;				
			}
		}
		cout << "-----------------------------------------" << endl;
	}	
}

