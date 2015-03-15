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

#include "GPULibInternals.h"
#include "TextureManager.h"
#include "AuxFuncsLib.h"
#include "support.h"
#include "GLContext.h"
#include "MathLib.h"
#include "TLShader.h"
#include "TLFactory.h"
#include "FPFactory.h"
#include <cmath>
#include <fstream>

using namespace libgl;
using namespace gpu3d;
using namespace std;

GLuint afl::computeHighestIndex(GLenum type, const GLvoid* indices, GLsizei count)
{
    switch ( type )
    {
        case GL_UNSIGNED_BYTE:
            return computeHighestIndex2((GLubyte*)indices,count);
        case GL_UNSIGNED_SHORT:
            return computeHighestIndex2((GLushort*)indices,count);
        case GL_UNSIGNED_INT:
            return computeHighestIndex2((GLuint*)indices,count);
        default:
            panic("LibAuxFunctions","computeHighestIndex()","Type index unsupported");
    }
    return 0; // avoid dummy warnings in some compilers (VS6)
}

void afl::computeIndexBounds(GLenum type, const GLvoid* indices, GLsizei count, GLuint& min, GLuint& max)
{
    switch ( type )
    {
        case GL_UNSIGNED_BYTE:
            return computeIndexBounds2((GLubyte*)indices,count, min, max);
        case GL_UNSIGNED_SHORT:
            return computeIndexBounds2((GLushort*)indices,count, min, max);
        case GL_UNSIGNED_INT:
            return computeIndexBounds2((GLuint*)indices,count, min, max);
        default:
            panic("LibAuxFunctions","computeIndexBounds()","Type index unsupported");
    }
}


GLuint afl::getSize(GLenum openGLType)
{
    switch ( openGLType )
    {
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
        case GL_UNSIGNED_INT_8_8_8_8:
            return sizeof(GLubyte);
        case GL_SHORT:
        case GL_UNSIGNED_SHORT:
            return sizeof(GLushort);
        case GL_INT:
        case GL_UNSIGNED_INT:
            return sizeof(GLuint);
        case GL_FLOAT:
            return sizeof(GLfloat);
        case GL_DOUBLE:
            return sizeof(GLdouble);
        case GL_2_BYTES:
            return 2*sizeof(GLubyte);
        case GL_3_BYTES:
            return 3*sizeof(GLubyte);
        case GL_4_BYTES:
            return 4*sizeof(GLubyte);
        default:
            panic("AuxFuncsLib", "getSize()", "Unknown openGL type size");
            return 0; /* avoid stupid warnings */
    }
}



StreamData afl::getStreamDataType(GLenum openGLType)
{
    switch ( openGLType ) {
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
            return SD_U8BIT;
        case GL_SHORT:
        case GL_UNSIGNED_SHORT:
            return SD_U16BIT;
        case GL_INT:
        case GL_UNSIGNED_INT:
            return SD_U32BIT;
        case GL_FLOAT:
        case GL_DOUBLE:
            return SD_FLOAT;
        default:
        {
            char msg[64];
            sprintf(msg, "Unknown OpenGL type size. Type == 0x%x", openGLType);
            panic("AuxFuncLib", "getStreamDataType()", msg);
        }
    }
    return SD_FLOAT; /* avoid stupid warnings... */

}

PrimitiveMode afl::translatePrimitive(GLenum primitive)
{
    switch ( primitive )
    {
        case GL_POINTS:
            return gpu3d::POINT;
        case GL_LINES:
            return LINE;
        case GL_LINE_LOOP:
            return LINE_FAN;
        case GL_LINE_STRIP:
            return LINE_STRIP;
        case GL_TRIANGLES:
            return TRIANGLE;
        case GL_TRIANGLE_STRIP:
            return TRIANGLE_STRIP;
        case GL_TRIANGLE_FAN:
            return TRIANGLE_FAN;
        case GL_QUADS:
            return QUAD;
        case GL_QUAD_STRIP:
            return QUAD_STRIP;
        case GL_POLYGON:
            /* not supported... */
        default:
            return TRIANGLE;
    }
}

GLfloat afl::clamp(GLfloat v)
{
    return ((v)>1.0f?1.0f:((v)<0.0f?0.0f:(v)));
}

GLdouble afl::clamp(GLdouble v)
{
    return ((v)>1.0?1.0:((v)<0.0?0.0:(v)));
}


void afl::normalizeQuad(QuadReg<float>& qr)
{
    float m = std::sqrt(qr[0]*qr[0] + qr[1]*qr[1] + qr[2]*qr[2]);
    qr[0] = qr[0]/m;
    qr[1] = qr[1]/m;
    qr[2] = qr[2]/m;
    qr[3] = qr[3]/m;
}

void afl::dumpBuffer(ofstream& out, const GLubyte* data, GLsizei components, GLenum type, GLsizei start, GLsizei stride, GLsizei count )
{
    const GLubyte* ptr_ub = (data + start);
    // ...
    const GLushort* ptr_us = 0;
    const GLfloat* ptr_f = 0;
    const GLuint* ptr_ui = 0;
    out << "{";
    GLint i, j;

    if (stride == 0) stride = 4;

    switch ( type )
    {
        case GL_UNSIGNED_BYTE:
        case SD_U8BIT:
            for ( i = 0; i < count; i++, ptr_ub += stride )
            {
                ptr_ub = (const GLubyte*)ptr_ub;
                if ( components != 1 )
                    out << "{";
                for ( j = 0; j < components; j++ )
                {
                    out << ptr_ub[j];
                    if ( j < components - 1 )
                        out << ",";
                }
                if ( components != 1 )
                    out << "}";
                if ( i < count -1 )
                    out << ",";
            }
            break;
        case GL_UNSIGNED_SHORT:
        case SD_U16BIT:
            for ( i = 0; i < count; i++, ptr_ub += stride )
            {
                ptr_us = (const GLushort*)ptr_ub;
                if ( components != 1 )
                    out << "{";
                for ( j = 0; j < components; j++ )
                {
                    out << ptr_us[j];
                    if ( j < components - 1 )
                        out << ",";
                }
                if ( components != 1 )
                    out << "}";
                if ( i < count -1 )
                    out << ",";
            }
            break;
        case GL_UNSIGNED_INT:
        case SD_U32BIT:
            for ( i = 0; i < count; i++, ptr_ub += stride )
            {
                ptr_ui = (const GLuint*)ptr_ub;
                if ( components != 1 )
                    out << "{";
                for ( j = 0; j < components; j++ )
                {
                    out << ptr_ui[j];
                    if ( j < components - 1 )
                        out << ",";
                }
                if ( components != 1 )
                    out << "}";
                if ( i < count -1 )
                    out << ",";
            }
            break;
        case GL_FLOAT:
        case SD_FLOAT:

            for ( i = 0; i < count; i++, ptr_ub += stride )
            {
                ptr_f = (const GLfloat*)ptr_ub;
                if ( components != 1 )
                    out << "{";
                for ( j = 0; j < components; j++ )
                {
                    out << ptr_f[j];
                    if ( j < components - 1 )
                        out << ",";
                }
                if ( components != 1 )
                    out << "}";
                if ( i < count -1 )
                    out << ",";
            }
            break;

    }
    out << "}";
}

void afl::drawElements(GLContext* ctx, ::GPUDriver* driver, GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
{
    using namespace std;

    if ( GL_POINTS <= mode && mode <= GL_POLYGON )
    {
        GPURegData data;
        data.primitive = afl::translatePrimitive(mode);
        driver->writeGPURegister(GPU_PRIMITIVE, data);
    }
    else
        panic("AuxFuncLib", "drawElements()", "Unknown primitive type");

    /**
     * This two macros set and check draw mode, if the primitive (mode) is not supported, then
     * the call is skipped
     */
    SET_BATCH_MODE_SUPPORT(mode);
    CHECK_BATCH_MODE_SUPPORT_MSG("Warning (drawElements). Primitive mode not supported. Batch ignored\n");

    bool disableColorMask;


#ifdef GLLIB_DUMP_STREAMS
    GLubyte filename[30];
    sprintf((char*)filename, "GLLibDumpStreams.txt");
    remove((char*)filename);
#endif

#ifdef GLLIB_DUMP_SAMPLERS

#ifndef GLLIB_DUMP_STREAMS
    GLubyte filename[30];
#endif
    for (GLuint i = 0; i < ProgramObject::MaxTextureUnits; i++)
        for (GLuint j = 0; j < 6; j++)
            for (GLuint k = 0; k < 13; k++)
            {
                sprintf((char*)filename, "Sampler%d_Face%d_Mipmap%d.ppm", i,j,k);
                remove((char*)filename);
            }
#endif

    afl::synchronizeTextureImages(ctx, driver, disableColorMask);

    // compute highest index in indices
    //u32bit minIndex, maxIndex;
    //afl::computeIndexBounds(type, indices, count, minIndex, maxIndex);

    /*****************************************
     * COPY INDICES & BUFFERS TO MEMORY CARD *
     *****************************************/
    //int mIndices = afl::sendIndices(driver, indices, type, count, -minIndex);
    int mIndices;
    vector<unsigned int> descs;
    GLuint nextFreeStream = 0;

    if ( !ctx->allBuffersBound() && !ctx->allBuffersUnbound() )
        panic("AuxFuncsLib", "drawElements", "Mixed VBO and client-side vertex arrays not supported in indexed mode");

    set<GLuint> iSet;

    if ( ctx->allBuffersBound() )
    {
#ifdef GLLIB_DUMP_STREAMS
        ofstream out;
        out.open("GLLibDumpStreams.txt",ios::app);

        if ( !out.is_open() )
            panic("RegisterWriteBuffer", "dumpRegisterStatus", "Dump failed (output file could not be opened)");
        
        out << " Dumping Stream Indexed" << endl;
        out << "\t Stride: " << 0 << endl;
        out << "\t Data type: " << type << endl;
        out << "\t Num. elemets: " << 1 << endl;
        out << "\t Offset: " << 0 << endl;
        out << "\t Dumping stream content: " << endl;

#endif
        if ( IS_BUFFER_BOUND(GL_ELEMENT_ARRAY_BUFFER) )
        {
            BufferObject& boElementArray = GET_ELEMENT_ARRAY_BUFFER;

#ifdef GLLIB_DUMP_STREAMS
            dumpBuffer(out, (const GLubyte*)boElementArray.getData(), 1, type, 0, 0, count);
#endif

            ctx->gpumem().allocate(boElementArray);
            mIndices = ctx->gpumem().md(boElementArray);
        }
        else
        {
            //BufferObject& boArray = GET_ARRAY_BUFFER;
            /*
            cout << endl;
            boArray.printfmt(3, GL_FLOAT, 0, 60, 6);
            cout << endl;
            boArray.printfmt(2, GL_FLOAT, 12, 60, 6);
            cout << endl;
            */
            mIndices = afl::sendRawIndices(driver, indices, type, count);
#ifdef GLLIB_DUMP_STREAMS
            dumpBuffer(out, (const GLubyte*)indices, 1, type, 0, 0, count);
#endif
            //cout << endl;
        }
    }
    else // ctx->allBuffersUnbound() == TRUE
    {
        if ( IS_BUFFER_BOUND(GL_ELEMENT_ARRAY_BUFFER) )
            panic("AuxFuncsLib", "drawElements", "Unbound GL_ARRAY_BUFFER & bound GL_ELEMENTS_ARRAY_BUFFER not supported");
        else
        {
            afl::getIndicesSet(iSet, type, indices, count);
            mIndices = afl::sendIndices(driver, indices, type, count, iSet);
        }
    }
    

    descs = afl::setVertexArraysBuffers(ctx, driver, 0, count, iSet, nextFreeStream);


    /******************************
     * SET INDEXED STREAMING MODE *
     ******************************/
    // use decs.size() for using the next available stream
    if ( IS_BUFFER_BOUND(GL_ELEMENT_ARRAY_BUFFER) )
        driver->setIndexedStreamingMode(nextFreeStream, count, 0, mIndices, u64bit(indices), afl::getStreamDataType(type));
    else
        driver->setIndexedStreamingMode(nextFreeStream, count, 0, mIndices, 0, afl::getStreamDataType(type));


    /************************
     * LOAD SHADER PROGRAMS *
     ************************/
    afl::setupShaders(ctx, driver);


    /*  Setup Hierarchical Z Test.  */
    afl::setupHZTest(ctx, driver);

    /*********************************
     * CONFIGURE FRAGMENT ATTRIBUTES *
     *********************************/
    //afl::setFragmentAttributes(driver, ctx);


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

    /***********************************************************
     * RELEASE STREAM BUFFERS MEMORY (CLIENT DESCRIPTORS ONLY) *
     ***********************************************************/
    vector<unsigned int>::iterator it = descs.begin();
    for ( ; it != descs.end(); it++ )
        driver->releaseMemory(*it);

    /*****************************************************
     * RELEASE SHADER PROGRAMS MEMORY AND INDICES BUFFER *
     *****************************************************/
    if ( !IS_BUFFER_BOUND(GL_ELEMENT_ARRAY_BUFFER) )
        driver->releaseMemory(mIndices);

}

void afl::glVertex(GLContext* ctx, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    CHECK_BATCH_MODE_SUPPORT;

    ctx->posbuf().add(x,y,z,w);
    ctx->colbuf().add(ctx->getColor());
    ctx->norbuf().add(ctx->getNormal());

    for ( GLuint i = 0; i < ctx->countTextureUnits(); i++ )
    {
        if ( ctx->getTextureUnit(i).getTarget() != 0 )
            ctx->texbuf(i).add(ctx->getTexCoord(i));
    }
}

void afl::compressedTexImage2D( GLenum target, GLint level, GLenum internalFormat, GLsizei width,
                                GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data )
{
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
            panic("AuxFuncsLib", "compressedTexImage2D", "only TEXTURE_2D or CUBE_MAP faces are allowed as target");
    }

    // Get current texture object
    TextureObject& to = GET_TEXTURE(tg);
    // target == CUBE_MAP face or GL_TEXTURE_2D

    // set Format = 0 && type = 0
    to.setContents(target, level, internalFormat, width, height, 1, border, 0, 0, (const GLubyte*)data, imageSize);
}



void afl::fillDBColor(DataBuffer<float>& db, GLenum type, const GLvoid* buf, GLsizei size, GLsizei stride, GLsizei start, GLsizei count)
{
    switch (type )
    {
        case GL_UNSIGNED_BYTE:
            fillDBColor(db, (GLubyte*)buf, size, stride, start, count);
            break;
        case GL_BYTE:
            fillDBColor(db, (GLbyte*)buf, size, stride, start, count);
            break;
        case GL_UNSIGNED_SHORT:
            fillDBColor(db, (GLushort*)buf, size, stride, start, count);
            break;
        case GL_SHORT:
            fillDBColor(db, (GLshort*)buf, size, stride, start, count);
            break;
        case GL_UNSIGNED_INT:
            fillDBColor(db, (GLuint*)buf, size, stride, start, count);
            break;
        case GL_INT:
            fillDBColor(db, (GLint*)buf, size, stride, start, count);
            break;
        case GL_FLOAT:
            fillDBColor(db, (GLfloat*)buf, size, stride, start, count);
            break;
        case GL_DOUBLE:
            fillDBColor(db, (GLdouble*)buf, size, stride, start, count);
            break;
        default:
            panic("AuxFuncsLib", "fillDBColor", "Unsupported type");
    }
}

void afl::fillDBColor_ind(DataBuffer<GLfloat>& db, GLenum type, const GLvoid* buf, GLsizei size, GLsizei stride,
                     const std::set<GLuint>& iSet)
{
    switch (type )
    {
        case GL_UNSIGNED_BYTE:
            fillDBColor_ind(db, (GLubyte*)buf, size, stride, iSet);
            break;
        case GL_BYTE:
            fillDBColor_ind(db, (GLbyte*)buf, size, stride, iSet);
            break;
        case GL_UNSIGNED_SHORT:
            fillDBColor_ind(db, (GLushort*)buf, size, stride, iSet);
            break;
        case GL_SHORT:
            fillDBColor_ind(db, (GLshort*)buf, size, stride, iSet);
            break;
        case GL_UNSIGNED_INT:
            fillDBColor_ind(db, (GLuint*)buf, size, stride, iSet);
            break;
        case GL_INT:
            fillDBColor_ind(db, (GLint*)buf, size, stride, iSet);
            break;
        case GL_FLOAT:
            fillDBColor_ind(db, (GLfloat*)buf, size, stride, iSet);
            break;
        case GL_DOUBLE:
            fillDBColor_ind(db, (GLdouble*)buf, size, stride, iSet);
            break;
        default:
            panic("AuxFuncsLib", "fillDBColor_ind", "Unsupported type");
    }
}


void afl::fillDB_ind(DataBuffer<GLfloat>& db, GLenum type, const GLvoid* buf, GLsizei size, GLsizei stride,
                     const std::set<GLuint>& iSet)
{

    switch (type )
    {
        case GL_UNSIGNED_BYTE:
            fillDB_ind(db, (GLubyte*)buf, size, stride, iSet);
            break;
        case GL_BYTE:
            fillDB_ind(db, (GLbyte*)buf, size, stride, iSet);
            break;
        case GL_UNSIGNED_SHORT:
            fillDB_ind(db, (GLushort*)buf, size, stride, iSet);
            break;
        case GL_SHORT:
            fillDB_ind(db, (GLshort*)buf, size, stride, iSet);
            break;
        case GL_UNSIGNED_INT:
            fillDB_ind(db, (GLuint*)buf, size, stride, iSet);
            break;
        case GL_INT:
            fillDB_ind(db, (GLint*)buf, size, stride, iSet);
            break;
        case GL_FLOAT:
            fillDB_ind(db, (GLfloat*)buf, size, stride, iSet);
            break;
        case GL_DOUBLE:
            fillDB_ind(db, (GLdouble*)buf, size, stride, iSet);
            break;
        default:
            panic("AuxFuncsLib", "fillDB_ind", "Unsupported type");
    }
}


void afl::fillDB(DataBuffer<float>& db, GLenum type, const GLvoid* buf, GLsizei size, GLsizei stride, GLsizei start, GLsizei count)
{
    switch (type )
    {
        case GL_UNSIGNED_BYTE:
            fillDB(db, (GLubyte*)buf, size, stride, start, count);
            break;
        case GL_BYTE:
            fillDB(db, (GLbyte*)buf, size, stride, start, count);
            break;
        case GL_UNSIGNED_SHORT:
            fillDB(db, (GLushort*)buf, size, stride, start, count);
            break;
        case GL_SHORT:
            fillDB(db, (GLshort*)buf, size, stride, start, count);
            break;
        case GL_UNSIGNED_INT:
            fillDB(db, (GLuint*)buf, size, stride, start, count);
            break;
        case GL_INT:
            fillDB(db, (GLint*)buf, size, stride, start, count);
            break;
        case GL_FLOAT:
            fillDB(db, (GLfloat*)buf, size, stride, start, count);
            break;
        case GL_DOUBLE:
            fillDB(db, (GLdouble*)buf, size, stride, start, count);
            break;
        default:
            panic("AuxFuncsLib", "fillDBColor", "Unsupported type");
    }
}

u32bit afl::sendDB(DataBuffer<float>& db, GPUDriver* drv)
{
    u32bit md = drv->obtainMemory(db.bytes());

    drv->writeMemory(md, 0, db.raw(), db.bytes());

    return md;
}


#define CONF_ATTRIB(arrayInfo_, attrib_, stream_, bo_)\
    {\
        GPUDriver::VertexAttribute va;\
        va.stream = stream_;\
        va.attrib = attrib_;\
        va.md = ctx->gpumem().md(bo_);\
        va.stride = arrayInfo_.stride;\
        va.offset = u32bit(arrayInfo_.userBuffer); /* pointer used as an offset */\
        va.componentsType = afl::getStreamDataType(arrayInfo_.type);\
        va.components = arrayInfo_.size;\
        driver->configureVertexAttribute(va);\
    }


GLuint afl::configureStream(GLContext* ctx, ::GPUDriver* driver,
                            GLContext::VArray& info,
                            ::GPUDriver::ShAttrib attrib,
                            GLuint stream, bool colorAttrib,
                            GLuint start, GLuint count,
                            std::set<GLuint>& iSet)
{
    GLContext::DBUF dbuf;
    GPUDriver::VertexAttribute va;
    va.enabled = true;
    va.stream = stream;
    va.attrib = attrib;
    va.components = info.size;
    va.offset = 0;

    bool indexedMode = ( iSet.empty() ? false : true ); // Check indexed mode

    GLubyte* auxData;
    auxData = 0;
    
    if ( info.bufferID != 0 )
    {
        // Using VBO server memory
        BufferObject& bo = GET_BUFFER_BY_NAME(GL_ARRAY_BUFFER, info.bufferID);
        //if ( libBufferObjects.used(info.BufferID) && !bo.isSynchronized() )
          //  ctx->gpumem().allocate(bo, true); // locked write to GPUMemory (disable batch pipelining)
        //else
        ctx->gpumem().allocate(bo); // Allow pipelining
        va.md = ctx->gpumem().md(bo);
        va.componentsType = afl::getStreamDataType(info.type);
        va.stride = info.stride;
        va.offset = u64bit(info.userBuffer); // pointer used as offset
        auxData = bo.getData();
    }
    else
    {
        // Using conventional client-side memory
        dbuf.set(0,info.size);
        if ( colorAttrib )
        {
            if ( indexedMode )
                fillDBColor_ind(dbuf, info.type, info.userBuffer, info.size, info.stride, iSet);
            else
                fillDBColor(dbuf, info.type, info.userBuffer, info.size, info.stride, start, count);
        }
        else
        {
            if ( indexedMode )
                afl::fillDB_ind(dbuf, info.type, info.userBuffer, info.size, info.stride, iSet);
            else
                afl::fillDB(dbuf, info.type, info.userBuffer, info.size, info.stride, start, count);
        }
        va.md = afl::sendDB(dbuf, driver);
        va.componentsType = SD_FLOAT; // SD_FLOAT forced
        va.stride = 0; // One single buffer per attribute
    }

    driver->configureVertexAttribute(va);

#ifdef GLLIB_DUMP_STREAMS
    ofstream out;
    out.open("GLLibDumpStreams.txt",ios::app);

    if ( !out.is_open() )
        panic("RegisterWriteBuffer", "dumpRegisterStatus", "Dump failed (output file could not be opened)");
        
        out << " Dumping Stream " << va.stream << endl;
        out << "\t Stride: " << va.stride << endl;
        out << "\t Data type: " << va.componentsType << endl;
        out << "\t Num. elemets: " << va.components << endl;
        out << "\t Offset: " << va.offset << endl;
        out << "\t Dumping stream content: " << endl;
        
        if (info.bufferID == 0)
            dbuf.dump(out);
        else
            dumpBuffer(out, auxData, va.components, va.componentsType, va.offset, va.stride, count );
            
        out << endl;
        out.close();
#endif

    if ( info.bufferID == 0 )
        return va.md; // Unbound buffer, md must be released after being used

    return 0; // VBO must not release its memory descriptor
}

std::vector<GLuint> afl::setVertexArraysBuffers(GLContext* ctx, ::GPUDriver* driver,
                                                GLuint start, GLuint count,
                                                std::set<GLuint>& iSet, GLuint& nextFreeStream)
{
    int stream = 0; // first free stream
    vector<GLuint> clientDescriptors;
    GLContext::DBUF dbuf;

    GLuint md;

    // Reset all attributes
    GPUDriver::VertexAttribute vaDisabled;
    vaDisabled.enabled = false;
    for ( GLuint i = 0; i < driver->getMaxVertexAttribs(); i++ )
    {
        vaDisabled.attrib = (GPUDriver::ShAttrib)i;
        driver->configureVertexAttribute(vaDisabled);
    }

    // Note:
    // If iSet is empty configureStream assumes sequential access
    // If iSet is NOT empty configureStream assumes indexed access


    if ( ctx->testFlags(GLContext::flagVaPos) )
    {
        GLContext::VArray& info = ctx->posVArray();
        md = configureStream(ctx, driver, info, GPUDriver::VS_VERTEX, stream, false, start, count, iSet);
        if ( md != 0 )
            clientDescriptors.push_back(md);
        stream++;
    }

    if ( ctx->testFlags(GLContext::flagVaCol) )
    {
        GLContext::VArray& info = ctx->colVArray();
        md = configureStream(ctx, driver, info, GPUDriver::VS_COLOR, stream, true, start, count, iSet);
        if ( md != 0 )
            clientDescriptors.push_back(md);
        stream++;
    }

    if ( ctx->testFlags(GLContext::flagVaNor) )
    {
        GLContext::VArray& info = ctx->norVArray();
        md = configureStream(ctx, driver, info, GPUDriver::VS_NORMAL, stream, false, start, count, iSet);
        if ( md != 0 )
            clientDescriptors.push_back(md);
        stream++;
    }

    // Copy client texture coordinates
    for ( GLuint i = 0; i < ctx->countTextureUnits(); i++ )
    {
        if ( ctx->isEnabledClientTextureUnit(i) )
        {
            GLContext::VArray& info = ctx->texVArray(i);
            md = configureStream(ctx, driver, info, GPUDriver::ShAttrib(GPUDriver::VS_TEXTURE_0 + i),
                                 stream, false, start, count, iSet);
            if ( md != 0 )
                clientDescriptors.push_back(md);
            stream++;
        }
    }

    if ( ctx->testFlags(GLContext::flagVS) )
    {
        for ( GLuint i = 0; i < ctx->countGenericVArrays(); i++ )
        {
            if ( ctx->isGenericAttribArrayEnabled(i) )
            {
                GLContext::VArray& info = ctx->genericVArray(i);
                md = configureStream(ctx, driver, info, (GPUDriver::ShAttrib)i, stream, false, start, count, iSet);
                if ( md != 0 )
                    clientDescriptors.push_back(md);
                stream++;
            }
        }
    }

    nextFreeStream = stream;
    return clientDescriptors; // Descriptors that must be released after use them
}


#define FIXED_FP_PID 0x255
#define FIXED_VS_PID 0x256

void afl::setupShaders(GLContext* ctx, ::GPUDriver* driver)
{
    if ( ctx->testFlags(GLContext::flagStencilTest) && ctx->checkClearStencilWarning() )
    {
        //cout << "afl::setupShaders -> Stencil buffer not cleared previously and now is enabled. Error ignored..." << endl;
        //panic("AuxFuncsLib", "setupShaders", "Stencil buffer not clear previously and now is enabled");
    }

    ProgramObject *vp;
    ProgramObject *fp;

    // SET_LIB_DIRTY; // Force rebuilt programs each batch (old mode)

    VSLoader& vsl = ctx->getShaderSetup(); // Object for setup a ProgramObject

    if ( !ctx->testFlags(GLContext::flagVS) ) // Emulate T&L via Vertex program
    {
        if( IS_LIB_DIRTY ) // check is it is required to rebuilt
        {
            TLShader* tlsh = TLFactory::constructVertexProgram(ctx->getTLState());
            bool cached;
            // Try to find a previous automatic code generation with the same generated code
            // or create a new one if it is not found
            vp = ctx->vpCache.get(tlsh->getCode(), cached);
            if ( vp == 0 ) // automatically generated vertex programs cache full
            {
                vector<ProgramObject*> toRemove = ctx->vpCache.clear();
                vector<ProgramObject*>::iterator it = toRemove.begin();
                for ( ; it != toRemove.end(); it++ )
                {
                    ctx->gpumem().deallocate(*(*it)); // deallocate from GPU memory if required
                    delete *it; // delete from main memory
                }
                vp = ctx->vpCache.get(tlsh->getCode(), cached); // force creation of a new ProgramObject
            }
            vp->setTarget(ProgramManager::instance().getTarget(GL_VERTEX_PROGRAM_ARB));
            tlsh->setup(*vp, *ctx, cached);  // copy program locals and target environment register values
            delete tlsh;
        }
        else
            vp = ctx->vpCache.getLastUsed();
    }
    else
        vp = &GET_VERTEX_PROGRAM;

    if ( !ctx->testFlags(GLContext::flagFS) ) // Emulate fragment functions & texture operations via fragment program
    {
        if ( IS_LIB_DIRTY ) // check is it is required to rebuilt
        {
            TLShader* tlsh = FPFactory::constructFragmentProgram(ctx->getFPState());
            bool cached;
            // Try to find a previous automatic code generation with the same generated code
            // or create a new one if it is not found
            fp = ctx->fpCache.get(tlsh->getCode(), cached);
            if ( fp == 0 ) // automatically generated fragment programs cache full
            {
                vector<ProgramObject*> toRemove = ctx->fpCache.clear();
                vector<ProgramObject*>::iterator it = toRemove.begin();
                for ( ; it != toRemove.end(); it++ )
                {
                    ctx->gpumem().deallocate(*(*it)); // deallocate from GPU memory if required
                    delete *it; // delete from main memory
                }
                fp = ctx->fpCache.get(tlsh->getCode(), cached); // force creation of a new ProgramObject
            }
            fp->setTarget(ProgramManager::instance().getTarget(GL_FRAGMENT_PROGRAM_ARB));
            tlsh->setup(*fp, *ctx, cached);
            delete tlsh;
        }
        else
            fp = ctx->fpCache.getLastUsed();
    }
    else
    {
        if (ctx->testFlags(GLContext::flagAlphaTest))
            panic("AuxFuncLib", "setupShaders", "Alpha Test and User Fragment program not supported yet");

        fp = &GET_FRAGMENT_PROGRAM;
    }


    /*  Check if triangle setup shader program is updated.  */
    if (!ctx->isSetupShaderProgramUpdated())
    {
        /*  Store the triangle setup shader and constants in the vertex shader partition.  */
        ProgramObject tp(0, GL_VERTEX_PROGRAM_ARB);
        CodeSnip tSetupCode;
        string tSetupText;

        /*  Build triangle setup shader.  */
        tSetupText = "!!ARBvp1.0\n"
                     "\n"
                     "# Definitions"
                     "\n"
                     "ATTRIB iX = vertex.attrib[0];\n"
                     "ATTRIB iY = vertex.attrib[1];\n"
                     "ATTRIB iZ = vertex.attrib[2];\n"
                     "ATTRIB iW = vertex.attrib[3];\n"
                     "\n"
                     "PARAM rFactor1 = program.env[0];\n"
                     "PARAM rFactor2 = program.env[1];\n"
                     "PARAM rFactor3 = program.env[2];\n"
                     "PARAM rFactor4 = program.env[3];\n"
                     "PARAM offset = program.env[4];\n"
                     "\n"
                     "OUTPUT oA = result.position;\n"
                     "OUTPUT oB = result.color.front.primary;\n"
                     "OUTPUT oC = result.color.front.secondary;\n"
                     "OUTPUT oArea = result.color.back.primary;\n"
                     "\n"
                     "TEMP rA, rB, rC, rArea, rT1, rT2;\n"
                     "# Code\n"
                     "\n"
                     "# Calculate setup matrix/edge equations.\n"
                     "\n"
                     "MUL rC.xyz, iX.zxyw, iY.yzxw;\n"
                     "MUL rB.xyz, iX.yzxw, iW.zxyw;\n"
                     "MUL rA.xyz, iY.zxyw, iW.yzxw;\n"
                     "MAD rC.xyz, iX.yzxw, iY.zxyw, -rC;\n"
                     "MAD rB.xyz, iX.zxyw, iW.yzxw, -rB;\n"
                     "MAD rA.xyz, iY.yzxw, iW.zxyw, -rA;\n"
                     "\n";

        /*  Check current front face mode.  */
        if (ctx->getFaceMode() == GL_CW)
        {
            tSetupText += "# Convert from CW to CCW vertex ordering.\n"
                         "\n"
                         "MOV rA, -rA;\n"
                         "MOV rB, -rB;\n"
                         "MOV rC, -rC;\n"
                         "\n";
        }

        tSetupText += "# Calculate signed 'area' for the triangle.\n"
                     "\n"
                     "DP3 rArea, rC, iW;\n"
                     "RCP rT2, rArea.x;\n"
                     "\n"
                     "# Calculate Z interpolation equation.\n"
                     "\n"
                     "DP3 rA.w, rA, iZ;\n"
                     "DP3 rB.w, rB, iZ;\n"
                     "DP3 rC.w, rC, iZ;\n"
                     "\n"
                     "# Apply viewport transformation to functions.\n"
                     "\n"
                     "MUL rT1, rA, rFactor3;\n"
                     "MAD rT1, rB, rFactor4, rT1;\n"
                     "ADD rC, rC, rT1;\n"
                     "MUL rA, rA, rFactor1;\n"
                     "MUL rB, rB, rFactor2;\n"
                     "MUL rA.w, rA.w, rT2.x;\n"
                     "MUL rB.w, rB.w, rT2.x;\n"
                     "MUL rC.w, rC.w, rT2.x;\n"
                     "\n"
                     "# Apply half pixel sample point offset.\n"
                     "\n"
                     "MUL rT1, rA, offset;\n"
                     "MAD rT1, rB, offset, rT1;\n"
                     "ADD rC, rC, rT1;\n"
                     "\n"
                     "# Write output registers.\n"
                     "\n"
                     "MOV oA, rA;\n"
                     "MOV oB, rB;\n"
                     "MOV oC, rC;\n"
                     "MOV oArea.x, rArea.x;\n"
                     "END";

        tSetupCode.clear();
        tSetupCode.append(tSetupText);

        tp.setTarget(ProgramManager::instance().getTarget(GL_VERTEX_PROGRAM_ARB));
        tp.setSource(tSetupCode.toString().c_str(), tSetupCode.toString().length());
        tp.setFormat(GL_PROGRAM_FORMAT_ASCII_ARB);

        tp.compile();

        ctx->gpumem().allocate(tp);

        driver->commitVertexProgram(ctx->gpumem().md(tp), tp.getBinarySize(), 384);

        /*  Sets triangle setup shader program as updated.  */
        ctx->setSetupShaderProgramUpdate(true);
    }

    /*  Check if triangle setup shader constants are updated.  */
    if (!ctx->areSetupShaderConstantsUpdated())
    {
        gpu3d::GPURegData data;
        GLint x, y;
        GLsizei w, h;

        ctx->getViewport(x, y, w, h);

        data.qfVal[0] = 2.0f / f32bit(w);
        data.qfVal[1] = 2.0f / f32bit(w);
        data.qfVal[2] = 2.0f / f32bit(w);
        data.qfVal[3] = 2.0f / f32bit(w);
        driver->writeGPURegister(gpu3d::GPU_VERTEX_CONSTANT, 200, data);

        data.qfVal[0] = 2.0f / f32bit(h);
        data.qfVal[1] = 2.0f / f32bit(h);
        data.qfVal[2] = 2.0f / f32bit(h);
        data.qfVal[3] = 2.0f / f32bit(h);
        driver->writeGPURegister(gpu3d::GPU_VERTEX_CONSTANT, 201, data);

        data.qfVal[0] =  data.qfVal[1] =  data.qfVal[2] =  data.qfVal[3] = -(((f32bit(x) * 2.0f) / f32bit(w)) + 1);
        driver->writeGPURegister(gpu3d::GPU_VERTEX_CONSTANT, 202, data);

        data.qfVal[0] =  data.qfVal[1] =  data.qfVal[2] =  data.qfVal[3] = -(((f32bit(y) * 2.0f) / f32bit(h)) + 1);
        driver->writeGPURegister(gpu3d::GPU_VERTEX_CONSTANT, 203, data);

        data.qfVal[0] =  data.qfVal[1] =  data.qfVal[2] =  data.qfVal[3] = 0.5f;
        driver->writeGPURegister(gpu3d::GPU_VERTEX_CONSTANT, 204, data);

        /*  Set as updated.  */
        ctx->setSetupShaderConstantsUpdate(true);
    }

    vsl.initShader(*vp, GL_VERTEX_PROGRAM_ARB, ctx);
    vsl.initShader(*fp, GL_FRAGMENT_PROGRAM_ARB, ctx);

    // Set input/output registers
    const ProgramObject::ProgramResources& vpres = vp->getResources();
    const ProgramObject::ProgramResources& fpres = fp->getResources();

    GPURegData bTrue, bFalse;

    bTrue.booleanVal = true;
    bFalse.booleanVal = false;

    for ( int i = 0; i < ProgramObject::MaxProgramAttribs; i++ )
    {
        if ( vpres.outputAttribsWritten[i] )
            driver->writeGPURegister(GPU_VERTEX_OUTPUT_ATTRIBUTE, i, bTrue);
        else
            driver->writeGPURegister(GPU_VERTEX_OUTPUT_ATTRIBUTE, i, bFalse);

        if ( fpres.inputAttribsRead[i] )
            driver->writeGPURegister(GPU_FRAGMENT_INPUT_ATTRIBUTES, i, bTrue);
        else
            driver->writeGPURegister(GPU_FRAGMENT_INPUT_ATTRIBUTES, i, bFalse);
    }

    // If fragment depth is modified by the fragment program disable Early Z
    if (fpres.outputAttribsWritten[0] || ctx->testFlags(GLContext::flagAlphaTest))
        driver->writeGPURegister(GPU_EARLYZ, bFalse);
    else
        driver->writeGPURegister(GPU_EARLYZ, bTrue);


    SET_LIB_NO_DIRTY; // library is coherent with respect to current shaders
}

/*  Setup the Hierarchical Z Test.  */
void afl::setupHZTest(GLContext* ctx, ::GPUDriver* driver)
{
    bool activateHZ = false;
    bool deactivateHZ = false;

    GLenum stFail;
    GLenum dpFail;
    GLenum dpPass;

    GLenum depthFunc;

    /*  Get depth test function.  */
    depthFunc = ctx->getDepthFunc();

    /*  Get stencil test update functions.  */
    ctx->getStencilOp(stFail, dpFail, dpPass);

    /*  Check if z test is enabled.  */
    if (ctx->testFlags(GLContext::flagDepthTest))
    {
        /*  Determine if HZ must be activated.  */
        activateHZ = (!ctx->isHZTestActive()) && ctx->isHZBufferValid() &&
            ((depthFunc == GL_LESS) || (depthFunc == GL_LEQUAL) || (depthFunc == GL_EQUAL));

        /*  Determine if HZ must be deactivated.  */
        deactivateHZ = (depthFunc != GL_LESS) && (depthFunc != GL_LEQUAL) && (depthFunc != GL_EQUAL);

        /*  Check if stencil test is enabled.  */
        if (ctx->testFlags(GLContext::flagStencilTest))
        {
            /*  Determine if HZ must be activated.  */
            activateHZ = activateHZ && (stFail == GL_KEEP) && (dpFail == GL_KEEP);

            /*  Determine if HZ must be deactivated.  */
            deactivateHZ = deactivateHZ || (stFail != GL_KEEP) || (dpFail != GL_KEEP);
        }

        /*  Determine if HZ must be deactivated.  */
        deactivateHZ = deactivateHZ && ctx->isHZTestActive();

        /*  Check if HZ buffer content is invalid.  */
        if (ctx->getDepthMask() && ((depthFunc == GL_ALWAYS) || (depthFunc == GL_GREATER) ||
            (depthFunc == GL_GEQUAL) || (depthFunc == GL_NOTEQUAL)))
        {
            /*  Set HZ buffer content as not valid.  */
            ctx->setHZBufferValidFlag(false);
        }
    }
    else
    {
        /*  Determine if HZ must be deactivated.  */
        deactivateHZ = ctx->isHZTestActive();
    }


    /*  Check if HZ test must be activated.  */
    if (activateHZ)
    {
        GPURegData data;
        data.booleanVal = true;
        driver->writeGPURegister(GPU_HIERARCHICALZ, data);

        /*  Set HZ test as activated.  */
        ctx->setHZTestActiveFlag(true);
    }

    /*  Check if HZ test must be deactivated.  */
    if (deactivateHZ)
    {
        GPURegData data;
        data.booleanVal = false;
        driver->writeGPURegister(GPU_HIERARCHICALZ, data);

        /*  Set HZ test as deactivated.  */
        ctx->setHZTestActiveFlag(false);
    }
}

/**
 *     Fragment Attribute Binding  Components  Underlying State                 Register
 *     --------------------------  ----------  ----------------------------  --------
 *     fragment.position           (x,y,z,1/w) window position                    0
 *     fragment.color              (r,g,b,a)   primary color                    1
 *     fragment.color.primary      (r,g,b,a)   primary color                    1
 *     fragment.color.secondary    (r,g,b,a)   secondary color                    2
 *     fragment.fogcoord           (f,0,0,1)   fog distance/coordinate            3
 *     fragment.texcoord           (s,t,r,q)   texture coordinate, unit 0        4
 *     fragment.texcoord[n]        (s,t,r,q)   texture coordinate, unit n        4+n
 */
void afl::setFragmentAttributes(GPUDriver* driver, GLContext* ctx)
{
    const ProgramObject::ProgramResources& vpres = GET_VERTEX_PROGRAM.getResources();
    const ProgramObject::ProgramResources& fpres = GET_FRAGMENT_PROGRAM.getResources();

    GPURegData data;

    data.booleanVal = true;
    driver->writeGPURegister( GPU_FRAGMENT_INPUT_ATTRIBUTES, 0, data );
    driver->writeGPURegister( GPU_FRAGMENT_INPUT_ATTRIBUTES, 1, data );

   if ( ctx->testFlags(GLContext::flagSeparateSpecular) )
        data.booleanVal = true;
    else
        data.booleanVal = false;
    driver->writeGPURegister(GPU_FRAGMENT_INPUT_ATTRIBUTES, 2, data);

    /* Enable or disable fog coordinate attribute  */

    if ( ctx->testFlags(GLContext::flagFog) )
        data.booleanVal = true;
    else
        data.booleanVal = false;
    driver->writeGPURegister(GPU_FRAGMENT_INPUT_ATTRIBUTES, 3, data);

    GLuint i;
    for ( i = 4; (i-4) < ctx->countTextureUnits() && i < MAX_FRAGMENT_ATTRIBUTES; i++ )
    {

        if ( ctx->getTextureUnit(i-4).getTarget() != 0 )
            data.booleanVal = true;
        else
            data.booleanVal = false;
        driver->writeGPURegister(GPU_FRAGMENT_INPUT_ATTRIBUTES, i, data);
    }
    // i cannot be modified, it's used in the next loop

    // Reset the remaning fragment attributes (if any)
    data.booleanVal = false;
    for (; i < MAX_FRAGMENT_ATTRIBUTES; i++ )
        driver->writeGPURegister(GPU_FRAGMENT_INPUT_ATTRIBUTES, i, data);

}

void afl::synchronizeTextureImages(GLContext* ctx, ::GPUDriver* driver, bool& disableColorMask)
{
    if ( ctx->countTextureUnits() != ProgramObject::MaxTextureUnits )
    {
        cout << "CountTextureUnits: " << ctx->countTextureUnits() << endl;
        cout << "ProgramObject::MaxTextureUnits: " << ProgramObject::MaxTextureUnits << endl;
        panic("AuxFuncsLib", "synchronizeTextureImages", "Inconsistent definitions for MaxTextureUnits");
    }

    disableColorMask = false;

    if ( ctx->testFlags(GLContext::flagFS) )
    {
        ProgramObject& fp = GET_FRAGMENT_PROGRAM;
        fp.compile(); // Compile & compute program resources
        // Obtain texture unit accesses (glEnable/glDisable texture hierarchy ignored)
        ProgramObject::ProgramResources& pr = fp.getResources();
        for ( GLuint i = 0; i < ProgramObject::MaxTextureUnits; i++ )
        {
            if ( pr.textureTargets[i] != 0 )
            {
                TextureUnit& tu = ctx->getTextureUnit(i);
                TextureObject* to = tu.getTextureObject(pr.textureTargets[i]);
                if ( !to )
                    panic("AuxFuncsLib", "synchronizedTextureImages", "Texture object not attached to this texture unit");

                /**********************************************************************************
                 *              CopyTex{Sub}Image patch used for Chronicles of Riddick:           *
                 *                 Until this call is fully supported a batch that                *
                 *                 uses a Texture based on FrameBuffer contents is                *
                 *                 ignored.                                                       *
                 **********************************************************************************/

                /*  Check if TextureObject contains a FrameBuffer based Texture Mipmap.       *
                 *  In this case, ColorBuffer write will be disabled,to ignore the batch.     */

                //if (to->getState() == BaseObject::Blit)
                //{
                //    disableColorMask = true;
                //}
                // Configure texture unit
                to->sync(); // synchronize internal texture state
                GLubyte filename[30];
                sprintf((char*)filename, "Sampler%d", i);
#ifdef GLLIB_DUMP_SAMPLERS
                to->dump(filename);
#endif
                ctx->gpumem().allocate(*to);
                setupTexture(ctx, driver, *to, i, pr.textureTargets[i]);
            }
        }
    }
    else if ( ctx->areTexturesEnabled() ) // Copy texture images to GPU
    {
        for ( GLuint i = 0; i < ctx->countTextureUnits(); i++ )
        {
            TextureUnit& tu = ctx->getTextureUnit(i);
            TextureObject* to = tu.getTextureObject();
            if ( to != 0 )
            {
                
                 /**********************************************************************************
                 *              CopyTex{Sub}Image patch used for Chronicles of Riddick:           *
                 *                 Until this call is fully supported a batch that                *
                 *                 uses a Texture based on FrameBuffer contents is                *
                 *                 ignored.                                                       *
                 **********************************************************************************/

                /*  Check if TextureObject contains a FrameBuffer based Texture Mipmap.       *
                 *  In this case, ColorBuffer write will be disabled,to ignore the batch.     */

                //if (to->getState() == BaseObject::Blit)
                //{
                //    disableColorMask = true;
                //}
                //cout << "Texture unit: "  << i << "  Texture object: " << to->getName() << endl;

                to->sync(); // synchronize internal texture state
                GLubyte filename[30];
                sprintf((char*)filename, "Sampler%d", i);
#ifdef GLLIB_DUMP_SAMPLERS
                to->dump(filename);
#endif
                ctx->gpumem().allocate(*to);
                afl::setupTexture(ctx, driver, *to, i);
            }
        }
    }
}


#define CASE_BRANCH_CLAMP(x) case GL_##x: data.txClamp = GPU_TEXT_##x; break;
#define CASE_BRANCH_FILTER(x) case GL_##x: data.txFilter = GPU_##x; break;

// If kind of access is 0, use glEnable/glDisable texture hierarchy
void afl::setupTexture(GLContext* ctx, GPUDriver* driver, TextureObject& to, GLuint texUnit, GLenum kindOfAccess)
{
    GPURegData data;

    /* Set state registers */
    data.booleanVal = true;
    driver->writeGPURegister(GPU_TEXTURE_ENABLE, texUnit, data);

    if ( kindOfAccess == 0 )
        kindOfAccess = to.getTargetName();

    switch ( kindOfAccess )
    {
        case GL_TEXTURE_1D:
            data.txMode = GPU_TEXTURE1D;
            break;
        case GL_TEXTURE_2D:
            data.txMode = GPU_TEXTURE2D;
            break;
        case GL_TEXTURE_3D:
            data.txMode = GPU_TEXTURE3D;
            break;
        case GL_TEXTURE_CUBE_MAP:
            data.txMode = GPU_TEXTURECUBEMAP;
            break;
        default:
            panic("AuxFuncsLib", "setupTexture()", "Unknown texture target");
    }
    driver->writeGPURegister(GPU_TEXTURE_MODE, texUnit, data);

    GLuint maxMipmaps = driver->getMaxMipmaps();
    GLuint maxUnits = driver->getTextureUnits();
    GLuint skipUnits = texUnit * maxMipmaps * 6; // precompute skip units

    GLuint start = to.getMinActiveMipmap();
    GLuint end = to.getMaxActiveMipmap();

    GLuint numberOfPortions = to.getNumberOfPortions();
    GLuint portion = 0;
    GLuint face = 0;
    GLuint level;

    if ( to.getTargetName() == GL_TEXTURE_CUBE_MAP )
    {
        if ( numberOfPortions % 6 != 0 )
            panic("AuxFuncsLib", "setupTexture", "Cubemap without same number of mipmaps per face");

        if ( numberOfPortions != ((end-start+1)*6) )
            panic("AuxFuncsLib", "setupTexture", "Cubemap portions/mipmaps inconsistent");


        for ( u32bit i = 0; i < 6; i++ ) // loop through all 6 face
        {
            level = start;
            for ( ; level <= end; level ++, portion++)
            {
                u32bit md = ctx->gpumem().md(to, portion);
                u32bit pos = skipUnits + 6*level + i; // i == CUBEMAP
                driver->writeGPUAddrRegister(GPU_TEXTURE_ADDRESS, pos, md);
            }
        }
    }
    else // 1D, 2D and 3D
    {
        level = start;
        if ( numberOfPortions != (end-start+1) )
            panic("AuxFuncsLib", "setupTexture", "Texture portions/mipmaps inconsistent");
        for ( ; level <= end; level++, portion++ )
        {
            // configure mipmaps from base to max
            u32bit md = ctx->gpumem().md(to,portion);
            u32bit pos = skipUnits + 6*level;
            driver->writeGPUAddrRegister(GPU_TEXTURE_ADDRESS, pos, md);
        }
    }

    // Set min & max level
    data.uintVal = start;
    driver->writeGPURegister(GPU_TEXTURE_MIN_LEVEL, texUnit, data);
    data.uintVal = end;
    driver->writeGPURegister(GPU_TEXTURE_MAX_LEVEL, texUnit, data);

    // set texture lod bias
    data.f32Val = to.getLodBias();
    driver->writeGPURegister(GPU_TEXTURE_LOD_BIAS, texUnit, data);

    // set texture unit lod bias (!= texture lod bias)
    TextureUnit& tu = ctx->getTextureUnit(texUnit);
    data.f32Val = tu.getLodBias();
    driver->writeGPURegister(GPU_TEXT_UNIT_LOD_BIAS, texUnit, data);

    data.uintVal = to.getWidth();
    driver->writeGPURegister(GPU_TEXTURE_WIDTH, texUnit, data);

    data.uintVal = to.getHeight();
    driver->writeGPURegister(GPU_TEXTURE_HEIGHT, texUnit, data);

    // Compute base mipmap log2(width)
    GLenum target = ( to.getTargetName() == GL_TEXTURE_CUBE_MAP ? GL_TEXTURE_CUBE_MAP_POSITIVE_X : to.getTargetName() );
    data.uintVal = to.getMipmapWidth(target, start);

    data.uintVal = (u32bit) mathlib::ceil2(mathlib::log2(data.uintVal));

    driver->writeGPURegister(GPU_TEXTURE_WIDTH2, texUnit, data);

    // Compute base mipmap log2(height)
    data.uintVal = to.getMipmapHeight(target, start);
    data.uintVal = (u32bit) mathlib::ceil2(mathlib::log2(data.uintVal));
    driver->writeGPURegister(GPU_TEXTURE_HEIGHT2, texUnit, data);

    GPURegData compression;
    switch ( to.getInternalFormat() )
    {
        case GL_RGBA:
            data.txFormat = GPU_RGBA8888;
            compression.txCompression = GPU_NO_TEXTURE_COMPRESSION;
            break;
        case GL_INTENSITY8:
            data.txFormat = GPU_INTENSITY8;
            compression.txCompression = GPU_NO_TEXTURE_COMPRESSION;
            break;
        case GL_ALPHA:
            data.txFormat = GPU_ALPHA8;
            compression.txCompression = GPU_NO_TEXTURE_COMPRESSION;
            break;
        case GL_LUMINANCE:
            data.txFormat = GPU_LUMINANCE8;
            compression.txCompression = GPU_NO_TEXTURE_COMPRESSION;
            break;
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
            data.txFormat = GPU_RGB888;
            compression.txCompression = GPU_S3TC_DXT1_RGB;
            break;
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            data.txFormat = GPU_RGBA8888;
            compression.txCompression = GPU_S3TC_DXT1_RGBA;
            break;
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
            data.txFormat = GPU_RGBA8888;
            compression.txCompression = GPU_S3TC_DXT3_RGBA;
            break;
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            data.txFormat = GPU_RGBA8888;
            compression.txCompression = GPU_S3TC_DXT5_RGBA;
            break;
        default:
            panic("AuxFuncsLib", "setupTexture()", "Unsupported texture format");
    }
    driver->writeGPURegister(GPU_TEXTURE_COMPRESSION, texUnit, compression);
    driver->writeGPURegister(GPU_TEXTURE_FORMAT, texUnit, data);

    data.booleanVal = to.getReverseTextureData();
    driver->writeGPURegister(GPU_TEXTURE_REVERSE, texUnit, data);

    switch ( to.getWrapS() )
    {
        CASE_BRANCH_CLAMP(CLAMP)
        CASE_BRANCH_CLAMP(CLAMP_TO_EDGE)
        CASE_BRANCH_CLAMP(REPEAT)
        CASE_BRANCH_CLAMP(CLAMP_TO_BORDER)
        CASE_BRANCH_CLAMP(MIRRORED_REPEAT)
        default:
            panic("AuxFuncsLib", "setTexture", "Unsupported WRAP_S mode");
    }
    driver->writeGPURegister(GPU_TEXTURE_WRAP_S, texUnit, data);

    switch ( to.getWrapT() )
    {
        CASE_BRANCH_CLAMP(CLAMP)
        CASE_BRANCH_CLAMP(CLAMP_TO_EDGE)
        CASE_BRANCH_CLAMP(REPEAT)
        CASE_BRANCH_CLAMP(CLAMP_TO_BORDER)
        CASE_BRANCH_CLAMP(MIRRORED_REPEAT)
        default:
            panic("AuxFuncsLib", "setTexture", "Unsupported WRAP_T mode");
    }
    driver->writeGPURegister(GPU_TEXTURE_WRAP_T, texUnit, data);

    switch ( to.getMinFilter() )
    {
        CASE_BRANCH_FILTER(NEAREST)
        CASE_BRANCH_FILTER(LINEAR)
        CASE_BRANCH_FILTER(NEAREST_MIPMAP_NEAREST)
        CASE_BRANCH_FILTER(NEAREST_MIPMAP_LINEAR)
        CASE_BRANCH_FILTER(LINEAR_MIPMAP_NEAREST)
        CASE_BRANCH_FILTER(LINEAR_MIPMAP_LINEAR)
        default:
            panic("AuxFuncsLib", "setTexture", "Unsupported MIN_FILTER");
    }
    driver->writeGPURegister(GPU_TEXTURE_MIN_FILTER, texUnit, data);

    switch ( to.getMagFilter() )
    {
        CASE_BRANCH_FILTER(NEAREST)
        CASE_BRANCH_FILTER(LINEAR)
        default:
            panic("AuxFuncsLib", "setTexture", "Unsupported MAG_FILTER");
    }
    driver->writeGPURegister(GPU_TEXTURE_MAG_FILTER, texUnit, data);

    data.uintVal = u32bit(to.getMaxAnisotropy());
    driver->writeGPURegister(GPU_TEXTURE_MAX_ANISOTROPY, texUnit, data);
}

#undef CASE_BRANCH_CLAMP
#undef CASE_BRANCH_FILTER


const char* afl::strIFMT(GLint ifmt)
{
    static const char* s[] =
    {
        "GL_ALPHA",
        "GL_ALPHA4",
        "GL_ALPHA8",
        "GL_ALPHA12",
        "GL_ALPHA16",
        "GL_DEPTH_COMPONENT",
        "GL_DEPTH_COMPONENT16",
        "GL_DEPTH_COMPONENT24",
        "GL_DEPTH_COMPONENT32",
        "GL_LUMINANCE",
        "GL_LUMINANCE4",
        "GL_LUMINANCE8",
        "GL_LUMINANCE12",
        "GL_LUMINANCE16",
        "GL_LUMINANCE_ALPHA",
        "GL_LUMINANCE4_ALPHA4",
        "GL_LUMINANCE6_ALPHA2",
        "GL_LUMINANCE8_ALPHA8",
        "GL_LUMINANCE12_ALPHA4",
        "GL_LUMINANCE12_ALPHA12",
        "GL_LUMINANCE16_ALPHA16",
        "GL_INTENSITY",
        "GL_INTENSITY4",
        "GL_INTENSITY8",
        "GL_INTENSITY12",
        "GL_INTENSITY16",
        "GL_RGB",
        "GL_R3_G3_B2",
        "GL_RGB4",
        "GL_RGB5",
        "GL_RGB8",
        "GL_RGB10",
        "GL_RGB12",
        "GL_RGB16",
        "GL_COMPRESSED_RGB_S3TC_DXT1_EXT",
        "GL_RGBA",
        "GL_RGBA2",
        "GL_RGBA4",
        "GL_RGB5_A1",
        "GL_RGBA8",
        "GL_RGB10_A2",
        "GL_RGBA12",
        "GL_RGBA16",
        "GL_COMPRESSED_RGBA_S3TC_DXT1_EXT",
        "GL_COMPRESSED_RGBA_S3TC_DXT3_EXT",
        "GL_COMPRESSED_RGBA_S3TC_DXT5_EXT",
    };

    // For compatibility
    static const char one[] = "1";
    static const char two[] = "2";
    static const char three[] = "3";
    static const char four[] = "4";

    // Returns the hex value when ifmt is unknown
    static char unknown[32];

    switch ( ifmt )
    {
        case 1: return one;
        case 2: return two;
        case 3: return three;
        case 4: return four;
        case GL_ALPHA: return s[0];
        case GL_ALPHA4: return s[1];
        case GL_ALPHA8: return s[2];
        case GL_ALPHA12: return s[3];
        case GL_ALPHA16: return s[4];
        case GL_DEPTH_COMPONENT: return s[5];
        case GL_DEPTH_COMPONENT16: return s[6];
        case GL_DEPTH_COMPONENT24: return s[7];
        case GL_DEPTH_COMPONENT32: return s[8];
        case GL_LUMINANCE:  return s[9];
        case GL_LUMINANCE4:  return s[10];
        case GL_LUMINANCE8:  return s[11];
        case GL_LUMINANCE12:  return s[12];
        case GL_LUMINANCE16:  return s[13];
        case GL_LUMINANCE_ALPHA: return s[14];
        case GL_LUMINANCE4_ALPHA4: return s[15];
        case GL_LUMINANCE6_ALPHA2: return s[16];
        case GL_LUMINANCE8_ALPHA8: return s[17];
        case GL_LUMINANCE12_ALPHA4: return s[18];
        case GL_LUMINANCE12_ALPHA12: return s[19];
        case GL_LUMINANCE16_ALPHA16: return s[20];
        case GL_INTENSITY: return s[21];
        case GL_INTENSITY4: return s[22];
        case GL_INTENSITY8: return s[23];
        case GL_INTENSITY12: return s[24];
        case GL_INTENSITY16: return s[25];
        case GL_RGB: return s[26];
        case GL_R3_G3_B2: return s[27];
        case GL_RGB4: return s[28];
        case GL_RGB5: return s[29];
        case GL_RGB8: return s[30];
        case GL_RGB10: return s[31];
        case GL_RGB12: return s[32];
        case GL_RGB16: return s[33];
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT: return s[34];
        case GL_RGBA: return s[35];
        case GL_RGBA2: return s[36];
        case GL_RGBA4: return s[37];
        case GL_RGB5_A1: return s[38];
        case GL_RGBA8: return s[39];
        case GL_RGB10_A2: return s[40];
        case GL_RGBA12: return s[41];
        case GL_RGBA16: return s[42];
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT: return s[43];
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT: return s[44];
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT: return s[45];
    }

    // Unknown internal format
    sprintf(unknown, "UNKOWN (0x%x)", ifmt);
    return unknown;
}


int afl::sendRawIndices(GPUDriver* driver, const GLvoid* indices, GLenum type, GLsizei count)
{
    GLsizei dataSize = afl::getSize(type) * count;

    GLubyte* ub_ptr = new GLubyte[dataSize];
    GLushort* us_ptr = (GLushort*)ub_ptr;
    GLuint* ui_ptr = (GLuint*)ub_ptr;
    GLint i = 0;

    switch ( type )
    {
        case GL_UNSIGNED_BYTE:
            for ( ; i < count; i++ )
                ub_ptr[i] = ((GLubyte*)indices)[i];
            break;
        case GL_UNSIGNED_SHORT:
            for ( ; i < count; i++ )
                us_ptr[i] = ((GLushort*)indices)[i];
            break;
        case GL_UNSIGNED_INT:
            for ( ; i < count; i++ )
                ui_ptr[i] = ((GLuint*)indices)[i];
            break;
        default:
            panic("AuxFuncsLib", "sendRawIndices", "Unsupported indices type");
    }
    int mIndices = driver->obtainMemory(dataSize);
    driver->writeMemory(mIndices, 0, ub_ptr, dataSize);
    delete[] ub_ptr; // destroy temporary
    return mIndices;

}


int afl::sendIndices(GPUDriver* driver, const GLvoid* indices, GLenum indicesType, GLsizei count, const std::set<GLuint>& usedIndices)
{
    using namespace std;

    GLint i = 0;

    map<GLuint,GLuint> conversion;

    set<GLuint>::const_iterator itSet = usedIndices.begin();
    for ( ; itSet != usedIndices.end(); itSet++, i++ )
        conversion[*itSet] = i; // create a conversion table

    GLsizei dataSize = afl::getSize(indicesType) * count;

    GLubyte* ub_ptr = new GLubyte[dataSize];
    GLushort* us_ptr = (GLushort*)ub_ptr;
    GLuint* ui_ptr = (GLuint*)ub_ptr;

    i = 0;
    switch ( indicesType )
    {
        case GL_UNSIGNED_BYTE:
            for ( ; i < count; i++ )
                ub_ptr[i] = (GLubyte)conversion[((GLubyte*)indices)[i]];
            break;
        case GL_UNSIGNED_SHORT:
            for ( ; i < count; i++ )
                us_ptr[i] = (GLushort)conversion[((GLushort*)indices)[i]];
            break;
        case GL_UNSIGNED_INT:
            for ( ; i < count; i++ )
                ui_ptr[i] = (GLuint)conversion[((GLuint*)indices)[i]];
            break;
        default:
            panic("AuxFuncsLib", "sendIndices", "Unsupported indices type");
    }

    int mIndices = driver->obtainMemory(dataSize);
    driver->writeMemory(mIndices, 0, ub_ptr, dataSize);
    delete[] ub_ptr; // destroy temporary
    return mIndices;

}


int afl::sendIndices(GPUDriver* driver, const GLvoid* indices, GLenum indicesType, GLsizei count, GLint offset)
{
    GLsizei dataSize = afl::getSize(indicesType)*count;

    GLubyte* ub_ptr = new GLubyte[dataSize];
    GLushort* us_ptr = (GLushort*)ub_ptr;
    GLuint* ui_ptr = (GLuint*)ub_ptr;

    int i;

    switch ( indicesType )
    {
        case GL_UNSIGNED_BYTE:
            for ( i = 0; i < count; i++ )
                ub_ptr[i] = ((GLubyte*)(indices))[i] + offset;
            break;
        case GL_UNSIGNED_SHORT:
            for ( i = 0; i < count; i++ )
                us_ptr[i] = ((GLushort*)(indices))[i] + offset;
            break;
        case GL_UNSIGNED_INT:
            for ( i = 0; i < count; i++ )
                ui_ptr[i] = ((GLuint*)(indices))[i] + offset;
            break;
        default:
            panic("AuxFuncsLib", "sendIndices", "Unsupported indices type");
    }

    int mIndices = driver->obtainMemory(dataSize);
    driver->writeMemory(mIndices, 0, ub_ptr, dataSize);
    delete[] ub_ptr; // destroy temporary
    return mIndices;
}

void afl::getIndicesSet(std::set<GLuint>& iSet, GLenum type, const GLvoid* indices, GLsizei count)
{
    GLubyte* ub_ptr = (GLubyte*)indices;
    GLushort* us_ptr = (GLushort*)indices;
    GLuint* ui_ptr = (GLuint*)indices;

    int i = 0;

    switch ( type )
    {
        case GL_UNSIGNED_BYTE:
            for ( ; i < count; i++ )
                iSet.insert(ub_ptr[i]);
            break;
        case GL_UNSIGNED_SHORT:
            for ( ; i < count; i++ )
                iSet.insert(us_ptr[i]);
            break;
        case GL_UNSIGNED_INT:
            for ( ; i < count; i++ )
                iSet.insert(ui_ptr[i]);
            break;
        default:
            panic("AuxFuncsLib", "getIndicesSet()", "Unsupported type");
    }

}

void afl::releaseObjects(BaseManager& bm, GLsizei n, const GLuint* names)
{
    for ( int i = 0; i < n; i++ )
    {
        BaseObject* bo = bm.findObject(names[i]);

        if ( bo )
        {
            if ( ctx->gpumem().isPresent(*bo) )
                 ctx->gpumem().deallocate(*bo);
        }

    }
    bm.removeObjects(n,names);
}


void afl::normalizeData(const GLvoid* dataIn, GLfloat* dataOut, GLsizei count, GLenum type)
{
    switch ( type )
    {
        case GL_BYTE:
            for (int i=0; i<count; i++)
                dataOut[i] = ((GLfloat)2*((GLbyte*)dataIn)[i] + 1)/(float)0x0FF;
            break;
        case GL_SHORT:
            for (int i=0; i<count; i++)
                dataOut[i] = ((GLfloat)2*((GLshort*)dataIn)[i] + 1)/(float)0x0FFFF;
            break;
        case GL_INT:
            for (int i=0; i<count; i++)
                dataOut[i] = ((GLfloat)2*((GLint*)dataIn)[i] + 1)/(float)0x0FFFFFFFF;
            break;
        case GL_UNSIGNED_BYTE:
            for(int i=0; i<count; i++)
                dataOut[i] = (GLfloat)((GLubyte*)dataIn)[i]/(float)0x0FF;
            break;
        case GL_UNSIGNED_SHORT:
            for(int i=0; i<count; i++)
                dataOut[i] = (GLfloat)((GLushort*)dataIn)[i]/(float)0x0FFFF;
            break;
        case GL_UNSIGNED_INT:
            for(int i=0; i<count; i++)
                dataOut[i] = (GLfloat)((GLuint*)dataIn)[i]/(float)0x0FFFFFFFF;
            break;
        case GL_FLOAT:
            for(int i=0; i<count; i++)
                dataOut[i] = ((GLfloat*)dataIn)[i];
            break;
        case GL_DOUBLE:
            for(int i=0; i<count; i++)
                dataOut[i] = (GLfloat)((GLdouble*)dataIn)[i];
            break;
        default:
        {
            char msg[64];
            sprintf(msg, "Unknown OpenGL type. Type == 0x%x", type);
            panic("AuxFuncLib", "normalizeData", msg);
        }
    }
}


void afl::clearStencilOnly(GLContext *ctx)
{
    GLboolean saved_red, saved_green, saved_blue, saved_alpha;
    GLboolean saved_depth_mask;
    GLboolean saved_enable_depth;
    GLint saved_stencilBufferClearValue;
    GLuint saved_stencilBufferUpdateMask;
    GLenum saved_stencilBufferFailFunc;
    GLenum saved_stencilBufferZFailFunc;
    GLenum saved_stencilBufferZPassFunc;
    GLenum saved_stencilBufferFunc;
    GLint saved_stencilBufferReferenceValue;
    GLuint saved_stencilBufferCompareMask;
    GLboolean saved_enable_stencil;
    GLuint saved_matrix_mode;
    GLboolean saved_enable_vertex_program;
    GLboolean saved_enable_fragment_program;
    ProgramObject* saved_current_vertex_program;
    ProgramObject* saved_current_fragment_program;
    GLboolean saved_enable_cullface;

    GPURegData data;

    /*******************************************
     * SAVE STATE THAT IS GOING TO BE MODIFIED
     *******************************************/

    // SAVE COLOR MASK
    ctx->getColorMask(saved_red,saved_green,saved_blue,saved_alpha);

    // SAVE DEPTH MASK AND DEPTH TEST
    saved_depth_mask = ctx->getDepthMask();
    saved_enable_depth = ctx->testFlags(GLContext::flagDepthTest);

    // SAVE STENCIL TEST PARAMETERS
    saved_stencilBufferFunc = ctx->getStencilBufferFunc();
    saved_stencilBufferReferenceValue = ctx->getStencilBufferReferenceValue();
    saved_stencilBufferCompareMask = ctx->getStencilBufferCompareMask();
    saved_stencilBufferClearValue = ctx->getStencilBufferClearValue();
    saved_stencilBufferUpdateMask = ctx->getStencilBufferUpdateMask();
    ctx->getStencilOp(saved_stencilBufferFailFunc, saved_stencilBufferZFailFunc, saved_stencilBufferZPassFunc);
    saved_enable_stencil = ctx->testFlags(GLContext::flagStencilTest);

    // SAVE CURRENT MATRIX
    saved_matrix_mode = ctx->getCurrentMatrix();

    // PUSH TRANSFORMATION MATRICES
    ctx->setCurrentMatrix(GL_PROJECTION);
    ctx->mpush(ctx->mtop());
    ctx->setCurrentMatrix(GL_MODELVIEW);
    ctx->mpush(ctx->mtop());

    // SAVE SHADER PROGRAMS
    saved_enable_vertex_program = ctx->testFlags(GLContext::flagVS);
    saved_enable_fragment_program = ctx->testFlags(GLContext::flagFS);

    if (saved_enable_vertex_program)
        saved_current_vertex_program = &GET_VERTEX_PROGRAM;

    if (saved_enable_fragment_program)
        saved_current_fragment_program = &GET_FRAGMENT_PROGRAM;

    // SAVE CULLING STATE
    saved_enable_cullface = ctx->testFlags(GLContext::flagCullFace);

    /*************
     * END SAVING
     *************/

    /******************************************
     * CONFIGURE NEEDED STATE TO WRITE STENCIL
     ******************************************/

    // DISABLE COLOR MASK
    ctx->setColorMask(false, false, false, false);
    data.booleanVal = false;
    driver->writeGPURegister(GPU_COLOR_MASK_R, data);
    driver->writeGPURegister(GPU_COLOR_MASK_G, data);
    driver->writeGPURegister(GPU_COLOR_MASK_B, data);
    driver->writeGPURegister(GPU_COLOR_MASK_A, data);

    // DISABLE DEPTH MASK
    ctx->setDepthMask(false);
    data.booleanVal = false;
    driver->writeGPURegister(GPU_DEPTH_MASK, data);

    // DISABLE FACE CULLING
    ctx->testAndResetFlags(GLContext::flagCullFace);
    data.culling = NONE;
    driver->writeGPURegister(GPU_CULLING, data);

    // REPLACE STENCIL VALUES TO CLEAR VALUE
    ctx->setStencilBufferFunc(GL_ALWAYS, ctx->getStencilBufferClearValue(), 255);
    ctx->setStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    ctx->setFlags(GLContext::flagStencilTest);
    data.compare = GPU_ALWAYS;
    driver->writeGPURegister(GPU_STENCIL_FUNCTION, data);
    data.uintVal = (u32bit)ctx->getStencilBufferClearValue();
    driver->writeGPURegister(GPU_STENCIL_REFERENCE, data);
    data.uintVal = 255;
    driver->writeGPURegister(GPU_STENCIL_COMPARE_MASK, data);
    data.stencilUpdate = STENCIL_REPLACE;
    driver->writeGPURegister(GPU_STENCIL_FAIL_UPDATE, data);
    driver->writeGPURegister(GPU_DEPTH_FAIL_UPDATE, data);
    driver->writeGPURegister(GPU_DEPTH_PASS_UPDATE, data);
    data.booleanVal = true;
    driver->writeGPURegister(GPU_STENCIL_TEST, data);

    // DISABLE DEPTH TEST
    ctx->resetFlags(GLContext::flagDepthTest);
    data.booleanVal = false;
    driver->writeGPURegister(GPU_DEPTH_TEST, data);

    // MODELVIEW = IDENTITY MATRIX
    ctx->setCurrentMatrix(GL_MODELVIEW);
    ctx->ctop(Matrixf::identity());

    // PROJECTION = glOrtho( 0.0, 1.0, 0.0, 1.0, -1.0, 1.0 );
    Matrixf mat;
    mat[0][0] = mat[1][1] = 2.0F;
    mat[1][0] = mat[2][0] = mat[3][0] = mat[0][1] = mat[2][1] = mat[3][1] = mat[0][2] = mat[1][2] = mat[3][2] = 0.0F;
    mat[2][2] = -1.0F;
    mat[0][3] = mat[1][3] = -1.0F;
    mat[2][3] = 0.0F;
    mat[3][3] = 1.0F;
    ctx->setCurrentMatrix(GL_PROJECTION);
    ctx->ctop(mat);

    /*************
       DRAW QUAD
     ************/

    ctx->setFlags(GLContext::flagBeginEnd);
    ctx->setPrimitive(GL_QUADS);
    ctx->clearBuffers();    /* Clean all previous contents */

    data.primitive = QUAD;
    driver->writeGPURegister(GPU_PRIMITIVE, data);

    afl::glVertex(ctx, 0.0f, 0.0f);
    afl::glVertex(ctx, 1.0f, 0.0f);
    afl::glVertex(ctx, 1.0f, 1.0f);
    afl::glVertex(ctx, 0.0f, 1.0f);

    ctx->resetFlags(GLContext::flagBeginEnd);

    u32bit mdVertex = 0;
    u32bit mdColor = 0;
    u32bit mdNormal = 0;
    vector<u32bit> mdTextures; // Texture unit memory descriptor mdTextures[i] for texUnit[i]
    vector<u32bit> texUnit; // Texture unit id's

    ProgramManager::instance().getTarget(GL_VERTEX_PROGRAM_ARB).setDefaultAsCurrent();
    ProgramManager::instance().getTarget(GL_FRAGMENT_PROGRAM_ARB).setDefaultAsCurrent();

    ctx->testAndSetFlags(GLContext::flagVS);
    ctx->testAndSetFlags(GLContext::flagFS);

    afl::setupShaders(ctx, driver);

    /*  Setup Hierarchical Z Test.  */
    afl::setupHZTest(ctx, driver);

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

    const ProgramObject::ProgramResources& vpres = GET_VERTEX_PROGRAM.getResources();

    GLuint stream = 0;

    if ( vpres.inputAttribsRead[GPUDriver::VS_VERTEX] )
    {
        mdVertex = afl::sendDB(ctx->posbuf(), driver);
        CONFIG_ATTRIB(stream, mdVertex, GPUDriver::VS_VERTEX, ctx->posbuf().numberOfComponents());
        stream++;
    }

    if ( vpres.inputAttribsRead[GPUDriver::VS_COLOR] )
    {
        mdColor = afl::sendDB(ctx->colbuf(), driver);
        CONFIG_ATTRIB(stream, mdColor, GPUDriver::VS_COLOR, ctx->colbuf().numberOfComponents());
        stream++;
    }

    if ( vpres.inputAttribsRead[GPUDriver::VS_NORMAL] )
    {
        mdNormal = afl::sendDB(ctx->norbuf(), driver);
        CONFIG_ATTRIB(stream, mdNormal, GPUDriver::VS_NORMAL, ctx->norbuf().numberOfComponents());
        stream++;
    }

#undef CONFIG_ATTRIB

    driver->setSequentialStreamingMode(ctx->posbuf().vectors(), 0); /* 0 means do not skip any vertex */

    driver->sendCommand( GPU_DRAW );

    driver->releaseMemory(mdVertex);
    driver->releaseMemory(mdColor);
    driver->releaseMemory(mdNormal);

    /*************
       END DRAW
     ************/

    /*****************
       RESTORE STATE
     ****************/

    // RESTORE CULLING STATE

    if (saved_enable_cullface)
    {
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
    }

    // RESTORE SHADER PROGRAMS
    if (saved_enable_vertex_program)
    {
        ctx->testAndSetFlags(GLContext::flagVS);
        ProgramManager::instance().getTarget(GL_VERTEX_PROGRAM_ARB).setCurrent(*saved_current_vertex_program);
    }
    else
        ctx->testAndResetFlags(GLContext::flagVS);


    if (saved_enable_fragment_program)
    {
        ctx->testAndSetFlags(GLContext::flagFS);
        ProgramManager::instance().getTarget(GL_FRAGMENT_PROGRAM_ARB).setCurrent(*saved_current_fragment_program);
    }
    else
        ctx->testAndResetFlags(GLContext::flagFS);

    // POP TRANSFORMATION MATRICES
    ctx->setCurrentMatrix(GL_MODELVIEW);
    ctx->mpop();
    ctx->setCurrentMatrix(GL_PROJECTION);
    ctx->mpop();

    // RESTORE CURRENT MATRIX MODE
    switch ( saved_matrix_mode )
    {
        case GLContext::MODELVIEW:
            ctx->setCurrentMatrix(GL_MODELVIEW);
            break;
        case GLContext::PROJECTION:
            ctx->setCurrentMatrix(GL_PROJECTION);
            break;
        case GLContext::TEXTURE:
            ctx->setCurrentMatrix(GL_TEXTURE);
            break;
    }


    // RESTORE STENCIL TEST
    if (saved_enable_stencil)
        ctx->setFlags(GLContext::flagStencilTest);
    else
        ctx->resetFlags(GLContext::flagStencilTest);

    data.booleanVal = saved_enable_stencil;
    driver->writeGPURegister(GPU_STENCIL_TEST, data);

    ctx->setStencilOp(saved_stencilBufferFailFunc, saved_stencilBufferZFailFunc, saved_stencilBufferZPassFunc);
    ctx->setStencilBufferUpdateMask(saved_stencilBufferUpdateMask);
    ctx->setStencilBufferClearValue(saved_stencilBufferClearValue);
    ctx->setStencilBufferFunc(saved_stencilBufferFunc, saved_stencilBufferReferenceValue, saved_stencilBufferCompareMask);

      #define _SWITCH(fail_) \
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
        }

    _SWITCH(saved_stencilBufferFailFunc)
    driver->writeGPURegister(GPU_STENCIL_FAIL_UPDATE, data);

    _SWITCH(saved_stencilBufferZFailFunc)
    driver->writeGPURegister(GPU_DEPTH_FAIL_UPDATE, data);

    _SWITCH(saved_stencilBufferZPassFunc)
    driver->writeGPURegister(GPU_DEPTH_PASS_UPDATE, data);

    #undef _SWITCH

    data.uintVal = saved_stencilBufferUpdateMask;
    driver->writeGPURegister(GPU_STENCIL_UPDATE_MASK, data);

    data.intVal = saved_stencilBufferClearValue;
    driver->writeGPURegister(GPU_STENCIL_BUFFER_CLEAR, data);

    data.uintVal = saved_stencilBufferCompareMask;
    driver->writeGPURegister(GPU_STENCIL_COMPARE_MASK, data);
    data.uintVal = (u32bit)saved_stencilBufferReferenceValue;
    driver->writeGPURegister(GPU_STENCIL_REFERENCE, data);
    #define BRANCH_CASE(c) case GL_##c : data.compare = GPU_##c; break;

    switch ( saved_stencilBufferFunc )
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

    // RESTORE DEPTH MASK & DEPTH TEST

    if (saved_enable_depth)
        ctx->setFlags(GLContext::flagDepthTest);
    else
        ctx->resetFlags(GLContext::flagDepthTest);

    ctx->setDepthMask(saved_depth_mask);

    data.booleanVal = saved_enable_depth;
    driver->writeGPURegister(GPU_DEPTH_TEST, data);

    data.booleanVal = saved_depth_mask;
    driver->writeGPURegister(GPU_DEPTH_MASK, data);

    // RESTORE COLOR MASK
    ctx->setColorMask(saved_red, saved_green, saved_blue, saved_alpha);
    data.booleanVal = saved_red;
    driver->writeGPURegister(GPU_COLOR_MASK_R, data);
    data.booleanVal = saved_green;
    driver->writeGPURegister(GPU_COLOR_MASK_G, data);
    data.booleanVal =  saved_blue;
    driver->writeGPURegister(GPU_COLOR_MASK_B, data);
    data.booleanVal = saved_alpha;
    driver->writeGPURegister(GPU_COLOR_MASK_A, data);

    /*****************
       END RESTORE
     ****************/
}

void afl::clearDepthOnly(GLContext *ctx)
{
    GLboolean saved_red, saved_green, saved_blue, saved_alpha;
    GLboolean saved_depth_mask;
    GLboolean saved_enable_depth;
    GLboolean saved_enable_stencil;
    GLuint saved_matrix_mode;
    GLboolean saved_enable_vertex_program;
    GLboolean saved_enable_fragment_program;
    ProgramObject* saved_current_vertex_program;
    ProgramObject* saved_current_fragment_program;
    GLboolean saved_enable_cullface;

    GPURegData data;

    /*******************************************
     * SAVE STATE THAT IS GOING TO BE MODIFIED
     *******************************************/

    // SAVE COLOR MASK
    ctx->getColorMask(saved_red,saved_green,saved_blue,saved_alpha);

    // SAVE DEPTH MASK AND DEPTH TEST
    saved_depth_mask = ctx->getDepthMask();
    saved_enable_depth = ctx->testFlags(GLContext::flagDepthTest);

    // SAVE STENCIL TEST
    saved_enable_stencil = ctx->testFlags(GLContext::flagStencilTest);

    // SAVE CURRENT MATRIX
    saved_matrix_mode = ctx->getCurrentMatrix();

    // PUSH TRANSFORMATION MATRICES
    ctx->setCurrentMatrix(GL_PROJECTION);
    ctx->mpush(ctx->mtop());
    ctx->setCurrentMatrix(GL_MODELVIEW);
    ctx->mpush(ctx->mtop());

    // SAVE SHADER PROGRAMS
    saved_enable_vertex_program = ctx->testFlags(GLContext::flagVS);
    saved_enable_fragment_program = ctx->testFlags(GLContext::flagFS);

    if (saved_enable_vertex_program)
        saved_current_vertex_program = &GET_VERTEX_PROGRAM;

    if (saved_enable_fragment_program)
        saved_current_fragment_program = &GET_FRAGMENT_PROGRAM;

    // SAVE CULLING STATE
    saved_enable_cullface = ctx->testFlags(GLContext::flagCullFace);

    /*************
     * END SAVING
     *************/

    /***********************************************
     * CONFIGURE NEEDED STATE TO WRITE DEPTH BUFFER
     ***********************************************/

    // DISABLE COLOR MASK
    ctx->setColorMask(false, false, false, false);
    data.booleanVal = false;
    driver->writeGPURegister(GPU_COLOR_MASK_R, data);
    driver->writeGPURegister(GPU_COLOR_MASK_G, data);
    driver->writeGPURegister(GPU_COLOR_MASK_B, data);
    driver->writeGPURegister(GPU_COLOR_MASK_A, data);

    // ENABLE DEPTH MASK
    ctx->setDepthMask(true);
    data.booleanVal = true;
    driver->writeGPURegister(GPU_DEPTH_MASK, data);

    // DISABLE DEPTH TEST
    ctx->resetFlags(GLContext::flagDepthTest);
    data.booleanVal = false;
    driver->writeGPURegister(GPU_DEPTH_TEST, data);

    // DISABLE STENCIL TEST
    ctx->resetFlags(GLContext::flagStencilTest);
    data.booleanVal = false;
    driver->writeGPURegister(GPU_STENCIL_TEST, data);

    // MODELVIEW = IDENTITY MATRIX
    ctx->setCurrentMatrix(GL_MODELVIEW);
    ctx->ctop(Matrixf::identity());

    // PROJECTION = glOrtho( 0.0, 1.0, 0.0, 1.0, -1.0, 1.0 );
    Matrixf mat;
    mat[0][0] = mat[1][1] = 2.0F;
    mat[1][0] = mat[2][0] = mat[3][0] = mat[0][1] = mat[2][1] = mat[3][1] = mat[0][2] = mat[1][2] = mat[3][2] = 0.0F;
    mat[2][2] = -1.0F;
    mat[0][3] = mat[1][3] = -1.0F;
    mat[2][3] = 0.0F;
    mat[3][3] = 1.0F;
    ctx->setCurrentMatrix(GL_PROJECTION);
    ctx->ctop(mat);

    /*************
       DRAW QUAD
     ************/

    ctx->setFlags(GLContext::flagBeginEnd);
    ctx->setPrimitive(GL_QUADS);
    ctx->clearBuffers();    /* Clean all previous contents */

    data.primitive = QUAD;
    driver->writeGPURegister(GPU_PRIMITIVE, data);

    afl::glVertex(ctx, 0.0f, 0.0f, (float) -(ctx->getDepthClearValue()));
    afl::glVertex(ctx, 1.0f, 0.0f, (float) -(ctx->getDepthClearValue()));
    afl::glVertex(ctx, 1.0f, 1.0f, (float) -(ctx->getDepthClearValue()));
    afl::glVertex(ctx, 0.0f, 1.0f, (float) -(ctx->getDepthClearValue()));

    ctx->resetFlags(GLContext::flagBeginEnd);

    u32bit mdVertex = 0;
    u32bit mdColor = 0;
    u32bit mdNormal = 0;
    vector<u32bit> mdTextures; // Texture unit memory descriptor mdTextures[i] for texUnit[i]
    vector<u32bit> texUnit; // Texture unit id's

    ProgramManager::instance().getTarget(GL_VERTEX_PROGRAM_ARB).setDefaultAsCurrent();
    ProgramManager::instance().getTarget(GL_FRAGMENT_PROGRAM_ARB).setDefaultAsCurrent();

    ctx->testAndSetFlags(GLContext::flagVS);
    ctx->testAndSetFlags(GLContext::flagFS);

    afl::setupShaders(ctx, driver);

    /*  Setup Hierarchical Z Test.  */
    afl::setupHZTest(ctx, driver);

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

    const ProgramObject::ProgramResources& vpres = GET_VERTEX_PROGRAM.getResources();

    GLuint stream = 0;

    if ( vpres.inputAttribsRead[GPUDriver::VS_VERTEX] )
    {
        mdVertex = afl::sendDB(ctx->posbuf(), driver);
        CONFIG_ATTRIB(stream, mdVertex, GPUDriver::VS_VERTEX, ctx->posbuf().numberOfComponents());
        stream++;
    }

    if ( vpres.inputAttribsRead[GPUDriver::VS_COLOR] )
    {
        mdColor = afl::sendDB(ctx->colbuf(), driver);
        CONFIG_ATTRIB(stream, mdColor, GPUDriver::VS_COLOR, ctx->colbuf().numberOfComponents());
        stream++;
    }

    if ( vpres.inputAttribsRead[GPUDriver::VS_NORMAL] )
    {
        mdNormal = afl::sendDB(ctx->norbuf(), driver);
        CONFIG_ATTRIB(stream, mdNormal, GPUDriver::VS_NORMAL, ctx->norbuf().numberOfComponents());
        stream++;
    }

#undef CONFIG_ATTRIB

    driver->setSequentialStreamingMode(ctx->posbuf().vectors(), 0); /* 0 means do not skip any vertex */

    driver->sendCommand( GPU_DRAW );

    driver->releaseMemory(mdVertex);
    driver->releaseMemory(mdColor);
    driver->releaseMemory(mdNormal);

    /*************
       END DRAW
     ************/

    /*****************
       RESTORE STATE
     ****************/

    // RESTORE CULLING STATE

    if (saved_enable_cullface)
    {
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
    }

    // RESTORE SHADER PROGRAMS
    if (saved_enable_vertex_program)
    {
        ctx->testAndSetFlags(GLContext::flagVS);
        ProgramManager::instance().getTarget(GL_VERTEX_PROGRAM_ARB).setCurrent(*saved_current_vertex_program);
    }
    else
        ctx->testAndResetFlags(GLContext::flagVS);


    if (saved_enable_fragment_program)
    {
        ctx->testAndSetFlags(GLContext::flagFS);
        ProgramManager::instance().getTarget(GL_FRAGMENT_PROGRAM_ARB).setCurrent(*saved_current_fragment_program);
    }
    else
        ctx->testAndResetFlags(GLContext::flagFS);

    // POP TRANSFORMATION MATRICES
    ctx->setCurrentMatrix(GL_MODELVIEW);
    ctx->mpop();
    ctx->setCurrentMatrix(GL_PROJECTION);
    ctx->mpop();

    // RESTORE CURRENT MATRIX MODE
    switch ( saved_matrix_mode )
    {
        case GLContext::MODELVIEW:
            ctx->setCurrentMatrix(GL_MODELVIEW);
            break;
        case GLContext::PROJECTION:
            ctx->setCurrentMatrix(GL_PROJECTION);
            break;
        case GLContext::TEXTURE:
            ctx->setCurrentMatrix(GL_TEXTURE);
            break;
    }

    // RESTORE STENCIL TEST
    if (saved_enable_stencil)
        ctx->setFlags(GLContext::flagStencilTest);
    else
        ctx->resetFlags(GLContext::flagStencilTest);

   // RESTORE DEPTH MASK & DEPTH TEST

    if (saved_enable_depth)
        ctx->setFlags(GLContext::flagDepthTest);
    else
        ctx->resetFlags(GLContext::flagDepthTest);

    ctx->setDepthMask(saved_depth_mask);

    data.booleanVal = saved_enable_depth;
    driver->writeGPURegister(GPU_DEPTH_TEST, data);

    data.booleanVal = saved_depth_mask;
    driver->writeGPURegister(GPU_DEPTH_MASK, data);

    // RESTORE COLOR MASK
    ctx->setColorMask(saved_red, saved_green, saved_blue, saved_alpha);
    data.booleanVal = saved_red;
    driver->writeGPURegister(GPU_COLOR_MASK_R, data);
    data.booleanVal = saved_green;
    driver->writeGPURegister(GPU_COLOR_MASK_G, data);
    data.booleanVal =  saved_blue;
    driver->writeGPURegister(GPU_COLOR_MASK_B, data);
    data.booleanVal = saved_alpha;
    driver->writeGPURegister(GPU_COLOR_MASK_A, data);

}

void afl::clear(GLContext *ctx,  GLbitfield mask)
{
    GLboolean saved_red, saved_green, saved_blue, saved_alpha;
    GLboolean saved_depth_mask;
    GLboolean saved_enable_depth;
    GLclampd savedNearDepth, savedFarDepth;
    GLenum savedDepthFunc;
    GLint saved_stencilBufferClearValue;
    GLuint saved_stencilBufferUpdateMask;
    GLenum saved_stencilBufferFailFunc;
    GLenum saved_stencilBufferZFailFunc;
    GLenum saved_stencilBufferZPassFunc;
    GLenum saved_stencilBufferFunc;
    GLint saved_stencilBufferReferenceValue;
    GLuint saved_stencilBufferCompareMask;
    GLboolean saved_enable_stencil;
    GLuint saved_matrix_mode;
    GLboolean saved_enable_vertex_program;
    GLboolean saved_enable_fragment_program;
    ProgramObject* saved_current_vertex_program;
    ProgramObject* saved_current_fragment_program;
    GLboolean saved_enable_cullface;
    GLboolean saved_enable_blend;
    Quadf savedCurrentColor;

    GPURegData data;

    //  Decode the buffers that are to be cleared.
    bool clearColor = ( mask & GL_COLOR_BUFFER_BIT ) == GL_COLOR_BUFFER_BIT;
    bool clearZ = ( mask & GL_DEPTH_BUFFER_BIT ) == GL_DEPTH_BUFFER_BIT;
    bool clearStencil = ( mask & GL_STENCIL_BUFFER_BIT ) == GL_STENCIL_BUFFER_BIT;

    /*******************************************
     * SAVE STATE THAT IS GOING TO BE MODIFIED
     *******************************************/

    // SAVE COLOR MASK
    ctx->getColorMask(saved_red,saved_green,saved_blue,saved_alpha);

    // SAVE CURRENT COLOR
    savedCurrentColor = ctx->getColor();

    // SAVE BLENDING PARAMETERS
    saved_enable_blend = ctx->testFlags(GLContext::flagBlending);

    // SAVE DEPTH MASK AND DEPTH TEST
    saved_depth_mask = ctx->getDepthMask();
    ctx->getDepthRange(savedNearDepth, savedFarDepth);
    savedDepthFunc = ctx->getDepthFunc();
    saved_enable_depth = ctx->testFlags(GLContext::flagDepthTest);

    // SAVE STENCIL TEST PARAMETERS
    saved_stencilBufferFunc = ctx->getStencilBufferFunc();
    saved_stencilBufferReferenceValue = ctx->getStencilBufferReferenceValue();
    saved_stencilBufferCompareMask = ctx->getStencilBufferCompareMask();
    saved_stencilBufferClearValue = ctx->getStencilBufferClearValue();
    saved_stencilBufferUpdateMask = ctx->getStencilBufferUpdateMask();
    ctx->getStencilOp(saved_stencilBufferFailFunc, saved_stencilBufferZFailFunc, saved_stencilBufferZPassFunc);
    saved_enable_stencil = ctx->testFlags(GLContext::flagStencilTest);

    // SAVE CURRENT MATRIX
    saved_matrix_mode = ctx->getCurrentMatrix();

    // PUSH TRANSFORMATION MATRICES
    ctx->setCurrentMatrix(GL_PROJECTION);
    ctx->mpush(ctx->mtop());
    ctx->setCurrentMatrix(GL_MODELVIEW);
    ctx->mpush(ctx->mtop());

    // SAVE SHADER PROGRAMS
    saved_enable_vertex_program = ctx->testFlags(GLContext::flagVS);
    saved_enable_fragment_program = ctx->testFlags(GLContext::flagFS);

    if (saved_enable_vertex_program)
        saved_current_vertex_program = &GET_VERTEX_PROGRAM;

    if (saved_enable_fragment_program)
        saved_current_fragment_program = &GET_FRAGMENT_PROGRAM;

    // SAVE CULLING STATE
    saved_enable_cullface = ctx->testFlags(GLContext::flagCullFace);

    /*************
     * END SAVING
     *************/

    /******************************************
     * CONFIGURE NEEDED STATE TO WRITE STENCIL
     ******************************************/

    // Check if the color buffer is to be cleared
    if (clearColor)
    {
        // SET COLOR TO ENABLE WRITING
        ctx->setColorMask(true, true, true, true);
        data.booleanVal = true;
        driver->writeGPURegister(GPU_COLOR_MASK_R, data);
        driver->writeGPURegister(GPU_COLOR_MASK_G, data);
        driver->writeGPURegister(GPU_COLOR_MASK_B, data);
        driver->writeGPURegister(GPU_COLOR_MASK_A, data);

        //  Set current to clear color
        Quadf clrColor;
        ctx->getClearColor(clrColor[0], clrColor[1], clrColor[2], clrColor[3]);

        ctx->setColor(clrColor);

        // Disable blending
        ctx->resetFlags(GLContext::flagBlending);

        data.booleanVal = false;
        driver->writeGPURegister(GPU_COLOR_BLEND, data);
    }
    else
    {
        // DISABLE COLOR MASK
        ctx->setColorMask(false, false, false, false);
        data.booleanVal = false;
        driver->writeGPURegister(GPU_COLOR_MASK_R, data);
        driver->writeGPURegister(GPU_COLOR_MASK_G, data);
        driver->writeGPURegister(GPU_COLOR_MASK_B, data);
        driver->writeGPURegister(GPU_COLOR_MASK_A, data);
    }

    //  Check if the z buffer is to be cleared
    if (clearZ)
    {
        // SET DEPTH TO ENABLE WRITING

        ctx->setDepthMask(true);
        data.booleanVal = true;
        driver->writeGPURegister(GPU_DEPTH_MASK, data);
        ctx->setFlags(GLContext::flagDepthTest);
        data.booleanVal = true;
        driver->writeGPURegister(GPU_DEPTH_TEST, data);
        data.compare = GPU_ALWAYS;
        driver->writeGPURegister(GPU_DEPTH_FUNCTION, data);
        data.f32Val = 0.0f;
        driver->writeGPURegister( GPU_DEPTH_RANGE_NEAR, data );
        data.f32Val = 1.0f;
        driver->writeGPURegister( GPU_DEPTH_RANGE_FAR, data );

        ctx->setFlags(GLContext::flagDepthTest);
        data.booleanVal = true;
        driver->writeGPURegister(GPU_DEPTH_TEST, data);

    }
    else
    {
        // DISABLE DEPTH MASK
        ctx->setDepthMask(false);
        data.booleanVal = false;
        driver->writeGPURegister(GPU_DEPTH_MASK, data);

        // DISABLE DEPTH TEST
        ctx->resetFlags(GLContext::flagDepthTest);
        data.booleanVal = false;
        driver->writeGPURegister(GPU_DEPTH_TEST, data);
    }

    // DISABLE FACE CULLING
    ctx->testAndResetFlags(GLContext::flagCullFace);
    data.culling = NONE;
    driver->writeGPURegister(GPU_CULLING, data);

    //  Check if the stencil buffer is to be cleared
    if (clearStencil)
    {
        // REPLACE STENCIL VALUES TO CLEAR VALUE
        //ctx->setStencilBufferFunc(GL_ALWAYS, ctx->getStencilBufferClearValue(), 255);
        ctx->setStencilBufferFunc(GL_NEVER, ctx->getStencilBufferClearValue(), 255);
        //ctx->setStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
        ctx->setStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
        ctx->setFlags(GLContext::flagStencilTest);
        //data.compare = GPU_ALWAYS;
        data.compare = GPU_NEVER;
        driver->writeGPURegister(GPU_STENCIL_FUNCTION, data);
        data.uintVal = (u32bit)ctx->getStencilBufferClearValue();
        driver->writeGPURegister(GPU_STENCIL_REFERENCE, data);
        data.uintVal = 255;
        driver->writeGPURegister(GPU_STENCIL_COMPARE_MASK, data);
        data.stencilUpdate = STENCIL_REPLACE;
        driver->writeGPURegister(GPU_STENCIL_FAIL_UPDATE, data);
        data.stencilUpdate = STENCIL_KEEP;
        driver->writeGPURegister(GPU_DEPTH_FAIL_UPDATE, data);
        driver->writeGPURegister(GPU_DEPTH_PASS_UPDATE, data);
        data.booleanVal = true;
        driver->writeGPURegister(GPU_STENCIL_TEST, data);
    }
    else
    {
        // Disable stencil.
        ctx->resetFlags(GLContext::flagStencilTest);
        data.booleanVal = false;
        driver->writeGPURegister(GPU_STENCIL_TEST, data);
    }

    // MODELVIEW = IDENTITY MATRIX
    ctx->setCurrentMatrix(GL_MODELVIEW);
    ctx->ctop(Matrixf::identity());

    // PROJECTION = glOrtho( 0.0, 1.0, 0.0, 1.0, -1.0, 1.0 );
    Matrixf mat;
    mat[0][0] = mat[1][1] = 2.0F;
    mat[1][0] = mat[2][0] = mat[3][0] = mat[0][1] = mat[2][1] = mat[3][1] = mat[0][2] = mat[1][2] = mat[3][2] = 0.0F;
    mat[2][2] = -1.0F;
    mat[0][3] = mat[1][3] = -1.0F;
    mat[2][3] = 0.0F;
    mat[3][3] = 1.0F;
    ctx->setCurrentMatrix(GL_PROJECTION);
    ctx->ctop(mat);

    /*************
       DRAW QUAD
     ************/

    ctx->setFlags(GLContext::flagBeginEnd);

    ctx->setPrimitive(GL_QUADS);
    ctx->clearBuffers();    /* Clean all previous contents */

    data.primitive = QUAD;
    driver->writeGPURegister(GPU_PRIMITIVE, data);

    GLclampd depthClear = 1.0f - 2.0f * (ctx->getDepthClearValue());

    afl::glVertex(ctx, 0.0f, 0.0f, (float) depthClear);
    afl::glVertex(ctx, 1.0f, 0.0f, (float) depthClear);
    afl::glVertex(ctx, 1.0f, 1.0f, (float) depthClear);
    afl::glVertex(ctx, 0.0f, 1.0f, (float) depthClear);

    ctx->resetFlags(GLContext::flagBeginEnd);

    u32bit mdVertex = 0;
    u32bit mdColor = 0;
    u32bit mdNormal = 0;
    vector<u32bit> mdTextures; // Texture unit memory descriptor mdTextures[i] for texUnit[i]
    vector<u32bit> texUnit; // Texture unit id's

    ProgramManager::instance().getTarget(GL_VERTEX_PROGRAM_ARB).setDefaultAsCurrent();
    ProgramManager::instance().getTarget(GL_FRAGMENT_PROGRAM_ARB).setDefaultAsCurrent();

    ctx->testAndSetFlags(GLContext::flagVS);
    ctx->testAndSetFlags(GLContext::flagFS);

    afl::setupShaders(ctx, driver);

    /*  Setup Hierarchical Z Test.  */
    afl::setupHZTest(ctx, driver);

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

    const ProgramObject::ProgramResources& vpres = GET_VERTEX_PROGRAM.getResources();

    GLuint stream = 0;

    if ( vpres.inputAttribsRead[GPUDriver::VS_VERTEX] )
    {
        mdVertex = afl::sendDB(ctx->posbuf(), driver);
        CONFIG_ATTRIB(stream, mdVertex, GPUDriver::VS_VERTEX, ctx->posbuf().numberOfComponents());
        stream++;
    }

    if ( vpres.inputAttribsRead[GPUDriver::VS_COLOR] )
    {
        mdColor = afl::sendDB(ctx->colbuf(), driver);
        CONFIG_ATTRIB(stream, mdColor, GPUDriver::VS_COLOR, ctx->colbuf().numberOfComponents());
        stream++;
    }

    if ( vpres.inputAttribsRead[GPUDriver::VS_NORMAL] )
    {
        mdNormal = afl::sendDB(ctx->norbuf(), driver);
        CONFIG_ATTRIB(stream, mdNormal, GPUDriver::VS_NORMAL, ctx->norbuf().numberOfComponents());
        stream++;
    }

#undef CONFIG_ATTRIB

    driver->setSequentialStreamingMode(ctx->posbuf().vectors(), 0); /* 0 means do not skip any vertex */

    driver->sendCommand( GPU_DRAW );

    driver->releaseMemory(mdVertex);
    driver->releaseMemory(mdColor);
    driver->releaseMemory(mdNormal);

    /*************
       END DRAW
     ************/

    /*****************
       RESTORE STATE
     ****************/

    // RESTORE CULLING STATE

    if (saved_enable_cullface)
    {
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
    }

    // RESTORE SHADER PROGRAMS
    if (saved_enable_vertex_program)
    {
        ctx->testAndSetFlags(GLContext::flagVS);
        ProgramManager::instance().getTarget(GL_VERTEX_PROGRAM_ARB).setCurrent(*saved_current_vertex_program);
    }
    else
        ctx->testAndResetFlags(GLContext::flagVS);


    if (saved_enable_fragment_program)
    {
        ctx->testAndSetFlags(GLContext::flagFS);
        ProgramManager::instance().getTarget(GL_FRAGMENT_PROGRAM_ARB).setCurrent(*saved_current_fragment_program);
    }
    else
        ctx->testAndResetFlags(GLContext::flagFS);

    // POP TRANSFORMATION MATRICES
    ctx->setCurrentMatrix(GL_MODELVIEW);
    ctx->mpop();
    ctx->setCurrentMatrix(GL_PROJECTION);
    ctx->mpop();

    // RESTORE CURRENT MATRIX MODE
    switch ( saved_matrix_mode )
    {
        case GLContext::MODELVIEW:
            ctx->setCurrentMatrix(GL_MODELVIEW);
            break;
        case GLContext::PROJECTION:
            ctx->setCurrentMatrix(GL_PROJECTION);
            break;
        case GLContext::TEXTURE:
            ctx->setCurrentMatrix(GL_TEXTURE);
            break;
    }

    // RESTORE BLENDING STATE
    if (saved_enable_blend)
        ctx->setFlags(GLContext::flagBlending);
    else
        ctx->resetFlags(GLContext::flagBlending);

    data.booleanVal = saved_enable_blend;
    driver->writeGPURegister(GPU_COLOR_BLEND, data);

    // RESTORE STENCIL TEST
    if (saved_enable_stencil)
        ctx->setFlags(GLContext::flagStencilTest);
    else
        ctx->resetFlags(GLContext::flagStencilTest);

    data.booleanVal = saved_enable_stencil;
    driver->writeGPURegister(GPU_STENCIL_TEST, data);

    ctx->setStencilOp(saved_stencilBufferFailFunc, saved_stencilBufferZFailFunc, saved_stencilBufferZPassFunc);
    ctx->setStencilBufferUpdateMask(saved_stencilBufferUpdateMask);
    ctx->setStencilBufferClearValue(saved_stencilBufferClearValue);
    ctx->setStencilBufferFunc(saved_stencilBufferFunc, saved_stencilBufferReferenceValue, saved_stencilBufferCompareMask);

      #define _SWITCH(fail_) \
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
        }

    _SWITCH(saved_stencilBufferFailFunc)
    driver->writeGPURegister(GPU_STENCIL_FAIL_UPDATE, data);

    _SWITCH(saved_stencilBufferZFailFunc)
    driver->writeGPURegister(GPU_DEPTH_FAIL_UPDATE, data);

    _SWITCH(saved_stencilBufferZPassFunc)
    driver->writeGPURegister(GPU_DEPTH_PASS_UPDATE, data);

    #undef _SWITCH

    data.uintVal = saved_stencilBufferUpdateMask;
    driver->writeGPURegister(GPU_STENCIL_UPDATE_MASK, data);

    data.intVal = saved_stencilBufferClearValue;
    driver->writeGPURegister(GPU_STENCIL_BUFFER_CLEAR, data);

    data.uintVal = saved_stencilBufferCompareMask;
    driver->writeGPURegister(GPU_STENCIL_COMPARE_MASK, data);
    data.uintVal = (u32bit)saved_stencilBufferReferenceValue;
    driver->writeGPURegister(GPU_STENCIL_REFERENCE, data);


    #define BRANCH_CASE(c) case GL_##c : data.compare = GPU_##c; break;

    switch ( saved_stencilBufferFunc )
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
            panic("AuxFuncsLib", "glClear", "Unexpected stencil function");
    }

    #undef BRANCH_CASE

    driver->writeGPURegister(GPU_STENCIL_FUNCTION, data);

    // RESTORE DEPTH MASK & DEPTH TEST

    ctx->setDepthRange(savedNearDepth, savedFarDepth);

    data.f32Val = savedNearDepth;
    driver->writeGPURegister( GPU_DEPTH_RANGE_NEAR, data );
    data.f32Val = savedFarDepth;
    driver->writeGPURegister( GPU_DEPTH_RANGE_FAR, data );

    ctx->setDepthFunc(savedDepthFunc);

    #define BRANCH_CASE(c) case GL_##c : data.compare = GPU_##c; break;

    switch ( savedDepthFunc )
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
            panic("AuxFuncsLib", "glClear", "Unexpected depth function");
    }

    #undef BRANCH_CASE

    driver->writeGPURegister(GPU_DEPTH_FUNCTION, data);

    if (saved_enable_depth)
        ctx->setFlags(GLContext::flagDepthTest);
    else
        ctx->resetFlags(GLContext::flagDepthTest);

    ctx->setDepthMask(saved_depth_mask);

    data.booleanVal = saved_enable_depth;
    driver->writeGPURegister(GPU_DEPTH_TEST, data);

    data.booleanVal = saved_depth_mask;
    driver->writeGPURegister(GPU_DEPTH_MASK, data);

    // RESTORE CURRENT COLOR
    ctx->setColor(savedCurrentColor);

    // RESTORE COLOR MASK
    ctx->setColorMask(saved_red, saved_green, saved_blue, saved_alpha);
    data.booleanVal = saved_red;
    driver->writeGPURegister(GPU_COLOR_MASK_R, data);
    data.booleanVal = saved_green;
    driver->writeGPURegister(GPU_COLOR_MASK_G, data);
    data.booleanVal =  saved_blue;
    driver->writeGPURegister(GPU_COLOR_MASK_B, data);
    data.booleanVal = saved_alpha;
    driver->writeGPURegister(GPU_COLOR_MASK_A, data);

    /*****************
       END RESTORE
     ****************/
}






void afl::dumpStencilBuffer(GLContext *ctx)
{
    GLboolean saved_red, saved_green, saved_blue, saved_alpha;
    GLboolean saved_depth_mask;
    GLboolean saved_enable_depth;
    GLint saved_stencilBufferClearValue;
    GLuint saved_stencilBufferUpdateMask;
    GLenum saved_stencilBufferFailFunc;
    GLenum saved_stencilBufferZFailFunc;
    GLenum saved_stencilBufferZPassFunc;
    GLenum saved_stencilBufferFunc;
    GLint saved_stencilBufferReferenceValue;
    GLuint saved_stencilBufferCompareMask;
    GLboolean saved_enable_stencil;
    GLuint saved_matrix_mode;
    GLboolean saved_enable_vertex_program;
    GLboolean saved_enable_fragment_program;
    ProgramObject* saved_current_vertex_program;
    ProgramObject* saved_current_fragment_program;
    GLboolean saved_enable_scissor;
    GLboolean saved_enable_cullface;
    GLenum saved_blend_src_rgb;
    GLenum saved_blend_dst_rgb;
    GLenum saved_blend_src_alpha;
    GLenum saved_blend_dst_alpha;
    GLenum saved_blend_equation;
    GLboolean saved_enable_blend;
    Quadf saved_clear_color;

    GPURegData data;

    /*******************************************
     * SAVE STATE THAT IS GOING TO BE MODIFIED
     *******************************************/

    // SAVE COLOR MASK
    ctx->getColorMask(saved_red,saved_green,saved_blue,saved_alpha);

    // SAVE DEPTH MASK AND DEPTH TEST
    saved_depth_mask = ctx->getDepthMask();
    saved_enable_depth = ctx->testFlags(GLContext::flagDepthTest);

    // SAVE STENCIL TEST PARAMETERS
    saved_stencilBufferFunc = ctx->getStencilBufferFunc();
    saved_stencilBufferReferenceValue = ctx->getStencilBufferReferenceValue();
    saved_stencilBufferCompareMask = ctx->getStencilBufferCompareMask();
    saved_stencilBufferClearValue = ctx->getStencilBufferClearValue();
    saved_stencilBufferUpdateMask = ctx->getStencilBufferUpdateMask();
    ctx->getStencilOp(saved_stencilBufferFailFunc, saved_stencilBufferZFailFunc, saved_stencilBufferZPassFunc);
    saved_enable_stencil = ctx->testFlags(GLContext::flagStencilTest);

    // SAVE BLENDING PARAMETERS
    ctx->getBlendFactor(saved_blend_src_rgb, saved_blend_dst_rgb, saved_blend_src_alpha, saved_blend_dst_alpha);
    saved_blend_equation = ctx->getBlendEquation();
    saved_enable_blend = ctx->testFlags(GLContext::flagBlending);

    // SAVE CURRENT MATRIX
    saved_matrix_mode = ctx->getCurrentMatrix();

    // PUSH TRANSFORMATION MATRICES
    ctx->setCurrentMatrix(GL_PROJECTION);
    ctx->mpush(ctx->mtop());
    ctx->setCurrentMatrix(GL_MODELVIEW);
    ctx->mpush(ctx->mtop());

    // SAVE SCISSOR PARAMETERS
    saved_enable_scissor = ctx->testFlags(GLContext::flagScissorTest);

    // SAVE SHADER PROGRAMS
    saved_enable_vertex_program = ctx->testFlags(GLContext::flagVS);
    saved_enable_fragment_program = ctx->testFlags(GLContext::flagFS);

    if (saved_enable_vertex_program)
        saved_current_vertex_program = &GET_VERTEX_PROGRAM;

    if (saved_enable_fragment_program)
        saved_current_fragment_program = &GET_FRAGMENT_PROGRAM;

    // SAVE CULLING STATE
    saved_enable_cullface = ctx->testFlags(GLContext::flagCullFace);

    // SAVE CLEAR COLOR
    ctx->getClearColor(saved_clear_color[0], saved_clear_color[1], saved_clear_color[2], saved_clear_color[3]);

    /*************
     * END SAVING
     *************/

    /***********************************************************
     * CONFIGURE NEEDED STATE TO DUMP STENCIL INTO COLOR BUFFER
     ***********************************************************/

    // ENABLE COLOR MASK
    ctx->setColorMask(true, true, true, true);
    data.booleanVal = true;
    driver->writeGPURegister(GPU_COLOR_MASK_R, data);
    driver->writeGPURegister(GPU_COLOR_MASK_G, data);
    driver->writeGPURegister(GPU_COLOR_MASK_B, data);
    driver->writeGPURegister(GPU_COLOR_MASK_A, data);

    // DISABLE DEPTH MASK
    ctx->setDepthMask(false);
    data.booleanVal = false;
    driver->writeGPURegister(GPU_DEPTH_MASK, data);

    // DISABLE FACE CULLING
    ctx->testAndResetFlags(GLContext::flagCullFace);
    data.culling = NONE;
    driver->writeGPURegister(GPU_CULLING, data);

    // DISABLE DEPTH TEST
    ctx->resetFlags(GLContext::flagDepthTest);
    data.booleanVal = false;
    driver->writeGPURegister(GPU_DEPTH_TEST, data);

    // REPLACE STENCIL VALUES TO CHECK BUFFER
    ctx->setStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    ctx->setFlags(GLContext::flagStencilTest);
    data.stencilUpdate = STENCIL_KEEP;
    driver->writeGPURegister(GPU_STENCIL_FAIL_UPDATE, data);
    driver->writeGPURegister(GPU_DEPTH_FAIL_UPDATE, data);
    driver->writeGPURegister(GPU_DEPTH_PASS_UPDATE, data);
    data.booleanVal = true;
    driver->writeGPURegister(GPU_STENCIL_TEST, data);

    // SET BLENDING PARAMETERS TO ACCUMULATE BIT CHECKS IN COLOR BUFFER
    ctx->setBlendFactor(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
    ctx->setBlendEquation(GL_FUNC_ADD);
    ctx->setFlags(GLContext::flagBlending);
    data.blendFunction = BLEND_ONE;
    driver->writeGPURegister(GPU_BLEND_SRC_RGB, data);
    driver->writeGPURegister(GPU_BLEND_SRC_ALPHA, data);
    driver->writeGPURegister(GPU_BLEND_DST_RGB, data);
    driver->writeGPURegister(GPU_BLEND_DST_ALPHA, data);
    data.blendEquation = BLEND_FUNC_ADD;
    driver->writeGPURegister(GPU_BLEND_EQUATION, data);
    data.booleanVal = true;
    driver->writeGPURegister(GPU_COLOR_BLEND, data);

    // DISABLE SCISSOR TEST
    ctx->resetFlags(GLContext::flagScissorTest);
    data.booleanVal = false;
    driver->writeGPURegister(GPU_SCISSOR_TEST, data);

    // MODELVIEW = IDENTITY MATRIX
    ctx->setCurrentMatrix(GL_MODELVIEW);
    ctx->ctop(Matrixf::identity());

    // PROJECTION = glOrtho( 0.0, 1.0, 0.0, 1.0, -1.0, 1.0 );
    Matrixf mat;
    mat[0][0] = mat[1][1] = 2.0F;
    mat[1][0] = mat[2][0] = mat[3][0] = mat[0][1] = mat[2][1] = mat[3][1] = mat[0][2] = mat[1][2] = mat[3][2] = 0.0F;
    mat[2][2] = -1.0F;
    mat[0][3] = mat[1][3] = -1.0F;
    mat[2][3] = 0.0F;
    mat[3][3] = 1.0F;
    ctx->setCurrentMatrix(GL_PROJECTION);
    ctx->ctop(mat);

    // CHANGE CLEAR COLOR TO BLACK AND CLEAR COLOR BUFFER ( INITIAL ZEROS TO ACCUMULATE COLOR VALUES )
    data.qfVal[0] = 0.0f;
    data.qfVal[1] = 0.0f;
    data.qfVal[2] = 0.0f;
    data.qfVal[3] = 0.0f;
    driver->writeGPURegister( GPU_COLOR_BUFFER_CLEAR, data );
    driver->sendCommand( GPU_CLEARCOLORBUFFER );

    // LOOP OVER ALL STENCIL BITS

    GLuint currentMaskBit = 0x80;

    while (currentMaskBit != 0x00)
    {
        // SET CORRECT VALUES FOR COMPARISON
        ctx->setStencilBufferFunc(GL_EQUAL, currentMaskBit, currentMaskBit);
        data.compare = GPU_EQUAL;
        driver->writeGPURegister(GPU_STENCIL_FUNCTION, data);
        data.uintVal = (u32bit)currentMaskBit;
        driver->writeGPURegister(GPU_STENCIL_REFERENCE, data);
        driver->writeGPURegister(GPU_STENCIL_COMPARE_MASK, data);

        /*************
           DRAW QUAD
         ************/

        ctx->setFlags(GLContext::flagBeginEnd);
        ctx->setPrimitive(GL_QUADS);
        ctx->clearBuffers();    /* Clean all previous contents */
        data.primitive = QUAD;
        driver->writeGPURegister(GPU_PRIMITIVE, data);

        // SET CORRESPONDING COLOR
        GLfloat componentColor = GLfloat(currentMaskBit) / 0xFF;
        ctx->setColor(componentColor,componentColor,componentColor,1.0f);

        afl::glVertex(ctx, 0.0f, 0.0f);
        afl::glVertex(ctx, 1.0f, 0.0f);
        afl::glVertex(ctx, 1.0f, 1.0f);
        afl::glVertex(ctx, 0.0f, 1.0f);

        ctx->resetFlags(GLContext::flagBeginEnd);

        u32bit mdVertex = 0;
        u32bit mdColor = 0;
        u32bit mdNormal = 0;
        vector<u32bit> mdTextures; // Texture unit memory descriptor mdTextures[i] for texUnit[i]
        vector<u32bit> texUnit; // Texture unit id's

        ProgramManager::instance().getTarget(GL_VERTEX_PROGRAM_ARB).setDefaultAsCurrent();
        ProgramManager::instance().getTarget(GL_FRAGMENT_PROGRAM_ARB).setDefaultAsCurrent();

        ctx->testAndSetFlags(GLContext::flagVS);
        ctx->testAndSetFlags(GLContext::flagFS);

        afl::setupShaders(ctx, driver);

        /*  Setup Hierarchical Z Test.  */
        afl::setupHZTest(ctx, driver);

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

        const ProgramObject::ProgramResources& vpres = GET_VERTEX_PROGRAM.getResources();

        GLuint stream = 0;

        if ( vpres.inputAttribsRead[GPUDriver::VS_VERTEX] )
        {
            mdVertex = afl::sendDB(ctx->posbuf(), driver);
            CONFIG_ATTRIB(stream, mdVertex, GPUDriver::VS_VERTEX, ctx->posbuf().numberOfComponents());
            stream++;
        }

        if ( vpres.inputAttribsRead[GPUDriver::VS_COLOR] )
        {
            mdColor = afl::sendDB(ctx->colbuf(), driver);
            CONFIG_ATTRIB(stream, mdColor, GPUDriver::VS_COLOR, ctx->colbuf().numberOfComponents());
            stream++;
        }

        if ( vpres.inputAttribsRead[GPUDriver::VS_NORMAL] )
        {
            mdNormal = afl::sendDB(ctx->norbuf(), driver);
            CONFIG_ATTRIB(stream, mdNormal, GPUDriver::VS_NORMAL, ctx->norbuf().numberOfComponents());
            stream++;
        }

    #undef CONFIG_ATTRIB

        driver->setSequentialStreamingMode(ctx->posbuf().vectors(), 0); /* 0 means do not skip any vertex */

        driver->sendCommand( GPU_DRAW );

        driver->releaseMemory(mdVertex);
        driver->releaseMemory(mdColor);
        driver->releaseMemory(mdNormal);

        /*************
           END DRAW
         ************/

        currentMaskBit = currentMaskBit >> 1;
    }

    /*****************
       RESTORE STATE
     ****************/

    // RESTORE CLEAR COLOR
    ctx->setClearColor(saved_clear_color[0], saved_clear_color[1], saved_clear_color[2], saved_clear_color[3]);

    data.qfVal[0] = saved_clear_color[0];
    data.qfVal[1] = saved_clear_color[1];
    data.qfVal[2] = saved_clear_color[2];
    data.qfVal[3] = saved_clear_color[3];
    
    driver->writeGPURegister( GPU_COLOR_BUFFER_CLEAR, data );

    // RESTORE CULLING STATE

    if (saved_enable_cullface)
    {
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
    }

    // RESTORE SHADER PROGRAMS
    if (saved_enable_vertex_program)
    {
        ctx->testAndSetFlags(GLContext::flagVS);
        ProgramManager::instance().getTarget(GL_VERTEX_PROGRAM_ARB).setCurrent(*saved_current_vertex_program);
    }
    else
        ctx->testAndResetFlags(GLContext::flagVS);


    if (saved_enable_fragment_program)
    {
        ctx->testAndSetFlags(GLContext::flagFS);
        ProgramManager::instance().getTarget(GL_FRAGMENT_PROGRAM_ARB).setCurrent(*saved_current_fragment_program);
    }
    else
        ctx->testAndResetFlags(GLContext::flagFS);

    // POP TRANSFORMATION MATRICES
    ctx->setCurrentMatrix(GL_MODELVIEW);
    ctx->mpop();
    ctx->setCurrentMatrix(GL_PROJECTION);
    ctx->mpop();

    // RESTORE CURRENT MATRIX MODE
    switch ( saved_matrix_mode )
    {
        case GLContext::MODELVIEW:
            ctx->setCurrentMatrix(GL_MODELVIEW);
            break;
        case GLContext::PROJECTION:
            ctx->setCurrentMatrix(GL_PROJECTION);
            break;
        case GLContext::TEXTURE:
            ctx->setCurrentMatrix(GL_TEXTURE);
            break;
    }

    // RESTORE BLENDING STATE
    ctx->setBlendFactor(saved_blend_src_rgb, saved_blend_dst_rgb, saved_blend_src_alpha, saved_blend_dst_alpha);
    ctx->setBlendEquation(saved_blend_equation);
    ctx->setFlags(saved_enable_blend);

    #define SWITCH_BR(v) case GL_##v: data.blendFunction = BLEND_##v; break;

    #define SWITCH_SENTENCE(condition) \
        switch ( condition )\
        {\
            SWITCH_BR(ZERO)\
            SWITCH_BR(ONE)\
            SWITCH_BR(SRC_COLOR)\
            SWITCH_BR(ONE_MINUS_SRC_COLOR)\
            SWITCH_BR(DST_COLOR)\
            SWITCH_BR(ONE_MINUS_DST_COLOR)\
            SWITCH_BR(SRC_ALPHA)\
            SWITCH_BR(ONE_MINUS_SRC_ALPHA)\
            SWITCH_BR(DST_ALPHA)\
            SWITCH_BR(ONE_MINUS_DST_ALPHA)\
            SWITCH_BR(CONSTANT_COLOR)\
            SWITCH_BR(ONE_MINUS_CONSTANT_COLOR)\
            SWITCH_BR(CONSTANT_ALPHA)\
            SWITCH_BR(ONE_MINUS_CONSTANT_ALPHA)\
            SWITCH_BR(SRC_ALPHA_SATURATE)\
        }

    SWITCH_SENTENCE(saved_blend_src_rgb);
    driver->writeGPURegister(GPU_BLEND_SRC_RGB, data);

    SWITCH_SENTENCE(saved_blend_src_alpha);
    driver->writeGPURegister(GPU_BLEND_SRC_ALPHA, data);

    SWITCH_SENTENCE(saved_blend_dst_rgb);
    driver->writeGPURegister(GPU_BLEND_DST_RGB, data);

    SWITCH_SENTENCE(saved_blend_dst_alpha);
    driver->writeGPURegister(GPU_BLEND_DST_ALPHA, data);

    #undef SWITCH_SENTENCE
    #undef SWITCH_BR

    #define SWITCH_BR(v) case GL_##v: data.blendEquation = BLEND_##v; break;

    switch ( saved_blend_equation )
    {
        SWITCH_BR(FUNC_ADD)
        SWITCH_BR(FUNC_SUBTRACT)
        SWITCH_BR(FUNC_REVERSE_SUBTRACT)
        SWITCH_BR(MIN)
        SWITCH_BR(MAX)
    }

    #undef SWITCH_BR

    driver->writeGPURegister(GPU_BLEND_EQUATION, data);
    data.booleanVal = saved_enable_blend;
    driver->writeGPURegister(GPU_COLOR_BLEND, data);


    // RESTORE SCISSOR TEST STATE
    if (saved_enable_scissor)
        ctx->testAndSetFlags(GLContext::flagScissorTest);
    else
        ctx->testAndResetFlags(GLContext::flagScissorTest);

    ctx->setStencilOp(saved_stencilBufferFailFunc, saved_stencilBufferZFailFunc, saved_stencilBufferZPassFunc);
    ctx->setStencilBufferUpdateMask(saved_stencilBufferUpdateMask);
    ctx->setStencilBufferClearValue(saved_stencilBufferClearValue);
    ctx->setStencilBufferFunc(saved_stencilBufferFunc, saved_stencilBufferReferenceValue, saved_stencilBufferCompareMask);

    data.booleanVal = saved_enable_scissor;
    driver->writeGPURegister(GPU_SCISSOR_TEST, data);

      #define _SWITCH(fail_) \
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
        }

    _SWITCH(saved_stencilBufferFailFunc)
    driver->writeGPURegister(GPU_STENCIL_FAIL_UPDATE, data);
    _SWITCH(saved_stencilBufferZFailFunc)
    driver->writeGPURegister(GPU_DEPTH_FAIL_UPDATE, data);
    _SWITCH(saved_stencilBufferZPassFunc)
    driver->writeGPURegister(GPU_DEPTH_PASS_UPDATE, data);

    #undef _SWITCH

    data.uintVal = saved_stencilBufferUpdateMask;
    driver->writeGPURegister(GPU_STENCIL_UPDATE_MASK, data);
    data.intVal = saved_stencilBufferClearValue;
    driver->writeGPURegister(GPU_STENCIL_BUFFER_CLEAR, data);
    data.uintVal = saved_stencilBufferCompareMask;
    driver->writeGPURegister(GPU_STENCIL_COMPARE_MASK, data);
    data.uintVal = (u32bit)saved_stencilBufferReferenceValue;
    driver->writeGPURegister(GPU_STENCIL_REFERENCE, data);

    #define BRANCH_CASE(c) case GL_##c : data.compare = GPU_##c; break;

    switch ( saved_stencilBufferFunc )
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

    // RESTORE DEPTH MASK & DEPTH TEST

    if (saved_enable_depth)
        ctx->setFlags(GLContext::flagDepthTest);
    else
        ctx->resetFlags(GLContext::flagDepthTest);

    ctx->setDepthMask(saved_depth_mask);

    data.booleanVal = saved_enable_depth;
    driver->writeGPURegister(GPU_DEPTH_TEST, data);

    data.booleanVal = saved_depth_mask;
    driver->writeGPURegister(GPU_DEPTH_MASK, data);

    // RESTORE COLOR MASK
    ctx->setColorMask(saved_red, saved_green, saved_blue, saved_alpha);
    data.booleanVal = saved_red;
    driver->writeGPURegister(GPU_COLOR_MASK_R, data);
    data.booleanVal = saved_green;
    driver->writeGPURegister(GPU_COLOR_MASK_G, data);
    data.booleanVal =  saved_blue;
    driver->writeGPURegister(GPU_COLOR_MASK_B, data);
    data.booleanVal = saved_alpha;
    driver->writeGPURegister(GPU_COLOR_MASK_A, data);

    /*****************
       END RESTORE
     ****************/
}

void afl::performBlitOperation(GLint xoffset, GLint yoffset, GLsizei texWidth, GLint x, GLint y, GLsizei width, GLsizei height, u32bit md, GLenum ifmt, GLsizei &size)
{
    GPURegData data;
    unsigned int blittedTexels;
    
    if (xoffset < 0 || yoffset < 0)
        panic("AuxFuncsLib","performBlitOperation()","Texture subregions are not allowed to start at negative x,y coordinates");
        
    if ( x < 0 || y < 0 )
        panic("AuxFuncsLib","performBlitOperation()","Framebuffer regions to blit are not allowed to start at negative x,y coordinates");
        
    if (width < 0 || height < 0 )
        panic("AuxFuncsLib","performBlitOperation()","Blitting of negative regions of the framebuffer not allowed");
        
    blittedTexels = width*height;
    
    switch ( ifmt )
    {
        case GL_RGB:
        case GL_RGB8:
        case GL_RGBA:
        case GL_RGBA8:
            
            data.txFormat = GPU_RGBA8888;
            driver->writeGPURegister(GPU_BLIT_DST_TX_FORMAT, 0, data);
            size = blittedTexels * 4;
            break;
        
        default:
            panic("AuxFuncsLib","performBlitOperation()", "Unsupported Blit operation texture format");
    }
    
//printf("AuxFuncsLib::preformBlitOperation => Forcing swap before blit.\n");
//driver->sendCommand(GPU_SWAPBUFFERS);
    
    data.uintVal = (u32bit) x;
    driver->writeGPURegister(GPU_BLIT_INI_X, 0, data);
    data.uintVal = (u32bit) y;
    driver->writeGPURegister(GPU_BLIT_INI_Y, 0, data);
    data.uintVal = (u32bit) xoffset;
    driver->writeGPURegister(GPU_BLIT_X_OFFSET, 0, data);
    data.uintVal = (u32bit) yoffset;
    driver->writeGPURegister(GPU_BLIT_Y_OFFSET, 0, data);
    
    /* Compute the ceiling of the texture width power of two. */
    data.uintVal = (u32bit) mathlib::ceil2(mathlib::log2((u32bit) texWidth));
    driver->writeGPURegister(GPU_BLIT_DST_TX_WIDTH2, 0, data);
    
    data.uintVal = (u32bit) width;
    driver->writeGPURegister(GPU_BLIT_WIDTH, 0, data);
    data.uintVal = (u32bit) height;
    driver->writeGPURegister(GPU_BLIT_HEIGHT, 0, data);
    
    driver->writeGPUAddrRegister(GPU_BLIT_DST_ADDRESS, 0, md);

//printf("AuxFuncsLib::performBlitOperation => framebuffer start position = (%d, %d) | texture offset = (%d, %d) | dimensions = (%d, %d) | address = %08x\n",
//x, y, xoffset, yoffset, width, height, md);

    driver->sendCommand(GPU_BLIT);
}
