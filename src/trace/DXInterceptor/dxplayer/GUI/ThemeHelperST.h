////////////////////////////////////////////////////////////////////////////////
//
//  ThemeHelperST
//
//  Created: 09/January/2002
//  Updated: 31/October/2002
//
//  Author: Davide Calabro' (davide_calabro@yahoo.com)
//
//  Note: Based on the CVisualStylesXP code published by David Yuheng Zhao
//        (yuheng_zhao@yahoo.com)
//
//  Disclaimer
//  ----------
//  THIS SOFTWARE AND THE ACCOMPANYING FILES ARE DISTRIBUTED "AS IS" AND WITHOUT
//  ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED. NO REPONSIBILITIES FOR POSSIBLE
//  DAMAGES OR EVEN FUNCTIONALITY CAN BE TAKEN. THE USER MUST ASSUME THE ENTIRE
//  RISK OF USING THIS SOFTWARE.
//
//  Terms of use
//  ------------
//  THIS SOFTWARE IS FREE FOR PERSONAL USE OR FREEWARE APPLICATIONS. IF YOU USE
//  THIS SOFTWARE IN COMMERCIAL OR SHAREWARE APPLICATIONS YOU	ARE GENTLY ASKED
//  TO DONATE 5$ (FIVE U.S. DOLLARS) TO THE AUTHOR:
//
//    Davide Calabro'
//    P.O. Box 65
//    21019 Somma Lombardo (VA)
//    Italy
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#ifndef	HTHEME
#define	HTHEME HANDLE
#endif

////////////////////////////////////////////////////////////////////////////////

class ThemeHelperST
{
public:
	
  ThemeHelperST();
	virtual ~ThemeHelperST();

	HTHEME OpenThemeData(HWND hwnd, LPCWSTR pszClassList);
	HRESULT CloseThemeData(HTHEME hTheme);
	
  HRESULT DrawThemeBackground(HTHEME hTheme, HWND hWnd, HDC hdc, int iPartId, int iStateId, const RECT* pRect, const RECT* pClipRect);
	HRESULT DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, const RECT* pRect);
	
  BOOL IsThemeActive();
	BOOL IsAppThemed();

  BOOL EnableWindowTheme(HWND hwnd);
  BOOL DisableWindowTheme(HWND hwnd);

private:
	
  LPVOID GetProc(LPCSTR szProc, LPVOID pfnFail = NULL);

	typedef HTHEME(__stdcall *PFNOPENTHEMEDATA)(HWND hwnd, LPCWSTR pszClassList);
	static HTHEME __stdcall OpenThemeDataFail(HWND, LPCWSTR)	{return NULL;}

	typedef HRESULT(__stdcall *PFNCLOSETHEMEDATA)(HTHEME hTheme);
	static HRESULT __stdcall CloseThemeDataFail(HTHEME)	{return E_FAIL;}

	typedef HRESULT(__stdcall *PFNDRAWTHEMEBACKGROUND)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pRect,  const RECT* pClipRect);
	static HRESULT __stdcall DrawThemeBackgroundFail(HTHEME, HDC, int, int, const RECT*, const RECT*)	{return E_FAIL;}

	typedef HRESULT(__stdcall *PFNDRAWTHEMEPARENTBACKGROUND)(HWND hWnd, HDC hdc, RECT* pRect);

	typedef HRESULT (__stdcall *PFNDRAWTHEMETEXT)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, const RECT* pRect);
	static HRESULT __stdcall DrawThemeTextFail(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, DWORD, const RECT*)	{return E_FAIL;}

	typedef BOOL(__stdcall *PFNISAPPTHEMED)();
	typedef BOOL (__stdcall *PFNISTHEMEACTIVE)();
  typedef HRESULT (_stdcall *PFNSETWINDOWTHEME)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
  
  static BOOL __stdcall HandleEntryPointFail() {return FALSE;}
  
  HMODULE	m_hDll;

};

////////////////////////////////////////////////////////////////////////////////
