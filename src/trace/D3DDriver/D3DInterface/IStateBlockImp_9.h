#ifndef ISTATEBLOCKIMP_9_H_INCLUDED
#define ISTATEBLOCKIMP_9_H_INCLUDED

class IStateBlockImp9 : public IDirect3DStateBlock9{
public:
    static IStateBlockImp9 &getInstance();
    HRESULT D3D_CALL QueryInterface (  REFIID riid , void** ppvObj );
    ULONG D3D_CALL AddRef ( );
    ULONG D3D_CALL Release ( );
    HRESULT D3D_CALL GetDevice (  IDirect3DDevice9** ppDevice );
    HRESULT D3D_CALL Capture ( );
    HRESULT D3D_CALL Apply ( );
private:
    IStateBlockImp9();
};



#endif
