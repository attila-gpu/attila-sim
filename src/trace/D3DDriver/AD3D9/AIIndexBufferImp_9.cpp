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
#include "AIIndexBufferImp_9.h"

#include "AD3D9State.h"

AIIndexBufferImp9::AIIndexBufferImp9(AIDeviceImp9* _i_parent, UINT _Length , DWORD _Usage , DWORD _Format, D3DPOOL _Pool):
i_parent(_i_parent), 
Length(_Length), 
Usage(_Usage), 
Format(_Format), 
Pool(_Pool) 
{
    
    // Create a new ACD Buffer
    acdIndexBuffer = AD3D9State::instance().createBuffer(Length);

    refs = 0;
}

AIIndexBufferImp9::AIIndexBufferImp9() 
{
	// Used to diferentiate when creating singleton cover object
	i_parent = 0;
}

AIIndexBufferImp9& AIIndexBufferImp9::getInstance() 
{
    static AIIndexBufferImp9 instance;
    return instance;
}

HRESULT D3D_CALL AIIndexBufferImp9::QueryInterface(REFIID riid, void** ppvObj) 
{
    D3D9_CALL(false, "AIIndexBufferImp9::QueryInterface")
    * ppvObj = cover_buffer_9;
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL AIIndexBufferImp9::AddRef() 
{
    D3D9_CALL(true, "AIIndexBufferImp9::AddRef")
	if(i_parent != 0) {
		refs ++;
    	return refs;
	}
	else return 0;
}

ULONG D3D_CALL AIIndexBufferImp9::Release() 
{
    D3D9_CALL(true, "AIIndexBufferImp9::Release")
    if(i_parent != 0) 
    {
        refs--;
        /*if(refs == 0) {
            // Remove state
            StateDataNode* parent = state->get_parent();
           parent->remove_child(state);
           delete state;
            state = 0;
        }*/

        return refs;
    }
    else {
        // Object is used as singleton "cover"
        return 0;
    }

}

HRESULT D3D_CALL AIIndexBufferImp9::GetDevice(IDirect3DDevice9** ppDevice) 
{
    D3D9_CALL(false, "AIIndexBufferImp9::GetDevice")

    * ppDevice = i_parent;
    return D3D_OK;
}

HRESULT D3D_CALL AIIndexBufferImp9::SetPrivateData(REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags) 
{
    D3D9_CALL(false, "AIIndexBufferImp9::SetPrivateData")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIIndexBufferImp9::GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData) 
{
    D3D9_CALL(false, "AIIndexBufferImp9::GetPrivateData")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIIndexBufferImp9::FreePrivateData(REFGUID refguid) 
{
    D3D9_CALL(false, "AIIndexBufferImp9::FreePrivateData")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

DWORD D3D_CALL AIIndexBufferImp9::SetPriority(DWORD PriorityNew) 
{
    D3D9_CALL(false, "AIIndexBufferImp9::SetPriority")
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL AIIndexBufferImp9::GetPriority() 
{
    D3D9_CALL(false, "AIIndexBufferImp9::GetPriority")
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

void D3D_CALL AIIndexBufferImp9::PreLoad() 
{
    D3D9_CALL(false, "AIIndexBufferImp9::PreLoad")
}

D3DRESOURCETYPE D3D_CALL AIIndexBufferImp9::GetType() 
{
    D3D9_CALL(false, "AIIndexBufferImp9::GetType")
    D3DRESOURCETYPE ret = static_cast< D3DRESOURCETYPE >(0);
    return ret;
}

HRESULT D3D_CALL AIIndexBufferImp9::Lock(UINT _OffsetToLock, UINT _SizeToLock, void** ppbData, DWORD Flags) 
{

    D3D9_CALL(true, "AIIndexBufferImp9::Lock")

    OffsetToLock = _OffsetToLock;
    SizeToLock = _SizeToLock;

    UINT actual_lock_size;
    if (SizeToLock == 0) 
    {
        // Remember a SizeToLock of 0 means lock entire vertex buffer.
        actual_lock_size = Length;
    }
    else 
    {
        actual_lock_size = SizeToLock;
    }

    BYTE* lData = new unsigned char[actual_lock_size];

    /*if (lockedData != NULL) 
        delete[] lockedData;*/

    //lockedData = new unsigned char[actual_lock_size];
    //memcpy(lockedData, (data + OffsetToLock), actual_lock_size);


    *ppbData = (void*)(lData);

    lockedData.push_back(lData);

    return D3D_OK;

}

HRESULT D3D_CALL AIIndexBufferImp9::Unlock() 
{

    D3D9_CALL(true, "AIIndexBufferImp9::Unlock")

    UINT actual_lock_size;

    if (SizeToLock == 0) 
    {
        // Remember a SizeToLock of 0 means lock entire vertex buffer.
        actual_lock_size = Length;
    }
    else 
    {
        actual_lock_size = SizeToLock;
    }

    //AD3D9State::instance().updateIndexBuffer(this, OffsetToLock, actual_lock_size, lockedData);


    BYTE* lData = lockedData.back();
    lockedData.pop_back();
    
    // Update index buffer data
    acdIndexBuffer->updateData(OffsetToLock, actual_lock_size, (acdlib::acd_ubyte*) lData, AD3D9State::instance().isPreloadCall());

    delete[] lData;
    //lockedData = NULL;
 
    //memcpy((data + OffsetToLock), lockedData, actual_lock_size);
    /*D3D_DEBUG( cout << "IINDEXBUFFER9: Buffer value " << endl; )
    for (int i = 0; i < Length; i++) {
        D3D_DEBUG( cout << (int)data[i] << " "; )
    }

    D3D_DEBUG( cout << endl; )*/


    return D3D_OK;
}

HRESULT D3D_CALL AIIndexBufferImp9::GetDesc(D3DINDEXBUFFER_DESC * pDesc) 
{
    /** @note This "Get" method is necessary for the d3d player, because it doesn't keep
        track of indexbuffers size. */

    D3D9_CALL(true, "AIIndexBufferImp9::GetDesc")

    pDesc->Size = Length;
    pDesc->Format = (D3DFORMAT)Format;

    D3D_DEBUG( cout << "IINDEXBUFFER9: Size is " << pDesc->Size << endl; )

    return D3D_OK;

}

acdlib::ACDBuffer* AIIndexBufferImp9::getAcdIndexBuffer() 
{
    return acdIndexBuffer;
}
