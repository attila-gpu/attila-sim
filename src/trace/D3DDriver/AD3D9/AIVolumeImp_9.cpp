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
#include "AIVolumeImp_9.h"
#include "AIVolumeTextureImp_9.h"
#include "../D3DControllers/Utils.h"

#include "AD3D9State.h"

AIVolumeImp9::AIVolumeImp9(AIDeviceImp9* _i_parent, UINT _Width, UINT _Height, UINT _Depth,
                           DWORD _Usage , D3DFORMAT _Format, D3DPOOL _Pool) :

 i_parent(_i_parent), Width(_Width), Height(_Height), Depth(_Depth),
 Usage(_Usage) , Format(_Format), Pool(_Pool)
                           
{
    lockedData = NULL;

    ref_count = 0;

    volumeTexture = NULL;
}

AIVolumeImp9 :: AIVolumeImp9() 
{
    ///@note used to differentiate when creating singleton
    i_parent = 0;
}

AIVolumeImp9 & AIVolumeImp9 :: getInstance() 
{
    static AIVolumeImp9 instance;
    return instance;
}

void AIVolumeImp9::setVolumeTexture(AIVolumeTextureImp9* tex) 
{
    volumeTexture = tex;
}

void AIVolumeImp9::setMipMapLevel(UINT m) 
{
    mipmap = m;
}

HRESULT D3D_CALL AIVolumeImp9 :: QueryInterface (  REFIID riid , void** ppvObj ) 
{
    D3D9_CALL(false, "AIVolumeImp9::QueryInterface")
    * ppvObj = cover_buffer_9;
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL AIVolumeImp9 :: AddRef ( ) 
{
    /*D3D_DEBUG( cout <<"WARNING:  IDirect3DVolume9 :: AddRef  NOT IMPLEMENTED" << endl; )
    ULONG ret = static_cast< ULONG >(0);
    return ret;*/
    D3D9_CALL(true, "AIVolumeImp9::AddRef")

    if(i_parent != 0) 
    {
        ref_count ++;
        return ref_count;
    }
    else 
    {
        // Object is used as singleton "cover"
        return 0;
    }
}

ULONG D3D_CALL AIVolumeImp9 :: Release ( ) 
{
    /*D3D_DEBUG( cout <<"WARNING:  IDirect3DVolume9 :: Release  NOT IMPLEMENTED" << endl; )
    ULONG ret = static_cast< ULONG >(0);
    return ret;*/
    D3D9_CALL(true, "AIVolumeImp9::Release")

    if(i_parent != 0)
    {
        ref_count--;
        /*if(ref_count == 0) {
            // Remove state
            StateDataNode* parent = state->get_parent();
            parent->remove_child(state);
            delete state;
            state = 0;
        }*/
        return ref_count;
    }
    else 
    {
        // Object is used as singleton "cover"
        return 0;
    }
}

HRESULT D3D_CALL AIVolumeImp9 :: GetDevice (  IDirect3DDevice9** ppDevice ) 
{
    ///@note Depending on the constructor used, parent can be null.
    D3D9_CALL(false, "AIVolumeImp9::GetDevice")
    *ppDevice = (i_parent != 0)? i_parent : &AIDeviceImp9::getInstance();
    return D3D_OK;
}

HRESULT D3D_CALL AIVolumeImp9 :: SetPrivateData (  REFGUID refguid , CONST void* pData , DWORD SizeOfData , DWORD Flags ) 
{
    D3D9_CALL(false, "AIVolumeImp9::SetPrivateData")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIVolumeImp9 :: GetPrivateData (  REFGUID refguid , void* pData , DWORD* pSizeOfData ) 
{
    D3D9_CALL(false, "AIVolumeImp9::GetPrivateData")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIVolumeImp9 :: FreePrivateData (  REFGUID refguid ) 
{
    D3D9_CALL(false, "AIVolumeImp9::FreePrivateData")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIVolumeImp9 :: GetContainer (  REFIID riid , void** ppContainer ) 
{
    D3D9_CALL(false, "AIVolumeImp9::GetContainer")
    * ppContainer = cover_buffer_9;
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIVolumeImp9 :: GetDesc (  D3DVOLUME_DESC * pDesc )
{
    D3D9_CALL(true, "AIVolumeImp9::GetDesc")

    pDesc->Width = Width;
    pDesc->Height = Height;
    pDesc->Depth = Depth;
    pDesc->Pool = Pool;
    pDesc->Type = D3DRTYPE_VOLUME;
    pDesc->Usage = Usage;
    pDesc->Format = Format;

    return D3D_OK;
}

HRESULT D3D_CALL AIVolumeImp9 :: LockBox (  D3DLOCKED_BOX * pLockedVolume , CONST D3DBOX* pBox , DWORD Flags ) 
{
    D3D9_CALL(true, "AIVolumeImp9::LockBox")
    // Surface is being used as a cover, so do nothing
    /*if(i_parent == 0) return D3D_OK;

    D3D_DEBUG( cout << "IVOLUME9<" << this << ">: LockVolume" << endl; )

    ///@todo Study possible problems of discard or readonly flags

    if(Flags & D3DLOCK_DISCARD) {
        D3D_DEBUG( cout << "WARNING: D3DLOCK_DISCARD NOT IMPLEMENTED" << endl; )
    }

    UINT lock_width;
    UINT lock_height;
    UINT lock_depth;
    UINT lock_offset_x;
    UINT lock_offset_y;
    UINT lock_offset_z;


    if (pBox == 0) {
        // Remember a pBox of 0 means lock entire volume.
        state->get_child(StateId("WIDTH"))->read_data(&lock_width);
        state->get_child(StateId("HEIGHT"))->read_data(&lock_height);
        state->get_child(StateId("DEPTH"))->read_data(&lock_depth);
        lock_offset_x = 0;
        lock_offset_y = 0;
        lock_offset_z = 0;
    }
    else {
        lock_width = pBox->Right - pBox->Left;
        lock_height = pBox->Bottom - pBox->Top;
        lock_depth = pBox->Back - pBox->Front;
        lock_offset_x = pBox->Left;
        lock_offset_y = pBox->Top;
        lock_offset_z = pBox->Front;
    }

    // Update lock state
    StateDataNode* lock_state = D3DState::create_lockbox_state_9();
    lock_state->get_child(StateId("LEFT"))->write_data(&lock_offset_x);
    lock_state->get_child(StateId("TOP"))->write_data(&lock_offset_y);
    lock_state->get_child(StateId("FRONT"))->write_data(&lock_offset_z);
    LONG lock_right = lock_offset_x + lock_width;
    lock_state->get_child(StateId("RIGHT"))->write_data(&lock_right);
    LONG lock_bottom = lock_offset_y + lock_height;
    lock_state->get_child(StateId("BOTTOM"))->write_data(&lock_bottom);
    LONG lock_back = lock_offset_z + lock_depth;
    lock_state->get_child(StateId("BACK"))->write_data(&lock_back);
    lock_state->get_child(StateId("FLAGS"))->write_data(&Flags);

    // add lock state
    state->add_child(lock_state);*/

    /* Now controllers layer has reserved memory in DATA node
       and has written ROW_PITCH and SLICE_PITCH. */

    /*lock_state->get_child(StateId("ROW_PITCH"))->read_data(&pLockedVolume->RowPitch);
    lock_state->get_child(StateId("SLICE_PITCH"))->read_data(&pLockedVolume->SlicePitch);
    // Map data node to allow user application to modify the contents
    StateDataNode* data_node = lock_state->get_child(StateId("DATA"));
    if(Flags & D3DLOCK_READONLY) {
        pLockedVolume->pBits = data_node->map_data(READ_ACCESS);
    }
    else {
        pLockedVolume->pBits = data_node->map_data(WRITE_ACCESS);
    }*/

    // Temporal stub
    /*pLockedVolume->RowPitch = 1;
    pLockedVolume->SlicePitch = 1;
    pLockedVolume->pBits = cover_buffer_9;*/

    
    // Surface is being used as a cover, so do nothing
    if(i_parent == 0) return D3D_OK;

    D3D_DEBUG( cout << "IVOLUME9<" << this << ">: LockRect" << endl; )

    LONG w, h, d;

    if (pBox == NULL) {
        leftLock = 0;
        topLock = 0;
        frontLock = 0;
        rightLock = Width;
        bottomLock = Height;
        backLock = Depth;
        w = Width;
        h = Height;
        d = Depth;
    }
    else {
        leftLock = pBox->Left;
        topLock = pBox->Top;
        frontLock = pBox->Front;
        rightLock = pBox->Right;
        bottomLock = pBox->Bottom;
        backLock = pBox->Back;
        w = rightLock - leftLock;
        h = bottomLock - topLock;
        d = backLock - frontLock;
    }

    if (lockedData != NULL) 
        delete[] lockedData;
    
    UINT actual_lock_size;

    actual_lock_size = getVolumeSize(w, h, d, Format);

    lockedData = new unsigned char[actual_lock_size];

    UINT mipPitch, volPitch;
    
    mipPitch = Width * texel_size(Format);

    volPitch = Width * Height * texel_size(Format);

    lockedPitch = getSurfacePitch(w, Format);
    slicePitch =  Width * Height * texel_size(Format);

    pLockedVolume->pBits = (void*)lockedData;
    pLockedVolume->RowPitch = lockedPitch;
    pLockedVolume->SlicePitch = slicePitch;

    D3D_DEBUG( cout << "ISURFACE9<" << this << ">: LockRect: Width: " << Width << endl; )
    D3D_DEBUG( cout << "ISURFACE9<" << this << ">: LockRect: Height: " << Height << endl; )
    D3D_DEBUG( cout << "ISURFACE9<" << this << ">: LockRect: lockedPitch: " << lockedPitch << endl; )
    D3D_DEBUG( cout << "ISURFACE9<" << this << ">: LockRect: actual_lock_size: " << actual_lock_size << endl; )
    D3D_DEBUG( cout << "ISURFACE9<" << this << ">: LockRect: mipPitch: " << mipPitch << endl; )
    D3D_DEBUG( cout << "ISURFACE9<" << this << ">: LockRect: Left: " << leftLock << endl; )
    D3D_DEBUG( cout << "ISURFACE9<" << this << ">: LockRect: Top: " << topLock << endl; )
    D3D_DEBUG( cout << "ISURFACE9<" << this << ">: LockRect: Right: " << rightLock << endl; )
    D3D_DEBUG( cout << "ISURFACE9<" << this << ">: LockRect: Bottom: " << bottomLock << endl; )

    return D3D_OK;
}

HRESULT D3D_CALL AIVolumeImp9 :: UnlockBox ( ) 
{
    D3D9_CALL(true, "AIVolumeImp9::UnlockBox")
     // Surface is being used as a cover, so do nothing
    /*if(i_parent == 0) return D3D_OK;

    D3D_DEBUG( cout << "IVOLUME9<" << this << ">: UnlockBox" << endl; )

    // Application has finished accessing data.
    StateDataNode* lock_state = state->get_child(StateId("LOCKBOX"));
    lock_state->get_child(StateId("DATA"))->unmap_data();

    // Remove lock state
    state->remove_child(lock_state);*/

    /** @todo Lock State is not deleted! At this point of
        development deletion policy is not decided. Revisit this code then. */

    /*D3DBOX lockBox;

    lockBox.Top = topLock;
    lockBox.Bottom = bottomLock;
    lockBox.Left = leftLock;
    lockBox.Right = rightLock;
    lockBox.Front = frontLock;
    lockBox.Back = backLock;*/

    //AD3D9State::instance().updateVolume(this, &lockBox, Format, lockedData);

    volumeTexture->getACDVolumeTexture()->updateData(mipmap, leftLock, topLock, frontLock,
                                                    (rightLock - leftLock), (bottomLock - topLock),
                                                    (backLock - frontLock), AD3D9State::instance().getACDFormat(Format),
                                                    getSurfacePitchACD((rightLock - leftLock), Format),
                                                    (acdlib::acd_ubyte*) lockedData,
                                                    AD3D9State::instance().isPreloadCall());

    //AD3D9State::instance().updateSurface(this, &lockRect, Format, lockedData);
    
    delete[] lockedData;
    lockedData = NULL;

    return D3D_OK;

}


DWORD AIVolumeImp9 :: SetPriority(DWORD) 
{
    D3D9_CALL(false, "AIVolumeImp9::SetPriority")
    return static_cast<DWORD>(0);
}

DWORD AIVolumeImp9 :: GetPriority() 
{
    D3D9_CALL(false, "AIVolumeImp9::GetPriority")
    return static_cast<DWORD>(0);
}

void AIVolumeImp9 :: PreLoad() 
{
    D3D9_CALL(false, "AIVolumeImp9::PreLoad")
}

D3DRESOURCETYPE AIVolumeImp9 :: GetType() 
{
    D3D9_CALL(false, "AIVolumeImp9::GetType")
    return static_cast<D3DRESOURCETYPE>(0);
}

