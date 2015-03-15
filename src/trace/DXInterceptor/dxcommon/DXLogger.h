////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

class DXLogger
{
public:

  DXLogger(const std::string& filename, bool writeTimeStamp = true);
  virtual ~DXLogger();

  void Write(const std::string& str);
  void Write(const char* fmt, ...);

protected:

  bool m_isOpened;
  bool m_writeTimeStamp;
  std::string m_filename;
  std::ofstream* m_output;

  bool Open();
  void Close();

  void WriteTimeStamp();

};

////////////////////////////////////////////////////////////////////////////////
