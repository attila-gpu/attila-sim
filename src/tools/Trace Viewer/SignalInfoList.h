#ifndef SIGNALINFOLIST_H
	#define SIGNALINFOLIST_H

#include "SignalInfo.h"
#include <vector>

/**
 * SignalInfoList class manage with all unchangeable and global information for all Signals
 * previously added via SignalInfoList::add() method
 *
 * Basically it is a container for SignalInfo objects ( internally created and destroyed )
 *
 * @version 1.0
 * @date 25/07/2003
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es
 */
class SignalInfoList
{

private:

	/**
	 * The position in the vector is the ID
	 * The name associated with this id is the element ( string ) contained in this position
	 */
	std::vector<SignalInfo*> v;

public:
	
	/**
	 * Inserts a new name with the last ID free, and returns this ID
	 *
	 * Creates a new SignalInfo with the parameter info provided
	 *
	 * @param signalName signal's name
	 * @param bw Signal's bandwidth
	 * @param latency Signal's latency
	 * @return the ID required to identify this Signal
	 */
	int add( const char* signalName, int bw, int latency );

	/**
	 * Obtains a SignalInfo given its name
	 *
	 * @param signalName Signal's name
	 * @return the SignalInfo with signalName as name
	 */
	const SignalInfo* get( const char* signalName ) const;	
	
	/**
	 * Obtains aSignalInfo given its identifier
	 *
	 * @param SignalId Signal's identifier
	 * @return the SignalInfo with the identifier signalId
	 */
	const SignalInfo* get( int signalId ) const;

	/**
	 * Returns the number of SignalInfo objects now created
	 *
	 * @return the number of SignalInfo objects in this SignalInfoList
	 */
	int size() const;

	/**
	 * Destructor
	 */
	~SignalInfoList();

	/**
	 * Dumps the contents of this object
	 */
	void dump();

};

#endif // SIGNALINFOLIST_H
