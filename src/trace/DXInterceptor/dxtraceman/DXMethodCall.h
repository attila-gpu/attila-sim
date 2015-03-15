////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "SmartPointer.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxtraceman
{
  //////////////////////////////////////////////////////////////////////////////
  
  class DXTraceManager;
  
  //////////////////////////////////////////////////////////////////////////////

  class DXMethodCall : public SmartPointer
  {
  public:

    DXMethodCall();
    DXMethodCall(DXMethodCallHelper::DXMethodCallToken token);
    DXMethodCall(DXMethodCallHelper::DXMethodCallToken token, DWORD creatorID);
    virtual ~DXMethodCall();

    void Clear();
    
    DXMethodCallHelper::DXMethodCallToken GetToken() const;
    void SetToken(DXMethodCallHelper::DXMethodCallToken token);

    DWORD GetCreatorID() const;
    void SetCreatorID(DWORD creatorID);

    bool GetIsSavedReturnValue() const;
    void SetIsSavedReturnValue(bool isSaved);
    
    unsigned int GetSize() const;
    unsigned int GetSerializedSize() const;
    DWORD GetCRC32();

    bool SerializeToBuffer(char** buffer, unsigned int* size);
    bool SerializeToString(std::string& cadena, const DXTypeHelper::StringConversionOptions* options = NULL);
    bool SerializeToString(std::ostream& stream, const DXTypeHelper::StringConversionOptions* options = NULL);

    bool DeserializeFromBuffer(const char* buffer, unsigned int size);
    bool DeserializeFromBufferFastInit(char** buffer, unsigned int size);
    bool DeserializeFromBufferFast();

    bool CheckNextPopType(DXTypeHelper::DXTypeType type);

    unsigned int GetParamCount() const;
    DXTypeHelper::DXTypeType GetParamType(unsigned int numparam) const;
    bool GetParam(unsigned int numparam, char* buffer);

    // basic types

    bool Pop_HRESULT(HRESULT* value);
    bool Push_HRESULT(HRESULT value);

    bool Pop_HMONITOR(HMONITOR* value);
    bool Push_HMONITOR(HMONITOR value);
    
    bool Pop_BYTE(BYTE* value);
    bool Push_BYTE(BYTE value);

    bool Pop_WORD(WORD* value);
    bool Push_WORD(WORD value);

    bool Pop_DWORD(DWORD* value);
    bool Push_DWORD(DWORD value);

    bool Pop_INT(INT* value);
    bool Push_INT(INT value);

    bool Pop_UINT(UINT* value);
    bool Push_UINT(UINT value);

    bool Pop_ULONG(ULONG* value);
    bool Push_ULONG(ULONG value);

    bool Pop_BOOL(BOOL* value);
    bool Push_BOOL(BOOL value);

    bool Pop_int(int* value);
    bool Push_int(int value);

    bool Pop_HWND(HWND* value);
    bool Push_HWND(HWND value);

    bool Pop_float(float* value);
    bool Push_float(float value);

    bool Pop_D3DCOLOR(D3DCOLOR* value);
    bool Push_D3DCOLOR(D3DCOLOR value);

    // enum types

    bool Pop_D3DFORMAT(D3DFORMAT* value);
    bool Push_D3DFORMAT(D3DFORMAT value);

    bool Pop_D3DDEVTYPE(D3DDEVTYPE* value);
    bool Push_D3DDEVTYPE(D3DDEVTYPE value);

    bool Pop_D3DSWAPEFFECT(D3DSWAPEFFECT* value);
    bool Push_D3DSWAPEFFECT(D3DSWAPEFFECT value);

    bool Pop_D3DMULTISAMPLE_TYPE(D3DMULTISAMPLE_TYPE* value);
    bool Push_D3DMULTISAMPLE_TYPE(D3DMULTISAMPLE_TYPE value);

    bool Pop_D3DRESOURCETYPE(D3DRESOURCETYPE* value);
    bool Push_D3DRESOURCETYPE(D3DRESOURCETYPE value);

    bool Pop_D3DRENDERSTATETYPE(D3DRENDERSTATETYPE* value);
    bool Push_D3DRENDERSTATETYPE(D3DRENDERSTATETYPE value);

    bool Pop_D3DPOOL(D3DPOOL* value);
    bool Push_D3DPOOL(D3DPOOL value);

    bool Pop_D3DQUERYTYPE(D3DQUERYTYPE* value);
    bool Push_D3DQUERYTYPE(D3DQUERYTYPE value);

    bool Pop_D3DTRANSFORMSTATETYPE(D3DTRANSFORMSTATETYPE* value);
    bool Push_D3DTRANSFORMSTATETYPE(D3DTRANSFORMSTATETYPE value);

    bool Pop_D3DPRIMITIVETYPE(D3DPRIMITIVETYPE* value);
    bool Push_D3DPRIMITIVETYPE(D3DPRIMITIVETYPE value);

    bool Pop_D3DSAMPLERSTATETYPE(D3DSAMPLERSTATETYPE* value);
    bool Push_D3DSAMPLERSTATETYPE(D3DSAMPLERSTATETYPE value);

    bool Pop_D3DTEXTURESTAGESTATETYPE(D3DTEXTURESTAGESTATETYPE* value);
    bool Push_D3DTEXTURESTAGESTATETYPE(D3DTEXTURESTAGESTATETYPE value);

    bool Pop_D3DBACKBUFFER_TYPE(D3DBACKBUFFER_TYPE* value);
    bool Push_D3DBACKBUFFER_TYPE(D3DBACKBUFFER_TYPE value);

    bool Pop_D3DSTATEBLOCKTYPE(D3DSTATEBLOCKTYPE* value);
    bool Push_D3DSTATEBLOCKTYPE(D3DSTATEBLOCKTYPE value);

    bool Pop_D3DTEXTUREFILTERTYPE(D3DTEXTUREFILTERTYPE* value);
    bool Push_D3DTEXTUREFILTERTYPE(D3DTEXTUREFILTERTYPE value);

    bool Pop_D3DCUBEMAP_FACES(D3DCUBEMAP_FACES* value);
    bool Push_D3DCUBEMAP_FACES(D3DCUBEMAP_FACES value);

    // struct types

    bool Pop_IID(IID* value);
    bool Push_IID(const IID* value);

    bool Pop_RECT(RECT* value);
    bool Push_RECT(const RECT* value);

    bool Pop_POINT(POINT* value);
    bool Push_POINT(const POINT* value);

    bool Pop_D3DBOX(D3DBOX* value);
    bool Push_D3DBOX(const D3DBOX* value);

    bool Pop_D3DCLIPSTATUS9(D3DCLIPSTATUS9* value);
    bool Push_D3DCLIPSTATUS9(const D3DCLIPSTATUS9* value);

    // special types

    bool Pop_DXBufferIdentifier(DXBufferIdentifier* value);
    bool Push_DXBufferIdentifier(DXBufferIdentifier value);

    bool Pop_DXTextureIdentifier(DXTextureIdentifier* value);
    bool Push_DXTextureIdentifier(DXTextureIdentifier value);

    bool Pop_DXIgnoredParameter(DXIgnoredParameter* value);
    bool Push_DXIgnoredParameter(DXIgnoredParameter value = 0);

    bool Pop_DXNullPointer(DXNullPointer* value);
    bool Push_DXNullPointer(DXNullPointer value = NULL);

    bool Pop_DXResourceObjectID(DXResourceObjectID* value);
    bool Push_DXResourceObjectID(DXResourceObjectID value);

    bool Pop_DXVoid(DXVoid* value);
    bool Push_DXVoid(DXVoid value = 0);

    // flags types

    bool Pop_DXFlagsD3DLOCK(DXFlagsD3DLOCK* value);
    bool Push_DXFlagsD3DLOCK(DXFlagsD3DLOCK value);

    bool Pop_DXFlagsD3DCREATE(DXFlagsD3DCREATE* value);
    bool Push_DXFlagsD3DCREATE(DXFlagsD3DCREATE value);

    bool Pop_DXFlagsD3DCLEAR(DXFlagsD3DCLEAR* value);
    bool Push_DXFlagsD3DCLEAR(DXFlagsD3DCLEAR value);

    bool Pop_DXFlagsD3DFVF(DXFlagsD3DFVF* value);
    bool Push_DXFlagsD3DFVF(DXFlagsD3DFVF value);

    bool Pop_DXFlagsD3DUSAGE(DXFlagsD3DUSAGE* value);
    bool Push_DXFlagsD3DUSAGE(DXFlagsD3DUSAGE value);

    bool Pop_DXFlagsD3DSGR(DXFlagsD3DSGR* value);
    bool Push_DXFlagsD3DSGR(DXFlagsD3DSGR value);

    bool Pop_DXFlagsD3DISSUE(DXFlagsD3DISSUE* value);
    bool Push_DXFlagsD3DISSUE(DXFlagsD3DISSUE value);

    bool Pop_DXFlagsD3DENUM(DXFlagsD3DENUM* value);
    bool Push_DXFlagsD3DENUM(DXFlagsD3DENUM value);

    bool Pop_DXFlagsD3DPRESENT(DXFlagsD3DPRESENT* value);
    bool Push_DXFlagsD3DPRESENT(DXFlagsD3DPRESENT value);

  protected:

    static const unsigned int DXCALL_MAX_SIZE = 128;
    static const unsigned int DXCALL_MAX_PARAMS = 16;

    struct DXMethodCallParam
    {
      DXTypeHelper::DXTypeType Type : 8;
      unsigned char Position : 8;
      unsigned char Size : 8;
    };

    char m_buffer[DXMethodCall::DXCALL_MAX_SIZE];
    unsigned int m_positionRead;
    unsigned int m_positionWrite;
    DWORD m_crc32;

    unsigned int m_paramsCount;
    unsigned int m_paramsCurrent;
    DXMethodCallParam m_params[DXMethodCall::DXCALL_MAX_PARAMS];

    unsigned int GetNumParams() const;
    void SetNumParams();
    bool ParamsBufferRead(char* buffer, BYTE size);
    bool ParamsBufferWrite(const char* buffer, BYTE type, BYTE size);

  };

  //////////////////////////////////////////////////////////////////////////////

  typedef smart_ptr<DXMethodCall> DXMethodCallPtr;

  //////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
