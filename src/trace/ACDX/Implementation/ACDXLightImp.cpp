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

#include "ACDXLightImp.h"
#include "ACDXFixedPipelineStateImp.h"

using namespace acdlib;

ACDXLightImp::ACDXLightImp(acd_uint lightNumber, ACDXFixedPipelineStateImp* fpStateImp)
: _lightNumber(lightNumber), _fpStateImp(fpStateImp)
{
}

void ACDXLightImp::setLightAmbientColor(acd_float r, acd_float g, acd_float b, acd_float a)
{
    ACDXFloatVector4 ambientColor;

    ambientColor[0] = r;
    ambientColor[1] = g;
    ambientColor[2] = b;
    ambientColor[3] = a;

    _fpStateImp->_lightAmbient[_lightNumber] = ambientColor;
}

void ACDXLightImp::setLightDiffuseColor(acd_float r, acd_float g, acd_float b, acd_float a)
{
    ACDXFloatVector4 diffuseColor;

    diffuseColor[0] = r;
    diffuseColor[1] = g;
    diffuseColor[2] = b;
    diffuseColor[3] = a;

    _fpStateImp->_lightDiffuse[_lightNumber] = diffuseColor;
}

void ACDXLightImp::setLightSpecularColor(acd_float r, acd_float g, acd_float b, acd_float a)
{
    ACDXFloatVector4 specularColor;

    specularColor[0] = r;
    specularColor[1] = g;
    specularColor[2] = b;
    specularColor[3] = a;

    _fpStateImp->_lightSpecular[_lightNumber] = specularColor;
}

void ACDXLightImp::setLightPosition(acd_float x, acd_float y, acd_float z, acd_float w)
{
    ACDXFloatVector4 position;

    position[0] = x;
    position[1] = y;
    position[2] = z;
    position[3] = w;

    _fpStateImp->_lightPosition[_lightNumber] = position;
}

void ACDXLightImp::setLightDirection(acd_float x, acd_float y, acd_float z)
{
    ACDXFloatVector3 direction;

    direction[0] = x;
    direction[1] = y;
    direction[2] = z;

    _fpStateImp->_lightDirection[_lightNumber] = direction;
}


void ACDXLightImp::setAttenuationCoeffs(acd_float constant, acd_float linear, acd_float quadratic)
{
    ACDXFloatVector3 attenuationCoeffs;

    attenuationCoeffs[0] = constant;
    attenuationCoeffs[1] = linear;
    attenuationCoeffs[2] = quadratic;

    _fpStateImp->_lightAttenuation[_lightNumber] = attenuationCoeffs;
}

acd_float ACDXLightImp::getConstantAttenuation () 
{
    ACDXFloatVector3 attenuationCoeffs;
    attenuationCoeffs = _fpStateImp->_lightAttenuation[_lightNumber];

    return attenuationCoeffs[0];
}

acd_float ACDXLightImp::getLinearAttenuation () 
{
    ACDXFloatVector3 attenuationCoeffs;
    attenuationCoeffs = _fpStateImp->_lightAttenuation[_lightNumber];

    return attenuationCoeffs[1];
}

acd_float ACDXLightImp::getQuadraticAttenuation () 
{
    ACDXFloatVector3 attenuationCoeffs;
    attenuationCoeffs = _fpStateImp->_lightAttenuation[_lightNumber];

    return attenuationCoeffs[2];
}

void ACDXLightImp::setSpotLightDirection(acd_float x, acd_float y, acd_float z)
{
    ACDXFloatVector3 spotDirection;

    spotDirection[0] = x;
    spotDirection[1] = y;
    spotDirection[2] = z;
    
    _fpStateImp->_lightSpotDirection[_lightNumber] = spotDirection;
}

void ACDXLightImp::setSpotLightCutOffAngle(acd_float cos_angle)
{
    _fpStateImp->_lightSpotCutOffAngle[_lightNumber] = cos_angle;
}

void ACDXLightImp::setSpotExponent(acd_float exponent)
{
    _fpStateImp->_lightSpotExponent[_lightNumber] = exponent;
}

