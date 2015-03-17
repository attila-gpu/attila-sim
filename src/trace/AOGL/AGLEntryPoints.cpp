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

#include "AGLEntryPoints.h"
#include "ACDDevice.h"
#include "AGL.h"
#include "ACDRasterizationStage.h"
#include "ACDZStencilStage.h"
#include "ACDBuffer.h"
#include "GL2ACD.h"
#include "GL2ACDX.h"

#include "support.h"

#include <sstream>
#include <iostream>
#include <stdio.h>
#include <string>

#include "ACDMatrix.h"

#include "GlobalProfiler.h"

using std::cout;
using std::hex;
using std::dec;
using std::endl;

using namespace acdlib;
using namespace agl;

/*
#define AGL_PRINTLN(str) { cout << str << endl; }
#define AGL_PRINT(str) { cout << str; cout.flush(); }
#define AGL_PRINTFUNC { cout << __FUNCTION__ << endl; }
*/

//#define ENDPRINTCALL return ; // Only print the call and exit
#define ENDPRINTCALL {}
//#define ACD_STATS

//#define _PRINTCALLS_

#ifdef _PRINTCALLS_
    #define AGL_PRINTCALL { cout << __FUNCTION__ << "()" << endl; }
    #define AGL_PRINTCALL_1(a) { printCall(__FUNCTION__, a); ENDPRINTCALL }
    #define AGL_PRINTCALL_2(a1,a2) { printCall(__FUNCTION__, a1, a2); ENDPRINTCALL }
    #define AGL_PRINTCALL_3(a1,a2,a3) { printCall(__FUNCTION__, a1, a2, a3); ENDPRINTCALL }
    #define AGL_PRINTCALL_4(a1,a2,a3,a4) { printCall(__FUNCTION__,a1,a2,a3,a4); ENDPRINTCALL }
    #define AGL_PRINTCALL_6(a1,a2,a3,a4,a5,a6) { printCall(__FUNCTION__,a1,a2,a3,a4,a5,a6); ENDPRINTCALL }
    #define AGL_PRINTCALL_8(a1,a2,a3,a4,a5,a6,a7,a8) { printCall(__FUNCTION__,a1,a2,a3,a4,a5,a6,a7,a8); ENDPRINTCALL }
    #define AGL_PRINTCALL_9(a1,a2,a3,a4,a5,a6,a7,a8,a9) { printCall(__FUNCTION__,a1,a2,a3,a4,a5,a6,a7,a8,a9); ENDPRINTCALL }
#else
    #define AGL_PRINTCALL { ENDPRINTCALL }
    #define AGL_PRINTCALL_1(a) { ENDPRINTCALL }
    #define AGL_PRINTCALL_2(a1,a2) { ENDPRINTCALL }
    #define AGL_PRINTCALL_3(a1,a2,a3) { ENDPRINTCALL }
    #define AGL_PRINTCALL_4(a1,a2,a3,a4) { ENDPRINTCALL }
    #define AGL_PRINTCALL_6(a1,a2,a3,a4,a5,a6) { ENDPRINTCALL }
    #define AGL_PRINTCALL_8(a1,a2,a3,a4,a5,a6,a7,a8) { ENDPRINTCALL }
    #define AGL_PRINTCALL_9(a1,a2,a3,a4,a5,a6,a7,a8,a9) { ENDPRINTCALL }
#endif

template<class T>
std::string arrayParam(const T* arrayVar, acd_uint arrayCount)
{
    stringstream ss;
    ss << "{";
    acd_uint i;
    for ( i = 0; i < arrayCount - 1; ++i ) {
        ss << arrayVar[i] << ",";
    }
    ss << arrayVar[i] << "}";

    return ss.str();
}

template<class T>
void printCall(const acd_char* funcName, const T arg)
{
    cout << funcName << "(" << arg << ")" << endl;
}

template<class T1, class T2>
void printCall(const acd_char* funcName, const T1 arg1, const T2 arg2)
{
    cout << funcName << "(" << arg1 << "," << arg2 << ")" << endl;
}


template<class T1, class T2, class T3>
void printCall(const acd_char* funcName, const T1 arg1, const T2 arg2, const T3 arg3)
{
    cout << funcName << "(" << arg1 << "," << arg2 << "," << arg3 << ")" << endl;
}

template<class T1, class T2, class T3, class T4>
void printCall(const acd_char* funcName, const T1 arg1, const T2 arg2, const T3 arg3, const T4 arg4)
{
    cout << funcName << "(" << arg1 << "," << arg2 << "," << arg3 << "," << arg4 << ")" << endl;
}

template<class T1, class T2, class T3, class T4, class T5, class T6>
void printCall(const acd_char* funcName, const T1 arg1, const T2 arg2, const T3 arg3, const T4 arg4,
               const T5 arg5, const T6 arg6)
{
    cout << funcName << "(" << arg1 << "," << arg2 << "," << arg3 << "," << arg4
                     << "," << arg5 << "," << arg6 << ")" << endl;
}

template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
void printCall(const acd_char* funcName, const T1 arg1, const T2 arg2, const T3 arg3, const T4 arg4,
               const T5 arg5, const T6 arg6, const T7 arg7, const T8 arg8)
{
    cout << funcName << "(" << arg1 << "," << arg2 << "," << arg3 << "," << arg4
                     << "," << arg5 << "," << arg6 << "," << arg7 << "," << arg8 << ")" << endl;
}

template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
void printCall(const acd_char* funcName, const T1 arg1, const T2 arg2, const T3 arg3, const T4 arg4,
               const T5 arg5, const T6 arg6, const T7 arg7, const T8 arg8, const T9 arg9)
{
    cout << funcName << "(" << arg1 << "," << arg2 << "," << arg3 << "," << arg4
                     << "," << arg5 << "," << arg6 << "," << arg7 << "," << arg8
                     << "," << arg9 << ")" << endl;
}

GLTextureObject& getTextureObject (GLenum target)
{
    switch ( target )
    {
        case GL_TEXTURE_2D:
            target = GL_TEXTURE_2D;
            break;
        case GL_TEXTURE_3D:
            target = GL_TEXTURE_3D;
            break;
        case GL_TEXTURE_RECTANGLE:
            target = GL_TEXTURE_RECTANGLE;
            break;
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            target = GL_TEXTURE_CUBE_MAP;
            break;
        default:
            panic("AGLEntryPoints", "getTextureObject", "only TEXTURE_2D or CUBEMAP faces are allowed as target");
    }

    // Get current texture object
    GLTextureManager& texTM = _ctx->textureManager();
    return texTM.getTarget(target).getCurrent();

}

/***********************
 * OpenGL entry points *
 ***********************/

GLAPI void GLAPIENTRY AGL_glBegin( GLenum primitive )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(primitive);

    _ctx->acd().setPrimitive( agl::trPrimitive(primitive) );

    _ctx->initInternalBuffers(true); // previous buffer contents can be discarded
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glBindProgramARB (GLenum target, GLuint pid)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_2(target, pid);

    ARBProgramManager& arbPM = _ctx->arbProgramManager();
    if ( pid == 0 )
        arbPM.target(target).setDefaultAsCurrent();
    else 
    {
        // Get/Create the program with pid 'pid'
        ARBProgramObject& arbp = static_cast<ARBProgramObject&>(arbPM.bindObject(target, pid));
        arbp.attachDevice(&_ctx->acd());
    }
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glClear( GLbitfield mask )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(mask);

    acd_bool clearColorBuffer = ((mask & GL_COLOR_BUFFER_BIT) == GL_COLOR_BUFFER_BIT);
    acd_bool clearZ = ((mask & GL_DEPTH_BUFFER_BIT) == GL_DEPTH_BUFFER_BIT);
    acd_bool clearStencil = ((mask & GL_STENCIL_BUFFER_BIT) == GL_STENCIL_BUFFER_BIT);

    ACDDevice& acddev = _ctx->acd();

    if ( clearColorBuffer ) 
    {
        acd_ubyte r, g, b, a;
        _ctx->getClearColor(r,g,b,a);
        acddev.clearColorBuffer(r,g,b,a);
    }

    if ( clearZ || clearStencil ) 
    {
        acd_float zValue = _ctx->getDepthClearValue();
        acd_int stencilValue = _ctx->getStencilClearValue();
        acddev.clearZStencilBuffer(clearZ, clearStencil, zValue, stencilValue);
    }
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glClearColor( GLclampf red, GLclampf green,
                                        GLclampf blue, GLclampf alpha )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_4(red,green,blue,alpha);

    _ctx->setClearColor(red, green, blue, alpha);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glClearDepth( GLclampd depthValue )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(depthValue);

    _ctx->setDepthClearValue(depthValue);

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glColor3f( GLfloat red, GLfloat green, GLfloat blue )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_3(red, green, blue);

    _ctx->setColor(red, green, blue);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glColor4f( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_4(red, green, blue, alpha);

    _ctx->setColor(red, green, blue, alpha);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glCullFace( GLenum mode )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(mode);

    ACD_CULL_MODE ACDmode = agl::getACDCullMode(mode);
    _ctx->acd().rast().setCullMode (ACDmode);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glDepthFunc (GLenum func)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(func);

    _ctx->acd().zStencil().setZFunc(agl::getACDDepthFunc(func));
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glDisable( GLenum cap )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(cap);

    switch ( cap ) {
        case GL_DEPTH_TEST:
            _ctx->acd().zStencil().setZEnabled(false);
            break;
        case GL_LIGHTING:
            _ctx->fixedPipelineSettings().lightingEnabled = false;
            break;
        case GL_LIGHT0:
        case GL_LIGHT1:
        case GL_LIGHT2:
        case GL_LIGHT3:
        case GL_LIGHT4:
        case GL_LIGHT5:
        case GL_LIGHT6:
        case GL_LIGHT7:
        {
            acd_uint light = cap - GL_LIGHT0;
            _ctx->fixedPipelineSettings().lights[light].enabled = false;
            break;
        }
        case GL_VERTEX_PROGRAM_ARB:
            _ctx->setRenderState(RS_VERTEX_PROGRAM, false); // Disable user vertex program
            break;
        case GL_FRAGMENT_PROGRAM_ARB:
            _ctx->setRenderState(RS_FRAGMENT_PROGRAM, false); // Disable user fragment program
            break;
        case GL_TEXTURE_1D:
        case GL_TEXTURE_2D:
        case GL_TEXTURE_3D:
        case GL_TEXTURE_CUBE_MAP:
        case GL_TEXTURE_RECTANGLE:
            {
                GLTextureManager& texTM = _ctx->textureManager();
                GLuint tu = texTM.getCurrentGroup(); // get active texture unit

                TextureUnit& texUnit = _ctx->getTextureUnit(tu);

                texUnit.disableTarget(cap); // disable target in the active texture unit
                GLuint texUnitNum = _ctx->getActiveTextureUnitNumber();
                //  Check if all targets have been disabled for the active texture unit.
                if (!texUnit.isTextureUnitActive())
                    _ctx->fixedPipelineSettings().textureStages[texUnitNum].enabled = false;
            }
            break;
        case GL_TEXTURE_GEN_S:
        case GL_TEXTURE_GEN_T:
        case GL_TEXTURE_GEN_R:
        case GL_TEXTURE_GEN_Q:
            {
                TextureUnit& texUnit = _ctx->getActiveTextureUnit();
                GLenum select = 0;

                if ( cap == GL_TEXTURE_GEN_S )
                    select = GL_S;
                else if ( cap == GL_TEXTURE_GEN_T )
                    select = GL_T;
                else if ( cap == GL_TEXTURE_GEN_R )
                    select = GL_R;
                else if ( cap == GL_TEXTURE_GEN_Q )
                    select = GL_Q;
                if ( texUnit.isEnabledTexGen(select) )
                    texUnit.setEnabledTexGen(select, false);
            }
        case GL_FOG:
            _ctx->fixedPipelineSettings().fogEnabled = false;
            break;
        case GL_BLEND:
            _ctx->acd().blending().setEnable(0,false);
            break;
        case GL_CULL_FACE:
            _ctx->acd().rast().setCullMode(ACD_CULL_NONE);
            break;
        case GL_NORMALIZE:
            _ctx->fixedPipelineSettings().normalizeNormals = acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_NO_NORMALIZE;
            break;
        case GL_STENCIL_TEST:
            _ctx->acd().zStencil().setStencilEnabled(false);
            break;
        case GL_ALPHA_TEST:
            _ctx->fixedPipelineSettings().alphaTestEnabled = false;
            _ctx->acd().alphaTestEnabled(false);
            break;
        case GL_COLOR_MATERIAL:
            _ctx->fixedPipelineSettings().colorMaterialEnabledFace = acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_NONE;
            break;
        case GL_SCISSOR_TEST:
            _ctx->acd().rast().enableScissor(false);
            break;
        case GL_POLYGON_OFFSET_FILL:
                _ctx->acd().zStencil().setPolygonOffset(0.0, 0.0);
            break;
        default:
            break;
            //panic("AGLEntryPoints", "AGL_glDisable", "Unknown glDisable parameter");
    }
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glEnable( GLenum cap )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(cap);

    switch ( cap ) {
        case GL_DEPTH_TEST:
            _ctx->acd().zStencil().setZEnabled(true);
            break;
        case GL_LIGHTING:
            _ctx->fixedPipelineSettings().lightingEnabled = true;
            break;
        case GL_LIGHT0:
        case GL_LIGHT1:
        case GL_LIGHT2:
        case GL_LIGHT3:
        case GL_LIGHT4:
        case GL_LIGHT5:
        case GL_LIGHT6:
        case GL_LIGHT7:
        {
            acd_uint light = cap - GL_LIGHT0;
            _ctx->fixedPipelineSettings().lights[light].enabled = true;
            break;
        }
        case GL_VERTEX_PROGRAM_ARB:
            _ctx->setRenderState(RS_VERTEX_PROGRAM, true); // Enable user vertex program
            break;
        case GL_FRAGMENT_PROGRAM_ARB:
            _ctx->setRenderState(RS_FRAGMENT_PROGRAM, true); // Enable user fragment program
            break;
        case GL_CULL_FACE:
            _ctx->acd().rast().setCullMode(ACD_CULL_BACK);
            break;
        case GL_TEXTURE_1D:
        case GL_TEXTURE_2D:
        case GL_TEXTURE_3D:
        case GL_TEXTURE_CUBE_MAP:
            {
                TextureUnit& texUnit = _ctx->getActiveTextureUnit();
                GLTextureManager& texTM = _ctx->textureManager();
                GLTextureObject& to = texTM.getTarget(cap).getCurrent();
                texUnit.enableTarget(cap); // enable target in the active texture unit
                texUnit.setTextureObject(cap, &to);
                GLuint texUnitNum = _ctx->getActiveTextureUnitNumber();
                _ctx->fixedPipelineSettings().textureStages[texUnitNum].enabled = true;
                _ctx->fixedPipelineSettings().textureStages[texUnitNum].activeTextureTarget = agl::getACDXTextureTarget(cap);
            }
            break;
        case GL_TEXTURE_GEN_S:
        case GL_TEXTURE_GEN_T:
        case GL_TEXTURE_GEN_R:
        case GL_TEXTURE_GEN_Q:
            {
                TextureUnit& texUnit = _ctx->getActiveTextureUnit();
                GLenum select = 0;

                if ( cap == GL_TEXTURE_GEN_S )
                    select = GL_S;
                else if ( cap == GL_TEXTURE_GEN_T )
                    select = GL_T;
                else if ( cap == GL_TEXTURE_GEN_R )
                    select = GL_R;
                else if ( cap == GL_TEXTURE_GEN_Q )
                    select = GL_Q;
                if ( !texUnit.isEnabledTexGen(select) )
                    texUnit.setEnabledTexGen(select, true);
            }
            break;
        case GL_FOG:
            _ctx->fixedPipelineSettings().fogEnabled = true;
            break;
        case GL_BLEND:
            _ctx->acd().blending().setEnable(0,true);
            break;
        case GL_NORMALIZE:
            _ctx->fixedPipelineSettings().normalizeNormals = acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_NORMALIZE;
            break;
        case GL_STENCIL_TEST:
            _ctx->acd().zStencil().setStencilEnabled(true);
            break;
        case GL_ALPHA_TEST:
            _ctx->fixedPipelineSettings().alphaTestEnabled = true;
            _ctx->acd().alphaTestEnabled(true);
            break;
        case GL_COLOR_MATERIAL:
            _ctx->fixedPipelineSettings().colorMaterialEnabledFace = acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_FRONT;
            break;
        case GL_SCISSOR_TEST:
            _ctx->acd().rast().enableScissor(true);
            break;
        case GL_POLYGON_OFFSET_FILL:
            // Nothing to be done
            break;
        default:
            break;
            //panic("AGLEntryPoints.cpp", "AGL_glEnable", "Unknown glEnable parameter");
    }
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glEnableClientState (GLenum cap)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")

    AGL_PRINTCALL_1(cap);

    switch ( cap )
    {
        case GL_VERTEX_ARRAY:
        case GL_COLOR_ARRAY:
        case GL_NORMAL_ARRAY:
        case GL_INDEX_ARRAY:
        case GL_TEXTURE_COORD_ARRAY:
        case GL_EDGE_FLAG_ARRAY:
            _ctx->enableBufferArray (cap);
            break;
        default:
            panic("AGLEntryPoints", "glEnableClientState", "Unexpected client state");
    }
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glDisableClientState (GLenum cap)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")

    AGL_PRINTCALL_1(cap);

    switch ( cap )
    {
        case GL_VERTEX_ARRAY:
        case GL_COLOR_ARRAY:
        case GL_NORMAL_ARRAY:
        case GL_INDEX_ARRAY:
        case GL_TEXTURE_COORD_ARRAY:
        case GL_EDGE_FLAG_ARRAY:
            _ctx->disableBufferArray (cap);
            break;
        default:
            panic("AGLEntryPoints", "glDisableClientState", "Unexpected client state");
    }
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glEnd( void )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL;

    // Sets the proper shader (user/auto-generated) and update GPU shader constants
    _ctx->setShaders();

    // Use internal buffers to draw
    _ctx->attachInternalBuffers(); // Attach internal buffers to streams

    _ctx->attachInternalSamplers();

    // Perform the draw using the internal buffers as input
    _ctx->acd().draw(0, _ctx->countInternalVertexes());

#ifdef ACD_STATS
    _ctx->acd().DBG_dump("acddump.txt", 0);
    _ctx->fixedPipelineState().DBG_dump("acdxjustbeforedraw.txt",0);
#endif

    _ctx->deattachInternalSamplers();

    _ctx->deattachInternalBuffers();
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glFlush( void )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL;

    // nothing to be done
    GLOBALPROFILER_EXITREGION()
}


GLAPI void GLAPIENTRY AGL_glLightfv( GLenum light, GLenum pname, const GLfloat *params )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_3(light,pname,params);

    _ctx->setLightParam(light, pname, params);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glLightf( GLenum light, GLenum pname, const GLfloat params )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_3(light,pname,params);

    _ctx->setLightParam(light, pname, &params);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glLightModelfv( GLenum pname, const GLfloat *params )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_2(pname,params);

    _ctx->setLightModel(pname, params);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glLightModelf( GLenum pname, const GLfloat param )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_2(pname,param);

    _ctx->setLightModel(pname, &param);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glLightModeli( GLenum pname, const GLint param )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_2(pname,param);

    GLfloat _param = static_cast<GLfloat>(param);
    _ctx->setLightModel(pname, &_param);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glLoadIdentity( void )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL;

    // Create identity matrix
    ACDXFloatMatrix4x4 identity;
    acdlib::identityMatrix(identity);

    // Load the identity matrix in the current active stack/unit
    _ctx->matrixStack().set(identity);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glMaterialfv( GLenum face, GLenum pname, const GLfloat *params )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_3(face,pname,params);

    _ctx->setMaterialParam(face, pname, params);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glMaterialf( GLenum face, GLenum pname, const GLfloat params )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_3(face,pname,params);

    _ctx->setMaterialParam(face, pname, &params);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glMatrixMode( GLenum mode )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(mode);

    // Set active modelview decoding the GLenum token if required
    if ( mode == GL_MODELVIEW )
        _ctx->setActiveModelview(0);
    else if ( GL_MODELVIEW1_ARB <= mode && mode <= GL_MODELVIEW31_ARB ) {
        mode = GL_MODELVIEW;
        _ctx->setActiveModelview(GL_MODELVIEW31_ARB - mode + 1);
    }

    _ctx->setMatrixMode(mode);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glMultMatrixd( const GLdouble *m )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(arrayParam(m,16));

    // Convert doubles to floats
    GLfloat floatData[16];
    for ( acd_uint i = 0; i < 16; ++i )
        floatData[i] = static_cast<GLfloat>(m[i]);

    ACDXFloatMatrix4x4 mat(floatData, false); // Set row-major to false, matrix is expressed in major-column

    _ctx->matrixStack().multiply(mat);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glNormal3f( GLfloat nx, GLfloat ny, GLfloat nz )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_3(nx, ny, nz);

    _ctx->setNormal(nx, ny, nz);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glNormal3fv( const GLfloat *v )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(arrayParam(v,3));

    _ctx->setNormal(v[0], v[1], v[2]);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glOrtho( GLdouble left, GLdouble right,
                                 GLdouble bottom, GLdouble top,
                                 GLdouble near_, GLdouble far_ )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_6(left, right, bottom, top, near_, far_);

    ACDXFloatMatrix4x4 mat;

    mat(0,0) = 2.0F / static_cast<acd_float>(right-left);
    mat(1,0) = 0.0F;
    mat(2,0) = 0.0F;
    mat(3,0) = 0.0F;

    mat(0,1) = 0.0F;
    mat(1,1) = 2.0F / static_cast<acd_float>(top-bottom);
    mat(2,1) = 0.0F;
    mat(3,1) = 0.0F;

    mat(0,2) = 0.0F;
    mat(1,2) = 0.0F;
    mat(2,2) = -2.0F / static_cast<acd_float>(far_-near_);
    mat(3,2) = 0.0F;

    mat(0,3) = static_cast<acd_float>(-(right+left) / (right-left));
    mat(1,3) = static_cast<acd_float>(-(top+bottom) / (top-bottom));
    mat(2,3) = static_cast<acd_float>(-(far_+near_) / (far_-near_));
    mat(3,3) = 1.0F;

    // Multiply the current matrix by 'mat'
    _ctx->matrixStack().multiply(mat);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glFrustum( GLdouble left, GLdouble right,
                                    GLdouble bottom, GLdouble top,
                                    GLdouble near_, GLdouble far_ )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_6(left, right, bottom, top, near_, far_);

    ACDXFloatMatrix4x4 mat;

    mat(0,0) = (2.0F*static_cast<acd_float>(near_)) / static_cast<acd_float>(right-left);
    mat(1,0) = 0.0F;
    mat(2,0) = 0.0F;
    mat(3,0) = 0.0F;

    mat(0,1) = 0.0F;
    mat(1,1) = (2.0F*static_cast<acd_float>(near_)) / static_cast<acd_float>(top-bottom);
    mat(2,1) = 0.0F;
    mat(3,1) = 0.0F;

    mat(0,2) = static_cast<acd_float>((right+left) / (right-left));
    mat(1,2) = static_cast<acd_float>((top+bottom) / (top-bottom));
    mat(2,2) = static_cast<acd_float>(-(far_+near_) / (far_-near_));
    mat(3,2) = -1.0F;

    mat(0,3) = 0.0F;
    mat(1,3) = 0.0F;
    mat(2,3) = static_cast<acd_float>(-(2*far_*near_) / (far_-near_));
    mat(3,3) = 0.0F;

    // Multiply the current matrix by 'mat'
    _ctx->matrixStack().multiply(mat);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glPopMatrix( void )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL;

    _ctx->matrixStack().pop();
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glProgramLocalParameter4fARB (GLenum target, GLuint index,
                                                  GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_6(target,index,x,y,z,w);

    // Get the local parameter bank of the current
    ARBRegisterBank& locals = _ctx->arbProgramManager().target(target).getCurrent().getLocals();

    // Update local parameter bank
    locals.set(index, x, y, z, w);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glProgramLocalParameters4fvEXT (GLenum target, GLuint index, GLsizei count, const GLfloat* d)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    //AGL_PRINTCALL_6(target,index,x,y,z,w);

    // Get the local parameter bank of the current
    ARBRegisterBank& locals = _ctx->arbProgramManager().target(target).getCurrent().getLocals();

    // Update local parameter bank
    for (acd_uint i = 0; i < count ; i++)
        locals.set(index + i, d[i*4], d[i*4+1], d[i*4+2], d[i*4+3]);

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glProgramLocalParameter4fvARB (GLenum target, GLuint index, const GLfloat *v)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_3(target, index, v);

    // Get the local parameter bank of the current
    ARBRegisterBank& locals = _ctx->arbProgramManager().target(target).getCurrent().getLocals();

    // Update local parameter bank
    locals.set(index,v[0], v[1], v[2], v[3]);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glProgramStringARB (GLenum target, GLenum format, GLsizei len, const GLvoid * str)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_4(target, format, len, str);

    _ctx->arbProgramManager().programString(target, format, len, str);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glPushMatrix( void )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL;

    _ctx->matrixStack().push();
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glRotatef( GLfloat angle, GLfloat x, GLfloat y, GLfloat z )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_4(angle, x, y, z);

    ACDFloatMatrix4x4 m = _ctx->matrixStack().get();

    acdlib::_rotate(m, angle, x, y, z);

    _ctx->matrixStack().set(m);

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glRotated( GLdouble angle, GLdouble x, GLdouble y, GLdouble z )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_4(angle, x, y, z);

    ACDFloatMatrix4x4 m = _ctx->matrixStack().get();

    acdlib::_rotate(m, static_cast<acd_float> (angle), static_cast<acd_float> (x), static_cast<acd_float> (y), static_cast<acd_float> (z));

    _ctx->matrixStack().set(m);

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glScalef( GLfloat x, GLfloat y, GLfloat z)
{
    AGL_PRINTCALL_3(x, y, z);

    ACDFloatMatrix4x4 m = _ctx->matrixStack().get();

    acdlib::_scale(m, static_cast<acd_float> (x), static_cast<acd_float> (y), static_cast<acd_float> (z));

    _ctx->matrixStack().set(m);
}

GLAPI void GLAPIENTRY AGL_glShadeModel( GLenum mode )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(mode);

    // Atila Rasterizer uses fshInputAttribute[1] to store the fragment color
    _ctx->acd().rast().setInterpolationMode(1, agl::trInterpolationMode(mode));
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glTranslatef( GLfloat x, GLfloat y, GLfloat z )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_3(x,y,z);

    ACDFloatMatrix4x4 m = _ctx->matrixStack().get();

    acdlib::_translate(m, x, y, z);

    _ctx->matrixStack().set(m);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glTranslated( GLdouble x, GLdouble y, GLdouble z )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_3(x,y,z);

    ACDFloatMatrix4x4 m = _ctx->matrixStack().get();

    acdlib::_translate(m,  static_cast<acd_float> (x),  static_cast<acd_float> (y),  static_cast<acd_float> (z));

    _ctx->matrixStack().set(m);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glVertex3f( GLfloat x, GLfloat y, GLfloat z )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_3(x, y, z);

    _ctx->addVertex(x, y, z);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glVertex2f( GLfloat x, GLfloat y)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_2(x, y);

    _ctx->addVertex(x, y, 0);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glVertex2i ( GLint x, GLint y)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_2(x, y);

    _ctx->addVertex(static_cast<GLfloat>(x), static_cast<GLfloat>(y), 0);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glVertex3fv( const GLfloat *v )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(arrayParam(v,3));

    _ctx->addVertex(v[0], v[1], v[2]);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glViewport( GLint x, GLint y, GLsizei width, GLsizei height )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_4(x,y,width,height);

    ACDDevice& acddev = _ctx->acd();

    acddev.rast().setViewport(x, y, width, height);

    // PATCH for old traces which do not contain the window resolution
    // Use the viewport as current window resolution
    acd_uint w, h;
    if ( !acddev.getResolution(w,h) ) // Resolution not defined yet (backwards compatibility)
        acddev.setResolution(static_cast<acd_uint>(width), static_cast<acd_uint>(height));
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glVertexPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_4(size,type,stride,ptr);

    BufferManager& bufBM = _ctx->bufferManager();

    _ctx->getPosVArray() = GLContext::VArray(size, type, stride, ptr, true);

    if (bufBM.getTarget(GL_ARRAY_BUFFER).hasCurrent())
        _ctx->getPosVArray().bufferID = bufBM.getTarget(GL_ARRAY_BUFFER).getCurrent().getName();
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glColorPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_4(size,type,stride,ptr);

    BufferManager& bufBM = _ctx->bufferManager();

    _ctx->getColorVArray() = GLContext::VArray(size, type, stride, ptr, false);

    if (bufBM.getTarget(GL_ARRAY_BUFFER).hasCurrent())
        _ctx->getColorVArray().bufferID = bufBM.getTarget(GL_ARRAY_BUFFER).getCurrent().getName();
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glNormalPointer( GLenum type, GLsizei stride, const GLvoid *ptr )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_3(type,stride,ptr);

    BufferManager& bufBM = _ctx->bufferManager();

    _ctx->getNormalVArray() = GLContext::VArray(3, type, stride, ptr, true);

      if (bufBM.getTarget(GL_ARRAY_BUFFER).hasCurrent())
        _ctx->getNormalVArray().bufferID = bufBM.getTarget(GL_ARRAY_BUFFER).getCurrent().getName();
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glDrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices )
{
    GLOBALPROFILER_ENTERREGION("glDraw", "", "")
    AGL_PRINTCALL_4(mode,count,type,indices);

    BufferManager& bufBM = _ctx->bufferManager();

    _ctx->initInternalBuffers(false);
    _ctx->acd().setPrimitive( agl::trPrimitive(mode) );

    set<GLuint> iSet;
    acdlib::ACDBuffer* indicesBuffer; //

    if ( !_ctx->allBuffersBound() && !_ctx->allBuffersUnbound() )
        panic("AGLEntryPoints", "glDrawElements", "Mixed VBO and client-side vertex arrays not supported in indexed mode");

    if ( _ctx->allBuffersBound())
    {
        if ( bufBM.getTarget(GL_ELEMENT_ARRAY_BUFFER).hasCurrent() )
        {
            BufferObject boElementArray = bufBM.target(GL_ELEMENT_ARRAY_BUFFER).getCurrent();
            indicesBuffer = boElementArray.getData();
                _ctx->acd().setIndexBuffer(indicesBuffer, (u64bit)indices, agl::getACDStreamType(type, false));

        }
        else
        {
            indicesBuffer = _ctx->rawIndices(indices, type, count);
                _ctx->acd().setIndexBuffer(indicesBuffer, 0, agl::getACDStreamType(type, false));

        }
    }
    else // ctx->allBuffersUnbound() == TRUE
    {
        if ( bufBM.getTarget(GL_ELEMENT_ARRAY_BUFFER).hasCurrent())
            panic("AGLEntryPoints", "glDrawElements", "Unbound GL_ARRAY_BUFFER & bound GL_ELEMENTS_ARRAY_BUFFER not supported");
        else
        {
            _ctx->getIndicesSet(iSet, type, indices, count);
            indicesBuffer = _ctx->createIndexBuffer (indices, type, count, iSet);
        }

            _ctx->acd().setIndexBuffer(indicesBuffer, 0, agl::getACDStreamType(type, false));

    }


    _ctx->setShaders(); // Sets the proper shader (user/auto-generated) and update GPU shader constants

    _ctx->setVertexArraysBuffers(0, count, iSet);

    _ctx->attachInternalBuffers();

    _ctx->attachInternalSamplers(); // This operation is done with the information provided by the shader. This is why it MUST BE DONE AFTER setShader

    _ctx->acd().drawIndexed(0, count, 0, 0, 0); // Perform the draw using the internal buffers as input

#ifdef ACD_STATS
    _ctx->acd().DBG_dump("acddump.txt", 0);
    _ctx->fixedPipelineState().DBG_dump("acdxjustbeforedraw.txt",0);
#endif

    _ctx->deattachInternalSamplers();

    _ctx->deattachInternalBuffers();

    _ctx->resetDescriptors();

    if (!bufBM.getTarget(GL_ELEMENT_ARRAY_BUFFER).hasCurrent())
        _ctx->acd().destroy(indicesBuffer);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glDrawArrays( GLenum mode, GLint first, GLsizei count )
{
    GLOBALPROFILER_ENTERREGION("glDraw", "", "")

    AGL_PRINTCALL_3(mode,first,count);

    _ctx->acd().setPrimitive( agl::trPrimitive(mode) );

    set<GLuint> iSet;

    _ctx->setShaders(); // Sets the proper shader (user/auto-generated) and update GPU shader constants

    _ctx->setVertexArraysBuffers(first, count, iSet);

    _ctx->attachInternalBuffers(); // Attach internal buffers to streams

    _ctx->attachInternalSamplers();

    _ctx->acd().draw(0, count); // Perform the draw using the internal buffers as input

#ifdef ACD_STATS
    _ctx->acd().DBG_dump("acddump.txt", 0);
    _ctx->fixedPipelineState().DBG_dump("acdxjustbeforedraw.txt",0);
#endif

    _ctx->deattachInternalSamplers();

    _ctx->deattachInternalBuffers();

    _ctx->resetDescriptors();

}

GLAPI void GLAPIENTRY AGL_glDrawRangeElements( GLenum mode, GLuint start, GLuint end, GLsizei count,
                                           GLenum type, const GLvoid *indices )

{
    GLOBALPROFILER_ENTERREGION("glDraw", "", "")

    AGL_PRINTCALL_6(mode, start, end, count, type, indices);

    BufferManager& bufBM = _ctx->bufferManager();

    _ctx->acd().setPrimitive( agl::trPrimitive(mode) );


    set<GLuint> iSet;   // Conjunt d'indexos que s'utilitzen en el drawElements
    acdlib::ACDBuffer* indicesBuffer; //

    if ( !_ctx->allBuffersBound() && !_ctx->allBuffersUnbound() )
        panic("AGLEntryPoints", "glDrawRangeElements", "Mixed VBO and client-side vertex arrays not supported in indexed mode");

    if ( _ctx->allBuffersBound())
    {
        if ( bufBM.getTarget(GL_ELEMENT_ARRAY_BUFFER).hasCurrent() )
        {
            BufferObject boElementArray = bufBM.target(GL_ELEMENT_ARRAY_BUFFER).getCurrent();
            indicesBuffer = _ctx->rawIndices(boElementArray.getData()->getData(), type, count);
        }
        else
            indicesBuffer = _ctx->rawIndices(indices, type, count);
    }
    else // ctx->allBuffersUnbound() == TRUE
    {
        if ( bufBM.getTarget(GL_ELEMENT_ARRAY_BUFFER).hasCurrent())
            panic("AGLEntryPoints", "glDrawRangeElements", "Unbound GL_ARRAY_BUFFER & bound GL_ELEMENTS_ARRAY_BUFFER not supported");
        else
        {
            _ctx->getIndicesSet(iSet, type, indices, count);
            indicesBuffer = _ctx->createIndexBuffer (indices, type, count, iSet);
        }
    }

    _ctx->acd().setIndexBuffer(indicesBuffer, 0, agl::getACDStreamType(type, false));

    _ctx->setShaders(); // Sets the proper shader (user/auto-generated) and update GPU shader constants

    _ctx->setVertexArraysBuffers(0, count, iSet);

    _ctx->attachInternalBuffers();

    _ctx->attachInternalSamplers();

    _ctx->acd().drawIndexed(0, count, 0, 0, 0); // Perform the draw using the internal buffers as input

#ifdef ACD_STATS
    _ctx->acd().DBG_dump("acddump.txt", 0);
    _ctx->fixedPipelineState().DBG_dump("acdxjustbeforedraw.txt",0);
#endif

    _ctx->deattachInternalBuffers();

    _ctx->deattachInternalSamplers();

    _ctx->resetDescriptors();

    if (!bufBM.getTarget(GL_ELEMENT_ARRAY_BUFFER).hasCurrent())
        _ctx->acd().destroy(indicesBuffer);

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glMultMatrixf( const GLfloat *m )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(arrayParam(m,16));

    ACDXFloatMatrix4x4 mat(m, false);
    _ctx->matrixStack().multiply(mat);

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glBindBufferARB(GLenum target, GLuint buffer)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")

    AGL_PRINTCALL_2(target,buffer);

    BufferManager& bufBM = _ctx->bufferManager();

    if ( buffer == 0 )
        bufBM.target(target).resetCurrent(); // unbind buffer (not using VBO)
    else
    {
        BufferObject& buff = static_cast<BufferObject&>(bufBM.bindObject(target, buffer));
        buff.attachDevice(&_ctx->acd());
    }
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glDeleteBuffersARB(GLsizei n, const GLuint *buffers)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_2(n,buffers);

    BufferManager& bufBM = _ctx->bufferManager();
    bufBM.removeObjects(n, buffers);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glBufferDataARB(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")

    AGL_PRINTCALL_4(target, size, data, usage);

    BufferManager& bufBM = _ctx->bufferManager();
    bufBM.target(target).getCurrent().setContents(size, (u8bit*)data, usage);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glBufferSubDataARB (GLenum target, GLintptrARB offset,
                                              GLsizeiptrARB size, const GLvoid *data)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    
    AGL_PRINTCALL_4(target, offset, size, data);

    BufferManager& bufBM = _ctx->bufferManager();
    bufBM.target(target).getCurrent().setPartialContents(offset, size, (u8bit*)data);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glTexCoord2f( GLfloat s, GLfloat t )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_2(s, t);

    _ctx->setTexCoord(0,s,t); // Set texture coordinate for texture unit 0
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glBindTexture( GLenum target, GLuint texture )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_2(target, texture);

    GLTextureManager& tm = _ctx->textureManager(); // Get the texture manager

    GLuint tu = tm.getCurrentGroup(); // get id of current texture unit

    if ( texture == 0 ) // binding the default (it is not a TextureObject)
    {
        GLTextureTarget& tg = tm.getTarget(target);
        tg.setDefaultAsCurrent(); // set the default as current
        _ctx->getTextureUnit(tu).setTextureObject(target, &tg.getCurrent());
    }
    else
    {
        // Find the texture object
        GLTextureObject& to = static_cast<GLTextureObject&>(tm.bindObject(target, texture)); // get (or create) the texture object
        // Set the object as the current for the texture unit
        to.attachDevice(&_ctx->acd(),target);
        _ctx->getTextureUnit(tu).setTextureObject(target, &to);
    }
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glTexParameteri( GLenum target, GLenum pname, GLint param )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_3(target, pname, param);

    GLTextureManager& texTM = _ctx->textureManager();
    texTM.getTarget(target).getCurrent().setParameter(pname, &param);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glTexParameterf( GLenum target, GLenum pname, GLfloat param )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_3(target, pname, param);

    GLTextureManager& texTM = _ctx->textureManager();
    texTM.getTarget(target).getCurrent().setParameter(pname, &param);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glTexImage2D( GLenum target, GLint level,
                                        GLint internalFormat,
                                        GLsizei width, GLsizei height,
                                        GLint border, GLenum format, GLenum type,
                                        const GLvoid *pixels )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")


    AGL_PRINTCALL_9(target,level, internalFormat, width, height, border, format, type, pixels);

    GLTextureObject& to = getTextureObject(target);
    to.attachDevice(&_ctx->acd(),target);

    GLOBALPROFILER_ENTERREGION("setContents", "GLTextureObject", "setContents")

    switch (internalFormat)
    {
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
            internalFormat = GL_RGBA;
            break;
    }

    to.setContents(target, level, internalFormat, width, height, 1, border, format, type, (const GLubyte*)pixels);
    
    GLOBALPROFILER_EXITREGION()
    GLOBALPROFILER_EXITREGION()
}


GLAPI void GLAPIENTRY AGL_glTexImage3D( GLenum target, GLint level,
                                        GLint internalFormat,
                                        GLsizei width, GLsizei height,
                                        GLsizei depth, GLint border,
                                        GLenum format, GLenum type,
                                        const GLvoid *pixels )

{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")


    AGL_PRINTCALL_9(target,level, internalFormat, width, height, border, format, type, pixels);

    GLTextureObject& to = getTextureObject(target);
    to.attachDevice(&_ctx->acd(),target);

    GLOBALPROFILER_ENTERREGION("setContents", "GLTextureObject", "setContents")
    
        switch (internalFormat)
    {
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
            internalFormat = GL_RGBA;
            break;
    }

    to.setContents(target, level, internalFormat, width, height, depth, border, format, type, (const GLubyte*)pixels);
    
    GLOBALPROFILER_EXITREGION()
    GLOBALPROFILER_EXITREGION()

}

GLAPI void GLAPIENTRY AGL_glCompressedTexImage2D( GLenum target, GLint level,
                                              GLenum internalFormat,
                                              GLsizei width, GLsizei height,
                                              GLint border, GLsizei imageSize,
                                              const GLvoid *data )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    

    AGL_PRINTCALL_8(target, level, internalFormat, width, height, border, imageSize, data);


    GLTextureObject& to = getTextureObject(target);
    to.attachDevice(&_ctx->acd(),target);


    GLOBALPROFILER_ENTERREGION("setContents", "GLTextureObject", "setContents")


    to.setContents(target, level, internalFormat, width, height, 1, border, 0, 0, (const GLubyte*)data, imageSize);


    GLOBALPROFILER_EXITREGION()
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glCompressedTexSubImage2DARB (  GLenum target,
                                                    GLint level,
                                                    GLint xoffset,
                                                    GLint yoffset,
                                                    GLsizei width,
                                                    GLsizei height,
                                                    GLenum format,
                                                    GLsizei imageSize,
                                                    const GLvoid *data)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    

    AGL_PRINTCALL_8(target, level, internalFormat, width, height, border, imageSize, data);

    GLTextureObject& to = getTextureObject(target);
    to.attachDevice(&_ctx->acd(),target);


    GLOBALPROFILER_ENTERREGION("setContents", "GLTextureObject", "setContents")


    to.setPartialContents(target, level, xoffset, yoffset, 0, width, height, 1, format, 0, (const GLubyte*)data, imageSize);


    GLOBALPROFILER_EXITREGION()
    GLOBALPROFILER_EXITREGION()

}

GLAPI void GLAPIENTRY AGL_glCompressedTexImage2DARB( GLenum target, GLint level,
                                                 GLenum internalFormat,
                                                 GLsizei width, GLsizei height,
                                                 GLint border, GLsizei imageSize,
                                                 const GLvoid *data )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")


    AGL_PRINTCALL_8(target, level, internalFormat, width, height, border, imageSize, data);

    GLTextureObject& to = getTextureObject(target);
    to.attachDevice(&_ctx->acd(),target);

    GLOBALPROFILER_ENTERREGION("setContents", "GLTextureObject", "setContents")

    to.setContents(target, level, internalFormat, width, height, 1, border, 0, 0, (const GLubyte*)data, imageSize);

    GLOBALPROFILER_EXITREGION()
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glLoadMatrixf( const GLfloat *m )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(m);
    // Create identity matrix
    ACDXFloatMatrix4x4 mat(m, false);

    _ctx->matrixStack().set(mat);

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glTexEnvi( GLenum target, GLenum pname, GLint param )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_3(target, pname, param);

    TextureUnit& tu = _ctx->getActiveTextureUnit();
    tu.setParameter(target, pname, &param);

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glTexEnvf( GLenum target, GLenum pname, GLfloat param )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_3(target, pname, param);

    TextureUnit& tu = _ctx->getActiveTextureUnit();
    tu.setParameter(target, pname, &param);

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glTexEnvfv( GLenum target, GLenum pname, const GLfloat* param )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_3(target, pname, param);

    TextureUnit& tu = _ctx->getActiveTextureUnit();
    tu.setParameter(target, pname, param);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glActiveTextureARB(GLenum texture)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(texture);

    GLTextureManager& texTM = _ctx->textureManager();
    GLuint maxUnit = texTM.numberOfGroups() - 1;
    GLuint group = texture - GL_TEXTURE0;

    if ( GL_TEXTURE0 <= texture && texture <= GL_TEXTURE0 + maxUnit )
        texTM.selectGroup(group); // select the active group (active texture unit)
    else
    {
        char msg[256];
        sprintf(msg, "TextureUnit %d does not exist", group);
        panic("AGLEntryPoints", "glActiveTextureARB", msg);
    }
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glFogf( GLenum pname, GLfloat param )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_2(pname,param);

    _ctx->setFog(pname, &param);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glFogi( GLenum pname, GLint param )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_2(pname,param);

    _ctx->setFog(pname, &param);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glFogfv( GLenum pname, const GLfloat *params )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_2(pname,params);

    _ctx->setFog(pname, params);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glFogiv( GLenum pname, const GLint *params )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_2(pname,params);

    _ctx->setFog(pname, params);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glTexCoordPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_4(size, type, stride, ptr);

    BufferManager& bufBM = _ctx->bufferManager();

    GLuint clientTUnit = _ctx->getCurrentClientTextureUnit();

    _ctx->getTextureVArray(clientTUnit) = GLContext::VArray(size, type, stride, ptr, true);

    if (bufBM.getTarget(GL_ARRAY_BUFFER).hasCurrent())
        _ctx->getTextureVArray(clientTUnit).bufferID = bufBM.getTarget(GL_ARRAY_BUFFER).getCurrent().getName();
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glBlendFunc( GLenum sfactor, GLenum dfactor )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_2(sfactor, dfactor);

    _ctx->acd().blending().setSrcBlend(0, agl::getACDBlendOption(sfactor));
    _ctx->acd().blending().setSrcBlendAlpha(0, agl::getACDBlendOption(sfactor));
    _ctx->acd().blending().setDestBlend(0, agl::getACDBlendOption(dfactor));
    _ctx->acd().blending().setDestBlendAlpha(0, agl::getACDBlendOption(dfactor));
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glFrontFace( GLenum mode )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(mode);

    switch ( mode )
    {
        case GL_CW:
            _ctx->acd().rast().setFaceMode(acdlib::ACD_FACE_CW);
            break;
        case GL_CCW:
            _ctx->acd().rast().setFaceMode(acdlib::ACD_FACE_CCW);
            break;
        default:
            panic("AGLEntryPoint", "glFrontFace()", "Unkown front face mode");
    }
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glTexGenfv(GLenum coord, GLenum pname, const GLfloat* param)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_3(coord,pname,param);

    _ctx->setTexGen(coord, pname, param);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glTexGeni(GLenum coord, GLenum pname, const GLint param)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_3(coord,pname,param);

    _ctx->setTexGen(coord, pname, param);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glTexGenf(GLenum coord, GLenum pname, const GLfloat param)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_3(coord,pname,param);

    _ctx->setTexGen(coord, pname, GLfloat(param));
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glClientActiveTextureARB(GLenum texture)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(texture);

    _ctx->setCurrentClientTextureUnit(texture - GL_TEXTURE0);
}

GLAPI void GLAPIENTRY AGL_glCopyTexImage2D( GLenum target, GLint level,
                                            GLenum internalFormat,
                                            GLint x, GLint y,
                                            GLsizei width, GLsizei height,
                                            GLint border )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_8(target, level, internalFormat, x, y, width, height, border);

    GLTextureObject& to = getTextureObject(target);

    switch(internalFormat)
    {
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            internalFormat = GL_RGBA;
            break;

        case GL_DEPTH_COMPONENT24:
            internalFormat = GL_RGBA;
    }

    GLubyte* data = new GLubyte[width*height*4];

    for (int i=0; i< width*height*4; i+=4)
    {
        if (i%0x3 == 0)
        {
            data[i] = 255;
            data[i+1] = 0;
            data[i+2] = 0;
            data[i+3] = 255;
        }
        else if (i%0x3 == 1)
        {
            data[i] = 0;
            data[i+1] = 255;
            data[i+2] = 0;
            data[i+3] = 255;
        }
        else if (i%0x3 == 2)
        {
            data[i] = 0;
            data[i+1] = 0;
            data[i+2] = 255;
            data[i+3] = 255;
        }
        else
        {
            data[i] = 255;
            data[i+1] = 255;
            data[i+2] = 0;
            data[i+3] = 255;
        }
    }

    GLOBALPROFILER_ENTERREGION("setContents", "GLTextureObject", "setContext")
    to.setContents(target, level, internalFormat, width, height, 1, border, internalFormat, GL_UNSIGNED_BYTE, (const GLubyte*)data, 0);
    GLOBALPROFILER_EXITREGION()

    delete[] data;

    _ctx->acd().sampler(_ctx->getActiveTextureUnitNumber()).performBlitOperation2D(0, 0,
        x, y, width, height, to.getWidth(target,level), agl::getACDTextureFormat(internalFormat), (acdlib::ACDTexture2D*)to.getTexture(), level);

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glCopyTexSubImage2D( GLenum target, GLint level,
                                           GLint xoffset, GLint yoffset,
                                           GLint x, GLint y,
                                           GLsizei width, GLsizei height )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")

    AGL_PRINTCALL_8(target, level, xoffset, yoffset, x, y, width, height);

    GLTextureObject& to = getTextureObject(target);

	GLenum internalFormat = to.getFormat(target,level);
    internalFormat = to.getGLTextureFormat((ACD_FORMAT) internalFormat);

    switch(internalFormat)
    {
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            internalFormat = GL_RGBA;
            break;
        case GL_DEPTH_COMPONENT24:
            internalFormat = GL_RGBA;
    }

    GLubyte* subData = new GLubyte[width * height * 4];

    for (int i=0; i< width*height*4; i+=4)
    {
        if (i%0x3 == 0)
        {
            subData[i] = 255;
            subData[i+1] = 0;
            subData[i+2] = 0;
            subData[i+3] = 255;
        }
        else if (i%0x3 == 1)
        {
            subData[i] = 0;
            subData[i+1] = 255;
            subData[i+2] = 0;
            subData[i+3] = 255;
        }
        else if (i%0x3 == 2)
        {
            subData[i] = 0;
            subData[i+1] = 0;
            subData[i+2] = 255;
            subData[i+3] = 255;
        }
        else
        {
            subData[i] = 255;
            subData[i+1] = 255;
            subData[i+2] = 0;
            subData[i+3] = 255;
        }
    }

    GLOBALPROFILER_ENTERREGION("setContents", "GLTextureObject", "setContext")
    to.setPartialContents(target, level, xoffset, yoffset, 0, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, subData, 0);
    GLOBALPROFILER_EXITREGION()

    delete[] subData;

    _ctx->acd().sampler(_ctx->getActiveTextureUnitNumber()).performBlitOperation2D(xoffset, yoffset,
        x, y, width, height,to.getWidth(target,level), agl::getACDTextureFormat(internalFormat), (acdlib::ACDTexture2D*)to.getTexture(), level);

    GLOBALPROFILER_EXITREGION()
}


GLAPI void GLAPIENTRY AGL_glDepthMask( GLboolean flag )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(flag);

    _ctx->acd().zStencil().setZMask(flag);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glEnableVertexAttribArrayARB (GLuint index)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(index);

    _ctx->setEnabledGenericAttribArray(index, true);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glDisableVertexAttribArrayARB (GLuint index)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(index);

    _ctx->setEnabledGenericAttribArray(index, false);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glVertexAttribPointerARB(GLuint index, GLint size, GLenum type,
                                                GLboolean normalized, GLsizei stride, const GLvoid *pointer)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_6(index, size, type, normalized, stride, pointer);

    BufferManager& bufBM = _ctx->bufferManager();

    _ctx->getGenericVArray(index) = GLContext::VArray(size, type, stride, pointer, normalized);

    if (bufBM.getTarget(GL_ARRAY_BUFFER).hasCurrent())
        _ctx->getGenericVArray(index).bufferID = bufBM.getTarget(GL_ARRAY_BUFFER).getCurrent().getName();

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glStencilFunc( GLenum func, GLint ref, GLuint mask )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_3(func, ref, mask);

    _ctx->acd().zStencil().setStencilFunc(acdlib::ACD_FACE_FRONT,agl::getACDCompareFunc(func), ref, mask);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glStencilOp( GLenum fail, GLenum zfail, GLenum zpass )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")

    AGL_PRINTCALL_3 (fail, zfail, zpass);

    _ctx->acd().zStencil().setStencilOp(acdlib::ACD_FACE_FRONT,
                                        agl::getACDStencilOp(fail),
                                        agl::getACDStencilOp(zfail),
                                        agl::getACDStencilOp(zpass));

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glStencilOpSeparateATI(GLenum face, GLenum fail, GLenum zfail, GLenum zpass )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")

    AGL_PRINTCALL_3 (fail, zfail, zpass);

    _ctx->acd().zStencil().setStencilOp(agl::getACDFace(face),
                                        agl::getACDStencilOp(fail),
                                        agl::getACDStencilOp(zfail),
                                        agl::getACDStencilOp(zpass));

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glClearStencil( GLint stencilValue )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")

    AGL_PRINTCALL_1(stencilValue);

    _ctx->setStencilClearValue(stencilValue);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glStencilMask( GLuint mask )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(mask);

    _ctx->acd().zStencil().setStencilUpdateMask(mask);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glColorMask( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")

    AGL_PRINTCALL_4(red, green, blue, alpha);

    _ctx->acd().blending().setColorMask(0, red, green, blue, alpha);

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glAlphaFunc( GLenum func, GLclampf ref )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_2(func, ref);

    _ctx->fixedPipelineSettings().alphaTestFunction = agl::getACDXAlphaFunc(func);
    //_ctx->fixedPipelineState().postshade().setAlphaTestRefValue((acd_float) ref);

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glBlendColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_4(red, green, blue, alpha);

    _ctx->acd().blending().setBlendColor(0, red, green, blue, alpha);

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glColorMaterial( GLenum face, GLenum mode )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")

    AGL_PRINTCALL_2(face, mode);

    _ctx->fixedPipelineSettings().colorMaterialEnabledFace = agl::getACDXColorMaterialFace(face);
    _ctx->fixedPipelineSettings().colorMaterialMode = agl::getACDXColorMaterialMode(mode);

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glScissor( GLint x, GLint y, GLsizei width, GLsizei height)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_4(x, y, width, height);

    _ctx->acd().rast().setScissor(x, y, width, height);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glDepthRange( GLclampd near_val, GLclampd far_val )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")

    AGL_PRINTCALL_2(near_val, far_val);

    _ctx->fixedPipelineState().fshade().setDepthRange(near_val, far_val);
    _ctx->acd().zStencil().setDepthRange(static_cast<GLfloat>(near_val), static_cast<GLfloat>(far_val));

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glPolygonOffset( GLfloat factor, GLfloat units )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")

    AGL_PRINTCALL_2(factor, units);

    _ctx->acd().zStencil().setPolygonOffset (factor, units);

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glPushAttrib( GLbitfield mask )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(mask);

    _ctx->pushAttrib(mask);
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glPopAttrib( void )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(void);

    _ctx->popAttrib();
    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glBlendEquation( GLenum mode )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(mode);

    _ctx->acd().blending().setBlendFunc(0, agl::getACDBlendFunction(mode));

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glColor4ub( GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")

    AGL_PRINTCALL_4(red, green, blue, alpha);

    GLfloat r = GLfloat(red) / 0xFF;
    GLfloat g = GLfloat(green) / 0xFF;
    GLfloat b = GLfloat(blue) / 0xFF;
    GLfloat a = GLfloat(alpha) / 0xFF;

    _ctx->setColor(r, g, b, a);

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glColor4fv( const GLfloat *v )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")

    AGL_PRINTCALL_4(red, green, blue, alpha);

    _ctx->setColor(v[0], v[1], v[2], v[3]);

    GLOBALPROFILER_EXITREGION()
}

GLAPI const GLubyte* GLAPIENTRY AGL_glGetString( GLenum name )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")

    AGL_PRINTCALL_1(name);

    static GLubyte vendor[] = "DAC";
    static GLubyte renderer[] = "ATILA GPU";
    static GLubyte ver[] = "1.0";
    static GLubyte extensions[] = "Why do you want to check extensions ? We support ALL";
    switch ( name )
    {
        case GL_VENDOR:
            return vendor;
        case GL_RENDERER:
            return renderer;
        case GL_VERSION:
            return ver;
        case GL_EXTENSIONS:
            return extensions;
    }

    GLOBALPROFILER_EXITREGION()
    panic("AGLEntryPoints","glGetString()","Unknown constant");
    return 0;

}

GLAPI void GLAPIENTRY AGL_glTexSubImage2D( GLenum target, GLint level,
                                       GLint xoffset, GLint yoffset,
                                       GLsizei width, GLsizei height,
                                       GLenum format, GLenum type,
                                       const GLvoid *pixels )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")

    AGL_PRINTCALL_9(target,level, xoffset, yoffset, width, height, format, type, pixels);

    GLTextureObject& to = getTextureObject(target);

    GLOBALPROFILER_ENTERREGION("setContents", "GLTextureObject", "setContext")
    to.setPartialContents(target, level, xoffset, yoffset, 0, width, height, 0, format, type, (const GLubyte*)pixels);
    GLOBALPROFILER_EXITREGION()

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glMultiTexCoord2f( GLenum texture, GLfloat s, GLfloat t )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")

    AGL_PRINTCALL_3(texture, s, t);

    GLuint texUnit = texture - GL_TEXTURE0;
    _ctx->setTexCoord(texUnit, s, t);

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glMultiTexCoord2fv( GLenum texture, const GLfloat *v )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")

    AGL_PRINTCALL_2(texture, v);

    GLuint texUnit = texture - GL_TEXTURE0;
    _ctx->setTexCoord(texUnit, v[0], v[1]);

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glPixelStorei( GLenum pname, GLint param )
{
    AGL_PRINTCALL_2(pname, param );

    //cout << "glPixelStorei ---> call ignored\n";

}

GLAPI void GLAPIENTRY AGL_glIndexPointer( GLenum type, GLsizei stride, const GLvoid *ptr )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")

    AGL_PRINTCALL_3(type, stride, ptr);

    BufferManager& bufBM = _ctx->bufferManager();

    _ctx->getIndexVArray() = GLContext::VArray(1, type, stride, ptr, false);

    if ( bufBM.getTarget(GL_ARRAY_BUFFER).hasCurrent())
        _ctx->getIndexVArray().bufferID = bufBM.getTarget(GL_ARRAY_BUFFER).getCurrent().getName();

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glEdgeFlagPointer( GLsizei stride, const GLvoid *ptr )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_2(stride, ptr);

    BufferManager& bufBM = _ctx->bufferManager();

    _ctx->getEdgeVArray() = GLContext::VArray(1,GL_UNSIGNED_BYTE,stride, ptr, false);

    if ( bufBM.getTarget(GL_ARRAY_BUFFER).hasCurrent() )
        _ctx->getEdgeVArray().bufferID = bufBM.getTarget(GL_ARRAY_BUFFER).getCurrent().getName();

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glArrayElement( GLint i )
{
    AGL_PRINTCALL_1(i);

    panic("AGLEntryPoints","glArrayElement()","Function not implemented :-)");

    // called inside glBegin/End
    // uses glBegin/glEnd buffers
}

GLAPI void GLAPIENTRY AGL_glInterleavedArrays( GLenum format, GLsizei stride, const GLvoid *pointer )
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")

    AGL_PRINTCALL_3(format, stride, pointer);

    switch ( format )
    {
        case GL_C4F_N3F_V3F:
            // enable vertex arrays
            _ctx->enableBufferArray (GL_VERTEX_ARRAY);
            _ctx->enableBufferArray (GL_COLOR_ARRAY);
            _ctx->enableBufferArray (GL_NORMAL_ARRAY);

            if (stride == 0)
                stride = 10*sizeof(GLfloat); // See OpenGL 2.0 Specification section 2.8 Vertex Arrays at pages 31 and 32

            _ctx->getColorVArray() = GLContext::VArray(4, GL_FLOAT, stride, pointer, true);
            _ctx->getNormalVArray() = GLContext::VArray(3, GL_FLOAT, stride, ((GLubyte *)pointer) + 4*sizeof(GLfloat), true);
            _ctx->getPosVArray() = GLContext::VArray(3, GL_FLOAT, stride, ((GLubyte *)pointer) + 7*sizeof(GLfloat), true);
            break;
        default:
            panic("AGLEntryPoints", "AGL_glInterleavedArrays()", "Format not yet unsupported");
    }

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glDeleteTextures(GLsizei n, const GLuint *textures)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")

    AGL_PRINTCALL_2(n, textures);

    GLTextureManager& texTM = _ctx->textureManager();
    texTM.removeObjects(n, textures);
    GLOBALPROFILER_EXITREGION()

}

GLAPI void GLAPIENTRY AGL_glProgramEnvParameter4fvARB (GLenum target, GLuint index, const GLfloat *params)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_3(target, index, params);

    // Get the enviromental parameter bank of the current
    ARBRegisterBank& env = _ctx->arbProgramManager().target(target).getEnv();

    // Update enviromental parameter bank
    env.set(index,params[0], params[1], params[2], params[3]);

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glProgramEnvParameters4fvEXT (GLenum target, GLuint index, GLsizei count, const GLfloat* d)
{
    GLOBALPROFILER_ENTERREGION("AOGL", "", "")

    // Get the enviromental parameter bank of the current
    ARBRegisterBank& env = _ctx->arbProgramManager().target(target).getEnv();

    // Update enviromental parameter bank
    for (acd_uint i = 0; i < count ; i++)
        env.set(index + i, d[i*4], d[i*4+1], d[i*4+2], d[i*4+3]);

    GLOBALPROFILER_EXITREGION()
}


GLAPI void GLAPIENTRY AGL_glPolygonMode	(GLenum face, GLenum mode)
{

    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_2(face, mode);

    if (mode != GL_FILL)
        panic("AGLEntryPoints", "glPolygonMode", "GPU only supports GL_FILL mode");

    GLOBALPROFILER_EXITREGION()
}

GLAPI void GLAPIENTRY AGL_glBlendEquationEXT (GLenum mode)
{

    GLOBALPROFILER_ENTERREGION("AOGL", "", "")
    AGL_PRINTCALL_1(mode);

    _ctx->acd().blending().setBlendFunc(0, agl::getACDBlendFunction(mode));

    GLOBALPROFILER_EXITREGION()
}
