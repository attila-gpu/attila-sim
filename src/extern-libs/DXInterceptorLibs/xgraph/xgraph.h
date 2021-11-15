///////////////////////////////////////////////////////////////////////////////////////
// XGraph.h
//
// Main header file for the CXGraph charting control
//
// You are free to use, modify and distribute this source, as long as
// there is no charge, and this HEADER stays intact. This source is
// supplied "AS-IS", without WARRANTY OF ANY KIND, and the user
// holds me blameless for any or all problems that may arise
// from the use of this code.
//
// Expect bugs.
//
// Gunnar Bolle (webmaster@beatmonsters.com)
//
//
// History
//
// 21 Mar 2002    1.00    First release
// 26 Mar 2002    1.01    Fixed DateTime-bug in cursor mode
//						  (thanks to Pavel Klocek for providing the fix)
//						  Fixed crash when adding trend or moving average
// 11 Apr 2002    1.02    Panning in cursor mode added
//						  Added support for very large and very small numbers
//						  Values greater/less 10E6/10E-6 are now automatically shown in
//                        exponential format
//                        Completely redesigned autoscaling (only for normal axes,
//                        DateTime types follow)
//                        Added data notation marks
//                        Added Get/Set member functions
//                        Added nth-order-polynomial trend
// 25. Sep 2002   1.03    Now compiles under VC7
//                        Load/Save for properties/data/object persistence
//						  Added zoom-history
// unreleased	  1.04    Added UNICODE support
//                        Added background bitmaps
//                        Added bitmap objects
//                        Added gradient colorranges
//						  Added axismarkers
// 02.12.2004     1.05    Added support for WINDOWS CE (needs improvement)
//						  Logarithmic axes (thanks to ...)
//						  Added polygonfilling between 2 curves
//   					  Several Bugs fixed by Joerg Blattner
//						  Added Cursor-Synchronisation
//						  Curve editing mode
//						  Measuring mode
//
//
// Known issues
//
// - clipping in print preview doesn't work
//
// Additional credits
//
// - Ishay Sivan (Bugfixing)
// - Pavel Klocek (Bugfixing)
// - James White (color button)
// - Chris Maunder (color popup)
// - Keith Rule (MemDC)
// - J.G. Hattingh (parts of Device Context Utilities)
// - Jörg Blattner (Bugfixing, improvements)
//
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __AFXTEMPL_H__
#include <afxtempl.h>
#endif

#include <vector>
#include <list>
#include <functional>

using namespace std;

class CXGraph;

#include "XGraphObjectBase.h"
#include "XGraphDataSerie.h"
#include "XGraphAxis.h"
#include "XGraphLabel.h"
#include "XGraphBitmap.h"
#include "XGraphDataNotation.h"

#define XG_YAXISCLICK	     WM_USER + 1
#define XG_XAXISCLICK	     WM_USER + 2
#define XG_CURVECLICK	     WM_USER + 3
#define XG_YAXISDBLCLICK   WM_USER + 4
#define XG_XAXISDBLCLICK   WM_USER + 5
#define XG_CURVEDBLCLICK   WM_USER + 6
#define XG_GRAPHDBLCLICK   WM_USER + 7
#define XG_RBUTTONUP       WM_USER + 8
#define XG_CURSORMOVED     WM_USER + 9
#define XG_ZOOMCHANGE      WM_USER + 10
#define XG_POINTCHANGED    WM_USER + 11
#define XG_MEASURECREATED  WM_USER + 12
#define XG_PANCHANGE       WM_USER + 13

#define BITMAP_CHART	     0x01
#define BITMAP_INNER       0x02
#define BITMAP_NOSIZE      0x04
#define BITMAP_STRETCH     0x08
#define BITMAP_TILE        0x10

#define PERSIST_PROPERTIES 0x01
#define PERSIST_DATA       0x02
#define PERSIST_OBJECTS    0x04

#define XGC_HORIZ		     0x01
#define XGC_VERT		     0x02
#define XGC_LEGEND		   0x04
#define XGC_ADJUSTSMOOTH 0x08

#define MIN_ZOOM_PIXELS    15


// Custom clipboard formats

#define CF_XBITMAP		   _T("XGRAPHBITMAP")
#define CF_XLABEL		   _T("XGRAPHLABEL")

const COLORREF BASECOLORTABLE[12] =
{
	RGB(255,0,0),
	RGB(255,255,0),
	RGB(0,255,0),
	RGB(0,255,255),
	RGB(0,0,255),
	RGB(255,0,255),
  
  RGB(128,0,0),
  RGB(128,128,0),
  RGB(0,128,0),
  RGB(0,128,128),
  RGB(0,0,128),
  RGB(128,0,128),
};

#define MAX_MARKERS 8

class CXGraph : public CWnd
{

	DECLARE_SERIAL( CXGraph )

	friend class CXGraphAxis;
	friend class CXGraphLogAxis;
	friend class CXGraphDataSerie;
	friend class CXGraphLabel;

public:

	enum { VERSION = 105 };

	CXGraph();

public:

	enum EOperation {

		opNone,
		opZoom,
		opReset,
		opPan,
		opCursor,
		opMoveObject,
		opEditObject,
		opEditCurve,
		opMeasure
	};

	enum EAlignment
	{
		left,
		right,
		top,
		bottom
	};

  enum EPanMode
  {
    AxisXOnly = 1,
    AxisYOnly = 2,
    BothAxis  = 3,
  };

	typedef struct tagNM_POINTMOVING
	{
		long    nCurve;
		long	nIndex;
		double  dOldVal;
		double  dNewVal;
	} NM_POINTMOVING;

	typedef struct tagMeasureDef
	{
		long nCurve1;
		long nIndex1;
		long nCurve2;
		long nIndex2;
	} MeasureDef;




public:

	//{{AFX_VIRTUAL(CXGraph)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

public:
	virtual ~CXGraph();

	CXGraphDataSerie& SetData(TDataPoint* pValues, long nCount, int nCurve = 0, int nXAxis = 0, int nYAxis = 0, bool bAutoDelete = false);

	CXGraphAxis& GetXAxis(int nAxis);
	CXGraphAxis& GetYAxis(int nAxis);
	CXGraphDataSerie& GetCurve(int nCurve);

	bool DeleteCurve(int nCurve);
	bool DeleteXAxis(int nAxis);
	bool DeleteYAxis(int nAxis);

	void InsertDataNotation(int nCurve, int nIndex);
	void AddLinearTrend(CXGraphDataSerie& serie);
	void AddCubicTrend(CXGraphDataSerie& serie);

	int  GetXAxisCount() { return (int) m_XAxis.size(); };
	int  GetYAxisCount() { return (int) m_YAxis.size(); };
	int  GetCurveCount() { return (int) m_Data.size(); };

	bool SelectCurve(int nCurve);
	bool SelectXAxis(int nAxis);
	bool SelectYAxis(int nAxis);

	void SetGraphMargins(int nLeft, int nTop, int nRight, int nBottom);

#ifndef _WIN32_WCE
	BOOL SaveBitmap(const CString cFile);
#endif

	// Graph operations
	void Edit();
	void Zoom();
	void ResetZoom();
	void Pan();
	void NoOp();
	void Cursor();
	void Measure();


	void OnDraw(CDC *pDC);

#ifndef _WIN32_WCE
	void PrintGraph(CDC *pDC);
	void PrintGraph(CDC *pDC, CRect printRect);
#endif

	int  GetZoomDepth();

	void ResetAll();

	bool SetBackgroundBitmap(CBitmap* pBitmap, UINT nFlags = BITMAP_CHART | BITMAP_STRETCH);

	CXGraphLabel&  InsertLabel(CString cText = _T(""));
	CXGraphLabel&  InsertLabel(CRect rect, CString cText = _T(""));
	CXGraphBitmap& InsertBitmap(CRect rect, CBitmapEx* pBitmap);

	inline void SetBackColor(COLORREF color)		 { m_crBackColor = color; };
	inline void SetGraphColor(COLORREF color)		 { m_crGraphColor = color; };
	inline void SetInnerColor(COLORREF color)		 { m_crInnerColor = color; };
	inline void SetDoubleBuffer(bool bValue)		 { m_bDoubleBuffer = bValue; };
	inline void SetShowLegend(bool bValue)			 { m_bShowLegend = bValue; };
	inline void SetInteraction(bool bValue)		     { m_bInteraction = bValue; };
	inline void SetLegendAlignment(EAlignment align) { m_LegendAlignment = align; };
	inline void SetSnapCursor(bool bValue)			 { m_bSnapCursor = bValue; };
	inline void SetSnapRange(int nValue)			 { m_nSnapRange = nValue; };
	inline void SetPrintHeader(CString cHeader)      { m_cPrintHeader = cHeader; };
	inline void SetPrintFooter(CString cFooter)      { m_cPrintFooter = cFooter; };
	inline void SetCursor(CPoint point)              { m_CurrentPoint = point; };
	inline void SetCursorFlags(UINT nFlags)			 { m_nCursorFlags = nFlags; };
  inline void SetPanMode(EPanMode mode)        { m_panMode = mode; }

	void Autoscale(int nXAxis, int nYAxis, int nCurve);
  void AutoscaleVisible(int nXAxis, int nYAxis);
  void AutoscaleAll(int nXAxis, int nYAxis);

	inline int GetMeasureCount() { return (int) m_Measures.size(); }
  void DeleteMeasures();
  void DeleteZooms();

	inline void ForceSnap (UINT nCurve)              { m_nForcedSnapCurve = nCurve; };

	inline COLORREF   GetBackColor()		{ return m_crBackColor; };
	inline COLORREF   GetGraphColor()		{ return m_crGraphColor; };
	inline COLORREF   GetInnerColor()		{ return m_crInnerColor; };
	inline bool		  GetDoubleBuffer()		{ return m_bDoubleBuffer; };
	inline bool		  GetShowLegend()		{ return m_bShowLegend; };
	inline bool       GetInteraction()      { return m_bInteraction; };
	inline EAlignment GetLegendAlignment()	{ return m_LegendAlignment; };
	inline bool		  GetSnapCursor()		{ return m_bSnapCursor; };
	inline int		  GetSnapRange()		{ return m_nSnapRange; };
	inline CPoint     GetCursor()           { return m_CurrentPoint; };
	inline EOperation GetOperation()        { return m_opOperation; };
  inline EPanMode   GetPanMode()          { return m_panMode; }

	TDataPoint GetCursorAbsolute(int nCurve);
	void	 SetCursorAbsolute(int nCurve, TDataPoint vPoint, bool bForceVisible = false);


	bool   Save(const CString cFile, UINT nFlags = PERSIST_PROPERTIES | PERSIST_DATA | PERSIST_OBJECTS);
	bool   Load(const CString cFile);

	lpDrawFunc  m_pDrawFn[MAX_MARKERS];

protected:

#ifndef _WIN32_WCE
	BOOL   WriteDIB( LPTSTR szFile, HANDLE hDIB);
	HANDLE DDBToDIB( CBitmap& bitmap, DWORD dwCompression, CPalette* pPal );
#endif

	void   Serialize( CArchive& archive, UINT nFlags);

	//{{AFX_MSG(CXGraph)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
#if _MFC_VER >= 0x0700
	afx_msg BOOL OnMouseWheel( UINT nFlags, short zDelta, CPoint pt );
#else
	afx_msg void OnMouseWheel( UINT nFlags, short zDelta, CPoint pt );
#endif
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
#ifndef _WIN32_WCE
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
#endif
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
#ifndef _WIN32_WCE
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
#endif
	afx_msg void ParentCallback();
	afx_msg void RestoreLastZoom();

#ifndef _WIN32_WCE
	afx_msg void OnCopy();
	afx_msg void OnPaste();
#endif

	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:


	typedef struct tagSelectByMarker {

		CRect markerRect;
		CXGraphObject* pObject;

	} SelectByMarker;

	static NM_POINTMOVING		m_nmCurrentPointMoving;
	COLORREF					      m_crBackColor;
	COLORREF					      m_crGraphColor;
	COLORREF					      m_crInnerColor;
	bool						        m_bDoubleBuffer;
	bool						        m_bShowLegend;
	bool					        	m_bInteraction;
	bool					        	m_bTmpDC;
	EAlignment				     	m_LegendAlignment;
	bool						        m_bSnapCursor;
	int						         	m_nSnapRange;
	UINT						        m_nCursorFlags;

  CBitmap*                    m_pBitmap;
  CDCEx*                      m_pPrintDC;
	CDCEx*                      m_pDrawDC;
	CRectTracker*               m_pTracker;
	CRect                       m_clInnerRect;
	CRect                       m_clCurrentMeasure;
	CRect                       m_OldCursorRect;
	CRect                       m_clPrintRect;
	CPoint                      m_OldPoint;
	EOperation                  m_opOperation;
	int                         m_nSnappedCurve;
	int                         m_nSnappedCurve1;
	int                         m_nLeftMargin,
								              m_nTopMargin,
							              	m_nRightMargin,
								              m_nBottomMargin;
	int                         m_nAxisSpace;
	int                         m_nBitmapFlags;
	int							            m_nForcedSnapCurve;
	CRect                       m_clCurrentZoom;
	CPoint				          		m_MouseDownPoint,
								              m_oldCursorPoint,
							              	m_MouseUpPoint,
							               	m_CurrentPoint;
	bool 				             		m_bLButtonDown;
	bool 						            m_bRButtonDown;
	bool						            m_bObjectSelected;
	double						          m_fCurrentEditValue;
	bool						            m_bDataPointMoving;
	int				                  m_nSelectedSerie;
	double                      m_fSnappedXVal,
		                          m_fSnappedYVal;
	CString                     m_cPrintHeader;
	CString                     m_cPrintFooter;
	CXGraphLabel                m_CursorLabel;
	MeasureDef                  m_MeasureDef;
  bool                        m_disableClipping;

	vector <MeasureDef>         m_Measures;
	vector <SelectByMarker>     m_SelectByMarker;
	vector <CXGraphDataSerie>	  m_Data;
	vector <CXGraphAxis>		    m_XAxis;
	vector <CXGraphAxis>		    m_YAxis;
	CObList				              m_Objects;
	CXGraphObject*              m_pCurrentObject;
	UINT                        m_nCFBitmap;
  EPanMode m_panMode;

	void AdjustPointToData(CPoint& point);
	void InsertEmptyLabel();
	bool CheckObjectSelection(bool bEditAction = false, bool bCheckFocusOnly = false);
	void GetMinMaxForData (CXGraphDataSerie& serie, double& fXMin, double& fXMax, double& fYMin, double& fYMax);
	int  GetAxisIndex(CXGraphAxis* pAxis);
	int  GetCurveIndex(CXGraphDataSerie* pSerie);
	void DrawLegend(CDCEx *pDC, CRect& ChartRect);
	void DrawCursorLegend (CDCEx* pDC);
	void DrawBackgroundBitmap(CDCEx* pDC, CRect clChartRect);
	void DoZoom();
  void DoMeasure();
	void DoPan(CPoint point);
	void DrawZoom(CDCEx* pDC);
	void DrawMeasure(CDCEx* pDC, CRect measureRect);
	void DrawMeasures(CDCEx* pDC);
	void DrawCursor(CDCEx* pDC);
	void LinearTrend();
	void CubicTrend();
	void OnProperties();
#ifndef _WIN32_WCE
	void OnPrint();
#endif
};

#if _MSC_VER < 1300

	// CArchive operators for C++ bool, guess the guys at microsoft have forgotten
	// this after introducing the "bool" type
	inline CArchive& operator <<(CArchive& ar, bool b)
	{
		ar << (b ? TRUE : FALSE);
		return ar;
	};

	inline CArchive& operator >>(CArchive& ar, bool& b)
	{
		BOOL old_b;
		ar >> old_b;
		(old_b ? b = true : b = false);
		return ar;
	};

#endif

////////////////////////////////////////////////////////////////////////////////
