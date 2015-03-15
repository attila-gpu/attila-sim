#include "SignalDescriptionList.h"
#include <fstream>
#include <iostream>

using namespace std;

const SignalDescription* SignalDescriptionList::get( int signalId ) const
{
	if ( signalId < 0 || signalId >= (int)v.size() )
		return 0;
	return v[signalId];
}

const SignalDescription* SignalDescriptionList::get( const string& signalName ) const
{
	for ( unsigned int i = 0; i < v.size(); ++i ) {
        if ( v[i]->getSignalName() == signalName )
            return v[i];
	}
	return 0;
}

int SignalDescriptionList::add( const string& signalName, int bw, int latency )
{
	v.push_back( new SignalDescription( signalName, v.size(), bw, latency ) );
	return ( v.size() - 1 );
}


int SignalDescriptionList::size() const
{
	return v.size();
}

SignalDescriptionList::~SignalDescriptionList()
{
	for ( unsigned int i = 0; i < v.size(); i++ )
		delete v[i];
}

void SignalDescriptionList::dump()
{
	for ( unsigned int i = 0; i < v.size(); i++ ) {
		SignalDescription* si = v[i];
		cout << "----------------------------------------------" << endl;
		cout << "Signal ID: " << si->getId() << endl;
		cout << "Signal name: " << si->getSignalName() << endl;
		cout << "Bandwidth: " << si->getBw() << "  Latency: " << si->getLatency() << endl;
		cout << "----------------------------------------------" << endl;
	}
}

