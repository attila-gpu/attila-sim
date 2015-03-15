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
 * Shader Architecture Parameters.
 *
 */

/**
 *  @file ShaderArchitectureParameters.h
 *
 *  This file defines the ShaderArchitectureParameters class.
 *
 *  The ShaderArchitectureParameters class defines a container for architecture parameters (instruction
 *  execution latencies, repeation rates, etc) for a simulated shader architecture.
 *
 */

#ifndef _SHADERARCHITECTUREPARAMETERS_

#define _SHADERARCHITECTUREPARAMETERS_

#include "support.h"
#include "GPUTypes.h"
#include "ShaderInstruction.h"

#include <string>

using namespace std;

namespace gpu3d
{

/**
 *
 *  The ShaderArchitectureParameters class is a container for architecture parameters of the
 *  shader processor.
 *
 *  The class is a singleton used as a container for parameters.
 *
 */

class ShaderArchitectureParameters
{
private:

    static ShaderArchitectureParameters * shArchParams; /**<  Pointer to the singleton instance of the class.  */
    
    static u32bit varLatAOSExecLatencyTable[];      /**<  Execution latency table for variable execution latency AOS architecture.  */
    static u32bit varLatAOSRepeatRateTable[];       /**<  Repeat rate table for variable execution latency AOS architecture.  */
    
    static u32bit fixedLatAOSExecLatencyTable[];    /**<  Execution latency table for fixed execution latency AOS architecture.  */
    static u32bit fixedLatAOSRepeatRateTable[];     /**<  Repeat rate table for fixed execution latency AOS architecture.  */
    
    static u32bit varLatSOAExecLatencyTable[];      /**<  Execution latency table for variable execution latency SOA architecture.  */
    static u32bit varLatSOARepeatRateTable[];       /**<  Repeat rate table for variable exeuction latency SOA architecture.  */    
    
    static u32bit fixedLatSOAExecLatencyTable[];    /**<  Execution latency table for fixed execution latency SOA architecture.  */
    static u32bit fixedLatSOARepeatRateTable[];     /**<  Repeat rate table for fixed exeuction latency SOA architecture.  */    
    
        
    u32bit *execLatencyTable;    /**<  Pointer to the selected execution latency table.  */    
    u32bit *repeatRateTable;     /**<  Pointer to the selected repeat rate table.  */
    
    //  Constructor
    ShaderArchitectureParameters();
    
public:

    /**
     *
     *  The function returns a pointer to the single instance of the ShaderArchitectureParameters class.
     *  If the instance was not yet created it is created at this point.
     *
     *  @return The pointer to the singleton instance of the ShaderArchitectureParameters class.
     *     
     */
     
    static ShaderArchitectureParameters *getShaderArchitectureParameters();     
     
    
    /**
     *
     *  The function returns a list of available shader architecture configurations.
     *
     *  @param archList A reference to a string where to store a list with the names of the shader architecture
     *  configurations available for selection.
     *
     */
    
    static void architectureList(string &archList); 
    
    /**
     *
     *  The function is used to select the shader architecture configuration to use for
     *  queries.
     *
     *  @param archName Name of the shader architecture configuration to use for shader architecture
     *  parameter queries.
     *
     */
     
    void selectArchitecture(string archName);

    /**
     *
     *  The function is used to obtain the execution latency (pipeline depth) for a given shader instruction opcode.
     *
     *  @param opcode Shader Instruction opcode for which the execution latency is queried.
     *
     *  @return The execution latency in cycles for the shader instruction opcode.
     *
     */
     
    u32bit getExecutionLatency(ShOpcode opcode);
    
    
    /**
     *
     *  The function is used to obtain the repeat rate (cycles until next shader instruction can be issued) for
     *  a given shader instruction opcode.
     *
     *  @param opcode Shader Instruction opcode for which the repeat rate is queried.
     *
     *  @return The repeat rate (cycles until next shader instruction can be issued) for the shader instruction opcode.
     *
     */
     
    u32bit getRepeatRate(ShOpcode opcode);
};

}  // namespace gpu3d

#endif
