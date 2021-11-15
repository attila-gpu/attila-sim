#include "stdafx.h"
#include "xgraph.h"
#include "xgraphlogaxis.h"
#include <math.h>

IMPLEMENT_SERIAL(CXGraphLogAxis,CXGraphAxis,1)

CXGraphLogAxis::CXGraphLogAxis(void)
{
	m_AxisType = LOG;
}

CXGraphLogAxis::CXGraphLogAxis(const CXGraphLogAxis& copy)
{
	*this = copy;
}

CXGraphLogAxis::~CXGraphLogAxis(void)
{
}

bool CXGraphLogAxis::SetRange(double fMin, double fMax)
{
	if (m_AxisType == LINEAR)
	{
		return CXGraphAxis::SetRange(fMin,fMax);
	}
	else
	{
		// in log scale, m_fStep is not be used.
		if (fMin == fMax)
		{
			fMin *= 0.1;
			fMax *= 10;
		}
		m_fStep = 0.0;

		if (fMin < 1e-16)
			m_fCurMin = m_fMin = 1e-16;
		else
			m_fCurMin = m_fMin = fMin;
		
		if (fMax < 1e-15)
			m_fCurMax = m_fMax = 1e-15;
		else
			m_fCurMax = m_fMax = fMax;

		return true;
	}
}

void CXGraphLogAxis::AutoScale(CDCEx* pDC)
{
	if (m_AxisType == LINEAR)
	{
		CXGraphAxis::AutoScale(pDC);
		return;
	}
	else
	{
		// set range of graph.. 
		double fStep1 = 0.0;
		double fStep2 = 0.0;
		BOOL bFound = FALSE;
		for (fStep1 = 1e-16; fStep1 < m_fCurMin; fStep1 *= 10)
		{
			for (fStep2 = fStep1; fStep2 < fStep1*10; fStep2 += fStep1)
			{
				if (fStep2 > m_fCurMin)
				{
					m_fCurMin = fStep2 - fStep1;
					bFound = TRUE;
					break;
				}
			}
			if (bFound == TRUE)
				break;
		}
		
		bFound = FALSE;
		for (fStep1 = 1e-16; fStep1 < m_fCurMax; fStep1 *= 10)
		{
			for (fStep2 = fStep1; fStep2 < fStep1*10; fStep2 += fStep1)
			{
				if (fStep2 > m_fCurMax)
				{
					m_fCurMax = fStep2;
					bFound = TRUE;
					break;
				}
			}
			if (bFound == TRUE)
				break;
		}
	}
}

void CXGraphLogAxis::Draw(CDCEx* pDC)
{
	if (m_AxisType == LINEAR)
	{
		CXGraphAxis::Draw(pDC);
	    return;
	}
	else
	{
		CDCEx *pTmpDC = NULL;

		if (!m_bVisible)
		{
			pTmpDC = new CDCEx;
			pTmpDC->CreateCompatibleDC (pDC);
			pDC = pTmpDC;
		}

		if (m_bAutoScale)
			AutoScale(pDC);


		m_fRange = m_fCurMax - m_fCurMin;

		if (m_AxisKind == yAxis)
		{	
			m_fSpareRange = (2*m_nArcSize);

			m_fMarkerSpareSize = DrawCurveMarkers(pDC, false);

			if (m_cLabel != "")
			{
				CSize titleSize;
				{
					CFontSelector fs(&m_TitleFont, pDC, false);
					titleSize = GetTitleSize(pDC);
					m_fSpareRange += titleSize.cy;
				}
			}
			int		nChartTop = m_clChart.top + m_fMarkerSpareSize;
			int		nTop      = m_pGraph->m_clInnerRect.top + m_fSpareRange + m_fMarkerSpareSize;
			int		nBottom   = m_pGraph->m_clInnerRect.bottom;
			double  fnY       = m_nTop;
			
			m_nRange = nBottom - nTop;
			//m_nRange = m_nTop - fnYC - fStep;
		
			DrawColorRanges(pDC);

			m_fMarkerSpareSize = DrawCurveMarkers(pDC);
					
			if (pDC->m_bMono)
				pDC->SetBkColor(RGB(255,255,255));
			else
				pDC->SetBkColor(m_pGraph->m_crGraphColor);

			pDC->SetBkMode(OPAQUE);
		
			if (m_cLabel != "")
			{
				CSize titleSize;

				{
					CFontSelector fs(&m_TitleFont, pDC, false);
					titleSize = GetTitleSize(pDC);
					m_fSpareRange += titleSize.cy;
				}
				
				CFontSelector fs(&m_TitleFont, pDC);
				
				if (m_Placement == apLeft)
				{
					CRect clTitle(m_nLeft - titleSize.cx - m_nTickSize, nChartTop, m_nLeft - m_nTickSize, nChartTop + titleSize.cy);
					pDC->DrawText(m_cLabel, clTitle, DT_RIGHT);
				}
				else
				{
					CRect clTitle(m_nLeft + m_nTickSize, nChartTop, m_nLeft + titleSize.cx + m_nTickSize, nChartTop + titleSize.cy);
					pDC->DrawText(m_cLabel, clTitle, DT_LEFT);
				}
			}
			
			CString cTxt;
			CSize   csText;
	 
			// log scale
			double minValue = pow(10,floor(log10(m_fCurMin)));
			double maxValue = pow(10,ceil(log10(m_fCurMax)));
			TDataPoint dataPoint;
			for (double fY1 = minValue; fY1 < maxValue; fY1 *= 10)
			{
				for (double fY2 = fY1; fY2 < (fY1*10); fY2 += fY1)
				{
					if ((fY2 >= m_fCurMin) && (fY2 <= m_fCurMax))
					{
						dataPoint.fYVal = fY2;
						fnY = GetPointForValue(&dataPoint).y;
						// draw text
						{
							CFontSelector fs(&m_Font, pDC, false);
								
							if (m_bDateTime)
								cTxt = COleDateTime(fY2).Format(m_cDisplayFmt);
							else
								cTxt.Format(m_cDisplayFmt, fY2);

							csText = GetTextExtentEx (pDC, cTxt);
						}

						CFontSelector fs(&m_Font, pDC);

						if (m_Placement == apLeft)
						{
							if (fY2 == fY1)
							{
								CRect crText(m_nLeft - csText.cx - m_nTickSize, fnY - csText.cy / 2, m_nLeft - m_nTickSize, fnY + csText.cy / 2);
								pDC->DrawText(cTxt, crText, DT_RIGHT);
							}
							CPenSelector ps(m_crGridColor, 1, pDC, m_nGridStyle);
							pDC->MoveTo(m_nLeft - m_nTickSize, fnY);
							// Spare last gridline to prevent axis to be overdrawn
							if (!m_bShowGrid || (int)fnY == m_nTop)
								pDC->LineTo(m_nLeft, fnY);
							else
								pDC->LineTo(m_pGraph->m_clInnerRect.right, fnY);
					
						}
						
						if (m_Placement == apRight)
						{
							if (fY2 == fY1)
							{
								CRect crText(m_nLeft + m_nTickSize, fnY - csText.cy / 2, m_nLeft + csText.cx + m_nTickSize, fnY + csText.cy / 2);
								pDC->DrawText(cTxt, crText, DT_LEFT);
							}
							CPenSelector ps(m_crGridColor, 1, pDC, m_nGridStyle);
							pDC->MoveTo(m_nLeft + m_nTickSize, fnY);
							// Spare last gridline to prevent axis to be overdrawn
							if (!m_bShowGrid || (int)fnY == m_nTop)
								pDC->LineTo(m_nLeft, fnY);
							else
								pDC->LineTo(m_pGraph->m_clInnerRect.left, fnY);
						}
					}
				}
			}
	
			CPenSelector ps(m_crColor, 1, pDC);
			pDC->MoveTo (m_nLeft, m_nTop);
			pDC->LineTo (m_nLeft, nChartTop);
			DrawArc(pDC, CPoint(m_nLeft, nChartTop), up, m_nArcSize);
			m_clRect.SetRect(m_nLeft - 5, nChartTop, m_nLeft + 5, m_nTop);
		//	m_nRange = m_nTop - fnY - fStep;

		}
		else
		{
			m_fSpareRange = (4*m_nArcSize);
			m_fMarkerSpareSize = DrawCurveMarkers(pDC, false);
			
			if (m_cLabel != "")
			{	
				CSize titleSize;
				{
					CFontSelector fs(&m_TitleFont, pDC, false);
					titleSize = GetTitleSize(pDC);
					m_fSpareRange += titleSize.cx;
				}
			}

			int    nChartRight = m_pGraph->m_clInnerRect.right - m_fMarkerSpareSize; 
			int    nLeft       = m_pGraph->m_clInnerRect.left;
			int	   nRight      = m_pGraph->m_clInnerRect.right - m_fSpareRange -  m_fMarkerSpareSize;
			double fSteps	   = ((m_fCurMax - m_fCurMin) / m_fStep);
			double fStep	   = ((double)(nChartRight - m_nLeft - m_fSpareRange) / fSteps);
			double fnX		   = m_nLeft;
			
			m_nRange = nRight - nLeft;
			//m_nRange = nChartRight - nLeft;
			//m_nRange = fnXC - m_nLeft - fStep;
		
			DrawColorRanges(pDC);

			m_fMarkerSpareSize = DrawCurveMarkers(pDC);

			if (pDC->m_bMono)
				pDC->SetBkColor(RGB(255,255,255));
			else
				pDC->SetBkColor(m_pGraph->m_crGraphColor);

			pDC->SetBkMode(OPAQUE);

			CString cTxt;
			CSize   csText;
			
			if (m_cLabel != "")
			{	
				CSize titleSize;
				{
					CFontSelector fs(&m_TitleFont, pDC, false);
					titleSize = GetTitleSize(pDC);
					m_fSpareRange += titleSize.cx;
				}
				CFontSelector fs(&m_TitleFont, pDC);
				CRect clTitle(nChartRight - titleSize.cx, m_nTop + m_nTickSize, nChartRight, m_nTop + m_nTickSize + titleSize.cy);
				pDC->DrawText(m_cLabel, clTitle, DT_LEFT);
			}

			// Draw X-Axis Number and Line..
			double minValue = pow(10,floor(log10(m_fCurMin)));
			double maxValue = pow(10,ceil(log10(m_fCurMax)));
			TDataPoint dataPoint;
			for (double fX1 = minValue; fX1 < maxValue; fX1 *= 10)
			{
				for (double fX2 = fX1; fX2 < (fX1*10); fX2 += fX1)
				{
					if ((fX2 >= m_fCurMin) && (fX2 <= m_fCurMax))
					{
						dataPoint.fXVal = fX2;
						fnX = GetPointForValue(&dataPoint).x;
						// draw text
						if (m_bDateTime)
							cTxt = COleDateTime(fX2).Format(m_cDisplayFmt);
						else
							cTxt.Format(m_cDisplayFmt, fX2);
		
						{
							CFontSelector fs(&m_Font, pDC, false);
							csText = GetTextExtentEx (pDC, cTxt);
						}

						CFontSelector fs(&m_Font, pDC);

						if (fX1 == fX2)
						{
							CRect crText(fnX - csText.cx / 2, m_nTop + m_nTickSize, fnX + csText.cx / 2, m_nTop + csText.cy + m_nTickSize);
							pDC->DrawText(cTxt, crText, DT_CENTER);
						}
						CPenSelector ps(m_crGridColor, 1, pDC, m_nGridStyle);
						pDC->MoveTo(fnX, m_nTop + m_nTickSize);
						
						// Spare last gridline to prevent axis to be overdrawn
						if (!m_bShowGrid || (int)fnX == m_nLeft)
							pDC->LineTo(fnX, m_nTop);
						else
							pDC->LineTo(fnX, m_clChart.top);
					}
				}
			}

			CPenSelector ps(m_crColor, 1, pDC);
			pDC->MoveTo (m_nLeft, m_nTop);
			pDC->LineTo (nChartRight, m_nTop);
			DrawArc(pDC, CPoint(nChartRight, m_nTop), right, m_nArcSize);
			m_clRect.SetRect(m_nLeft, m_nTop - 5, nChartRight, m_nTop + 5);
	//		m_nRange = fnX - m_nLeft - fStep;
		}

		if (m_bSelected)
			pDC->DrawFocusRect (m_clRect);

		if (pTmpDC)
			delete pTmpDC;
	}
}

double CXGraphLogAxis::GetValueForPos(int nPos)
{
	if (m_AxisType == LINEAR)
	{
		return CXGraphAxis::GetValueForPos(nPos);
	}
	else
	{
		double fVal = 0.0;
		double fRange = log10(m_fCurMax) - log10(m_fCurMin);
		if (m_AxisKind == yAxis)
			fVal = pow(10,-(nPos-m_pGraph->m_clInnerRect.bottom)/(double)m_nRange *fRange + log10(m_fCurMin));
		else
			fVal = pow(10, (nPos-m_pGraph->m_clInnerRect.left)/(double)m_nRange * fRange + log10(m_fCurMin));
		return fVal;
	}
}

CPoint CXGraphLogAxis::GetPointForValue(TDataPoint* point)
{
	if (m_AxisType == LINEAR)
	{
		return CXGraphAxis::GetPointForValue(point);
	}
	else
	{
		CPoint res = CPoint(0,0);
		m_fRange = m_fCurMax - m_fCurMin;
		double ratio = 0.0;
		if (m_AxisKind == yAxis)
		{
            ratio = (log10(point->fYVal) - log10(m_fCurMin))/(log10(m_fCurMax)-log10(m_fCurMin));
			res.y = m_pGraph->m_clInnerRect.bottom - ratio * (double) m_nRange;
		}
		else
		{
			ratio = (log10(point->fXVal) - log10(m_fCurMin))/(log10(m_fCurMax)-log10(m_fCurMin));
			res.x = m_pGraph->m_clInnerRect.left + ratio * (double) m_nRange;
		}
		return res;
	}
}

CXGraphLogAxis& CXGraphLogAxis::operator =(const CXGraphLogAxis& copy)
{
	CXGraphAxis::operator = (copy);

	m_AxisType = copy.m_AxisType;

	return *this;
}
