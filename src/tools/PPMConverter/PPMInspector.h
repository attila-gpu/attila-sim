#ifndef PPMINSPECTOR_H
	#define PPMINSPECTOR_H

#include "PPMFile.h"
#include <vector>

class PPMInspector
{
public:

	PPMInspector(const PPMFile& ppm);

	typedef PPMFile::Size Size;
	typedef PPMFile::Color Color;

	const std::vector<Color> getDifferentRGBValues() const;
	Size countDiffRGBValues() const;
	Color getMaxRGBValue() const;
	Color getMinRGBValue() const;

	const PPMFile& ppm() const { return ppm_; }

	void recompute();

private:

	// Vars for precomputed values
	mutable bool computed;
	mutable Size diffs;
	mutable Size min;
	mutable Size max;
	mutable std::vector<Color> diffsV;

	const PPMFile& ppm_;
	
	PPMInspector(const PPMInspector&);
	PPMInspector& operator=(const PPMInspector&);

};

#endif // PPMINSPECTOR_H

