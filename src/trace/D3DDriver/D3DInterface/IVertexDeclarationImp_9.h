#ifndef IVERTEXDECLARATIONIMP_9_H
#define IVERTEXDECLARATIONIMP_9_H

class IVertexDeclarationImp9 : public IDirect3DVertexDeclaration9{
public:
   /// Singleton method, maintained to allow unimplemented methods to return valid interface addresses.
    static IVertexDeclarationImp9 &getInstance();

    IVertexDeclarationImp9(StateDataNode* s_parent, IDeviceImp9* i_parent, CONST D3DVERTEXELEMENT9* elements);
private:
    /// Singleton constructor method
    IVertexDeclarationImp9();

    IDeviceImp9* i_parent;
    StateDataNode* state;

public:

    HRESULT D3D_CALL QueryInterface (  REFIID riid , void** ppvObj );
    ULONG D3D_CALL AddRef ( );
    ULONG D3D_CALL Release ( );
    HRESULT D3D_CALL GetDevice (  IDirect3DDevice9** ppDevice );
    HRESULT D3D_CALL GetDeclaration (  D3DVERTEXELEMENT9* pElement , UINT* pNumElements );
};

#endif

