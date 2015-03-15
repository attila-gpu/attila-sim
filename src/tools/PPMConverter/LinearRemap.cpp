#include "LinearRemap.h"

LinearRemap::LinearRemap()
{}


PPMFile LinearRemap::transform(const PPMFile& ppm)
{
	PPMFile colorMap(ppm.width(), ppm.height());
	colorMap.allocate();

	if ( !ppm.isDefined() )
		return PPMFile();

	// Compute min and max interpreting RGB colors as a 24-bit unsigned integer
	// B is considered the most significant byte and R the less significant
	Size min = ppm.get(0,0).toValue();
	Size max = min;

	for ( Size i = 0; i < ppm.width(); i++ )
	{
		for ( Size j = 0; j < ppm.height(); j++ )
		{
			Size value = ppm.get(i,j).toValue();
			if ( value < min )
				min = value;
			if ( value > max )
				max = value;
		}
	}

	// Optimize colors usage
	reduceColors(max - min + 1);

	const std::vector<Color>& colors = getColors();
	Size nColors = colors.size();
	Size nValues = max - min + 1;

	double valuesPerColor = double(nValues) / double(nColors);

	if ( nValues / nColors == 0 ) // more colors than different values
		valuesPerColor = 1;

	for ( Size i = 0; i < ppm.width(); i++ )
	{
		for ( Size j = 0; j < ppm.height(); j++ )
		{
			Size value = ppm.get(i,j).toValue();
			value -= min;
			value = (Size)(value / valuesPerColor);
			colorMap.set(i, j, colors[value]);
		}
	}

	return colorMap;
}
