// XGraphLabel.h: Schnittstelle für die Klasse CXGraphLabel.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XGRAPHLABEL_H__2EFB495F_400B_4D8E_8D3D_E685375EF88C__INCLUDED_)
#define AFX_XGRAPHLABEL_H__2EFB495F_400B_4D8E_8D3D_E685375EF88C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XGraphObjectBase.h"

class CXGraphLabel : public CXGraphObject  
{

	DECLARE_SERIAL( CXGraphLabel )

private:
	

	friend class CXGraph;

public:
	CXGraphLabel();
	virtual ~CXGraphLabel();

	
protected:
	
	virtual void Draw(CDCEx *pDC);
	virtual void Edit();
	virtual void EndEdit();
	virtual void InvokeProperties();
	

	CString  m_cText;
	CEdit    m_Edit;
	CFont    m_Font;
	COLORREF m_crColor;
	COLORREF m_crTextColor;
	bool     m_bBorder;
	bool     m_bTransparent;
	UINT     m_nAlignment;
	int      m_nCurve;
	
	

public:

	void SetFont(LOGFONT* pLogFont);

	inline void SetText(CString cText) { m_cText = cText; };
	inline void SetColor(COLORREF color) { m_crColor = color; };
	inline void SetTextColor(COLORREF color) { m_crTextColor = color; };
	inline void SetBorder(bool bValue) { m_bBorder = bValue; };
	inline void SetTransparent(bool bValue) { m_bTransparent = bValue; };
	inline void SetAlignment(UINT nValue) { m_nAlignment = nValue; };
	inline void SetCurve(int nCurve) { m_nCurve = nCurve; };

	inline CString GetText() const { return m_cText; };
	inline COLORREF GetColor() const { return m_crColor; };
	inline COLORREF GetTextColor() const { return m_crTextColor; };
	inline bool GetBorder() const { return m_bBorder; };
	inline bool GetTransparent() const { return m_bTransparent; };
	inline UINT GetAlignment() const { return m_nAlignment; };
	inline int  GetCurve() const { return m_nCurve; };

	virtual void Serialize( CArchive& archive );

};

#endif // !defined(AFX_XGRAPHLABEL_H__2EFB495F_400B_4D8E_8D3D_E685375EF88C__INCLUDED_)
