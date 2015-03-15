#include "CycleData.h"
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>

using namespace std;

SignalDescriptionList* CycleData::sil = 0; // redundant (static is always set to zero by the compiler, but... just in case :-))

// Mandatory first
void CycleData::setSignalDescriptionList( SignalDescriptionList* sil ) 
{
	CycleData::sil = sil;
}


CycleData::CycleData() : cycle(-1), nSignals(CycleData::sil->size())
{		
    if ( sil == 0 ) {
        // fix this with more verbose code!
        abort();
    }

	sc = new SignalData*[nSignals];
	for ( int i = 0; i < nSignals; i++ ) {
		// create signals with appropiate number of slots
		sc[i] = new SignalData( sil->get(i)->getBw() );
	}
}


void CycleData::clear()
{
	for ( int i = 0; i < nSignals; ++i ) {
		delete sc[i];
		sc[i] = new SignalData( sil->get(i)->getBw() );
	}
}

CycleData::~CycleData()
{
	for ( int i = 0; i < nSignals; i++ )
		delete sc[i];
	delete[] sc;
}


void CycleData::dump() const
{
	cout << "CycleData for cycle: " << cycle << endl;
	for ( int i = 0; i < nSignals; i++ ) {
		cout << "Signal Name: " << sil->get( i )->getSignalName() << endl;
		cout << "Signal Id: " << sil->get( i )->getId() << endl;
		sc[i]->dump();	
		cout << "-------------------------------------------" << endl;
	}
}
