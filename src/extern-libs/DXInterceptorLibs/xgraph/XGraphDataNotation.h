// XGraphDataNotation.h: Schnittstelle für die Klasse XGraphDataNotation.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XGRAPHDATANOTATION_H__65ED10CC_9CF1_4A5E_B72F_0C37AF298660__INCLUDED_)
#define AFX_XGRAPHDATANOTATION_H__65ED10CC_9CF1_4A5E_B72F_0C37AF298660__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XGraphObjectBase.h"

class CXGraphDataNotation : public CXGraphObject  
{
	DECLARE_SERIAL( CXGraphDataNotation )

	friend class CXGraph;

private:

	CFont m_Font;
	bool  m_bPositioned;

public:
	CXGraphDataNotation();
	virtual ~CXGraphDataNotation();
	virtual void Draw(CDCEx *pDC);
	
	double  m_fXVal;
	double  m_fYVal;
	int		m_nCurve;
	int     m_nIndex;
	CString m_cText;

	virtual void Serialize( CArchive& archive );

};

#endif // !defined(AFX_XGRAPHDATANOTATION_H__65ED10CC_9CF1_4A5E_B72F_0C37AF298660__INCLUDED_)
