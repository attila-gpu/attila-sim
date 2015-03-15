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
 
#include "GLContext.h"
#include <algorithm>
#include "ProgramManager.h"
#include <iostream>
#include "MathLib.h"
#include "TextureManager.h"
#include "AuxFuncsLib.h"
#include "GLResolver.h"
#include "GPULibInternals.h"
#include <sstream>

using namespace libgl;
using namespace std;

GLuint GLContext::VArray::bytes(GLuint elems) const
{
    return (size * afl::getSize(type) * elems);
}



GLContext::GLContext(GPUDriver* driver, bool triangleSetupOnShader) :
    driver(driver),
    _triangleSetupOnShader(triangleSetupOnShader),
    stateAdapter(driver),
    mem(0),
    tlState(this),
    fpState(this),
    vsl(this, driver),
    primitive(GL_TRIANGLES),
    id(0),
    posBuffer(4,1000),
    colBuffer(4,1000),
    norBuffer(3,1000),
    //texBuffer(3,1000),
    textureUnits(driver->getTextureUnits()),
    clientTextureUnits(driver->getTextureUnits(), false),
    currentClientTextureUnit(0),
    hzTestActive(false),
    hzBufferValid(false),
    setupShProgramUpdated(false),
    setupShConstantsUpdated(false),
    genericVa(16),
    genericAttribArrayFlags(16, false),
    libDirty(true), // initially the internal libreary state is not synchronized with respect to current shaders
    flags(0),
    shadeModel(GL_TRUE),
    colorMaterialMode(GL_AMBIENT_AND_DIFFUSE),
    colorMaterialFace(GL_FRONT_AND_BACK),
    lights(0),
    depthClear(1.0f),
    depthMask(true),
    depthFunc(GL_LESS),
    stFailUpdateFunc(GL_KEEP),
    dpFailUpdateFunc(GL_KEEP),
    dpPassUpdateFunc(GL_KEEP),
    stBufferClearValue(0),
    stBufferUpdateMask(0xFFFF),
    stBufferFunc(GL_ALWAYS),
    stBufferRefValue(0),
    stBufferCompMask(0xFFFF),
    fogMode(GL_EXP),
    fogCoordinateSource(GL_FRAGMENT_DEPTH),
    currentColor(1,1,1,1),
    currentNormal(0,0,1,1),
    currentMatrix(MODELVIEW),
    viewportX(0),
    viewportY(0),
    near_(0.0f),
    far_(1.0f),
    scissorIniX(0),
    scissorIniY(0),
    scissorWidth(512),
    scissorHeight(512),
    cullFace(GL_BACK),
    faceMode(GL_CCW),
    slopeFactor(0.0f),
    unitsFactor(0.0f),
    alphaFunc(GL_ALWAYS),
    alphaRef(0),
    blendSFactorRGB(GL_ONE),
    blendDFactorRGB(GL_ZERO),
    blendSFactorAlpha(GL_ONE),
    blendDFactorAlpha(GL_ZERO),
    blendEquation(GL_FUNC_ADD),
    blendColor(0,0,0,0),
    red(true),
    green(true),
    blue(true),
    alpha(true),
    clearColor(0,0,0,0),
    vpCache(100, 0x00FFFFFF, GL_VERTEX_PROGRAM_ARB),
    fpCache(100, 0x0FFFFFFF, GL_FRAGMENT_PROGRAM_ARB)
{

    driver->setContext(this);

    GLuint nTUs = driver->getTextureUnits();

    /* Push identity matrix in all stacks */
    GLuint i;
    for (i = 0;i < NStacks; i++)
        mstack[i].push(Matrixf::identity());

    currentTexCoord = new Quadf[nTUs];

    texBuffer = new DBUF[nTUs];
    for (i = 0; i < nTUs; i++ ) // Can consume a lot of memory...
    {
        currentTexCoord[i] = Quadf(0,0,0,1);
        texBuffer[i].set(1000,4);
    }

    texVa = new VArray[nTUs];

    if (driver->isResolutionDefined())
    {
        u32bit width, height;
        driver->getResolution(width, height);

        viewportWidth = scissorWidth = (GLsizei)width;
        viewportHeight = scissorHeight = (GLsizei)height;
    }
}

void GLContext::setLibStateDirty(bool dirty)
{ 
    libDirty = dirty; 
}


TextureUnit& GLContext::getActiveTextureUnit()
{
    return getTextureUnit(TextureManager::instance().getCurrentGroup());
}


void GLContext::setCurrentMatrix(GLenum mt)
{
    switch ( mt )
    {
        case GL_MODELVIEW:
            currentMatrix = MODELVIEW;
            break;
        case GL_PROJECTION:
            currentMatrix = PROJECTION;
            break;
        case GL_TEXTURE:
            currentMatrix = TEXTURE;
            break;
        default:
            panic("GLContext","setCurrentMatrix()","GLenum Matrix not supported");
    }
}

GPUMemory& GLContext::gpumem()
{
    if ( mem == 0 )
        mem = new GPUMemory(driver);
    return *mem;

}

void GLContext::mpush(const Matrixf& newTop, MatrixType mt)
{
    /* Synchronize GLState */
    switch ( mt )
    {
        case MODELVIEW:
            gls.setMatrix(glsNS::M_MODELVIEW, 0, newTop);
            break;
        case PROJECTION:
            gls.setMatrix(glsNS::M_PROJECTION, 0, newTop);
            break;
        case TEXTURE:
        {
            GLuint tu = TextureManager::instance().getCurrentGroup();
            gls.setMatrix(glsNS::M_TEXTURE, tu, newTop);

            //  Set library state as dirty.  When the Texture Matrix changes from identity to non identity
            //  the vertex shader must be updated with a matrix texture coordinate multiplication.
            libDirty = true;

            break;
        }
        default:
            panic("GLContext","mpush()","Invalid current matrix");
    }
    mstack[mt].push(newTop);

}

void GLContext::mpush(const Matrixf& newTop)
{
    mpush(newTop,currentMatrix);
}


void GLContext::mpop(MatrixType mt)
{
    std::stack<Matrixf>& m = mstack[mt];
    if ( m.empty() )
        panic("GLContext","mpop()", "Trying to pop an empty matrix stack");

    m.pop(); /* discard current matrix */

    /* Synchronize GLState */
    switch ( mt )
    {
        case MODELVIEW:
            gls.setMatrix(glsNS::M_MODELVIEW, 0, m.top());
            break;
        case PROJECTION:
            gls.setMatrix(glsNS::M_PROJECTION, 0, m.top());
            break;
        case TEXTURE:
        {
            GLuint tu = TextureManager::instance().getCurrentGroup();
            gls.setMatrix(glsNS::M_TEXTURE, tu, m.top());

            //  Set library state as dirty.  When the Texture Matrix changes from identity to non identity
            //  the vertex shader must be updated with a matrix texture coordinate multiplication.
            libDirty = true;

            break;
        }
        default:
            panic("GLContext","mpop()","Invalid current matrix");
    }
}

void GLContext::mpop()
{
    mpop(currentMatrix);
}


const Matrixf& GLContext::mtop(MatrixType mt)
{
    std::stack<Matrixf>& m = mstack[mt];
    if ( m.empty() )
        panic("GLContext","mtop()", "Trying to get the top of an empty matrix stack");
    return mstack[mt].top();
}

const Matrixf& GLContext::mtop()
{
    return mtop(currentMatrix);
}

void GLContext::ctop(const Matrixf& newTop, MatrixType mt)
{
    /* Synchronize GLState */
    switch ( mt )
    {
        case MODELVIEW:
            gls.setMatrix(glsNS::M_MODELVIEW, 0, newTop);
            break;
        case PROJECTION:
            gls.setMatrix(glsNS::M_PROJECTION, 0, newTop);
            break;
        case TEXTURE:
        {
            GLuint tu = TextureManager::instance().getCurrentGroup();
            gls.setMatrix(glsNS::M_TEXTURE, tu, newTop);

            //  Set library state as dirty.  When the Texture Matrix changes from identity to non identity
            //  the vertex shader must be updated with a matrix texture coordinate multiplication.
            libDirty = true;

            break;
        }
        default:
            panic("GLContext","mpush()","Invalid current matrix");
    }
    mstack[mt].top() = newTop; /* change current top */
}

void GLContext::ctop(const Matrixf& newTop)
{
    ctop(newTop,currentMatrix);
}


void GLContext::setPrimitive(GLenum p)
{
    if ( GL_POINTS > p || p > GL_POLYGON )
        panic("GLContext","setPrimitive()","Unknown primitive");
    primitive = p;
}

GLenum GLContext::getPrimitive()
{
    return primitive;
}


bool GLContext::areTexturesEnabled() const
{
    vector<TextureUnit>::const_iterator it = textureUnits.begin();
    for  ( it ; it != textureUnits.end(); it ++ )
    {
        if ( it->getTarget() != 0 )
            return true;
    }
    return false;
}


GLboolean GLContext::getShadeModel()
{
    return shadeModel;
}

void GLContext::setShadeModel(GLboolean isShade)
{
    shadeModel = isShade;
}

TextureUnit& GLContext::getTextureUnit(GLuint unit)
{
    if ( unit >= textureUnits.size() )
    {
        char msg[256];
        sprintf(msg, "Texture unit %d does not exist", unit);
        panic("GLCOntext", "getTextureUnit", msg);
    }
    return textureUnits[unit];
}



void GLContext::dump(std::ostream& os)
{
    if ( testFlags(flagBeginEnd) )
        os << "* Into glBegin/glEnd block" << endl;

    if ( testFlags(flagLighting) )
        os << "* Lighting enabled" << endl;

    if ( testFlags(flagLocalViewer) )
        os << "* Local Viewer selected" << endl;

    if ( testFlags(flagCullFace) )
        os << "* Cull face enabled: " << GLResolver::getConstantName(getCullFace()) << endl;

    if ( testFlags(flagAlphaTest) )
        os << "* Alpha test enabled. Alpha function: " << GLResolver::getConstantName(getAlphaFunc())
             << "   Alpha reference value: " << alphaRef << endl;

    if ( testFlags(flagBlending) )
    {
        GLenum srcRGB, dstRGB, srcAlpha, dstAlpha;
        getBlendFactor(srcRGB, dstRGB, srcAlpha, dstAlpha);

        os << "* Alpha blending enabled. Blending equation: " << GLResolver::getConstantName(getBlendEquation()) << endl
           << "   Blending source RGB factor: " << GLResolver::getConstantName(srcRGB) << endl
           << "   Blending destination RGB factor: " << GLResolver::getConstantName(dstRGB) << endl
           << "   Blending source Alpha factor: " << GLResolver::getConstantName(srcAlpha) << endl
           << "   Blending destination Alpha factor: " << GLResolver::getConstantName(dstAlpha) << endl;

    }

    GLclampd near_, far_;

    getDepthRange(near_, far_);

    os << "  Depth mask: " << ( depthMask ? "TRUE" : "FALSE" ) << endl;
    os << "  Depth clear value: " << getDepthClearValue() << endl;
    os << "  Depth range: [" << near_ << " n, " << far_ << " f]" << endl;

    if ( testFlags(flagDepthTest) )
        os << "* Depth test enabled: " << GLResolver::getConstantName(getDepthFunc()) << endl;

    if ( testFlags(flagStencilTest) )
    {
        GLenum sfail, zfail, zpass;

        getStencilOp(sfail, zfail, zpass);

        os << "* Stencil test enabled: " << GLResolver::getConstantName(getStencilBufferFunc())
           << "  Stencil reference value: " << getStencilBufferReferenceValue()
           << "  Stencil compare mask: " << getStencilBufferCompareMask() << endl
           << "  Stencil clear value: " << getStencilBufferClearValue() << endl
           << "  Stencil update mask: " << getStencilBufferUpdateMask() << endl
           << "  Stencil fail function: " << GLResolver::getConstantName(sfail) << endl
           << "  Stencil zfail function: " << GLResolver::getConstantName(zfail) << endl
           << "  Stencil zpass function: " << GLResolver::getConstantName(zpass) << endl;
    }
    if ( testFlags(flagPolygonOffsetFill) )
    {
        GLfloat slope, units;

        getPolygonOffset(slope, units);

        os << "* Polygon offset fill enabled. Slope factor: "
             << slope << "  Units: "
             << units << endl;
    }
    bool one = false;
    os << "* Current bound vertex shader -> ";
    ProgramObject* po;
    if ( testFlags(flagVS) )
    {
        po = & ProgramManager::instance().getTarget(GL_VERTEX_PROGRAM_ARB).getCurrent();
        os << po->getName() << endl;
        //po->printSource();
        //os << "------------" << endl;
    }
    else
        os << "Vertex program not enabled" << endl;

    os << "* Current bound fragment shader -> ";
    if ( testFlags(flagFS) )
    {
        po = & ProgramManager::instance().getTarget(GL_FRAGMENT_PROGRAM_ARB).getCurrent();
        os << po->getName() << endl;
        //po->printSource();
        //os << "------------" << endl;
    }
    else
        os << "Fragment program not enabled" << endl;

    os << "* Current Array buffer object bound: ";
    if ( IS_BUFFER_BOUND(GL_ARRAY_BUFFER) )
        os << GET_ARRAY_BUFFER.getName() << endl;
    else
        os << " NONE" << endl;

    os << "* Current Array Element buffer object bound: ";
    if ( IS_BUFFER_BOUND(GL_ELEMENT_ARRAY_BUFFER) )
        os << GET_ELEMENT_ARRAY_BUFFER.getName() << endl;
    else
        os << " NONE" << endl;

    os << "* Current texture objects bound:" << endl;
    os << "*    TEXTURE_1D: " << GET_TEXTURE_1D.getName() << endl;
    os << "*    TEXTURE_2D: " << GET_TEXTURE_2D.getName() << endl;
    os << "*    TEXTURE_3D: " << GET_TEXTURE_3D.getName() << endl;
    os << "*    TEXTURE_CM: " << GET_TEXTURE_CUBE_MAP.getName() << endl;




    os << "* Enabled texture units";
    for ( int i = 0; i < textureUnits.size(); i++ )
    {
        GLenum target = textureUnits[i].getTarget();
        if ( target != 0)
        {
            if ( !one )
                os << "\n";
            one = true;
            os << "*    -> Tex Unit: " << i;
            TextureObject* to = textureUnits[i].getTextureObject();
            if ( !to )
            {
                os << "   Texture Object NOT found" << endl;
                continue;
            }
            os << "   Name: " << to->getName();
            os << "   Type: ";
            switch ( target )
            {
                case GL_TEXTURE_1D:
                    os << "TEXTURE_1D";
                    break;
                case GL_TEXTURE_2D:
                    os << "TEXTURE_2D";
                    break;
                case GL_TEXTURE_3D:
                    os << "TEXTURE_3D";
                    break;
                case GL_TEXTURE_CUBE_MAP:
                    os << "CUBE_MAP";
                    break;
                default:
                    os << "UNKNOWN";
                    break;
            }
            const char* ifmt = GLResolver::getConstantName(to->getInternalFormat());
            if ( ifmt )
                os << "   ifmt: " << ifmt << endl;
            else
                os << "   ifmt: 0x" << hex << to->getInternalFormat() << dec << endl;
        }
    }

    if ( !one )
        os << " -> NONE" << endl;
}


void GLContext::setNewFlags(GLuint mask)
{
    flags = mask;
}


void GLContext::setFlags(GLuint orMask)
{
    flags |= orMask;
}

void GLContext::resetFlags(Flag mask)
{
    flags &= (~mask);
}


bool GLContext::testFlags(GLuint andMask) const
{
    return ((flags & andMask) == andMask) ;
}

void GLContext::testAndSetFlags(Flag f)
{
    if ((flags & f) != f)
    {
        flags |= f;
        libDirty = true;
    }
}

void GLContext::testAndResetFlags(Flag f)
{
    if ((flags & f) == f)
    {
        flags &= (~f);
        libDirty = true;
    }
}


GLuint GLContext::getFlags()
{
    return flags;
}

GLContext::Flag GLContext::makeMask(GLuint bit)
{
    return (1 << bit);
}


GLContext::DBUF& GLContext::posbuf()
{
    return posBuffer;
}

GLContext::DBUF& GLContext::colbuf()
{
    return colBuffer;
}

GLContext::DBUF& GLContext::texbuf(GLuint unit)
{
    if ( unit >= textureUnits.size() )
        panic("GLContext", "texbuf", "Texture unit not available");
    return texBuffer[unit];
}

GLContext::DBUF& GLContext::norbuf()
{
    return norBuffer;
}


void GLContext::clearBuffers()
{
    posbuf().clear();
    colbuf().clear();

    for ( u32bit i = 0; i < countTextureUnits(); i++ )
        texbuf(i).clear();

    norbuf().clear();
}


/*  Sets the viewport coordinates and dimensions.  */
void GLContext::setViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    viewportX = x;
    viewportY = y;
    viewportWidth = width;
    viewportHeight = height;
}

/*  Sets the polygon face mode.  */
void GLContext::setFaceMode(GLenum mode)
{
    faceMode = mode;
}

/*  Sets if the triangle setup shader program is updated.  */
void GLContext::setSetupShaderProgramUpdate(bool updated)
{
    setupShProgramUpdated = updated;
}

/*  Sets if the triangle setup shader constants are updated.  */
void GLContext::setSetupShaderConstantsUpdate(bool updated)
{
    setupShConstantsUpdated = updated;
}

/*  Gets the viewport coordinates and dimensions.  */
void GLContext::getViewport(GLint &x, GLint &y, GLsizei &width, GLsizei &height) const
{
    x = viewportX;
    y = viewportY;
    width = viewportWidth;
    height = viewportHeight;
}

/*  Gets the polygon face mode.  */
GLenum GLContext::getFaceMode() const
{
    return faceMode;
}

/*  Returns if the triangle setup shader program is updated.  */
bool GLContext::isSetupShaderProgramUpdated() const
{
    if ( !_triangleSetupOnShader )
        return true;
    return setupShProgramUpdated;
}

/*  Returns if the triangle setup shader constants are updated.  */
bool GLContext::areSetupShaderConstantsUpdated() const
{
    if ( !_triangleSetupOnShader )
        return true;
    return setupShConstantsUpdated;
}

void GLContext::setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    Quadf c(r,g,b,a);
    currentColor = c;

    /*******************************************
     * Update default color value GPU register *
     *******************************************/
    gpu3d::GPURegData data;
    data.qfVal[0] = r;
    data.qfVal[1] = g;
    data.qfVal[2] = b;
    data.qfVal[3] = a;
// cout << "GLContext::setColor() -> ignoring GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE to debug purposes" << endl;
    driver->writeGPURegister(gpu3d::GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE, (u32bit)GPUDriver::VS_COLOR, data);
}

const glsNS::GLState& GLContext::getGLState() const
{
    return gls;
}

void GLContext::setColor(Quadf& rgba)
{
    currentColor = rgba;
}

const Quadf& GLContext::getColor() const
{
    return currentColor;
}


void GLContext::setNormal(GLfloat r, GLfloat s, GLfloat t)
{
    currentNormal[0] = r;
    currentNormal[1] = s;
    currentNormal[2] = t;
}

void GLContext::setNormal(Quadf& rst)
{
    currentNormal = rst;
    currentNormal[3] = 0.0f;
}


const Quadf& GLContext::getNormal() const
{
    return currentNormal;
}

void GLContext::setTexCoord(GLuint unit, GLfloat r, GLfloat s, GLfloat t, GLfloat q)
{
    if ( unit >= textureUnits.size() )
        panic("GLContext", "setTexCoord", "Unit not available");
    currentTexCoord[unit][0] = r;
    currentTexCoord[unit][1] = s;
    currentTexCoord[unit][2] = t;
    currentTexCoord[unit][3] = q; // by default 1
}

const Quadf& GLContext::getTexCoord(GLuint unit) const
{
    if ( unit >= textureUnits.size() )
        panic("GLContext", "getTexCoord", "Texture unit is not available");
    return currentTexCoord[unit];
}


GLContext::VArray& GLContext::posVArray()
{
    return posVa;
}

GLContext::VArray& GLContext::colVArray()
{
    return colVa;
}

GLContext::VArray& GLContext::indVArray()
{
    return indVa;
}

GLContext::VArray& GLContext::norVArray()
{
    return norVa;
}

GLContext::VArray& GLContext::texVArray(GLuint unit)
{
    if ( unit >= textureUnits.size() )
        panic("GLContext", "texVArray", "Texture unit does not exist");
    return texVa[unit];
}

GLContext::VArray& GLContext::edgVArray()
{
    return edgVa;
}

GLContext::VArray& GLContext::indexesVArray()
{
    return indexesVa;
}

GLContext::VArray& GLContext::genericVArray(GLuint attrib)
{
    if ( attrib >= genericVa.size() )
        panic("GLContext", "genericVArray()", "Generic Vertex Array does not exist");
    return genericVa[attrib];
}

void GLContext::setEnabledGenericAttribArray(GLuint index, bool enabled)
{
    if ( index >= genericAttribArrayFlags.size() )
        panic("GLContext", "setEnabledGenericAttribArray", "Generic Vertex Array does not exist");

    genericAttribArrayFlags[index] = enabled;
}

bool GLContext::isGenericAttribArrayEnabled(GLuint index) const
{
    if ( index >= genericAttribArrayFlags.size() )
        panic("GLContext", "isGenericAttribArrayEnabled", "Generic Vertex Array does not exist");

    return genericAttribArrayFlags[index];
}

GLuint GLContext::countGenericVArrays() const
{
    return genericAttribArrayFlags.size();
}


void GLContext::enableLight(GLuint light)
{
    if ( !lights[light] )
    {
        lights[light] = true;
        libDirty = true;
    }
}

void GLContext::disableLight(GLuint light)
{
    if ( lights[light] )
    {
        lights[light] = false;
        libDirty = true;
    }
}


bool GLContext::isLightEnabled(GLuint light) const
{
    return lights[light];
}


void GLContext::setParamLight(GLenum light_, GLenum pname, const float* params)
{
    using namespace glsNS;

    GLuint light = light_ - GL_LIGHT0;

    QuadReg<float> v;
    switch ( pname )
    {
        case GL_AMBIENT:
        case GL_DIFFUSE:
        case GL_SPECULAR:
        case GL_POSITION:

            v[0] = params[0];
            v[1] = params[1];
            v[2] = params[2];
            v[3] = params[3];

            if ( pname == GL_AMBIENT )
                gls.setVectorGroup(v, V_LIGHT_AMBIENT, light);
            else if ( pname == GL_DIFFUSE )
                gls.setVectorGroup(v, V_LIGHT_DIFFUSE, light);
            else if ( pname == GL_SPECULAR )
                gls.setVectorGroup(v, V_LIGHT_SPECULAR, light);
            else
            {
                /* OpenGL requires multiply the position by the current Modelview matrix */
                GLfloat pos[4] = {params[0], params[1], params[2], params[3]};
                mathlib::_mult_mat_vect(mtop(MODELVIEW).transpose().getRows(), pos);
                v[0] = pos[0];
                v[1] = pos[1];
                v[2] = pos[2];
                v[3] = pos[3];
                gls.setVectorGroup(v, V_LIGHT_POSITION, light);
            }

            break;

        case GL_SPOT_DIRECTION:
            {
                /* OpenGL requires multiply the position by the current Modelview matrix */
                GLfloat pos[4] = {params[0], params[1], params[2], 1.0f};
                mathlib::_mult_mat_vect(mtop(MODELVIEW).transpose().getRows(), pos);
                v[0] = pos[0];
                v[1] = pos[1];
                v[2] = pos[2];
                v[3] = pos[3];
            }

            gls.setVectorGroup(v, V_LIGHT_SPOT_DIRECTION, light, 0x07); // mask = 0111
            break;

        case GL_SPOT_CUTOFF:
            v[3] = mathlib::_cos(params[0]);
            gls.setVectorGroup(v, V_LIGHT_SPOT_DIRECTION, light, 0x08);
            break;

        case GL_CONSTANT_ATTENUATION:
            v[0] = params[0];
            gls.setVectorGroup(v, V_LIGHT_ATTENUATION, light, 0x01);
            break;

        case GL_LINEAR_ATTENUATION:
            v[1] = params[0];
            gls.setVectorGroup(v, V_LIGHT_ATTENUATION, light, 0x02);
            break;

        case GL_QUADRATIC_ATTENUATION:
            v[2] = params[0];
            gls.setVectorGroup(v, V_LIGHT_ATTENUATION, light, 0x04);
            break;

        case GL_SPOT_EXPONENT:
            v[3] = params[0];
            gls.setVectorGroup(v, V_LIGHT_ATTENUATION, light, 0x08);
            break;

        default:
            panic("GLContext","setParamLight()", "Pname value not supported");
    }
}


void GLContext::setMaterial(GLenum face, GLenum pname, const float* params)
{
    using namespace glsNS;

    if ( face != GL_FRONT && face != GL_BACK && face != GL_FRONT_AND_BACK )
        panic("GLContext","setMaterial()", "GL_FRONT, GL_BACK or GL_FRONT_AND_BACK required to select a 'face'");

    bool front = ((face == GL_FRONT) || (face == GL_FRONT_AND_BACK));
    bool back = ((face == GL_BACK) || (face == GL_FRONT_AND_BACK));

    switch ( pname ) /* CHECK ERRORS */
    {
        case GL_AMBIENT:
        case GL_DIFFUSE:
        case GL_AMBIENT_AND_DIFFUSE:
        case GL_SPECULAR:
        case GL_SHININESS:
        case GL_EMISSION:
            break; // OK
        case GL_COLOR_INDEXES:
            panic("GLContext","setMaterial()","GL_COLOR_INDEXES not supported");
        default:
            panic("GLContext","setMaterial()", "Unknown parameter name");
    }

    QuadReg<float> v;
    if ( pname == GL_SHININESS )
        v = QuadReg<float>(params[0], 0.0f, 0.0f, 0.0f);
    else
        v = QuadReg<float>(params[0], params[1], params[2], params[3]);

    if ( front )
    {
        switch ( pname )
        {
            case GL_AMBIENT:
                gls.setVector(v, V_MATERIAL_FRONT_AMBIENT);
                break;
            case GL_DIFFUSE:
                gls.setVector(v, V_MATERIAL_FRONT_DIFUSSE);
                break;
            case GL_AMBIENT_AND_DIFFUSE:
                gls.setVector(v, V_MATERIAL_FRONT_AMBIENT);
                gls.setVector(v, V_MATERIAL_FRONT_DIFUSSE);
                break;
            case GL_SPECULAR:
                gls.setVector(v, V_MATERIAL_FRONT_SPECULAR);
                break;
            case GL_SHININESS:
                gls.setVector(v, V_MATERIAL_FRONT_SHININESS);
                break;
            case GL_EMISSION:
                gls.setVector(v, V_MATERIAL_FRONT_EMISSION);
                break;
        }
    }
    if ( back )// GL_BACK
    {
        switch ( pname )
        {
            case GL_AMBIENT:
                gls.setVector(v, V_MATERIAL_BACK_AMBIENT);
                break;
            case GL_DIFFUSE:
                gls.setVector(v, V_MATERIAL_BACK_DIFUSSE);
                break;
            case GL_AMBIENT_AND_DIFFUSE:
                gls.setVector(v, V_MATERIAL_BACK_AMBIENT);
                gls.setVector(v, V_MATERIAL_BACK_DIFUSSE);
                break;
            case GL_SPECULAR:
                gls.setVector(v, V_MATERIAL_BACK_SPECULAR);
                break;
            case GL_SHININESS:
                gls.setVector(v, V_MATERIAL_BACK_SHININESS);
                break;
            case GL_EMISSION:
                gls.setVector(v, V_MATERIAL_BACK_EMISSION);
                break;
        }
    }
}

void GLContext::setLightModel(GLenum pname, const float* params)
{
    using namespace glsNS;

    if ( pname == GL_LIGHT_MODEL_AMBIENT )
    {
        QuadReg<float> v(params[0], params[1], params[2], params[3]);
        gls.setVector(v, V_LIGHTMODEL_AMBIENT);
    }
    else if ( pname == GL_LIGHT_MODEL_LOCAL_VIEWER )
    {
        if ( params[0] != 0.0f )
            testAndSetFlags(flagLocalViewer);
        else
            testAndResetFlags(flagLocalViewer);
        // local viewer
    }
    else if ( pname == GL_LIGHT_MODEL_TWO_SIDE )
    {
        gpu3d::GPURegData data;

        if ( params[0] != 0.0f )
        {
            testAndSetFlags(flagTwoSidedLighting);
            data.booleanVal = true;
        }
        else
        {
            testAndResetFlags(flagTwoSidedLighting);
            data.booleanVal = false;
        }

        driver->writeGPURegister(gpu3d::GPU_TWOSIDED_LIGHTING, data);
    }
    else if ( pname == GL_LIGHT_MODEL_COLOR_CONTROL )
        panic("GLContext","setLightModel()", "GL_LIGHT_MODEL_COLOR_CONTROL not supported here");
    else
        panic("GLContext","setLightModel()", "Unsupported property value");
}

void GLContext::setLightModel(GLenum pname, const GLint* params)
{
    using namespace glsNS;

    if ( pname == GL_LIGHT_MODEL_AMBIENT )
    {
        QuadReg<float> v;
        v[0] = GLfloat(params[0]);
        v[1] = GLfloat(params[1]);
        v[2] = GLfloat(params[2]);
        v[3] = GLfloat(params[3]);
        gls.setVector(v, V_LIGHTMODEL_AMBIENT);
    }
    else if ( pname == GL_LIGHT_MODEL_LOCAL_VIEWER )
    {
        if ( params[0] != GL_FALSE )
            testAndSetFlags(flagLocalViewer);
        else
            testAndResetFlags(flagLocalViewer);
        // local viewer
    }
    else if ( pname == GL_LIGHT_MODEL_TWO_SIDE )
    {
        gpu3d::GPURegData data;

        if ( params[0] != 0.0f )
        {
            testAndSetFlags(flagTwoSidedLighting);
            data.booleanVal = true;
        }
        else
        {
            testAndResetFlags(flagTwoSidedLighting);
            data.booleanVal = false;
        }
        driver->writeGPURegister(gpu3d::GPU_TWOSIDED_LIGHTING, data);
    }
    else if ( pname == GL_LIGHT_MODEL_COLOR_CONTROL )
    {
        if ( params[0] == GL_SEPARATE_SPECULAR_COLOR )
            testAndSetFlags(flagSeparateSpecular);
        else if ( params[0] == GL_SINGLE_COLOR )
            testAndResetFlags(flagSeparateSpecular);
        else
            panic("GLContext", "setLightModel()", "GL_LIGHT_MODEL_COLOR_CONTROL unsupported value");
    }
    else
        panic("GLContext","setLightModel()", "Unsupported property value");
}


void GLContext::setTexGen(GLenum coord, GLenum pname, GLint param)
{
    switch ( pname )
    {
        case GL_TEXTURE_GEN_MODE:
            {
                GLuint tu = TextureManager::instance().getCurrentGroup();
                getTextureUnit(tu).setTexGenMode(coord, param);
            }
            break;
        default:
            panic("GLContext", "setTexGen", "Unsuported parameter name");
    }
}

void GLContext::setTexGen(GLenum coord, GLenum pname, const GLfloat* params)
{
    using namespace glsNS;
    QuadReg<float> v;

    switch ( pname )
    {
        case GL_OBJECT_PLANE:
            {
                v[0] = params[0];
                v[1] = params[1];
                v[2] = params[2];
                v[3] = params[3];
                GLuint tu = TextureManager::instance().getCurrentGroup();
                // update GLState
                if ( coord == GL_S )
                    gls.setVectorGroup(v, V_TEXGEN_OBJECT_S, tu);
                else if ( coord == GL_T )
                    gls.setVectorGroup(v, V_TEXGEN_OBJECT_T, tu);
                else if ( coord == GL_R )
                    gls.setVectorGroup(v, V_TEXGEN_OBJECT_R, tu);
                else if ( coord == GL_Q )
                    gls.setVectorGroup(v, V_TEXGEN_OBJECT_Q, tu);
                else
                    panic("GLContext", "setTexGen", "Unexpected object plane coordinate");
            }
            break;
        case GL_EYE_PLANE:
            {
                /* OpenGL requires multiply the plane by the current Modelview matrix.
                 * The exact operation to do is (p1, p2, p3, p4) * inverse(Modelview).
                 */
                GLfloat pos[16] = {
                                    params[0], 0.0f, 0.0f, 0.0f,
                                    params[1], 0.0f, 0.0f, 0.0f,
                                    params[2], 0.0f, 0.0f, 0.0f,
                                    params[3], 0.0f, 0.0f, 0.0f,
                                  };

                mathlib::_mult_matrixf(pos, pos, mtop(MODELVIEW).invtrans().getRows());

                v[0] = pos[0];
                v[1] = pos[4];
                v[2] = pos[8];
                v[3] = pos[12];

                GLuint tu = TextureManager::instance().getCurrentGroup();
                // update GLState
                if ( coord == GL_S )
                    gls.setVectorGroup(v, V_TEXGEN_EYE_S, tu);
                else if ( coord == GL_T )
                    gls.setVectorGroup(v, V_TEXGEN_EYE_T, tu);
                else if ( coord == GL_R )
                    gls.setVectorGroup(v, V_TEXGEN_EYE_R, tu);
                else if ( coord == GL_Q )
                    gls.setVectorGroup(v, V_TEXGEN_EYE_Q, tu);
                else
                    panic("GLContext", "setTexGen", "Unexpected eye plane coordinate");
            }
            break;
        default:
            panic("GLContext", "setTexGen", "Unsuported parameter name");
    }
}

void GLContext::setCurrentClientTextureUnit(GLenum tex)
{
    if ( tex >= clientTextureUnits.size() )
        panic("GLContext", "setCurrentClientTextureUnit", "texture unit does not exist");

    currentClientTextureUnit = tex;
}

bool GLContext::isEnabledClientTextureUnit(GLuint tex)
{
    if ( tex >= clientTextureUnits.size() )
        panic("GLContext", "isEnabledClientTextureUnit", "Texture unit does not exist");
    return clientTextureUnits[tex];
}

GLenum GLContext::getCurrentClientTextureUnit() const
{
    return currentClientTextureUnit;
}

bool GLContext::isEnabledCurrentClientTextureUnit() const
{
    return clientTextureUnits[currentClientTextureUnit];
}

void GLContext::setEnabledCurrentClientTextureUnit(bool enabled)
{
    clientTextureUnits[currentClientTextureUnit] = enabled;
}

void GLContext::setTexEnvColor(GLuint unit, GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    using namespace glsNS;
    QuadReg<float> v;
    v[0] = r;
    v[1] = g;
    v[2] = b;
    v[3] = a;
    gls.setVectorGroup(v, V_TEXENV_COLOR, unit);
}


void GLContext::setFog(GLenum pname, const GLfloat* params)
{
    using namespace glsNS;
    if ( pname == GL_FOG_COLOR )
    {
        QuadReg<float> v;
        v[0] = params[0];
        v[1] = params[1];
        v[2] = params[2];
        v[3] = params[3];
        gls.setVector(v, V_FOG_COLOR);
    }
    else if ( pname == GL_FOG_DENSITY )
    {
        QuadReg<float> v;
        v[0] = params[0];
        gls.setVector(v, V_FOG_PARAMS, 0x01); // Write only first component
    }
    else if ( pname == GL_FOG_START )
    {
        QuadReg<float> v;
        v[1] = params[0];
        gls.setVector(v, V_FOG_PARAMS, 0x02); // write only second component
    }
    else if ( pname == GL_FOG_END )
    {
        QuadReg<float> v;
        v[2] = params[0];
        gls.setVector(v, V_FOG_PARAMS, 0x04); // write only third component
        // GLState code
    }
    else
    {
        GLint value = GLint(params[0]);
        setFog(pname, &value); // Call integer version
    }
}

void GLContext::setFog(GLenum pname, const GLint* params)
{
    using namespace glsNS;
    GLint value = params[0];
    if ( pname == GL_FOG_MODE )
    {
        if ( value != GL_EXP && value != GL_EXP2 && value != GL_LINEAR )
            panic("GLContext", "setFog", "Unexpected fog mode");
        fogMode = value;
    }
    else if ( pname == GL_FOG_COORDINATE_SOURCE )
    {
        if ( value != GL_FRAGMENT_DEPTH && value != GL_FOG_COORD )
            panic("GLContext", "setFog", "unexpected GL_FOG_COORD_SOURCE value");
        fogCoordinateSource = value;
    }
    else if ( pname == GL_FOG_COLOR )
    {
        QuadReg<float> v;
        v[0] = params[0];
        v[1] = params[1];
        v[2] = params[2];
        v[3] = params[3];
        gls.setVector(v, V_FOG_COLOR);
    }
    else if ( pname == GL_FOG_DENSITY )
    {
        QuadReg<float> v;
        v[0] = static_cast<GLfloat>(params[0]);
        gls.setVector(v, V_FOG_PARAMS, 0x01); // Write only first component
    }
    else if ( pname == GL_FOG_START )
    {
        QuadReg<float> v;
        v[1] = static_cast<GLfloat>(params[0]);
        gls.setVector(v, V_FOG_PARAMS, 0x02); // write only second component
    }
    else if ( pname == GL_FOG_END )
    {
        QuadReg<float> v;
        v[2] = static_cast<GLfloat>(params[0]);
        gls.setVector(v, V_FOG_PARAMS, 0x04); // write only third component
        // GLState code
    }
    else
        panic("GLContext", "setFog", "Unexpected FOG parameter");
}


void GLContext::setAlphaTest(GLenum func, GLclampf ref)
{
    switch ( func )
    {
        case GL_NEVER:
        case GL_ALWAYS:
        case GL_LESS:
        case GL_LEQUAL:
        case GL_EQUAL:
        case GL_GEQUAL:
        case GL_GREATER:
        case GL_NOTEQUAL:
            if ( alphaFunc != func ) // Alpha func has changed, it is required to recompile shaders (libDirty = true)
            {
                alphaFunc = func;
                libDirty = true;
            }
            alphaRef = afl::clamp(ref);
            break;
        default:
            panic("GLContext", "setAlphaTest", "Unexpected alpha function");
    }
}

GLenum GLContext::getAlphaFunc() const
{
    return alphaFunc;
}

GLclampf GLContext::getAlphaTestReferenceValue() const
{
    return alphaRef;
}

void GLContext::setColorMaterial(GLenum face, GLenum mode)
{
    switch ( face )
    {
        case GL_FRONT:
        case GL_BACK:
        case GL_FRONT_AND_BACK:
            if ( colorMaterialFace != face )
            {
                colorMaterialFace = face;
                libDirty = true;
            }
            break;
        default:
            panic("GLContext", "setColorMaterial", "Unexpected color material face");
    }

    switch ( mode )
    {
        case GL_EMISSION:
        case GL_AMBIENT:
        case GL_DIFFUSE:
        case GL_SPECULAR:
        case GL_AMBIENT_AND_DIFFUSE:
            if ( colorMaterialMode != mode )
            {
                colorMaterialMode = mode;
                libDirty = true;
            }
            break;
        default:
            panic("GLContext","setColorMaterial","Unexpected color material mode");
    }
}

void GLContext::getColorMaterial(GLenum& face, GLenum& mode) const
{
    face = colorMaterialFace;
    mode = colorMaterialMode;
}

void GLContext::setCullFace(GLenum face)
{
    switch ( face )
    {
        case GL_FRONT:
        case GL_BACK:
        case GL_FRONT_AND_BACK:
            if ( cullFace != face )
            {
                cullFace = face;
                libDirty = true;
            }
            break;
        default:
            panic("GLContext", "setCullFace", "Unexpected culling face");
    }
}

/*  Sets the depth function mode.  */
void GLContext::setDepthFunc(GLenum func)
{
    depthFunc = func;
}

/*  Return the depth function mode.  */
GLenum GLContext::getDepthFunc()
{
    return depthFunc;
}

void GLContext::setDepthMask(bool flag)
{
    depthMask = flag;
}

bool GLContext::getDepthMask() const
{
    return depthMask;
}

void GLContext::setDepthClearValue(GLclampd depth)
{
    depthClear = depth;
}

void GLContext::setColorMask(GLboolean _red, GLboolean _green, GLboolean _blue, GLboolean _alpha)
{
    red = _red;
    green = _green;
    blue = _blue;
    alpha = _alpha;
}

void GLContext::getColorMask(GLboolean& _red, GLboolean& _green, GLboolean& _blue, GLboolean& _alpha) const
{
    _red = red;
    _green = green;
    _blue = blue;
    _alpha = alpha;
}

void GLContext::setDepthRange( GLclampd near_val, GLclampd far_val )
{
    near_ = near_val;
    far_ = far_val;
}

void GLContext::getDepthRange( GLclampd& near_val, GLclampd& far_val ) const
{
    near_val = near_;
    far_val = far_;
}

/*  Sets the stencil update functions.  */
void GLContext::setStencilOp(GLenum stFail, GLenum dpFail, GLenum dpPass)
{
    stFailUpdateFunc = stFail;
    dpFailUpdateFunc = dpFail;
    dpPassUpdateFunc = dpPass;
}

/*  Gets the stencil update functions.  */
void GLContext::getStencilOp(GLenum &stFail, GLenum &dpFail, GLenum &dpPass)
{
    stFail = stFailUpdateFunc;
    dpFail = dpFailUpdateFunc;
    dpPass = dpPassUpdateFunc;
}

/*  Sets the HZ buffer valid flag.  */
void GLContext::setHZBufferValidFlag(bool valid)
{
    hzBufferValid = valid;
}

/*  Returns if the HZ buffer is valid.  */
bool GLContext::isHZBufferValid()
{
    return hzBufferValid;
}

/*  Sets the HZ test active flag.  */
void GLContext::setHZTestActiveFlag(bool active)
{
    hzTestActive = active;
}

/*  Returns if the HZ test is activated.  */
bool GLContext::isHZTestActive()
{
    return hzTestActive;
}

void GLContext::setStencilBufferClearValue(GLint value)
{
    stBufferClearValue = value;
}

void GLContext::setStencilBufferUpdateMask(GLuint mask)
{
    stBufferUpdateMask = mask;
}

void GLContext::setStencilBufferFunc(GLenum func, GLint ref, GLuint mask)
{
    stBufferFunc = func;
    stBufferRefValue = ref;
    stBufferCompMask = mask;
}

GLenum GLContext::getCullFace()
{
    return cullFace;
}

void GLContext::setPolygonOffset( GLfloat factor, GLfloat units )
{
    slopeFactor = factor;
    unitsFactor = units;
}

void GLContext::getPolygonOffset( GLfloat& factor, GLfloat& units ) const
{
    factor = slopeFactor;
    units = unitsFactor;
}

void GLContext::setScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    scissorIniX = x;
    scissorIniY = y;
    scissorWidth = width;
    scissorHeight = height;
}

void GLContext::getScissor(GLint& x, GLint& y, GLsizei& width, GLsizei& height) const
{
    x = scissorIniX;
    y = scissorIniY;
    width = scissorWidth;
    height = scissorHeight;
}

void GLContext::setBlendFactor(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
    blendSFactorRGB = srcRGB;
    blendDFactorRGB = dstRGB;
    blendSFactorAlpha = srcAlpha;
    blendDFactorAlpha = dstAlpha;
}

void GLContext::getBlendFactor(GLenum& srcRGB, GLenum& dstRGB, GLenum& srcAlpha, GLenum& dstAlpha) const
{
    srcRGB = blendSFactorRGB;
    dstRGB = blendDFactorRGB;
    srcAlpha = blendSFactorAlpha;
    dstAlpha = blendDFactorAlpha;
}

void GLContext::setBlendEquation(GLenum mode)
{
    blendEquation = mode;
}

GLenum GLContext::getBlendEquation() const
{
    return blendEquation;
}

void GLContext::setBlendColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    Quadf c(r,g,b,a);
    blendColor = c;
}

void GLContext::getBlendColor(GLfloat& r, GLfloat& g, GLfloat& b, GLfloat& a) const
{
    r = blendColor[0];
    g = blendColor[1];
    b = blendColor[2];
    a = blendColor[3];
}

void GLContext::setClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    Quadf c(r,g,b,a);
    clearColor = c;
}

void GLContext::getClearColor(GLfloat& r, GLfloat& g, GLfloat& b, GLfloat& a) const
{
    r = clearColor[0];
    g = clearColor[1];
    b = clearColor[2];
    a = clearColor[3];
}

bool GLContext::anyBufferBound() const
{
    if ( testFlags(flagVaPos) )
    {
        if ( posVa.bufferID != 0 )
            return true;
    }
    if ( testFlags(flagVaCol) )
    {
        if ( colVa.bufferID != 0 )
            return true;
    }
    if ( testFlags(flagVaNor) )
    {
        if ( norVa.bufferID != 0 )
            return true;
    }

    if ( testFlags(flagVaInd) )
    {
        if ( indVa.bufferID != 0 )
            return true;
    }

    if ( testFlags(flagVaEdg) )
    {
        if ( edgVa.bufferID != 0 )
            return true;
    }

    for ( GLuint i = 0; i < clientTextureUnits.size(); i++ )
    {
        if ( clientTextureUnits[i] )
        {
            if ( texVa[i].bufferID != 0 )
                return true;
        }
    }

    return false;
}


bool GLContext::allBuffersBound() const
{
    if ( testFlags(flagVaPos) )
    {
        if ( posVa.bufferID == 0 )
            return false;
    }
    if ( testFlags(flagVaCol) )
    {
        if ( colVa.bufferID == 0 )
            return false;
    }
    if ( testFlags(flagVaNor) )
    {
        if ( norVa.bufferID == 0 )
            return false;
    }

    if ( testFlags(flagVaInd) )
    {
        if ( indVa.bufferID == 0 )
            return false;
    }

    if ( testFlags(flagVaEdg) )
    {
        if ( edgVa.bufferID == 0 )
            return false;
    }

    for ( GLuint i = 0; i < clientTextureUnits.size(); i++ )
    {
        if ( clientTextureUnits[i] )
        {
            if ( texVa[i].bufferID == 0 )
                return false;
        }
    }

    return true;

}

bool GLContext::allBuffersUnbound() const
{
    if ( testFlags(flagVaPos) )
    {
        if ( posVa.bufferID != 0 )
            return false;
    }
    if ( testFlags(flagVaCol) )
    {
        if ( colVa.bufferID != 0 )
            return false;
    }
    if ( testFlags(flagVaNor) )
    {
        if ( norVa.bufferID != 0 )
            return false;
    }

    if ( testFlags(flagVaInd) )
    {
        if ( indVa.bufferID != 0 )
            return false;
    }

    if ( testFlags(flagVaEdg) )
    {
        if ( edgVa.bufferID != 0 )
            return false;
    }

    for ( GLuint i = 0; i < clientTextureUnits.size(); i++ )
    {
        if ( clientTextureUnits[i] )
        {
            if ( texVa[i].bufferID != 0 )
                return false;
        }
    }

    return true;

}


void GLContext::setPreloadMemory(bool enable)
{
    driver->setPreloadMemory(enable);
}


void GLContext::preloadMemory(bool isDrawElements)
{
    driver->setPreloadMemory(true); // Enable preloading

    /*******************************
     * Synchronize textures images *
     *******************************/

    if ( countTextureUnits() != ProgramObject::MaxTextureUnits )
    {
        cout << "CountTextureUnits: " << ctx->countTextureUnits() << endl;
        cout << "ProgramObject::MaxTextureUnits: " << ProgramObject::MaxTextureUnits << endl;
        stringstream ss;
        ss << "Inconsistent definitions for MaxTextureUnits. ctx->countTextureUnits() = "
           << ctx->countTextureUnits() << "  ProgramObject::MaxTextureUnits: "
           << ProgramObject::MaxTextureUnits;
        panic("GLContext", "preloadMemory", ss.str().c_str());
    }

    if ( testFlags(flagFS) )
    {
        ProgramObject& fp = GET_FRAGMENT_PROGRAM;
        fp.compile();
        ProgramObject::ProgramResources& pr = fp.getResources();
        for ( GLuint i = 0; i < ProgramObject::MaxTextureUnits; i++ )
        {
            if ( pr.textureTargets[i] != 0 )
            {
                TextureUnit& tu = ctx->getTextureUnit(i);
                TextureObject* to = tu.getTextureObject(pr.textureTargets[i]);
                if ( !to )
                {
                    stringstream ss;
                    ss << "Texture object (id: " << to->getName() << ") not attached to any texture unit";
                    panic("GLContext", "preloadMemory", ss.str().c_str());
                }
                to->sync();
                gpumem().allocate(*to);
            }
        }
    }
    else if ( areTexturesEnabled() )
    {
        GLuint count = ctx->countTextureUnits();
        for ( GLuint i = 0; i < count; i++ )
        {
            TextureUnit& tu = ctx->getTextureUnit(i);
            TextureObject* to = tu.getTextureObject();
            if ( to != 0 )
            {
                to->sync();
                ctx->gpumem().allocate(*to);
            }
        }
    }

    /******************************
     * Synchronize static buffers *
     ******************************/
    if ( isDrawElements && IS_BUFFER_BOUND(GL_ELEMENT_ARRAY_BUFFER) )
        gpumem().allocate(GET_ELEMENT_ARRAY_BUFFER);

    if ( testFlags(flagVaPos) && posVa.bufferID != 0 )
        gpumem().allocate(GET_BUFFER_BY_NAME(GL_ARRAY_BUFFER, posVa.bufferID));

    if ( testFlags(flagVaCol) && colVa.bufferID != 0 )
        gpumem().allocate(GET_BUFFER_BY_NAME(GL_ARRAY_BUFFER, colVa.bufferID));

    if ( testFlags(flagVaNor) && norVa.bufferID != 0 )
        gpumem().allocate(GET_BUFFER_BY_NAME(GL_ARRAY_BUFFER, norVa.bufferID));

    GLuint count = countTextureUnits();
    for ( GLuint i = 0; i < count; i++ )
    {
        if ( isEnabledClientTextureUnit(i) && texVArray(i).bufferID != 0 )
            gpumem().allocate(GET_BUFFER_BY_NAME(GL_ARRAY_BUFFER, texVArray(i).bufferID));
    }

    if ( testFlags(flagVS) )
    {
        count = countGenericVArrays();
        for ( GLuint i = 0; i < count; i++ )
        {
            if ( isGenericAttribArrayEnabled(i) && genericVArray(i).bufferID != 0 )
                gpumem().allocate(GET_BUFFER_BY_NAME(GL_ARRAY_BUFFER, genericVArray(i).bufferID));
        }
    }

    driver->setPreloadMemory(false); // Disable preloading

}

ProgramObject& GLContext::getCurrentVertexProgram()
{
    if ( testFlags(flagVS) )
        return GET_VERTEX_PROGRAM;

    // else, the program is an automatically generated program
    ProgramObject* po = vpCache.getLastUsed();
    if ( po == 0 )
        panic("GLContext", "getCurrentVertexProgram", "Automatically generated vertex program not found in cache!");
    return *po;
}


ProgramObject& GLContext::getCurrentFragmentProgram()
{
    if ( testFlags(flagFS) )
        return GET_FRAGMENT_PROGRAM;

    // else, the program is an automatically generated program
    ProgramObject* po = fpCache.getLastUsed();
    if ( po == 0 )
        panic("GLContext", "getCurrentFragmentProgram", "Automatically generated fragment program not found in cache!");
    return *po;
}

void GLContext::pushAttrib(GLbitfield mask)
{
    pushstate::TstateStack& s = stateStack;
    GLContext& ctx = *this;
    TextureManager& tm = TextureManager::instance();

    pushstate::pushAttrib(s, (const GLContext&)ctx, (const TextureManager&)tm, mask);
}

void GLContext::popAttrib()
{
    pushstate::TstateStack& s = stateStack;
    GLContext& ctx = *this;
    TextureManager& tm = TextureManager::instance();

    pushstate::popAttrib(s, ctx, tm);
}


/////////////////////////////////////////////////////////////////////////
///////// Implementation of inner class TLStateImplementation ///////////
/////////////////////////////////////////////////////////////////////////

#define TLSImp GLContext::TLStateImplementation

TLSImp::TLStateImplementation(GLContext* parent) : ctx(parent)
{}


bool TLSImp::normalizeNormals()
{
    return ctx->testFlags(GLContext::flagNormalize);
}

bool TLSImp::isLighting()
{
    return ctx->testFlags(GLContext::flagLighting);
}

bool TLSImp::isRescaling()
{
    return ctx->testFlags(GLContext::flagRescaleNormal);
}


bool TLSImp::infiniteViewer()
{
    return !ctx->testFlags(GLContext::flagLocalViewer);
}

bool TLSImp::localLightSource(Light light)
{
    using namespace glsNS;

    const GLState& gls = ctx->getGLState();
    QuadReg<float> pos = gls.getVector(V_LIGHT_POSITION + 7*light);
    return (pos[3] != 0);
}

void TLSImp::attenuationCoeficients(Light light, float& constantAtt, float& linearAtt, float& quadraticAtt)
{
    using namespace glsNS;

    const GLState& gls = ctx->getGLState();
    QuadReg<float> att = gls.getVector(V_LIGHT_ATTENUATION + 7*light);

    constantAtt = att[0];
    linearAtt = att[1];
    quadraticAtt = att[2];
}

bool TLSImp::isSpotLight(Light light)
{
    using namespace glsNS;

    const GLState& gls = ctx->getGLState();
    const QuadReg<float>& sl = gls.getVector(V_LIGHT_SPOT_DIRECTION + 7*light);
    return (sl[3] != -1.0f);
}

bool TLSImp::isLightEnabled(Light light)
{
    return ctx->lights[light];
}

int TLSImp::maxLights()
{
    return MAX_LIGHTS_ARB;
}


bool TLSImp::separateSpecular()
{
    return ctx->testFlags(GLContext::flagSeparateSpecular);
}

bool TLSImp::twoSidedLighting()
{
    return ctx->testFlags(GLContext::flagTwoSidedLighting);
}



bool TLSImp::anyLightEnabled()
{
    return ctx->lights.any();
}

bool TLSImp::anyLocalLight()
{
    using namespace glsNS;
    int i;
    for ( i = 0; i < MAX_LIGHTS_ARB; i++ )
    {
        if ( ctx->lights[i] )
        {
            QuadReg<float> pos = ctx->gls.getVector(V_LIGHT_POSITION + 7*i);
            if ( pos[3] != 0.0f )
                return true;
        }
    }
    return false;
}

bool TLSImp::isCullingEnabled()
{
    return ctx->testFlags(GLContext::flagCullFace);
}

bool TLSImp::isFrontFaceCulling()
{
    return (ctx->cullFace == GL_FRONT || ctx->cullFace == GL_FRONT_AND_BACK);
}

bool TLSImp::isBackFaceCulling()
{
    return (ctx->cullFace == GL_BACK || ctx->cullFace == GL_FRONT_AND_BACK);
}

TLState::ColorMaterialMode TLSImp::colorMaterialMode()
{
    switch(ctx->colorMaterialMode)
    {
        case GL_EMISSION: return TLState::EMISSION;
        case GL_AMBIENT:
        case GL_DIFFUSE:
        case GL_SPECULAR:
        case GL_AMBIENT_AND_DIFFUSE:
            panic("GLContext","TLStateImplementation::colorMaterialMode","Color material mode not implemented yet");
    }
    panic("GLContext","TLStateImplementation::colorMaterialMode","Color material mode unknown");
    return TLState::EMISSION;
}

bool TLSImp::colorMaterialFrontEnabled()
{
    return (ctx->testFlags(flagColorMaterial) &&
           ( (ctx->colorMaterialFace == GL_FRONT) || (ctx->colorMaterialFace == GL_FRONT_AND_BACK) ));
}

bool TLSImp::colorMaterialBackEnabled()
{
    return (ctx->testFlags(flagColorMaterial) &&
           ( (ctx->colorMaterialFace == GL_BACK) || (ctx->colorMaterialFace == GL_FRONT_AND_BACK) ));
}

bool TLSImp::isTextureUnitEnabled(GLuint texUnit)
{
    return (ctx->textureUnits[texUnit].getTarget() != 0);
}

bool TLSImp::anyTextureUnitEnabled()
{
    return ctx->areTexturesEnabled();
}


int TLSImp::maxTextureUnits()
{
    return ctx->driver->getTextureUnits();
    //return MAX_TEXTURE_UNITS_ARB;
}


TLState::TextureUnit TLSImp::getTextureUnit(GLuint unit)
{
    TLState::TextureUnit tu;

    const libgl::TextureUnit& texUnit = ctx->getTextureUnit(unit);

    tu.unitId = unit;

    tu.activeTexGenS = texUnit.isEnabledTexGen(GL_S);
    tu.activeTexGenT = texUnit.isEnabledTexGen(GL_T);
    tu.activeTexGenR = texUnit.isEnabledTexGen(GL_R);
    tu.activeTexGenQ = texUnit.isEnabledTexGen(GL_Q);

    switch ( texUnit.getTexGenMode(GL_S) )
    {
        case GL_OBJECT_LINEAR:
            tu.texGenModeS = TLState::TextureUnit::OBJECT_LINEAR;
            break;
        case GL_EYE_LINEAR:
            tu.texGenModeS = TLState::TextureUnit::EYE_LINEAR;
            break;
        case GL_REFLECTION_MAP:
            tu.texGenModeS = TLState::TextureUnit::REFLECTION_MAP;
            break;
        case GL_SPHERE_MAP:
            tu.texGenModeS = TLState::TextureUnit::SPHERE_MAP;
            break;
        case GL_NORMAL_MAP:
            tu.texGenModeS = TLState::TextureUnit::NORMAL_MAP;
            break;
    }

    switch ( texUnit.getTexGenMode(GL_T) )
    {
        case GL_OBJECT_LINEAR:
            tu.texGenModeT = TLState::TextureUnit::OBJECT_LINEAR;
            break;
        case GL_EYE_LINEAR:
            tu.texGenModeT = TLState::TextureUnit::EYE_LINEAR;
            break;
        case GL_REFLECTION_MAP:
            tu.texGenModeT = TLState::TextureUnit::REFLECTION_MAP;
            break;
        case GL_SPHERE_MAP:
            tu.texGenModeT = TLState::TextureUnit::SPHERE_MAP;
            break;
        case GL_NORMAL_MAP:
            tu.texGenModeT = TLState::TextureUnit::NORMAL_MAP;
            break;
    }

    switch ( texUnit.getTexGenMode(GL_R) )
    {
        case GL_OBJECT_LINEAR:
            tu.texGenModeR = TLState::TextureUnit::OBJECT_LINEAR;
            break;
        case GL_EYE_LINEAR:
            tu.texGenModeR = TLState::TextureUnit::EYE_LINEAR;
            break;
        case GL_REFLECTION_MAP:
            tu.texGenModeR = TLState::TextureUnit::REFLECTION_MAP;
            break;
        case GL_SPHERE_MAP:
            tu.texGenModeR = TLState::TextureUnit::SPHERE_MAP;
            break;
        case GL_NORMAL_MAP:
            tu.texGenModeR = TLState::TextureUnit::NORMAL_MAP;
            break;
    }

    switch ( texUnit.getTexGenMode(GL_Q) )
    {
        case GL_OBJECT_LINEAR:
            tu.texGenModeQ = TLState::TextureUnit::OBJECT_LINEAR;
            break;
        case GL_EYE_LINEAR:
            tu.texGenModeQ = TLState::TextureUnit::EYE_LINEAR;
            break;
        case GL_REFLECTION_MAP:
            tu.texGenModeQ = TLState::TextureUnit::REFLECTION_MAP;
            break;
        case GL_SPHERE_MAP:
            tu.texGenModeQ = TLState::TextureUnit::SPHERE_MAP;
            break;
        case GL_NORMAL_MAP:
            tu.texGenModeQ = TLState::TextureUnit::NORMAL_MAP;
            break;
    }

    using namespace glsNS;
    const GLState& gls = ctx->getGLState();
    Matrixf temp = gls.getMatrix(M_TEXTURE, MT_NONE, unit);

    tu.textureMatrixIsIdentity = (temp == Matrixf::identity())? true: false;

    return tu;
}

TLState::FogCoordSrc TLSImp::fogCoordSrc()
{
    GLenum fcs = ctx->getFogCoordSource();
    if ( fcs == GL_FRAGMENT_DEPTH )
        return TLState::FRAGMENT_DEPTH;
    else if ( fcs == GL_FOG_COORD )
        return TLState::FOG_COORD;

    panic("GLContext::TLStateImp", "fogCoordSrc", "Unexpected fog coordinate source");
    return TLState::FOG_COORD; // dummy
}

bool TLSImp::fogEnabled()
{
    return ctx->testFlags(flagFog);
}


/////////////////////////////////////////////////////////////////////////
///////// Implementation of inner class FPStateImplementation ///////////
/////////////////////////////////////////////////////////////////////////

#define FPSImp GLContext::FPStateImplementation


FPState::AlphaTestFunc FPSImp::alphaTestFunc()
{
    switch ( ctx->alphaFunc )
    {
        case GL_NEVER:
            return FPState::NEVER;
        case GL_ALWAYS:
            return FPState::ALWAYS;
        case GL_LESS:
            return FPState::LESS;
        case GL_LEQUAL:
            return FPState::LEQUAL;
        case GL_EQUAL:
            return FPState::EQUAL;
        case GL_GEQUAL:
            return FPState::GEQUAL;
        case GL_GREATER:
            return FPState::GREATER;
        case GL_NOTEQUAL:
            return FPState::NOTEQUAL;
        default:
            panic("GLContext::FPStateImplementation", "alphaTestFunc", "Unexpected alpha function");
            return FPState::ALWAYS; // avoids warnings
    }
}
/*
float FPSImp::alphaTestRefValue()
{
    return ctx->alphaRef;
}
*/
bool FPSImp::alphaTestEnabled()
{
    return ctx->testFlags(GLContext::flagAlphaTest);
}


FPSImp::FPStateImplementation(GLContext* parent) : ctx(parent)
{}


bool FPSImp::separateSpecular()
{
    return ctx->testFlags(GLContext::flagSeparateSpecular);
}


bool FPSImp::anyTextureUnitEnabled()
{
    return ctx->areTexturesEnabled();
}

int FPSImp::maxTextureUnits()
{
    return ctx->driver->getTextureUnits();
}

bool FPSImp::isTextureUnitEnabled(GLuint unit)
{
    return (ctx->textureUnits[unit].getTarget() != 0);
}

FPState::TextureUnit FPSImp::getTextureUnit(GLuint unit)
{
    libgl::TextureUnit& tuConf = ctx->getTextureUnit(unit);

    FPState::TextureUnit tu(unit);

    tu.unitId = unit;
    switch ( tuConf.getTarget() )
    {
        case GL_TEXTURE_1D:
            tu.activeTarget = FPState::TextureUnit::TEXTURE_1D;
            break;
        case GL_TEXTURE_2D:
            tu.activeTarget = FPState::TextureUnit::TEXTURE_2D;
            break;
        case GL_TEXTURE_3D:
            tu.activeTarget = FPState::TextureUnit::TEXTURE_3D;
            break;
        case GL_TEXTURE_CUBE_MAP:
            tu.activeTarget = FPState::TextureUnit::CUBE_MAP;
            break;
        default:
            panic("GLContext::FPStateImplementation", "getTextureUnit", "Unknown target");
    }
    switch ( tuConf.getTextureFunction() )
    {
        case GL_DECAL:
            tu.function = FPState::TextureUnit::DECAL;
            break;
        case GL_REPLACE:
            tu.function = FPState::TextureUnit::REPLACE;
            break;
        case GL_MODULATE:
            tu.function = FPState::TextureUnit::MODULATE;
            break;
        case GL_BLEND:
            tu.function = FPState::TextureUnit::BLEND;
            break;
        case GL_ADD:
            tu.function = FPState::TextureUnit::ADD;
            break;
        case GL_COMBINE:
            tu.function = FPState::TextureUnit::COMBINE;
            break;

        /* For "NV_texture_env_combine4" extension support */
        case GL_COMBINE4_NV:
            tu.function = FPState::TextureUnit::COMBINE4_NV;
            break;

        default:
            panic("GLContext::FPStateImplementation", "getTextureUnit","Unexpected texture function");
    }

    TextureObject* to = tuConf.getTextureObject();
    if ( !to )
        panic("GLContext::FPStateImplementation", "getTextureUnit", "Texture object not defined in this Texture unit!");

    switch ( to->getBaseInternalFormat() )
    {
        case GL_ALPHA:
            tu.format = FPState::TextureUnit::ALPHA;
            break;
        case GL_LUMINANCE:
            tu.format = FPState::TextureUnit::LUMINANCE;
            break;
        case GL_LUMINANCE_ALPHA:
            tu.format = FPState::TextureUnit::LUMINANCE_ALPHA;
            break;
        case GL_DEPTH:
            tu.format = FPState::TextureUnit::DEPTH;
            break;
        case GL_INTENSITY:
            tu.format = FPState::TextureUnit::INTENSITY;
            break;
        case GL_RGB:
            tu.format = FPState::TextureUnit::RGB;
            break;
        case GL_RGBA:
            tu.format = FPState::TextureUnit::RGBA;
            break;
        default:
            panic("GLContext::FPStateImplementation", "getTextureUnit", "Unknown base internal format");
    }

    if ( tu.function == FPState::TextureUnit::COMBINE || tu.function == FPState::TextureUnit::COMBINE4_NV )
    {
        switch ( tuConf.getRGBCombinerFunction() )
        {
            case GL_REPLACE:
                tu.combineMode.combineRGBFunction = FPState::TextureUnit::CombineMode::REPLACE;
                break;
            case GL_MODULATE:
                tu.combineMode.combineRGBFunction = FPState::TextureUnit::CombineMode::MODULATE;
                break;
            case GL_ADD:
                tu.combineMode.combineRGBFunction = FPState::TextureUnit::CombineMode::ADD;
                break;
            case GL_ADD_SIGNED:
                tu.combineMode.combineRGBFunction = FPState::TextureUnit::CombineMode::ADD_SIGNED;
                break;
            case GL_INTERPOLATE:
                tu.combineMode.combineRGBFunction = FPState::TextureUnit::CombineMode::INTERPOLATE;
                break;
            case GL_SUBTRACT:
                tu.combineMode.combineRGBFunction = FPState::TextureUnit::CombineMode::SUBTRACT;
                break;
            case GL_DOT3_RGB:
                tu.combineMode.combineRGBFunction = FPState::TextureUnit::CombineMode::DOT3_RGB;
                break;
            case GL_DOT3_RGBA:
                tu.combineMode.combineRGBFunction = FPState::TextureUnit::CombineMode::DOT3_RGBA;
                break;

            /* For "ATI_texture_env_combine3" extension support */
            case GL_MODULATE_ADD_ATI:
                tu.combineMode.combineRGBFunction = FPState::TextureUnit::CombineMode::MODULATE_ADD_ATI;
                break;
            case GL_MODULATE_SIGNED_ADD_ATI:
                tu.combineMode.combineRGBFunction = FPState::TextureUnit::CombineMode::MODULATE_SIGNED_ADD_ATI;
                break;
            case GL_MODULATE_SUBTRACT_ATI:
                tu.combineMode.combineRGBFunction = FPState::TextureUnit::CombineMode::MODULATE_SUBTRACT_ATI;
                break;

            default:
                panic("GLContext::FPStateImplementation", "getTextureUnit", "Unknown RGB Combiner function");
        }

        switch ( tuConf.getAlphaCombinerFunction() )
        {
            case GL_REPLACE:
                tu.combineMode.combineALPHAFunction = FPState::TextureUnit::CombineMode::REPLACE;
                break;
            case GL_MODULATE:
                tu.combineMode.combineALPHAFunction = FPState::TextureUnit::CombineMode::MODULATE;
                break;
            case GL_ADD:
                tu.combineMode.combineALPHAFunction = FPState::TextureUnit::CombineMode::ADD;
                break;
            case GL_ADD_SIGNED:
                tu.combineMode.combineALPHAFunction = FPState::TextureUnit::CombineMode::ADD_SIGNED;
                break;
            case GL_INTERPOLATE:
                tu.combineMode.combineALPHAFunction = FPState::TextureUnit::CombineMode::INTERPOLATE;
                break;
            case GL_SUBTRACT:
                tu.combineMode.combineALPHAFunction = FPState::TextureUnit::CombineMode::SUBTRACT;
                break;

            /* For "ATI_texture_env_combine3" extension support */
            case GL_MODULATE_ADD_ATI:
                tu.combineMode.combineALPHAFunction = FPState::TextureUnit::CombineMode::MODULATE_ADD_ATI;
                break;
            case GL_MODULATE_SIGNED_ADD_ATI:
                tu.combineMode.combineALPHAFunction = FPState::TextureUnit::CombineMode::MODULATE_SIGNED_ADD_ATI;
                break;
            case GL_MODULATE_SUBTRACT_ATI:
                tu.combineMode.combineALPHAFunction = FPState::TextureUnit::CombineMode::MODULATE_SUBTRACT_ATI;
                break;

            default:
                panic("GLContext::FPStateImplementation", "getTextureUnit", "Unknown Alpha Combiner function");
        }

        tu.combineMode.rgbScale = tuConf.getRGBScale();
        tu.combineMode.alphaScale = tuConf.getAlphaScale();

        GLenum nTexUnits = ctx->driver->getTextureUnits();

        for ( GLuint i = 0; i < 4; i++ )
        {
            // RGB
            GLenum src = tuConf.getSrcRGB(i);
            if ( src == GL_TEXTURE )
                tu.combineMode.srcRGB[i] = FPState::TextureUnit::CombineMode::TEXTURE;
            else if ( GL_TEXTURE0 <= src && src < nTexUnits + GL_TEXTURE0 )
            {
                tu.combineMode.srcRGB[i] = FPState::TextureUnit::CombineMode::TEXTUREn;
                tu.combineMode.srcRGB_texCrossbarReference[i] = src - GL_TEXTURE0;
            }
            else if ( src == GL_CONSTANT )
                tu.combineMode.srcRGB[i] = FPState::TextureUnit::CombineMode::CONSTANT;
            else if ( src == GL_PRIMARY_COLOR )
                tu.combineMode.srcRGB[i] = FPState::TextureUnit::CombineMode::PRIMARY_COLOR;
             else if ( src == GL_PREVIOUS )
                tu.combineMode.srcRGB[i] = FPState::TextureUnit::CombineMode::PREVIOUS;

            /* For "NV_texture_env_combine4" extension support */
            else if ( src == GL_ZERO )
                tu.combineMode.srcRGB[i] = FPState::TextureUnit::CombineMode::ZERO;

            /* For "ATI_texture_env_combine3" extension support */
            else if ( src == GL_ONE )
                tu.combineMode.srcRGB[i] = FPState::TextureUnit::CombineMode::ONE;

            else
                panic("GLContext::FPStateImplementation", "getTextureUnit", "Unexpected RGB Source or GL_TEXTUREXXX out of bounds");

            // Alpha
            src = tuConf.getSrcAlpha(i);
            if ( src == GL_TEXTURE )
                tu.combineMode.srcALPHA[i] = FPState::TextureUnit::CombineMode::TEXTURE;
            else if ( GL_TEXTURE0 <= src && src < nTexUnits + GL_TEXTURE0 )
            {
                tu.combineMode.srcALPHA[i] = FPState::TextureUnit::CombineMode::TEXTUREn;
                tu.combineMode.srcALPHA_texCrossbarReference[i] = src - GL_TEXTURE0;
            }
            else if ( src == GL_CONSTANT )
                tu.combineMode.srcALPHA[i] = FPState::TextureUnit::CombineMode::CONSTANT;
            else if ( src == GL_PRIMARY_COLOR )
                tu.combineMode.srcALPHA[i] = FPState::TextureUnit::CombineMode::PRIMARY_COLOR;
             else if ( src == GL_PREVIOUS )
                tu.combineMode.srcALPHA[i] = FPState::TextureUnit::CombineMode::PREVIOUS;

            /* For "NV_texture_env_combine4" extension support */
            else if ( src == GL_ZERO )
                tu.combineMode.srcALPHA[i] = FPState::TextureUnit::CombineMode::ZERO;

            /* For "ATI_texture_env_combine3" extension support */
            else if ( src == GL_ONE )
                tu.combineMode.srcALPHA[i] = FPState::TextureUnit::CombineMode::ONE;

            else
                panic("GLContext::FPStateImplementation", "getTextureUnit", "Unexpected Alpha Source or GL_TEXTUREXXX out of bounds");

            switch ( tuConf.getOperandRGB(i) )
            {
                case GL_SRC_COLOR:
                    tu.combineMode.operandRGB[i] = FPState::TextureUnit::CombineMode::SRC_COLOR;
                    break;
                case GL_ONE_MINUS_SRC_COLOR:
                    tu.combineMode.operandRGB[i] = FPState::TextureUnit::CombineMode::ONE_MINUS_SRC_COLOR;
                    break;
                case GL_SRC_ALPHA:
                    tu.combineMode.operandRGB[i] = FPState::TextureUnit::CombineMode::SRC_ALPHA;
                    break;
                case GL_ONE_MINUS_SRC_ALPHA:
                    tu.combineMode.operandRGB[i] = FPState::TextureUnit::CombineMode::ONE_MINUS_SRC_ALPHA;
                    break;
                default:
                    panic("GLContext::FPStateImplementation", "getTextureUnit", "Unexpected operand RGB");
            }
            switch ( tuConf.getOperandAlpha(i) )
            {
                case GL_SRC_ALPHA:
                    tu.combineMode.operandALPHA[i] = FPState::TextureUnit::CombineMode::SRC_ALPHA;
                    break;
                case GL_ONE_MINUS_SRC_ALPHA:
                    tu.combineMode.operandALPHA[i] = FPState::TextureUnit::CombineMode::ONE_MINUS_SRC_ALPHA;
                    break;
                default:
                    panic("GLContext::FPStateImplementation", "getTextureUnit", "Unexpected operand Alpha");
            }
        }
    }

    return tu;
}

bool FPSImp::fogEnabled()
{
    return ctx->testFlags(GLContext::flagFog);
}

FPState::FogMode FPSImp::fogMode()
{
    GLenum fm = ctx->getFogMode();
    if ( fm == GL_EXP )
        return FPState::EXP;
    else if ( fm == GL_EXP2 )
        return FPState::EXP2;
    else if ( fm = GL_LINEAR )
        return FPState::LINEAR;

    panic("GLContext::FPStateImp", "fogMode", "Unexpected fog mode");
    return FPState::EXP; // dummy

}

