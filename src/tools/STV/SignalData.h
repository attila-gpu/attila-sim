#ifndef SIGNALDATA_H
	#define SIGNALDATA_H

#include <vector>
#include <string>

/**
 * SignalData class encapsulates the dumped contents of a Signal for a concrete cycle
 *
 * @version 2.0
 * @date 24/10/2008
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es
 */
class SignalData
{

private:

	/**
	 * Obsolete (now inside SignalSlot)
	 */
	//int nCookies; ///< cookies in the cookies list ( only one counter for all cookieLists )
	////////////////////////////////////////////////////
	// Changed for supporting diferent cookies lenght //
	////////////////////////////////////////////////////
	/// Now it is inside SignalSlot
	
	/**
	 * Obsolete (now inside SignalSlot)
	 * color ( only one color for all slots )
	 * @note maybe is not correct only one color for all slots... ( it could be revised soon )
	 */
	//int color; 
	/// Now it is inside SignalSlot
	
	int index; ///< next free SignalSlot
	int nSlots; ///<  signal's bw ( number of slots )

	/**
	 * 'bw' slots per SignalData object
	 */
	struct SignalSlot 
	{
        std::vector<int> cookieList;
        std::string info;
		int color;
	};

    std::vector<SignalSlot> ss; ///< Array of slots

public:

	/**
	 * Constructor
	 *
	 * Creates a Signal content with supoprt for log signals with arbitrary bandwidth
	 *
	 * @param bw signal's bandwidth ( number of slots )
	 */
	inline SignalData( int bw );

	/**
	 * Obtains the cookie list ( array ) for a concrete slot in this signal
	 *
	 * @param slot slot identifier, must be a value between 0 and signal's bandwidth - 1
	 * @param nCookies out parameter, returns the number of cookies in the cookie array
	 * @return cookie array
	 */
    inline const std::vector<int>& getCookieList(int slot) const;
	
	/**
	 * Obtains the info asociated for a concrete slot
	 *
	 * @param slot must be a value between 0 and signal's bandwidth - 1
	 * @return information for this slot
	 */ 
    inline const std::string& getInfo(int slot) const;

	/**
	 * Returns the color for this SignalData
	 *
	 * @return color ID
	 *
	 * @note in further versions it could change to SignalData::getColor( int slot ), allowing
	 * diferent colors per slot
	 */
	inline int getColor(int slot) const;	
	
	/**
	 * Add new contents for the next free slot
	 *
	 * @param sigContents contents in char format ( same format that in tracefile )
	 * @return true if the sigContents have been parsed correctly, false otherwise ( and not added )
	 */
    bool addSignalDatas( const std::string& sigContents );
	
	/**
	 * Number of slots with content added
	 *
	 * @return slots that have contents
	 */
	inline int countSignalDatas() const;

	/**
	 * Clear all information in this SignalData
	 */
	void clear();

	/**
	 * Dumps all information in this SignalData
	 */
	void dump() const;
};

///////////////////////////////////
/// Inline methods' definitions ///
///////////////////////////////////

inline SignalData::SignalData( int bw ) : index(0), nSlots(bw)
{
    ss.reserve(bw);
    ss.resize(bw);
}

inline int SignalData::getColor(int slot) const
{
	return ss[slot].color;
}

inline const std::vector<int>& SignalData::getCookieList( int slot ) const
{
	return ss[slot].cookieList;
}

inline const std::string& SignalData::getInfo( int slot ) const
{
	return ss[slot].info;
}

inline int SignalData::countSignalDatas() const
{
	return index;
}

#endif // SIGNALDATA_H
