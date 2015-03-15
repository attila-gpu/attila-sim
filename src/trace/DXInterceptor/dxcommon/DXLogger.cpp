////////////////////////////////////////////////////////////////////////////////

#include <sys/timeb.h>
#include <ctime>
#include <cstdarg>
#include <string>
#include <fstream>
#include "DXLogger.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

DXLogger::DXLogger(const string& filename, bool writeTimeStamp) :
m_isOpened(false),
m_writeTimeStamp(writeTimeStamp),
m_filename(filename),
m_output(NULL)
{
}

////////////////////////////////////////////////////////////////////////////////

DXLogger::~DXLogger()
{
  Close();
}

////////////////////////////////////////////////////////////////////////////////

void DXLogger::Write(const string& str)
{
  if (!m_isOpened)
  {
    if (!Open())
    {
      return;
    }
  }

  WriteTimeStamp();
  
  *m_output << str << endl;

  m_output->flush();
}

////////////////////////////////////////////////////////////////////////////////

void DXLogger::Write(const char* fmt, ...)
{
  if (!m_isOpened)
  {
    if (!Open())
    {
      return;
    }
  }

  WriteTimeStamp();
  
  va_list ap;
  static char buffer1[1024];
  static char buffer2[1024];

  strcpy(buffer1, fmt);
  va_start(ap, fmt);
  vsprintf(buffer2, buffer1, ap);
  va_end(ap);
  
  *m_output << buffer2 << endl;

  m_output->flush();
}

////////////////////////////////////////////////////////////////////////////////

bool DXLogger::Open()
{
  if (m_isOpened)
  {
    Close();
  }

  m_output = new ofstream(m_filename.c_str(), ios::out | ios::app);

  if (m_output && m_output->is_open())
  {
    m_isOpened = true;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

void DXLogger::Close()
{
  if (m_isOpened)
  {
    if (m_output)
    {
      if (m_output->is_open())
      {
        m_output->close();
      }
      delete m_output;
      m_output = NULL;
    }

    m_isOpened = false;
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXLogger::WriteTimeStamp()
{
  if (!m_writeTimeStamp)
  {
    return;
  }
  
  char buffer1[256];
  char buffer2[256];
  struct _timeb ltime;
  struct tm* today;
		
  _ftime(&ltime);
  today = localtime(&(ltime.time));
  strftime(buffer1, sizeof(buffer1), "%d/%m/%Y %H:%M:%S", today);		
  sprintf(buffer2, "%s.%03hu", buffer1, ltime.millitm);
  *m_output << buffer2 << " ";
}

////////////////////////////////////////////////////////////////////////////////
