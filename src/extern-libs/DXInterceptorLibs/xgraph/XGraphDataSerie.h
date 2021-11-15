// XGraphDataSerie.h: Schnittstelle für die Klasse CXGraphDataSerie.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XGRAPHDATASERIE_H__D5C7052F_E160_4A76_8FCE_3AF92077F85A__INCLUDED_)
#define AFX_XGRAPHDATASERIE_H__D5C7052F_E160_4A76_8FCE_3AF92077F85A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define _PI 3.14159265358979
//Macro to convert mouse points (short) to points (long).
#define MPOINT2POINT(mpt, pt)   ((pt).x = (mpt).x, (pt).y = (mpt).y)
//Macro to convert two points to a 2-D vector.
#define POINTS2VECTOR2D(pt0, pt1, vect) \
                       ((vect).x = (double)((pt1).x - (pt0).x), \
                       (vect).y = (double)((pt1).y - (pt0).y))

typedef struct tagDataPoint
{
	double fXVal;
  double fYVal;

  tagDataPoint() :
  fXVal(0.0),
  fYVal(0.0)
  {
  }

  tagDataPoint(double x, double y) :
  fXVal(x),
  fYVal(y)
  {
  }
} TDataPoint;

typedef struct tagHITTEST
{
	POINT p1;
	POINT p2;
} HITTEST;

class CXGraph;

#include "XGraphObjectBase.h"
#include "GfxUtils.h"
#include "vector2d.h"

#include <vector>

using namespace std;

#define MAX_DEGREE 21

// Clipboard data
typedef struct tagCFDATASERIE
{
	long			nCount;
	long            nFirstVisible;
	long            nLastVisible;
	int				nXAxis;
	int				nYAxis;
	int             nIndex;
	int				nLineSize;
	int             nFillStyle;
	int             nMarkerType;
	int             nMarker;
	int             nMarkerSize;
	UINT			nLineStyle;
	bool			bAutoDelete;
	bool			bShowMarker;
	bool            bFillBeneath;
	short           nFillCurve;
	bool            bFillTransparent;
	COLORREF        crFillColor;
#ifndef _WIN32_WCE
	_TCHAR          cLabel[_MAX_PATH];
#else
	char            cLabel[_MAX_PATH];
#endif
	int			    gtType;

} CFDATASERIE;

class CPolynomialSolver
{

public:

	int     m_nGlobalO;
	double  m_fC[MAX_DEGREE];

	CPolynomialSolver() {;};

	virtual ~CPolynomialSolver() {;};
		
	bool   Solve(double a[], double b[], int n);
	bool   Polyfit(int nRows, int nOrder, TDataPoint *pData);
	double GetValue(double fX);
};

class CXGraphDataSerie : public CXGraphObject
{

	DECLARE_SERIAL( CXGraphDataSerie )

	friend class CXGraph;
	friend class CXGraphAxis;
	friend class CXGraphLabel;

private :

	BOOL HitTestLine(POINT pt0, POINT pt1, POINT ptMouse, int nWidth);
	void CreateCurveGripRgn(CPoint oldPoint, CPoint point);

public:

	enum EGraphType
	{
		gtLine,
		gtScatter
	};

	CXGraphDataSerie();
	CXGraphDataSerie(const CXGraphDataSerie& copy);
	CXGraphDataSerie& operator =(const CXGraphDataSerie& copy);
	virtual ~CXGraphDataSerie();

protected:
	
	TDataPoint*		m_pData;
	long			m_nCount;
	long            m_nFirstVisible;
	long            m_nLastVisible;
	int				m_nXAxis;
	int				m_nYAxis;
	int             m_nIndex;
	int				m_nLineSize;
	int             m_nFillStyle;
	int             m_nMarkerType;
	int             m_nMarker;
	int             m_nMarkerSize;
	UINT			m_nLineStyle;
	bool			m_bAutoDelete;
	bool			m_bShowMarker;
	bool            m_bFillBeneath;
	bool            m_bFillTransparent;
	COLORREF		m_crFillColor;
	CString         m_cLabel;
	EGraphType      m_gtType;

	short           m_nFillCurve;

	vector<HITTEST> m_CurveRegions;	

	virtual void	Draw(CDCEx *pDC);
	void			DrawMarker(CDCEx *pDC);
	void            ResetVisibleRange();

public:

	CPolynomialSolver m_PS;

	void			PrepareClipboard(CFDATASERIE& serie);

	CXGraph*		GetGraph() { return m_pGraph; };

	TDataPoint*     GetLinearTrend(long nPoints = 0);
	TDataPoint*		GetCubicTrend(long nPoints = 0);
	TDataPoint*     GetPolynomialTrend(int nDegree, int nPoints = 0);

	TDataPoint*		GetSimpleMovingAverage(int span);
	TDataPoint*     GetExponentialMovingAverage(int span);
	TDataPoint*		GetLinearMovingAverage(int span);
	TDataPoint*     GetTriangularMovingAverage(int span);
	TDataPoint*		GetSineWeightedMovingAverage(int span);

	inline void     SetLineSize(int nValue) { m_nLineSize = nValue; };
	inline void     SetFillStyle(int nValue) { m_nFillStyle = nValue; };
	inline void     SetMarkerType(int nValue) { m_nMarkerType = nValue; };
	inline void     SetMarker(int nValue) { m_nMarker = nValue; };
	inline void     SetMarkerSize(int nValue) { m_nMarkerSize = nValue; };
	inline void     SetLineStyle(UINT nValue) { m_nLineStyle = nValue; };
	inline void     SetShowMarker(bool bValue) { m_bShowMarker = bValue; };
	inline void     SetFillColor(COLORREF crColor) { m_crFillColor = crColor; };
	inline void     SetFillCurve(short nCurve) { m_nFillCurve = nCurve;};
	inline void     SetFillBeneath(bool bValue, short nCurve = -1 ) { m_bFillBeneath = bValue; m_nFillCurve = nCurve;};
	inline void     SetFillTransparent(bool bValue) { m_bFillTransparent = bValue; };

	inline void     SetLabel(CString cValue) { m_cLabel = cValue; };
	inline void     SetType(EGraphType type) { m_gtType = type; };

	inline long     GetCount() const { return m_nCount; };
	inline int      GetIndex() const { return m_nIndex; };
	inline int      GetXAxis() const { return m_nXAxis; };
	inline int      GetYAxis() const { return m_nYAxis; };
	inline int      GetLineSize() const { return m_nLineSize; };
	inline int      GetFillStyle() const { return m_nFillStyle; };
	inline int      GetMarkerType() const { return m_nMarkerType; };
	inline int      GetMarker() const { return m_nMarker; };
	inline int      GetMarkerSize() const { return m_nMarkerSize; };
	inline UINT     GetLineStyle() const { return m_nLineStyle; };
	inline bool     GetShowMarker() const { return m_bShowMarker; };
	inline bool     GetFillBeneath() const { return m_bFillBeneath; };
	inline short    GetFillCurve() const { return m_nFillCurve; };
	inline bool     GetFillTransparent() const { return m_bFillTransparent; };
	inline CString  GetLabel() const { return m_cLabel; };
	inline EGraphType GetType() const { return m_gtType; };

	virtual void Serialize( CArchive& archive );
};


#endif // !defined(AFX_XGRAPHDATASERIE_H__D5C7052F_E160_4A76_8FCE_3AF92077F85A__INCLUDED_)
