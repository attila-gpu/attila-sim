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
#include "AIDeviceImp_9.h"
#include "AIStateBlockImp_9.h"

AIStateBlockImp9::AIStateBlockImp9()
{
    parentDevice = 0;
}

AIStateBlockImp9::AIStateBlockImp9(AIDeviceImp9 *device, map<D3DRENDERSTATETYPE, DWORD> renderState)
{
    parentDevice = device;
    blockRenderState = renderState;
    referenceCount = 0;
}

AIStateBlockImp9 &AIStateBlockImp9::getInstance() 
{
    static AIStateBlockImp9 instance;
    return instance;
}

HRESULT D3D_CALL AIStateBlockImp9::QueryInterface(REFIID riid , void** ppvObj) 
{
    D3D9_CALL(false, "AIStateBlockImp9::QueryInterface")
    * ppvObj = cover_buffer_9;
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL AIStateBlockImp9::AddRef()
{
    D3D9_CALL(true, "AIStateBlockImp9::AddRef")

    if (parentDevice != 0) {
        referenceCount ++;
        return referenceCount;
    }
    else
    {
        // Object is used as singleton "cover"
        return 0;
    }
}

ULONG D3D_CALL AIStateBlockImp9::Release() 
{
    D3D9_CALL(true, "AIStateBlockImp9::Release")

    if (parentDevice != 0)
    {
        referenceCount--;
        return referenceCount;
    }
    else
    {
        // Object is used as singleton "cover"
        return 0;
    }
}

HRESULT D3D_CALL AIStateBlockImp9 :: GetDevice (  IDirect3DDevice9** ppDevice ) 
{
    D3D9_CALL(false, "AIStateBlockImp9::GetDevice")
    * ppDevice = & AIDeviceImp9 :: getInstance();
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIStateBlockImp9::Capture()
{
    D3D9_CALL(true, "AIStateBlockImp9::Capture")
    
    parentDevice->captureRenderState(blockRenderState);
    
    return D3D_OK;
}

HRESULT D3D_CALL AIStateBlockImp9::Apply() 
{
    D3D9_CALL(true, "AIStateBlockImp9::Apply")

    map<D3DRENDERSTATETYPE, DWORD>::iterator it;
    
    it = blockRenderState.begin();
    while (it != blockRenderState.end())
    {
        parentDevice->SetRenderState(it->first, it->second);
        it++;
    }
    
    return D3D_OK;
}

