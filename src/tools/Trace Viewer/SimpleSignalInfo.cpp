#include "SimpleSignalInfo.h"
#include <cstring>

#include <ostream.h>

SimpleSignalInfo::SimpleSignalInfo( int color, const int* cookies, int nCookies, const char* info )
{
	this->color = color;
	this->nCookies = nCookies;
	this->cookies = new int[nCookies];
	for ( int i = 0; i < nCookies; i++ )
		this->cookies[i] = cookies[i];
	this->info = NULL; // 3 horas de debug por no poner esta línea...
	if ( info ) {		
		this->info = new char[strlen(info)+1];
		strcpy( this->info, info );
	}
	
	
}

SimpleSignalInfo::~SimpleSignalInfo()
{
	delete[] cookies;
	if ( info )
		delete[] info;
}

int SimpleSignalInfo::getColor() const
{
	return color;
}


const char* SimpleSignalInfo::getInfo() const
{
	return info;
}

const int* SimpleSignalInfo::getCookies( int& nCookies ) const
{
	nCookies = this->nCookies;
	return cookies;
}
