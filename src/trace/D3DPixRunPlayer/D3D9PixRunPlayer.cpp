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

#include "D3DPlayer.h"
#include <D3DTrace.h>

#include "D3DBuffer.h"
#include "IniFile.h"
#include "D3DConfiguration.h"
#include "D3D9Status.h"
#include "D3DPixRunReader.h"
#include "D3D9PixRunPlayer.h"

#include "IncludeLog.h"

#ifdef D3D9_USER_STADISTICS
//#include <D3D9Stadistics.h>
#include <D3D9Stats.h>
#endif

using namespace std;


/**********************************************************************
 Panics
**********************************************************************/
 #define D3D9OP_IDIRECT3D9_REGISTERSOFTWAREDEVICE_SPECIFIC_PANIC
 #define D3D9OP_IDIRECT3DDEVICE9_CREATEADDITIONALSWAPCHAIN_PANIC
 #define D3D9OP_IDIRECT3DDEVICE9_DRAWTRIPATCH_PANIC
 #define D3D9OP_IDIRECT3DDEVICE9_DRAWRECTPATCH_PANIC
 #define D3D9OP_IDIRECT3DSWAPCHAIN9_QUERYINTERFACE_PANIC
 #define D3D9OP_IDIRECT3DSWAPCHAIN9_GETFRONTBUFFERDATA_PANIC
 // FOLLOWING PANIC IS COMMENTED BECAUSE OF A PATCH FOR THE GAME OBLIVION
 // #define D3D9OP_IDIRECT3DTEXTURE9_QUERYINTERFACE_PANIC
 #define D3D9OP_IDIRECT3DVOLUMETEXTURE9_QUERYINTERFACE_PANIC
 #define D3D9OP_IDIRECT3DCUBETEXTURE9_QUERYINTERFACE_PANIC
 #define D3D9OP_IDIRECT3DVERTEXBUFFER9_QUERYINTERFACE_PANIC
 #define D3D9OP_IDIRECT3DINDEXBUFFER9_QUERYINTERFACE_PANIC
 #define D3D9OP_IDIRECT3DSURFACE9_QUERYINTERFACE_PANIC
 #define D3D9OP_IDIRECT3DVOLUME9_QUERYINTERFACE_PANIC
 #define D3D9OP_IDIRECT3DVERTEXDECLARATION9_QUERYINTERFACE_PANIC
 #define D3D9OP_IDIRECT3DPIXELSHADER9_QUERYINTERFACE_PANIC
 #define D3D9OP_IDIRECT3DSTATEBLOCK9_QUERYINTERFACE_PANIC
 #define D3D9OP_IDIRECT3DQUERY9_QUERYINTERFACE_PANIC


/**********************************************************************
 Specific Code definitions
**********************************************************************/

/*********************************************
This call returns an interface and autogeneration
doesn't handle this case.
**********************************************/

inline void D3D9PixRunPlayer::Direct3DCreate9_SPECIFIC_PRE()
{
    u32bit oip_Result;
    
    IDirect3D9 *sip_Result;    
    reader.readParameter<u32bit>(&oip_Result);
    
    UINT ov_SDKVersion;
    UINT sv_SDKVersion;
    reader.readParameter<UINT>(&ov_SDKVersion);    
    sv_SDKVersion = ov_SDKVersion;
    
    sip_Result = Direct3DCreate9(sv_SDKVersion);
    
    status.setSubstitute(oip_Result, sip_Result);    
}

#define D3D9OP_DIRECT3DCREATE9_SPECIFIC_PRE \
    Direct3DCreate9_SPECIFIC_PRE();
    
inline void D3D9PixRunPlayer::IDirect3D9_CreateDevice_SPECIFIC_POST(HRESULT result, IDirect3DDevice9* returnedDeviceInterface)
{
    if (result != D3D_OK)
        panic("D3D9PixRunPlayer", "D3D9OP_IDIRECT3D9_CREATEDEVICE", "CreateDevice returned an error.");
        
    status.newMainSubstituteDevice(returnedDeviceInterface);
}

#define D3D9OP_IDIRECT3D9_CREATEDEVICE_SPECIFIC_POST \
    IDirect3D9_CreateDevice_SPECIFIC_POST(sv_Return, sip_ppReturnedDeviceInterface);
    
inline void D3D9PixRunPlayer::IDirect3D9_CreateDevice_SPECIFIC_PRE(UINT &adapter, D3DDEVTYPE &deviceType,
                                                                   HWND &hFocusWindow, 
                                                                   DWORD &behaviorFlags,
                                                                   D3DPRESENT_PARAMETERS *&pp)
{
    status.onCreateDevice(adapter, deviceType, hFocusWindow, behaviorFlags, pp);
}

#define D3D9OP_IDIRECT3D9_CREATEDEVICE_SPECIFIC_PRE \
    IDirect3D9_CreateDevice_SPECIFIC_PRE(sv_Adapter, sv_DeviceType, sv_hFocusWindow, sv_BehaviorFlags, spv_pPresentationParameters);
    

//  Patch for traces captured on ATI that use float point depth buffers.
#define D3D9OP_IDIRECT3DDEVICE9_CREATEDEPTHSTENCILSURFACE_USER_POST \
    if ((sv_Format == D3DFMT_D24FS8) && (sv_Return != D3D_OK) && status.isEnabledATIFPDepthBufferHack()) \
        sv_Return = sip_This -> CreateDepthStencilSurface( \
                                    sv_Width, sv_Height, \
                                    D3DFMT_D24S8, \
                                    sv_MultiSample, \
                                    sv_MultisampleQuality, \
                                    sv_Discard, \
                                    spip_ppSurface, \
                                    sv_pSharedHandle);

// Support for Texture9 LockRect/UnlockRect with mip levels.
/*inline void D3D9PixRunPlayer::IDirect3D9_CreateTexture_SPECIFIC_PRE(IDirect3DTexture9* texture)
{
    for(UINT currLevel = 0; currLevel < texture->GetLevelCount(); currLevel++)
    {
        IDirect3DSurface9 *surface;
        texture->GetSurfaceLevel(currLevel, &surface);
        status.addLockUpdater((void *) surface);
        surface->Release();
    };
}*/   

//#define D3D9OP_IDIRECT3DDEVICE9_CREATETEXTURE_SPECIFIC_POST \
//    IDirect3D9_CreateTexture_SPECIFIC_POST(sip_ppTexture);

#define D3D9OP_IDIRECT3DDEVICE9_CREATETEXTURE_SPECIFIC_PRE \
if (sv_Format == MAKEFOURCC('A', 'T', 'I', '2') && status.isEnabledATI2FormatHack()) \
    sv_Format = D3DFMT_DXT5;
//else if (sv_Format == D3DFMT_D24S8) \
//    sv_Format = D3DFMT_D16_LOCKABLE;

/*#define D3D9OP_IDIRECT3DCUBETEXTURE9_LOCKRECT_SPECIFIC_POST \
    IDirect3DSurface9 *surface;\
    sip_This->GetCubeMapSurface(sv_FaceType, sv_Level, &surface);\
    D3DSURFACE_DESC desc;\
    surface->GetDesc(&desc);\
    surface->Release();\
    status.lockSurface(oip_This, &desc, spv_pLockedRect, spv_pRect);
*/

inline void D3D9PixRunPlayer::IDirect3DCubeTexture9_LockRect_SPECIFIC_POST(IDirect3DCubeTexture9 *sip_This,
                                                                           D3DCUBEMAP_FACES faceType, UINT level,
                                                                           D3DLOCKED_RECT *lockedRect, RECT *pRect)
{
    IDirect3DSurface9 *surface;
    sip_This->GetCubeMapSurface(faceType, level, &surface);
    D3DSURFACE_DESC desc;
    surface->GetDesc(&desc);
    surface->Release();
    status.lockSurface((void *) surface, &desc, lockedRect, pRect);
}

#define D3D9OP_IDIRECT3DCUBETEXTURE9_LOCKRECT_SPECIFIC_POST \
    IDirect3DCubeTexture9_LockRect_SPECIFIC_POST(sip_This, sv_FaceType, sv_Level, spv_pLockedRect, spv_pRect);
    
    
/*#define D3D9OP_IDIRECT3DCUBETEXTURE9_UNLOCKRECT_SPECIFIC_PRE \
    D3DBuffer buffer;\
    reader.readParameter(&buffer);\
    void *pdata;\
    buffer.lock(&pdata);\
    status.unlock(oip_This, buffer.getSize(), pdata);\
    buffer.unlock();
*/

inline void D3D9PixRunPlayer::IDirect3DCubeTexture9_UnlockRect_SPECIFIC_PRE(IDirect3DCubeTexture9 *sip_This,
                                                                            D3DCUBEMAP_FACES faceType, UINT level)
{
    IDirect3DSurface9 *surface;
    sip_This->GetCubeMapSurface(faceType, level, &surface);
    surface->Release();    
    D3DBuffer buffer;
    reader.readParameter(&buffer);
    void *pdata;
    buffer.lock(&pdata);
    status.unlock((void *) surface, buffer.getSize(), pdata);
    buffer.unlock();
}

#define D3D9OP_IDIRECT3DCUBETEXTURE9_UNLOCKRECT_SPECIFIC_PRE \
    IDirect3DCubeTexture9_UnlockRect_SPECIFIC_PRE(sip_This, sv_FaceType, sv_Level);


#define D3D9OP_IDIRECT3DDEVICE9_CREATEPIXELSHADER_SPECIFIC_PRE \
    D3DBuffer buffer;\
    reader.readParameter(&buffer);\
    void *spmr;\
    buffer.lock(&spmr);\
    sv_pFunction = static_cast< DWORD * >(spmr);
    

#define D3D9OP_IDIRECT3DDEVICE9_CREATEVERTEXDECLARATION_SPECIFIC_PRE \
    D3DBuffer buffer;\
    reader.readParameter(&buffer);\
    void *spmr;\
    buffer.lock(&spmr);\
    sv_pVertexElements = static_cast< D3DVERTEXELEMENT9 * >(spmr);

#define D3D9OP_IDIRECT3DDEVICE9_CREATEVERTEXSHADER_SPECIFIC_PRE \
    D3DBuffer buffer;\
    reader.readParameter(&buffer);\
    void *spmr;\
    buffer.lock(&spmr);\
    sv_pFunction = static_cast< DWORD * >(spmr);

#define D3D9OP_IDIRECT3DDEVICE9_DRAWINDEXEDPRIMITIVEUP_SPECIFIC_PRE \
    D3DBuffer buffer1;\
    reader.readParameter(&buffer1);\
    void *spmr1;\
    buffer1.lock(&spmr1);\
    sv_pVertexStreamZeroData = spmr1;\
    D3DBuffer buffer2;\
    reader.readParameter(&buffer2);\
    void *spmr2;\
    buffer2.lock(&spmr2);\
    sv_pIndexData = spmr2;

#define D3D9OP_IDIRECT3DDEVICE9_DRAWPRIMITIVEUP_SPECIFIC_PRE \
    D3DBuffer buffer;\
    reader.readParameter(&buffer);\
    void *spmr;\
    buffer.lock(&spmr);\
    sv_pVertexStreamZeroData = spmr;

#define D3D9OP_IDIRECT3DDEVICE9_PRESENT_SPECIFIC_PRE \
    sv_hDestWindowOverride = 0;

#define D3D9OP_IDIRECT3DDEVICE9_RESET_SPECIFIC_PRE \
    status.onDeviceReset(spv_pPresentationParameters);

/* The problem here is that codegen treat float * as a pointer to value
   and in this case the float * refers to a vector of 4 float's that
   follows the pointer. As it's the last parameter we are lucky and can
   read it. */
#define D3D9OP_IDIRECT3DDEVICE9_SETCLIPPLANE_SPECIFIC_PRE \
    float specific_spv_pPlane[4];\
    specific_spv_pPlane[0] = sv_pPlane;\
    reader.readParameter<float>(&specific_spv_pPlane[1]);\
    reader.readParameter<float>(&specific_spv_pPlane[2]);\
    reader.readParameter<float>(&specific_spv_pPlane[3]);\
    spv_pPlane = (opv_pPlane == 0 ) ? 0: specific_spv_pPlane;

#define D3D9OP_IDIRECT3DDEVICE9_SETPIXELSHADERCONSTANTB_SPECIFIC_PRE \
    D3DBuffer buffer;\
    reader.readParameter(&buffer);\
    void *spmr;\
    buffer.lock(&spmr);\
    spv_pConstantData = static_cast< BOOL * >(spmr);

#define D3D9OP_IDIRECT3DDEVICE9_SETPIXELSHADERCONSTANTF_SPECIFIC_PRE \
    D3DBuffer buffer;\
    reader.readParameter(&buffer);\
    void *spmr;\
    buffer.lock(&spmr);\
    spv_pConstantData = static_cast< float * >(spmr);

#define D3D9OP_IDIRECT3DDEVICE9_SETPIXELSHADERCONSTANTI_SPECIFIC_PRE \
    D3DBuffer buffer;\
    reader.readParameter(&buffer);\
    void *spmr;\
    buffer.lock(&spmr);\
    spv_pConstantData = static_cast< int * >(spmr);

#define D3D9OP_IDIRECT3DDEVICE9_SETVERTEXSHADERCONSTANTB_SPECIFIC_PRE \
    D3DBuffer buffer;\
    reader.readParameter(&buffer);\
    void *spmr;\
    buffer.lock(&spmr);\
    spv_pConstantData = static_cast< BOOL * >(spmr);

#define D3D9OP_IDIRECT3DDEVICE9_SETVERTEXSHADERCONSTANTF_SPECIFIC_PRE \
    D3DBuffer buffer;\
    reader.readParameter(&buffer);\
    void *spmr;\
    buffer.lock(&spmr);\
    spv_pConstantData = static_cast< float * >(spmr);

#define D3D9OP_IDIRECT3DDEVICE9_SETVERTEXSHADERCONSTANTI_SPECIFIC_PRE \
    D3DBuffer buffer;\
    reader.readParameter(&buffer);\
    void *spmr;\
    buffer.lock(&spmr);\
    spv_pConstantData = static_cast< int * >(spmr);

#define D3D9OP_IDIRECT3DINDEXBUFFER9_LOCK_SPECIFIC_POST \
    D3DINDEXBUFFER_DESC desc;\
    sip_This->GetDesc(&desc);\
    status.lockIndexBuffer((void *) sip_This, &desc, sv_OffsetToLock, sv_SizeToLock, specific_sv_ppbData);

/* The problem here is that codegen treat void ** as a value because Pix doesn't
   store the referenced void * in pixRun file. We must create a void * and put
   the void ** pointing to it. */
#define D3D9OP_IDIRECT3DINDEXBUFFER9_LOCK_SPECIFIC_PRE \
    void *specific_sv_ppbData;\
    sv_ppbData = &specific_sv_ppbData;

#define D3D9OP_IDIRECT3DINDEXBUFFER9_UNLOCK_SPECIFIC_PRE \
    D3DBuffer buffer;\
    reader.readParameter(&buffer);\
    void *pdata;\
    buffer.lock(&pdata);\
    status.unlock((void *) sip_This, buffer.getSize(), pdata);\
    buffer.unlock();

//#define D3D9OP_IDIRECT3DSURFACE9_GETCONTAINER_SPECIFIC_POST \
//    status.setSubstitute(specific_ov_ppContainer, specific_sv_ppContainer);

/* The problem here is that codegen treat void ** as a value
   and in this case a void * with the original container address
   follows it. As it's the last parameter we are lucky and can
   read it. Finally we redirect the pointer to the specific void *
*/
#define D3D9OP_IDIRECT3DSURFACE9_GETCONTAINER_SPECIFIC_PRE \
    u32bit specific_ov_ppContainer;\
    reader.readParameter<u32bit>(&specific_ov_ppContainer);\
    void *specific_sv_ppContainer;\
    sv_ppContainer = &specific_sv_ppContainer;

#define D3D9OP_IDIRECT3DSURFACE9_LOCKRECT_SPECIFIC_POST \
    D3DSURFACE_DESC desc;\
    sip_This->GetDesc(&desc);\
    status.lockSurface((void *) sip_This, &desc, spv_pLockedRect, spv_pRect);

#define D3D9OP_IDIRECT3DSURFACE9_UNLOCKRECT_SPECIFIC_PRE \
    D3DBuffer buffer;\
    reader.readParameter(&buffer);\
    void *pdata;\
    buffer.lock(&pdata);\
    status.unlock((void *) sip_This, buffer.getSize(), pdata);\
    buffer.unlock();

#define D3D9OP_IDIRECT3DTEXTURE9_LOCKRECT_SPECIFIC_POST \
    IDirect3DSurface9 *surface;\
    sip_This->GetSurfaceLevel(sv_Level, &surface);\
    D3DSURFACE_DESC desc;\
    surface->GetDesc(&desc);\
    surface->Release();\
    status.lockSurface((void *) surface, &desc, spv_pLockedRect, spv_pRect);

#define D3D9OP_IDIRECT3DTEXTURE9_UNLOCKRECT_SPECIFIC_PRE \
    D3DBuffer buffer;\
    reader.readParameter(&buffer);\
    void *pdata;\
    buffer.lock(&pdata);\
    IDirect3DSurface9 *surface;\
    sip_This->GetSurfaceLevel(sv_Level, &surface);\
    surface->Release();\
    status.unlock((void *) surface, buffer.getSize(), pdata);\
    buffer.unlock();

#define D3D9OP_IDIRECT3DVERTEXBUFFER9_LOCK_SPECIFIC_POST \
    D3DVERTEXBUFFER_DESC desc;\
    sip_This->GetDesc(&desc);\
    status.lockVertexBuffer((void *) sip_This, &desc, sv_OffsetToLock, sv_SizeToLock, specific_sv_ppbData);

/* The problem here is that codegen treat void ** as a value because Pix doesn't
   store the referenced void * in pixRun file. We must create a void * and put
   the void ** pointing to it. */
#define D3D9OP_IDIRECT3DVERTEXBUFFER9_LOCK_SPECIFIC_PRE \
    void * specific_sv_ppbData;\
    sv_ppbData = &specific_sv_ppbData;

#define D3D9OP_IDIRECT3DVERTEXBUFFER9_UNLOCK_SPECIFIC_PRE \
    D3DBuffer buffer;\
    reader.readParameter(&buffer);\
    void *pdata;\
    buffer.lock(&pdata);\
    status.unlock((void *) sip_This, buffer.getSize(), pdata);\
    buffer.unlock();

//#define D3D9OP_IDIRECT3DVOLUME9_GETCONTAINER_SPECIFIC_POST \
//    status.setSubstitute(specific_ov_ppContainer, specific_sv_ppContainer);

/* The problem here is that codegen treat void ** as a value
   and in this case a void * with the original container address
   follows it. As it's the last parameter we are lucky and can
   read it. Finally we redirect the pointer to the specific void *
*/
#define D3D9OP_IDIRECT3DVOLUME9_GETCONTAINER_SPECIFIC_PRE \
    PIXPointer specific_ov_ppContainer;\
    reader.readParameter<PIXPointer>(&specific_ov_ppContainer);\
    void *specific_sv_ppContainer;\
    sv_ppContainer = &specific_sv_ppContainer;

#define D3D9OP_IDIRECT3DVOLUME9_LOCKBOX_SPECIFIC_POST \
    D3DVOLUME_DESC desc;\
    sip_This->GetDesc(&desc);\
    status.lockVolume((void *) sip_This, &desc, spv_pLockedVolume, spv_pBox);

#define D3D9OP_IDIRECT3DVOLUME9_UNLOCKBOX_SPECIFIC_PRE \
    D3DBuffer buffer;\
    reader.readParameter(&buffer);\
    void *pdata;\
    buffer.lock(&pdata);\
    status.unlock((void *) sip_This, buffer.getSize(), pdata);\
    buffer.unlock();

#define D3D9OP_IDIRECT3DVOLUMETEXTURE9_LOCKBOX_SPECIFIC_POST \
    IDirect3DVolume9 *volume;\
    sip_This->GetVolumeLevel(sv_Level, &volume);\
    D3DVOLUME_DESC desc;\
    volume->GetDesc(&desc);\
    volume->Release();\
    status.lockVolume((void *) volume, &desc, spv_pLockedVolume, spv_pBox);

#define D3D9OP_IDIRECT3DVOLUMETEXTURE9_UNLOCKBOX_SPECIFIC_PRE \
    IDirect3DVolume9 *volume;\
    sip_This->GetVolumeLevel(sv_Level, &volume);\
    volume->Release();\
    D3DBuffer buffer;\
    reader.readParameter(&buffer);\
    void *pdata;\
    buffer.lock(&pdata);\
    status.unlock((void *) volume, buffer.getSize(), pdata);\
    buffer.unlock();


// FOLLOWING LINE IS  A PATCH FOR THE GAME OBLIVION
#define D3D9OP_IDIRECT3DTEXTURE9_QUERYINTERFACE_SPECIFIC_PRE \
    u32bit ov_Return; \
    HRESULT sv_Return; \
    reader.readParameter<u32bit>(&ov_Return); \
    sv_Return = ov_Return; \
    \
    PIXPointer oip_This; \
    IDirect3DTexture9 * sip_This; \
    reader.readParameter<PIXPointer>(&oip_This); \
    sip_This = static_cast<IDirect3DTexture9 *>(status.getSubstitute(oip_This)); \
    \
    GUID ov_riid; \
    GUID sv_riid; \
    reader.readParameter<GUID>(&ov_riid); \
    sv_riid = ov_riid; \
    \
    PIXPointer opip_ppvObj; \
    PIXPointer oip_ppvObj; \
    void ** spip_ppvObj; \
    void * sip_ppvObj; \
    reader.readParameter<PIXPointer>(&opip_ppvObj); \
    reader.readParameter<PIXPointer>(&oip_ppvObj); \
    spip_ppvObj = &sip_ppvObj; \
    \
    sv_Return = sip_This -> QueryInterface( \
    sv_riid ,spip_ppvObj); \
    \
    status.setSubstitute(oip_ppvObj, sip_ppvObj );


void D3D9PixRunPlayer::open(string filename) {

    reader.open(filename);

    callOffset = 0;
    status.reset();

    #ifdef   D3D9_USER_BEGIN
    /* A user initialization code */
    D3D9_USER_BEGIN
    #endif

}

void D3D9PixRunPlayer::close() {

    #ifdef   D3D9_USER_END
    /* User cleanup code */
    D3D9_USER_END
    #endif
    reader.close();
}


bool D3D9PixRunPlayer::playBatch()
{
    // Execute commands until a "batch" command is found
    bool foundNextDraw = false;
    bool foundPresent = false;
    bool fetched = true;

    while(fetched && !foundNextDraw && !foundPresent)
    {
        fetched = reader.fetch();
        
        if (fetched)
        {
            //  Check if the next draw call was found.
            foundNextDraw = (reader.getFetchedCID() == D3D9CID_IDIRECT3DDEVICE9_DRAWPRIMITIVE) ||
                            (reader.getFetchedCID() == D3D9CID_IDIRECT3DDEVICE9_DRAWINDEXEDPRIMITIVE) ||
                            (reader.getFetchedCID() == D3D9CID_IDIRECT3DDEVICE9_DRAWPRIMITIVEUP) ||
                            (reader.getFetchedCID() == D3D9CID_IDIRECT3DDEVICE9_DRAWINDEXEDPRIMITIVEUP) ||
                            (reader.getFetchedCID() == D3D9CID_IDIRECT3DDEVICE9_DRAWRECTPATCH) ||
                            (reader.getFetchedCID() == D3D9CID_IDIRECT3DDEVICE9_DRAWTRIPATCH);
           
            //  Check if present (end of frame) was found.
            foundPresent = (reader.getFetchedCID() == D3D9CID_IDIRECT3DDEVICE9_PRESENT) ||
                           (reader.getFetchedCID() == D3D9CID_IDIRECT3DSWAPCHAIN9_PRESENT);
            
            execute();
        }
    }

    if (fetched)
    {
        if (!foundPresent)
        {
            // Present batch results.
            status.mainSubstituteDevicePresentBatch();
            currentBatch++;
        }
        else
        {
            currentBatch = 0;
            endOfFrame = true;
        }
    }
    
    return foundNextDraw;
}

bool D3D9PixRunPlayer::playFrame()
{
    currentBatch = 0;

    // Execute commands until a "present" command is found
    bool foundPresent= false;
    bool fetched = true;
    
    //  Execute until a present (end of frame) is found.
    while(fetched & !foundPresent)
    {
        fetched = reader.fetch();
        
        if (fetched)
        {
            foundPresent =  (reader.getFetchedCID() == D3D9CID_IDIRECT3DDEVICE9_PRESENT) ||
			                (reader.getFetchedCID() == D3D9CID_IDIRECT3DSWAPCHAIN9_PRESENT);
            execute();
        }
    }

    if(foundPresent)
    {
        endOfFrame = true;
    }

    return foundPresent;
}

unsigned long D3D9PixRunPlayer::getCallOffset() {
    return callOffset;
}

void D3D9PixRunPlayer::setWindow(HWND window)
{
    status.setMainSubstituteWindow(window);
}

bool D3D9PixRunPlayer::isEndOfFrame()
{
    bool end = endOfFrame;
    endOfFrame = false;
    return end;
}


D3D9PixRunPlayer::D3D9PixRunPlayer()
{
    D3DConfiguration &cfg = D3DConfiguration::instance();
    cfg.load("D3D9PixRunPlayer.ini");
    status.updateConfiguration();
    frameEnded = false;
    callOffset = 0;
    currentFrame = 0;
    currentBatch = 0;
    endOfFrame = false;
}

void D3D9PixRunPlayer::execute() {

    bool isD3D9Call = true;

    D3D9CID cid = static_cast<D3D9CID>(reader.getFetchedCID());
    switch (cid) {
        #include "D3D9PixRunSwitchBranches.gen"

        default:
            // The call id doesn't match any D3D9 call (perhaps is a D3D9X call id)
            isD3D9Call = false;
    }

    // Increment offset
    if(isD3D9Call) {
        callOffset++;
    }
    
    // Frame ended
    if((cid == D3D9CID_IDIRECT3DDEVICE9_PRESENT) || (cid == D3D9CID_IDIRECT3DSWAPCHAIN9_PRESENT) )
    {
        frameEnded = true;
    }
    else
    {
        if (frameEnded)
        {
            char buffer[256];
            sprintf(buffer, "Frame %d was rendered\n", currentFrame);
            includelog::logfile().write(includelog::Debug, buffer);
            
            currentFrame++;
        }
        
        frameEnded = false;
    }

}

bool D3D9PixRunPlayer::next()
{
    bool fetched = reader.fetch();
    if(fetched)
        execute();
    return fetched;
}

bool D3D9PixRunPlayer::getFrameEnded()
{
    return frameEnded;
}

u32bit D3D9PixRunPlayer::getCurrentEventID()
{
    return reader.getFetchedEID();
}

bool D3D9PixRunPlayer::isPreloadCall()
{
    return reader.isPreloadCall();
}

void *D3D9PixRunPlayer::getDirect3DDevice()
{
    return (void *) status.getSubstituteDevice();
}

#include "D3D9PixRunDefinitions.gen"

D3DPlayer *createD3D9Player(HWND window) {
    D3D9PixRunPlayer *p = new D3D9PixRunPlayer();
    p->setWindow(window);
    return p;
}

D3DTrace *create_d3d_trace(char *file_name)
{
    D3D9PixRunPlayer *p = new D3D9PixRunPlayer();
    p->setWindow(0);
    
    string filenameStr = file_name;
    
    p->open(filenameStr);

    return p;
}
