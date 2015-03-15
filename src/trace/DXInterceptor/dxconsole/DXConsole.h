////////////////////////////////////////////////////////////////////////////////

#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <atlstr.h>
#include <tchar.h>
#include "resource.h"

////////////////////////////////////////////////////////////////////////////////

#define SAFE_DELETE(p) { if (p) { delete p; p = NULL; } }

////////////////////////////////////////////////////////////////////////////////

struct WindowInformation
{
  HINSTANCE hInstance;
  HWND hWnd;
  HACCEL hAccel;
  TCHAR szWindowClass[256];

  WindowInformation()
  {
    hInstance = 0;
    hWnd = 0;
    hAccel = 0;
    ZeroMemory(szWindowClass, sizeof(szWindowClass));
  };
};

////////////////////////////////////////////////////////////////////////////////

bool DrawScreen(dxpainter::DXPainter* painter);
void ShowMessage(LPCSTR szMessage);
void ShowError(LPCSTR szMessage);

bool InitWindowInstance(HINSTANCE hInstance, int width, int height, bool windowed, WindowInformation& winfo);
void ShowWindowInstance(WindowInformation& winfo, int width, int height);
void DestroyWindowInstance(WindowInformation& winfo);

ATOM RegisterWindowClass(HINSTANCE hInstance, LPCSTR windowClass);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

////////////////////////////////////////////////////////////////////////////////
