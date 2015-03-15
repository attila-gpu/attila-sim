#ifndef DATAMANAGER_H
	#define DATAMANAGER_H

#include <vector>
#include <map>
#include <string>

#include "SignalSlotData.h"
#include "SignalTraceReader.h"
#include "CyclePageCache.h"

/**
 * This class implements an interface for the visual component that demands data that must be shown.
 *
 * The communication between the visual component and this class is done via SignalSlotData objects.
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
 *    // Accessing data in SignalSlot identified by 3 id, in cycle 25
 *    SignalSlotData* ssi = dm.getData( 3, 25 );
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
 * @version 2.0
 * @date 24/10/2008
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
        std::string originalSignalName;
        std::string simpleSignalName;		

        SigIdentification( const std::string& name, int nSlot );
	};
		
	/**
	 * Allow fast acces to associated contents of a SignalSlotData 
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
	//std::vector<long> vOffset;
	std::map<long,long> offsetMap; // (page,offset)

	/**
	 * interface object responsible to manage the tracefile
	 */
	SignalTraceReader str;

	/**
	 * List with all original signals ( not simpleSignals ) in the tracefile
	 */
	SignalDescriptionList sil;

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

	/**
	 * Max number of pages in cache
	 */
	int cacheSize;

public:
	
	/**
	 * Constructor
	 */
	DataManager(int cacheSize);	

	/**
	 * Returns the number of cycles in the tracefile asociated to this DataManager
	 * Also returns the first cycle in tracefile
	 */
	int getCycles(int& firstCycle);

	/**
	 * Returns the number of cycles in the tracefile asociated to this DataManager
	 */
	int getCycles();

	/**
	 * Load and initialize the DataManager object
	 *
	 * @param traceFilePath tracefile's name
	 * @param maxCycles Maximum number of cycles that "should be" loaded
	 * @return 0 ok
	 *        -1 File not found
	 *        -2 Trace file format invalid 
	 */	
	int loadData( const char* traceFilePath, int maxCycles = -1 );	

	/**
	 * Gets the SignalSlotData in the coordinates specified
	 *
	 * @param x row identifier
	 * @param y column identifier
	 * @return SignalSlotData object in that position, returns NULL if there is not object in
	 * those coordinates
	 */
	const SignalSlotData* getData( int x, int y );
	
	/**
	 * Returs the original trace file signal name
	 *
	 * @return original trace file signal name
	 */
    const std::string& getSignalName(int simpleSignalId) const;

	/** 
	 * Returns how many simpleSignals are accesible ( how many there are )
	 */
	int getSignalSlots() const { return v.size(); }

	/**
     * Returns all the simple signals that match with the original signal
	 */
	std::vector<int> getSignalSlots(const char* originalSignalName);

	/**
	 * Returns all the original signals that match with the regular expression
	 */
	std::vector<std::string> getMatchingSignals(const char* pattern);

	/**
	 * Returns the SignalSlotName for a simpleSignalId
	 *
	 * @return simple signal name
	 */
    const std::string& getSignalSlotName( int simpleSignalId ) const;	

	/**
	 * Number of simple signals that compound this signal
	 *
	 * @note This value is equivalent to signal bandwidth
	 * @note returns 0 in case signal is not found
	 *
	 * @param signalName Name of the target signal
	 */
    int countSignalSlots(const std::string& signalName) const;

	/**
	 * Returns the simple signal position for a given signal
	 * The position of the first simple signal that is part of this signal in case
	 * the signal has more than one simple signal
	 *
	 * @note returns -1 if the signal is not found
	 */
	int getSignalSlotPosition(const char* signalName) const;

	/**
	 * Dumps the state of this DataManager object
	 */ 
	void dump() const;
	
};

#endif // DATAMANAGER_H