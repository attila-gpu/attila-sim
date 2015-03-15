/**************************************************************************
 *
 * Copyright (c) 2002 - 2011 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 */

#include "Common.h"
#include "AD3D9State.h"

#include "AIVolumeTextureImp_9.h"
#include "AICubeTextureImp_9.h"
#include "AIIndexBufferImp_9.h"
#include "AIStateBlockImp_9.h"
#include "AIBaseTextureImp_9.h"
#include "AIVertexBufferImp_9.h"
#include "AIVertexDeclarationImp_9.h"
#include "AIVertexShaderImp_9.h"
#include "AIPixelShaderImp_9.h"
#include "AIQueryImp_9.h"
#include "AIDirect3DImp_9.h"
#include "AISwapChainImp_9.h"
#include "AISurfaceImp_9.h"
#include "AITextureImp_9.h"
#include "AIDeviceImp_9.h"

#include "ACD.h"
#include "ACDRasterizationStage.h"
#include "ACDZStencilStage.h"
#include "ACDBlendingStage.h"
#include "ACDShaderProgram.h"
#include "ACDBuffer.h"
#include "ACDTexture2D.h"
#include "ACDSampler.h"

#include "ShaderTranslator.h"
#include "Utils.h"

#include <cstring>

char cover_buffer_9[COVER_BUFFER_SIZE_9];

void AIDeviceImp9::captureRenderState(map<D3DRENDERSTATETYPE, DWORD> &capture)
{
    capture = currentRenderState;
}

AIDeviceImp9 :: AIDeviceImp9() {}

AIDeviceImp9 & AIDeviceImp9 :: getInstance()
{
    static AIDeviceImp9 instance;
    return instance;
}

AIDeviceImp9 :: AIDeviceImp9(/*StateDataNode *s_parent, */ AIDirect3DImp9* _i_parent, D3DPRESENT_PARAMETERS * pp) :
i_parent(_i_parent)
{

#ifdef LOWER_RESOLUTION_HACK
    AD3D9State::instance().initialize(this, max(1, pp->BackBufferWidth >> 1), max(1, pp->BackBufferHeight >> 1));
#else
    AD3D9State::instance().initialize(this, pp->BackBufferWidth, pp->BackBufferHeight);
#endif
    
    refs = 0;

    i_surface_childs.insert(AD3D9State::instance().getZStencilBuffer());
    
    if(pp->EnableAutoDepthStencil)
        D3D_DEBUG( D3D_DEBUG( cout << "WARNING: No auto depth stencil" << endl; ) )

}

bool AIDeviceImp9::using_implicit_render_target_zstencil()
{
    return true;
}


AIDeviceImp9 :: ~AIDeviceImp9() {
 /// @todo delete state
    set< AIVertexBufferImp9 * > :: iterator it_vb;
    for(it_vb =  i_vertex_buffer_childs.begin(); it_vb != i_vertex_buffer_childs.end(); it_vb ++)
        delete (*it_vb);

    set< AIVertexDeclarationImp9 * > :: iterator it_vd;
    for(it_vd =  i_vertex_declaration_childs.begin(); it_vd != i_vertex_declaration_childs.end(); it_vd ++)
        delete (*it_vd);

    set< AIIndexBufferImp9 * > :: iterator it_ib;
    for(it_ib =  i_index_buffer_childs.begin(); it_ib != i_index_buffer_childs.end(); it_ib ++)
        delete (*it_ib);

    set< AIVertexShaderImp9 * > :: iterator it_vsh;
    for(it_vsh =  i_vertex_shader_childs.begin(); it_vsh != i_vertex_shader_childs.end(); it_vsh ++)
        delete (*it_vsh);

    set< AIPixelShaderImp9 * > :: iterator it_psh;
    for(it_psh =  i_pixel_shader_childs.begin(); it_psh != i_pixel_shader_childs.end(); it_psh ++)
        delete (*it_psh);

    set< AITextureImp9 * > :: iterator it_tex;
    for(it_tex =  i_texture_childs.begin(); it_tex != i_texture_childs.end(); it_tex ++)
        delete (*it_tex);

    set< AICubeTextureImp9 * > :: iterator it_cubetex;
    for(it_cubetex =  i_cubetexture_childs.begin(); it_cubetex != i_cubetexture_childs.end(); it_cubetex ++)
        delete (*it_cubetex);

    set< AIVolumeTextureImp9 * > :: iterator it_volumetex;
    for(it_volumetex =  i_volumetexture_childs.begin(); it_volumetex != i_volumetexture_childs.end(); it_volumetex ++)
        delete (*it_volumetex);

    set< AISurfaceImp9 * > :: iterator it_surf;
    for(it_surf =  i_surface_childs.begin(); it_surf != i_surface_childs.end(); it_surf ++)
        delete (*it_surf);
}


HRESULT D3D_CALL AIDeviceImp9 :: QueryInterface (  REFIID riid , void** ppvObj ) 
{
    D3D9_CALL(false, "AIDeviceImp9::QueryInterface")

    * ppvObj = cover_buffer_9;
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL AIDeviceImp9 :: AddRef ( ) 
{
    D3D9_CALL(true, "AIDeviceImp9::AddRef")
    /*ULONG ret = static_cast< ULONG >(0);
    return ret;*/

    refs++;
    return refs;

}

ULONG D3D_CALL AIDeviceImp9 :: Release ( ) 
{
    D3D9_CALL(true, "AIDeviceImp9::Release")
    /*ULONG ret = static_cast< ULONG >(0);
    return ret;*/

    refs--;

    if (refs == 0)
        AD3D9State::instance().destroy(); // TODO: Destroy everything...

    return refs;

}

HRESULT D3D_CALL AIDeviceImp9 :: TestCooperativeLevel ( ) 
{
    D3D9_CALL(false, "AIDeviceImp9::TestCooperativeLevel")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

UINT D3D_CALL AIDeviceImp9 :: GetAvailableTextureMem ( ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetAvailableTextureMem")
    UINT ret = static_cast< UINT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: EvictManagedResources ( ) 
{
    D3D9_CALL(false, "AIDeviceImp9::EvictManagedResources")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetDirect3D (  IDirect3D9** ppD3D9 ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetDirect3D")
    * ppD3D9 = & AIDirect3DImp9 :: getInstance();
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetDeviceCaps (  D3DCAPS9* pCaps ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetDeviceCaps")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetDisplayMode (  UINT iSwapChain , D3DDISPLAYMODE* pMode ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetDisplayMode")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetCreationParameters (  D3DDEVICE_CREATION_PARAMETERS * pParameters ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetCreationParameters")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: SetCursorProperties (  UINT XHotSpot , UINT YHotSpot , IDirect3DSurface9* pCursorBitmap ) 
{
    D3D9_CALL(false, "AIDeviceImp9::SetCursorProperties")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

void D3D_CALL AIDeviceImp9 :: SetCursorPosition (  int X , int Y , DWORD Flags ) 
{
    D3D9_CALL(false, "AIDeviceImp9::SetCursorPosition")
}

BOOL D3D_CALL AIDeviceImp9 :: ShowCursor (  BOOL bShow ) 
{
    D3D9_CALL(false, "AIDeviceImp9::ShowCursor")
    BOOL ret = static_cast< BOOL >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: CreateAdditionalSwapChain (  D3DPRESENT_PARAMETERS* pPresentationParameters , IDirect3DSwapChain9** pSwapChain ) 
{
    D3D9_CALL(false, "AIDeviceImp9::CreateAdditionalSwapChain")
    * pSwapChain = & AISwapChainImp9 :: getInstance();
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetSwapChain (  UINT iSwapChain , IDirect3DSwapChain9** pSwapChain ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetSwapChain")
    * pSwapChain = & AISwapChainImp9 :: getInstance();
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

UINT D3D_CALL AIDeviceImp9 :: GetNumberOfSwapChains ( ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetNumberOfSwapChains")
    UINT ret = static_cast< UINT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: Reset (  D3DPRESENT_PARAMETERS* pPresentationParameters ) 
{
    D3D9_CALL(false, "AIDeviceImp9::Reset")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: Present (  CONST RECT* pSourceRect , CONST RECT* pDestRect , HWND hDestWindowOverride , CONST RGNDATA* pDirtyRegion ) 
{

    D3D9_CALL(true, "AIDeviceImp9::Present")

    AD3D9State::instance().swapBuffers();

    return D3D_OK;

}

HRESULT D3D_CALL AIDeviceImp9 :: GetBackBuffer (  UINT iSwapChain , UINT iBackBuffer , D3DBACKBUFFER_TYPE Type , IDirect3DSurface9** ppBackBuffer ) 
{
    D3D9_CALL(true, "AIDeviceImp9::GetBackBuffer")

    /** ppBackBuffer = & AISurfaceImp9 :: getInstance();
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetBackBuffer  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;*/

    //*ppBackBuffer = (IDirect3DSurface9 *) AD3D9State::instance().getRenderTarget(iBackBuffer);
    *ppBackBuffer = (IDirect3DSurface9 *) AD3D9State::instance().getDefaultRenderSurface();
    
    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetRasterStatus (  UINT iSwapChain , D3DRASTER_STATUS* pRasterStatus ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetRasterStatus")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: SetDialogBoxMode (  BOOL bEnableDialogs ) 
{
    D3D9_CALL(false, "AIDeviceImp9::SetDialogBoxMode")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

void D3D_CALL AIDeviceImp9 :: SetGammaRamp (  UINT iSwapChain , DWORD Flags , CONST D3DGAMMARAMP* pRamp ) 
{
    D3D9_CALL(false, "AIDeviceImp9::SetGammaRamp")
}

void D3D_CALL AIDeviceImp9 :: GetGammaRamp (  UINT iSwapChain , D3DGAMMARAMP* pRamp ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetGammaRamp")
}

HRESULT D3D_CALL AIDeviceImp9 :: CreateTexture (  UINT Width , UINT Height , UINT Levels , DWORD Usage , D3DFORMAT Format , D3DPOOL Pool , IDirect3DTexture9** ppTexture , HANDLE* pSharedHandle )
{
    D3D9_CALL(true, "AIDeviceImp9::CreateTexture")

#ifdef LOWER_RESOLUTION_HACK
    if ((Usage & D3DUSAGE_DEPTHSTENCIL) || (Usage & D3DUSAGE_RENDERTARGET))
    {
        Width = max(1, Width >> 1);
        Height = max(1, Height >> 1);
    }
#endif

    //  Hack for S.T.A.L.K.E.R. as it seems to be using a surface of this type
    //  but with all the write mask set to false, so seems a way to implement
    //  a null surface (depth only passes) with the minimum texel size (that
    //  is supported for a render target in D3D9).
    if (Format == D3DFMT_R5G6B5)
        Format = D3DFMT_NULL;

    AITextureImp9* tex = new AITextureImp9(this, Width, Height, Levels, Usage, Format, Pool);

    tex->AddRef();

    i_texture_childs.insert(tex);

    * ppTexture = tex;

    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: CreateVolumeTexture (  UINT Width , UINT Height , UINT Depth , UINT Levels , DWORD Usage , D3DFORMAT Format , D3DPOOL Pool , IDirect3DVolumeTexture9** ppVolumeTexture , HANDLE* pSharedHandle ) 
{
    D3D9_CALL(true, "AIDeviceImp9::CreateVolumeTexture")
    AIVolumeTextureImp9* vol_tex = new AIVolumeTextureImp9(this, Width, Height, Depth, Levels, Usage, Format, Pool);
    i_volumetexture_childs.insert(vol_tex);
    * ppVolumeTexture = vol_tex;
    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: CreateCubeTexture (  UINT EdgeLength , UINT Levels , DWORD Usage , D3DFORMAT Format , D3DPOOL Pool , IDirect3DCubeTexture9** ppCubeTexture , HANDLE* pSharedHandle ) 
{
    D3D9_CALL(true, "AIDeviceImp9::CreateCubeTexture")
    AICubeTextureImp9* tex = new AICubeTextureImp9(this, EdgeLength, Levels, Usage, Format, Pool);
    tex->AddRef();
    i_cubetexture_childs.insert(tex);
    * ppCubeTexture = tex;
    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: CreateVertexBuffer (  UINT Length , DWORD Usage , DWORD FVF , D3DPOOL Pool , IDirect3DVertexBuffer9** ppVertexBuffer , HANDLE* pSharedHandle ) 
{

    D3D9_CALL(true, "AIDeviceImp9::CreateVertexBuffer")

    AIVertexBufferImp9* vb = new AIVertexBufferImp9(this, Length, Usage, FVF, Pool);

    vb->AddRef();

    i_vertex_buffer_childs.insert(vb);

    * ppVertexBuffer = vb;

    return D3D_OK;

}

HRESULT D3D_CALL AIDeviceImp9 :: CreateIndexBuffer (  UINT Length , DWORD Usage , D3DFORMAT Format , D3DPOOL Pool , IDirect3DIndexBuffer9** ppIndexBuffer , HANDLE* pSharedHandle ) 
{

    D3D9_CALL(true, "AIDeviceImp9::CreateIndexBuffer")

    AIIndexBufferImp9* ib = new AIIndexBufferImp9(this, Length, Usage, Format, Pool);

    ib->AddRef();

    i_index_buffer_childs.insert(ib);

    * ppIndexBuffer = ib;

    return D3D_OK;

}

HRESULT D3D_CALL AIDeviceImp9 :: CreateRenderTarget (  UINT Width , UINT Height , D3DFORMAT Format , D3DMULTISAMPLE_TYPE MultiSample , DWORD MultisampleQuality , BOOL Lockable , IDirect3DSurface9** ppSurface , HANDLE* pSharedHandle ) 
{
    /** ppSurface = & AISurfaceImp9 :: getInstance();
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: CreateRenderTarget  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;*/
    
    D3D9_CALL(true, "AIDeviceImp9::CreateRenderTarget")

#ifdef LOWER_RESOLUTION_HACK
    Width = max(1, Width >> 1);
    Height = max(1, Height >> 1);
#endif

    AISurfaceImp9* rt = new AISurfaceImp9(this, Width, Height, D3DUSAGE_DEPTHSTENCIL, Format, D3DPOOL_DEFAULT);

    AD3D9State::instance().addRenderTarget(rt, Width, Height, Format);

    rt->AddRef();

    i_surface_childs.insert(rt);

    * ppSurface = rt;

    return D3D_OK;

}

HRESULT D3D_CALL AIDeviceImp9 :: CreateDepthStencilSurface (  UINT Width , UINT Height , D3DFORMAT Format , D3DMULTISAMPLE_TYPE MultiSample , DWORD MultisampleQuality , BOOL Discard , IDirect3DSurface9** ppSurface , HANDLE* pSharedHandle ) 
{
    /** ppSurface = & AISurfaceImp9 :: getInstance();
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: CreateDepthStencilSurface  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;*/
    
    D3D9_CALL(true, "AIDeviceImp9::CreateDepthStencilSurface")

#ifdef LOWER_RESOLUTION_HACK
    Width = max(1, Width >> 1);
    Height = max(1, Height >> 1);
#endif

    AISurfaceImp9* zstencil = new AISurfaceImp9(this, Width, Height, D3DUSAGE_DEPTHSTENCIL, Format, D3DPOOL_DEFAULT);

    AD3D9State::instance().addRenderTarget(zstencil, Width, Height, Format);

    zstencil->AddRef();

    i_surface_childs.insert(zstencil);

    * ppSurface = zstencil;

    return D3D_OK;

}

#ifndef min
    #define min(a, b) ((a) < (b) ? (a) : (b))
#endif

HRESULT D3D_CALL AIDeviceImp9 :: UpdateSurface (  IDirect3DSurface9* pSourceSurface , CONST RECT* pSourceRect , IDirect3DSurface9* pDestinationSurface , CONST POINT* pDestPoint ) 
{
    /*D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: UpdateSurface  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;*/
    
    D3D9_CALL(true, "AIDeviceImp9::UpdateSurface")

    /*D3DLOCKED_RECT lockedRect;
    RECT destRect;
    D3DSURFACE_DESC sourceDesc;
    D3DSURFACE_DESC destDesc;
    LONG sourceLeft, sourceTop;

    pSourceSurface->GetDesc(&sourceDesc);
    pDestinationSurface->GetDesc(&destDesc);

    if (pDestPoint) {
        destRect.left = pDestPoint->x;
        destRect.top = pDestPoint->y;
    }
    else {
        destRect.left = 0;
        destRect.top = 0;
    }

    if (pSourceRect) {
        destRect.right = pSourceRect->right - pSourceRect->left + destRect.left;
        destRect.bottom = pSourceRect->bottom - pSourceRect->top + destRect.top;

        sourceLeft = pSourceRect->left;
        sourceTop = pSourceRect->top;
    }
    else {
        destRect.right = sourceDesc.Width + destRect.left;
        destRect.bottom = sourceDesc.Height + destRect.top;

        sourceLeft = 0;
        sourceTop = 0;
    }
    
    pDestinationSurface->LockRect(&lockedRect, &destRect, D3DLOCK_NOSYSLOCK); // Flags are ignored, but I don't know what to put...

    const BYTE* sourceData = ((AISurfaceImp9*)pSourceSurface)->getData();

    UINT sourcePitch = sourceDesc.Width * texel_size(sourceDesc.Format);
    UINT destPitch = destDesc.Width * texel_size(destDesc.Format);

    if (is_compressed(sourceDesc.Format))
        memcpy(lockedRect.pBits, sourceData, getSurfaceSize(sourceDesc.Width, sourceDesc.Height, sourceDesc.Format));
    else {
        for (UINT i = 0; i < (destRect.bottom - destRect.top); i++) {
            memcpy((((BYTE*)lockedRect.pBits) + destRect.top * destPitch + i * destPitch + destRect.left * texel_size(destDesc.Format)), (sourceData + sourceTop * sourcePitch + i * sourcePitch + sourceLeft * texel_size(sourceDesc.Format)), lockedRect.Pitch);
        }
    }

    pDestinationSurface->UnlockRect();*/

    //AD3D9State::instance().getDataSurface(this);

    D3DSURFACE_DESC srcDesc;
    D3DSURFACE_DESC dstDesc;

    RECT srcRect;
    RECT dstRect;
    
    pSourceSurface->GetDesc(&srcDesc);
    pDestinationSurface->GetDesc(&dstDesc);

    if (pSourceRect == NULL) {
        srcRect.bottom = srcDesc.Height;
        srcRect.left = 0;
        srcRect.right = srcDesc.Width;
        srcRect.top = 0;
    }
    else {
        srcRect.bottom = pSourceRect->bottom;
        srcRect.left = pSourceRect->left;
        srcRect.right = pSourceRect->right;
        srcRect.top = pSourceRect->top;
    }
    
    if (pDestPoint == NULL) {
        dstRect.bottom = dstDesc.Height;
        dstRect.left = 0;
        dstRect.right = dstDesc.Width;
        dstRect.top = 0;
    }
    else {
        dstRect.bottom = pDestPoint->y + min((srcRect.bottom - srcRect.top), dstDesc.Height);
        dstRect.left = pDestPoint->x;
        dstRect.right = pDestPoint->x + min((srcRect.right - srcRect.left), dstDesc.Width);
        dstRect.top = pDestPoint->y;
    }

    AD3D9State::instance().copySurface((AISurfaceImp9 *)pSourceSurface, &srcRect, (AISurfaceImp9 *)pDestinationSurface, &dstRect, D3DTEXF_POINT);

    return D3D_OK;

}

HRESULT D3D_CALL AIDeviceImp9 :: UpdateTexture (  IDirect3DBaseTexture9* pSourceTexture , IDirect3DBaseTexture9* pDestinationTexture ) 
{
    D3D9_CALL(false, "AIDeviceImp9::UpdateTexture")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetRenderTargetData (  IDirect3DSurface9* pRenderTarget , IDirect3DSurface9* pDestSurface ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetRenderTargetData")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetFrontBufferData (  UINT iSwapChain , IDirect3DSurface9* pDestSurface ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetFrontBufferData")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: StretchRect (  IDirect3DSurface9* pSourceSurface , CONST RECT* pSourceRect , IDirect3DSurface9* pDestSurface , CONST RECT* pDestRect , D3DTEXTUREFILTERTYPE Filter )
{
    /*D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: StretchRect  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;*/

    D3D9_CALL(true, "AIDeviceImp9::StretchRect")

    /*if (pSourceRect != NULL) {
        cout << "AIDeviceImp9: srcRect: " << pSourceRect->top << ", " << pSourceRect->left << ", " << pSourceRect->bottom << ", " << pSourceRect->right << endl;
            //panic("AD3D9State", "copySurface", "Partial copy not supported.");
    }

    if (pDestRect != NULL) {
        cout << "AIDeviceImp9: srcRect: " << pDestRect->top << ", " << pDestRect->left << ", " << pDestRect->bottom << ", " << pDestRect->right << endl;
            //panic("AD3D9State", "copySurface", "Position diferent than (0,0) not supported.");
    }*/

    D3DSURFACE_DESC srcDesc;
    D3DSURFACE_DESC dstDesc;

    RECT srcRect;
    RECT dstRect;
    
    pSourceSurface->GetDesc(&srcDesc);
    pDestSurface->GetDesc(&dstDesc);

    if (pSourceRect == NULL) {
        srcRect.bottom = srcDesc.Height;
        srcRect.left = 0;
        srcRect.right = srcDesc.Width;
        srcRect.top = 0;
    }
    else {
        srcRect.bottom = pSourceRect->bottom;
        srcRect.left = pSourceRect->left;
        srcRect.right = pSourceRect->right;
        srcRect.top = pSourceRect->top;
    }
    
    if (pDestRect == NULL) {
        dstRect.bottom = dstDesc.Height;
        dstRect.left = 0;
        dstRect.right = dstDesc.Width;
        dstRect.top = 0;
    }
    else {
        dstRect.bottom = pDestRect->bottom;
        dstRect.left = pDestRect->left;
        dstRect.right = pDestRect->right;
        dstRect.top = pDestRect->top;
    }
    
    AD3D9State::instance().copySurface((AISurfaceImp9 *)pSourceSurface, &srcRect, (AISurfaceImp9 *)pDestSurface, &dstRect, Filter);

    return D3D_OK;

}

HRESULT D3D_CALL AIDeviceImp9 :: ColorFill (  IDirect3DSurface9* pSurface , CONST RECT* pRect , D3DCOLOR color ) 
{
    D3D9_CALL(false, "AIDeviceImp9::ColorFill")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: CreateOffscreenPlainSurface (  UINT Width , UINT Height , D3DFORMAT Format , D3DPOOL Pool , IDirect3DSurface9** ppSurface , HANDLE* pSharedHandle )
{
    //* ppSurface = & AISurfaceImp9 :: getInstance();
    //D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: CreateOffscreenPlainSurface  NOT IMPLEMENTED" << endl; ) )
    //HRESULT ret = static_cast< HRESULT >(0);
    //
    //return ret;

    D3D9_CALL(true, "AIDeviceImp9::CreateOffscreenPlainSurface")

    AITextureImp9* tex = new AITextureImp9(this, Width, Height, 1, 0, Format, Pool);
    tex->AddRef();

    i_texture_childs.insert(tex);

    tex->GetSurfaceLevel(0, ppSurface);
    (*ppSurface)->AddRef();

    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: SetRenderTarget (  DWORD RenderTargetIndex , IDirect3DSurface9* pRenderTarget ) 
{
    //D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: SetRenderTarget  NOT IMPLEMENTED" << endl; ) ) //added__    
    D3D9_CALL(true, "AIDeviceImp9::SetRenderTarget")
    /*///@todo check index is always 0
    state->get_child("CURRENT_RENDER_TARGET")->write_data(&pRenderTarget);*/
    
    if (RenderTargetIndex < 0 || RenderTargetIndex >= 4)
    { 
        if (pRenderTarget != 0)
            panic("AIDeviceImp9", "SetRenderTarget", "Render Target index out of range.");
        
        return D3D_OK;
    }
    
    AD3D9State::instance().setRenderTarget(RenderTargetIndex, (AISurfaceImp9 *) pRenderTarget);
    
    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetRenderTarget (  DWORD RenderTargetIndex , IDirect3DSurface9** ppRenderTarget ) 
{
    D3D9_CALL(true, "AIDeviceImp9::GetRenderTarget")

    //D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: GetRenderTarget  NOT IMPLEMENTED" << endl; ) ) //added__
    /*D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9: GetRenderTarget" << endl; ) )
    state->get_child("CURRENT_RENDER_TARGET")->read_data(ppRenderTarget);
    if((*ppRenderTarget) != 0 )
        (*ppRenderTarget)->AddRef();*/
    ///@todo check index is always 0
    
    if (RenderTargetIndex != 0)
        panic("AIDeviceImp9", "GetRenderTarget", "Multiple render target support not yet implemented.");
        
    *ppRenderTarget = (IDirect3DSurface9 *) AD3D9State::instance().getRenderTarget(RenderTargetIndex);
    (*ppRenderTarget)->AddRef();
    
    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: SetDepthStencilSurface (  IDirect3DSurface9* pNewZStencil ) 
{
    D3D9_CALL(true, "AIDeviceImp9::SetDepthStencilSurface")
    //D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: SetDepthStencilSurface  NOT IMPLEMENTED" << endl; ) ) //added__
    /*D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9: SetDepthStencilSurface" << endl; ) )
    state->get_child("CURRENT_DEPTHSTENCIL")->write_data(&pNewZStencil);*/
    
    AD3D9State::instance().setZStencilBuffer((AISurfaceImp9 *) pNewZStencil);
    
    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetDepthStencilSurface (  IDirect3DSurface9** ppZStencilSurface ) 
{
    D3D9_CALL(true, "AIDeviceImp9::GetDepthStencilSurface")
    /*D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9: GetDepthStencilSurface" << endl; ) )
    state->get_child("CURRENT_DEPTHSTENCIL")->read_data(ppZStencilSurface);
    if((*ppZStencilSurface) != 0) (*ppZStencilSurface)->AddRef();*/
    
    *ppZStencilSurface = (IDirect3DSurface9 *) AD3D9State::instance().getZStencilBuffer();    
    (*ppZStencilSurface)->AddRef();
    
    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: BeginScene ( ) 
{
    D3D9_CALL(false, "AIDeviceImp9::BeginScene")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: EndScene ( ) 
{
    D3D9_CALL(false, "AIDeviceImp9::EndScene")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) 
{
    D3D9_CALL(true, "AIDeviceImp9::Clear")

    AD3D9State::instance().clear(Count, pRects, Flags, Color, Z, Stencil);

    return D3D_OK;

}

HRESULT D3D_CALL AIDeviceImp9 :: SetTransform (  D3DTRANSFORMSTATETYPE State , CONST D3DMATRIX* pMatrix ) 
{
    D3D9_CALL(true, "AIDeviceImp9::SetTransform")
    
    AD3D9State::instance().setTransform(State, pMatrix);
    
    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetTransform (  D3DTRANSFORMSTATETYPE State , D3DMATRIX* pMatrix ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetTransform")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: MultiplyTransform (  D3DTRANSFORMSTATETYPE State , CONST D3DMATRIX* pMatrix ) 
{
    D3D9_CALL(false, "AIDeviceImp9::MultiplyTransform")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: SetViewport (  CONST D3DVIEWPORT9* pViewport ) 
{

    D3D9_CALL(true, "AIDeviceImp9::SetViewport")

#ifdef LOWER_RESOLUTION_HACK
    s32bit x = pViewport->X >> 1;
    s32bit y = pViewport->Y >> 1;
    u32bit width = max(1, pViewport->Width >> 1);
    u32bit height = max(1, pViewport->Height >> 1);
    AD3D9State::instance().setViewport(x, y, width, height, pViewport->MinZ, pViewport->MaxZ);
#else    
    AD3D9State::instance().setViewport(pViewport->X, pViewport->Y, pViewport->Width, pViewport->Height, pViewport->MinZ, pViewport->MaxZ);
#endif
    
    return D3D_OK;

}

HRESULT D3D_CALL AIDeviceImp9 :: GetViewport (  D3DVIEWPORT9* pViewport ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetViewport")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: SetMaterial (  CONST D3DMATERIAL9* pMaterial ) 
{
    D3D9_CALL(true, "AIDeviceImp9::SetMaterial")

    AD3D9State::instance().setMaterial(pMaterial);
    
    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetMaterial (  D3DMATERIAL9* pMaterial ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetMaterial")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: SetLight (  DWORD Index , CONST D3DLIGHT9* pLight ) 
{
    D3D9_CALL(true, "AIDeviceImp9::SetLight")       
    
    AD3D9State::instance().setLight(Index, pLight);
    
    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetLight (  DWORD Index , D3DLIGHT9* pLight ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetLight")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: LightEnable (  DWORD Index , BOOL Enable ) 
{
    D3D9_CALL(true, "AIDeviceImp9::LightEnable")
    
    AD3D9State::instance().enableLight(Index, Enable);

    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetLightEnable (  DWORD Index , BOOL* pEnable ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetLightEnable")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: SetClipPlane (  DWORD Index , CONST float* pPlane ) 
{
    D3D9_CALL(false, "AIDeviceImp9::SetClipPlane")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetClipPlane (  DWORD Index , float* pPlane ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetClipPlane")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9::SetRenderState (D3DRENDERSTATETYPE State , DWORD Value) 
{

    D3D9_CALL(true, "AIDeviceImp9::SetRenderState")

    switch(State)
    {
        case D3DRS_FOGENABLE:
        case D3DRS_FOGCOLOR:
        case D3DRS_FOGTABLEMODE:
        case D3DRS_FOGSTART:
        case D3DRS_FOGEND:
        case D3DRS_FOGDENSITY:
        case D3DRS_RANGEFOGENABLE:
        case D3DRS_FOGVERTEXMODE:

        case D3DRS_WRAP0:
        case D3DRS_WRAP1:
        case D3DRS_WRAP2:
        case D3DRS_WRAP3:
        case D3DRS_WRAP4:
        case D3DRS_WRAP5:
        case D3DRS_WRAP6:
        case D3DRS_WRAP7:
        case D3DRS_WRAP8:
        case D3DRS_WRAP9:
        case D3DRS_WRAP10:
        case D3DRS_WRAP11:
        case D3DRS_WRAP12:
        case D3DRS_WRAP13:
        case D3DRS_WRAP14:
        case D3DRS_WRAP15:
        
        case D3DRS_SPECULARENABLE:
        case D3DRS_LIGHTING:
        case D3DRS_AMBIENT:
        case D3DRS_COLORVERTEX:
        case D3DRS_LOCALVIEWER:
        case D3DRS_NORMALIZENORMALS:
        case D3DRS_DIFFUSEMATERIALSOURCE:
        case D3DRS_SPECULARMATERIALSOURCE:
        case D3DRS_AMBIENTMATERIALSOURCE:
        case D3DRS_EMISSIVEMATERIALSOURCE:

        case D3DRS_VERTEXBLEND:
        case D3DRS_INDEXEDVERTEXBLENDENABLE:
        case D3DRS_TWEENFACTOR:
        
        case D3DRS_TEXTUREFACTOR: 
               
            AD3D9State::instance().setFixedFunctionState(State, Value);
            break;
        
        //case D3DRS_FOGENABLE:
        //case D3DRS_FOGCOLOR:
        //    AD3D9State::instance().setFogState(State, Value);
        //    break;

        case D3DRS_CULLMODE:
            AD3D9State::instance().setCullMode(Value);
            break;

        case D3DRS_ZENABLE:
        case D3DRS_ZFUNC:
        case D3DRS_ZWRITEENABLE:
            AD3D9State::instance().setZState(State, Value);
            break;

        case D3DRS_ALPHATESTENABLE:
        case D3DRS_ALPHAREF:
        case D3DRS_ALPHAFUNC:
            AD3D9State::instance().setAlphaTestState(State, Value);
            break;

        case D3DRS_ALPHABLENDENABLE:
        case D3DRS_SEPARATEALPHABLENDENABLE:
        case D3DRS_SRCBLEND:
        case D3DRS_SRCBLENDALPHA:
        case D3DRS_DESTBLEND:
        case D3DRS_DESTBLENDALPHA:
        case D3DRS_BLENDOP:
        case D3DRS_BLENDOPALPHA:
        case D3DRS_BLENDFACTOR:
            AD3D9State::instance().setBlendingState(State, Value);
            break;

        case D3DRS_SCISSORTESTENABLE:
            AD3D9State::instance().enableScissorTest(Value);
            break;

        case D3DRS_STENCILENABLE:
        case D3DRS_STENCILFAIL:
        case D3DRS_STENCILZFAIL:
        case D3DRS_STENCILPASS:
        case D3DRS_STENCILFUNC:
        case D3DRS_STENCILREF:
        case D3DRS_STENCILMASK:
        case D3DRS_STENCILWRITEMASK:
        case D3DRS_TWOSIDEDSTENCILMODE:
        case D3DRS_CCW_STENCILFAIL:
        case D3DRS_CCW_STENCILZFAIL:
        case D3DRS_CCW_STENCILPASS:
        case D3DRS_CCW_STENCILFUNC:
            AD3D9State::instance().setStencilState(State, Value);
            break;

        case D3DRS_COLORWRITEENABLE:
            AD3D9State::instance().setColorWriteEnable(0, Value);
            break;

        case D3DRS_COLORWRITEENABLE1:
            AD3D9State::instance().setColorWriteEnable(1, Value);
            break;

        case D3DRS_COLORWRITEENABLE2:
            AD3D9State::instance().setColorWriteEnable(2, Value);
            break;

        case D3DRS_COLORWRITEENABLE3:
            AD3D9State::instance().setColorWriteEnable(3, Value);
            break;

        case D3DRS_SRGBWRITEENABLE:
            AD3D9State::instance().setColorSRGBWrite(Value);
            break;
            
        default:
            D3D_DEBUG( D3D_DEBUG( cout << "IDEVICE9: WARNING: " << renderstate2string(State) << ", value " << (int)Value << " is not supported." << endl; ) )
            break;

    }

    //  Update current and captured render state.
    currentRenderState[State] = Value;
    capturedRenderState[State] = Value;

    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetRenderState (  D3DRENDERSTATETYPE State , DWORD* pValue ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetRenderState")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: CreateStateBlock (  D3DSTATEBLOCKTYPE Type , IDirect3DStateBlock9** ppSB ) 
{
    D3D9_CALL(true, "AIDeviceImp9::CreateStateBlock")
    
    *ppSB = new AIStateBlockImp9(this, currentRenderState);

    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: BeginStateBlock ( ) 
{
    D3D9_CALL(true, "AIDeviceImp9::BeginStateBlock")

    //  Clear the captured state.
    capturedRenderState.clear();

    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: EndStateBlock (  IDirect3DStateBlock9** ppSB ) 
{
    D3D9_CALL(true, "AIDeviceImp9::EndStateBlock")

    *ppSB = new AIStateBlockImp9(this, capturedRenderState);
    
    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: SetClipStatus (  CONST D3DCLIPSTATUS9* pClipStatus ) 
{
    D3D9_CALL(false, "AIDeviceImp9::SetClipStatus")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetClipStatus (  D3DCLIPSTATUS9* pClipStatus ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetClipStatus")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetTexture (  DWORD Stage , IDirect3DBaseTexture9** ppTexture ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetTexture")
    /*D3D_DEBUG( cout <<"IDEVICE9: GetTexture" << endl; )
    StateDataNode* s_sampler = state->get_child(StateId("SAMPLER", StateIndex(Stage)));
    s_sampler->get_child(StateId("TEXTURE"))->read_data(ppTexture);
    if(*ppTexture != 0) (*ppTexture)->AddRef();*/
    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: SetTexture (  DWORD Stage , IDirect3DBaseTexture9* pTexture ) 
{

    D3D9_CALL(true, "AIDeviceImp9::SetTexture")

    //texturesSetted[Stage].samplerData = (AITextureImp9*)pTexture;

    AD3D9State::instance().setTexture((AIBaseTextureImp9*)pTexture, Stage);

    return D3D_OK;

}

HRESULT D3D_CALL AIDeviceImp9 :: GetTextureStageState (  DWORD Stage , D3DTEXTURESTAGESTATETYPE Type , DWORD* pValue ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetTextureStageState")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: SetTextureStageState (  DWORD Stage , D3DTEXTURESTAGESTATETYPE Type , DWORD Value ) 
{
    D3D9_CALL(true, "AIDeviceImp9::SetTextureStageState")
    
    AD3D9State::instance().setTextureStage(Stage, Type, Value);
    
    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetSamplerState (  DWORD Sampler , D3DSAMPLERSTATETYPE Type , DWORD* pValue ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetSamplerState")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: SetSamplerState (  DWORD Sampler , D3DSAMPLERSTATETYPE Type , DWORD Value )
{
    D3D9_CALL(true, "AIDeviceImp9::SetSamplerState")

    AD3D9State::instance().setSamplerState(Sampler, Type, Value);

    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: ValidateDevice (  DWORD* pNumPasses ) 
{
    D3D9_CALL(false, "AIDeviceImp9::ValidateDevice")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: SetPaletteEntries (  UINT PaletteNumber , CONST PALETTEENTRY* pEntries ) 
{
    D3D9_CALL(false, "AIDeviceImp9::SetPaletteEntries")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetPaletteEntries (  UINT PaletteNumber , PALETTEENTRY* pEntries ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetPaletteEntries")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: SetCurrentTexturePalette (  UINT PaletteNumber ) 
{
    D3D9_CALL(false, "AIDeviceImp9::SetCurrentTexturePalette")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetCurrentTexturePalette (  UINT * PaletteNumber ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetCurrentTexturePalette")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: SetScissorRect (  CONST RECT* pRect ) 
{

    D3D9_CALL(true, "AIDeviceImp9::SetScissorRect")

    AD3D9State::instance().scissorRect(pRect->left, pRect->top, pRect->right, pRect->bottom);

    return D3D_OK;

}

HRESULT D3D_CALL AIDeviceImp9 :: GetScissorRect (  RECT* pRect ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetScissorRect")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: SetSoftwareVertexProcessing (  BOOL bSoftware ) 
{
    D3D9_CALL(false, "AIDeviceImp9::SetSoftwareVertexProcessing")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

BOOL D3D_CALL AIDeviceImp9 :: GetSoftwareVertexProcessing ( ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetSoftwareVertexProcessing")
    BOOL ret = static_cast< BOOL >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: SetNPatchMode (  float nSegments ) 
{
    D3D9_CALL(false, "AIDeviceImp9::SetNPatchMode")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

float D3D_CALL AIDeviceImp9 :: GetNPatchMode ( ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetNPatchMode")
    float ret = static_cast< float >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) 
{

    D3D9_CALL(true, "AIDeviceImp9::DrawPrimitive")

    AD3D9State::instance().draw(PrimitiveType, StartVertex, PrimitiveCount);

    return D3D_OK;

}

HRESULT D3D_CALL AIDeviceImp9 :: DrawPrimitiveUP (  D3DPRIMITIVETYPE PrimitiveType , UINT PrimitiveCount , CONST void* pVertexStreamZeroData , UINT VertexStreamZeroStride ) 
{
    D3D9_CALL(false, "AIDeviceImp9::DrawPrimitiveUP")

    // Temporal vertex buffer
    INT vertexBufferSize = VertexStreamZeroStride * get_vertex_count(PrimitiveType, PrimitiveCount);
    AIVertexBufferImp9* vertexBuffer = new AIVertexBufferImp9(this, vertexBufferSize, D3DUSAGE_DYNAMIC, NULL, D3DPOOL_DEFAULT);
    void** vertexBufferData;
    vertexBufferData = new void*();
    vertexBuffer->Lock(0, 0, vertexBufferData, NULL);
    memcpy((*vertexBufferData), pVertexStreamZeroData, vertexBufferSize);
    vertexBuffer->Unlock();

    // Replace current setted stream0
    UINT originalOffset;
    UINT originalStride;
    AIVertexBufferImp9* originalStream0 = AD3D9State::instance().getVertexBuffer(0, originalOffset, originalStride);
    AD3D9State::instance().setVertexBuffer(vertexBuffer, 0, 0, VertexStreamZeroStride);

    AD3D9State::instance().draw(PrimitiveType, 0, PrimitiveCount);

    AD3D9State::instance().setVertexBuffer(originalStream0, 0, originalOffset, originalStride);

    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: DrawIndexedPrimitive (  D3DPRIMITIVETYPE Type , INT BaseVertexIndex , UINT MinVertexIndex , UINT NumVertices , UINT startIndex , UINT primCount ) 
{

    D3D9_CALL(true, "AIDeviceImp9::DrawIndexedPrimitive")

    AD3D9State::instance().drawIndexed(Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);

    return D3D_OK;


}

HRESULT D3D_CALL AIDeviceImp9 :: DrawIndexedPrimitiveUP (  D3DPRIMITIVETYPE PrimitiveType , UINT MinVertexIndex , UINT NumVertices , UINT PrimitiveCount , CONST void* pIndexData , D3DFORMAT IndexDataFormat , CONST void* pVertexStreamZeroData , UINT VertexStreamZeroStride ) 
{
    D3D9_CALL(true, "AIDeviceImp9::DrawIndexedPrimitiveUP")

    // Temporal index buffer
    INT indexLenght = get_index_size(IndexDataFormat) * get_vertex_count(PrimitiveType, PrimitiveCount);
    AIIndexBufferImp9* indexBuffer = new AIIndexBufferImp9(this, indexLenght, D3DUSAGE_DYNAMIC, IndexDataFormat, D3DPOOL_DEFAULT);
    void** indexBufferData;
    indexBufferData = new void*();
    indexBuffer->Lock(0, 0, indexBufferData, NULL);
    memcpy((*indexBufferData), pIndexData, indexLenght);
    indexBuffer->Unlock();

    // Replace current indexBuffer
    AIIndexBufferImp9* originalIndexBuffer = AD3D9State::instance().getIndexBuffer();
    AD3D9State::instance().setIndexBuffer(indexBuffer);

    // Temporal vertex buffer
    //INT vertexBufferSize = VertexStreamZeroStride * get_vertex_count(PrimitiveType, PrimitiveCount);
    INT vertexBufferSize = VertexStreamZeroStride * NumVertices;
    AIVertexBufferImp9* vertexBuffer = new AIVertexBufferImp9(this, vertexBufferSize, D3DUSAGE_DYNAMIC, NULL, D3DPOOL_DEFAULT);
    void** vertexBufferData;
    vertexBufferData = new void*();
    vertexBuffer->Lock(0, 0, vertexBufferData, NULL);
    memcpy((*vertexBufferData), pVertexStreamZeroData, vertexBufferSize);
    vertexBuffer->Unlock();

    // Replace current setted stream0
    UINT originalOffset;
    UINT originalStride;
    AIVertexBufferImp9* originalStream0 = AD3D9State::instance().getVertexBuffer(0, originalOffset, originalStride);
    AD3D9State::instance().setVertexBuffer(vertexBuffer, 0, 0, VertexStreamZeroStride);

    AD3D9State::instance().drawIndexed(PrimitiveType, 0, MinVertexIndex, NumVertices, 0, PrimitiveCount);

    // Return indexBuffer and stream0 to the original state
    AD3D9State::instance().setIndexBuffer(originalIndexBuffer);
    AD3D9State::instance().setVertexBuffer(originalStream0, 0, originalOffset, originalStride);

    // Delete temporal index buffer
    

    // Delete temporal vertex buffer

    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: ProcessVertices (  UINT SrcStartIndex , UINT DestIndex , UINT VertexCount , IDirect3DVertexBuffer9* pDestBuffer , IDirect3DVertexDeclaration9* pVertexDecl , DWORD Flags ) 
{
    D3D9_CALL(false, "AIDeviceImp9::ProcessVertices")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl) 
{

    D3D9_CALL(true, "AIDeviceImp9::CreateVertexDeclaration")

    AIVertexDeclarationImp9* vd = new AIVertexDeclarationImp9(this, pVertexElements);

    i_vertex_declaration_childs.insert(vd);

    * ppDecl = vd;

    return D3D_OK;

}

HRESULT D3D_CALL AIDeviceImp9::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl) 
{

    D3D9_CALL(true, "AIDeviceImp9::SetVertexDeclaration")

    AD3D9State::instance().setVertexDeclaration((AIVertexDeclarationImp9*)pDecl);

    return D3D_OK;

}

HRESULT D3D_CALL AIDeviceImp9 :: GetVertexDeclaration (  IDirect3DVertexDeclaration9** ppDecl ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetVertexDeclaration")

    (*ppDecl) = AD3D9State::instance().getVertexDeclaration();

    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9::SetFVF (  DWORD FVF ) 
{

    D3D9_CALL(true, "AIDeviceImp9::SetFVF")

    AD3D9State::instance().setFVF(FVF);

    return D3D_OK;

}

HRESULT D3D_CALL AIDeviceImp9 :: GetFVF (  DWORD* pFVF ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetFVF")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: CreateVertexShader (  CONST DWORD* pFunction , IDirect3DVertexShader9** ppShader ) 
{

    D3D9_CALL(true, "AIDeviceImp9::CreateVertexShader")

    AIVertexShaderImp9* vsh = new AIVertexShaderImp9(this, pFunction);

    vsh->AddRef();

    i_vertex_shader_childs.insert(vsh);

    * ppShader = vsh;

    return D3D_OK;

}

HRESULT D3D_CALL AIDeviceImp9 :: SetVertexShader (  IDirect3DVertexShader9* pShader ) 
{

    D3D9_CALL(true, "AIDeviceImp9::SetVertexShader")

    AD3D9State::instance().setVertexShader((AIVertexShaderImp9*)pShader);

    return D3D_OK;

}

HRESULT D3D_CALL AIDeviceImp9 :: GetVertexShader (  IDirect3DVertexShader9** ppShader ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetVertexShader")
    /*D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9:  GetVertexShader" << endl; ) )
    state->get_child(StateId("CURRENT_VERTEX_SHADER"))->read_data(ppShader);
    if(*ppShader != 0) (*ppShader)->AddRef();*/
    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: SetVertexShaderConstantF (  UINT StartRegister , CONST float* pConstantData , UINT Vector4fCount ) 
{

    D3D9_CALL(true, "AIDeviceImp9::SetVertexShaderConstantF")

    AD3D9State::instance().setVertexShaderConstant(StartRegister, pConstantData, Vector4fCount);

    return D3D_OK;

}

HRESULT D3D_CALL AIDeviceImp9 :: GetVertexShaderConstantF (  UINT StartRegister , float* pConstantData , UINT Vector4fCount ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetVertexShaderConstantF")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: SetVertexShaderConstantI (  UINT StartRegister , CONST int* pConstantData , UINT Vector4iCount ) 
{
    D3D9_CALL(true, "AIDeviceImp9::SetVertexShaderConstantI")
    
    AD3D9State::instance().setVertexShaderConstant(StartRegister, pConstantData, Vector4iCount);

    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetVertexShaderConstantI (  UINT StartRegister , int* pConstantData , UINT Vector4iCount ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetVertexShaderConstantI")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: SetVertexShaderConstantB (  UINT StartRegister , CONST BOOL* pConstantData , UINT BoolCount ) 
{
    D3D9_CALL(true, "AIDeviceImp9::SetVertexShaderConstantB")

    AD3D9State::instance().setVertexShaderConstantBool(StartRegister, pConstantData, BoolCount);

    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetVertexShaderConstantB (  UINT StartRegister , BOOL* pConstantData , UINT BoolCount ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetVertexShaderConstantB")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) 
{

    D3D9_CALL(true, "AIDeviceImp9::SetStreamSource")
    /*StateDataNode* s_stream = state->get_child(StateId("STREAM", StreamNumber));*/
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
    /*s_stream->get_child(StateId("SOURCE"))->write_data(&pStreamData);
    s_stream->get_child(StateId("OFFSET"))->write_data(&OffsetInBytes);
    s_stream->get_child(StateId("STRIDE"))->write_data(&Stride);*/

    AD3D9State::instance().setVertexBuffer((AIVertexBufferImp9*)pStreamData, StreamNumber, OffsetInBytes, Stride);

    /*StreamDescription tmpSD;

    tmpSD.pStreamData = (AIVertexBufferImp9*)pStreamData;
    tmpSD.OffsetInBytes = OffsetInBytes;
    tmpSD.Stride = Stride;

    streamsSetted[StreamNumber] = tmpSD;*/

    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetStreamSource (  UINT StreamNumber , IDirect3DVertexBuffer9** ppStreamData , UINT* pOffsetInBytes , UINT* pStride ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetStreamSource")
    /*D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9: GetStreamSource" << endl; ) )
    StateDataNode* s_stream = state->get_child(StateId("STREAM", StreamNumber));
    s_stream->get_child(StateId("SOURCE"))->read_data(ppStreamData);
    s_stream->get_child(StateId("OFFSET"))->read_data(pOffsetInBytes);
    s_stream->get_child(StateId("STRIDE"))->read_data(pStride);
    if(*ppStreamData != 0) (*ppStreamData)->AddRef();*/
    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: SetStreamSourceFreq (  UINT StreamNumber , UINT Setting ) 
{
    D3D9_CALL(true, "AIDeviceImp9::SetStreamSourceFreq")

    /*D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3DDevice9 :: SetStreamSourceFreq  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;*/

    D3D_DEBUG( cout << "IDEVICE9:  SetStreamSourceFreq" << endl; )
    D3D_DEBUG( cout << " * StreamNumber: " << StreamNumber << endl; )

    if (Setting & D3DSTREAMSOURCE_INDEXEDDATA) {

        D3D_DEBUG( cout << " * D3DSTREAMSOURCE_INDEXEDDATA" << endl; )
        D3D_DEBUG( cout << " * Instances: " << (Setting & ~D3DSTREAMSOURCE_INDEXEDDATA) << endl; )

        AD3D9State::instance().setFreqType(StreamNumber, D3DSTREAMSOURCE_INDEXEDDATA);
        AD3D9State::instance().setFreqValue(StreamNumber, (Setting & ~D3DSTREAMSOURCE_INDEXEDDATA));

    }
    else if (Setting & D3DSTREAMSOURCE_INSTANCEDATA) {

        D3D_DEBUG( cout << " * D3DSTREAMSOURCE_INSTANCEDATA" << endl; )
        D3D_DEBUG( cout << " * Increment: " << (Setting & ~D3DSTREAMSOURCE_INSTANCEDATA) << endl; )

        AD3D9State::instance().setFreqType(StreamNumber, D3DSTREAMSOURCE_INSTANCEDATA);
        AD3D9State::instance().setFreqValue(StreamNumber, (Setting & ~D3DSTREAMSOURCE_INSTANCEDATA));

    }
    else {

        AD3D9State::instance().setFreqType(StreamNumber, 0);
        AD3D9State::instance().setFreqValue(StreamNumber, Setting);

    }

    return D3D_OK;

}

HRESULT D3D_CALL AIDeviceImp9 :: GetStreamSourceFreq (  UINT StreamNumber , UINT* pSetting ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetStreamSourceFreq")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9::SetIndices(IDirect3DIndexBuffer9* pIndexData) 
{
    D3D9_CALL(true, "AIDeviceImp9::SetIndices")

    AD3D9State::instance().setIndexBuffer((AIIndexBufferImp9*)pIndexData);

    return D3D_OK;

}

HRESULT D3D_CALL AIDeviceImp9 :: GetIndices (  IDirect3DIndexBuffer9** ppIndexData ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetIndices")
    /*D3D_DEBUG( cout << "IDEVICE9:  GetIndices" << endl; )
    state->get_child(StateId("CURRENT_INDEX_BUFFER"))->read_data(ppIndexData);
    if(*ppIndexData != 0) (*ppIndexData)->AddRef();*/
    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: CreatePixelShader (  CONST DWORD* pFunction , IDirect3DPixelShader9** ppShader ) 
{

    D3D9_CALL(true, "AIDeviceImp9::CreatePixelShader")

    AIPixelShaderImp9* psh = new AIPixelShaderImp9(this, pFunction);

    psh->AddRef();

    i_pixel_shader_childs.insert(psh);

    * ppShader = psh;

    return D3D_OK;

}

HRESULT D3D_CALL AIDeviceImp9 :: SetPixelShader (  IDirect3DPixelShader9* pShader ) 
{
    D3D9_CALL(true, "AIDeviceImp9::SetPixelShader")

    AD3D9State::instance().setPixelShader((AIPixelShaderImp9*)pShader);

    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetPixelShader (  IDirect3DPixelShader9** ppShader ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetPixelShader")
    /*D3D_DEBUG( D3D_DEBUG( cout <<"IDEVICE9:  GetPixelShader" << endl; ) )
    state->get_child(StateId("CURRENT_PIXEL_SHADER"))->read_data(ppShader);
    if(*ppShader != 0) (*ppShader)->AddRef();*/
    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: SetPixelShaderConstantF (  UINT StartRegister , CONST float* pConstantData , UINT Vector4fCount ) 
{
    D3D9_CALL(true, "AIDeviceImp9::SetPixelShaderConstantF")

    AD3D9State::instance().setPixelShaderConstant(StartRegister, pConstantData, Vector4fCount);

    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetPixelShaderConstantF (  UINT StartRegister , float* pConstantData , UINT Vector4fCount ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetPixelShaderConstantF")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: SetPixelShaderConstantI (  UINT StartRegister , CONST int* pConstantData , UINT Vector4iCount ) 
{
    D3D9_CALL(true, "AIDeviceImp9::SetPixelShaderConstantI")

    AD3D9State::instance().setPixelShaderConstant(StartRegister, pConstantData, Vector4iCount);

    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetPixelShaderConstantI (  UINT StartRegister , int* pConstantData , UINT Vector4iCount ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetPixelShaderConstantI")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: SetPixelShaderConstantB (  UINT StartRegister , CONST BOOL* pConstantData , UINT BoolCount ) 
{
    D3D9_CALL(true, "AIDeviceImp9::SetPixelShaderConstantB")

    AD3D9State::instance().setPixelShaderConstantBool(StartRegister, pConstantData, BoolCount);

    return D3D_OK;
}

HRESULT D3D_CALL AIDeviceImp9 :: GetPixelShaderConstantB (  UINT StartRegister , BOOL* pConstantData , UINT BoolCount ) 
{
    D3D9_CALL(false, "AIDeviceImp9::GetPixelShaderConstantB")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: DrawRectPatch (  UINT Handle , CONST float* pNumSegs , CONST D3DRECTPATCH_INFO* pRectPatchInfo ) 
{
    D3D9_CALL(false, "AIDeviceImp9::DrawRectPatch")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: DrawTriPatch (  UINT Handle , CONST float* pNumSegs , CONST D3DTRIPATCH_INFO* pTriPatchInfo ) 
{
    D3D9_CALL(false, "AIDeviceImp9::DrawTriPatch")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: DeletePatch (  UINT Handle ) 
{
    D3D9_CALL(false, "AIDeviceImp9::DeletePatch")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDeviceImp9 :: CreateQuery (  D3DQUERYTYPE Type , IDirect3DQuery9** ppQuery ) 
{
    D3D9_CALL(false, "AIDeviceImp9::CreateQuery")
    * ppQuery = & AIQueryImp9 :: getInstance();
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

