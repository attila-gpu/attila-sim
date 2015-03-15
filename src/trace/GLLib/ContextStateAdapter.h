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

#ifndef CONTEXTSTATEADAPTER_H
    #define CONTEXTSTATEADAPTER_H

#include "gl.h"
#include "GPUDriver.h"
#include "GPUTypes.h"
#include "QuadReg.h"
#include <string>

namespace libgl
{

#ifndef Quadf
    typedef QuadReg<float> Quadf;
#endif

class ContextStateAdapter
{
/**
 * This Adapter Class is used to convert some specific OpenGL state changes to the equivalent 
 * writes in GPU registers, hiding semantic differences.
 *
 * For example, the Polygon offset state in OpenGL Context state is implemented in GLLib using 2 state variables 
 * to manage slope factor and offset units, and a flag to store if polygon offset is enabled or not.
 * However, GPU does not includes any register to enable/disable this feature. The semantic translation
 * is done writing 0.0 to slope factor and offset units registers to disable Polygon offset.
 *
 * Currently, it is implemented as a regular object instead of class with static methods and attributes.
 * It uses the GPU driver to access/write GPU registers.
 *
 * To initialize it, do the following:
 *
 * @code
 *
 * GPUDriver* driver = GPUDriver::getGPUDriver(); // Gets the 3D card driver
 *
 * ContextStateAdapter* ctxAdapter = new ContextStateAdapter(driver); // Creates a ContextStateAdapter that will use 
 *                                                                    // this driver
 *
 * @endcode
 *
 * @version 1.0
 * @date 17/12/2005
 * @author Jordi Roca Monfort - jroca@ac.upc.es
 */
private:
    
    GPUDriver* driver;  ///< Pointer to access to the GPU Driver
    
    /* Initial default values for some OpenGL context state */
    
    GLenum cullFaceMode;
    bool previousEnabledCullFace;
    bool previousEnabledPolygonOffset;
    GLfloat slopeFactor;
    GLfloat unitsOffset;
    
public:
    
    ContextStateAdapter( GPUDriver* driver );

    std::string functionCallerName; ///< Mecanism for print the caller of the functions in the panic string
                                    ///  in order to identify the GLLib module that causes the panic.
    
    void setPrimitiveMode( GLenum primitive );
    
    void setTextureTarget( GLuint textureUnit, GLenum target, GLboolean enabled );
    void setTextureMaxMinLevels( GLuint textureUnit, GLuint minLevel, GLuint maxLevel );
    void setTextureLodBias( GLuint textureUnit, GLfloat lodBias );
    void setTextureUnitLodBias( GLuint textureUnit, GLfloat lodBias );
    void setTextureDimensions( GLuint textureUnit, GLuint width, GLuint height );
    void setBaseMipMapLog2Dimensions( GLuint textureUnit, GLuint log2_width, GLuint log2_height );
    void setTextureInternalFormat( GLuint textureUnit, GLenum internalformat );
    void setTextureReverseData( GLuint textureUnit, GLboolean reverse );
    void setTextureWrap( GLuint textureUnit, GLenum wrapS, GLenum wrapT );
    void setMinMagFilters( GLuint textureUnit, GLenum minFilter, GLenum magFilter, GLfloat maxAnisotropy );
    
    void setCullingFace( GLboolean enable );
    void setCullFaceMode( GLenum cullFace );
    void setFrontFace( GLenum mode );
        
    void setStencilTest( GLboolean enable );
    void setStencilFunc( GLenum func, GLint ref, GLuint mask );
    void setStencilOp( GLenum fail, GLenum zfail, GLenum zpass );
    void setStencilClearValue( GLint s );
    void setStencilMask( GLuint mask );
        
    void setPolygonOffset( GLboolean enabled );
    void setPolygonOffsetFactors( GLfloat slopeFactor, GLfloat unitsOffset );
    
    void setDepthTest( GLboolean enable );
    void setDepthFunction( GLenum depthFunc );
    void setDepthMask( GLboolean enable );
    void setDepthClearValue( GLclampd clearValue );
    void setDepthRange( GLclampd near_val, GLclampd far_val );
    
    void setCurrentColor( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha );
    void setColorBlend( GLboolean enable );
    void setBlendColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha );
    void setColorMask( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha );
    void setClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha );
    void setShadeModel( GLenum shadeMode );
    void setViewportArea( GLint iniX, GLint iniY, GLuint width, GLuint height );
        
    void setScissorTest(GLboolean enable);
    void setScissorArea( GLint iniX, GLint iniY, GLsizei width, GLsizei height );
    
    void setBlendFactors( GLenum sRGBFactor, GLenum dRGBFactor, GLenum sAlphaFactor, GLenum dAlphaFactor );
    void setBlendEquation( GLenum mode );

    void setTwoSidedLighting( GLboolean enable );
};

} // namespace libgl

#endif // CONTEXTSTATEADAPTER_H
