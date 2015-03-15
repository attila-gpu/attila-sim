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

#include "ACDXMaterialImp.h"
#include "ACDXFixedPipelineStateImp.h"

using namespace acdlib;

ACDXMaterialImp::ACDXMaterialImp(acd_uint face, ACDXFixedPipelineStateImp *fpStateImp)
: _face(face), _fpStateImp(fpStateImp)
{

}

void ACDXMaterialImp::setMaterialAmbientColor(acd_float r, acd_float g, acd_float b, acd_float a)
{
    ACDXFloatVector4 materialAmbientColor;

    materialAmbientColor[0] = r;
    materialAmbientColor[1] = g;
    materialAmbientColor[2] = b;
    materialAmbientColor[3] = a;

    if (_face == 0)
        _fpStateImp->_materialFrontAmbient = materialAmbientColor;
    else
        _fpStateImp->_materialBackAmbient = materialAmbientColor;
}

void ACDXMaterialImp::setMaterialDiffuseColor(acd_float r, acd_float g, acd_float b, acd_float a)
{
    ACDXFloatVector4 materialDiffuseColor;

    materialDiffuseColor[0] = r;
    materialDiffuseColor[1] = g;
    materialDiffuseColor[2] = b;
    materialDiffuseColor[3] = a;

    if (_face == 0)
        _fpStateImp->_materialFrontDiffuse = materialDiffuseColor;
    else
        _fpStateImp->_materialBackDiffuse = materialDiffuseColor;
}

void ACDXMaterialImp::setMaterialSpecularColor(acd_float r, acd_float g, acd_float b, acd_float a)
{
    ACDXFloatVector4 materialSpecularColor;

    materialSpecularColor[0] = r;
    materialSpecularColor[1] = g;
    materialSpecularColor[2] = b;
    materialSpecularColor[3] = a;

    if (_face == 0)
        _fpStateImp->_materialFrontSpecular = materialSpecularColor;
    else
        _fpStateImp->_materialBackSpecular = materialSpecularColor;
}

void ACDXMaterialImp::setMaterialEmissionColor(acd_float r, acd_float g, acd_float b, acd_float a)
{
    ACDXFloatVector4 materialEmissionColor;

    materialEmissionColor[0] = r;
    materialEmissionColor[1] = g;
    materialEmissionColor[2] = b;
    materialEmissionColor[3] = a;

    if (_face == 0)
        _fpStateImp->_materialFrontEmission = materialEmissionColor;
    else
        _fpStateImp->_materialBackEmission = materialEmissionColor;

}

void ACDXMaterialImp::setMaterialShininess(acd_float shininess)
{
    if (_face == 0)
        _fpStateImp->_materialFrontShininess = shininess;
    else
        _fpStateImp->_materialBackShininess = shininess;
}
