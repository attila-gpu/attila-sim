// XGraphObjectBase.h: Schnittstelle für die Klasse CXGraphObjectBase.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XGRAPHOBJECTBASE_H__4FC71E88_00C5_4518_A3F9_48AB5F5499BF__INCLUDED_)
#define AFX_XGRAPHOBJECTBASE_H__4FC71E88_00C5_4518_A3F9_48AB5F5499BF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "GfxUtils.h"

class CXGraph;

#pragma warning (disable : 4244)
#pragma warning (disable : 4800)

class CXGraphObject : public CObject
{
    DECLARE_SERIAL( CXGraphObject )
	
	friend class CXGraph;

protected:

	CRect    m_clRect;
	bool     m_bSelected;
	bool     m_bEditing;
	bool     m_bCanResize;
	bool     m_bCanMove;
	bool     m_bSizing;
	bool     m_bVisible;
	bool     m_bCanEdit;
	COLORREF m_crColor;
	CXGraph* m_pGraph;
	
	CRectTracker m_Tracker;

	CXGraphObject();
	CXGraphObject(const CXGraphObject& copy);
	CXGraphObject& operator=(const CXGraphObject& copy);
	virtual ~CXGraphObject();

	virtual void Draw(CDCEx *pDC) {} ;
	virtual void Edit() {} ;
	virtual void EndEdit() {} ;
	virtual void BeginSize();
	virtual void EndSize();
	virtual void InvokeProperties() {} ;

public:

	inline  void SetRect(CRect rect) { m_clRect = rect; };
	inline  void SetSelected(bool bValue) { m_bSelected = bValue; };
	inline  void SetVisible(bool bValue) { m_bVisible = bValue; };
	inline  void SetColor(COLORREF color) { m_crColor = color; };

	inline  CRect GetRect() const { return m_clRect; };
	inline  bool  GetSelected() const { return m_bSelected; };
	inline  bool  GetVisible() const { return m_bVisible; };
	inline  COLORREF GetColor() const { return m_crColor; };
	inline  CXGraph* GetGraph() const { return m_pGraph; };

	virtual void Serialize( CArchive& archive );


};

#pragma warning (default : 4244)
#pragma warning (default : 4800)

#endif // !defined(AFX_XGRAPHOBJECTBASE_H__4FC71E88_00C5_4518_A3F9_48AB5F5499BF__INCLUDED_)
