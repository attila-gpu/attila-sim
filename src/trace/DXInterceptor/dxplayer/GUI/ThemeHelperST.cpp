////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ThemeHelperST.h"

////////////////////////////////////////////////////////////////////////////////

ThemeHelperST::ThemeHelperST()
{
	m_hDll = ::LoadLibrary(_T("UxTheme.dll"));
}

////////////////////////////////////////////////////////////////////////////////

ThemeHelperST::~ThemeHelperST()
{
	if (m_hDll)	::FreeLibrary(m_hDll);
	m_hDll = NULL;
}

////////////////////////////////////////////////////////////////////////////////

LPVOID ThemeHelperST::GetProc(LPCSTR szProc, LPVOID pfnFail)
{
	LPVOID lpRet = pfnFail;

	if (m_hDll)
		lpRet = GetProcAddress(m_hDll, szProc);

	return lpRet;
}

////////////////////////////////////////////////////////////////////////////////

HTHEME ThemeHelperST::OpenThemeData(HWND hwnd, LPCWSTR pszClassList)
{
	PFNOPENTHEMEDATA pfnOpenThemeData = (PFNOPENTHEMEDATA)GetProc("OpenThemeData", (LPVOID)OpenThemeDataFail);
	return (*pfnOpenThemeData)(hwnd, pszClassList);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT ThemeHelperST::CloseThemeData(HTHEME hTheme)
{
	PFNCLOSETHEMEDATA pfnCloseThemeData = (PFNCLOSETHEMEDATA)GetProc("CloseThemeData", (LPVOID)CloseThemeDataFail);
	return (*pfnCloseThemeData)(hTheme);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT ThemeHelperST::DrawThemeBackground(HTHEME hTheme, HWND hWnd, HDC hdc, int iPartId, int iStateId, const RECT* pRect, const RECT* pClipRect)
{
  PFNDRAWTHEMEPARENTBACKGROUND pfnDrawThemeParentBackground = (PFNDRAWTHEMEPARENTBACKGROUND)GetProc("DrawThemeParentBackground", NULL);
  if (pfnDrawThemeParentBackground && hWnd)
  {    
	  (*pfnDrawThemeParentBackground)(hWnd, hdc, (PRECT)pRect);  
  }
  
  PFNDRAWTHEMEBACKGROUND pfnDrawThemeBackground = (PFNDRAWTHEMEBACKGROUND)GetProc("DrawThemeBackground", (LPVOID)DrawThemeBackgroundFail);
	return (*pfnDrawThemeBackground)(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT ThemeHelperST::DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, const RECT *pRect)
{
	PFNDRAWTHEMETEXT pfn = (PFNDRAWTHEMETEXT)GetProc("DrawThemeText", (LPVOID)DrawThemeTextFail);
	return (*pfn)(hTheme, hdc, iPartId, iStateId, pszText, iCharCount, dwTextFlags, dwTextFlags2, pRect);
}

////////////////////////////////////////////////////////////////////////////////

BOOL ThemeHelperST::IsAppThemed()
{
	PFNISAPPTHEMED pfn = (PFNISAPPTHEMED) GetProc("IsAppThemed", (LPVOID) HandleEntryPointFail);
	return (*pfn)();
}

////////////////////////////////////////////////////////////////////////////////

BOOL ThemeHelperST::IsThemeActive()
{
	PFNISTHEMEACTIVE pfn = (PFNISTHEMEACTIVE) GetProc("IsThemeActive", (LPVOID) HandleEntryPointFail);
	return (*pfn)();
}

////////////////////////////////////////////////////////////////////////////////

BOOL ThemeHelperST::EnableWindowTheme(HWND hwnd)
{
  PFNSETWINDOWTHEME pfn = (PFNSETWINDOWTHEME) GetProc("SetWindowTheme");
  if (pfn)
  {
    return SUCCEEDED((*pfn)(hwnd, NULL, NULL));
  }
  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////

BOOL ThemeHelperST::DisableWindowTheme(HWND hwnd)
{
  PFNSETWINDOWTHEME pfn = (PFNSETWINDOWTHEME) GetProc("SetWindowTheme");
  if (pfn)
  {
    return SUCCEEDED((*pfn)(hwnd, L"", L""));
  }
  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
