#include "Box.h"
#include "StatisticsManager.h"

#include <ostream.h>

/*
 * Ejemplo del registro de estadisticas en una box 
 * Tb se muestra el sistema automático de registros de las boxes... ( dumpNameBoxes() )
 *
 * Nota, para que no salgan mensajes infomativos -> comentar el define #define GPU_MESSAGES
 */
class Box1 : public Box {
private:

	u32bit& nMisses;
	u32bit& anotherStat;


public:	
	// las estadisticas se registran en la initializer list del contructor
	Box1( const char* name, Box* parent ) : Box(name, parent),
		nMisses( getStatU32BIT( "miss count" ) ), // estadistica 1
		anotherStat( getStatU32BIT( "Another count" ) ) // estadistica 2
	{		
	}

	// stupid clock
	void clock( u32bit cycle ) 
	{ 		
		nMisses++; // se usan como una variable cualquiera...
		if ( cycle % 2 == 0 )
			anotherStat ++;
	}

	~Box1() 
	{
		cout << "Me destruyo ( soy '" << getName() << "' )" << endl;

	}
};


int main()
{

	Box1* b1 = new Box1("Box b1",0);

	// boxes registered until now ( 1 )
	Box::dumpNameBoxes();
	
	for ( u32bit cycle = 0; cycle < 10; cycle++ ) {
		b1->clock(cycle);
	}
	
	delete b1;
	
	// get statistics...
	StatisticsManager::getManager().dump();


	

	return 0;
}