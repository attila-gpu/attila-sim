// XGRAPH.cpp : Definiert den Einsprungpunkt für die DLL-Anwendung.
//

#include "stdafx.h"
#include "XGRAPH.h"
#include "XGraphLabel.h"
#ifndef _WIN32_WCE
#include "AxisDlg.h"
#include "ChartPage.h"
#include "CurveDlg.h"
#include "ChartDlg.h"
#endif
#include <afxpriv.h>
#include "xgraph_resources.h"
#include "float.h"
#include "GfxUtils.h"
#include "math.h"
#include "MemDC.h"
#include "WinGDI.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// XGraph.cpp: Implementation
//

/////////////////////////////////////////////////////////////////////////////
// CXGraph

IMPLEMENT_SERIAL( CXGraph, CWnd, 1 )
  
#pragma warning (disable : 4244)
#pragma warning (disable : 4800)

CXGraph::NM_POINTMOVING CXGraph::m_nmCurrentPointMoving;
	
CXGraph::CXGraph()
{
	WNDCLASS wndcls;

	// setup drawing function pointers
	m_pDrawFn[0] = DrawRect;
	m_pDrawFn[1] = DrawCircle;
	m_pDrawFn[2] = DrawCross;
	m_pDrawFn[3] = DrawRhombus;
	m_pDrawFn[4] = DrawLeftTriangle;
	m_pDrawFn[5] = DrawUpTriangle;
	m_pDrawFn[6] = DrawRightTriangle;
	m_pDrawFn[7] = DrawDownTriangle;
	
  HINSTANCE hInst   = AfxGetInstanceHandle();
	m_crBackColor     = RGB(255,255,255);
	m_crGraphColor    = RGB(255,255,255);
	m_crInnerColor    = RGB(240,240,240);
	m_nLeftMargin     = 
	m_nTopMargin      = 
	m_nRightMargin    =
	m_nBottomMargin   = 5;
	m_LegendAlignment = right;
	m_bLButtonDown    = false;
	m_bRButtonDown    = false;
	m_bObjectSelected = false;
	m_bTmpDC		      = false;
	m_bDoubleBuffer   = true;
	m_bShowLegend     = true;
	m_bInteraction    = true;
	m_bDataPointMoving= false;
	m_nSelectedSerie  = -1;
	m_pTracker        = NULL;
	m_pCurrentObject  = NULL;
	m_pBitmap         = NULL;
  m_pPrintDC        = NULL;
	m_pDrawDC         = NULL;
	m_nAxisSpace      = 5;
	m_nCursorFlags    = XGC_LEGEND | XGC_VERT | XGC_HORIZ;
	m_OldPoint        = CPoint(0,0);
	m_opOperation	  = opNone;
	m_bSnapCursor     = true;
	m_nSnapRange      = 10;
	m_cPrintHeader    = _T("");
	m_cPrintFooter    = _T("");
	m_pCurrentObject  = NULL;
	m_nForcedSnapCurve = -1;
	m_nSnappedCurve1   = 0;
	m_fCurrentEditValue = 0.0;
  m_panMode           = BothAxis;
  m_disableClipping   = false;
	
	m_OldCursorRect.SetRectEmpty();

	// Create one legend for the cursor
	m_CursorLabel.m_bVisible = false;
	m_CursorLabel.m_bCanEdit = false;
	m_CursorLabel.m_bBorder  = true;
	m_CursorLabel.m_clRect.SetRect(100,100,200,175);

	m_Objects.AddHead (&m_CursorLabel);

    // Register window class
    if (!(::GetClassInfo(hInst, _T("XGraph"), &wndcls)))
    {
        wndcls.style            = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wndcls.lpfnWndProc      = ::DefWindowProc;
        wndcls.cbClsExtra       = 0;
		    wndcls.cbWndExtra		= 0;
        wndcls.hInstance        = hInst;
        wndcls.hIcon            = NULL;
        wndcls.hCursor          = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
        wndcls.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
        wndcls.lpszMenuName     = NULL;
        wndcls.lpszClassName    = _T("XGraph");

        if (!AfxRegisterClass(&wndcls))
        {
            AfxThrowResourceException();
            return ;
        }
    }
}

CXGraph::~CXGraph()
{
	ResetAll();
}



BEGIN_MESSAGE_MAP(CXGraph, CWnd)
	//{{AFX_MSG_MAP(CXGraph)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SETCURSOR()
#ifndef _WIN32_WCE
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
#endif
	ON_WM_KEYUP()
	ON_COMMAND(IDM_ZOOM, Zoom )
	ON_COMMAND(IDM_PAN, Pan )
	ON_COMMAND(IDM_CURSOR, Cursor )
	ON_COMMAND(IDM_MEASURE, Measure)
  ON_COMMAND(IDM_DELETE_MEASURES, DeleteMeasures)
	ON_COMMAND(IDM_SELECT, NoOp )
	ON_COMMAND(IDM_RESET, ResetZoom )
	ON_COMMAND(IDM_INSERTLABEL, InsertEmptyLabel)
	ON_COMMAND(IDM_PROPERTIES, OnProperties)
#ifndef _WIN32_WCE
	ON_COMMAND(IDM_PRINT, OnPrint)
#endif
	ON_COMMAND(IDM_LINEARTREND, LinearTrend)
	ON_COMMAND(IDM_CUBICTREND, CubicTrend)
	ON_COMMAND(IDM_PARENTCALLBACK, ParentCallback)
	ON_COMMAND(IDM_ZOOM_BACK, RestoreLastZoom)
	ON_COMMAND(IDM_EDITCURVE, Edit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


bool CXGraph::DeleteXAxis(int nAxis)
{
	if (nAxis < 0 || nAxis >= static_cast<int>(m_XAxis.size()))
		return false;

	m_XAxis.erase (m_XAxis.begin() + nAxis);
	
	return true;
}

bool CXGraph::DeleteYAxis(int nAxis)
{
	if (nAxis < 0 || nAxis >= static_cast<int>(m_YAxis.size()))
		return false;

	m_YAxis.erase (m_YAxis.begin() + nAxis);
	
	return true;
}

bool CXGraph::DeleteCurve(int nCurve)
{
	if (nCurve < 0 || nCurve >= static_cast<int>(m_Data.size()))
		return false;
	
	m_Data.erase (m_Data.begin() + nCurve);

	return true;
}

CXGraphDataSerie& CXGraph::SetData(TDataPoint* pValues, long nCount, int nCurve, int nXAxis, int nYAxis, bool bAutoDelete)
{
	ASSERT (nCurve <= static_cast<int>(m_Data.size()));
	ASSERT (nXAxis <= static_cast<int>(m_XAxis.size()));
	ASSERT (nYAxis <= static_cast<int>(m_YAxis.size()));

			
	if (pValues == NULL || nCount == 0)
		return (CXGraphDataSerie&)*((CXGraphDataSerie*)NULL);


	if (nCurve == m_Data.size())
	{
		// New data serie

		CXGraphDataSerie serie;
		
		serie.m_bAutoDelete = bAutoDelete;
		serie.m_nXAxis		= nXAxis;
		serie.m_nYAxis		= nYAxis;
		serie.m_nCount		= nCount;
		serie.m_nIndex      = nCurve;

		if (bAutoDelete)
		{
			serie.m_pData = (TDataPoint*) new TDataPoint[nCount];
			memcpy(serie.m_pData, pValues, sizeof(TDataPoint) * nCount);
		}
		else
			serie.m_pData = pValues;
		
		serie.m_crColor = BASECOLORTABLE[m_Data.size () % (sizeof(BASECOLORTABLE) / sizeof(COLORREF))];

		m_Data.push_back (serie);
	}
	else
	{
		// existing data serie, just update

		m_Data[nCurve].m_nXAxis = nXAxis;
		m_Data[nCurve].m_nYAxis = nYAxis;
		m_Data[nCurve].m_pData  = pValues;
		m_Data[nCurve].m_nCount = nCount;
		m_Data[nCurve].m_bAutoDelete = bAutoDelete;
	}

	// Need additional X-Axis
	if (nXAxis == m_XAxis.size())
	{
		CXGraphAxis axis;
		axis.m_pGraph   = this;
		axis.m_AxisKind = CXGraphAxis::xAxis;
    axis.SetShowMarker(false);
		m_XAxis.push_back (axis);
	}
	
	// Need additional Y-Axis
	if (nYAxis == m_YAxis.size())
	{
		CXGraphAxis axis;
		axis.m_pGraph   = this;
		axis.m_AxisKind = CXGraphAxis::yAxis;
    axis.SetShowMarker(false);
		m_YAxis.push_back (axis);
	}

	m_Data[nCurve].m_pGraph = this;
	
	Autoscale(nXAxis, nYAxis, nCurve);
		
	return m_Data[nCurve];
}

////////////////////////////////////////////////////////////////////////////////

void CXGraph::Autoscale(int nXAxis, int nYAxis, int nCurve)
{
	NoOp();
  DeleteZooms();
  
  // Get min/max for data serie
	double fxMin = m_XAxis[nXAxis].m_fMin;
	double fxMax = m_XAxis[nXAxis].m_fMax;
	double fyMin = m_YAxis[nYAxis].m_fMin;
	double fyMax = m_YAxis[nYAxis].m_fMax;

	GetMinMaxForData(m_Data[nCurve], fxMin, fxMax, fyMin, fyMax);

	// Set axis ranges
	VERIFY(m_XAxis[nXAxis].SetRange(fxMin, fxMax));
	VERIFY(m_YAxis[nYAxis].SetRange(fyMin, fyMax));
}

////////////////////////////////////////////////////////////////////////////////

void CXGraph::AutoscaleVisible(int nXAxis, int nYAxis)
{
  NoOp();
  DeleteZooms();
  
  double fxMin = DBL_MAX;
  double fxMax = -DBL_MAX;
  double fyMin = DBL_MAX;
  double fyMax = -DBL_MAX;

  unsigned int numVisibles = 0;
  for (size_t i=0; i < m_Data.size(); ++i)
  {
    if (m_Data[i].GetVisible())
    {
      GetMinMaxForData(m_Data[i], fxMin, fxMax, fyMin, fyMax);
      numVisibles++;
    }
  }

  if (numVisibles)
  {
    if (nXAxis >= 0) VERIFY(m_XAxis[nXAxis].SetCurrentRange(fxMin, fxMax));
    if (nYAxis >= 0) VERIFY(m_YAxis[nYAxis].SetCurrentRange(fyMin, fyMax));
  }
  else
  {
    if (nXAxis >= 0) m_XAxis[nXAxis].Reset();
    if (nYAxis >= 0) m_YAxis[nYAxis].Reset();
  }
}

////////////////////////////////////////////////////////////////////////////////

void CXGraph::AutoscaleAll(int nXAxis, int nYAxis)
{
  NoOp();
  DeleteZooms();
  
  double fxMin = DBL_MAX;
  double fxMax = -DBL_MAX;
  double fyMin = DBL_MAX;
  double fyMax = -DBL_MAX;

  for (size_t i=0; i < m_Data.size(); ++i)
  {
    GetMinMaxForData(m_Data[i], fxMin, fxMax, fyMin, fyMax);
  }

  if (nXAxis >= 0 && m_Data.size()) VERIFY(m_XAxis[nXAxis].SetCurrentRange(fxMin, fxMax));
  if (nYAxis >= 0 && m_Data.size()) VERIFY(m_YAxis[nYAxis].SetCurrentRange(fyMin, fyMax));
}

////////////////////////////////////////////////////////////////////////////////

CXGraphAxis& CXGraph::GetXAxis(int nAxis)
{
	ASSERT(nAxis >= 0 && nAxis < static_cast<int>(m_XAxis.size()));
	return m_XAxis[nAxis]; 
}

CXGraphAxis& CXGraph::GetYAxis(int nAxis)
{
	ASSERT(nAxis >= 0 && nAxis < static_cast<int>(m_YAxis.size()));
	return m_YAxis[nAxis]; 
}

CXGraphDataSerie& CXGraph::GetCurve(int nCurve)
{
	ASSERT(nCurve >= 0 && nCurve < static_cast<int>(m_Data.size()));
	return m_Data[nCurve];
}

TDataPoint CXGraph::GetCursorAbsolute(int nCurve)
{
	TDataPoint vPoint;
	long     nIndex;

	vPoint.fXVal = m_XAxis[m_Data[nCurve].m_nXAxis].GetValueForPos (m_CurrentPoint.x);

	double fSnappedXVal = m_XAxis[m_Data[nCurve].m_nXAxis].GetValueForPos (m_CurrentPoint.x);
	// Find index for this value
	m_XAxis[m_Data[nCurve].m_nXAxis].GetIndexByXVal(nIndex, fSnappedXVal, nCurve);
	// get yval for index
	vPoint.fYVal = m_Data[nCurve].m_pData[nIndex].fYVal;

	return vPoint;
}

void CXGraph::SetCursorAbsolute(int nCurve, TDataPoint vPoint, bool bForceVisible )
{
	long nIndex;

	m_CurrentPoint = m_XAxis[m_Data[nCurve].m_nXAxis].GetPointForValue (&vPoint);

	m_XAxis[m_Data[nCurve].m_nXAxis].GetIndexByXVal(nIndex, vPoint.fXVal, nCurve);
	vPoint.fYVal = m_Data[nCurve].m_pData[nIndex].fYVal;


	if (m_nCursorFlags & XGC_ADJUSTSMOOTH && bForceVisible)
	{
		double fRange = m_XAxis[m_Data[nCurve].m_nXAxis].m_fCurMax - m_XAxis[m_Data[nCurve].m_nXAxis].m_fCurMin;
		m_XAxis[m_Data[nCurve].m_nXAxis].SetCurrentRange(vPoint.fXVal -  fRange / 2, vPoint.fXVal + fRange / 2);
	}
	else
	if ( (vPoint.fXVal < m_XAxis[m_Data[nCurve].m_nXAxis].m_fCurMin || 
		  vPoint.fXVal > m_XAxis[m_Data[nCurve].m_nXAxis].m_fCurMax ) &&
		  bForceVisible)
	{
		double fRange = m_XAxis[m_Data[nCurve].m_nXAxis].m_fCurMax - m_XAxis[m_Data[nCurve].m_nXAxis].m_fCurMin;
		m_XAxis[m_Data[nCurve].m_nXAxis].SetCurrentRange(vPoint.fXVal - (fRange / 2.0), vPoint.fXVal + (fRange / 2.0));
	}

	if (m_nCursorFlags & XGC_ADJUSTSMOOTH && bForceVisible)
	{
		double fRange = m_YAxis[m_Data[nCurve].m_nYAxis].m_fCurMax - m_YAxis[m_Data[nCurve].m_nYAxis].m_fCurMin;
		m_YAxis[m_Data[nCurve].m_nYAxis].SetCurrentRange(vPoint.fYVal - (fRange / 2.0), vPoint.fYVal + (fRange / 2.0));
	}
	else
	if ( (vPoint.fYVal < m_YAxis[m_Data[nCurve].m_nYAxis].m_fCurMin || 
		  vPoint.fYVal > m_YAxis[m_Data[nCurve].m_nYAxis].m_fCurMax ) &&
		  bForceVisible)
	{
		double fRange = m_YAxis[m_Data[nCurve].m_nYAxis].m_fCurMax - m_YAxis[m_Data[nCurve].m_nYAxis].m_fCurMin;
		m_YAxis[m_Data[nCurve].m_nYAxis].SetCurrentRange(vPoint.fYVal - (fRange / 2.0), vPoint.fYVal + (fRange / 2.0));
	}

	Invalidate();

}

void CXGraph::GetMinMaxForData (CXGraphDataSerie& serie, double& fXMin, double& fXMax, double& fYMin, double& fYMax)
{	
	// check mins and maxs
	for (long i = 0; i < serie.m_nCount; i++)
	{
		if (serie.m_pData[i].fXVal > fXMax)
			fXMax = serie.m_pData[i].fXVal;

		if (serie.m_pData[i].fXVal < fXMin)
			fXMin = serie.m_pData[i].fXVal;

		if (serie.m_pData[i].fYVal > fYMax)
			fYMax = serie.m_pData[i].fYVal;

		if (serie.m_pData[i].fYVal < fYMin)
			fYMin = serie.m_pData[i].fYVal;
	}
}

// Returns the index for the given axis
int CXGraph::GetAxisIndex(CXGraphAxis* pAxis)
{
	for (int x = 0; x < static_cast<int>(m_XAxis.size()); x++)
		if (pAxis == &m_XAxis[x])
			return x;
	
	for (int y = 0; y < static_cast<int>(m_YAxis.size()); y++)
		if (pAxis == &m_YAxis[y])
			return y;

	return -1;
}

// Returns the index for the given curve
int CXGraph::GetCurveIndex(CXGraphDataSerie* pSerie)
{
	for (int i = 0; i < static_cast<int>(m_Data.size()); i++)
		if (pSerie == &m_Data[i])
			return i;

	return -1;
}

void CXGraph::SetGraphMargins(int nLeft, int nTop, int nRight, int nBottom)
{
	m_nLeftMargin   = nLeft;
	m_nTopMargin    = nTop;
	m_nRightMargin  = nRight;
	m_nBottomMargin = nBottom;
}

void CXGraph::DrawMeasures (CDCEx* pDC)
{
	for (UINT i = 0; i < m_Measures.size(); i++)
	{
		CRect measureRect;
						
		measureRect.left = m_XAxis[m_Data[m_Measures[i].nCurve1].m_nXAxis].GetPointForValue(&m_Data[m_Measures[i].nCurve1].m_pData[m_Measures[i].nIndex1]).x;
		measureRect.right= m_XAxis[m_Data[m_Measures[i].nCurve2].m_nXAxis].GetPointForValue(&m_Data[m_Measures[i].nCurve2].m_pData[m_Measures[i].nIndex2]).x;
		
		measureRect.top = m_YAxis[m_Data[m_Measures[i].nCurve1].m_nYAxis].GetPointForValue(&m_Data[m_Measures[i].nCurve1].m_pData[m_Measures[i].nIndex1]).y;
		measureRect.bottom = m_YAxis[m_Data[m_Measures[i].nCurve2].m_nYAxis].GetPointForValue(&m_Data[m_Measures[i].nCurve2].m_pData[m_Measures[i].nIndex2]).y;
		
		DrawMeasure(pDC, measureRect);
	}
}

void CXGraph::DrawMeasure(CDCEx* pDC, CRect measureRect)
{
	COLORREF crColor = RGB(255,0,0);
	COLORREF crColorGray = RGB(200, 200, 200);
	int      nArrowSize = 6;
	CString  cMarker;
	long	 nIndex1, nIndex2;

	UINT nOldBkColor = pDC->SetBkColor(m_crInnerColor);
	
	CQuickFont font("Arial", -11, FW_BOLD);
	CFontSelector fs(&font, pDC, false);

	if (measureRect.IsRectEmpty() && m_opOperation == opMeasure)
	{
		measureRect.SetRect(m_MouseDownPoint.x, m_MouseDownPoint.y, m_CurrentPoint.x, m_CurrentPoint.y);
	
		measureRect.left = max(measureRect.left, m_clInnerRect.left);
		measureRect.top = max(measureRect.top, m_clInnerRect.top);
		measureRect.right = min(measureRect.right, m_clInnerRect.right);
		measureRect.bottom = min(measureRect.bottom, m_clInnerRect.bottom);

		m_clCurrentMeasure = measureRect;
	}


	{
		CPenSelector ps(crColorGray, 1, pDC, PS_DOT);

		pDC->MoveTo (m_clInnerRect.left, measureRect.top);
		pDC->LineTo (m_clInnerRect.right, measureRect.top);

		pDC->MoveTo (measureRect.right, m_clInnerRect.top);
		pDC->LineTo (measureRect.right, m_clInnerRect.bottom);
	}	
	
  {
		CPenSelector ps(crColor, 1, pDC, PS_DOT);

		pDC->MoveTo (measureRect.left, measureRect.top);
		pDC->LineTo (measureRect.right, measureRect.top);
		pDC->LineTo (measureRect.right, measureRect.bottom);

		if (abs(measureRect.Width()) > nArrowSize )
			DrawLeftTriangle(CPoint(measureRect.left, measureRect.top - (nArrowSize/2)), nArrowSize, crColor, false, pDC);
		else
			DrawUpTriangle(CPoint(measureRect.right - (nArrowSize/2), measureRect.top + (nArrowSize/2)), nArrowSize, crColor, false, pDC);

		if (abs(measureRect.Height()) > nArrowSize )
			DrawDownTriangle(CPoint(measureRect.right - (nArrowSize/2), measureRect.bottom - nArrowSize), nArrowSize, crColor, false, pDC);
		else
			DrawRightTriangle(CPoint(measureRect.right - nArrowSize, measureRect.bottom - nArrowSize), nArrowSize, crColor, false, pDC);
	}
	

	double fSnappedX1 = m_XAxis[m_Data[m_nSnappedCurve].m_nXAxis].GetValueForPos (measureRect.left);
	double fSnappedX2 = m_XAxis[m_Data[m_nSnappedCurve1].m_nXAxis].GetValueForPos (measureRect.right);
	
	double fX = fSnappedX2 - fSnappedX1;

	if (m_XAxis[m_Data[m_nSnappedCurve].m_nXAxis].GetDateTime())
#ifndef _WIN32_WCE
		cMarker = COleDateTime(fX).Format(m_XAxis[m_Data[m_nSnappedCurve].m_nXAxis].GetDisplayFmt());
#else
		cMarker = COleDateTime(fX).Format();
#endif
	else
		cMarker.Format(m_XAxis[m_Data[m_nSnappedCurve].m_nXAxis].GetDisplayFmt(), fX);

		
	pDC->SetBkMode(TRANSPARENT);

	if (abs(measureRect.Width()) > pDC->GetTextExtent(cMarker).cx)
		pDC->DrawText(cMarker, measureRect, DT_CENTER | DT_SINGLELINE | DT_TOP | DT_NOCLIP);
		
	m_XAxis[m_Data[m_nSnappedCurve].m_nXAxis].GetIndexByXVal(nIndex1, fSnappedX1, m_nSnappedCurve);
	m_XAxis[m_Data[m_nSnappedCurve1].m_nXAxis].GetIndexByXVal(nIndex2, fSnappedX2, m_nSnappedCurve1);
		
	double fSnappedY1 = m_Data[m_nSnappedCurve].m_pData[nIndex1].fYVal;
	double fSnappedY2 = m_Data[m_nSnappedCurve1].m_pData[nIndex2].fYVal;

	double fY = fabs(fSnappedY2 - fSnappedY1);

	cMarker.Format(m_YAxis[m_Data[m_nSnappedCurve].m_nYAxis].GetDisplayFmt(), fY);
	cMarker += (" " + m_YAxis[m_Data[m_nSnappedCurve].m_nYAxis].GetLabel());

	if (abs(measureRect.Height()) > pDC->GetTextExtent(cMarker).cy)
		pDC->DrawText(cMarker, measureRect, DT_RIGHT | DT_SINGLELINE | DT_VCENTER | DT_NOCLIP);

	pDC->SetBkColor(nOldBkColor);
}

void CXGraph::DrawZoom (CDCEx* pDC)
{
	CRect zoomRect(m_MouseDownPoint.x, m_MouseDownPoint.y, m_CurrentPoint.x, m_CurrentPoint.y);

	zoomRect.NormalizeRect ();

	zoomRect.left = max(zoomRect.left, m_clInnerRect.left);
	zoomRect.top = max(zoomRect.top, m_clInnerRect.top);
	zoomRect.right = min(zoomRect.right, m_clInnerRect.right);
	zoomRect.bottom = min(zoomRect.bottom, m_clInnerRect.bottom);


	// If not in doublebuffer mode and zoomrect is not empty delete old zoomrect
	if (!m_bDoubleBuffer && !m_clCurrentZoom.IsRectEmpty ())
		pDC->DrawFocusRect (m_clCurrentZoom);

	m_clCurrentZoom = zoomRect;

	if (m_bDoubleBuffer)
	{
		// In db-mode we use a semi-transparent zoomrect ...
		CBrushSelector bs(RGB(150,150,255), pDC);
		COLORREF crOldBkColor = pDC->SetBkColor (m_crInnerColor); 
		int	nOldROP2 = pDC->SetROP2 (R2_NOTXORPEN);

		pDC->Rectangle(zoomRect);
		pDC->SetBkColor (crOldBkColor);
		pDC->SetROP2 (nOldROP2);
		
	}
	
	pDC->DrawFocusRect (m_clCurrentZoom);
	
}

CXGraphLabel& CXGraph::InsertLabel(CRect rect, CString cText)
{
	CXGraphLabel *pLabel = new CXGraphLabel;

	pLabel->m_clRect = rect;
	pLabel->m_cText  = cText;
	pLabel->m_pGraph = this;
	
	m_Objects.AddTail (pLabel);

	return *pLabel;
}

CXGraphLabel& CXGraph::InsertLabel(CString cText)
{
	CXGraphLabel *pLabel = new CXGraphLabel;

	CRect rect(m_clInnerRect.CenterPoint().x - 50,m_clInnerRect.CenterPoint().y - 50,
		       m_clInnerRect.CenterPoint().x + 50,m_clInnerRect.CenterPoint().y + 50);
	
	pLabel->m_clRect = rect;
	pLabel->m_cText  = cText;
	pLabel->m_pGraph = this;
	
	m_Objects.AddTail (pLabel);

	return *pLabel;
}

void CXGraph::Cursor()
{
	Pan();
	m_nSnappedCurve = m_nForcedSnapCurve;
	m_oldCursorPoint = CPoint(-1, -1);
	m_opOperation = opCursor;
	
	if (m_nCursorFlags & XGC_LEGEND)
		m_CursorLabel.m_clRect.SetRect(m_clInnerRect.left + 1, m_clInnerRect.top , m_clInnerRect.left + 150, m_clInnerRect.top + 50);
	

}

////////////////////////////////////////////////////////////////////////////////

void CXGraph::Measure()
{
	DeleteMeasures();
  m_opOperation = opMeasure;
	m_clCurrentMeasure.SetRectEmpty();
	Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

void CXGraph::Zoom()
{
	m_CursorLabel.m_bVisible = false;
	m_clCurrentZoom.SetRectEmpty ();
	m_opOperation = opZoom;
	Invalidate();
}

void CXGraph::Edit()
{
	m_opOperation = opEditCurve;
	Invalidate();
}

void CXGraph::ResetZoom()
{
	m_CursorLabel.m_bVisible = false;
	for (int y = 0; y < static_cast<int>(m_YAxis.size()); y++)
		m_YAxis[y].Reset();
	for (int x = 0; x < static_cast<int>(m_XAxis.size()); x++)
		m_XAxis[x].Reset();
	Invalidate();
}

void CXGraph::NoOp()
{
	m_CursorLabel.m_bVisible = false;
	m_opOperation = opNone;
	Invalidate();
}

void CXGraph::Pan()
{
	m_CursorLabel.m_bVisible = false;
	m_opOperation = opPan;

	for (int y = 0; y < static_cast<int>(m_YAxis.size()); y++)
		m_YAxis[y].m_bAutoScale = false;
	
	for (int x = 0; x < static_cast<int>(m_XAxis.size()); x++)
		m_XAxis[x].m_bAutoScale = false;

	Invalidate();
}

void CXGraph::DoPan(CPoint point)
{
  if (m_panMode & AxisYOnly)
  {
    for (int y = 0; y < static_cast<int>(m_YAxis.size()); y++)
    {
      double fY1 = m_YAxis[y].GetValueForPos (point.y);
      double fY2 = m_YAxis[y].GetValueForPos (m_OldPoint.y);
      double fOffset = fY1 - fY2;

      // PERFER : No volem baixar mai de 0.0 [
      if ((m_YAxis[y].m_fCurMin - fOffset) < 0.0)
      {
        fOffset = m_YAxis[y].m_fCurMin;
      }
      // ]

      VERIFY(m_YAxis[y].SetCurrentRange(m_YAxis[y].m_fCurMin - fOffset,m_YAxis[y].m_fCurMax - fOffset));
    }
  }

	if (m_panMode & AxisXOnly)
  {
    for (int x = 0; x < static_cast<int>(m_XAxis.size()); x++)
    {
      double fX1 = m_XAxis[x].GetValueForPos (point.x);
      double fX2 = m_XAxis[x].GetValueForPos (m_OldPoint.x);
      double fOffset = fX1 - fX2;
      
      // PERFER : No volem baixar mai de 0.0 [
      if ((m_XAxis[x].m_fCurMin - fOffset) < 0.0)
      {
        fOffset = m_XAxis[x].m_fCurMin;
      }
      // ]
      
      VERIFY(m_XAxis[x].SetCurrentRange(m_XAxis[x].m_fCurMin - fOffset,m_XAxis[x].m_fCurMax - fOffset));
    }
  }

	::PostMessage(GetParent()->m_hWnd, XG_PANCHANGE, 0, (long) this);
  
  m_OldPoint = point;
	Invalidate();
}

void CXGraph::DoZoom()
{		
	if (m_clCurrentZoom.Width () < MIN_ZOOM_PIXELS ||
		  m_clCurrentZoom.Height () < MIN_ZOOM_PIXELS)
  {
		return;
  }

	ZOOM zoom;
			
	for (UINT y = 0; y < m_YAxis.size(); y++)
	{
		double fMin, fMax;

		fMin = m_YAxis[y].GetValueForPos(min(m_clCurrentZoom.bottom, m_clInnerRect.bottom));
		fMax = m_YAxis[y].GetValueForPos(max(m_clCurrentZoom.top, m_clInnerRect.top));

		if ((fMax - fMin) <= 1.0)
    {
      m_clCurrentZoom.SetRectEmpty();
      return;
    }
    else
    if ((fMax - fMin) != 0.0)
		{			
			zoom.fMin = m_YAxis[y].m_fCurMin;
			zoom.fMax = m_YAxis[y].m_fCurMax;
	
			m_YAxis[y].m_ZoomHistory.push_back (zoom);

			m_YAxis[y].m_fCurMin = fMin;
			m_YAxis[y].m_fCurMax = fMax;
			m_YAxis[y].SetBestStep();
		}
	}

	for (UINT x = 0; x < m_XAxis.size(); x++)
	{
		double fMin, fMax;

		fMin = m_XAxis[x].GetValueForPos(max(m_clCurrentZoom.left, m_clInnerRect.left));
		fMax = m_XAxis[x].GetValueForPos(min(m_clCurrentZoom.right, m_clInnerRect.right));
		
		if ((fMax - fMin) <= 1.0)
    {
      m_clCurrentZoom.SetRectEmpty();
      return;
    }
    else
    if ((fMax - fMin) != 0.0)
		{
			zoom.fMin = m_XAxis[x].m_fCurMin;
			zoom.fMax = m_XAxis[x].m_fCurMax;
	
			m_XAxis[x].m_ZoomHistory.push_back (zoom);
	
			m_XAxis[x].m_fCurMin = fMin;
			m_XAxis[x].m_fCurMax = fMax;
			m_XAxis[x].SetBestStep();
		}
	}

	::PostMessage(GetParent()->m_hWnd, XG_ZOOMCHANGE, 0, (long) this);
	m_clCurrentZoom.SetRectEmpty();
}

////////////////////////////////////////////////////////////////////////////////

void CXGraph::DoMeasure()
{
  ::PostMessage(GetParent()->m_hWnd, XG_MEASURECREATED, 0, (long) this);
}

////////////////////////////////////////////////////////////////////////////////

int CXGraph::GetZoomDepth()
{
	if (m_XAxis.size() > 0)
  {
		return m_XAxis[0].m_ZoomHistory.size();
  }
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

void CXGraph::RestoreLastZoom()
{		
	for (UINT y = 0; y < m_YAxis.size(); y++)
	{
		if (m_YAxis[y].m_ZoomHistory.size() > 0)
		{
			m_YAxis[y].m_fCurMin = m_YAxis[y].m_ZoomHistory[m_YAxis[y].m_ZoomHistory.size() - 1].fMin;
			m_YAxis[y].m_fCurMax = m_YAxis[y].m_ZoomHistory[m_YAxis[y].m_ZoomHistory.size() - 1].fMax;
			m_YAxis[y].m_ZoomHistory.pop_back ();
			m_YAxis[y].SetBestStep ();
		}
	}

	for (UINT x = 0; x < m_XAxis.size(); x++)
	{
		if (m_XAxis[x].m_ZoomHistory.size() > 0)
		{
			m_XAxis[x].m_fCurMin = m_XAxis[x].m_ZoomHistory[m_XAxis[x].m_ZoomHistory.size() - 1].fMin;
			m_XAxis[x].m_fCurMax = m_XAxis[x].m_ZoomHistory[m_XAxis[x].m_ZoomHistory.size() - 1].fMax;
			m_XAxis[x].m_ZoomHistory.pop_back ();
			m_XAxis[x].SetBestStep ();
		}
	}
	
	::PostMessage(GetParent()->m_hWnd, XG_ZOOMCHANGE, 0, (long) this);
	m_clCurrentZoom.SetRectEmpty ();
	Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

void CXGraph::DeleteMeasures()
{
  m_Measures.clear();
  Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

void CXGraph::DeleteZooms()
{
  for (UINT y = 0; y < m_YAxis.size(); y++)
  {
    m_YAxis[y].m_ZoomHistory.clear();
  }

  for (UINT x = 0; x < m_XAxis.size(); x++)
  {
    m_XAxis[x].m_ZoomHistory.clear();
  }
}

////////////////////////////////////////////////////////////////////////////////

void CXGraph::DrawLegend(CDCEx *pDC, CRect& ChartRect)
{
	int nLegendWidth = 0;
	int nLegendHeight = 0;
	int nItemHeight = 0;

	CQuickFont font("Arial", 13, FW_NORMAL);
	{
		CFontSelector fs(&font, pDC, false);

		for (UINT i = 0; i < m_Data.size(); i++)
		{
			if (!m_Data[i].m_bVisible)
				continue;

			CString cLegend;
			cLegend.Format(_T("%02d %s"), i + 1, m_Data[i].m_cLabel);
			nLegendWidth = __max(nLegendWidth, pDC->GetTextExtent (cLegend).cx);
			nLegendHeight += (nItemHeight = pDC->GetTextExtent (cLegend).cy);
		}
	}

	CFontSelector fs(&font, pDC);

	nLegendWidth += 16;

	CRect legendRect;
	
	if (m_LegendAlignment == left)
	{
		legendRect.SetRect (ChartRect.left + 10, ChartRect.top + 10, ChartRect.left + 10 + nLegendWidth, ChartRect.top + 12 + nLegendHeight);
		ChartRect.left += (nLegendWidth + 20);
	}
	
	if (m_LegendAlignment == right)
	{
		legendRect.SetRect (ChartRect.right - 10 - nLegendWidth, ChartRect.top + 10, ChartRect.right - 10, ChartRect.top + 12 + nLegendHeight);
		ChartRect.right -= (nLegendWidth + 20);
	}

	CPenSelector ps(0, 1, pDC);

	COLORREF brushColor = pDC->m_bMono ? RGB(255,255,255) : ::GetSysColor(COLOR_WINDOW);

	CBrushSelector bs(brushColor, pDC);
	
	pDC->Rectangle(legendRect);

	legendRect.DeflateRect (1,1,1,1);
	
	int nItem = 0;

	m_SelectByMarker.clear();

	for (UINT i = 0; i < m_Data.size(); i++)
	{
		SelectByMarker sbm;

		if (!m_Data[i].m_bVisible)
			continue;

		CString cLegend;
		cLegend.Format(_T("%02d %s"), i + 1, m_Data[i].m_cLabel);
		CRect itemRect(legendRect.left + 12, legendRect.top + nItemHeight * nItem, legendRect.right, legendRect.top + nItemHeight * nItem + nItemHeight);

		sbm.markerRect = itemRect;
		sbm.markerRect.left -= 12;
		sbm.pObject = &m_Data[i];

		m_SelectByMarker.push_back(sbm);
				
		if (pDC->m_bMono)
			pDC->SetBkColor(RGB(255,255,255));
		else
			pDC->SetBkColor(::GetSysColor(COLOR_WINDOW));

		pDC->DrawText(cLegend, itemRect, DT_LEFT);

		if (m_Data[i].m_bSelected)
			pDC->DrawFocusRect (sbm.markerRect);

		itemRect.left = legendRect.left + 2;
		itemRect.right = legendRect.left + 10;
		itemRect.DeflateRect (0,1,0,2);

		if (m_Data[i].m_nMarkerType == 0)
			pDC->FillSolidRect (itemRect, pDC->m_bMono ? 0L : m_Data[i].m_crColor);
		else
			m_YAxis[0].DrawMarker (pDC, CPoint(itemRect.left - 1, itemRect.top + 1), m_Data[i].m_nMarker, 8, pDC->m_bMono ? 0L : m_Data[i].m_crColor, true);
	
		nItem++;
	}
}

void CXGraph::OnDraw(CDC *pDC)
{
	CDCEx dc;
	dc.Attach (pDC->m_hDC);
	dc.Prepare (pDC->m_hAttribDC);
	
	m_pDrawDC = &dc;
	m_pDrawDC->m_bMono = false;
	m_pDrawDC->m_bPrinting = false;
	m_pDrawDC->m_bPrintPreview = false;
	
	m_clPrintRect.SetRect(0, 0, pDC->GetDeviceCaps(HORZRES), pDC->GetDeviceCaps(VERTRES));

	OnPaint();

	m_pDrawDC = NULL;
	dc.Detach();
}
	
void CXGraph::OnPaint() 
{
  CDCEx* pDC;
  CMemDC* pmdc = NULL;
  CPaintDC* pdc = new CPaintDC(this);
  CRect clRect;
  CRect clChartRect;
  bool bMustDeleteDC = false;
  int nSaveDC;

  m_oldCursorPoint = CPoint(-1, -1);

  if (m_pPrintDC != NULL)
  {
    pDC = new CDCEx;
    pDC->Attach(m_pPrintDC->m_hDC);
    pDC->m_bPrinting = true;
    bMustDeleteDC = true;
  }
  else
  {
    if(m_pDrawDC != NULL)
      pDC = (CDCEx*) m_pDrawDC;
    else
    {
      pDC = new CDCEx;
      bMustDeleteDC = true;
    }
  }

  if (m_pPrintDC == NULL)
  {
    if (m_bDoubleBuffer)
    {
      pmdc = new CMemDC(pdc);
      pDC->Attach(pmdc->m_hDC);
      nSaveDC = pDC->SaveDC();
      pDC->m_bMono = false;
      pDC->m_bPrinting = false;
      pDC->m_bPrintPreview = false;
    }
    else
    {
      pDC->Attach(pdc->m_hDC);
      nSaveDC = pDC->SaveDC();
      pDC->m_bMono = false;
      pDC->m_bPrinting = false;
      pDC->m_bPrintPreview = false;
    }
  }

  m_oldCursorPoint = CPoint(-1, -1);

  //////////////////////////////////////////////////////////////////////////////
  // Background
  
  if (pDC->m_bPrinting)
    clRect = m_clPrintRect;
  else
    GetClientRect(clRect);

  if (pDC->m_bMono)
    pDC->FillSolidRect(clRect, RGB(255,255,255));
  else
    pDC->FillSolidRect(clRect, m_crBackColor);

	//////////////////////////////////////////////////////////////////////////////
  // Chart
	
  clChartRect.left   = clRect.left + m_nLeftMargin;
	clChartRect.top    = clRect.top + m_nTopMargin;
	clChartRect.right  = clRect.right - m_nRightMargin;
	clChartRect.bottom = clRect.bottom - m_nBottomMargin;

	if (m_bShowLegend)
  {
		DrawLegend(pDC, clChartRect);
  }
  
  if (pDC->m_bMono)
		pDC->FillSolidRect(clChartRect, RGB(255,255,255));
	else
		pDC->FillSolidRect(clChartRect, m_crGraphColor);

	int nLeft   = clChartRect.left;
	int nRight  = clChartRect.right;
	int nBottom = clChartRect.bottom;

	CXGraphAxis::EAxisPlacement lastPlacement = CXGraphAxis::apAutomatic;

  // Calculate layout, prepare automatic axis
  for (UINT nYAxis = 0; nYAxis < m_YAxis.size(); nYAxis++)
	{
		if (m_YAxis[nYAxis].m_Placement == CXGraphAxis::apAutomatic)
		{
			if (lastPlacement == CXGraphAxis::apLeft)
			{
				lastPlacement = CXGraphAxis::apRight;
				m_YAxis[nYAxis].m_Placement = CXGraphAxis::apRight;
			}
			else
			{
				lastPlacement = CXGraphAxis::apLeft;
				m_YAxis[nYAxis].m_Placement = CXGraphAxis::apLeft;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////////
  // Y-Axis, left side
	
  for (UINT nYAxis = 0; nYAxis < m_YAxis.size(); nYAxis++)
	{
		if (m_YAxis[nYAxis].m_Placement != CXGraphAxis::apLeft)
			continue;
	
		int nTickWidth = m_YAxis[nYAxis].GetMaxLabelWidth(pDC); 
		int nLabelWidth = (m_YAxis[nYAxis].GetTitleSize(pDC).cx + m_YAxis[nYAxis].m_nArcSize);

		if (m_YAxis[nYAxis].m_bVisible)
			nLeft += (max (nTickWidth, nLabelWidth) + m_nAxisSpace );

		m_YAxis[nYAxis].m_nLeft   = nLeft;
		m_YAxis[nYAxis].m_clChart = clChartRect;
	}
	
	//////////////////////////////////////////////////////////////////////////////
  // Y-Axis, right side
	
  for (UINT nYAxis = 0; nYAxis < m_YAxis.size(); nYAxis++)
	{
		if (m_YAxis[nYAxis].m_Placement != CXGraphAxis::apRight)
			continue;

		int nTickWidth  = m_YAxis[nYAxis].GetMaxLabelWidth(pDC); 
		int nLabelWidth = m_YAxis[nYAxis].GetTitleSize(pDC).cx;
	
		if (m_YAxis[nYAxis].m_bVisible)
			nRight -= (max (nTickWidth, nLabelWidth) + m_nAxisSpace );
		
		m_YAxis[nYAxis].m_nLeft   = nRight;
		m_YAxis[nYAxis].m_clChart = clChartRect;
	}

	//////////////////////////////////////////////////////////////////////////////
  // X-Axis
	
  for (UINT nXAxis = 0; nXAxis < m_XAxis.size(); nXAxis++)
	{
		if (m_XAxis[nXAxis].m_bVisible)
			nBottom -= (m_XAxis[nXAxis].GetMaxLabelHeight(pDC) + m_nAxisSpace);

		m_XAxis[nXAxis].m_nTop    = nBottom;
		m_XAxis[nXAxis].m_nLeft   = nLeft;
		m_XAxis[nXAxis].m_clChart = clChartRect;
	}

	//////////////////////////////////////////////////////////////////////////////
  // Draw Inner Rect

	m_clInnerRect.SetRect(nLeft + 1, clChartRect.top, nRight, nBottom);
	if (pDC->m_bMono)
		pDC->FillSolidRect(m_clInnerRect, RGB(255,255,255));
	else
		pDC->FillSolidRect(m_clInnerRect, m_crInnerColor);
	
	//////////////////////////////////////////////////////////////////////////////
  // Draw axis
	
  // Y-Axis
	for (UINT nYAxis = 0; nYAxis < m_YAxis.size (); nYAxis++)
	{
		m_YAxis[nYAxis].m_nTop = nBottom;
		m_YAxis[nYAxis].Draw(pDC);
	}
	
	// X-Axis
	for (UINT nXAxis = 0; nXAxis < m_XAxis.size (); nXAxis++)
  {
		m_XAxis[nXAxis].Draw(pDC);
  }
	
	//////////////////////////////////////////////////////////////////////////////
  // Setup clipping
	
  if (!m_disableClipping)
  {
    CRgn clipRegion;
    CRect rectClip(m_clInnerRect.left, m_clInnerRect.top, m_clInnerRect.right, m_clInnerRect.bottom);

    if (pDC->m_bPrinting && !pDC->m_bPrintPreview)
    {
      pDC->AdjustRatio(rectClip);
    }

    clipRegion.CreateRectRgn(rectClip.left, rectClip.top, rectClip.right, rectClip.bottom);
    pDC->SelectClipRgn(&clipRegion);
  }

	//////////////////////////////////////////////////////////////////////////////
  // Draw curves

	for (UINT nCurve = 0; nCurve < m_Data.size (); nCurve++)
		if (m_Data[nCurve].m_bVisible)
			m_Data[nCurve].Draw(pDC);

	//////////////////////////////////////////////////////////////////////////////
  // Draw curve markers

	for (UINT nCurve = 0; nCurve < m_Data.size (); nCurve++)
		if (m_Data[nCurve].m_bVisible && m_Data[nCurve].m_bShowMarker)
			m_Data[nCurve].DrawMarker(pDC);

	//////////////////////////////////////////////////////////////////////////////
  // Draw measures
  
  DrawMeasures(pDC);

  //////////////////////////////////////////////////////////////////////////////
	// Draw zoom if active

	if (m_opOperation == opZoom && m_bLButtonDown)
  {
		DrawZoom(pDC);
  }
	
  //////////////////////////////////////////////////////////////////////////////
  // Draw measure if active
  
  if (m_opOperation == opMeasure && m_bLButtonDown)
	{
		CRect tmpRect;
		tmpRect.SetRectEmpty();
		DrawMeasure(pDC, tmpRect);
	}

	//////////////////////////////////////////////////////////////////////////////
  // Draw cursor if active
	
  if (m_opOperation == opCursor)
  {
		DrawCursor(pDC);
  }

	//////////////////////////////////////////////////////////////////////////////
  // DrawObjects
	
  for (POSITION pos = m_Objects.GetHeadPosition (); pos != NULL; )
	{
		CXGraphObject *pObject = (CXGraphObject*) m_Objects.GetNext(pos);
		pObject->Draw(pDC);
	}

	if (m_bDataPointMoving)
	{
		CQuickFont font("Arial", 13, FW_NORMAL);
		CFontSelector fs(&font, pDC, false);
		pDC->SetBkMode (TRANSPARENT);
		CString cPoint;
		cPoint.Format ("%.2f", m_fCurrentEditValue);
		pDC->TextOut(m_CurrentPoint.x + 10, m_CurrentPoint.y, cPoint);
	}

  //////////////////////////////////////////////////////////////////////////////
  // Save the current chart in bitmap
  
  CDC memDC;
  memDC.CreateCompatibleDC(pDC);
  if (!m_pBitmap)
    m_pBitmap = (CBitmap*) new CBitmap;
  else
    m_pBitmap->DeleteObject();
  m_pBitmap->CreateCompatibleBitmap(pDC,clRect.Width(),clRect.Height());
  CBitmap* pOldBitmap = memDC.SelectObject(m_pBitmap);
  memDC.BitBlt(0,0,clRect.Width(),clRect.Height(),pDC,0,0,SRCCOPY);
  memDC.SelectObject(pOldBitmap);

  //////////////////////////////////////////////////////////////////////////////
  
  if (nSaveDC) pDC->RestoreDC(nSaveDC);
  pDC->Detach();

  if (bMustDeleteDC && pDC) delete pDC;
  if (pmdc) delete pmdc;
  if (pdc) delete pdc;
}

void CXGraph::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	Invalidate(TRUE);
}

#if _MFC_VER >= 0x0700
BOOL CXGraph::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt )
#else
void CXGraph::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt )
#endif
{

	if (nFlags & MK_SHIFT)
	{
		for (int y = 0; y < GetYAxisCount(); y++)
		{
			m_YAxis[y].SetAutoScale(false);
			double fStep = (m_YAxis[y].m_fCurMax - m_YAxis[y].m_fCurMin) / 10.0;
			if (zDelta < 0)
      {
        // PERFER : No volem baixar mai de 0.0 [
        if ((m_YAxis[y].m_fCurMin - fStep) < 0.0)
        {
          fStep = m_YAxis[y].m_fCurMin;
        }
        // ]

				m_YAxis[y].SetCurrentRange(m_YAxis[y].m_fCurMin - fStep,m_YAxis[y].m_fCurMax - fStep);
      }
			else
      {
        // PERFER : No volem baixar mai de 0.0 [
        if ((m_YAxis[y].m_fCurMin + fStep) < 0.0)
        {
          fStep = -m_YAxis[y].m_fCurMin;
        }
        // ]

				m_YAxis[y].SetCurrentRange(m_YAxis[y].m_fCurMin + fStep,m_YAxis[y].m_fCurMax + fStep);
      }
		}
	}
	else
	{
		for (int x = 0; x < GetXAxisCount(); x++)
		{
			m_XAxis[x].SetAutoScale(false);
			double fStep = (m_XAxis[x].m_fCurMax - m_XAxis[x].m_fCurMin) / 10.0;
			if (zDelta < 0)
      {
        // PERFER : No volem baixar mai de 0.0 [
        if ((m_XAxis[x].m_fCurMin - fStep) < 0.0)
        {
          fStep = m_XAxis[x].m_fCurMin;
        }
        // ]
        
        m_XAxis[x].SetCurrentRange(m_XAxis[x].m_fCurMin - fStep,m_XAxis[x].m_fCurMax - fStep);
      }
			else
      {
				// PERFER : No volem baixar mai de 0.0 [
        if ((m_XAxis[x].m_fCurMin + fStep) < 0.0)
        {
          fStep = -m_XAxis[x].m_fCurMin;
        }
        // ]
        
        m_XAxis[x].SetCurrentRange(m_XAxis[x].m_fCurMin + fStep,m_XAxis[x].m_fCurMax + fStep);
      }
		}
	}

	Invalidate(TRUE);

#if _MFC_VER >= 0x0700
	return TRUE;
#endif
}


void CXGraph::OnMouseMove(UINT nFlags, CPoint point) 
{
	m_bLButtonDown = (nFlags & MK_LBUTTON);

	if (m_opOperation == opMeasure && m_bLButtonDown)
		AdjustPointToData(point);

	m_CurrentPoint = point;
		
	if (m_opOperation == opCursor)
	{
		m_bTmpDC = true;
		CDCEx dc;
		dc.Attach(GetDC()->m_hDC);
		DrawCursor(&dc);
		ReleaseDC(&dc);
		dc.Detach ();
		m_bTmpDC = false;
		::PostMessage(GetParent()->m_hWnd, XG_CURSORMOVED, 0, (long) this);
	}	

	m_bObjectSelected = CheckObjectSelection(false, m_opOperation != opEditCurve);

	if (m_bLButtonDown && m_opOperation == opPan && m_clInnerRect.PtInRect (point))
	{
		if (m_OldPoint == CPoint(0,0))
			m_OldPoint = point;
		else
			DoPan(point);
	}

	if (m_bRButtonDown && m_opOperation == opCursor && m_clInnerRect.PtInRect (point))
	{
		if (m_OldPoint == CPoint(0,0))
			m_OldPoint = point;
		else
			DoPan(point);
	}

	if (m_bLButtonDown && m_opOperation == opZoom /*&& m_clInnerRect.PtInRect (point)*/)
	{
		if (m_bDoubleBuffer)
			Invalidate();
		else
		{
			CDCEx dc;
			dc.Attach(GetDC()->m_hDC);
			DrawZoom(&dc);
			ReleaseDC(&dc);
			dc.Detach ();
		}
	}
	
	if (m_bLButtonDown && m_opOperation == opMeasure /*&& m_clInnerRect.PtInRect (point)*/)
	{
		if (m_bDoubleBuffer)
			Invalidate();
		else
		{
			CDCEx dc;
			dc.Attach(GetDC()->m_hDC);
			CRect tmpRect;
			tmpRect.SetRectEmpty ();
			DrawMeasure(&dc, tmpRect);
			ReleaseDC(&dc);
			dc.Detach ();
		}
	}
	
	CWnd::OnMouseMove(nFlags, point);
}


void CXGraph::AdjustPointToData(CPoint& point)
{
	int  nYDistance = 10000000;
	int  nY = point.y;
	long nIndex;

	for (int i = 0; i < (int) m_Data.size(); i++)
	{
		// Get xval for current position
		double fSnappedXVal = m_XAxis[m_Data[i].m_nXAxis].GetValueForPos (point.x);
		// Find index for this value
		m_XAxis[m_Data[i].m_nXAxis].GetIndexByXVal(nIndex, fSnappedXVal, i);
		
		int y = m_YAxis[m_Data[i].m_nYAxis].GetPointForValue(&m_Data[i].m_pData[nIndex]).y;
				
		if (abs(point.y - y) < nYDistance)
		{
			nYDistance = abs(point.y - y);
			nY = y;
			if (point == m_MouseDownPoint)
			{
				m_nSnappedCurve = i;
				m_MeasureDef.nIndex1 = nIndex;
				m_MeasureDef.nCurve1 = i;
			}
			else
			{
				m_nSnappedCurve1 = i;
				m_MeasureDef.nIndex2 = nIndex;
				m_MeasureDef.nCurve2 = i;
			}
		}
	}

	point.y = nY;	
}

bool CXGraph::CheckObjectSelection(bool bEditAction, bool bCheckFocusOnly)
{
	UINT i;
	static int nXMarkerMoving = -1;
	static int nYMarkerMoving = -1;

	AFX_MANAGE_STATE(AfxGetAppModuleState());

	bool bHasNotified = false;

	if (!m_bInteraction)
		return false;

		
	// Check y axis selection
	for (i = 0; i < m_YAxis.size(); i++)
	{

		// Check for Marker selections
		if (m_opOperation == opNone)
		for (int nMarker = 0; nMarker < (int) m_YAxis[i].m_AxisMarkers.size(); nMarker++)
		{
			CRect	   hitRect;
			TDataPoint dPoint;


			if (nYMarkerMoving == -1)
			{
			
				dPoint.fYVal = m_YAxis[i].m_AxisMarkers[nMarker].fValue;
				CPoint point = m_YAxis[i].GetPointForValue (&dPoint);

				hitRect.SetRect(m_clInnerRect.left, point.y - 10, m_clInnerRect.right, point.y + 10);
				
				if (bCheckFocusOnly && nYMarkerMoving == -1 && !m_bLButtonDown)
					if (hitRect.PtInRect (m_CurrentPoint))
						return true;
					else
						continue;
			}

			if (m_bRButtonDown && hitRect.PtInRect (m_CurrentPoint))
			{
				m_YAxis[i].DeleteAxisMarker(nMarker);
				Invalidate();
			}
						
			if (m_bLButtonDown && nYMarkerMoving == -1 && hitRect.PtInRect (m_CurrentPoint))
				nYMarkerMoving = nMarker;
			
			if (!m_bLButtonDown && nYMarkerMoving != -1)
				nYMarkerMoving = -1;

			if (nYMarkerMoving != -1 && m_bLButtonDown)
			{
				m_YAxis[i].m_AxisMarkers[nYMarkerMoving].fValue = m_YAxis[i].GetValueForPos (m_CurrentPoint.y);
				Invalidate();
			}

		}
		
		if (bCheckFocusOnly)
			if (m_YAxis[i].m_clRect.PtInRect (m_CurrentPoint))
				return true;
			else
				continue;
	
		if (m_opOperation == opNone)
		if (m_YAxis[i].m_bSelected && !m_YAxis[i].m_clRect.PtInRect (m_CurrentPoint))
		{
			double fValue = m_YAxis[i].GetValueForPos (m_CurrentPoint.y);
			m_YAxis[i].SetAxisMarker(m_YAxis[i].m_AxisMarkers.size(), fValue, 0L);
		}
		
		m_YAxis[i].m_bSelected = m_YAxis[i].m_clRect.PtInRect (m_CurrentPoint);

		
		if (m_YAxis[i].m_bSelected)
		{
			bHasNotified = true;
			::PostMessage(GetParent()->m_hWnd, bEditAction ? XG_YAXISDBLCLICK :XG_YAXISCLICK, i, (long) this);
#ifndef _WIN32_WCE
			if (bEditAction && m_bInteraction)
			{
				CChartPage dlg;
				dlg.m_pGraph = this;
				dlg.m_pGraphAxis = &m_YAxis[i];
				dlg.DoModal();		
				Invalidate();
			}
#endif
		}
	}
			 
	// Check x axis selection
	for (i = 0; i < m_XAxis.size(); i++)
	{

		// Check for Marker selections
		if (m_opOperation == opNone)
		for (int nMarker = 0; nMarker < (int) m_XAxis[i].m_AxisMarkers.size(); nMarker++)
		{
			CRect	   hitRect;
			TDataPoint dPoint;

			if (nXMarkerMoving == -1)
			{
				dPoint.fXVal = m_XAxis[i].m_AxisMarkers[nMarker].fValue;
				CPoint point = m_XAxis[i].GetPointForValue (&dPoint);

				hitRect.SetRect(point.x - 10, m_clInnerRect.top, point.x + 10, m_clInnerRect.bottom);
				
				if (bCheckFocusOnly && nXMarkerMoving == -1 && !m_bLButtonDown)
					if (hitRect.PtInRect (m_CurrentPoint))
						return true;
					else
						continue;
			}

			if (m_bRButtonDown && hitRect.PtInRect (m_CurrentPoint))
			{
				m_XAxis[i].DeleteAxisMarker(nMarker);
				Invalidate();
			}


			if (m_bLButtonDown && nXMarkerMoving == -1 && hitRect.PtInRect (m_CurrentPoint))
				nXMarkerMoving = nMarker;

			if (!m_bLButtonDown && nXMarkerMoving != -1)
				nXMarkerMoving = -1;

			if (nXMarkerMoving != -1 && m_bLButtonDown)
			{
				m_XAxis[i].m_AxisMarkers[nXMarkerMoving].fValue = m_XAxis[i].GetValueForPos (m_CurrentPoint.x);
				Invalidate();
			}

		}
	
		if (bCheckFocusOnly)
			if (m_XAxis[i].m_clRect.PtInRect (m_CurrentPoint))
				return true;
			else
				continue;
		
		if (m_opOperation == opNone)
		if (m_XAxis[i].m_bSelected && !m_XAxis[i].m_clRect.PtInRect (m_CurrentPoint))
		{
			double fValue = m_XAxis[i].GetValueForPos (m_CurrentPoint.x);
			m_XAxis[i].SetAxisMarker(m_XAxis[i].m_AxisMarkers.size(), fValue, 0L);
		}
		
		m_XAxis[i].m_bSelected = m_XAxis[i].m_clRect.PtInRect (m_CurrentPoint);

		if (m_XAxis[i].m_bSelected && !bHasNotified)
		{
			bHasNotified = true;
			::PostMessage(GetParent()->m_hWnd, bEditAction ? XG_XAXISDBLCLICK : XG_XAXISCLICK, i, (long) this);
#ifndef _WIN32_WCE
			if (bEditAction && m_bInteraction)
			{
				CChartPage dlg;
				dlg.m_pGraph = this;
				dlg.m_pGraphAxis = &m_XAxis[i];
				dlg.DoModal();		
				Invalidate();
			}
#endif
		}
	}

	bool bCheckCurves = true;

	// Check object selection
	for (POSITION pos = m_Objects.GetHeadPosition (); pos != NULL; )
	{
		CXGraphObject *pObject = (CXGraphObject*) m_Objects.GetNext (pos);

		if (bCheckFocusOnly)
			if (pObject->m_clRect.PtInRect (m_CurrentPoint) && pObject->m_bVisible)
				return true;
			else
				continue;
		
		pObject->m_bSelected = (pObject->m_clRect.PtInRect (m_CurrentPoint) && pObject->m_bVisible);
		
		if (bEditAction && pObject->m_bSelected )
		{
			bHasNotified = true;
			bCheckCurves = false;
			pObject->Edit ();
		}
		
		if (pObject->m_bSelected && !pObject->m_bEditing && m_pTracker == NULL)
		{
			bHasNotified = true;
			pObject->BeginSize ();
			m_pTracker = &pObject->m_Tracker; 
			bCheckCurves = false;
		}

		if (!pObject->m_bSelected && pObject->m_bEditing)
			pObject->EndEdit ();

		if (!pObject->m_bSelected && pObject->m_bSizing)
		{
			m_pTracker = NULL;
			pObject->EndSize ();
		}
		
	}

	if (!bCheckFocusOnly)
		m_nSelectedSerie = -1;

	
	// Special check for moving datapoint in edit mode
	static int  nEditSerie = -1;
	static long nXIndex = -1;

	// Reset previous stored informations about last selected curve and index
	if (!m_bLButtonDown)
	{
		nXIndex = -1;
		nEditSerie = -1;
	}

	
	// check curve selection
	for (i = 0; i < m_Data.size(); i++)
	{
		CXGraphDataSerie& serie = m_Data[i];
		
		if (!serie.m_bVisible)
			continue;

		if (!bCheckFocusOnly)
			serie.m_bSelected = false;

		if (!bCheckCurves)
			continue;
		

		for (int j = 0; j < (int) serie.m_CurveRegions.size(); j++)
		{
			CRect hitRect(serie.m_CurveRegions[j].p1.x, serie.m_CurveRegions[j].p1.y,serie.m_CurveRegions[j].p2.x, serie.m_CurveRegions[j].p2.y);
			hitRect.NormalizeRect ();
			hitRect.InflateRect (2,2,2,2);
			
			if (bCheckFocusOnly)
				if (hitRect.PtInRect (m_CurrentPoint) && serie.HitTestLine(serie.m_CurveRegions[j].p1, serie.m_CurveRegions[j].p2, m_CurrentPoint, 4))
					return true;
				else
					continue;
			
			if (hitRect.PtInRect (m_CurrentPoint) && serie.HitTestLine(serie.m_CurveRegions[j].p1, serie.m_CurveRegions[j].p2,m_CurrentPoint, 4) && !bHasNotified)
			{
				::PostMessage(GetParent()->m_hWnd, bEditAction ? XG_CURVEDBLCLICK : XG_CURVECLICK, i, (long) this);
				m_nSelectedSerie = i;

				// User is editing a datapoint
				if (nXIndex == -1)
				{
					nEditSerie = i;
					double fX = m_XAxis[serie.GetXAxis ()].GetValueForPos (m_CurrentPoint.x - serie.m_nMarkerSize / 2);
					// Remember index of moving point
					GetXAxis(serie.GetXAxis ()).GetIndexByXVal (nXIndex, fX, i);
				}
				
				serie.m_bSelected = true;
				bHasNotified = true;
#ifndef _WIN32_WCE
				if (bEditAction && m_bInteraction)
				{
					CChartPage dlg;
					dlg.m_pGraph = this;
					dlg.m_pGraphDataSerie = &serie;
					dlg.DoModal();			
					Invalidate();
				}
#endif
				break;
			}
		}
	}

	m_bDataPointMoving = false;

	if (m_bLButtonDown && nXIndex != -1 && m_opOperation == opEditCurve)
	{
		m_nmCurrentPointMoving.nCurve = nEditSerie;
		m_nmCurrentPointMoving.nIndex = nXIndex;
		m_nmCurrentPointMoving.dOldVal = m_Data[nEditSerie].m_pData[nXIndex].fYVal;
		m_nmCurrentPointMoving.dNewVal = GetYAxis(m_Data[nEditSerie].GetYAxis()).GetValueForPos (m_CurrentPoint.y);
		
		// Inform parent
		::PostMessage(GetParent()->m_hWnd, XG_POINTCHANGED, (long)&m_nmCurrentPointMoving, (long) this);

		m_Data[nEditSerie].m_pData[nXIndex].fYVal = GetYAxis(m_Data[nEditSerie].GetYAxis()).GetValueForPos (m_CurrentPoint.y);
		m_fCurrentEditValue = m_Data[nEditSerie].m_pData[nXIndex].fYVal;
		m_bDataPointMoving = true;
		
		Invalidate();
	}

	if (!bHasNotified)
	for (i = 0; i < m_SelectByMarker.size(); i++)
	{
		bool bSelected = m_SelectByMarker[i].markerRect.PtInRect (m_CurrentPoint);
		
		if (bCheckFocusOnly && bSelected)
			return true;
		
		m_SelectByMarker[i].pObject->m_bSelected = bSelected;

		if (bSelected)
		{
			bHasNotified = true;
#ifndef _WIN32_WCE
			if (bEditAction && m_bInteraction)
			{
				CXGraphDataSerie& serie = m_Data[i];
				m_nSelectedSerie = i;
				CChartPage dlg;
				dlg.m_pGraph = this;
				dlg.m_pGraphDataSerie = &serie;
				dlg.DoModal();			
				Invalidate();
			}
#endif
			break;
		}
	}

#ifndef _WIN32_WCE	
	if (!bHasNotified && bEditAction && m_bInteraction)
	{
		::PostMessage(GetParent()->m_hWnd, XG_GRAPHDBLCLICK, 0, (long) this);
		CChartPage dlg;
		dlg.m_pGraph = this;
		dlg.DoModal();
		Invalidate();
	}
#endif

		
	return false;
}

void CXGraph::InsertDataNotation(int nCurve, int nIndex)
{
	CString cText, cXFmt, cYFmt, cFmt;

	bool bXDT = m_XAxis[m_Data[nCurve].m_nXAxis].m_bDateTime;
	bool bYDT = m_YAxis[m_Data[nCurve].m_nYAxis].m_bDateTime;

	cYFmt = cXFmt = "%s";
	cYFmt = "%s";
	cFmt  = "[%d] : " + cXFmt + "%s," + cYFmt + "%s";

	if (bXDT)
#ifndef _WIN32_WCE
			cXFmt = COleDateTime(m_Data[nCurve].m_pData[nIndex].fXVal).Format(m_XAxis[m_Data[nCurve].m_nXAxis].m_cDisplayFmt);
#else
			cXFmt = COleDateTime(m_Data[nCurve].m_pData[nIndex].fXVal).Format();
#endif

	else
		cXFmt.Format(m_XAxis[m_Data[nCurve].m_nXAxis].m_cDisplayFmt, m_Data[nCurve].m_pData[nIndex].fXVal);

	if (bYDT)
#ifndef _WIN32_WCE
		cYFmt = COleDateTime(m_Data[nCurve].m_pData[nIndex].fYVal).Format(m_YAxis[m_Data[nCurve].m_nYAxis].m_cDisplayFmt);
#else
		cYFmt = COleDateTime(m_Data[nCurve].m_pData[nIndex].fYVal).Format();
#endif
	else
		cYFmt.Format(m_YAxis[m_Data[nCurve].m_nYAxis].m_cDisplayFmt, m_Data[nCurve].m_pData[nIndex].fYVal);

	cText.Format(cFmt, nIndex + 1, cXFmt, m_XAxis[m_Data[nCurve].m_nXAxis].m_cLabel, cYFmt, m_YAxis[m_Data[nCurve].m_nYAxis].m_cLabel);
	
	CXGraphDataNotation *pNotation = (CXGraphDataNotation*) new CXGraphDataNotation;
	
	pNotation->m_fXVal  = m_Data[nCurve].m_pData[nIndex].fXVal;
	pNotation->m_fYVal  = m_Data[nCurve].m_pData[nIndex].fYVal;
	pNotation->m_cText  = cText;
	pNotation->m_nCurve = nCurve;
	pNotation->m_nIndex = nIndex;
	pNotation->m_pGraph = this;

	m_Objects.AddTail (pNotation);

	Invalidate();
}

void CXGraph::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (!m_bLButtonDown)
		return;

	ReleaseCapture();

	m_OldPoint     = CPoint(0,0);
	m_MouseUpPoint = point;
	m_bLButtonDown = false;

	if (m_opOperation == opMeasure)
	{
		AdjustPointToData(point);
		m_Measures.push_back(m_MeasureDef);
	  DoMeasure();
	}

	if (m_opOperation == opCursor)
	{
		if (!CheckObjectSelection() && m_nSnappedCurve != -1)
		{
			long nIndex;
			m_XAxis[m_Data[m_nSnappedCurve].m_nXAxis].GetIndexByXVal(nIndex, m_fSnappedXVal, m_nSnappedCurve);
			InsertDataNotation(m_nSnappedCurve, nIndex);
		}
	}
	
	if (m_opOperation == opZoom)
		DoZoom();
	
	if (m_opOperation == opNone || m_opOperation == opEditCurve)
		CheckObjectSelection();

	Invalidate(TRUE);

	CWnd::OnLButtonUp(nFlags, point);
}

void CXGraph::OnLButtonDown(UINT nFlags, CPoint point) 
{

	SetFocus();
	SetCapture();

	m_MouseDownPoint = point;

	if (m_opOperation == opMeasure)
		AdjustPointToData(point);

	m_MouseDownPoint = point;

	
	m_bLButtonDown   = true;
	m_pCurrentObject = NULL;

			
	// Check objects
	for (POSITION pos = m_Objects.GetHeadPosition (); pos != NULL; )
	{
		CXGraphObject *pObject = (CXGraphObject*) m_Objects.GetNext (pos);

		if (pObject->m_clRect.PtInRect(point))
			m_pCurrentObject = pObject;
				
		if (pObject->m_bSizing && pObject->m_Tracker.HitTest(point) != CRectTracker::hitNothing)
		{
			ReleaseCapture();
			
			if (pObject->m_Tracker.Track(this, point) && pObject->m_bCanMove && pObject->m_bCanResize)
			{
				pObject->m_clRect = pObject->m_Tracker.m_rect;
				Invalidate(FALSE);
			}
		}
	}

	CWnd::OnLButtonDown(nFlags, point);
}

BOOL CXGraph::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

void CXGraph::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if (m_opOperation == opNone)
	{
		m_pTracker = NULL;
		CheckObjectSelection(true);
	}
	
	CWnd::OnLButtonDblClk(nFlags, point);
}

BOOL CXGraph::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{	
	if (m_pTracker && m_pTracker->SetCursor(this,  nHitTest))
        return TRUE;

	if (m_opOperation == opPan)
	{
		HCURSOR hCur = AfxGetApp()->LoadStandardCursor(IDC_SIZEALL);
		if (hCur)
		{
			::SetCursor(hCur);			 
			return TRUE;
		}
	}
	else
	if (m_opOperation == opZoom || m_opOperation == opCursor)
	{
		HCURSOR hCur = AfxGetApp()->LoadStandardCursor(IDC_CROSS);
		if (hCur)
		{
			::SetCursor(hCur);			 
			return TRUE;
		}
	}
	else
	if (m_bObjectSelected && m_opOperation == opNone)
	{
		HCURSOR hCur = AfxGetApp()->LoadStandardCursor(MAKEINTRESOURCE(32649));
		if (hCur)
		{
			::SetCursor(hCur);			 
			return TRUE;
		}
	}
	else
	if (m_opOperation == opEditCurve && m_nSelectedSerie != -1)
	{
		HCURSOR hCur = AfxGetApp()->LoadStandardCursor(IDC_SIZENS);
		if (hCur)
		{
			::SetCursor(hCur);			 
			return TRUE;
		}
	}

	
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

BOOL CXGraph::PreTranslateMessage(MSG* pMsg) 
{
	if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP) && pMsg->wParam == VK_DELETE)
	{
		if (m_pCurrentObject && m_pCurrentObject->m_bSizing)
		{
			if (m_Objects.GetHead() != m_pCurrentObject)
			{
				m_Objects.RemoveAt(m_Objects.Find(m_pCurrentObject));
				delete m_pCurrentObject;
				m_pCurrentObject = NULL;
				m_pTracker = NULL;
				Invalidate();
			}
		}
	}
		
	return CWnd::PreTranslateMessage(pMsg);
}

#ifndef _WIN32_WCE
void CXGraph::OnRButtonUp(UINT nFlags, CPoint point) 
{
	CheckObjectSelection();

	m_bRButtonDown = false;

	if (m_opOperation == opCursor)
	{
		m_oldCursorPoint = CPoint(-1,-1);
		Invalidate();
	}

	if (m_pCurrentObject && m_pCurrentObject->m_clRect.PtInRect (point))
		m_pCurrentObject->InvokeProperties();
	else
		::PostMessage(GetParent()->m_hWnd, XG_RBUTTONUP, 0, (long) this);
		

	CWnd::OnRButtonUp(nFlags, point);
}
#endif

void CXGraph::InsertEmptyLabel()
{
	InsertLabel();
	Invalidate();
}

void CXGraph::OnProperties()
{
	AFX_MANAGE_STATE(AfxGetModuleState( ));
	CheckObjectSelection(true);
}


#ifndef _WIN32_WCE
void CXGraph::OnPrint()
{
	AFX_MANAGE_STATE(AfxGetModuleState());

	CPrintDialog dlg(FALSE);

	if (dlg.DoModal () == IDOK)
	{
		CDCEx dc;
		
		dc.Attach (dlg.GetPrinterDC ());

		dc.Prepare (dc.m_hAttribDC);

		int nWidth  = dc.GetDeviceCaps (HORZRES);
		int nHeight = dc.GetDeviceCaps (VERTRES);

		m_clPrintRect.SetRect(0, 0, nWidth / dc.m_fScaleX, nHeight / dc.m_fScaleY);

		m_pPrintDC = &dc;
		m_pPrintDC->m_bPrinting = true;
		m_pPrintDC->StartDoc ("Graph"); 
		m_pPrintDC->StartPage ();
		OnPaint();
		m_pPrintDC->EndPage ();
		m_pPrintDC->EndDoc ();
		m_pPrintDC = NULL;
		dc.Detach ();
	}
	
	//Invalidate();
}

void CXGraph::PrintGraph(CDC *pDC)
{ 
	CDCEx dc;
	
	dc.Attach (pDC->m_hDC);

	dc.Prepare (pDC->m_hAttribDC);
	
	int nWidth, nHeight;

	if (pDC->m_hDC == pDC->m_hAttribDC)
	{
		m_pPrintDC = &dc;
		nWidth  = pDC->GetDeviceCaps (HORZRES);
		nHeight = pDC->GetDeviceCaps (VERTRES);
		dc.m_bPrintPreview = false;
		dc.m_bPrinting = true;
	    DOCINFO docinfo;
	    memset(&docinfo, 0, sizeof(docinfo));
	    docinfo.cbSize = sizeof(docinfo);
        docinfo.lpszDocName = _T("Graph");
		int nRet;//= dc.StartDoc(&docinfo);
		nRet = GetLastError();
		nRet = dc.StartPage();
		m_clPrintRect.SetRect(0, 0, nWidth / dc.m_fScaleX, nHeight / dc.m_fScaleY);
		OnPaint();
		dc.EndPage ();
		dc.EndDoc ();
		m_pPrintDC = NULL;

	}
	else
	{
		nWidth  = ::GetDeviceCaps (pDC->m_hAttribDC ,HORZRES);
		nHeight = ::GetDeviceCaps (pDC->m_hAttribDC ,VERTRES);
		dc.m_bPrintPreview = true;
		m_clPrintRect.SetRect(0, 0, nWidth / dc.m_fScaleX, nHeight / dc.m_fScaleY);
		m_pPrintDC = &dc;
		m_pPrintDC->m_bPrinting = true;
		OnPaint();
		m_pPrintDC = NULL;
	}
}

void CXGraph::PrintGraph(CDC *pDC, CRect printRect)
{ 
	CDCEx dc;
	
	dc.Attach (pDC->m_hDC);

	dc.Prepare (pDC->m_hAttribDC);

	int nWidth, nHeight;

	nWidth  = pDC->GetDeviceCaps (HORZRES);
	nHeight = pDC->GetDeviceCaps (VERTRES);
		
	if (pDC->m_hDC == pDC->m_hAttribDC)
	{
		m_pPrintDC = &dc;
		dc.m_bPrintPreview = false;
		dc.m_bPrinting = true;
	    DOCINFO docinfo;
	    memset(&docinfo, 0, sizeof(docinfo));
	    docinfo.cbSize = sizeof(docinfo);
        docinfo.lpszDocName = _T("Graph");
		int nRet;//= dc.StartDoc(&docinfo);
		nRet = GetLastError();
		nRet = dc.StartPage();
		m_clPrintRect.SetRect(printRect.left / dc.m_fScaleX, printRect.top / dc.m_fScaleX, printRect.right / dc.m_fScaleX, printRect.bottom / dc.m_fScaleX);
		OnPaint();
		dc.EndPage ();
		dc.EndDoc ();
		m_pPrintDC = NULL;

	}
	else
	{
		dc.m_bPrintPreview = true;
		m_clPrintRect.SetRect(printRect.left / dc.m_fScaleX, printRect.top / dc.m_fScaleX, printRect.right / dc.m_fScaleX, printRect.bottom / dc.m_fScaleX);
		m_pPrintDC = &dc;
		m_pPrintDC->m_bPrinting = true;
		OnPaint();
		m_pPrintDC = NULL;
	}

}
#endif

void CXGraph::CubicTrend()
{
	AddCubicTrend(m_Data[m_nSelectedSerie]);
}

void CXGraph::LinearTrend()
{
	AddLinearTrend(m_Data[m_nSelectedSerie]);
}

void CXGraph::AddLinearTrend(CXGraphDataSerie& serie)
{
	TDataPoint* pTrend = serie.GetLinearTrend ();
	int			nCurveCount = GetCurveCount ();
	CString cLabel = serie.m_cLabel;
	CXGraphDataSerie &ret = SetData (pTrend, serie.m_nCount, nCurveCount, serie.m_nXAxis, serie.m_nYAxis, true);
	ret.m_cLabel = cLabel + " linear trend";
	delete pTrend;
}

void CXGraph::AddCubicTrend(CXGraphDataSerie& serie)
{
	TDataPoint* pTrend = serie.GetCubicTrend ();
	int			nCurveCount = GetCurveCount ();
	CString cLabel = serie.m_cLabel;
	CXGraphDataSerie &ret = SetData (pTrend, serie.m_nCount, nCurveCount, serie.m_nXAxis, serie.m_nYAxis, true);
	ret.m_cLabel = cLabel + " cubic trend";
	delete pTrend;
}

void CXGraph::DrawCursor(CDCEx* pDC)
{
	CPenSelector ps(0L, 1, pDC);
	long		 nIndex;
	TDataPoint   point;
	int			 nOldROP2 = pDC->SetROP2 (R2_NOTXORPEN);

	m_nSnappedCurve = -1;

	if (m_nForcedSnapCurve != -1)
	{
		m_nSnappedCurve = m_nForcedSnapCurve;
		m_fSnappedXVal = m_XAxis[m_Data[m_nForcedSnapCurve].m_nXAxis].GetValueForPos (m_CurrentPoint.x);
		// Find index for this value
		m_XAxis[m_Data[m_nForcedSnapCurve].m_nXAxis].GetIndexByXVal(nIndex, m_fSnappedXVal, m_nForcedSnapCurve);
		// get yval for index
		m_fSnappedYVal = m_Data[m_nForcedSnapCurve].m_pData[nIndex].fYVal;
		point.fYVal = m_fSnappedYVal;
		m_CurrentPoint.y = m_YAxis[m_Data[m_nSnappedCurve].m_nYAxis].GetPointForValue(&point).y;
	}
	else
	if (m_bSnapCursor)
	// Snap cursor to nearest curve
	for (int i = 0; i < (int) m_Data.size(); i++)
	{
		// Get xval for current position
		m_fSnappedXVal = m_XAxis[m_Data[i].m_nXAxis].GetValueForPos (m_CurrentPoint.x);
		// Find index for this value
		m_XAxis[m_Data[i].m_nXAxis].GetIndexByXVal(nIndex, m_fSnappedXVal, i);
		// get yval for index
		m_fSnappedYVal = m_Data[i].m_pData[nIndex].fYVal;
		point.fYVal = m_fSnappedYVal;
		// Check if cursor is in snap-range
		int y = m_YAxis[m_Data[i].m_nYAxis].GetPointForValue(&point).y;
		if (abs(m_CurrentPoint.y - y) < m_nSnapRange)
		{
			m_nSnappedCurve = i;
			m_CurrentPoint.y = y;
			break;
		}
	}
		
	if (m_oldCursorPoint != CPoint(-1,-1))
	{
		if (m_nCursorFlags & XGC_VERT)
		{
			pDC->MoveTo (m_oldCursorPoint.x, m_clInnerRect.top);
			pDC->LineTo (m_oldCursorPoint.x, m_clInnerRect.bottom);
		}

		if (m_nCursorFlags & XGC_HORIZ)
		{
			pDC->MoveTo (m_clInnerRect.left, m_oldCursorPoint.y);
			pDC->LineTo (m_clInnerRect.right, m_oldCursorPoint.y);
		}

		pDC->Rectangle(m_oldCursorPoint.x - (m_nSnapRange / 2), m_oldCursorPoint.y - (m_nSnapRange / 2), m_oldCursorPoint.x + (m_nSnapRange / 2) + 1, m_oldCursorPoint.y + (m_nSnapRange / 2) + 1);
	}

	if (m_nCursorFlags & XGC_VERT)
	{
		pDC->MoveTo (m_CurrentPoint.x, m_clInnerRect.top);
		pDC->LineTo (m_CurrentPoint.x, m_clInnerRect.bottom);
	}

	if (m_nCursorFlags & XGC_HORIZ)
	{
		pDC->MoveTo (m_clInnerRect.left, m_CurrentPoint.y);
		pDC->LineTo (m_clInnerRect.right, m_CurrentPoint.y);
	}
	
	pDC->Rectangle(m_CurrentPoint.x - (m_nSnapRange / 2), m_CurrentPoint.y - (m_nSnapRange / 2), m_CurrentPoint.x + (m_nSnapRange / 2) + 1, m_CurrentPoint.y + (m_nSnapRange / 2) + 1);

	m_oldCursorPoint = m_CurrentPoint;

	pDC->SetROP2 (nOldROP2);

	if (m_nCursorFlags & XGC_LEGEND)
		DrawCursorLegend(pDC);
}

void CXGraph::DrawCursorLegend (CDCEx* pDC)
{
	int    i;
    double fVal;

    CString cLabel = "", cFmt, cItem;

    m_CursorLabel.m_bVisible = true;
	   
    for (i = 0; i < (int) m_XAxis.size(); i++)
    {
        //CHANGED: bugfix, correct handling of X axis in time mode
        fVal = m_XAxis[i].GetValueForPos (m_CurrentPoint.x);
        if(m_XAxis[i].m_bDateTime)
        {
			cFmt = m_XAxis[i].m_cDisplayFmt;
			cFmt.Replace (_T("\r"),_T(" "));
			cFmt.Replace (_T("\n"),_T(" "));
#ifndef _WIN32_WCE
            cFmt = _T("X[%02d] : ") + COleDateTime(fVal).Format(cFmt) + _T(" %s");
#else
			cFmt = _T("X[%02d] : ") + COleDateTime(fVal).Format() + _T(" %s");
#endif
            cItem.Format(cFmt, i+1, m_XAxis[i].m_cLabel);
            cLabel += (_T(" ") + cItem + _T("\r\n"));
        }
        else
        {
            cFmt = _T("X[%02d] : ") + m_XAxis[i].m_cDisplayFmt + _T(" %s");
            cItem.Format(cFmt, i+1, fVal, m_XAxis[i].m_cLabel);
            cLabel += (_T(" ") + cItem + _T("\r\n"));
        }
        //ENDCHANGES
    }
    
    for (i = 0; i < (int) m_YAxis.size(); i++)
    {
        fVal = m_YAxis[i].GetValueForPos (m_CurrentPoint.y);
        cFmt = _T("Y[%02d] : ") + m_YAxis[i].m_cDisplayFmt + _T(" %s");
        cItem.Format(cFmt, i+1, fVal, m_YAxis[i].m_cLabel);
        cLabel += (_T(" ") + cItem + _T("\r\n"));
    }

    if (m_nSnappedCurve != -1)
    {
        //CHANGED: bugfix, correct handling of X axis in time mode
        CString cXFmt = m_XAxis[m_Data[m_nSnappedCurve].m_nXAxis].m_cDisplayFmt;
		cXFmt.Replace (_T("\r"),_T(" "));
		cXFmt.Replace (_T("\n"),_T(" "));
        CString cYFmt = m_YAxis[m_Data[m_nSnappedCurve].m_nYAxis].m_cDisplayFmt;
        if(m_XAxis[m_Data[m_nSnappedCurve].m_nXAxis].m_bDateTime)
        {			
#ifndef _WIN32_WCE
            cFmt = _T("%s[%02d] : ") + COleDateTime(m_fSnappedXVal).Format(cXFmt) + _T(", ") + cYFmt;
#else
			cFmt = _T("%s[%02d] : ") + COleDateTime(m_fSnappedXVal).Format() + _T(", ") + cYFmt;
#endif
            cItem.Format(cFmt, m_Data[m_nSnappedCurve].m_cLabel, m_nSnappedCurve + 1, m_fSnappedYVal);
            cLabel += (_T(" ") + cItem + _T("\r\n"));
        }
        else
        {
            cFmt = _T("%s[%02d] : ") + cXFmt + _T(", ") + cYFmt;
            cItem.Format(cFmt, m_Data[m_nSnappedCurve].m_cLabel, m_nSnappedCurve + 1, m_fSnappedXVal, m_fSnappedYVal);
            cLabel += (_T(" ") + cItem + _T("\r\n"));
        }
        //ENDCHANGES
    }

    m_CursorLabel.m_cText = cLabel;

    m_CursorLabel.Draw (pDC);
}

LRESULT CXGraph::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	return CWnd::WindowProc(message, wParam, lParam);
}

bool CXGraph::SelectCurve(int nCurve)
{
	if (nCurve < 0 || nCurve >= (int) m_Data.size())
		return false;

	for (int i = 0; i < (int) m_Data.size(); i++)
		m_Data[i].m_bSelected = (i == nCurve);

	Invalidate();

	return true;
}

bool CXGraph::SelectXAxis(int nAxis)
{
	if (nAxis < 0 || nAxis >= (int) m_XAxis.size())
		return false;

	for (int i = 0; i < (int) m_XAxis.size(); i++)
		m_XAxis[i].m_bSelected = (i == nAxis);

	Invalidate();

	return true;
}

bool CXGraph::SelectYAxis(int nAxis)
{
	if (nAxis < 0 || nAxis >= (int) m_YAxis.size())
		return false;

	for (int i = 0; i < (int) m_YAxis.size(); i++)
		m_YAxis[i].m_bSelected = (i == nAxis);

	Invalidate();
	
	return true;
}

void CXGraph::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_HOME)
	{
		for (int i = 0; i < (int) m_XAxis.size(); i++)
			m_XAxis[i].Reset ();
		
		for (int i = 0; i < (int) m_YAxis.size(); i++)
			m_YAxis[i].Reset();
		
		Invalidate();
	}

	if (nChar == VK_ADD)
	{
		for (int i = 0; i < (int) m_XAxis.size(); i++)
		{
			double fStep = m_XAxis[i].m_fRange / 10.0;
			VERIFY(m_XAxis[i].SetCurrentRange (m_XAxis[i].m_fCurMin + fStep, m_XAxis[i].m_fCurMax - fStep));
		}
		for (int i = 0; i < (int) m_YAxis.size(); i++)
		{
			double fStep = m_YAxis[i].m_fRange / 10.0;
			VERIFY(m_YAxis[i].SetCurrentRange (m_YAxis[i].m_fCurMin + fStep, m_YAxis[i].m_fCurMax - fStep));
		}
		
		Invalidate();
	}

	if (nChar == VK_SUBTRACT)
	{
		for (int i = 0; i < (int) m_XAxis.size(); i++)
		{
			double fStep = m_XAxis[i].m_fRange / 10.0;
			VERIFY(m_XAxis[i].SetCurrentRange (m_XAxis[i].m_fCurMin - fStep, m_XAxis[i].m_fCurMax + fStep));
		}
		for (int i = 0; i < (int) m_YAxis.size(); i++)
		{
			double fStep = m_YAxis[i].m_fRange / 10.0;
			VERIFY(m_YAxis[i].SetCurrentRange (m_YAxis[i].m_fCurMin - fStep, m_YAxis[i].m_fCurMax + fStep));
		}
		
		Invalidate();
	}

	if (nChar == VK_LEFT)
	{
		if (m_opOperation == opCursor)
			m_CurrentPoint.x --;
		else
		for (int i = 0; i < (int) m_XAxis.size(); i++)
		{
			double fStep = m_XAxis[i].m_fRange / 10.0;
			VERIFY(m_XAxis[i].SetCurrentRange (m_XAxis[i].m_fCurMin + fStep, m_XAxis[i].m_fCurMax + fStep));
		}

		Invalidate();
	}
	
	if (nChar == VK_RIGHT)
	{
		if (m_opOperation == opCursor)
			m_CurrentPoint.x++;
		else
		for (int i = 0; i < (int) m_XAxis.size(); i++)
		{
			double fStep = m_XAxis[i].m_fRange / 10.0;
			VERIFY(m_XAxis[i].SetCurrentRange (m_XAxis[i].m_fCurMin - fStep, m_XAxis[i].m_fCurMax - fStep));
		}

		Invalidate();
	}

	if (nChar == VK_UP)
	{
		if (m_opOperation == opCursor)
			m_CurrentPoint.y--;
		else
		for (int i = 0; i < (int) m_YAxis.size(); i++)
		{
			double fStep = m_YAxis[i].m_fRange / 10.0;
			VERIFY(m_YAxis[i].SetCurrentRange (m_YAxis[i].m_fCurMin - fStep, m_YAxis[i].m_fCurMax - fStep));
		}

		Invalidate();
	}

	if (nChar == VK_DOWN)
	{
		if (m_opOperation == opCursor)
			m_CurrentPoint.y++;
		else
		for (int i = 0; i < (int) m_YAxis.size(); i++)
		{
			double fStep = m_YAxis[i].m_fRange / 10.0;
			VERIFY(m_YAxis[i].SetCurrentRange (m_YAxis[i].m_fCurMin + fStep, m_YAxis[i].m_fCurMax + fStep));
		}

		Invalidate();
	}
	
	CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CXGraph::ResetAll()
{
	m_XAxis.clear();
	m_YAxis.clear();
	m_Data.clear();
  m_Measures.clear();

	// Delete all objects
	// Spare 1st object (CursorLabel)
	while (m_Objects.GetCount() > 1)
  {
		delete m_Objects.RemoveTail();
  }

  if (m_pBitmap)
  {
    delete m_pBitmap;
    m_pBitmap = NULL;
  }
}

#ifndef _WIN32_WCE
void CXGraph::OnRButtonDown(UINT nFlags, CPoint point) 
{
	m_bRButtonDown = true;
	
	CWnd::OnRButtonDown(nFlags, point);
}
#endif

void CXGraph::Serialize( CArchive& archive, UINT nFlags)
{	
	int nHelper;
	
	CWnd::Serialize (archive);

    if( archive.IsStoring() )
    {
		archive << nFlags;
		
		if (nFlags & PERSIST_PROPERTIES)
		{
			archive << m_crBackColor;
			archive << m_crGraphColor;
			archive << m_crInnerColor;
			archive << m_bDoubleBuffer;
			archive << m_bShowLegend;
			archive << (int) m_LegendAlignment;
			archive << m_bSnapCursor;
			archive << m_nSnapRange;
			archive << m_nLeftMargin;
			archive << m_nTopMargin;
			archive << m_nRightMargin;
			archive << m_nBottomMargin;
			archive << m_nAxisSpace;
			archive << m_nSelectedSerie;
		}

		if (nFlags & PERSIST_DATA)
		{
			archive << m_Data.size ();
			for (int i = 0; i < (int) m_Data.size(); i++)
				m_Data[i].Serialize (archive);

			archive << m_XAxis.size ();
			for (int i = 0; i < (int) m_XAxis.size(); i++)
				m_XAxis[i].Serialize (archive);

			archive << m_YAxis.size ();
			for (int i = 0; i < (int) m_YAxis.size(); i++)
				m_YAxis[i].Serialize (archive);
		}

		if (nFlags & PERSIST_OBJECTS)
		{

			archive << m_Measures.size();
			for (int i = 0; i < (int) m_Measures.size(); i++)
				archive.Write(&m_Measures[i], sizeof(MeasureDef));
		
			// Spare cursor label (1st object), created within constructor
			archive << m_Objects.GetCount () - 1;

			int i = 0;

			for (POSITION pos = m_Objects.GetHeadPosition (); pos != NULL; i++ )
			{			
				CXGraphObject *pObject = (CXGraphObject*) m_Objects.GetNext (pos);
				if ( i > 0)
					archive << pObject;
		
			}
		}
	}
	else
    {
		archive >> nFlags;

		if (nFlags & PERSIST_PROPERTIES)
		{
			archive >> m_crBackColor;
			archive >> m_crGraphColor;
			archive >> m_crInnerColor;
			archive >> m_bDoubleBuffer;
			archive >> m_bShowLegend;
			archive >> nHelper;
			m_LegendAlignment = (EAlignment) nHelper;
			archive >> m_bSnapCursor;
			archive >> m_nSnapRange;
			archive >> m_nLeftMargin;
			archive >> m_nTopMargin;
			archive >> m_nRightMargin;
			archive >> m_nBottomMargin;
			archive >> m_nAxisSpace;
			archive >> m_nSelectedSerie;
		}

		if (nFlags & PERSIST_DATA)
		{
			archive >> nHelper;
			for (int i = 0; i < nHelper; i++)
			{
				CXGraphDataSerie serie;
				serie.Serialize (archive);
				serie.m_pGraph = this;
				m_Data.push_back (serie);
			}

			archive >> nHelper;
			for (int i = 0; i < nHelper; i++)
			{
				CXGraphAxis axis;
				axis.Serialize (archive);
				axis.m_pGraph = this;
				m_XAxis.push_back (axis);
			}

			archive >> nHelper;
			for (int i = 0; i < nHelper; i++)
			{
				CXGraphAxis axis;
				axis.Serialize (archive);
				axis.m_pGraph = this;
				m_YAxis.push_back (axis);
			}
		}

		if (nFlags & PERSIST_OBJECTS)
		{
			archive >> nHelper;

			for (int i = 0; i < nHelper; i++)
			{
				MeasureDef measure;
				archive.Read(&measure, sizeof(MeasureDef));
				m_Measures.push_back (measure);
			}
			
			archive >> nHelper;

			for (int i = 0; i < nHelper; i++)
			{			
				CXGraphObject *pObject;
				archive >> pObject;
				pObject->m_pGraph = this;
				m_Objects.AddTail (pObject);
			}
		}

		Invalidate();
	}
}

bool CXGraph::Save(const CString cFile, UINT nFlags)
{
	CFile file;

	if (nFlags == 0)
		return false;

	m_pTracker = NULL;
	m_pCurrentObject = NULL;

	if (!file.Open (cFile, CFile::modeCreate | CFile::modeWrite ))
		return false;

#ifndef _WIN32_WCE
	try	
	{
#endif
		CArchive ar(&file, CArchive::store);
		Serialize(ar, nFlags);
#ifndef _WIN32_WCE
	} 
	catch (CException* e)	
	{
		e->Delete ();
		return false;
	}
#endif

	return true;
}

bool CXGraph::Load(const CString cFile)
{
	CFile file;
	
	if (!file.Open (cFile, CFile::modeRead ))
		return false;

	ResetAll();
#ifndef _WIN32_WCE
	try
#endif
	{

		CArchive ar(&file, CArchive::load);
		Serialize(ar, 0);
	} 
#ifndef _WIN32_WCE
	catch (CException* e)
	{
		e->Delete ();
		return false;
	}
#endif
	return true;
}

void CXGraph::ParentCallback()
{
	::PostMessage(GetParent()->m_hWnd, IDM_PARENTCALLBACK, 0, (long) this);		
}

#ifndef _WIN32_WCE
HANDLE CXGraph::DDBToDIB( CBitmap& bitmap, DWORD dwCompression, CPalette* pPal ) 
{
	BITMAP			bm;
	BITMAPINFOHEADER	bi;
	LPBITMAPINFOHEADER 	lpbi;
	DWORD			dwLen;
	HANDLE			hDIB;
	HANDLE			handle;
	HDC 			hDC;
	HPALETTE		hPal;


	ASSERT( bitmap.GetSafeHandle() );

	// The function has no arg for bitfields
	if( dwCompression == BI_BITFIELDS )
		return NULL;

	// If a palette has not been supplied use defaul palette
	hPal = (HPALETTE) pPal->GetSafeHandle();
	if (hPal==NULL)
		hPal = (HPALETTE) GetStockObject(DEFAULT_PALETTE);

	// Get bitmap information
	bitmap.GetObject(sizeof(bm),(LPSTR)&bm);

	// Initialize the bitmapinfoheader
	bi.biSize		= sizeof(BITMAPINFOHEADER);
	bi.biWidth		= bm.bmWidth;
	bi.biHeight 		= bm.bmHeight;
	bi.biPlanes 		= 1;
	bi.biBitCount		= bm.bmPlanes * bm.bmBitsPixel;
	bi.biCompression	= dwCompression;
	bi.biSizeImage		= 0;
	bi.biXPelsPerMeter	= 0;
	bi.biYPelsPerMeter	= 0;
	bi.biClrUsed		= 0;
	bi.biClrImportant	= 0;

	// Compute the size of the  infoheader and the color table
	int nColors = (1 << bi.biBitCount);
	if( nColors > 256 ) 
		nColors = 0;
	dwLen  = bi.biSize + nColors * sizeof(RGBQUAD);

	// We need a device context to get the DIB from
	hDC = ::GetDC(NULL);
	hPal = SelectPalette(hDC,hPal,FALSE);
	RealizePalette(hDC);

	// Allocate enough memory to hold bitmapinfoheader and color table
	hDIB = GlobalAlloc(GMEM_FIXED,dwLen);

	if (!hDIB){
		SelectPalette(hDC,hPal,FALSE);
		::ReleaseDC(NULL,hDC);
		return NULL;
	}

	lpbi = (LPBITMAPINFOHEADER)hDIB;

	*lpbi = bi;

	// Call GetDIBits with a NULL lpBits param, so the device driver 
	// will calculate the biSizeImage field 
	GetDIBits(hDC, (HBITMAP)bitmap.GetSafeHandle(), 0L, (DWORD)bi.biHeight,
			(LPBYTE)NULL, (LPBITMAPINFO)lpbi, (DWORD)DIB_RGB_COLORS);

	bi = *lpbi;

	// If the driver did not fill in the biSizeImage field, then compute it
	// Each scan line of the image is aligned on a DWORD (32bit) boundary
	if (bi.biSizeImage == 0){
		bi.biSizeImage = ((((bi.biWidth * bi.biBitCount) + 31) & ~31) / 8) 
						* bi.biHeight;

		// If a compression scheme is used the result may infact be larger
		// Increase the size to account for this.
		if (dwCompression != BI_RGB)
			bi.biSizeImage = (bi.biSizeImage * 3) / 2;
	}

	// Realloc the buffer so that it can hold all the bits
	dwLen += bi.biSizeImage;
	if (handle = GlobalReAlloc(hDIB, dwLen, GMEM_MOVEABLE))
		hDIB = handle;
	else{
		GlobalFree(hDIB);

		// Reselect the original palette
		SelectPalette(hDC,hPal,FALSE);
		::ReleaseDC(NULL,hDC);
		return NULL;
	}

	// Get the bitmap bits
	lpbi = (LPBITMAPINFOHEADER)hDIB;

	// FINALLY get the DIB
	BOOL bGotBits = GetDIBits( hDC, (HBITMAP)bitmap.GetSafeHandle(),
				0L,				// Start scan line
				(DWORD)bi.biHeight,		// # of scan lines
				(LPBYTE)lpbi 			// address for bitmap bits
				+ (bi.biSize + nColors * sizeof(RGBQUAD)),
				(LPBITMAPINFO)lpbi,		// address of bitmapinfo
				(DWORD)DIB_RGB_COLORS);		// Use RGB for color table

	if( !bGotBits )
	{
		GlobalFree(hDIB);
		
		SelectPalette(hDC,hPal,FALSE);
		::ReleaseDC(NULL,hDC);
		return NULL;
	}

	SelectPalette(hDC,hPal,FALSE);
	::ReleaseDC(NULL,hDC);
	return hDIB;
}


BOOL CXGraph::WriteDIB( LPTSTR szFile, HANDLE hDIB)
{
	BITMAPFILEHEADER	hdr;
	LPBITMAPINFOHEADER	lpbi;

	if (!hDIB)
		return FALSE;

	CFile file;
	if( !file.Open( szFile, CFile::modeWrite|CFile::modeCreate) )
		return FALSE;

	lpbi = (LPBITMAPINFOHEADER)hDIB;

	int nColors = 1 << lpbi->biBitCount;

	// Fill in the fields of the file header 
	hdr.bfType		= ((WORD) ('M' << 8) | 'B');	// is always "BM"
	hdr.bfSize		= GlobalSize (hDIB) + sizeof( hdr );
	hdr.bfReserved1 	= 0;
	hdr.bfReserved2 	= 0;
	hdr.bfOffBits		= (DWORD) (sizeof( hdr ) + lpbi->biSize +
						nColors * sizeof(RGBQUAD));

	// Write the file header 
	file.Write( &hdr, sizeof(hdr) );

	// Write the DIB header and the bits 
	file.Write( lpbi, GlobalSize(hDIB) );

	return TRUE;
}

BOOL CXGraph::SaveBitmap( const CString cFile )
{
  // PERFER : small hack to take screenshots correctly [
  m_disableClipping = true;
  Invalidate();
  m_disableClipping = false;
  // ]
  
  CWnd* pWnd = this;
  CWindowDC dc(pWnd);

  // Create logical palette if device support a palette
  CPalette pal;
  if (dc.GetDeviceCaps(RASTERCAPS) & RC_PALETTE)
  {
    UINT nSize = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * 256);
    LOGPALETTE *pLP = (LOGPALETTE*) new BYTE[nSize];
    pLP->palVersion = 0x300;

    pLP->palNumEntries = GetSystemPaletteEntries(dc,0,255,pLP->palPalEntry);

    // Create the palette
    pal.CreatePalette(pLP);

    delete[] pLP;
  }

  // Convert the bitmap to a DIB
  HANDLE hDIB = DDBToDIB(*m_pBitmap, BI_RGB, &pal);

  if (hDIB == NULL)
    return FALSE;

  // Write it to file
  WriteDIB (const_cast<char*>((LPCTSTR)cFile), hDIB);

  // Free the memory allocated by DDBToDIB for the DIB
  GlobalFree(hDIB);
  return TRUE;
}
#endif

#pragma warning (default : 4244)
#pragma warning (default : 4800)


