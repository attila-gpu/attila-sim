
#include "zfstream.h"
#include <cstring>

gzfilebuf::gzfilebuf() :
  gzPosition(0),
  gzNextReadPosition(0),
  file(NULL),
  mode(0),
  own_file_descriptor(0)
{
    readBuffer = new char[BUFFER_SIZE];
    writeBuffer = new char[BUFFER_SIZE];

    memset(readBuffer, 0xDEADCAFE, BUFFER_SIZE);
    memset(writeBuffer, 0xDEADCAFE, BUFFER_SIZE);
    setg(readBuffer, readBuffer + BUFFER_SIZE, readBuffer + BUFFER_SIZE);
    //setp(writeBuffer, writeBuffer, writeBuffer + BUFFER_SIZE);
    setp(writeBuffer, writeBuffer + BUFFER_SIZE);
}

gzfilebuf::~gzfilebuf()
{
    sync();
    if (own_file_descriptor)
        close();

    //fclose(log);

    delete[] readBuffer;
    delete[] writeBuffer;
}

gzfilebuf *gzfilebuf::open(const char *name, int io_mode)
{
    //log = fopen("log.txt", "a");

    //fprintf(log, "------------------------\n");
    //fprintf(log, "Abriendo Fichero : %s mode = %d\n", name, io_mode);

    if (is_open())
        return NULL;

    // Only read and write modes allowed
    if ((io_mode & ios::in) && (io_mode & ios::out))
        return NULL;

    char char_mode[10];
    char *p = char_mode;

    if (io_mode & ios::in)
    {
        mode = ios::in;
        *p++ = 'r';
    }
    else if (io_mode & ios::app)
    {
        mode = ios::app;
        *p++ = 'a';
    }
    else
    {
        mode = ios::out;
        *p++ = 'w';
    }

    if (io_mode & ios::binary)
    {
        mode |= ios::binary;
        *p++ = 'b';
    }

    // Hard code the compression level
    if (io_mode & (ios::out|ios::app ))
    {
        *p++ = '9';
    }

    // Put the end-of-string indicator
    *p = '\0';

    if ((file = gzopen(name, char_mode)) == NULL)
        return NULL;

    own_file_descriptor = 1;
    gzPosition = 0;
    gzNextReadPosition = 0;

    memset(readBuffer, 0xDEADCAFE, BUFFER_SIZE);
    memset(writeBuffer, 0xDEADCAFE, BUFFER_SIZE);
    setg(readBuffer, readBuffer + BUFFER_SIZE, readBuffer + BUFFER_SIZE);
    //setp(writeBuffer, writeBuffer, writeBuffer + BUFFER_SIZE);
    setp(writeBuffer, writeBuffer + BUFFER_SIZE);

    return this;
}

gzfilebuf *gzfilebuf::attach(int file_descriptor, int io_mode)
{

    if (is_open())
        return NULL;

    char char_mode[10];
    char *p = char_mode;

    if (io_mode & ios::in)
    {
        mode = ios::in;
        *p++ = 'r';
    }
    else if (io_mode & ios::app)
    {
        mode = ios::app;
        *p++ = 'a';
    }
    else if (io_mode & ios::out)
    {
        mode = ios::out;
        *p++ = 'w';
    }

    if (io_mode & ios::binary)
    {
        mode |= ios::binary;
        *p++ = 'b';
    }

    // Hard code the compression level
    if (io_mode & (ios::out|ios::app))
    {
        *p++ = '9';
    }

    // Put the end-of-string indicator
    *p = '\0';

    if ((file = gzdopen(file_descriptor, char_mode)) == NULL)
        return NULL;

    own_file_descriptor = 0;

    return this;
}

gzfilebuf *gzfilebuf::close()
{
    //fprintf(log, "Cerrando Fichero\n");
    //fprintf(log, "------------------------\n");

    if (is_open())
    {
        sync();
        gzclose( file );
        file = NULL;
    }

    //fflush(log);

    return this;
}

int gzfilebuf::setcompressionlevel(int comp_level)
{
  return gzsetparams(file, comp_level, -2);
}

int gzfilebuf::setcompressionstrategy(int comp_strategy)
{
  return gzsetparams(file, -2, comp_strategy);
}


streampos gzfilebuf::seekpos(streamoff off, int which)
{
    streampos pos;
    bool seekZip = false;

    //fprintf(log, "LOG >> seekpos >> offset %ld gzPosition %ld\n", off, gzPosition);

    //fprintf(log, "LOG >> eback %p gptr %p egptr %p\n", eback(), gptr(), egptr());
    //fprintf(log, "LOG >> pbase %p pptr %p epptr %p\n", pbase(), pptr(), epptr());

    if (mode & ios_base::in)
    {
        if ((off >= gzPosition) && (off <= (gzPosition + BUFFER_SIZE)))
        {
            setg(eback(), eback() + (off - gzPosition), egptr());
        }
        else
        {
            setg(eback(), egptr(), egptr());
            seekZip = true;
        }
    }

    if (mode & ios_base::out)
    {
        if ((off >= gzPosition) && (off <= (gzPosition + BUFFER_SIZE)))
        {
            //setp(pbase(), pbase() + (off - gzPosition), epptr());
            pbump(off - (gzPosition + (pptr() - pbase())));
        }
        else
        {
            if (out_waiting())
            {
                if (flushbuf() == EOF)
                    return streampos(EOF);
            }

            //setp(pbase(), pbase(), epptr());
            setp(pbase(), epptr());
            seekZip = true;
        }
    }

    //fprintf(log, "LOG >> eback %p gptr %p egptr %p\n", eback(), gptr(), egptr());
    //fprintf(log, "LOG >> pbase %p pptr %p epptr %p\n", pbase(), pptr(), epptr());
    //fprintf(log, "LOG >> seekZip %d\n", seekZip);

    if (seekZip)
        pos = gzseek(file, off, SEEK_SET);
    else
        pos = off;

    //fprintf(log, "LOG >> pos %ld\n", int(pos));

    return pos;
}

streampos gzfilebuf::seekoff(streamoff off, ios::seekdir dir, int which)
{
    streampos pos;
    bool seekZip = false;

    //fprintf(log, "LOG >> seekoff >> offset %ld\n", off);

    switch(dir)
    {
        case ios_base::beg:

            //fprintf(log, "LOG >> beg >> seekoff >> offset %ld\n", off);
            //fprintf(log, "LOG >> eback %p gptr %p egptr %p\n", eback(), gptr(), egptr());
            //fprintf(log, "LOG >> pbase %p pptr %p epptr %p\n", pbase(), pptr(), epptr());

            if (which & ios_base::in)
            {
                if ((off >= gzPosition) && (off <= (gzPosition + BUFFER_SIZE)))
                {
                    setg(eback(), eback() + (off - gzPosition), egptr());
                }
                else
                {
                    setg(eback(), egptr(), egptr());
                    seekZip = true;
                }
            }

            if (which & ios_base::out)
            {
                if ((off >= gzPosition) && (off <= (gzPosition + BUFFER_SIZE)))
                {
                    //setp(pbase(), pbase() + (off - gzPosition), epptr());
                    pbump(off - gzPosition);
                }
                else
                {
                    if (out_waiting())
                    {
                        if (flushbuf() == EOF)
                            return streampos(EOF);
                    }

                    //setg(pbase(), pbase(), epptr());

                    setp(pbase(), epptr());
                    seekZip = true;
                }
            }

            //fprintf(log, "LOG >> eback %p gptr %p egptr %p\n", eback(), gptr(), egptr());
            //fprintf(log, "LOG >> pbase %p pptr %p epptr %p\n", pbase(), pptr(), epptr());
            //fprintf(log, "LOG >> seekZip %d\n", seekZip);

            if (seekZip)
                pos = gzseek(file, off, SEEK_SET);
            else
                pos = off;

            //fprintf(log, "LOG >> pos %ld\n", int(pos));

            return pos;

            break;

        case ios_base::cur:

            //fprintf(log, "LOG >> cur >> seekoff >> offset %ld\n", off);
            //fprintf(log, "LOG >> eback %p gptr %p egptr %p\n", eback(), gptr(), egptr());
            //fprintf(log, "LOG >> pbase %p pptr %p epptr %p\n", pbase(), pptr(), epptr());

            //if (which & ios_base::in)
            if (mode & ios_base::in)
            {
                if (((gptr() + off) >= eback()) && ((gptr() + off) <= egptr()))
                {
                    setg(eback(), gptr() + off, egptr());
                    pos = gzPosition + (gptr() - eback());
                }
                else
                {
                    setg(eback(), egptr(), egptr());
                    seekZip = true;
                }
            }

            //if (which & ios_base::out)
            if (mode & ios_base::out)
            {
                if (((pptr() + off) >= pbase()) && ((pptr() + off) <= epptr()))
                {
                    //setp(pbase(), pptr() + off, epptr());
                    pbump(off);
                    pos = gzPosition + (pptr() - pbase());
                }
                else
                {
                    if (out_waiting())
                    {
                        if (flushbuf() == EOF)
                            return streampos(EOF);
                    }

                    //setp(pbase(), pbase(), epptr());

                    setp(pbase(), epptr());
                    seekZip = true;
                }
            }

            //fprintf(log, "LOG >> eback %p gptr %p egptr %p\n", eback(), gptr(), egptr());
            //fprintf(log, "LOG >> pbase %p pptr %p epptr %p\n", pbase(), pptr(), epptr());
            //fprintf(log, "LOG >> seekZip %d\n", seekZip);

            if (seekZip)
                pos = gzseek(file, off, SEEK_CUR);

            //fprintf(log, "LOG >> pos %ld\n", int(pos));

            return pos;

            break;

        case ios_base::end:

            // Unsupported
            streampos(EOF);
            
            return pos;
            break;

    }
}

int gzfilebuf::underflow()
{
    //fprintf(log, "Underflow \n");

    // If the file hasn't been opened for reading, error.
    if (!is_open() || !(mode & ios::in))
        return EOF;

    if (in_avail())
        return (unsigned char) *gptr();

    // Attempt to fill the buffer.
    int result = fillbuf();

    if (result == EOF)
    {
        // disable get area
        setg(0,0,0);
        return EOF;
    }

    return (unsigned char) *gptr();
}

int gzfilebuf::overflow(int c)
{
    //fprintf(log, "Overflow \n");

    if (!is_open() || !(mode & ios::out))
        return EOF;

    if (out_waiting())
    {
        if (flushbuf() == EOF)
            return EOF;
    }

    if (c != EOF)
    {
        *pptr() = c;
        pbump(1);
    }

    return 0;
}

int gzfilebuf::sync()
{
    if (!is_open())
        return EOF;

    if (out_waiting())
        return flushbuf();

    return 0;
}

long gzfilebuf::out_waiting()
{
    return pptr() - pbase();
}

int gzfilebuf::flushbuf()
{
    int n;
    int written;
    char *q;


    q = pbase();
    n = pptr() - q;

    written = gzwrite(file,q,n);
    if (written < n)
        return EOF;

    gzPosition = gzPosition + n;

    //fprintf(log, "flushbuff => written bytes %d new position %ld\n", written, gzPosition);

    setp(pbase(), epptr());

    return 0;
}

int gzfilebuf::fillbuf()
{
    int required;
    char *p;

    p = eback();

    required = egptr() - eback();

    int read = gzread(file, p, required);

    if (read <= 0) return EOF;

    gzPosition = gzNextReadPosition;
    gzNextReadPosition = gzPosition + read;

    //fprintf(log, "fillbuf => read bytes %d new position %ld\n", read, gzPosition);

    //setg(eback(), eback(), egptr());
    setg(eback(), eback(), eback() + read);

    return read;
}

gzfilestream_common::gzfilestream_common() :
  ios(gzfilestream_common::rdbuf())
{ }

gzfilestream_common::~gzfilestream_common()
{ }

void gzfilestream_common::attach(int fd, int io_mode)
{
    if (!buffer.attach( fd, io_mode))
        clear(ios::failbit | ios::badbit);
    else
        clear();
}

void gzfilestream_common::open(const char *name, int io_mode)
{
    if (!buffer.open( name, io_mode ))
        clear( ios::failbit | ios::badbit );
    else
        clear();
}

void gzfilestream_common::close()
{
    if (!buffer.close())
        clear(ios::failbit | ios::badbit);

}

gzfilebuf *gzfilestream_common::rdbuf()
{
    return &buffer;
}

gzifstream::gzifstream() :
    istream(gzfilestream_common::rdbuf())
{
    clear(ios::badbit);
}

gzifstream::gzifstream(const char *name, int io_mode) :
    istream(gzfilestream_common::rdbuf())
{
    gzfilestream_common::open(name, io_mode);
}

gzifstream::gzifstream(int fd, int io_mode) :
    istream(gzfilestream_common::rdbuf())
{
    gzfilestream_common::attach( fd, io_mode );
}

gzifstream::~gzifstream() { }

gzofstream::gzofstream() :
    ostream(gzfilestream_common::rdbuf())
{
    clear(ios::badbit);
}

gzofstream::gzofstream(const char *name, int io_mode) :
    ostream(gzfilestream_common::rdbuf())
{
    gzfilestream_common::open(name, io_mode);
}

gzofstream::gzofstream(int fd, int io_mode) :
    ostream(gzfilestream_common::rdbuf())
{
    gzfilestream_common::attach(fd, io_mode);
}

gzofstream::~gzofstream() { }
