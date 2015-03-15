#include "DataManager.h"
#include <windows.h>

#define MSG(title,msg) MessageBox( NULL, msg, title, MB_OK );
#define SHOWDATA_SIZE 40

#define SHOWDATA { char __buf[SHOWDATA_SIZE];\
	str.readAndPutback( __buf, sizeof(__buf)-1 );\
	__buf[SHOWDATA_SIZE-1] = 0;\
	MSG( "buf", __buf );\
}

DataManager::SigIdentification::SigIdentification( const char* name, int nSlot )
{
	char buffer[10];
	originalSignalName = new char[strlen(name)+1];
	strcpy( originalSignalName, name );
	sprintf( buffer, ".%d", nSlot );
	simpleSignalName = new char[strlen(name)+strlen(buffer)+2];
	strcpy( simpleSignalName, name );	
	strcat( simpleSignalName, buffer );		
}

DataManager::SigIdentification::~SigIdentification()
{
	delete[] originalSignalName;
	delete[] simpleSignalName;
}

using namespace std;

const char* DataManager::getSimpleSignalName( int simpleSignalId ) const
{
	return v[simpleSignalId]->simpleSignalName;
}


DataManager::DataManager() : totalCyclesInFile(-1)
{		
	
}


int DataManager::loadData( const char* traceFilePath, int maxCycles )
{
	// maxCycles ignored in this version ( 0.4 beta )
	if ( !str.open( traceFilePath ) )
		return -1;
	
	str.skipLines( 4 );
	str.readSignalInfo( sil );

	// Compute simple signal objects
	// ex: signal with bw 2 is converted in two simple signals	
	int i, j;
	for ( i = 0; i < sil.size(); i++ ) {
		const SignalInfo* si = sil.get( i );
		const char* sName = si->getSignalName();
		for ( j = 0; j < si->getBw(); j++ )
			v.push_back( new SigIdentification( sName, j ) );
	}

	// initialize number of rows for CyclePages ( set to simpleSignals count )
	CyclePage::setRows( v.size() );
	// create cache
	cache = new CyclePageCache( 5 );


	// load first page ( not necessary )
	//CyclePage* cp = new CyclePage( 0 );
	//loadData( cp );	
	CycleInfo::setSignalInfoList( &sil ); // Configure CycleInfo with current SignalInfoList	
	vOffset.push_back( str.getPosition() );	

	return 0;
}

int DataManager::getCycles()
{
	if ( totalCyclesInFile == -1 ) // computed only once
		totalCyclesInFile = str.countCyclesInfo( str.getFileName() );
	return totalCyclesInFile;
}


const SimpleSignalInfo* DataManager::getData( int x, int y )
{
	int idPage = y / CyclePage::getPageCapacity(); // compute idPage
	int relativeY = y % CyclePage::getPageCapacity(); // compute page offset
	CyclePage* cp = cache->getPage( idPage ); // find out if the page is in cache
	if ( cp != NULL ) // hit ( page is in cache ) 		
		return cp->getData( x, relativeY );			
	// miss ( page is not in cache )	
	cp = cache->getFreePage( idPage ); // obtains a free page and asociates its identifier	
	// find file position
	if ( vOffset.size() > idPage ) // offset was saved previously ( directly positioning )
		str.setPosition( vOffset[idPage] );
	else { // find offset page
		/*
		MSG( "find offset page", "NOT IMPLEMENTED ( accept implies exiting )" );		
		*/
		int ini = vOffset.size() - 1;		
		// set position at last known offset
		str.setPosition( vOffset[ini] );					
		while ( ini++ < idPage )
			vOffset.push_back( skipPage() ); // save offsets found through the file		
		str.setPosition( vOffset[vOffset.size()-1] );
	}	
	loadPage( cp ); // load data for this page
	vOffset.push_back( str.getPosition() ); // save next offset to increase sequential accesses		
	return cp->getData( x, relativeY ); // returns data
}


void DataManager::dump() const
{
	/*
	int i, j;

	for ( j = 0; j < cycles; j++ ) {
		cout << "Cycle " << j << " infomation:" << endl;
		for ( i = 0; i < v.size(); i++ ) {
			if ( ssArray[i][j] != NULL ) {
				cout << "Signal: " << v[i]->simpleSignalName << endl;
				cout << "  Cookies: ";
				int nCookies;
				const int* cookies = ssArray[i][j]->getCookies( nCookies );
				for ( int k = 0; k < nCookies; k++ ) {
					cout << cookies[k];
					if ( k != nCookies - 1 ) cout << ":";
				}
				cout << "  Color: " << ssArray[i][j]->getColor();
				const char* info = ssArray[i][j]->getInfo();
				if ( info ) 					
					cout << "  Info: " << info << endl;
				else
					cout << endl;
				//cout << endl;				
			}
		}
		cout << "-----------------------------------------" << endl;
	}
	*/
}


int DataManager::skipPage()
{	
	int pageCycles = CyclePage::getPageCapacity();
	for ( int i = 0; i < pageCycles; i++ )
		str.skipCycleInfo();
	return str.getPosition();	
}

void DataManager::loadPage( CyclePage* page )
{
	// load data from file to CyclePage	
	//SHOWDATA;
	cout << "*********************** LOADING PAGE *************************** " << endl;
	CycleInfo ci;
	int PAGE_CYCLES = page->getPageCapacity();
	int cycles = 0;
	int i, j;
	while (  cycles < PAGE_CYCLES && str.readCycleInfo( ci ) ) {		
		cycles++;
		int vPosition = 0;
		int cycle = ci.getCycle(); // cycle is the horizontal position
		for ( i = 0; i < sil.size(); i++ ) {			
			const SignalInfo* si = sil.get( i ); // gets general signal information
			SignalContent* sc = ci.getSignalContent( i ); // gets signal's content for a particular cycle
			int nsc = sc->countSignalContents();
			for ( j = 0; j < si->getBw(); j++ ) { // forall slots
				if ( j < nsc )  { // only get data from slots with data ( first nsc slots )
					int nCookies;
					const int* cookies = sc->getCookieList( j, nCookies );
					page->setData( vPosition, cycle % CyclePage::getPageCapacity(), 
						new SimpleSignalInfo( sc->getColor(), cookies, nCookies, sc->getInfo(j) ) );					
				}
				vPosition++;
			}
		}			
	}
}
