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

/**************************************************
Classes for keep status of the "Substitute" D3D9
environment reflecting the status of the "Original"
(captured) D3D9 environment.

A call stored in trace was handled by a D3D9 object
that no longer exists, the "Original" object.
When replaying the call some object, the "Substitute"
object, handles the call instead of the "Original".

Also lock operations made over original objects
generate a original memory area whose contents must
be copied to the substitute memory area, perhaps
with another layout.

Author: José Manuel Solís
**************************************************/

#ifndef __D3D9STATUS
#define __D3D9STATUS

#include <map>
#include <stack>
#include "GPUTypes.h"
#include "PIXTypes.h"

/************************************************
Updates a substitute locked memory area
************************************************/
class D3D9LockUpdater {
public:
    /***********************************************
    Updates locked region. Maps original locked
    region to the substitute locked area.

    Buffer has the contents of locked memory in the
    original lock.
    ***********************************************/
    virtual void update(size_t buffer_size, void * buffer) = 0;
};

class D3D9SurfaceLockUpdater : public D3D9LockUpdater {
public:
    /*****************
     D3D9SubstituteLockedRegion methods
    *****************/
    void update(size_t buffer_size, void * buffer);

    /*****************
    Accesors
    *****************/

    void setDesc(D3DSURFACE_DESC *desc);
    void setSubstituteLockedRect(D3DLOCKED_RECT *lr);
    void setRect(RECT *r);

    D3D9SurfaceLockUpdater();
private:

    D3DSURFACE_DESC description;
    D3DLOCKED_RECT substituteLockedRect;

    bool fullLock;
    RECT rect;
};


class D3D9VolumeLockUpdater : public D3D9LockUpdater {
public:
    /*****************
     D3D9SubstituteLockedRegion methods
    *****************/
    void update(size_t buffer_size, void * buffer);

    /*****************
    Accesors
    *****************/

    void setDesc(D3DVOLUME_DESC *desc);
    void setSubstituteLockedBox(D3DLOCKED_BOX *lockedBox);
    void setBox(D3DBOX *box);

    D3D9VolumeLockUpdater();
private:

    D3DVOLUME_DESC description;
    D3DLOCKED_BOX substituteLockedBox;

    bool fullLock;
    D3DBOX box;
};

class D3D9IndexBufferLockUpdater: public D3D9LockUpdater {
public:
    /*****************
     D3D9LockUpdater methods
    *****************/
    void update(size_t buffer_size, void * buffer);

    /*****************
    Accessors
    *****************/
    void setDesc(D3DINDEXBUFFER_DESC *desc);
    void setSubstituteData(void *data);
    void setOffset(UINT off);
    void setSize(UINT sizeToLock);

    D3D9IndexBufferLockUpdater();

private:

    D3DINDEXBUFFER_DESC description;
    void *substituteData;

    UINT offset;
    UINT sizeToLock;
    
};

class D3D9VertexBufferLockUpdater: public D3D9LockUpdater {
public:
    /*****************
     D3D9LockUpdater methods
    *****************/
    void update(size_t buffer_size, void * buffer);
    
    /*****************
    Accessors
    *****************/
    void setDesc(D3DVERTEXBUFFER_DESC *desc);
    void setSubstituteData(void *data);
    void setOffset(UINT off);
    void setSize(UINT sizeToLock);

    D3D9VertexBufferLockUpdater();

private:

    D3DVERTEXBUFFER_DESC description;
    void *substituteData;

    UINT offset;
    UINT sizeToLock;
};

/*************************************************
Manages a D3D9 substitute device
*************************************************/
class D3D9SubstituteDeviceManager {
public:
    void setSubstituteWindow(HWND w);

    /***********************
    Do commands on device to present
    a batch operation.
    ************************/
    void presentBatch();
    
    /*************************
    Override presentation parameters (disable if override = false)
    **************************/
    void overrideWindowed(bool override, bool windowed);
    void overrideBackBufferCount(bool override, int backBufferCount);
    void overrideBackBufferDimensions(bool override, int width, int height);
    void overrideSwapEffect(bool override, D3DSWAPEFFECT swapEffect);

    /***************************
    Inform this object about a new substitute device
    ***************************/
    void onNewSubstituteDevice(IDirect3DDevice9 * d);

    /****************************
    Modify creational parameters of the device,
    substitute creational parameters can be
    specified in configuration.
    *****************************/
    void onCreateDevice(
        UINT &Adapter, D3DDEVTYPE &DeviceType,
        HWND &hFocusWindow, DWORD &BehaviorFlags, D3DPRESENT_PARAMETERS *&pp );

    /****************************
    Modify presentation parameters of the device,
    substitute presentation parameters can be
    specified in configuration.
    *****************************/
    void onDeviceReset(D3DPRESENT_PARAMETERS *&pp);

    D3D9SubstituteDeviceManager();
    
    IDirect3DDevice9 *getSubstituteDevice();    
    
    //
    //  Updates the substituted configurated parameters in D3D9SubstituteDeviceManager
    //  using the information provided by the D3D9Configuration class.
    //
    void updateConfiguration();
    
private:
    IDirect3DDevice9 *device;
    bool oWindowed;
    bool windowed;
    bool oBackBufferCount;
    int  backBufferCount;
    bool oBackBufferWidth;
    int  backBufferWidth;
    bool oBackBufferHeight;
    int  backBufferHeight;
    bool oSwapEffect;
    D3DSWAPEFFECT swapEffect;
    bool oMultiSampleType;
    D3DMULTISAMPLE_TYPE multiSampleType;
    bool oMultiSampleQuality;
    DWORD multiSampleQuality;
    bool oFullScreen_RefreshRateInHz;
    UINT fullScreen_RefreshRateInHz;
    bool oPresentationInterval;
    UINT presentationInterval;


    bool oDeviceType;
    D3DDEVTYPE deviceType;
    bool oBehaviorFlags;
    DWORD behaviorFlags;

    /********************************
    Modify presentation parameters to
    match configuration and
    resize windows if needed.
    ********************************/
    void onNewPresentationParameters(D3DPRESENT_PARAMETERS *&pp);

    HWND window;

    void resizeWindow(int width, int height);
};


/*************************************************
Singleton class that manages substitute objects.
**************************************************/
class D3D9Status {
public:
    /******************************************
    Set this object to its initial state
    ******************************************/
    void reset();

    /******************************************
    Management of substitute objects
    *******************************************/
    void setSubstitute(PIXPointer original, void *substitute);
    void *getSubstitute(PIXPointer original);
    void removeSubstitute(PIXPointer original);

    /************************
    Management of lock operations
    ************************/
    void lockSurface(void *original, D3DSURFACE_DESC * desc, D3DLOCKED_RECT *lockedRect, RECT *pRect);

    void lockVolume (void *original, D3DVOLUME_DESC * desc, D3DLOCKED_BOX *lockedBox, D3DBOX *pBox);

    void lockVertexBuffer(void *original, D3DVERTEXBUFFER_DESC * desc, UINT offsetToLock, UINT sizeToLock, void *sdata);

    void lockIndexBuffer(void *original, D3DINDEXBUFFER_DESC * desc, UINT offsetToLock, UINT sizeToLock, void *sdata);

    void unlock(void *original, size_t bufferSize, void *buffer);

    void addLockUpdater(void *substitute);

    /***************************
    Sets for substitute window handle
    ****************************/
    void setMainSubstituteWindow(HWND w);

    /***************************
    Inform status about a new substitute device
    ****************************/
    void newMainSubstituteDevice(IDirect3DDevice9 *device);
    
    /**
     *
     *  Get the subtstitue Direct3D device.
     *
     */

    IDirect3DDevice9 *getSubstituteDevice();     

    /*****************************
    Modify the device's creation parameters to
    suit our needs, i. e. use less
    resolution. Substitute parameters can
    be specified in configuration file.
    ******************************/
    void onCreateDevice(
        UINT &Adapter, D3DDEVTYPE &DeviceType,
        HWND &hFocusWindow, DWORD &BehaviorFlags, D3DPRESENT_PARAMETERS *&pp );

    /*****************************
    Modify the device's presentation parameters to
    suit our needs, i. e. use less
    resolution. Substitute parameters can
    be specified in configuration file.
    ******************************/
    void onDeviceReset( D3DPRESENT_PARAMETERS *&pp );


    /*****************************
    Do a serie of commands in main substitute
    device to present the results of a drawing
    operation.
    ******************************/
    void mainSubstituteDevicePresentBatch();

    //
    //  Updates the substituted configurated parameters in D3D9SubstituteDeviceManager
    //  using the information provided by the D3D9Configuration class.
    //
    void updateConfiguration();
    
    //
    //  Check if hack for ATI float point depth buffers is enabled.
    // 
    bool isEnabledATIFPDepthBufferHack();
    
    //
    //  Check if the hack to continue on NULL interfaces is enabled.
    //
    bool isEnabledContinueOnNULLHack();
    
    //
    //  Check if the hack to replace the ATI2 surface format is enabled.
    //
    bool isEnabledATI2FormatHack();

    D3D9Status();
    ~D3D9Status();

private:

    std::map <PIXPointer, void *> substitutes;
    std::map <void *, std::stack<D3D9LockUpdater *> * > lockUpdaters;

    HWND substituteWindow;

    bool enableATIFPDepthBufferHack;
    bool continueOnNULLHack;
    bool replaceATI2Format;
         
    D3D9SubstituteDeviceManager substituteDeviceManager;
};

#endif
