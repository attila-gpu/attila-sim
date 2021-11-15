#pragma once
#include "xgraphaxis.h"

#ifdef  _AFXDLL
class __declspec(dllexport) CXGraphLogAxis : public CXGraphAxis
#else
class __declspec(dllimport) CXGraphLogAxis : public CXGraphAxis
#endif
{
	DECLARE_SERIAL(CXGraphLogAxis)
public:
	CXGraphLogAxis(void);
	CXGraphLogAxis(const CXGraphLogAxis& copy);
	~CXGraphLogAxis(void);

// Attribute
	enum EAxisType {LOG, LINEAR};
private:
	EAxisType m_AxisType;

// Operation
public:
	bool	SetRange(double fMin, double fMax);
	void	AutoScale(CDCEx* pDC);
	void	Draw(CDCEx* pDC);
	double	GetValueForPos(int nPos);
	CPoint	GetPointForValue(TDataPoint* point);
	void	SetAxisType(EAxisType type){m_AxisType = type;};

	CXGraphLogAxis& operator = (const CXGraphLogAxis& copy);
};
