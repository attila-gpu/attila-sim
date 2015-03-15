#include "SignalBinder.h"

int main( void ) {

	// Aliasing ( two alias but same binder )
	SignalBinder& binder = SignalBinder::getBinder();
	SignalBinder& binder2 = SignalBinder::getBinder();

	// Demonstration: the alias can't be created or copied
	// SignalBinder aBinder; // Private constructor ( access denied )	
	// SignalBinder copyBinder = binder2; // Copy constructor private ( access denied )
	
	Signal* s1 = binder.registerSignal( "Vertex Input", SignalBinder::BIND_MODE_WRITE, 3 );
	// This call produce a warning if debug MODE is enable ( same name, diferent latency )
	Signal* s2 = binder.registerSignal( "Vertex Input", SignalBinder::BIND_MODE_READ, 3, 1 );

	cout << s1 -> getName() << endl;
	if ( s1 == s2 )
		cout << "Same pointer..." << endl;

	cout << "Elements: " << binder2.getElements() << endl;

	binder.getSignal( "Vertex Input" )->dump();
	
	return 0;
}
