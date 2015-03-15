#ifndef DATAMANAGER_H
	#define DATAMANAGER_H

#include <vector>
#include "SimpleSignalInfo.h"
#include "SignalTraceReader.h"
#include "CyclePageCache.h"

/**
 * This class implements an interface for the visual component that demands data that must be shown.
 *
 * The communication between the visual component and this class is done via SimpleSignalInfo objects.
 *
 * Example of access
 * @code
 *
 *    // This code could be placed in a visual component for example
 *    DataManager dm;
 *
 *    // ...
 *    // ...
 *
 *    // Accessing data in SimpleSignal identified by 3 id, in cycle 25
 *    SimpleSignalInfo* ssi = dm.getData( 3, 25 );
 *    if ( ssi == NULL ) {
 *       // NO data for this simpleSignal ( 3 ) in this cycle ( 25 )
 *       
 *    }
 *    else {
 *       // Available contents ( accessibles via ssi pointer )
 *       ssi.doStuff( ... );
 *    }
 *
 *    // ...
 *
 * @endcode
 *
 * @note since version 1.1 is using a cache to avoid loading all tracefile data
 * @see CyclePageCache object
 *
 * @version 1.1
 * @date 24/07/2003
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es
 */
class DataManager
{

private:

	/**
	 * Encapsulate information needed to manage simpleSignalInfo objects
	 */ 
	struct SigIdentification
	{
		char* originalSignalName;
		char* simpleSignalName;		

		SigIdentification( const char* name, int nSlot );
		~SigIdentification();
	};
		
	/**
	 * Allow fast acces to associated contents of a SimpleSignalInfo 
	 */
	std::vector<SigIdentification*> v; // v.size() -> number of simple signals
	
	/**
	 * Cache object used to 'cache' the tracefile avoiding load all the file
	 */ 
	CyclePageCache* cache;

	/**
	 * Offsets for previous pages loaded or skipped ( allow fast access in tracefile )
	 * example: previousOffset[1] : offset in bytes for second page
	 */
	std::vector<long> vOffset;

	/**
	 * interface object responsible to manage the tracefile
	 */
	SignalTraceReader str;

	/**
	 * List with all original signals ( not simpleSignals ) in the tracefile
	 */
	SignalInfoList sil;

	int totalCyclesInFile; ///< Precalculated number of cycles in tracefile

	/**
	 * Load a CyclePage from tracefile ( using the SignalTraceReader object interface )
	 *
	 * @param page CyclePage used to write the data loaded from tracefile
	 */
	void loadPage( CyclePage* page );
	
	/**
	 * Skips one page in the tracefile
	 *
	 * @return the new offset in the file once the skip has been performed
	 */
	int skipPage();

public:
	
	/**
	 * Constructor
	 */
	DataManager();	

	/**
	 * Returns the number of cycles in the tracefile asociated to this DataManager
	 */
	int getCycles();

	/**
	 * Load and initialize the DataManager object
	 *
	 * @param traceFilePath tracefile's name
	 * @param maxCycles Maximum number of cycles that "should be" loaded
	 * @return number of cycles loaded ( not necessary ) or at least accesibles ( necessary )
	 */	
	int loadData( const char* traceFilePath, int maxCycles = -1 );	

	/**
	 * Gets the SimpleSignalInfo in the coordinates specified
	 *
	 * @param x row identifier
	 * @param y column identifier
	 * @return SimpleSignalInfo object in that position, returns NULL if there is not object in
	 * those coordinates
	 */
	const SimpleSignalInfo* getData( int x, int y );
	
	/**
	 * Returns the SimpleSignalName for a simpleSignalId
	 *
	 * @return simple signal name
	 */
	const char* getSimpleSignalName( int simpleSignalId ) const;	
		
	/** 
	 * Returns how many simpleSignals are accesible ( how many there are )
	 */
	int getSimpleSignals() const { return v.size(); }

	/**
	 * Dumps the state of this DataManager object
	 */ 
	void dump() const;
	
};

#endif // DATAMANAGER_H