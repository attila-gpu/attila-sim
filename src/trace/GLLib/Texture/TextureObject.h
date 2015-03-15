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

#ifndef TEXTUREOBJECT_H
    #define TEXTUREOBJECT_H

#include "gl.h"
#include "BaseObject.h"
#include "GPUTypes.h"
#include <vector>
#include "PushState.h"

namespace libgl
{

class TextureTarget;

class TextureObject : public BaseObject
{
public:

    class Mipmap
    {
        private:

            /* Mipmap required state */
            GLint width;
            GLint height;
            GLint depth;
            GLint border;
            GLint ifmt; // can be modified if the ifmt specified is not supported directly
            GLint red;
            GLint green;
            GLint blue;
            GLint alpha;
            GLint luminance;
            GLint intensity;

            bool imageCompression;

            GLubyte* originalData;
            GLubyte* data; // (binary data) after formating (Morton)
            u32bit size; // (binary size) after formating (Morton) or compressedSize

            GLenum format; // external type (just stored for computing getImageSize() )
            GLenum type;

        public:

            Mipmap();

            Mipmap(const Mipmap &mip);

            GLenum getFormat() const { return format; }
            const GLubyte* getData() { return data; }

            // returns size after applying morton order
            GLuint getSize() { return size; }

            GLuint getImageSize();
            GLint getInternalFormat() const { return ifmt; }
            bool reverseData() const;
            GLint getBorder() const { return border; }
            GLsizei getWidth() const { return width; }
            GLsizei getHeight() const { return height; }
            GLsizei getDepth() const { return depth; }
            bool isCompressed() const { return imageCompression; }

            void setWithDefaults();

            void set1D(GLint ifmt, GLint width, GLint border, GLenum format,
                       GLenum type, const GLvoid* data, GLint compressedSize = 0);

            void set2D(GLint ifmt, GLint width, GLint height, GLint border, GLenum format,
                       GLenum type, const GLvoid* data, GLint compressedSize = 0);

            void setPartial2D(GLsizei width, GLint xoffset, GLint yoffset, GLint height,
                       GLenum format, GLenum type, const GLvoid* data, GLint compressedSize = 0);

            void set3D(GLint ifmt, GLint width, GLint height, GLint depth, GLint border, GLenum format,
                       GLenum type, const GLvoid* data, GLint compressedSize = 0);



            static void convertRGBtoRGBA(const GLubyte* input, GLubyte* output,
                                         GLsizei inputComponentSz, GLsizei outputComponentSz,
                                         GLsizei width, GLint height);

            /*
             * Not used currently (simulator just use RGBA internal format)
             */
            static void convertRGBtoRGB(const GLubyte* input, GLubyte* output,
                                        GLsizei inputComponentSz, GLsizei outputComponentSz,
                                        GLsizei width, GLint height);

            /**
             * Allow to transform RGBA to RGBA with different components size
             *
             * @code
             *
             *   // RGBA16 (2 bytes per component) to RGBA8 (1 byte per component)
             *   convertRGBAtoRGBA(in_rgba16, out_rgba8, 2, 1, width, heigth)
             *
             * @endcode
             */
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
            /**
             * Allow to transform between ALPHA internal formats with different components size
             */
            static void convertALPHAtoALPHA(const GLubyte* input, GLubyte* output,
                                          GLsizei inputComponentSz, GLsizei outputComponentSz,
                                          GLsizei width, GLint height);
            
            /**
             * Allow to transform between LUMINANCES internal formats with different components size
             */
            static void convertLUMINANCEtoLUMINANCE(const GLubyte* input, GLubyte* output,
                                                    GLsizei inputComponentSz, GLsizei outputComponentSz,
                                                    GLsizei width, GLint height);
            ~Mipmap();

            void dump2PPM(GLubyte* filename);
    };

private:

    enum { mipmapArraySize = 13 }; // textures up to 4096x4096 texels

    /**
     * Face is always 0 with texture objects different of GL_TEXTURE_CUBE_MAP
     * pos is the position in the MipmapArray of the given face
     */
    void getMipmapPosition(GLuint portion, GLuint& face, GLuint& pos);

    typedef std::vector<Mipmap> MipmapArray;

    /**
     * if target == 1D 2D or 3D -> mips = new MipmapArray[1];
     * else ( CM ) -> mips = new MipmapArray[6];
     */
    MipmapArray* mips;

    GLubyte alignment; // only alignment 1 supported for now

    /**
     * True when a level diferent of 0 is defined
     */
    bool useMipmaps;
    
    /***********************************
     * Texture parameters              *
     *                                 *
     * See Opengl spec 2.0 table 3.19  *
     ***********************************/

    // Wrap modes
    GLenum wrapS;
    GLenum wrapT;
    GLenum wrapR;

    // Texture filters
    GLenum magFilter;
    GLenum minFilter;

    GLfloat maxAnisotropy;

    GLclampf borderColor[4];

    GLclampf texturePriority;

    GLfloat minLOD;
    GLfloat maxLOD;

    // Define the window of available mipmaps
    GLuint baseLevel;
    GLuint maxLevel;

    GLfloat biasLOD;

    GLenum depthTextureMode; // Can be LUMINANCE, INTENSITY, ALPHA
    GLenum compareMode; // Can be NONE, COMPARE_R_TO_TEXTURE
    GLenum compareFunc; // LEQUAL, GEQUAL, LESS, GREATER, EQUAL, NOTEQUAL, ALWAYS, NEVER

    GLboolean generateMipmap;


    /*
     * @note targetFace accepted
     *        GL_TEXTURE_1D
     *      GL_TEXTURE_2D
     *      GL_TEXTURE_3D
     *         GL_TEXTURE_CUBE_MAP_POSITIVE_X
     *        GL_TEXTURE_CUBE_MAP_NEGATIVE_X
     *         GL_TEXTURE_CUBE_MAP_POSITIVE_Y
     *         GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
     *        GL_TEXTURE_CUBE_MAP_POSITIVE_Z
     *         GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
     */
    Mipmap& getMipmap(GLenum targetFace, GLint level) const;

    friend class pushstate::TextureGroup; ///< Allows TextureGroup class to save and restore 
                                          ///  TextureUnit State for glPush/glPop Attribs.
public:


    ~TextureObject();

    GLsizei getWidth() const;
    GLsizei getHeight() const;
    GLsizei getDepth() const;
    GLenum getFormat() const;

    GLenum getWrapS() const { return wrapS; }
    GLenum getWrapT() const { return wrapT; }
    GLenum getWrapR() const { return wrapR; }

    GLenum getMinFilter() const { return minFilter; }
    GLenum getMagFilter() const { return magFilter; }

    GLfloat getMaxAnisotropy() const;

    Mipmap& getBaseMipmap() const;
    Mipmap& getFirstMipmap() const;

    GLsizei getMipmapWidth(GLenum targetFace, GLint level) const;
    GLsizei getMipmapHeight(GLenum targetFace, GLint level) const;
    GLsizei getMipmapDepth(GLenum targetFace, GLint level) const;

    GLint getInternalFormat() const;
    GLenum getBaseInternalFormat() const;

    bool getReverseTextureData() const;

    bool checkCompleteness() const;

    bool isCompressed() const;

    GLuint binarySize(GLuint portion = 0);
    const GLubyte* binaryData(GLuint portion = 0);

    /* Create a texture object */
    TextureObject(GLuint name, GLenum targetName);

    void setParameter(GLenum pname, const GLint* parameter);
    void setParameter(GLenum pname, const GLfloat* parameter);

    GLint getBaseLevel() const { return baseLevel; }
    GLint getMaxLevel() const { return maxLevel; }

    GLfloat getLodBias() const { return biasLOD; }
    
    /*
     * Returns q = min{p, maxLevel}, being p = ceil(log2(max(wb,hb,db))) + baseLevel
     * and being wb,hb,db the dimensions of baseLevel mipmap
     */
    GLuint getMaxActiveMipmap() const;
    GLuint getMinActiveMipmap() const;

    /**
     * The opposite as the private getMipmapPosition() method. Given the face and the mipmap position
     * returns the portion number in the memory object.
     */
    void getMipmapPortion(GLuint face, GLuint pos, GLuint& portion);

    GLenum getBaseInternalFormat(GLenum target);

    void setContents(
        GLenum targetFace,
        GLint level,
        GLint ifmt,
        GLsizei width,
        GLsizei height,
        GLint depth,
        GLint border,
        GLenum fmt,
        GLenum type,
        const GLubyte* data,
        GLint compressionSize = 0 // by default not compressed
        );


    void setPartialContents(
        GLenum targetFace,
        GLint level,
        GLint xoffset,
        GLint yoffset,
        GLint zoffset,
        GLsizei width,
        GLsizei height,
        GLsizei depth,
        GLenum fmt,
        GLenum type,
        const GLubyte* subData,
        GLint compressionSize = 0 // by default not compressed
        );


    /**
     * Synchronizes internal state and checks completeness
     */
    void sync();

    void dump(GLubyte* filename);

};

} // namespace libgl

#endif // TEXTUREOBJECT_H
