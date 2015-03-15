#include "Box.h"
#include "StatisticsManager.h"


class DummyBox : public Box 
{

private:

	u32bit s1;
	u32bit s2;
	u32bit s3;
	u32bit s4;
	u32bit s5;
	StatisticsManager& sm;

public:
	DummyBox( const char* name, Box* parent ) : Box(name,parent),

		// Usar punteros !!!!!!!!! en vez de referencias
		sm(StatisticsManager::getManager())
	{	
		// Register some statistics
		/*
		s1 = sm.getStatU32BIT( "stat1", this );
		s2 = sm.getStatU32BIT( "stat2", this );
		s3 = sm.getStatU32BIT( "stat3", this );
		s4 = sm.getStatU32BIT( "stat4", this );
		s5 = sm.getStatU32BIT( "stat5", this );
		*/		

		sm.getStatU32BIT( "stat1", s1, this );
		sm.getStatU32BIT( "stat2", s2, this );
		sm.getStatU32BIT( "stat3", s3, this );
		sm.getStatU32BIT( "stat4", s4, this );
		sm.getStatU32BIT( "stat5", s5, this );

		cout << "COnstructor OK" << endl;
		// initialization
		s1 = 5;
		s2 = 10;
		s3 = 15;
		s4 = 20;
		s5 = 25;
		
	}

	void clock( u32bit cycle ) // dummy clock, only increase in 1 the references...
	{
		s1++;
		s2++;
		s3++;
		s4++;
		s5++;
	}
};

int main( void ) 
{
	StatisticsManager& sm = StatisticsManager::getManager();
	

	DummyBox db1("Dummy1", 0 );
	DummyBox db2("Dummy2", 0 );

	u32bit ref1;
	sm.getStatU32BIT( "stat1", ref1 );
	

	cout << "REF1: " << ref1 << endl;

	ref1 = 777;
	/*


	u32bit& ref1 = sm.getStatU32BIT( "otro stat" );
	u32bit& ref2 = sm.getStatU32BIT( "otra stat mas" );	
	u32bit& ref3 = sm.getStatU32BIT( "otra stat mas, la ultima" );

  */
	sm.dump();
	return 0;
}