////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

class CRC32
{
public:
	
	static unsigned int Calculate(const char* buffer, unsigned int bufferSize, unsigned int initCRC32 = 0xFFFFFFFF);

protected:
	
	static unsigned int ms_crc32Table[256];

};

////////////////////////////////////////////////////////////////////////////////
