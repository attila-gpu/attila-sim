// XGraphBitmap.h: Schnittstelle für die Klasse CXGraphBitmap.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XGRAPHBITMAP_H__2EFB495F_400B_4D8E_8D3D_E685375EF88C__INCLUDED_)
#define AFX_XGRAPHBITMAP_H__2EFB495F_400B_4D8E_8D3D_E685375EF88C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XGraphObjectBase.h"
//#include "XGraph.h"
#include "BitmapEx.h"


typedef struct tagCFBITMAP 
{
	RECT   rect;
	bool   bBorder;

} CFBITMAP;

class CXGraphBitmap : public CXGraphObject  
{

	DECLARE_SERIAL( CXGraphBitmap )

private:
	

	friend class CXGraph;

	bool m_bLocalCreated;

public:
	CXGraphBitmap();
	CXGraphBitmap(const CXGraphBitmap& copy);
	CXGraphBitmap& operator=(const CXGraphBitmap& copy);

	virtual ~CXGraphBitmap();

	
protected:
	
	virtual void Draw(CDCEx *pDC);
	virtual void InvokeProperties();
	
	CBitmapEx* m_pBitmap;
	bool       m_bBorder;
	
	

public:

	void   PrepareClipboard(CFBITMAP& bitmap);

	inline void SetBorder(bool bValue) { m_bBorder = bValue; };
	inline void SetBitmap(CBitmapEx* pBitmap) { m_pBitmap = pBitmap; };
	
	inline bool		  GetBorder() const { return m_bBorder; };
	inline CBitmapEx* GetBitmap() const { return m_pBitmap; };
	
	virtual void Serialize( CArchive& archive );

};

#endif // !defined(AFX_XGRAPHBITMAP_H__2EFB495F_400B_4D8E_8D3D_E685375EF88C__INCLUDED_)
