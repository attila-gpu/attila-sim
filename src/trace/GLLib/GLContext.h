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
 
#ifndef GLCONTEXT_H
    #define GLCONTEXT_H

#include "TLState.h"
#include "FPState.h"
#include "GLState.h"
#include "VSLoader.h"
#include "QuadReg.h"
#include "Matrixf.h"
#include "gl.h"
#include "DataBuffer.h"
#include "TextureUnitLib.h"
#include "GPUMemory.h"
#include "ProgramObjectCache.h"
#include "PushState.h"
#include "ContextStateAdapter.h"

#include <iostream>
#include <bitset>
#include <stack>
#include <iostream>

namespace libgl
{

#ifndef Quadf
    typedef QuadReg<float> Quadf;
#endif


    class libgl::VSLoader;


/**
 * Class GLContext stores all the state maintained by OpenGL
 *
 * This class also provides methods for udpating and querying this internal state
 * GLCOntext provides an interface for accesing to OpenGL state
 *
 * GLContext provides an intertace for access to
 *
 *    - Internal vertex buffers (position buffer, color buffer, texture buffer, etc)
 *    - State Matrixes (Modelview, projection, etc). Push, pop, top methods, etc
 *    - Several State flags (glBegin/glEnd mode, normals enabled, etc)
 *    - Vertex arrays
 *    - Lights state control
 *    - Current color, normal, primitive, etc
 *
 * Also does transparently & automatically:
 *
 *    - Synchronization of all dependant states (@see GLState object)
 *    - Select a proper shader (using VSloader) to emulate a fixed function
 *    - Check buffer overflows
 *
 * Some examples of use:
 *    - Creating a GLContext
 *    - Clearing buffers
 *    - Getting the size of a data buffer
 *    - Enabling a light & setting light parameters
 *    - Testing flags
 *    - GLContext Matrix operations: mtop, mpush & ctop
 *    - Vertex arrays
 *
 * @code
 *
 * GPUDriver* driver = GPUDriver::getGPUDriver(); // Gets the 3D card driver
 *
 * GLContext* ctx = new GLContext(driver); // Creates a GLContext that will use this driver
 *
 * // Examples of some internal buffer operations
 * ctx->clearBuffers(); // Clear all internal buffers (position, color, texture, etc)
 * ctx->posbuf().add(1,0,0,1).add(0,1,0,1); // Add two colors, red and green, to color buffer
 * usigned int bytes = ctx->posbuf().bytes(); // Number of bytes in the color buffer
 *
 *
 * // Example of GLContext lights interface
 * ctx->enableLight(GL_LIGHT0); // Enable a light (GL_LIGHT0)
 * GLfloat mat_ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
 * ctx->setParamLight(GL_LIGHT0, GL_AMBIENT, mat_ambient); // Modify a property of GL_LIGHT0
 *
 *
 * // Example of how to test a flag
 * if ( ctx->testFlags(GLContext::flagVP) ) // Example of texting a flag
 *     std::cout << "ARB Vertext programs enabled" << std::endl;
 * else
 *     std::cout << "Using conventional T&L" << std::endl;
 *
 *
 * // Example of some GLContext matrix manipulations
 * ctx->mpush(Matrixf::identity()); // Push a new identity matrix in current matrix
 * ctx->mpop(); // Remove that previously top matrix
 *
 * GLfloat matRows[] = {1.0f, 2.0f, -3.0f, 2.0f,
 *                      5.0f, 1.0f, -2.5f, 2.1f,
 *                     -1.0f, 0.5f, -1.5f, 3.25f,
 *                      1.0f, 2.0f, 3.0f, 4.0f};
 *
 * Matrixf mat(matRows); // Create a matrix using matRows (matrix sorted by rows)
 *
 * ctx->ctop(ctx->mtop() * mat); // Multiplies current top by 'mat' and stores the result in top
 *
 * // Example of Vertex arrays in GLContext
 * ctx->posVArray() = GLContext::Varray(4, GL_FLOAT, 0, vertexes); // initialize a Vertex array
 *
 * @endcode
 *
 * @version 1.0
 * @date 17/09/2004
 * @author Carlos González Rodríguez - cgonzale@ac.upc.es
 */
class GLContext
{
public:

    ProgramObjectCache vpCache;
    ProgramObjectCache fpCache;
    
    /**
     * Get the current vertex program that is selected
     * If GL_VERTEX_PROGRAM_ARB is enabled, returns the current user program bound
     * otherwise it returns the last automatically generated vertex program that
     * emulates T&L
     */
    ProgramObject& getCurrentVertexProgram();

    /**
     * Get the current fragment program that is selected
     * If GL_FRAGMENT_PROGRAM_ARB is enabled, returns the current user program bound
     * otherwise it returns the last automatically generated fragment program that
     * emulates T&L
     */
    ProgramObject& getCurrentFragmentProgram();

    /**
     * Struct for storing information about vertex arrays
     */
    struct VArray
    {
        GLuint size; ///< # of components
        GLenum type; ///< type of each components
        GLsizei stride; ///< stride
        const GLvoid* userBuffer; ///< user data
        GLuint bufferID; ///< bound buffer (0 means unbound)
        bool normalized;

        /**
         * Default constructor (empty means no penalty creation)
         */
        VArray() : bufferID(0), normalized(false) {}

        /**
         * Create an initialized VArray struct
         *
         * @param size number of components that has any "item"
         * @param type type of each component (GL_FLOAT, GL_INT, etc)
         * @stride Vertex array stride
         * @userBuffer pointer to data (user data)
         */
        VArray(GLuint size, GLenum type, GLsizei stride, const GLvoid* userBuffer, bool normalized = false) :
            size(size), type(type), stride(stride), userBuffer(userBuffer), bufferID(0), normalized(normalized)
        {}

        /**
         * Computes how many space occupies a given number of items
         *
         * @param elems number of items
         *
         * @code
         * GLContext::VArray va = GLContext::VArray(3, GL_FLOAT, 0, data);
         *
         * va.bytes(10); // means: "how many bytes occupies 10 items in this VArray ?"
         *               // 3 GL_FLOAT per item
         *               // 4 bytes per GL_FLOAT
         *               // 3 * 4 * 10 = 120 bytes
         * @endcode
         */
        GLuint bytes(GLuint elems = 1) const;


        void dump() const
        {
            std::cout << "size: " << size << std::endl;
            std::cout << "type: " << type << std::endl;
            std::cout << "stride: " << stride << std::endl;
            std::cout << "normalized: " << normalized << std::endl;
        }

    };

    typedef DataBuffer<float> DBUF;
    typedef GLuint MatrixType;
    typedef GLuint Flag;

private:

    bool _triangleSetupOnShader;

    /**
     * see methods setLibStateDirty and isLibStateDirty
     */
    bool libDirty;

    /**
     * Interface for accessing to local gpu memory
     */
    GPUMemory* mem;

    /**
     * Implements TLState
     */
    class TLStateImplementation; // define an implementation

    friend class GLContext::TLStateImplementation; // Allow internal access to parent class (GLContext)

    class TLStateImplementation : public TLState
    {
    private:
        GLContext* ctx; // parent
    public:
        TLStateImplementation(GLContext* parent);

        bool normalizeNormals();
        bool isLighting();
        bool isRescaling();
        bool infiniteViewer();
        bool localLightSource(Light light);
        void attenuationCoeficients(Light light, float& constantAtt, float& linearAtt, float& quadraticAtt);
        bool isSpotLight(Light light);
        bool isLightEnabled(Light light);
        int maxLights();
        bool separateSpecular();
        bool twoSidedLighting();
        bool anyLightEnabled();
        bool anyLocalLight();

        bool isCullingEnabled();
        bool isFrontFaceCulling();
        bool isBackFaceCulling();

        TLState::ColorMaterialMode colorMaterialMode();
        bool colorMaterialFrontEnabled();
        bool colorMaterialBackEnabled();

        bool isTextureUnitEnabled(GLuint texUnit);
        bool anyTextureUnitEnabled();
        int maxTextureUnits();
        TLState::TextureUnit getTextureUnit(GLuint unit);

        TLState::FogCoordSrc fogCoordSrc();
        bool fogEnabled();

    } tlState; // declare a TLState implementor object

    /**
     * Implements FPState
     */
    class FPStateImplementation; // define an implementation

    friend class GLContext::FPStateImplementation; // Allow internal access to parent class (GLContext)

    class FPStateImplementation : public FPState
    {
    private:
        GLContext* ctx; // parent
    public:

        FPStateImplementation(GLContext* parent);

        bool separateSpecular();
        bool anyTextureUnitEnabled();
        bool isTextureUnitEnabled(GLuint unit);
        int maxTextureUnits();
        FPState::TextureUnit getTextureUnit(GLuint unit);
        bool fogEnabled();
        FPState::FogMode fogMode();

        FPState::AlphaTestFunc alphaTestFunc();
        //float alphaTestRefValue();
        bool alphaTestEnabled();

    } fpState; // declare a TLState implementor object


    GPUDriver* driver; ///< Pointer to GPUDriver
 
    ContextStateAdapter stateAdapter; ///< Context State Adapter object to send OpenGL State changes to GPU.

    enum { NStacks = 3 }; ///< Number of stacks managed by GLContext

    GLuint id; ///< Context identifier (not used currently)
    
    bool hzTestActive;      /**<  Stores if the Hierarchical Z test is enabled.  */
    bool hzBufferValid;     /**<  Stores if the content of the HZ buffer is valid.  */
    
    glsNS::GLState gls; ///< All visible state for a ARB Vertex Program (shared with VSLoader)

    VSLoader vsl; ///< Shader loader & compiler (supports native ARB Vertex programs)

    std::stack<Matrixf> mstack[NStacks]; ///< Stacks for supporting push/pop matrices
    
    GLenum primitive; ///< Current rendering primitive.
    
    bool setupShProgramUpdated;     /**<  Flag storing if the triangle setup shader is updated.  */
    bool setupShConstantsUpdated;   /**<  Flag storing if the triangle setup shader constants are updated.  */

    /*  Stores the
    /**
     * General flags
     *
     * @note Soon will be a std::bitset<N> C++ object
     *
     * 0 -> inside/outside begin-end
     * 1 -> using normals
     * 2 -> VS enabled (0 means using T&L)
     * 3 -> FS enabled (0 means using fixed pipe)
     * 4 -> Lighting enabled
     * 5 -> Is local viewer
     * 6 -> Separate specular
     * 7 -> rescale normals
     * 8 -> normalize normals
     * 9 -> Two sided lighting
     * 10 -> fog enabled/disabled
     * 11 -> stencil test enabled/disabled
     * 12 -> alpha test enabled/disabled
     * 13 -> color material enabled/disabled
     * 14 -> Face culling enabled/disabled
     * 15 -> Depth testing enabled/disabled
     * 16 -> Polygon offset enabled/disabled
     * 17 -> Scissor test enabled/disabled
     * 18 -> Blending operation enabled/disabled
     * ...
     * 31 -> vertex array enabled
     * 30 -> color array enabled
     * 29 -> index array enabled
     * 28 -> normal array enabled
     * 27 -> texture array enabled
     * 26 -> edge flag array enabled
     */
    GLuint flags;

    /* Internal buffers */
    DBUF posBuffer; ///< Internal position (coordinates) buffer
    DBUF colBuffer; ///< Internal color buffer
    DBUF norBuffer; ///< Internal normal buffer
    DBUF* texBuffer; ///< Internal texture buffer
    // ...

    /* Vertex arrays */
    VArray posVa; ///< postion vertex array structure
    VArray colVa; ///< color vertex array structure
    VArray indVa; ///< indec color vertex array structure
    VArray* texVa; ///< texture vertex array structure
    VArray norVa; ///< normal vertex array structure
    VArray edgVa; ///< edge flag vertex array structure
    VArray indexesVa; ///< vertex array structure used for managinf index mode
    std::vector<VArray> genericVa;
    std::vector<bool> genericAttribArrayFlags;

    /**
     * Texture units state
     */
    std::vector<TextureUnit> textureUnits;

    /**
     * Current Client texture
     */
    GLuint currentClientTextureUnit;

    /**
     * Client texture unit flags
     */
    std::vector<bool> clientTextureUnits;
    
    /**
     * Friend directives to allow each subclass of AttribGroup to save and restore the related GLContext internal state
     */
    friend class pushstate::EnableGroup; 
    friend class pushstate::ColorGroup; 
    friend class pushstate::CurrentGroup; 
    friend class pushstate::DepthBufferGroup;
    friend class pushstate::FogGroup; 
    friend class pushstate::LightingGroup; 
    friend class pushstate::PolygonGroup; 
    friend class pushstate::ScissorGroup; 
    friend class pushstate::StencilGroup; 
    friend class pushstate::TransformGroup; 
    friend class pushstate::ViewportGroup; 
    friend class pushstate::TextureGroup; 
    
    pushstate::TstateStack stateStack;   ///< The GLContext stack to push Attribute groups for glPush/PopAttribs();
    
    /***************************
     * Lighting related state.
     ***************************/
    GLboolean shadeModel;          ///< Current shade model. True if smooth mode, false if flat mode.
    GLenum    colorMaterialMode;   ///< Color material parameter used in lighting computation.
    GLenum    colorMaterialFace;   ///< Color material face mode.
    
    std::bitset<MAX_LIGHTS_ARB> lights; ///< Flags for enable/disable lights.

    /*************************************************
     * Depth Buffer and Depth Test related state.
     *************************************************/
    GLclampd  depthClear;   /**< Stores the current depth clear value. */
    GLboolean depthMask;    /**< if true, updates z buffer, false, doesn't update it */
    GLenum    depthFunc;    /**< Stores the current depth function.  */

    /*************************************************
     * Stencil Buffer and Stencil Test related state.
     *************************************************/
    GLenum stFailUpdateFunc;    /**<  Stencil test update function for stencil fail.  */
    GLenum dpFailUpdateFunc;    /**<  Stencil test update function for depth test fail.  */
    GLenum dpPassUpdateFunc;    /**<  Stencil test update function for depth test pass.  */
    
    GLint  stBufferClearValue;  /**< Stencil buffer clear value. */
    GLuint stBufferUpdateMask;  /**< Stencil buffer write mask. */
    GLenum stBufferFunc;        /**< Stencil test compare function. */
    GLint  stBufferRefValue;    /**< Stencil test reference value to compare with. */
    GLuint stBufferCompMask;    /**< Stencil test comparison mask. */

    /**********************
     * FOG related state.
     **********************/
    GLenum fogMode;             /**< Current fog mode.  */
    GLenum fogCoordinateSource; /**< Current fog coordinate source. */

    /************************
     * Current values state.
     ************************/
    Quadf  currentColor;         ///< Current color
    Quadf  currentNormal;        ///< Current normal
    Quadf* currentTexCoord;      ///< Current Texture coordinate (one per texture unit)

    /********************************
     * Transformation related state.
     ********************************/
    GLuint currentMatrix; ///< Current work matrix
    
    /******************************************
     * Viewport and depth range related state
     ******************************************/
    GLint   viewportX;        /**<  Viewport start x coordinate.  */
    GLint   viewportY;        /**<  Viewport start y coordinate.  */
    GLsizei viewportWidth;    /**<  Viewport width.  */
    GLsizei viewportHeight;   /**<  Viewport height.  */

    GLclampd near_;          /**<  Depth near value. */
    GLclampd far_;           /**<  Depth far value.  */

    /********************************
     * Scissor Test Box related state.
     ********************************/
    GLint   scissorIniX;      /**< Initial x corner of scissor box. */
    GLint   scissorIniY;      /**< Initial y corner of scissor box. */
    GLsizei scissorWidth;     /**< Width dimension of scissor box. */
    GLsizei scissorHeight;    /**< Height dimension of scissor box. */

    /********************************
     * Polygon related state.
     ********************************/
    GLenum cullFace;        /**<  Stores the culling face. */
    GLenum faceMode;        /**<  Stores the polygon facing mode (CW or CCW).  */

    GLfloat slopeFactor;    ///< Polygon Offset slope factor parameter.
    GLfloat unitsFactor;    ///< Polygon Offset units parameter.
    
    /********************************
     * Color Buffer related state.
     ********************************/
    GLenum   alphaFunc;       ///< The alpha test comparison function.
    GLclampf alphaRef;        ///< The alpha test reference value.

    GLenum blendSFactorRGB;    ///< The blending equation Source Factor for RGB. (GL Spec. v1.3)
    GLenum blendDFactorRGB;    ///< The blending equation Destination Factor for RGB. (GL Spec. v1.3)
    GLenum blendSFactorAlpha;  ///< The blending equation Source Factor for Alpha. (GL Spec. v1.3)
    GLenum blendDFactorAlpha;  ///< The blending equation Destination Factor for Alpha. (GL Spec. v1.3)
    GLenum blendEquation;      ///< The blending equation (GL Spec. v1.5) 
    Quadf  blendColor;         ///< The blending equation constant color.
    
    GLboolean red;          ///< Color mask for red component.
    GLboolean green;        ///< Color mask for green component.
    GLboolean blue;         ///< Color mask for blue component.
    GLboolean alpha;        ///< Color mask for alpha component.
    
    Quadf clearColor;       ///< Color when clearing color buffer.

    /**************
     * DEBUG VARS *
     **************/
    GLuint curFrame;

public:


    operator TLState&() { return tlState; }

    TLState& getTLState() { return tlState; }

    operator FPState&() { return fpState; }

    FPState& getFPState() { return fpState; }


    static const Flag flagBeginEnd = 0x00000001; ///< Flag telling if we are in glBegin/glEnd block
    static const Flag flagUsingNormals = 0x00000002; ///< Flag telling if we are using normals
    static const Flag flagVS = 0x00000004; ///< Flag telling if we are using user vertex programs
    static const Flag flagFS = 0x00000008; ///< Flag telling if we are using user fragment programs
    static const Flag flagLighting = 0x00000010; ///< Flag telling if lighting is enabled or not
    static const Flag flagLocalViewer = 0x00000020; ///< Flag telling if local viewer is selected
    static const Flag flagSeparateSpecular = 0x00000040; ///< Flag telling specular component is applied after texturing
    static const Flag flagRescaleNormal = 0x00000080; ///< Flag telling auto-rescaling normals
    static const Flag flagNormalize = 0x00000100; ///< Flag telling auto-normalize normals
    static const Flag flagTwoSidedLighting = 0x00000200; ///< Flag telling if lighting is applied to both front & back faces
    static const Flag flagFog = 0x00000400; ///< Flag telling if fog is enabled/disabled
    static const Flag flagStencilTest = 0x00000800; ///< Enable/disable stencil test
    static const Flag flagAlphaTest = 0x00001000; ///< Enable/disable alpha test
    static const Flag flagColorMaterial = 0x00002000; ///< Enable/disable colorMaterial
    static const Flag flagCullFace = 0x00004000; ///< Enable/disable face culling
    static const Flag flagDepthTest = 0x00008000; ///< Enable/disable depth testing
    static const Flag flagPolygonOffsetFill = 0x00010000; ///< Enable/disable Polygon Offset for fill rasterization mode
    static const Flag flagScissorTest = 0x00020000; ///< Enable/disable Scissor test
    static const Flag flagBlending = 0x00040000;    ///< Enable/disable blending
    
    // deprecated
    //static const Flag flagTexture2D; ///< Flag telling texture 2D is enabled

    /* Vertex arrays flags */
    static const Flag flagVaPos = 0x80000000; ///< Flag telling if we are using position vertex array
    static const Flag flagVaCol = 0x40000000; ///< Flag telling if we are using color vertex array
    static const Flag flagVaInd = 0x20000000; ///< Flag telling if we are using index color vertex array
    static const Flag flagVaNor = 0x10000000; ///< Flag telling if we are using normal vertex array
    static const Flag flagVaTex = 0x08000000; ///< Flag telling if we are using texture vertex array
    static const Flag flagVaEdg = 0x04000000; ///< Flag telling if we are using edge flags vertex array

    static const MatrixType MODELVIEW = 0; ///< Identifier for modelview matrix
    static const MatrixType PROJECTION = 1; ///< Identifier for projection matrix
    static const MatrixType TEXTURE = 2; ///< Identifier for texture matrix

    /**
     * Returns the current active texture unit
     */
    TextureUnit& getActiveTextureUnit();

    /**
     * Checks if any Texture unit have an enabled target
     */
    bool areTexturesEnabled() const;

    /**
     * Returns the number of texture units available
     */
    GLuint countTextureUnits() const { return (GLuint) textureUnits.size(); }

    /**
     * Helper function for create 1-bit enabled mask
     *
     * @param number of enabled bit
     *
     * @return the created flag
     */
    static Flag makeMask(GLuint bit);

    /**
     * Create a GLContext
     *
     * @param driver a pointer to a GPUDriver
     */
    GLContext(GPUDriver* driver, bool triangleSetupOnShader);

    /**
     * GLContext destructor
     *
     */
    ~GLContext();
    
    /**
     * Returns a reference to GPUMemory object which is responsible of managing BaseObjects
     * synchronization
     */
    GPUMemory& gpumem();

    /**
     * Pushes a new matrix in one of the available stacks
     *
     * @param newTop new matrix that is the new stack top
     * @param mt the stack where newTop is pushed
     *
     * @code
     *     // push identity in PROJECTION stack
     *     ctx->mpush(Matrixf::identity(), GLContext::PROJECTION);
     * @endcode
     */
    void mpush(const Matrixf& newTop, MatrixType mt);

    /**
     * Pushes anew matrix in the current selected stack matrix
     *
     * @param newTop new matrix that is the new stack top (of the current matrix stack)
     *
     * @code
     *    // push identity in texture matrix stack
     *    ctx->setCurrentMatrix(GLContext::TEXTURE);
     *    ctx->mpush(Matrixf::identity());
     * @endcode
     */
    void mpush(const Matrixf& newTop);

    /**
     * Pops the top matrix of one of the available stacks
     *
     * @param mt matrix stack (ie: GLContext::MODELVIEW)
     */
    void mpop(MatrixType mt);

    /**
     * Pops the top of the current matrix stack
     */
    void mpop();


    /**
     * Returns the top of one of the available matrix stacks
     *
     * @param mt matrix stack (ie: GLContext::MODELVIEW)
     * @return the matrix in the top of the stack
     */
    const Matrixf& mtop(MatrixType mt);

    /**
     * Returns the top of the current matrix stack
     *
     * @return the matrix in the top of the stack
     */
    const Matrixf& mtop();

    /* Changes the contents of current top */
    /* Implicity updates associated GLState */

    /**
     * Changes the value of one of the available matrix stack top
     *
     * @param newTop new top contents
     * @param mt target matrix stack (ie: GLContext:PROJECTION)
     */
    void ctop(const Matrixf& newTop, MatrixType mt);

    /**
     * Change the value of the current matrix stack top
     *
     * @param newTop new top contents
     */
    void ctop(const Matrixf& newTop);


    /**
     *
     *  Sets the viewport start position and dimension.
     *
     *  @param x Viewport start x coordinate.
     *  @param y Viewport start y coordinate.
     *  @param width Viewport width.
     *  @param height Viewport height.
     *
     */

    void setViewport(GLint x, GLint y, GLsizei width, GLsizei height);

    /**
     *
     *  Sets the polygon facing mode (CW or CCW).
     *
     *  @param faceMode The polygon facing mode.
     *
     */

    void setFaceMode(GLenum faceMode);

    /**
     *
     *  Sets the triangle setup shader program update state.
     *
     *  @param updated Stores if the setup shader program is updated.
     *
     */

    void setSetupShaderProgramUpdate(bool updated);

    /**
     *
     *  Sets the triangle setup shader consntas update state.
     *
     *  @param updated Stores if the setup shader constants are updated.
     *
     */

    void setSetupShaderConstantsUpdate(bool updated);

    /**
     *
     *  Gets the viewport start position and dimension.
     *
     *  @param x Reference to a variable where to store the viewport start x coordinate.
     *  @param y Reference to a variable where to store the viewport start y coordinate.
     *  @param width Reference to a variable where to store the viewport width.
     *  @param height Reference to a variable where to store the viewport height.
     *
     */

    void getViewport(GLint &x, GLint &y, GLsizei &width, GLsizei &height) const;

    /**
     *
     *  Gets the polygon facing mode (CW or CCW).
     *
     *  @return The current polygon facing mode.
     *
     */

    GLenum getFaceMode() const;

    /**
     *
     *  Returns if the triangle setup shader program is updated.
     *
     *  @return The triangle setup shader program update state.
     *  @note This method returns always true if TriangleSetupOnShader option is FALSE in bGPU.ini
     *
     */

    bool isSetupShaderProgramUpdated() const;

    /**
     *
     *  Returns if the triangle setup shader constants are updated.
     *
     *  @return The state of the triangle setup shader constants, if they are update.
     *  @note This method returns always true if TriangleSetupOnShader option is FALSE in bGPU.ini
     */

    bool areSetupShaderConstantsUpdated() const;


    /**
     * Sets current color
     *
     * @param r red contribution
     * @param g green contribution
     * @param b blue contribution
     * @param a alpha value
     */
    void setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);

    /**
     * Sets current color
     *
     * @param rgba Quadf containing RGBA values
     */
    void setColor(Quadf& rgba);

    /**
     * Gets current color
     *
     * @return current color
     */
    const Quadf& getColor() const;

    /**
     * Sets the current normal
     *
     * @param x first normal coordinate
     * @param y second normal coordinate
     * @param z third normal coordinate
     */
    void setNormal(GLfloat x, GLfloat y, GLfloat z);

    /**
     * Sets current normal
     *
     * @param xyz Quadf containing ht e3 coordinates of a normal, fourth coordinate is ignored
     */
    void setNormal(Quadf& xyz);

    /**
     * Gets current normal
     *
     * @return current normal
     */
    const Quadf& getNormal() const;


    /**
     * Sets the current texture coordinate for a given texture unit
     */
    void setTexCoord(GLuint unit, GLfloat r, GLfloat s, GLfloat t = 0, GLfloat q = 1);

    const Quadf& getTexCoord(GLuint unit = 0) const;

    /**
     * Sets current primitive
     *
     * @param p GLenum containing a primitive value
     */
    void setPrimitive(GLenum p);

    /**
     * Gets current primitive
     *
     * @return current primitive
     */
    GLenum getPrimitive();

    /**
     * Sets shadeModel
     *
     * @param isSmooth true means GL_SMOOTH behaviour, false means GL_FLAT
     */
    void setShadeModel(GLboolean isSmooth); /* false means GL_FLAT */

    /**
     * Returns current shade model
     *
     * @return true if GL_SMOOTH is enabled, false if GL_FLAT is enabled
     */
    GLboolean getShadeModel();

    /**
     * Selects a matrix to be the current matrix (the current matrix stack)
     *
     * @param mt Matrix stack target
     */
    void setCurrentMatrix(GLenum mt);

    GLuint getCurrentMatrix() const {   return currentMatrix;   }
    
    /**
     * Resets all flags with new values
     */
    void setNewFlags(Flag newFlags);

    /**
     * Enables some flags
     *
     * @param orMask OR mask of flags
     *
     * @code
     *     ctx->setFlags((Flag)0x10); // Enable bit 1 of globals flags
     *     ctx->setFlags(GLContext::flagVP); // preferred form...
     * @endcode
     */
    void setFlags(Flag orMask); /* OR with previous */

    /**
     * Resets some flags
     *
     * @param mask erase flags mask
     *
     * @code
     *     ctx->resetFlags((Flag)0x110); // Reset bits 1 and 2 of global flags
     *     ctx->resetFlags(GLContext::flagVP | GLContext::flagUsingNormals); // preferred form...
     * @endcode
     */
    void resetFlags(Flag mask);

    /**
     * Checks some flags
     *
     * @param andMask AND mask
     * @return true if AND with global flags is != 0, false otherwise
     */
    bool testFlags(Flag andMask) const; /* AND checking */

    /**
     * Checks if a flag is disabled, if it is disabled enables(sets) the flag and set libDirty to true,
     * otherwise it does nothing
     */
    void testAndSetFlags(Flag f);

    /**
     * Checks if a falg is enabled, if it is enabled disables(resets) the flag and set libDirty to true,
     * otherwise it does nothing
     */
    void testAndResetFlags(Flag f);

    /**
     * Gets global flags
     *
     * @return global flags
     */
    Flag getFlags();

    /**
     * Gets the associated GLState
     *
     * @return a reference to internal GLState
     *
     * @note constness warranties only query (not modify) operations are allowed
     */
    const glsNS::GLState& getGLState() const;

    /**
     * Gets internal position vertex buffer
     *
     * @return internal position vertex buffer
     */
    DBUF& posbuf();

    /**
     * Gets internal color vertex buffer
     *
     * @return internal color vertex buffer
     */
    DBUF& colbuf();

    /**
     * Gets internal texture vertex buffer
     *
     * @return internal texture vertex buffer (of a texture unit)
     */
    DBUF& texbuf(GLuint unit=0);

    /**
     * Gets internal normal vertex buffer
     *
     * @return internal normal vertex buffer
     */
    DBUF& norbuf();

    /*
     * Clears all internal buffers
     */
    void clearBuffers();

    /**
     * Gets/sets position vertex array attributes
     *
     * @return position vertex array attributes reference
     */
    VArray& posVArray();

    /**
     * Gets/sets color vertex array attributes
     *
     * @return color vertex array attributes reference
     */
    VArray& colVArray();

    /**
     * Gets/sets index color vertex array attributes
     *
     * @return index color vertex array attributes reference
     */
    VArray& indVArray();

    /**
     * Gets/sets normal vertex array attributes
     *
     * @return normal vertex array attributes reference
     */
    VArray& norVArray();

    /**
     * Gets/sets texture vertex array attributes
     *
     * @return texture vertex array attributes reference
     */
    VArray& texVArray(GLuint unit);

    /**
     * Gets/sets edge flag vertex array attributes
     *
     * @return edge flag vertex array attributes reference
     */
    VArray& edgVArray();

    /**
     * Gets/sets indexes properties
     *
     * @note This is not a vertex array but uses same interface (VArray object)
     *
     * @return indexes properties reference
     */
    VArray& indexesVArray();

    /**
     * Gets/sets properties of a generic vertex attribute pointer
     */
    VArray& genericVArray(GLuint attrib);

    /**
     * Enables/Disables a generic vertex attribute array
     */
    void setEnabledGenericAttribArray(GLuint index, bool enabled);

    /**
     * Checks if a generic attrib array is enabled
     */
    bool isGenericAttribArrayEnabled(GLuint index) const;

    /**
     * Number of generic attrib arrays available
     */
    GLuint countGenericVArrays() const;

    /**
     * Selects the proper shader depending on the state of GLContext
     *
     * @return a driver memory descriptor for allowing the release of resources after shader use
     */
    //GLuint initShaderProgram(GLenum target);

    /**
     * Enables a GL Light
     *
     * @param GLenum identifying a light
     */
    void enableLight(GLuint light);

    void disableLight(GLuint light);

    bool isLightEnabled(GLuint light) const;

    /**
     * Sets a light property
     *
     * @param light target light
     * @param pname parameter identifier (parameter name)
     * @param params new value
     */
    void setParamLight(GLenum light, GLenum pname, const GLfloat* params);

    /**
     * Sets a material property
     *
     * @param target face (GL_FRONT or GL_BACK)
     * @param pname parameter identifier (parameter name)
     * @param params new value
     */
    void setMaterial(GLenum face, GLenum pname, const float* params);

    /**
     * Sets light model properties
     *
     * @param pname property selector:
     *        - GL_LIGHT_MODEL_AMBIENT : ambient RGBA intensity of the entire scene
     *        - GL_LIGHT_MODEL_LOCAL_VIEWER : how specular reflection angles are computed
     *        - GL_LIGHT_MODEL_TWO_SIDE : choose between one-sided or two-sided lighting
     *
     * @param params new value
     */
    void setLightModel(GLenum pname, const float* params);

    /**
     * Overloaded version
     */
    void setLightModel(GLenum pname, const GLint* params);

    /**
     * Gets the texture unit with name 'unit', being unit a value from 0 to MAX_TEXTURE_UNITS-1
     */
    TextureUnit& getTextureUnit(GLuint unit);

    /**
     * Dump current gl context
     */
    void dump(std::ostream& os = std::cout);

    /**
     * Texture coordinates generation
     */
    void setTexGen(GLenum coord, GLenum pname, GLint param);

    void setTexGen(GLenum coord, GLenum pname, const GLfloat* param);

    /**
     * Sets/gets the current client textyre unit
     * Used in vertex arrays with multitexture (see glTexCoordPointer)
     */
    void setCurrentClientTextureUnit(GLenum tex);
    GLenum getCurrentClientTextureUnit() const;
    void setEnabledCurrentClientTextureUnit(bool enabled);
    bool isEnabledCurrentClientTextureUnit() const;
    bool isEnabledClientTextureUnit(GLuint tex);

    /**
     * Sets the current environment color to the specified texture unit
     *
     */
    void setTexEnvColor(GLuint unit, GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1);

    /**
     * Sets fog
     */
    void setFog(GLenum pname, const GLfloat* params);
    void setFog(GLenum pname, const GLint* params);

    /**
     * Get the current fog mode
     */
    GLenum getFogMode() const { return fogMode; }

    /**
     * Get the current fog coordinate source
     */
    GLenum getFogCoordSource() const { return fogCoordinateSource; }

    /**
     * Configure alpha test
     */
    void setAlphaTest(GLenum alphaFunc, GLclampf alphaRef);

    /**
     * Get the alpha test reference value
     */

    GLclampf getAlphaTestReferenceValue() const;


    GLenum getAlphaFunc() const;

    /**
     *
     *  Sets the Depth Function.
     *
     *  @param depthFunc OpenGL identifier for the depth comparation function.
     *
     */

    void setDepthFunc(GLenum depthFunc);

    /**
     *
     *  Returns the depth function.
     *
     *  @return The current depth comparation function.
     *
     */

    GLenum getDepthFunc();

    /**
     *
     *  Sets the stencil test update functions.
     *
     *  @param stFail Update function for stencil test fail.
     *  @param dpFail Update function for depth test fail.
     *  @param dpPass Update function for depth test pass.
     *
     */

    void setStencilOp(GLenum stFail, GLenum dpFail, GLenum dpPass);

    /**
     *
     *  Gets the stencil test update functions.
     *
     *  @param stFail Reference to a variable where to store the update function for stencil test fail.
     *  @param dpFail Reference to a variable where to store the update function for depth test fail.
     *  @param dpPass Reference to a variable where to store the update function for depth test pass.
     *
     */

    void getStencilOp(GLenum &stFail, GLenum &dpFail, GLenum &dpPass);


    /**
     *
     *  Sets the Hierarchical Z buffer valid flag.
     *
     *  @param valid TRUE if the HZ buffer is set to valid, FALSE if the HZ buffer is set to invalid.
     *
     */

    void setHZBufferValidFlag(bool valid);

    /**
     *
     *  Returns if the Hierarchical Z buffer is valid.
     *
     *  @return TRUE if the HZ buffer is valid, FALSE otherwise.
     *
     */

    bool isHZBufferValid();

    /**
     *
     *  Sets if the Hierarchical Z test is enabled.
     *
     *  @param active TRUE to enable the HZ test, FALSE to set the HZ test as disabled.
     *
     */

    void setHZTestActiveFlag(bool active);

    /**
     *
     *  Returns if the Hierarchical Z Test is enabled.
     *
     *  @return TRUE if the HZ test is enabled, FALSE otherwise.
     *
     */

    bool isHZTestActive();

    
    void setStencilBufferClearValue(GLint value);
    
    void setStencilBufferUpdateMask(GLuint mask);
    
    void setStencilBufferFunc(GLenum func, GLint ref, GLuint mask);
    
    GLint getStencilBufferClearValue() const {  return stBufferClearValue; }
    
    GLuint getStencilBufferUpdateMask() const { return stBufferUpdateMask; }
    
    GLenum getStencilBufferFunc()   const   {   return stBufferFunc;   }
    
    GLint getStencilBufferReferenceValue()  const   {   return stBufferRefValue; }
    
    GLuint getStencilBufferCompareMask()   const   {   return stBufferCompMask;   }
    
    /**
     * Set color material properties
     */
    void setColorMaterial(GLenum face, GLenum mode);
    
    /**
     * Get color material properties
     */
    void getColorMaterial(GLenum& face, GLenum& mode) const;
    

    VSLoader& getShaderSetup() { return vsl; }

    /**
     * Set culling face
     */
    void setCullFace(GLenum face);

    /**
     *
     *  Returns the culling face
     */

     GLenum getCullFace();

    /**
     * Set Polygon Offset parameters
     */
     void setPolygonOffset( GLfloat factor, GLfloat units );

    /**
     * Returns the Polygon Offset parameter
     */

    void getPolygonOffset( GLfloat& factor, GLfloat& units) const;

    /**
     * Returns the Polygon Offset units parameter
     */

    GLfloat getPolygonOffsetUnits() const;

    void setDepthMask(bool flag);

    bool getDepthMask() const;
    
    void setDepthClearValue(GLclampd depth);
    
    GLclampd getDepthClearValue() const {   return depthClear;  }

    void setColorMask(GLboolean _red, GLboolean _green, GLboolean _blue, GLboolean _alpha);
    
    void getColorMask(GLboolean& _red, GLboolean& _green, GLboolean& _blue, GLboolean& _alpha) const;
    
    
    void setDepthRange( GLclampd near_val, GLclampd far_val );

    void getDepthRange( GLclampd& near_val, GLclampd& far_val ) const;
    
    /**
     * Sets Scissor parameters
     */
    void setScissor(GLint x, GLint y, GLsizei width, GLsizei height);

    /**
     * Gets Scissor parameters
     */
    void getScissor(GLint& x, GLint& y, GLsizei& width, GLsizei& height) const;
    
    /**
     * Sets blending parameters
     */
    void setBlendFactor(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
    
    /**
     * Gets blending parameters
     */
    void getBlendFactor(GLenum& srcRGB, GLenum& dstRGB, GLenum& srcAlpha, GLenum& dstAlpha) const;
    
    /**
     * Sets blending equation
     */
    void setBlendEquation(GLenum mode);
    
    /**
     * Sets blending equation
     */
    GLenum getBlendEquation() const;
    
    /**
     * Sets blending color
     *
     * @param r red contribution
     * @param g green contribution
     * @param b blue contribution
     * @param a alpha value
     */
    void setBlendColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    
    /**
     * Gets blending color
     */
    void getBlendColor(GLfloat& r, GLfloat& g, GLfloat& b, GLfloat& a) const;
    
    /**
     * Sets the Color Buffer clear value
     *
     * @param r red contribution
     * @param g green contribution
     * @param b blue contribution
     * @param a alpha value
     */
    void setClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    
    /**
     * Gets clear color of color buffer
     */
    void getClearColor(GLfloat& r, GLfloat& g, GLfloat& b, GLfloat& a) const;
     
     
    /**
     * Pushes the Attrib Group State in the top of a Stack
     *
     */
    void pushAttrib(GLbitfield mask);
    
    /**
     * Restores the GLContext state with the Attrib Group Stack top
     */
    void popAttrib();
    
      
    /**
     * Must be set each time a library state requires shader building or recompiling
     */
    void setLibStateDirty(bool dirty);

    /**
     * Checks if current shader must be built again or not
     *
     * if return false the current shaders don't have to be built or recompiled again
     * if return true, shaders should be rebuilt again (or recompiled)
     */
    bool isLibStateDirty() { return libDirty; }


    /**
     * Returns true if any enabled vertex array pointer is using data from a VBO
     */
    bool anyBufferBound() const;

    bool allBuffersBound() const;

    bool allBuffersUnbound() const;




    /*****************
     * DEBUG METHODS *
     *****************/
private:

    bool clearStencilWarning;
    GLuint startFrame;

public:

    void setClearStencilWarning(bool flag) { clearStencilWarning = flag; }
    bool checkClearStencilWarning() const { return clearStencilWarning; }
    
    /**
     * Tells the associated driver to preload gpu local memory data writes
     * Used to implement the path "HOT START" within the GPU simulator
     */ 
    void setPreloadMemory(bool enable);
    

    GLuint currentFrame() const { return curFrame; }

    void currentFrame(GLuint frame) { curFrame = frame; }
    
    /*
     * Preloads all the local memory instantaneously 
     */
    void preloadMemory(bool isDrawElements = false);

};

} // namespace libgl

#endif
