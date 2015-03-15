#ifndef SIGNALDESCRIPTIONLIST_H
	#define SIGNALDESCRIPTIONLIST_H

#include <vector>
#include <string>

#include "SignalDescription.h"


/**
 * SignalDescriptionList class manage with all unchangeable and global information for all Signals
 * previously added via SignalDescriptionList::add() method
 *
 * Basically it is a container for SignalDescription objects ( internally created and destroyed )
 *
 * @version 2.0
 * @date 24/10/2008
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es
 */
class SignalDescriptionList
{

private:

	/**
	 * The position in the vector is the ID
	 * The name associated with this id is the element ( string ) contained in this position
	 */
	std::vector<SignalDescription*> v;

public:
	
	/**
	 * Inserts a new name with the last ID free, and returns this ID
	 *
	 * Creates a new SignalDescription with the parameter info provided
	 *
	 * @param signalName signal's name
	 * @param bw Signal's bandwidth
	 * @param latency Signal's latency
	 * @return the ID required to identify this Signal
	 */
    int add( const std::string& signalName, int bw, int latency );

	/**
	 * Obtains a SignalDescription given its name
	 *
	 * @param signalName Signal's name
	 * @return the SignalDescription with signalName as name
	 */
    const SignalDescription* get( const std::string& signalName ) const;	
	
	/**
	 * Obtains aSignalDescription given its identifier
	 *
	 * @param SignalId Signal's identifier
	 * @return the SignalDescription with the identifier signalId
	 */
	const SignalDescription* get( int signalId ) const;

	/**
	 * Returns the number of SignalDescription objects now created
	 *
	 * @return the number of SignalDescription objects in this SignalDescriptionList
	 */
	int size() const;

	/**
	 * Destructor
	 */
	~SignalDescriptionList();

	/**
	 * Dumps the contents of this object
	 */
	void dump();

};

#endif // SIGNALDESCRIPTIONLIST_H
