#ifndef __GLOBAL_NEW__
	#define __GLOBAL_NEW__

#include <cstdlib>
#include <cstdio>
#include "GPUTypes.h"

static u32bit newCounts = 0;

/**
 * memory size available ( in bytes )
 */
#define MEM_SIZE 1024

/**
 * Array of memory available
 */
static char mem[MEM_SIZE];

/**
 * Global new operator
 */
void* operator new( size_t sz );

/**
 * Global delete operator
 */
void operator delete( void* object );


#endif