#include "Log.h"

/* create log file */
#ifdef LOG_MODE_ENABLED

#include "support.h"
#include <sys/timeb.h>
#include <time.h>

using namespace std;

Log* Log::theLog = 0;

Log& Log::log( const char* filename )
{
	if ( theLog )
		return *theLog;
	
	if ( filename == NULL )
		panic("Log", "log()", "Log file name required");

	theLog = new Log;
	theLog->open(filename);
	
	if ( !theLog->is_open() )
		panic("Log", "log()", "Log file cannot be opened");
	
	return *theLog;
}


string Log::time()
{
	/* simple time clock */

	struct _timeb timebuffer;
	char *timeline;

	_ftime(&timebuffer);
	timeline = ctime(&(timebuffer.time));

	char theTime[256];

	sprintf(theTime, "%.8s.%hu", &timeline[11], timebuffer.millitm);	

	return string(theTime);
}

#endif
