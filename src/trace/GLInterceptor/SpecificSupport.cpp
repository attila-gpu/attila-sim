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

#include "SpecificSupport.h"
#include "GLInterceptor.h"
#include "GLResolver.h"
#include <sstream>
#include <cstdio>
#include <vector>
#include "includelog.h"

using namespace std;
using includelog::logfile;

BufferObjectStruct boArray;
BufferObjectStruct boElementArray;

SpecificSupportInitializer::SpecificSupportInitializer()
{    
    boArray.currentBind = 0;
    boArray.addr = 0;
    boArray.dataBind = 0;
    boArray.contents = 0;
    boArray.mappedAddr = 0;

    boElementArray.currentBind = 0;
    boElementArray.addr = 0;
    boElementArray.dataBind = 0;
    boElementArray.contents = 0;
    boElementArray.mappedAddr = 0;

    //LOG(0, Log::log() << "OK\n";)
}

static SpecificSupportInitializer ssinit;

/* extern */
PointerStruct arrayPointer[13] =
{
    INIT_POINTER, // vertex pointer
    INIT_POINTER, // color pointer
    INIT_POINTER, // index pointer
    INIT_POINTER, // normal pointer
    INIT_POINTER, // tex coord pointer 0
    INIT_POINTER, // tex coord pointer 1
    INIT_POINTER, // tex coord pointer 2
    INIT_POINTER, // tex coord pointer 3
    INIT_POINTER, // tex coord pointer 4
    INIT_POINTER, // tex coord pointer 5
    INIT_POINTER, // tex coord pointer 6
    INIT_POINTER, // tex coord pointer 7
// Add more if required
    INIT_POINTER // Edge flags
};

PointerStruct genericPointer[16] =
{
    INIT_POINTER,
    INIT_POINTER,
    INIT_POINTER,
    INIT_POINTER,
    INIT_POINTER,
    INIT_POINTER,
    INIT_POINTER,
    INIT_POINTER,
    INIT_POINTER,
    INIT_POINTER,
    INIT_POINTER,
    INIT_POINTER,
    INIT_POINTER,
    INIT_POINTER,
    INIT_POINTER,
    INIT_POINTER
};

string globalMsg;
bool vertexProgramEnabled = false;
GLuint clientActiveTexture = TEXTURE_COORD;

using namespace std;

/* Assume all buffers are static */
void pointerArrayCall( const char* callName, PointerTag pt, GLint size, GLenum type, GLsizei stride, const GLvoid* ptr )
{    
    PointerStruct& ps = arrayPointer[pt];

    ps.size = size;
    ps.type = type;
    ps.stride = stride;

    if ( boArray.isBound() )
    {
        /* it is merely an offset */
        ps.bufferBound = true;
        WRITER.write((unsigned int)ptr);
        return ;
    }

    ps.bufferBound = false;

    if ( !LOG_BUFFERS_MACRO )
    {
        WRITER.writeAddress((uint)ptr);
        return ;
    }

    
    if ( ps.enabled && ps.buf != NULL )
    {
        stringstream ss;
        ss << "Warning. Modifying enabled pointer (" << callName << ")\n";
        logfile().pushInfo(__FILE__,__FUNCTION__);
        logfile().write(includelog::Warning, ss.str());
        logfile().popInfo();
        /* Modifying an enabled pointer !!! */
        /* Some aplications do not disable the pointer between pointer switching */
        /* Ignore this warning... */
    }

    /* A new buffer descriptor is always created, never shared */
    //ps.buf = BufferObj::createBuffer(ptr, 0);
    ps.buf = BM.createDeferred((const BPtr)ptr);


    /* log parameter 4 */    
    //WRITER.writeBufferID(ps.buf->getId());    
    WRITER.writeBufferID(ps.buf->getID());    
}

void interleavedArrayCall(PointerTag pt, GLint size, GLenum type, GLsizei stride, const GLvoid* ptr, GLuint offset)
{
    PointerStruct& ps = arrayPointer[pt];

    ps.size = size;
    ps.type = type;
    ps.stride = stride;
    ps.enabled = true;
    //ps.buf = BufferObj::createBuffer(((GLubyte*)ptr) + offset, 0);
    ps.buf = BM.createDeferred( (const BPtr)(((GLubyte*)ptr)+offset));
}

int getPixelTypeSize( GLenum pixelTypeConstant )
{
    char message[256];
    switch ( pixelTypeConstant )
    {
        case GL_UNSIGNED_BYTE:
        case GL_BYTE:
            return sizeof(GLbyte);
        case GL_UNSIGNED_SHORT:
        case GL_SHORT:
            return sizeof(GLshort);
        case GL_INT:
        case GL_UNSIGNED_INT:
            return sizeof(GLint);
        case GL_FLOAT:
            return sizeof(GLfloat);
        case GL_UNSIGNED_INT_8_8_8_8:
            return 1;
        case GL_BITMAP:
            logfile().pushInfo(__FILE__,__FUNCTION__);
            logfile().write(includelog::Panic, "Panic. GL_BITMAT is currently unsupported");
            logfile().popInfo();
            panic("Specific Support","getPixelTypeSize()","GL_BITMAT is currently unsupported");
            return sizeof(GLbyte);
        default:
            const char* name = GLResolver::getConstantName(pixelTypeConstant);
            if ( name == NULL )
                name = "UNKNOWN";
            sprintf(message,"Unknown or unsupported pixel type.\nValue = 0x%X\nConstant name: %s",pixelTypeConstant,name);
            logfile().pushInfo(__FILE__,__FUNCTION__);
            logfile().write(includelog::Panic, message);
            logfile().popInfo();
            panic("Specific Support", "getPixelTypeSize()",message);
            return sizeof(GLint);
    }
}

int getPixelFormatSize( GLenum formatConstant )
{
    char message[256];
    switch ( formatConstant )
    {
        case GL_COLOR_INDEX:
        case GL_RED:
        case GL_BLUE:
        case GL_GREEN:
        case GL_ALPHA:
        case GL_LUMINANCE:
        case GL_STENCIL_INDEX:
        case GL_DEPTH_COMPONENT:
            return 1;
        case GL_LUMINANCE_ALPHA:
            return 2;
        case GL_RGB:
        case GL_BGR:
            return 3;
        case GL_RGBA:
        case GL_BGRA:
            return 4;
        default:
            const char* name = GLResolver::getConstantName(formatConstant);
            if ( name == NULL )
                name = "UNKNOWN";
            sprintf(message,"Unknown or unsupported pixel format type. Value = 0x%X\nConstant name: %s",formatConstant,name);
            logfile().pushInfo(__FILE__,__FUNCTION__);
            logfile().write(includelog::Panic, message);
            logfile().popInfo();
            panic("Specific support", "getPixelFormatSize()",message);
            return sizeof(GLint);
    }
}

int getTypeSize( GLenum openGLType ) 
{
    char message[256];
    switch ( openGLType ) {
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
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
            const char* name = GLResolver::getConstantName(openGLType);
            if ( name == NULL )
                name = "UNKNOWN";
            sprintf(message,"Unknown openGL type size.Value = 0x%X\nConstant name: %s",openGLType,name);
            logfile().pushInfo(__FILE__,__FUNCTION__);
            logfile().write(includelog::Panic, message);
            logfile().popInfo();
            panic("Specific support", "getTypeSize()",message);
            return sizeof(GLuint);
    }
}


int findMaxIndex( GLsizei count, GLenum type, const void *indices )
{
    int iMax = 0;
    int i;
    /* Pointers for each possible index type */
    GLubyte *indices_ub;
    GLushort *indices_us;
    GLuint *indices_ui;

    char myBuf[256];

    switch ( type ) 
    {
        case GL_UNSIGNED_BYTE :
            indices_ub = (GLubyte *)indices;
            for ( i = 1; i < count; i++ ) {
                if ( indices_ub[i] > indices_ub[iMax] )
                    iMax = i;
            }
            return (int)indices_ub[iMax];
        case GL_UNSIGNED_SHORT :            
            indices_us = (GLushort *)indices;        
            for ( i = 1; i < count; i++ ) 
            {                
                if ( indices_us[i] > indices_us[iMax] )
                {
                    iMax = i;
                }
            }
            return (int)indices_us[iMax];
        case GL_UNSIGNED_INT :
            indices_ui = (GLuint *)indices;
            for ( i = 1; i < count; i++ ) {
                if ( indices_ui[i] > indices_ui[iMax] )
                    iMax = i;
            }
            return (int)indices_ui[iMax];
        default:
            sprintf(myBuf, "Index format %s not supported by GLInterceptor", GLResolver::getConstantName(type));
            logfile().pushInfo(__FILE__,__FUNCTION__);
            logfile().write(includelog::Panic, myBuf);
            logfile().popInfo();
            panic("SpecificSupport","findMaxIndex()", myBuf);
            return 0;
    }
}

/*
 * size : how many components per vertex data element
 * type : type of each individual components in a vertex data element
 * stride : buffer stride
 * count : number of vertex data elements in buffer
 */
int computeBufferSize( int size, GLenum type, int stride, int count )
{
    int x = size * getTypeSize(type); /* Vertex Data size */
    if ( stride != 0 )
    {
        if ( stride < x )
            return -1; /* Error stride lower than vertex data element */
        else
            x = stride * count - (stride - x);            
    }
    else
        x *= count;
    
    return x; /* Total size */
}


int configureVertexArrays(GLuint start, GLuint count)
{
    vector<BufferDescriptor*> bObj;
    
    //int maxPointers = 0;
    GLuint i;

    for ( i = 0; i < MAX_POINTERS; i++ )
    {
        if ( arrayPointer[i].enabled && !arrayPointer[i].bufferBound ) /* new pointer buffer */
        {
            if ( arrayPointer[i].buf == NULL )
            {
                logfile().pushInfo(__FILE__,__FUNCTION__);
                logfile().write(includelog::Panic, "Panic. glDrawElements() using null pointer as buffer\n");
                logfile().popInfo();
                panic("Specific", "glDrawElements()", "Buffer with null pointer");
            }

            GLuint size = computeBufferSize(arrayPointer[i].size, arrayPointer[i].type,
                                            arrayPointer[i].stride, count);
            if ( arrayPointer[i].buf->isDeferred() )
            {
                arrayPointer[i].buf->setSize( size );
                bObj.push_back( arrayPointer[i].buf );
            }
            else
            {
                // it is the vertex array used just before used because a 
                // new call to glXXXPointer creates a new (deferred) buffer
                // SO, if it is not deferred, it was used just before.
                if ( arrayPointer[i].buf->getSize() < size )
                {
                    // Size should remain constant...
                    stringstream ss;
                    ss << "GLI Unsupported Feature -> Reusing a vertex array buffer (id: " << arrayPointer[i].buf->getID() <<
                        " but with possibly different format (size). Prev. size: " <<
                        arrayPointer[i].buf->getSize() << " New size: " << size;
                    logfile().pushInfo(__FILE__,__FUNCTION__);
                    logfile().write(includelog::Panic, ss.str());
                    logfile().popInfo();
                    panic("SpecificSupport", "configureVertexArrays", ss.str().c_str());
                    // Supporting this feature can be implemented allowing the growing
                    // of memory regions and lazy commit behaviour
                    // i.e:
                    // glVertexPOinter(.., *30);
                    // ...
                    // glDrawElements(...);
                    // ...
                    // glDrawElements(...);
                    // glVertexPointer(..., *56); -> At this point *30 can be commited
                }
                else
                {
                    // check that pointer contents have not changed (should not)
                    const BPtr ptr = (const BPtr) arrayPointer[i].buf->getStartAddress();
                    if ( !arrayPointer[i].buf->equals(ptr, size) )
                    {
                        stringstream ss;
                        ss << "Pointer contents of buffer: " <<
                            arrayPointer[i].buf->getID() << " has been modified. " 
                            << " Prev. size: " << arrayPointer[i].buf->getSize() 
                            << "  New size: " << size;
                        ss << "\n\n" << globalMsg;
                        logfile().pushInfo(__FILE__, __FUNCTION__);
                        logfile().write(includelog::Warning, ss.str());
                        logfile().popInfo();
                        //popup("SpecificSupport::configureVertexArrays", ss.str().c_str());
                    }
                    /*
                    else
                    {    
                        stringstream ss;
                        ss << "Pointer contents of buffer: " <<
                            arrayPointer[i].buf->getID() << " has not changed. " 
                            << " Prev. size: " << arrayPointer[i].buf->getSize() 
                            << "  New size: " << size;
                        popup("SpecificSupport::configureVertexArrays", ss.str().c_str());
                    }
                    */

                }
            }
        }
    }

    for ( i = 0; i < MAX_GENERIC_POINTERS; i++ )
    {
        if ( genericPointer[i].enabled && !genericPointer[i].bufferBound )
        {
            if ( genericPointer[i].buf == NULL )
            {
                logfile().pushInfo(__FILE__,__FUNCTION__);
                logfile().write(includelog::Panic, "Panic. glDrawElements() using null pointer as buffer (generic pointer)\n");
                logfile().popInfo();
                panic("Specific", "glDrawElements()", "Buffer with null pointer");
            }

            GLuint size = computeBufferSize(genericPointer[i].size, genericPointer[i].type,
                                            genericPointer[i].stride, count);

            if ( genericPointer[i].buf->isDeferred() )
            {
                genericPointer[i].buf->setSize( size );
                bObj.push_back( genericPointer[i].buf );
            }
            else
            {
                // it is the vertex array used just before used because a 
                // new call to glXXXPointer creates a new (deferred) buffer
                // SO, if it is not deferred, it was used just before.
                if ( genericPointer[i].buf->getSize() < size )
                {
                    // Size should remain constant...
                    stringstream ss;
                    ss << "(2) GLI Unsupported Feature -> Reusing a vertex array buffer (id: " << arrayPointer[i].buf->getID() <<
                        " but with possibly different format (size). Prev. size: " <<
                        arrayPointer[i].buf->getSize() << " New size: " << size;
                    logfile().pushInfo(__FILE__,__FUNCTION__);
                    logfile().write(includelog::Panic, ss.str());
                    logfile().popInfo();
                    panic("SpecificSupport", "configureVertexArrays", ss.str().c_str());
                    // see above (with conventional vertex arrays)
                }
            }            
        }
    }

    BM.setMemory(bObj);

    // Checking for memory regions assignments to buffer descriptors
    for ( i = 0; i < bObj.size(); i++ )
    {
        if ( bObj[i]->isDeferred() )
        {
            stringstream ss;
            ss << "Buffer " << bObj[i]->getID() << " is deferred after configureVertexArray";
            logfile().pushInfo(__FILE__,__FUNCTION__);
            logfile().write(includelog::Panic, ss.str());
            logfile().popInfo();
            panic("SpecificSupport","configureVertexArray", ss.str().c_str());
        }
    }

    return bObj.size();
}


void generateArrayPointerCall(GLuint pointerID, bool isGeneric)
{
    WRITER.writeComment("GLInterceptor call used for creating a new array"
                        " pointer with updated contents");
    if ( !isGeneric )
    {
        if ( pointerID == VERTEX )
            WRITER.writeAPICall(APICall_glVertexPointer);
        else if ( pointerID == COLOR )
            WRITER.writeAPICall(APICall_glColorPointer);
        else if ( pointerID == INDEX )
            WRITER.writeAPICall(APICall_glIndexPointer);
        else if ( pointerID == NORMAL )
            WRITER.writeAPICall(APICall_glNormalPointer);
        else if ( TEXTURE_COORD <= pointerID && pointerID <= TEXTURE_COORD_MAX )
            WRITER.writeAPICall(APICall_glTexCoordPointer);
        else if ( pointerID == EDGE_FLAG )
            WRITER.writeAPICall(APICall_glEdgeFlagPointer);
        else
        {
            stringstream ss;
            ss << "Unknown pointer tag: " << pointerID;
            logfile().pushInfo(__FILE__,__FUNCTION__);
            logfile().write(includelog::Panic, ss.str());
            logfile().popInfo();
            panic("SpecificSupport", "generateArrayPointerCall", ss.str().c_str());
        }

        WRITER.writeMark("(");
        PointerStruct& ps = arrayPointer[pointerID];
        if ( pointerID == VERTEX || pointerID == COLOR || pointerID == TEXTURE_COORD )
        {
            WRITER.write(ps.size); 
            WRITER.writeMark(",");
        }
        if ( pointerID != EDGE_FLAG )
        {
            WRITER.writeEnum(ps.type); 
            WRITER.writeMark(",");
        }
        WRITER.write(ps.stride); 
        WRITER.writeMark(",");
        ps.buf = BM.createDeferred((const BPtr)ps.buf->getStartAddress());
        WRITER.writeBufferID(ps.buf->getID());
    }
    else // generic pointer
    {
        if ( pointerID >= MAX_GENERIC_POINTERS )
            panic("SpecificSupport", "generateArrayPointerCall", "Too high generic pointer");
        
        PointerStruct& ps = genericPointer[pointerID];
        WRITER.writeAPICall(APICall_glVertexAttribPointerARB);
        WRITER.writeMark("(");
        WRITER.write(pointerID);
        WRITER.writeMark(",");
        WRITER.write(ps.size);
        WRITER.writeMark(",");
        WRITER.writeEnum(ps.type);
        WRITER.writeMark(",");
        WRITER.write(ps.normalized);
        WRITER.writeMark(",");
        WRITER.write(ps.stride);
        WRITER.writeMark(",");
        ps.buf = BM.createDeferred((const BPtr)ps.buf->getStartAddress());
        WRITER.writeBufferID(ps.buf->getID());
    }
    WRITER.writeMark(")\n");
    WRITER.writeComment("End of call generated by GLInterceptor");    
}

void generateExtraArrayPointersAux(GLuint count, PointerStruct& ps, GLuint psIndex, bool isGeneric)
{
    if ( ps.enabled && !ps.bufferBound )
    {
        if ( ps.buf == 0 )
        {
            if (LOG_BUFFERS_MACRO)
                panic("SpecificSupport", "generateExtraArrayPointersAux", "Buffer with null pointer");
            else
                return;
        }
        
        if ( !ps.buf->isDeferred() )
        {
            // Compute new buffer size
            GLuint newSize = computeBufferSize(ps.size, ps.type, ps.stride, count);
            if ( ps.buf->getSize() < newSize )
            {
                // requires new glXXXCall since the buffer contents are larger which implies
                // that are different
                generateArrayPointerCall(psIndex, false);
                stringstream ss;
                ss << "BufID: " << ps.buf->getID() << ". Generating extra call. Larger buffer. Prev. size: "
                    << ps.buf->getSize() << "  New size: " << newSize;
                logfile().pushInfo(__FILE__,__FUNCTION__);
                logfile().write(includelog::Warning, ss.str());
                logfile().popInfo();
                //panic("SpecificSupport","generateExtraArrayPointersAux",ss.str().c_str());
            }
            else
            {
                const BPtr ptr = (const BPtr)ps.buf->getStartAddress();
                if ( !ps.buf->equals(ptr, newSize) )
                {
                    stringstream ss;
                    ss << "BufID: " << ps.buf->getID() << 
                        ". Generating extra call (different contents)";
                    logfile().pushInfo(__FILE__,__FUNCTION__);
                    logfile().write(includelog::Warning, ss.str());
                    logfile().popInfo();
                    // requires new glXXXCall -> contents are differents
                    generateArrayPointerCall(psIndex, false);
                    //panic("SpecificSupport","generateExtraArrayPointersAux",ss.str().c_str());
                }
            }
        }
    }
}

void generateExtraArrayPointers(GLuint count)
{
    GLuint i = 0;
    for ( i = 0; i < MAX_POINTERS; i++ )
        generateExtraArrayPointersAux(count, arrayPointer[i], i, false);
    
    for ( i = 0; i < MAX_GENERIC_POINTERS; i++ )
        generateExtraArrayPointersAux(count, genericPointer[i], i, true);
}