#include "FragmentMapConverter.h"
#include "LinearRemap.h"
#include "GroupRemap.h"
#include <iostream>


FragmentMapConverter::TransformationException::TransformationException(std::string reasonStr) :
reasonStr(reasonStr)
{}

std::string FragmentMapConverter::TransformationException::reason() const
{
	return reasonStr;
}

FragmentMapConverter* FragmentMapConverterFactory::create(std::string functionMap)
{
	FragmentMapConverter* fmc;
	if ( functionMap == "linear" )
		fmc = new LinearRemap;
	else if ( functionMap == "group" )
		fmc = new GroupRemap;
	else
		fmc = 0; // Unsupported function map
	return fmc;
}

void FragmentMapConverter::reduceColors(Size values)
{
	if ( values >= colors.size() )
		return ;

	// more colors than values
	std::vector<Color> newColors;
	double step = double(colors.size()) / double(values);
	for ( Size i = 0; i < values; i++ )
		newColors.push_back( colors[(Size)(step * i)] );

	colors.swap(newColors);
}

void FragmentMapConverter::addColor(Byte r, Byte g, Byte b)
{
	colors.push_back(Color(r,g,b));
}

void FragmentMapConverter::addColor(Color col)
{
	colors.push_back(col);
}

const std::vector<FragmentMapConverter::Color>& FragmentMapConverter::getColors()
{
	return colors;
}

FragmentMapConverter::Size FragmentMapConverter::countColors() const
{
	return colors.size();
}

FragmentMapConverter::~FragmentMapConverter()
{
	// nothing to do (just required a polymorphic dtor in subclasses)
}
