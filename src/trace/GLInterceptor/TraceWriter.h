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

#ifndef TRACEWRITER_H
    #define TRACEWRITER_H

#include "glAll.h"
#include "GLResolver.h"
//#include <fstream>
#include "zfstream.h"

/*
 * Writes info in a file
 */
class TraceWriter
{

public:

    enum TW_MODE
    {
        devNull = 0, /* Do not write anyting */
        binary, /* True binary mode ( fully reproducible ) */
        hex, /* Hex mode ( aprox. readable & fully reproducible)*/
        text /* Readable mode ( fully readable but not perfectly reproducible ) */
    };

    TraceWriter();

    /**
     * Writes trace format
     */
    void writeTraceFormat( const char* appStr = "GLI0" );
    
    void setMode( TW_MODE mode );

    /**
     * WriteResult functions always write a 0 value
     */
    void setWriteResultToZero( bool setToZero );
    
    TW_MODE getMode() const;

    bool open( const char* stream, bool compressed = false );

    bool close();
    
       
    void write( bool value );
    
    void write( char value );

    void write( unsigned char value );
        
    void write( short value );
    
    void write( unsigned short value );    

    void write( int value );

    /* void write( GLhandleARB value ); */
    void write( unsigned int value );
    
    void write( long value );
    
    void write( unsigned long value );
    
    void write(unsigned long long value);
    
    void write(long long value);
    
    void write( float value );
    
    void write( double value );

    template<typename T>
    void writeResult( T value )
    {
        if ( mode == devNull || mode == binary )
            return ; // occupies 0 bytes in binary format
        write(value); // write result only in text or hex mode
    }

    void writeResultUnknown(const void* unknown);

    void writeResultEnum(unsigned int value);
        
    /*
     * Unique funtion for writing all vector versions
     *
     * Template mechanism creates one version for each possible version
     *
     * @note must be inlined due to VS 6.0 limitations with templates
     * @note if this code is modified the all application should be rebuilt
     */
    template<typename T>
    void write( const T *values, int count )
    {
        if ( mode == devNull ) return ;

        if ( mode == binary )
        {
            writeInlinedBufferBINARY((const char *)values,count*sizeof(T));
            return;
        }

        int i = 0;
        *ff << "{";
        for ( i = 0; i < count; i++ )
        {
            write(values[i]); // use customized version
            if ( i < count -1 )
                *ff << ",";
        }
        *ff << "}";
    }

    /** 
     * Write text
     */
    void write( const char* str );

    /**
     * Deprecated
     *
     * @see writeAPICall( APICall apicall)
     */
    //void writeAPICall( const char* str );

    /**
     * Writes call name
     *
     * @note: must be used instead of previous version
     */
    void writeAPICall( APICall apicall);

    /**
     * Method for write a buffer ID independent of current TW_MODE
     *
     * @note use instead of the previous method ( 2 WRITES )
     *
     * @code
     *
     *  // old method:
     *  TraceWriter tw;
     *
     *  tr.write("*"); // indicates we are going to write an indirect buffer
     *  tr.write(bufferID); // write buffer identificator
     *  // This old method can work with binary format
     *  // "*" is replaced correctly by its identifier
     *
     *  // new method ( preferred method )
     *  tr.writeBufferID(bufferID);
     *
     *  @endcode
     */
    void writeBufferID( int id );

    /**
     * Used for writing special marks like ')' '(' , ',', etc
     *
     * Allow writing one or more consecutive marks
     */
    void writeMark( const char* mark );

    void writeUnknown( const void* unknown );
    void writeUnknown( const void** unknown, int count);

    void writeOring( GLbitfield value );

    void writeEnum( GLenum value );
    void writeEnum( GLenum *values, int count );

    void writeAddress(unsigned int value);

    void flush();

    unsigned long tellp();

    void seekp(unsigned long pos);

    void write( HDC hdc );
    void write( HGLRC hglrc );

    void writeComment( const char* comment, bool skipDevNull = true );
    
    void writeResolution( GLint width, GLint height);

    ~TraceWriter();

private:

    unsigned int posRes;

    bool resultSetToZero;

    bool compressedStream;

    TraceWriter( const TraceWriter& );

    void writeHEX( const void* data, int size );    

    void writeInlinedBufferBINARY( const char* data, int bytes);

    std::ostream *ff;
    std::ofstream of; /* output stream.  */
    gzofstream gzf; /* compressed output stream.  */

    TW_MODE mode; /* Working mode */

};


#endif /* TRACEWRITER_H */
