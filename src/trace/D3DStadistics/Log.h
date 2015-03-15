#ifndef LOG_H
	#define LOG_H


/**
 * Add ENABLE_LOG_MODE definition in compiler options
 * for generating a log file with GLInterceptor info
 */ 


/* from 0 to N */
#define LOGLEVEL 0



#ifdef LOG_MODE_ENABLED

#include <fstream>
#include <string>
#include "support.h"

/*
 * Macro for writing Log code 
 */
#define LOG(level,code) if ( level <= LOGLEVEL ) { Log::log() << Log::time() << " "; code; Log::log().flush(); }

class Log : public std::ofstream
{
private:
	
	static Log* theLog;

public:

	static Log& log( const char* fileName = 0 );
	
	static std::string time();
};

	

#else /* disable log mode */
	#define LOG(level,code)
#endif


#endif // LOG_H

