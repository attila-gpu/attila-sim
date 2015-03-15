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

#include "ACDXFixedPipelineStateImp.h"
#include "ACDXStoredFPStateImp.h"
#include "StateItemUtils.h"
#include <sstream>
#include <fstream>

using namespace acdlib;

/////////////////////////////////////////////////////
// ACDXFixedPipelineState interface implementation //
/////////////////////////////////////////////////////

ACDXTransformAndLightingStage& ACDXFixedPipelineStateImp::tl()
{
    return *this;
}

ACDXTextCoordGenerationStage& ACDXFixedPipelineStateImp::txtcoord()
{
    return *this;
}

ACDXFragmentShadingStage& ACDXFixedPipelineStateImp::fshade()
{
    return *this;
}

ACDXPostShadingStage& ACDXFixedPipelineStateImp::postshade()
{
    return *this;
}

ACDXStoredFPState* ACDXFixedPipelineStateImp::saveState(ACDXStoredFPItemIDList siIds) const
{
    ACDXStoredFPStateImp* ret = new ACDXStoredFPStateImp();

    ACDXStoredFPItemIDList::const_iterator iter = siIds.begin();
    
    while( iter != siIds.end() )
    {
        ret->addStoredStateItem(createStoredStateItem((*iter)));        
        iter++;
    }

    return ret;
}

ACDXStoredFPState* ACDXFixedPipelineStateImp::saveState(ACDX_STORED_FP_ITEM_ID stateId) const
{
    ACDXStoredFPStateImp* ret = new ACDXStoredFPStateImp();

    ret->addStoredStateItem(createStoredStateItem(stateId));        

    return ret;
}

ACDXStoredFPState* ACDXFixedPipelineStateImp::saveAllState() const
{
    ACDXStoredFPStateImp* ret = new ACDXStoredFPStateImp();

    for (acd_uint i=0; i < ACDX_LAST_STATE; i++)
    {
        ret->addStoredStateItem(createStoredStateItem(ACDX_STORED_FP_ITEM_ID(i)));        
    }

    return ret;
}

void ACDXFixedPipelineStateImp::restoreState(const ACDXStoredFPState* state)
{
    const ACDXStoredFPStateImp* sFPsi = static_cast<const ACDXStoredFPStateImp*>(state);

    std::list<const StoredStateItem*> ssiList = sFPsi->getSSIList();

    std::list<const StoredStateItem*>::const_iterator iter = ssiList.begin();
    
    while ( iter != ssiList.end() )
    {
        restoreStoredStateItem((*iter));
        iter++;
    }
}

void ACDXFixedPipelineStateImp::destroyState(ACDXStoredFPState* state) const
{
    const ACDXStoredFPStateImp* sFPsi = static_cast<const ACDXStoredFPStateImp*>(state);

    std::list<const StoredStateItem*> stateList = sFPsi->getSSIList();

    std::list<const StoredStateItem*>::iterator iter = stateList.begin();

    while ( iter != stateList.end() )
    {
        delete (*iter);
        iter++;
    }

    delete sFPsi;
}

////////////////////////////////////////////////////////////
// ACDXTransformAndLightingStage interface implementation //
////////////////////////////////////////////////////////////

ACDXLight& ACDXFixedPipelineStateImp::light(acd_uint light)
{
    if (light >= ACDX_FP_MAX_LIGHTS_LIMIT)
        panic("ACDXFixedPipelineStateImp","light","Unexpected light number");

    return *(_light[light]);
}

ACDXMaterial& ACDXFixedPipelineStateImp::material(ACDX_FACE face)
{
    switch(face)
    {
        case ACDX_FRONT:
            return (*_material[0]);
        case ACDX_BACK:
            return (*_material[1]);
        case ACDX_FRONT_AND_BACK: // Can only ask for a single face material state
        default:
            panic("ACDXFixedPipelineStateImp","material","Unexpected material face");
            return (*_material[0]); // to avoid stupid compiler warnings

    }
}

void ACDXFixedPipelineStateImp::setLightModelAmbientColor(acd_float r, acd_float g, acd_float b, acd_float a)
{    
    ACDXFloatVector4 color;
    color[0] = r; color[1] = g; color[2] = b; color[3] = a;

    _lightModelAmbientColor = color;
}

void ACDXFixedPipelineStateImp::setLightModelSceneColor(ACDX_FACE face, acd_float r, acd_float g, acd_float b, acd_float a)
{
    ACDXFloatVector4 color;
    color[0] = r; color[1] = g; color[2] = b; color[3] = a;

    switch(face)
    {
        case ACDX_FRONT: 
        case ACDX_BACK:
        case ACDX_FRONT_AND_BACK: // Setting the two faces at the same time is allowed
            break;
        default:
            panic("ACDXFixedPipelineStateImp","setLightModelSceneColor","Unexpected material face");
    }

    if (face == ACDX_FRONT || face == ACDX_FRONT_AND_BACK)
    {
        _lightModelSceneFrontColor = color;    
    }

    if (face == ACDX_BACK || face == ACDX_FRONT_AND_BACK)
    {
        _lightModelSceneBackColor = color;    
    }
}

void ACDXFixedPipelineStateImp::setModelviewMatrix(acd_uint unit, const ACDXFloatMatrix4x4& matrix)
{
    if (unit >= ACDX_FP_MAX_MODELVIEW_MATRICES_LIMIT)
        panic("ACDXFixedPipelineStateImp","setModelviewMatrix","Unexpected modelview matrix number");

    _modelViewMatrix[unit] = matrix;
}

ACDXFloatMatrix4x4 ACDXFixedPipelineStateImp::getModelviewMatrix(acd_uint unit) const
{
    if ( unit >= ACDX_FP_MAX_MODELVIEW_MATRICES_LIMIT )
        panic("ACDXFixedPipelineStateImp","getModelviewMatrix","Unexpected modelview matrix number");

    return _modelViewMatrix[unit];
}

void ACDXFixedPipelineStateImp::setProjectionMatrix(const ACDXFloatMatrix4x4& matrix)
{
    _projectionMatrix = matrix;
}

ACDXFloatMatrix4x4 ACDXFixedPipelineStateImp::getProjectionMatrix() const
{
    return _projectionMatrix;
}

///////////////////////////////////////////////////////////
// ACDXTextCoordGenerationStage interface implementation //
///////////////////////////////////////////////////////////

void ACDXFixedPipelineStateImp::setTextureCoordPlane(acd_uint textureStage, ACDX_TEXTURE_COORD_PLANE plane, ACDX_TEXTURE_COORD coord, acd_float a, acd_float b, acd_float c, acd_float d)
{
    if (textureStage >= ACDX_FP_MAX_TEXTURE_STAGES_LIMIT)
        panic("ACDXFixedPipelineStateImp","setTextureCoordPlane","Unexpected texture stage number");

    switch(plane)
    {
        case ACDX_OBJECT_PLANE: 
        case ACDX_EYE_PLANE:
            break;
        default:
            panic("ACDXFixedPipelineStateImp","setTextureCoordPlane","Unexpected plane");
    }

    switch(coord)
    {
        case ACDX_COORD_S: 
        case ACDX_COORD_T:
        case ACDX_COORD_R:
        case ACDX_COORD_Q:
            break;
        default:
            panic("ACDXFixedPipelineStateImp","setTextureCoordPlane","Unexpected coordinate");
    }

    ACDXFloatVector4 planev;
    planev[0] = a; planev[1] = b; planev[2] = c; planev[3] = d;

    if ( coord == ACDX_COORD_S )
    {
        if ( plane == ACDX_OBJECT_PLANE )
            _textCoordSObjectPlane[textureStage] = planev;
        else // plane == ACDX_EYE_PLANE
            _textCoordSEyePlane[textureStage] = planev;
    }
    else if ( coord == ACDX_COORD_T )
    {
        if ( plane == ACDX_OBJECT_PLANE )
            _textCoordTObjectPlane[textureStage] = planev;
        else // plane == ACDX_EYE_PLANE
            _textCoordTEyePlane[textureStage] = planev;
    }
    else if ( coord == ACDX_COORD_R )
    {
        if ( plane == ACDX_OBJECT_PLANE )
            _textCoordRObjectPlane[textureStage] = planev;
        else // plane == ACDX_EYE_PLANE
            _textCoordREyePlane[textureStage] = planev;
    }
    else if ( coord == ACDX_COORD_Q )
    {
        if ( plane == ACDX_OBJECT_PLANE )
            _textCoordQObjectPlane[textureStage] = planev;
        else // plane == ACDX_EYE_PLANE
            _textCoordQEyePlane[textureStage] = planev;
    }
}

void ACDXFixedPipelineStateImp::setTextureCoordMatrix(acd_uint textureStage, const ACDXFloatMatrix4x4& matrix)
{
    _textureCoordMatrix[textureStage] = matrix;
}

ACDXFloatMatrix4x4 ACDXFixedPipelineStateImp::getTextureCoordMatrix(acd_uint textureStage) const
{
    return _textureCoordMatrix[textureStage];
}

///////////////////////////////////////////////////////
// ACDXFragmentShadingStage interface implementation //
///////////////////////////////////////////////////////

void ACDXFixedPipelineStateImp::setTextureEnvColor(acd_uint textureStage, acd_float r, acd_float g, acd_float b, acd_float a)
{
    if (textureStage >= ACDX_FP_MAX_TEXTURE_STAGES_LIMIT)
        panic("ACDXFixedPipelineStateImp","setTextureEnvColor","Unexpected texture stage number");

    ACDXFloatVector4 color;
    color[0] = r; color[1] = g; color[2] = b; color[3] = a;

    _textEnvColor[textureStage] = color;
}

void ACDXFixedPipelineStateImp::setDepthRange(acd_float nearValue, acd_float farValue)
{
    _near = nearValue;
    _far = farValue;
}

///////////////////////////////////////////////////
// ACDXPostShadingStage interface implementation //
///////////////////////////////////////////////////

void ACDXFixedPipelineStateImp::setFOGBlendColor(acd_float r, acd_float g, acd_float b, acd_float a)
{
    ACDXFloatVector4 color;
    color[0] = r; color[1] = g; color[2] = b; color[3] = a;

    _fogBlendColor = color;
}

void ACDXFixedPipelineStateImp::setFOGDensity(acd_float exponent)
{
    _fogDensity = exponent;
}

void ACDXFixedPipelineStateImp::setFOGLinearDistances(acd_float start, acd_float end)
{
    _fogLinearStart = start;
    _fogLinearEnd = end;
}

void ACDXFixedPipelineStateImp::setAlphaTestRefValue(acd_float refValue)
{
    _alphaTestRefValue = refValue;
}

//////////////////////////////////////////////
// ACDXFixedPipelineStateImp implementation //
//////////////////////////////////////////////

ACDXFixedPipelineStateImp::ACDXFixedPipelineStateImp(ACDXGLState *gls)
:   
    _gls(gls),
    _requiredSync(false),

    _light(ACDX_FP_MAX_LIGHTS_LIMIT),
    _material(2),

    // Material state initialization

    _materialFrontAmbient(getVectorState4(ACDX_MATERIAL_FRONT_AMBIENT)),
    _materialFrontDiffuse(getVectorState4(ACDX_MATERIAL_FRONT_DIFFUSE)),
    _materialFrontSpecular(getVectorState4(ACDX_MATERIAL_FRONT_SPECULAR)),
    _materialFrontEmission(getVectorState4(ACDX_MATERIAL_FRONT_EMISSION)),
    _materialFrontShininess(getSingleState(ACDX_MATERIAL_FRONT_SHININESS)),
        
    _materialBackAmbient(getVectorState4(ACDX_MATERIAL_BACK_AMBIENT)),
    _materialBackDiffuse(getVectorState4(ACDX_MATERIAL_BACK_DIFFUSE)),
    _materialBackSpecular(getVectorState4(ACDX_MATERIAL_BACK_SPECULAR)),
    _materialBackEmission(getVectorState4(ACDX_MATERIAL_BACK_EMISSION)),
    _materialBackShininess(getSingleState(ACDX_MATERIAL_BACK_SHININESS)),

    // Light state initialization

    _lightAmbient(ACDX_FP_MAX_LIGHTS_LIMIT, getVectorState4(ACDX_LIGHT_AMBIENT)),
    _lightDiffuse(ACDX_FP_MAX_LIGHTS_LIMIT, getVectorState4(ACDX_LIGHT_DIFFUSE)),
    _lightSpecular(ACDX_FP_MAX_LIGHTS_LIMIT, getVectorState4(ACDX_LIGHT_SPECULAR)),
    _lightPosition(ACDX_FP_MAX_LIGHTS_LIMIT, getVectorState4(ACDX_LIGHT_POSITION)),
    _lightDirection(ACDX_FP_MAX_LIGHTS_LIMIT, getVectorState3(ACDX_LIGHT_DIRECTION)),
    _lightAttenuation(ACDX_FP_MAX_LIGHTS_LIMIT, getVectorState3(ACDX_LIGHT_ATTENUATION)),
    _lightSpotDirection(ACDX_FP_MAX_LIGHTS_LIMIT, getVectorState3(ACDX_LIGHT_SPOT_DIRECTION)),
    _lightSpotCutOffAngle(ACDX_FP_MAX_LIGHTS_LIMIT, getSingleState(ACDX_LIGHT_SPOT_CUTOFF)),
    _lightSpotExponent(ACDX_FP_MAX_LIGHTS_LIMIT, getSingleState(ACDX_LIGHT_SPOT_EXPONENT)),

    // Light model state initialization

    _lightModelAmbientColor(getVectorState4(ACDX_LIGHTMODEL_AMBIENT)),
    _lightModelSceneFrontColor(getVectorState4(ACDX_LIGHTMODEL_FRONT_SCENECOLOR)),
    _lightModelSceneBackColor(getVectorState4(ACDX_LIGHTMODEL_BACK_SCENECOLOR)),

    // Transformation matrices initialization

    _modelViewMatrix(ACDX_FP_MAX_MODELVIEW_MATRICES_LIMIT, getMatrixState(ACDX_M_MODELVIEW)),
    _projectionMatrix(getMatrixState(ACDX_M_PROJECTION)),

    // Texture coordinate generation planes initialization

    _textCoordSObjectPlane(ACDX_FP_MAX_TEXTURE_STAGES_LIMIT, getVectorState4(ACDX_TEXGEN_OBJECT_S)),
    _textCoordTObjectPlane(ACDX_FP_MAX_TEXTURE_STAGES_LIMIT, getVectorState4(ACDX_TEXGEN_OBJECT_T)),
    _textCoordRObjectPlane(ACDX_FP_MAX_TEXTURE_STAGES_LIMIT, getVectorState4(ACDX_TEXGEN_OBJECT_R)),
    _textCoordQObjectPlane(ACDX_FP_MAX_TEXTURE_STAGES_LIMIT, getVectorState4(ACDX_TEXGEN_OBJECT_Q)),

    _textCoordSEyePlane(ACDX_FP_MAX_TEXTURE_STAGES_LIMIT, getVectorState4(ACDX_TEXGEN_EYE_S)),
    _textCoordTEyePlane(ACDX_FP_MAX_TEXTURE_STAGES_LIMIT, getVectorState4(ACDX_TEXGEN_EYE_T)),
    _textCoordREyePlane(ACDX_FP_MAX_TEXTURE_STAGES_LIMIT, getVectorState4(ACDX_TEXGEN_EYE_R)),
    _textCoordQEyePlane(ACDX_FP_MAX_TEXTURE_STAGES_LIMIT, getVectorState4(ACDX_TEXGEN_EYE_Q)),

    // Texture coordinate matrices

    _textureCoordMatrix(ACDX_FP_MAX_TEXTURE_STAGES_LIMIT, getMatrixState(ACDX_M_TEXTURE)),

    // Texture stage environment colors

    _textEnvColor(ACDX_FP_MAX_TEXTURE_STAGES_LIMIT, getVectorState4(ACDX_TEXENV_COLOR)),

    // Depth range

    _near(acd_float(0)),    // This state is not located in ACDXGLState
    _far(acd_float(1)),        // This state is not located in ACDXGLState

    // FOG params

    _fogBlendColor(getVectorState4(ACDX_FOG_COLOR)),
    _fogDensity(getSingleState(ACDX_FOG_DENSITY)),
    _fogLinearStart(getSingleState(ACDX_FOG_LINEAR_START)),
    _fogLinearEnd(getSingleState(ACDX_FOG_LINEAR_END)),

    // Alpha test params

    _alphaTestRefValue(acd_float(0)) // This state is not located in ACDXGLState
{

    _fogBlendColor = getVectorState4(ACDX_FOG_COLOR);

    if (!gls)
        panic("ACDXFixedPipelineStateImp","constructor","ACDXGLState null pointer is not allowed");

    for (acd_uint i = 0; i < ACDX_FP_MAX_LIGHTS_LIMIT; i++)
        _light[i] = new ACDXLightImp(i, this);

    for (acd_uint i = 1; i < ACDX_FP_MAX_LIGHTS_LIMIT; i++){
       _lightDiffuse[i] = getVectorState4((ACDX_STORED_FP_ITEM_ID)(ACDX_LIGHT_DIFFUSE+9));
       _lightDiffuse[i].restart();
    }

    for (acd_uint i = 1; i < ACDX_FP_MAX_LIGHTS_LIMIT; i++){
        _lightSpecular[i] = getVectorState4((ACDX_STORED_FP_ITEM_ID)(ACDX_LIGHT_SPECULAR+9));
       _lightSpecular[i].restart();
    }

    _material[0] = new ACDXMaterialImp(0, this);
    _material[1] = new ACDXMaterialImp(1, this);
}

ACDXFixedPipelineStateImp::~ACDXFixedPipelineStateImp()
{
    for (acd_uint i = 0; i < ACDX_FP_MAX_LIGHTS_LIMIT; i++)
        delete _light[i];

    delete _material[0];
    delete _material[1];
}

const ACDXGLState* ACDXFixedPipelineStateImp::getGLState() const
{
    return _gls;
}

acd_float ACDXFixedPipelineStateImp::getSingleState(ACDX_STORED_FP_ITEM_ID stateId) const
{
    acd_float ret = 1.0f;
    VectorId vId;
    MatrixId mId;
    acd_uint componentMask;
    acd_uint unit;
    MatrixType type;
    acd_bool isMatrix;

    switch(stateId)
    {
        // Put here the code for resolution of special Fixed Pipeline States
        // not present in ACDXGLState
        case ACDX_ALPHA_TEST_REF_VALUE:
            return _alphaTestRefValue;
    }

    getGLStateId(stateId, vId, componentMask, mId, unit, type, isMatrix);

    if (isMatrix)
        panic("ACDXFixedPipelineStateImp","getSingleState","Requesting a Matrix State. Use getMatrixState() instead");

    switch(componentMask)
    {
        case 0x08: ret = _gls->getVector(vId)[0]; break;
        case 0x04: ret = _gls->getVector(vId)[1]; break;
        case 0x02: ret = _gls->getVector(vId)[2]; break;
        case 0x01: ret = _gls->getVector(vId)[3]; break;
        default:
            panic("ACDXFixedPipelineStateImp","getSingleState","Not a single State. Use getVectorState4() or getVectorState3() instead");
    }

    return ret;
}

ACDXFloatVector3 ACDXFixedPipelineStateImp::getVectorState3(ACDX_STORED_FP_ITEM_ID stateId) const
{
    ACDXFloatVector3 ret(acd_float(1.0f));
    VectorId vId;
    MatrixId mId;
    acd_uint componentMask;
    acd_uint unit;
    MatrixType type;
    acd_bool isMatrix;

    getGLStateId(stateId, vId, componentMask, mId, unit, type, isMatrix);

    if (isMatrix)
        panic("ACDXFixedPipelineStateImp","getVectorState3","Requesting a Matrix State. Use getMatrixState() instead");

    switch(componentMask)
    {
        case 0x07: ret[0] = _gls->getVector(vId)[1]; 
                   ret[1] = _gls->getVector(vId)[2]; 
                   ret[2] = _gls->getVector(vId)[3]; 
                   break;
        case 0x0B: ret[0] = _gls->getVector(vId)[0]; 
                   ret[1] = _gls->getVector(vId)[2]; 
                   ret[2] = _gls->getVector(vId)[3]; 
                   break;
        case 0x0D: ret[0] = _gls->getVector(vId)[0]; 
                   ret[1] = _gls->getVector(vId)[1]; 
                   ret[2] = _gls->getVector(vId)[3]; 
                   break;
        case 0x0E: ret[0] = _gls->getVector(vId)[0]; 
                   ret[1] = _gls->getVector(vId)[1]; 
                   ret[2] = _gls->getVector(vId)[2]; 
                   break;
        default:
            panic("ACDXFixedPipelineStateImp","getVectorState3","Not a vector3 State. Use getSingleState() or getVectorState4() instead");

    }
    
    return ret;
}

ACDXFloatVector4 ACDXFixedPipelineStateImp::getVectorState4(ACDX_STORED_FP_ITEM_ID stateId) const
{
    ACDXFloatVector4 ret(acd_float(1.0f));
    VectorId vId;
    MatrixId mId;
    acd_uint componentMask;
    acd_uint unit;
    MatrixType type;
    acd_bool isMatrix;

    getGLStateId(stateId, vId, componentMask, mId, unit, type, isMatrix);

    if (isMatrix)
        panic("ACDXFixedPipelineStateImp","getVectorState4","Requesting a Matrix State. Use getMatrixState() instead");

    if (componentMask != 0x0F)
        panic("ACDXFixedPipelineStateImp","getVectorState4","Not a vector4 State. Use getSingleState() or getVectorState3() instead");

    ret[0] = _gls->getVector(vId)[0];
    ret[1] = _gls->getVector(vId)[1];
    ret[2] = _gls->getVector(vId)[2];
    ret[3] = _gls->getVector(vId)[3];

    return ret;
}
    
ACDXFloatMatrix4x4 ACDXFixedPipelineStateImp::getMatrixState(ACDX_STORED_FP_ITEM_ID stateId) const
{
    ACDXFloatMatrix4x4 ret(acd_float(0));
    VectorId vId;
    MatrixId mId;
    acd_uint componentMask;
    acd_uint unit;
    MatrixType type;
    acd_bool isMatrix;

    getGLStateId(stateId, vId, componentMask, mId, unit, type, isMatrix);

    if (!isMatrix)
        panic("ACDXFixedPipelineStateImp","getMatrixState","Requesting a Vector State. Use getSingleState(), getVectorState3() or getVectorState4()");

    const ACDXMatrixf& mat = _gls->getMatrix(mId, type, unit);

    for (int i=0; i<4; i++)
        for (int j=0; j<4; j++)
            ret[i][j] = mat.getRows()[i*4 + j];

    return ret;
}

void ACDXFixedPipelineStateImp::setSingleState(ACDX_STORED_FP_ITEM_ID stateId, acd_float value)
{
    VectorId vId;
    MatrixId mId;
    acd_uint componentMask;
    acd_uint unit;
    MatrixType type;
    acd_bool isMatrix;

    switch(stateId)
    {
        // Put here the code for resolution of special Fixed Pipeline States
        // not present in ACDXGLState
        case ACDX_ALPHA_TEST_REF_VALUE:
            _alphaTestRefValue = value;
            return;
    }

    getGLStateId(stateId, vId, componentMask, mId, unit, type, isMatrix);

    if (isMatrix)
        panic("ACDXFixedPipelineStateImp","setSingleState","Setting a Matrix State. Use getMatrixState() instead");

    Quadf glsvalue = _gls->getVector(vId);

    switch(componentMask)
    {
        case 0x08:  glsvalue[0] = value; break;
        case 0x04:  glsvalue[1] = value; break;
        case 0x02:  glsvalue[2] = value; break;
        case 0x01:  glsvalue[3] = value; break;
        default:
            panic("ACDXFixedPipelineStateImp","setSingleState","Not a single State. Use setVectorState4() or setVectorState3() instead");
    }

    _gls->setVector(glsvalue, vId);
}

void ACDXFixedPipelineStateImp::setVectorState3(ACDX_STORED_FP_ITEM_ID stateId, ACDXFloatVector3 value)
{
    VectorId vId;
    MatrixId mId;
    acd_uint componentMask;
    acd_uint unit;
    MatrixType type;
    acd_bool isMatrix;

    getGLStateId(stateId, vId, componentMask, mId, unit, type, isMatrix);

    if (isMatrix)
        panic("ACDXFixedPipelineStateImp","setVectorState3","Setting a Matrix State. Use setMatrixState() instead");

    Quadf glsvalue = _gls->getVector(vId);

    switch(componentMask)
    {
        case 0x07: glsvalue[1] = value[0]; 
                   glsvalue[2] = value[1]; 
                   glsvalue[3] = value[2]; 
                   break;
        case 0x0B: glsvalue[0] = value[0]; 
                   glsvalue[2] = value[1]; 
                   glsvalue[3] = value[2]; 
                   break;
        case 0x0D: glsvalue[0] = value[0]; 
                   glsvalue[1] = value[1]; 
                   glsvalue[3] = value[2]; 
                   break;
        case 0x0E: glsvalue[0] = value[0]; 
                   glsvalue[1] = value[1]; 
                   glsvalue[2] = value[2]; 
                   break;
        default:
            panic("ACDXFixedPipelineStateImp","setVectorState3","Not a vector3 State. Use setSingleState() or setVectorState4() instead");

    }
    
    _gls->setVector(glsvalue, vId);
}

void ACDXFixedPipelineStateImp::setVectorState4(ACDX_STORED_FP_ITEM_ID stateId, ACDXFloatVector4 value)
{
    VectorId vId;
    MatrixId mId;
    acd_uint componentMask;
    acd_uint unit;
    MatrixType type;
    acd_bool isMatrix;

    getGLStateId(stateId, vId, componentMask, mId, unit, type, isMatrix);

    if (isMatrix)
        panic("ACDXFixedPipelineStateImp","setVectorState4","Setting a Matrix State. Use setMatrixState() instead");

    if (componentMask != 0x0F)
        panic("ACDXFixedPipelineStateImp","setVectorState4","Not a vector4 State. Use setSingleState() or setVectorState3() instead");

    Quadf glsvalue = _gls->getVector(vId);

    glsvalue[0] = value[0];
    glsvalue[1] = value[1];
    glsvalue[2] = value[2];
    glsvalue[3] = value[3];

    _gls->setVector(glsvalue, vId);
}

void ACDXFixedPipelineStateImp::setMatrixState(ACDX_STORED_FP_ITEM_ID stateId, ACDXFloatMatrix4x4 value)
{
    VectorId vId;
    MatrixId mId;
    acd_uint componentMask;
    acd_uint unit;
    MatrixType type;
    acd_bool isMatrix;

    getGLStateId(stateId, vId, componentMask, mId, unit, type, isMatrix);

    if (!isMatrix)
        panic("ACDXFixedPipelineStateImp","setMatrixState","Setting a Vector State. Use setSingleState(), setVectorState3() or setVectorState4()");

    const ACDXMatrixf& glsvalue = _gls->getMatrix(mId, type, unit);

    for (int i=0; i<4; i++)
        for (int j=0; j<4; j++)
            glsvalue.getRows()[i*4 + j] = value[i][j];
}

void ACDXFixedPipelineStateImp::sync()
{
    if ( _materialFrontAmbient.changed() || _requiredSync )
    {
        _materialFrontAmbient.restart();
        setVectorState4(ACDX_MATERIAL_FRONT_AMBIENT, _materialFrontAmbient);
    }

    if ( _materialFrontDiffuse.changed() || _requiredSync )
    {
        _materialFrontDiffuse.restart();
        setVectorState4(ACDX_MATERIAL_FRONT_DIFFUSE, _materialFrontDiffuse);
    }

    if ( _materialFrontSpecular.changed() || _requiredSync )
    {
        _materialFrontSpecular.restart();
        setVectorState4(ACDX_MATERIAL_FRONT_SPECULAR, _materialFrontSpecular);
    }

    if ( _materialFrontEmission.changed() || _requiredSync )
    {
        _materialFrontEmission.restart();
        setVectorState4(ACDX_MATERIAL_FRONT_EMISSION, _materialFrontEmission);
    }

    if ( _materialFrontShininess.changed() || _requiredSync )
    {
        _materialFrontShininess.restart();
        setSingleState(ACDX_MATERIAL_FRONT_SHININESS, _materialFrontShininess);
    }

    if ( _materialBackAmbient.changed() || _requiredSync )
    {
        _materialBackAmbient.restart();
        setVectorState4(ACDX_MATERIAL_BACK_AMBIENT, _materialBackAmbient);
    }

    if ( _materialBackDiffuse.changed() || _requiredSync )
    {
        _materialBackDiffuse.restart();
        setVectorState4(ACDX_MATERIAL_BACK_DIFFUSE, _materialBackDiffuse);
    }

    if ( _materialBackSpecular.changed() || _requiredSync )
    {
        _materialBackSpecular.restart();
        setVectorState4(ACDX_MATERIAL_BACK_DIFFUSE, _materialBackDiffuse);
    }

    if ( _materialBackEmission.changed() || _requiredSync )
    {
        _materialBackEmission.restart();
        setVectorState4(ACDX_MATERIAL_BACK_EMISSION, _materialBackDiffuse);
    
    }

    if ( _materialBackShininess.changed() || _requiredSync )
    {
        _materialBackShininess.restart();
        setSingleState(ACDX_MATERIAL_BACK_SHININESS, _materialBackShininess);
    }

    for (acd_uint i=0; i < ACDX_FP_MAX_LIGHTS_LIMIT; i++)
    {
        if ( _lightAmbient[i].changed() || _requiredSync )
        {
            _lightAmbient[i].restart();
            setVectorState4(ACDX_STORED_FP_ITEM_ID(ACDX_LIGHT_AMBIENT + ACDX_LIGHT_PROPERTIES_COUNT * i), _lightAmbient[i]);
        }

        if ( _lightDiffuse[i].changed() || _requiredSync )
        {
            _lightDiffuse[i].restart();
            setVectorState4(ACDX_STORED_FP_ITEM_ID(ACDX_LIGHT_DIFFUSE + ACDX_LIGHT_PROPERTIES_COUNT * i), _lightDiffuse[i]);
        }

        if ( _lightSpecular[i].changed() || _requiredSync )
        {
            _lightSpecular[i].restart();
            setVectorState4(ACDX_STORED_FP_ITEM_ID(ACDX_LIGHT_SPECULAR + ACDX_LIGHT_PROPERTIES_COUNT * i), _lightSpecular[i]);
        }
        
        if ( _lightPosition[i].changed() || _requiredSync )
        {
            _lightPosition[i].restart();
            setVectorState4(ACDX_STORED_FP_ITEM_ID(ACDX_LIGHT_POSITION + ACDX_LIGHT_PROPERTIES_COUNT * i), _lightPosition[i]);
        }
        
        if ( _lightDirection[i].changed() || _requiredSync )
        {
            _lightDirection[i].restart();
            setVectorState3(ACDX_STORED_FP_ITEM_ID(ACDX_LIGHT_DIRECTION + ACDX_LIGHT_PROPERTIES_COUNT * i), _lightDirection[i]);
        }
        
        if ( _lightAttenuation[i].changed() || _requiredSync )
        {
            _lightAttenuation[i].restart();
            setVectorState3(ACDX_STORED_FP_ITEM_ID(ACDX_LIGHT_ATTENUATION + ACDX_LIGHT_PROPERTIES_COUNT * i), _lightAttenuation[i]);
        }
        
        if ( _lightSpotDirection[i].changed() || _requiredSync )
        {
            _lightSpotDirection[i].restart();
            setVectorState3(ACDX_STORED_FP_ITEM_ID(ACDX_LIGHT_SPOT_DIRECTION + ACDX_LIGHT_PROPERTIES_COUNT * i), _lightSpotDirection[i]);
        }
        
        if ( _lightSpotCutOffAngle[i].changed() || _requiredSync )
        {
            _lightSpotCutOffAngle[i].restart();
            setSingleState(ACDX_STORED_FP_ITEM_ID(ACDX_LIGHT_SPOT_CUTOFF + ACDX_LIGHT_PROPERTIES_COUNT * i), _lightSpotCutOffAngle[i]);
        }
        
        if ( _lightSpotExponent[i].changed() || _requiredSync )
        {
            _lightSpotExponent[i].restart();
            setSingleState(ACDX_STORED_FP_ITEM_ID(ACDX_LIGHT_SPOT_EXPONENT + ACDX_LIGHT_PROPERTIES_COUNT * i), _lightSpotExponent[i]);
        }
    }

    if (_lightModelAmbientColor.changed() || _requiredSync )
    {
        _lightModelAmbientColor.restart();
        setVectorState4(ACDX_LIGHTMODEL_AMBIENT, _lightModelAmbientColor);
    }

    if (_lightModelSceneFrontColor.changed() || _requiredSync )
    {
        _lightModelSceneFrontColor.restart();
        setVectorState4(ACDX_LIGHTMODEL_FRONT_SCENECOLOR, _lightModelSceneFrontColor);
    }

    if (_lightModelSceneBackColor.changed() || _requiredSync )
    {
        _lightModelSceneBackColor.restart();
        setVectorState4(ACDX_LIGHTMODEL_BACK_SCENECOLOR, _lightModelSceneBackColor);
    }

    for (acd_uint i=0; i < ACDX_FP_MAX_MODELVIEW_MATRICES_LIMIT ; i++)
    {
        if (_modelViewMatrix[i].changed() || _requiredSync )
        {
            _modelViewMatrix[i].restart();
            setMatrixState(ACDX_STORED_FP_ITEM_ID(ACDX_M_MODELVIEW + i), _modelViewMatrix[i]);
        }
    }

    if (_projectionMatrix.changed() || _requiredSync )
    {
        _projectionMatrix.restart();
        setMatrixState(ACDX_M_PROJECTION, _projectionMatrix);
    }


    for (acd_uint i=0; i < ACDX_FP_MAX_TEXTURE_STAGES_LIMIT ; i++)
    {
        if (_textCoordSObjectPlane[i].changed() || _requiredSync )
        {
            _textCoordSObjectPlane[i].restart();
            setVectorState4(ACDX_STORED_FP_ITEM_ID(ACDX_TEXGEN_OBJECT_S + ACDX_TEXGEN_PROPERTIES_COUNT * i), _textCoordSObjectPlane[i]);
        }

        if (_textCoordTObjectPlane[i].changed() || _requiredSync )
        {
            _textCoordTObjectPlane[i].restart();
            setVectorState4(ACDX_STORED_FP_ITEM_ID(ACDX_TEXGEN_OBJECT_T + ACDX_TEXGEN_PROPERTIES_COUNT * i), _textCoordTObjectPlane[i]);
        }

        if (_textCoordRObjectPlane[i].changed() || _requiredSync )
        {
            _textCoordRObjectPlane[i].restart();
            setVectorState4(ACDX_STORED_FP_ITEM_ID(ACDX_TEXGEN_OBJECT_R + ACDX_TEXGEN_PROPERTIES_COUNT * i), _textCoordRObjectPlane[i]);
        }

        if (_textCoordQObjectPlane[i].changed() || _requiredSync )
        {
            _textCoordQObjectPlane[i].restart();
            setVectorState4(ACDX_STORED_FP_ITEM_ID(ACDX_TEXGEN_OBJECT_Q + ACDX_TEXGEN_PROPERTIES_COUNT * i), _textCoordQObjectPlane[i]);
        }

        if (_textCoordSEyePlane[i].changed() || _requiredSync )
        {
            _textCoordSEyePlane[i].restart();
            setVectorState4(ACDX_STORED_FP_ITEM_ID(ACDX_TEXGEN_EYE_S + ACDX_TEXGEN_PROPERTIES_COUNT * i), _textCoordSEyePlane[i]);
        }

        if (_textCoordTEyePlane[i].changed() || _requiredSync )
        {
            _textCoordTEyePlane[i].restart();
            setVectorState4(ACDX_STORED_FP_ITEM_ID(ACDX_TEXGEN_EYE_T + ACDX_TEXGEN_PROPERTIES_COUNT * i), _textCoordTEyePlane[i]);
        }

        if (_textCoordREyePlane[i].changed() || _requiredSync )
        {
            _textCoordREyePlane[i].restart();
            setVectorState4(ACDX_STORED_FP_ITEM_ID(ACDX_TEXGEN_EYE_R + ACDX_TEXGEN_PROPERTIES_COUNT * i), _textCoordREyePlane[i]);
        }

        if (_textCoordQEyePlane[i].changed() || _requiredSync )
        {
            _textCoordQEyePlane[i].restart();
            setVectorState4(ACDX_STORED_FP_ITEM_ID(ACDX_TEXGEN_EYE_Q + ACDX_TEXGEN_PROPERTIES_COUNT * i), _textCoordQEyePlane[i]);
        }

        if (_textureCoordMatrix[i].changed() || _requiredSync )
        {
            _textureCoordMatrix[i].restart();
            setMatrixState(ACDX_STORED_FP_ITEM_ID(ACDX_M_TEXTURE + i),_textureCoordMatrix[i]);
        }

        if (_textEnvColor[i].changed() || _requiredSync )
        {
            _textEnvColor[i].restart();
            setVectorState4(ACDX_STORED_FP_ITEM_ID(ACDX_TEXENV_COLOR + i), _textEnvColor[i]);
        }

    }
    
    if (_near.changed() || _requiredSync )
    {
        _near.restart();
        // Do nothing. Already synchronized in ACDXFixedPipelineState
        //setSingleState(ACDX_DEPTH_RANGE_NEAR, _near);
    }

    if (_far.changed() || _requiredSync )
    {
        _far.restart();
        // Do nothing. Already synchronized in ACDXFixedPipelineState
        //setSingleState(ACDX_DEPTH_RANGE_FAR, _far);
    }

    if (_fogBlendColor.changed() || _requiredSync )
    {
        _fogBlendColor.restart();
        setVectorState4(ACDX_FOG_COLOR, _fogBlendColor);
    }

    if (_fogDensity.changed() || _requiredSync )
    {
        _fogDensity.restart();
        setSingleState(ACDX_FOG_DENSITY, _fogDensity);
    }

    if (_fogLinearStart.changed() || _requiredSync )
    {
        _fogLinearStart.restart();
        setSingleState(ACDX_FOG_LINEAR_START, _fogLinearStart);
    }

    if (_fogLinearEnd.changed() || _requiredSync )
    {
        _fogLinearEnd.restart();
        setSingleState(ACDX_FOG_LINEAR_END, _fogLinearEnd);
    }

    if (_alphaTestRefValue.changed() || _requiredSync )
    {
        _alphaTestRefValue.restart();
        // Do nothing. Already synchronized in ACDXFixedPipelineState
        //setSingleState(ACDX_ALPHA_TEST_REF_VALUE, _alphaTestRefValue);
    }
}


void ACDXFixedPipelineStateImp::forceSync()
{
    _requiredSync = true;
    sync();
    _requiredSync = false;
}

string ACDXFixedPipelineStateImp::getInternalState() const
{
    stringstream ss;
    stringstream ssAux;

    ss << stateItemString(_materialFrontAmbient,"MATERIAL_FRONT_AMBIENT", false);
    ss << stateItemString(_materialFrontDiffuse,"MATERIAL_FRONT_DIFFUSE", false);
    ss << stateItemString(_materialFrontSpecular,"MATERIAL_FRONT_SPECULAR", false);
    ss << stateItemString(_materialFrontEmission,"MATERIAL_FRONT_EMISSION", false);
    ss << stateItemString(_materialFrontShininess,"MATERIAL_FRONT_SHININESS", false);

    ss << stateItemString(_materialBackAmbient,"MATERIAL_BACK_AMBIENT", false);
    ss << stateItemString(_materialBackDiffuse,"MATERIAL_BACK_DIFFUSE", false);
    ss << stateItemString(_materialBackSpecular,"MATERIAL_BACK_SPECULAR", false);
    ss << stateItemString(_materialBackEmission,"MATERIAL_BACK_EMISSION", false);
    ss << stateItemString(_materialBackShininess,"MATERIAL_BACK_SHININESS", false);
    
    ssAux.str("");
    for (acd_uint i=0; i < ACDX_FP_MAX_LIGHTS_LIMIT; i++)
    {
        ssAux << "LIGHT" << i << "_AMBIENT_COLOR"; ss << stateItemString(_lightAmbient[i], ssAux.str().c_str(), false); ssAux.str("");
        ssAux << "LIGHT" << i << "_DIFFUSE_COLOR"; ss << stateItemString(_lightDiffuse[i], ssAux.str().c_str(), false); ssAux.str("");
        ssAux << "LIGHT" << i << "_SPECULAR_COLOR"; ss << stateItemString(_lightSpecular[i], ssAux.str().c_str(), false); ssAux.str("");
        ssAux << "LIGHT" << i << "_POSITION"; ss << stateItemString(_lightPosition[i], ssAux.str().c_str(), false); ssAux.str("");
        ssAux << "LIGHT" << i << "_DIRECTION"; ss << stateItemString(_lightDirection[i], ssAux.str().c_str(), false); ssAux.str("");
        ssAux << "LIGHT" << i << "_ATTENUATION_COEFFS"; ss << stateItemString(_lightAttenuation[i], ssAux.str().c_str(), false); ssAux.str("");
        ssAux << "LIGHT" << i << "_SPOT_DIRECTION"; ss << stateItemString(_lightSpotDirection[i], ssAux.str().c_str(), false); ssAux.str("");
        ssAux << "LIGHT" << i << "_SPOT_CUT_OFF"; ss << stateItemString(_lightSpotCutOffAngle[i], ssAux.str().c_str(), false); ssAux.str("");
        ssAux << "LIGHT" << i << "_SPOT_EXPONENT"; ss << stateItemString(_lightSpotExponent[i], ssAux.str().c_str(), false); ssAux.str("");
    }

    ss << stateItemString(_lightModelAmbientColor, "LIGHTMODEL_AMBIENT_COLOR", false);
    ss << stateItemString(_lightModelSceneFrontColor, "LIGHTMODEL_FRONT_SCENE_COLOR", false);
    ss << stateItemString(_lightModelSceneBackColor, "LIGHTMODEL_BACK_SCENE_COLOR", false);

    ssAux.str("");
    for (acd_uint i=0; i < ACDX_FP_MAX_MODELVIEW_MATRICES_LIMIT ; i++)
    {
        ssAux << "MODELVIEW" << i << "_MATRIX"; ss << stateItemString(_modelViewMatrix[i], ssAux.str().c_str(), false); ssAux.str("");
    }

    ss << stateItemString(_projectionMatrix, "PROJECTION_MATRIX", false);

    ssAux.str("");
    for (acd_uint i=0; i < ACDX_FP_MAX_TEXTURE_STAGES_LIMIT ; i++)
    {
        ssAux << "TEXTURE_STAGE" << i << "_COORD_S_OBJECT_PLANE"; ss << stateItemString(_textCoordSObjectPlane[i], ssAux.str().c_str(), false); ssAux.str("");
        ssAux << "TEXTURE_STAGE" << i << "_COORD_T_OBJECT_PLANE"; ss << stateItemString(_textCoordTObjectPlane[i], ssAux.str().c_str(), false); ssAux.str("");
        ssAux << "TEXTURE_STAGE" << i << "_COORD_R_OBJECT_PLANE"; ss << stateItemString(_textCoordRObjectPlane[i], ssAux.str().c_str(), false); ssAux.str("");
        ssAux << "TEXTURE_STAGE" << i << "_COORD_Q_OBJECT_PLANE"; ss << stateItemString(_textCoordQObjectPlane[i], ssAux.str().c_str(), false); ssAux.str("");

        ssAux << "TEXTURE_STAGE" << i << "_COORD_S_EYE_PLANE"; ss << stateItemString(_textCoordSEyePlane[i], ssAux.str().c_str(), false); ssAux.str("");
        ssAux << "TEXTURE_STAGE" << i << "_COORD_T_EYE_PLANE"; ss << stateItemString(_textCoordTEyePlane[i], ssAux.str().c_str(), false); ssAux.str("");
        ssAux << "TEXTURE_STAGE" << i << "_COORD_R_EYE_PLANE"; ss << stateItemString(_textCoordREyePlane[i], ssAux.str().c_str(), false); ssAux.str("");
        ssAux << "TEXTURE_STAGE" << i << "_COORD_Q_EYE_PLANE"; ss << stateItemString(_textCoordQEyePlane[i], ssAux.str().c_str(), false); ssAux.str("");

        ssAux << "TEXTURE_STAGE" << i << "_COORD_MATRIX"; ss << stateItemString(_textureCoordMatrix[i], ssAux.str().c_str(), false); ssAux.str("");
        ssAux << "TEXTURE_STAGE" << i << "_ENV_COLOR"; ss << stateItemString(_textEnvColor[i], ssAux.str().c_str(), false); ssAux.str("");
    }

    ss << stateItemString(_near, "DEPTH_NEAR_VALUE", false);
    ss << stateItemString(_far, "DEPTH_FAR_VALUE", false);
    ss << stateItemString(_fogBlendColor, "FOG_BLENDING_COLOR", false);
    ss << stateItemString(_fogDensity, "FOG_DENSITY", false);
    ss << stateItemString(_fogLinearStart, "FOG_LINEAR_START", false);
    ss << stateItemString(_fogLinearEnd, "FOG_LINEAR_END", false);
    ss << stateItemString(_alphaTestRefValue, "ALPHA_TEST_REF_VALUE", false);

    return ss.str();
}

void ACDXFixedPipelineStateImp::getGLStateId(ACDX_STORED_FP_ITEM_ID stateId, VectorId& vId, acd_uint& componentMask, MatrixId& mId, acd_uint& unit, MatrixType& type, acd_bool& isMatrix) const
{
    if (stateId >= ACDX_MATRIX_FIRST_ID)
    {    
        // It큦 a matrix state
        isMatrix = true;
        
        if ((stateId >= ACDX_M_MODELVIEW) && (stateId < ACDX_M_MODELVIEW + ACDX_FP_MAX_MODELVIEW_MATRICES_LIMIT))
        {    
            // It큦 a modelview matrix
            unit = stateId - ACDX_M_MODELVIEW;
            mId = M_MODELVIEW;
            type = MT_NONE;
        }
        else if ( (stateId >= ACDX_M_TEXTURE) && (stateId < ACDX_M_TEXTURE + ACDX_FP_MAX_TEXTURE_STAGES_LIMIT))
        {
            // It큦 a texture matrix
            unit = stateId - ACDX_M_TEXTURE;
            mId = M_TEXTURE;
            type = MT_NONE;
        }
        else if ( stateId == ACDX_M_PROJECTION )
        {
            // It큦 a projection matrix
            unit = 0;
            mId = M_PROJECTION;
            type = MT_NONE;
        }
        // else if (... <- write here for future additional defined matrix type.
    }
    else // Vector state
    {
        isMatrix = false;
        componentMask = 0x0F;
        
        if ( stateId < ACDX_LIGHT_FIRST_ID )
        {
            // It큦 a material state
            unit = 0;

            switch(stateId)
            {
                case ACDX_MATERIAL_FRONT_AMBIENT: vId = VectorId(V_MATERIAL_FRONT_AMBIENT); break;
                case ACDX_MATERIAL_FRONT_DIFFUSE: vId = VectorId(V_MATERIAL_FRONT_DIFUSSE); break;
                case ACDX_MATERIAL_FRONT_SPECULAR: vId = VectorId(V_MATERIAL_FRONT_SPECULAR); break;
                case ACDX_MATERIAL_FRONT_EMISSION: vId = VectorId(V_MATERIAL_FRONT_EMISSION); break;
                case ACDX_MATERIAL_FRONT_SHININESS: vId = VectorId(V_MATERIAL_FRONT_SHININESS); componentMask = 0x08; break;
                case ACDX_MATERIAL_BACK_AMBIENT: vId = VectorId(V_MATERIAL_BACK_AMBIENT); break;
                case ACDX_MATERIAL_BACK_DIFFUSE: vId = VectorId(V_MATERIAL_BACK_DIFUSSE); break;
                case ACDX_MATERIAL_BACK_SPECULAR: vId = VectorId(V_MATERIAL_BACK_SPECULAR); break;
                case ACDX_MATERIAL_BACK_EMISSION: vId = VectorId(V_MATERIAL_BACK_EMISSION); break;
                case ACDX_MATERIAL_BACK_SHININESS: vId = VectorId(V_MATERIAL_BACK_SHININESS); componentMask = 0x08; break;
            }
        }      
        else if ( stateId < ACDX_LIGHTMODEL_FIRST_ID )
        {
            // It큦 a light state
            acd_uint aux = stateId - ACDX_LIGHT_FIRST_ID;
            unit = acd_uint(acd_float(aux) / acd_float(ACDX_LIGHT_PROPERTIES_COUNT));
            
            ACDX_STORED_FP_ITEM_ID baseLight = ACDX_STORED_FP_ITEM_ID( (aux % ACDX_LIGHT_PROPERTIES_COUNT) + ACDX_LIGHT_FIRST_ID);
            switch(baseLight)
            {
                case ACDX_LIGHT_AMBIENT: vId = VectorId(V_LIGHT_AMBIENT + 7 * unit); break;
                case ACDX_LIGHT_DIFFUSE: vId = VectorId(V_LIGHT_DIFFUSE + 7 * unit); break;
                case ACDX_LIGHT_SPECULAR: vId = VectorId(V_LIGHT_SPECULAR + 7 * unit); break;
                case ACDX_LIGHT_POSITION: vId = VectorId(V_LIGHT_POSITION + 7 * unit); componentMask = 0x0F; break;
                case ACDX_LIGHT_DIRECTION: vId = VectorId(V_LIGHT_POSITION + 7 * unit); componentMask = 0x0E; break;
                case ACDX_LIGHT_ATTENUATION: vId = VectorId(V_LIGHT_ATTENUATION + 7 * unit); componentMask = 0x0E; break;
                case ACDX_LIGHT_SPOT_DIRECTION: vId = VectorId(V_LIGHT_SPOT_DIRECTION + 7 * unit); componentMask = 0x0E; break;
                case ACDX_LIGHT_SPOT_CUTOFF: vId = VectorId(V_LIGHT_SPOT_DIRECTION + 7 * unit); componentMask = 0x01; break;
                case ACDX_LIGHT_SPOT_EXPONENT: vId = VectorId(V_LIGHT_ATTENUATION + 7 * unit); componentMask = 0x01; break;
            }
        }
        else if ( stateId < ACDX_TEXGEN_FIRST_ID )
        {
            // It큦 a lightmodel state
            switch( stateId )
            {
                case ACDX_LIGHTMODEL_AMBIENT: vId = VectorId(V_LIGHTMODEL_AMBIENT); break;
                case ACDX_LIGHTMODEL_FRONT_SCENECOLOR: vId = VectorId(V_LIGHTMODEL_FRONT_SCENECOLOR); break;
                case ACDX_LIGHTMODEL_BACK_SCENECOLOR: vId = VectorId(V_LIGHTMODEL_BACK_SCENECOLOR); break;
            }
        }
        else if ( stateId < ACDX_FOG_FIRST_ID )
        {
            // It큦 a texture generation state
            
            acd_uint aux = stateId - ACDX_TEXGEN_FIRST_ID;
            unit = acd_uint(acd_float(aux) / acd_float(ACDX_TEXGEN_PROPERTIES_COUNT));
            
            ACDX_STORED_FP_ITEM_ID baseTexStage = ACDX_STORED_FP_ITEM_ID((aux % ACDX_TEXGEN_PROPERTIES_COUNT) + ACDX_TEXGEN_FIRST_ID);

            switch(baseTexStage)
            {
                case ACDX_TEXGEN_EYE_S: vId = VectorId(V_TEXGEN_EYE_S + 8 * unit); break;
                case ACDX_TEXGEN_EYE_T: vId = VectorId(V_TEXGEN_EYE_T + 8 * unit); break;
                case ACDX_TEXGEN_EYE_R: vId = VectorId(V_TEXGEN_EYE_R + 8 * unit); break;
                case ACDX_TEXGEN_EYE_Q: vId = VectorId(V_TEXGEN_EYE_Q + 8 * unit); break;
                case ACDX_TEXGEN_OBJECT_S: vId = VectorId(V_TEXGEN_OBJECT_S + 8 * unit); break;
                case ACDX_TEXGEN_OBJECT_T: vId = VectorId(V_TEXGEN_OBJECT_T + 8 * unit); break;
                case ACDX_TEXGEN_OBJECT_R: vId = VectorId(V_TEXGEN_OBJECT_R + 8 * unit); break;
                case ACDX_TEXGEN_OBJECT_Q: vId = VectorId(V_TEXGEN_OBJECT_Q + 8 * unit); break;
            }
        }
        else if ( stateId < ACDX_TEXENV_FIRST_ID )
        {
            // It큦 a FOG state
            switch(stateId)
            {
                case ACDX_FOG_COLOR: vId = VectorId(V_FOG_COLOR); break;
                case ACDX_FOG_DENSITY: vId = VectorId(V_FOG_PARAMS); componentMask = 0x08; break;
                case ACDX_FOG_LINEAR_START: vId = VectorId(V_FOG_PARAMS); componentMask = 0x04; break;
                case ACDX_FOG_LINEAR_END: vId = VectorId(V_FOG_PARAMS); componentMask = 0x02; break;
            }
        }
        else if ( stateId < ACDX_DEPTH_FIRST_ID )
        {
            // It큦 a texture environment state
            acd_uint aux = stateId - ACDX_TEXENV_FIRST_ID;
            unit = acd_uint(acd_float(aux) / acd_float(ACDX_TEXENV_PROPERTIES_COUNT));
            
            ACDX_STORED_FP_ITEM_ID baseTexStage = ACDX_STORED_FP_ITEM_ID((aux % ACDX_TEXENV_PROPERTIES_COUNT) + ACDX_TEXENV_FIRST_ID);
        
            switch( baseTexStage )
            {
                case ACDX_TEXENV_COLOR: vId = VectorId(V_TEXENV_COLOR + unit); break;
            }
        }
        else if ( stateId < ACDX_ALPHA_TEST_FIRST_ID )
        {
            // It큦 a depth range state
            switch( stateId )
            {
                case ACDX_DEPTH_RANGE_NEAR: 
                case ACDX_DEPTH_RANGE_FAR: panic("ACDXFixedPipelineStateImp","getGLStateId","Depth range not in ACDXGLState! Shouldn큧 reach here! "); break;
            }
        }
        else if ( stateId < ACDX_MATRIX_FIRST_ID )
        {
            // It큦 a alpha test state. 
            switch( stateId )
            {
                case ACDX_ALPHA_TEST_REF_VALUE: panic("ACDXFixedPipelineStateImp","getGLStateId","Alpha test ref value not in ACDXGLState! Shouldn큧 reach here!"); break;
            }
        }
    }

}

void ACDXFixedPipelineStateImp::printStateEnum(std::ostream& os, ACDX_STORED_FP_ITEM_ID stateId)
{
    acd_uint unit = 0;

    if (stateId >= ACDX_MATRIX_FIRST_ID)
    {    
        // It큦 a matrix state
        
        if ((stateId >= ACDX_M_MODELVIEW) && (stateId < ACDX_M_MODELVIEW + ACDX_FP_MAX_MODELVIEW_MATRICES_LIMIT))
        {    
            // It큦 a modelview matrix
            unit = stateId - ACDX_M_MODELVIEW;
            os << "ACDX_M_MODELVIEW(" << unit << ")";
        }
        else if ( (stateId >= ACDX_M_TEXTURE) && (stateId < ACDX_M_TEXTURE + ACDX_FP_MAX_TEXTURE_STAGES_LIMIT))
        {
            // It큦 a texture matrix
            unit = stateId - ACDX_M_TEXTURE;
            os << "ACDX_M_TEXTURE(" << unit << ")";
        }
        else if ( stateId == ACDX_M_PROJECTION )
        {
            // It큦 a projection matrix
            os << "ACDX_M_PROJECTION";
        }
        // else if (... <- write here for future additional defined matrix type.
    }
    else // Vector state
    {
        if ( stateId < ACDX_LIGHT_FIRST_ID )
        {
            // It큦 a material state
            switch(stateId)
            {
                case ACDX_MATERIAL_FRONT_AMBIENT: os << "ACDX_MATERIAL_FRONT_AMBIENT"; break;
                case ACDX_MATERIAL_FRONT_DIFFUSE: os << "ACDX_MATERIAL_FRONT_DIFFUSE"; break;
                case ACDX_MATERIAL_FRONT_SPECULAR: os << "ACDX_MATERIAL_FRONT_SPECULAR"; break;
                case ACDX_MATERIAL_FRONT_EMISSION: os << "ACDX_MATERIAL_FRONT_EMISSION"; break;
                case ACDX_MATERIAL_FRONT_SHININESS: os << "ACDX_MATERIAL_FRONT_SHININESS"; break;
                case ACDX_MATERIAL_BACK_AMBIENT: os << "ACDX_MATERIAL_BACK_AMBIENT"; break;
                case ACDX_MATERIAL_BACK_DIFFUSE: os << "ACDX_MATERIAL_BACK_DIFFUSE"; break;
                case ACDX_MATERIAL_BACK_SPECULAR: os << "ACDX_MATERIAL_BACK_SPECULAR"; break;
                case ACDX_MATERIAL_BACK_EMISSION: os << "ACDX_MATERIAL_BACK_EMISSION"; break;
                case ACDX_MATERIAL_BACK_SHININESS: os << "ACDX_MATERIAL_BACK_SHININESS"; break;
            }
        }
        else if ( stateId < ACDX_LIGHTMODEL_FIRST_ID )
        {
            // It큦 a light state
            acd_uint aux = stateId - ACDX_LIGHT_FIRST_ID;
            unit = acd_uint(acd_float(aux) / acd_float(ACDX_LIGHT_PROPERTIES_COUNT));
            
            ACDX_STORED_FP_ITEM_ID baseLight = ACDX_STORED_FP_ITEM_ID( (aux % ACDX_LIGHT_PROPERTIES_COUNT) + ACDX_LIGHT_FIRST_ID);
            
            switch(baseLight)
            {
                case ACDX_LIGHT_AMBIENT: os << "ACDX_LIGHT_AMBIENT(" << unit << ")"; break;
                case ACDX_LIGHT_DIFFUSE: os << "ACDX_LIGHT_DIFFUSE(" << unit << ")"; break;
                case ACDX_LIGHT_SPECULAR: os << "ACDX_LIGHT_SPECULAR(" << unit << ")"; break;
                case ACDX_LIGHT_POSITION: os << "ACDX_LIGHT_POSITION(" << unit << ")"; break;
                case ACDX_LIGHT_DIRECTION: os << "ACDX_LIGHT_DIRECTION(" << unit << ")"; break;
                case ACDX_LIGHT_ATTENUATION: os << "ACDX_LIGHT_ATTENUATION(" << unit << ")"; break;
                case ACDX_LIGHT_SPOT_DIRECTION: os << "ACDX_LIGHT_SPOT_DIRECTION(" << unit << ")"; break;
                case ACDX_LIGHT_SPOT_CUTOFF: os << "ACDX_LIGHT_SPOT_CUTOFF(" << unit << ")"; break;
                case ACDX_LIGHT_SPOT_EXPONENT: os << "ACDX_LIGHT_SPOT_EXPONENT(" << unit << ")"; break;
            }
        }
        else if ( stateId < ACDX_TEXGEN_FIRST_ID )
        {
            // It큦 a lightmodel state
            switch( stateId )
            {
                case ACDX_LIGHTMODEL_AMBIENT: os << "ACDX_LIGHTMODEL_AMBIENT";  break;
                case ACDX_LIGHTMODEL_FRONT_SCENECOLOR: os << "ACDX_LIGHTMODEL_FRONT_SCENECOLOR"; break;
                case ACDX_LIGHTMODEL_BACK_SCENECOLOR: os << "ACDX_LIGHTMODEL_BACK_SCENECOLOR"; break;
            }
        }
        else if ( stateId < ACDX_FOG_FIRST_ID )
        {
            // It큦 a texture generation state
            
            acd_uint aux = stateId - ACDX_TEXGEN_FIRST_ID;
            unit = acd_uint(acd_float(aux) / acd_float(ACDX_TEXGEN_PROPERTIES_COUNT));
            
            ACDX_STORED_FP_ITEM_ID baseTexStage = ACDX_STORED_FP_ITEM_ID((aux % ACDX_TEXGEN_PROPERTIES_COUNT) + ACDX_TEXGEN_FIRST_ID);

            switch(baseTexStage)
            {
                case ACDX_TEXGEN_EYE_S: os << "ACDX_TEXGEN_EYE_S(" << unit << ")"; break;
                case ACDX_TEXGEN_EYE_T: os << "ACDX_TEXGEN_EYE_T(" << unit << ")"; break;
                case ACDX_TEXGEN_EYE_R: os << "ACDX_TEXGEN_EYE_R(" << unit << ")"; break;
                case ACDX_TEXGEN_EYE_Q: os << "ACDX_TEXGEN_EYE_Q(" << unit << ")"; break;
                case ACDX_TEXGEN_OBJECT_S: os << "ACDX_TEXGEN_OBJECT_S(" << unit << ")"; break;
                case ACDX_TEXGEN_OBJECT_T: os << "ACDX_TEXGEN_OBJECT_T(" << unit << ")"; break;
                case ACDX_TEXGEN_OBJECT_R: os << "ACDX_TEXGEN_OBJECT_R(" << unit << ")"; break;
                case ACDX_TEXGEN_OBJECT_Q: os << "ACDX_TEXGEN_OBJECT_Q(" << unit << ")"; break;
            }
        }
        else if ( stateId < ACDX_TEXENV_FIRST_ID )
        {
            // It큦 a FOG state
            switch(stateId)
            {
                case ACDX_FOG_COLOR: os << "ACDX_FOG_COLOR"; break;
                case ACDX_FOG_DENSITY: os << "ACDX_FOG_DENSITY"; break;
                case ACDX_FOG_LINEAR_START: os << "ACDX_FOG_LINEAR_START"; break;
                case ACDX_FOG_LINEAR_END: os << "ACDX_FOG_LINEAR_END"; break;
            }
        }
        else if ( stateId < ACDX_DEPTH_FIRST_ID )
        {
            // It큦 a texture environment state
            acd_uint aux = stateId - ACDX_TEXENV_FIRST_ID;
            unit = acd_uint(acd_float(aux) / acd_float(ACDX_TEXENV_PROPERTIES_COUNT));
            
            ACDX_STORED_FP_ITEM_ID baseTexStage = ACDX_STORED_FP_ITEM_ID((aux % ACDX_TEXENV_PROPERTIES_COUNT) + ACDX_TEXENV_FIRST_ID);
        
            switch( baseTexStage )
            {
                case ACDX_TEXENV_COLOR: os << "ACDX_TEXENV_COLOR(" << unit << ")"; break;
            }
        }
        else if ( stateId < ACDX_ALPHA_TEST_FIRST_ID )
        {
            // It큦 a depth range state
            switch( stateId )
            {
                case ACDX_DEPTH_RANGE_NEAR: os << "ACDX_DEPTH_RANGE_NEAR"; break;
                case ACDX_DEPTH_RANGE_FAR: os << "ACDX_DEPTH_RANGE_FAR"; break;
            }
        }
        else if ( stateId < ACDX_MATRIX_FIRST_ID )
        {
            // It큦 a alpha test state. 
            switch( stateId )
            {
                case ACDX_ALPHA_TEST_REF_VALUE: os << "ACDX_ALPHA_TEST_REF_VALUE"; break;
            }
        }
    }

}

const StoredStateItem* ACDXFixedPipelineStateImp::createStoredStateItem(ACDX_STORED_FP_ITEM_ID stateId) const
{
    acd_uint unit = 0;
    
    ACDXStoredFPStateItem* ret;

    if (stateId >= ACDX_MATRIX_FIRST_ID)
    {    
        // It큦 a matrix state
        
        if ((stateId >= ACDX_M_MODELVIEW) && (stateId < ACDX_M_MODELVIEW + ACDX_FP_MAX_MODELVIEW_MATRICES_LIMIT))
        {    
            // It큦 a modelview matrix
            unit = stateId - ACDX_M_MODELVIEW;
            ret = new ACDXFloatMatrix4x4StoredStateItem(_modelViewMatrix[unit]);
        }
        else if ( (stateId >= ACDX_M_TEXTURE) && (stateId < ACDX_M_TEXTURE + ACDX_FP_MAX_TEXTURE_STAGES_LIMIT))
        {
            // It큦 a texture matrix
            unit = stateId - ACDX_M_TEXTURE;
            ret = new ACDXFloatMatrix4x4StoredStateItem(_textureCoordMatrix[unit]);
        }
        else if ( stateId == ACDX_M_PROJECTION )
        {
            // It큦 a projection matrix
            ret = new ACDXFloatMatrix4x4StoredStateItem(_projectionMatrix);
        }
        // else if (... <- write here for future additional defined matrix type.
        else
        {
            panic("ACDXFixedPipelineStateImp","createStoredStateItem()","Unknown Matrix state");
            ret = 0;
        }
    }
    else // Vector state
    {
        if ( stateId < ACDX_LIGHT_FIRST_ID )
        {
            // It큦 a material state
            switch(stateId)
            {
                case ACDX_MATERIAL_FRONT_AMBIENT:  ret = new ACDXFloatVector4StoredStateItem(_materialFrontAmbient); break;
                case ACDX_MATERIAL_FRONT_DIFFUSE:  ret = new ACDXFloatVector4StoredStateItem(_materialFrontDiffuse); break;
                case ACDX_MATERIAL_FRONT_SPECULAR:  ret = new ACDXFloatVector4StoredStateItem(_materialFrontSpecular); break;
                case ACDX_MATERIAL_FRONT_EMISSION:  ret = new ACDXFloatVector4StoredStateItem(_materialFrontEmission); break;
                case ACDX_MATERIAL_FRONT_SHININESS:  ret = new ACDXSingleFloatStoredStateItem(_materialFrontShininess); break;
                case ACDX_MATERIAL_BACK_AMBIENT:  ret = new ACDXFloatVector4StoredStateItem(_materialBackAmbient); break;
                case ACDX_MATERIAL_BACK_DIFFUSE:  ret = new ACDXFloatVector4StoredStateItem(_materialBackDiffuse); break;
                case ACDX_MATERIAL_BACK_SPECULAR:  ret = new ACDXFloatVector4StoredStateItem(_materialBackSpecular); break;
                case ACDX_MATERIAL_BACK_EMISSION:  ret = new ACDXFloatVector4StoredStateItem(_materialBackEmission); break;
                case ACDX_MATERIAL_BACK_SHININESS:  ret = new ACDXSingleFloatStoredStateItem(_materialBackShininess); break;
                // case ACDX_MATERIAL_... <- add here future additional material states.
                default:
                    panic("ACDXFixedPipelineStateImp","createStoredStateItem()","Unknown material state");
                    ret = 0;
            }
        }
        else if ( stateId < ACDX_LIGHTMODEL_FIRST_ID )
        {
            // It큦 a light state
            acd_uint aux = stateId - ACDX_LIGHT_FIRST_ID;
            unit = acd_uint(acd_float(aux) / acd_float(ACDX_LIGHT_PROPERTIES_COUNT));
            
            ACDX_STORED_FP_ITEM_ID baseLight = ACDX_STORED_FP_ITEM_ID( (aux % ACDX_LIGHT_PROPERTIES_COUNT) + ACDX_LIGHT_FIRST_ID);
            
            switch(baseLight)
            {
                case ACDX_LIGHT_AMBIENT: ret = new ACDXFloatVector4StoredStateItem(_lightAmbient[unit]); break;
                case ACDX_LIGHT_DIFFUSE: ret = new ACDXFloatVector4StoredStateItem(_lightDiffuse[unit]); break;
                case ACDX_LIGHT_SPECULAR: ret = new ACDXFloatVector4StoredStateItem(_lightSpecular[unit]); break;
                case ACDX_LIGHT_POSITION: ret = new ACDXFloatVector4StoredStateItem(_lightPosition[unit]); break;
                case ACDX_LIGHT_DIRECTION: ret = new ACDXFloatVector3StoredStateItem(_lightDirection[unit]); break;
                case ACDX_LIGHT_ATTENUATION: ret = new ACDXFloatVector3StoredStateItem(_lightAttenuation[unit]); break;
                case ACDX_LIGHT_SPOT_DIRECTION: ret = new ACDXFloatVector3StoredStateItem(_lightSpotDirection[unit]); break;
                case ACDX_LIGHT_SPOT_CUTOFF: ret = new ACDXSingleFloatStoredStateItem(_lightSpotCutOffAngle[unit]); break;
                case ACDX_LIGHT_SPOT_EXPONENT: ret = new ACDXSingleFloatStoredStateItem(_lightSpotExponent[unit]); break;
                // case ACDX_LIGHT_... <- add here future additional light states.
                default:
                    panic("ACDXFixedPipelineStateImp","createStoredStateItem()","Unknown light state");
                    ret = 0;
            }
        }
        else if ( stateId < ACDX_TEXGEN_FIRST_ID )
        {
            // It큦 a lightmodel state
            switch( stateId )
            {
                case ACDX_LIGHTMODEL_AMBIENT: ret = new ACDXFloatVector4StoredStateItem(_lightModelAmbientColor); break;
                case ACDX_LIGHTMODEL_FRONT_SCENECOLOR: ret = new ACDXFloatVector4StoredStateItem(_lightModelSceneFrontColor); break;
                case ACDX_LIGHTMODEL_BACK_SCENECOLOR: ret = new ACDXFloatVector4StoredStateItem(_lightModelSceneBackColor); break;
                // case ACDX_LIGHTMODEL_... <- add here future additional lightmodel states.
                default:
                    panic("ACDXFixedPipelineStateImp","createStoredStateItem()","Unknown lightmodel state");
                    ret = 0;
            }
        }
        else if ( stateId < ACDX_FOG_FIRST_ID )
        {
            // It큦 a texture generation state
            
            acd_uint aux = stateId - ACDX_TEXGEN_FIRST_ID;
            unit = acd_uint(acd_float(aux) / acd_float(ACDX_TEXGEN_PROPERTIES_COUNT));
            
            ACDX_STORED_FP_ITEM_ID baseTexStage = ACDX_STORED_FP_ITEM_ID((aux % ACDX_TEXGEN_PROPERTIES_COUNT) + ACDX_TEXGEN_FIRST_ID);

            switch(baseTexStage)
            {
                case ACDX_TEXGEN_EYE_S: ret = new ACDXFloatVector4StoredStateItem(_textCoordSEyePlane[unit]); break;
                case ACDX_TEXGEN_EYE_T: ret = new ACDXFloatVector4StoredStateItem(_textCoordTEyePlane[unit]); break;
                case ACDX_TEXGEN_EYE_R: ret = new ACDXFloatVector4StoredStateItem(_textCoordREyePlane[unit]); break;
                case ACDX_TEXGEN_EYE_Q: ret = new ACDXFloatVector4StoredStateItem(_textCoordQEyePlane[unit]); break;
                case ACDX_TEXGEN_OBJECT_S: ret = new ACDXFloatVector4StoredStateItem(_textCoordSObjectPlane[unit]); break;
                case ACDX_TEXGEN_OBJECT_T: ret = new ACDXFloatVector4StoredStateItem(_textCoordTObjectPlane[unit]); break;
                case ACDX_TEXGEN_OBJECT_R: ret = new ACDXFloatVector4StoredStateItem(_textCoordRObjectPlane[unit]); break;
                case ACDX_TEXGEN_OBJECT_Q: ret = new ACDXFloatVector4StoredStateItem(_textCoordQObjectPlane[unit]); break;
                // case ACDX_TEXGEN_... <- add here future additional texgen states.
                default:
                    panic("ACDXFixedPipelineStateImp","createStoredStateItem()","Unknown texgen state");
                    ret = 0;
            }
        }
        else if ( stateId < ACDX_TEXENV_FIRST_ID )
        {
            // It큦 a FOG state
            switch(stateId)
            {
                case ACDX_FOG_COLOR: ret = new ACDXFloatVector4StoredStateItem(_fogBlendColor); break;
                case ACDX_FOG_DENSITY: ret = new ACDXSingleFloatStoredStateItem(_fogDensity); break;
                case ACDX_FOG_LINEAR_START: ret = new ACDXSingleFloatStoredStateItem(_fogLinearStart); break;
                case ACDX_FOG_LINEAR_END: ret = new ACDXSingleFloatStoredStateItem(_fogLinearEnd); break;
                // case ACDX_FOG_... <- add here future additional FOG states.
                default:
                    panic("ACDXFixedPipelineStateImp","createStoredStateItem()","Unknown FOG state");
                    ret = 0;
            }
        }
        else if ( stateId < ACDX_DEPTH_FIRST_ID )
        {
            // It큦 a texture environment state
            acd_uint aux = stateId - ACDX_TEXENV_FIRST_ID;
            unit = acd_uint(acd_float(aux) / acd_float(ACDX_TEXENV_PROPERTIES_COUNT));
            
            ACDX_STORED_FP_ITEM_ID baseTexStage = ACDX_STORED_FP_ITEM_ID((aux % ACDX_TEXENV_PROPERTIES_COUNT) + ACDX_TEXENV_FIRST_ID);
        
            switch( baseTexStage )
            {
                case ACDX_TEXENV_COLOR: ret = new ACDXFloatVector4StoredStateItem(_textEnvColor[unit]); break;
                // case ACDX_TEXENV_... <- add here future additional texenv states.
                default:
                    panic("ACDXFixedPipelineStateImp","createStoredStateItem()","Unknown texenv state");
                    ret = 0;
            }
        }
        else if ( stateId < ACDX_ALPHA_TEST_FIRST_ID )
        {
            // It큦 a depth range state
            switch( stateId )
            {
                case ACDX_DEPTH_RANGE_NEAR: ret = new ACDXSingleFloatStoredStateItem(_near); break;
                case ACDX_DEPTH_RANGE_FAR: ret = new ACDXSingleFloatStoredStateItem(_far); break;
                // case ACDX_DEPTH_... <- add here future additional depth range states.
                default:
                    panic("ACDXFixedPipelineStateImp","createStoredStateItem()","Unknown depth range state");
                    ret = 0;
            }
        }
        else if ( stateId < ACDX_MATRIX_FIRST_ID )
        {
            // It큦 a alpha test state. 
            switch( stateId )
            {
                case ACDX_ALPHA_TEST_REF_VALUE: ret = new ACDXSingleFloatStoredStateItem(_alphaTestRefValue); break;
                // case ACDX_ALPHA_TEST_... <- add here future additional alpha test states.
                default:
                    panic("ACDXFixedPipelineStateImp","createStoredStateItem()","Unknown alpha test state");
                    ret = 0;

            }
        }
        // else if (... <- write here for future additional defined vector state groups.
        else
        {
            panic("ACDXFixedPipelineStateImp","createStoredStateItem()","Unknown Vector state");
            ret = 0;
        }
    }
    
    ret->setItemId(stateId);

    return ret;
}

#define CAST_TO_MATRIX(X) static_cast<const ACDXFloatMatrix4x4StoredStateItem*>(X)
#define CAST_TO_VECT4(X) static_cast<const ACDXFloatVector4StoredStateItem*>(X)
#define CAST_TO_VECT3(X) static_cast<const ACDXFloatVector3StoredStateItem*>(X)
#define CAST_TO_SINGLE(X) static_cast<const ACDXSingleFloatStoredStateItem*>(X)

void ACDXFixedPipelineStateImp::restoreStoredStateItem(const StoredStateItem* ssi)
{
    acd_uint unit = 0;
    
    const ACDXStoredFPStateItem* sFPsi = static_cast<const ACDXStoredFPStateItem*>(ssi);

    ACDX_STORED_FP_ITEM_ID stateId = sFPsi->getItemId();

    if (stateId >= ACDX_MATRIX_FIRST_ID)
    {    
        // It큦 a matrix state
        
        if ((stateId >= ACDX_M_MODELVIEW) && (stateId < ACDX_M_MODELVIEW + ACDX_FP_MAX_MODELVIEW_MATRICES_LIMIT))
        {    
            // It큦 a modelview matrix
            unit = stateId - ACDX_M_MODELVIEW;
            _modelViewMatrix[unit] = *(CAST_TO_MATRIX(sFPsi));
        }
        else if ( (stateId >= ACDX_M_TEXTURE) && (stateId < ACDX_M_TEXTURE + ACDX_FP_MAX_TEXTURE_STAGES_LIMIT))
        {
            // It큦 a texture matrix
            unit = stateId - ACDX_M_TEXTURE;
            _textureCoordMatrix[unit] = *(CAST_TO_MATRIX(sFPsi));
        }
        else if ( stateId == ACDX_M_PROJECTION )
        {
            // It큦 a projection matrix
            _projectionMatrix = *(CAST_TO_MATRIX(sFPsi));
        }
        // else if (... <- write here for future additional defined matrix type.
    }
    else // Vector state
    {
        if ( stateId < ACDX_LIGHT_FIRST_ID )
        {
            // It큦 a material state
            switch(stateId)
            {
                case ACDX_MATERIAL_FRONT_AMBIENT: _materialFrontAmbient = *(CAST_TO_VECT4(sFPsi)); break;
                case ACDX_MATERIAL_FRONT_DIFFUSE:  _materialFrontDiffuse = *(CAST_TO_VECT4(sFPsi)); break;
                case ACDX_MATERIAL_FRONT_SPECULAR:  _materialFrontSpecular = *(CAST_TO_VECT4(sFPsi)); break;
                case ACDX_MATERIAL_FRONT_EMISSION:  _materialFrontEmission = *(CAST_TO_VECT4(sFPsi)); break;
                case ACDX_MATERIAL_FRONT_SHININESS:  _materialFrontShininess = *(CAST_TO_SINGLE(sFPsi)); break;
                case ACDX_MATERIAL_BACK_AMBIENT:  _materialBackAmbient = *(CAST_TO_VECT4(sFPsi)); break;
                case ACDX_MATERIAL_BACK_DIFFUSE:  _materialBackDiffuse = *(CAST_TO_VECT4(sFPsi)); break;
                case ACDX_MATERIAL_BACK_SPECULAR:  _materialBackSpecular = *(CAST_TO_VECT4(sFPsi)); break;
                case ACDX_MATERIAL_BACK_EMISSION:  _materialBackEmission = *(CAST_TO_VECT4(sFPsi)); break;
                case ACDX_MATERIAL_BACK_SHININESS:  _materialBackShininess = *(CAST_TO_SINGLE(sFPsi)); break;
            }
        }
        else if ( stateId < ACDX_LIGHTMODEL_FIRST_ID )
        {
            // It큦 a light state
            acd_uint aux = stateId - ACDX_LIGHT_FIRST_ID;
            unit = acd_uint(acd_float(aux) / acd_float(ACDX_LIGHT_PROPERTIES_COUNT));
            
            ACDX_STORED_FP_ITEM_ID baseLight = ACDX_STORED_FP_ITEM_ID( (aux % ACDX_LIGHT_PROPERTIES_COUNT) + ACDX_LIGHT_FIRST_ID);
            
            switch(baseLight)
            {
                case ACDX_LIGHT_AMBIENT: _lightAmbient[unit] = *(CAST_TO_VECT4(sFPsi)); break;
                case ACDX_LIGHT_DIFFUSE: _lightDiffuse[unit] = *(CAST_TO_VECT4(sFPsi)); break;
                case ACDX_LIGHT_SPECULAR: _lightSpecular[unit] = *(CAST_TO_VECT4(sFPsi)); break;
                case ACDX_LIGHT_POSITION: _lightPosition[unit] = *(CAST_TO_VECT4(sFPsi)); break;
                case ACDX_LIGHT_DIRECTION: _lightDirection[unit] = *(CAST_TO_VECT3(sFPsi)); break;
                case ACDX_LIGHT_ATTENUATION: _lightAttenuation[unit] = *(CAST_TO_VECT3(sFPsi)); break;
                case ACDX_LIGHT_SPOT_DIRECTION: _lightSpotDirection[unit] = *(CAST_TO_VECT3(sFPsi)); break;
                case ACDX_LIGHT_SPOT_CUTOFF: _lightSpotCutOffAngle[unit] = *(CAST_TO_SINGLE(sFPsi)); break;
                case ACDX_LIGHT_SPOT_EXPONENT: _lightSpotExponent[unit] = *(CAST_TO_SINGLE(sFPsi)); break;
            }
        }
        else if ( stateId < ACDX_TEXGEN_FIRST_ID )
        {
            // It큦 a lightmodel state
            switch( stateId )
            {
                case ACDX_LIGHTMODEL_AMBIENT: _lightModelAmbientColor = *(CAST_TO_VECT4(sFPsi)); break;
                case ACDX_LIGHTMODEL_FRONT_SCENECOLOR: _lightModelSceneFrontColor = *(CAST_TO_VECT4(sFPsi)); break;
                case ACDX_LIGHTMODEL_BACK_SCENECOLOR: _lightModelSceneBackColor = *(CAST_TO_VECT4(sFPsi)); break;
            }
        }
        else if ( stateId < ACDX_FOG_FIRST_ID )
        {
            // It큦 a texture generation state
            
            acd_uint aux = stateId - ACDX_TEXGEN_FIRST_ID;
            unit = acd_uint(acd_float(aux) / acd_float(ACDX_TEXGEN_PROPERTIES_COUNT));
            
            ACDX_STORED_FP_ITEM_ID baseTexStage = ACDX_STORED_FP_ITEM_ID((aux % ACDX_TEXGEN_PROPERTIES_COUNT) + ACDX_TEXGEN_FIRST_ID);

            switch(baseTexStage)
            {
                case ACDX_TEXGEN_EYE_S: _textCoordSEyePlane[unit] = *(CAST_TO_VECT4(sFPsi)); break;
                case ACDX_TEXGEN_EYE_T: _textCoordTEyePlane[unit] = *(CAST_TO_VECT4(sFPsi)); break;
                case ACDX_TEXGEN_EYE_R: _textCoordREyePlane[unit] = *(CAST_TO_VECT4(sFPsi)); break;
                case ACDX_TEXGEN_EYE_Q: _textCoordQEyePlane[unit] = *(CAST_TO_VECT4(sFPsi)); break;
                case ACDX_TEXGEN_OBJECT_S: _textCoordSObjectPlane[unit] = *(CAST_TO_VECT4(sFPsi)); break;
                case ACDX_TEXGEN_OBJECT_T: _textCoordTObjectPlane[unit] = *(CAST_TO_VECT4(sFPsi)); break;
                case ACDX_TEXGEN_OBJECT_R: _textCoordRObjectPlane[unit] = *(CAST_TO_VECT4(sFPsi)); break;
                case ACDX_TEXGEN_OBJECT_Q: _textCoordQObjectPlane[unit] = *(CAST_TO_VECT4(sFPsi)); break;
            }
        }
        else if ( stateId < ACDX_TEXENV_FIRST_ID )
        {
            // It큦 a FOG state
            switch(stateId)
            {
                case ACDX_FOG_COLOR: _fogBlendColor = *(CAST_TO_VECT4(sFPsi)); break;
                case ACDX_FOG_DENSITY: _fogDensity = *(CAST_TO_SINGLE(sFPsi)); break;
                case ACDX_FOG_LINEAR_START: _fogLinearStart = *(CAST_TO_SINGLE(sFPsi)); break;
                case ACDX_FOG_LINEAR_END: _fogLinearEnd = *(CAST_TO_SINGLE(sFPsi)); break;
            }
        }
        else if ( stateId < ACDX_DEPTH_FIRST_ID )
        {
            // It큦 a texture environment state
            acd_uint aux = stateId - ACDX_TEXENV_FIRST_ID;
            unit = acd_uint(acd_float(aux) / acd_float(ACDX_TEXENV_PROPERTIES_COUNT));
            
            ACDX_STORED_FP_ITEM_ID baseTexStage = ACDX_STORED_FP_ITEM_ID((aux % ACDX_TEXENV_PROPERTIES_COUNT) + ACDX_TEXENV_FIRST_ID);
        
            switch( baseTexStage )
            {
                case ACDX_TEXENV_COLOR: _textEnvColor[unit] = *(CAST_TO_VECT4(sFPsi)); break;
            }
        }
        else if ( stateId < ACDX_ALPHA_TEST_FIRST_ID )
        {
            // It큦 a depth range state
            switch( stateId )
            {
                case ACDX_DEPTH_RANGE_NEAR: _near = *(CAST_TO_SINGLE(sFPsi)); break;
                case ACDX_DEPTH_RANGE_FAR: _far = *(CAST_TO_SINGLE(sFPsi)); break;
            }
        }
        else if ( stateId < ACDX_MATRIX_FIRST_ID )
        {
            // It큦 a alpha test state. 
            switch( stateId )
            {
                case ACDX_ALPHA_TEST_REF_VALUE: _alphaTestRefValue = *(CAST_TO_SINGLE(sFPsi)); break;
            }
        }
    }

}

void ACDXFixedPipelineStateImp::DBG_dump(const acd_char* file, acd_enum) const
{
    fstream f(file,ios_base::out);

    f << getInternalState();

    f.close();
}

acd_bool ACDXFixedPipelineStateImp::DBG_save(const acd_char*) const
{
    return true;
}

acd_bool ACDXFixedPipelineStateImp::DBG_load(const acd_char*)
{
    return true;
}
