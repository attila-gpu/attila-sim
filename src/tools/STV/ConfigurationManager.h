#ifndef CONFIGURATIONMANAGER_H
	#define CONFIGURATIONMANAGER_H

#include <string>

/**
 * This class implements a basic configuration manager for the visualizer
 * once a configuration is loaded can be modified via simple methods
 * it's possible to save the configuration via saveConfiguration() method
 *
 * format file config.ini example:
 *
 * CyclesRulerOff=1
 * SizeSquares=10
 * SignalsRulerOff=1
 * AutoHideRulers=1
 * DivisionLinesOn=0
 * LoadMaxCycles=0
 *
 * @version 1.0
 * @date 24/07/2003
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es
 */

class ConfigurationManager {

private:
	
	// char* fileName; ///< configuration file name
    std::string fileName; ///< Configuration file name

	bool cyclesRulerOff; ///< negate flag for cycles ruler ( show or not )
	bool signalsRulerOff; ///< negate flag for signals ruler ( show or not )
	bool autoHideRulers; ///< flag for autohide rulers
	bool divisionLinesOn; ///< flag for division lines ( show or not )
	int loadMaxCycles; ///< cycles that are going to be loaded at maximum
	int sizeSquares; ///< Square sizes
	int cacheSize; ///<Cache size
	int pageStep; ///< UpPag/DownPag increments (in cycles)
	int colorOptions; ///< 0 -> none, 1 -> show color enum, 2 -> show cookies & color enum
	bool showIndividualInfo;
	bool showDefaultInfo;
	unsigned int treeDependencyLevel;
	std::string layoutFile;
	std::string colorMapFile;
    std::string signalTraceFile;

	enum // private constants
	{ 
		defaultSizeSquares = 10,
		defaultCacheSize = 5,
		defaultPageStep = 1,
		defaultColorOptions = 0
	};

public:

	/**
	 * Constructor
	 *
	 * @param pathName name of the config file
	 */
	// ConfigurationManager( const char* pathName );
    ConfigurationManager( const std::string& pathName );

	/**
	 * Loads data from the file specified in the constructor
	 *
	 * @return true if all goes well, otherwise returns false
	 */
	bool loadConfiguration();

	/**
	 * Save the current state in the file
	 *
	 * @param omitFalseValues if enabled false values are not saved in the file
	 * @return true if all goes well, otherwise returns false
	 */
	bool saveConfiguration( bool omitFalseValues = false );
	

	/**
	 * returns if cycles ruler must be shown or not
	 * 
	 * @return true implies cycles ruler must be shown, false implies not
	 */
	bool isCyclesRulerOn();
	
	
	/**
	 * returns if signals ruler must be shown or not
	 *
	 * @return true implies signals ruler must be shown, false implies not
	 */
	bool isSignalsRulerOn();

	/**
	 * returns if Autohide rulers is enabled or not
	 *
	 * @return true implies autohide enabled, false implies not
	 */
	bool isAutoHideRulersOn();

	/**
	 * returns if division lines must be shown or not
	 *
	 * @return true implies division lines must be shown, false implies not
	 */
	bool isDivisionLinesOn();
	
	
	/**
	 * returns the number of cycles "should be" loaded at maximum
	 *
	 * @return the number of cycles
	 */
	int getLoadMaxCycles();
	
	
	/**
	 * returns the default square size 
	 *
	 * @return square size
	 */
	int getSizeSquares();

	/**
	 * sets if Cycles ruler is going to be show or not
	 *
	 * @param on true is going to be shown, false otherwise
	 */
	void setCyclesRulerOn( bool on );
	
	/**
	 * sets if Signals ruler is gonna be show or not
	 *
	 * @param on true is going to be shown, false otherwise
	 */
	void setSignalsRulerOn( bool on );
	
	/**
	 * sets if Autohide is going to be performed or not
	 *
	 * @param on true implies autohide is going to be performed, false otherwise
	 */
	void setAutoHideRulersOn( bool on );
		
	/**
	 * sets if division lines are going to be shown or not
	 *
	 * @param on true implies yes, false no
	 */
	void setDivisionLinesOn( bool on );

	/**
	 * sets the maximum cycles "should be" loaded
	 *
	 * @param cycles number of cycles
	 */
	void setLoadMaxCycles( int cycles );
		
	/**
	 * sets the square size
	 *
	 * @param sizeS number of pixels per square
	 */
	void setSizeSquares( int sizeS );

	int getCacheSize() const { return cacheSize; }
	void setCacheSize(int sz) { cacheSize = sz; }

	int getPageStep() const;
	void setPageStep(int ps);

	int getColorOptions() const { return colorOptions; }
	void setColorOptions(int co) { colorOptions = co; }

	bool isShowIndividualInfo() const { return showIndividualInfo; }
	void setShowIndividualInfo( bool ii ) { showIndividualInfo = ii; }

	bool isShowDefaultInfo() const { return showDefaultInfo; }
	void setShowDefaultInfo( bool di ) { showDefaultInfo = di; }

	void setTreeDependencyLevel(unsigned int level);
	unsigned int getTreeDependencyLevel() const;

	std::string getLayoutFile() const;
	void setLayoutFile(std::string path);

	std::string getColorMapFile() const;
	void setColorMapFile(std::string path);

    std::string getSignalTraceFile() const;
    void setSignalTraceFile(const std::string& path);

	/**
	 * Dump the configuration manager state
	 */
	void dump();

};

#endif