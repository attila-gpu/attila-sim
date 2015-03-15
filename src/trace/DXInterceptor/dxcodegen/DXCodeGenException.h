////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  class DXCodeGenException : public std::exception
  {
  public:

    DXCodeGenException(const std::string& message) :
    m_message(message)
    {
    }

    virtual ~DXCodeGenException()
    {
    }

    virtual const char* what() const
    {
      return m_message.c_str();
    }

  protected:

    std::string m_message;

  };
}

////////////////////////////////////////////////////////////////////////////////
