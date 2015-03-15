////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXLogger.h"
#include "DXTraceManagerHeaders.h"
#include "DXIntPluginManager.h"
#include "DXInterceptorOptions.h"
#include "main.h"
#include "GeometryTypes.h"
#include "DXInterceptorBanner.h"

////////////////////////////////////////////////////////////////////////////////

const DWORD DXInterceptorBanner::MESSAGE_DURATION_MIN = 2000;
DWORD DXInterceptorBanner::ms_timeElapsed = 0;
DWORD DXInterceptorBanner::ms_frameCount = 0;

////////////////////////////////////////////////////////////////////////////////

struct DXInterceptorBanner::MessageInformation
{
  char Message[256];
  D3DCOLOR Color;
  DWORD InsertionTimeStamp;
  DWORD Duration;
};

////////////////////////////////////////////////////////////////////////////////

DXInterceptorBanner::DXInterceptorBanner(LPDIRECT3DDEVICE9 device) :
m_device(device),
m_texture1(NULL),
m_texture2(NULL),
m_sprite(NULL),
m_font(NULL),
m_initialized(false),
m_traceCompressed(false),
m_traceDiscardByCRC(false),
m_modeComplete(true),
m_deviceReferenceCount(0)
{
  InitCounters();
}

////////////////////////////////////////////////////////////////////////////////

DXInterceptorBanner::~DXInterceptorBanner()
{
  m_messages.clear();
}

////////////////////////////////////////////////////////////////////////////////

bool DXInterceptorBanner::Close()
{
  if (!m_initialized)
  {
    return false;
  }
  
  ResetCounters();
  
  m_initialized = false;
  m_traceCompressed = false;
  m_traceDiscardByCRC = false;
  m_modeComplete = true;
  m_deviceReferenceCount = 0;
  
  if (m_texture1)
  {
    m_texture1->Release();
    m_texture1 = NULL;
  }

  if (m_texture2)
  {
    m_texture2->Release();
    m_texture2 = NULL;
  }

  if (m_sprite)
  {
    m_sprite->Release();
    m_sprite = NULL;
  }

  if (m_font)
  {
    m_font->Release();
    m_font = NULL;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXInterceptorBanner::Init()
{
  if (m_initialized)
  {
    return false;
  }
  
  m_modeComplete = true;
  
  if (!InitResources())
  {
    return false;
  }
  else
  {
    SetOptions();
    ResetCounters();
    m_initialized = true;
    return true;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool DXInterceptorBanner::InitResources()
{
  m_deviceReferenceCount = 0;

  m_device->AddRef();
  ULONG referencesDeviceBefore = m_device->Release();

  if (m_modeComplete)
  {
    if (FAILED(D3DXCreateTextureFromResourceEx(m_device, gDllHandle, MAKEINTRESOURCE(IDB_DXLOGO1), D3DX_DEFAULT, D3DX_DEFAULT, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, D3DCOLOR_RGBA(255,0,255,255), NULL, NULL, &m_texture1)))
    {
      return false;
    }

    if (FAILED(D3DXCreateTextureFromResourceEx(m_device, gDllHandle, MAKEINTRESOURCE(IDB_DXLOGO2), D3DX_DEFAULT, D3DX_DEFAULT, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, D3DCOLOR_RGBA(255,0,255,255), NULL, NULL, &m_texture2)))
    {
      if (m_texture1)
      {
        m_texture1->Release();
        m_texture1 = NULL;
      }
      m_texture2 = NULL;
      m_sprite = NULL;
      m_font = NULL;
      m_deviceReferenceCount = 0;
      return false;
    }

    if (FAILED(D3DXCreateSprite(m_device, &m_sprite)))
    {
      if (m_texture1)
      {
        m_texture1->Release();
        m_texture1 = NULL;
      }
      if (m_texture2)
      {
        m_texture2->Release();
        m_texture2 = NULL;
      }
      m_sprite = NULL;
      m_font = NULL;
      m_deviceReferenceCount = 0;
      return false;
    }
  }

  if (FAILED(D3DXCreateFont(m_device, 14, 0, FW_BOLD, 0, 0, ANSI_CHARSET, OUT_TT_ONLY_PRECIS, 0, 0, "System", &m_font)))
  {
    if (m_texture1)
    {
      m_texture1->Release();
      m_texture1 = NULL;
    }
    if (m_texture2)
    {
      m_texture2->Release();
      m_texture2 = NULL;
    }
    if (m_sprite)
    {
      m_sprite->Release();
      m_sprite = NULL;
    }
    m_font = NULL;
    m_deviceReferenceCount = 0;
    return false;
  }
  else
  {
    m_font->PreloadCharacters(0, 255);
  }

  m_device->AddRef();
  m_deviceReferenceCount += m_device->Release() - referencesDeviceBefore;
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorBanner::Message(const char* cadena, D3DCOLOR color, unsigned int duration)
{
  MessageInformation msg;
  if (strlen(cadena) >= sizeof(msg.Message))
  {
    strncpy(msg.Message, cadena, sizeof(msg.Message));
    msg.Message[sizeof(msg.Message)-1] = '\0';
  }
  else
  {
    strcpy(msg.Message, cadena);
  }
  msg.Color = color;
  msg.InsertionTimeStamp = timeGetTime();
  msg.Duration = max(duration, MESSAGE_DURATION_MIN);

  m_messages.push_back(msg);
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorBanner::SetViewport(const D3DVIEWPORT9* viewportD3D)
{
  CRect viewport(viewportD3D->X, viewportD3D->Y, viewportD3D->X+viewportD3D->Width, viewportD3D->Y+viewportD3D->Height);
  SetPositions(&viewport);
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorBanner::SetOptions()
{
  IDirect3DSurface9* surface;
  m_device->GetRenderTarget(0, &surface);
  D3DSURFACE_DESC description;
  surface->GetDesc(&description);
  surface->Release();
  CRect viewport(0, 0, description.Width, description.Height);
  
  SetPositions(&viewport);
  SetFunctions();
  
  m_traceCompressed = g_options->GetCompression();
  m_colorCounters = g_options->GetBannerTextColor();
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorBanner::SetPositions(const CRect* viewport)
{
  if (m_modeComplete)
  {
    switch (g_options->GetBannerPosition())
    {
    default:
    case DXInterceptorOptions::BP_TopLeft:
      m_positionIcon.x = (float) viewport->left;
      m_positionIcon.y = (float) viewport->top;
      m_positionIcon.z = 0;
      m_positionText.SetRect(viewport->left+32+2, viewport->top, 0, 0);
      break;

    case DXInterceptorOptions::BP_TopRight:
      m_positionIcon.x = (float) viewport->right - 32;
      m_positionIcon.y = (float) viewport->top;
      m_positionIcon.z = 0;
      m_positionText.SetRect(0, viewport->top, viewport->right-32-2, 0);
      break;

    case DXInterceptorOptions::BP_BottomLeft:
      m_positionIcon.x = (float) viewport->left;
      m_positionIcon.y = (float) viewport->bottom - 32;
      m_positionIcon.z = 0;
      m_positionText.SetRect(viewport->left+1+32+2, viewport->bottom, 0, 0);
      break;

    case DXInterceptorOptions::BP_BottomRight:
      m_positionIcon.x = (float) viewport->right - 32;
      m_positionIcon.y = (float) viewport->bottom - 32;
      m_positionIcon.z = 0;
      m_positionText.SetRect(0, viewport->bottom, viewport->right-32-2, 0);
      break;
    }
  }
  else
  {
    switch (g_options->GetBannerPosition())
    {
    default:
    case DXInterceptorOptions::BP_TopLeft:
      m_positionText.SetRect(viewport->left+1, viewport->top+1, 0, 0);
      break;

    case DXInterceptorOptions::BP_TopRight:
      m_positionText.SetRect(0, viewport->top+1, viewport->right-1, 0);
      break;

    case DXInterceptorOptions::BP_BottomLeft:
      m_positionText.SetRect(viewport->left+1, viewport->bottom-1, 0, 0);
      break;

    case DXInterceptorOptions::BP_BottomRight:
      m_positionText.SetRect(0, viewport->bottom-1, viewport->right-1, 0);
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorBanner::SetFunctions()
{
  if (m_modeComplete)
  {
    DrawFunction = &DXInterceptorBanner::DrawComplete;
  }
  else
  {
    DrawFunction = &DXInterceptorBanner::DrawSimple;
  }

  switch (g_options->GetBannerPosition())
  {
  default:
  case DXInterceptorOptions::BP_TopLeft:
    DrawTextFunction = &DXInterceptorBanner::DrawText_TopLeft;
    break;

  case DXInterceptorOptions::BP_TopRight:
    DrawTextFunction = &DXInterceptorBanner::DrawText_TopRight;
    break;

  case DXInterceptorOptions::BP_BottomLeft:
    DrawTextFunction = &DXInterceptorBanner::DrawText_BottomLeft;
    break;

  case DXInterceptorOptions::BP_BottomRight:
    DrawTextFunction = &DXInterceptorBanner::DrawText_BottomRight;
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXInterceptorBanner::GetDeviceReferenceCount() const
{
  return m_deviceReferenceCount;
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorBanner::DrawSimple()
{
#ifdef _DEBUG
  if (!m_initialized)
  {
    return;
  }
#endif // ifdef _DEBUG

  m_positionTextWork = m_positionText;
  
  m_device->AddRef();
  ULONG referencesDeviceBefore = m_device->Release();
  
  DrawTextCountersSimple();

  m_device->AddRef();
  m_deviceReferenceCount += m_device->Release() - referencesDeviceBefore;
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorBanner::DrawComplete()
{
#ifdef _DEBUG
  if (!m_initialized)
  {
    return;
  }
#endif // ifdef _DEBUG
  
  m_positionTextWork = m_positionText;
  
  m_device->AddRef();
  ULONG referencesDeviceBefore = m_device->Release();
  
  UpdateCounters();
  DrawDirectXIcon();
  DrawTextCountersComplete();
  DrawTextMessages();

  m_device->AddRef();
  m_deviceReferenceCount += m_device->Release() - referencesDeviceBefore;
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorBanner::DrawDirectXIcon()
{
  m_sprite->Begin(D3DXSPRITE_ALPHABLEND);	
  m_sprite->Draw((ms_timeElapsed % 1000) >= 500 ? m_texture2 : m_texture1, NULL, NULL, &m_positionIcon, D3DCOLOR_RGBA(255, 255, 255, 255));
  m_sprite->End();
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorBanner::DrawTextCountersSimple()
{
  static char cadena[512];
  sprintf(cadena, "[DXInterceptor] Frame %u", ++ms_frameCount);

  DrawText(cadena, m_colorCounters);
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorBanner::DrawTextCountersComplete()
{
  static char cadena[512];
  
  if (m_traceCompressed)
  {
    sprintf(cadena, "FPS %u   Frame %u   Time %.1f sec   File %.2f MB (%sLZO %.1f%%)", m_FPS, g_traceman->GetFrameCount()+1, ms_timeElapsed / 1000.0f, g_traceman->GetFileSizeInDisk() / (1024.0f*1024.0f), (m_traceDiscardByCRC ? "CRC+" : ""), (g_traceman->GetFileSize() ? g_traceman->GetFileSizeInDisk() * 100.0f / g_traceman->GetFileSize() : 0.0f));
  }
  else
  {
    sprintf(cadena, "FPS %u   Frame %u   Time %.1f sec   File %.2f MB (%sRAW)", m_FPS, g_traceman->GetFrameCount()+1, ms_timeElapsed / 1000.0f, g_traceman->GetFileSizeInDisk() / (1024.0f*1024.0f), (m_traceDiscardByCRC ? "CRC+" : ""));
  }

  DrawText(cadena, m_colorCounters);
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorBanner::DrawTextMessages()
{
  if (!m_messages.size())
  {
    return;
  }
  
  for (std::vector<MessageInformation>::iterator it=m_messages.begin(); it != m_messages.end();)
  {
    DWORD elapsedTime = timeGetTime() - (*it).InsertionTimeStamp;
    
    if (elapsedTime >= (*it).Duration)
    {
      it = m_messages.erase(it);
    }
    else
    {
      D3DCOLOR color = (*it).Color;
      DWORD pendingTime = (*it).Duration - elapsedTime;
      if (pendingTime <= MESSAGE_DURATION_MIN)
      {
        BYTE alpha = (BYTE) (((color >> 24) & 0xFF) * pendingTime / MESSAGE_DURATION_MIN);
        BYTE red   = (BYTE) ((color >> 16) & 0xFF);
        BYTE green = (BYTE) ((color >>  8) & 0xFF);
        BYTE blue  = (BYTE) ((color >>  0) & 0xFF);
        color = D3DCOLOR_ARGB(alpha, red, green, blue);
      }
      DrawText((*it).Message, color);
      it++;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorBanner::DrawText_TopLeft(const char* cadena, D3DCOLOR color)
{
  CRect rect = m_positionTextWork;
  
  m_font->DrawTextA(NULL, cadena, -1, rect, DT_LEFT | DT_WORDBREAK | DT_NOCLIP | DT_CALCRECT, 0); 
  m_font->DrawTextA(NULL, cadena, -1, rect, DT_LEFT | DT_WORDBREAK | DT_NOCLIP, color);

  m_positionTextWork.top = rect.bottom;
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorBanner::DrawText_TopRight(const char* cadena, D3DCOLOR color)
{
  CRect rect = m_positionTextWork;
  
  m_font->DrawTextA(NULL, cadena, -1, rect, DT_RIGHT | DT_WORDBREAK | DT_NOCLIP | DT_CALCRECT, 0);
  m_font->DrawTextA(NULL, cadena, -1, rect, DT_RIGHT | DT_WORDBREAK | DT_NOCLIP, color);

  m_positionTextWork.top = rect.bottom;
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorBanner::DrawText_BottomLeft(const char* cadena, D3DCOLOR color)
{
  CRect rect = m_positionTextWork;
  
  m_font->DrawTextA(NULL, cadena, -1, rect, DT_LEFT | DT_WORDBREAK | DT_NOCLIP | DT_CALCRECT, 0);
  
  int height = rect.Height();
  rect.top -= height;
  rect.bottom -= height;
  m_positionTextWork.top = rect.top - 1;
  
  m_font->DrawTextA(NULL, cadena, -1, rect, DT_LEFT | DT_WORDBREAK | DT_NOCLIP, color);
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorBanner::DrawText_BottomRight(const char* cadena, D3DCOLOR color)
{
  CRect rect = m_positionTextWork;

  m_font->DrawTextA(NULL, cadena, -1, rect, DT_RIGHT | DT_WORDBREAK | DT_NOCLIP | DT_CALCRECT, 0);
  
  int height = rect.Height();
  rect.top -= height;
  rect.bottom -= height;
  m_positionTextWork.top = rect.top - 1;
  
  m_font->DrawTextA(NULL, cadena, -1, rect, DT_RIGHT | DT_WORDBREAK | DT_NOCLIP, color);
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorBanner::InitCounters()
{
  ms_timeElapsed = 0;
  ms_frameCount = 0;
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorBanner::ResetCounters()
{
  m_FPS = 0;
  m_lastTimeStamp = timeGetTime();
  m_numFramesFromLastSecond = 0;
  m_timeElapsedFromLastSecond = 0;
}

////////////////////////////////////////////////////////////////////////////////

void DXInterceptorBanner::UpdateCounters()
{
  m_numFramesFromLastSecond++;
  
  DWORD currentTimeStamp = timeGetTime();
  DWORD timeElapsed = currentTimeStamp - m_lastTimeStamp;
  m_lastTimeStamp = currentTimeStamp;
  ms_timeElapsed += timeElapsed;
  m_timeElapsedFromLastSecond += timeElapsed;
  
  if (m_timeElapsedFromLastSecond >= 1000)
  {
    if (m_numFramesFromLastSecond)
    {
      m_FPS = m_numFramesFromLastSecond * 1000 / m_timeElapsedFromLastSecond;
    }
    else
    {
      m_FPS = 0;
    }
    
    m_numFramesFromLastSecond = 0;
    m_timeElapsedFromLastSecond = 0;
  }
}

////////////////////////////////////////////////////////////////////////////////
