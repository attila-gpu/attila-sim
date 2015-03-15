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

#include <windows.h>
#include <gdiplus.h>
#include <stdio.h>
#include <wchar.h>
#include <GLExec.h>
#include <support.h>
#include <time.h>
#include "mesa_wgl.h"
#include <vector>
#include <istream>
#include "ScreenUtils.h"
#include "BufferDescriptor.h"
#include <sstream>
#include "includelog.h"
#include <fstream>
using namespace Gdiplus;

using namespace std;
using includelog::logfile;
 
static clock_t timeElapsed = 0;

int fps = 60;
char traceFileName[256] = "tracefile.txt";
char memRegFileName[256] = "MemoryRegions.dat";
char bufferDescriptorsFileName[256] = "BufferDescriptors.dat";
int frame = 0; // current frame
int startFrame = 0; // first frame to be displayed
int batch = 0;  // current batch
bool freezeFrame = false;
bool autoCap = false;
bool useResTrace = true;
bool useResViewport = true;

bool fpsTest = false;
int xRes = 800;
int yRes = 600;

int cacheSz = 100;
bool acceptDeferredBuffers = true;

// use viewport values as resolution
bool useViewport = false;

// synchronizing variable
bool playingFrame = false;

// interactive batch reproduction mode flag
bool batchMode = false;

HWND hWnd; // main windows (global)
HDC hDC; // Handle device context
HGLRC hRC; // Handle GL Raster context
char frameText[256];

HDC currentImage = 0;
HBITMAP currentBitmap = 0;

char *backBuffer = NULL;
char *frontBuffer = NULL;
char *stencilBuffer = NULL;
char *depthBuffer = NULL;

bool savedFrame = false;
bool frontBufferConverted = false;
bool backBufferConverted = false;

GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;
Graphics *glGraphics;


/*
 * Define function prototypes
 */
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void enableOpenGL(HWND hWnd, HDC * hDC, HGLRC * hRC);
void disableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC);
bool playFrame(GLExec& tr);
void parseConfigFile(const char* configFile);

GLExec gle;

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

void captureFrame(int frame)
{
    WCHAR filename[256];

    if (!batchMode)
        swprintf(filename, L"frame%04d.png", frame-1);
    else
        swprintf(filename, L"frame%04d-batch%04d.png", frame, batch - 1);

    /*HDC hCaptureDC = CreateCompatibleDC(hDC);
    HBITMAP hCaptureBitmap = CreateCompatibleBitmap(hDC, xRes, yRes);
    SelectObject(hCaptureDC, hCaptureBitmap);
    BitBlt(hCaptureDC, 0,0,xRes, yRes, hDC, 0, 0, SRCCOPY);
    PBITMAPINFO pbi = createBitmapInfoStruct(hCaptureBitmap);
    createBMPFile((char *)filename, pbi, hCaptureBitmap, hDC);

    DeleteObject(pbi);
    DeleteDC(hCaptureDC);
    DeleteObject(hCaptureBitmap);*/

    CLSID  encoderClsid;
    INT    result;

    GetEncoderClsid(L"image/png", &encoderClsid);

    Bitmap *frameBitmap = new Bitmap(xRes, yRes, PixelFormat32bppARGB);
    Graphics g(frameBitmap);
    HDC frameHDC = g.GetHDC();
    BitBlt(frameHDC, 0, 0, xRes, yRes, hDC, 0, 0, SRCCOPY);
    g.ReleaseHDC(frameHDC);
    frameBitmap->Save(filename, &encoderClsid, NULL);
    delete frameBitmap;
}

void dumpBuffers()
{
    char filename[256];
    WCHAR filenameW[256];

    CLSID  encoderClsid;
    INT    result;
    WCHAR  strGuid[39];

    result = GetEncoderClsid(L"image/png", &encoderClsid);

    Bitmap *frontBufferBitmap;
    Bitmap *backBufferBitmap;

    //  Check buffers
    if (frontBuffer == NULL)
        frontBuffer = new char[xRes * yRes * 4];

    if (backBuffer == NULL)
        backBuffer = new char[xRes * yRes * 4];

    if (stencilBuffer == NULL)
        stencilBuffer = new char[xRes * yRes];

    if (depthBuffer == NULL)
        depthBuffer = new char[xRes * yRes * 4];

    //  Read front buffer
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, xRes, yRes, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, frontBuffer);

    //  Read back buffer
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, xRes, yRes, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, backBuffer);

    //  Read depth buffer
    glReadPixels(0, 0, xRes, yRes, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, depthBuffer);

    //  Read stencil buffer
    glReadPixels(0, 0, xRes, yRes, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, stencilBuffer);

    if (frontBuffer != NULL)
    {
        if (!batchMode)
            swprintf(filenameW, L"frame%04d.frontbuffer.png", frame - 1);
        else
            swprintf(filenameW, L"frame%04d-batch%04d.frontbuffer.png", frame, batch - 1);
        
        for(int i = 0; i < (yRes/2); i++)
        {
            for(int j = 0; j < xRes; j++)
            {
                unsigned long aux1;
                unsigned long aux2;
                aux1 = ((unsigned long *) frontBuffer)[i * xRes + j];
                aux2 = ((unsigned long *) frontBuffer)[(yRes - i - 1) * xRes + j];
                ((unsigned long *) frontBuffer)[(yRes - i - 1) * xRes + j] = (aux1 >> 8) | (aux1 << 24);
                ((unsigned long *) frontBuffer)[i * xRes + j] = (aux2 >> 8) | (aux2 << 24);
            }
        }

        frontBufferBitmap = new Bitmap(xRes, yRes, xRes * 4, PixelFormat32bppARGB, (BYTE *) frontBuffer);

        frontBufferBitmap->Save(filenameW, &encoderClsid, NULL);

        delete frontBufferBitmap;

        //ofstream frontBufferFile;

        //frontBufferFile.open(filename, ios::binary);

        //frontBufferFile.write(frontBuffer, xRes * yRes * 4);

        //frontBufferFile.close();
    }

    if (backBuffer != NULL)
    {
        if (!batchMode)
            swprintf(filenameW, L"frame%04d.backbuffer.png", frame - 1);
        else
            swprintf(filenameW, L"frame%04d-batch%04d.backbuffer.png", frame, batch - 1);

        for(int i = 0; i < (yRes/2); i++)
        {
            for(int j = 0; j < xRes; j++)
            {
                unsigned long aux1;
                unsigned long aux2;
                aux1 = ((unsigned long *) backBuffer)[i * xRes + j];
                aux2 = ((unsigned long *) backBuffer)[(yRes - i - 1) * xRes + j];
                ((unsigned long *) backBuffer)[(yRes - i - 1) * xRes + j] = (aux1 >> 8) | (aux1 << 24);
                ((unsigned long *) backBuffer)[i * xRes + j] = (aux2 >> 8) | (aux2 << 24);
            }
        }

        backBufferBitmap = new Bitmap(xRes, yRes, xRes * 4, PixelFormat32bppARGB, (BYTE *) backBuffer);

        backBufferBitmap->Save(filenameW, &encoderClsid, NULL);

        delete backBufferBitmap;

        /*if (!batchMode)
            sprintf(filename, "frame%04d.backbuffer.dat", frame - 1);
        else
            sprintf(filename, "frame%04d-batch%04d.backbuffer.dat", frame, batch - 1);
        
        ofstream backBufferFile;

        backBufferFile.open(filename, ios::binary);

        backBufferFile.write(backBuffer, xRes * yRes * 4);

        backBufferFile.close();*/
    }

    if (depthBuffer != NULL)
    {
        if (!batchMode)
            sprintf(filename, "frame%04d.depthbuffer.dat", frame - 1);
        else
            sprintf(filename, "frame%04d-batch%04d.depthbuffer.dat", frame, batch - 1);
        
        ofstream depthBufferFile;

        depthBufferFile.open(filename, ios::binary);

        depthBufferFile.write(depthBuffer, xRes * yRes * 4);

        depthBufferFile.close();
    }

    if (stencilBuffer != NULL)
    {
        if (!batchMode)
            sprintf(filename, "frame%04d.stencilbuffer.dat", frame - 1);
        else
            sprintf(filename, "frame%04d-batch%04d.stencilbuffer.dat", frame, batch - 1);
        
        ofstream stencilBufferFile;

        stencilBufferFile.open(filename, ios::binary);

        stencilBufferFile.write(stencilBuffer, xRes * yRes);

        stencilBufferFile.close();
    }
}

void saveFrame()
{
    /*
    //  Check buffers
    if (frontBuffer == NULL)
        frontBuffer = new char[xRes * yRes * 4];

    if (backBuffer == NULL)
        backBuffer = new char[xRes * yRes * 4];

    if (!batchMode)
    {
        //  Read front buffer
        glReadBuffer(GL_FRONT);
        glReadPixels(0, 0, xRes, yRes, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, frontBuffer);
    
        frontBufferConverted = false;
    }
    else
    {
        //  Read back buffer
        glReadBuffer(GL_BACK);
        glReadPixels(0, 0, xRes, yRes, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, backBuffer);

        backBufferConverted = false;
    }
    */

    savedFrame = true;

    if ( currentImage != 0 )
    {   // Delete previous memory
        DeleteDC(currentImage);
        DeleteObject(currentBitmap);
    }

    HDC hCaptureDC = CreateCompatibleDC(hDC);
    HBITMAP hCaptureBitmap = CreateCompatibleBitmap(hDC, xRes, yRes);
    SelectObject(hCaptureDC, hCaptureBitmap);
    BitBlt(hCaptureDC, 0,0,xRes, yRes, hDC, 0, 0, SRCCOPY);
    
    currentImage = hCaptureDC;
    currentBitmap = hCaptureBitmap;
}


void repaintFrame()
{
    /*
    Bitmap *backBufferBitmap;
    Bitmap *frontBufferBitmap;

    if (!savedFrame)
        return;

    if (batchMode)
    {
        if (!backBufferConverted)
        {
            for(int i = 0; i < (yRes/2); i++)
            {
                for(int j = 0; j < xRes; j++)
                {
                    unsigned long aux1;
                    unsigned long aux2;
                    aux1 = ((unsigned long *) backBuffer)[i * xRes + j];
                    aux2 = ((unsigned long *) backBuffer)[(yRes - i - 1) * xRes + j];
                    ((unsigned long *) backBuffer)[(yRes - i - 1) * xRes + j] = (aux1 >> 8) | (aux1 << 24);
                    ((unsigned long *) backBuffer)[i * xRes + j] = (aux2 >> 8) | (aux2 << 24);
                }
            }

            backBufferConverted = true;
        }

        backBufferBitmap = new Bitmap(xRes, yRes, xRes * 4, PixelFormat32bppARGB, (BYTE *) backBuffer);
        glGraphics->DrawImage(backBufferBitmap, 0, 0);

        delete backBufferBitmap;
    }
    else
    {
        if (!frontBufferConverted)
        {
            for(int i = 0; i < (yRes/2); i++)
            {
                for(int j = 0; j < xRes; j++)
                {
                    unsigned long aux1;
                    unsigned long aux2;
                    aux1 = ((unsigned long *) frontBuffer)[i * xRes + j];
                    aux2 = ((unsigned long *) frontBuffer)[(yRes - i - 1) * xRes + j];
                    ((unsigned long *) frontBuffer)[(yRes - i - 1) * xRes + j] = (aux1 >> 8) | (aux1 << 24);
                    ((unsigned long *) frontBuffer)[i * xRes + j] = (aux2 >> 8) | (aux2 << 24);
                }
            }

            frontBufferConverted = true;
        }

        frontBufferBitmap = new Bitmap(xRes, yRes, xRes * 4, PixelFormat32bppARGB, (BYTE *) frontBuffer);
        glGraphics->DrawImage(frontBufferBitmap, 0, 0);

        delete frontBufferBitmap;
    }
    */

    BitBlt(hDC, 0, 0, xRes, yRes, currentImage, 0, 0, SRCCOPY);
}

void reExecuteFrame()
{    
    
    gle.restoreTracePosition();
    clock_t t1 = clock();            
    playFrame(gle);
    clock_t t2 = clock();
    frame--;
    char msg[256];
    sprintf(msg, "Performing FPS Test: %.2f", float(CLOCKS_PER_SEC)/float(t2 - t1));
    SetWindowText(hWnd,msg);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int iCmdShow)
{

    WNDCLASS wc; // window class
    RECT windowRect;    
    
    MSG msg; // For messages
    BOOL quit = FALSE;

    logfile().enable();
    logfile().pushInfo(__FILE__, __FUNCTION__);
    if ( !logfile().config("GLPLog.ini") )
    {
        logfile().open("GLPLog.log");
        logfile().logFile(true);
        logfile().logFunction(true);
        logfile().enableLevels(includelog::Init | includelog::Panic);
    }
    logfile().writeConfig();

    logfile().write(includelog::Init, "Parsing config file... ");

    string args(lpCmdLine);
    if ( args.empty() )
        parseConfigFile("GLPconfig.ini");
    else
    {
        int start = args.find_first_of('"');
        int end = args.find_first_of('"', start+1);
        if ( start != string::npos )
            args = args.substr(start+1, end - start - 1);

        parseConfigFile(args.c_str());
    }

    logfile().write(includelog::Init, "OK\n", false);

    BufferManager& bm = gle.getBufferManager();
    // Must be selected before opening Buffer Descriptors file
    bm.acceptDeferredBuffers(acceptDeferredBuffers);
    

    /**
     * Init GLExec object
     * Load opengl32.dll functions, open tracefile, open buffer file
     */

    int error = gle.init(traceFileName, bufferDescriptorsFileName, memRegFileName, true);        
    bm.setCacheMemorySize(cacheSz);

    //bm.initDynMemory(0, 150*1024*1024, 1024);
    
    if ( error == -1 )
    {
        logfile().write(includelog::Panic, "Error loading opengl32.dll functions");
        panic("Dll", "WinMain","Error loading opengl32.dll functions");
    }
    else if ( error == -2 )
    {
        logfile().write(includelog::Panic, "Error opening tracefile");
        panic("Dll","WinMain","Error opening tracefile");
    }
    else if ( error == -3 )
    {
        logfile().write(includelog::Panic, "Error opening buffer descriptors file");
        panic("Dll","WinMain", "Error opening buffer descriptors file");
    }
    else if ( error == -4 )
    {
        logfile().write(includelog::Panic, "Error opening memory regions file");
        panic("Dll","WinMain", "Error opening memory regions file");
    }

    unsigned int xAux = 0;
    unsigned int yAux = 0;

    if ( useResTrace )
        gle.getTraceResolution(xAux, yAux);
    
    if ( xAux == 0 ) // resolution in trace file not found
    {
        useResTrace = false; // not found
        if ( useResViewport )
            useViewport = true;
        else
            useViewport = false;
        useResViewport = false; // will be set again when a glViewport call will be found
    }
    else
    {
        // use resolution defined in tracefile
        xRes = xAux;
        yRes = yAux;
    }
    
    // Calculate window rectange.
    windowRect.left = 0;
    windowRect.right = xRes;
    windowRect.top = 0;
    windowRect.bottom = yRes;
    AdjustWindowRectEx(&windowRect, WS_BORDER | WS_CAPTION | WS_EX_OVERLAPPEDWINDOW, false, WS_POPUP);

    // register window class
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "GLSample";
    RegisterClass( &wc );

    // create main window
    hWnd = CreateWindow( 
        "GLSample", "GLPlayer 0.1", 
        WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE,
        0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
        NULL, NULL, hInstance, NULL );    

    SwitchToThisWindow(hWnd, TRUE);

    enableOpenGL( hWnd, &hDC, &hRC );    

    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    glGraphics->FromHDC(hDC);

    /*
    while ( frame < startFrame )
        playFrame(gle); // playFrame increments frame variable
        */
    
    // program main loop
    while ( !quit )
    {
        // check for messages
        if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE )  )
        {
            // handle or dispatch messages
            if ( msg.message == WM_QUIT ) 
                quit = TRUE;
            else 
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
        } 
        else 
        {            
            clock_t t = clock();

            if ( frame < startFrame ) // skip frames...
                playFrame(gle);

            else if ( fpsTest ) // assumes freezeFrame == true
            {
                // perform test
                reExecuteFrame();                
            }
            else if ( freezeFrame )
            {
                static bool freezeFlag = true;
                if ( freezeFlag )
                {
                    // play always the first frame
                    freezeFlag = false;
                    if ( playFrame(gle) )
                    {
                        popup("WinMain","Trace finished");
                        return 0;
                    }
                    //saveFrame(); // save current frame (allows repainting)
                }
                // do nothing...
            }
            // limit clock rate setting a value for MAX_FRAMES_PER_SEC
            else if ( t - timeElapsed >= CLOCKS_PER_SEC / fps )
            {
                timeElapsed = t; // reset counter
            
                // Player code is called here...
                if ( playFrame(gle) )
                {
                    popup("WinMain","Trace finished");
                    return 0;
                }
                
            }
        }
    }
    
    GdiplusShutdown(gdiplusToken);

    // shutdown OpenGL
    disableOpenGL( hWnd, hDC, hRC );
    
    // destroy the window explicitly
    DestroyWindow( hWnd );

    logfile().write(includelog::Init, "GLPlayer finished");
    logfile().popInfo();
    
    return msg.wParam;    
}


// Enable OpenGL
void enableOpenGL(HWND hWnd, HDC * hDC, HGLRC * hRC)
{
    PIXELFORMATDESCRIPTOR pfd;
    int format;
    
    // get the device context (DC)
    *hDC = GetDC( hWnd );
    
    // set the pixel format for the DC
    ZeroMemory( &pfd, sizeof( pfd ) );
    /*
    pfd.nSize = sizeof( pfd );
    pfd.cStencilBits = 24;
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;
    */
pfd.nSize = 0x28;
pfd.nVersion = 0x1;
pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
pfd.iPixelType = PFD_TYPE_RGBA;
pfd.cColorBits = 32;
pfd.cRedBits = 8;
pfd.cRedShift = 16;
pfd.cGreenBits = 8;
pfd.cGreenShift = 8;
pfd.cBlueBits = 8;
pfd.cBlueShift = 0;
pfd.cAlphaBits = 8;
pfd.cAlphaShift = 24;
pfd.cAccumBits = 0;
pfd.cAccumRedBits = 0;
pfd.cAccumGreenBits = 0;
pfd.cAccumBlueBits = 0;
pfd.cAccumAlphaBits = 0;
pfd.cDepthBits = 24;
pfd.cStencilBits = 8;
pfd.cAuxBuffers = 0;
pfd.iLayerType = 0x0;
pfd.bReserved = 0x0;
pfd.dwLayerMask = 0x0;
pfd.dwVisibleMask = 0x0;
pfd.dwDamageMask = 0x0;
    format = ChoosePixelFormat( *hDC, &pfd );
    SetPixelFormat( *hDC, format, &pfd );
    
    // create and enable the render context (RC)
    *hRC = wglCreateContext( *hDC );
    wglMakeCurrent( *hDC, *hRC );    
    
}

// disable openGL
void disableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent( NULL, NULL );
    wglDeleteContext( hRC );
    ReleaseDC( hWnd, hDC );
}


// Window Procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        return 0;
        
    case WM_CLOSE:
        PostQuitMessage( 0 );
        return 0;
    case WM_DESTROY:
        return 0;
        
    case WM_PAINT:
        if ( frame >= startFrame )
            repaintFrame();
        break;
        
    case WM_CHAR:
        if ( (TCHAR)wParam == 'f' || (TCHAR)wParam == 'F' )
        {
            if ( !playingFrame && !fpsTest)
            {
                batchMode = false;
                playingFrame = true;
                                 
                if ( playFrame(gle) )
                {
                    logfile().pushInfo(__FILE__,__FUNCTION__);
                    logfile().write(includelog::Init, "Trace finished");
                    logfile().popInfo();
                    popup("WinMain","Trace finished");
                    return 0;
                }
                playingFrame = false;
            }
        }

        else if (((TCHAR)wParam == 'b') || ((TCHAR)wParam == 'B'))
        {
            if (!playingFrame && !fpsTest)
            {
                playingFrame = true;
                batchMode = true;

                if ( playFrame(gle) )
                {
                    logfile().pushInfo(__FILE__,__FUNCTION__);
                    logfile().write(includelog::Init, "Trace finished");
                    logfile().popInfo();
                    popup("WinMain","Trace finished");
                    return 0;
                }
                playingFrame = false;
            }
        }

        else if ( (TCHAR)wParam == 'c' || (TCHAR)wParam == 'C' )
        {
            captureFrame(frame);
        }
        
        else if (((TCHAR)wParam == 'd') || ((TCHAR)wParam == 'D'))
        {
            dumpBuffers();
        }
        else if ( (TCHAR)wParam == 'r' || (TCHAR)wParam == 'R' )
        {    
            if ( freezeFrame )
                fpsTest = !fpsTest;
            // else (ignored, fpsTest can be done only with freezeFrame enabled)
        }

    case WM_KEYDOWN:
        switch ( wParam )
        {            
            case VK_ESCAPE:
                PostQuitMessage(0);
                return 0;
        }
        return 0;
    }
    
    
    return DefWindowProc(hWnd, message, wParam, lParam); // default behaviour
    
}

void check_wglSwapBuffers()
{
    /*
    static ofstream statsFile("vertexCount.csv");
    statsFile << Stats::instance().getVertexCount() << endl;
    Stats::instance().resetVertexCount();
    */
}


bool playFrame(GLExec& gle )
{   
    {
        logfile().pushInfo(__FILE__,__FUNCTION__);
        stringstream ss;
        ss << "Starting play frame " << frame;
        logfile().write(includelog::Init, ss.str());
    }

    if ( frame >= startFrame )
        gle.saveTracePosition();

    APICall apicall;
    bool result;

    bool showFrame;
    bool skipCall;

    bool bufferSwapped = false;

    while ( true ) // Play frame...
    {        
        // identify call
        apicall = gle.getCurrentCall();

        skipCall = false;
        showFrame = false;

        switch ( apicall ) // Check call 
        {        
            case APICall_glBegin:
            case APICall_glVertex2f:
            case APICall_glVertex3f:
            case APICall_glVertex3fv:
            case APICall_glVertex2i:
                if ( frame < startFrame )
                    skipCall = true; // Skip call
                break;
            case APICall_glDrawArrays:
            case APICall_glEnd:
            case APICall_glDrawElements:
            case APICall_glDrawRangeElements:
                if ( gle.checkBatchesAsFrames() || batchMode )
                    showFrame = true;
                if ( frame < startFrame )
                    skipCall = true;
                break;
            case APICall_UNDECLARED:
                logfile().write(includelog::Init, " OK\n", false);
                logfile().popInfo();
                return true; // End of trace
            default:
                    ; // process that call in a regular way
        }
        
        // Execute call -> read parameters & execute OR skip call        
        if ( skipCall )
        {
            gle.skipCurrentCall();
            if ( showFrame )
            {
                if ( gle.getCurrentCall() == APICall_wglSwapBuffers )
                {
                    check_wglSwapBuffers();
                    gle.skipCurrentCall();
                    apicall = APICall_wglSwapBuffers;
                    bufferSwapped = true;
                }
                result = false;
                break;
            }
        }
        else
        {
            apicall = gle.executeCurrentCall();
            
            GLenum  error = glGetError();
            char buffer[256];

            switch(error)
            {
                case GL_NO_ERROR:
                    break;
                case GL_INVALID_ENUM:
                    sprintf(buffer, "GL error code GL_INVALID_ENUM for GL API Call ID %x\n", apicall);
                    logfile().write(includelog::Panic, buffer);
                    break;
                case GL_INVALID_VALUE:
                    sprintf(buffer, "GL error code GL_INVALID_VALUE for GL API Call ID %x\n", apicall);
                    logfile().write(includelog::Panic, buffer);
                    break;
                case GL_INVALID_OPERATION:
                    sprintf(buffer, "GL error code GL_INVALID_OPERATION for GL API Call ID %x\n", apicall);
                    logfile().write(includelog::Panic, buffer);
                    break;
                    
                default:
                    char buffer[256];
                    sprintf(buffer, "GL error code : %x for GL API Call ID %x\n", error, apicall);
                    logfile().write(includelog::Panic, buffer);
                    break;
                    
            }
            
            if ( showFrame )
            {
                if ( gle.getCurrentCall() == APICall_wglSwapBuffers )
                {   
                    // Do not swap a batch twice (if it has a swapBuffers just behind)
                    check_wglSwapBuffers();
                    gle.skipCurrentCall();
                    apicall = APICall_wglSwapBuffers;
                    bufferSwapped = true;
                }
                result = false; // frame finished
                break;
            }
        }        

        if ( apicall == APICall_wglSwapBuffers/* || (apicall == APICall_SwapBuffers)*/ )
        {
            check_wglSwapBuffers();
            bufferSwapped = true;
            result =  false; // frame has finished        
            break;
        }
        else if ( apicall == APICall_glViewport )
        {
            static bool firstViewport = true;            
            if ( useViewport && firstViewport )
            {
                firstViewport = false;
                useResViewport = true;
                int p0 = gle.getCurrentParam(0).v32bit;
                int p1 = gle.getCurrentParam(1).v32bit;
                xRes = gle.getCurrentParam(2).v32bit;
                yRes = gle.getCurrentParam(3).v32bit;

                RECT cl, wnd;
                GetClientRect(hWnd, &cl);
                GetWindowRect(hWnd, &wnd);
                MoveWindow(hWnd, 0, 0, xRes + (wnd.right - cl.right), yRes + (wnd.bottom - cl.bottom), true);

                GetClientRect(hWnd, &cl);

                if ( cl.left != p0 || cl.top != p1 || cl.right != xRes || cl.bottom != yRes )
                {
                    char buffer[256];
                    sprintf(buffer, "Client area: (%d, %d), (%d, %d)\n"
                                    "Viewport: (%d, %d), (%d, %d)"
                                    , cl.left, cl.top, cl.right, cl.bottom,
                                    p0, p1, xRes, yRes);
                    MessageBox(NULL, buffer, "Warning. Client Area & viewport are not equal:", MB_OK);
                }
            }
        }
    }

    char resMsg[256];
    
    if ( useResTrace )
        sprintf(resMsg, "Resolution (trace): %dx%d", xRes, yRes);
    else if ( useResViewport )
        sprintf(resMsg, "Resolution (viewport): %dx%d", xRes, yRes);
    else
        sprintf(resMsg, "Resolution (default): %dx%d", xRes, yRes);

    if ( frame < startFrame )
    {
        sprintf(frameText,"Skipping frame %d   -   %s", frame, resMsg);
        if ( showFrame )
            strcat(frameText, "  (It is a BATCH)");
    }
    else
    {
        if (!batchMode)
        {
            sprintf(frameText,"Frame %d   -   %s", frame, resMsg);
        }
        else
        {
            sprintf(frameText,"Frame %d Batch %d  -   %s", frame, batch, resMsg);
        }

        if ( showFrame )
            strcat(frameText, "  (BATCH)");
    }

    SetWindowText(hWnd,frameText);
    SwapBuffers(hDC ); // end of frame, swap buffers

    if ( frame >= startFrame )
    {
        saveFrame();
        if ( autoCap )
            captureFrame(frame);                    
    }    

    logfile().write(includelog::Init, " OK\n", false);
    logfile().popInfo();

    if (!batchMode)
        frame++;
    else
    {
        //  Update batch counter.
        batch++;

        //  Check if a swap command was processed
        if (bufferSwapped)
        {
            bufferSwapped = false;
            batch = 0;
            frame++;
        }
    }

    return result;
}

void parseConfigFile( const char* configFile )
{
    ifstream config;

    config.open(configFile); 
    if ( !config.is_open() )
        return ;

    char option[1024];
    char* value;
    
    while ( !config.eof() )
    {
        config.getline(option,sizeof(option));
        value = option;

        while ( *value != '\0' && *value != '=' )
            value++;
        if ( *value == '=' )
        {
            *value = '\0';
            value++; // skip '='
            
            // opt: has an option ; value: contains the value for this option
            if ( EQ(option, "forward") )
                parseConfigFile(value);
            else if ( EQ(option,"inputFile") )
                strcpy(traceFileName, value);
            else if ( EQ(option,"bufferFile") )
                strcpy(bufferDescriptorsFileName,value);
            else if ( EQ(option,"memFile") )
                strcpy(memRegFileName, value);
            else if ( EQ(option, "freezeFrame") )
            {
                if ( EQ(value,"1") || EQ(value, "YES") || EQ(value, "yes") )
                    freezeFrame = true;
            }
            else if ( EQ(option, "autoCapture") )
            {
                if ( EQ(value,"1") || EQ(value, "YES") || EQ(value, "yes") )
                    autoCap = true;
            }
            else if ( EQ(option, "fps") || EQ(option, "FPS") )
            {
                // no error checking...
                fps = atoi(value);
            }
            else if ( EQ(option, "resTrace") )
            {
                if ( EQ(value,"0") )
                    useResTrace = false;
                // else -> use default
            }
            else if ( EQ(option, "resViewport") )
            {
                if ( EQ(value,"0") )
                    useResViewport = false;
                // else  -> use default
            }
            else if ( EQ(option, "defaultRes") )
            {                
                xRes = 800; 
                yRes = 600;
                
                xRes = atoi(value);
                int i = 0;
                while ( value[i] != 0 && value[i] != 'x' )
                    i++;
                if ( value[i] != 0 )
                    yRes = atoi(&value[i+1]);
                else
                {
                    // reset, use harcoded resolution
                    xRes = 800;
                    yRes = 600;
                }

            }
            else if ( EQ(option, "startFrame") )
            {
                startFrame = atoi(value);
            }
            else if ( EQ(option, "bufferCache") )
            {
                cacheSz = atoi(value);
                // cache size can range from 1 to 10000
                if ( cacheSz <= 0 )
                    cacheSz = 100;
                else if ( cacheSz > 100000 )
                    cacheSz = 100000;
            }
            else if ( EQ(option, "allowUndefinedBuffers") )
            {
                if ( EQ(value, "1") )
                    acceptDeferredBuffers = true;
                else
                    acceptDeferredBuffers = false;
            }
        }
    }
}
