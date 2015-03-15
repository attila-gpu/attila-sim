#ifndef SIGNALDESCRIPTION_H
	#define SIGNALDESCRIPTION_H

#include <string>

/**
 * SignalDescription class maintains all global and relevant information needed to represent the concept
 * of a Signal ( bandwidth, latency, etc )
 *
 * It is independent of the cycle we are. Its information is independant of time ( constant ).
 * Basically represent the characteristics for a Signal. 
 *
 * @see SignalDescriptionList
 * 
 * @version 2.0
 * @date 24/10/2008
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es
 */
class SignalDescription
{

private:

    std::string signalName; ///< Signal Name
	int id; ///< Signal identifier
	int bw; ///< Signal's bandwidth
	int latency; ///< Signal's latency

public:

	/**
	 * Constructor
	 *
	 * @param signalName signal's name
	 * @param id Signal's identifier
	 * @param bw Signal's bandwidth
	 * @param latency Signal's latency
	 */
    inline SignalDescription( const std::string& signalName, int id, int bw, int latency );

	/**
	 * Obtains Signal's name
	 *
	 * @return the name for this Signal
	 */
    inline const std::string& getSignalName() const;
	
	/**
	 * Obtains Signal's identifier
	 *
	 * @return Signal's identifier
	 */
	inline int getId() const;

	/**
	 * Obtains Signal's bandwidth
	 *
	 * @return Signal's bandwidth
	 */
	inline int getBw() const;
	
	/**
	 * Obtains Signal's latency
	 *
	 * @return Signal's latency
	 */
	inline int getLatency() const;

};

///////////////////////////////////
/// Inline methods' definitions ///
///////////////////////////////////

inline SignalDescription::SignalDescription( const std::string& sigName, int id, int bw, int latency ) :
    signalName(sigName), id(id), bw(bw), latency(latency)
{}

inline const std::string& SignalDescription::getSignalName() const
{
	return signalName;
}

inline int SignalDescription::getId() const
{
	return id;
}

inline int SignalDescription::getBw() const
{
	return bw;
}

inline int SignalDescription::getLatency() const
{
	return latency;
}

#endif // SIGNADESCRIPTION_H