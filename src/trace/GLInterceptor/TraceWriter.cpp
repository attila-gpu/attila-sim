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

#include "TraceWriter.h"
#include "support.h"
#include "GLResolver.h"
#include <cstdio>
#include <iostream>

#include <utility> /* include pair<T1,T2> */

using namespace std;

#define CHECK_DEVNULL if ( mode == devNull ) return ;

typedef std::pair<GLenum,const char*> Map;
#define MAKE_MAP(value) Map(value,#value)


TraceWriter::TraceWriter() : mode(TraceWriter::hex), resultSetToZero(false), posRes(0)
{    
    /* Empty */
}

bool TraceWriter::open( const char *stream, bool compressed )
{

    compressedStream = compressed;

    if (compressed)
    {
        gzf.open(stream, ios::binary | ios::out);
        if ( !gzf.is_open() )
            return false;     
        ff = &gzf;
    }
    else
    {
        of.open(stream, ios::binary | ios::out);
        if ( !of.is_open() )
            return false;     
        ff = &of;
    }

    //ff->open( stream, ios::binary | ios::out);
    
    //if ( !ff->is_open() )
    //    return false;     
    return true;
}

void TraceWriter::writeComment( const char* comment, bool skipDevNull )
{
    if ( mode == TraceWriter::binary )
        return ;

    if ( !skipDevNull )
    {
        CHECK_DEVNULL;
    }

    *ff << "# ";

    while ( *comment != '\0' )
    {
        if ( *comment == '\n' )
            *ff << "\n# ";
        else
            *ff << *comment;
        comment++;
    }
    *ff << "\n";
}

void TraceWriter::writeResolution( GLint width, GLint height)
{
    if ( mode == TraceWriter::binary )
            panic("TraceWriter", "writeResolution", "Binary mode not supported");

    CHECK_DEVNULL;

    //unsigned int prevPos = ff->tellp();
    //ff->seekp(posRes);
    unsigned int prevPos = tellp();
    seekp(posRes);
    *ff << "#! RES " << width << "x" << height;
    //ff->seekp(prevPos);
    seekp(prevPos);
}

void TraceWriter::setMode( TW_MODE mode )
{       
    this->mode = mode;
    if ( mode == TW_MODE::binary )
        panic("TraceWriter", "setMode", "Binary mode not supported for now");
}

TraceWriter::TW_MODE TraceWriter::getMode() const
{
    return mode;
}



void TraceWriter::write( const char* rawString )
{
    CHECK_DEVNULL;

    if ( mode != binary )
        *ff << rawString; 
    else
    {
        if ( *rawString == '*' && *(rawString+1) == '\0' ) 
        {   
            // for compatibility            
            char type = 2;
            ff->write(&type,sizeof(unsigned char));
        }
        else 
            writeInlinedBufferBINARY(rawString,strlen(rawString)); // inlined string
    }
}

void TraceWriter::write( bool value )
{
    CHECK_DEVNULL;
    
    //popup("Writing bool!","Writing bool");

    if ( mode == TraceWriter::binary )
    {
        char _value = ( value == 0 ? 0 : 1 );
        ff->write(&_value,1);
    }
    else if ( mode == TraceWriter::hex || mode == TraceWriter::text )
        *ff << ( value ? "1" : "0" );
}

void TraceWriter::write( HDC hdc )
{
    write((unsigned int)hdc);
}

void TraceWriter::write( HGLRC hglrc )
{
    write((unsigned int)hglrc);
}


void TraceWriter::write( char value )
{
    CHECK_DEVNULL;

    if ( mode == binary )
        ff->write(&value, sizeof(value));
    else 
        *ff << value; // hex or text
}

void TraceWriter::write( unsigned char value_ )
{
    CHECK_DEVNULL;

    /* opengl specifies GLboolean like an unsigned char */

    char value = static_cast<char>(value_);

    if ( mode == binary )
        ff->write(&value, sizeof(value));
    else if ( mode == hex || mode == text )
        *ff << ( value ? "1" : "0");
        //*ff << ( value != 0 ? "TRUE" : "FALSE");
}

void TraceWriter::write( short value )
{
    CHECK_DEVNULL;

    if ( mode == binary )
        ff->write((const char*)&value,sizeof(value));
    else 
        *ff << value; // hex or text
}

void TraceWriter::write( unsigned short value )
{
    CHECK_DEVNULL; 

    if ( mode == binary )
        ff->write((const char*)&value,sizeof(value));
    else 
        *ff << value; // hex or text
}

void TraceWriter::write( int value )
{    
    CHECK_DEVNULL;

    if ( mode == binary )
        ff->write((const char*)&value,sizeof(value));
    else 
        *ff << value; // hex or text
}

void TraceWriter::write( unsigned int value )
{
    CHECK_DEVNULL;

    if ( mode == binary )
        ff->write((const char*)&value,sizeof(value));
    else 
        *ff << value; // hex or text
}

void TraceWriter::write( long value )
{
    CHECK_DEVNULL;

    if ( mode == binary )
        ff->write((const char*)&value,sizeof(value));
    else 
        *ff << value; // hex or text
}

void TraceWriter::write( unsigned long value )
{
    CHECK_DEVNULL;

    if ( mode == binary )
        ff->write((const char*)&value,sizeof(value));
    else 
        *ff << value; // hex or text
}

void TraceWriter::write( long long value )
{
    CHECK_DEVNULL;

    if ( mode == binary )
        ff->write((const char*)&value,sizeof(value));
    else 
        *ff << value; // hex or text
}

void TraceWriter::write( unsigned long long value )
{
    CHECK_DEVNULL;

    if ( mode == binary )
        ff->write((const char*)&value,sizeof(value));
    else 
        *ff << value; // hex or text
}

void TraceWriter::write( float value )
{
    CHECK_DEVNULL;

    if ( mode == binary )
        ff->write((const char*)&value,sizeof(value));

    else if ( mode == hex )
    {
        
        /*
        char myBuffer[256];
        sprintf(myBuffer,"float: %f   - hex: 0x%x", value, *((unsigned int*)(&value)));
        POPUP("TraceWriter::write() - HEX MODE", myBuffer);
        */        
        *ff << "0x" << ::hex << *((unsigned int*)(&value)) << ::dec;

        
    }
    else
        *ff << value;
}

void TraceWriter::write( double value )
{
    CHECK_DEVNULL;

    if ( mode == binary )
        ff->write((const char*)&value,sizeof(value));

    else if ( mode == hex )
    {
        unsigned int* ptrU;
        unsigned int a, b;    

        // double 8bytes ( write two values of type 0xAAAAAAAA.0xBBBBBBBB )
        ptrU = ( unsigned int *)&value;
        a = *ptrU;
        b = *(ptrU+1);
        
        *ff << "0x" << ::hex << a << ".";        
        *ff << "0x" << ::hex << b << ::dec;
    }
    else
        *ff << value;
}


void TraceWriter::writeHEX( const void* data, int size )
{   
    CHECK_DEVNULL;

    unsigned char* c = (unsigned char*)data;
    for ( int i = 0; i < size; i++ )
    {
        *ff << ::hex << c[i];
        cout << "byte " << i << ": " << ::hex << c[i] << ::dec << endl;
    }
}



void TraceWriter::writeEnum( GLenum value )
{        
    CHECK_DEVNULL;

    if ( mode == binary )
        ff->write((const char*)&value,sizeof(value));
    else if ( mode == hex || mode == text )        
        //*ff << "0x" << ::hex << value << ::dec;        
    //else /* text mode */
    {
        const char* name = GLResolver::getConstantName(value);
        if (name == NULL)
            *ff << "0x" << ::hex << value << ::dec;
        else
            *ff << name;
    }        
}


bool TraceWriter::close()
{
    if (compressedStream)
    {
        gzf.close();
        if ( gzf.is_open() )
            return false;
    }
    else
    {
    {
        of.close();
        if ( of.is_open() )
            return false;
    }
    }
    return true;
}


void TraceWriter::writeUnknown( const void* unknown )
{
    CHECK_DEVNULL;

    if ( mode == binary )
        return ; /* Unknown occupies 0 bytes in binary format */

    *ff << "U0x" << (unsigned int)unknown;
}

void TraceWriter::writeResultUnknown(const void* unknown)
{    

    if ( mode == binary || mode == devNull )
        return ; // result occupies 0 bytes i binary format
    

    *ff << "U0x" << (unsigned int)unknown;
}

void TraceWriter::writeResultEnum(unsigned int value)
{
    CHECK_DEVNULL;

    if ( mode == binary )
        ff->write((const char*)&value,sizeof(value));

    else if ( mode == hex || mode == text )        
        //*ff << "0x" << ::hex << value << ::dec;        
    //else // TEXT
    {
        // translate name ( not implemented )
        const char* name = GLResolver::getConstantName(value);
        if ( !name )
            *ff << "0x" << ::hex << value << ::dec;
        *ff << name;
    }        
}


TraceWriter::~TraceWriter()
{
    if (compressedStream)
    {
        if ( gzf.is_open() )
        {
            cout << "Still opened" << endl;
        }
        else
        {
            cout << "Not opened!" << endl;
        }
    }
    else
    {
        if ( of.is_open() )
        {
            cout << "Still opened" << endl;
        }
        else
        {
            cout << "Not opened!" << endl;
        }
    }
    cout << "Bye" << endl;    
}


void TraceWriter::writeEnum( GLenum *values, int count )
{
    CHECK_DEVNULL;

    int i = 0;
    *ff << "{";
    for ( i = 0; i < count; i++ )
    {
        writeEnum(values[i]);
        if ( i < count -1 )
            *ff << ", ";
    }
    *ff << "}";
}

void TraceWriter::writeUnknown( const void** unknown, int count)
{
    CHECK_DEVNULL;

    int i = 0;
    *ff << "{";
    for ( i = 0; i < count; i++ )
    {
        writeUnknown(unknown[i]);
        if ( i < count -1 )
            *ff << ", ";
    }
    *ff << "}";
}


/*
// O(n) for binary writes
void TraceWriter::writeAPICall( const char* apiCallName )
{
    CHECK_DEVNULL;

    if ( mode == binary ) 
    {
        // warning: assume 65536 (2^16) as the maximum value getID can return
        unsigned short id = (unsigned short)GLResolver::getFunctionID(apiCallName);
        ff->write((const char *)&id, sizeof(unsigned short)); // 2 bytes
    }
    else
        *ff << apiCallName;
}
*/

// O(1) for both text & binary writes
void TraceWriter::writeAPICall( APICall apiCallName )
{
    CHECK_DEVNULL;

    if ( mode == binary )
    {
        unsigned short id = (unsigned short)apiCallName;
        ff->write((const char*)&id, sizeof(unsigned short)); // 2 bytes
    }
    else
        *ff << GLResolver::getFunctionName(apiCallName);
}



void TraceWriter::writeInlinedBufferBINARY( const char* data, int bytes)
{
    char byteElem;
    if ( data == NULL )
    {
        byteElem = 0; // write ID value indicating is a null buffer
        ff->write(&byteElem, sizeof(unsigned char)); // finish
    }
    
    byteElem = 1; // write ID value indicating it is a inline buffer
    ff->write(&byteElem, sizeof(unsigned char));
    ff->write((const char *)&bytes, sizeof(int)); // write buffer size
    ff->write(data, bytes); // write buffer contents
}


void TraceWriter::writeBufferID( int id )
{
    CHECK_DEVNULL;

    if ( mode == binary )
    {
        char byteElem = 2; // write ID value indicating it is an indirect buffer
        ff->write(&byteElem,sizeof(unsigned char));
        ff->write((const char *)&id,sizeof(int));
    }
    else // text or hex
        *ff << "*" << id;
}

void TraceWriter::writeMark( const char* mark )
{
    if ( mode == devNull || mode == binary )
        return ; // ignore all marks
    *ff << mark;
}


void TraceWriter::writeTraceFormat( const char* appStr )
{
    CHECK_DEVNULL;        
    
    ff->write(appStr,strlen(appStr));
    
    if ( mode == binary )        
        ff->write(".b\n",3);

    else if ( mode == hex )
        ff->write(".h\n",3);
    
    else if ( mode == text )
        ff->write(".t\n",3);

    //posRes = ff->tellp(); // store the file pointer to write the resolution
    posRes = tellp(); // store the file pointer to write the resolution

    // Space reserved for writing resolution
    *ff << "#! RES           \n";
    //*ff << "#! RES **********\n";
}


void TraceWriter::flush()
{
    if (compressedStream)
        gzf.flush();
    else
        of.flush();
}

void TraceWriter::seekp(unsigned long pos)
{
    if (compressedStream)
        gzf.seekp(pos);
    else
        of.seekp(pos);
}

unsigned long TraceWriter::tellp()
{
    if (compressedStream)
        return gzf.tellp();
    else
        return of.tellp();
}

void TraceWriter::writeAddress( unsigned int value )
{
    CHECK_DEVNULL;

    *ff << "[0x0" << value << "]";
}

void TraceWriter::setWriteResultToZero( bool setToZero )
{
    resultSetToZero = setToZero;
}


void TraceWriter::writeOring( GLbitfield value )
{
    CHECK_DEVNULL;

    if ( mode == binary )// || mode == hex )
    {
        writeEnum(value);
        return ;
    }

    /* Text mode */

    /* GLbitfields */
    static Map bf[] =
    {
        MAKE_MAP(GL_CURRENT_BIT),
        MAKE_MAP(GL_POINT_BIT),
        MAKE_MAP(GL_LINE_BIT),
        MAKE_MAP(GL_POLYGON_BIT),
        MAKE_MAP(GL_POLYGON_STIPPLE_BIT),
        MAKE_MAP(GL_PIXEL_MODE_BIT),
        MAKE_MAP(GL_LIGHTING_BIT),
        MAKE_MAP(GL_FOG_BIT),
        MAKE_MAP(GL_DEPTH_BUFFER_BIT),
        MAKE_MAP(GL_ACCUM_BUFFER_BIT),
        MAKE_MAP(GL_STENCIL_BUFFER_BIT),
        MAKE_MAP(GL_VIEWPORT_BIT),
        MAKE_MAP(GL_TRANSFORM_BIT),
        MAKE_MAP(GL_ENABLE_BIT),
        MAKE_MAP(GL_COLOR_BUFFER_BIT),
        MAKE_MAP(GL_HINT_BIT),
        MAKE_MAP(GL_EVAL_BIT),
        MAKE_MAP(GL_LIST_BIT),
        MAKE_MAP(GL_TEXTURE_BIT),
        MAKE_MAP(GL_SCISSOR_BIT),
        MAKE_MAP(GL_MULTISAMPLE_BIT_ARB)
        /* MAKE_MAP(GL_ALL_ATTRIB_BITS) */ /* Disabled cause it appears in all bitfield */
    };

    bool isFirst = true;
    for ( int i = 0; i < COUNT_ARRAY(bf); i++ )
    {
        if ( (bf[i].first & value) != 0 )
        {
            if ( !isFirst )
                *ff << " | ";
            else
                isFirst = false;
            *ff << bf[i].second;
        }
    }
}
