#include "PPMInspector.h"
#include <set>

struct ColorComparatorLess
{
	bool operator()(const PPMFile::Color& c1, const PPMFile::Color& c2)
	{
		return c1.toValue() < c2.toValue();
	}
};

PPMInspector::PPMInspector(const PPMFile& ppm_) : ppm_(ppm_), computed(false)
{}

PPMInspector::Color PPMInspector::getMaxRGBValue() const
{
	if ( !computed )
		countDiffRGBValues();
	return Color(max);
}

PPMInspector::Color PPMInspector::getMinRGBValue() const
{
	if ( !computed )
		countDiffRGBValues();
	return Color(min);
}

PPMInspector::Size PPMInspector::countDiffRGBValues() const
{
	std::set<Color,ColorComparatorLess> diffValues;
	for ( Size i = 0; i < ppm_.width(); i++ )
	{
		for ( Size j = 0; j < ppm_.height(); j++ )
			diffValues.insert(ppm_.get(i,j));
	}
	min = diffValues.begin()->toValue();
	max = diffValues.rbegin()->toValue();
	diffs = diffValues.size();
	diffsV.assign(diffValues.begin(), diffValues.end());
	computed = true;
	return diffs;
}

const std::vector<PPMInspector::Color> PPMInspector::getDifferentRGBValues() const
{
	if ( !computed )
		countDiffRGBValues();
	return diffsV;
}

void PPMInspector::recompute()
{
	computed = false;
}

