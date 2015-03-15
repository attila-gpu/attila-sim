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

#include "PushState.h"
#include "GLContext.h"
#include "TextureManager.h"
#include "GLState.h"
#include "glext.h"

using namespace std;
using namespace libgl; // To use directly GLContext
using namespace libgl::pushstate;
using namespace libgl::glsNS;

#define SET_RESET_FLAGS(flag)   if (flag) ctx.setFlags(GLContext::flag); else ctx.resetFlags(GLContext::flag);

void pushstate::pushAttrib(TstateStack& stateStack, const GLContext& ctx, const TextureManager& tm, GLbitfield mask)
{
    GroupContainerMemento* cont = new GroupContainerMemento;    // Join together attrib groups in a group container.
    
    if ( ( mask & GL_ENABLE_BIT ) == GL_ENABLE_BIT )
    {
         EnableGroup* group = new EnableGroup;
         group->saveState(ctx,tm);
         cont->addAttribGroup(group);
    }
    if ( ( mask & GL_COLOR_BUFFER_BIT ) == GL_COLOR_BUFFER_BIT )
    {
         ColorGroup* group = new ColorGroup;
         group->saveState(ctx,tm);
         cont->addAttribGroup(group);
    }
    if ( ( mask & GL_CURRENT_BIT ) == GL_CURRENT_BIT )
    {
         CurrentGroup* group = new CurrentGroup;
         group->saveState(ctx,tm);
         cont->addAttribGroup(group);
    }
    if ( ( mask & GL_DEPTH_BUFFER_BIT ) == GL_DEPTH_BUFFER_BIT )
    {
         DepthBufferGroup* group = new DepthBufferGroup;
         group->saveState(ctx,tm);
         cont->addAttribGroup(group);
    }
    if ( ( mask & GL_FOG_BIT ) == GL_FOG_BIT )
    {
         FogGroup* group = new FogGroup;
         group->saveState(ctx,tm);
         cont->addAttribGroup(group);
    }
    if ( ( mask & GL_LIGHTING_BIT ) == GL_LIGHTING_BIT )
    {
         LightingGroup* group = new LightingGroup;
         group->saveState(ctx,tm);
         cont->addAttribGroup(group);
    }
    if ( ( mask & GL_POLYGON_BIT ) == GL_POLYGON_BIT )
    {
         PolygonGroup* group = new PolygonGroup;
         group->saveState(ctx,tm);
         cont->addAttribGroup(group);
    }           
    if ( ( mask & GL_SCISSOR_BIT ) == GL_SCISSOR_BIT )
    {
         ScissorGroup* group = new ScissorGroup;
         group->saveState(ctx,tm);
         cont->addAttribGroup(group);
    }
    if ( ( mask & GL_STENCIL_BUFFER_BIT ) == GL_STENCIL_BUFFER_BIT )
    {
         StencilGroup* group = new StencilGroup;
         group->saveState(ctx,tm);
         cont->addAttribGroup(group);
    }
    if ( ( mask & GL_TEXTURE_BIT ) == GL_TEXTURE_BIT )
    {
         TextureGroup* group = new TextureGroup;
         group->saveState(ctx,tm);
         cont->addAttribGroup(group);
    }
    if ( ( mask & GL_TRANSFORM_BIT ) == GL_TRANSFORM_BIT )
    {
         TransformGroup* group = new TransformGroup;
         group->saveState(ctx,tm);
         cont->addAttribGroup(group);
    }
    if ( ( mask & GL_VIEWPORT_BIT ) == GL_VIEWPORT_BIT )
    {
         ViewportGroup* group = new ViewportGroup;
         group->saveState(ctx,tm);
         cont->addAttribGroup(group);
    }
    if ( ( mask & GL_ACCUM_BUFFER_BIT ) == GL_ACCUM_BUFFER_BIT && ( mask & GL_ALL_ATTRIB_BITS ) != GL_ALL_ATTRIB_BITS )
    {
        //panic("GLContext","pushAttribs","Accum Buffer Group not implemented yet");
    }
    if ( ( mask & GL_EVAL_BIT ) == GL_EVAL_BIT && ( mask & GL_ALL_ATTRIB_BITS ) != GL_ALL_ATTRIB_BITS)
    {
        //panic("GLContext","pushAttribs","Eval Group not implemented yet");
    }
    if ( ( mask & GL_HINT_BIT ) == GL_HINT_BIT && ( mask & GL_ALL_ATTRIB_BITS ) != GL_ALL_ATTRIB_BITS)
    {
        //panic("GLContext","pushAttribs","Hint Group not implemented yet");
    }
    if ( ( mask & GL_LINE_BIT ) == GL_LINE_BIT && ( mask & GL_ALL_ATTRIB_BITS ) != GL_ALL_ATTRIB_BITS)
    {
        //panic("GLContext","pushAttribs","Line Group not implemented yet");
    }
    if ( ( mask & GL_MULTISAMPLE_BIT ) == GL_MULTISAMPLE_BIT && ( mask & GL_ALL_ATTRIB_BITS ) != GL_ALL_ATTRIB_BITS)
    {
        //panic("GLContext","pushAttribs","Multisample Group not implemented yet");
    }
    if ( ( mask & GL_PIXEL_MODE_BIT ) == GL_PIXEL_MODE_BIT && ( mask & GL_ALL_ATTRIB_BITS ) != GL_ALL_ATTRIB_BITS)
    {
        //panic("GLContext","pushAttribs","Pixel Mode Group not implemented yet");
    }
    if ( ( mask & GL_POINT_BIT ) == GL_POINT_BIT && ( mask & GL_ALL_ATTRIB_BITS ) != GL_ALL_ATTRIB_BITS)
    {
        //panic("GLContext","pushAttribs","Point Group not implemented yet");
    }
    if ( ( mask & GL_POLYGON_STIPPLE_BIT ) == GL_POLYGON_STIPPLE_BIT && ( mask & GL_ALL_ATTRIB_BITS ) != GL_ALL_ATTRIB_BITS)
    {
        //panic("GLContext","pushAttribs","Polygon Stipple Group not implemented yet");
    }
    
    if ( mask > GL_ALL_ATTRIB_BITS )
        panic("GLContext","pushAttribs","unexpected Attrib Group");
    
    stateStack.push(cont);    
}

void pushstate::popAttrib(TstateStack& stateStack, GLContext& ctx, TextureManager& tm)
{
    if (stateStack.empty())
        panic("PushState","popAttrib()","Stack underflow. The PushState stack is empty");
        
    GroupContainerMemento* cont = stateStack.top();
    
    cont->restoreState(ctx, tm);
    
    cont->syncState(ctx); // Sends inmediately all the state changes to GPU. This responsibility could be
                          // left to GLContext in the future.
                               
    delete cont; // Internal AttribGroups are deleted in the redefined destructor of GroupContainerMemento.
    stateStack.pop();
}   

void GroupContainerMemento::addAttribGroup(AttribGroup* group)
{
    groupList.push_back(group);
}

void GroupContainerMemento::restoreState(GLContext& ctx, TextureManager& tm) const
{
    list<AttribGroup*>::const_iterator iter = groupList.begin();
    
    while ( iter != groupList.end() )
    {
        (*iter)->restoreState(ctx, tm);
        iter++;
    }
}

void GroupContainerMemento::syncState(GLContext &ctx) const
{
    list<AttribGroup*>::const_iterator iter = groupList.begin();
    
    while ( iter != groupList.end() )
    {
        (*iter)->syncState(ctx);
        iter++;
    }
}

GroupContainerMemento::~GroupContainerMemento()
{
    list<AttribGroup*>::iterator iter = groupList.begin();
    
    while ( iter != groupList.end() )
    {
        delete (*iter);
        iter++;
    }
}

/*****************************************************************
 *                Save State Implementations                     *
 *****************************************************************/
 
void EnableGroup::saveState(const GLContext& ctx, const TextureManager& tm)
{
    flagSeparateSpecular = ctx.testFlags(GLContext::flagSeparateSpecular);
    flagRescaleNormal = ctx.testFlags(GLContext::flagRescaleNormal);
    flagNormalize = ctx.testFlags(GLContext::flagNormalize);
    flagFog = ctx.testFlags(GLContext::flagFog);
    flagStencilTest = ctx.testFlags(GLContext::flagStencilTest);
    flagAlphaTest = ctx.testFlags(GLContext::flagAlphaTest);
    flagCullFace = ctx.testFlags(GLContext::flagCullFace);
    flagDepthTest = ctx.testFlags(GLContext::flagDepthTest);
    flagPolygonOffsetFill = ctx.testFlags(GLContext::flagPolygonOffsetFill);
    flagScissorTest = ctx.testFlags(GLContext::flagScissorTest);
    flagBlending = ctx.testFlags(GLContext::flagBlending);
    lights = ctx.lights;
}
        
void ColorGroup::saveState(const GLContext& ctx, const TextureManager& tm)
{
    flagAlphaTest = ctx.testFlags(GLContext::flagAlphaTest);
    alphaFunc = ctx.alphaFunc;
    alphaRef = ctx.alphaRef;
    flagBlending = ctx.testFlags(GLContext::flagBlending);
    blendSFactorRGB = ctx.blendSFactorRGB;
    blendDFactorRGB = ctx.blendDFactorRGB;
    blendSFactorAlpha = ctx.blendSFactorAlpha;
    blendDFactorAlpha = ctx.blendDFactorAlpha;
    blendEquation = ctx.blendEquation;
    blendColor = ctx.blendColor;
    red = ctx.red;
    green = ctx.green;
    blue = ctx.blue;
    alpha = ctx.alpha;
    clearColor = ctx.clearColor;
}
        

void CurrentGroup::saveState(const GLContext& ctx, const TextureManager& tm)
{
    currentColor = ctx.currentColor;
    currentNormal = ctx.currentNormal;
    
    GLuint nTUs = ctx.driver->getTextureUnits();
    
    for (unsigned int i=0; i < nTUs; i++)
        currentTexCoord.push_back(ctx.currentTexCoord[i]);
}
        

void DepthBufferGroup::saveState(const GLContext& ctx, const TextureManager& tm)
{
     flagDepthTest = ctx.testFlags(GLContext::flagDepthTest);
     depthFunc = ctx.depthFunc;
     depthMask = ctx.depthMask;
     depthClear = ctx.depthClear;
}
        

void FogGroup::saveState(const GLContext& ctx, const TextureManager& tm)
{
    using namespace glsNS;
    
    flagFog = ctx.testFlags(GLContext::flagFog);
    fogColor = ctx.gls.getVector(V_FOG_COLOR);
    fogParams = ctx.gls.getVector(V_FOG_PARAMS);
    fogMode = ctx.fogMode;
    fogCoordinateSource = ctx.fogCoordinateSource;
}
        

void LightingGroup::saveState(const GLContext& ctx, const TextureManager& tm)
{
    using namespace glsNS;
    
    shadeModel = ctx.shadeModel;
    flagLighting = ctx.testFlags(GLContext::flagLighting);
    flagColorMaterial = ctx.testFlags(GLContext::flagColorMaterial);
    colorMaterialMode = ctx.colorMaterialMode;
    colorMaterialFace = ctx.colorMaterialFace;
    ambientFront = ctx.gls.getVector(V_MATERIAL_FRONT_AMBIENT);
    diffuseFront = ctx.gls.getVector(V_MATERIAL_FRONT_DIFUSSE);
    specularFront = ctx.gls.getVector(V_MATERIAL_FRONT_SPECULAR);
    emissionFront = ctx.gls.getVector(V_MATERIAL_FRONT_EMISSION);
    shininessFront = ctx.gls.getVector(V_MATERIAL_FRONT_SHININESS);
    ambientBack = ctx.gls.getVector(V_MATERIAL_BACK_AMBIENT);
    diffuseBack = ctx.gls.getVector(V_MATERIAL_BACK_DIFUSSE);
    specularBack = ctx.gls.getVector(V_MATERIAL_BACK_SPECULAR);
    emissionBack = ctx.gls.getVector(V_MATERIAL_BACK_EMISSION);
    shininessBack = ctx.gls.getVector(V_MATERIAL_BACK_SHININESS);
    lightModelAmbient = ctx.gls.getVector(V_LIGHTMODEL_AMBIENT);
    flagLocalViewer = ctx.testFlags(GLContext::flagLocalViewer);
    flagTwoSidedLighting = ctx.testFlags(GLContext::flagTwoSidedLighting);
    flagSeparateSpecular = ctx.testFlags(GLContext::flagSeparateSpecular);
    
    for (int i=0; i < MAX_LIGHTS_ARB; i++)
    {
        ambient[i] = ctx.gls.getVector(V_LIGHT_AMBIENT + 7*i);
        diffuse[i] = ctx.gls.getVector(V_LIGHT_DIFFUSE + 7*i);
        specular[i] = ctx.gls.getVector(V_LIGHT_SPECULAR + 7*i);
        position[i] = ctx.gls.getVector(V_LIGHT_POSITION + 7*i);
        attenuation[i] = ctx.gls.getVector(V_LIGHT_ATTENUATION + 7*i);
        spotParams[i] = ctx.gls.getVector(V_LIGHT_SPOT_DIRECTION + 7*i);
        
    }

    lights = ctx.lights;

}
        

void PolygonGroup::saveState(const GLContext& ctx, const TextureManager& tm)
{
    flagCullFace = ctx.testFlags(GLContext::flagCullFace);
    cullFace = ctx.cullFace;
    faceMode = ctx.faceMode;
    slopeFactor = ctx.slopeFactor;
    unitsFactor = ctx.unitsFactor;
    flagPolygonOffsetFill = ctx.testFlags(GLContext::flagPolygonOffsetFill);
}
        

void ScissorGroup::saveState(const GLContext& ctx, const TextureManager& tm)
{
    flagScissorTest = ctx.testFlags(GLContext::flagScissorTest);
    scissorIniX = ctx.scissorIniX;
    scissorIniY = ctx.scissorIniY;
    scissorWidth = ctx.scissorWidth;
    scissorHeight = ctx.scissorHeight;
}
        

void StencilGroup::saveState(const GLContext& ctx, const TextureManager& tm)
{
    flagStencilTest = ctx.testFlags(GLContext::flagStencilTest);
    stFailUpdateFunc = ctx.stFailUpdateFunc;
    dpFailUpdateFunc = ctx.dpFailUpdateFunc;
    dpPassUpdateFunc =  ctx.dpPassUpdateFunc;
    stBufferClearValue = ctx.stBufferClearValue;
    stBufferUpdateMask = ctx.stBufferUpdateMask;
    stBufferFunc = ctx.stBufferFunc;
    stBufferRefValue = ctx.stBufferRefValue;
    stBufferCompMask = ctx.stBufferCompMask;
}
        

void TransformGroup::saveState(const GLContext& ctx, const TextureManager& tm)
{
    currentMatrix = ctx.currentMatrix;
    flagNormalize = ctx.testFlags(GLContext::flagNormalize);
    flagRescaleNormal = ctx.testFlags(GLContext::flagRescaleNormal);
}


        
void ViewportGroup::saveState(const GLContext& ctx, const TextureManager& tm)
{
    viewportX = ctx.viewportX;
    viewportY = ctx.viewportY;
    viewportWidth = ctx.viewportWidth;
    viewportHeight = ctx.viewportHeight;
    near_ = ctx.near_;
    far_ = ctx.far_;
}

        
        
void TextureGroup::saveState(const GLContext& ctx, const TextureManager& tm)
{
    activeTextureUnit = tm.getCurrentGroup();
    
    /****************************************
     *   Save state for each texture unit
     ****************************************/
     
    vector<TextureUnit>::const_iterator unit = ctx.textureUnits.begin();
    
    unsigned int i = 0;
    
    while (unit != ctx.textureUnits.end())
    {
        map<GLenum,GLboolean> aux;
        aux.insert(make_pair(GL_TEXTURE_1D, unit->flags[0]? GL_TRUE : GL_FALSE));
        aux.insert(make_pair(GL_TEXTURE_2D, unit->flags[1]? GL_TRUE : GL_FALSE));
        aux.insert(make_pair(GL_TEXTURE_3D, unit->flags[2]? GL_TRUE : GL_FALSE));
        aux.insert(make_pair(GL_TEXTURE_CUBE_MAP, unit->flags[3]? GL_TRUE : GL_FALSE));
        enabledTargets.push_back(aux);
        
        map<GLenum,GLuint> aux2;
        
        if (unit->texObjs[0])
            aux2.insert(make_pair(GL_TEXTURE_1D, unit->texObjs[0]->getName()));
        else
            aux2.insert(make_pair(GL_TEXTURE_1D, 0 ));

        if (unit->texObjs[1])
            aux2.insert(make_pair(GL_TEXTURE_2D, unit->texObjs[1]->getName()));
        else
            aux2.insert(make_pair(GL_TEXTURE_2D, 0 ));

        if (unit->texObjs[2])
            aux2.insert(make_pair(GL_TEXTURE_3D, unit->texObjs[2]->getName()));
        else
            aux2.insert(make_pair(GL_TEXTURE_3D, 0 ));

        if (unit->texObjs[3])
            aux2.insert(make_pair(GL_TEXTURE_CUBE_MAP, unit->texObjs[3]->getName()));
        else
            aux2.insert(make_pair(GL_TEXTURE_CUBE_MAP, 0 ));

        textureObjectBounds.push_back(aux2);
        
        /***************************************************************************************
         *   Save state for each texture object currently bound to each texture unitïs targets
         ***************************************************************************************/
     
        TextureObject* to;
        
        for (unsigned int t=0; t<4; t++)
        {
            to = unit->texObjs[t];
            if (to)
            {
                // Avoid inserting the same texture object information twice.
                if (textureBorderColors.find(to->getName()) == textureBorderColors.end())
                {
                    textureBorderColors.insert(make_pair(to->getName(), Quadf(to->borderColor[0],
                                                                              to->borderColor[1],
                                                                              to->borderColor[2],
                                                                              to->borderColor[3])));
                    textureMinFilters.insert(make_pair(to->getName(), to->minFilter));
                    textureMagFilters.insert(make_pair(to->getName(), to->magFilter));
                    textureObjectPriority.insert(make_pair(to->getName(), to->texturePriority));
                    //textureResidency.insert(make_pair(unit->texObjs[t]->getName(),  ));
                    textureMinLODs.insert(make_pair(to->getName(), to->minLOD));
                    textureMaxLODs.insert(make_pair(to->getName(), to->maxLOD));
                    textureBaseLevels.insert(make_pair(to->getName(), to->baseLevel));
                    textureMaxLevels.insert(make_pair(to->getName(), to->maxLevel));
                    textureLODBias.insert(make_pair(to->getName(), to->biasLOD));
                    textureDepthModes.insert(make_pair(to->getName(), to->depthTextureMode));
                    textureCompareModes.insert(make_pair(to->getName(), to->compareMode));
                    textureCompareFuncs.insert(make_pair(to->getName(), to->compareFunc));
                    textureAutoGenMipMap.insert(make_pair(to->getName(), to->generateMipmap));
                 }
            }
        }        
        
        aux.clear();
        
        aux.insert(make_pair(GL_TEXTURE_GEN_S, unit->activeTexGenS? GL_TRUE : GL_FALSE));
        aux.insert(make_pair(GL_TEXTURE_GEN_T, unit->activeTexGenT? GL_TRUE : GL_FALSE));
        aux.insert(make_pair(GL_TEXTURE_GEN_R, unit->activeTexGenR? GL_TRUE : GL_FALSE));
        aux.insert(make_pair(GL_TEXTURE_GEN_Q, unit->activeTexGenQ? GL_TRUE : GL_FALSE));
        texGenEnabled.push_back(aux);
        
        textureEnvModes.push_back(unit->texFunc);
        
        textureEnvColors.push_back(ctx.gls.getVector(V_TEXENV_COLOR + i));
        
        textureUnitLODBias.push_back(unit->lodBias);    
        
        map<GLenum, Quadf> aux3;

        aux3.insert(make_pair(GL_TEXTURE_GEN_S, ctx.gls.getVector(V_TEXGEN_EYE_S + 8*i)));
        aux3.insert(make_pair(GL_TEXTURE_GEN_T, ctx.gls.getVector(V_TEXGEN_EYE_T + 8*i)));
        aux3.insert(make_pair(GL_TEXTURE_GEN_R, ctx.gls.getVector(V_TEXGEN_EYE_R + 8*i)));
        aux3.insert(make_pair(GL_TEXTURE_GEN_Q, ctx.gls.getVector(V_TEXGEN_EYE_Q + 8*i)));
        texGenPlaneEqCoefs.push_back(aux3);
        
        aux3.clear();
        aux3.insert(make_pair(GL_TEXTURE_GEN_S, ctx.gls.getVector(V_TEXGEN_OBJECT_S + 8*i)));
        aux3.insert(make_pair(GL_TEXTURE_GEN_T, ctx.gls.getVector(V_TEXGEN_OBJECT_T + 8*i)));
        aux3.insert(make_pair(GL_TEXTURE_GEN_R, ctx.gls.getVector(V_TEXGEN_OBJECT_R + 8*i)));
        aux3.insert(make_pair(GL_TEXTURE_GEN_Q, ctx.gls.getVector(V_TEXGEN_OBJECT_Q + 8*i)));
        texGenObjEqCoefs.push_back(aux3);
        
        map<GLenum, GLenum> aux4;
        aux4.insert(make_pair(GL_TEXTURE_GEN_S, unit->texGenModeS));
        aux4.insert(make_pair(GL_TEXTURE_GEN_T, unit->texGenModeT));
        aux4.insert(make_pair(GL_TEXTURE_GEN_R, unit->texGenModeR));
        aux4.insert(make_pair(GL_TEXTURE_GEN_Q, unit->texGenModeQ));
        texGenModes.push_back(aux4);
        
        texEnvCombRGB.push_back(unit->rgbCombinerFunc);
        texEnvCombAlpha.push_back(unit->alphaCombinerFunc);
        
        aux4.clear();
        aux4.insert(make_pair(GL_SOURCE0_RGB, unit->srcRGB[0]));
        aux4.insert(make_pair(GL_SOURCE1_RGB, unit->srcRGB[1]));
        aux4.insert(make_pair(GL_SOURCE2_RGB, unit->srcRGB[2]));
        aux4.insert(make_pair(GL_SOURCE3_RGB_NV, unit->srcRGB[3]));
        texEnvSourcesRGB.push_back(aux4);
        
        aux4.clear();
        aux4.insert(make_pair(GL_SOURCE0_ALPHA, unit->srcAlpha[0]));
        aux4.insert(make_pair(GL_SOURCE1_ALPHA, unit->srcAlpha[1]));
        aux4.insert(make_pair(GL_SOURCE2_ALPHA, unit->srcAlpha[2]));
        aux4.insert(make_pair(GL_SOURCE3_ALPHA_NV, unit->srcAlpha[3]));
        texEnvSourcesAlpha.push_back(aux4);
        
        aux4.clear();
        aux4.insert(make_pair(GL_OPERAND0_RGB, unit->operandRGB[0]));
        aux4.insert(make_pair(GL_OPERAND1_RGB, unit->operandRGB[1]));
        aux4.insert(make_pair(GL_OPERAND2_RGB, unit->operandRGB[2]));
        aux4.insert(make_pair(GL_OPERAND3_RGB_NV, unit->operandRGB[3]));
        texEnvOperandsRGB.push_back(aux4);
        
        aux4.clear();
        aux4.insert(make_pair(GL_OPERAND0_ALPHA, unit->operandAlpha[0]));
        aux4.insert(make_pair(GL_OPERAND1_ALPHA, unit->operandAlpha[1]));
        aux4.insert(make_pair(GL_OPERAND2_ALPHA, unit->operandAlpha[2]));
        aux4.insert(make_pair(GL_OPERAND3_ALPHA_NV, unit->operandAlpha[3]));
        texEnvOperandsAlpha.push_back(aux4);
        
        texScaleRGB.push_back(unit->rgbScale);
        texScaleAlpha.push_back(unit->alphaScale);
        
        i++;
        unit++;
    }
}

/*****************************************************************
 *             Restore State Implementations                     *
 *****************************************************************/
 
void EnableGroup::restoreState(GLContext& ctx, TextureManager& tm) const
{
    SET_RESET_FLAGS(flagSeparateSpecular) 
    SET_RESET_FLAGS(flagRescaleNormal) 
    SET_RESET_FLAGS(flagNormalize) 
    SET_RESET_FLAGS(flagFog) 
    SET_RESET_FLAGS(flagStencilTest) 
    SET_RESET_FLAGS(flagAlphaTest) 
    SET_RESET_FLAGS(flagCullFace) 
    SET_RESET_FLAGS(flagDepthTest) 
    SET_RESET_FLAGS(flagPolygonOffsetFill) 
    SET_RESET_FLAGS(flagScissorTest) 
    SET_RESET_FLAGS(flagBlending) 
    ctx.lights = lights;                  
} 

void ColorGroup::restoreState(GLContext& ctx, TextureManager& tm) const
{
    SET_RESET_FLAGS(flagAlphaTest) 
    ctx.alphaFunc = alphaFunc;
    ctx.alphaRef = alphaRef;
    SET_RESET_FLAGS(flagBlending) 
    ctx.blendSFactorRGB = blendSFactorRGB;
    ctx.blendDFactorRGB = blendDFactorRGB;
    ctx.blendSFactorAlpha = blendSFactorAlpha;
    ctx.blendDFactorAlpha = blendDFactorAlpha;
    ctx.blendEquation = blendEquation;
    ctx.blendColor = blendColor;
    ctx.red = red;
    ctx.green = green;
    ctx.blue = blue;
    ctx.alpha = alpha;
    ctx.clearColor = clearColor;
}

void CurrentGroup::restoreState(GLContext& ctx, TextureManager& tm) const
{
    ctx.currentColor = currentColor;
    ctx.currentNormal = currentNormal;
    
    GLuint nTUs = ctx.driver->getTextureUnits();
    
    for (unsigned int i=0; i < nTUs; i++)
        ctx.currentTexCoord[i] = currentTexCoord[i];
}

void DepthBufferGroup::restoreState(GLContext& ctx, TextureManager& tm) const
{
     SET_RESET_FLAGS(flagDepthTest) 
     ctx.depthFunc = depthFunc;
     ctx.depthMask = depthMask;
     ctx.depthClear = depthClear;
}

void FogGroup::restoreState(GLContext& ctx, TextureManager& tm) const
{
    using namespace glsNS;
    
    SET_RESET_FLAGS(flagFog) 
    ctx.gls.setVector(fogColor, V_FOG_COLOR);
    ctx.gls.setVector(fogParams, V_FOG_PARAMS, 0x07);
    ctx.fogMode = fogMode;
    ctx.fogCoordinateSource = fogCoordinateSource;
}

void LightingGroup::restoreState(GLContext& ctx, TextureManager& tm) const
{
    using namespace glsNS;
    
    ctx.shadeModel = shadeModel;
    SET_RESET_FLAGS(flagLighting)
    SET_RESET_FLAGS(flagColorMaterial)
    ctx.colorMaterialMode = colorMaterialMode;
    ctx.colorMaterialFace = colorMaterialFace;
    ctx.gls.setVector(ambientFront, V_MATERIAL_FRONT_AMBIENT);
    ctx.gls.setVector(diffuseFront, V_MATERIAL_FRONT_DIFUSSE);
    ctx.gls.setVector(specularFront, V_MATERIAL_FRONT_SPECULAR);
    ctx.gls.setVector(emissionFront, V_MATERIAL_FRONT_EMISSION);
    ctx.gls.setVector(shininessFront, V_MATERIAL_FRONT_SHININESS);
    ctx.gls.setVector(ambientBack, V_MATERIAL_BACK_AMBIENT);
    ctx.gls.setVector(diffuseBack, V_MATERIAL_BACK_DIFUSSE);
    ctx.gls.setVector(specularBack, V_MATERIAL_BACK_SPECULAR);
    ctx.gls.setVector(emissionBack, V_MATERIAL_BACK_EMISSION);
    ctx.gls.setVector(shininessBack, V_MATERIAL_BACK_SHININESS);
    ctx.gls.setVector(lightModelAmbient, V_LIGHTMODEL_AMBIENT);
    SET_RESET_FLAGS(flagLocalViewer)
    SET_RESET_FLAGS(flagTwoSidedLighting)
    SET_RESET_FLAGS(flagSeparateSpecular)
    
    for (int i=0; i < MAX_LIGHTS_ARB; i++)
    {
        ctx.gls.setVectorGroup(ambient[i], V_LIGHT_AMBIENT, i);
        ctx.gls.setVectorGroup(diffuse[i], V_LIGHT_DIFFUSE, i);
        ctx.gls.setVectorGroup(specular[i], V_LIGHT_SPECULAR, i);
        ctx.gls.setVectorGroup(position[i], V_LIGHT_POSITION, i);
        ctx.gls.setVectorGroup(attenuation[i], V_LIGHT_ATTENUATION, i);
        ctx.gls.setVectorGroup(spotParams[i], V_LIGHT_SPOT_DIRECTION, i);
    }
    
    ctx.lights = lights;
}

void PolygonGroup::restoreState(GLContext& ctx, TextureManager& tm) const
{
    SET_RESET_FLAGS(flagCullFace)
    ctx.cullFace = cullFace;
    ctx.faceMode = faceMode;
    ctx.slopeFactor = slopeFactor;
    ctx.unitsFactor = unitsFactor;
    SET_RESET_FLAGS(flagPolygonOffsetFill);
}

void ScissorGroup::restoreState(GLContext& ctx, TextureManager& tm) const
{
    SET_RESET_FLAGS(flagScissorTest)
    ctx.scissorIniX = scissorIniX;
    ctx.scissorIniY = scissorIniY;
    ctx.scissorWidth = scissorWidth;
    ctx.scissorHeight = scissorHeight;
}

void StencilGroup::restoreState(GLContext& ctx, TextureManager& tm) const
{
    SET_RESET_FLAGS(flagStencilTest);
    ctx.stFailUpdateFunc = stFailUpdateFunc;
    ctx.dpFailUpdateFunc = dpFailUpdateFunc;
    ctx.dpPassUpdateFunc = dpPassUpdateFunc;
    ctx.stBufferClearValue = stBufferClearValue;
    ctx.stBufferUpdateMask = stBufferUpdateMask;
    ctx.stBufferFunc = stBufferFunc;
    ctx.stBufferRefValue = stBufferRefValue;
    ctx.stBufferCompMask = stBufferCompMask;
}

void TransformGroup::restoreState(GLContext& ctx, TextureManager& tm) const
{
    ctx.currentMatrix = currentMatrix;
    SET_RESET_FLAGS(flagNormalize);
    SET_RESET_FLAGS(flagRescaleNormal);
}

void ViewportGroup::restoreState(GLContext& ctx, TextureManager& tm) const
{
    ctx.viewportX = viewportX;
    ctx.viewportY = viewportY;
    ctx.viewportWidth = viewportWidth;
    ctx.viewportHeight = viewportHeight;
    ctx.near_ = near_;
    ctx.far_ = far_;
}

void TextureGroup::restoreState(GLContext& ctx, TextureManager& tm) const
{
    tm.selectGroup(activeTextureUnit);
    
    /****************************************
     *   Restore state for each texture unit
     ****************************************/
     
    //vector<TextureUnit>::reverse_iterator unit = ctx.textureUnits.rend();
    vector<TextureUnit>::iterator unit = ctx.textureUnits.begin();
    
    unsigned int i = ctx.textureUnits.size() - 1;
    
    GLuint name;
    TextureObject *texObj;
    bool successfulrestored[4];
    
    for (int j=0; j<4; j++)
        successfulrestored[j] = false;
    
//    while (unit != ctx.textureUnits.rbegin())
    while (unit != ctx.textureUnits.end())
    {
        unit->flags[0] = (enabledTargets[i].find(GL_TEXTURE_1D)->second) == 0 ? false : true ;
        unit->flags[1] = (enabledTargets[i].find(GL_TEXTURE_2D)->second) == 0 ? false : true ;
        unit->flags[2] = (enabledTargets[i].find(GL_TEXTURE_3D)->second) == 0 ? false : true ;
        unit->flags[3] = (enabledTargets[i].find(GL_TEXTURE_CUBE_MAP)->second) == 0 ? false : true ;    
        
        /**
         *  Each texture object restored name if checked against current texture objects if it no longer exists.
         *  In this case the texture object binding for this target is left unaltered.
         */
         
        name = textureObjectBounds[i].find(GL_TEXTURE_1D)->second;
        
        if (texObj = static_cast<TextureObject*>(tm.findObject(name))) 
        {
            unit->texObjs[0] = texObj;
            successfulrestored[0] = true;
        }
        
        name = textureObjectBounds[i].find(GL_TEXTURE_2D)->second;
        
        if (texObj = static_cast<TextureObject*>(tm.findObject(name))) 
        {
            unit->texObjs[1] = texObj;
            successfulrestored[1] = true;
        }
        
        name = textureObjectBounds[i].find(GL_TEXTURE_3D)->second;
        
        if (texObj = static_cast<TextureObject*>(tm.findObject(name))) 
        {
            unit->texObjs[2] = texObj;
            successfulrestored[2] = true;
        }
        
        name = textureObjectBounds[i].find(GL_TEXTURE_CUBE_MAP)->second;
        
        if (texObj = static_cast<TextureObject*>(tm.findObject(name))) 
        {
            unit->texObjs[3] = texObj;
            successfulrestored[3] = true;
        }

        /********************************************************************
         *   Restore state for each texture object restored above.
         ********************************************************************/
     
        TextureObject* to;
        
        for (unsigned int t=0; t<4; t++)
        {
            to = unit->texObjs[t];
            
            if (to && successfulrestored[t])
            {
                Quadf borderColor = textureBorderColors.find(to->getName())->second;
                
                to->borderColor[0] = borderColor[0];
                to->borderColor[1] = borderColor[1];
                to->borderColor[2] = borderColor[2];
                to->borderColor[3] = borderColor[3];
                
                to->minFilter = textureMinFilters.find(to->getName())->second;
                to->magFilter = textureMagFilters.find(to->getName())->second;
                to->texturePriority = textureObjectPriority.find(to->getName())->second;
                //textureResidency.insert(make_pair(unit->texObjs[t]->getName(),  ));
                to->minLOD = textureMinLODs.find(to->getName())->second;
                to->maxLOD = textureMaxLODs.find(to->getName())->second;
                to->baseLevel = textureBaseLevels.find(to->getName())->second;
                to->maxLevel = textureMaxLevels.find(to->getName())->second;
                to->biasLOD = textureLODBias.find(to->getName())->second;
                to->depthTextureMode = textureDepthModes.find(to->getName())->second;
                to->compareMode = textureCompareModes.find(to->getName())->second;
                to->compareFunc = textureCompareFuncs.find(to->getName())->second;
                to->generateMipmap = textureAutoGenMipMap.find(to->getName())->second;
            }
        }        
                    
        unit->activeTexGenS = (texGenEnabled[i].find(GL_TEXTURE_GEN_S)->second) == 0 ? false : true ;
        unit->activeTexGenT = (texGenEnabled[i].find(GL_TEXTURE_GEN_T)->second) == 0 ? false : true ;
        unit->activeTexGenR = (texGenEnabled[i].find(GL_TEXTURE_GEN_R)->second) == 0 ? false : true ;
        unit->activeTexGenQ = (texGenEnabled[i].find(GL_TEXTURE_GEN_Q)->second) == 0 ? false : true ;
        
        unit->texFunc = textureEnvModes[i];
        
        ctx.gls.setVector(textureEnvColors[i], V_TEXENV_COLOR, i);
        
        unit->lodBias = textureUnitLODBias[i];    
        
        ctx.gls.setVector(texGenPlaneEqCoefs[i].find(GL_TEXTURE_GEN_S)->second, V_TEXGEN_EYE_S, i);
        ctx.gls.setVector(texGenPlaneEqCoefs[i].find(GL_TEXTURE_GEN_T)->second, V_TEXGEN_EYE_T, i);
        ctx.gls.setVector(texGenPlaneEqCoefs[i].find(GL_TEXTURE_GEN_R)->second, V_TEXGEN_EYE_R, i);
        ctx.gls.setVector(texGenPlaneEqCoefs[i].find(GL_TEXTURE_GEN_Q)->second, V_TEXGEN_EYE_Q, i);
        
        ctx.gls.setVector(texGenObjEqCoefs[i].find(GL_TEXTURE_GEN_S)->second, V_TEXGEN_OBJECT_S, i);
        ctx.gls.setVector(texGenObjEqCoefs[i].find(GL_TEXTURE_GEN_T)->second, V_TEXGEN_OBJECT_T, i);
        ctx.gls.setVector(texGenObjEqCoefs[i].find(GL_TEXTURE_GEN_R)->second, V_TEXGEN_OBJECT_R, i);
        ctx.gls.setVector(texGenObjEqCoefs[i].find(GL_TEXTURE_GEN_Q)->second, V_TEXGEN_OBJECT_Q, i);
        
        unit->texGenModeS = texGenModes[i].find(GL_TEXTURE_GEN_S)->second;
        unit->texGenModeT = texGenModes[i].find(GL_TEXTURE_GEN_T)->second;
        unit->texGenModeR = texGenModes[i].find(GL_TEXTURE_GEN_R)->second;
        unit->texGenModeQ = texGenModes[i].find(GL_TEXTURE_GEN_Q)->second;
        
        unit->rgbCombinerFunc = texEnvCombRGB[i];
        unit->alphaCombinerFunc = texEnvCombAlpha[i];
        
        unit->srcRGB[0] = texEnvSourcesRGB[i].find(GL_SOURCE0_RGB)->second;
        unit->srcRGB[1] = texEnvSourcesRGB[i].find(GL_SOURCE1_RGB)->second;
        unit->srcRGB[2] = texEnvSourcesRGB[i].find(GL_SOURCE2_RGB)->second;
        unit->srcRGB[3] = texEnvSourcesRGB[i].find(GL_SOURCE3_RGB_NV)->second;
       
        unit->srcAlpha[0] = texEnvSourcesAlpha[i].find(GL_SOURCE0_ALPHA)->second;
        unit->srcAlpha[1] = texEnvSourcesAlpha[i].find(GL_SOURCE1_ALPHA)->second;
        unit->srcAlpha[2] = texEnvSourcesAlpha[i].find(GL_SOURCE2_ALPHA)->second;
        unit->srcAlpha[3] = texEnvSourcesAlpha[i].find(GL_SOURCE3_ALPHA_NV)->second;
        
        unit->operandRGB[0] = texEnvOperandsRGB[i].find(GL_OPERAND0_RGB)->second;
        unit->operandRGB[1] = texEnvOperandsRGB[i].find(GL_OPERAND1_RGB)->second;
        unit->operandRGB[2] = texEnvOperandsRGB[i].find(GL_OPERAND2_RGB)->second;
        unit->operandRGB[3] = texEnvOperandsRGB[i].find(GL_OPERAND3_RGB_NV)->second;
        
        unit->operandAlpha[0] = texEnvOperandsAlpha[i].find(GL_OPERAND0_ALPHA)->second;
        unit->operandAlpha[1] = texEnvOperandsAlpha[i].find(GL_OPERAND1_ALPHA)->second;
        unit->operandAlpha[2] = texEnvOperandsAlpha[i].find(GL_OPERAND2_ALPHA)->second;
        unit->operandAlpha[3] = texEnvOperandsAlpha[i].find(GL_OPERAND3_ALPHA_NV)->second;
        
        unit->rgbScale = texScaleRGB[i];
        unit->alphaScale = texScaleAlpha[i];
        
        i--;
//        unit--;
        unit++;
    }
}

/*****************************************************************
 *             Sync State to GPU Implementations                 *
 *****************************************************************/

void EnableGroup::syncState(GLContext& ctx) const
{
    /* Flags:
     *
     *     flagSeparateSpecular,
     *     flagRescaleNormal,
     *     flagNormalize,
     *     flagFog,
     *     flagAlphaTest,
     *     lights     
     *
     * only affect the construction of vertex and fragment programs generated by GLLib
     * and have not and associated register. We have to set state dirty to reconstruct
     * shaders.
     *
     */
    
    ctx.setLibStateDirty(true);
    
    ctx.stateAdapter.functionCallerName = "Poped EnableGroup";
    
    ctx.stateAdapter.setStencilTest(flagStencilTest);
    ctx.stateAdapter.setCullingFace(flagCullFace);
    ctx.stateAdapter.setDepthTest(flagDepthTest);
    ctx.stateAdapter.setPolygonOffset(flagPolygonOffsetFill);
    ctx.stateAdapter.setScissorTest(flagScissorTest);
    ctx.stateAdapter.setColorBlend(flagBlending);

    ctx.stateAdapter.functionCallerName = "";
} 

void ColorGroup::syncState(GLContext& ctx) const
{
    /* Flags and variables:
     *
     *     alphaFunc,
     *     alphaRef,
     *     flagAlphaTest,
     *
     * only affect the construction of vertex and fragment programs generated by GLLib
     * and have not and associated register. We have to set state dirty to reconstruct
     * shaders.
     *
     */
    
    ctx.setLibStateDirty(true);
    
    ctx.stateAdapter.functionCallerName = "Poped ColorGroup";
    
    ctx.stateAdapter.setColorBlend(flagBlending);
    ctx.stateAdapter.setBlendFactors( blendSFactorRGB, blendDFactorRGB, blendSFactorAlpha, blendDFactorAlpha );
    ctx.stateAdapter.setBlendEquation( blendEquation );
    ctx.stateAdapter.setBlendColor( blendColor[0], blendColor[1], blendColor[2], blendColor[3] );
    ctx.stateAdapter.setColorMask( red, green, blue, alpha );
    ctx.stateAdapter.setClearColor( clearColor[0], clearColor[1], clearColor[2], clearColor[3] );

    ctx.stateAdapter.functionCallerName = "";
}

void CurrentGroup::syncState(GLContext& ctx) const
{
    ctx.stateAdapter.functionCallerName = "Poped CurrentGroup";
    
    ctx.stateAdapter.setCurrentColor( currentColor[0], currentColor[1], currentColor[2], currentColor[3] );

    ctx.stateAdapter.functionCallerName = "";
}

void DepthBufferGroup::syncState(GLContext& ctx) const
{
    ctx.stateAdapter.functionCallerName = "Poped DepthBufferGroup";
    
    ctx.stateAdapter.setDepthTest(flagDepthTest);
    ctx.stateAdapter.setDepthFunction(depthFunc);
    ctx.stateAdapter.setDepthMask(depthMask);
    ctx.stateAdapter.setDepthClearValue(depthClear);
    
    ctx.stateAdapter.functionCallerName = "";
}

void FogGroup::syncState(GLContext& ctx) const
{
     /* Flags and variables:
     *
     *     flagFog,
     *     fogColor,
     *     fogParams,
     *     fogMode,
     *     fogCoordinateSource,
     *
     * only affect the construction of vertex and fragment programs generated by GLLib
     * and have not and associated register. We have to set state dirty to reconstruct
     * shaders.
     *
     */
    
    ctx.setLibStateDirty(true);
}

void LightingGroup::syncState(GLContext& ctx) const
{
    /* Flags and variables:
     *
     *     flagLighting,
     *     flagColorMaterial,
     *     colorMaterialMode,
     *     colorMaterialFace,
     *     flagLocalViewer,
     *     flagSeparateSpecular
     *
     * only affect the construction of vertex and fragment programs generated by GLLib
     * and have not and associated register. We have to set state dirty to reconstruct
     * shaders.
     *
     */
    
    ctx.setLibStateDirty(true);
    
    ctx.stateAdapter.functionCallerName = "Poped LightingGroup";
    
    ctx.stateAdapter.setShadeModel( shadeModel? GL_SMOOTH: GL_FLAT );
    ctx.stateAdapter.setTwoSidedLighting(flagTwoSidedLighting);

    ctx.stateAdapter.functionCallerName = "";
}

void PolygonGroup::syncState(GLContext& ctx) const
{
    ctx.stateAdapter.functionCallerName = "Poped PolygonGroup";
    
    ctx.stateAdapter.setCullingFace(flagCullFace);
    ctx.stateAdapter.setCullFaceMode(cullFace);
    ctx.stateAdapter.setFrontFace(faceMode);
    ctx.stateAdapter.setPolygonOffsetFactors(slopeFactor, unitsFactor);
    ctx.stateAdapter.setPolygonOffset(flagPolygonOffsetFill);

    ctx.stateAdapter.functionCallerName = "";
}

void ScissorGroup::syncState(GLContext& ctx) const
{
    ctx.stateAdapter.functionCallerName = "Poped ScissorGroup";
    
    ctx.stateAdapter.setScissorTest(flagScissorTest);
    ctx.stateAdapter.setScissorArea(scissorIniX, scissorIniY, scissorWidth, scissorHeight);
    
    ctx.stateAdapter.functionCallerName = "";
}

void StencilGroup::syncState(GLContext& ctx) const
{
    ctx.stateAdapter.functionCallerName = "Poped StencilGroup";
    
    ctx.stateAdapter.setStencilTest(flagStencilTest);
    ctx.stateAdapter.setStencilFunc(stBufferFunc, stBufferRefValue, stBufferCompMask);
    ctx.stateAdapter.setStencilOp(stFailUpdateFunc, dpFailUpdateFunc, dpPassUpdateFunc);
    ctx.stateAdapter.setStencilClearValue(stBufferClearValue);
    ctx.stateAdapter.setStencilMask(stBufferUpdateMask);
    
    ctx.stateAdapter.functionCallerName = "";
}

void TransformGroup::syncState(GLContext& ctx) const
{
    /* Flags:
     *
     *     flagNormalize,
     *     flagRescaleNormal,
     *
     * only affect the construction of vertex and fragment programs generated by GLLib
     * and have not and associated register. We have to set state dirty to reconstruct
     * shaders.
     *
     */
    
    ctx.setLibStateDirty(true);
}

void ViewportGroup::syncState(GLContext& ctx) const
{
    ctx.stateAdapter.functionCallerName = "Poped ViewportGroup";
    
    ctx.stateAdapter.setViewportArea(viewportX, viewportY, viewportWidth, viewportHeight);
    ctx.stateAdapter.setDepthRange(near_, far_);

    ctx.stateAdapter.functionCallerName = "";
}

void TextureGroup::syncState(GLContext& ctx) const
{
    /* Flags and variables:
     *
     *     texGenEnabled,
     *     texGenModes,
     *     texEnvSources,
     *     texEnvOperands,
     *     texScales
     *
     * only affect the construction of vertex and fragment programs generated by GLLib
     * and have not and associated register. We have to set state dirty to reconstruct
     * shaders.
     *
     * The remainder are updated to the GPU in setupTexture of AuxFuncLib,
     * when a drawElements, drawArrays or glEnd() command is processed.
     *
     */
    
    ctx.setLibStateDirty(true);
}
