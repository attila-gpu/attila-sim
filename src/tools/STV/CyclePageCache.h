#ifndef CYCLEPAGECACHE_H
	#define CYCLEPAGECACHE_H

#include "CyclePage.h"
#include <map>
#include <vector>


/**
 * CyclePageCache class implements a read-only cache for catching data from a tracefile
 *
 * It avoids load all tracefile, benefits are speed loading and memory resources
 *
 * @note The replace policy is LRU
 * @see CyclePage
 *
 * @version 1.0
 * @date 25/07/2003
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es
 */
class CyclePageCache 
{
private:
	
	//std::vector<CyclePage> cpArray; ///< Array of CyclePage objects ( the cache pages )
	CyclePage* cpArray;
	
	std::map<int,int> cacheMap; ///< Maps fron id -> position in cpArray

	int nPages; ///< Number of pages for this cache

public:
	
	/** 
	 * Creates a cache for nPages CyclePages
	 *
	 * @param nPages number of pages for this cache
	 *        0 >= means infinite
	 */
	CyclePageCache( int nPages );

	/**
	 * Obtains a CyclePage given its identifier
	 *
	 * @param idPage Number page
	 * @return  if hit -> returns the page, otherwise ( miss ) -> return null pointer
	 */
	CyclePage* getPage( int idPage );

	/**
	 * Returns the least recently used page ( or a free one if not all pages are occupied )
	 *
 	 * The page is marked as new and reserved
	 *
	 * @note Always returns a free page ( if there isn't free pages it selects one LRU policy
	 * and remove previous contents )
	 *
	 * @param newIdPage Number page ( the new id for the new association )
	 * @return a cleared CyclePage with pageNumber equals to newIdPage
	 */
	CyclePage* getFreePage( int newIdPage ); 
	
	/**
	 * Check if cache is full
	 *
	 * @return true if cache is full, false otherwise
	 */
	inline bool isFull();

	/**
	 * Clears cache data ( clear all pages )
	 */
	void clear();

	/**
	 * returns the size in pages of the cache
	 *
	 * @return maximum number of pages for this cache
	 */
	inline int getMaxPages();

	/**
	 * Destructor
	 *
	 * Destroy cache allocations
	 */
	~CyclePageCache();

};

///////////////////////////////////
/// Inline methods' definitions ///
///////////////////////////////////

inline bool CyclePageCache::isFull()
{
	return (nPages == cacheMap.size());	
}

inline int CyclePageCache::getMaxPages()
{
	return nPages;
}

#endif // CYCLEPAGECACHE_H
