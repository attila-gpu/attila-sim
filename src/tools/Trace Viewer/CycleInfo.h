#ifndef CYCLEINFO_H
	#define CYCLEINFO_H

#include "SignalInfoList.h"
#include "SignalContent.h"

/**
 * Class that encapsulates the information of a cycle in a tracefile, every cycle from
 * a tracefile can be encapsulated in this object for subsequent ease of access
 *
 * @warning setSignalInfoList must be called before any CycleInfo creation or other method call
 *
 * @version 1.0
 * @date 24/07/2003
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es
 */
class CycleInfo
{
	/**
	 * Asociated object that contains all relative and important information about all signals that
	 * can dump data to the tracefile
	 */
	static SignalInfoList* sil;
		
	/** 
	 * Flag that  is true if setSignalInfoList() method has been called, false otherwise
	 */
	static bool isInfoListSet;
	
	int cycle; ///< cycle number
	int nSignals; ///< Number of signals that can dump data in a cycle	
	SignalContent** sc; ///< Array containing the contents of all signals that can dump data


public:
	
	/**
	 * Associate a list of possible signals that can dump data in a cycle
	 *
	 * @param sil SignalInfoList with information about all signals that can dump data
	 */
	static void setSignalInfoList( SignalInfoList* sil );

	/**
	 * Constructor
	 *
	 * @note uses information from SignalInfoList associated via setSignalInfoList
	 */
	CycleInfo();

	/**
	 * obtains the cycle number of this CycleInfo object
	 *
	 * @return cycle number
	 */
	int getCycle() const;

	/** 
	 * sets the cycle number for this CycleInfo object
	 *
	 * @param cycle cycle number
	 */
	void setCycle( int cycle );

	/**
	 * Obtains the contents for a particular signal in this cycle
	 *
	 * @param sigId identificator from the signal we want contents
	 * @return The contents for this signal in the cycle represented by this CycleInfo object
	 */
	SignalContent* getSignalContent( int sigId );
	
	/**
	 * Insert contents for a particular signal
	 *
	 * Directly calls SignalContents::addSignalContents() for the Signal identified by sigId
	 *
	 * @param sigId Signal identificator
	 * @param sigContents contents in string format
	 */	
	bool addSignalContents( int sigId, const char* sigContents );
	
	/**
	 * Insert contents for a particular signal 
	 *
	 * Directly calls SignalContents::addSignalContents() for the Signal identified by sigName
	 * First translate signal's name to its identifier and after that 
	 * calls SignalContents::addSignalContents()
	 *
	 * @param sigName Signal's name
	 * @param sigContents
	 */
	bool addSignalContents( const char* sigName, const char* sigContents );

	/**
	 * Release CycleInfo data
	 */
	void clear();

	/**
	 * Destructor
	 *
	 * Release all dynamic memory associated with this object
	 */
	~CycleInfo();

	/**
	 * Dumps all contents in this object
	 */
	void dump() const;

};

#endif // CYCLEINFO_H
