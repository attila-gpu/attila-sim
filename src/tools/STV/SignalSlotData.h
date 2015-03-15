#ifndef SIGNALSLOTDATA_H
	#define SIGNALSLOTDATA_H

#include <string>
#include <vector>

/**
 * Class used to exchange data between DataManager and the Visual component
 *
 * It contains the information for a SignalContent "slot" in a concrete cycle and for a concrete slot
 *
 * The number of slots in a signal are equal to its bandwidth
 *
 * A SignalSlotData is not exactly a SignalData object. One SignalData can be transformed
 * in one or more SignalSlotData object.
 *
 * If one SignalData is the content for a SignalData with bandwidth equals to 1 then only one
 * SignalSlotData is required, if the SignalData asociated to its content ( SignalData ) has
 * bandwidth N, then N SignalSlotData objects are required ( one for each slot )
 *
 * @version 2.0
 * @date 24/10/2008
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es
 */
class SignalSlotData
{
private:
	
	int color; ///< Color used for represent this SignalSlotData
    std::vector<int> cookies; ///< Cookies for this SignalSlotData
    std::string info; ///< Adition information ( for example the text for a vertex shader instruction )
	int cycle; ///< For supporting multi-part files

public:
	
	/**
	 * Constructor
	 *
	 * @param color color for the visualization of this SignalSlotData object
	 * @param cookies array with the cookies associated to this SignalSlotData object
	 * @param nCookies number of cookies in 'cookies' parameter
	 * @param info option information ( for example a vertex shader instruction string )
	 */
    inline SignalSlotData( int color, const std::vector<int>& cookies, int cycle, const std::string& info);
	
	/**
	 * Obtains the color used to visualize this SignalSlotData object
	 *
	 * @return color identifier
	 */
	inline int getColor() const;

	/**
	 * Returns the information associated to this SignalSlotData object
	 *
	 * @return cstring pointing to associated information
	 */
    inline const std::string& getInfo() const;
	
	/**
	 * Returns the cookies contained in this SignalSlotData
	 *
	 * @param nCookies out parameter that collect the number of cookies in this SignalSlotData object
	 * @returns the array of cookies, the array contains exactly nCookies cookies ( after call this method )
	 */
    inline const std::vector<int>& getCookies() const;

	/**
	 * Get cycle
	 */
	inline int getCycle() const;
};

///////////////////////////////////
/// Inline methods' definitions ///
///////////////////////////////////

inline SignalSlotData::SignalSlotData( int color, const std::vector<int>& cookies, int cycle, const std::string& info) :
    color(color), cookies(cookies), cycle(cycle), info(info)
{}

inline int SignalSlotData::getColor() const
{
	return color;
}

inline int SignalSlotData::getCycle() const
{
	return cycle;
}

inline const std::string& SignalSlotData::getInfo() const
{
	return info;
}

inline const std::vector<int>& SignalSlotData::getCookies() const
{
    return cookies;
}

#endif // SIGNALSLOTDATA_H
