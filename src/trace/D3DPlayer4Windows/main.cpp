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

// Enable visual leak detector
//#define VLD_AGGREGATE_DUPLICATES
//#include <vld.h> 

#include <gdiplus.h>
#include <d3d9.h>
#include <d3dx9tex.h>

#include "WriteBMP.h"
#include "D3DPlayer.h"
#include "IniFile.h"
#include "D3DConfiguration.h"
#include "includelog.h"

using namespace std;
using namespace Gdiplus;

/*********************
Globals
*********************/

D3DPlayer *player = 0;
int frameCount = 0;
int batchCount = 0;
HWND hWnd = 0;
string tracename;
bool playing = false;
bool oneFrameOnly = false;
bool oneBatchOnly = false;
bool autoCapture = false;
bool enableStatus = false;

GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;

/*********************
 Forward declarations
*********************/
void cleanup();
void showStatus(HWND w);

/********************
Appends frame number and bmp extension
********************/
string getCaptureFileName(const string &s, int frame, unsigned int callOffset, const char *extension)
{
    string result = s;
    char frameStr[256];
    sprintf(frameStr, "_F%05d_C%08d", frame, callOffset);
    result.append(frameStr);
    result.append(extension);
    return result;
}

string getCaptureFileName(const string &s, int frame, int batch, unsigned int callOffset, const char *extension)
{
    string result = s;
    char frameStr[256];
    sprintf(frameStr, "_F%05d_B%05d_C%08d", frame, batch, callOffset);
    result.append(frameStr);
    result.append(extension);
    return result;
}

/*********************
 Saves a screenshot in BMP format of client area
 of a window
*********************/
void saveScreenshot(string filename, HWND window)
{
    HDC hDC = GetDC(window);

    RECT client;
    GetClientRect(window, &client);
    int width  = client.right - client.left;
    int height = client.bottom - client.top;
    HDC hCaptureDC = CreateCompatibleDC(hDC);
    HBITMAP hCaptureBitmap = CreateCompatibleBitmap(hDC, width, height);
    SelectObject(hCaptureDC, hCaptureBitmap);
    BitBlt(hCaptureDC, 0,0, width, height, hDC, 0, 0, SRCCOPY);
    PBITMAPINFO pbi = createBitmapInfoStruct(hCaptureBitmap);
    createBMPFile((char *)filename.c_str(), pbi, hCaptureBitmap, hDC);
    DeleteObject(pbi);
    DeleteDC(hCaptureDC);
    DeleteObject(hCaptureBitmap);
}

//
//  Gets the an image encoder CLSID.
//
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    ImageCodecInfo* pImageCodecInfo = NULL;

    GetImageEncodersSize(&num, &size);
    if(size == 0)
        return -1;  // Failure

    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if(pImageCodecInfo == NULL)
        return -1;  // Failure

    GetImageEncoders(num, size, pImageCodecInfo);

    for(UINT j = 0; j < num; ++j)
    {
        if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }    
    }

    free(pImageCodecInfo);
    return -1;  // Failure
}

//
//  Captures the color buffer and outputs a PNG file.
//
void captureFrame(string filename, HWND window)
{
    char buffer[1024];
    sprintf(buffer, "Saving frame to file %s\n", filename.c_str());

    includelog::logfile().write(includelog::Debug, buffer);

    CLSID  encoderClsid;

    GetEncoderClsid(L"image/bmp", &encoderClsid);
    IDirect3DDevice9 *device = (IDirect3DDevice9 *) player->getDirect3DDevice();
    
    //  Get current render target.    
    IDirect3DSurface9 *currentRT;
    if (device->GetRenderTarget(0, &currentRT) != D3D_OK)
    {
        includelog::logfile().write(includelog::Panic, "Error obtaining current render target.\n");        
        return;
    }
    
    //  Get current render target description.
    D3DSURFACE_DESC currentRTDescription;
    currentRT->GetDesc(&currentRTDescription);
        
    //  Create surface where to copy the render target data.
    IDirect3DSurface9 *dataSurface;
    if (device->CreateOffscreenPlainSurface(currentRTDescription.Width, currentRTDescription.Height,
                                                      currentRTDescription.Format, D3DPOOL_SYSTEMMEM, 
                                                      &dataSurface, NULL) != D3D_OK)
    {
        includelog::logfile().write(includelog::Panic, "Error creating surface to hold render target data.\n");
        return;
    }
                                                      
    //  Get data from the render target.                                                      
    if (device->GetRenderTargetData(currentRT, dataSurface) != D3D_OK)
    {
        includelog::logfile().write(includelog::Panic, "Error obtaining current render target data.\n");        
        return;
    }
    
    //  Save as a PNG.
    if (D3DXSaveSurfaceToFile(filename.c_str(), D3DXIFF_PNG, dataSurface, NULL, NULL) != D3D_OK)
    {
        includelog::logfile().write(includelog::Panic, "Error saving surface with render target data as a png file.\n");
        return;
    }
        
    //  Release data surface.
    dataSurface->Release();
    
    //  Release current render target surface.
    currentRT->Release();
}

void dumpBuffers()
{
    char filename[256];
    char buffer[256];

    CLSID  encoderClsid;
    INT    result;

    result = GetEncoderClsid(L"image/png", &encoderClsid);

    IDirect3DDevice9 *device = (IDirect3DDevice9 *) player->getDirect3DDevice();
    
    //  Get current render target.    
    IDirect3DSurface9 *currentRT;
    if (device->GetRenderTarget(0, &currentRT) != D3D_OK)
    {
        includelog::logfile().write(includelog::Panic, "Error obtaining current render target.\n");        
        return;
    }
    
    //  Get current render target description.
    D3DSURFACE_DESC currentRTDescription;
    currentRT->GetDesc(&currentRTDescription);
        
    //  Create surface where to copy the render target data.
    IDirect3DSurface9 *dataSurface;
    if (device->CreateOffscreenPlainSurface(currentRTDescription.Width, currentRTDescription.Height,
                                                      currentRTDescription.Format, D3DPOOL_SYSTEMMEM, 
                                                      &dataSurface, NULL) != D3D_OK)
    {
        includelog::logfile().write(includelog::Panic, "Error creating surface to hold render target data.\n");
        return;
    }
                                                      
    //  Get data from the render target.                                                      
    if (device->GetRenderTargetData(currentRT, dataSurface) != D3D_OK)
    {
        includelog::logfile().write(includelog::Panic, "Error obtaining current render target data.\n");        
        return;
    }
   
    sprintf(buffer, "Dumping render target (%p) width = %d height = %d\n", currentRT, 
        currentRTDescription.Width, currentRTDescription.Height);       
    includelog::logfile().write(includelog::Debug, buffer);

    //  Lock the data surface.
    D3DLOCKED_RECT lockedRect;
    if (dataSurface->LockRect(&lockedRect, NULL,  D3DLOCK_READONLY) != D3D_OK)
    {
        includelog::logfile().write(includelog::Panic, "Error obtaining lock surface with render target data.\n");
        return;
    }
    
    //  Create file for the render target data dump.
    sprintf(filename, "frame%04d.rt0.dat", frameCount - 1);    
    ofstream rtFile;
    rtFile.open(filename, ios::binary);
    
    //  Write surface data into a file.
    rtFile.write((char *) lockedRect.pBits, currentRTDescription.Height * lockedRect.Pitch);

    //  Close file.
    rtFile.close();
              
    // Unlock data surface.
    dataSurface->UnlockRect();
    
    //  Save as a PNG.
    sprintf(filename, "frame%04d.rt0.png", frameCount - 1);
    if (D3DXSaveSurfaceToFile(filename, D3DXIFF_PNG, dataSurface, NULL, NULL) != D3D_OK)
    {
        includelog::logfile().write(includelog::Panic, "Error saving surface with render target data as a png file.\n");
        return;
    }
        
    //  Release data surface.
    dataSurface->Release();
    
    //  Release current render target surface.
    currentRT->Release();
    
    //  Get current depth stencil buffer.    
    //IDirect3DSurface9 *depthStencilSurface;
    //if (device->GetDepthStencilSurface(&depthStencilSurface) != D3D_OK)
    //{
    //    includelog::logfile().write(includelog::Panic, "Error obtaining current depth stencil surface.\n");        
    //    return;
    //}
    
    //  Get current depth stencil buffer description.
    //D3DSURFACE_DESC currentDepthStencilDescription;
    //depthStencilSurface->GetDesc(&currentDepthStencilDescription);
        
    //  Create lockable depth stencil surface.
    //IDirect3DSurface9 *depthStencilLockableSurface;
    //if (device->CreateDepthStencilSurface(currentDepthStencilDescription.Width,
    //                                  currentDepthStencilDescription.Height,
    //                                  D3DFMT_D32F_LOCKABLE,
    //                                  D3DMULTISAMPLE_NONE,
    //                                  0,
    //                                  false,
    //                                  &depthStencilLockableSurface,
    //                                  NULL) != D3D_OK)
    //{
    //    includelog::logfile().write(includelog::Panic, "Error creating lockable depth stencil surface.\n");        
    //    return;
    //}

    //  Copy depth stencil data to the lockable surface.
    //if (device->StretchRect(depthStencilSurface, NULL, depthStencilLockableSurface, NULL, D3DTEXF_NONE) != D3D_OK)
    //{
    //    includelog::logfile().write(includelog::Panic, "Error copying depth stencil data to lockable surface.\n");
    //    return;
    //}
                                                
    //sprintf(buffer, "Dumping depth stencil buffer (%p) width = %d height = %d\n", depthStencilSurface, 
    //    currentDepthStencilDescription.Width, currentDepthStencilDescription.Height);       
    //includelog::logfile().write(includelog::Debug, buffer);

    //  Lock and dump data from the data surface.
    //if (depthStencilLockableSurface->LockRect(&lockedRect, NULL,  D3DLOCK_READONLY) != D3D_OK)
    //{
    //    includelog::logfile().write(includelog::Panic, "Error locking depth stencil surface.");
    //    return;
    //}
      
    //sprintf(buffer, "Depth stencil surface locked rect info : Pitch = %d pBits = %p", lockedRect.Pitch, lockedRect.pBits);       
    //includelog::logfile().write(includelog::Debug, buffer);

    //  Write data into a file.
    //sprintf(filename, "frame%04d.depthStencil.dat", frameCount - 1);    
    //ofstream depthStencilFile;
    //depthStencilFile.open(filename, ios::binary);    
    //depthStencilFile.write((char *) lockedRect.pBits, currentDepthStencilDescription.Height * lockedRect.Pitch);    
    //depthStencilFile.close();
                               
    ///depthStencilLockableSurface->UnlockRect();
    //depthStencilLockableSurface->Release();
    //depthStencilSurface->Release();
    
}


/************************************************************
Window message handler
************************************************************/
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_ERASEBKGND:
        
            // Override this message handling for
            // preventing that windows paints in
            // direct3D area.
            return 0;
            
        case WM_DESTROY:
        
            // Free resources and quit
            cleanup();
            PostQuitMessage(0);
            return 0;
            
        case WM_KEYDOWN :
        
            // Handle input
            try {
                if (wParam == VK_ESCAPE)
                {
                    cleanup();
                    PostQuitMessage(0);
                    return 0;
                }
                else if (wParam == VK_RETURN)
                {
                    playing = !playing;
                    oneFrameOnly = false;
                    oneBatchOnly = false;
                }
                else if (wParam == 'B')
                {
                    playing = true;
                    oneFrameOnly = false;
                    oneBatchOnly = true;
                }
                else if (wParam == 'F')
                {
                    playing = true;
                    oneFrameOnly = true;
                    oneBatchOnly = false;
                }
                else if (wParam == 'A')
                {
                    autoCapture = !autoCapture;
                }
                else if (wParam == 'C') 
                {
                    string filename = getCaptureFileName(tracename, frameCount, player->getCallOffset(), ".png");
                    captureFrame(filename, hWnd);
                }
                else if (wParam == 'D')
                {
                    dumpBuffers();
                }
            }
            // Handle the exception
            catch (string &e)
            {
                MessageBox(hWnd, e.c_str(), "Exception", 0);
            }
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

/***********************************************************
 Application entry point
***********************************************************/
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR commandline, INT)
{
    // Register the window class
    WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
                     GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                     "D3DPlayer4Windows", NULL};
    RegisterClassEx(&wc);

    // Create the application's window
    hWnd = CreateWindow("D3DPlayer4Windows", "D3DPlayer4Windows",
                        WS_OVERLAPPEDWINDOW, 100, 100, 256, 256,
                        NULL, NULL, wc.hInstance, NULL);
    try
    {
        // Show the window
        ShowWindow(hWnd, SW_SHOWDEFAULT);
        UpdateWindow(hWnd);
 
        //  Create log.
        includelog::logfile().enable();
        includelog::logfile().config("PIXLog.cfg");

        includelog::logfile().write(includelog::Init, "Creating PIX Player\n");
       
        // Create player
        player = createD3D9Player(hWnd);

        D3DConfiguration &config = D3DConfiguration::instance();

        if (config.existsVariable("Player", "EnableStatus"))
            enableStatus = (config.getValue("Player", "EnableStatus") == "true");
        else
            enableStatus = true;

        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

        // Open file from command line if provided
        string s = commandline;
        
        if (s.length() != 0)
        {
            tracename = s;

            string logMsg;        
            logMsg = "Opening PIXRun file: ";
            logMsg.append(tracename);
            logMsg.append("\n");
            includelog::logfile().write(includelog::Init, logMsg);

            player->open(tracename);
            
        }

        // Enter the message loop
        MSG msg;
        ZeroMemory(&msg, sizeof(msg));
        while (msg.message != WM_QUIT)
        {
            // Is a windows message waiting?
            if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                // Draw frames
                if (playing)
                {
                    bool success = false;
                    
                    if (oneBatchOnly) 
                        success = player->playBatch();
                    else
                        success = player->playFrame();

                    if (success) 
                    {
                        if (!oneBatchOnly)
                            frameCount++;
                        
                        bool endOfFrame = player->isEndOfFrame();
                        
                        if (autoCapture)
                        {
                            string filename;
                            if (oneBatchOnly && !endOfFrame)
                                filename = getCaptureFileName(tracename, frameCount, batchCount, player->getCallOffset(), ".png");
                            else
                                filename = getCaptureFileName(tracename, frameCount, player->getCallOffset(), ".png");
                            //saveScreenshot(filename, hWnd);
                            captureFrame(filename, hWnd);
                        }

                        if (oneBatchOnly)
                        {
                            if (endOfFrame)
                            {
                                batchCount = 0;
                                frameCount++;
                            }
                            else
                                batchCount++;
                        }
                           
                        if (oneFrameOnly | oneBatchOnly)
                        {
                            playing = false;
                            oneFrameOnly = false;
                            oneBatchOnly = false;
                        }
                    }
                    else
                    {
                        playing = false;
                        oneFrameOnly = false;
                        oneBatchOnly = false;
                        //cleanup();
                        //PostQuitMessage( 0 );
                        //return 0;
                    }

                }
                
                if (enableStatus)
                    showStatus(hWnd);
            }
        }
    }
    // Handle the exception
    catch (string e)
    {
        MessageBox(hWnd, e.c_str(), "Exception", 0);
    }

    UnregisterClass( "D3DPlayer4Windows", wc.hInstance );

    return 0;
}

// Draw status info
void showStatus(HWND w) 
{
    char str[100];
    HDC dc = GetDC(w);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(255, 0, 255));
    sprintf(str, "Frame %5d", frameCount);
    TextOut(dc, 16, 16, str, sizeof(char)*strlen(str));
    sprintf(str, "Batch %5d", batchCount);
    TextOut(dc, 16, 32, str, sizeof(char)*strlen(str));
    sprintf(str, "Calls %5d", player->getCallOffset());
    TextOut(dc, 16, 48, str, sizeof(char)*strlen(str));
    ReleaseDC(w, dc);
}

void cleanup()
{
    GdiplusShutdown(gdiplusToken);

    includelog::logfile().write(includelog::Init, "Closing log\n");
    includelog::logfile().close();

    if (player != 0)
    {
        player->close();
        delete player;
        player = 0;

    }
}