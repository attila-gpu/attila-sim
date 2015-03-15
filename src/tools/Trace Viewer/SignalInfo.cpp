#include "SignalInfo.h"
#include <cstring>

SignalInfo::SignalInfo( const char* sigName, int id, int bw, int latency )
{
	signalName = new char[strlen(sigName)+1];
	strcpy( signalName, sigName );
	this->id = id;
	this->bw = bw;
	this->latency = latency;
}

const char* SignalInfo::getSignalName() const
{
	return signalName;
}

int SignalInfo::getId() const
{
	return id;
}

int SignalInfo::getBw() const
{
	return bw;
}

int SignalInfo::getLatency() const
{
	return latency;
}

SignalInfo::~SignalInfo()
{
	delete[] signalName;
}