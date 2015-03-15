////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "arraystream.h"
#include "DXEnumHelper.h"
#include "DXStructHelper.h"
#include "DXTypeHelper.h"

using namespace std;
using namespace dxtraceman;

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

////////////////////////////////////////////////////////////////////////////////

unsigned int DXTypeHelper::ms_typeSizes[DXTypeHelper::TYPE_ARR_SIZE];

// simulate a static constructor in C++
namespace { bool TypesArray_initialiser = (DXTypeHelper::InitializeTypesArray(), true); }

////////////////////////////////////////////////////////////////////////////////

#define ptr_to_value(ptr, type) (##type) *((##type*) ptr)

////////////////////////////////////////////////////////////////////////////////

#define INSERT_SIZE_MACRO(type) DXTypeHelper::ms_typeSizes[TT_##type] = sizeof(##type)

////////////////////////////////////////////////////////////////////////////////

#define PRINT_ENUM_MACRO(type) \
  if (options) \
  { \
    switch (options->enumerationFormat) \
    { \
    case EF_Textual: \
      return DXEnumHelper::type##_ToString(buffer, size, ptr_to_value(data, ##type)); \
    case EF_HexaDecimal: \
      return snprintf(buffer, size, "0x%08X", ptr_to_value(data, UINT)); \
    case EF_Decimal: \
      return snprintf(buffer, size, "%u", ptr_to_value(data, UINT)); \
    } \
  } \
  else \
  { \
    return DXEnumHelper::type##_ToString(buffer, size, ptr_to_value(data, ##type)); \
  }

////////////////////////////////////////////////////////////////////////////////

#define PRINT_FLAGS_MACRO(type) \
  if (options) \
  { \
    switch (options->enumerationFormat) \
    { \
    case EF_Textual: \
      return type##_ToString(buffer, size, ptr_to_value(data, ##type)); \
    case EF_HexaDecimal: \
      return snprintf(buffer, size, "0x%08X", ptr_to_value(data, ##type)); \
    case EF_Decimal: \
      return snprintf(buffer, size, "%u", ptr_to_value(data, ##type)); \
    } \
  } \
  else \
  { \
    return type##_ToString(buffer, size, ptr_to_value(data, ##type)); \
  }

////////////////////////////////////////////////////////////////////////////////

#define CHECK_FLAG_VALUE_INIT_MACRO() \
  unsigned int numFlags = 0; \
  if (!flags) \
  { \
    return snprintf(buffer, size, "0x00000000"); \
  } \
  arraystream buf(buffer, size); \
  ostream out(&buf);

////////////////////////////////////////////////////////////////////////////////

#define CHECK_FLAG_VALUE_MACRO(constant) \
  if (flags & ##constant) \
  { \
    flags &= ~##constant; \
    if (numFlags++) out << " | "; \
    out << #constant; \
  }

////////////////////////////////////////////////////////////////////////////////

#define CHECK_FLAG_VALUE_FINAL_MACRO() \
  if (flags) \
  { \
    if (numFlags++) out << " | "; \
    out << "0x" << setbase(16) << uppercase << setw(8) << setfill('0') << flags; \
  } \
  buf.finalize(); \
  return buf.tellp();

////////////////////////////////////////////////////////////////////////////////

int IID_ToString(char* buffer, unsigned int size, const IID* iid)
{
  struct
  {
    unsigned int   dw1;
    unsigned short w1;
    unsigned short w2;
    unsigned char  b1;
    unsigned char  b2;
    unsigned char  b3;
    unsigned char  b4;
    unsigned char  b5;
    unsigned char  b6;
    unsigned char  b7;
    unsigned char  b8;
  } local_iid;

  memcpy(&local_iid, iid, sizeof(local_iid));

  arraystream buf(buffer, size);
  ostream out(&buf);
  
  out << "{" <<
    setbase(16) << uppercase << setw(8) << setfill('0') << local_iid.dw1 << "-" <<
    setbase(16) << uppercase << setw(4) << setfill('0') << local_iid.w1  << "-" <<
    setbase(16) << uppercase << setw(4) << setfill('0') << local_iid.w2  << "-" <<
    setbase(16) << uppercase << setw(2) << setfill('0') << (int) local_iid.b1  <<
    setbase(16) << uppercase << setw(2) << setfill('0') << (int) local_iid.b2  << "-" <<
    setbase(16) << uppercase << setw(2) << setfill('0') << (int) local_iid.b3  <<
    setbase(16) << uppercase << setw(2) << setfill('0') << (int) local_iid.b4  <<
    setbase(16) << uppercase << setw(2) << setfill('0') << (int) local_iid.b5  <<
    setbase(16) << uppercase << setw(2) << setfill('0') << (int) local_iid.b6  <<
    setbase(16) << uppercase << setw(2) << setfill('0') << (int) local_iid.b7  <<
    setbase(16) << uppercase << setw(2) << setfill('0') << (int) local_iid.b8  <<
    "}";

  buf.finalize();
  return buf.tellp();
}

////////////////////////////////////////////////////////////////////////////////

int RECT_ToString(char* buffer, unsigned int size, const RECT* rect)
{
  return snprintf(buffer, size, "{%u, %u, %u, %u}", rect->left, rect->top, rect->right, rect->bottom);
}

////////////////////////////////////////////////////////////////////////////////

int POINT_ToString(char* buffer, unsigned int size, const POINT* point)
{
  return snprintf(buffer, size, "{%u, %u}", point->x, point->y);
}

////////////////////////////////////////////////////////////////////////////////

int D3DBOX_ToString(char* buffer, unsigned int size, const D3DBOX* box)
{
  return snprintf(buffer, size, "{%u, %u, %u, %u, %u, %u}", box->Left, box->Top, box->Right, box->Bottom, box->Front, box->Back);
}

////////////////////////////////////////////////////////////////////////////////

int D3DCLIPSTATUS9_ToString(char* buffer, unsigned int size, const D3DCLIPSTATUS9* cs)
{
  char local_buffer[256];
  arraystream buf(buffer, size);
  ostream out(&buf);

  DXTypeHelper::ToString(local_buffer, sizeof(local_buffer), &cs->ClipUnion, DXTypeHelper::TT_DXFlagsD3DCS);
  out << "{" << local_buffer << ", ";
  DXTypeHelper::ToString(local_buffer, sizeof(local_buffer), &cs->ClipIntersection, DXTypeHelper::TT_DXFlagsD3DCS);
  out << local_buffer << "}";

  buf.finalize();
  return buf.tellp();
}

////////////////////////////////////////////////////////////////////////////////

int D3DCOLOR_ToString(char* buffer, unsigned int size, const D3DCOLOR color)
{
  return snprintf(buffer, size, "D3DCOLOR_ARGB(%u, %u, %u, %u)", (unsigned int) ((color >> 24) & 0xFF), (unsigned int) ((color >> 16) & 0xFF), (unsigned int) ((color >> 8) & 0xFF), (unsigned int) (color & 0xFF));
}

////////////////////////////////////////////////////////////////////////////////

int DXFlagsD3DLOCK_ToString(char* buffer, unsigned int size, DXFlagsD3DLOCK flags)
{
  CHECK_FLAG_VALUE_INIT_MACRO();
  CHECK_FLAG_VALUE_MACRO(D3DLOCK_DISCARD);
  CHECK_FLAG_VALUE_MACRO(D3DLOCK_NO_DIRTY_UPDATE);
  CHECK_FLAG_VALUE_MACRO(D3DLOCK_NOSYSLOCK);
  CHECK_FLAG_VALUE_MACRO(D3DLOCK_READONLY);
  CHECK_FLAG_VALUE_MACRO(D3DLOCK_NOOVERWRITE);
  CHECK_FLAG_VALUE_FINAL_MACRO();
}

////////////////////////////////////////////////////////////////////////////////

int DXFlagsD3DCREATE_ToString(char* buffer, unsigned int size, DXFlagsD3DCREATE flags)
{
  CHECK_FLAG_VALUE_INIT_MACRO();
  CHECK_FLAG_VALUE_MACRO(D3DCREATE_ADAPTERGROUP_DEVICE);
  CHECK_FLAG_VALUE_MACRO(D3DCREATE_DISABLE_DRIVER_MANAGEMENT);
  CHECK_FLAG_VALUE_MACRO(D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX);
  CHECK_FLAG_VALUE_MACRO(D3DCREATE_FPU_PRESERVE);
  CHECK_FLAG_VALUE_MACRO(D3DCREATE_HARDWARE_VERTEXPROCESSING);
  CHECK_FLAG_VALUE_MACRO(D3DCREATE_MIXED_VERTEXPROCESSING);
  CHECK_FLAG_VALUE_MACRO(D3DCREATE_MULTITHREADED);
  CHECK_FLAG_VALUE_MACRO(D3DCREATE_NOWINDOWCHANGES);
  CHECK_FLAG_VALUE_MACRO(D3DCREATE_PUREDEVICE);
  CHECK_FLAG_VALUE_MACRO(D3DCREATE_SOFTWARE_VERTEXPROCESSING);
  CHECK_FLAG_VALUE_FINAL_MACRO();
}

////////////////////////////////////////////////////////////////////////////////

int DXFlagsD3DCLEAR_ToString(char* buffer, unsigned int size, DXFlagsD3DCLEAR flags)
{
  CHECK_FLAG_VALUE_INIT_MACRO();
  CHECK_FLAG_VALUE_MACRO(D3DCLEAR_STENCIL);
  CHECK_FLAG_VALUE_MACRO(D3DCLEAR_TARGET);
  CHECK_FLAG_VALUE_MACRO(D3DCLEAR_ZBUFFER);
  CHECK_FLAG_VALUE_FINAL_MACRO();
}

////////////////////////////////////////////////////////////////////////////////

int DXFlagsD3DFVF_ToString(char* buffer, unsigned int size, DXFlagsD3DFVF flags)
{
  CHECK_FLAG_VALUE_INIT_MACRO();
  CHECK_FLAG_VALUE_MACRO(D3DFVF_DIFFUSE);
  CHECK_FLAG_VALUE_MACRO(D3DFVF_NORMAL);
  CHECK_FLAG_VALUE_MACRO(D3DFVF_PSIZE);
  CHECK_FLAG_VALUE_MACRO(D3DFVF_SPECULAR);
  CHECK_FLAG_VALUE_MACRO(D3DFVF_XYZ);
  CHECK_FLAG_VALUE_MACRO(D3DFVF_XYZRHW);
  CHECK_FLAG_VALUE_MACRO(D3DFVF_XYZB1);
  CHECK_FLAG_VALUE_MACRO(D3DFVF_XYZB2);
  CHECK_FLAG_VALUE_MACRO(D3DFVF_XYZB3);
  CHECK_FLAG_VALUE_MACRO(D3DFVF_XYZB4);
  CHECK_FLAG_VALUE_MACRO(D3DFVF_XYZB5);
  CHECK_FLAG_VALUE_MACRO(D3DFVF_XYZW);
  CHECK_FLAG_VALUE_MACRO(D3DFVF_TEX0);
  CHECK_FLAG_VALUE_MACRO(D3DFVF_TEX1);
  CHECK_FLAG_VALUE_MACRO(D3DFVF_TEX2);
  CHECK_FLAG_VALUE_MACRO(D3DFVF_TEX3);
  CHECK_FLAG_VALUE_MACRO(D3DFVF_TEX4);
  CHECK_FLAG_VALUE_MACRO(D3DFVF_TEX5);
  CHECK_FLAG_VALUE_MACRO(D3DFVF_TEX6);
  CHECK_FLAG_VALUE_MACRO(D3DFVF_TEX7);
  CHECK_FLAG_VALUE_MACRO(D3DFVF_TEX8);
  CHECK_FLAG_VALUE_MACRO(D3DFVF_POSITION_MASK);
  CHECK_FLAG_VALUE_MACRO(D3DFVF_TEXCOUNT_MASK);
  CHECK_FLAG_VALUE_MACRO(D3DFVF_LASTBETA_D3DCOLOR);
  CHECK_FLAG_VALUE_MACRO(D3DFVF_LASTBETA_UBYTE4);
  CHECK_FLAG_VALUE_FINAL_MACRO();
}

////////////////////////////////////////////////////////////////////////////////

int DXFlagsD3DUSAGE_ToString(char* buffer, unsigned int size, DXFlagsD3DUSAGE flags)
{
  CHECK_FLAG_VALUE_INIT_MACRO();
  CHECK_FLAG_VALUE_MACRO(D3DUSAGE_AUTOGENMIPMAP);
  CHECK_FLAG_VALUE_MACRO(D3DUSAGE_DEPTHSTENCIL);
  CHECK_FLAG_VALUE_MACRO(D3DUSAGE_DMAP);
  CHECK_FLAG_VALUE_MACRO(D3DUSAGE_DONOTCLIP);
  CHECK_FLAG_VALUE_MACRO(D3DUSAGE_DYNAMIC);
  CHECK_FLAG_VALUE_MACRO(D3DUSAGE_NPATCHES);
  CHECK_FLAG_VALUE_MACRO(D3DUSAGE_POINTS);
  CHECK_FLAG_VALUE_MACRO(D3DUSAGE_RENDERTARGET);
  CHECK_FLAG_VALUE_MACRO(D3DUSAGE_RTPATCHES);
  CHECK_FLAG_VALUE_MACRO(D3DUSAGE_SOFTWAREPROCESSING);
  CHECK_FLAG_VALUE_MACRO(D3DUSAGE_WRITEONLY);
  CHECK_FLAG_VALUE_FINAL_MACRO();
}

////////////////////////////////////////////////////////////////////////////////

int DXFlagsD3DSGR_ToString(char* buffer, unsigned int size, DXFlagsD3DSGR flags)
{
  CHECK_FLAG_VALUE_INIT_MACRO();
  CHECK_FLAG_VALUE_MACRO(D3DSGR_NO_CALIBRATION);
  CHECK_FLAG_VALUE_MACRO(D3DSGR_CALIBRATE);
  CHECK_FLAG_VALUE_FINAL_MACRO();
}

////////////////////////////////////////////////////////////////////////////////

int DXFlagsD3DISSUE_ToString(char* buffer, unsigned int size, DXFlagsD3DISSUE flags)
{
  CHECK_FLAG_VALUE_INIT_MACRO();
  CHECK_FLAG_VALUE_MACRO(D3DISSUE_BEGIN);
  CHECK_FLAG_VALUE_MACRO(D3DISSUE_END);
  CHECK_FLAG_VALUE_FINAL_MACRO();
}

////////////////////////////////////////////////////////////////////////////////

int DXFlagsD3DCS_ToString(char* buffer, unsigned int size, DXFlagsD3DCS flags)
{
  CHECK_FLAG_VALUE_INIT_MACRO();
  CHECK_FLAG_VALUE_MACRO(D3DCS_LEFT);
  CHECK_FLAG_VALUE_MACRO(D3DCS_RIGHT);
  CHECK_FLAG_VALUE_MACRO(D3DCS_TOP);
  CHECK_FLAG_VALUE_MACRO(D3DCS_BOTTOM);
  CHECK_FLAG_VALUE_MACRO(D3DCS_FRONT);
  CHECK_FLAG_VALUE_MACRO(D3DCS_BACK);
  CHECK_FLAG_VALUE_MACRO(D3DCS_PLANE0);
  CHECK_FLAG_VALUE_MACRO(D3DCS_PLANE1);
  CHECK_FLAG_VALUE_MACRO(D3DCS_PLANE2);
  CHECK_FLAG_VALUE_MACRO(D3DCS_PLANE3);
  CHECK_FLAG_VALUE_MACRO(D3DCS_PLANE4);
  CHECK_FLAG_VALUE_MACRO(D3DCS_PLANE5);
  CHECK_FLAG_VALUE_FINAL_MACRO();
}

////////////////////////////////////////////////////////////////////////////////

int DXFlagsD3DENUM_ToString(char* buffer, unsigned int size, DXFlagsD3DENUM flags)
{
  CHECK_FLAG_VALUE_INIT_MACRO();
  CHECK_FLAG_VALUE_MACRO(D3DENUM_WHQL_LEVEL);
  CHECK_FLAG_VALUE_FINAL_MACRO();
}

////////////////////////////////////////////////////////////////////////////////

int DXFlagsD3DPRESENT_ToString(char* buffer, unsigned int size, DXFlagsD3DPRESENT flags)
{
  CHECK_FLAG_VALUE_INIT_MACRO();
  CHECK_FLAG_VALUE_MACRO(D3DPRESENT_DONOTWAIT);
  CHECK_FLAG_VALUE_MACRO(D3DPRESENT_INTERVAL_DEFAULT);
  CHECK_FLAG_VALUE_MACRO(D3DPRESENT_INTERVAL_ONE);
  CHECK_FLAG_VALUE_MACRO(D3DPRESENT_INTERVAL_TWO);
  CHECK_FLAG_VALUE_MACRO(D3DPRESENT_INTERVAL_THREE);
  CHECK_FLAG_VALUE_MACRO(D3DPRESENT_INTERVAL_FOUR);
  CHECK_FLAG_VALUE_MACRO(D3DPRESENT_INTERVAL_IMMEDIATE);
  CHECK_FLAG_VALUE_MACRO(D3DPRESENT_LINEAR_CONTENT);
  CHECK_FLAG_VALUE_FINAL_MACRO();
}

////////////////////////////////////////////////////////////////////////////////

int DXFlagsD3DPRESENTFLAG_ToString(char* buffer, unsigned int size, DXFlagsD3DPRESENTFLAG flags)
{
  CHECK_FLAG_VALUE_INIT_MACRO();
  CHECK_FLAG_VALUE_MACRO(D3DPRESENTFLAG_DEVICECLIP);
  CHECK_FLAG_VALUE_MACRO(D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL);
  CHECK_FLAG_VALUE_MACRO(D3DPRESENTFLAG_LOCKABLE_BACKBUFFER);
  CHECK_FLAG_VALUE_MACRO(D3DPRESENTFLAG_VIDEO);
  CHECK_FLAG_VALUE_FINAL_MACRO();
}

////////////////////////////////////////////////////////////////////////////////

int HRESULT_ToString(char* buffer, unsigned int size, HRESULT result)
{
  switch (result)
  {
  case D3D_OK:
    return snprintf(buffer, size, "D3D_OK");
  case E_FAIL:
    return snprintf(buffer, size, "E_FAIL");
  case E_INVALIDARG:
    return snprintf(buffer, size, "E_INVALIDARG");
  case E_NOINTERFACE:
    return snprintf(buffer, size, "E_NOINTERFACE");
  case E_NOTIMPL:
    return snprintf(buffer, size, "E_NOTIMPL");
  case E_OUTOFMEMORY:
    return snprintf(buffer, size, "E_OUTOFMEMORY");
  case D3DOK_NOAUTOGEN:
    return snprintf(buffer, size, "D3DOK_NOAUTOGEN");
  case D3DERR_WRONGTEXTUREFORMAT:
    return snprintf(buffer, size, "D3DERR_WRONGTEXTUREFORMAT");
  case D3DERR_UNSUPPORTEDCOLOROPERATION:
    return snprintf(buffer, size, "D3DERR_UNSUPPORTEDCOLOROPERATION");
  case D3DERR_UNSUPPORTEDCOLORARG:
    return snprintf(buffer, size, "D3DERR_UNSUPPORTEDCOLORARG");
  case D3DERR_UNSUPPORTEDALPHAOPERATION:
    return snprintf(buffer, size, "D3DERR_UNSUPPORTEDALPHAOPERATION");
  case D3DERR_UNSUPPORTEDALPHAARG:
    return snprintf(buffer, size, "D3DERR_UNSUPPORTEDALPHAARG");
  case D3DERR_TOOMANYOPERATIONS:
    return snprintf(buffer, size, "D3DERR_TOOMANYOPERATIONS");
  case D3DERR_CONFLICTINGTEXTUREFILTER:
    return snprintf(buffer, size, "D3DERR_CONFLICTINGTEXTUREFILTER");
  case D3DERR_UNSUPPORTEDFACTORVALUE:
    return snprintf(buffer, size, "D3DERR_UNSUPPORTEDFACTORVALUE");
  case D3DERR_CONFLICTINGRENDERSTATE:
    return snprintf(buffer, size, "D3DERR_CONFLICTINGRENDERSTATE");
  case D3DERR_UNSUPPORTEDTEXTUREFILTER:
    return snprintf(buffer, size, "D3DERR_UNSUPPORTEDTEXTUREFILTER");
  case D3DERR_CONFLICTINGTEXTUREPALETTE:
    return snprintf(buffer, size, "D3DERR_CONFLICTINGTEXTUREPALETTE");
  case D3DERR_DRIVERINTERNALERROR:
    return snprintf(buffer, size, "D3DERR_DRIVERINTERNALERROR");
  case D3DERR_NOTFOUND:
    return snprintf(buffer, size, "D3DERR_NOTFOUND");
  case D3DERR_MOREDATA:
    return snprintf(buffer, size, "D3DERR_MOREDATA");
  case D3DERR_DEVICELOST:
    return snprintf(buffer, size, "D3DERR_DEVICELOST");
  case D3DERR_DEVICENOTRESET:
    return snprintf(buffer, size, "D3DERR_DEVICENOTRESET");
  case D3DERR_NOTAVAILABLE:
    return snprintf(buffer, size, "D3DERR_NOTAVAILABLE");
  case D3DERR_OUTOFVIDEOMEMORY:
    return snprintf(buffer, size, "D3DERR_OUTOFVIDEOMEMORY");
  case D3DERR_INVALIDDEVICE:
    return snprintf(buffer, size, "D3DERR_INVALIDDEVICE");
  case D3DERR_INVALIDCALL:
    return snprintf(buffer, size, "D3DERR_INVALIDCALL");
  case D3DERR_DRIVERINVALIDCALL:
    return snprintf(buffer, size, "D3DERR_DRIVERINVALIDCALL");
  case D3DERR_WASSTILLDRAWING:
    return snprintf(buffer, size, "D3DERR_WASSTILLDRAWING");
  default:
    return snprintf(buffer, size, "0x%08X", result);
  }
}

////////////////////////////////////////////////////////////////////////////////

int DXTypeHelper::ToString(string& cadena, const void* data, DXTypeType type, const StringConversionOptions* options)
{
  char buffer[256];
  int writedBytes = ToString(buffer, sizeof(buffer), data, type, options);
  cadena = buffer;
  return writedBytes;
}

////////////////////////////////////////////////////////////////////////////////

int DXTypeHelper::ToString(ostream& stream, const void* data, DXTypeType type, const StringConversionOptions* options)
{
  char buffer[256];
  int writedBytes = ToString(buffer, sizeof(buffer), data, type, options);
  stream << buffer;
  return writedBytes;
}

////////////////////////////////////////////////////////////////////////////////

int DXTypeHelper::ToString(char* buffer, unsigned int size, const void* data, DXTypeType type, const StringConversionOptions* options)
{
  switch (type)
  {
  // basic types
  
  case TT_HRESULT:
    return HRESULT_ToString(buffer, size, ptr_to_value(data, HRESULT));
    break;

  case TT_HMONITOR:
    return snprintf(buffer, size, "0x%08X", ptr_to_value(data, unsigned int));
    break;
  
  case TT_BYTE:
    return snprintf(buffer, size, "%u", ptr_to_value(data, BYTE));
    break;

  case TT_WORD:
    return snprintf(buffer, size, "%u", ptr_to_value(data, WORD));
    break;

  case TT_DWORD:
  case TT_UINT:
  case TT_ULONG:
    return snprintf(buffer, size, "%u", ptr_to_value(data, DWORD));
    break;

  case TT_INT:
  case TT_int:
    return snprintf(buffer, size, "%d", ptr_to_value(data, INT));
    break;

  case TT_BOOL:
    {
      BOOL value = ptr_to_value(data, BOOL);
      if (value)
      {
        return snprintf(buffer, size, "TRUE");
      }
      else
      {
        return snprintf(buffer, size, "FALSE");
      }
    }
    break;

  case TT_HWND:
    if (ptr_to_value(data, HWND))
    {
      return snprintf(buffer, size, "0x%08X", ptr_to_value(data, unsigned int));
    }
    else
    {
      return snprintf(buffer, size, "NULL");
    }
    break;

  case TT_float:
    if (options)
    {
      if (options->floatFormat == FF_Textual)
      {
        return snprintf(buffer, size, "%.6f", ptr_to_value(data, float));
      }
      else
      {
        unsigned int serializedFloat;
        float valueFloat = ptr_to_value(data, float);
        memcpy(&serializedFloat, &valueFloat, sizeof(float));
        return snprintf(buffer, size, "0x%08X", serializedFloat);
      }
    }
    else
    {
      return snprintf(buffer, size, "%.6f", ptr_to_value(data, float));
    }
    break;

  case TT_D3DCOLOR:
    return D3DCOLOR_ToString(buffer, size, ptr_to_value(data, D3DCOLOR));
    break;
    
  // enum types
  
  case TT_D3DFORMAT:
    PRINT_ENUM_MACRO(D3DFORMAT);
    break;
  
  case TT_D3DDEVTYPE:
    PRINT_ENUM_MACRO(D3DDEVTYPE);
    break;

  case TT_D3DSWAPEFFECT:
    PRINT_ENUM_MACRO(D3DSWAPEFFECT);
    break;

  case TT_D3DMULTISAMPLE_TYPE:
    PRINT_ENUM_MACRO(D3DMULTISAMPLE_TYPE);
    break;

  case TT_D3DRESOURCETYPE:
    PRINT_ENUM_MACRO(D3DRESOURCETYPE);
    break;

  case TT_D3DRENDERSTATETYPE:
    PRINT_ENUM_MACRO(D3DRENDERSTATETYPE);
    break;
  
  case TT_D3DPOOL:
    PRINT_ENUM_MACRO(D3DPOOL);
    break;

  case TT_D3DQUERYTYPE:
    PRINT_ENUM_MACRO(D3DQUERYTYPE);
    break;

  case TT_D3DTRANSFORMSTATETYPE:
    if (((unsigned int) ptr_to_value(data, D3DTRANSFORMSTATETYPE) < 256) || (options && options->enumerationFormat != EF_Textual))
    {
      PRINT_ENUM_MACRO(D3DTRANSFORMSTATETYPE);
    }
    else if ((unsigned int) ptr_to_value(data, D3DTRANSFORMSTATETYPE) == 256)
    {
      return snprintf(buffer, size, "D3DTS_WORLD");
    }
    else
    {
      return snprintf(buffer, size, "D3DTS_WORLD%u", (unsigned int) ptr_to_value(data, D3DTRANSFORMSTATETYPE) - 256);
    }
    break;

  case TT_D3DPRIMITIVETYPE:
    PRINT_ENUM_MACRO(D3DPRIMITIVETYPE);
    break;

  case TT_D3DSAMPLERSTATETYPE:
    PRINT_ENUM_MACRO(D3DSAMPLERSTATETYPE);
    break;

  case TT_D3DTEXTURESTAGESTATETYPE:
    PRINT_ENUM_MACRO(D3DTEXTURESTAGESTATETYPE);
    break;

  case TT_D3DBACKBUFFER_TYPE:
    PRINT_ENUM_MACRO(D3DBACKBUFFER_TYPE);
    break;

  case TT_D3DSTATEBLOCKTYPE:
    PRINT_ENUM_MACRO(D3DSTATEBLOCKTYPE);
    break;

  case TT_D3DTEXTUREFILTERTYPE:
    PRINT_ENUM_MACRO(D3DTEXTUREFILTERTYPE);
    break;

  case TT_D3DCUBEMAP_FACES:
    PRINT_ENUM_MACRO(D3DCUBEMAP_FACES);
    break;

  // struct types (string serializable)

  case TT_IID:
    return IID_ToString(buffer, size, (const IID*) data);
    break;
  
  case TT_RECT:
    return RECT_ToString(buffer, size, (const RECT*) data);
    break;

  case TT_POINT:
    return POINT_ToString(buffer, size, (const POINT*) data);
    break;

  case TT_D3DBOX:
    return D3DBOX_ToString(buffer, size, (const D3DBOX*) data);
    break;

  case TT_D3DCLIPSTATUS9:
    return D3DCLIPSTATUS9_ToString(buffer, size, (const D3DCLIPSTATUS9*) data);
    break;

  // struct types (not string serializable but printable as comments)

  case TT_D3DPRESENT_PARAMETERS:
    return DXStructHelper::D3DPRESENT_PARAMETERS_ToString(buffer, size, (D3DPRESENT_PARAMETERS*) data);
    break;

  case TT_D3DVIEWPORT9:
    return DXStructHelper::D3DVIEWPORT9_ToString(buffer, size, (D3DVIEWPORT9*) data);
    break;
    
  // special types
  
  case TT_DXResourceObjectID:
    return snprintf(buffer, size, "{obj_%u}", ptr_to_value(data, DXResourceObjectID));
    break;
  
  case TT_DXBufferIdentifier:
    return snprintf(buffer, size, "{buf_%u}", ptr_to_value(data, DXBufferIdentifier));
    break;

  case TT_DXTextureIdentifier:
    return snprintf(buffer, size, "{tex_%u}", ptr_to_value(data, DXTextureIdentifier));
    break;

  case TT_DXIgnoredParameter:
    return snprintf(buffer, size, "#IGNORED#");
    break;

  case TT_DXNullPointer:
    return snprintf(buffer, size, "NULL");
    break;

  case TT_DXVoid:
    return snprintf(buffer, size, "void");
    break;
  
  // flags types

  case TT_DXFlagsD3DLOCK:
    PRINT_FLAGS_MACRO(DXFlagsD3DLOCK);
    break;

  case TT_DXFlagsD3DCREATE:
    PRINT_FLAGS_MACRO(DXFlagsD3DCREATE);
    break;

  case TT_DXFlagsD3DCLEAR:
    PRINT_FLAGS_MACRO(DXFlagsD3DCLEAR);
    break;

  case TT_DXFlagsD3DFVF:
    PRINT_FLAGS_MACRO(DXFlagsD3DFVF);
    break;

  case TT_DXFlagsD3DUSAGE:
    PRINT_FLAGS_MACRO(DXFlagsD3DUSAGE);
    break;
  
  case TT_DXFlagsD3DSGR:
    PRINT_FLAGS_MACRO(DXFlagsD3DSGR);
    break;

  case TT_DXFlagsD3DISSUE:
    PRINT_FLAGS_MACRO(DXFlagsD3DISSUE);
    break;

  case TT_DXFlagsD3DCS:
    PRINT_FLAGS_MACRO(DXFlagsD3DCS);
    break;

  case TT_DXFlagsD3DENUM:
    PRINT_FLAGS_MACRO(DXFlagsD3DENUM);
    break;

  case TT_DXFlagsD3DPRESENT:
    PRINT_FLAGS_MACRO(DXFlagsD3DPRESENT);
    break;

  case TT_DXFlagsD3DPRESENTFLAG:
    PRINT_FLAGS_MACRO(DXFlagsD3DPRESENTFLAG);
    break;

  default:
    return snprintf(buffer, size, "UNKNOW_TYPE_TO_CONVERT_TO_STRING");
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

void DXTypeHelper::InitializeTypesArray()
{
  for (unsigned int i=0; i < DXTypeHelper::TYPE_ARR_SIZE; ++i)
  {
    DXTypeHelper::ms_typeSizes[i] = 0;
  }

  // basic types
  INSERT_SIZE_MACRO(HRESULT);
  INSERT_SIZE_MACRO(HMONITOR);
  INSERT_SIZE_MACRO(BYTE);
  INSERT_SIZE_MACRO(WORD);
  INSERT_SIZE_MACRO(DWORD);
  INSERT_SIZE_MACRO(INT);
  INSERT_SIZE_MACRO(UINT);
  INSERT_SIZE_MACRO(ULONG);
  INSERT_SIZE_MACRO(BOOL);
  INSERT_SIZE_MACRO(int);
  INSERT_SIZE_MACRO(HWND);
  INSERT_SIZE_MACRO(float);
  INSERT_SIZE_MACRO(D3DCOLOR);

  // enum types
  INSERT_SIZE_MACRO(D3DFORMAT);
  INSERT_SIZE_MACRO(D3DDEVTYPE);
  INSERT_SIZE_MACRO(D3DSWAPEFFECT);
  INSERT_SIZE_MACRO(D3DMULTISAMPLE_TYPE);
  INSERT_SIZE_MACRO(D3DRESOURCETYPE);
  INSERT_SIZE_MACRO(D3DRENDERSTATETYPE);
  INSERT_SIZE_MACRO(D3DPOOL);
  INSERT_SIZE_MACRO(D3DQUERYTYPE);
  INSERT_SIZE_MACRO(D3DTRANSFORMSTATETYPE);
  INSERT_SIZE_MACRO(D3DPRIMITIVETYPE);
  INSERT_SIZE_MACRO(D3DSAMPLERSTATETYPE);
  INSERT_SIZE_MACRO(D3DTEXTURESTAGESTATETYPE);
  INSERT_SIZE_MACRO(D3DBACKBUFFER_TYPE);
  INSERT_SIZE_MACRO(D3DSTATEBLOCKTYPE);
  INSERT_SIZE_MACRO(D3DTEXTUREFILTERTYPE);
  INSERT_SIZE_MACRO(D3DCUBEMAP_FACES);

  // struct types
  INSERT_SIZE_MACRO(IID);
  INSERT_SIZE_MACRO(RECT);
  INSERT_SIZE_MACRO(POINT);
  INSERT_SIZE_MACRO(D3DBOX);
  INSERT_SIZE_MACRO(D3DCLIPSTATUS9);
  INSERT_SIZE_MACRO(D3DPRESENT_PARAMETERS);
  INSERT_SIZE_MACRO(D3DLIGHT9);
  INSERT_SIZE_MACRO(D3DMATRIX);
  INSERT_SIZE_MACRO(D3DMATERIAL9);
  INSERT_SIZE_MACRO(D3DVIEWPORT9);
  INSERT_SIZE_MACRO(D3DGAMMARAMP);

  // special types
  INSERT_SIZE_MACRO(DXBufferIdentifier);
  INSERT_SIZE_MACRO(DXTextureIdentifier);
  INSERT_SIZE_MACRO(DXIgnoredParameter);
  INSERT_SIZE_MACRO(DXNullPointer);
  INSERT_SIZE_MACRO(DXResourceObjectID);
  INSERT_SIZE_MACRO(DXVoid);

  // flags types
  INSERT_SIZE_MACRO(DXFlagsD3DLOCK);
  INSERT_SIZE_MACRO(DXFlagsD3DCREATE);
  INSERT_SIZE_MACRO(DXFlagsD3DCLEAR);
  INSERT_SIZE_MACRO(DXFlagsD3DFVF);
  INSERT_SIZE_MACRO(DXFlagsD3DUSAGE);
  INSERT_SIZE_MACRO(DXFlagsD3DSGR);
  INSERT_SIZE_MACRO(DXFlagsD3DISSUE);
  INSERT_SIZE_MACRO(DXFlagsD3DCS);
  INSERT_SIZE_MACRO(DXFlagsD3DENUM);
  INSERT_SIZE_MACRO(DXFlagsD3DPRESENT);
  INSERT_SIZE_MACRO(DXFlagsD3DPRESENTFLAG);
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXTypeHelper::GetTypeSize(DXTypeType type)
{
  return ms_typeSizes[type];
}

////////////////////////////////////////////////////////////////////////////////
