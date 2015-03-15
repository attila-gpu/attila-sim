#include "SignalBinder.h"
#include <cstring>
#include <cstdio>

int main( void ) {

	SignalBinder& sb = SignalBinder::getBinder();

	char buffer[20];
	
	for ( int i = 0; i < 102; i++ ) {
		sprintf( buffer, "Signal%d", i + 1 );
		sb.registerSignal( buffer, SignalBinder::BIND_MODE_READ, 1,1 );		
	}
	
	sb.dump();

	return 0;
}