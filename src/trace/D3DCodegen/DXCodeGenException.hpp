/**************************************************************************
 *
 * Copyright (c) 2002 - 2011 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 */

////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"

////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////
