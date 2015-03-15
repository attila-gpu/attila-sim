
/**
 *
 * This file defines types for the PIX trace. 
 *
 */

#ifndef _PIXTYPESH_
#define _PIXTYPESH_

#include "GPUTypes.h"

#ifndef PIX_X64
    typedef u32bit PIXPointer;
#else
    typedef u64bit PIXPointer
#endif    

#endif 

