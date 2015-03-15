////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GdiPlusHelper.h"

using namespace Gdiplus;

////////////////////////////////////////////////////////////////////////////////

int GdiPlusHelper::GetEncoderCLSID(const WCHAR* format, CLSID* pClsid)
{
  UINT num;  // number of image encoders
  UINT size; // size of the image encoder array in bytes

  GetImageEncodersSize(&num, &size);
  if (size == 0)
  {
    return -1; // Failure
  }

  ImageCodecInfo* pImageCodecInfo = new ImageCodecInfo[size];
  if (pImageCodecInfo == NULL)
  {
    return -1; // Failure
  }

  GetImageEncoders(num, size, pImageCodecInfo);

  for (UINT j = 0; j < num; ++j)
  {
    if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
    {
      *pClsid = pImageCodecInfo[j].Clsid;
      delete[] pImageCodecInfo;
      return j; // Success
    }    
  }

  delete[] pImageCodecInfo;
  return -1; // Failure
}

////////////////////////////////////////////////////////////////////////////////
