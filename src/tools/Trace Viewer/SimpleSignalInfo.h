#ifndef SIMPLESIGNALINFO_H
	#define SIMPLESIGNALINFO_H

/**
 * Class used to exchange info between DataManager and the Visual component
 *
 * It contains the information for a SignalContent "slot" in a concrete cycle and for a concrete slot
 *
 * The number of slots in a signal are equal to its bandwidth
 *
 * A SimpleSignalInfo is not exactly a SignalContent. One SignalContent can be transformed
 * in one or more SimpleSignalInfo object.
 *
 * If one SignalContent is the content for a SignalInfo with bandwidth equals to 1 then only one
 * SimpleSignalInfo is required, if the SignalInfo asociated to its content ( SignalContent ) has
 * bandwidth N, then N SimpleSignalInfo are required ( one for each slot )
 *
 * @version 1.0
 * @date 25/07/2003
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es
 */
class SimpleSignalInfo 
{
private:

	
	int color; ///< Color used for represent this SimpleSignalInfo
	int nCookies; ///< Number of cookies
	int* cookies; ///< Cookies for this SimpleSignalInfo
	char* info; ///< Adition information ( for example the text for a vertex shader instruction )

public:
	
	/**
	 * Constructor
	 *
	 * @param color color for the visualization of this SimpleSignalInfo object
	 * @param cookies array with the cookies associated to this SimpleSignalInfo object
	 * @param nCookies number of cookies in 'cookies' parameter
	 * @param info option information ( for example a vertex shader instruction string )
	 */
	SimpleSignalInfo( int color, const int* cookies, int nCookies, const char* info = 0);
	
	/**
	 * Destructor
	 */
	~SimpleSignalInfo();

	/**
	 * Obtains the color used to visualize this SimpleSignalInfo object
	 *
	 * @return color identifier
	 */
	int getColor() const;

	/**
	 * Returns the information associated to this SimpleSignalInfo object
	 *
	 * @return cstring pointing to associated information
	 */
	const char* getInfo() const;
	
	/**
	 * Returns the cookies contained in this SimpleSignalInfo
	 *
	 * @param nCookies out parameter that collect the number of cookies in this SimpleSignalInfo object
	 * @returns the array of cookies, the array contains exactly nCookies cookies ( after call this method )
	 */
	const int* getCookies( int& nCookies ) const;
};

#endif // SIMPLESIGNALINFO_H
