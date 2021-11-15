// XGraphDataSerie.cpp: Implementierung der Klasse CXGraphDataSerie.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XGraphDataSerie.h"
#include "XGraph.h"
#include "GfxUtils.h"
#include "math.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL( CXGraphDataSerie, CXGraphObject, 1 )

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CXGraphDataSerie::CXGraphDataSerie()
{
	m_bAutoDelete = false;
	m_bVisible = true;
	m_bShowMarker = false;
	m_bFillBeneath = false;
	m_bFillTransparent = true;
	m_pGraph   = NULL;
	m_pData    = NULL;
	m_nLineStyle = PS_SOLID;
	m_nFillStyle = 6;
	m_nLineSize = 1;
	m_gtType = gtLine;
	m_nFirstVisible = 0;
	m_nLastVisible  = 0;
	m_cLabel = _T(" ");
	m_nMarkerType = 0;
	m_nMarker = 0;
	m_nMarkerSize = 10;
	m_nXAxis  = 0;
	m_nYAxis  = 0;
	m_crColor = 0L;
	m_nCount  = 0; 
	m_nIndex  = 0;
	m_nFillCurve = 1;
	m_crFillColor = RGB(255, 255, 255);

}

CXGraphDataSerie::CXGraphDataSerie(const CXGraphDataSerie& copy)
{
	m_pData  = NULL;
	*this = copy;
}

CXGraphDataSerie& CXGraphDataSerie::operator =(const CXGraphDataSerie& copy)
{
  if (m_pData != NULL && m_bAutoDelete)
    delete []m_pData;
  
  m_bVisible			= copy.m_bVisible;
	m_bShowMarker		= copy.m_bShowMarker;
	m_bFillBeneath		= copy.m_bFillBeneath;
	m_pGraph			= copy.m_pGraph;
	m_nLineStyle		= copy.m_nLineStyle;
	m_nLineSize			= copy.m_nLineSize; 
	m_gtType			= copy.m_gtType;  
	m_nXAxis			= copy.m_nXAxis;
	m_nYAxis			= copy.m_nYAxis;
	m_crColor			= copy.m_crColor;
	m_nFillStyle		= copy.m_nFillStyle;
	m_nFirstVisible		= copy.m_nFirstVisible;
	m_nLastVisible		= copy.m_nLastVisible;
	m_bFillTransparent  = copy.m_bFillTransparent;
	m_CurveRegions      = copy.m_CurveRegions;
	m_nMarkerType	    = copy.m_nMarkerType;
	m_nMarker           = copy.m_nMarker;
	m_nMarkerSize		= copy.m_nMarkerSize;
	m_bAutoDelete		= copy.m_bAutoDelete;
	m_nCount			= copy.m_nCount; 
	m_nIndex            = copy.m_nIndex;
	m_cLabel		    = copy.m_cLabel;
	m_PS                = copy.m_PS;
	m_nFillCurve		= copy.m_nFillCurve;
	m_crFillColor       = copy.m_crFillColor;

	if (copy.m_bAutoDelete && copy.m_pData)
	{
		m_pData = new TDataPoint[copy.m_nCount];
		memcpy(m_pData, copy.m_pData, sizeof(TDataPoint) * copy.m_nCount);
	}
	else
		m_pData	= copy.m_pData;
		
	return *this;
}

CXGraphDataSerie::~CXGraphDataSerie()
{
	if (m_bAutoDelete)
		delete[] m_pData;
}

void CXGraphDataSerie::PrepareClipboard(CFDATASERIE& serie)
{
	serie.nCount = m_nCount;
	serie.nFirstVisible = m_nFirstVisible;
	serie.nLastVisible = m_nLastVisible;
	serie.nXAxis = m_nXAxis;
	serie.nYAxis = m_nYAxis;
	serie.nIndex = m_nIndex;
	serie.nLineSize = m_nLineSize;
	serie.nFillStyle = m_nFillStyle;
	serie.nMarkerType = m_nMarkerType;
	serie.nMarker = m_nMarker;
	serie.nMarkerSize = m_nMarkerSize;
	serie.nLineStyle = m_nLineStyle;
	serie.bAutoDelete = true;
	serie.bShowMarker = m_bShowMarker;
	serie.bFillBeneath = m_bFillBeneath;
	serie.nFillCurve = m_nFillCurve;
	serie.bFillTransparent = m_bFillTransparent;
	serie.crFillColor = m_crFillColor;

	serie.gtType = m_gtType;
#ifndef _WIN32_WCE
	_tcscpy(serie.cLabel, m_cLabel);
#else
	strcpy(serie.cLabel, (const char*)(LPCTSTR) m_cLabel);
#endif

}

void CXGraphDataSerie::DrawMarker(CDCEx *pDC)
{	
	CPoint point;

	int nWidth = m_pGraph->m_clInnerRect.Width() / m_nMarkerSize / 4;

	int nMarkerCount = 2;
	
	if (nWidth > 0)
		nMarkerCount = m_nCount / nWidth;

	if (nMarkerCount <= 0)
		nMarkerCount = 2;

	if (m_pGraph->m_opOperation == CXGraph::opEditCurve)
		nMarkerCount = 1;


	CXGraphAxis& yaxis = m_pGraph->GetYAxis(m_nYAxis);
	CXGraphAxis& xaxis = m_pGraph->GetXAxis(m_nXAxis);

	for (int i = 0; i < m_nCount; i += nMarkerCount)
	{
		CPoint point;

		point.x = xaxis.GetPointForValue(&m_pData[i]).x - (m_nMarkerSize / 2);
		point.y = yaxis.GetPointForValue(&m_pData[i]).y - (m_nMarkerSize / 2);
	
		int nMarker = m_nMarkerType == 0 ? m_pGraph->GetCurveIndex(this) : m_nMarker;

		yaxis.DrawMarker (pDC, point, nMarker, m_nMarkerSize , pDC->m_bMono ? 0L : m_crColor, m_nMarkerType == 1 );
	}
}

void CXGraphDataSerie::CreateCurveGripRgn(CPoint oldPoint, CPoint point)
{
	HITTEST ht;
	
	ht.p1 = oldPoint;
	ht.p2 = point;

	m_CurveRegions.push_back (ht);
}


void CXGraphDataSerie::Draw(CDCEx *pDC)
{
	static CPoint oldPoint(-1,-1);
	CPoint point;
	int    i = 0;
	POINT* pPoly;
		
	COLORREF crInverted = RGB(255 - GetRValue(m_crColor),
							  255 - GetGValue(m_crColor),
							  255 - GetBValue(m_crColor));

	CPenSelector ps(pDC->m_bMono ? 0L : (m_bSelected ? crInverted : m_crColor), m_nLineSize, pDC, m_nLineStyle); 

	point.x = m_pGraph->GetXAxis(m_nXAxis).GetPointForValue(&m_pData[i]).x;
	point.y = m_pGraph->GetYAxis(m_nYAxis).GetPointForValue(&m_pData[i]).y;


	if (m_bFillBeneath)
	{
		if (m_nFillCurve != -1)
		{
			CXGraphDataSerie& ds = GetGraph()->GetCurve (m_nFillCurve);
			long nCount = ds.GetCount () ;
			pPoly = new POINT[m_nCount + nCount];

		}
		else
		{
			pPoly = new POINT[m_nCount + 2];
		}

		pPoly[0].x = point.x;
		pPoly[0].y = point.y;
		
	}

	oldPoint = point;

	CXGraphAxis& yaxis = m_pGraph->GetYAxis(m_nYAxis);
	CXGraphAxis& xaxis = m_pGraph->GetXAxis(m_nXAxis);

	if (m_gtType == gtLine)
		pDC->MoveTo(point);

	if (m_gtType == gtScatter)
		pDC->Ellipse (point.x-m_nLineSize, point.y-m_nLineSize,point.x+m_nLineSize, point.y+m_nLineSize);

	m_CurveRegions.clear ();
	
	int nPoint = 1;
	
	for (i = 1; i < m_nCount; i++)
	{
		point.x = xaxis.GetPointForValue(&m_pData[i]).x;
		point.y = yaxis.GetPointForValue(&m_pData[i]).y;
					
		if (point.x != oldPoint.x )
		{
			if ((m_pGraph->m_clInnerRect.PtInRect (point) || m_pGraph->m_clInnerRect.PtInRect (oldPoint)))
				CreateCurveGripRgn(oldPoint, point);
			
			if (m_gtType == gtLine)
				pDC->LineTo(point);

			if (m_gtType == gtScatter)
				pDC->Ellipse (point.x-m_nLineSize, point.y-m_nLineSize,point.x+m_nLineSize, point.y+m_nLineSize);

			if (m_bFillBeneath)
			{
				pPoly[nPoint].x = point.x;
				pPoly[nPoint++].y = point.y;
			}
					
			oldPoint = point;
		}
	}

	if (m_bFillBeneath)
	{
		if (m_nFillCurve != -1)
		{
			CXGraphDataSerie& ds = GetGraph()->GetCurve (m_nFillCurve);
			CXGraphAxis& yaxis =  GetGraph()->GetYAxis (ds.GetYAxis());
			CXGraphAxis& xaxis = GetGraph()->GetXAxis (ds.GetXAxis());

			for (long nFC = ds.GetCount () - 1; nFC >= 0; nFC--)
			{
				point.x = xaxis.GetPointForValue(&ds.m_pData[nFC]).x;
				point.y = yaxis.GetPointForValue(&ds.m_pData[nFC]).y;
				pPoly[nPoint].x = point.x;
				pPoly[nPoint++].y = point.y;
			}

		}
		else
		{
			pPoly[nPoint++] = CPoint(oldPoint.x, m_pGraph->m_clInnerRect.bottom) ;
			pPoly[nPoint++] = CPoint(pPoly[0].x, m_pGraph->m_clInnerRect.bottom) ;
		}
		
		int nOldBkMode = pDC->GetBkMode ();

		pDC->SetBkMode(m_bFillTransparent ? TRANSPARENT : OPAQUE);
		
		int nOldROP2 = R2_COPYPEN;
		
		if (m_bFillTransparent)
			nOldROP2 = pDC->SetROP2 (R2_NOTXORPEN);
		
		
		// Make shure the polygon's outline is invisible
		CPenSelector ps(0, 0, pDC, PS_NULL);

		if (m_nFillStyle == HS_SOLID) 
		{
			// Solid
			
			CBrushSelector bs(pDC->m_bMono ? 0L : m_crFillColor, pDC);
			pDC->Polygon (pPoly, nPoint);
		}
#ifndef _WIN32_WCE
		else
		{
			// Hatched
			CBrushSelector bs(pDC->m_bMono ? 0L : m_crFillColor, m_nFillStyle, pDC);
			pDC->Polygon (pPoly, nPoint);
		}
#endif

		pDC->SetBkMode(nOldBkMode);
		pDC->SetROP2 (nOldROP2);

		delete pPoly;
	}

}

void CXGraphDataSerie::ResetVisibleRange()
{
	m_nFirstVisible = 0;
	m_nLastVisible  = m_nCount - 1;
}


TDataPoint* CXGraphDataSerie::GetLinearTrend(long nPoints)
{
	TDataPoint* pData;
	
	pData = new TDataPoint[m_nCount + nPoints];

	double fYSum  = 0.0,
		   fTYSum = 0.0,
		   fTSum  = 0.0,
		   fT2Sum = 0.0;
			
	for (long i = 0; i < m_nCount; i ++)
	{
		fYSum += m_pData[i].fYVal;
		fTYSum += (m_pData[i].fYVal * (i+1));
		fTSum += (i+1);
		fT2Sum += ((i+1)*(i+1));
	}

	double fDiff = m_pData[1].fXVal - m_pData[0].fXVal;

	for (int i = 0; i < (m_nCount + nPoints); i ++)
	{
		if (i < m_nCount)
			pData[i].fXVal = m_pData[i].fXVal;
		else
			pData[i].fXVal = pData[i-1].fXVal + fDiff;

		double fB = (m_nCount * fTYSum - fTSum * fYSum) / (m_nCount * fT2Sum - (fTSum * fTSum));
		double fA = (fYSum - fB * fTSum) / m_nCount;
		pData[i].fYVal = fA + fB * (i+1);
	}

	return pData;
}

TDataPoint* CXGraphDataSerie::GetCubicTrend(long nPoints)
{
	TDataPoint* pData;
	
	pData = new TDataPoint[m_nCount + nPoints];

	double fTY3Sum = 0.0,
		   fT6Sum = 0.0;
			
	for (long i = 0; i < m_nCount; i ++)
	{
		fTY3Sum += (m_pData[i].fYVal * pow(double(i+1), double(3)));
		fT6Sum  += (pow (double(i+1), double(6)));
	}

	double fDiff = m_pData[1].fXVal - m_pData[0].fXVal;

	for (int i = 0; i < (m_nCount + nPoints); i ++)
	{
		if (i < m_nCount)
			pData[i].fXVal = m_pData[i].fXVal;
		else
			pData[i].fXVal = pData[i-1].fXVal + fDiff;
	
		pData[i].fYVal = (fTY3Sum/fT6Sum) * pow (double(i+1), double(3));
	}

	return pData;
}



TDataPoint* CXGraphDataSerie::GetPolynomialTrend(int nDegree, int nPoints)
{
    TDataPoint* pData;
	
	pData = new TDataPoint[m_nCount + nPoints];

	m_PS.m_nGlobalO = nDegree;
	
	if (!m_PS.Polyfit (m_nCount, nDegree, m_pData))
	{
		delete pData;
		return NULL;
	}

	for ( int i = 0; i < (m_nCount + nPoints); i++ )
	{
		if (i < m_nCount)
			pData[i].fXVal = m_pData[i].fXVal;
		else
			pData[i].fXVal = pData[i-1].fXVal + (pData[i-1].fXVal - pData[i-2].fXVal);

		pData[i].fYVal = m_PS.GetValue (pData[i].fXVal);
	}

    return pData;
}


TDataPoint* CXGraphDataSerie::GetSimpleMovingAverage(int span)
{
    int	   p;
	double fSum = 0.0;
    
	TDataPoint* pData;
	
	pData = new TDataPoint[m_nCount];

	pData[0].fXVal = m_pData[0].fXVal;
	pData[0].fYVal = m_pData[0].fYVal;
	
    for ( int i = 1; i < m_nCount; i++ )
    {
		p = min(i, span);

  	    if ( p <= span )
        {
            fSum = 0.0;

            for ( int y = 0; y < p; y++ )
                fSum += m_pData[i-y].fYVal;
        }
        else
            fSum = fSum - m_pData[i-p].fYVal + m_pData[i].fYVal;
		
        pData[i].fYVal = fSum / (double) p;
		pData[i].fXVal = m_pData[i].fXVal;
    }

    return pData;
}

TDataPoint* CXGraphDataSerie::GetExponentialMovingAverage(int span)
{
    double  se    = 2.0 / ((double)span + 1.0);
    double  le    = 1.0 - se;

	TDataPoint* pData;
	
	pData = new TDataPoint[m_nCount];

	pData[0].fXVal = m_pData[0].fXVal;
	pData[0].fYVal = m_pData[0].fYVal;

    for ( int i = 1; i < m_nCount; i++ )
	{
        pData[i].fYVal = m_pData[i].fYVal * se + pData[i-1].fYVal * le;
		pData[i].fXVal = m_pData[i].fXVal;
	}

    return pData;
}

TDataPoint* CXGraphDataSerie::GetLinearMovingAverage(int span)
{
    TDataPoint* pData;

	double  fSum = 0.0;
    double  fDiv = 0.0;
	int     p;
	
	pData = new TDataPoint[m_nCount];

	pData[0].fXVal = m_pData[0].fXVal;
	pData[0].fYVal = m_pData[0].fYVal;

    for ( int i = 1; i < m_nCount; i++ )
	{
        fSum = 0.0;
        p = min(i, span);

        if ( p <= span )
        {
            fDiv = 0.0;

            for ( int y = 1; y <= p; y++ )
                fDiv += (double) y;
        }

        for ( int y = 0; y < p; y++ )
            fSum += m_pData[i-y].fYVal * (p - y);

        pData[i].fYVal = fSum / fDiv;
		pData[i].fXVal = m_pData[i].fXVal;
    }

	return pData;
}

inline double W_BARTLETT(double n, double k, double d = 1.0)
{
    return (n == 0) ? 0 : (d * (1 - fabs((k - 0.5 * n) / (0.5 * n))));
}

TDataPoint* CXGraphDataSerie::GetTriangularMovingAverage(int span)
{
    TDataPoint* pData;

	double  fSum = 0.0;
    double  fDiv = 0.0;
	int     p;
	
	pData = new TDataPoint[m_nCount];

	pData[0].fXVal = m_pData[0].fXVal;
	pData[0].fYVal = m_pData[0].fYVal;

    for ( int i = 1; i < m_nCount; i++ )
	{
        fSum = 0.0;
        p    = min(i, span);

        if ( p <= span )
        {
            fDiv = 0.0;

            for ( int y = 1; y <= p; y++ )
                fDiv += W_BARTLETT(p+1, y, p);
        }

        for ( int y = 1; y <= p; y++ )
            fSum += m_pData[i-y+1].fYVal * W_BARTLETT(p+1, y, p);

        pData[i].fYVal = fSum / fDiv;
		pData[i].fXVal = m_pData[i].fXVal;
    }

    return pData;
}


TDataPoint* CXGraphDataSerie::GetSineWeightedMovingAverage(int span)
{
    TDataPoint* pData;

	double  fSum = 0.0;
    double  fDiv = 0.0;
	double  d    = 0.0;
	double  f    = _PI / (double) (span + 1);
	int     p;
	
	pData = new TDataPoint[m_nCount];

	pData[0].fXVal = m_pData[0].fXVal;
	pData[0].fYVal = m_pData[0].fYVal;

    for ( int i = 1; i < m_nCount; i++ )
	{
        fSum = 0.0;
        p    = min( i, span );

        for ( int y = 0; y < p; y++ )
            fSum += sin((double)(p - y) * f) * m_pData[i-y].fYVal;

        if ( i <= span )
        {
            d = 0.0;
            for ( int y = 1; y <= p; y++ )
                d += sin((double) y * f);
        }

        pData[i].fYVal = fSum / d;
		pData[i].fXVal = m_pData[i].fXVal;
    }

    return pData;
}

BOOL CXGraphDataSerie::HitTestLine(POINT pt0, POINT pt1, POINT ptMouse, int nWidth)
{
  VECTOR2D tt0, tt1;
  double   dist;
  int      nHalfWidth;
  //
  //Get the half width of the line to adjust for hit testing of wide lines.
  //
  nHalfWidth = (nWidth/2 < 1) ? 1 : nWidth/2;
  
  //
  //Convert the line into a vector using the two endpoints.
  //
  POINTS2VECTOR2D(pt0, pt1, tt0);
  
  //Convert the line from the left endpoint to the mouse point into a vector.
  //
  POINTS2VECTOR2D(pt0, ptMouse, tt1);
  //
  //Obtain the distance of the point from the line.
  //
  dist = vDistFromPointToLine(&pt0, &pt1, &ptMouse);
  //
  //Return TRUE if the distance of the point from the line is within the width 
  //of the line
  //

  BOOL bRet = (dist >= -nHalfWidth && dist <= nHalfWidth);
  
  return bRet;
}



void CXGraphDataSerie::Serialize( CArchive& archive )
{
	int nHelper;

	CXGraphObject::Serialize (archive);

    if( archive.IsStoring() )
    {
		archive << m_nCount;

		archive.Write (m_pData, sizeof(TDataPoint) * m_nCount);
		
		archive << m_nFirstVisible;
		archive << m_nLastVisible;
		archive << m_nXAxis;
		archive << m_nYAxis;
		archive << m_nIndex;
		archive << m_nLineSize;
		archive << m_nFillStyle;
		archive << m_nMarkerType;
		archive << m_nMarker;
		archive << m_nMarkerSize;
		archive << m_nLineStyle;
		archive << m_bShowMarker;
		archive << m_bFillBeneath;
		archive << m_bFillTransparent;
		archive << m_nFillCurve;
		archive << m_crFillColor;
		archive << m_cLabel;
		nHelper = (int) m_gtType;
		archive << nHelper;

		
    }
	else
    {
		archive >> m_nCount;

		m_bAutoDelete = true;

		m_pData = new TDataPoint[m_nCount];
		
		archive.Read (m_pData, sizeof(TDataPoint) * m_nCount);

		archive >> m_nFirstVisible;
		archive >> m_nLastVisible;
		archive >> m_nXAxis;
		archive >> m_nYAxis;
		archive >> m_nIndex;
		archive >> m_nLineSize;
		archive >> m_nFillStyle;
		archive >> m_nMarkerType;
		archive >> m_nMarker;
		archive >> m_nMarkerSize;
		archive >> m_nLineStyle;
		archive >> m_bShowMarker;
		archive >> m_bFillBeneath;
		archive >> m_bFillTransparent;
		archive >> m_nFillCurve;
		archive >> m_crFillColor;
		archive >> m_cLabel;
		archive >> nHelper;
		m_gtType = (EGraphType) nHelper;
    }
}


/////////////////////////////////////////////////////////////////

bool CPolynomialSolver::Solve(double a[], double b[], int n)
{
	for (int i = 0; i < n; i ++)
	{
		// find pivot
		double mag = 0;
		int pivot = -1;
		
		for (int j = i; j < n; j ++)
		{
			double mag2 = fabs(a[i + j * n]);
			if (mag2 > mag)
			{
				mag = mag2;
				pivot = j;
			}
		}
			
		// no pivot: error
		if (pivot == -1 || mag == 0) 
			return false;
		
		// move pivot row into position
		if (pivot != i)
		{
			double temp;
			for (int j = i; j < n; j ++)
			{
				temp = a[j + i * n];
				a[j + i * n] = a[j + pivot * n];
				a[j + pivot * n] = temp;
			}
				
			temp = b[i];
			b[i] = b[pivot];
			b[pivot] = temp;
		}
			
		// normalize pivot row
		mag = a[i + i * n];
		for (int j = i; j < n; j ++) a[j + i * n] /= mag;
		b[i] /= mag;
		
		// eliminate pivot row component from other rows
		for (int i2 = 0; i2 < n; i2 ++)
		{
			if (i2 == i) continue;
			
			double mag2 = a[i + i2 * n];
			
			for (int j = i; j < n; j ++) 
				a[j + i2 * n] -= mag2 * a[j + i * n];
			
			b[i2] -= mag2 * b[i];
		}
	}

	return true;
}

bool CPolynomialSolver::Polyfit(int nRows, int nOrder, TDataPoint *pData)
{
	int rows   = nRows;
	int order  = nOrder;
	m_nGlobalO = order;

	double *base   = new double[order * rows];
	double *alpha  = new double[order * order];
	double *alpha2 = new double[order * order];
	double *beta   = new double[order];
			
	// calc base
	for (int i = 0; i < order; i ++)
	{
		for (int j = 0; j < rows; j ++)
		{
			int k = i + j * order;
			base[k] = i == 0 ? 1.0 : pData[j].fXVal * base[k - 1];
		}
	}
				
	// calc alpha2
	for (int i = 0; i < order; i ++)
	{
		for (int j = 0; j <= i; j ++)
		{
			double sum = 0.0;
			for (int k = 0; k < rows; k ++)
			{
				int k2 = i + k * order;
				int k3 = j + k * order;
				sum += base[k2] * base[k3];
			}

			int k2 = i + j * order;

			alpha2[k2] = sum;
			
			if (i != j)
			{
				k2 = j + i * order;
				alpha2[k2] = sum;
			}
		}
	}
	
	// calc beta
	for (int j = 0; j < order; j ++)
	{
		double sum = 0;
		for (int k = 0; k < rows; k ++)
		{
			int k3 = j + k * order;
			sum += pData[k].fYVal  * base[k3];
		}

		beta[j] = sum;
	}
		
	// get alpha
	for (int j = 0; j < order * order; j ++) 
		alpha[j] = alpha2[j];
	
	// solve for params
	bool bRes = Solve(alpha, beta, order);
				
	for (int j = 0; j < order; j ++)
		m_fC[j] = beta[j];
	
	delete base;
	delete beta;
	delete alpha;
	delete alpha2;

	return bRes;
}


double CPolynomialSolver::GetValue(double fX)
{
	double fRes = 0.0;

	for (int i = 0; i < m_nGlobalO; i++)
		fRes += (m_fC[i] * pow(fX, i));

	return fRes;
}


