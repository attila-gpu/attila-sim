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
#include "AIBaseTextureImp_9.h"

AIBaseTextureImp9 :: AIBaseTextureImp9() {}

AIBaseTextureImp9 & AIBaseTextureImp9 :: getInstance() 
{
    static AIBaseTextureImp9 instance;
    return instance;
}

HRESULT D3D_CALL AIBaseTextureImp9 :: QueryInterface (  REFIID riid , void** ppvObj ) 
{
    D3D9_CALL(false, "AIBaseTextureImp9::QueryInterface")
    * ppvObj = cover_buffer_9;
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL AIBaseTextureImp9 :: AddRef ( ) 
{
    D3D9_CALL(false, "AIBaseTextureImp9::AddRef")    
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

ULONG D3D_CALL AIBaseTextureImp9 :: Release ( ) 
{
    D3D9_CALL(false, "AIBaseTextureImp9::Release")    
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

HRESULT D3D_CALL AIBaseTextureImp9 :: GetDevice (  IDirect3DDevice9** ppDevice ) 
{
    * ppDevice = & AIDeviceImp9 :: getInstance();
    D3D9_CALL(false, "AIBaseTextureImp9::GetDevice")    
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIBaseTextureImp9 :: SetPrivateData (  REFGUID refguid , CONST void* pData , DWORD SizeOfData , DWORD Flags ) 
{
    D3D9_CALL(false, "AIBaseTextureImp9::SetPrivateData")    
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIBaseTextureImp9 :: GetPrivateData (  REFGUID refguid , void* pData , DWORD* pSizeOfData ) 
{
    D3D9_CALL(false, "AIBaseTextureImp9::GetPrivateData")    
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIBaseTextureImp9 :: FreePrivateData (  REFGUID refguid ) 
{
    D3D9_CALL(false, "AIBaseTextureImp9::FreePrivateData")    
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

DWORD D3D_CALL AIBaseTextureImp9 :: SetPriority (  DWORD PriorityNew ) 
{
    D3D9_CALL(false, "AIBaseTextureImp9::SetPriority")    
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL AIBaseTextureImp9 :: GetPriority ( ) 
{
    D3D9_CALL(false, "AIBaseTextureImp9::GetPriority")    
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

void D3D_CALL AIBaseTextureImp9 :: PreLoad ( ) 
{
    D3D9_CALL(false, "AIBaseTextureImp9::PreLoad")    
}

D3DRESOURCETYPE D3D_CALL AIBaseTextureImp9 :: GetType ( ) 
{
    D3D9_CALL(false, "AIBaseTextureImp9::GetType")    
    D3DRESOURCETYPE ret = static_cast< D3DRESOURCETYPE >(0);
    return ret;
}

DWORD D3D_CALL AIBaseTextureImp9 :: SetLOD (  DWORD LODNew ) 
{
    D3D9_CALL(false, "AIBaseTextureImp9::SetLOD")    
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL AIBaseTextureImp9 :: GetLOD ( ) 
{
    D3D9_CALL(false, "AIBaseTextureImp9::GetLOD")    
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL AIBaseTextureImp9 :: GetLevelCount ( ) 
{
    D3D9_CALL(false, "AIBaseTextureImp9::GetLevelCount")    
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

HRESULT D3D_CALL AIBaseTextureImp9 :: SetAutoGenFilterType (  D3DTEXTUREFILTERTYPE FilterType ) 
{
    D3D9_CALL(false, "AIBaseTextureImp9::SetAutoGenFilterType")    
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

D3DTEXTUREFILTERTYPE D3D_CALL AIBaseTextureImp9 :: GetAutoGenFilterType ( ) 
{
    D3D9_CALL(false, "AIBaseTextureImp9::GetAutoGenFilterType")    
    D3DTEXTUREFILTERTYPE ret = static_cast< D3DTEXTUREFILTERTYPE >(0);
    return ret;
}

void D3D_CALL AIBaseTextureImp9 :: GenerateMipSubLevels ( ) 
{
    D3D9_CALL(false, "AIBaseTextureImp9::GenerateMipSubLevels")    
}

