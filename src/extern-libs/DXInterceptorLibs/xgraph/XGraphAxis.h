// XGraphAxis.h: Schnittstelle für die Klasse CXGraphAxis.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XGRAPHAXIS_H__C1683795_71B2_4784_BAB6_CEBEB78C3723__INCLUDED_)
#define AFX_XGRAPHAXIS_H__C1683795_71B2_4784_BAB6_CEBEB78C3723__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XGraphDataSerie.h"

typedef struct tagZOOM
{
	double fMin;
	double fMax;

} ZOOM;

typedef struct tagCOLORRANGE
{
	double   fMin;
	double   fMax;
	COLORREF crMinColor;
	COLORREF crMaxColor;
	UINT     nStyle;
#ifndef _WIN32_WCE
	_TCHAR   szLabel[_MAX_PATH];
#else
	char     szLabel[_MAX_PATH];
#endif
} COLORRANGE;


typedef struct tagAXISMARKER
{
	double   fValue;
	COLORREF crColor;
	bool     bVisible;
	bool	 bShowValue;
	UINT     nStyle;
	short    nSize;
	LOGFONT  lfFont;
	CString  szLabel;

} AXISMARKER, *PAXISMARKER;

class CXGraphAxis : public CXGraphObject 
{
	DECLARE_SERIAL( CXGraphAxis )

	friend class CXGraph;
	friend class CXGraphDataSerie;

public:

	enum EAxisPlacement
	{
		apLeft,
		apRight,
		apAutomatic
	};

	enum EAxisType 
	{
		atLog,
		atLinear
	};

	CXGraphAxis();
	CXGraphAxis(const CXGraphAxis& copy);
	virtual ~CXGraphAxis();

	CXGraphAxis& operator = (const CXGraphAxis& copy);

protected:

	

	enum EAxisKind {
		xAxis,
		yAxis
	};

	enum EDirection {

		left,
		up,
		right,
		down,
		circle,
		circleClosed,
		rect,
		rectClosed
	};

	vector <COLORRANGE> m_ColorRanges;
	vector <ZOOM>       m_ZoomHistory;
	vector <AXISMARKER> m_AxisMarkers;

	double m_fMin;
	double m_fMax;
	double m_fCurMin;
	double m_fCurMax;
	double m_fStep;
	double m_fSpareRange;
	double m_fMarkerSpareSize;
	double m_fRange;
	
	static double TimeStepTable[];
	
	int   m_nTop;
	int   m_nLeft;
	int   m_nRange;
	CRect m_clChart;
	CFont m_Font;
	CFont m_TitleFont;
	
	EAxisKind m_AxisKind;
	EAxisType m_AxisType;

	virtual void Draw(CDCEx* pDC);
	void   DrawLog(CDCEx* pDC);

	double RoundDouble(double doValue, int nPrecision);
	void   AdaptAxis(double* fStart, double* fEnd, double fStep);
	void   AdaptTimeAxis(double * pfStart, double * pfEnd, double fStep);
	void   FitTimeScale(double *fStepWidth, int nBestCount, double fStart, double fEnd);
	void   FitScale (double *fStepWidth, int nBestCount, double fStart, double fEnd);

	void   DrawAxisMarkers(CDCEx *pDC);
	void   DrawColorRanges(CDCEx *pDC);
	void   DrawArc(CDCEx *pDC, CPoint point, EDirection direction, int nSize);
	void   DrawMarker(CDCEx *pDC, CPoint point, int nMarker, int nSize, COLORREF crColor, bool bSymbol = false);
	int	   DrawCurveMarkers(CDCEx *pDC, bool bDraw = true);

	int    GetMaxLabelWidth(CDCEx *pDC);
	int    GetMaxLabelHeight(CDCEx *pDC);

	void   GetIndexByXVal(long& nIndex, double x, int nCurve);
	void   AutoScale(CDCEx *pDC);
	void   SetBestStep();

	CSize  GetTitleSize(CDCEx *pDC);


protected:

	EAxisPlacement m_Placement;

	bool     m_bShowMarker;
	bool     m_bDateTime;
	bool     m_bShowGrid;
	bool     m_bAutoScale;
	CString  m_cLabel;
	CString  m_cDisplayFmt;
	int      m_nArcSize;
	int      m_nMarkerSize;
	int      m_nTickSize;
	COLORREF m_crGridColor;
	UINT     m_nGridStyle;
	
public:

	inline void SetAxisType(EAxisType type){ m_AxisType = type; };
	inline EAxisType GetAxisType(){ return m_AxisType; };

	inline void SetShowMarker(bool bValue) { m_bShowMarker = bValue; };
	inline void SetDateTime(bool bValue) { m_bDateTime = bValue; };
	inline void SetShowGrid(bool bValue) { m_bShowGrid = bValue; };
	inline void SetAutoScale(bool bValue) { m_bAutoScale = bValue; };
	inline void SetArcSize(int nValue) { m_nArcSize = nValue; };
	inline void SetMarkerSize(int nValue) { m_nMarkerSize = nValue; };
	inline void SetTickSize(int nValue) { m_nTickSize = nValue; };
	inline void SetGridStyle(UINT nValue) { m_nGridStyle = nValue; };
	inline void SetLabel(CString cValue) { m_cLabel = cValue; };
	inline void SetDisplayFmt(CString cValue) { m_cDisplayFmt = cValue; };
	inline void SetGridColor(COLORREF color) { m_crGridColor = color; };
	inline void SetPlacement(EAxisPlacement placement) { m_Placement = placement; };
	 
	inline EAxisPlacement GetPlacement() const { return m_Placement; };
	inline bool GetShowMarker() const { return m_bShowMarker; };
	inline bool GetDateTime() const { return m_bDateTime; };
	inline bool GetShowGrid() const { return m_bShowGrid; };
	inline bool GetAutoScale() const { return m_bAutoScale; };
	inline int  GetArcSize() const { return m_nArcSize; };
	inline int  GetMarkerSize() const { return m_nMarkerSize; };
	inline int  GetTickSize() const { return m_nTickSize; };
	inline UINT GetGridStyle() const { return m_nGridStyle; };
	inline CString GetLabel() const { return m_cLabel; };
	inline CString GetDisplayFmt() const { return m_cDisplayFmt; };
	inline COLORREF GetGridColor() const { return m_crGridColor; };
	inline long GetAxisMarkerCount() { return (int) m_AxisMarkers.size(); };

	CPoint GetPointForValue(TDataPoint* point);
	double GetValueForPos(int nPos);

	void   SetAxisMarker(int nMarker, double fValue, COLORREF crColor, UINT nStyle = PS_SOLID);
	void   DeleteAxisMarker(int nMarker);
	AXISMARKER& GetAxisMarker(int nMarker);
	

	void   SetColorRange(int nRange, double fMin, double fMax, COLORREF crMinColor, COLORREF crMaxColor, CString cLabel, UINT nStyle = HS_SOLID);
	void   DeleteColorRange(int nRange);
	void   DeleteAllColorRanges();
		
	void   Reset();
	bool   SetRange(double fMin, double fMax);
	bool   SetCurrentRange(double fMin, double fMax, double fStep = 0.0);
	void   SetFont (CString cFontName, int nFontHeight, int nFontStyle);
	void   SetFont (LPLOGFONT pLogFont);
	void   SetTitleFont (LPLOGFONT pLogFont);
	void   SetTitleFont (CString cFontName, int nFontHeight, int nFontStyle);
	void   GetRange(double& fMin, double& fMax);
  void   GetCurrentRange(double& fMin, double& fMax, double& fStep);
	void   Scale(double fFactor);

	virtual void Serialize( CArchive& archive );
};

#endif // !defined(AFX_XGRAPHAXIS_H__C1683795_71B2_4784_BAB6_CEBEB78C3723__INCLUDED_)
