#ifndef SIGNALTRACEREADER_H
	#define SIGNALTRACEREADER_H

#include "SignalDescriptionList.h"
#include "CycleData.h"
#include <fstream>


/**
 * SignalTraceReader class is an interface that provides easy reading manipulation for a tracefile
 *
 * It provides methods for:
 *    - Read heading information about signals
 *    - Read cycle information
 *    - Random positioning in the file
 *    - Count cycles logged in tracefile
 *    - skip lines, white spaces, etc
 *
 * @version 2.0
 * @date 24/10/2008
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es
 */
class SignalTraceReader 
{
private:
    
    std::ifstream f; ///< input stream ( virtual file )
	char* fileName; ///< tracefile's name

public:

	/**
	 * Creates a SignalTraceReader object
	 */
	SignalTraceReader();

	/**
	 * Opens a tracefile
	 *
	 * @param filePath tracefile's name
	 * @return true if the file was opened correctly, false otherwise
	 */
	bool open( const char* filePath );

	/**
	 * Obtains the name of the current tracefile ( last opened )
	 *
	 * @return tracefile's name, NULL if no file has been opened yet
	 */
	const char* getFileName();

	/**
	 * Skip an arbitrary number of lines in the tracefile ( default 1 )
	 *
	 * @param nLines number of lines to skip
	 * @return true if all was ok, false otherwise ( eof found, or another strange problem )
	 */
	bool skipLines( int nLines = 1 );

	/**
	 * Checks if this trace is a valid trace
	 *
	 * @return true is the trace is a valid trace, false otherwise
	 * @note Current implementation is so simple, it just checks if first trace line
	 * is equal to "Signal Trace File v. 1.0"
	 */
	bool checkTrace( const char* file );

	/**
	 * Skip lines that only contains white spaces, tabs, etc
	 *
	 * @return true if all was ok, false otherwise ( eof found, or another strange problem )
	 *1/
	bool skipWhiteLines();

	/**
	 * Loads all global information about Signal traced in this file ( bw, latency, id, etc )
	 *
	 * @param sil out parameter that will collect all data read from the file
	 * @return number of signal's read
	 *
	 * @warning the positioning in the file must be performed explicity, for example you should
	 * skip first lines of the tracefile via SignalTraceReader::skipLines()
	 */
	int readSignalDescription( SignalDescriptionList& sil );
	
	/**
	 * Loads all the information ( traffic in signals ) in a cycle
	 *
	 * @param ci Object that will collect the informatio read
	 * @return true if all was ok, false otherwise ( for example eof reached )
	 *
     * @warning the positioning in the file must be performed explicity, for example you should
	 * skip first lines of the tracefile via SignalTraceReader::skipLines().
	 * It is required to be in the first position of the cycle information.
	 */
	bool readCycleData( CycleData& ci );
	
	/**
	 * Skips the current pendent information for the current cycle
	 *
	 * Formally locates the read pointer in the next cycle information in the tracefile
	 *
	 * @return true if all goes ok, false otherwise ( for example eof reached )
	 */
	bool skipCycleData();

	/**
	 * Move the read pointer to an absolute position in tracefile
	 *
	 * @param newPosition new position in the tracefile, this position is specified in bytes from
	 * the beginning of tracefile
	 */
	void setPosition( long newPosition );
	
	/**
	 * Obtains the current absolute position in the tracefile
	 *
	 * @return number of byte which the read pointer is pointing
	 */
	long getPosition();

	/**
	 * Count the number of cycles logged in a tracefile. Can be called in whichever moment because
	 * it do not share the same input stream therefore it do not modify the current read pointer
	 * position
	 *
	 * @param filePath tracefile's name
	 * 
	 * @warning in very large tracefiles this method can be very expensive in time execution
	 */
	static int countCyclesInfo( const char* filePath, int& firstCycle );

};


#endif // SIGNALTRACEREADER_H