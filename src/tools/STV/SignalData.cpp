#include "SignalData.h"

#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>

using namespace std;

void SignalData::clear()
{
	for ( int i = 0; i < index; ++i ) 
	{
        ss[i].cookieList.clear();
        ss[i].info = "";
	}
	index = 0;
}


bool SignalData::addSignalDatas( const std::string& sigContentsStr )
{
	if ( index == nSlots )
		return false;

	int& color = ss[index].color;

	int temp[100];
	char buffer[256];
	int itemp = 0;
	
	// parse cookie list
	int i, iPrev = 0;

    // To mix all code with new interface (pending to be changed and optimized)
    const char* sigContents = sigContentsStr.c_str();

	for ( i = 0; sigContents[i] != ';' ; ++i ) {		
		if ( sigContents[i] == ':' ) {
			strncpy( buffer, &sigContents[iPrev], i - iPrev );
			buffer[i-iPrev] = 0;			
			iPrev = i + 1;
			temp[itemp++] = atoi( buffer );
		}
	}
	// process last cookie
	strncpy( buffer, &sigContents[iPrev], i - iPrev );
	buffer[i-iPrev] = 0;
	temp[itemp++] = atoi( buffer );
	i++;
	
	// i is pointing to color
	iPrev = i;
	for ( ; sigContents[i] != ';' && sigContents[i] != 0; i++ ) ;
	strncpy( buffer, &sigContents[iPrev], i - iPrev );
	buffer[i-iPrev] = 0;
	color = atoi( buffer );
	// extract info if available
	if ( sigContents[i] == ';' )
		i++;
	iPrev = i;
	for ( ; sigContents[i] != (char)0; i++ ) ;
	
	if ( iPrev != i )// Contents available
        ss[index].info = &sigContents[iPrev];
	else
		ss[index].info = "";
    
	// Copy cookie list
    ss[index].cookieList.reserve(itemp);
    ss[index].cookieList.resize(itemp);
	for ( i = 0; i < itemp; ++i )
		ss[index].cookieList[i] = temp[i];

	++index; // slot occupied

	return true;
}


void SignalData::dump() const
{	
	cout << "Signal bw (slots): " << nSlots << endl;
	int i, j;
	for ( i = 0; i < index; i++ ) { // for all slots
		cout << "slot(" << i << ") info: " << endl;
		cout << " cookies: ";
        for ( j = 0; j < ss[i].cookieList.size(); ++j ) {
			cout << ss[i].cookieList[j];
            if ( j != ss[i].cookieList.size() - 1 )
				cout << ":";
		}
		if ( !ss[i].info.empty() )
			cout << "  info: \"" << ss[i].info << "\"";
		cout << endl;
	}
}
