#include "globalNew.h"

void* operator new( size_t sz ) 
{
	printf( "Operator new: %d bytes\n", sz );
	//allocate memory chunk
	void* object = malloc( sz );
	
	if ( !object )
		puts( "out of memory" );
	
	return object;
}


void operator delete( void* object )
{
	puts( "operator delete" );
	free( object );
}