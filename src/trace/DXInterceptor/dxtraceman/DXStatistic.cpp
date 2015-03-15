////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXFileManagerBase.h"
#include "DXStatistic.h"

using namespace dxtraceman;

////////////////////////////////////////////////////////////////////////////////

DXStatistic::DXStatistic()
{
  Clear();
}

////////////////////////////////////////////////////////////////////////////////

DXStatistic::~DXStatistic()
{
  Clear();
}

////////////////////////////////////////////////////////////////////////////////

void DXStatistic::Clear()
{
  m_type = ST_DUMMY;
  m_sync = 0;
  m_legends.clear();
  m_counters.clear();
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXStatistic::GetSerializedSize() const
{
  if (m_type == DXStatistic::ST_DUMMY)
  {
    return 0;
  }
  
  unsigned int size = sizeof(char) + sizeof(dx_uint32) + sizeof(dx_uint16);
  
  switch (m_type)
  {
  case ST_LEGEND:
    size += (unsigned int) m_legends.size() * sizeof(DXStatisticLegendInformation);
    break;
  case ST_COUNTER:
    size += (unsigned int) m_counters.size() * sizeof(DXStatisticCounterDataInformation);
    break;
  }
  
  return size;
}

////////////////////////////////////////////////////////////////////////////////

DXStatistic::StatisticType DXStatistic::GetType() const
{
  return m_type;
}

////////////////////////////////////////////////////////////////////////////////

void DXStatistic::SetType(DXStatistic::StatisticType type)
{
  m_type = type;
}

////////////////////////////////////////////////////////////////////////////////

dx_uint32 DXStatistic::GetSyncPaquet()
{
  return m_sync;
}

////////////////////////////////////////////////////////////////////////////////

void DXStatistic::SetSyncPaquet(dx_uint32 instant)
{
  m_sync = instant;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXStatistic::GetLegendCount() const
{
  if (m_type == ST_LEGEND)
  {
    return (unsigned int) m_legends.size();
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

bool DXStatistic::GetLegend(unsigned int position, DXStatisticLegendInformation** legendInfo)
{
  if (m_type == ST_LEGEND)
  {
    if (position < (unsigned int) m_legends.size())
    {
      *legendInfo = &m_legends[position];
      return true;
    }
    return false;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool DXStatistic::GetLegend(unsigned int position, char** legendName, char** legendDescription, StatisticDataType* legendDataType)
{
  if (m_type == ST_LEGEND)
  {
    if (position < (unsigned int) m_legends.size())
    {
      *legendName = m_legends[position].Name;
      *legendDescription = m_legends[position].Description;
      *legendDataType = m_legends[position].DataType;
      return true;
    }
    return false;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool DXStatistic::AddLegend(char* legendName, char* legendDescription, StatisticDataType legendDataType)
{
  if (m_type == ST_LEGEND)
  {
    m_legends.push_back(DXStatisticLegendInformation(legendName, legendDescription, legendDataType));
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXStatistic::GetCounterDataCount() const
{
  if (m_type == ST_COUNTER)
  {
    return (unsigned int) m_counters.size();
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

bool DXStatistic::GetCounterData(unsigned int position, dx_uint32* data)
{
  if (m_type == ST_COUNTER)
  {
    if (position < (unsigned int) m_counters.size())
    {
      *data = m_counters[position].DataInt32;
      return true;
    }
    return false;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool DXStatistic::GetCounterData(unsigned int position, dx_uint64* data)
{
  if (m_type == ST_COUNTER)
  {
    if (position < (unsigned int) m_counters.size())
    {
      *data = m_counters[position].DataInt64;
      return true;
    }
    return false;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool DXStatistic::GetCounterData(unsigned int position, dx_float* data)
{
  if (m_type == ST_COUNTER)
  {
    if (position < (unsigned int) m_counters.size())
    {
      *data = m_counters[position].DataFloat;
      return true;
    }
    return false;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool DXStatistic::GetCounterData(unsigned int position, dx_double* data)
{
  if (m_type == ST_COUNTER)
  {
    if (position < (unsigned int) m_counters.size())
    {
      *data = m_counters[position].DataDouble;
      return true;
    }
    return false;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool DXStatistic::GetCounterData(unsigned int position, char** data)
{
  if (m_type == ST_COUNTER)
  {
    if (position < (unsigned int) m_counters.size())
    {
      *data = m_counters[position].DataString;
      return true;
    }
    return false;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool DXStatistic::AddCounterData(dx_uint32 data)
{
  m_counters.push_back(DXStatisticCounterDataInformation(data));
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXStatistic::AddCounterData(dx_uint64 data)
{
  m_counters.push_back(DXStatisticCounterDataInformation(data));
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXStatistic::AddCounterData(dx_float data)
{
  m_counters.push_back(DXStatisticCounterDataInformation(data));
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXStatistic::AddCounterData(dx_double data)
{
  m_counters.push_back(DXStatisticCounterDataInformation(data));
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXStatistic::AddCounterData(char* data)
{
  m_counters.push_back(DXStatisticCounterDataInformation(data));
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXStatistic::SerializeToFile(DXFileManagerBase* file)
{
#ifdef _DEBUG
  if (m_type == DXStatistic::ST_DUMMY)
  {
    return false;
  }
#endif // ifdef _DEBUG
  
  if (!file->Write((char*) &m_type, sizeof(char)))
  {
    return false;
  }

  if (!file->Write((char*) &m_sync, sizeof(dx_uint32)))
  {
    return false;
  }

  bool problems = false;
  
  switch (m_type)
  {
  case ST_LEGEND:
    {
      dx_uint16 numElems = (dx_uint16) m_legends.size();
      if (!file->Write((char*) &numElems, sizeof(dx_uint16)))
      {
        return false;
      }
      for (dx_uint16 i=0; i < numElems && !problems; ++i)
      {
        problems = !file->Write((char*) &m_legends[i], sizeof(DXStatisticLegendInformation));
      }
    }
    break;
  
  case ST_COUNTER:
    {
      dx_uint16 numElems = (dx_uint16) m_counters.size();
      if (!file->Write((char*) &numElems, sizeof(dx_uint16)))
      {
        return false;
      }
      for (dx_uint16 i=0; i < numElems && !problems; ++i)
      {
        problems = !file->Write((char*) &m_counters[i], sizeof(DXStatisticCounterDataInformation));
      }
    }
    break;
  }
  
  return !problems;
}

////////////////////////////////////////////////////////////////////////////////

bool DXStatistic::DeserializeFromFile(DXFileManagerBase* file, unsigned int size)
{
#ifdef _DEBUG
  if (size <= sizeof(char) + sizeof(dx_uint32) + sizeof(dx_uint16))
  {
    return false;
  }
#endif // ifdef _DEBUG
  
  Clear();

  char* data = new char[size];

  if (!data)
  {
    return false;
  }

  if (!file->Read(data, size))
  {
    delete[] data;
    return false;
  }

  register unsigned int readedBytes = 0;
  unsigned int numElems = 0;
  
  memcpy((char*) &m_type, &data[readedBytes], sizeof(char));
  readedBytes += sizeof(char);
  
  if (m_type != DXStatistic::ST_LEGEND &&
      m_type != DXStatistic::ST_COUNTER)
  {
    delete[] data;
    return false;
  }
  
  memcpy((char*) &m_sync, &data[readedBytes], sizeof(dx_uint32));
  readedBytes += sizeof(dx_uint32);
  memcpy((char*) &numElems, &data[readedBytes], sizeof(dx_uint16));
  readedBytes += sizeof(dx_uint16);
  
  switch (m_type)
  {
  case ST_LEGEND:
    {
      DXStatisticLegendInformation legend;
      for (unsigned int i=0; i < numElems; ++i)
      {
        memcpy((char*) &legend, &data[readedBytes], sizeof(DXStatisticLegendInformation));
        readedBytes += sizeof(DXStatisticLegendInformation);
        m_legends.push_back(legend);
      }
    }
    break;
  case ST_COUNTER:
    {
      DXStatisticCounterDataInformation counter;
      for (unsigned int i=0; i < numElems; ++i)
      {
        memcpy((char*) &counter, &data[readedBytes], sizeof(DXStatisticCounterDataInformation));
        readedBytes += sizeof(DXStatisticCounterDataInformation);
        m_counters.push_back(counter);
      }
    }
    break;
  }
  
  delete[] data;
  return true;
}

////////////////////////////////////////////////////////////////////////////////
