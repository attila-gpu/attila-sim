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

#ifndef AIVERTEXBUFFERIMP_9_H_INCLUDED
#define AIVERTEXBUFFERIMP_9_H_INCLUDED

#include <list>

class AIDeviceImp9;

class AIVertexBufferImp9 : public IDirect3DVertexBuffer9 {

public:
   /// Singleton method, maintained to allow unimplemented methods to return valid interface addresses.
    static AIVertexBufferImp9 &getInstance();

    AIVertexBufferImp9(AIDeviceImp9* i_parent, UINT Length , DWORD Usage , DWORD FVF , D3DPOOL Pool);

private:
    /// Singleton constructor method
    AIVertexBufferImp9();

    AIDeviceImp9* i_parent;

    UINT Length;
    DWORD Usage;
    DWORD FVF;
    DWORD Pool;

    //BYTE* data;

    UINT OffsetToLock;
    UINT SizeToLock;

    //BYTE* lockedData;

    ULONG refs;

    std::list<unsigned char*> lockedData;

    acdlib::ACDBuffer* acdVertexBuffer;

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
    HRESULT D3D_CALL GetDesc(D3DVERTEXBUFFER_DESC * pDesc);

    acdlib::ACDBuffer* getAcdVertexBuffer();

};

#endif

