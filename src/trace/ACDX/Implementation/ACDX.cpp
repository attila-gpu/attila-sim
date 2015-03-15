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

#include "ACDX.h"
#include "ACDXFixedPipelineStateImp.h"
#include "ACDXConstantBindingImp.h"
#include "ACDXCompiledProgramImp.h"
#include "InternalConstantBinding.h"
#include "ACDXProgramExecutionEnvironment.h"
#include "ACDXGLState.h"
#include "support.h"
#include "SettingsAdapter.h"
#include "ClusterBankAdapter.h"
#include "ACDXFPFactory.h"
#include "ACDXTLFactory.h"
#include "ACDXTLShader.h"
#include "ACDXShaderCache.h"
#include <list>

//#define ENABLE_GLOBAL_PROFILER
#include "GlobalProfiler.h"

using namespace acdlib;
using namespace std;

ACDXShaderCache vShaderCache;
ACDXShaderCache fShaderCache;

/* Creates a fixed pipeline state interface. */
ACDXFixedPipelineState* acdlib::ACDXCreateFixedPipelineState(const ACDDevice*)
{
    return new ACDXFixedPipelineStateImp(new ACDXGLState());
}

/* Releases a ACDXFixedPipelineState interface. */ 
void acdlib::ACDXDestroyFixedPipelineState(ACDXFixedPipelineState* fpState)
{
    ACDXFixedPipelineStateImp* fpStateImp = static_cast<ACDXFixedPipelineStateImp*>(fpState);

    delete fpStateImp->getGLState();
    
    delete fpStateImp;
}

/* Initializes a ACDX_FIXED_PIPELINE_SETTINGS struct with the default values */
void acdlib::ACDXLoadFPDefaultSettings(ACDX_FIXED_PIPELINE_SETTINGS& fpSettings)
{
    GLOBALPROFILER_ENTERREGION("ACDX", "", "")
    
    fpSettings.lightingEnabled = false;
    fpSettings.localViewer = false;
    fpSettings.normalizeNormals = ACDX_FIXED_PIPELINE_SETTINGS::ACDX_NO_NORMALIZE;
    fpSettings.separateSpecular = false;
    fpSettings.twoSidedLighting = false;
    
    for (unsigned int i = 0; i < ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_LIGHTS; i++)
    {
        fpSettings.lights[i].enabled = false;
        fpSettings.lights[i].lightType = ACDX_FIXED_PIPELINE_SETTINGS::ACDX_LIGHT_SETTINGS::ACDX_DIRECTIONAL;
    };

    fpSettings.cullEnabled = true;

    fpSettings.cullMode = ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CULL_BACK;

    fpSettings.colorMaterialMode = ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_EMISSION;

    fpSettings.colorMaterialEnabledFace = ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_NONE;

    fpSettings.fogEnabled = false;

    fpSettings.fogCoordinateSource = ACDX_FIXED_PIPELINE_SETTINGS::ACDX_FRAGMENT_DEPTH;

    fpSettings.fogMode = ACDX_FIXED_PIPELINE_SETTINGS::ACDX_FOG_EXP;

    for (unsigned int i = 0; i < ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_TEXTURE_STAGES; i++)
    {
        fpSettings.textureCoordinates[i].coordS = ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_VERTEX_ATTRIB;
        fpSettings.textureCoordinates[i].coordT = ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_VERTEX_ATTRIB;
        fpSettings.textureCoordinates[i].coordR = ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_VERTEX_ATTRIB;
        fpSettings.textureCoordinates[i].coordQ = ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_VERTEX_ATTRIB;
        
        fpSettings.textureCoordinates[i].textureMatrixIsIdentity = true;
    };

    for (unsigned int i = 0; i < ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_TEXTURE_STAGES; i++)
    {
        fpSettings.textureStages[i].enabled = false;
        fpSettings.textureStages[i].activeTextureTarget = ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_TEXTURE_2D;
        fpSettings.textureStages[i].textureStageFunction = ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_MODULATE;
        fpSettings.textureStages[i].baseInternalFormat = ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_RGBA;

        fpSettings.textureStages[i].combineSettings.RGBFunction = fpSettings.textureStages[i].combineSettings.ALPHAFunction = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_MODULATE;

        fpSettings.textureStages[i].combineSettings.srcRGB[0] = fpSettings.textureStages[i].combineSettings.srcALPHA[0] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_TEXTURE; 
        fpSettings.textureStages[i].combineSettings.srcRGB[1] = fpSettings.textureStages[i].combineSettings.srcALPHA[1] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_PREVIOUS;

        fpSettings.textureStages[i].combineSettings.srcRGB[2] = fpSettings.textureStages[i].combineSettings.srcALPHA[2] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_CONSTANT;
        fpSettings.textureStages[i].combineSettings.srcRGB[3] = fpSettings.textureStages[i].combineSettings.srcALPHA[3] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ZERO;

        fpSettings.textureStages[i].combineSettings.operandRGB[0] = fpSettings.textureStages[i].combineSettings.operandRGB[1] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SRC_COLOR;
        fpSettings.textureStages[i].combineSettings.operandRGB[2] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SRC_ALPHA;
    
        fpSettings.textureStages[i].combineSettings.operandALPHA[0] = fpSettings.textureStages[i].combineSettings.operandALPHA[1] = fpSettings.textureStages[i].combineSettings.operandALPHA[2] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SRC_ALPHA;
        fpSettings.textureStages[i].combineSettings.operandRGB[3] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ONE_MINUS_SRC_COLOR;
        fpSettings.textureStages[i].combineSettings.operandALPHA[3] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ONE_MINUS_SRC_ALPHA;

        fpSettings.textureStages[i].combineSettings.srcRGB_texCrossbarReference[0] = fpSettings.textureStages[i].combineSettings.srcRGB_texCrossbarReference[1] = fpSettings.textureStages[i].combineSettings.srcRGB_texCrossbarReference[2] = fpSettings.textureStages[i].combineSettings.srcRGB_texCrossbarReference[3] = i;
        fpSettings.textureStages[i].combineSettings.srcALPHA_texCrossbarReference[0] = fpSettings.textureStages[i].combineSettings.srcALPHA_texCrossbarReference[1] = fpSettings.textureStages[i].combineSettings.srcALPHA_texCrossbarReference[2] = fpSettings.textureStages[i].combineSettings.srcALPHA_texCrossbarReference[3] = i;
        fpSettings.textureStages[i].combineSettings.RGBScale = 1;
        fpSettings.textureStages[i].combineSettings.ALPHAScale = 1;
    };

    fpSettings.alphaTestEnabled = false;
    fpSettings.alphaTestFunction = ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_ALWAYS;

    GLOBALPROFILER_EXITREGION()
}



/* Private auxiliar function */
void checkValues(const ACDX_FIXED_PIPELINE_SETTINGS& fpSettings)
{
    GLOBALPROFILER_ENTERREGION("ACDX", "", "")
    switch(fpSettings.normalizeNormals)
    {
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_NO_NORMALIZE:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_RESCALE:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_NORMALIZE:
            break;
        default:
            panic("ACDX.cpp","checkValues()","Unexpected normalize mode value");
    }

    for (unsigned int i = 0; i < ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_LIGHTS; i++)
    {
        switch(fpSettings.lights[i].lightType)
        {
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_LIGHT_SETTINGS::ACDX_DIRECTIONAL:
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_LIGHT_SETTINGS::ACDX_POINT:
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_LIGHT_SETTINGS::ACDX_SPOT:
                break;
            default:
                panic("ACDX.cpp","checkValues()","Unexpected light type");
        }
    };

    switch(fpSettings.cullMode)
    {
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CULL_NONE:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CULL_FRONT:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CULL_BACK:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CULL_FRONT_AND_BACK:
            break;
        default:
            panic("ACDX.cpp","checkValues()","Unexpected cull mode");
    }

    switch(fpSettings.colorMaterialMode)
    {
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_EMISSION:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_AMBIENT:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_DIFFUSE:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_SPECULAR:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_AMBIENT_AND_DIFFUSE:
            break;
        default:
            panic("ACDX.cpp","checkValues()","Unexpected color material mode");
    }

    switch(fpSettings.colorMaterialMode)
    {
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_EMISSION:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_AMBIENT:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_DIFFUSE:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_SPECULAR:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_AMBIENT_AND_DIFFUSE:
            break;
        default:
            panic("ACDX.cpp","checkValues()","Unexpected color material mode");
    }

    switch(fpSettings.colorMaterialEnabledFace)
    {
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_NONE:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_FRONT:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_BACK:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_FRONT_AND_BACK:
            break;
        default:
            panic("ACDX.cpp","checkValues()","Unexpected color material face");
    }

    switch(fpSettings.fogCoordinateSource)
    {
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_FRAGMENT_DEPTH:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_FOG_COORD:
            break;
        default:
            panic("ACDX.cpp","checkValues()","Unexpected FOG coordinate source");
    }

    switch(fpSettings.fogMode)
    {
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_FOG_LINEAR:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_FOG_EXP:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_FOG_EXP2:
            break;
        default:
            panic("ACDX.cpp","checkValues()","Unexpected FOG Mode");
    }

    for (unsigned int i = 0; i < ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_TEXTURE_STAGES; i++)
    {
        switch(fpSettings.textureCoordinates[i].coordS)
        {
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_VERTEX_ATTRIB:
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_OBJECT_LINEAR:
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_EYE_LINEAR:
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_SPHERE_MAP:
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_REFLECTION_MAP:
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_NORMAL_MAP:
                break;
            default:
                panic("ACDX.cpp","checkValues()","Unexpected texture coordinate mode");
        }
    };

    for (unsigned int i = 0; i < ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_TEXTURE_STAGES; i++)
    {
        switch(fpSettings.textureStages[i].activeTextureTarget)
        {
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_TEXTURE_1D:
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_TEXTURE_2D:
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_TEXTURE_3D:
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_TEXTURE_CUBE_MAP:
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_TEXTURE_RECT:
                break;
            default:
                panic("ACDX.cpp","checkValues()","Unexpected active texture target");
        }
    
        switch(fpSettings.textureStages[i].textureStageFunction)
        {
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_REPLACE:
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_MODULATE:
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_DECAL:
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_BLEND:
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_ADD:
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_COMBINE:
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_COMBINE_NV:
                break;
            default:
                panic("ACDX.cpp","checkValues()","Unexpected texture stage function");
        }

        switch(fpSettings.textureStages[i].baseInternalFormat)
        {
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_ALPHA:
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_LUMINANCE:
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_LUMINANCE_ALPHA:
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_DEPTH:
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_INTENSITY:
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_RGB:
            case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_RGBA:
                break;
            default:
                panic("ACDX.cpp","checkValues()","Unexpected base internal format");
        }
        
        switch(fpSettings.textureStages[i].combineSettings.RGBFunction)
        {
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_REPLACE:
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_MODULATE:
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ADD:
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ADD_SIGNED:
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_INTERPOLATE:
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SUBTRACT:
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_DOT3_RGB:
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_DOT3_RGBA:
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_MODULATE_ADD:
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_MODULATE_SIGNED_ADD:
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_MODULATE_SUBTRACT:
                break;
            default:
                panic("ACDX.cpp","checkValues()","Unexpected combine RGB function");
        }

        switch(fpSettings.textureStages[i].combineSettings.ALPHAFunction)
        {
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_REPLACE:
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_MODULATE:
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ADD:
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ADD_SIGNED:
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_INTERPOLATE:
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SUBTRACT:
            //case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_DOT3_RGB:
            //case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_DOT3_RGBA:
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_MODULATE_ADD:
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_MODULATE_SIGNED_ADD:
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_MODULATE_SUBTRACT:
                break;
            default:
                panic("ACDX.cpp","checkValues()","Unexpected combine ALPHA function");
        }

        for (int j=0; j<4; j++)
        {
            switch(fpSettings.textureStages[i].combineSettings.srcRGB[j])
            {
                case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ZERO:
                case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ONE:
                case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_TEXTURE:
                case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_TEXTUREn:
                case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_CONSTANT:
                case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_PRIMARY_COLOR:
                case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_PREVIOUS:
                    break;
                default:
                    panic("ACDX.cpp","checkValues()","Unexpected combine RGB source");
            }

            switch(fpSettings.textureStages[i].combineSettings.srcALPHA[j])
            {
                case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ZERO:
                case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ONE:
                case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_TEXTURE:
                case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_TEXTUREn:
                case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_CONSTANT:
                case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_PRIMARY_COLOR:
                case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_PREVIOUS:
                    break;
                default:
                    panic("ACDX.cpp","checkValues()","Unexpected combine ALPHA source");
            }

            switch(fpSettings.textureStages[i].combineSettings.operandRGB[j])
            {
                case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SRC_COLOR:
                case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ONE_MINUS_SRC_COLOR:
                case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SRC_ALPHA:
                case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ONE_MINUS_SRC_ALPHA:
                    break;
                default:
                    panic("ACDX.cpp","checkValues()","Unexpected combine RGB operand");
            }

            switch(fpSettings.textureStages[i].combineSettings.operandALPHA[j])
            {
                case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SRC_COLOR:
                case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ONE_MINUS_SRC_COLOR:
                case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SRC_ALPHA:
                case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ONE_MINUS_SRC_ALPHA:
                    break;
                default:
                    panic("ACDX.cpp","checkValues()","Unexpected combine ALPHA operand");
            }

            if (fpSettings.textureStages[i].combineSettings.srcRGB_texCrossbarReference[j] >=
                ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_TEXTURE_STAGES)
                panic("ACDX.cpp","checkValues()","Unexpected TEXTUREn RGB source with 'n' greater than max texture stages");

            if (fpSettings.textureStages[i].combineSettings.srcALPHA_texCrossbarReference[j] >=
                ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_TEXTURE_STAGES)
                panic("ACDX.cpp","checkValues()","Unexpected TEXTUREn ALPHA source with 'n' greater than max texture stages");
            
            if ( (fpSettings.textureStages[i].combineSettings.RGBScale != 1) &&
                 (fpSettings.textureStages[i].combineSettings.RGBScale != 2) &&
                 (fpSettings.textureStages[i].combineSettings.RGBScale != 4) )
                 panic("ACDX.cpp","checkValues()","Unexpected RGB scale factor <> {1,2,4}");

            if ( (fpSettings.textureStages[i].combineSettings.ALPHAScale != 1) &&
                 (fpSettings.textureStages[i].combineSettings.ALPHAScale != 2) &&
                 (fpSettings.textureStages[i].combineSettings.ALPHAScale != 4) )
                 panic("ACDX.cpp","checkValues()","Unexpected ALPHA scale factor <> {1,2,4}");
        }    

    };

    switch(fpSettings.alphaTestFunction)
    {
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_NEVER:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_ALWAYS:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_LESS:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_LEQUAL:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_EQUAL:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_GEQUAL:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_GREATER:
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_NOTEQUAL:
            break;
        default:
            panic("ACDX.cpp","checkValues()","Unexpected alpha test function");
    }


    GLOBALPROFILER_EXITREGION()
};

ACDXConstantBinding* acdlib::ACDXCreateConstantBinding(ACDX_BINDING_TARGET target, 
                                                       acd_uint constantIndex, 
                                                       ACDX_STORED_FP_ITEM_ID stateId,
                                                       const ACDXBindingFunction* function,
                                                       ACDXFloatVector4 directSource)
{
    std::vector<ACDX_STORED_FP_ITEM_ID> vStateIds;
    
    vStateIds.push_back(stateId);

    return new ACDXConstantBindingImp(target, constantIndex, vStateIds, function, directSource);
}

ACDXConstantBinding* acdlib::ACDXCreateConstantBinding(ACDX_BINDING_TARGET target, 
                                                       acd_uint constantIndex, 
                                                       std::vector<ACDX_STORED_FP_ITEM_ID> vStateIds, 
                                                       const ACDXBindingFunction* function,
                                                       ACDXFloatVector4 directSource)
{
    return new ACDXConstantBindingImp(target, constantIndex, vStateIds, function, directSource);
}


void acdlib::ACDXDestroyConstantBinding(ACDXConstantBinding* constantBinding)
{
    ACDXConstantBindingImp* cbi = static_cast<ACDXConstantBindingImp*>(constantBinding);
    delete cbi;
}

/* Generates the compiled vertex program that emulate the Fixed Pipeline */
void acdlib::ACDXGenerateVertexProgram(ACDXFixedPipelineState* fpState, const ACDX_FIXED_PIPELINE_SETTINGS& fpSettings, ACDShaderProgram* &vertexProgram)
{
    GLOBALPROFILER_ENTERREGION("ACDXGenerateVertexProgram", "", "")

    ACDXConstantBindingList vpcbList;
    ACDXTLShader* vtlsh;
    ACDXConstantBindingList* finalBindingList;

    /* Check all the settings values in the input struct to be proper values. */
    checkValues(fpSettings);

    /* Generate the settings adapter to get ACDXTLState and ACDXFPState interfaces. */
    SettingsAdapter sa(fpSettings);

    if ( !vShaderCache.isCached(fpSettings, vertexProgram, finalBindingList) )
    {
        //cout << "VERTEX SHADER MISSSSSSSSSSSSSS " << endl;

        ////////////////////////////////////
        // Construct the fragment program //
        ////////////////////////////////////

        vtlsh = ACDXTLFactory::constructVertexProgram(sa.getTLState());

        vpcbList = vtlsh->getConstantBindingList();

        ///////////////////////////////////////////////////////////////////////////
        // Compile and resolve the vertex program in case it is not in the cache //
        ///////////////////////////////////////////////////////////////////////////

        GLOBALPROFILER_ENTERREGION("compile FX VSH", "", "")
        ACDXCompiledProgram* cVertexProgram = ACDXCompileProgram(vtlsh->getCode());
        GLOBALPROFILER_EXITREGION()

        finalBindingList = ACDXResolveProgram(fpState, cVertexProgram, &vpcbList, vertexProgram);

        ACDXDestroyCompiledProgram(cVertexProgram);

        //printf("Created Vertex Shader :\n");
        //vertexProgram->printASM(cout);
        //printf("----------------------\n");
        
        vShaderCache.addCacheEntry(fpSettings, vertexProgram, finalBindingList);   

        delete vtlsh;
    }
    else
    {
        //cout << "VERTEX SHADER HITTTTTTTTTTTT " << endl;

        //printf("Cached Vertex Shader :\n");
        //vertexProgram->printASM(cout);
        //printf("----------------------\n");     

        // Synchronize ACDXFixedPipelineState states to ACDXGLState
        ACDXFixedPipelineStateImp* fpStateImp = static_cast<ACDXFixedPipelineStateImp*>(fpState);
        fpStateImp->sync();

        ACDXConstantBindingList::iterator iter = finalBindingList->begin();

        while (iter != finalBindingList->end())
        {
            ACDXFloatVector4 constant;
            
            // Resolve the constant
            (*iter)->resolveConstant(fpStateImp, constant);

            // Put the constant value in the ACDShaderProgram object
            vertexProgram->setConstant((*iter)->getConstantIndex(), constant.c_array());
            
            iter++;
        }

    }

    
    GLOBALPROFILER_EXITREGION()   
}

/* Generates the compiled shader programs that emulate the Fixed Pipeline */
void acdlib::ACDXGenerateFragmentProgram(ACDXFixedPipelineState* fpState, const ACDX_FIXED_PIPELINE_SETTINGS& fpSettings, ACDShaderProgram* &fragmentProgram)
{
    GLOBALPROFILER_ENTERREGION("ACDXGenerateFragmentProgram", "", "")

    ACDXConstantBindingList vpcbList;
    ACDXConstantBindingList* finalBindingList;

    /* Check all the settings values in the input struct to be proper values. */
    checkValues(fpSettings);

    /* Generate the settings adapter to get ACDXTLState and ACDXFPState interfaces. */
    SettingsAdapter sa(fpSettings);
    
    if ( !fShaderCache.isCached(fpSettings, fragmentProgram, finalBindingList) )
    {
        //cout << "FRAGMENT SHADER MISSSSSSSSSSSSSS " << endl;
          
        ////////////////////////////////////
        // Construct the fragment program //
        ////////////////////////////////////

        ACDXTLShader* ftlsh = ACDXFPFactory::constructFragmentProgram(sa.getFPState());
        
        ACDXConstantBindingList fpcbList = ftlsh->getConstantBindingList();

        //////////////////////////////////////////////
        // Compile and resolve the fragment program //
        //////////////////////////////////////////////

        GLOBALPROFILER_ENTERREGION("compile FX FSH", "", "")
        ACDXCompiledProgram* cFragmentProgram = ACDXCompileProgram(ftlsh->getCode());
        GLOBALPROFILER_EXITREGION()

        finalBindingList = ACDXResolveProgram(fpState, cFragmentProgram, &fpcbList, fragmentProgram);

        ACDXDestroyCompiledProgram(cFragmentProgram);

        //printf("Created Fragment Shader :\n");
        //fragmentProgram->printASM(cout);
        //printf("----------------------\n");

        fShaderCache.addCacheEntry(fpSettings, fragmentProgram, finalBindingList);   

        delete ftlsh;
            
        GLOBALPROFILER_EXITREGION()
    }
    else
    {
        //printf("Cached Fragment Shader :\n");
        //fragmentProgram->printASM(cout);
        //printf("----------------------\n");

        // Synchronize ACDXFixedPipelineState states to ACDXGLState
        ACDXFixedPipelineStateImp* fpStateImp = static_cast<ACDXFixedPipelineStateImp*>(fpState);
        fpStateImp->sync();

        ACDXConstantBindingList::iterator iter = finalBindingList->begin();

        while (iter != finalBindingList->end())
        {
            ACDXFloatVector4 constant;
            
            // Resolve the constant
            (*iter)->resolveConstant(fpStateImp, constant);

            // Put the constant value in the ACDShaderProgram object
            fragmentProgram->setConstant((*iter)->getConstantIndex(), constant.c_array());

            iter++;
        }

    }

    GLOBALPROFILER_EXITREGION()   
}

/* Generates the compiled shader programs that emulate the Fixed Pipeline */
void acdlib::ACDXGeneratePrograms(ACDXFixedPipelineState* fpState, const ACDX_FIXED_PIPELINE_SETTINGS& fpSettings, ACDShaderProgram* &vertexProgram, ACDShaderProgram* &fragmentProgram)
{
    GLOBALPROFILER_ENTERREGION("ACDXGeneratePrograms", "", "")

    /* Check all the settings values in the input struct to be proper values. */
    checkValues(fpSettings);

    /* Generate the settings adapter to get ACDXTLState and ACDXFPState interfaces. */
    SettingsAdapter sa(fpSettings);

    //////////////////////////////////
    // Construct the vertex program //
    //////////////////////////////////

    ACDXConstantBindingList vpcbList;
    ACDXTLShader* vtlsh;
    ACDXConstantBindingList* finalBindingList;

    if ( !vShaderCache.isCached(fpSettings, vertexProgram, finalBindingList) )
    {
        //cout << "VERTEX SHADER MISSSSSSSSSSSSSS " << endl;

        ////////////////////////////////////
        // Construct the fragment program //
        ////////////////////////////////////

        vtlsh = ACDXTLFactory::constructVertexProgram(sa.getTLState());

        vpcbList = vtlsh->getConstantBindingList();

        ///////////////////////////////////////////////////////////////////////////
        // Compile and resolve the vertex program in case it is not in the cache //
        ///////////////////////////////////////////////////////////////////////////

        GLOBALPROFILER_ENTERREGION("compile FX VSH", "", "")
        ACDXCompiledProgram* cVertexProgram = ACDXCompileProgram(vtlsh->getCode());
        GLOBALPROFILER_EXITREGION()

        finalBindingList = ACDXResolveProgram(fpState, cVertexProgram, &vpcbList, vertexProgram);

        ACDXDestroyCompiledProgram(cVertexProgram);

        //printf("Created Vertex Shader :\n");
        //vertexProgram->printASM(cout);
        //printf("----------------------\n");
        
        vShaderCache.addCacheEntry(fpSettings, vertexProgram, finalBindingList);   

        delete vtlsh;
    }
    else
    {
        //cout << "VERTEX SHADER HITTTTTTTTTTTT " << endl;

        //printf("Cached Vertex Shader :\n");
        //vertexProgram->printASM(cout);
        //printf("----------------------\n");

        // Synchronize ACDXFixedPipelineState states to ACDXGLState
        ACDXFixedPipelineStateImp* fpStateImp = static_cast<ACDXFixedPipelineStateImp*>(fpState);
        fpStateImp->sync();

        ACDXConstantBindingList::iterator iter = finalBindingList->begin();

        while (iter != finalBindingList->end())
        {
            ACDXFloatVector4 constant;
            
            // Resolve the constant
            (*iter)->resolveConstant(fpStateImp, constant);

            // Put the constant value in the ACDShaderProgram object
            vertexProgram->setConstant((*iter)->getConstantIndex(), constant.c_array());
            
            iter++;
        }

    }
    
    if ( !fShaderCache.isCached(fpSettings, fragmentProgram, finalBindingList) )
    {
        //cout << "FRAGMENT SHADER MISSSSSSSSSSSSSS " << endl;
          
        ////////////////////////////////////
        // Construct the fragment program //
        ////////////////////////////////////

        ACDXTLShader* ftlsh = ACDXFPFactory::constructFragmentProgram(sa.getFPState());
        
        ACDXConstantBindingList fpcbList = ftlsh->getConstantBindingList();

        //////////////////////////////////////////////
        // Compile and resolve the fragment program //
        //////////////////////////////////////////////

        GLOBALPROFILER_ENTERREGION("compile FX FSH", "", "")
        ACDXCompiledProgram* cFragmentProgram = ACDXCompileProgram(ftlsh->getCode());
        GLOBALPROFILER_EXITREGION()

        finalBindingList = ACDXResolveProgram(fpState, cFragmentProgram, &fpcbList, fragmentProgram);

        ACDXDestroyCompiledProgram(cFragmentProgram);

        //printf("Created Fragment Shader :\n");
        //fragmentProgram->printASM(cout);
        //printf("----------------------\n");

        fShaderCache.addCacheEntry(fpSettings, fragmentProgram, finalBindingList);   

        delete ftlsh;
            
        GLOBALPROFILER_EXITREGION()
    }
    else
    {
        //cout << "FRAGMENT SHADER HITTTTTTTTTTTT " << endl;

        //printf("Cached Fragment Shader :\n");
        //fragmentProgram->printASM(cout);
        //printf("----------------------\n");

        // Synchronize ACDXFixedPipelineState states to ACDXGLState
        ACDXFixedPipelineStateImp* fpStateImp = static_cast<ACDXFixedPipelineStateImp*>(fpState);
        fpStateImp->sync();

        ACDXConstantBindingList::iterator iter = finalBindingList->begin();

        while (iter != finalBindingList->end())
        {
            ACDXFloatVector4 constant;
            
            // Resolve the constant
            (*iter)->resolveConstant(fpStateImp, constant);

            // Put the constant value in the ACDShaderProgram object
            fragmentProgram->setConstant((*iter)->getConstantIndex(), constant.c_array());

            iter++;
        }

    }
}

static acdlib::ACDX_COMPILATION_LOG lastResults;


ACDXCompiledProgram* acdlib::ACDXCompileProgram(const std::string& code)
{
    GLOBALPROFILER_ENTERREGION("ACDXCompileProgram", "", "")
    ACDXCompiledProgramImp* cProgramImp = new ACDXCompiledProgramImp;

    // Initialize compilers for the first time (using a static pointer)
    static ACDXProgramExecutionEnvironment* vpee = new ACDXVP1ExecEnvironment;
    static ACDXProgramExecutionEnvironment* fpee = new ACDXFP1ExecEnvironment;

    u32bit size_binary;
    ACDXRBank<float> clusterBank(250,"clusterBank");

    if (!code.compare( 0, 10, "!!ARBvp1.0" ))
    {    
        // It´s an ARB vertex program 1.0
        
        // Compile
        ResourceUsage resourceUsage;

        const acd_ubyte* compiledCode = (const acd_ubyte*)vpee->compile(code, size_binary, clusterBank, resourceUsage);
        
        // Put the code and cluster bank in the ACDXCompiledProgram
        cProgramImp->setCode(compiledCode, size_binary);
        cProgramImp->setCompiledConstantBank(clusterBank);

        for(int tu = 0; tu < MAX_TEXTURE_UNITS_ARB; tu++)
            cProgramImp->setTextureUnitsUsage(tu, resourceUsage.textureUnitUsage[tu]);

        cProgramImp->setKillInstructions(resourceUsage.killInstructions);

        // Update last results structure
        lastResults.vpSource = code;
        lastResults.vpNumInstructions = resourceUsage.numberOfInstructions;
        lastResults.vpParameterRegisters = resourceUsage.numberOfParamRegs;

        delete[] compiledCode;
    }
    else if (!code.compare( 0, 10, "!!ARBfp1.0" ))
    {    
        // It´s an ARB fragment program 1.0

        // Compile
        ResourceUsage resourceUsage;

        const acd_ubyte* compiledCode = (const acd_ubyte*)fpee->compile(code, size_binary, clusterBank, resourceUsage);
        
        // Put the code and cluster bank in the ACDXCompiledProgram
        cProgramImp->setCode(compiledCode, size_binary);
        cProgramImp->setCompiledConstantBank(clusterBank);

        for(int tu = 0; tu < MAX_TEXTURE_UNITS_ARB; tu++)
            cProgramImp->setTextureUnitsUsage(tu, resourceUsage.textureUnitUsage[tu]);

        cProgramImp->setKillInstructions(resourceUsage.killInstructions);


        // Update last results structure
        lastResults.fpSource = code;
        lastResults.fpNumInstructions = resourceUsage.numberOfInstructions;
        lastResults.fpParameterRegisters = resourceUsage.numberOfParamRegs;

        delete[] compiledCode;
    }
    else
        panic("ACDX","ACDXCompileProgram","Unexpected or not supported shader program model");
    
    GLOBALPROFILER_EXITREGION()
    return cProgramImp;
}

/* resolves the compiled program updating a ACDShaderProgram code and constant parameters accordingly to the the Fixed Pipeline state. */
ACDXConstantBindingList* acdlib::ACDXResolveProgram(ACDXFixedPipelineState* fpState, const ACDXCompiledProgram* cProgram, const ACDXConstantBindingList* constantList, ACDShaderProgram* program)
{
    GLOBALPROFILER_ENTERREGION("ACDXResolveProgram", "", "")
    /* Get the pointer to the ACDXFixedPipelineState implementation class */
    ACDXFixedPipelineStateImp* fpStateImp = static_cast<ACDXFixedPipelineStateImp*>(fpState);

    /* Get the pointer to the ACDXCompiledProgram implementation class */
    const ACDXCompiledProgramImp* cProgramImp = static_cast<const ACDXCompiledProgramImp*>(cProgram);

    /* Copy the already compiled code */
    program->setCode(cProgramImp->getCode(), cProgramImp->getCodeSize());

    ACDXRBank<float> clusterBank(200,"clusterBank");

    clusterBank = cProgramImp->getCompiledConstantBank();

    for(acd_uint tu = 0; tu < 16 /*TEX_UNITS*/; tu++)
        program->setTextureUnitsUsage(tu,cProgramImp->getTextureUnitsUsage(tu));

    program->setKillInstructions(cProgramImp->getKillInstructions());

    // Merge both the input program resolve constant binding list with the
    // compiler generated constant binding list

    ClusterBankAdapter cbAdapter(&clusterBank);

    ACDXConstantBindingList* finalConstantList = cbAdapter.getFinalConstantBindings(constantList);

    //////////////////////////////////////
    // Program constant resolving phase //
    //////////////////////////////////////

    // Synchronize ACDXFixedPipelineState states to ACDXGLState
    fpStateImp->sync();

    ACDXConstantBindingList::iterator iter = finalConstantList->begin();

    while (iter != finalConstantList->end())
    {
        ACDXFloatVector4 constant;
        
        // Resolve the constant
        (*iter)->resolveConstant(fpStateImp, constant);

        // Put the constant value in the ACDShaderProgram object
        program->setConstant((*iter)->getConstantIndex(), constant.c_array());
        
        // Delete InternalConstantBinding object TO DO: CAUTION: Not all the cb are icb´s
        //InternalConstantBinding* icb = static_cast<InternalConstantBinding*>(*iter);

        //delete icb;

        iter++;
    }
    
    //finalConstantList->clear();
    //delete finalConstantList;
    
    GLOBALPROFILER_EXITREGION()

    return finalConstantList;
}

void acdlib::ACDXDestroyCompiledProgram(ACDXCompiledProgram* cProgram)
{
    /* Get the pointer to the ACDXCompiledProgram implementation class */
    const ACDXCompiledProgramImp* cProgramImp = static_cast<const ACDXCompiledProgramImp*>(cProgram);

    delete cProgramImp;
}

void acdlib::ACDXGetCompilationLog(ACDX_COMPILATION_LOG& compileLog)
{
    compileLog = lastResults;
}
