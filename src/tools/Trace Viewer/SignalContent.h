#ifndef SIGNALCONTENT_H
	#define SIGNALCONTENT_H

/**
 * SignalContent class encapsulates the dumped contents of a Signal for a concrete cycle
 *
 * @version 1.0
 * @date 25/07/2003
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es
 */
class SignalContent
{

private:

	int nCookies; ///< cookies in the cookies list ( only one counter for all cookieLists )
	
	/**
	 * color ( only one color for all slots )
	 * @note maybe is not correct only one color for all slots... ( it could be revised soon )
	 */
	int color; 
	
	int index; ///< next free SignalSlot
	int nSlots; ///<  signal's bw ( number of slots )

	/**
	 * 'bw' slots per SignalContent object
	 */
	struct SignalSlot 
	{
		int* cookieList;
		char* info;

		SignalSlot();
		~SignalSlot();
	};

	SignalSlot* ss; ///< Array of slots

public:

	/**
	 * Constructor
	 *
	 * Creates a Signal content with supoprt for log signals with arbitrary bandwidth
	 *
	 * @param bw signal's bandwidth ( number of slots )
	 */
	SignalContent( int bw );

	/**
	 * Obtains the cookie list ( array ) for a concrete slot in this signal
	 *
	 * @param slot slot identifier, must be a value between 0 and signal's bandwidth - 1
	 * @param nCookies out parameter, returns the number of cookies in the cookie array
	 * @return cookie array
	 */
	const int* getCookieList( int slot, int& nCookies ) const;
	
	/**
	 * Obtains the info asociated for a concrete slot
	 *
	 * @param slot must be a value between 0 and signal's bandwidth - 1
	 * @return information for this slot
	 */ 
	const char* getInfo( int nSlot ) const;

	/**
	 * Returns the color for this SignalContent
	 *
	 * @return color ID
	 *
	 * @note in further versions it could change to SignalContent::getColor( int slot ), allowing
	 * diferent colors per slot
	 */
	int getColor() const;	
	
	/**
	 * Add new contents for the next free slot
	 *
	 * @param sigContents contents in char format ( same format that in tracefile )
	 * @return true if the sigContents have been parsed correctly, false otherwise ( and not added )
	 */
	bool addSignalContents( const char* sigContents );
	
	/**
	 * Number of slots with content added
	 *
	 * @return slots that have contents
	 */
	int countSignalContents() const;

	/**
	 * Clear all information in this SignalContent
	 */
	void clear();

	/**
	 * Destructor
	 */
	~SignalContent();

	/**
	 * Dumps all information in this SignalContent
	 */
	void dump() const;
};

#endif // SIGNALCONTENT_H
