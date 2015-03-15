#include "DataManager.h"
#include <windows.h>
#include <qregexp.h>
#include <qmessagebox.h>
#include <set>
#include <sstream>

using std::string;
using std::stringstream;

#define MSG(title,msg) MessageBox( NULL, msg, title, MB_OK );
#define SHOWDATA_SIZE 40

#define SHOWDATA { char __buf[SHOWDATA_SIZE];\
	str.readAndPutback( __buf, sizeof(__buf)-1 );\
	__buf[SHOWDATA_SIZE-1] = 0;\
	MSG( "buf", __buf );\
}

DataManager::SigIdentification::SigIdentification( const string& name, int nSlot ) :
originalSignalName(name)
{
	//char buffer[10];
	// originalSignalName = new char[strlen(name)+1];
	// strcpy( originalSignalName, name );
    stringstream ss;
    ss << name << "." << nSlot;
    simpleSignalName = ss.str();
	//sprintf( buffer, ".%d", nSlot );
	//simpleSignalName = new char[strlen(name)+strlen(buffer)+2];
    //simpleSignalName 
	//strcpy( simpleSignalName, name );	
	//strcat( simpleSignalName, buffer );		
}

using namespace std;

const string& DataManager::getSignalSlotName( int simpleSignalId ) const
{
	return v[simpleSignalId]->simpleSignalName;
}

const string& DataManager::getSignalName(int simpleSignalId) const
{
	return v[simpleSignalId]->originalSignalName;
}


DataManager::DataManager(int cacheSize) : totalCyclesInFile(-1), cacheSize(cacheSize)
{		

	
}


int DataManager::loadData( const char* traceFilePath, int /*maxCycles*/ )
{
	// max cycles is not used never more (deprecated)
	if ( !str.open( traceFilePath ) )
		return -1;	

	if ( !str.checkTrace( traceFilePath ) )
		return -2;
	
	str.skipLines( 4 );
	str.readSignalDescription( sil );

	// Compute simple signal objects
	// ex: signal with bw 2 is converted in two simple signals	
	int i, j;
	for ( i = 0; i < sil.size(); i++ ) {
		const SignalDescription* si = sil.get( i );
		const string& sName = si->getSignalName();
		for ( j = 0; j < si->getBw(); j++ )
			v.push_back( new SigIdentification( sName, j ) );
	}

	// initialize number of rows for CyclePages ( set to simpleSignals count )
	CyclePage::setRows( v.size() );
	
	// create cache
	cache = new CyclePageCache( cacheSize );

	CycleData::setSignalDescriptionList( &sil ); // Configure CycleData with current SignalDescriptionList	
	offsetMap.insert(make_pair(0, str.getPosition()));
	return 0;
}

int DataManager::getCycles(int& firstCycle)
{
	static int cycle1st = 0;
	if ( totalCyclesInFile == -1 ) // computed only once
		totalCyclesInFile = str.countCyclesInfo( str.getFileName(), cycle1st );
	
	firstCycle = cycle1st;
	return totalCyclesInFile;
}

int DataManager::getCycles()
{
	int dummy;
	return getCycles(dummy);
}

const SignalSlotData* DataManager::getData( int x, int y )
{
	int idPage = y / CyclePage::getPageCapacity(); // compute idPage
	int relativeY = y % CyclePage::getPageCapacity(); // compute page offset
	CyclePage* cp = cache->getPage( idPage ); // find out if the page is in cache
	if ( cp != NULL ) // hit ( page is in cache ) 		
		return cp->getData( x, relativeY );			
	// miss ( page is not in cache )	
	cp = cache->getFreePage( idPage ); // obtains a free page and asociates its identifier	
	// find file position

	map<long,long>::iterator it = offsetMap.find(idPage);
	if ( it != offsetMap.end() )
		str.setPosition(it->second);
	//if ( vOffset.size() > idPage ) // offset was saved previously ( directly positioning )
	//	str.setPosition( vOffset[idPage] );
	else 
	{ 
		it = offsetMap.begin(); // at least first page exists
		map<long,long>::iterator itPrev;
		//long minOffset = 0;
		
		while ( it != offsetMap.end() )
		{
			if ( it->first < idPage )
			{
				itPrev = it;
				it++;
			}
			else
				break;
		}
		
		str.setPosition(itPrev->second);
		
		long pos = itPrev->first;

		while ( pos < idPage )
		{
			pos++;
			offsetMap[pos] = skipPage();
		}		

		str.setPosition(offsetMap[idPage]); // pos == idPage

		/*
		// find closed lower offset
		// find offset page
		int ini = vOffset.size() - 1;
		// set position at last known offset
		str.setPosition( vOffset[ini] );					
		while ( ini++ < idPage )
			vOffset.push_back( skipPage() ); // save offsets found through the file		
		str.setPosition( vOffset[vOffset.size()-1] );
		*/
	}	
	loadPage( cp ); // load data for this page
	offsetMap[idPage+1] = str.getPosition(); // save next offset to increase sequential accesses		
	//vOffset.push_back( str.getPosition() ); // save next offset to increase sequential accesses		
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
		str.skipCycleData();
	return str.getPosition();	
}

void DataManager::loadPage( CyclePage* page )
{
	// load data from file to CyclePage	
	//SHOWDATA;
	//cout << "*********************** LOADING PAGE *************************** " << endl;
	CycleData ci;
	int PAGE_CYCLES = page->getPageCapacity();
	int cycles = 0;
	int i, j;
	while (  cycles < PAGE_CYCLES && str.readCycleData( ci ) ) {		
		int vPosition = 0;
		int cycle = ci.getCycle(); // cycle is the horizontal position
		for ( i = 0; i < sil.size(); i++ ) {			
			const SignalDescription* si = sil.get( i ); // gets general signal information
			SignalData* sc = ci.getSignalData( i ); // gets signal's content for a particular cycle
			int nsc = sc->countSignalDatas();
			for ( j = 0; j < si->getBw(); j++ ) { // forall slots
				if ( j < nsc )  { // only get data from slots with data ( first nsc slots )
					//int nCookies;
					const vector<int> cookies = sc->getCookieList( j );
					// Don't use cycle (it doesn't work with multi-part files), use instead 'cycles' counter
					//page->setData( vPosition, cycle % CyclePage::getPageCapacity(), 
					//	new SignalSlotDescription( sc->getColor(j), cookies, nCookies, cycle, sc->getInfo(j) ) );					

					page->setData( vPosition, cycles % PAGE_CYCLES, 
						//new SignalSlotDescription( sc->getColor(j), cookies, cookies.size(), cycle, sc->getInfo(j) ) );
                    new SignalSlotData(sc->getColor(j), cookies, cycle, sc->getInfo(j) ) );

				
				}
				vPosition++;
			}
		}
		cycles++;
	}
}

vector<std::string> DataManager::getMatchingSignals(const char* pattern)
{	
	using namespace std;
	vector<string> matches;
	QRegExp regexp(pattern);
	// regexp.setWildcard(true);
    regexp.setPatternSyntax(QRegExp::Wildcard);
	if ( !regexp.isValid() )
	{
		QMessageBox::critical(0, "Panic", 
		                         "LoadLayout. DataManager regular expression invalid",
								 QMessageBox::Ok, QMessageBox::NoButton);
		return matches;
	}

	set<string> signalNames;
	for ( unsigned int i = 0; i < v.size(); i++ )
		signalNames.insert(v[i]->originalSignalName);

	for ( set<string>::const_iterator it = signalNames.begin();
		  it != signalNames.end(); it++ )
	{
		if ( regexp.exactMatch(it->c_str()) )
			matches.push_back(*it);
	}

	return matches;
}

vector<int> DataManager::getSignalSlots(const char* originalSignalName)
{
	vector<int> matchings;
	bool found = false;
	for ( unsigned int i = 0; i < v.size(); ++i )
	{
		if ( v[i]->originalSignalName == originalSignalName )
		{
			matchings.push_back(i);
			found = true;
		}
		else if ( found )
			break;
	}
	return matchings;
}

int DataManager::countSignalSlots(const string& signalName) const
{
	const SignalDescription* si = sil.get(signalName);
	if ( si )
		return si->getBw();
	return 0;
}


int DataManager::getSignalSlotPosition(const char* signalName) const
{
	for ( int i = 0; i < v.size(); i++ )
	{
		if ( v[i]->originalSignalName == signalName )
			return i;
	}
	return -1;
}
