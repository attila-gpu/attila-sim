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

#ifndef AGL_TEXTUREUNIT_H
    #define AGL_TEXTUREUNIT_H

#include "AGLTextureObject.h"


namespace agl
{

/**
* ACDBuffer interface is the common interface to declare API independent data buffers.
* API-dependant vertex and index buffers are mapped onto this interface
*
* This objects are created from ResourceManager factory
*
* @author
* @version
* @date
*/

class TextureUnit
{
private:

#define NUM_TEXTURE_TYPES 5

    enum
    {
        TEXTURE_1D,
        TEXTURE_2D,
        TEXTURE_3D,
        TEXTURE_CUBE_MAP,
        TEXTURE_RECTANGLE
    };

    GLTextureObject* textureObject[NUM_TEXTURE_TYPES];
    GLboolean activeObject[NUM_TEXTURE_TYPES];

    GLfloat lodBias;
  
    GLboolean activeTexGenS;
    GLboolean activeTexGenT;
    GLboolean activeTexGenR;
    GLboolean activeTexGenQ;        
    
    GLenum texGenModeS;
    GLenum texGenModeT;
    GLenum texGenModeR;
    GLenum texGenModeQ;

    GLfloat objPlaneS[4];
    GLfloat objPlaneT[4];
    GLfloat objPlaneR[4];
    GLfloat objPlaneQ[4];

    GLfloat eyePlaneS[4];
    GLfloat eyePlaneT[4];
    GLfloat eyePlaneR[4];
    GLfloat eyePlaneQ[4];

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
    
public:

    TextureUnit();

    /**
     * Enables a texture unit
     *
     * @param target. Target we want to enable (possible values GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_RECTANGLE)
     */
    void enableTarget(GLenum target);

    /**
     * Disable a texture unit
     *
     * @param target. Target we want to enable (possible values GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_RECTANGLE)
     */
    void disableTarget(GLenum target);

    /**
     * Enables a texture unit
     *
     * @returns if the selected texture unit is active
     */
    GLenum isTextureUnitActive();
      
    // Texture image + filtering parameters
    // Returns null if it does not have a texture object set
    // Returns the preferent GLTextureObject (based in active targets)
    /**
     * Enables a texture unit
     *
     */
    GLTextureObject* getTextureObject(GLenum textureType) const;
    

    void setParameter(GLenum target, GLenum pname, const GLfloat* param); /* Not implemented yet */
    void setParameter(GLenum target, GLenum pname, const GLint* param);
    
    // defines a new texture object attached to this texture unit
    // null can be used to deattach the current texture object
    void setTextureObject(GLenum target, GLTextureObject* to);
    
    GLenum getTextureFunction() const { return texFunc; }    
    GLenum getRGBCombinerFunction() const { return rgbCombinerFunc; }
    GLenum getAlphaCombinerFunction() const { return alphaCombinerFunc; }
    GLint getRGBScale() const { return rgbScale; }
    GLint getAlphaScale() const { return alphaScale; }
    GLenum getSrcRGB(GLuint n) const;
    GLenum getSrcAlpha(GLuint n) const;
    GLenum getOperandRGB(GLuint n) const;
    GLenum getOperandAlpha(GLuint n) const;
    
    void setTexGenMode(GLenum coord, GLint mode);
    
    GLint getTexGenMode(GLenum coord) const;

    /**
     * accepted coord values: GL_S, GL_T, GL_R, GL_Q
     */
    void setEnabledTexGen(GLenum coord, bool active);
    
    /**
     * accepted coord values: GL_S, GL_T, GL_R, GL_Q
     */
    GLboolean isEnabledTexGen(GLenum coord) const;

    GLenum getTexMode ();
    
    
    GLfloat getLodBias() const { return lodBias; }
    
};

} // namespace agl

#endif // AGL_TEXTUREUNIT_H


