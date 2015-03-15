////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include "DXPainterHeaders.h"
#include "DXConsole.h"

using namespace dxpainter;

////////////////////////////////////////////////////////////////////////////////

struct ParamsOptions
{
  bool   displayWindowed;
  LPTSTR traceFilename;
  LPTSTR measuresFilename;
  unsigned int measureRangeMin;
  unsigned int measureRangeMax;

  ParamsOptions()
  {
    displayWindowed = false;
    traceFilename = NULL;
    measuresFilename = NULL;
    measureRangeMin = 0;
    measureRangeMax = 0;
  }
};

////////////////////////////////////////////////////////////////////////////////

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
  int argc = __argc;
  LPTSTR *argv = __argv;
  ParamsOptions options;
  
  for (int i=1; i < argc; ++i)
  {
    if (!_tcsicmp(argv[i], TEXT("--windowed")) || !_tcsicmp(argv[i], TEXT("-w")))
    {
      options.displayWindowed = true;
    }
    else
    if (!_tcsicmp(argv[i], TEXT("-frmin")))
    {
      i++;
      if (i < argc) options.measureRangeMin = atoi(argv[i]);
    }
    else
    if (!_tcsicmp(argv[i], TEXT("-frmax")))
    {
      i++;
      if (i < argc) options.measureRangeMax = atoi(argv[i]);
    }
    else
    {
      if (!options.traceFilename)
      {
        options.traceFilename = argv[i];
        continue;
      }

      if (!options.measuresFilename)
      {
        options.measuresFilename = argv[i];
        continue;
      }
    }
  }
  
  if (!options.traceFilename)
  {
    CString message;
    message += "Syntax: dxconsole [options] <trace-filename> [measures-filename]\n";
    message += "\n";
    message += "Options:\n";
    message += "\n";
    message += "  -w|--windowed : Disable fullscreen\n";
    message += "  -frmin : Saved frame measures range min\n";
    message += "  -frmax : Saved frame measures range max";
    ShowMessage(message);
    return 0;
  }
  
  DXPainter* painter = DXPainterCreate();
  
  if (!painter)
  {
    ShowError("Could'nt create a DXPainter instance");
    return -1;
  }
  
  WindowInformation winfo;
  
  if (!InitWindowInstance(hInstance, 640, 480, options.displayWindowed, winfo))
  {
    ShowError("Could'nt create a window instance");
    SAFE_DELETE(painter);
    return -1;
  }

  DXPainter::InitParameters params;
  params.TraceFilename = options.traceFilename;
  params.ViewportWindowed = winfo.hWnd;
  params.ViewportFullScreen = winfo.hWnd;
  params.StartWindowed = options.displayWindowed;
  params.EnableVSync = false;

  if (options.measuresFilename)
  {
    params.MeasurePerformanceFilename = options.measuresFilename;
    params.MeasurePerformance = true;
    params.MeasureFrameRangeMin = options.measureRangeMin;
    params.MeasureFrameRangeMax = options.measureRangeMax;
  }

  if (!painter->Init(&params))
  {
    ShowError("Could'nt initialize DXPainter");
    SAFE_DELETE(painter);
    DestroyWindowInstance(winfo);
    return -1;
  }
  
  ShowCursor(FALSE);
  ShowWindowInstance(winfo, 640, 480);
  painter->Play();
  
  while (1) 
  {
    MSG msg;
    bool stop = false;

    while (PeekMessage(&msg,NULL, 0, 0, PM_NOREMOVE))
    {
      if (!GetMessage(&msg, NULL, 0, 0))
      {
        stop = true;
        break;
      }

      if (!TranslateAccelerator(msg.hwnd, winfo.hAccel, &msg)) 
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }

    if (!stop)
    {
      stop = !DrawScreen(painter);
    }
    
    if (stop)
    {
      break;
    }
  }
  
  ShowCursor(TRUE);
  painter->Close();
  SAFE_DELETE(painter);  
  DestroyWindowInstance(winfo);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

bool DrawScreen(DXPainter* painter)
{
  switch (painter->Paint())
  {
  case DXPainter::PR_ERROR_DIRECT3D:
  case DXPainter::PR_ERROR_PAINTER:
  case DXPainter::PR_ERROR_TRACEMANAGER:
    return false;

  case DXPainter::PR_OK_NOMOREFRAMES:
    return false;

  case DXPainter::PR_OK_PAUSE:
  case DXPainter::PR_OK_STOP:
  case DXPainter::PR_OK_PLAYSTEPPERFRAME:
    return false;

  case DXPainter::PR_OK_PLAY:
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

void ShowMessage(LPCSTR szMessage)
{
  MessageBox(NULL, szMessage, "DXConsole", MB_ICONINFORMATION | MB_OK);
}

////////////////////////////////////////////////////////////////////////////////

void ShowError(LPCSTR szMessage)
{
  MessageBox(NULL, szMessage, "Error", MB_ICONWARNING | MB_OK);
}

////////////////////////////////////////////////////////////////////////////////

bool InitWindowInstance(HINSTANCE hInstance, int width, int height, bool windowed, WindowInformation& winfo)
{
  TCHAR szWindowClass[256] = "DXCONSOLE";
  TCHAR szTitle[256] = "DXConsole";
  
  if (!RegisterWindowClass(winfo.hInstance, szWindowClass))
  {
    return false;
  }

  HDC hdcScreen = GetDC(NULL);
  winfo.hWnd = CreateWindow(szWindowClass, szTitle, WS_POPUP | (windowed ? WS_BORDER : 0), (GetDeviceCaps(hdcScreen, HORZRES)-width)/2, (GetDeviceCaps(hdcScreen, VERTRES)-height)/2, width, height, NULL, NULL, hInstance, NULL);
  ReleaseDC(NULL, hdcScreen);

  if (!winfo.hWnd)
  {
    return false;
  }

  winfo.hInstance = hInstance;
  winfo.hAccel = LoadAccelerators(hInstance, (LPCTSTR) IDR_ACCELERATOR);
  _tcscpy(winfo.szWindowClass, szWindowClass);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void ShowWindowInstance(WindowInformation& winfo, int width, int height)
{
  HDC hDC = GetDC(NULL);
  SetWindowPos(winfo.hWnd, HWND_TOP, (GetDeviceCaps(hDC, HORZRES)-width)/2, (GetDeviceCaps(hDC, VERTRES)-height)/2, width, height, SWP_SHOWWINDOW);
  ReleaseDC(NULL, hDC);
}

////////////////////////////////////////////////////////////////////////////////

void DestroyWindowInstance(WindowInformation& winfo)
{
  if (winfo.hInstance)
  {
    DestroyWindow(winfo.hWnd);
    UnregisterClass(winfo.szWindowClass, winfo.hInstance);

    winfo.hInstance = 0;
    winfo.hWnd = 0;
    winfo.hAccel = 0;
    ZeroMemory(winfo.szWindowClass, sizeof(winfo.szWindowClass));
  }
}

////////////////////////////////////////////////////////////////////////////////

ATOM RegisterWindowClass(HINSTANCE hInstance, LPCSTR windowClass)
{
  WNDCLASSEX wcex;
  
  ZeroMemory(&wcex, sizeof(WNDCLASSEX));
  
  wcex.cbSize        = sizeof(WNDCLASSEX); 
  wcex.hInstance     = hInstance;
  wcex.style			   = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc   = (WNDPROC) WndProc;
  wcex.cbClsExtra    = 0;
  wcex.cbWndExtra    = 0;
  wcex.lpszMenuName  = 0;
  wcex.lpszClassName = windowClass;
  wcex.hIcon         = NULL;
  wcex.hIconSm       = NULL;
  wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW+1);

  return RegisterClassEx(&wcex);
}

////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) 
  {
  case WM_COMMAND:
    {
      int wmId    = LOWORD(wParam); 
      int wmEvent = HIWORD(wParam); 
      switch (wmId)
      {
      case IDM_EXIT:
        DestroyWindow(hWnd);
        break;
      default:
        return DefWindowProc(hWnd, message, wParam, lParam);
      }
    }
    break;
  
  case WM_PAINT:
    return DefWindowProc(hWnd, message, wParam, lParam);
    break;
  
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
