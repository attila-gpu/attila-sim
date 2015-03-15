#ifndef IQUERYIMP_9_H_INCLUDED
#define IQUERYIMP_9_H_INCLUDED

class IQueryImp9 : public IDirect3DQuery9{
public:
    static IQueryImp9 &getInstance();
    HRESULT D3D_CALL QueryInterface (  REFIID riid , void** ppvObj );
    ULONG D3D_CALL AddRef ( );
    ULONG D3D_CALL Release ( );
    HRESULT D3D_CALL GetDevice (  IDirect3DDevice9** ppDevice );
    D3DQUERYTYPE D3D_CALL GetType ( );
    DWORD D3D_CALL GetDataSize ( );
    HRESULT D3D_CALL Issue (  DWORD dwIssueFlags );
    HRESULT D3D_CALL GetData (  void* pData , DWORD dwSize , DWORD dwGetDataFlags );
private:
    IQueryImp9();
};


#endif

