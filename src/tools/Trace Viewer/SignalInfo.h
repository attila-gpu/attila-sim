#ifndef SIGNALINFO_H
	#define SIGNALINFO_H

/**
 * SignalInfo class maintains all global and relevant information needed to represent the concept
 * of a Signal ( bandwidth, latency, etc )
 *
 * It is independent of the cycle we are. Its information is independant of time ( constant ).
 * Basically represent the characteristics for a Signal. 
 *
 * @see SignalInfoList
 * 
 * @version 1.0
 * @date 25/07/2003
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es
 */
class SignalInfo 
{

private:

	char* signalName; ///< Signal Name
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
	SignalInfo( const char* signalName, int id, int bw, int latency );

	/**
	 * Obtains Signal's name
	 *
	 * @return the name for this Signal
	 */
	const char* getSignalName() const;
	
	/**
	 * Obtains Signal's identifier
	 *
	 * @return Signal's identifier
	 */
	int getId() const;

	/**
	 * Obtains Signal's bandwidth
	 *
	 * @return Signal's bandwidth
	 */
	int getBw() const;
	
	/**
	 * Obtains Signal's latency
	 *
	 * @return Signal's latency
	 */
	int getLatency() const;

	/**
	 * Destructor
	 */
	~SignalInfo();
};

#endif // SIGNALINFO_H