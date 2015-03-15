#ifndef GROUPREMAP_H
	#define GROUPREMAP_H

#include "FragmentMapConverter.h"

class GroupRemap : public FragmentMapConverter
{
private:

	// Only friend classes can create LinearRemap objects
	GroupRemap();
	GroupRemap(const GroupRemap&);
	GroupRemap& operator=(const GroupRemap&);

public:

	friend class FragmentMapConverterFactory;

	PPMFile transform(const PPMFile& fragmentMap);

};

#endif // GROUPREMAP_H
