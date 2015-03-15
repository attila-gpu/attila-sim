/**************************************************************************
 *
 * Copyright (c) 2002 - 2011 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 * Box class definition file.
 *
 */

#ifndef __BOX__
   #define __BOX__

#include "GPUTypes.h"
#include "GPUSignal.h"
#include "SignalBinder.h"
#include "StatisticsManager.h"
#include <string>
#include <sstream>

//using namespace std;

namespace gpu3d
{

#ifdef GPU_DEBUG_ON
    #define GPU_DEBUG_BOX(expr) { expr }
#else
    #define GPU_DEBUG_BOX(expr) if (debugMode) { expr }
#endif

/**
 *
 *  Defines the threshold in cycles that marks when a queue or stage is considered
 *  to be stalled after no input or output is sent or received.
 *
 */
 
static const u32bit STALL_CYCLE_THRESHOLD = 1000000;

/**
 * @b Box class is a common interface for all Boxes
 *
 * @b Files: Box.h,Box.cpp
 *
 * - Interface for all Boxes, all boxes must inherit from this class
 * - It is only an interface, can't be instanciated
 * - Has protected methods for registering signals
 *   ( newInputSignal & newOutputSignal versions )
 * - Has a @b static method @c getBox( @c name @c ) that give you a pointer
 *   to the box with this name
 *
 * @note Latest modifications:
 *   - Added support for name's identification ( @c getBox() )
 *   - Automatic registration of boxes
 *   - Added support for create built-in statistics ( only U32BIT types at the moment )
 *
 * @version 1.1
 * @date 9/12/2002 ( previous 23/11/2002 )
 * @author Carlos Gonzalez Rodriguez ( cgonzale@ac.upc.es )
 */
class Box {

private:

    /// Auxiliar container to implement a list of 'alive' ( created and not destroyed ) boxes
    struct ListBox {
        Box* box;
        ListBox* next;
        // Basic constructor performs link operation in the list
        ListBox( Box* aBox, ListBox* theNext ) : box(aBox), next(theNext) {}
    };

    static ListBox* lBox; ///< List of all boxes created so far

    char* name; ///< Name box
    Box* parent;///< Pointer to the parent's box
    SignalBinder& binder; ///< Reference to the global SignalBinder
    static GPUStatistics::StatisticsManager& sManager; ///< Reference to the global StatisticsManager

    static bool signalTracingFlag;
    static u64bit startCycle;
    static u64bit dumpCycles;

protected:

    bool debugMode;     /**<  Flag used to enable or disable debug messages.  */

    /**
     * Registers a new input Signal
     *
     * @param name signal's name ( if no prefix is especified )
     * @param bw bandwidth for this signal
     * @param latency latency for this signal
     * @param prefix if specified the the signal's name is prefix/name
     *
     * - Creates a new Signal for reading with defined bandwidth & latency (if the Signal did not exist ).
     * - Otherwise checks matching between previous values and returns the previous Signal created ( a pointer )
     * - An optional name can be appended to the first name: "prefix/name".
     * - 'bandwidth' should be defined with value > 0 to fast errors detecting.
     * - Defining latency or bandwidth like 0 makes latency or bandwidth remain undefined.
     *
     * @return a pointer to the signal
     */
    Signal* newInputSignal( const char* name, u32bit bw,
      u32bit latency = 0, const char* prefix = 0 );

    /**
     * Registers a new input Signal ( preferable version ) prefix
     *
     * @param name signal's name ( if no prefix is especified )
     * @param bw bandwidth for this signal
     * @param prefix if specified the the signal's name is prefix/name
     *
     * - Creates a new Signal for reading without defined latency  (if the Signal did not exist ).
     * - Otherwise checks matching between previous values and returns the previous Signal created ( a pointer )
     * - An optional name can be append to the first name: "prefix/name".
     * - 'bandwidth' should be defined with value > 0 to fast errors detecting.
     * - If bandwidth is set to 0 then bandwidth remains undefined.
     *
     * @return a pointer to the signal
     */
    Signal* newInputSignal( const char* name, u32bit bw, const char* prefix );

    /**
     * Registers a new output Signal ( preferable version )
     *
     * @param name signal's name ( if no prefix is especified )
     * @param bw bandwidth for this signal
     * @param latency latency for this signal
     * @param prefix if specified the the signal's name is prefix/name
     *
     * - Creates a new Signal for writing with defined bandwidth & latency (if the Signal did not exist ).
     * - Otherwise checks matching between previous values and returns the previous Signal created ( a pointer )
     * - An optional name can be append to the first name: "prefix/name".
     * - 'bandwidth' should be defined with value > 0 to fast errors detecting.
     * - Defining latency or bandwidth like 0 makes latency or bandwidth remain undefined.
     *
     * @return a pointer to the signal
     */
    Signal* newOutputSignal( const char* name, u32bit bw,
        u32bit latency, const char* prefix = 0);


    /**
     * Registers a new output Signal
     *
     * @param name signal's name ( if no prefix is especified )
     * @param bw bandwidth for this signal
     * @param prefix if specified the the signal's name is prefix/name
     *
     * - Creates a new Signal for writing without defined bandwidth & latency  (if the Signal did not exist ).
     * - Otherwise checks matching between previous values and returns the previous Signal created ( a pointer )
     * - An optional name can be append to the first name: "prefix/name".
     * - 'bandwidth' should be defined with value > 0 to fast errors detecting.
     * - If bandwidth is set to 0 then bandwidth remains undefined.
     *
     * @return a pointer to the signal
     */
    Signal* newOutputSignal( const char* name, u32bit bw, const char* prefix = 0);

    /**
     * Gets a reference to StatisticsManager
     */
    static GPUStatistics::StatisticsManager& getSM();

public:

    static void setSignalTracing(bool enabled, u64bit startCycle, u64bit dumpCycles);
    static bool isSignalTracingRequired(u64bit currentCycle);

    /**
     * Basic constructor
     *
     * Basic constructor called in initializers from derived boxes to initialize
     * general box properties ( name and parent Box )
     * Also adds the Box to the list of 'alive' Boxes
     *
     * @param nameBox name of the box
     *
     * @param parentBox A pointer to the parent Box, if there is not parent box defined the parameter must be 0
     *
     */
    Box( const char* nameBox, Box* parentBox = 0 );

    /**
     * Destructor
     *
     * Cleans static information when a box is deleted
     */
    virtual ~Box();

    /**
     *
     *  Assignment operator.  Implemented to remove some warnings.
     *
     */
     
    Box& operator=(const Box &in);

    /**
     * Work done in a cycle by the box
     * @note Must be implemented by all derived boxes ( pure virtual )
     *
     * @param cycle cycle in which clock is performed
     */
    virtual void clock( u64bit cycle )=0;

    /**
     *
     *  Returns a single line string with information about the state of the box.
     *
     *  @param stateString Reference to a string were to return the state line.
     *
     */

    virtual void getState(std::string &stateString);

    /**
     *
     *  Returns a string with debug information about the box.  The information can
     *  span over multiple lines.
     *
     *  @param debugInfo Reference to a string where to return debug information.
     *
     */

    virtual void getDebugInfo(std::string &debugInfo) const;

    /**
     *
     *  Returns a string with a list of the commands supported by the box.  The information
     *  may span multiple lines.
     *
     *  @param commandList Reference to a string where to return the command list.
     *
     */
    virtual void getCommandList(std::string &commandList);

    /**
     *
     *  Executes a box command.
     *
     *  @param commandStream A string stream with a box command and parameters.
     *
     */

    virtual void execCommand(std::stringstream &commandStream);

    /**
     *
     *  Sets the debug flag for the box.
     *
     *  @param debug New value of the debug flag.
     *
     */

    void setDebugMode(bool debug);
    
    /**
     *
     *  Detects if the box is stalled.
     *
     *  @param cycle Current simulation cycle.
     *  @param active Reference to a boolen variable where to store if the stall detection logic is active/implemented.
     *  @param stalled Reference to a boolean variable where to store if the box has been detected to be stalled.
     *
     */
     
    virtual void detectStall(u64bit cycle, bool &active, bool &stalled);

    /**
     *
     *  Returns a string reporting the stall condition of the box.
     *
     *  @param cycle Current simulation cycle.
     *  @param stallReport Reference to a string where to write the stall condition report for the box.
     *
     */
     
    virtual void stallReport(u64bit cycle, std::string &stallReport);
    
    /**
     * Gets name box
     *
     * @return the name of the box
     */
    const char* getName() const;

    /**
     * Gets a pointer to parent's box, the returning can be NULL.
     * @return A pointer to the parent box
     */
    const Box* getParent() const;

    /**
     * Gets a pointer to the box identified by name 'name'
     *
     * @param nameBox name of the target box
     * @return A pointer to the requested box ( null if name is not the name of a "alive" box )
     */
    static Box* getBox( const char* nameBox );

    /**
     *
     *  Returns a newline separated list with the names of all the boxes alive.
     *
     *  @param boxList A reference to a string were to stored the names of the
     *  boxes that are currently alive.
     *
     */

    static void getBoxNameList(std::string &boxList);

    /// Dumps the name of all boxes created so far
    static void dumpNameBoxes();

};

} // namespace gpu3d

#endif
