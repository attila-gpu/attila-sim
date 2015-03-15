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

#include "stdafx.h"

#include "D3D9Info.h"
#include "IniFile.h"
#include "D3DConfiguration.h"
#include "D3D9Status.h"
#include "IncludeLog.h"

#include <cstdlib>
#include <cstring>

using namespace std;

// TEST
//#define NO_UNLOCK_OPERATIONS


void D3D9VolumeLockUpdater::update(size_t buffer_size, void * buffer) {

    // Store in char * for simplify addressing
    char *original = static_cast<char *>(buffer);
    char *substitute = static_cast<char *>(substituteLockedBox.pBits);

    // Calculate dimensions
    UINT width =  fullLock ? description.Width : (box.Right - box.Left);
    UINT height = fullLock ? description.Height : (box.Bottom - box.Top);
    UINT depth =  fullLock ? description.Depth : (box.Back - box.Front);

    if(D3D9Info::instance().isCompressed(description.Format)) {
        // Compressed formats hold 16 pixels per texel (4x4) and dimensions
        // are in pixels, we have to translate to dimensions in texels
        width = (( width % 4 ) == 0) ? width / 4 : width / 4 + 1;
        height = (( height % 4 ) == 0) ? height / 4 : height / 4 + 1;
        depth = (( depth % 4 ) == 0) ? depth / 4 : depth / 4 + 1;
    }

    // Calculate size of a row
    UINT rowSize = D3D9Info::instance().getSize(description.Format) * width;

    UINT requiredSize = rowSize * height * depth;
    
    if (buffer_size < requiredSize)
    {
        char buff[256];
        sprintf(buff, "D3D9VolumeLockUpdater::update => Buffer size is %d.  Required size is %d.  Format is %08x.  Ignoring update.\n",
            buffer_size, requiredSize, description.Format);
        includelog::logfile().write(includelog::Warning, buff);
        return;
    }

    // Iterate copying memory according to pitches
    UINT rowPitch = substituteLockedBox.RowPitch;
    UINT slicePitch = substituteLockedBox.SlicePitch;
    for(UINT z = 0; z < depth; z++ )
    {
        for(UINT y = 0; y < height; y++ ) 
        {
            memcpy(&substitute[y * rowPitch + z * slicePitch],
                   &original[ y * rowSize + height * rowSize * z ],
                   rowSize);
        }
    }
}


void D3D9VolumeLockUpdater::setDesc(D3DVOLUME_DESC *desc) {
    description = *desc;
}

void D3D9VolumeLockUpdater::setSubstituteLockedBox(D3DLOCKED_BOX *lockedBox) {
    substituteLockedBox = *lockedBox;
}

void D3D9VolumeLockUpdater::setBox(D3DBOX *b) {
    if (b == 0)
        fullLock = true;
    else {
        fullLock = false;
        box = *b;
    }
}

D3D9VolumeLockUpdater::D3D9VolumeLockUpdater(): fullLock(false) {
}

/************************************************************************************/

void D3D9SurfaceLockUpdater::update(size_t buffer_size, void * buffer)
{
    // Store in char * for simplify addressing
    char *original = static_cast<char *>(buffer);
    char *substitute = static_cast<char *>(substituteLockedRect.pBits);

    // Calculate dimensions
    UINT width =  fullLock ? description.Width : (rect.right - rect.left);
    UINT height = fullLock ? description.Height : (rect.bottom - rect.top);

    if(D3D9Info::instance().isCompressed(description.Format)) {
        // Compressed formats hold 16 pixels per texel (4x4) and width and height
        // is in pixels, we have to translate to width and height in texels
        width = (( width % 4 ) == 0) ? width / 4 : width / 4 + 1;
        height = (( height % 4 ) == 0) ? height / 4 : height / 4 + 1;
    }

    // Calculate size of a row
    UINT rowSize = D3D9Info::instance().getSize(description.Format) * width;

    UINT requiredSize = rowSize * height;
    
    if (buffer_size < requiredSize)
    {
        char buff[256];
        sprintf(buff, "D3D9SurfaceLockUpdater::update => Buffer size is %d.  Required size is %d.  Format is %08x.  Ignoring update.\n",
            buffer_size, requiredSize, description.Format);
        includelog::logfile().write(includelog::Warning, buff);
        return;
    }
   
    // Iterate copying memory according to pitches
    UINT rowPitch = substituteLockedRect.Pitch;
    for(UINT y = 0; y < height; y++ )
    {
        memcpy(&substitute[y * rowPitch ],
               &original[ y * rowSize ],
               rowSize);
    }
}

void D3D9SurfaceLockUpdater::setDesc(D3DSURFACE_DESC *desc) {
    description = *desc;
}

void D3D9SurfaceLockUpdater::setSubstituteLockedRect(D3DLOCKED_RECT *lockedRect) {
    substituteLockedRect = *lockedRect;
}

void D3D9SurfaceLockUpdater::setRect(RECT *r) {
    if (r == 0)
        fullLock = true;
    else {
        fullLock = false;
        rect = *r;
    }
}

D3D9SurfaceLockUpdater::D3D9SurfaceLockUpdater(): fullLock(false) {
}

/************************************************************************************/

void D3D9IndexBufferLockUpdater::update(size_t buffer_size, void * buffer)
{
    unsigned long dataSize = (sizeToLock == 0) ? (description.Size - offset) : sizeToLock;

    if (buffer_size < dataSize)
    {
        char buff[256];
        sprintf(buff, "D3D9IndexBufferLockUpdater::update => Buffer size is %d.  Required size is %d.  Ignoring update.\n",
            buffer_size, dataSize);
        includelog::logfile().write(includelog::Warning, buff);
        return;
    }

    memcpy(substituteData, buffer, dataSize);
}

void D3D9IndexBufferLockUpdater::setDesc(D3DINDEXBUFFER_DESC *desc) {
    description = *desc;
}

void D3D9IndexBufferLockUpdater::setSubstituteData(void *data) {
    substituteData = data;
}

void D3D9IndexBufferLockUpdater::setOffset(UINT off) {
    offset = off;
}

void D3D9IndexBufferLockUpdater::setSize(UINT aSizeToLock) {
    sizeToLock = aSizeToLock;
}

D3D9IndexBufferLockUpdater::D3D9IndexBufferLockUpdater(): offset(0), sizeToLock(0), substituteData(0){
}

/************************************************************************************/

void D3D9VertexBufferLockUpdater::update(size_t buffer_size, void * buffer)
{
    unsigned long dataSize = ((offset == 0) & (sizeToLock == 0)) ? description.Size : sizeToLock;
    
    if (buffer_size < dataSize)
    {
        char buff[256];
        sprintf(buff, "D3D9VertexBufferLockUpdater::update => Buffer size is %d.  Required size is %d.  Ignoring update.\n",
            buffer_size, dataSize);
        includelog::logfile().write(includelog::Warning, buff);
        return;
    }
    
    memcpy(substituteData, buffer, dataSize);
}

void D3D9VertexBufferLockUpdater::setDesc(D3DVERTEXBUFFER_DESC *desc) {
    description = *desc;
}

void D3D9VertexBufferLockUpdater::setSubstituteData(void *data) {
    substituteData = data;
}

void D3D9VertexBufferLockUpdater::setOffset(UINT off) {
    offset = off;
}

void D3D9VertexBufferLockUpdater::setSize(UINT aSizeToLock) {
    sizeToLock = aSizeToLock;
}

D3D9VertexBufferLockUpdater::D3D9VertexBufferLockUpdater(): offset(0), sizeToLock(0), substituteData(0){
}

/************************************************************************************/

D3D9SubstituteDeviceManager::D3D9SubstituteDeviceManager():
    device(0), window(0), oWindowed(false),    oBackBufferCount(false),
    oBackBufferWidth(false),oBackBufferHeight(false), oSwapEffect(false),
    oDeviceType(false), oBehaviorFlags(false), oMultiSampleType(false),
    oMultiSampleQuality(false), oFullScreen_RefreshRateInHz(false),
    oPresentationInterval(false)
{
}


void D3D9SubstituteDeviceManager::updateConfiguration()
{
    D3DConfiguration &config = D3DConfiguration::instance();

    oBackBufferWidth = config.existsVariable("SubstituteDevice", "BackBufferWidth");

    if(oBackBufferWidth)
        backBufferWidth = atoi(config.getValue("SubstituteDevice", "BackBufferWidth").c_str());

    oBackBufferHeight = config.existsVariable("SubstituteDevice", "BackBufferHeight");

    if(oBackBufferHeight)
        backBufferHeight = atoi(config.getValue("SubstituteDevice", "BackBufferHeight").c_str());

    oBackBufferCount = config.existsVariable("SubstituteDevice","OverrideBackBufferCount");

    if(oBackBufferCount)
        backBufferCount = atoi(config.getValue("SubstituteDevice","BackBufferCount").c_str());

    oSwapEffect = config.existsVariable("SubstituteDevice","SwapEffect");

    if(oSwapEffect)
        swapEffect  = (D3DSWAPEFFECT)atoi(config.getValue("SubstituteDevice","SwapEffect").c_str());

    oWindowed = config.existsVariable("SubstituteDevice","Windowed");

    if(oWindowed)
        windowed = (config.getValue("SubstituteDevice","Windowed") == "true");

    oDeviceType = config.existsVariable("SubstituteDevice","DeviceType");

    if(oDeviceType)
        deviceType = (D3DDEVTYPE)atoi(config.getValue("SubstituteDevice", "DeviceType").c_str());

    oBehaviorFlags = config.existsVariable("SubstituteDevice","BehaviorFlags");

    if(oBehaviorFlags) {
        char *aux;
        behaviorFlags = (DWORD)strtoul(config.getValue("SubstituteDevice", "BehaviorFlags").c_str(), &aux, 0);
    }

    oMultiSampleType = config.existsVariable("SubstituteDevice","MultiSampleType");

    if(oMultiSampleType)
        multiSampleType = (D3DMULTISAMPLE_TYPE)atoi(config.getValue("SubstituteDevice", "MultiSampleType").c_str());

    oMultiSampleQuality = config.existsVariable("SubstituteDevice","MultiSampleQuality");

    if(oMultiSampleQuality)
        multiSampleQuality = (DWORD)atoi(config.getValue("SubstituteDevice","MultiSampleQuality").c_str());

    oFullScreen_RefreshRateInHz = config.existsVariable("SubstituteDevice","FullScreen_RefreshRateInHz");

    if(oFullScreen_RefreshRateInHz)
        fullScreen_RefreshRateInHz = (UINT)atoi(config.getValue("SubstituteDevice", "FullScreen_RefreshRateInHz").c_str());

    oPresentationInterval = config.existsVariable("SubstituteDevice","PresentationInterval");

    if(oPresentationInterval)
        presentationInterval = (UINT)atoi(config.getValue("SubstituteDevice","PresentationInterval").c_str());

    //  Fulfill some requirements for windowed mode:
    //
    //    In window mode the refresh rate must be 0.  Disregard whatever is in the configuration file.
    // 
    if (oWindowed && windowed)
    {
        //  Force refresh rate to zero in windowed mode.
        oFullScreen_RefreshRateInHz = true;
        fullScreen_RefreshRateInHz = 0;
    }
}


void D3D9SubstituteDeviceManager::onNewSubstituteDevice(IDirect3DDevice9 * d) {
    device = d;
}

void D3D9SubstituteDeviceManager::setSubstituteWindow(HWND w) {
    window = w;
}

void D3D9SubstituteDeviceManager::presentBatch()  {
    if(device != 0) {
        device->EndScene();
        device->Present(0, 0, 0, 0);
        device->BeginScene();
    }
}


void D3D9SubstituteDeviceManager::onCreateDevice(
    UINT &Adapter, D3DDEVTYPE &DeviceType,
    HWND &hFocusWindow, DWORD &BehaviorFlags, D3DPRESENT_PARAMETERS *&pp ) {

    if(oDeviceType)
        DeviceType = this->deviceType;
    if(oBehaviorFlags)
        BehaviorFlags = this->behaviorFlags;

    hFocusWindow = window;
    onNewPresentationParameters(pp);
}

void D3D9SubstituteDeviceManager::onDeviceReset(D3DPRESENT_PARAMETERS *&pp) {
    onNewPresentationParameters(pp);
}

void D3D9SubstituteDeviceManager::onNewPresentationParameters(D3DPRESENT_PARAMETERS *&pp) {

    // Override required parameters.

    if(oWindowed) pp->Windowed = windowed;
    if(oBackBufferWidth) pp->BackBufferWidth = backBufferWidth;
    if(oBackBufferHeight) pp->BackBufferHeight = backBufferHeight;
    if(oSwapEffect) { pp->SwapEffect = swapEffect; }
    if(oBackBufferCount) pp->BackBufferCount = backBufferCount;
    if(oMultiSampleType) pp->MultiSampleType = multiSampleType;
    if(oMultiSampleQuality) pp->MultiSampleQuality = multiSampleQuality;
    if(oFullScreen_RefreshRateInHz) pp->FullScreen_RefreshRateInHz = fullScreen_RefreshRateInHz;
    if(oPresentationInterval) pp->PresentationInterval = presentationInterval;


    // Change the window size if required

    if(pp->Windowed) resizeWindow(pp->BackBufferWidth, pp->BackBufferHeight);

    // Put a valid window handle.

#if defined(WIN32) && !defined(D3D9_WIN32_COMP)
    pp->hDeviceWindow = window;
#endif    
}

IDirect3DDevice9 *D3D9SubstituteDeviceManager::getSubstituteDevice()
{
    return device;
}


void D3D9SubstituteDeviceManager::resizeWindow(int width, int height) {
#if defined(WIN32) && !defined(D3D9_WIN32_COMP)

    // Direct3D draws in window client area, so we must
    // set windows dimensions that determine a client area
    // that fits the backbuffer dimensions.
    RECT rect;
    GetWindowRect(window, &rect);
    RECT clientRect;
    GetClientRect(window, &clientRect);

    // Remind ClientRect coords are relative to Window Rect
    int additionalWidth = rect.right - rect.left - clientRect.right;
    int additionalHeight = rect.bottom - rect.top - clientRect.bottom;

    // Trick for resize the substituteWindow without need to reposition
    SetWindowPos(window, NULL, 0, 0,
        width + additionalWidth,
        height + additionalHeight,
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

#endif //WIN32
}


/************************************************************************************/

D3D9Status::D3D9Status() {
    reset();
}

D3D9Status::~D3D9Status() {
    // Actually reset frees all.
    reset();
}

void D3D9Status::updateConfiguration()
{
    D3DConfiguration &config = D3DConfiguration::instance();

    if (config.existsVariable("Hacks", "ATIFPDepthBuffer"))
        enableATIFPDepthBufferHack = (config.getValue("Hacks", "ATIFPDepthBuffer") == "true");
    else
        enableATIFPDepthBufferHack = false;

    if (config.existsVariable("Hacks", "ContinueOnNULL"))
        continueOnNULLHack = (config.getValue("Hacks", "ContinueOnNULL") == "true");
    else
        continueOnNULLHack = false;

    if (config.existsVariable("Hacks", "ReplaceATI2Format"))
        replaceATI2Format = (config.getValue("Hacks", "ReplaceATI2Format") == "true");
        
    substituteDeviceManager.updateConfiguration();
}

void D3D9Status::reset() {

    // Iterate through stacks
    map<void *, stack< D3D9LockUpdater *> *>::iterator itStacks;
    for(itStacks = lockUpdaters.begin(); itStacks != lockUpdaters.end(); itStacks ++ ) {

        // Delete updaters referenced by stack and finally the stack itself
        stack< D3D9LockUpdater *> * currentStack = (*itStacks).second;
        while(!currentStack->empty()) {
            D3D9LockUpdater *upd = currentStack->top();
            delete upd;
            currentStack->pop();
        }
        delete(currentStack);
    }

    // Clear addresses
    substitutes.clear();
    lockUpdaters.clear();
}


void D3D9Status::addLockUpdater(void *substitute)
{
    if (lockUpdaters[substitute] == 0)
    {
        char buff[256];
        sprintf(buff, "Adding Lock Updater Stack for interface pointer %p\n", substitute);
        includelog::logfile().write(includelog::Debug, buff);
        lockUpdaters[substitute] = new stack<D3D9LockUpdater *>();
    }
}


void D3D9Status::setSubstitute(PIXPointer original, void *substitute)
{
    //if ((original != NULL) && (substitute == NULL))
    //    panic("D3D9Status", "setSubstitute", "Trying to substitute the original pointer with a NULL pointer.");

    if (substitutes[original] == 0)
    {
        substitutes[original] = substitute;

        char buff[256];
        sprintf(buff, "Setting substitute interface pointer %p for original interface pointer %p\n",
            substitute, (void *) pointer(original));
        includelog::logfile().write(includelog::Debug, buff);
    }
    else if (substitutes[original] != substitute)
    {
        char buff[256];
        sprintf(buff, "Warning! Substituting original interface pointer %p with substitute pointer %p but already existing substitute found %p\n",
            (void *) pointer(original), substitute, substitutes[original]);
        includelog::logfile().write(includelog::Debug, buff);
        
        substitutes[original] = substitute;
    }    
    // Create a stack if it's a new substitute
    //if(lockUpdaters[originalAsPointer] == 0)
    //    lockUpdaters[originalAsPointer] = new stack< D3D9LockUpdater *>();
}

void *D3D9Status::getSubstitute(PIXPointer original)
{
    if (original == 0)
        return NULL;
    
    if (substitutes[original] == 0)
    {
        char buff[256];
        sprintf(buff, "Substitute interface for original interface pointer %p has not been defined.\n",
            (void *) pointer(original));
        includelog::logfile().write(includelog::Debug, buff);
    }

    return substitutes[original];
}

void D3D9Status::removeSubstitute(PIXPointer original)
{
    if (substitutes[original] != 0)
    {
        char buff[256];
        sprintf(buff, "Removing substitute interface pointer %p for original interface pointer %p.\n",
            substitutes[original], (void *) pointer(original));
        includelog::logfile().write(includelog::Debug, buff);
        
        substitutes.erase(original);
    }
    else
    {
        char buff[256];
        sprintf(buff, "Removing substitute for original interface pointer %p.  Substitute not found.\n",
            (void *) pointer(original));
        includelog::logfile().write(includelog::Panic, buff);
    }
}

void D3D9Status::lockSurface(void *original, D3DSURFACE_DESC *desc, D3DLOCKED_RECT *lockedRect, RECT *pRect)
{
    D3D9SurfaceLockUpdater *upd = new D3D9SurfaceLockUpdater();
    upd->setDesc(desc);
    upd->setRect(pRect);
    upd->setSubstituteLockedRect(lockedRect);
    addLockUpdater(original);
    lockUpdaters[original]->push(upd);
}

void D3D9Status::lockVolume (void *original, D3DVOLUME_DESC *desc, D3DLOCKED_BOX *lockedBox, D3DBOX *pBox)
{
    D3D9VolumeLockUpdater *upd = new D3D9VolumeLockUpdater();
    upd->setDesc(desc);
    upd->setBox(pBox);
    upd->setSubstituteLockedBox(lockedBox);
    addLockUpdater(original);
    lockUpdaters[original]->push(upd);
}

void D3D9Status::lockVertexBuffer(void *original, D3DVERTEXBUFFER_DESC *desc, UINT offsetToLock, UINT sizeToLock, void *sdata)
{
    D3D9VertexBufferLockUpdater *upd = new D3D9VertexBufferLockUpdater();
    upd->setDesc(desc);
    upd->setOffset(offsetToLock);
    upd->setSize(sizeToLock);
    upd->setSubstituteData(sdata);
    addLockUpdater(original);
    lockUpdaters[original]->push(upd);
}

void D3D9Status::lockIndexBuffer(void *original, D3DINDEXBUFFER_DESC *desc, UINT offsetToLock, UINT sizeToLock, void *sdata)
{
    D3D9IndexBufferLockUpdater *upd = new D3D9IndexBufferLockUpdater();
    upd->setDesc(desc);
    upd->setOffset(offsetToLock);
    upd->setSize(sizeToLock);
    upd->setSubstituteData(sdata);
    addLockUpdater(original);
    lockUpdaters[original]->push(upd);
}

void D3D9Status::unlock(void *original, size_t bufferSize, void *buffer)
{
#ifndef NO_UNLOCK_OPERATIONS // TEST
    stack< D3D9LockUpdater * > *s = lockUpdaters[original];

    if (lockUpdaters[original] == 0)
        panic("D3D9Status", "unlock", "D3D9LockUpdater for interface pointer not found.");
        
    if (lockUpdaters[original]->empty())
        return;

    D3D9LockUpdater *upd =     lockUpdaters[original]->top();
    upd->update(bufferSize, buffer);
    delete upd;
    
    lockUpdaters[original]->pop();
#endif 
}

void D3D9Status::setMainSubstituteWindow(HWND w) {
    substituteDeviceManager.setSubstituteWindow(w);
}

void D3D9Status::newMainSubstituteDevice(IDirect3DDevice9 *d) {
    substituteDeviceManager.onNewSubstituteDevice(d);
}

IDirect3DDevice9 *D3D9Status::getSubstituteDevice()
{
    return substituteDeviceManager.getSubstituteDevice();

}

void D3D9Status::onCreateDevice(
        UINT &Adapter, D3DDEVTYPE &DeviceType,
        HWND &hFocusWindow, DWORD &BehaviorFlags,
        D3DPRESENT_PARAMETERS *&pp ) {

        substituteDeviceManager.onCreateDevice(Adapter, DeviceType,
            hFocusWindow, BehaviorFlags, pp);

}

void D3D9Status::onDeviceReset( D3DPRESENT_PARAMETERS *&pp ) {
    substituteDeviceManager.onDeviceReset(pp);
}

void D3D9Status::mainSubstituteDevicePresentBatch() {
    substituteDeviceManager.presentBatch();
}

bool D3D9Status::isEnabledATIFPDepthBufferHack()
{
    return enableATIFPDepthBufferHack;
}


bool D3D9Status::isEnabledContinueOnNULLHack()
{
    return continueOnNULLHack;
}

bool D3D9Status::isEnabledATI2FormatHack()
{
    return replaceATI2Format;
}

