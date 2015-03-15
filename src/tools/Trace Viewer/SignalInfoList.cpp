#include "SignalInfoList.h"
#include <fstream.h>

const SignalInfo* SignalInfoList::get( int signalId ) const
{
	if ( signalId < 0 || signalId >= v.size() )
		return 0;
	return v[signalId];
}

const SignalInfo* SignalInfoList::get( const char* signalName ) const
{
	for ( int i = 0; i < v.size(); i++ ) {
		if ( strcmp( v[i]->getSignalName(), signalName ) == 0 )
			return v[i];
	}
	return 0;
}

int SignalInfoList::add( const char* signalName, int bw, int latency )
{
	char* name = new char[strlen(signalName)+1];
	strcpy( name, signalName );
	v.push_back( new SignalInfo( name, v.size(), bw, latency ) );
	return ( v.size() - 1 );
}


int SignalInfoList::size() const
{
	return v.size();
}

SignalInfoList::~SignalInfoList()
{
	for ( int i = 0; i < v.size(); i++ )
		delete v[i];
}

void SignalInfoList::dump()
{
	for ( int i = 0; i < v.size(); i++ ) {
		SignalInfo* si = v[i];
		cout << "----------------------------------------------" << endl;
		cout << "Signal ID: " << si->getId() << endl;
		cout << "Signal name: " << si->getSignalName() << endl;
		cout << "Bandwidth: " << si->getBw() << "  Latency: " << si->getLatency() << endl;
		cout << "----------------------------------------------" << endl;
	}
}

