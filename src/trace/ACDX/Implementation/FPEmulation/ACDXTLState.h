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

#ifndef ACDX_TLSTATE_H
    #define ACDX_TLSTATE_H

#include "gl.h"

/**
 * Interface for accessing some OpenGL state
 */

namespace acdlib
{

class ACDXTLState
{
public:

    struct TextureUnit
    {
        unsigned int unitId;

        bool activeTexGenS;
        bool activeTexGenT;
        bool activeTexGenR;
        bool activeTexGenQ;

        enum TexGenMode
        {
            OBJECT_LINEAR,
            EYE_LINEAR,
            SPHERE_MAP,
            REFLECTION_MAP,
            NORMAL_MAP,
        };

        TexGenMode texGenModeS;
        TexGenMode texGenModeT;
        TexGenMode texGenModeR;
        TexGenMode texGenModeQ;

        bool textureMatrixIsIdentity;

        TextureUnit() {}
        /*
                    unsigned int unitId = 0,
                    bool activeTexGenS = false, bool activeTexGenT = false,
                    bool activeTexGenR = false, bool activeTexGenQ = false,
                    TexGenMode texGenModeS = EYE_LINEAR,
                    TexGenMode texGenModeT = EYE_LINEAR,
                    TexGenMode texGenModeR = EYE_LINEAR,
                    TexGenMode texGenModeQ = EYE_LINEAR,
                    bool textureMatrixIsIdentity = true)
                    : unitId(unitId), activeTexGenS(activeTexGenS),
                      activeTexGenT(activeTexGenT), activeTexGenR(activeTexGenR),
                      activeTexGenQ(activeTexGenQ), texGenModeS(texGenModeS),
                      texGenModeT(texGenModeT), texGenModeR(texGenModeR),
                      texGenModeQ(texGenModeQ), textureMatrixIsIdentity(textureMatrixIsIdentity)
        {
        }
        */
    };

    /* Lighting State */
    typedef unsigned int Light;

    virtual bool normalizeNormals()= 0;
    virtual bool isLighting()=0;
    virtual bool isRescaling()=0;
    virtual bool infiniteViewer()=0;
    virtual bool localLightSource(Light light)=0;
    virtual void attenuationCoeficients(Light light, float& constantAtt, float& linearAtt, float& quadraticAtt)=0;
    virtual bool isSpotLight(Light light)=0;
    virtual bool isLightEnabled(Light light)=0;
    virtual int maxLights()=0;
    virtual bool separateSpecular()=0;
    virtual bool twoSidedLighting()=0;
    virtual bool anyLightEnabled()=0;
    virtual bool anyLocalLight()=0;

    /* Primitive Culling State */
    virtual bool isCullingEnabled()=0;
    virtual bool isFrontFaceCulling()=0;
    virtual bool isBackFaceCulling()=0;

    /* Color Material State */

    enum ColorMaterialMode { EMISSION, AMBIENT, DIFFUSE, SPECULAR, AMBIENT_AND_DIFFUSE };

    virtual ColorMaterialMode colorMaterialMode()=0;
    virtual bool colorMaterialFrontEnabled()=0;
    virtual bool colorMaterialBackEnabled()=0;

    /* Texture Environment State */
    virtual bool anyTextureUnitEnabled()=0;
    virtual bool isTextureUnitEnabled(GLuint unit)=0;
    virtual TextureUnit getTextureUnit(GLuint unit)=0;
    virtual int maxTextureUnits()=0;

    /* Fog environment State */
    enum FogCoordSrc { FRAGMENT_DEPTH , FOG_COORD };

    virtual FogCoordSrc fogCoordSrc()=0;
    virtual bool fogEnabled()=0;
};

} // namespace acdlib

#endif // ACDX_TLSTATE_H
