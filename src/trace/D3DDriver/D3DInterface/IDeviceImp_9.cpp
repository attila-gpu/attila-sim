#include "Common.h"
#include "IVolumeTextureImp_9.h"
#include "ICubeTextureImp_9.h"
#include "IIndexBufferImp_9.h"
#include "IStateBlockImp_9.h"
#include "IBaseTextureImp_9.h"
#include "IVertexBufferImp_9.h"
#include "IVertexDeclarationImp_9.h"
#include "IVertexShaderImp_9.h"
#include "IPixelShaderImp_9.h"
#include "IQueryImp_9.h"
#include "IDirect3DImp_9.h"
#include "ISwapChainImp_9.h"
#include "ISurfaceImp_9.h"
#include "ITextureImp_9.h"
#include "IDeviceImp_9.h"

IDeviceImp9 :: IDeviceImp9() {}

IDeviceImp9 & IDeviceImp9 :: getInstance() {
    static IDeviceImp9 instance;
    return instance;
}

IDeviceImp9 :: IDeviceImp9(StateDataNode *s_parent, IDirect3DImp9* _i_parent, D3DPRESENT_PARAMETERS * pp) :
i_parent(_i_parent)
{
    // Create default state
    state = D3DState::create_device_state_9(this);

    // Add streams
    ///@note
    UINT stream_count = MAX_D3D_STREAMS;
    state->get_child(StateId(NAME_STREAM_COUNT))->write_data(&stream_count);
    for(UINT i = 0; i < MAX_D3D_STREAMS; i++) {
        state->add_child(D3DState::create_stream_state_9(i));
    }

    // Add samplers
    UINT sampler_count = MAX_D3D_SAMPLERS;
    state->get_child(StateId(NAME_SAMPLER_COUNT))->write_data(&sampler_count);
    for(UINT i = 0; i < MAX_D3D_SAMPLERS; i++) {
        state->add_child(D3DState::create_sampler_state_9(i));
    }
    // Special samplers for vertex textures
    /// @note They are not operative, this is only a patch
    /// @todo Make vertex texture samplers operative
    state->add_child(D3DState::create_sampler_state_9(D3DVERTEXTEXTURESAMPLER0));
    state->add_child(D3DState::create_sampler_state_9(D3DVERTEXTEXTURESAMPLER1));
    state->add_child(D3DState::create_sampler_state_9(D3DVERTEXTEXTURESAMPLER2));
    state->add_child(D3DState::create_sampler_state_9(D3DVERTEXTEXTURESAMPLER3));

    // Initialice device state before adding to state data tree.
    // This state MUST MATCH THE DEFAULT VALUES ON GPU setted by the
    // device controller.



    // Create implicit render target and zstencil surfaces
    /**@note Implicit render target and zstencil surfaces are handled differently.
             GPU memory for them is already reserved by GPUDriver and assigned to
             GPU_FRONTBUFFERADDR, GPU_BACKBUFFERADDR and GPU_ZSTENCILADDR.
             To avoid that a controller handles them as regular surfaces they are
             added to device state BEFORE adding device state itself to D3DState tree.
             This ensures no "child_added" event is fired when the surfaces add their state
             to device state. Ok, the event is fired but no one is listening at it. */
    /**@note As they are allocated with other method, is important to ensure reference
            counting never reaches zero for this surfaces. If that happens the surface controller
            will try to deallocate them. To avoid that a reference is added. This reference
            is not decremented, so gpu memory for implicit RT/ZStencil is deallocated
            when simulation ends. */
    ISurfaceImp9* implicit_render_target = new ISurfaceImp9(state, this,
            pp->BackBufferWidth, pp->BackBufferHeight, D3DUSAGE_RENDERTARGET,
            pp->BackBufferFormat, D3DPOOL_DEFAULT);
    implicit_render_target->AddRef();
    i_surface_childs.insert(implicit_render_target);

    ISurfaceImp9* implicit_depthstencil = new ISurfaceImp9(state, this,
            pp->BackBufferWidth, pp->BackBufferHeight, D3DUSAGE_DEPTHSTENCIL,
            pp->AutoDepthStencilFormat, D3DPOOL_DEFAULT);
    implicit_depthstencil->AddRef();
    i_surface_childs.insert(implicit_depthstencil);

    state->get_child(StateId(NAME_IMPLICIT_RENDER_TARGET))->write_data(&implicit_render_target);
    state->get_child(StateId(NAME_CURRENT_RENDER_TARGET))->write_data(&implicit_render_target);
    state->get_child(StateId(NAME_IMPLICIT_DEPTHSTENCIL))->write_data(&implicit_depthstencil);
    state->get_child(StateId(NAME_CURRENT_DEPTHSTENCIL))->write_data(&implicit_depthstencil);

    state->get_child(StateId(NAME_BACKBUFFER_WIDTH))->write_data(&pp->BackBufferWidth);
    state->get_child(StateId(NAME_BACKBUFFER_HEIGHT))->write_data(&pp->BackBufferHeight);
    state->get_child(StateId(NAME_BACKBUFFER_FORMAT))->write_data(&pp->BackBufferFormat);

    if(pp->EnableAutoDepthStencil) D3D_DEBUG( D3D_DEBUG( cout << "WARNING: No auto depth stencil" << endl; ) )

    state->get_child(StateId(NAME_DEPTHSTENCIL_FORMAT))->write_data(&pp->AutoDepthStencilFormat);


    DWORD dw_value;
    INT int_value;
    dw_value = D3DCULL_CCW;
    state->get_child(StateId(NAME_CULL_MODE))->write_data(&dw_value);

	dw_value = FALSE;
    state->get_child(StateId(NAME_ALPHA_TEST_ENABLED))->write_data(&dw_value);
	dw_value = D3DCMP_ALWAYS;
    state->get_child(StateId(NAME_ALPHA_FUNC))->write_data(&dw_value);
	dw_value = 0;
    state->get_child(StateId(NAME_ALPHA_REF))->write_data(&dw_value);

    dw_value = D3DZB_TRUE; /// @note Default is true only if EnableAutoDepthStencil is true, see below
    state->get_child(StateId(NAME_Z_ENABLED))->write_data(&dw_value);
    dw_value = D3DCMP_LESSEQUAL;
    state->get_child(StateId(NAME_Z_FUNCTION))->write_data(&dw_value);
    dw_value = TRUE;
    state->get_child(StateId(NAME_Z_WRITE_ENABLE))->write_data(&dw_value);

    dw_value = FALSE;
    state->get_child(StateId(NAME_ALPHA_BLEND_ENABLED))->write_data(&dw_value);
    dw_value = FALSE;
    state->get_child(StateId(NAME_SEPARATE_ALPHA_BLEND_ENABLED))->write_data(&dw_value);
    dw_value = 0xFFFFFFFF;
    state->get_child(StateId(NAME_BLEND_FACTOR))->write_data(&dw_value);
    dw_value = D3DBLEND_ONE;
    state->get_child(StateId(NAME_SRC_BLEND))->write_data(&dw_value);
    dw_value = D3DBLEND_ZERO;
    state->get_child(StateId(NAME_DST_BLEND))->write_data(&dw_value);
    dw_value = D3DBLEND_ONE;
    state->get_child(StateId(NAME_SRC_BLEND_ALPHA))->write_data(&dw_value);
    dw_value = D3DBLEND_ZERO;
    state->get_child(StateId(NAME_DST_BLEND_ALPHA))->write_data(&dw_value);
    dw_value = D3DBLENDOP_ADD;
    state->get_child(StateId(NAME_BLEND_OP))->write_data(&dw_value);
    dw_value = D3DBLENDOP_ADD;
    state->get_child(StateId(NAME_BLEND_OP_ALPHA))->write_data(&dw_value);

    dw_value = FALSE;
    state->get_child(StateId(NAME_STENCIL_ENABLED))->write_data(&dw_value);
    int_value = 0;
    state->get_child(StateId(NAME_STENCIL_REF))->write_data(&int_value);
    dw_value = D3DCMP_ALWAYS;
    state->get_child(StateId(NAME_STENCIL_FUNC))->write_data(&dw_value);
    dw_value = D3DSTENCILOP_KEEP;
    state->get_child(StateId(NAME_STENCIL_FAIL))->write_data(&dw_value);
    dw_value = D3DSTENCILOP_KEEP;
    state->get_child(StateId(NAME_STENCIL_ZFAIL))->write_data(&dw_value);
    dw_value = D3DSTENCILOP_KEEP;
    state->get_child(StateId(NAME_STENCIL_PASS))->write_data(&dw_value);
    dw_value = 0xFFFFFFFF;
    state->get_child(StateId(NAME_STENCIL_MASK))->write_data(&dw_value);
    dw_value = 0xFFFFFFFF;
    state->get_child(StateId(NAME_STENCIL_WRITE_MASK))->write_data(&dw_value);

    for(int i = 0; i < MAX_D3D_SAMPLERS; i ++) {
        StateDataNode* s_samp = state->get_child(StateId(NAME_SAMPLER, StateIndex(i)));
        DWORD addr;
        addr = D3DTADDRESS_WRAP;
        s_samp->get_child(StateId(NAME_ADDRESSU))->write_data(&addr);
        s_samp->get_child(StateId(NAME_ADDRESSV))->write_data(&addr);
        s_samp->get_child(StateId(NAME_ADDRESSW))->write_data(&addr);
        D3DTEXTUREFILTERTYPE filter;
        filter = D3DTEXF_POINT;
        s_samp->get_child(StateId(NAME_MIN_FILTER))->write_data(&filter);
        filter = D3DTEXF_NONE;
        s_samp->get_child(StateId(NAME_MIP_FILTER))->write_data(&filter);
        filter = D3DTEXF_POINT;
        s_samp->get_child(StateId(NAME_MAG_FILTER))->write_data(&filter);
        DWORD aniso;
        aniso = 1;
        s_samp->get_child(StateId(NAME_MAX_ANISOTROPY))->write_data(&aniso);

    }


    // Add state to state data tree
    s_parent->add_child(state);

    /**@note This is a patch for disabling ZTest (firing an event) when no autodepthstencil has to be created,
             that is the specified behavior for D3D. */
    /**@todo Actually a zstencil buffer IS created when no autodepthstencil is needed. Find
             the way to not create implicit zstencil. */
    dw_value = pp->EnableAutoDepthStencil ? D3DZB_TRUE : D3DZB_FALSE;
    state->get_child(StateId(NAME_Z_ENABLED))->write_data(&dw_value);

}

bool IDeviceImp9::using_implicit_render_target_zstencil() {
    IDirect3DSurface9* implicit_rt;
    state->get_child(StateId(NAME_IMPLICIT_RENDER_TARGET))->read_data(&implicit_rt);
    IDirect3DSurface9* implicit_zs;
    state->get_child(StateId(NAME_IMPLICIT_DEPTHSTENCIL))->read_data(&implicit_zs);
    IDirect3DSurface9* current_rt;
    state->get_child(StateId(NAME_CURRENT_RENDER_TARGET))->read_data(&current_rt);
    IDirect3DSurface9* current_zs;
    state->get_child(StateId(NAME_CURRENT_DEPTHSTENCIL))->read_data(&current_zs);
    return (implicit_rt == current_rt) && (current_zs == implicit_zs);


}


IDeviceImp9 :: ~IDeviceImp9() {
 /// @todo delete state
    set< IVertexBufferImp9 * > :: iterator it_vb;
    for(it_vb =  i_vertex_buffer_childs.begin(); it_vb != i_vertex_buffer_childs.end(); it_vb ++)
        delete (*it_vb);

    set< IVertexDeclarationImp9 * > :: iterator it_vd;
    for(it_vd =  i_vertex_declaration_childs.begin(); it_vd != i_vertex_declaration_childs.end(); it_vd ++)
        delete (*it_vd);

    set< IIndexBufferImp9 * > :: iterator it_ib;
    for(it_ib =  i_index_buffer_childs.begin(); it_ib != i_index_buffer_childs.end(); it_ib ++)
        delete (*it_ib);

    set< IVertexShaderImp9 * > :: iterator it_vsh;
    for(it_vsh =  i_vertex_shader_childs.begin(); it_vsh != i_vertex_shader_childs.end(); it_vsh ++)
        delete (*it_vsh);

    set< IPixelShaderImp9 * > :: iterator it_psh;
    for(it_psh =  i_pixel_shader_childs.begin(); it_psh != i_pixel_shader_childs.end(); it_psh ++)
        delete (*it_psh);

    set< ITextureImp9 * > :: iterator it_tex;
    for(it_tex =  i_texture_childs.begin(); it_tex != i_texture_childs.end(); it_tex ++)
        delete (*it_tex);

    set< ICubeTextureImp9 * > :: iterator it_cubetex;
    for(it_cubetex =  i_cubetexture_childs.begin(); it_cubetex != i_cubetexture_childs.end(); it_cubetex ++)
        delete (*it_cubetex);

    set< IVolumeTextureImp9 * > :: iterator it_volumetex;
    for(it_volumetex =  i_volumetexture_childs.begin(); it_volumetex != i_volumetexture_childs.end(); it_volumetex ++)
        delete (*it_volumetex);

    set< ISurfaceImp9 * > :: iterator it_surf;
    for(it_surf =  i_surface_childs.begin(); it_surf != i_surface_childs.end(); it_surf ++)
        delete (*it_surf);
}

HRESULT D3D_CALL IDeviceImp9 :: QueryInterface (  REFIID riid , void** ppvObj ) {
    * ppvObj = cover_buffer_9;
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: QueryInterface  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL IDeviceImp9 :: AddRef ( ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: AddRef  NOT IMPLEMENTED" << endl; ) )
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

ULONG D3D_CALL IDeviceImp9 :: Release ( ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: Release  NOT IMPLEMENTED" << endl; ) )
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: TestCooperativeLevel ( ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: TestCooperativeLevel  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

UINT D3D_CALL IDeviceImp9 :: GetAvailableTextureMem ( ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetAvailableTextureMem  NOT IMPLEMENTED" << endl; ) )
    UINT ret = static_cast< UINT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: EvictManagedResources ( ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: EvictManagedResources  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetDirect3D (  IDirect3D9** ppD3D9 ) {
    * ppD3D9 = & IDirect3DImp9 :: getInstance();
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetDirect3D  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetDeviceCaps (  D3DCAPS9* pCaps ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetDeviceCaps  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetDisplayMode (  UINT iSwapChain , D3DDISPLAYMODE* pMode ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetDisplayMode  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetCreationParameters (  D3DDEVICE_CREATION_PARAMETERS * pParameters ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetCreationParameters  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: SetCursorProperties (  UINT XHotSpot , UINT YHotSpot , IDirect3DSurface9* pCursorBitmap ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: SetCursorProperties  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

void D3D_CALL IDeviceImp9 :: SetCursorPosition (  int X , int Y , DWORD Flags ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: SetCursorPosition  NOT IMPLEMENTED" << endl; ) )
}

BOOL D3D_CALL IDeviceImp9 :: ShowCursor (  BOOL bShow ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: ShowCursor  NOT IMPLEMENTED" << endl; ) )
    BOOL ret = static_cast< BOOL >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: CreateAdditionalSwapChain (  D3DPRESENT_PARAMETERS* pPresentationParameters , IDirect3DSwapChain9** pSwapChain ) {
    * pSwapChain = & ISwapChainImp9 :: getInstance();
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: CreateAdditionalSwapChain  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetSwapChain (  UINT iSwapChain , IDirect3DSwapChain9** pSwapChain ) {
    * pSwapChain = & ISwapChainImp9 :: getInstance();
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetSwapChain  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

UINT D3D_CALL IDeviceImp9 :: GetNumberOfSwapChains ( ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetNumberOfSwapChains  NOT IMPLEMENTED" << endl; ) )
    UINT ret = static_cast< UINT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: Reset (  D3DPRESENT_PARAMETERS* pPresentationParameters ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: Reset  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: Present (  CONST RECT* pSourceRect , CONST RECT* pDestRect , HWND hDestWindowOverride , CONST RGNDATA* pDirtyRegion ) {
    D3D_DEBUG( D3D_DEBUG( cout << "IDEVICE9<" << this << ">: Present" << endl; ) )

    StateDataNode* present_state = D3DState::create_present_state_9();
    state->get_child(StateId(NAME_COMMANDS))->add_child(present_state);
    state->get_child(StateId(NAME_COMMANDS))->remove_child(present_state);
    /// @todo Delete present state
    return D3D_OK;

}

HRESULT D3D_CALL IDeviceImp9 :: GetBackBuffer (  UINT iSwapChain , UINT iBackBuffer , D3DBACKBUFFER_TYPE Type , IDirect3DSurface9** ppBackBuffer ) {
    * ppBackBuffer = & ISurfaceImp9 :: getInstance();
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetBackBuffer  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetRasterStatus (  UINT iSwapChain , D3DRASTER_STATUS* pRasterStatus ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetRasterStatus  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: SetDialogBoxMode (  BOOL bEnableDialogs ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: SetDialogBoxMode  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

void D3D_CALL IDeviceImp9 :: SetGammaRamp (  UINT iSwapChain , DWORD Flags , CONST D3DGAMMARAMP* pRamp ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: SetGammaRamp  NOT IMPLEMENTED" << endl; ) )
}

void D3D_CALL IDeviceImp9 :: GetGammaRamp (  UINT iSwapChain , D3DGAMMARAMP* pRamp ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetGammaRamp  NOT IMPLEMENTED" << endl; ) )
}

HRESULT D3D_CALL IDeviceImp9 :: CreateTexture (  UINT Width , UINT Height , UINT Levels , DWORD Usage , D3DFORMAT Format , D3DPOOL Pool , IDirect3DTexture9** ppTexture , HANDLE* pSharedHandle ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9: CreateTexture" << endl; ) )
    ITextureImp9* tex = new ITextureImp9(state, this, Width, Height, Levels, Usage, Format, Pool);
    tex->AddRef();
    i_texture_childs.insert(tex);
    * ppTexture = tex;
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: CreateVolumeTexture (  UINT Width , UINT Height , UINT Depth , UINT Levels , DWORD Usage , D3DFORMAT Format , D3DPOOL Pool , IDirect3DVolumeTexture9** ppVolumeTexture , HANDLE* pSharedHandle ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9: CreateVolumeTexture " << endl; ) )
    IVolumeTextureImp9* vol_tex = new IVolumeTextureImp9(state, this, Width, Height, Depth, Levels, Usage, Format, Pool);
    i_volumetexture_childs.insert(vol_tex);
    * ppVolumeTexture = vol_tex;
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: CreateCubeTexture (  UINT EdgeLength , UINT Levels , DWORD Usage , D3DFORMAT Format , D3DPOOL Pool , IDirect3DCubeTexture9** ppCubeTexture , HANDLE* pSharedHandle ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9: CreateCubeTexture" << endl; ) )
    ICubeTextureImp9* tex = new ICubeTextureImp9(state, this, EdgeLength, Levels, Usage, Format, Pool);
    tex->AddRef();
    i_cubetexture_childs.insert(tex);
    * ppCubeTexture = tex;
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: CreateVertexBuffer (  UINT Length , DWORD Usage , DWORD FVF , D3DPOOL Pool , IDirect3DVertexBuffer9** ppVertexBuffer , HANDLE* pSharedHandle ) {
    D3D_DEBUG( cout <<"IDEVICE9:  CreateVertexBuffer" << endl; )
    IVertexBufferImp9* vb = new IVertexBufferImp9(state, this, Length, Usage, FVF, Pool);
    vb->AddRef();
    i_vertex_buffer_childs.insert(vb);
    * ppVertexBuffer = vb;
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: CreateIndexBuffer (  UINT Length , DWORD Usage , D3DFORMAT Format , D3DPOOL Pool , IDirect3DIndexBuffer9** ppIndexBuffer , HANDLE* pSharedHandle ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9:  CreateIndexBuffer" << endl; ) )
    IIndexBufferImp9* ib = new IIndexBufferImp9(state, this, Length, Usage, Format, Pool);
    ib->AddRef();
    i_index_buffer_childs.insert(ib);
    * ppIndexBuffer = ib;
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: CreateRenderTarget (  UINT Width , UINT Height , D3DFORMAT Format , D3DMULTISAMPLE_TYPE MultiSample , DWORD MultisampleQuality , BOOL Lockable , IDirect3DSurface9** ppSurface , HANDLE* pSharedHandle ) {
    * ppSurface = & ISurfaceImp9 :: getInstance();
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: CreateRenderTarget  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: CreateDepthStencilSurface (  UINT Width , UINT Height , D3DFORMAT Format , D3DMULTISAMPLE_TYPE MultiSample , DWORD MultisampleQuality , BOOL Discard , IDirect3DSurface9** ppSurface , HANDLE* pSharedHandle ) {
    * ppSurface = & ISurfaceImp9 :: getInstance();
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: CreateDepthStencilSurface  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: UpdateSurface (  IDirect3DSurface9* pSourceSurface , CONST RECT* pSourceRect , IDirect3DSurface9* pDestinationSurface , CONST POINT* pDestPoint ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: UpdateSurface  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: UpdateTexture (  IDirect3DBaseTexture9* pSourceTexture , IDirect3DBaseTexture9* pDestinationTexture ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: UpdateTexture  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetRenderTargetData (  IDirect3DSurface9* pRenderTarget , IDirect3DSurface9* pDestSurface ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetRenderTargetData  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetFrontBufferData (  UINT iSwapChain , IDirect3DSurface9* pDestSurface ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetFrontBufferData  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: StretchRect (  IDirect3DSurface9* pSourceSurface , CONST RECT* pSourceRect , IDirect3DSurface9* pDestSurface , CONST RECT* pDestRect , D3DTEXTUREFILTERTYPE Filter ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: StretchRect  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: ColorFill (  IDirect3DSurface9* pSurface , CONST RECT* pRect , D3DCOLOR color ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: ColorFill  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: CreateOffscreenPlainSurface (  UINT Width , UINT Height , D3DFORMAT Format , D3DPOOL Pool , IDirect3DSurface9** ppSurface , HANDLE* pSharedHandle ) {
    * ppSurface = & ISurfaceImp9 :: getInstance();
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: CreateOffscreenPlainSurface  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: SetRenderTarget (  DWORD RenderTargetIndex , IDirect3DSurface9* pRenderTarget ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9: SetRenderTarget" << endl; ) )
    ///@todo check index is always 0
    state->get_child(NAME_CURRENT_RENDER_TARGET)->write_data(&pRenderTarget);
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: GetRenderTarget (  DWORD RenderTargetIndex , IDirect3DSurface9** ppRenderTarget ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9: GetRenderTarget" << endl; ) )
    state->get_child(NAME_CURRENT_RENDER_TARGET)->read_data(ppRenderTarget);
    if((*ppRenderTarget) != 0 )
        (*ppRenderTarget)->AddRef();
    ///@todo check index is always 0
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: SetDepthStencilSurface (  IDirect3DSurface9* pNewZStencil ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9: SetDepthStencilSurface" << endl; ) )
    state->get_child(NAME_CURRENT_DEPTHSTENCIL)->write_data(&pNewZStencil);
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: GetDepthStencilSurface (  IDirect3DSurface9** ppZStencilSurface ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9: GetDepthStencilSurface" << endl; ) )
    state->get_child(NAME_CURRENT_DEPTHSTENCIL)->read_data(ppZStencilSurface);
    if((*ppZStencilSurface) != 0) (*ppZStencilSurface)->AddRef();
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: BeginScene ( ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: BeginScene  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: EndScene ( ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: EndScene  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: Clear (  DWORD Count , CONST D3DRECT* pRects , DWORD Flags , D3DCOLOR Color , float Z , DWORD Stencil ) {
    D3D_DEBUG( D3D_DEBUG( cout << "IDEVICE9<" << this << ">: Clear" << endl; ) )


    /// @todo Show a warning or panic if rects are used
    StateDataNode* clear_state = D3DState::create_clear_state_9();

    clear_state->get_child(StateId(NAME_FLAGS))->write_data(&Flags);
    clear_state->get_child(StateId(NAME_COLOR))->write_data(&Color);
    clear_state->get_child(StateId(NAME_Z))->write_data(&Z);
    clear_state->get_child(StateId(NAME_STENCIL))->write_data(&Stencil);

    state->get_child(StateId(NAME_COMMANDS))->add_child(clear_state);
    state->get_child(StateId(NAME_COMMANDS))->remove_child(clear_state);
    /// @todo Delete clear state
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: SetTransform (  D3DTRANSFORMSTATETYPE State , CONST D3DMATRIX* pMatrix ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: SetTransform  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetTransform (  D3DTRANSFORMSTATETYPE State , D3DMATRIX* pMatrix ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetTransform  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: MultiplyTransform (  D3DTRANSFORMSTATETYPE State , CONST D3DMATRIX* pMatrix ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: MultiplyTransform  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: SetViewport (  CONST D3DVIEWPORT9* pViewport ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: SetViewport  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetViewport (  D3DVIEWPORT9* pViewport ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetViewport  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: SetMaterial (  CONST D3DMATERIAL9* pMaterial ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: SetMaterial  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetMaterial (  D3DMATERIAL9* pMaterial ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetMaterial  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: SetLight (  DWORD Index , CONST D3DLIGHT9* pLight ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: SetLight  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetLight (  DWORD Index , D3DLIGHT9* pLight ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetLight  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: LightEnable (  DWORD Index , BOOL Enable ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: LightEnable  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetLightEnable (  DWORD Index , BOOL* pEnable ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetLightEnable  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: SetClipPlane (  DWORD Index , CONST float* pPlane ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: SetClipPlane  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetClipPlane (  DWORD Index , float* pPlane ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetClipPlane  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: SetRenderState (  D3DRENDERSTATETYPE State , DWORD Value ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9: SetRenderState" << endl; ) )


    switch(State) {
        case D3DRS_CULLMODE:
            state->get_child(StateId(NAME_CULL_MODE))->write_data(&Value);
            break;
		case D3DRS_ALPHATESTENABLE:
            state->get_child(StateId(NAME_ALPHA_TEST_ENABLED))->write_data(&Value);
            break;
		case D3DRS_ALPHAFUNC:
            state->get_child(StateId(NAME_ALPHA_FUNC))->write_data(&Value);
            break;
		case D3DRS_ALPHAREF:
            state->get_child(StateId(NAME_ALPHA_REF))->write_data(&Value);
            break;
        case D3DRS_ZENABLE:
            state->get_child(StateId(NAME_Z_ENABLED))->write_data(&Value);
            break;
        case D3DRS_ZFUNC:
            state->get_child(StateId(NAME_Z_FUNCTION))->write_data(&Value);
            break;
        case D3DRS_ZWRITEENABLE:
            state->get_child(StateId(NAME_Z_WRITE_ENABLE))->write_data(&Value);
            break;
        case D3DRS_ALPHABLENDENABLE:
            state->get_child(StateId(NAME_ALPHA_BLEND_ENABLED))->write_data(&Value);
            break;
        case D3DRS_BLENDFACTOR:
            state->get_child(StateId(NAME_BLEND_FACTOR))->write_data(&Value);
            break;
        case D3DRS_SRCBLEND:
            state->get_child(StateId(NAME_SRC_BLEND))->write_data(&Value);
            break;
        case D3DRS_DESTBLEND:
            state->get_child(StateId(NAME_DST_BLEND))->write_data(&Value);
            break;
        case D3DRS_BLENDOP:
            state->get_child(StateId(NAME_BLEND_OP))->write_data(&Value);
            break;
        case D3DRS_SEPARATEALPHABLENDENABLE:
            state->get_child(StateId(NAME_SEPARATE_ALPHA_BLEND_ENABLED))->write_data(&Value);
            break;
        case D3DRS_SRCBLENDALPHA:
            state->get_child(StateId(NAME_SRC_BLEND_ALPHA))->write_data(&Value);
            break;
        case D3DRS_DESTBLENDALPHA:
            state->get_child(StateId(NAME_DST_BLEND_ALPHA))->write_data(&Value);
            break;
        case D3DRS_BLENDOPALPHA:
            state->get_child(StateId(NAME_BLEND_OP_ALPHA))->write_data(&Value);
            break;
        case D3DRS_STENCILENABLE:
            state->get_child(StateId(NAME_STENCIL_ENABLED))->write_data(&Value);
            break;
        case D3DRS_STENCILREF:
            state->get_child(StateId(NAME_STENCIL_REF))->write_data(&Value);
            break;
        case D3DRS_STENCILFUNC:
            state->get_child(StateId(NAME_STENCIL_FUNC))->write_data(&Value);
            break;
        case D3DRS_STENCILFAIL:
            state->get_child(StateId(NAME_STENCIL_FAIL))->write_data(&Value);
            break;
        case D3DRS_STENCILZFAIL:
            state->get_child(StateId(NAME_STENCIL_ZFAIL))->write_data(&Value);
            break;
        case D3DRS_STENCILPASS:
            state->get_child(StateId(NAME_STENCIL_PASS))->write_data(&Value);
            break;
        case D3DRS_STENCILMASK:
            state->get_child(StateId(NAME_STENCIL_MASK))->write_data(&Value);
            break;
        case D3DRS_STENCILWRITEMASK:
            state->get_child(StateId(NAME_STENCIL_WRITE_MASK))->write_data(&Value);
            break;
        default:
            D3D_DEBUG( D3D_DEBUG( cout << "IDEVICE9: WARNING: " << renderstate2string(State) << ", value " << (int)Value << " is not supported." << endl; ) )
            break;
    }
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: GetRenderState (  D3DRENDERSTATETYPE State , DWORD* pValue ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetRenderState  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: CreateStateBlock (  D3DSTATEBLOCKTYPE Type , IDirect3DStateBlock9** ppSB ) {
    * ppSB = & IStateBlockImp9 :: getInstance();
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: CreateStateBlock  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: BeginStateBlock ( ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: BeginStateBlock  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: EndStateBlock (  IDirect3DStateBlock9** ppSB ) {
    * ppSB = & IStateBlockImp9 :: getInstance();
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: EndStateBlock  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: SetClipStatus (  CONST D3DCLIPSTATUS9* pClipStatus ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: SetClipStatus  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetClipStatus (  D3DCLIPSTATUS9* pClipStatus ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetClipStatus  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetTexture (  DWORD Stage , IDirect3DBaseTexture9** ppTexture ) {
    D3D_DEBUG( cout <<"IDEVICE9: GetTexture" << endl; )
    StateDataNode* s_sampler = state->get_child(StateId(NAME_SAMPLER, StateIndex(Stage)));
    s_sampler->get_child(StateId(NAME_TEXTURE))->read_data(ppTexture);
    if(*ppTexture != 0) (*ppTexture)->AddRef();
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: SetTexture (  DWORD Stage , IDirect3DBaseTexture9* pTexture ) {
    D3D_DEBUG( cout <<"IDEVICE9: SetTexture" << endl; )
    StateDataNode* s_sampler = state->get_child(StateId(NAME_SAMPLER, StateIndex(Stage)));
    s_sampler->get_child(StateId(NAME_TEXTURE))->write_data(&pTexture);
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: GetTextureStageState (  DWORD Stage , D3DTEXTURESTAGESTATETYPE Type , DWORD* pValue ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetTextureStageState  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: SetTextureStageState (  DWORD Stage , D3DTEXTURESTAGESTATETYPE Type , DWORD Value ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: SetTextureStageState  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetSamplerState (  DWORD Sampler , D3DSAMPLERSTATETYPE Type , DWORD* pValue ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetSamplerState  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: SetSamplerState (  DWORD Sampler , D3DSAMPLERSTATETYPE Type , DWORD Value ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9: SetSamplerState" << endl; ) )
    StateDataNode* s_samp = state->get_child(StateId(NAME_SAMPLER, StateIndex(Sampler)));
    switch(Type) {
      case D3DSAMP_ADDRESSU:
        s_samp->get_child(NAME_ADDRESSU)->write_data(&Value);
        break;
      case D3DSAMP_ADDRESSV:
        s_samp->get_child(NAME_ADDRESSV)->write_data(&Value);
        break;
      case D3DSAMP_MINFILTER:
        s_samp->get_child(NAME_MIN_FILTER)->write_data(&Value);
        break;
      case D3DSAMP_MIPFILTER:
        s_samp->get_child(NAME_MIP_FILTER)->write_data(&Value);
        break;
      case D3DSAMP_MAGFILTER:
        s_samp->get_child(NAME_MAG_FILTER)->write_data(&Value);
        break;
      case D3DSAMP_MAXANISOTROPY:
        s_samp->get_child(NAME_MAX_ANISOTROPY)->write_data(&Value);
        break;
      default:
        D3D_DEBUG( D3D_DEBUG( cout << "IDEVICE9: WARNING: Sampler state " << samplerstate2string(Type) << ", value " << (int)Value << " not supported." << endl; ) )

    }
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: ValidateDevice (  DWORD* pNumPasses ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: ValidateDevice  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: SetPaletteEntries (  UINT PaletteNumber , CONST PALETTEENTRY* pEntries ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: SetPaletteEntries  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetPaletteEntries (  UINT PaletteNumber , PALETTEENTRY* pEntries ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetPaletteEntries  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: SetCurrentTexturePalette (  UINT PaletteNumber ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: SetCurrentTexturePalette  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetCurrentTexturePalette (  UINT * PaletteNumber ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetCurrentTexturePalette  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: SetScissorRect (  CONST RECT* pRect ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: SetScissorRect  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetScissorRect (  RECT* pRect ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetScissorRect  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: SetSoftwareVertexProcessing (  BOOL bSoftware ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: SetSoftwareVertexProcessing  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

BOOL D3D_CALL IDeviceImp9 :: GetSoftwareVertexProcessing ( ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetSoftwareVertexProcessing  NOT IMPLEMENTED" << endl; ) )
    BOOL ret = static_cast< BOOL >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: SetNPatchMode (  float nSegments ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: SetNPatchMode  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

float D3D_CALL IDeviceImp9 :: GetNPatchMode ( ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetNPatchMode  NOT IMPLEMENTED" << endl; ) )
    float ret = static_cast< float >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: DrawPrimitive (  D3DPRIMITIVETYPE PrimitiveType , UINT StartVertex , UINT PrimitiveCount ) {
    D3D_DEBUG( D3D_DEBUG( cout << "IDEVICE9<" << this << ">: DrawPrimitive" << endl; ) )


   StateDataNode* draw_state = D3DState::create_draw_primitive_state_9();

    draw_state->get_child(StateId(NAME_PRIMITIVE_MODE))->write_data(&PrimitiveType);
    draw_state->get_child(StateId(NAME_START_VERTEX))->write_data(&StartVertex);
    draw_state->get_child(StateId(NAME_PRIMITIVE_COUNT))->write_data(&PrimitiveCount);

    state->get_child(StateId(NAME_COMMANDS))->add_child(draw_state);
    state->get_child(StateId(NAME_COMMANDS))->remove_child(draw_state);
    /// @todo Delete draw state
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: DrawIndexedPrimitive (  D3DPRIMITIVETYPE Type , INT BaseVertexIndex , UINT MinVertexIndex , UINT NumVertices , UINT startIndex , UINT primCount ) {

    D3D_DEBUG( D3D_DEBUG( cout << "IDEVICE9<" << this << ">: DrawIndexedPrimitive" << endl; ) )


    StateDataNode* draw_state = D3DState::create_draw_indexed_primitive_state_9();

    if(BaseVertexIndex != 0)
        D3D_DEBUG( D3D_DEBUG( cout << "WARNING: DrawIndexedPrimitive: Using not zero base vertex index." << endl; ) )
    /// @note Min vertex and num vertexes are just hints to the d3d runtime.
    draw_state->get_child(StateId(NAME_PRIMITIVE_MODE))->write_data(&Type);
    draw_state->get_child(StateId(NAME_START_INDEX))->write_data(&startIndex);
    draw_state->get_child(StateId(NAME_PRIMITIVE_COUNT))->write_data(&primCount);


    state->get_child(StateId(NAME_COMMANDS))->add_child(draw_state);
    state->get_child(StateId(NAME_COMMANDS))->remove_child(draw_state);
    /// @todo Delete draw state
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: DrawPrimitiveUP (  D3DPRIMITIVETYPE PrimitiveType , UINT PrimitiveCount , CONST void* pVertexStreamZeroData , UINT VertexStreamZeroStride ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: DrawPrimitiveUP  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: DrawIndexedPrimitiveUP (  D3DPRIMITIVETYPE PrimitiveType , UINT MinVertexIndex , UINT NumVertices , UINT PrimitiveCount , CONST void* pIndexData , D3DFORMAT IndexDataFormat , CONST void* pVertexStreamZeroData , UINT VertexStreamZeroStride ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: DrawIndexedPrimitiveUP  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: ProcessVertices (  UINT SrcStartIndex , UINT DestIndex , UINT VertexCount , IDirect3DVertexBuffer9* pDestBuffer , IDirect3DVertexDeclaration9* pVertexDecl , DWORD Flags ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: ProcessVertices  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: CreateVertexDeclaration (  CONST D3DVERTEXELEMENT9* pVertexElements , IDirect3DVertexDeclaration9** ppDecl ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9:  CreateVertexDeclaration" << endl; ) )

    IVertexDeclarationImp9* vd = new IVertexDeclarationImp9(state, this, pVertexElements);
    i_vertex_declaration_childs.insert(vd);
    * ppDecl = vd;
    return D3D_OK;

}

HRESULT D3D_CALL IDeviceImp9 :: SetVertexDeclaration (  IDirect3DVertexDeclaration9* pDecl ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9:  SetVertexDeclaration" << endl; ) )

    state->get_child(StateId(NAME_CURRENT_VERTEX_DECLARATION))->write_data(&pDecl);

    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: GetVertexDeclaration (  IDirect3DVertexDeclaration9** ppDecl ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9:  GetVertexDeclaration" << endl; ) )

    state->get_child(StateId(NAME_CURRENT_VERTEX_DECLARATION))->read_data(ppDecl);

    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: SetFVF (  DWORD FVF ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9: SetFVF " << endl; ) )

    state->get_child(StateId(NAME_FVF))->write_data(&FVF);

    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: GetFVF (  DWORD* pFVF ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetFVF  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: CreateVertexShader (  CONST DWORD* pFunction , IDirect3DVertexShader9** ppShader ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9:  CreateVertexShader" << endl; ) )
    IVertexShaderImp9* vsh = new IVertexShaderImp9(state, this, pFunction);
    vsh->AddRef();
    i_vertex_shader_childs.insert(vsh);
    * ppShader = vsh;
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: SetVertexShader (  IDirect3DVertexShader9* pShader ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9:  SetVertexShader" << endl; ) )
    state->get_child(StateId(NAME_CURRENT_VERTEX_SHADER))->write_data(&pShader);
    return D3D_OK;

}

HRESULT D3D_CALL IDeviceImp9 :: GetVertexShader (  IDirect3DVertexShader9** ppShader ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9:  GetVertexShader" << endl; ) )
    state->get_child(StateId(NAME_CURRENT_VERTEX_SHADER))->read_data(ppShader);
    if(*ppShader != 0) (*ppShader)->AddRef();
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: SetVertexShaderConstantF (  UINT StartRegister , CONST float* pConstantData , UINT Vector4fCount ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9: SetVertexShaderConstantF " << (int)StartRegister << "to " << int(StartRegister + Vector4fCount) << endl; ) )
    for(UINT i = 0; i < Vector4fCount; i ++) {
        StateDataNode* s_const;
        s_const = state->get_child(StateId(NAME_VS_CONSTANT,StateIndex(StartRegister + i)));

        D3D_DEBUG(
            cout << "IDEVICE9: " << int(i + StartRegister) << " = (";
            for(UINT j = 0; j < 4; j ++) {
                if(j!= 0) cout << ", ";
                cout << pConstantData[i * 4 + j];
            }
            cout << ")" << endl;
        )

        s_const->write_data(&pConstantData[i * 4]);
    }
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: GetVertexShaderConstantF (  UINT StartRegister , float* pConstantData , UINT Vector4fCount ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetVertexShaderConstantF  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: SetVertexShaderConstantI (  UINT StartRegister , CONST int* pConstantData , UINT Vector4iCount ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: SetVertexShaderConstantI  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetVertexShaderConstantI (  UINT StartRegister , int* pConstantData , UINT Vector4iCount ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetVertexShaderConstantI  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: SetVertexShaderConstantB (  UINT StartRegister , CONST BOOL* pConstantData , UINT BoolCount ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: SetVertexShaderConstantB  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetVertexShaderConstantB (  UINT StartRegister , BOOL* pConstantData , UINT BoolCount ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetVertexShaderConstantB  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: SetStreamSource (  UINT StreamNumber , IDirect3DVertexBuffer9* pStreamData , UINT OffsetInBytes , UINT Stride ) {
    D3D_DEBUG( cout <<"IDEVICE9: SetStreamSource(" << StreamNumber << ", " << pStreamData << ", " << OffsetInBytes << ", " << Stride << ")" << endl; )
    StateDataNode* s_stream = state->get_child(StateId(NAME_STREAM, StreamNumber));
    /**@note Always update SOURCE, then OFFSET because there's a subtle potential problem here.
    		Do you want to know why? Have you taken your meds? Then you can continue...
    		If OFFSET is written before SOURCE, the controller that handles the event 'OFFSET written'
            will use the previous value of SOURCE, that hasn't been written yet with
            the new value. If the previous value of SOURCE corresponds to a vertex buffer
            that has been released the controller will not find the memory associated to it.
            If SOURCE is written before OFFSET, then when 'SOURCE written' is handled the
            previous value of OFFSET is used, this OFFSET could be outside the boundaries of
            the new SOURCE vertex buffer.
            The current event model can't generate an event saying 'SOURCE and OFFSET has been written'.
            The solution adopted is to update gpu stream address only when OFFSET is written. So the
            order of writting must be SOURCE then OFFSET. */
    s_stream->get_child(StateId(NAME_SOURCE))->write_data(&pStreamData);
    s_stream->get_child(StateId(NAME_OFFSET))->write_data(&OffsetInBytes);
    s_stream->get_child(StateId(NAME_STRIDE))->write_data(&Stride);
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: GetStreamSource (  UINT StreamNumber , IDirect3DVertexBuffer9** ppStreamData , UINT* pOffsetInBytes , UINT* pStride ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9: GetStreamSource" << endl; ) )
    StateDataNode* s_stream = state->get_child(StateId(NAME_STREAM, StreamNumber));
    s_stream->get_child(StateId(NAME_SOURCE))->read_data(ppStreamData);
    s_stream->get_child(StateId(NAME_OFFSET))->read_data(pOffsetInBytes);
    s_stream->get_child(StateId(NAME_STRIDE))->read_data(pStride);
    if(*ppStreamData != 0) (*ppStreamData)->AddRef();
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: SetStreamSourceFreq (  UINT StreamNumber , UINT Setting ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: SetStreamSourceFreq  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetStreamSourceFreq (  UINT StreamNumber , UINT* pSetting ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetStreamSourceFreq  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: SetIndices (  IDirect3DIndexBuffer9* pIndexData ) {
    D3D_DEBUG( cout << "IDEVICE9:  SetIndices" << endl; )
    state->get_child(StateId(NAME_CURRENT_INDEX_BUFFER))->write_data(&pIndexData);
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: GetIndices (  IDirect3DIndexBuffer9** ppIndexData ) {
    D3D_DEBUG( cout << "IDEVICE9:  GetIndices" << endl; )
    state->get_child(StateId(NAME_CURRENT_INDEX_BUFFER))->read_data(ppIndexData);
    if(*ppIndexData != 0) (*ppIndexData)->AddRef();
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: CreatePixelShader (  CONST DWORD* pFunction , IDirect3DPixelShader9** ppShader ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9:  CreatePixelShader" << endl; ) )
    IPixelShaderImp9* psh = new IPixelShaderImp9(state, this, pFunction);
    psh->AddRef();
    i_pixel_shader_childs.insert(psh);
    * ppShader = psh;
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: SetPixelShader (  IDirect3DPixelShader9* pShader ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9:  SetPixelShader" << endl; ) )
    state->get_child(StateId(NAME_CURRENT_PIXEL_SHADER))->write_data(&pShader);
    return D3D_OK;

}

HRESULT D3D_CALL IDeviceImp9 :: GetPixelShader (  IDirect3DPixelShader9** ppShader ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9:  GetPixelShader" << endl; ) )
    state->get_child(StateId(NAME_CURRENT_PIXEL_SHADER))->read_data(ppShader);
    if(*ppShader != 0) (*ppShader)->AddRef();
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: SetPixelShaderConstantF (  UINT StartRegister , CONST float* pConstantData , UINT Vector4fCount ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9: SetPixelShaderConstantF " << (int)StartRegister << "to " << int(StartRegister + Vector4fCount) << endl; ) )
    for(UINT i = 0; i < Vector4fCount; i ++) {
        StateDataNode* s_const;
        s_const = state->get_child(StateId(NAME_PS_CONSTANT,StateIndex(StartRegister + i)));
        D3D_DEBUG(
            cout << "IDEVICE9: " << int(i + StartRegister) << " = (";
            for(UINT j = 0; j < 4; j ++) {
                if(j!= 0) cout << ", ";
                cout << pConstantData[i * 4 + j];
            }
            cout << ")" << endl;
        )
        s_const->write_data(&pConstantData[i * 4]);
    }
    return D3D_OK;
}

HRESULT D3D_CALL IDeviceImp9 :: GetPixelShaderConstantF (  UINT StartRegister , float* pConstantData , UINT Vector4fCount ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetPixelShaderConstantF  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: SetPixelShaderConstantI (  UINT StartRegister , CONST int* pConstantData , UINT Vector4iCount ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: SetPixelShaderConstantI  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetPixelShaderConstantI (  UINT StartRegister , int* pConstantData , UINT Vector4iCount ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetPixelShaderConstantI  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: SetPixelShaderConstantB (  UINT StartRegister , CONST BOOL* pConstantData , UINT BoolCount ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: SetPixelShaderConstantB  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: GetPixelShaderConstantB (  UINT StartRegister , BOOL* pConstantData , UINT BoolCount ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetPixelShaderConstantB  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: DrawRectPatch (  UINT Handle , CONST float* pNumSegs , CONST D3DRECTPATCH_INFO* pRectPatchInfo ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: DrawRectPatch  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: DrawTriPatch (  UINT Handle , CONST float* pNumSegs , CONST D3DTRIPATCH_INFO* pTriPatchInfo ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: DrawTriPatch  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: DeletePatch (  UINT Handle ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: DeletePatch  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDeviceImp9 :: CreateQuery (  D3DQUERYTYPE Type , IDirect3DQuery9** ppQuery ) {
    * ppQuery = & IQueryImp9 :: getInstance();
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: CreateQuery  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}
