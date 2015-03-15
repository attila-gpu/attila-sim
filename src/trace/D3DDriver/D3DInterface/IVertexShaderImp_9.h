#ifndef IVERTEXSHADERIMP_9_H
#define IVERTEXSHADERIMP_9_H

class IVertexShaderImp9 : public IDirect3DVertexShader9{
public:
    /// Singleton method, maintained to allow unimplemented methods to return valid interface addresses.
    static IVertexShaderImp9 &getInstance();

    IVertexShaderImp9(StateDataNode* s_parent, IDeviceImp9* i_parent, CONST DWORD* pFunction);
private:
    /// Singleton constructor method
    IVertexShaderImp9();

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
