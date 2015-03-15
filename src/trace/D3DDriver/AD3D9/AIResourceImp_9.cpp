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
#include "AIResourceImp_9.h"

AIResourceImp9 :: AIResourceImp9() {}

AIResourceImp9 & AIResourceImp9 :: getInstance() {
    static AIResourceImp9 instance;
    return instance;
}

HRESULT D3D_CALL AIResourceImp9 :: QueryInterface (  REFIID riid , void** ppvObj ) 
{
    D3D9_CALL(false, "AIResourceImp9::QueryInterface")
    * ppvObj = cover_buffer_9;
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL AIResourceImp9 :: AddRef ( ) 
{
    D3D9_CALL(false, "AIResourceImp9::AddRef")
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

ULONG D3D_CALL AIResourceImp9 :: Release ( ) 
{
    D3D9_CALL(false, "AIResourceImp9::Release")
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

HRESULT D3D_CALL AIResourceImp9 :: GetDevice (  IDirect3DDevice9** ppDevice ) 
{
    D3D9_CALL(false, "AIResourceImp9::GetDevice")
    D3D_DEBUG( cout <<"WARNING:  IDirect3DResource9 :: GetDevice  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIResourceImp9 :: SetPrivateData (  REFGUID refguid , CONST void* pData , DWORD SizeOfData , DWORD Flags ) 
{
    D3D9_CALL(false, "AIResourceImp9::SetPrivateData")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIResourceImp9 :: GetPrivateData (  REFGUID refguid , void* pData , DWORD* pSizeOfData ) 
{
    D3D9_CALL(false, "AIResourceImp9::GetPrivateData")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIResourceImp9 :: FreePrivateData (  REFGUID refguid ) 
{
    D3D9_CALL(false, "AIResourceImp9::FreePrivateData")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

DWORD D3D_CALL AIResourceImp9 :: SetPriority (  DWORD PriorityNew ) 
{
    D3D9_CALL(false, "AIResourceImp9::SetPriority")
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL AIResourceImp9 :: GetPriority ( ) 
{
    D3D9_CALL(false, "AIResourceImp9::GetPriority")
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

void D3D_CALL AIResourceImp9 :: PreLoad ( ) 
{
    D3D9_CALL(false, "AIResourceImp9::PreLoad")
}

D3DRESOURCETYPE D3D_CALL AIResourceImp9 :: GetType ( ) 
{
    D3D9_CALL(false, "AIResourceImp9::GetType")
    D3DRESOURCETYPE ret = static_cast< D3DRESOURCETYPE >(0);
    return ret;
}

