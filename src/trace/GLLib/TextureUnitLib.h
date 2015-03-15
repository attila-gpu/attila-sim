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

#ifndef TEXTUREUNITLIB_H
    #define TEXTUREUNITLIB_H

#include "TextureObject.h"


namespace libgl
{

class TextureUnit
{
private:

    // 0 -> 1D, 2 -> 2D, 3 -> 3D, 4 -> CUBE_MAP
    bool flags[4];
    TextureObject* texObjs[4];
    
    //GLfloat envColor[4];

    GLfloat lodBias;
  
    bool activeTexGenS;
    bool activeTexGenT;
    bool activeTexGenR;
    bool activeTexGenQ;        
    
    GLenum texGenModeS;
    GLenum texGenModeT;
    GLenum texGenModeR;
    GLenum texGenModeQ;
    
    GLenum texFunc;    
    GLenum rgbCombinerFunc;
    GLenum alphaCombinerFunc;
    
    GLint rgbScale;
    GLint alphaScale;    
        
    /* Fourth source extended for "NV_texture_env_combine4" extension support */
    GLenum srcRGB[4];    
    GLenum srcAlpha[4];    
    GLenum operandRGB[4];
    GLenum operandAlpha[4];
    
    friend class pushstate::TextureGroup; ///< Allows TextureGroup class to save and restore 
                                          ///  TextureUnit State for glPush/glPop Attribs.
    
public:

    TextureUnit();
        
    /**
     * @param target GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP
     *
     * @note Extensions will be added soon (ie SPHERE_MAP).
     */
    void enableTarget(GLenum target);
    void disableTarget(GLenum target);
    
    GLenum getTarget() const;
    
    // Texture image + filtering parameters
    // Returns null if it does not have a texture object set
    // Returns the preferent TextureObject (based in active targets)
    TextureObject* getTextureObject() const;
    
    TextureObject* getTextureObject(GLenum target) const;
    
    void setParameter(GLenum target, GLenum pname, const GLfloat* param); /* Not implemented yet */
    void setParameter(GLenum target, GLenum pname, const GLint* param);
    
    // defines a new texture object attached to this texture unit
    // null can be used to deattach the current texture object
    void setTextureObject(GLenum target, TextureObject* to);
    
    GLenum getTextureFunction() const { return texFunc; }    
    GLenum getRGBCombinerFunction() const { return rgbCombinerFunc; }
    GLenum getAlphaCombinerFunction() const { return alphaCombinerFunc; }

    GLint getRGBScale() const { return rgbScale; }
    GLint getAlphaScale() const { return alphaScale; }
    GLenum getSrcRGB(GLuint n) const;
    GLenum getSrcAlpha(GLuint n) const;
    GLenum getOperandRGB(GLuint n) const;
    GLenum getOperandAlpha(GLuint n) const;
    
    void setPlaneValues(GLenum coord, GLenum plane, const GLfloat* values);
    
    void setTexGenMode(GLenum coord, GLint mode);
    
    GLint getTexGenMode(GLenum coord) const;

    /**
     * accepted coord values: GL_S, GL_T, GL_R, GL_Q
     */
    void setEnabledTexGen(GLenum coord, bool active);
    
    /**
     * accepted coord values: GL_S, GL_T, GL_R, GL_Q
     */
    bool isEnabledTexGen(GLenum coord) const;
    
    
    GLfloat getLodBias() const { return lodBias; }
    
    
};

} // namespace libgl

#endif // TEXTUREUNITLIB_H


