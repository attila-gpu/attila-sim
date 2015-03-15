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
 */

#ifndef ACDX_H
    #define ACDX_H

#include "ACDDevice.h"
#include "ACDShaderProgram.h"

#include "ACDXGlobalTypeDefinitions.h"
#include "ACDXFixedPipelineState.h"
#include "ACDXFixedPipelineSettings.h"
#include "ACDXConstantBinding.h"

#include <string>

/**
 * The acdlib namespace containing all the ACD interface classes and methods.
 */
namespace acdlib
{

/**
 * Define the vertex input semantic of the automatically generated vertex shaders
 */
enum ACDX_VERTEX_ATTRIBUTE_MAP
{
    ACDX_VAM_VERTEX = 0,
    ACDX_VAM_WEIGHT = 1,
    ACDX_VAM_NORMAL = 2,
    ACDX_VAM_COLOR = 3,
    ACDX_VAM_SEC_COLOR = 4,
    ACDX_VAM_FOG = 5,
    ACDX_VAM_SEC_WEIGHT = 6,
    ACDX_VAM_THIRD_WEIGHT = 7,
    ACDX_VAM_TEXTURE_0 = 8,
    ACDX_VAM_TEXTURE_1 = 9,
    ACDX_VAM_TEXTURE_2 = 10,
    ACDX_VAM_TEXTURE_3 = 11,
    ACDX_VAM_TEXTURE_4 = 12,
    ACDX_VAM_TEXTURE_5 = 13,
    ACDX_VAM_TEXTURE_6 = 14,
    ACDX_VAM_TEXTURE_7 = 15,
};

/**
 * ACDX utility/extension library
 * 
 * Utility library build on top of the ACD library that provides some
 * useful extended functions.
 *
 * This library provides methods and interfaces for:
 *
 *    1. Fixed Pipeline State Management. Compatible with both OpenGL큦 and 
 *       D3D9큦 Fixed Function.
 *
 *    2. Fixed Pipeline emulation through state-based generated 
 *       shader programs. Compatible with both OpenGL큦 and D3D9큦 
 *       Fixed Function.
 *
 *    3. Shader program compilation and constant resolving. Compatible
 *       with the OpenGL ARB vertex/fragment program 1.0 specification.
 *
 *
 * @author Jordi Roca Monfort (jroca@ac.upc.edu)
 * @version 1.0
 * @date 04/10/2007
 */

///////////////////////////////////////////////
// Fixed Pipeline State Management functions //
///////////////////////////////////////////////

/**
 * Creates a fixed pipeline state interface.
 *
 * @param acd_device The ACD rendering device.
 * @returns             The new created Fixed Pipeline state.
 */
ACDXFixedPipelineState* ACDXCreateFixedPipelineState(const ACDDevice* acd_device);

/**
 * Releases a ACDXFixedPipelineState interface object.
 *
 * @param fpState The ACDXFixedPipelineState interface to release.
 */
void ACDXDestroyFixedPipelineState(ACDXFixedPipelineState* fpState);


/////////////////////////////////////
// Fixed Pipeline Shader Emulation //
/////////////////////////////////////

/**
 * Initializes a ACDX_FIXED_PIPELINE_SETTINGS struct with the default
 * values, as follows:
 *
 * 1. Lighting disabled.
 *
 * 2. Local viewer is disabled (infinite viewer).
 *
 * 3. The normal normalization mode is "no normalize".
 *
 * 4. Separate specular color is disabled.
 *
 * 5. Lighting of both faces mode is disabled.
 *
 * 6. All the lights are disabled and the default type is directional.
 *
 * 7. Back face culling is enabled.
 *
 * 8. Color Material is disabled and the default mode is emission.
 *
 * 9. FOG is disabled, the default coordinate source is fragment depth and the
 *    default FOG mode is linear.
 *
 * 10. All the texture stages have a identity texture coordinate generation matrix
 *     and all the texture generate modes are set to vertex attrib.
 *
 * 11. All the texture stages are disabled, the active texture target is 2D, the
 *     default texture stage function is modulate and the base internal format is RGB.
 *       For the combine function mode the default parameters are:
 *       11.1 RGB and ALPHA combine function are "modulate".
 *     11.2 [COMPLETE]
 *         
 */
void ACDXLoadFPDefaultSettings(ACDX_FIXED_PIPELINE_SETTINGS& fpSettings);


/**
 * Generates the compiled vertex program that emulate the Fixed 
 * Pipeline based on the input struct and resolves the constant 
 * parameters according to the Fixed Pipeline state.
 *
 * @param  fpState           The Fixed Pipeline state interface.
 * @param  fpSettings       The input struct with the Fixed Pipeline settings.
 * @retval vertexProgram   Pointer to the result vertex program.
 *
 * @code
 *
 *  // Get the ACD device
 *  ACDDevice* acd_device = getACDDevice();
 *
 *  // Get the shader programs
 *  ACDShaderProgram* vertexProgram = acd_device->createShaderProgram();
 *
 *  ACDXFixedPipelineState* fpState = ACDXCreateFixedPipelineState(acd_device);
 *
 *  ACDX_FIXED_PIPELINE_SETTINGS fpSettings;
 *
 *  ACDXLoadFPDefaultSettings(fpSettings);
 *
 *  // Set FP State configuration
 *
 *  fpSettings.alphaTestEnabled = true;
 *  fpSettings.lights[0].enabled = true;
 *
 *  // ....
 *
 *  ACDXGenerateVertexProgram(fpState, fpSettings, vertexProgram);
 *
 *  acd_device->setVertexShader(vertexProgram);
 *
 * @endcode
 */
void ACDXGenerateVertexProgram(ACDXFixedPipelineState* fpState, const ACDX_FIXED_PIPELINE_SETTINGS& fpSettings, ACDShaderProgram* &vertexProgram);


/**
 * Generates the compiled fragment program that emulate the Fixed 
 * Pipeline based on the input struct and resolves the constant 
 * parameters according to the Fixed Pipeline state.
 *
 * @param  fpState           The Fixed Pipeline state interface.
 * @param  fpSettings       The input struct with the Fixed Pipeline settings.
 * @retval fragmentProgram   Pointer to the result vertex program.
 *
 * @code
 *
 *  // Get the ACD device
 *  ACDDevice* acd_device = getACDDevice();
 *
 *  // Get the shader programs
 *  ACDShaderProgram* vertexProgram = acd_device->createShaderProgram();
 *
 *  ACDXFixedPipelineState* fpState = ACDXCreateFixedPipelineState(acd_device);
 *
 *  ACDX_FIXED_PIPELINE_SETTINGS fpSettings;
 *
 *  ACDXLoadFPDefaultSettings(fpSettings);
 *
 *  // Set FP State configuration
 *
 *  fpSettings.alphaTestEnabled = true;
 *  fpSettings.lights[0].enabled = true;
 *
 *  // ....
 *
 *  ACDXGeneratePrograms(fpState, fpSettings, fragmentProgram);
 *
 *  acd_device->setFragmentShader(fragmentProgram);
 *
 * @endcode
 */
void ACDXGenerateFragmentProgram(ACDXFixedPipelineState* fpState, const ACDX_FIXED_PIPELINE_SETTINGS& fpSettings, ACDShaderProgram* &fragmentProgram);


/**
 * Generates the compiled shader programs that emulate the Fixed 
 * Pipeline based on the input struct and resolves the constant 
 * parameters according to the Fixed Pipeline state.
 *
 * @param  fpState           The Fixed Pipeline state interface.
 * @param  fpSettings       The input struct with the Fixed Pipeline settings.
 * @retval vertexProgram   Pointer to the result vertex program.
 * @retval fragmentProgram Pointer to the result fragment program.
 *
 * @code
 *
 *  // Get the ACD device
 *  ACDDevice* acd_device = getACDDevice();
 *
 *  // Get the shader programs
 *  ACDShaderProgram* vertexProgram = acd_device->createShaderProgram();
 *  ACDShaderProgram* fragmentProgram = acd_device->createShaderProgram();
 *
 *  ACDXFixedPipelineState* fpState = ACDXCreateFixedPipelineState(acd_device);
 *
 *  ACDX_FIXED_PIPELINE_SETTINGS fpSettings;
 *
 *  ACDXLoadFPDefaultSettings(fpSettings);
 *
 *  // Set FP State configuration
 *
 *  fpSettings.alphaTestEnabled = true;
 *  fpSettings.lights[0].enabled = true;
 *
 *  // ....
 *
 *  ACDXGeneratePrograms(fpState, fpSettings, vertexProgram, fragmentProgram);
 *
 *  acd_device->setVertexShader(vertexProgram);
 *  acd_device->setFragmentShader(fragmentProgram);
 *
 * @endcode
 */
void ACDXGeneratePrograms(ACDXFixedPipelineState* fpState, const ACDX_FIXED_PIPELINE_SETTINGS& fpSettings, ACDShaderProgram* &vertexProgram, ACDShaderProgram* &fragmentProgram);


////////////////////////////////
// Shader program compilation //
////////////////////////////////

/**
 * Creates and defines a new ACDXConstantBinding interface object given a single state Id.
 *
 * @param target        The way/target the constant is refered in the program.
 * @param constantIndex    The index of the refered constant in the program.
 * @param stateId        The id for the requested state to be used in the function
 * @param function        The pointer to the binding function interface
 * @param directSource  The optional/complementary source value to be used in the function
 * @returns                The new created constant binding.
 */
ACDXConstantBinding* ACDXCreateConstantBinding(ACDX_BINDING_TARGET target, 
                                               acd_uint constantIndex, 
                                               ACDX_STORED_FP_ITEM_ID stateId,
                                               const ACDXBindingFunction* function,
                                               ACDXFloatVector4 directSource = ACDXFloatVector4(acd_float(0)));

/**
 * Creates and defines a new ACDXConstantBinding interface object given a vector of state Ids.
 *
 * @param target        The way/target the constant is refered in the program.
 * @param constantIndex    The index of the refered constant in the program.
 * @param vStateId        The vector of ids for the requested states to be used in the function
 * @param function        The pointer to the binding function interface
 * @param directSource  The optional/complementary source value to be used in the function
 * @returns                The new created constant binding.
 */
ACDXConstantBinding* ACDXCreateConstantBinding(ACDX_BINDING_TARGET target, 
                                               acd_uint constantIndex, 
                                               std::vector<ACDX_STORED_FP_ITEM_ID> vStateId, 
                                               const ACDXBindingFunction* function,
                                               ACDXFloatVector4 directSource = ACDXFloatVector4(acd_float(0)));


/**
 * Releases a ACDXConstantBinding interface object.
 *
 * @param constantBinding The ACDXConstantBinding interface to release.
 */
void ACDXDestroyConstantBinding(ACDXConstantBinding* constantBinding);

/**
 * The ACDXCompiledProgram interface is empty of public methods
 * as long as it represents a closed object containing a compiled program
 * information only interpretable by the internal ACDX implementation.
 */
class ACDXCompiledProgram
{
};

/**
 * The ACDXCompileProgram functions compiles an ARB shader program source and
 * returns a ACDXCompiledProgram interface object that can be further resolved
 * (the corresponding constant bindings can be resolved) using the ACDXResolveProgram() 
 * function.
 *
 * @param  code        The program code string.
 * @returns            The compiled program object.
 */
ACDXCompiledProgram* ACDXCompileProgram(const std::string& code);

/**
 * The ACDXResolveProgram function resolves the compiled program updating a
 * ACDShaderProgram code and constant parameters accordingly to the
 * the Fixed Pipeline state.
 *
 * @param  fpState          The Fixed Pipeline state interface.
 * @param  cProgram          The compiled program object.
 * @param  constantList   The list of optional constant bindings.
 * @retval program          A valid/initialized ACDShaderProgram where to 
 *                          output the compiled code and constants.
 *
 * @code 
 *  
 *  // The following example creates a constant binding for the example program
 *  // at the program local 41 location. The constant binding consists in the
 *  // normalization of the light 0 direction.
 *  
 *  string example_program = 
 *  "!!ARBvp1.0
 *  #
 *  # Shade vertex color using the normalized light direction
 *  #
 *  PARAM mvp[4]    = { state.matrix.mvp };
 *  PARAM lightDir = { program.local[41] };
 *
 *  # This just does out vertex transform by the modelview projection matrix
 *  DP4 oPos.x, mvp[0], vertex.position;
 *  DP4 oPos.y, mvp[1], vertex.position;
 *  DP4 oPos.z, mvp[2], vertex.position;
 *  DP4 oPos.w, mvp[3], vertex.position;
 *  # Modulate vertex color with light direction
 *  MUL result.color, vertex.color, lightDir;
 *  END";
 *
 *  // Define the normalize binding function for the light direction
 *    class LightDirNormalizeFunction: public acdlib::ACDXBindingFunction
 *    {
 *    public:
 *
 *        virtual void function(acdlib::ACDXFloatVector4& constant, 
 *                              std::vector<acdlib::ACDXFloatVector4> vState,
 *                              const acdlib::ACDXFloatVector4& directSource) const
 *        {
 *            // Take the direction from the first STL vector element
 *            constant = acdlib::_normalize(vState[0]); 
 *        };
 *    };
 *
 *  // Get the ACD device
 *  ACDDevice* acd_device = getACDDevice();
 *
 *  // Get the shader programs
 *  ACDShaderProgram* vertexProgram = acd_device->createShaderProgram();
 *  ACDShaderProgram* fragmentProgram = acd_device->createShaderProgram();
 *
 *  ACDXFixedPipelineState* fpState = ACDXCreateFixedPipelineState(acd_device);
 *
 *  // The vector for required states.
 *  std::vector<ACDX_STORED_FP_ITEM_ID> vState;
 *    
 *  // Add the light 0 direction state
 *    vState.push_back(ACDX_LIGHT_DIRECTION);  
 *
 *  // Create the constant binding object
 *    ACDXConstantBinding* cb = ACDXCreateConstantBinding(ACDX_BINDING_TARGET_LOCAL, 
 *                                                        41, vState, 
 *                                                        new LightPosNormalizeFunction);
 *
 *  // Create the constant binding list
 *  ACDXConstantBindingList* cbl = new ACDXConstantBindingList();
 *  cbl->push_back(cb);
 *
 *  // Make the proper state changes
 *  ACDXLight& light = fpState->tl().light(0).setLightDirection(22.0f,30.0f,2.0f);
 *
 *  // Compile the program
 *    ACDXCompiledProgram* cProgram = ACDXCompileProgram(example_program);
 *
 *  // Resolve the program
 *  ACDXResolveProgram(fpState, cProgram, cbl, vertexProgram);
 *
 *  ACDXDestroyConstantBinding(cb);       // Release the constant binding object
 *  ACDXDestroyCompiledProgram(cProgram); // Release the compiled program object
 *  delete cbl;                              // Release the constant binding list
 *
 * @endcode
 */
ACDXConstantBindingList* ACDXResolveProgram(ACDXFixedPipelineState* fpState, const ACDXCompiledProgram* cProgram, const ACDXConstantBindingList* constantList, ACDShaderProgram* program);

/**
 * The ACDXDestroyCompiledProgram releases a ACDXCompiledProgram object
 *
 * @param cProgram The compiled program to destroy
 */
void ACDXDestroyCompiledProgram(ACDXCompiledProgram* cProgram);

/**
 * This struct represents a program compilation/resolving log.
 */
struct ACDX_COMPILATION_LOG
{
    /////////////////////////////////////
    // Vertex program compilation info //
    /////////////////////////////////////

    std::string vpSource;            ///< Source ARB code of the last compiled vertex program
    acd_uint vpNumInstructions;        ///< Number of ARB instructions of the last compiled vertex program
    acd_uint vpParameterRegisters;  ///< Number of used different parameter registers (see at ARB specifications) of the last compiled vertex program

    ///////////////////////////////////////
    // Fragment program compilation info //
    ///////////////////////////////////////

    std::string fpSource;            ///< Source ARB code of the last compiled fragment program
    acd_uint fpNumInstructions;        ///< Number of ARB instructions of the last compiled fragment program
    acd_uint fpParameterRegisters;    ///< Number of used different parameter registers (see at ARB specifications) of the last compiled fragment program
};

/**
 * The ACDXGetCompilationLog returns the log of the last compiled program 
 * by the ACDX library.
 *
 * @retval compileLog The compilation log structure to be filled out.
 */
void ACDXGetCompilationLog(ACDX_COMPILATION_LOG& compileLog);


////////////////////////////////
// Texture format conversions //
////////////////////////////////
// TO DO ... //

} // namespace acdlib

#endif // ACDX_H
