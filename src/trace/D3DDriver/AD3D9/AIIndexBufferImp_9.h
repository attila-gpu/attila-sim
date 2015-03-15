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

#ifndef AIINDEXBUFFERIMP_9_H_INCLUDED
#define AIINDEXBUFFERIMP_9_H_INCLUDED

#include <list>
#include "ACDBuffer.h"
class AIDeviceImp9;

class AIIndexBufferImp9 : public IDirect3DIndexBuffer9 {

public:
    /// Singleton method, maintained to allow unimplemented methods to return valid interface addresses.
    static AIIndexBufferImp9 &getInstance();

    AIIndexBufferImp9(AIDeviceImp9* i_parent, UINT Length , DWORD Usage , DWORD Format, D3DPOOL Pool);

private:
    /// Singleton constructor method
    AIIndexBufferImp9();

    AIDeviceImp9* i_parent;

    UINT Length;
    DWORD Usage;
    DWORD Format;
    D3DPOOL Pool;

    UINT OffsetToLock;
    UINT SizeToLock;

    //BYTE* lockedData;

    ULONG refs;

    std::list<unsigned char*> lockedData;

    acdlib::ACDBuffer* acdIndexBuffer;

public:
    HRESULT D3D_CALL QueryInterface(REFIID riid, void** ppvObj);
    ULONG D3D_CALL AddRef();
    ULONG D3D_CALL Release();
    HRESULT D3D_CALL GetDevice(IDirect3DDevice9** ppDevice);
    HRESULT D3D_CALL SetPrivateData(REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags);
    HRESULT D3D_CALL GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData);
    HRESULT D3D_CALL FreePrivateData(REFGUID refguid);
    DWORD D3D_CALL SetPriority(DWORD PriorityNew);
    DWORD D3D_CALL GetPriority();
    void D3D_CALL PreLoad();
    D3DRESOURCETYPE D3D_CALL GetType();
    HRESULT D3D_CALL Lock(UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags);
    HRESULT D3D_CALL Unlock();
    HRESULT D3D_CALL GetDesc(D3DINDEXBUFFER_DESC * pDesc);

    acdlib::ACDBuffer* getAcdIndexBuffer();

};

#endif
