////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "SmartPointer.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxtraceman
{
  class DXFileManagerBase;
  
  class DXStatistic : public SmartPointer
  {
  public:

    enum StatisticType
    {
      ST_DUMMY    = 0,
      ST_LEGEND   = 1,
      ST_COUNTER = 2,
    };

    enum StatisticDataType
    {
      SDT_UINT32 = 1,
      SDT_UINT64 = 2,
      SDT_FLOAT  = 3,
      SDT_DOUBLE = 4,
      SDT_STRING = 5,
    };

#pragma pack(push)
#pragma pack(1)

    struct DXStatisticLegendInformation
    {
      char Name[64];
      char Description[128];
      StatisticDataType DataType;

      DXStatisticLegendInformation()
      {
      }

      DXStatisticLegendInformation(char* legendName, char* legendDescription, StatisticDataType legendDataType)
      {
        strncpy(Name, legendName, min(sizeof(Name)-1, strlen(legendName))+1);
        Name[sizeof(Name)-1] = '\0';
        strncpy(Description, legendDescription, min(sizeof(Description)-1, strlen(legendDescription))+1);
        Description[sizeof(Description)-1] = '\0';
        DataType = legendDataType;
      }
    };

#pragma pack(pop)

    DXStatistic();
    virtual ~DXStatistic();

    void Clear();
    unsigned int GetSerializedSize() const;

    StatisticType GetType() const;
    void SetType(StatisticType type);

    dx_uint32 GetSyncPaquet();
    void SetSyncPaquet(dx_uint32 instant);

    unsigned int GetLegendCount() const;
    bool GetLegend(unsigned int position, DXStatisticLegendInformation** legendInfo);
    bool GetLegend(unsigned int position, char** legendName, char** legendDescription, StatisticDataType* legendDataType);
    bool AddLegend(char* legendName, char* legendDescription, StatisticDataType legendDataType);

    unsigned int GetCounterDataCount() const;
    bool GetCounterData(unsigned int position, dx_uint32* data);
    bool GetCounterData(unsigned int position, dx_uint64* data);
    bool GetCounterData(unsigned int position, dx_float*  data);
    bool GetCounterData(unsigned int position, dx_double* data);
    bool GetCounterData(unsigned int position, char**     data);
    bool AddCounterData(dx_uint32 data);
    bool AddCounterData(dx_uint64 data);
    bool AddCounterData(dx_float  data);
    bool AddCounterData(dx_double data);
    bool AddCounterData(char*     data);

    bool SerializeToFile(DXFileManagerBase* file);
    bool DeserializeFromFile(DXFileManagerBase* file, unsigned int size);

  protected:

#pragma pack(push)
#pragma pack(1)

    struct DXStatisticCounterDataInformation
    {
      union
      {
        dx_uint32 DataInt32;
        dx_uint64 DataInt64;
        dx_float  DataFloat;
        dx_double DataDouble;
        char      DataString[32];
      };

      DXStatisticCounterDataInformation()
      {
        memset(DataString, 0, sizeof(DataString));
      }
      
      DXStatisticCounterDataInformation(dx_uint32 value) :
      DataInt32(value)
      {
      }

      DXStatisticCounterDataInformation(dx_uint64 value) :
      DataInt64(value)
      {
      }

      DXStatisticCounterDataInformation(dx_float value) :
      DataFloat(value)
      {
      }

      DXStatisticCounterDataInformation(dx_double value) :
      DataDouble(value)
      {
      }

      DXStatisticCounterDataInformation(char* value)
      {
        strncpy(DataString, value, min(sizeof(DataString)-1, strlen(value))+1);
        DataString[sizeof(DataString)-1] = '\0';
      }
    };

#pragma pack(pop)

    StatisticType m_type;
    dx_uint32 m_sync;
    std::vector<DXStatisticLegendInformation> m_legends;
    std::vector<DXStatisticCounterDataInformation> m_counters;

  };

  //////////////////////////////////////////////////////////////////////////////

  typedef smart_ptr<DXStatistic> DXStatisticPtr;

  //////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
