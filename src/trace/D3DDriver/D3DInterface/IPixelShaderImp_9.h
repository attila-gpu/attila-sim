#ifndef IPIXELSHADERIMP_9_H
#define IPIXELSHADERIMP_9_H

class IPixelShaderImp9 : public IDirect3DPixelShader9{
public:
    /// Singleton method, maintained to allow unimplemented methods to return valid interface addresses.
    static IPixelShaderImp9 &getInstance();

    IPixelShaderImp9(StateDataNode* s_parent, IDeviceImp9* i_parent, CONST DWORD* pFunction);

private:
    /// Singleton constructor method
    IPixelShaderImp9();

    IDeviceImp9* i_parent;
    StateDataNode* state;

    ULONG refs;

public:
    HRESULT D3D_CALL QueryInterface (  REFIID riid , void** ppvObj );
    ULONG D3D_CALL AddRef ( );
    ULONG D3D_CALL Release ( );
    HRESULT D3D_CALL GetDevice (  IDirect3DDevice9** ppDevice );
    HRESULT D3D_CALL GetFunction (  void* pData , UINT* pSizeOfData );
};

#endif
