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

#ifndef AGL_CONTEXT
    #define AGL_CONTEXT

#include "gl.h"
#include "glext.h"
#include "ACDTypes.h"
#include "ACDDevice.h"
#include "ACDBuffer.h"
#include "ACDSampler.h"
#include "ACDShaderProgram.h"
#include "MatrixStack.h"
#include "ACDX.h"
#include "AGLTextureUnit.h"
#include "ACDRasterizationStage.h"
#include "ACDXTextCoordGenerationStage.h"
#include "ACDZStencilStage.h"

#include "ARBProgramManager.h"
#include "AGLBufferManager.h"
#include "AGLTextureManager.h"
#include "ACDBlendingStage.h"

#include <stack>
#include <bitset>
#include <set>
#include <vector>
#include <math.h>

namespace agl
{

enum AGL_RENDERSTATE
{
    RS_VERTEX_PROGRAM,
    RS_FRAGMENT_PROGRAM,
    RS_MAXRENDERSTATE
};

enum AGL_ARRAY
{
    VERTEX_ARRAY,
    NORMAL_ARRAY,
    COLOR_ARRAY,
    TEX_COORD_ARRAY_0,
    TEX_COORD_ARRAY_1,
    TEX_COORD_ARRAY_2,
    TEX_COORD_ARRAY_3,
    TEX_COORD_ARRAY_4,
    TEX_COORD_ARRAY_5,
    TEX_COORD_ARRAY_6,
    TEX_COORD_ARRAY_7,
    GENERIC_ARRAY_0,
    GENERIC_ARRAY_1,
    GENERIC_ARRAY_2,
    GENERIC_ARRAY_3,
    GENERIC_ARRAY_4,
    GENERIC_ARRAY_5,
    GENERIC_ARRAY_6,
    GENERIC_ARRAY_7,
    GENERIC_ARRAY_8,
    GENERIC_ARRAY_9,
    GENERIC_ARRAY_10,
    GENERIC_ARRAY_11,
    GENERIC_ARRAY_12,
    GENERIC_ARRAY_13,
    GENERIC_ARRAY_14,
    GENERIC_ARRAY_15,

};

//using namespace acdlib; // Open acdlib into namespace agl

class GLContext
{
public:

    typedef acdlib::acd_int acd_int;
    typedef acdlib::acd_uint acd_uint;
    typedef acdlib::acd_float acd_float;
    typedef acdlib::acd_ubyte acd_ubyte;
    typedef acdlib::acd_bool acd_bool;

    struct VArray
    {
        bool inicialized;
        GLuint size; ///< # of components
        GLenum type; ///< type of each components
        GLsizei stride; ///< stride
        const GLvoid* userBuffer; ///< user data
        GLuint bufferID; ///< bound buffer (0 means unbound)
        bool normalized;

        /**
         * Default constructor (empty means no penalty creation)
         */
        VArray() : inicialized(false), bufferID(0), normalized(false), stride(0), userBuffer(0), size(0), type(0){}

        /**
         * Create an initialized VArray struct
         *
         * @param size number of components that has any "item"
         * @param type type of each component (GL_FLOAT, GL_INT, etc)
         * @stride Vertex array stride
         * @userBuffer pointer to data (user data)
         */
        VArray(GLuint size, GLenum type, GLsizei stride, const GLvoid* userBuffer, bool normalized = false) :
            size(size), type(type), stride(stride), userBuffer(userBuffer), bufferID(0), normalized(normalized), inicialized(true)
        {}

        GLuint bytes(GLuint elems = 1) const;


        void dump() const
        {
            std::cout << "size: " << size << std::endl;
            std::cout << "type: " << type << std::endl;
            std::cout << "stride: " << stride << std::endl;
            std::cout << "normalized: " << normalized << std::endl;
        }

    };

    ARBProgramManager& arbProgramManager();
    BufferManager& bufferManager();
    GLTextureManager& textureManager();

    GLContext(acdlib::ACDDevice* acddev);

    void setLightParam(GLenum light, GLenum pname, const GLfloat* params);
    void setLightModel(GLenum pname, const GLfloat* params);
    void setMaterialParam(GLenum face, GLenum pname, const GLfloat* params);

    acdlib::ACDDevice& acd();

    acdlib::ACDXFixedPipelineState& fixedPipelineState();
    acdlib::ACDX_FIXED_PIPELINE_SETTINGS& fixedPipelineSettings();

    void setShaders();

    // Put data into private buffers
    void addVertex(acd_float x, acd_float y, acd_float z = 0, acd_float w = 1);

    void drawElements ( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices );

    void setColor(acd_float red, acd_float green, acd_float blue, acd_float alpha = 1);
    void setColor(acd_uint red, acd_uint green, acd_uint blue);
    void setNormal(acd_float x, acd_float y, acd_float z);

    void setClearColor(acd_ubyte red, acd_ubyte green, acd_ubyte blue, acd_ubyte alpha);
    void setClearColor(acd_float red, acd_float green, acd_float blue, acd_float alpha);
    void getClearColor(acd_ubyte& red, acd_ubyte& green, acd_ubyte& blue, acd_ubyte& alpha) const;

    void setDepthClearValue(acd_float depthValue);
    acd_float getDepthClearValue() const;

    void setStencilClearValue(acd_int stencilValue);
    acd_int getStencilClearValue() const;

    acd_uint countInternalVertexes() const;
    void initInternalBuffers(acd_bool createBuffers);
    void attachInternalBuffers();
    void deattachInternalBuffers();

    void attachInternalSamplers();
    void deattachInternalSamplers();
    
    /**
     * Accepts GL_PROJECTION_MATRIX, GL_MODELVIEW_MATRIX, GL_TEXTURE
     */
    void setMatrixMode(GLenum mode);

    void setActiveTextureUnit(acd_uint textureStage);
    void setActiveModelview(acd_uint modelviewStack);

    // Three flavors to access matrix stacks
    MatrixStack& matrixStack();
    MatrixStack& matrixStack(GLenum mode);
    MatrixStack& matrixStack(GLenum mode, acd_uint unit);

    void setRenderState(AGL_RENDERSTATE state, acd_bool enabled);
    acd_bool getRenderState(AGL_RENDERSTATE state) const;

    void dump() const;

    acd_uint obtainStream();
    void releaseStream(acd_uint streamID);

    acd_uint obtainSampler();
    void releaseSampler(acd_uint streamID);

    GLContext::VArray& getPosVArray();
    GLContext::VArray& getColorVArray();
    GLContext::VArray& getNormalVArray(); 
    GLContext::VArray& getIndexVArray(); 
    GLContext::VArray& getEdgeVArray();
    GLContext::VArray& getTextureVArray(GLuint unit);
    GLContext::VArray& getGenericVArray(GLuint unit);

    void setDescriptor ( AGL_ARRAY descr, GLint offset, GLint components, acdlib::ACD_STREAM_DATA type, GLint stride );
    void setVertexBuffer (acdlib::ACDBuffer *ptr);
    void setNormalBuffer (acdlib::ACDBuffer *ptr);
    void setColorBuffer (acdlib::ACDBuffer *ptr);
    void setTexBuffer (acdlib::ACDBuffer *ptr);
    
    void enableBufferArray (GLenum cap);
    void disableBufferArray (GLenum cap);
    void getIndicesSet(std::set<GLuint>& iSet, GLenum type, const GLvoid* indices, GLsizei count);
    acdlib::ACDBuffer* createIndexBuffer(const GLvoid* indices, GLenum indicesType, GLsizei count, const std::set<GLuint>& usedIndices);
    void setVertexArraysBuffers(GLuint start, GLuint count, std::set<GLuint>& iSet);
    acdlib::ACDBuffer* configureStream (VArray *array, bool colorAttrib, GLuint start, GLuint count, set<GLuint>& iSet);

    acdlib::ACDBuffer* fillVA_ind (GLenum type, const GLvoid* buf, GLsizei size, GLsizei stride, const std::set<GLuint>& iSet);
    acdlib::ACDBuffer* fillVAColor_ind (GLenum type, const GLvoid* buf, GLsizei size, GLsizei stride, const std::set<GLuint>& iSet);
    template<typename T> acdlib::ACDBuffer* fillVA_ind(const T* buf, GLsizei size, GLsizei stride, const std::set<GLuint>& iSet);
    template<typename A> acdlib::ACDBuffer* fillVAColor_ind(const A* buf, GLsizei size, GLsizei stride, const std::set<GLuint>& iSet);

    acdlib::ACDBuffer* fillVAColor(GLenum type, const GLvoid* buf, GLsizei size, GLsizei stride, GLsizei start, GLsizei count);
    acdlib::ACDBuffer* fillVA(GLenum type, const GLvoid* buf, GLsizei size, GLsizei stride, GLsizei start, GLsizei count);
    template<typename T> acdlib::ACDBuffer* fillVA(const T* buf, GLsizei size, GLsizei stride, GLsizei start, GLsizei count);
    template<typename A> acdlib::ACDBuffer* fillVAColor(const A* buf, GLsizei size, GLsizei stride, GLsizei start, GLsizei count);

    bool allBuffersBound() const;
    bool allBuffersUnbound() const;
    acdlib::ACDBuffer* rawIndices(const GLvoid* indices, GLenum type, GLsizei count);
    void resetDescriptors ();

    GLuint getSize(GLenum openGLType);

    void fillVertexArrayBuffer (AGL_ARRAY stream, GLint count);

    void setTexCoord(GLuint unit, GLfloat r, GLfloat s, GLfloat t = 0, GLfloat q = 1);

    TextureUnit& getTextureUnit(GLuint unit);
    TextureUnit& getActiveTextureUnit();
    GLuint getActiveTextureUnitNumber();
    void setFog(GLenum pname, const GLfloat* params);
    void setFog(GLenum pname, const GLint* params);

    void setCurrentClientTextureUnit(GLenum tex);
    GLenum getCurrentClientTextureUnit() const;
    void setEnabledCurrentClientTextureUnit(bool enabled);
    bool isEnabledCurrentClientTextureUnit() const;
    bool isEnabledClientTextureUnit(GLuint tex);


    GLContext::VArray& getTexVArray(GLuint unit);

    void setTexGen(GLenum coord, GLenum pname, GLint param);
    void setTexGen(GLenum coord, GLenum pname, const GLfloat* params);


    void setEnabledGenericAttribArray(GLuint index, bool enabled);
    bool isGenericAttribArrayEnabled(GLuint index) const;
    GLuint countGenericVArrays() const;

    void pushAttrib (GLbitfield mask);
    void popAttrib (void);

private:

    int _numTU;

    std::set<acd_uint> _freeStream;
    std::set<acd_uint> _freeSampler;

    // Managers
    ARBProgramManager _arbProgramManager;
    agl::BufferManager _bufferManager;
    agl::GLTextureManager _textureManager;

    acd_bool _renderStates[RS_MAXRENDERSTATE];

    // Fixed function emulation
    acdlib::ACDXFixedPipelineState* _fpState;
    acdlib::ACDX_FIXED_PIPELINE_SETTINGS _fpSettings;

    // Stream descriptors used with internal buffers
    acdlib::ACD_STREAM_DESC _vertexDesc;
    acdlib::ACD_STREAM_DESC _colorDesc;
    acdlib::ACD_STREAM_DESC _normalDesc;
    acdlib::ACD_STREAM_DESC _textureDesc[16];

    acd_uint _vertexStreamMap;
    acd_uint _colorStreamMap;
    acd_uint _normalStreamMap;
    acd_uint _textureStreamMap[16];

    acd_uint _textureSamplerMap[16];

    acdlib::ACDDevice* _acddev;

    // Buffers to emulate glBegin/glEnd paradigm
    acdlib::ACDBuffer* _vertexBuffer;
    acdlib::ACDBuffer* _colorBuffer;
    acdlib::ACDBuffer* _normalBuffer;
    acdlib::ACDBuffer* _textureBuffer[16];

    acd_bool _vertexVBO;
    acd_bool _colorVBO;
    acd_bool _normalVBO;
    acd_bool _textureVBO[16];

    acd_uint _bufferVertexes;   // Number of vertices added (glBegin/glEng)

    acd_float _currentColor[4];
    acd_float _currentNormal[3];
    acd_float _currentTexCoord[8][4];

    // Vertex arrays
    VArray _posVArray;
    VArray _colVArray;
    VArray _norVArray;
    VArray _indVArray;
    VArray _edgeVArray;
    std::vector<VArray> _texVArray;
    std::vector<VArray> _genericVArray;
    std::vector<acdlib::ACD_STREAM_DESC> _genericDesc;
    std::vector<acdlib::ACDBuffer*> _genericBuffer;
    std::vector<acd_uint> _genericStreamMap;
    GLboolean _genericAttribArrayEnabled;
    std::vector<bool> _genericAttribArrayFlags;

    acd_ubyte _clearColorR;
    acd_ubyte _clearColorG;
    acd_ubyte _clearColorB;
    acd_ubyte _clearColorA;
    
    acd_float _depthClearValue;
    acd_int _stencilClearValue;

    acdlib::ACDShaderProgram* _emulVertexProgram;
    acdlib::ACDShaderProgram* _emulFragmentProgram;
    acdlib::ACDShaderProgram* _currentFragmentProgram;

    std::vector<MatrixStack*> _stackGroup[3];
    acd_uint _currentStackGroup;

    acd_uint _activeTextureUnit;
    acd_uint _currentModelview;

    static acd_ubyte _clamp(acd_float v);

    acd_int _activeBuffer; /*        GL_VERTEX_ARRAY: bit 0
                                    GL_COLOR_ARRAY: bit 1
                                    GL_NORMAL_ARRAY: bit 2
                                    GL_INDEX_ARRAY: bit 3
                                    GL_TEXTURE_COORD_ARRAY: bit 4
                                    GL_EDGE_FLAG_ARRAY: bit 5*/

    /**
     * Texture units state
     */
    vector<TextureUnit> _textureUnits;
    float _fogStart;
    float _fogEnd;
    GLuint _currentClientTextureUnit;
	vector<bool> _clientTextureUnits;

    void _setSamplerConfiguration();

};

}

#endif // AGL_CONTEXT
