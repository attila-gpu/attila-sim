#ifndef FRAGMENTMAPCONVERTER_H
	#define FRAGMENTMAPCONVERTER_H

#include <vector>
#include <string>
#include "PPMFile.h"

class FragmentMapConverter;

class FragmentMapConverterFactory
{
public:

	static FragmentMapConverter* create(std::string functionMap);
};

class FragmentMapConverter
{
public:

	class TransformationException
	{
	public:
		
		TransformationException(std::string reasonStr);
		std::string reason() const;
	private:
		std::string reasonStr;
	};

	typedef PPMFile::Byte Byte;
	typedef PPMFile::Color Color;
	typedef PPMFile::Size Size;

	/**
	 * Adds a new color to the colletion of colors used to color the ppm output file
	 */
	void addColor(Byte r, Byte g, Byte b);
	void addColor(Color col);

	/**
	 * If there are excess of colors (more colors than different values)
	 * selects a number of colors equal to the number of values
	 */
	void reduceColors(Size numberOfValues);

	Size countColors() const;

	/**
	 * Create a new PPMFile with the color transformation applied
	 *
	 * Can throw TransformationException if transformation can not be performed
	 */
	virtual PPMFile transform(const PPMFile& fragmentMap) = 0;

	virtual ~FragmentMapConverter();

protected:

	const std::vector<Color>& getColors();

private:

	std::vector<Color> colors;

};


#endif // FRAGMENTMAPCONVERTER_H
