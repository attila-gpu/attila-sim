#include "GroupRemap.h"
#include "PPMInspector.h"
#include <cmath>
#include <map>
#include <utility>

using std::vector;
using std::map;
using std::make_pair;

GroupRemap::GroupRemap()
{}

PPMFile GroupRemap::transform(const PPMFile& ppm)
{
	if ( !ppm.isDefined() )
		return PPMFile(); // create an undefined PPMFile

	PPMFile colorMap(ppm.width(), ppm.height());
	colorMap.allocate();

	PPMInspector inspector(ppm);

	//Size min = inspector.getMinRGBValue().toValue();
	//Size max = inspector.getMaxRGBValue().toValue();
	const vector<Color>& diffValues = inspector.getDifferentRGBValues();
	Size nDiffValues = diffValues.size();

	// Optimize colors usage
	reduceColors(nDiffValues);

	const vector<Color>& colors = getColors();
	Size nColors = colors.size();

	if ( nColors > nDiffValues )
		nColors = nDiffValues;

	const Size groupCount = (Size)std::ceil(double(nDiffValues) / double(nColors));
	
	map<Color,Color> colorAssign;
	for ( Size i = 0; i < diffValues.size(); i++ )
	{
		Color c = colors[i / groupCount];
		colorAssign.insert(make_pair(diffValues[i], c));
	}

	// Apply colorAssign map to the ppm file
	for ( Size i = 0; i < ppm.width(); i++ )
	{
		for ( Size j = 0; j < ppm.height(); j++ )
		{
			Color c = ppm.get(i,j);
			map<Color,Color>::const_iterator it = colorAssign.find(c);
			if ( it == colorAssign.end() )
				throw TransformationException("GroupRemap::transform: Unexpected color when applying color map");
			colorMap.set(i,j,it->second);
		}
	}	

	return colorMap;

}
