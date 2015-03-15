#ifndef IDEVICE_9_H_INCLUDED
#define IDEVICE_9_H_INCLUDED

class IDirect3DImp9;
class IVertexBufferImp9;
class IIndexBufferImp9;
class IVertexDeclarationImp9;
class IVertexShaderImp9;
class IPixelShaderImp9;
class ISurfaceImp9;
class ITextureImp9;
class ICubeTextureImp9;
class IVolumeTextureImp9;

class IDeviceImp9 : public IDirect3DDevice9{

public:
    /// Singleton method, maintained to allow unimplemented methods to return valid interface addresses.
    static IDeviceImp9 &getInstance();

    IDeviceImp9(StateDataNode *s_parent, IDirect3DImp9* i_parent, D3DPRESENT_PARAMETERS * pPresentationParameters);
    ~IDeviceImp9();

private:
    /// Singleton constructor method
    IDeviceImp9();

    IDirect3DImp9* i_parent;
    StateDataNode* state;

    set< IVertexBufferImp9 * > i_vertex_buffer_childs;
    set< IIndexBufferImp9 * > i_index_buffer_childs;
    set< IVertexDeclarationImp9 * > i_vertex_declaration_childs;
    set< IVertexShaderImp9 * > i_vertex_shader_childs;
    set< IPixelShaderImp9 * > i_pixel_shader_childs;
    set< ITextureImp9* > i_texture_childs;
    set< ICubeTextureImp9* > i_cubetexture_childs;
    set< IVolumeTextureImp9* > i_volumetexture_childs;
    set< ISurfaceImp9* > i_surface_childs;

    /// Checks implicit and current rt/zs are the same
    bool using_implicit_render_target_zstencil();

public:

// IDirect3DDevice9 implementation

    HRESULT D3D_CALL QueryInterface (  REFIID riid , void** ppvObj );
    ULONG D3D_CALL AddRef ( );
    ULONG D3D_CALL Release ( );
    HRESULT D3D_CALL TestCooperativeLevel ( );
    UINT D3D_CALL GetAvailableTextureMem ( );
    HRESULT D3D_CALL EvictManagedResources ( );
    HRESULT D3D_CALL GetDirect3D (  IDirect3D9** ppD3D9 );
    HRESULT D3D_CALL GetDeviceCaps (  D3DCAPS9* pCaps );
    HRESULT D3D_CALL GetDisplayMode (  UINT iSwapChain , D3DDISPLAYMODE* pMode );
    HRESULT D3D_CALL GetCreationParameters (  D3DDEVICE_CREATION_PARAMETERS * pParameters );
    HRESULT D3D_CALL SetCursorProperties (  UINT XHotSpot , UINT YHotSpot , IDirect3DSurface9* pCursorBitmap );
    void D3D_CALL SetCursorPosition (  int X , int Y , DWORD Flags );
    BOOL D3D_CALL ShowCursor (  BOOL bShow );
    HRESULT D3D_CALL CreateAdditionalSwapChain (  D3DPRESENT_PARAMETERS* pPresentationParameters , IDirect3DSwapChain9** pSwapChain );
    HRESULT D3D_CALL GetSwapChain (  UINT iSwapChain , IDirect3DSwapChain9** pSwapChain );
    UINT D3D_CALL GetNumberOfSwapChains ( );
    HRESULT D3D_CALL Reset (  D3DPRESENT_PARAMETERS* pPresentationParameters );
    HRESULT D3D_CALL Present (  CONST RECT* pSourceRect , CONST RECT* pDestRect , HWND hDestWindowOverride , CONST RGNDATA* pDirtyRegion );
    HRESULT D3D_CALL GetBackBuffer (  UINT iSwapChain , UINT iBackBuffer , D3DBACKBUFFER_TYPE Type , IDirect3DSurface9** ppBackBuffer );
    HRESULT D3D_CALL GetRasterStatus (  UINT iSwapChain , D3DRASTER_STATUS* pRasterStatus );
    HRESULT D3D_CALL SetDialogBoxMode (  BOOL bEnableDialogs );
    void D3D_CALL SetGammaRamp (  UINT iSwapChain , DWORD Flags , CONST D3DGAMMARAMP* pRamp );
    void D3D_CALL GetGammaRamp (  UINT iSwapChain , D3DGAMMARAMP* pRamp );
    HRESULT D3D_CALL CreateTexture (  UINT Width , UINT Height , UINT Levels , DWORD Usage , D3DFORMAT Format , D3DPOOL Pool , IDirect3DTexture9** ppTexture , HANDLE* pSharedHandle );
    HRESULT D3D_CALL CreateVolumeTexture (  UINT Width , UINT Height , UINT Depth , UINT Levels , DWORD Usage , D3DFORMAT Format , D3DPOOL Pool , IDirect3DVolumeTexture9** ppVolumeTexture , HANDLE* pSharedHandle );
    HRESULT D3D_CALL CreateCubeTexture (  UINT EdgeLength , UINT Levels , DWORD Usage , D3DFORMAT Format , D3DPOOL Pool , IDirect3DCubeTexture9** ppCubeTexture , HANDLE* pSharedHandle );
    HRESULT D3D_CALL CreateVertexBuffer (  UINT Length , DWORD Usage , DWORD FVF , D3DPOOL Pool , IDirect3DVertexBuffer9** ppVertexBuffer , HANDLE* pSharedHandle );
    HRESULT D3D_CALL CreateIndexBuffer (  UINT Length , DWORD Usage , D3DFORMAT Format , D3DPOOL Pool , IDirect3DIndexBuffer9** ppIndexBuffer , HANDLE* pSharedHandle );
    HRESULT D3D_CALL CreateRenderTarget (  UINT Width , UINT Height , D3DFORMAT Format , D3DMULTISAMPLE_TYPE MultiSample , DWORD MultisampleQuality , BOOL Lockable , IDirect3DSurface9** ppSurface , HANDLE* pSharedHandle );
    HRESULT D3D_CALL CreateDepthStencilSurface (  UINT Width , UINT Height , D3DFORMAT Format , D3DMULTISAMPLE_TYPE MultiSample , DWORD MultisampleQuality , BOOL Discard , IDirect3DSurface9** ppSurface , HANDLE* pSharedHandle );
    HRESULT D3D_CALL UpdateSurface (  IDirect3DSurface9* pSourceSurface , CONST RECT* pSourceRect , IDirect3DSurface9* pDestinationSurface , CONST POINT* pDestPoint );
    HRESULT D3D_CALL UpdateTexture (  IDirect3DBaseTexture9* pSourceTexture , IDirect3DBaseTexture9* pDestinationTexture );
    HRESULT D3D_CALL GetRenderTargetData (  IDirect3DSurface9* pRenderTarget , IDirect3DSurface9* pDestSurface );
    HRESULT D3D_CALL GetFrontBufferData (  UINT iSwapChain , IDirect3DSurface9* pDestSurface );
    HRESULT D3D_CALL StretchRect (  IDirect3DSurface9* pSourceSurface , CONST RECT* pSourceRect , IDirect3DSurface9* pDestSurface , CONST RECT* pDestRect , D3DTEXTUREFILTERTYPE Filter );
    HRESULT D3D_CALL ColorFill (  IDirect3DSurface9* pSurface , CONST RECT* pRect , D3DCOLOR color );
    HRESULT D3D_CALL CreateOffscreenPlainSurface (  UINT Width , UINT Height , D3DFORMAT Format , D3DPOOL Pool , IDirect3DSurface9** ppSurface , HANDLE* pSharedHandle );
    HRESULT D3D_CALL SetRenderTarget (  DWORD RenderTargetIndex , IDirect3DSurface9* pRenderTarget );
    HRESULT D3D_CALL GetRenderTarget (  DWORD RenderTargetIndex , IDirect3DSurface9** ppRenderTarget );
    HRESULT D3D_CALL SetDepthStencilSurface (  IDirect3DSurface9* pNewZStencil );
    HRESULT D3D_CALL GetDepthStencilSurface (  IDirect3DSurface9** ppZStencilSurface );
    HRESULT D3D_CALL BeginScene ( );
    HRESULT D3D_CALL EndScene ( );
    HRESULT D3D_CALL Clear (  DWORD Count , CONST D3DRECT* pRects , DWORD Flags , D3DCOLOR Color , float Z , DWORD Stencil );
    HRESULT D3D_CALL SetTransform (  D3DTRANSFORMSTATETYPE State , CONST D3DMATRIX* pMatrix );
    HRESULT D3D_CALL GetTransform (  D3DTRANSFORMSTATETYPE State , D3DMATRIX* pMatrix );
    HRESULT D3D_CALL MultiplyTransform (  D3DTRANSFORMSTATETYPE State , CONST D3DMATRIX* pMatrix );
    HRESULT D3D_CALL SetViewport (  CONST D3DVIEWPORT9* pViewport );
    HRESULT D3D_CALL GetViewport (  D3DVIEWPORT9* pViewport );
    HRESULT D3D_CALL SetMaterial (  CONST D3DMATERIAL9* pMaterial );
    HRESULT D3D_CALL GetMaterial (  D3DMATERIAL9* pMaterial );
    HRESULT D3D_CALL SetLight (  DWORD Index , CONST D3DLIGHT9* pLight );
    HRESULT D3D_CALL GetLight (  DWORD Index , D3DLIGHT9* pLight );
    HRESULT D3D_CALL LightEnable (  DWORD Index , BOOL Enable );
    HRESULT D3D_CALL GetLightEnable (  DWORD Index , BOOL* pEnable );
    HRESULT D3D_CALL SetClipPlane (  DWORD Index , CONST float* pPlane );
    HRESULT D3D_CALL GetClipPlane (  DWORD Index , float* pPlane );
    HRESULT D3D_CALL SetRenderState (  D3DRENDERSTATETYPE State , DWORD Value );
    HRESULT D3D_CALL GetRenderState (  D3DRENDERSTATETYPE State , DWORD* pValue );
    HRESULT D3D_CALL CreateStateBlock (  D3DSTATEBLOCKTYPE Type , IDirect3DStateBlock9** ppSB );
    HRESULT D3D_CALL BeginStateBlock ( );
    HRESULT D3D_CALL EndStateBlock (  IDirect3DStateBlock9** ppSB );
    HRESULT D3D_CALL SetClipStatus (  CONST D3DCLIPSTATUS9* pClipStatus );
    HRESULT D3D_CALL GetClipStatus (  D3DCLIPSTATUS9* pClipStatus );
    HRESULT D3D_CALL GetTexture (  DWORD Stage , IDirect3DBaseTexture9** ppTexture );
    HRESULT D3D_CALL SetTexture (  DWORD Stage , IDirect3DBaseTexture9* pTexture );
    HRESULT D3D_CALL GetTextureStageState (  DWORD Stage , D3DTEXTURESTAGESTATETYPE Type , DWORD* pValue );
    HRESULT D3D_CALL SetTextureStageState (  DWORD Stage , D3DTEXTURESTAGESTATETYPE Type , DWORD Value );
    HRESULT D3D_CALL GetSamplerState (  DWORD Sampler , D3DSAMPLERSTATETYPE Type , DWORD* pValue );
    HRESULT D3D_CALL SetSamplerState (  DWORD Sampler , D3DSAMPLERSTATETYPE Type , DWORD Value );
    HRESULT D3D_CALL ValidateDevice (  DWORD* pNumPasses );
    HRESULT D3D_CALL SetPaletteEntries (  UINT PaletteNumber , CONST PALETTEENTRY* pEntries );
    HRESULT D3D_CALL GetPaletteEntries (  UINT PaletteNumber , PALETTEENTRY* pEntries );
    HRESULT D3D_CALL SetCurrentTexturePalette (  UINT PaletteNumber );
    HRESULT D3D_CALL GetCurrentTexturePalette (  UINT * PaletteNumber );
    HRESULT D3D_CALL SetScissorRect (  CONST RECT* pRect );
    HRESULT D3D_CALL GetScissorRect (  RECT* pRect );
    HRESULT D3D_CALL SetSoftwareVertexProcessing (  BOOL bSoftware );
    BOOL D3D_CALL GetSoftwareVertexProcessing ( );
    HRESULT D3D_CALL SetNPatchMode (  float nSegments );
    float D3D_CALL GetNPatchMode ( );
    HRESULT D3D_CALL DrawPrimitive (  D3DPRIMITIVETYPE PrimitiveType , UINT StartVertex , UINT PrimitiveCount );
    HRESULT D3D_CALL DrawIndexedPrimitive (  D3DPRIMITIVETYPE Type , INT BaseVertexIndex , UINT MinVertexIndex , UINT NumVertices , UINT startIndex , UINT primCount );
    HRESULT D3D_CALL DrawPrimitiveUP (  D3DPRIMITIVETYPE PrimitiveType , UINT PrimitiveCount , CONST void* pVertexStreamZeroData , UINT VertexStreamZeroStride );
    HRESULT D3D_CALL DrawIndexedPrimitiveUP (  D3DPRIMITIVETYPE PrimitiveType , UINT MinVertexIndex , UINT NumVertices , UINT PrimitiveCount , CONST void* pIndexData , D3DFORMAT IndexDataFormat , CONST void* pVertexStreamZeroData , UINT VertexStreamZeroStride );
    HRESULT D3D_CALL ProcessVertices (  UINT SrcStartIndex , UINT DestIndex , UINT VertexCount , IDirect3DVertexBuffer9* pDestBuffer , IDirect3DVertexDeclaration9* pVertexDecl , DWORD Flags );
    HRESULT D3D_CALL CreateVertexDeclaration (  CONST D3DVERTEXELEMENT9* pVertexElements , IDirect3DVertexDeclaration9** ppDecl );
    HRESULT D3D_CALL SetVertexDeclaration (  IDirect3DVertexDeclaration9* pDecl );
    HRESULT D3D_CALL GetVertexDeclaration (  IDirect3DVertexDeclaration9** ppDecl );
    HRESULT D3D_CALL SetFVF (  DWORD FVF );
    HRESULT D3D_CALL GetFVF (  DWORD* pFVF );
    HRESULT D3D_CALL CreateVertexShader (  CONST DWORD* pFunction , IDirect3DVertexShader9** ppShader );
    HRESULT D3D_CALL SetVertexShader (  IDirect3DVertexShader9* pShader );
    HRESULT D3D_CALL GetVertexShader (  IDirect3DVertexShader9** ppShader );
    HRESULT D3D_CALL SetVertexShaderConstantF (  UINT StartRegister , CONST float* pConstantData , UINT Vector4fCount );
    HRESULT D3D_CALL GetVertexShaderConstantF (  UINT StartRegister , float* pConstantData , UINT Vector4fCount );
    HRESULT D3D_CALL SetVertexShaderConstantI (  UINT StartRegister , CONST int* pConstantData , UINT Vector4iCount );
    HRESULT D3D_CALL GetVertexShaderConstantI (  UINT StartRegister , int* pConstantData , UINT Vector4iCount );
    HRESULT D3D_CALL SetVertexShaderConstantB (  UINT StartRegister , CONST BOOL* pConstantData , UINT BoolCount );
    HRESULT D3D_CALL GetVertexShaderConstantB (  UINT StartRegister , BOOL* pConstantData , UINT BoolCount );
    HRESULT D3D_CALL SetStreamSource (  UINT StreamNumber , IDirect3DVertexBuffer9* pStreamData , UINT OffsetInBytes , UINT Stride );
    HRESULT D3D_CALL GetStreamSource (  UINT StreamNumber , IDirect3DVertexBuffer9** ppStreamData , UINT* pOffsetInBytes , UINT* pStride );
    HRESULT D3D_CALL SetStreamSourceFreq (  UINT StreamNumber , UINT Setting );
    HRESULT D3D_CALL GetStreamSourceFreq (  UINT StreamNumber , UINT* pSetting );
    HRESULT D3D_CALL SetIndices (  IDirect3DIndexBuffer9* pIndexData );
    HRESULT D3D_CALL GetIndices (  IDirect3DIndexBuffer9** ppIndexData );
    HRESULT D3D_CALL CreatePixelShader (  CONST DWORD* pFunction , IDirect3DPixelShader9** ppShader );
    HRESULT D3D_CALL SetPixelShader (  IDirect3DPixelShader9* pShader );
    HRESULT D3D_CALL GetPixelShader (  IDirect3DPixelShader9** ppShader );
    HRESULT D3D_CALL SetPixelShaderConstantF (  UINT StartRegister , CONST float* pConstantData , UINT Vector4fCount );
    HRESULT D3D_CALL GetPixelShaderConstantF (  UINT StartRegister , float* pConstantData , UINT Vector4fCount );
    HRESULT D3D_CALL SetPixelShaderConstantI (  UINT StartRegister , CONST int* pConstantData , UINT Vector4iCount );
    HRESULT D3D_CALL GetPixelShaderConstantI (  UINT StartRegister , int* pConstantData , UINT Vector4iCount );
    HRESULT D3D_CALL SetPixelShaderConstantB (  UINT StartRegister , CONST BOOL* pConstantData , UINT BoolCount );
    HRESULT D3D_CALL GetPixelShaderConstantB (  UINT StartRegister , BOOL* pConstantData , UINT BoolCount );
    HRESULT D3D_CALL DrawRectPatch (  UINT Handle , CONST float* pNumSegs , CONST D3DRECTPATCH_INFO* pRectPatchInfo );
    HRESULT D3D_CALL DrawTriPatch (  UINT Handle , CONST float* pNumSegs , CONST D3DTRIPATCH_INFO* pTriPatchInfo );
    HRESULT D3D_CALL DeletePatch (  UINT Handle );
    HRESULT D3D_CALL CreateQuery (  D3DQUERYTYPE Type , IDirect3DQuery9** ppQuery );

};

#endif

