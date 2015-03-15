#include "CyclePageCache.h"
#include <cstdio>
#include <windows.h>

#define MSG(title,msg) MessageBoxA( NULL, msg, title, MB_OK );

using namespace std;

CyclePageCache::CyclePageCache( int nPages ) : nPages(nPages)
{
	cpArray = new CyclePage[nPages];
/*	
	if ( nPages > 0 )
		cpArray.resize(nPages, CyclePage());
	else
		this->nPages = -1; // unlimited space
*/

}

CyclePage* CyclePageCache::getPage( int idPage )
{
	map<int,int>::iterator it = cacheMap.find(idPage);
	if ( it != cacheMap.end() )
		return & cpArray[it->second];
	return 0;	
}


CyclePage* CyclePageCache::getFreePage( int newIdPage )
{	
/*
	if ( nPages <= 0 ) // infinite page resources 
	{		
		// Pages are never released 
		cpArray.push_back(CyclePage());
		CyclePage& cp = cpArray.back();
		cp.clearPageData();
		cp.setPageNumber(newIdPage);
		cp.touch();
		cacheMap.insert(make_pair(newIdPage, cpArray.size()-1));
		return &cp;
	}
*/

	/* Limited page resources. Policy: LRU */
	
	/* find a free page */
	int iLast = 0;
	int i;
	for ( i = 0; i < nPages/*cpArray.size()*/; i++ ) {
		if ( cpArray[i].getPageNumber() == -1 ) 
		{
			cacheMap.insert(make_pair(newIdPage, i));
			cpArray[i].setPageNumber( newIdPage ); // mark as not free
			cpArray[i].touch();
			return &(cpArray[i]);
		}			
		if ( cpArray[iLast].getTimeStamping() > cpArray[i].getTimeStamping() )
			iLast = i;
	}

	// No free pages, remove LRU page
	// we have to erase the current contents	
	cacheMap.erase(cpArray[iLast].getPageNumber()); // the page is not present
	cpArray[iLast].clearPageData();
	cpArray[iLast].setPageNumber( newIdPage );
	cpArray[iLast].touch(); // mark as new page ( the new one )	
	cacheMap.insert(make_pair(newIdPage, iLast)); // update map
	return &(cpArray[iLast]);
}

void CyclePageCache::clear()
{
	//cpArray.clear();
	delete[] cpArray;
	cpArray = new CyclePage[nPages];
	/*
	if ( nPages > 0 )
		cpArray.resize(nPages, CyclePage());
	*/
	cacheMap.clear();
}


CyclePageCache::~CyclePageCache()
{
	delete[] cpArray;
}

