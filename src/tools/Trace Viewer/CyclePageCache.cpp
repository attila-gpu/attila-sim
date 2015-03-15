#include "CyclePageCache.h"
#include <cstdio>
#include <windows.h>
#define MSG(title,msg) MessageBox( NULL, msg, title, MB_OK );

CyclePageCache::CyclePageCache( int nPages ) : nPages(nPages)
{
	cpArray = new CyclePage[nPages];
}

CyclePage* CyclePageCache::getPage( int idPage )
{
	for ( int i = 0; i < nPages; i++ ) {
		if ( cpArray[i].getPageNumber() == idPage )
			return &cpArray[i];
	}
	return 0;
}


CyclePage* CyclePageCache::getFreePage( int newIdPage )
{	
	int iLast = 0;
	int i;
	for ( i = 0; i < nPages; i++ ) {		
		if ( cpArray[i].getPageNumber() == -1 ) {
			cpArray[i].setPageNumber( newIdPage ); // mark as not free
			return &cpArray[i];
		}			
		if ( cpArray[iLast].getTimeStamping() > cpArray[i].getTimeStamping() )
			iLast = i;
	}
	/*
	char buf[2048];
	char aux[256];
	sprintf( buf, "Time stampings:\n" );
	for ( i = 0; i < nPages; i++ ) {
		sprintf( aux, " - page %d: %lu\n", cpArray[i].getPageNumber(), cpArray[i].getTimeStamping() );
		strcat( buf, aux );
	}
	sprintf( aux, "Page that is gonna to be erased: %d", cpArray[iLast].getPageNumber() );
	strcat( buf, aux );
	MSG( "replace policy", buf );
	*/
	// we have to erase the current contents
	cpArray[iLast].clearPageData();
	cpArray[iLast].setPageNumber( newIdPage );
	cpArray[iLast].touch(); // mark as new page ( the new one )
	return &cpArray[iLast];
}


bool CyclePageCache::isFull()
{
	for ( int i = 0; i < nPages; i++ ) {
		if ( cpArray[i].getPageNumber() == -1 )
			return false;
	}
	return true;
}

int CyclePageCache::getMaxPages()
{
	return nPages;
}

void CyclePageCache::clear()
{
	delete[] cpArray;
	cpArray = new CyclePage[nPages];
}


CyclePageCache::~CyclePageCache()
{
	delete[] cpArray;
}
