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
 * $RCSfile: GPULib.cpp,v $

/*
 * OpenGL API library implementation.
 *
 */

// Our includes

/*
 * Define all private structures and constants used in this file
 */
#include "GPULibInternals.h"


// System includes
#include <malloc.h>
#include <stdio.h>
#include "support.h"
#include "ProgramManager.h"
#include "TextureManager.h"
#include "BufferManager.h"
#include "glext.h"
#include "MathLib.h"
#include "GPUMemory.h"
#include "AuxFuncsLib.h"
#include <set>

using namespace std;
using namespace libgl;
using namespace gpu3d;

/*************************
 * Vertex Buffer Objects *
 *************************/

//  Hack for Riddick
//static bool ignoreClearAfterTexSubImage = false;

GLAPI void GLAPIENTRY glBindBufferARB(GLenum target, GLuint buffer)
{

    PRINT_CALL("glBindBufferARB(0x" << hex << target << dec << "," << GLuint(buffer) << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glBindBufferARB);

    if ( buffer == 0 )
        BufferManager::instance().getTarget(target).resetCurrent(); // unbind buffer (not using VBO)
    else
        BufferManager::instance().bindObject(target, buffer);
}

GLAPI void GLAPIENTRY glDeleteBuffersARB(GLsizei n, const GLuint *buffers)
{
    PRINT_CALL("glDeleteBuffersARB(" << n << "," << std::hex << GLuint(buffers) << std::dec << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glDeleteBuffersARB);

    afl::releaseObjects(BufferManager::instance(), n, buffers);
}


GLAPI void GLAPIENTRY glBufferDataARB(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
{
    PRINT_CALL("glBufferDataARB(0x" << hex << target << dec << "," << GLint(size) << ",0x" << std::hex << GLuint(data) << std::dec << "," << usage << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glBufferDataARB);

    // checks for errors and sets contents
    GET_BUFFER(target).setContents(size, (u8bit*)data, usage);
}

GLAPI void GLAPIENTRY glBufferSubDataARB (GLenum target, GLintptrARB offset,
                                          GLsizeiptrARB size, const GLvoid *data)
{
    PRINT_CALL("glBufferSubDataARB(0x" << hex << target << dec << "," << GLint(offset) << "," << GLint(size) << ",0x" << std::hex << GLuint(data) << std::dec << endl; )

    CHECK_NOT_BEGIN_MODE(glBufferSubDataARB);

    GET_BUFFER(target).setPartialContents(offset, size, (u8bit*)data);
}

GLAPI void GLAPIENTRY glBegin( GLenum mode )
{
    PRINT_CALL("glBegin(" << mode << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glBegin);

    SET_BATCH_MODE_SUPPORT(mode); // path that allows to ignore unsupported batches (i.e GL_LINES, GL_POINT, etc)

    ctx->setFlags(GLContext::flagBeginEnd);

    if ( GL_POLYGON < mode || mode < GL_POINTS )
        panic("GPULib","glBegin()", "GLenum is not a valid primitive mode");

    ctx->setPrimitive(mode);

    ctx->clearBuffers();    /* Clean all previous contents */

    // If mode not supported, return (do not modify gpu state)
    CHECK_BATCH_MODE_SUPPORT_MSG("Warning (glBegin). Primitive mode not supported. Batch ignored\n");

    // temporary code
    switch( mode )
    {
        case GL_POINTS:
            panic("GPULib", "glBegin", "Primitive mode GL_POINTS not supported.");
            break;
        case GL_LINES:
            panic("GPULib", "glBegin", "Primitive mode GL_LINES not supported.");
            break;
        case GL_LINE_STRIP:
            panic("GPULib", "glBegin", "Primitive mode GL_LINE_STRIP not supported.");
            break;
        case GL_LINE_LOOP:
            panic("GPULib", "glBegin", "Primitive mode GL_LINE_LOOP not supported.");
            break;
        case GL_TRIANGLES:
            break;
        case GL_TRIANGLE_STRIP:
            break;
        case GL_TRIANGLE_FAN:
            break;
        case GL_QUADS:
            break;
        case GL_QUAD_STRIP:
            break;
        case GL_POLYGON:
            panic("GPULib", "glBegin", "Primitive mode GL_POLYGON not supported.");
            break;
    }
    // end of temporary code

    // Primitive value is maintained in both GPUSimulator and GPULibrary
    GPURegData data;
    data.primitive = afl::translatePrimitive(mode);
    driver->writeGPURegister(GPU_PRIMITIVE, data);

}

GLAPI void GLAPIENTRY glClear( GLbitfield mask )
{
    PRINT_CALL("glClear(" << mask << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glClear);

    /*  NOTE:  THE SIMULATOR DOESN'T SUPPORT CLEARING STENCIL AND
        Z BUFFER SEPARATELY YET!!!!.  */

    bool clearColor = ( mask & GL_COLOR_BUFFER_BIT ) == GL_COLOR_BUFFER_BIT;
    bool clearZ = ( mask & GL_DEPTH_BUFFER_BIT ) == GL_DEPTH_BUFFER_BIT;
    bool clearStencil = ( mask & GL_STENCIL_BUFFER_BIT ) == GL_STENCIL_BUFFER_BIT;

    //  Hack for Riddick
/*    if (ignoreClearAfterTexSubImage)
    {
        //  Ignore clear color after glCopyTexSubImage2D call
        clearColor = false;
        ignoreClearAfterTexSubImage = false;

        cout << "glClear => Hack for Riddick.  Ignoring color clear after glCopyTexSubImage2D()" << endl;
    }
*/

    //  Check if scissor test is enabled
    if (ctx->testFlags(GLContext::flagScissorTest))
    {
        GLint scissorIniX;
        GLint scissorIniY;
        GLsizei scissorWidth;
        GLsizei scissorHeight;

        //  Get scissor rectangle.
        ctx->getScissor(scissorIniX, scissorIniY, scissorWidth, scissorHeight);

        u32bit resWidth;
        u32bit resHeight;

        //  Get framebuffer resolution
        driver->getResolution(resWidth, resHeight);

        //  Check if the scissor rectangle doesn't cover all the resolution
        if ((scissorIniX != 0) || (scissorIniY != 0) || (scissorWidth != resWidth) || (scissorHeight != resHeight))
        {
            afl::clear(ctx, mask);
            return;
        }
    }


    //  Deferred change of the clear color value
    if (clearColor)
    {
        GLfloat red, green, blue, alpha;
        GPURegData data;

        ctx->getClearColor(red, green, blue, alpha);

        data.qfVal[0] = red;
        data.qfVal[1] = green;
        data.qfVal[2] = blue;
        data.qfVal[3] = alpha;
        
        driver->writeGPURegister( GPU_COLOR_BUFFER_CLEAR, data );
    }

    // Deferred change of the clear depth value
    if (clearZ)
    {
        GLclampd depth;
        GPURegData data;

        depth = ctx->getDepthClearValue();

        //  The float point depth value in the 0 .. 1 range must be converted to an integer
        //  value of the selected depth bit precission.
        //
        //  WARNING: This function should depend on the current depth bit precission.  Should be
        //  configurable using the wgl functions that initialize the framebuffer.  By default
        //  should be 24 (stencil 8 depth 24).  Currently the simulator only supports 24.
        //

        data.uintVal = u32bit((f32bit(depth) * f32bit((1 << 24) - 1)));
        driver->writeGPURegister(GPU_Z_BUFFER_CLEAR, data);
    }

    // Deferred change of the clear stencil value
    if (clearStencil)
    {
        GLint s = ctx->getStencilBufferClearValue();

        GPURegData data;
        data.intVal = s;

        driver->writeGPURegister(GPU_STENCIL_BUFFER_CLEAR, data);
    }

    if ( clearZ && clearColor && clearStencil )
    {
        ctx->setClearStencilWarning(false);
        driver->sendCommand( GPU_CLEARZSTENCILBUFFER );
        driver->sendCommand( GPU_CLEARCOLORBUFFER );

        /*  Set HZ Buffer content as valid.  */
        ctx->setHZBufferValidFlag(true);
    }
    else {
        if ( clearZ && clearStencil )
        {
            ctx->setClearStencilWarning(false);
            driver->sendCommand( GPU_CLEARZSTENCILBUFFER );

            /*  Set HZ Buffer content as valid.  */
            ctx->setHZBufferValidFlag(true);
        }
        else if ( clearZ || clearStencil )
        {
            if ( clearStencil )
            {
                afl::clear(ctx, mask);
            }

            if ( clearZ )
            {
                cout << endl << "Warning: clearZ implemented as clear z and stencil (optimization)" << endl;

                //afl::clear(ctx, mask);
                driver->sendCommand( GPU_CLEARZSTENCILBUFFER );

                /*  Set HZ Buffer content as valid.  */
                ctx->setHZBufferValidFlag(true);
            }
        }

        if ( clearColor )
            driver->sendCommand( GPU_CLEARCOLORBUFFER );
    }

}

GLAPI void GLAPIENTRY glClearColor( GLclampf red, GLclampf green,
                                   GLclampf blue, GLclampf alpha )
{
    PRINT_CALL("glClearColor(" << red << ", " << green << ", " << blue << ", " << alpha << ");" << endl;)

    CHECK_NOT_BEGIN_MODE(glClearColor);

    red = afl::clamp(red);
    green = afl::clamp(green);
    blue = afl::clamp(blue);
    alpha = afl::clamp(alpha);

    ctx->setClearColor(red, green, blue, alpha);
}


GLAPI void GLAPIENTRY glColor3f( GLfloat red, GLfloat green, GLfloat blue )
{
    PRINT_CALL("glColor3f(" << red << ", " << green << ", " << blue << ");" << endl; )
    ctx->setColor(red,green,blue,1.0f);
}


GLAPI void GLAPIENTRY glLoadIdentity( void )
{
    PRINT_CALL("glLoadIdentity();" << endl; )

    CHECK_NOT_BEGIN_MODE(glLoadIdentity);

    ctx->ctop(Matrixf::identity()); /* replace current matrix with identity */

}


GLAPI void GLAPIENTRY glMatrixMode( GLenum mode )
{
    PRINT_CALL("glMatrixMode(" << mode << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glMatrixMode);

    ctx->setCurrentMatrix(mode);
}

GLAPI void GLAPIENTRY glOrtho( GLdouble left, GLdouble right,
                                 GLdouble bottom, GLdouble top,
                                 GLdouble near_, GLdouble far_ )
{
    PRINT_CALL("glOrtho(" << left << ", " << right << ", " << bottom << ", " << top << ", " << near_ << ", " << far_ << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glMatrixMode);

    if ( near_ == far_  || near_ == far_ || top == bottom )
        panic("GPULib", "glOrtho", "Invalid GL parameter values");

    Matrixf mat;

    mat[0][0] = 2.0F / static_cast<f32bit>(right-left);
    mat[1][0] = 0.0F;
    mat[2][0] = 0.0F;
    mat[3][0] = 0.0F;

    mat[0][1] = 0.0F;
    mat[1][1] = 2.0F / static_cast<f32bit>(top-bottom);
    mat[2][1] = 0.0F;
    mat[3][1] = 0.0F;

    mat[0][2] = 0.0F;
    mat[1][2] = 0.0F;
    mat[2][2] = -2.0F / static_cast<f32bit>(far_-near_);
    mat[3][2] = 0.0F;

    mat[0][3] = static_cast<f32bit>(-(right+left) / (right-left));
    mat[1][3] = static_cast<f32bit>(-(top+bottom) / (top-bottom));
    mat[2][3] = static_cast<f32bit>(-(far_+near_) / (far_-near_));
    mat[3][3] = 1.0F;

    Matrixf newProj = ctx->mtop(GLContext::PROJECTION) * mat;
    ctx->ctop(newProj, GLContext::PROJECTION);

}

GLAPI void GLAPIENTRY glFrustum( GLdouble left, GLdouble right,
                                   GLdouble bottom, GLdouble top,
                                   GLdouble near_, GLdouble far_ )
{
    PRINT_CALL("glFrustum(" << left << ", " << right << ", " << bottom << ", " << top << ", " << near_ << ", " << far_ << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glFrustum);

    if ( near_ <= 0.0  || far_ <= 0.0 || near_ == far_ || top == bottom )
        panic("GPULib", "glFrustum", "Invalid GL parameter values");

    Matrixf mat;

    mat[0][0] = (2.0F*static_cast<f32bit>(near_)) / static_cast<f32bit>(right-left); // M(0,0)
    mat[1][0] = 0.0F; // M(1,0)
    mat[2][0] = 0.0F; // M(2,0)
    mat[3][0] = 0.0F; // M(3,0)

    mat[0][1] = 0.0F; // M(0,1)
    mat[1][1] = (2.0F*static_cast<f32bit>(near_)) / static_cast<f32bit>(top-bottom); // M(1,1)
    mat[2][1] = 0.0F; // M(2,1)
    mat[3][1] = 0.0F; // M(3,1)

    mat[0][2] = static_cast<f32bit>((right+left) / (right-left)); // M(0,2)
    mat[1][2] = static_cast<f32bit>((top+bottom) / (top-bottom)); // M(1,2)
    mat[2][2] = static_cast<f32bit>(-(far_+near_) / (far_-near_)); // M(2,2)
    mat[3][2] = -1.0F; // M(3,2)

    mat[0][3] = 0.0F; // M(0,3)
    mat[1][3] = 0.0F; // M(1,3)
    mat[2][3] = static_cast<f32bit>(-(2*far_*near_) / (far_-near_)); // M(2,3)
    mat[3][3] = 0.0F; // M(3,3)

    Matrixf newProj = ctx->mtop(GLContext::PROJECTION) * mat;
    ctx->ctop(newProj, GLContext::PROJECTION);

}


GLAPI void GLAPIENTRY glShadeModel( GLenum mode )
{
    PRINT_CALL("glShadeModel(" << mode << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glShadeModel);

    ctx->setShadeModel( mode == GL_FLAT ? false : true );

    GPURegData data;
    data.booleanVal = ( mode == GL_FLAT ? false : true );

    driver->writeGPURegister( GPU_INTERPOLATION, COLOR_ATTRIBUTE, data );
}


GLAPI void GLAPIENTRY glVertex2i( GLint x, GLint y )
{
    PRINT_CALL("glVertex2i(" << x << ", " << y << ");" << endl )

    CHECK_BEGIN_MODE(glVertex2i);

    afl::glVertex(ctx, GLfloat(x), GLfloat(y));
}

GLAPI void GLAPIENTRY glVertex2f( GLfloat x, GLfloat y )
{
    PRINT_CALL("glVertex2f(" << x << ", " << y << ");" << endl )

    CHECK_BEGIN_MODE(glVertex2f);

    afl::glVertex(ctx, x, y);
}

GLAPI void GLAPIENTRY glVertex3f( GLfloat x, GLfloat y, GLfloat z )
{
    PRINT_CALL("glVertex3f(" << x << ", " << y << ", " << z << ");" << endl; )

    CHECK_BEGIN_MODE(glVertex3f);

    afl::glVertex(ctx, x, y, z);
}

GLAPI void GLAPIENTRY glViewport( GLint x, GLint y, GLsizei width, GLsizei height )
{
    PRINT_CALL("glViewport(" << x << ", " << y << ", " << width << ", " << height << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glViewport);

    GPURegData data;

    data.intVal = x;
    driver->writeGPURegister( GPU_VIEWPORT_INI_X, data );
    data.intVal = y;
    driver->writeGPURegister( GPU_VIEWPORT_INI_Y, data );
    data.uintVal = width;
    driver->writeGPURegister( GPU_VIEWPORT_WIDTH, data );
    data.uintVal = height;
    driver->writeGPURegister( GPU_VIEWPORT_HEIGHT, data );

    ctx->setViewport(x, y, width, height);
    ctx->setSetupShaderConstantsUpdate(false);

    if ( !driver->isResolutionDefined() ) // backwards compatibility
    {
        driver->setResolution(width, height);
        driver->initBuffers();
        char msg[256];
        sprintf(msg, "Warning. Resolution not defined, using viewport as resolution: %dx%d\n",width, height);
        popup("GPULib", msg);
        //cout << "Warning. Resolution not defined, using viewport as resolution: " << width << "x" << height << endl;
    }
}


GLAPI void GLAPIENTRY glNormal3f( GLfloat nx, GLfloat ny, GLfloat nz )
{
    PRINT_CALL("glNormal3f(" << nx << ", " << ny << ", " << nz << ");" << endl; )

    ctx->setNormal(nx,ny,nz);

    ENABLE_USENORMALS;
}

GLAPI void GLAPIENTRY glEnd( void )
{
    PRINT_CALL("glEnd();" << endl; )

    CHECK_BEGIN_MODE(glEnd);

    ctx->resetFlags(GLContext::flagBeginEnd);

    CHECK_BATCH_MODE_SUPPORT; // quits this function if not supported

    bool useTextures = ctx->areTexturesEnabled();

    bool disableColorMask = false;


#ifdef GLLIB_DUMP_STREAMS
    GLubyte filename[30];
    sprintf((char*)filename, "GLLibDumpStreams.txt");
    remove((char*)filename);
#endif

#ifdef GLLIB_DUMP_SAMPLERS
    for (GLuint i = 0; i < ProgramObject::MaxTextureUnits; i++)
        for (GLuint j = 0; j < 6; j++)
            for (GLuint k = 0; k < 13; k++)
            {
                sprintf((char*)filename, "Sampler%d_Face%d_Mipmap%d.ppm", i,j,k);
                remove((char*)filename);
            }
#endif

    /************************************
     * Synchronize texture image in GPU *
     ************************************/
    if ( useTextures )
        afl::synchronizeTextureImages(ctx, driver, disableColorMask);

    /**********************
     * Memory descriptors *
     **********************/
    u32bit mdVertex = 0;
    u32bit mdColor = 0;
    u32bit mdNormal = 0;
    vector<u32bit> mdTextures; // Texture unit memory descriptor mdTextures[i] for texUnit[i]
    vector<u32bit> texUnit; // Texture unit id's

    /***********************************
     * Setup Vertex & Fragment shaders *
     ***********************************/
    afl::setupShaders(ctx, driver);

    /*  Setup Hierarchical Z Test.  */
    afl::setupHZTest(ctx, driver);

    /**************************
     * CONFIGURE GPU STREAMER *
     **************************/
    GPUDriver::VertexAttribute va;
    va.enabled = false; /* disable the other attributes */

    int i;
    for ( i = 0; i < MAX_VERTEX_ATTRIBUTES; i++ )
    {
        va.attrib = (GPUDriver::ShAttrib)i;
        driver->configureVertexAttribute(va);
    }

    // global setting for data streams
    va.offset = 0; // no offset
    va.stride = 0;
    va.componentsType = SD_FLOAT;
    va.enabled = true;

#define CONFIG_ATTRIB(stream_,md_,attrib_,compos_)\
    { va.stream = stream_;\
    va.attrib = attrib_;\
    va.md = md_;\
    va.components = compos_;\
    driver->configureVertexAttribute(va); }

    const ProgramObject::ProgramResources& vpres = ctx->getCurrentVertexProgram().getResources();

    GLuint stream = 0;

#ifdef GLLIB_DUMP_STREAMS
    ofstream out;
    
    out.open("GLLibDumpStreams.txt");

    if ( !out.is_open() )
        panic("GPULib", "glEnd", "Dump failed (output file could not be opened)");
#endif

    if ( vpres.inputAttribsRead[GPUDriver::VS_VERTEX] )
    {
        mdVertex = afl::sendDB(ctx->posbuf(), driver);
        CONFIG_ATTRIB(stream, mdVertex, GPUDriver::VS_VERTEX, ctx->posbuf().numberOfComponents());
        stream++;

#ifdef GLLIB_DUMP_STREAMS
        out << " Dumping Stream " << va.stream << endl;
        out << "\t Stride: " << va.stride << endl;
        out << "\t Data type: " << va.componentsType << endl;
        out << "\t Num. elemets: " << va.components << endl;
        out << "\t Offset: " << va.offset << endl;
        out << "\t Dumping stream content: " << endl;
        ctx->posbuf().dump(out);
        out << endl;
#endif
    }

    if ( vpres.inputAttribsRead[GPUDriver::VS_COLOR] )
    {
        mdColor = afl::sendDB(ctx->colbuf(), driver);
        CONFIG_ATTRIB(stream, mdColor, GPUDriver::VS_COLOR, ctx->colbuf().numberOfComponents());
        stream++;

#ifdef GLLIB_DUMP_STREAMS
        if (ctx->getTextureUnit(0).getTarget() == 0)
        {
            out << " Dumping Stream " << va.stream << endl;
            out << "\t Stride: " << va.stride << endl;
            out << "\t Data type: " << va.componentsType << endl;
            out << "\t Num. elemets: " << va.components << endl;
            out << "\t Offset: " << va.offset << endl;
            out << "\t Dumping stream content: " << endl;
            ctx->colbuf().dump(out);
            out << endl;
        }
#endif
    }

    if ( vpres.inputAttribsRead[GPUDriver::VS_NORMAL] )
    {
        mdNormal = afl::sendDB(ctx->norbuf(), driver);
        CONFIG_ATTRIB(stream, mdNormal, GPUDriver::VS_NORMAL, ctx->norbuf().numberOfComponents());
        stream++;

#ifdef GLLIB_DUMP_STREAMS
        out << " Dumping Stream " << va.stream << endl;
        out << "\t Stride: " << va.stride << endl;
        out << "\t Data type: " << va.componentsType << endl;
        out << "\t Num. elemets: " << va.components << endl;
        out << "\t Offset: " << va.offset << endl;
        out << "\t Dumping stream content: " << endl;
        ctx->norbuf().dump(out);
        out << endl;
#endif
    }

    if ( useTextures )
    {
        for ( GLuint i = 0; i < ctx->countTextureUnits(); i++ )
        {
            if ( ctx->getTextureUnit(i).getTarget() != 0 ) // Texture unit enabled, allocate tex coords into gpu
            {
                texUnit.push_back(i);
                mdTextures.push_back( afl::sendDB(ctx->texbuf(i), driver) );

#ifdef GLLIB_DUMP_STREAMS
                        out << " Dumping Stream " << va.stream << endl;
                        out << "\t Stride: " << va.stride << endl;
                        out << "\t Data type: " << va.componentsType << endl;
                        out << "\t Num. elemets: " << va.components << endl;
                        out << "\t Offset: " << va.offset << endl;
                        out << "\t Dumping stream content: " << endl;
                        ctx->texbuf(i).dump(out);
                        out << endl;
#endif
            }
        }

        for ( GLuint i = 0; i < texUnit.size();  i++, stream++ )
        {
            GPUDriver::ShAttrib tu = GPUDriver::ShAttrib(u32bit(GPUDriver::VS_TEXTURE_0) + texUnit[i]);
            u32bit md = mdTextures[i];
            CONFIG_ATTRIB(stream, md, tu, ctx->texbuf().numberOfComponents());
            stream++;

#ifdef GLLIB_DUMP_STREAMS
            out << " Dumping Stream " << va.stream << endl;
            out << "\t Stride: " << va.stride << endl;
            out << "\t Data type: " << va.componentsType << endl;
            out << "\t Num. elemets: " << va.components << endl;
            out << "\t Offset: " << va.offset << endl;
            out << "\t Dumping stream content: " << endl;
            ctx->texbuf().dump(out);
            out << endl;
#endif
        }
    }

#undef CONFIG_ATTRIB

    /*****************************************
     * Configure streaming mode = sequential *
     *****************************************/
    driver->setSequentialStreamingMode(ctx->posbuf().vectors(), 0); /* 0 means do not skip any vertex */

    /**********************************************************************************
     *              CopyTex{Sub}Image patch used for Chronicles of Riddick:           *
     *                 Until this call is fully supported a batch that                *
     *                 uses a Texture based on FrameBuffer contents is                *
     *                 ignored.                                                       *
     **********************************************************************************/

    if (disableColorMask)
    {
        ctx->pushAttrib(GL_COLOR_BUFFER_BIT);

        GPURegData data;
        data.booleanVal = false;
        driver->writeGPURegister(GPU_COLOR_MASK_R, data);
        driver->writeGPURegister(GPU_COLOR_MASK_G, data);
        driver->writeGPURegister(GPU_COLOR_MASK_B, data);
        driver->writeGPURegister(GPU_COLOR_MASK_A, data);
    }

    /*********************
     * SEND DRAW COMMAND *
     *********************/
    driver->sendCommand( GPU_DRAW );

    if (disableColorMask)
    {
        ctx->popAttrib();
    }

    driver->releaseMemory(mdVertex);
    driver->releaseMemory(mdColor);
    driver->releaseMemory(mdNormal);

    if ( useTextures )
    {
        vector<u32bit>::iterator it = mdTextures.begin();
        for ( ; it != mdTextures.end() ; it++ )
            driver->releaseMemory(*it);
    }

}

GLAPI void GLAPIENTRY glFlush( void )
{
    PRINT_CALL("glFlush();" << endl; )

    CHECK_NOT_BEGIN_MODE(glFlush);

    // empty ...
}


GLAPI void GLAPIENTRY glEnableClientState( GLenum cap )
{
    PRINT_CALL("glEnableClientState(" << cap << ");" << endl; )

    switch ( cap )
    {
        case GL_VERTEX_ARRAY:
            ctx->setFlags(GLContext::flagVaPos);
            break;
        case GL_COLOR_ARRAY:
            ctx->setFlags(GLContext::flagVaCol);
            break;
        case GL_NORMAL_ARRAY:
            ctx->setFlags(GLContext::flagVaNor);
            break;
        case GL_INDEX_ARRAY:
            ctx->setFlags(GLContext::flagVaInd);
            break;
        case GL_TEXTURE_COORD_ARRAY:
            //ctx->setFlags(GLContext::flagVaTex); // should have one bit per texture unit
            ctx->setEnabledCurrentClientTextureUnit(true);
            break;
        case GL_EDGE_FLAG_ARRAY:
            ctx->setFlags(GLContext::flagVaEdg);
            break;
        default:
            panic("GPULib", "glEnableClientState", "Unexpected client state");
    }
}

GLAPI void GLAPIENTRY glDrawArrays( GLenum mode, GLint first, GLsizei count )
{
    PRINT_CALL("glDrawArrays(" << mode << ", " << first << ", " << count << ");" << endl; )

    if ( GL_POINTS <= mode && mode <= GL_POLYGON )
    {
        GPURegData data;
        data.primitive = afl::translatePrimitive(mode);
        driver->writeGPURegister(GPU_PRIMITIVE, data);
    }
    else
        panic("GPULib", "glDrawArrays()", "Unknown primitive type");

    /**
     * This two macros set and check draw mode, if the primitive (mode) is not supported, then
     * the call is skipped
     */
    SET_BATCH_MODE_SUPPORT(mode);
    CHECK_BATCH_MODE_SUPPORT_MSG("Warning (drawArrays). Primitive mode not supported. Batch ignored\n");

#ifdef GLLIB_DUMP_STREAMS
    GLubyte filename[30];
    sprintf((char*)filename, "GLLibDumpStreams.txt");
    remove((char*)filename);
#endif

#ifdef GLLIB_DUMP_SAMPLERS
    for (GLuint i = 0; i < ProgramObject::MaxTextureUnits; i++)
        for (GLuint j = 0; j < 6; j++)
            for (GLuint k = 0; k < 13; k++)
            {
                sprintf((char*)filename, "Sampler%d_Face%d_Mipmap%d.ppm", i,j,k);
                remove((char*)filename);
            }
#endif

    bool disableColorMask = false;

    /*********** ****************************
     * SYNCHRONIZE TEXTURE DATA IF REQUIRED *
     ****************************************/
    if ( ctx->areTexturesEnabled() )
        afl::synchronizeTextureImages(ctx, driver, disableColorMask);

    /***********************************
     * CONFIGURE & COPY STREAM BUFFERS *
     ***********************************/
    std::set<GLuint> iSet; // empty means sequential access
    GLuint nextFreeStream; // dummy
    vector<GLuint> descs = afl::setVertexArraysBuffers(ctx, driver, first, count, iSet, nextFreeStream);

    /**********************************
     * SET SEQUENTIAL STREAMING MODE  *
     **********************************/
    driver->setSequentialStreamingMode(count, 0); /* 0 means do not skip any vertex */

    /************************
     * LOAD SHADER PROGRAMS *
     ************************/
    afl::setupShaders(ctx, driver);

    /*  Setup Hierarchical Z Test.  */
    afl::setupHZTest(ctx, driver);

    /**********************************************************************************
     *              CopyTex{Sub}Image patch used for Chronicles of Riddick:           *
     *                 Until this call is fully supported a batch that                *
     *                 uses a Texture based on FrameBuffer contents is                *
     *                 ignored.                                                       *
     **********************************************************************************/
    if (disableColorMask)
    {
        ctx->pushAttrib(GL_COLOR_BUFFER_BIT);

        GPURegData data;
        data.booleanVal = false;
        driver->writeGPURegister(GPU_COLOR_MASK_R, data);
        driver->writeGPURegister(GPU_COLOR_MASK_G, data);
        driver->writeGPURegister(GPU_COLOR_MASK_B, data);
        driver->writeGPURegister(GPU_COLOR_MASK_A, data);
    }

    /*********************
     * SEND DRAW COMMAND *
     *********************/
    driver->sendCommand( GPU_DRAW );

    if (disableColorMask)
    {
        ctx->popAttrib();
    }

    /*********************************
     * RELEASE STREAM BUFFERS MEMORY *
     *********************************/
    vector<unsigned int>::iterator it = descs.begin();
    for ( ; it != descs.end(); it++ )
        driver->releaseMemory(*it);


}

GLAPI void GLAPIENTRY glDisableClientState( GLenum cap )
{
    PRINT_CALL("glDisableClientState(" << cap << ");" << endl; )

    switch ( cap ) {
        case GL_VERTEX_ARRAY:
            ctx->resetFlags(GLContext::flagVaPos);
            break;
        case GL_COLOR_ARRAY:
            ctx->resetFlags(GLContext::flagVaCol);
            break;
        case GL_NORMAL_ARRAY:
            ctx->resetFlags(GLContext::flagVaNor);
            break;
        case GL_INDEX_ARRAY:
            ctx->resetFlags(GLContext::flagVaInd);
            break;
        case GL_TEXTURE_COORD_ARRAY:
            //ctx->resetFlags(GLContext::flagVaTex);
            ctx->setEnabledCurrentClientTextureUnit(false);
            break;
        case GL_EDGE_FLAG_ARRAY:
            ctx->resetFlags(GLContext::flagVaEdg);
            break;
        default:
            panic("GPULib", "glDisableClientState()", "State not supported");
    }
}

GLAPI void GLAPIENTRY glEnableVertexAttribArrayARB (GLuint index)
{
    PRINT_CALL("glEnableVertexAttribArrayARB(" << index << ");" << endl);

    ctx->setEnabledGenericAttribArray(index, true);
}

GLAPI void GLAPIENTRY glDisableVertexAttribArrayARB (GLuint index)
{
    PRINT_CALL("glDisableVertexAttribArrayARB(" << index << ");" << endl);
    ctx->setEnabledGenericAttribArray(index, false);
}


// generic vertex arrays
GLAPI void GLAPIENTRY glVertexAttribPointerARB(GLuint index, GLint size, GLenum type,
                                                GLboolean normalized, GLsizei stride, const GLvoid *pointer)
{
    PRINT_CALL("glVertexAttribPointerARB(" << index << "," << size << ", " << type << ", " << normalized << ","
                                           << stride << "," << pointer << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glVertexAttribPointerARB);

    //if ( normalized )
        //panic("GPULib", "glVertexAttribPointerARB", "Normalization not supported yet");

    ctx->genericVArray(index) = GLContext::VArray(size, type, stride, pointer, normalized);

    if ( IS_BUFFER_BOUND(GL_ARRAY_BUFFER) )
        ctx->genericVArray(index).bufferID = GET_ARRAY_BUFFER.getName();
}

// vertex arrays
GLAPI void GLAPIENTRY glVertexPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr )
{
    PRINT_CALL("glVertexPointer(" << size << ", " << type << ", " << stride << ", " << ptr << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glVertexPointer);

    ctx->posVArray() = GLContext::VArray(size, type, stride, ptr);

    if ( IS_BUFFER_BOUND(GL_ARRAY_BUFFER) )
        ctx->posVArray().bufferID = GET_ARRAY_BUFFER.getName();
}

GLAPI void GLAPIENTRY glColorPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr )
{
    PRINT_CALL("glColorPointer(" << size << ", " << type << ", " << stride << ", " << ptr << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glColorPointer);

    ctx->colVArray() = GLContext::VArray(size, type, stride, ptr);

    if ( IS_BUFFER_BOUND(GL_ARRAY_BUFFER) )
        ctx->colVArray().bufferID = GET_ARRAY_BUFFER.getName();
}

GLAPI void GLAPIENTRY glNormalPointer( GLenum type, GLsizei stride, const GLvoid *ptr )
{
    PRINT_CALL("glNormalPointer(" << type << ", " << stride << ", " << ptr << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glNormalPointer);

    ctx->norVArray() = GLContext::VArray(3, type, stride, ptr);

    if ( IS_BUFFER_BOUND(GL_ARRAY_BUFFER) )
        ctx->norVArray().bufferID = GET_ARRAY_BUFFER.getName();
}

GLAPI void GLAPIENTRY glIndexPointer( GLenum type, GLsizei stride, const GLvoid *ptr )
{
    PRINT_CALL("glIndexPointer(" << type << ", " << stride << ", " << ptr << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glIndexPointer);

    ctx->indVArray() = GLContext::VArray(1, type, stride, ptr);

    if ( IS_BUFFER_BOUND(GL_ARRAY_BUFFER) )
        ctx->indVArray().bufferID = GET_ARRAY_BUFFER.getName();
}

GLAPI void GLAPIENTRY glTexCoordPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr )
{
    PRINT_CALL("glTexCoordPointer(" << size << ", " << type << ", " << stride << ", " << ptr << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glTexCoordPointer);

    GLuint clientTUnit = ctx->getCurrentClientTextureUnit();

    ctx->texVArray(clientTUnit) = GLContext::VArray(size, type, stride, ptr);

    if ( IS_BUFFER_BOUND(GL_ARRAY_BUFFER) )
        ctx->texVArray(clientTUnit).bufferID = GET_ARRAY_BUFFER.getName();
}

GLAPI void GLAPIENTRY glEdgeFlagPointer( GLsizei stride, const GLvoid *ptr )
{
    PRINT_CALL("glEdgeFlagPointer(" << stride << ", " << ptr << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glTexCoordPointer);

    ctx->edgVArray() = GLContext::VArray(1,GL_UNSIGNED_BYTE,stride, ptr);

    if ( IS_BUFFER_BOUND(GL_ARRAY_BUFFER) )
        ctx->edgVArray().bufferID = GET_ARRAY_BUFFER.getName();
}


GLAPI void GLAPIENTRY glArrayElement( GLint i )
{
    PRINT_CALL("glArrayElement" << i << ");" << endl; )

    panic("GPULib","glArrayElement()","Function not implemented :-)");

    // called inside glBegin/End
    // uses glBegin/glEnd buffers
}


GLAPI void GLAPIENTRY glInterleavedArrays( GLenum format, GLsizei stride, const GLvoid *pointer )
{

    PRINT_CALL("glInterleavedArrays(" << format << ", " << stride << ", " << pointer << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glInterleavedArrays);

    switch ( format )
    {
        case GL_C4F_N3F_V3F:
            // enable vertex arrays
            ctx->setFlags(GLContext::flagVaPos | GLContext::flagVaCol | GLContext::flagVaNor);

            if (stride == 0)
                stride = 10*sizeof(GLfloat); // See OpenGL 2.0 Specification section 2.8 Vertex Arrays at pages 31 and 32

            ctx->colVArray() = GLContext::VArray(4, GL_FLOAT, stride, pointer);
            ctx->norVArray() = GLContext::VArray(3, GL_FLOAT, stride, ((GLubyte *)pointer) + 4*sizeof(GLfloat));
            ctx->posVArray() = GLContext::VArray(3, GL_FLOAT, stride, ((GLubyte *)pointer) + 7*sizeof(GLfloat));
            break;
        default:
            panic("GPULib", "glInterleavedArrays()", "Format not yet unsupported");
    }
}


GLAPI void GLAPIENTRY glDrawRangeElements( GLenum mode, GLuint start, GLuint end, GLsizei count,
                                           GLenum type, const GLvoid *indices )
{

    PRINT_CALL("glDrawRangeElements(0x" << hex << mode << ", " << dec << start << ", " << end << "," << count << ",0x"
                                        << hex << type << dec << ", " << GLuint(indices) << ");" << endl; )

    // start & end ignored
    afl::drawElements(ctx, driver, mode, count, type, indices);
}


GLAPI void GLAPIENTRY glDrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices )
{
    PRINT_CALL("glDrawElements(0x" << hex << mode << dec << ", " << count << ", " << type << ", " << GLuint(indices) << ");" << endl; )

    //  Hack for Riddick.
    //ignoreClearAfterTexSubImage = false;

    afl::drawElements(ctx, driver, mode, count, type, indices);
}

GLAPI void GLAPIENTRY glLoadMatrixf( const GLfloat *m )
{
    PRINT_CALL("glLoadMatrixf" << m << ");" << endl; )

    Matrixf mat(m,true);
    ctx->ctop(mat);

}

GLAPI void GLAPIENTRY glMultMatrixd( const GLdouble *m )
{
    PRINT_CALL("glMultMatrixd(" << m << ");" << endl; )

    Matrixf mat(m,true); /* create a matrix (representation in m is in columns) */
    ctx->ctop(ctx->mtop() * mat); /* multiply current top matrix by 'mat' */
}

GLAPI void GLAPIENTRY glMultMatrixf( const GLfloat *m )
{
    PRINT_CALL("glMultMatrixf(" << m << ");" << endl; )

    Matrixf mat(m,true);
    ctx->ctop(ctx->mtop() * mat);

}

GLAPI void GLAPIENTRY glTranslated( GLdouble x, GLdouble y, GLdouble z )
{
    PRINT_CALL("glTranslated(" << x << ", " << y << ", " << z << ");" << endl; )

    Matrixf current = ctx->mtop().transpose(); /* current in columns */
    mathlib::_translate_matrix(current.getRows(), GLfloat(x), GLfloat(y), GLfloat(z)); // translate
    ctx->ctop(current.transpose()); /* current translate again in rows */

}

GLAPI void GLAPIENTRY glTranslatef( GLfloat x, GLfloat y, GLfloat z )
{
    PRINT_CALL("glTranslatef(" << x << ", " << y << ", " << z << ");" << endl; )

    Matrixf current = ctx->mtop().transpose(); /* current in columns */
    mathlib::_translate_matrix(current.getRows(), x, y, z); // translate
    ctx->ctop(current.transpose()); /* current translate again in rows */
}

GLAPI void GLAPIENTRY glRotated( GLdouble angle, GLdouble x, GLdouble y, GLdouble z )
{
    PRINT_CALL("glRotated(" << angle << ", " << x << ", " << y << ", " << z << ");" << endl; )

    Matrixf current = ctx->mtop().transpose(); /* current in columns */
    mathlib::_rotate_matrix(current.getRows(), (GLfloat)angle, (GLfloat)x, (GLfloat)y, (GLfloat)z);
    ctx->ctop(current.transpose());
}

GLAPI void GLAPIENTRY glRotatef( GLfloat angle, GLfloat x, GLfloat y, GLfloat z )
{
    PRINT_CALL("glRotatef(" << angle << ", " << x << ", " << y << ", " << z << ");" << endl; )

    Matrixf current = ctx->mtop().transpose(); /* current in columns */
    mathlib::_rotate_matrix(current.getRows(), angle, x, y, z);
    ctx->ctop(current.transpose());
}

GLAPI void GLAPIENTRY glScalef( GLfloat x, GLfloat y, GLfloat z )
{
    PRINT_CALL("glScalef(" << x << ", " << y << ", " << z << ");" << endl; )

    Matrixf current = ctx->mtop().transpose(); /* current in columns */
    mathlib::_scale_matrix(current.getRows(), x, y, z);
    ctx->ctop(current.transpose());
}

GLAPI void GLAPIENTRY glEnable( GLenum cap )
{
    PRINT_CALL("glEnable(" << std::hex << "0x" << cap << std::dec << ");" << endl; )

    GPURegData data;

    switch ( cap )
    {
        case GL_DEPTH_TEST:
            /* Enable Z TEST */
            data.booleanVal = true;
            driver->writeGPURegister(GPU_DEPTH_TEST, data);

            //data.booleanVal = true;
            //driver->writeGPURegister(GPU_DEPTH_MASK, data);
            //data.uintVal = GPU_LESS;
            //driver->writeGPURegister(GPU_DEPTH_FUNCTION,data);

            ctx->testAndSetFlags(GLContext::flagDepthTest);
            break;
        case GL_VERTEX_PROGRAM_ARB:
            ctx->testAndSetFlags(GLContext::flagVS);
            break;
        case GL_FRAGMENT_PROGRAM_ARB:
            ctx->testAndSetFlags(GLContext::flagFS);
            break;
        case GL_LIGHTING:
            ctx->testAndSetFlags(GLContext::flagLighting);
            break;
        case GL_LIGHT0:
        case GL_LIGHT1:
        case GL_LIGHT2:
        case GL_LIGHT3:
        case GL_LIGHT4:
        case GL_LIGHT5:
        case GL_LIGHT6:
        case GL_LIGHT7:
            ctx->enableLight(cap- GL_LIGHT0);
            break;
        case GL_CULL_FACE:
            ctx->testAndSetFlags(GLContext::flagCullFace);

#define CASE_BRANCH(x)\
        case GL_##x:\
            data.culling = x;\
            break;\

            switch(ctx->getCullFace())
            {
                CASE_BRANCH(FRONT)
                CASE_BRANCH(BACK)
                CASE_BRANCH(FRONT_AND_BACK)
            }
#undef CASE_BRANCH

            driver->writeGPURegister(GPU_CULLING, data);
            break;
        case GL_TEXTURE_1D:
        case GL_TEXTURE_2D:
        case GL_TEXTURE_3D:
        case GL_TEXTURE_CUBE_MAP:
            {
                TextureUnit& texUnit = ctx->getActiveTextureUnit();
                TextureObject& to = GET_TEXTURE(cap);
                GLenum old = texUnit.getTarget();
                texUnit.enableTarget(cap); // enable target in the active texture unit
                texUnit.setTextureObject(cap, &to);
                if ( texUnit.getTarget() != old )
                    SET_LIB_DIRTY; // new target selected, requires to rebuilt shaders
            }
            break;
        case GL_TEXTURE_GEN_S:
        case GL_TEXTURE_GEN_T:
        case GL_TEXTURE_GEN_R:
        case GL_TEXTURE_GEN_Q:
            {
                TextureUnit& texUnit = ctx->getActiveTextureUnit();
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
                {
                    texUnit.setEnabledTexGen(select, true);
                    SET_LIB_DIRTY;
                }
            }
            break;
        case GL_FOG:
            ctx->testAndSetFlags(GLContext::flagFog);
            break;
        case GL_NORMALIZE:
            ctx->testAndSetFlags(GLContext::flagNormalize);
            break;
        case GL_STENCIL_TEST:
            ctx->setFlags(GLContext::flagStencilTest); // maybe not required in GLContext
            data.booleanVal = true;
            driver->writeGPURegister(GPU_STENCIL_TEST, data);
            break;
        case GL_ALPHA_TEST:
            ctx->testAndSetFlags(GLContext::flagAlphaTest);
            //{
            //    GPURegData data;
            //    data.booleanVal = false;
            //    driver->writeGPURegister(GPU_EARLYZ, data);
            //}
            break;
        case GL_BLEND:
            {
                ctx->testAndSetFlags(GLContext::flagBlending);
                GPURegData data;
                data.booleanVal = true;
                driver->writeGPURegister(GPU_COLOR_BLEND, data);
            }
            break;
        case GL_COLOR_MATERIAL:
            ctx->setFlags(GLContext::flagColorMaterial);
            break;
        case GL_SCISSOR_TEST:
            {
                GPURegData data;
                data.booleanVal = true;
                driver->writeGPURegister(GPU_SCISSOR_TEST, data);
                ctx->setFlags(GLContext::flagScissorTest);
            }
            break;
        case GL_POLYGON_OFFSET_FILL:
            {
                ctx->setFlags(GLContext::flagPolygonOffsetFill);

                GLfloat factor, units;

                ctx->getPolygonOffset(factor, units);

                if ( factor != 0.0f || units != 0.0f )
                {
                    GPURegData data;

                    data.f32Val = factor;
                    driver->writeGPURegister( GPU_DEPTH_SLOPE_FACTOR, data );
                    data.f32Val = units;
                    driver->writeGPURegister( GPU_DEPTH_UNIT_OFFSET, data );

                }
            }
            break;
        default:
        {
            char msg[64];
            sprintf(msg, "glEnable. Unsupported constant ( 0x%X )", cap);
            //panic("GPULib", "glEnable()", msg);
            popup("GPULib", msg);

        }
    }
}

GLAPI void GLAPIENTRY glDisable( GLenum cap )
{
    PRINT_CALL("glDisable(" << cap << ");" << endl; )

    GPURegData data;

    switch ( cap )
    {
        case GL_DEPTH_TEST:
            /* Disable Z TEST */
            data.booleanVal = false;
            driver->writeGPURegister(GPU_DEPTH_TEST, data);
            ctx->testAndResetFlags(GLContext::flagDepthTest);
            break;
        case GL_VERTEX_PROGRAM_ARB:
            ctx->testAndResetFlags(GLContext::flagVS);
            break;
        case GL_FRAGMENT_PROGRAM_ARB:
            ctx->testAndResetFlags(GLContext::flagFS);
            break;
        case GL_LIGHTING:
            ctx->testAndResetFlags(GLContext::flagLighting);
            break;
        case GL_LIGHT0: case GL_LIGHT1: case GL_LIGHT2:
        case GL_LIGHT3: case GL_LIGHT4: case GL_LIGHT5:
        case GL_LIGHT6: case GL_LIGHT7:
            ctx->disableLight(cap - GL_LIGHT0);
            break;
        case GL_CULL_FACE:
            ctx->testAndResetFlags(GLContext::flagCullFace);
            data.culling = NONE;
            driver->writeGPURegister(GPU_CULLING, data);
            break;
        case GL_TEXTURE_1D:
        case GL_TEXTURE_2D:
        case GL_TEXTURE_3D:
        case GL_TEXTURE_CUBE_MAP:
            {
                GLuint tu = TextureManager::instance().getCurrentGroup(); // get active texture unit
                TextureUnit& texUnit = ctx->getTextureUnit(tu);
                GLenum old = texUnit.getTarget();
                texUnit.disableTarget(cap); // enable target in the active texture unit
                if ( texUnit.getTarget() != old )
                    SET_LIB_DIRTY;
            }
            break;
        case GL_TEXTURE_GEN_S:
        case GL_TEXTURE_GEN_T:
        case GL_TEXTURE_GEN_R:
        case GL_TEXTURE_GEN_Q:
            {
                TextureUnit& texUnit = ctx->getActiveTextureUnit();
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
                {
                    texUnit.setEnabledTexGen(select, false);
                    SET_LIB_DIRTY;
                }
            }
            break;
        case GL_FOG:
            ctx->testAndResetFlags(GLContext::flagFog);
            break;
        case GL_NORMALIZE:
            ctx->testAndResetFlags(GLContext::flagNormalize);
            break;
        case GL_STENCIL_TEST:
            ctx->resetFlags(GLContext::flagStencilTest); // maybe not required
            data.booleanVal = false;
            driver->writeGPURegister(GPU_STENCIL_TEST, data);
            break;
        case GL_ALPHA_TEST:
            ctx->testAndResetFlags(GLContext::flagAlphaTest);
            //{
            //    GPURegData data;
               //    data.booleanVal = true;
            //    driver->writeGPURegister(GPU_EARLYZ, data);
            //}
            break;
        case GL_BLEND:
            {
                ctx->testAndResetFlags(GLContext::flagBlending);
                GPURegData data;
                data.booleanVal = false;
                driver->writeGPURegister(GPU_COLOR_BLEND, data);
            }
            break;
        case GL_COLOR_MATERIAL:
            ctx->resetFlags(GLContext::flagColorMaterial);
            break;
        case GL_SCISSOR_TEST:
            {
                GPURegData data;
                data.booleanVal = false;
                driver->writeGPURegister(GPU_SCISSOR_TEST, data);
                ctx->resetFlags(GLContext::flagScissorTest);
            }
            break;
        case GL_POLYGON_OFFSET_FILL:
            {
                ctx->resetFlags(GLContext::flagPolygonOffsetFill);

                data.f32Val = 0.0f;
                driver->writeGPURegister( GPU_DEPTH_SLOPE_FACTOR, data );
                data.f32Val = 0.0f;
                driver->writeGPURegister( GPU_DEPTH_UNIT_OFFSET, data );
            }
            break;
        default:
            char msg[64];
            sprintf(msg, "glDisable. Unsupported constant ( 0x%X )", cap);
            //cout << "Ignoring panic. " << msg << endl;
            //panic("GPULib", "glDisable()", "Unsupported constant");
            popup("GPULib", msg);
    }
}

GLAPI void GLAPIENTRY glBindProgramARB (GLenum target, GLuint pid)
{
    PRINT_CALL("glBindProgramARB(" << std::hex << "0x" << target << std::dec << ", " << pid << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glBindProgramARB);

    ProgramManager& pm = ProgramManager::instance(); // Get the program manager

    if ( pid == 0 ) // binding the default (it is not a TextuereObject)
    {
        ProgramTarget& tg = pm.getTarget(target);
        tg.setDefaultAsCurrent(); // set the default as current
    }
    else
    {
        // Bind the program object
        pm.bindObject(target, pid); // get (or create) the program object
    }
}


GLAPI const GLubyte* GLAPIENTRY glGetString( GLenum name )
{
    PRINT_CALL("glGetString(" << name << ");" << endl; )

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

    panic("GPULib","glGetString()","Unknown constant");
    return 0; /* avoid compile warnings */
}


GLAPI void GLAPIENTRY glProgramEnvParameter4fvARB (GLenum target, GLuint index, const GLfloat *params)
{
    PRINT_CALL("glProgramEnvParameter4fvARB(0x" << hex << target << "," << index << ",{" << params[0] << ","
                << params[1] << "," << params[2] << "," << params[3] <<"});" << endl; )

    RBank<float>& globals = ProgramManager::instance().getTarget(target).getEnvironmentParameters();
    globals[index] = QuadReg<float>(params[0], params[1], params[2], params[3]);
}

GLAPI void GLAPIENTRY glProgramLocalParameter4fvARB (GLenum target, GLuint index, const GLfloat *v)
{
    PRINT_CALL("glProgramLocalParameter4fvARB(" << target << ", " << index << ", " << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << ");" << endl; )

    RBank<float>& locals = ProgramManager::instance().getTarget(target).getCurrent().getLocalParams();
    locals[index] = QuadReg<float>(v[0], v[1], v[2], v[3]);
}

GLAPI void GLAPIENTRY glProgramLocalParameter4fARB (GLenum target, GLuint index,
                                                  GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    PRINT_CALL("glProgramLocalParameter4fARB(" << target << ", " << index << ", " << x << ", " << y << ", " << z << ", " << w << ");" << endl; )

    RBank<float>& locals = ProgramManager::instance().getTarget(target).getCurrent().getLocalParams();
    locals[index] = QuadReg<float>(x,y,z,w);
}


GLAPI void GLAPIENTRY glProgramStringARB (GLenum target, GLenum format,
                                        GLsizei len, const GLvoid * str)
{
    PRINT_CALL("glProgramStringARB(" << target << ", " << format << ", " << len << ", " << str << ");" << endl; )

    ProgramManager::instance().programString(target, format, len, str);
}

/* LIGHTS */

GLAPI void GLAPIENTRY glLightfv( GLenum light, GLenum pname, const GLfloat *params )
{
    PRINT_CALL("glLightfv(" << light << ", " << pname << ", {" << params[0] <<",...});" << endl;)
    ctx->setParamLight(light, pname, params);
}

GLAPI void GLAPIENTRY glMaterialfv( GLenum face, GLenum pname, const GLfloat *params )
{
    PRINT_CALL("glMaterialfv(" << face << ", " << pname << ", {" << params[0] <<",...});" << endl;)
    ctx->setMaterial(face, pname, params);
}


GLAPI void GLAPIENTRY glLightModelfv( GLenum pname, const GLfloat *params )
{
    PRINT_CALL("glLightModelfv(" << pname << ", {" << params[0] <<",...});" << endl;)
    ctx->setLightModel(pname, params);
}

GLAPI void GLAPIENTRY glColor4ub( GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha )
{
    PRINT_CALL("glColor4ub(" << GLuint(red) << ", " << GLuint(green) <<", " << GLuint(blue) << ", " << GLuint(alpha) << ");" << endl; )

    GLfloat r = GLfloat(red) / 0xFF;
    GLfloat g = GLfloat(green) / 0xFF;
    GLfloat b = GLfloat(blue) / 0xFF;
    GLfloat a = GLfloat(alpha) / 0xFF;
    ctx->setColor(r, g, b, a);
}


GLAPI void GLAPIENTRY glColor4f( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{
    PRINT_CALL("glColor4f(" << red << ", " << green <<", " << blue << ", " << alpha << ");" << endl;)

    ctx->setColor(red,green,blue,alpha);
}

GLAPI void GLAPIENTRY glColor4fv( const GLfloat *v )
{
    PRINT_CALL("glColor4fv({" << v[0] << "," << v[1] << "," << v[2] << "," << v[3] << "});" << endl; )

    ctx->setColor(v[0], v[1], v[2], v[3]);
}

GLAPI void GLAPIENTRY glMaterialf( GLenum face, GLenum pname, GLfloat param )
{
    PRINT_CALL("glMaterialf(" << face << ", " << pname << ", " << param << ");" << endl;)
    switch ( pname )
    {
        case GL_SHININESS:
            ctx->setMaterial(face, pname, &param);
            break;
        default:
            panic("GPULib","glMaterialf()","This call can be used to set the only single-valued material property : shininess");
    }
}

GLAPI void GLAPIENTRY glLightModelf( GLenum pname, GLfloat param )
{
    PRINT_CALL("glLightModelf(" << pname << ", " << param <<");" << endl;)

    switch ( pname )
    {
        case GL_LIGHT_MODEL_LOCAL_VIEWER:
        case GL_LIGHT_MODEL_TWO_SIDE:
            ctx->setLightModel(pname, &param);
            break;
        default:
            panic("GPULib","glLightModelf()","This call can be used to set only single-valued lightmodel characteristics");
    }
}

GLAPI void GLAPIENTRY glLightModeli( GLenum pname, GLint param )
{
    PRINT_CALL("glLightModeli(" << pname << ", " << param <<");" << endl;)

    switch ( pname )
    {
        case GL_LIGHT_MODEL_LOCAL_VIEWER:
        case GL_LIGHT_MODEL_TWO_SIDE:
        case GL_LIGHT_MODEL_COLOR_CONTROL:
            ctx->setLightModel(pname, &param);
            break;
        default:
            panic("GPULib","glLightModeli()","This call can be used to set only single-valued lightmodel characteristics");
    }
}

GLAPI void GLAPIENTRY glLightf( GLenum light, GLenum pname, GLfloat param )
{
    PRINT_CALL("glLightfv(" << light << ", " << pname << ", {" << param <<",...});" << endl;)
    switch ( pname )
    {
        case GL_SPOT_EXPONENT:
        case GL_SPOT_CUTOFF:
        case GL_CONSTANT_ATTENUATION:
        case GL_LINEAR_ATTENUATION:
        case GL_QUADRATIC_ATTENUATION:
            ctx->setParamLight(light, pname, &param);
            break;
        default:
            panic("GPULib","glLightf()","This call can be used to set only single-valued light characteristics");
    }
}

GLAPI void GLAPIENTRY glNormal3fv( const GLfloat *v )
{
    PRINT_CALL("glNormal3fv({" << v[0] << ", " << v[1] << ", " << v[2] << "});" << endl; )

    ctx->setNormal(v[0], v[1], v[2]);

    ENABLE_USENORMALS; // maybe it is not required
}

GLAPI void GLAPIENTRY glVertex3fv( const GLfloat *v )
{
    PRINT_CALL("glVertex3fv({" << v[0] << ", " << v[1] << ", " << v[2] << "});" << endl; )

    CHECK_BEGIN_MODE(glVertex3fv);

    afl::glVertex(ctx, v[0], v[1], v[2]);
}


GLAPI void GLAPIENTRY glDepthFunc( GLenum func )
{
    PRINT_CALL("glDepthFunc(" << func << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glDepthFunc);

    #define CASE_BRANCH(x)\
        case GL_##x:\
            data.compare = GPU_##x;\
            break;\

    //#define CASE_BRANCH_WARNING(x)\
    //    case GL_##x:\
    //        data.compare = GPU_LESS;\
    //        cout << "GPULib::glDepthFunc(). Warning: GL_" #x " does not work, using GL_LESS..." << endl;\
    //        break;\

    /*  Set the new depth function.  */
    ctx->setDepthFunc(func);

    GPURegData data;
    switch ( func )
    {
        CASE_BRANCH(NEVER)
        CASE_BRANCH(ALWAYS)
        CASE_BRANCH(LESS)
        CASE_BRANCH(LEQUAL)
        //CASE_BRANCH_WARNING(EQUAL)
        //CASE_BRANCH_WARNING(GEQUAL)
        //CASE_BRANCH_WARNING(GREATER)
        //CASE_BRANCH_WARNING(NOTEQUAL)
        CASE_BRANCH(EQUAL)
        CASE_BRANCH(GEQUAL)
        CASE_BRANCH(GREATER)
        CASE_BRANCH(NOTEQUAL)
        default:
            panic("GPULib", "glDepthFunc", "Unknown depth function");
    }

    #undef CASE_BRANCH

    driver->writeGPURegister(GPU_DEPTH_FUNCTION, data);
}

GLAPI void GLAPIENTRY glDepthMask( GLboolean flag )
{
    PRINT_CALL("glDepthMask(" << bool(flag) << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glDepthMask);

    if ( flag )
        ctx->setDepthMask(true);
    else
        ctx->setDepthMask(false);

    GPURegData data;

    data.booleanVal = (flag == 0 ? false : true);

    driver->writeGPURegister(GPU_DEPTH_MASK, data);
}

GLAPI void GLAPIENTRY glClearDepth( GLclampd depth )
{
    PRINT_CALL("glClearDepth(" << depth << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glClearDepth);

    depth = afl::clamp(depth);

    ctx->setDepthClearValue(depth);

    //  NOTE: This change must be deferred until the next clear for correct behaviour.

    //  The float point depth value in the 0 .. 1 range must be converted to an integer
    //  value of the selected depth bit precission.
    //
    //  WARNING: This function should depend on the current depth bit precission.  Should be
    //  configurable using the wgl functions that initialize the framebuffer.  By default
    //  should be 24 (stencil 8 depth 24).  Currently the simulator only supports 24.
    //

    //data.uintVal = u32bit((f32bit(depth) * f32bit((1 << 24) - 1)));
    //driver->writeGPURegister(GPU_Z_BUFFER_CLEAR, data);
}

GLAPI void GLAPIENTRY glCullFace( GLenum mode )
{
    PRINT_CALL("glCullFace(" << mode << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glCullFace);

    GPURegData data;

    ctx->setCullFace(mode);

    if (ctx->testFlags(GLContext::flagCullFace))
    {
        switch ( mode )
        {
            case GL_FRONT:
                data.culling = FRONT;
                break;
            case GL_BACK:
                data.culling = BACK;
                break;
            case GL_FRONT_AND_BACK:
                data.culling = FRONT_AND_BACK;
                break;
            default:
                panic("GPULib", "glCullFace()", "Unkown culling mode");
        }

        driver->writeGPURegister(GPU_CULLING, data);
    }
}

GLAPI void GLAPIENTRY glFrontFace( GLenum mode )
{
    PRINT_CALL("glFrontFace(" << mode << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glFrontFace);

    GPURegData data;

    switch ( mode )
    {
        case GL_CW:
            data.faceMode = GPU_CW;
            break;
        case GL_CCW:
            data.faceMode = GPU_CCW;
            break;
        default:
            panic("GPULib", "glFrontFace()", "Unkown front face mode");
    }

    ctx->setSetupShaderProgramUpdate(false);
    ctx->setFaceMode(mode);

    driver->writeGPURegister(GPU_FACEMODE, data);
}


GLAPI void GLAPIENTRY glPushMatrix( void )
{
    PRINT_CALL("glPushMatrix();" << endl; )

    ctx->mpush(ctx->mtop());
}

GLAPI void GLAPIENTRY glPopMatrix( void )
{
    PRINT_CALL("glPopMatrix();" << endl; )

    ctx->mpop();
}

GLAPI void GLAPIENTRY glActiveTextureARB(GLenum texture)
{
    PRINT_CALL("glActiveTextureARB(" << "0x" << hex << texture << dec << ");" << endl; )

    GLuint maxUnit = TextureManager::instance().numberOfGroups() - 1;
    GLuint group = texture - GL_TEXTURE0;
    if ( GL_TEXTURE0 <= texture && texture <= GL_TEXTURE0 + maxUnit )
        TextureManager::instance().selectGroup(group); // select the active group (active texture unit)
    else
    {
        char msg[256];
        sprintf(msg, "TextureUnit %d does not exist", group);
        panic("GPULib", "glActiveTextureARB", msg);
    }
}

GLAPI void GLAPIENTRY glBindTexture( GLenum target, GLuint texture )
{
    PRINT_CALL("glBindTexture(" << target << ", " << texture << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glBindTexture);

    TextureManager& tm = TextureManager::instance(); // Get the texture manager

    GLuint tu = tm.getCurrentGroup(); // get id of current texture unit

    if ( texture == 0 ) // binding the default (it is not a TextuereObject)
    {
        TextureTarget& tg = tm.getTarget(target);
        tg.setDefaultAsCurrent(); // set the default as current
        ctx->getTextureUnit(tu).setTextureObject(target, &tg.getCurrent());

    }
    else
    {
        // Find the texture object
        TextureObject& to = tm.bindObject(target, texture); // get (or create) the texture object
        // Set the object as the current for the texture unit
        ctx->getTextureUnit(tu).setTextureObject(target, &to);
    }
}

GLAPI void GLAPIENTRY glDeleteTextures(GLsizei n, const GLuint *textures)
{
    PRINT_CALL("glDeleteTextures(" << n << "," << std::hex << GLuint(textures) << std::dec << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glDeleteBuffersARB);

    afl::releaseObjects(TextureManager::instance(), n, textures);
}

GLAPI void GLAPIENTRY glTexParameteri( GLenum target, GLenum pname, GLint param )
{

    PRINT_CALL("glTexParameteri(" << target << ", " << pname << "," << param <<");" << endl; )

    GET_TEXTURE(target).setParameter(pname, &param);
}

GLAPI void GLAPIENTRY glTexParameterf( GLenum target, GLenum pname, GLfloat param )
{
    PRINT_CALL("glTexParameterf(" << target << ", " << pname << "," << param <<");" << endl; )

    GET_TEXTURE(target).setParameter(pname, &param);
}


GLAPI void GLAPIENTRY glPixelStorei( GLenum pname, GLint param )
{
    PRINT_CALL("glPixelStorei(" << pname << ", " << param << "); # call ignored" << endl; )

}


GLAPI void GLAPIENTRY glCompressedTexImage2DARB( GLenum target, GLint level,
                                                 GLenum internalFormat,
                                                 GLsizei width, GLsizei height,
                                                 GLint border, GLsizei imageSize,
                                                 const GLvoid *data )
{
    CHECK_NOT_BEGIN_MODE(glCompressedTexImage2DARB);

    PRINT_CALL("glCompressedTexImage2DARB(" << target << "," << level << "," << afl::strIFMT(internalFormat) << "," << width << ","
                               << height << "," << border << "," << imageSize << ","
                               << GLuint(data) << ");" << endl; )

    afl::compressedTexImage2D(target, level, internalFormat, width, height, border, imageSize, data);

}

GLAPI void GLAPIENTRY glCompressedTexImage2D( GLenum target, GLint level,
                                              GLenum internalFormat,
                                              GLsizei width, GLsizei height,
                                              GLint border, GLsizei imageSize,
                                              const GLvoid *data )
{
    CHECK_NOT_BEGIN_MODE(glCompressedTexImage2D);

    PRINT_CALL("glCompressedTexImage2D(" << target << "," << level << "," << afl::strIFMT(internalFormat) << "," << width << ","
                               << height << "," << border << "," << imageSize << ","
                               << GLuint(data) << ");" << endl; )

    afl::compressedTexImage2D(target, level, internalFormat, width, height, border, imageSize, data);
}

GLAPI void GLAPIENTRY glTexImage2D( GLenum target, GLint level,
                                    GLint internalFormat,
                                    GLsizei width, GLsizei height,
                                    GLint border, GLenum format, GLenum type,
                                    const GLvoid *pixels )
{

    CHECK_NOT_BEGIN_MODE(glTexImage2D);

    PRINT_CALL("glTexImage2D(" << target << "," << level << "," << afl::strIFMT(internalFormat) << "," << width << ","
                               << height << "," << border << "," << format << "," << type << ","
                               << GLuint(pixels) << ");" << endl; )


    GLenum tg; // All cube map faces have GL_TEXTURE_CUBE_MAP as target
    switch ( target )
    {
        case GL_TEXTURE_2D:
            tg = GL_TEXTURE_2D;
            break;
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            tg = GL_TEXTURE_CUBE_MAP;
            break;
        default:
            panic("GPULib", "glTexImage2D", "only TEXTURE_2D or CUBEMAP faces are allowed as target");
    }

    // Get current texture object
    TextureObject& to = GET_TEXTURE(tg);
    // target == CUBE_MAP face or GL_TEXTURE_2D

    GLuint id = to.getName();

    if ( internalFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT )
    {
        WARNING_CODE
        (
        cout << "-----------------------------------------------------------------------------" << endl;
        cout << "Warning: glTexImage2D: Internal format COMPRESSED_RGB_S3TC_DXT1 not supported" << endl;
        cout << "Texture object " << id << " uses uncompressed GL_RGB" << endl;
        cout << "-----------------------------------------------------------------------------" << endl;
        )
        internalFormat = GL_RGB;
        //return ;
    }
    else if ( internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT )
    {
        WARNING_CODE
        (
        cout << "-----------------------------------------------------------------------------" << endl;
        cout << "Warning: glTexImage2D: Internal format GL_COMPRESSED_RGBA_S3TC_DXT1_EXT not supported" << endl;
        cout << "Texture object " << id << " uses uncompressed GL_RGBA" << endl;
        cout << "-----------------------------------------------------------------------------" << endl;
        )
        internalFormat = GL_RGBA;
        //return ;
    }
    else if ( internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT )
    {
        WARNING_CODE
        (
        cout << "-----------------------------------------------------------------------------" << endl;
        cout << "Warning: glTexImage2D: Internal format GL_COMPRESSED_RGBA_S3TC_DXT3_EXT not supported" << endl;
        cout << "Texture object " << id << " uses uncompressed GL_RGBA" << endl;
        cout << "-----------------------------------------------------------------------------" << endl;
        )
        internalFormat = GL_RGBA;
        //return ;
    }
    else if ( internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT )
    {
        WARNING_CODE
        (
        cout << "-----------------------------------------------------------------------------" << endl;
        cout << "Warning: glTexImage2D: Internal format GL_COMPRESSED_RGBA_S3TC_DXT5_EXT not supported" << endl;
        cout << "Texture object " << id << " uses uncompressed GL_RGBA" << endl;
        cout << "-----------------------------------------------------------------------------" << endl;
        )
        internalFormat = GL_RGBA;
        //return ;
    }

    to.setContents(target, level, internalFormat, width, height, 1, border, format, type, (const GLubyte*)pixels);
}

GLAPI void GLAPIENTRY glTexSubImage2D( GLenum target, GLint level,
                                       GLint xoffset, GLint yoffset,
                                       GLsizei width, GLsizei height,
                                       GLenum format, GLenum type,
                                       const GLvoid *pixels )
{
    CHECK_NOT_BEGIN_MODE(glTexSubImage2D);

    PRINT_CALL("glTexSubImage2D(" << target << "," << level << "," << xoffset << "," << yoffset << "," << width << ","
                                  << height << "," << format << "," << type << "," << GLuint(pixels) << ");" << endl )

    GLenum tg;
    switch ( target )
    {
        case GL_TEXTURE_2D:
            tg = GL_TEXTURE_2D;
            break;
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            tg = GL_TEXTURE_CUBE_MAP;
            break;
        default:
            panic("GPULib", "glTexSubImage2D", "only TEXTURE_2D or CUBEMAP faces are allowed as target");
    }

    TextureObject& to = GET_TEXTURE(tg);

    GLuint id = to.getName();

    to.setPartialContents(target, level, xoffset, yoffset, 0, width, height, 0, format, type, (const GLubyte*)pixels);

}

GLAPI void GLAPIENTRY glCopyTexImage2D( GLenum target, GLint level,
                                        GLenum internalformat,
                                        GLint x, GLint y,
                                        GLsizei width, GLsizei height,
                                        GLint border )
{
    CHECK_NOT_BEGIN_MODE(glCopyTexImage2D);

    PRINT_CALL("glCopyTexImage2D(" << target << "," << level << "," << afl::strIFMT(internalformat) << "," << x << ","
                                   << y << "," << width << "," << height << "," << border << ");" << endl; )

/*
    WARNING_CODE
    (
    cout << "-------------------------------------------------------------------------------" << endl;
    cout << "Warning: glCopyTexImage2D is not really supported by now. Instead of taking" << endl;
    cout << "         the contents from the back buffer, the batch which uses this texture" << endl;
    cout << "         is ignored." << endl;
    cout << "-------------------------------------------------------------------------------" << endl;
    )
*/

    GLenum tg; // All cube map faces have GL_TEXTURE_CUBE_MAP as target
    switch ( target )
    {
        case GL_TEXTURE_2D:
            tg = GL_TEXTURE_2D;
            break;
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            tg = GL_TEXTURE_CUBE_MAP;
            break;
        default:
            panic("GPULib", "glCopyTexImage2D", "only TEXTURE_2D or CUBEMAP faces are allowed as target");
    }

    // Get current texture object
    TextureObject& to = GET_TEXTURE(tg);
    
    // target == CUBE_MAP face or GL_TEXTURE_2D

    GLuint id = to.getName();

    if ( internalformat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT )
    {
        WARNING_CODE
        (
        cout << "----------------------------------------------------------------------------------" << endl;
        cout << "Warning: glCopyTexImage2D: Internal format COMPRESSED_RGB_S3TC_DXT1 not supported" << endl;
        cout << "Texture object " << id << " uses uncompressed GL_RGB" << endl;
        cout << "----------------------------------------------------------------------------------" << endl;
        )
        internalformat = GL_RGB;
    }
    else if ( internalformat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT )
    {
        WARNING_CODE
        (
        cout << "------------------------------------------------------------------------------------------" << endl;
        cout << "Warning: glCopyTexImage2D: Internal format GL_COMPRESSED_RGBA_S3TC_DXT1_EXT not supported" << endl;
        cout << "Texture object " << id << " uses uncompressed GL_RGBA" << endl;
        cout << "------------------------------------------------------------------------------------------" << endl;
        )
        internalformat = GL_RGBA;
    }
    else if ( internalformat == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT )
    {
        WARNING_CODE
        (
        cout << "------------------------------------------------------------------------------------------" << endl;
        cout << "Warning: glCopyTexImage2D: Internal format GL_COMPRESSED_RGBA_S3TC_DXT3_EXT not supported" << endl;
        cout << "Texture object " << id << " uses uncompressed GL_RGBA" << endl;
        cout << "------------------------------------------------------------------------------------------" << endl;
        )
        internalformat = GL_RGBA;
    }
    else if ( internalformat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT )
    {
        WARNING_CODE
        (
        cout << "------------------------------------------------------------------------------------------" << endl;
        cout << "Warning: glCopyTexImage2D: Internal format GL_COMPRESSED_RGBA_S3TC_DXT5_EXT not supported" << endl;
        cout << "Texture object " << id << " uses uncompressed GL_RGBA" << endl;
        cout << "-------------------------------------------------------------------------------------------" << endl;
        )
        internalformat = GL_RGBA;
    }

    GLubyte* data = new GLubyte[width*height*4];

    for (int i=0; i< width*height*4; i+=4)
    {
        data[i] = 0;
        data[i+1] = 0;
        data[i+2] = 0;
        data[i+3] = 255;
    }

    /**
      * The mipmap level information of the texture object is updated anyway. This sets properly the width an height parameters
      * of the texture mipmap, format and other parameters to be consistent with the real contents that will be present in the
      * memory region after performing the bit blit operation.
      *
      * It is also important to define the mipmap contents as a previous action to the texture object allocation, to get a
      * valid memory descriptor pointing to a large enough memory region.
      *
      */
    to.setContents(target, level, internalformat, width, height, 1, border, GL_RGBA, GL_UNSIGNED_BYTE, (const GLubyte*)data, 0);

    delete[] data;
    
    /* After TextureObject::setContents() call, correctness of the target and mipmap level is guaranteed */
    
    /**
     * Allocation of the texture object is necessary to obtain a valid memory descriptor.
     */
    to.sync();
    ctx->gpumem().allocate(to);
    
    // Finally, the texture object state is set to 'Blit', to avoid further syncronizations
    // of non-real data stored in the texture object.
    
    to.setBlitted();
    
    // After allocation, all the texture object mipmaps have associated memory descriptors. 
    // Let´s obtain the related md !!
    
    u32bit md;
    
    if (tg == GL_TEXTURE_2D)
    {
        md = ctx->gpumem().md(to, level);
    }
    else // tg == GL_TEXTURE_CUBE_MAP
    {
        // Compute the face + mipmap offset portion.
        GLuint face = GL_TEXTURE_CUBE_MAP_POSITIVE_X - tg;
        GLuint portion;

        to.getMipmapPortion(face, level, portion);
        
        md = ctx->gpumem().md(to, portion);
    }   
    
    GLsizei size;
    
    afl::performBlitOperation(0, 0, width, x, y, width, height, md, internalformat, size);
}

GLAPI void GLAPIENTRY glCopyTexSubImage2D( GLenum target, GLint level,
                                           GLint xoffset, GLint yoffset,
                                           GLint x, GLint y,
                                           GLsizei width, GLsizei height )
{
    CHECK_NOT_BEGIN_MODE(glCopyTexSubImage2D);

    PRINT_CALL("glCopyTexSubImage2D(" << target << "," << level << "," << xoffset << "," << yoffset << "," << x << ","
                                      << y << "," << width << "," << height << ");" << endl )
                                      
    // Hack for Riddick
    //ignoreClearAfterTexSubImage = true;
/*
    WARNING_CODE
    (
    cout << "-------------------------------------------------------------------------------" << endl;
    cout << "Warning: glCopyTexSubImage2D is not really supported by now. Instead of taking" << endl;
    cout << "         the contents from the back buffer, the batch which uses this texture" << endl;
    cout << "         is ignored." << endl;
    cout << "-------------------------------------------------------------------------------" << endl;
    )
*/
    GLenum tg;
    switch ( target )
    {
        case GL_TEXTURE_2D:
            tg = GL_TEXTURE_2D;
            break;
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            tg = GL_TEXTURE_CUBE_MAP;
            break;
        default:
            panic("GPULib", "glCopyTexSubImage2D", "only TEXTURE_2D or CUBEMAP faces are allowed as target");
    }

    TextureObject& to = GET_TEXTURE(tg);

    GLuint id = to.getName();

    
    GLubyte* subData = new GLubyte[width * height * 4];

    for (int i = 0; i < width * height * 4; i+=4)
    {
        subData[i] = 0;
        subData[i+1] = 255;
        subData[i+2] = 0;
        subData[i+3] = 255;
    }

    //to.setPartialContents(target, level, xoffset, yoffset, 0, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, subData, 0);
       
    delete[] subData;
    
    /* After TextureObject::setPartialContents() call, correctness of the target and mipmap level, the xoffset and yoffset
     * boundaries, is guaranteed 
     */
    
    /*
     * Further update of a texture region is allowed, but only using the glCopyTexSubImage() call.
     *
     * If further updates were made with glTexSubImage(), this situation would be detected (throwing the proper panic) 
     * in the BaseObject class, when trying to update the contents of a BaseObject in Blit state.
     *
     */

    /**
     * If texture mipmap level contents not defined previously with glCopyTexImage() or glCopyTexSubImage() then first update
     * contents to memory and then perform bit blit operation.
     */
     
    to.sync();
    if (to.getState() != BaseObject::Blit)
    {
        ctx->gpumem().allocate(to);
    }
    
    // Finally, the texture object state is set to 'Blit', to avoid further syncronizations
    // of non-real data stored in the texture object.
    
    to.setBlitted();    
    
    // After allocation, all the texture object mipmaps have associated memory descriptors. 
    // Let´s obtain the related md !!
    
    u32bit md;
    
    if (tg == GL_TEXTURE_2D)
    {
        md = ctx->gpumem().md(to, level);
    }
    else // tg == GL_TEXTURE_CUBE_MAP
    {
        // Compute the face + mipmap offset portion.
        GLuint face = GL_TEXTURE_CUBE_MAP_POSITIVE_X - tg;
        GLuint portion;

        to.getMipmapPortion(face, level, portion);
        
        md = ctx->gpumem().md(to, portion);
    }   
    
    GLsizei size;
    
    afl::performBlitOperation(xoffset, yoffset, to.getMipmapWidth(target, level), x, y, width, height, md, to.getInternalFormat(), size);
}

GLAPI void GLAPIENTRY glTexCoord2f( GLfloat s, GLfloat t )
{
    PRINT_CALL("glTexCoord2f(" << s << ", " << t << ");" << endl; )
    ctx->setTexCoord(0,s,t); // Set texture coordinate for texture unit 0
}

GLAPI void GLAPIENTRY glMultiTexCoord2f( GLenum texture, GLfloat s, GLfloat t )
{
    PRINT_CALL("glMultiTexCoord2f(0x" << hex << texture << dec << s << ", " << t << ");" << endl; )

    GLuint texUnit = texture - GL_TEXTURE0;
    ctx->setTexCoord(texUnit, s, t);
}

GLAPI void GLAPIENTRY glMultiTexCoord2fv( GLenum texture, const GLfloat *v )
{
    PRINT_CALL("glMultiTexCoord2f(0x" << hex << texture << dec << "{" << v[0] << "," << v[1] << "});" << endl; )

    GLuint texUnit = texture - GL_TEXTURE0;
    ctx->setTexCoord(texUnit, v[0], v[1]);
}

GLAPI void GLAPIENTRY glTexGeni(GLenum coord, GLenum pname, GLint param)
{
    PRINT_CALL("glTexGeni(0x" << hex << coord << ",0x" << hex << pname << ", " << param << ");" << endl; )

    ctx->setTexGen(coord, pname, param);
}

GLAPI void GLAPIENTRY glTexGenfv(GLenum coord, GLenum pname, const GLfloat* param)
{
    PRINT_CALL("glTexGeni(0x" << hex << coord << ",0x" << hex << pname << ", " << hex << param << dec << ");" << endl; )

    ctx->setTexGen(coord, pname, param);
}

GLAPI void GLAPIENTRY glTexGenf( GLenum coord, GLenum pname, GLfloat param )
{
    PRINT_CALL("glTexGenf(0x" << hex << coord << ",0x" << hex << pname << ", " << param << ");" << endl; )

    ctx->setTexGen(coord, pname, GLint(param));
}

GLAPI void GLAPIENTRY glClientActiveTextureARB(GLenum texture)
{
    PRINT_CALL( "glClientActiveTextureARB(" << hex << texture << dec << ");" << endl; )

    ctx->setCurrentClientTextureUnit(texture - GL_TEXTURE0);
}

GLAPI void GLAPIENTRY glTexEnvf( GLenum target, GLenum pname, GLfloat param )
{
    PRINT_CALL( "glTexEnvf(0x" << hex << target << ", 0x" << hex << pname << dec << ", " << param << ");" << endl; )

    TextureUnit& tu = ctx->getActiveTextureUnit();
    tu.setParameter(target, pname, &param);

    SET_LIB_DIRTY;
}

GLAPI void GLAPIENTRY glTexEnvi( GLenum target, GLenum pname, GLint param )
{
    PRINT_CALL( "glTexEnvf(0x" << hex << target << ",0x" << hex << pname << dec << ", " << param << ");" << endl; )

    TextureUnit& tu = ctx->getActiveTextureUnit();
    tu.setParameter(target, pname, &param);

    SET_LIB_DIRTY;
}

/*
GLAPI void GLAPIENTRY glTexEnvfv( GLenum target, GLenum pname, const GLfloat *params )
{
    PRINT_CALL( "glTexEnvfv(0x" << hex << target << ",0x" << hex << pname << "," << dec << GLuint(params) << dec << ");" << endl; )

    TextureUnit& tu = ctx->getActiveTextureUnit();
    tu.setParameter(target, pname, params);
}
*/

GLAPI void GLAPIENTRY glTexEnvfv( GLenum target, GLenum pname, const GLfloat *params )
{
    PRINT_CALL( "glTexEnvfv(0x" << hex << target << ",0x" << hex << pname << "," << dec << GLuint(params) << dec << ");" << endl; )

    TextureUnit& tu = ctx->getActiveTextureUnit();

    if ( pname == GL_TEXTURE_ENV_COLOR )
    {
        GLuint tu = TextureManager::instance().getCurrentGroup(); // get active texture unit
        ctx->setTexEnvColor(tu, params[0], params[1], params[2], params[3]);
    }
    else
    {
        tu.setParameter(target, pname, params);
        SET_LIB_DIRTY;
    }
}

GLAPI void GLAPIENTRY glFogf( GLenum pname, GLfloat param )
{
    PRINT_CALL( "glFogf(0x" << hex << pname << dec << ", " << param << ");" << endl; )

    ctx->setFog(pname, &param);
}

GLAPI void GLAPIENTRY glFogi( GLenum pname, GLint param )
{
    PRINT_CALL( "glFogi(0x" << hex << pname << dec << ", " << param << ");" << endl; )

    ctx->setFog(pname, &param);
}

GLAPI void GLAPIENTRY glFogfv( GLenum pname, const GLfloat *params )
{
    PRINT_CALL( "glFogfv(0x" << hex << pname << dec << ", " << GLuint(params) << ");" << endl; )

    ctx->setFog(pname, params);
}

GLAPI void GLAPIENTRY glFogiv( GLenum pname, const GLint *params )
{
    PRINT_CALL( "glFogiv(0x" << hex << pname << dec << ", " << GLuint(params) << ");" << endl; )

    ctx->setFog(pname, params);
}


GLAPI void GLAPIENTRY glStencilFunc( GLenum func, GLint ref, GLuint mask )
{
    PRINT_CALL( "glStencilFunc(0x" << hex << func << dec << ", " << ref << ", " << mask << ");" << endl; )

    ctx->setStencilBufferFunc(func, ref, mask);

    GPURegData data;

    #define BRANCH_CASE(c) case GL_##c : data.compare = GPU_##c; break;

    switch ( func )
    {
        BRANCH_CASE(NEVER)
        BRANCH_CASE(ALWAYS)
        BRANCH_CASE(LESS)
        BRANCH_CASE(LEQUAL)
        BRANCH_CASE(EQUAL)
        BRANCH_CASE(GEQUAL)
        BRANCH_CASE(GREATER)
        BRANCH_CASE(NOTEQUAL)
        default:
            panic("GPULib", "glStencilFunc", "Unexpected function");
    }

    #undef BRANCH_CASE

    driver->writeGPURegister(GPU_STENCIL_FUNCTION, data);

    data.uintVal = (u32bit)ref;
    driver->writeGPURegister(GPU_STENCIL_REFERENCE, data);

    data.uintVal = mask;
    driver->writeGPURegister(GPU_STENCIL_COMPARE_MASK, data);
}


GLAPI void GLAPIENTRY glStencilOp( GLenum fail, GLenum zfail, GLenum zpass )
{
    PRINT_CALL( "glStencilOp(0x" << hex << fail << ", " << zfail << ", " << zpass << ");" << endl; )

    GPURegData data;

    #define _SWITCH(fail_, msgError) \
        switch ( fail_ )\
        {\
            case GL_KEEP : data.stencilUpdate = STENCIL_KEEP; break;\
            case GL_ZERO : data.stencilUpdate = STENCIL_ZERO; break;\
            case GL_REPLACE : data.stencilUpdate = STENCIL_REPLACE; break;\
            case GL_INCR : data.stencilUpdate = STENCIL_INCR; break;\
            case GL_DECR : data.stencilUpdate = STENCIL_DECR; break;\
            case GL_INVERT : data.stencilUpdate = STENCIL_INVERT; break;\
            case GL_INCR_WRAP : data.stencilUpdate = STENCIL_INCR_WRAP; break;\
            case GL_DECR_WRAP : data.stencilUpdate = STENCIL_DECR_WRAP; break;\
            default: panic("GPULib", "glStencilOp", msgError);\
        }

    _SWITCH(fail, "Unexpected first parameter (fail)")
    driver->writeGPURegister(GPU_STENCIL_FAIL_UPDATE, data);

    _SWITCH(zfail, "Unexpected second parameter (zfail)")
    driver->writeGPURegister(GPU_DEPTH_FAIL_UPDATE, data);

    _SWITCH(zpass, "Unexpected third parameter (zpass)")
    driver->writeGPURegister(GPU_DEPTH_PASS_UPDATE, data);

    ctx->setStencilOp(fail, zfail, zpass);

    #undef _SWITCH
}

GLAPI void GLAPIENTRY glClearStencil( GLint s )
{
    PRINT_CALL( "glClearStencil(" << s << ");" << endl; )

    ctx->setStencilBufferClearValue(s);

    //  NOTE: This change must be deferred until the next clear for correct behaviour.

    //GPURegData data;
    //data.intVal = s;

    //driver->writeGPURegister(GPU_STENCIL_BUFFER_CLEAR, data);
}

GLAPI void GLAPIENTRY glStencilMask( GLuint mask )
{
    PRINT_CALL( "glStencilMask(" << mask << ");" << endl; )

    ctx->setStencilBufferUpdateMask(mask);

    GPURegData data;
    data.uintVal = mask;

    driver->writeGPURegister(GPU_STENCIL_UPDATE_MASK, data);
}

GLAPI void GLAPIENTRY glColorMask( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha )
{
    PRINT_CALL( "glColorMask(" << (red != 0?"TRUE,":"FALSE,") << (green != 0?"TRUE,":"FALSE,")
                << (blue != 0?"TRUE,":"FALSE,") << (alpha != 0?"TRUE);":"FALSE);") << endl; )

    ctx->setColorMask(red, green, blue, alpha);

    GPURegData data;

    data.booleanVal = (red != 0 ? true : false);
    driver->writeGPURegister(GPU_COLOR_MASK_R, data);

    data.booleanVal = (green != 0 ? true : false);
    driver->writeGPURegister(GPU_COLOR_MASK_G, data);

    data.booleanVal = (blue != 0 ? true : false);
    driver->writeGPURegister(GPU_COLOR_MASK_B, data);

    data.booleanVal = (alpha != 0 ? true : false);
    driver->writeGPURegister(GPU_COLOR_MASK_A, data);
}

GLAPI void GLAPIENTRY glAlphaFunc( GLenum func, GLclampf ref )
{
    PRINT_CALL( "glAlphaFunc(0x" << hex << func <<  ", " << dec << ref << ");" << endl; )

    ctx->setAlphaTest(func, ref);
}

GLAPI void GLAPIENTRY glBlendFunc( GLenum sfactor, GLenum dfactor )
{
    PRINT_CALL( "glAlphaFunc(0x" << hex << ", " << sfactor << "," << hex << dfactor << dec << ");" << endl; )

    GPURegData data;

    #define SWITCH_BR(v) case GL_##v: data.blendFunction = BLEND_##v; break;

    switch ( sfactor )
    {
        SWITCH_BR(ZERO)
        SWITCH_BR(ONE)
        SWITCH_BR(SRC_COLOR)
        SWITCH_BR(ONE_MINUS_SRC_COLOR)
        SWITCH_BR(DST_COLOR)
        SWITCH_BR(ONE_MINUS_DST_COLOR)
        SWITCH_BR(SRC_ALPHA)
        SWITCH_BR(ONE_MINUS_SRC_ALPHA)
        SWITCH_BR(DST_ALPHA)
        SWITCH_BR(ONE_MINUS_DST_ALPHA)
        SWITCH_BR(CONSTANT_COLOR)
        SWITCH_BR(ONE_MINUS_CONSTANT_COLOR)
        SWITCH_BR(CONSTANT_ALPHA)
        SWITCH_BR(ONE_MINUS_CONSTANT_ALPHA)
        SWITCH_BR(SRC_ALPHA_SATURATE)
    }
    driver->writeGPURegister(GPU_BLEND_SRC_RGB, data);
    driver->writeGPURegister(GPU_BLEND_SRC_ALPHA, data);

    switch ( dfactor )
    {
        SWITCH_BR(ZERO)
        SWITCH_BR(ONE)
        SWITCH_BR(SRC_COLOR)
        SWITCH_BR(ONE_MINUS_SRC_COLOR)
        SWITCH_BR(DST_COLOR)
        SWITCH_BR(ONE_MINUS_DST_COLOR)
        SWITCH_BR(SRC_ALPHA)
        SWITCH_BR(ONE_MINUS_SRC_ALPHA)
        SWITCH_BR(DST_ALPHA)
        SWITCH_BR(ONE_MINUS_DST_ALPHA)
        SWITCH_BR(CONSTANT_COLOR)
        SWITCH_BR(ONE_MINUS_CONSTANT_COLOR)
        SWITCH_BR(CONSTANT_ALPHA)
        SWITCH_BR(ONE_MINUS_CONSTANT_ALPHA)
        SWITCH_BR(SRC_ALPHA_SATURATE)
    }
    driver->writeGPURegister(GPU_BLEND_DST_RGB, data);
    driver->writeGPURegister(GPU_BLEND_DST_ALPHA, data);

    ctx->setBlendFactor(sfactor, dfactor, sfactor, dfactor);

    #undef SWITCH_BR
}


GLAPI void GLAPIENTRY glBlendEquation( GLenum mode )
{
    PRINT_CALL( "glBlendEquation(0x" << hex << mode << dec << ");" << endl; );
    GPURegData eq;
    switch ( mode )
    {
        case GL_FUNC_ADD:
            eq.blendEquation = BLEND_FUNC_ADD;
            break;
        case GL_FUNC_SUBTRACT:
            eq.blendEquation = BLEND_FUNC_SUBTRACT;
            break;
        case GL_FUNC_REVERSE_SUBTRACT:
            eq.blendEquation = BLEND_FUNC_REVERSE_SUBTRACT;
            break;
        case GL_MIN:
            eq.blendEquation = BLEND_MIN;
            break;
        case GL_MAX:
            eq.blendEquation = BLEND_MAX;
            break;
        default:
            panic("GPULib", "glBlendEquation", "Unexpected blend mode equation");
    }
    driver->writeGPURegister(GPU_BLEND_EQUATION, eq);

    ctx->setBlendEquation(mode);
}

GLAPI void GLAPIENTRY glBlendColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )
{
    PRINT_CALL( "glBlendColor(" << red << ", " << green << ", " << blue << ", " << alpha << ");" << endl; )

    GPURegData color;
    color.qfVal[0] = afl::clamp(red);
    color.qfVal[1] = afl::clamp(green);
    color.qfVal[2] = afl::clamp(blue);
    color.qfVal[3] = afl::clamp(alpha);
    driver->writeGPURegister(GPU_BLEND_COLOR, color);

    ctx->setBlendColor(color.qfVal[0],color.qfVal[1],color.qfVal[2],color.qfVal[3]);
}

GLAPI void GLAPIENTRY glColorMaterial( GLenum face, GLenum mode )
{
    PRINT_CALL( "glColorMaterial(0x" << hex << face << ",0x" << hex << mode << ");" << endl; )

    ctx->setColorMaterial(face, mode);
}

GLAPI void GLAPIENTRY glScissor( GLint x, GLint y, GLsizei width, GLsizei height)
{
    PRINT_CALL( "glScissor(" << x << "," << y << "," << width << "," << height << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glScissor);

    ctx->setScissor(x, y, width, height);

    GPURegData data;

    data.intVal = x;
    driver->writeGPURegister( GPU_SCISSOR_INI_X, data );
    data.intVal = y;
    driver->writeGPURegister( GPU_SCISSOR_INI_Y, data );
    data.uintVal = width;
    driver->writeGPURegister( GPU_SCISSOR_WIDTH, data );
    data.uintVal = height;
    driver->writeGPURegister( GPU_SCISSOR_HEIGHT, data );

    ctx->setSetupShaderConstantsUpdate(false);

}

GLAPI void GLAPIENTRY glDepthRange( GLclampd near_val, GLclampd far_val )
{
    PRINT_CALL( "glDepthRange(" << near_val << "," << far_val << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glDepthRange);

    near_val = afl::clamp(near_val);
    far_val = afl::clamp(far_val);

    ctx->setDepthRange(near_val, far_val);

    GPURegData data;

    data.f32Val = near_val;
    driver->writeGPURegister( GPU_DEPTH_RANGE_NEAR, data );
    data.f32Val = far_val;
    driver->writeGPURegister( GPU_DEPTH_RANGE_FAR, data );

    ctx->setSetupShaderConstantsUpdate(false);

}

GLAPI void GLAPIENTRY glPolygonOffset( GLfloat factor, GLfloat units )
{
    PRINT_CALL( "glPolygonOffset(" << factor << "," << units << ");" << endl; )

    CHECK_NOT_BEGIN_MODE(glPolygonOffset);

    ctx->setPolygonOffset(factor,units);

    if ( ctx->testFlags(GLContext::flagPolygonOffsetFill) &&
       ( factor != 0.0f || units != 0.0f ))
    {
        GPURegData data;

        data.f32Val = factor;
        driver->writeGPURegister( GPU_DEPTH_SLOPE_FACTOR, data );
        data.f32Val = units;
        driver->writeGPURegister( GPU_DEPTH_UNIT_OFFSET, data );

    }
    ctx->setSetupShaderConstantsUpdate(false);

}

GLAPI void GLAPIENTRY glPushAttrib( GLbitfield mask )
{
    PRINT_CALL( "glPushAttrib(" << hex << mask << ");" << endl; )

    ctx->pushAttrib(mask);

    ctx->setSetupShaderConstantsUpdate(false);
}

GLAPI void GLAPIENTRY glPopAttrib( void )
{
    PRINT_CALL( "glPopAttrib(void);" << endl; )

    ctx->popAttrib();

    ctx->setSetupShaderConstantsUpdate(false);
}
