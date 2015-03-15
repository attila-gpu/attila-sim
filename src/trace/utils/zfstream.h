#ifndef zfstream_h
#define zfstream_h

#include <fstream>
#include <stdio.h>

//#ifdef WIN32
//    #ifndef ZLIB_WINAPI
//        #define ZLIB_WINAPI
//    #endif
//#endif

#include "zlib.h"

using namespace std;

class gzfilebuf : public streambuf {

public:

    gzfilebuf( );
    virtual ~gzfilebuf();

    gzfilebuf *open( const char *name, int io_mode );
    gzfilebuf *attach( int file_descriptor, int io_mode );
    gzfilebuf *close();

    int setcompressionlevel( int comp_level );
    int setcompressionstrategy( int comp_strategy );

    inline int is_open() const { return (file !=NULL); }

    virtual streampos seekoff(streamoff, ios_base::seekdir, int );
    virtual streampos seekpos(streamoff, int);

    virtual int sync();

protected:

    virtual int underflow();
    virtual int overflow( int = EOF );

private:

    static const int BUFFER_SIZE = 65536;
    char *readBuffer;
    char *writeBuffer;

    long gzPosition;
    long gzNextReadPosition;

    FILE *log;

    gzFile file;
    short mode;
    short own_file_descriptor;

    int flushbuf();
    int fillbuf();

    long out_waiting();
};

class gzofstream;
class gzifstream;

class gzfilestream_common : virtual public ios {

    friend class gzifstream;
    friend class gzofstream;
    friend gzofstream &setcompressionlevel( gzofstream &, int );
    friend gzofstream &setcompressionstrategy( gzofstream &, int );

public:
    virtual ~gzfilestream_common();

    void attach( int fd, int io_mode );
    void open( const char *name, int io_mode );
    void close();

    bool is_open()
    {
        return (buffer.is_open()) ? true : false;
    };

protected:
    gzfilestream_common();

private:
    gzfilebuf *rdbuf();

    gzfilebuf buffer;

};

class gzifstream : public gzfilestream_common, public istream {

    public:

    gzifstream();
    gzifstream( const char *name, int io_mode = ios::in | ios::binary );
    gzifstream( int fd, int io_mode = ios::in | ios::binary );

    virtual ~gzifstream();

    pos_type tellg()
    {
        return buffer.seekoff(0, cur, in);
    }

    gzifstream& seekg(pos_type _Pos, ios::seekdir _Way)
    {
        buffer.seekoff(_Pos, _Way, ios::in);

        return *this;
    }

};

class gzofstream : public gzfilestream_common, public ostream {

    public:

    gzofstream();
    gzofstream( const char *name, int io_mode = ios::out | ios::binary );
    gzofstream( int fd, int io_mode = ios::out | ios::binary );

    void setcompressionlevel(int l)
    {
            rdbuf()->setcompressionlevel(l);
    };

    virtual ~gzofstream();

    pos_type tellp()
    {
        return buffer.seekoff(0, cur, ios::in);
    }

    gzofstream& seekp(pos_type _Pos)
    {
        buffer.seekpos(_Pos, in);

        return *this;
    }
};


#endif
