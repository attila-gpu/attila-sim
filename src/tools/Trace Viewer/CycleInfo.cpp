#include "CycleInfo.h"
#include <cstring>
#include <cstdlib>
#include <fstream.h>

SignalInfoList* CycleInfo::sil = 0;

bool CycleInfo::isInfoListSet = false;

// Mandatory first
void CycleInfo::setSignalInfoList( SignalInfoList* sil ) 
{
	CycleInfo::sil = sil;
	isInfoListSet = true;
}


CycleInfo::CycleInfo() : cycle(-1), nSignals(CycleInfo::sil->size())
{		
	sc = new SignalContent*[nSignals];
	for ( int i = 0; i < nSignals; i++ ) {
		// create signals with appropiate number of slots
		sc[i] = new SignalContent( sil->get(i)->getBw() );
	}
}


void CycleInfo::clear()
{
	for ( int i = 0; i < nSignals; i++ ) {
		delete sc[i];
		sc[i] = new SignalContent( sil->get(i)->getBw() );
	}
}

CycleInfo::~CycleInfo()
{
	for ( int i = 0; i < nSignals; i++ )
		delete sc[i];
	delete[] sc;
}


int CycleInfo::getCycle() const
{
	return cycle;
}

void CycleInfo::setCycle( int cycle )
{
	this->cycle = cycle;
}


SignalContent* CycleInfo::getSignalContent( int sigId )
{
	return sc[sigId];
}


bool CycleInfo::addSignalContents( int sigId, const char* sigContents )
{
	return ( sc[sigId]->addSignalContents( sigContents ) );
}

bool CycleInfo::addSignalContents( const char* sigName, const char* sigContents )
{
	int sigId = sil->get( sigName )->getId();
	return ( sc[sigId]->addSignalContents( sigContents ) );
}


void CycleInfo::dump() const
{
	cout << "CycleInfo for cycle: " << cycle << endl;
	for ( int i = 0; i < nSignals; i++ ) {
		cout << "Signal Name: " << sil->get( i )->getSignalName() << endl;
		cout << "Signal Id: " << sil->get( i )->getId() << endl;
		sc[i]->dump();	
		cout << "-------------------------------------------" << endl;
	}
}
