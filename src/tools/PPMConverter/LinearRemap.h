#ifndef LINEARREMAP_H
	#define LINEARREMAP_H

#include "FragmentMapConverter.h"


class LinearRemap : public FragmentMapConverter
{
private:

	// Only friend classes can create LinearRemap objects
	LinearRemap();
	LinearRemap(const LinearRemap&);
	LinearRemap& operator=(const LinearRemap&);

public:

	friend class FragmentMapConverterFactory;

	PPMFile transform(const PPMFile& fragmentMap);
};

#endif // LINEARREMAP_H
