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

#ifndef SETTINGS_ADAPTER_H
    #define SETTINGS_ADAPTER_H

#include "ACDXFixedPipelineSettings.h"

#include "ACDXTLState.h"
#include "ACDXFPState.h"

namespace acdlib
{

class SettingsAdapter
{
public:
    
    SettingsAdapter(const acdlib::ACDX_FIXED_PIPELINE_SETTINGS& fpSettings);

    ACDXTLState& getTLState() { return tlState; }

    ACDXFPState& getFPState() { return fpState; }

private:

    acdlib::ACDX_FIXED_PIPELINE_SETTINGS settings;

    /**
     * Implements ACDXTLState
     */
    class TLStateImplementation; // define an implementation

    friend class TLStateImplementation; // Allow internal access to parent class (SettingsAdapter)

    class TLStateImplementation : public ACDXTLState
    {
    private:
        SettingsAdapter* parent; // The parent class

    public:
        TLStateImplementation(SettingsAdapter* parent);

        bool normalizeNormals();
        bool isLighting();
        bool isRescaling();
        bool infiniteViewer();
        bool localLightSource(Light light);
        void attenuationCoeficients(Light light, float& constantAtt, float& linearAtt, float& quadraticAtt);
        bool isSpotLight(Light light);
        bool isLightEnabled(Light light);
        int maxLights();
        bool separateSpecular();
        bool twoSidedLighting();
        bool anyLightEnabled();
        bool anyLocalLight();

        bool isCullingEnabled();
        bool isFrontFaceCulling();
        bool isBackFaceCulling();

        ACDXTLState::ColorMaterialMode colorMaterialMode();
        bool colorMaterialFrontEnabled();
        bool colorMaterialBackEnabled();

        bool isTextureUnitEnabled(GLuint texUnit);
        bool anyTextureUnitEnabled();
        int maxTextureUnits();
        ACDXTLState::TextureUnit getTextureUnit(GLuint unit);

        ACDXTLState::FogCoordSrc fogCoordSrc();
        bool fogEnabled();

    } tlState; // declare a ACDXTLState implementor object

    /**
     * Implements ACDXFPState
     */
    class FPStateImplementation; // define an implementation

    friend class FPStateImplementation; // Allow internal access to parent class (SettingsAdapter)

    class FPStateImplementation : public ACDXFPState
    {
    private:
        SettingsAdapter* parent; // The parent class
    
    public:

        FPStateImplementation(SettingsAdapter* parent);

        bool separateSpecular();
        bool anyTextureUnitEnabled();
        bool isTextureUnitEnabled(GLuint unit);
        int maxTextureUnits();
        ACDXFPState::TextureUnit getTextureUnit(GLuint unit);
        bool fogEnabled();
        ACDXFPState::FogMode fogMode();

        ACDXFPState::AlphaTestFunc alphaTestFunc();
        bool alphaTestEnabled();

    } fpState; // declare a ACDXFPState implementor object
};

} // namespace acdlib

#endif // SETTINGS_ADAPTER_H
