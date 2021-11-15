// XGraphObjectBase.cpp: Implementierung der Klasse CXGraphObjectBase.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XGraph.h"
#include "XGraphObjectBase.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL( CXGraphObject, CObject, 1 )

CXGraphObject::CXGraphObject()
{
	m_bSelected	 = false;
	m_bCanResize = false;
	m_bCanMove	 = false;
	m_bSizing	 = false;
	m_bEditing	 = false;
	m_bVisible   = true;
	m_bCanEdit   = true;
	m_pGraph	 = NULL;
	m_clRect.SetRectEmpty ();
}

CXGraphObject::CXGraphObject(const CXGraphObject& copy)
{
	*this = copy;
}

CXGraphObject& CXGraphObject::operator=(const CXGraphObject& copy)
{
	m_clRect     = copy.m_clRect;
	m_bSelected  = copy.m_bSelected;
	m_bCanResize = copy.m_bCanResize;
	m_bCanMove	 = copy.m_bCanMove;
	m_bSizing	 = copy.m_bSizing;
	m_bEditing	 = copy.m_bEditing;
	m_crColor	 = copy.m_crColor;
	m_pGraph	 = copy.m_pGraph;
	m_Tracker	 = copy.m_Tracker;
	m_bVisible   = copy.m_bVisible;
	m_bCanEdit   = copy.m_bCanEdit;
	
	return *this;
}

CXGraphObject::~CXGraphObject()
{
}

void CXGraphObject::BeginSize()
{
	m_bSizing = true;
	m_Tracker.m_nStyle = CRectTracker::hatchedBorder | CRectTracker::resizeOutside;
	m_Tracker.m_rect   = m_clRect;
}

void CXGraphObject::EndSize()
{
	m_bSizing = false;
};


void CXGraphObject::Serialize( CArchive& archive )
{
	CObject::Serialize (archive);

    if( archive.IsStoring() )
    {
		archive << m_bSelected;
		archive << m_bCanResize;
		archive << m_bCanMove;
		archive << m_bSizing;
		archive << m_bEditing;
		archive << m_bVisible;
		archive << m_bCanEdit;
		archive << m_clRect;
		archive << m_crColor;
    }
	else
    {
		archive >> m_bSelected;
		archive >> m_bCanResize;
		archive >> m_bCanMove;
		archive >> m_bSizing;
		archive >> m_bEditing;
		archive >> m_bVisible;
		archive >> m_bCanEdit;
		archive >> m_clRect;
		archive >> m_crColor;
    }
}