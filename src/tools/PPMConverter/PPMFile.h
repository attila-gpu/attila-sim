#ifndef PPMFILE_H
	#define PPMFILE_H

#include <cctype>
#include <string>

/**
 * Class to read/write and manipulate a PPM file
 *
 * PPM spec: http://netpbm.sourceforge.net/doc/ppm.html
 */
class PPMFile
{
public:

	typedef unsigned char Byte;
	typedef unsigned int Coord;
	typedef size_t Size;

	struct Color
	{
		Byte r, g, b;
		Color() { }
		Color(Byte r, Byte g, Byte b) : r(r), g(g), b(b) {} // inline ctor
		explicit Color(Size value24bit);
		Size toValue() const;
		bool operator<(const Color& c) const
		{
			return toValue() < c.toValue();
		}
	};

	struct PPMException 
	{	
		std::string methodName;
		std::string message;

		PPMException(std::string methodName, std::string message);

		std::string toString() const;
	};

	struct PPMOpeningException : public PPMException 
	{ 
		PPMOpeningException(std::string methodName, std::string message);
	};

	struct PPMFormatException : public PPMException  
	{ 
		PPMFormatException(std::string methodName, std::string message);
	};
	
	struct PPMEOFException : public PPMException 
	{
		PPMEOFException(std::string methodName, std::string message);
	};

	struct PPMUnsupportedException : public PPMException
	{
		PPMUnsupportedException(std::string methodName, std::string message);
	};

	struct PPMAccessException : public PPMException
	{
		PPMAccessException(std::string methodName, std::string message);	
	};

	struct PPMUndefinedException : public PPMException
	{
		PPMUndefinedException(std::string methodName, std::string message);
	};

	bool isDefined() const { return data != 0; }

	PPMFile(Size width = 1, Size height = 1);
	PPMFile(const PPMFile& ppmFile);
	PPMFile& operator=(const PPMFile& ppmFile); ///< Assigning operator
	bool operator==(const PPMFile& ppmFile); ///< Checks if two PPMs are equal
	bool operator!=(const PPMFile& ppmFile); ///< Checks if two PPMs are not equal

	void read(std::string pathname);
	void write(std::string pathname) const;

	Color get(Coord x, Coord y) const;
	void get(Coord x, Coord y, Byte& r, Byte& g, Byte& b) const;
	void set(Coord x, Coord y, Color col);
	void set(Coord x, Coord y, Byte r, Byte g, Byte b);

	/**
	 * Reserve memory required to store pixel data
	 *
	 * @note based on current width and height
	 * @warning If the PPM has already allocated contents, these previous
	 * contents are discarded
	 */
	void allocate();

	void fill(Color c);
	void fill(Byte r, Byte g, Byte b);

	Size width() const { return w; }
	Size height() const { return h; }

	Size bytes() const { return w * h * 3; }

	~PPMFile();

private:

	Size w; ///< PPM file width (number of columns)
	Size h; ///< PPM file height (number of rows)
	Size maxColor; ///< Maximum color component value
	Byte* data; ///< PPM pixels data
};

// inline implementation of PPM exceptions

inline PPMFile::PPMException::PPMException(std::string methodName, std::string message) :
	methodName(methodName), message(message) {}
	
inline std::string PPMFile::PPMException::toString() const
{
	return methodName + "() -> " + message;
}


inline PPMFile::PPMOpeningException::PPMOpeningException(
	std::string methodName, std::string message) : PPMException(methodName, message) {}

inline PPMFile::PPMFormatException::PPMFormatException(
	std::string methodName, std::string message) : PPMException(methodName, message) {}

inline PPMFile::PPMEOFException::PPMEOFException(
	std::string methodName, std::string message) : PPMException(methodName, message) {}

inline PPMFile::PPMUnsupportedException::PPMUnsupportedException(
	std::string methodName, std::string message) : PPMException(methodName, message) {}

inline PPMFile::PPMAccessException::PPMAccessException(
	std::string methodName, std::string message) : PPMException(methodName, message) {}

inline PPMFile::PPMUndefinedException::PPMUndefinedException(
	std::string methodName, std::string message) : PPMException(methodName, message) {}

#endif // PPMFILE_H
