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

#ifndef AGL_TEXTUREOBJECT
    #define AGL_TEXTUREOBJECT

#include "gl.h"
#include "AGLBaseObject.h"
#include "ACDTexture2D.h"
#include "ACDTextureCubeMap.h"
#include "ACDDevice.h"

namespace agl
{

class GLTextureTarget;

class GLTextureObject : public BaseObject
{
public:

    GLTextureObject(GLuint name, GLenum targetName);
    void attachDevice(acdlib::ACDDevice* device, GLuint target);
    acdlib::ACDTexture* getTexture (); 

    void set1D(GLint ifmt, GLint width, GLint border, GLenum format,
               GLenum type, GLvoid* data, GLint compressedSize = 0);

    GLenum set2D(GLint &width, GLint &height, GLint border, GLenum format, GLint ifmt, 
               GLenum type, const GLvoid* data, GLubyte* &convertedData, GLint &compressedSize);

    void setPartial2D(GLsizei width, GLint height, GLint xoffset, GLint yoffset, GLenum format, 
                        GLenum iformat, GLenum type, const GLvoid* data, GLubyte* &convertedData, GLint &compressedSize);


    void setContents( GLenum targetFace, GLint level, GLint ifmt, GLsizei width, GLsizei height, GLint depth,
                        GLint border, GLenum fmt, GLenum type, const GLubyte* data, GLint compressionSize = 0/*by default not compressed*/);


    void setPartialContents(GLenum targetFace, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
                            GLsizei width, GLsizei height, GLsizei depth, GLenum fmt, GLenum type,
                            const GLubyte* subData, GLint compressionSize = 0 /*by default not compressed*/);


    void setParameter(GLenum pname, const GLint* parameter);
    void setParameter(GLenum pname, const GLfloat* parameter);

    GLenum getWrapS() const { return _wrapS; }
    GLenum getWrapT() const { return _wrapT; }
    GLenum getWrapR() const { return _wrapR; }
    GLenum getMinFilter() const { return _minFilter; }
    GLenum getMagFilter() const { return _magFilter; }
    GLint getBaseLevel() const  { return _baseLevel; }
    GLint getMaxLevel() const   { return _maxLevel; }
    GLfloat getLodBias() const  { return _biasLOD; }
    GLfloat getMaxLOD() const { return _maxLOD;}
    GLfloat getMinLOD() const { return _minLOD;}
    GLfloat getMaxAnisotropy() const {return _maxAnisotropy;}

    GLsizei getWidth(GLenum targetFace, GLenum mipmap) const;
    GLsizei getHeight(GLenum targetFace, GLenum mipmap) const;
    GLsizei getDepth(GLenum targetFace, GLenum mipmap) const;
    GLenum  getFormat(GLenum targetFace, GLenum mipmap) const;

    GLenum getBaseInternalFormat(GLenum target);
    GLenum getBaseInternalFormat() const; //
    GLuint getImageSize(GLenum targetFace, GLenum mipmap);
    GLuint getSize(GLenum openGLType);
    GLenum getGLTextureFormat(acdlib::ACD_FORMAT format);


    static void convertDEPTHtoDEPTH24(const GLubyte* input, GLubyte* output,
                                    GLsizei ics, GLsizei ocs,
                                    GLsizei width, GLsizei height);

    static void convertRGBtoRGBA(const GLubyte* input, GLubyte* output,
                                 GLsizei inputComponentSz, GLsizei outputComponentSz,
                                 GLsizei width, GLint height);

    static void convertRGBtoRGB(const GLubyte* input, GLubyte* output,
                                GLsizei inputComponentSz, GLsizei outputComponentSz,
                                GLsizei width, GLint height);

    static void convertRGBAtoRGBA(const GLubyte* input, GLubyte* output,
                                  GLsizei inputComponentSz, GLsizei outputComponentSz,
                                  GLsizei width, GLint height);

    static void convertBGRAtoRGBA(const GLubyte* input, GLubyte* output,
                                GLsizei inputComponentSz, GLsizei outputComponentSz,
                                GLsizei width, GLint height);

    static void convertBGRtoRGBA(const GLubyte* input, GLubyte* output,
                                 GLsizei inputComponentSz, GLsizei outputComponentSz,
                                 GLsizei width, GLint height);

    static void convertRGBAtoINTENSITY(const GLubyte* input, GLubyte* output,
                                 GLsizei inputComponentSz, GLsizei outputComponentSz,
                                 GLsizei width, GLint height);

    static void convertALPHAtoALPHA(const GLubyte* input, GLubyte* output,
                                  GLsizei inputComponentSz, GLsizei outputComponentSz,
                                  GLsizei width, GLint height);
    
    static void convertLUMINANCEtoLUMINANCE(const GLubyte* input, GLubyte* output,
                                            GLsizei inputComponentSz, GLsizei outputComponentSz,
                                            GLsizei width, GLint height);

private:
    
    acdlib::ACDDevice* _acddev;

    enum { mipmapMaxSize = 13 };

    acdlib::ACDTexture2D* _texture2D;
    acdlib::ACDTexture3D* _texture3D;
    acdlib::ACDTexture2D* _textureRECT;
    acdlib::ACDTextureCubeMap* _textureCM;

    // Wrap modes
    GLenum _wrapS;
    GLenum _wrapT;
    GLenum _wrapR;

    // Texture filters
    GLenum _magFilter;
    GLenum _minFilter;

    // LOD levels
    GLfloat _minLOD;
    GLfloat _maxLOD;
    GLfloat _biasLOD;

    // Aniso
    GLfloat _maxAnisotropy;

    // Define the window of available mipmaps
    GLuint _baseLevel;
    GLuint _maxLevel; 
};

} // namespace agl

#endif // AGL_TEXTUREOBJECT
