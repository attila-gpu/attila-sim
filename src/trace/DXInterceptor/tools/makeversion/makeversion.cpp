////////////////////////////////////////////////////////////////////////////////
// makeversion.cpp : Defines the entry point for the console application.
//
// Copyright (c) 2005 George Gugulea
// All Rights Reserved
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 1, or (at your option)
// any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// Creates a file version.ver that will contain different version numbers.
// it will be a standard include c++ file containing #define-s
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <windows.h>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <cstdlib>

////////////////////////////////////////////////////////////////////////////////

#ifndef NO_ERROR
 #define NO_ERROR 0
#endif

#ifndef MAX_PATH
 #define MAX_PATH 260
#endif

#define MAX_FILE_SIZE 1000

#define STRING_MAJOR    "VERSION_MAJOR"
#define STRING_MINOR    "VERSION_MINOR"
#define STRING_BUILD    "VERSION_BUILD"
#define STRING_REVISION "VERSION_REVISION"
#define STRING_DATE     "VERSION_BUILD_DATE"
#define STRING_TIME     "VERSION_BUILD_TIME"

////////////////////////////////////////////////////////////////////////////////

bool	bIncrementVersion; // build number
bool  bIncrementRevision; // revision number (aka Quick Fix Engineering number)
bool	bSimulate;
bool	bVerbose;
char*	sVersionFile;

////////////////////////////////////////////////////////////////////////////////

char* str_Delete(char* buf, int index, int count)
{
	char* c;

	c = buf+index;
	while(*c)
	{
		*c = c[count];
		if (!*c) break;
		c++;
	}
	return buf;
}

////////////////////////////////////////////////////////////////////////////////

bool str_ReplaceOnce(char* buf, const char* find, const char* repl)
{
	int flen=0;
	int	rlen=0;
	int insert=0;
	int t=0;
	char* pos=NULL;

	// Don't replace anything if the find-string is empty
	if (!find || !find[0])
		return true;

	flen = (int) strlen(find);
	rlen = repl ? (int) strlen(repl) : 0;
	insert = rlen-flen;
	pos = strstr( buf, find );
	if(!pos)
		return false;

	if( insert )
	{
		if( insert > 0 )
		{
			for (int len=(int) strlen(pos); len>=flen; len-- )
				pos[len+insert] = pos[len];
		}
		else
			str_Delete(pos,0,-insert);
	}
	for( t=0; t<rlen; t++ )
		*pos++ = repl[t];

	return true;
}

////////////////////////////////////////////////////////////////////////////////

bool GetDate(char* str_date, char* str_time, bool q)
{
	SYSTEMTIME st;

	GetLocalTime(&st);

	if(str_date)
		sprintf(str_date, q?"\"%02u/%02u/%02u\"":"%02u/%02u/%02u", st.wDay, st.wMonth, st.wYear);

	if(str_time)
		sprintf(str_time, q?"\"%02u:%02u:%02u\"":"%02u/%02u/%02u", st.wHour, st.wMinute, st.wSecond);

	return true;
}

////////////////////////////////////////////////////////////////////////////////

void StandardVersionFile(char *str)
{
	char _date[50], _time[50];
	GetDate(_date, _time, true);
	sprintf(str,
				"#define " STRING_MAJOR    "\t%d"	"\n"
				"#define " STRING_MINOR    "\t%d"	"\n"
				"#define " STRING_BUILD    "\t%d"	"\n"
				"#define " STRING_REVISION "\t%d"	"\n"
				"#define " STRING_DATE     "\t%s"	"\n"
				"#define " STRING_TIME     "\t%s"	"\n",
				1,0,0,0, _date, _time
			);
}

////////////////////////////////////////////////////////////////////////////////

bool CheckCommandLine(int argc, char* argv[])
{
	int i;
	for( i=1; i<argc; i++)
	{
		if( argv[i][0]=='/' || argv[i][0]=='-' )
		{
			if( !strcmp(argv[i]+1, "inc") )
				bIncrementVersion = true;
      if( !strcmp(argv[i]+1, "rev") )
        bIncrementRevision = true;
			if( !strcmp(argv[i]+1, "sim"))
				bSimulate = true;
			if( !strcmp(argv[i]+1, "v") || !strcmp(argv[i]+1, "vervose"))
				bVerbose = true;
			if( !strcmp(argv[i]+1, "h") || !strcmp(argv[i]+1, "?") || !strcmp(argv[i]+1, "help"))
				return false;
		}
		else
		{
			sVersionFile = argv[i];
		}
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////

void Usage(int argc, char* argv[])
{
	printf(	"Usage: %s [-h] [-inc] [-sim] [-v] [version.ver]\n\n", argv[0] );
	printf( "  -inc = Autonincrement build number\n" );
  printf( "  -rev = Autonincrement revision number (QFE)\n" );
	printf( "  -sim = Writes to stdout, version.ver stays untouched.\n" );
	printf( "  -v   = Verbose; prints the modifications to stdout too.\n" );
	printf( "  -h/? = This screen.\n" );
}

////////////////////////////////////////////////////////////////////////////////

bool CheckSymbolsAndWriteOut(const char *define_type, const char *define_value, char *sOut)
{
	if( !strcmp(define_type, STRING_DATE) )
	{
		char sNewDate[50];
		if( !GetDate(sNewDate, NULL, true) )
			return false;

		str_ReplaceOnce(strstr(sOut, STRING_DATE), define_value, sNewDate);
	}
	if( !strcmp(define_type, STRING_TIME) )
	{
		char sNewTime[50];
		if( !GetDate(NULL, sNewTime, true) )
			return false;

		str_ReplaceOnce(strstr(sOut, STRING_TIME), define_value, sNewTime);
	}
	if( !strcmp(define_type, STRING_BUILD) )
	{
		char sNewBuild[50];
		if(!bIncrementVersion)
			return true;
		sprintf( sNewBuild, "%d", atoi(define_value)+1 );
		str_ReplaceOnce(strstr(sOut, STRING_BUILD), define_value, sNewBuild);
	}
  if( !strcmp(define_type, STRING_REVISION) )
  {
    char sNewBuild[50];
    if(!bIncrementRevision)
      return true;
    sprintf( sNewBuild, "%d", atoi(define_value)+1 );
    str_ReplaceOnce(strstr(sOut, STRING_REVISION), define_value, sNewBuild);
  }
	return true;
}

////////////////////////////////////////////////////////////////////////////////

HANDLE OpenVersionFile(const char *sVersionFile)
{
	return CreateFile(sVersionFile,
						GENERIC_READ|GENERIC_WRITE,
						FILE_SHARE_READ,
						NULL,
						OPEN_ALWAYS,
						FILE_ATTRIBUTE_NORMAL,
						NULL);
}

////////////////////////////////////////////////////////////////////////////////

bool ParseVersionFile(HANDLE hFile, char *sOut)
{
	char sIn[MAX_FILE_SIZE];
	DWORD dwBytesRead;

	if( INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN) )
		return false;

	ZeroMemory(sIn, MAX_FILE_SIZE);

	if( !ReadFile(hFile, sIn, MAX_FILE_SIZE, &dwBytesRead, NULL) )
		return false;

	// if the file is empty because it didn't exist and was created by a previous function
	//	or because it IS empty add the standard define values
	if( dwBytesRead == 0 )
	{
		StandardVersionFile(sOut);
		return true;
	}
	else
	{
		char	_define_type[100];
		char	_define_value[100];
		int		ret = 0;
		char	*pIn = sIn;

		strcpy(sOut, sIn);

		while( (ret=sscanf(pIn, " #define %s %s", _define_type, _define_value)) == 2)
		{
			pIn = strstr(pIn, _define_type);
			pIn += strlen(_define_type);
			while(*pIn==' ' || *pIn=='\t')
				pIn++;
			pIn += strlen(_define_value);
			if( !CheckSymbolsAndWriteOut(_define_type, _define_value, sOut) )
				return false;
		}
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////

bool WriteVersionFile(HANDLE hFile, const char *str)
{
	DWORD dwBytesWritten;

	if( INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN) )
	{
		return false;
	}

	if( FALSE == WriteFile(hFile, str, (DWORD) strlen(str), &dwBytesWritten, NULL) || dwBytesWritten != (DWORD) strlen(str))
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	HANDLE hFile = NULL;
	char sOut[MAX_FILE_SIZE];

	bIncrementVersion = false;
  bIncrementRevision = false;
	bSimulate = false;
	bVerbose = false;
	sVersionFile = "version.ver";

	if (argc == 1 || !CheckCommandLine(argc, argv))
	{
		Usage(argc, argv);
		return -1;
	}

	hFile = OpenVersionFile(sVersionFile);
	if( !hFile )
	{
		printf("Failed to open file %s\n", sVersionFile);
		return -1;
	}

	if( !ParseVersionFile(hFile, sOut) )
	{
		puts("ERROR in parsing version file.");
		CloseHandle(hFile);
		return -1;
	}

	if (bSimulate || bVerbose)
	{
		puts(sOut);
	}
	if(!bSimulate)
	{
		if( !WriteVersionFile(hFile, sOut) )
		{
			puts("ERROR: failed to update the file");
			CloseHandle(hFile);
			return -1;
		}
	}

	CloseHandle(hFile);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
