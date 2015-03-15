#ifndef D3D_TRACE
#define D3D_TRACE

#include "GPUTypes.h"

class D3DTrace
{
public:
	virtual bool next() = 0;
	virtual bool getFrameEnded() = 0;
	virtual u32bit getCurrentEventID() = 0;
	virtual bool isPreloadCall() = 0;
};

D3DTrace *create_d3d_trace(char *file);

#endif
