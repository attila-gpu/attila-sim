#include "PPMFile.h"
#include <fstream>
#include <iostream>

using std::cout;
using std::endl;

PPMFile::Color::Color(Size value24bit)
{
/*
	r = (value24bit >> 16) & 0xFF;
	g = (value24bit >> 8) & 0xFF;
	b = value24bit & 0xFF;
*/
	r = Byte(value24bit & 0xFF);
	g = Byte((value24bit >> 8) & 0xFF);
	b = Byte((value24bit >> 16) & 0xFF);
}

PPMFile::Size PPMFile::Color::toValue() const
{
	//return r << 16 | g << 8 | b;
	return b << 16 | g << 8 | r;
}

PPMFile::Color PPMFile::get(Coord x, Coord y) const
{
	if ( !data )
		throw PPMUndefinedException("get", "PPM without contents defined");
	if ( x >= w )
		throw PPMAccessException("get", "Column out of bounds");
	if ( y >= h )
		throw PPMAccessException("get", "Row out of bounds");

	const Byte* ptr = &data[3*x + 3*y*w];
	Color c;
	c.r = *ptr;
	c.g = *(ptr+1);
	c.b = *(ptr+2);
	return c;
}

void PPMFile::get(Coord x, Coord y, Byte& r, Byte& g, Byte& b) const
{
	if ( !data )
		throw PPMUndefinedException("get", "PPM without contents defined");
	if ( x >= w )
		throw PPMAccessException("get", "Column out of bounds");
	if ( y >= h )
		throw PPMAccessException("get", "Row out of bounds");
	const Byte* ptr = &data[3*x + 3*y*w];
	r = *ptr;
	g = *(ptr+1);
	b = *(ptr+2);
}

void PPMFile::set(Coord x, Coord y, Color col)
{
	if ( !data )
		throw PPMUndefinedException("set", "PPM without contents defined");
	if ( x >= w )
		throw PPMAccessException("set", "Column out of bounds");
	if ( y >= h )
		throw PPMAccessException("set", "Row out of bounds");

	Byte* ptr = &data[3*x + 3*y*w];
	*ptr = col.r;
	*(ptr+1) = col.g;
	*(ptr+2) = col.b;
}

void PPMFile::set(Coord x, Coord y, Byte r, Byte g, Byte b)
{
	if ( !data )
		throw PPMUndefinedException("set", "PPM without contents defined");
	if ( x >= w )
		throw PPMAccessException("set", "Column out of bounds");
	if ( y >= h )
		throw PPMAccessException("set", "Row out of bounds");
	Byte* ptr = &data[3*x + 3*y*w];
	*ptr = r;
	*(ptr+1) = g;
	*(ptr+2) = b;
}

void PPMFile::allocate()
{
	if ( data )
		delete [] data;
	data = new Byte[3 * w * h];
}

PPMFile::~PPMFile()
{
	delete [] data;
}

PPMFile::PPMFile(std::size_t width, std::size_t height) : 
	w(width), h(height), maxColor(255), data(0)
{
	if ( height == 0 || width == 0 )
		throw PPMUndefinedException("Ctor", "height and weight must be greater than 0");
}

PPMFile::PPMFile(const PPMFile& ppmFile) : 
	w(ppmFile.w),
	h(ppmFile.h),
	maxColor(ppmFile.maxColor),
	data(0)
{
	if ( ppmFile.data )
	{
		// Copy contents if exist
		data = new Byte[3 * w * h];
		memcpy(data, ppmFile.data, 3 * w * h);
	}
}

PPMFile& PPMFile::operator=(const PPMFile& ppmFile)
{
	delete[] data; // deallocate previous pixel contents

	w = ppmFile.w;
	h = ppmFile.h;
	maxColor = ppmFile.maxColor;
	if ( ppmFile.data )
	{
		// Copy contents if exist
		data = new Byte[3 * w * h];
		memcpy(data, ppmFile.data, 3 * w * h);
	}
	return *this;
}

bool PPMFile::operator==(const PPMFile& ppmFile)
{
	if ( w != ppmFile.w || h != ppmFile.h || maxColor != ppmFile.maxColor )
		return false;
	
	if ( ppmFile.data != 0 && data != 0 )
		return memcmp(data, ppmFile.data, 3 * w * h) == 0;
	else if ( ppmFile.data == 0 && data == 0 )
		return true;
	
	return false; // only one of the PPM's is undefined
}

bool PPMFile::operator!=(const PPMFile& ppmFile)
{
	return !(*this == ppmFile);
}

void PPMFile::fill(Color c)
{
	fill(c.r, c.g, c.b);	
}

void PPMFile::fill(Byte r, Byte g, Byte b)
{
	const Size sz = 3* w * h;
	
	if ( !data )
		data = new Byte[sz];

	for ( Coord i = 0; i < sz; i+=3 )
	{
		data[i] = r;
		data[i+1] = g;
		data[i+2] = b;
	}
}

void PPMFile::write(std::string pathname) const
{
	if ( !data )
		throw PPMUndefinedException("write", "PPM file undefined yet (no data defined)");

	std::ofstream f;
	f.open(pathname.c_str(), std::ios::binary);
	
	if ( !f.is_open() )
		throw PPMOpeningException("write", "ppm output path could not be opened");

	f << "P6\n" << w << " " << h << "\n" << maxColor << "\n";
	
	const Size sz = 3 * h * w;
	for ( Size i = 0; i < sz; i++ )
		f.write((char *)&data[i], 1);
}

void PPMFile::read(std::string pathname)
{
	
	std::ifstream f;

	f.open(pathname.c_str(), std::ios::binary);
	if ( !f.is_open() )
		throw PPMOpeningException("read", "ppm file could not be opened");
	
	if ( f.eof() )
		throw PPMEOFException("read", "eof not expected");

	char c;
	
	f.get(c);
	if ( c != 'P' )
		throw PPMFormatException("read", "Byte 'P' was expected");

	if ( f.eof() )
		throw PPMEOFException("read", "eof not expected");

	f.get(c);
	if ( c != '6' )
		throw PPMFormatException("read", "Byte '6' was expected");

	// read width and height
	w = 0xFFFFFFFF;
	h = 0xFFFFFFFF;
	f >> w;
	f >> h;

	if ( w == 0xFFFFFFFF || h == 0xFFFFFFFF )
		throw PPMFormatException("read", "width and height expected");

	maxColor = 0xFFFFFFFF;
	f >> maxColor;
	if ( maxColor >= 65536 )
		throw PPMFormatException("read", "Maximum color overflow");

	if ( maxColor > 255 )
		throw PPMUnsupportedException("read", "Max color value over 255 not supported");

	f.get(c);
	if ( !isspace(c) )
		throw PPMFormatException("read", "white space character or return expected");
	
	if ( data ) // destroy previous contents
		delete[] data;

	const Size sz = 3 * h * w;
	
	
	data = new Byte[sz];
	char aux;
	
	
	for ( unsigned int i = 0; i < sz; i++ )
	{
		if ( f.eof() )
			throw PPMEOFException("read", "Unexpected EOF while reading PPM data");
		f.get(aux);
		data[i] = (Byte)aux;
	}
	
	

	// pmm read correctly
}


