////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

namespace dxtraceman
{
  class DXTypeHelper
  {
  public:
    
    enum DXTypeType
    {
      TT_DummyType = 0,

      // basic types
      TT_HRESULT = 1,
      TT_HMONITOR,
      TT_BYTE,
      TT_WORD,
      TT_DWORD,
      TT_INT,
      TT_UINT,
      TT_ULONG,
      TT_BOOL,
      TT_int,
      TT_HWND,
      TT_float,
      TT_D3DCOLOR,

      // enum types
      TT_D3DFORMAT = 21,
      TT_D3DDEVTYPE,
      TT_D3DSWAPEFFECT,
      TT_D3DMULTISAMPLE_TYPE,
      TT_D3DRESOURCETYPE,
      TT_D3DRENDERSTATETYPE,
      TT_D3DPOOL,
      TT_D3DQUERYTYPE,
      TT_D3DTRANSFORMSTATETYPE,
      TT_D3DPRIMITIVETYPE,
      TT_D3DSAMPLERSTATETYPE,
      TT_D3DTEXTURESTAGESTATETYPE,
      TT_D3DBACKBUFFER_TYPE,
      TT_D3DSTATEBLOCKTYPE,
      TT_D3DTEXTUREFILTERTYPE,
      TT_D3DCUBEMAP_FACES,

      // struct types
      TT_IID = 41,
      TT_RECT,
      TT_POINT,
      TT_D3DBOX,
      TT_D3DCLIPSTATUS9,
      TT_D3DPRESENT_PARAMETERS,
      TT_D3DLIGHT9,
      TT_D3DMATRIX,
      TT_D3DMATERIAL9,
      TT_D3DVIEWPORT9,
      TT_D3DGAMMARAMP,

      // special types (used internaly in the library)
      TT_DXBufferIdentifier = 61,
      TT_DXTextureIdentifier,
      TT_DXIgnoredParameter,
      TT_DXNullPointer,
      TT_DXResourceObjectID,
      TT_DXVoid,
      
      // flags types (used internaly in the library)
      TT_DXFlagsD3DLOCK,
      TT_DXFlagsD3DCREATE,
      TT_DXFlagsD3DCLEAR,
      TT_DXFlagsD3DFVF,
      TT_DXFlagsD3DUSAGE,
      TT_DXFlagsD3DSGR,
      TT_DXFlagsD3DISSUE,
      TT_DXFlagsD3DCS,
      TT_DXFlagsD3DENUM,
      TT_DXFlagsD3DPRESENT,
      TT_DXFlagsD3DPRESENTFLAG,

      // buffer types (used internaly in the library)
      TT_ARR_D3DVERTEXELEMENT9 = 81,
      TT_ARR_RGNDATA,
      TT_ARR_D3DRECT,
      TT_ARR_DRAWPRIMITIVEUP,
      TT_ARR_DRAWINDEXEDPRIMITIVEUPINDICES,
      TT_ARR_DRAWINDEXEDPRIMITIVEUPVERTICES,
      TT_ARR_SHADERFUNCTIONTOKEN,
      TT_ARR_SHADERCONSTANTBOOL,
      TT_ARR_SHADERCONSTANTFLOAT,
      TT_ARR_SHADERCONSTANTINT,
      TT_ARR_SETCLIPPLANE,
      TT_ARR_PALETTEENTRY,
      TT_DXRawData,
      TT_DXTexture,
      TT_DXVolumeTexture
    };

    enum TraceFormat {TF_Textual, TF_Binary};
    enum EnumerationFormat {EF_Textual, EF_HexaDecimal, EF_Decimal};
    enum FloatFormat {FF_Textual, FF_HexaDecimal};
    
    struct StringConversionOptions
    {
      TraceFormat traceFormat;
      FloatFormat floatFormat;
      EnumerationFormat enumerationFormat;
    };
    
    static int ToString(std::string& cadena, const void* data, DXTypeType type, const StringConversionOptions* options = NULL);
    static int ToString(std::ostream& stream, const void* data, DXTypeType type, const StringConversionOptions* options = NULL);
    static int ToString(char* buffer, unsigned int size, const void* data, DXTypeType type, const StringConversionOptions* options = NULL);

    static void InitializeTypesArray();
    static unsigned int GetTypeSize(DXTypeType type);

  protected:
    
    static const unsigned int TYPE_ARR_SIZE = 256;
    static unsigned int ms_typeSizes[TYPE_ARR_SIZE];

  };
}

////////////////////////////////////////////////////////////////////////////////

typedef unsigned int  DXBufferIdentifier;
typedef unsigned int  DXTextureIdentifier;
typedef unsigned char DXIgnoredParameter;
typedef unsigned char DXNullPointer;
typedef DWORD         DXResourceObjectID;
typedef unsigned char DXVoid;
typedef DWORD         DXFlagsD3DLOCK;
typedef DWORD         DXFlagsD3DCREATE;
typedef DWORD         DXFlagsD3DCLEAR;
typedef DWORD         DXFlagsD3DFVF;
typedef DWORD         DXFlagsD3DUSAGE;
typedef DWORD         DXFlagsD3DSGR;
typedef DWORD         DXFlagsD3DISSUE;
typedef DWORD         DXFlagsD3DCS;
typedef DWORD         DXFlagsD3DENUM;
typedef DWORD         DXFlagsD3DPRESENT;
typedef DWORD         DXFlagsD3DPRESENTFLAG;

////////////////////////////////////////////////////////////////////////////////
