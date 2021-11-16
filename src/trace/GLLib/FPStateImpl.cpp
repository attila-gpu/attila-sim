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
 
// FPStateImpl.cpp: implementation of the FPStateImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "FPStateImpl.h"

using namespace libgl;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FPStateImpl::FPStateImpl()
{

}

FPStateImpl::~FPStateImpl()
{

}

bool FPStateImpl::fogEnabled()
{
    return false;
}

bool FPStateImpl::separateSpecular()
{
    return true;
}

bool FPStateImpl::anyTextureUnitEnabled()
{
    return true;
}

bool FPStateImpl::isTextureUnitEnabled(GLuint unit)
{
    switch(unit)
    {
        case 0: 
            return true;
            break;
        case 1:
            return true;
            break;
        case 2:
            return true;
            break;
        default: return false;
    }
    
}

FPState::TextureUnit FPStateImpl::getTextureUnit(GLuint unit)
{
    switch(unit)
    {
        case 0: 
        {
            TextureUnit texunit0(0,FPState::TextureUnit::TEXTURE_2D,
                                   FPState::TextureUnit::COMBINE);
            break;
        }
        case 1:
            return TextureUnit(1,FPState::TextureUnit::TEXTURE_2D, 
                               FPState::TextureUnit::MODULATE,FPState::TextureUnit::RGB);
            break;
        case 2:
        {
            TextureUnit texunit2(2,FPState::TextureUnit::TEXTURE_2D,
                FPState::TextureUnit::COMBINE,FPState::TextureUnit::ALPHA);
            
            texunit2.combineMode.combineRGBFunction = FPState::TextureUnit::CombineMode::DOT3_RGB;
            texunit2.combineMode.combineALPHAFunction = FPState::TextureUnit::CombineMode::INTERPOLATE;
            texunit2.combineMode.srcRGB[0] = FPState::TextureUnit::CombineMode::PREVIOUS;
            texunit2.combineMode.srcRGB[1] = FPState::TextureUnit::CombineMode::PRIMARY_COLOR;
            texunit2.combineMode.srcRGB[2] = FPState::TextureUnit::CombineMode::CONSTANT;
            texunit2.combineMode.rgbScale = 2;

            texunit2.combineMode.operandRGB[0] = FPState::TextureUnit::CombineMode::SRC_COLOR;
            texunit2.combineMode.operandRGB[1] = FPState::TextureUnit::CombineMode::SRC_COLOR;
            texunit2.combineMode.operandRGB[2] = FPState::TextureUnit::CombineMode::SRC_COLOR;

            texunit2.combineMode.srcALPHA[0] = FPState::TextureUnit::CombineMode::PRIMARY_COLOR;
            texunit2.combineMode.srcALPHA[1] = FPState::TextureUnit::CombineMode::TEXTURE;
            texunit2.combineMode.srcALPHA[2] = FPState::TextureUnit::CombineMode::TEXTUREn;
            texunit2.combineMode.srcALPHA_texCrossbarReference[2] = 1;

            texunit2.combineMode.operandALPHA[0] = FPState::TextureUnit::CombineMode::SRC_ALPHA;
            texunit2.combineMode.operandALPHA[1] = FPState::TextureUnit::CombineMode::SRC_ALPHA;
            texunit2.combineMode.operandALPHA[2] = FPState::TextureUnit::CombineMode::SRC_ALPHA;

            // ????
            return 0;
            
            break;
        }
        default: return TextureUnit(unit);
    }
    return TextureUnit(unit);
}

int FPStateImpl::maxTextureUnits()
{
    return 16;
}


