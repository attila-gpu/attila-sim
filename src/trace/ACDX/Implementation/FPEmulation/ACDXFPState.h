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

#ifndef ACDX_FPSTATE_H
    #define ACDX_FPSTATE_H

#include "gl.h"

/**
 * Interface for accessing some OpenGL state related to Fragment Processing
 */

namespace acdlib
{

class ACDXFPState
{
public:

    struct TextureUnit
    {       
        enum TextureTarget
        { 
            TEXTURE_1D,
            TEXTURE_2D,
            TEXTURE_3D,
            CUBE_MAP 
            /* RECT (Rectangular texture) target not yet supported by now */
        };
    
        /* Texture Functions for OpenGL Spec 1.3 */
        enum TextureFunction
        { 
            REPLACE,
            MODULATE,
            DECAL,
            BLEND,
            ADD,
            COMBINE,
            COMBINE4_NV, /* For "NV_texture_env_combine4" extension support */
        };  
    
        /* Base internal formats for OpenGL Spec 1.4. 
        * Depth component not supported yet 
        */
        enum BaseInternalFormat
        { 
            ALPHA,
            LUMINANCE,
            LUMINANCE_ALPHA,
            DEPTH,
            INTENSITY,
            RGB,
            RGBA
        };
        
        
        unsigned int unitId;
        TextureTarget activeTarget;
        TextureFunction function;
        BaseInternalFormat format;

        /* -- Combine function mode related state --
         *  Only applicable if function == COMBINE
         */

        struct CombineMode
        {
            enum CombineFunction 
            { 
                REPLACE, 
                MODULATE, 
                ADD, 
                ADD_SIGNED, 
                INTERPOLATE,
                SUBTRACT,
                DOT3_RGB, /* Only applicable to Combine RGB function */
                DOT3_RGBA,/* Only applicable to Combine RGB function */
                
                /* For "ATI_texture_env_combine3" extension support */
                MODULATE_ADD_ATI,
                MODULATE_SIGNED_ADD_ATI,
                MODULATE_SUBTRACT_ATI
            };

            enum SourceCombine
            {
                ZERO,  /* For "NV_texture_env_combine4" extension support */
                ONE,   /* For "ATI_texture_env_combine3" extension support */
                TEXTURE,
                TEXTUREn,
                CONSTANT,
                PRIMARY_COLOR,
                PREVIOUS
            };

            enum OperandCombine
            {
                SRC_COLOR,
                ONE_MINUS_SRC_COLOR,
                SRC_ALPHA,
                ONE_MINUS_SRC_ALPHA
            };

            CombineFunction combineRGBFunction;
            CombineFunction combineALPHAFunction;

            /* Fourth source extended for "NV_texture_env_combine4" extension support */
            
            SourceCombine srcRGB[4];
            SourceCombine srcALPHA[4];

            OperandCombine operandRGB[4];
            OperandCombine operandALPHA[4];

            unsigned int srcRGB_texCrossbarReference[4];
            unsigned int srcALPHA_texCrossbarReference[4];

            unsigned int rgbScale;
            unsigned int alphaScale;

            CombineMode(CombineFunction combineRGBFunction = MODULATE,
                        CombineFunction combineALPHAFunction = MODULATE,
                        unsigned int rgbScale = 1, 
                        unsigned int alphaScale = 1)
                        : combineRGBFunction(combineRGBFunction),
                          combineALPHAFunction(combineALPHAFunction),
                          rgbScale(rgbScale), alphaScale(alphaScale)

            {
                srcRGB[0] = srcALPHA[0] = TEXTURE; 
                srcRGB[1] = srcALPHA[1] = PREVIOUS; 
                srcRGB[2] = srcALPHA[2] = CONSTANT;
                srcRGB[3] = srcALPHA[3] = ZERO;

                operandRGB[0] = operandRGB[1] = SRC_COLOR;
                operandRGB[2] = SRC_ALPHA;
                operandALPHA[0] = operandALPHA[1] = operandALPHA[2] = SRC_ALPHA;
                operandRGB[3] = ONE_MINUS_SRC_COLOR;
                operandALPHA[3] = ONE_MINUS_SRC_ALPHA;
            }

        } combineMode;

        /* ----------------------------------------- */

        TextureUnit(unsigned int unitId, TextureTarget activeTarget = TEXTURE_2D,
                    TextureFunction function = MODULATE, BaseInternalFormat format = RGBA)
                    : unitId(unitId), activeTarget(activeTarget), function(function), format(format)
        {
        }
    };

    /* Fog related State */
    enum FogMode { LINEAR, EXP, EXP2 };

    virtual bool fogEnabled()=0;
    virtual FogMode fogMode()=0;

    /* Alpha Test related State */
    
    enum AlphaTestFunc { NEVER, ALWAYS, LESS, LEQUAL, EQUAL, GEQUAL, GREATER, NOTEQUAL };
    
    virtual AlphaTestFunc alphaTestFunc()=0;
    //virtual float alphaTestRefValue()=0;
    virtual bool alphaTestEnabled()=0;
    
    virtual bool separateSpecular()=0;
    virtual bool anyTextureUnitEnabled()=0;
    virtual bool isTextureUnitEnabled(GLuint unit)=0;
    virtual TextureUnit getTextureUnit(GLuint unit)=0;
    virtual int maxTextureUnits()=0;
};

} // namespace acdlib


#endif // ACDX_FPSTATE_H
