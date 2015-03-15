//
//	Class:		CXPStyleButtonST
//
//	Compiler:	Visual C++
//	Tested on:	Visual C++ 6.0
//
//	Version:	See GetVersionC() or GetVersionI()
//
//	Created:	21/January/2002
//	Updated:	24/January/2003
//
//	Author:		Davide Calabro'		davide_calabro@yahoo.com
//									http://www.softechsoftware.it
//
//	Disclaimer
//	----------
//	THIS SOFTWARE AND THE ACCOMPANYING FILES ARE DISTRIBUTED "AS IS" AND WITHOUT
//	ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED. NO REPONSIBILITIES FOR POSSIBLE
//	DAMAGES OR EVEN FUNCTIONALITY CAN BE TAKEN. THE USER MUST ASSUME THE ENTIRE
//	RISK OF USING THIS SOFTWARE.
//
//	Terms of use
//	------------
//	THIS SOFTWARE IS FREE FOR PERSONAL USE OR FREEWARE APPLICATIONS.
//	IF YOU USE THIS SOFTWARE IN COMMERCIAL OR SHAREWARE APPLICATIONS YOU
//	ARE GENTLY ASKED TO DONATE 5$ (FIVE U.S. DOLLARS) TO THE AUTHOR:
//
//		Davide Calabro'
//		P.O. Box 65
//		21019 Somma Lombardo (VA)
//		Italy
//
#ifndef _XPSTYLEBTNST_H
#define _XPSTYLEBTNST_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XBtnST.h"
#include "ThemeHelperST.h"

class CXPStyleButtonST  : public CButtonST
{
public:
	CXPStyleButtonST();
	virtual ~CXPStyleButtonST();

	void SetThemeHelper(ThemeHelperST* pTheme);
	DWORD DrawAsToolbar(BOOL bDrawAsToolbar, BOOL bRepaint = TRUE);

	static short GetVersionI()		{return 12;}
	static LPCTSTR GetVersionC()	{return (LPCTSTR)_T("1.2");}

protected:
	virtual DWORD OnDrawBackground(CDC* pDC, CRect* pRect);
	virtual DWORD OnDrawBorder(CDC* pDC, CRect* pRect);

private:
	ThemeHelperST* m_pTheme;
	BOOL m_bDrawAsToolbar; // Use flat toolbar-style ?
};

#endif
