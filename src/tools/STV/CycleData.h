#ifndef CYCLEDATA_H
	#define CYCLEDATA_H

#include "SignalDescriptionList.h"
#include "SignalData.h"

/**
 * Class that encapsulates the information of a cycle in a tracefile, every cycle from
 * a tracefile can be encapsulated in this object for subsequent ease of access
 *
 * @warning setSignalDescriptionList must be called before any CycleData creation or other method call
 *
 * @version 2.0
 * @date 24/10/2008
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es
 */
class CycleData
{
	/**
	 * Asociated object that contains all relative and important information about all signals that
	 * can dump data to the tracefile
	 */
	static SignalDescriptionList* sil;
		
	/** 
	 * Flag that  is true if setSignalDescriptionList() method has been called, false otherwise
	 */
	static bool isInfoListSet;
	
	int cycle; ///< cycle number
	int nSignals; ///< Number of signals that can dump data in a cycle	
	SignalData** sc; ///< Array containing the contents of all signals that can dump data

public:
	
	/**
	 * Associate a list of possible signals that can dump data in a cycle
	 *
	 * @param sil SignalDescriptionList with information about all signals that can dump data
	 */
	static void setSignalDescriptionList( SignalDescriptionList* sil );

	/**
	 * Constructor
	 *
	 * @note uses information from SignalDescriptionList associated via setSignalDescriptionList
	 */
	CycleData();

	/**
	 * obtains the cycle number of this CycleData object
	 *
	 * @return cycle number
	 */
	inline int getCycle() const;

	/** 
	 * sets the cycle number for this CycleData object
	 *
	 * @param cycle cycle number
	 */
	inline void setCycle( int cycle );

	/**
	 * Obtains the contents for a particular signal in this cycle
	 *
	 * @param sigId identificator from the signal we want contents
	 * @return The contents for this signal in the cycle represented by this CycleData object
	 */
	inline SignalData* getSignalData( int sigId );
	
	/**
	 * Insert contents for a particular signal
	 *
	 * Directly calls SignalDatas::addSignalDatas() for the Signal identified by sigId
	 *
	 * @param sigId Signal identificator
	 * @param sigContents contents in string format
	 */	
	inline bool addSignalDatas( int sigId, const char* sigContents );
	
	/**
	 * Insert contents for a particular signal 
	 *
	 * Directly calls SignalDatas::addSignalDatas() for the Signal identified by sigName
	 * First translate signal's name to its identifier and after that 
	 * calls SignalDatas::addSignalDatas()
	 *
	 * @param sigName Signal's name
	 * @param sigContents
	 */
	inline bool addSignalDatas( const char* sigName, const char* sigContents );

	/**
	 * Release CycleData data
	 */
	void clear();

	/**
	 * Destructor
	 *
	 * Release all dynamic memory associated with this object
	 */
	~CycleData();

	/**
	 * Dumps all contents in this object
	 */
	void dump() const;

};

///////////////////////////////////
/// Inline methods' definitions ///
///////////////////////////////////

inline int CycleData::getCycle() const
{
	return cycle;
}

inline void CycleData::setCycle( int cycle )
{
	this->cycle = cycle;
}


inline SignalData* CycleData::getSignalData( int sigId )
{
	return sc[sigId];
}

inline bool CycleData::addSignalDatas( int sigId, const char* sigContents )
{
	return ( sc[sigId]->addSignalDatas( sigContents ) );
}

inline bool CycleData::addSignalDatas( const char* sigName, const char* sigContents )
{
	int sigId = sil->get( sigName )->getId();
	return ( sc[sigId]->addSignalDatas( sigContents ) );
}

#endif // CYCLEDATA_H
