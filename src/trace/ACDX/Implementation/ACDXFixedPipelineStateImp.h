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

#ifndef ACDX_FIXED_PIPELINE_STATE_IMP_H
    #define ACDX_FIXED_PIPELINE_STATE_IMP_H

#include "ACDXGlobalTypeDefinitions.h"

#include "ACDXFixedPipelineState.h"
#include "StateItem.h"

#include "ACDXGLState.h"

#include "ACDXLightImp.h"
#include "ACDXMaterialImp.h"

#include "ACDXStoredFPStateImp.h"

#include <vector>
#include <string>

namespace acdlib
{

class ACDXFixedPipelineStateImp: public ACDXFixedPipelineState, public ACDXTransformAndLightingStage, public ACDXTextCoordGenerationStage, public ACDXFragmentShadingStage, public ACDXPostShadingStage 
{
public:

    //////////////////////////////////////
    // ACDXFixedPipelineState interface //
    //////////////////////////////////////

    /**
     * Gets an interface to the transform & lighting stages.
     */
    virtual ACDXTransformAndLightingStage& tl();

    /**
     * Gets an interface to the texture coordinate generation stage.
     */
    virtual ACDXTextCoordGenerationStage& txtcoord();

    /**
     * Gets an interface to the fragment shading and texturing stages.
     */
    virtual ACDXFragmentShadingStage& fshade();

    /**
     * Gets an interface to the post fragment shading stages (FOG and Alpha).
     */
    virtual ACDXPostShadingStage& postshade();

    /**
     * Save a group of Fixed Pipeline states
     *
     * @param siIds List of state identifiers to save
     * @returns The group of states saved
     */
    virtual ACDXStoredFPState* saveState(ACDXStoredFPItemIDList siIds) const;

/**
     * Save a single Fixed Pipeline state
     *
     * @param    stateId the identifier of the state to save
     * @returns The group of states saved
     */
    virtual ACDXStoredFPState* saveState(ACDX_STORED_FP_ITEM_ID stateId) const;

    /**
     * Save the whole set of Fixed Pipeline states
     *
     * @returns The whole states group saved
     */
    virtual ACDXStoredFPState* saveAllState() const;

    /**
     * Restores the value of a group of Fixed Pipeline states
     *
     * @param state a previously saved state group to be restored
     */
    virtual void restoreState(const ACDXStoredFPState* state);

    /**
     * Releases a ACDXStoredFPState interface object
     *
     * @param state The saved state object to be released
     */
    virtual void destroyState(ACDXStoredFPState* state) const;

    /**
     * Dumps the ACDXFixedPipelineState state
     */
    virtual void DBG_dump(const acd_char* file, acd_enum flags) const;

    /**
     * Saves a file with an image of the current ACDXFixedPipelineState state
     */
    virtual acd_bool DBG_save(const acd_char* file) const;

    /**
     * Restores the ACDXFixedPipelineState state from a image file previously saved
     */
    virtual acd_bool DBG_load(const acd_char* file);

    /////////////////////////////////////////////
    // ACDXTransformAndLightingStage interface //
    /////////////////////////////////////////////

    /**
     * Select an light object to be configured
     *
     * @param light The light number.
     * @return        The corresponding ACDXLight object.
     */
    virtual ACDXLight& light(acd_uint light);

    /**
     * Select a face material object to be configured
     *
     * @param face  The material properties face.
     * @return        The corresponding ACDXMaterial object.
     */
    virtual ACDXMaterial& material(ACDX_FACE face);

    /**
     * Sets the light model ambient color
     *
     * @param r        The red component of the color.
     * @param g        The green component of the color.
     * @param b        The blue component of the color.
     * @param a        The alpha component of the color.
     */
    virtual void setLightModelAmbientColor(acd_float r, acd_float g, acd_float b, acd_float a);
        
    /**
     * Sets the light model scene color
     *
     * @param face  The face where to set the scene color.
     * @param r        The red component of the color.
     * @param g        The green component of the color.
     * @param b        The blue component of the color.
     * @param a        The alpha component of the color.
     */
    virtual void setLightModelSceneColor(ACDX_FACE face, acd_float r, acd_float g, acd_float b, acd_float a);

    /**
     * Sets the Modelview Matrix.
     *
     * @param unit     The vertex unit matrix(Vertex blending support).
     * @param matrix The input matrix.
     */
    virtual void setModelviewMatrix(acd_uint unit, const ACDXFloatMatrix4x4& matrix);

    /**
     * Gets the Modelview Matrix
     *
     * @param unit The vertex unit matrix
     */
    virtual ACDXFloatMatrix4x4 getModelviewMatrix(acd_uint unit) const;

    /**
     * Sets the Projection Matrix.
     *
     * @param matrix The input matrix.
     */
    virtual void setProjectionMatrix(const ACDXFloatMatrix4x4& matrix);

    /**
     * Gets the Projection Matrix
     *
     * @return A copy of the current Projection Matrix
     */
    virtual ACDXFloatMatrix4x4 getProjectionMatrix() const;

    ////////////////////////////////////////////
    // ACDXTextCoordGenerationStage interface //
    ////////////////////////////////////////////

    /**
     * Sets the texture coordinate generation object and eye planes.
     *
     * @param textureStage The texture stage affected.
     * @param plane           The Eye or Object plane.
     * @param coord           The texture coordinate affected.
     * @param a               The a coefficient of the plane equation.
     * @param b               The b coefficient of the plane equation.
     * @param c               The c coefficient of the plane equation.
     * @param d               The d coefficient of the plane equation.
     */
    virtual void setTextureCoordPlane(acd_uint textureStage, ACDX_TEXTURE_COORD_PLANE plane, ACDX_TEXTURE_COORD coord, acd_float a, acd_float b, acd_float c, acd_float d);

    /**
     * Sets the texture coordinate matrix
     *
     * @param textureStage The texture stage affected.
     * @param matrix       The texture coordinate matrix.
     */
    virtual void setTextureCoordMatrix(acd_uint textureStage, const ACDXFloatMatrix4x4& matrix);

    /**
     * Gets the texture coordinate matrix
     *
     * @param textureStage The texture stage matrix selected
     */
    virtual ACDXFloatMatrix4x4 getTextureCoordMatrix(acd_uint textureStage) const;

    ////////////////////////////////////////
    // ACDXFragmentShadingStage interface //
    ////////////////////////////////////////

    /**
     * Sets the texture environment color used for each texture stage.
     *
     * @param textureStage    The corresponding texture stage.
     * @param r                The red component of the color.
     * @param g                The green component of the color.
     * @param b                The    blue component of the color.
     * @param a                The alpha component of the color.
     */
    virtual void setTextureEnvColor(acd_uint textureStage, acd_float r, acd_float g, acd_float b, acd_float a);

   /**
    * Sets the depth range (near and far plane distances)
    *
    * @param near           The distance of the near plane.
    * @param far           The distance of the far plane.
    */
    virtual void setDepthRange(acd_float near, acd_float far);

    ////////////////////////////////////
    // ACDXPostShadingStage interface //
    ////////////////////////////////////

   /**
    * Sets the FOG blending color.
    *
    * @param r        The red component of the color.
    * @param g        The green component of the color.
    * @param b        The blue component of the color.
    * @param a        The alpha component of the color.
    */
    virtual void setFOGBlendColor(acd_float r, acd_float g, acd_float b, acd_float a); 

    /**
     * Sets the FOG density exponent
     *
     * @param exponent The density exponent
     */
     virtual void setFOGDensity(acd_float exponent);

    /**
     * Sets the FOG linear mode start and end distances.
     *
     * @param start The FOG linear start distance.
     * @param end   The FOG linear end distance.
     */
     virtual void setFOGLinearDistances(acd_float start, acd_float end);

    /**
     * Sets the Alpha Test Reference Value.
     *
     * @param refValue The reference value.
     */
     virtual void setAlphaTestRefValue(acd_float refValue);

//////////////////////////
//  interface extension //
//////////////////////////
public:

    ACDXFixedPipelineStateImp(ACDXGLState *gls);

    const ACDXGLState *getGLState() const;

    void sync();

    void forceSync();

    std::string getInternalState() const;

    acd_float getSingleState(ACDX_STORED_FP_ITEM_ID stateId) const;

    ACDXFloatVector3 getVectorState3(ACDX_STORED_FP_ITEM_ID stateId) const;

    ACDXFloatVector4 getVectorState4(ACDX_STORED_FP_ITEM_ID stateId) const;
    
    ACDXFloatMatrix4x4 getMatrixState(ACDX_STORED_FP_ITEM_ID stateId) const;

    void getGLStateId(ACDX_STORED_FP_ITEM_ID stateId, VectorId& vId, acd_uint& componentMask, MatrixId& mId, acd_uint& unit, MatrixType& type, acd_bool& isMatrix) const;

    void setSingleState(ACDX_STORED_FP_ITEM_ID stateId, acd_float value);

    void setVectorState3(ACDX_STORED_FP_ITEM_ID stateId, ACDXFloatVector3 value);

    void setVectorState4(ACDX_STORED_FP_ITEM_ID stateId, ACDXFloatVector4 value);

    void setMatrixState(ACDX_STORED_FP_ITEM_ID stateId, ACDXFloatMatrix4x4 value);

    static void printStateEnum(std::ostream& os, ACDX_STORED_FP_ITEM_ID stateId);

    const StoredStateItem* createStoredStateItem(ACDX_STORED_FP_ITEM_ID stateId) const;

    void restoreStoredStateItem(const StoredStateItem* ssi);

    ~ACDXFixedPipelineStateImp();

private:

    ACDXGLState* _gls;

    friend class ACDXLightImp;
    friend class ACDXMaterialImp;

    std::vector<ACDXLightImp*> _light;
    std::vector<ACDXMaterialImp*> _material;

    acd_bool _requiredSync;

    //////////////////////////
    // Internal State Items //
    //////////////////////////

    // Surface Material

    StateItem<ACDXFloatVector4> _materialFrontAmbient;
    StateItem<ACDXFloatVector4> _materialFrontDiffuse;
    StateItem<ACDXFloatVector4> _materialFrontSpecular;
    StateItem<ACDXFloatVector4> _materialFrontEmission;
    StateItem<acd_float>        _materialFrontShininess;
    
    StateItem<ACDXFloatVector4> _materialBackAmbient;
    StateItem<ACDXFloatVector4> _materialBackDiffuse;
    StateItem<ACDXFloatVector4> _materialBackSpecular;
    StateItem<ACDXFloatVector4> _materialBackEmission;
    StateItem<acd_float>        _materialBackShininess;

    // Light properties

    std::vector<StateItem<ACDXFloatVector4> > _lightAmbient;
    std::vector<StateItem<ACDXFloatVector4> > _lightDiffuse;
    std::vector<StateItem<ACDXFloatVector4> > _lightSpecular;
    std::vector<StateItem<ACDXFloatVector4> > _lightPosition;
    std::vector<StateItem<ACDXFloatVector3> > _lightDirection;
    std::vector<StateItem<ACDXFloatVector3> > _lightAttenuation;
    std::vector<StateItem<ACDXFloatVector3> > _lightSpotDirection;
    std::vector<StateItem<acd_float> >          _lightSpotCutOffAngle;
    std::vector<StateItem<acd_float> >          _lightSpotExponent;

    // Light model properties

    StateItem<ACDXFloatVector4> _lightModelAmbientColor;
    StateItem<ACDXFloatVector4> _lightModelSceneFrontColor;
    StateItem<ACDXFloatVector4> _lightModelSceneBackColor;
    
    // Transformation matrices

    std::vector<StateItem<ACDXFloatMatrix4x4> > _modelViewMatrix;
    StateItem<ACDXFloatMatrix4x4>               _projectionMatrix;

    // Texture coordinate generation planes

    std::vector<StateItem<ACDXFloatVector4> > _textCoordSObjectPlane;
    std::vector<StateItem<ACDXFloatVector4> > _textCoordTObjectPlane;
    std::vector<StateItem<ACDXFloatVector4> > _textCoordRObjectPlane;
    std::vector<StateItem<ACDXFloatVector4> > _textCoordQObjectPlane;

    std::vector<StateItem<ACDXFloatVector4> > _textCoordSEyePlane;
    std::vector<StateItem<ACDXFloatVector4> > _textCoordTEyePlane;
    std::vector<StateItem<ACDXFloatVector4> > _textCoordREyePlane;
    std::vector<StateItem<ACDXFloatVector4> > _textCoordQEyePlane;

    std::vector<StateItem<ACDXFloatMatrix4x4> > _textureCoordMatrix;

    // Texture stage environment colors

    std::vector<StateItem<ACDXFloatVector4> > _textEnvColor;

    // Depth range

    StateItem<acd_float> _near;
    StateItem<acd_float> _far;

    // FOG params

    StateItem<ACDXFloatVector4> _fogBlendColor;
    StateItem<acd_float>        _fogDensity;
    StateItem<acd_float>        _fogLinearStart;
    StateItem<acd_float>        _fogLinearEnd;

    // Alpha test params

    StateItem<acd_float>        _alphaTestRefValue;
};

} // namespace acdlib

#endif // ACDX_FIXED_PIPELINE_STATE_IMP_H
