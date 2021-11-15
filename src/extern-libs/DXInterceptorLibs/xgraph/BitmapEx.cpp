// BitmapEx.cpp: Implementierung der Klasse CBitmapEx.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BitmapEx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL( CBitmapEx, CBitmap, 1 )

void CBitmapEx::Serialize( CArchive& archive )
{
#ifndef _WIN32_WCE
	if (archive.IsStoring())
	{
		BITMAP bm;
		int nSize;
		GetObject (sizeof(BITMAP),&bm);
		archive << bm.bmType;
		archive << bm.bmWidth;
		archive << bm.bmHeight;
		archive << bm.bmWidthBytes;
		archive << bm.bmPlanes;
		archive << bm.bmBitsPixel;

		nSize = bm.bmWidthBytes * bm.bmHeight;

		if (bm.bmBits == NULL)
		{
			bm.bmBits = new BYTE[nSize];
			GetBitmapBits(nSize, bm.bmBits);
			archive.Write(bm.bmBits, nSize);
			delete bm.bmBits;
		}
		else
			archive.Write(bm.bmBits, nSize);
	}
	else
	{
		BITMAP bm;
		int nSize;
		archive >> bm.bmType;
		archive >> bm.bmWidth;
		archive >> bm.bmHeight;
		archive >> bm.bmWidthBytes;
		archive >> bm.bmPlanes;
		archive >> bm.bmBitsPixel;
		nSize = bm.bmWidthBytes * bm.bmHeight;
		bm.bmBits = new BYTE[nSize];
		archive.Read(bm.bmBits, nSize);
		CreateBitmapIndirect(&bm);
		delete bm.bmBits;

	}
#endif
}
