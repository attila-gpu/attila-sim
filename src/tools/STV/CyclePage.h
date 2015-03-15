#ifndef CYCLEPAGE_H
	#define CYCLEPAGE_H

#include "SignalSlotData.h"

/**
 * CyclePage class implements the concept of a cache page in a tracefile context
 *
 * A CyclePage is defined as an bi-dimensional array of SignalSlotData objects
 * It contains information of a fixed number of cycles, the real size is dependant
 * of information in the concrete cycles
 *
 * @see CyclePageCache
 *
 * @version 2.0
 * @date 24/10/2008
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es
 */
class CyclePage
{

private:
	
	static const int PAGE_SIZE; ///< number of cycles contained in any CyclePage 
	static int rows; ///< Number of rows in any CyclePage

	/**
	 * Time stamping counter to implement replacement policies, it is updated ( increased ) every time
	 * CyclePage::getData() or CyclePage::touch() methods are invoked
	 */
	static unsigned long globalTimeStamping;

	SignalSlotData*** ssiArray; ///< CyclePage data
	int pageNumber; ///< Page number
	unsigned long myTimeStamping; ///< last access to this page ( it is updated using globalTimeStamping )

public:

	/**
	 * Creates a blank page with an specified id
	 *
	 * If id is lower than 0 then the page has no association with any portion of the tracefile
	 *
	 * @param pageNumber page number in the tracefile
	 */
	CyclePage( int pageNumber = -1 );
	
	/**
	 * set the rows for any CyclePage 
	 *
	 * @warning  it should not be called if some page has been created, it should be called once only once
	 *
	 * @param rows rows in any CyclePage created after call this method
	 */
	inline static void setRows( int rows );

	/**
	 * obtains the rows in any CyclePage created after last CyclePage::setRows() call
	 *
	 * @return rows in any CyclePage created after last CyclePage::setRows() call
	 */
	inline static int getRows();

	/**
	 * Associates tis page with a portion of the tracefile
	 *
	 * @param idPage page number
	 */
	inline void setPageNumber( int idPage );
	
	/**
	 * Obtains the page number
	 *
	 * @return page number
	 */
	inline int getPageNumber();
	
	/**
	 * Obtains the capacity ( number of cycles any CyclePage is capable of maintain cached )
	 *
	 * @return max capacity
	 */
	inline static int getPageCapacity();

	/**
	 * Obtains the time stamping for this page
	 *
	 * @return current time stamping for this page
	 */
	inline int getTimeStamping();
	
	/**
	 * Obtains the data ( SignalSlotData object ) in the specified position
	 *
	 * @param absoluteX row for this CyclePage
	 * @param relativeY column for this cyclePage ( offset )
	 *
	 * @note modifies the page time stamping ( updates the CyclePage time stamping with the current
	 * global time stamping and increase the global time stamping )
	 */
	inline SignalSlotData* getData( int absoluteX, int relativeY );

	/**
	 * Modifies the page time stamping ( used to mark the page as new one and newest )
	 */
	inline void touch();
	
	/**
	 * Puts data ( SignalSlotData ) in an arbitrary position of the page
	 *
	 * @param absoluteX row for this CyclePage
	 * @param relativeY column for this cyclePage ( offset )
	 * @param ssi data
	 * 
	 * @note CyclePage is the ownership of SignalSlotData inside when the CyclePage is destroyed 
	 * SignalSlotData associated is destroyed too. It is recomended use new directly in this call
	 * to avoid pointer pointing invalid data
	 *
	 * @code
	 *    // example:
	 *    CyclePage* cp = cache.getPage( ... );
	 *
	 *    cp->setData( x, offset, new SignalSlotData( ... ) ); // new directly in call method
	 * @endcode
	 */
	inline void setData( int absoluteX, int relativeY, SignalSlotData* ssi );
	
	/**
	 * Clears al data in this CyclePage
	 */
	void clearPageData();

	/**
	 * Dumps the contents in this CyclePage
	 */
	void dump();

	/**
	 * Destructor
	 *
	 * Release all dynamic memory allocations
	 */
	~CyclePage();
};

///////////////////////////////////
/// Inline methods' definitions ///
///////////////////////////////////

inline int CyclePage::getPageCapacity()
{
	return PAGE_SIZE;
}

inline SignalSlotData* CyclePage::getData( int absoluteX, int relativeY )
{
	myTimeStamping = globalTimeStamping++;
	return ssiArray[absoluteX][relativeY];
}

inline void CyclePage::setData( int absoluteX, int relativeY, SignalSlotData* ssi )
{	
	ssiArray[absoluteX][relativeY] = ssi;
}

inline int CyclePage::getTimeStamping()
{
	return myTimeStamping;
}

inline void CyclePage::touch()
{
	myTimeStamping = globalTimeStamping++;
}

inline void CyclePage::setRows( int _rows )
{
	rows = _rows;
}

inline int CyclePage::getRows()
{
	return rows;
}

inline void CyclePage::setPageNumber( int idPage )
{
	pageNumber = idPage;
}

inline int CyclePage::getPageNumber()
{
	return pageNumber;
}

#endif // CYCLEPAGE_H
