////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "dxerr.h"
#include "regexpr2.h"
#include "GeometryTypes.h"
#include "DXLogger.h"
#include "DXTraceManagerHeaders.h"
#include "DXInterceptorStub.h"
#include "DXCreate9InterceptorStub.h"
#include "DXProfiler.h"
#include "DXPainter.h"

using namespace std;
using namespace regex;
using namespace dxpainter;
using namespace dxtraceman;

////////////////////////////////////////////////////////////////////////////////
// Save readed method calls names in LOG (to debug purposes only when the player
// crashes in the middle of a reproduction session).

//#define WRITE_METHOD_CALLS_TO_LOG

////////////////////////////////////////////////////////////////////////////////

DXPainter::DXPainter() :
m_initialized(false),
m_isWindowed(false),
m_playNextStep(false),
m_state(PS_Stoped),
m_callCurrent(0),
m_callTotal(0),
m_frameCurrent(0),
m_frameTotal(0),
m_logger(NULL),
m_device(NULL),
m_traceman(NULL),
m_internalD3D(NULL),
m_internalDEV(NULL)
{
  m_logger = new DXLogger("dxpainter.log");
  m_profiler = new DXProfiler(GetCurrentProcess());
  m_traceman = new DXTraceManager;
}

////////////////////////////////////////////////////////////////////////////////

DXPainter::~DXPainter()
{
  Close();

  if (m_logger)
  {
    delete m_logger;
    m_logger = NULL;
  }

  if (m_profiler)
  {
    delete m_profiler;
    m_profiler = NULL;
  }
  
  if (m_traceman)
  {
    delete m_traceman;
    m_traceman = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPainter::Close()
{
  if (m_measurePerformace)
  {
    m_profiler->SaveResultsCSV(m_params.MeasurePerformanceFilename, m_params.MeasureFrameRangeMin, m_params.MeasureFrameRangeMax);
  }
  
  m_initialized = false;
  m_isWindowed = false;
  m_measurePerformace = false;
  
  m_playNextStep = false;
  m_state = PS_Stoped;
  
  m_callCurrent = 0;
  m_callTotal = 0;
  m_frameCurrent = 0;
  m_frameTotal = 0;
  
  m_profiler->Reset();
  m_traceman->Close();
  
  DeleteStubsPending();
  DeleteStubs();
  
  m_device = NULL;
  DestroyInternalDirect3DDevice();
}

////////////////////////////////////////////////////////////////////////////////

HWND DXPainter::GetViewportHWND() const
{
#ifdef _DEBUG
  if (!m_initialized)
  {
    return NULL;
  }
#endif // ifdef _DEBUG

  if (m_isWindowed)
  {
    return m_params.ViewportWindowed;
  }
  else
  {
    return m_params.ViewportFullScreen;
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPainter::SetViewportDimensions(UINT width, UINT height)
{
  // Cal llançar un event per avisar que volem canviar les dimensions de la
  // pantalla. També hem de llançar un event quan vulguem passar a FullScreen
  // perque la traça es va guardar originalment en aquest format. Serà el player
  // qui pregunti a l'usuari que vol fer i nosaltres actuarem en conseqüència.
}

////////////////////////////////////////////////////////////////////////////////

void DXPainter::SetDevice(IDirect3DDevice9* device)
{
  m_device = device;
}

////////////////////////////////////////////////////////////////////////////////

void DXPainter::CorrectDevicePresentationParameters(D3DPRESENT_PARAMETERS* presentationParameters)
{
  if (presentationParameters)
  {
    if (m_isWindowed)
    {
      presentationParameters->hDeviceWindow = GetViewportHWND();
      presentationParameters->Windowed = TRUE;

      if (presentationParameters->BackBufferWidth && presentationParameters->BackBufferHeight)
      {
        SetViewportDimensions(presentationParameters->BackBufferWidth, presentationParameters->BackBufferHeight);
      }

      presentationParameters->FullScreen_RefreshRateInHz = 0;
      
      if (m_params.EnableVSync)
      {
        presentationParameters->Flags = 0;
        presentationParameters->PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
      }
      else
      {
        presentationParameters->Flags = D3DPRESENTFLAG_DEVICECLIP;
        presentationParameters->PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
      }
    }
    else
    {
      presentationParameters->hDeviceWindow = GetViewportHWND();
      presentationParameters->Windowed = FALSE;
      
      if (m_params.EnableVSync)
      {
        presentationParameters->FullScreen_RefreshRateInHz = 0;
        presentationParameters->Flags = 0;
        presentationParameters->PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPainter::CorrectDeviceFlags(DXFlagsD3DCREATE* flags)
{
  if (flags)
  {
    *flags &= ~D3DCREATE_MULTITHREADED;
    
    if (m_isWindowed)
    {
      *flags &= ~D3DCREATE_PUREDEVICE;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPainter::UpdateFrameStatistics()
{
  if (m_measurePerformace)
  {
    m_profiler->UpdateCounters();
  }
}

////////////////////////////////////////////////////////////////////////////////

std::string DXPainter::GetDXTraceManagerErrorMessage()
{
  return m_traceman->GetCurrentErrorMessage();
}

////////////////////////////////////////////////////////////////////////////////

bool DXPainter::GetBuffer(DXBufferPtr* buffer, unsigned int buffer_id)
{
#ifdef _DEBUG
  if (!m_initialized)
  {
    return false;
  }
#endif // ifdef _DEBUG

  return m_traceman->ReadBuffer(buffer, buffer_id);
}

////////////////////////////////////////////////////////////////////////////////

bool DXPainter::GetTexture(DXTexturePtr* buffer, unsigned int texture_id)
{
#ifdef _DEBUG
  if (!m_initialized)
  {
    return false;
  }
#endif // ifdef _DEBUG

  return m_traceman->ReadTexture(buffer, texture_id);
}

////////////////////////////////////////////////////////////////////////////////

void DXPainter::ShowError(const char* fmt, ...)
{
  va_list ap;
  static char buffer1[1024];
  static char buffer2[1024];

  _snprintf(buffer1, sizeof(buffer1)-1, "%s", fmt);
  va_start(ap, fmt);
  _vsnprintf(buffer2, sizeof(buffer2)-1, buffer1, ap);
  va_end(ap);

  if (m_isWindowed)
  {
    ::MessageBox(m_params.ViewportWindowed, buffer2, "Error", MB_OK | MB_ICONERROR);
  }
  else
  {
    ::MessageBox(m_params.ViewportFullScreen, buffer2, "Error", MB_OK | MB_ICONERROR);
  }
  m_logger->Write(buffer2);
  m_logger->Write("--------------------------------------------------------");
}

////////////////////////////////////////////////////////////////////////////////

void DXPainter::WriteLog(const char* fmt, ...)
{
  va_list ap;
  static char buffer1[1024];
  static char buffer2[1024];

  _snprintf(buffer1, sizeof(buffer1)-1, "%s", fmt);
  va_start(ap, fmt);
  _vsnprintf(buffer2, sizeof(buffer2)-1, buffer1, ap);
  va_end(ap);
  
  m_logger->Write(buffer2);
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXPainter::GetCurrentCallCount() const
{
#ifdef _DEBUG
  if (!m_initialized)
  {
    return 0;
  }
#endif // ifdef _DEBUG

  return m_callCurrent;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXPainter::GetTotalCallCount() const
{
#ifdef _DEBUG
  if (!m_initialized)
  {
    return 0;
  }
#endif // ifdef _DEBUG

  return m_callTotal;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXPainter::GetCurrentFrameCount() const
{
#ifdef _DEBUG
  if (!m_initialized)
  {
    return 0;
  }
#endif // ifdef _DEBUG

  return m_frameCurrent;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXPainter::GetTotalFrameCount() const
{
#ifdef _DEBUG
  if (!m_initialized)
  {
    return 0;
  }
#endif // ifdef _DEBUG

  return m_frameTotal;
}

////////////////////////////////////////////////////////////////////////////////

string DXPainter::GetProjectGameName()
{
#ifdef _DEBUG
  if (!m_initialized)
  {
    return "";
  }
#endif // ifdef _DEBUG

  return m_traceman->GetProjectGameName();
}

////////////////////////////////////////////////////////////////////////////////

string DXPainter::GetProjectFilePath()
{
#ifdef _DEBUG
  if (!m_initialized)
  {
    return "";
  }
#endif // ifdef _DEBUG

  return m_traceman->GetProjectFileName();
}

////////////////////////////////////////////////////////////////////////////////

const DXTraceManager& DXPainter::GetTraceManager() const
{
  return *m_traceman;
}

////////////////////////////////////////////////////////////////////////////////

bool DXPainter::UpdateProjectInformation(const std::string& projectName, const std::string& projectAnotations)
{
  string projectFilename = m_traceman->GetProjectFileName();
  InitParameters params = m_params;
  Close();
  bool result = DXTraceManager::UpdateProjectInformation(projectFilename, projectName, projectAnotations);
  Init(&params);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

DWORD DXPainter::AddStub(DXInterceptorStub* stub)
{
  m_stubs.push_back(stub);
  return (DWORD) m_stubs.size() - 1;
}

////////////////////////////////////////////////////////////////////////////////

void DXPainter::RemoveStub(DXInterceptorStub* stub)
{
  if (stub->GetStubID() < m_stubs.size())
  {
    m_stubsPendingToDelete.push_back(stub);
    m_stubs[stub->GetStubID()] = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////

DXInterceptorStub* DXPainter::GetStub(DWORD stubID) const
{
#ifdef _DEBUG
  if (stubID >= m_stubs.size())
  {
    return NULL;
  }
#endif // ifdef _DEBUG

  return m_stubs[stubID];
}

////////////////////////////////////////////////////////////////////////////////

void DXPainter::DeleteStubs()
{
  if(!m_stubs.size())
  {
    return;
  }
  
  for (std::vector<DXInterceptorStub*>::reverse_iterator it=m_stubs.rbegin(); it != m_stubs.rend(); ++it)
  {
    if (*it)
    {
      try
      {
        delete *it;
        *it = NULL;
      }
      catch (unsigned)
      {
      }
    }
  }
  m_stubs.clear();
}

////////////////////////////////////////////////////////////////////////////////

void DXPainter::DeleteStubsPending()
{
  if (!m_stubsPendingToDelete.size())
  {
    return;
  }

  for (std::vector<DXInterceptorStub*>::iterator it=m_stubsPendingToDelete.begin(); it != m_stubsPendingToDelete.end(); ++it)
  {
    if (*it)
    {
      try
      {
        delete *it;
        *it = NULL;
      }
      catch (unsigned)
      {
      }
    }
  }
  m_stubsPendingToDelete.clear();
}

////////////////////////////////////////////////////////////////////////////////

bool DXPainter::TakeDeviceScreenshot(const string& filename, IDirect3DDevice9* device, DXPainter::ScreenshotFormat format)
{
#ifdef _DEBUG
  if (!m_initialized)
  {
    return false;
  }
#endif // ifdef _DEBUG
  
  if (filename.empty())
  {
    return false;
  }
  
  D3DDEVICE_CREATION_PARAMETERS dcp;
  dcp.AdapterOrdinal = D3DADAPTER_DEFAULT;
  if (FAILED(device->GetCreationParameters(&dcp)))
  {
    return false;
  }

  RECT rectCaptura;
  ::GetWindowRect(dcp.hFocusWindow, &rectCaptura);
  
  if (m_isWindowed)
  {
    RECT rectDesktop;
    ::GetWindowRect(::GetDesktopWindow(), &rectDesktop);

    // If no portion of the viewport is inside de desktop, abort the capture
    if (rectCaptura.right <= rectDesktop.left ||
        rectCaptura.left >= rectDesktop.right ||
        rectCaptura.bottom <= rectDesktop.top ||
        rectCaptura.top >= rectDesktop.bottom)
    {
      return false;
    }

    if (rectCaptura.left < 0) rectCaptura.left = 0;
    if (rectCaptura.right > rectDesktop.right) rectCaptura.right = rectDesktop.right;
    if (rectCaptura.top < 0) rectCaptura.top = 0;
    if (rectCaptura.bottom > rectDesktop.bottom) rectCaptura.bottom = rectDesktop.bottom;
  }
  
  D3DDISPLAYMODE dm;
  dm.Width = 0;
  dm.Height = 0;

  IDirect3D9* d3d;
  if (SUCCEEDED(device->GetDirect3D(&d3d)) && d3d)
  {
    d3d->GetAdapterDisplayMode(dcp.AdapterOrdinal, &dm);
    d3d->Release();
    d3d = NULL;
  }
  else
  {
    return false;
  }
  
  IDirect3DSurface9* surface;
  if (FAILED(device->CreateOffscreenPlainSurface(dm.Width, dm.Height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &surface, NULL)))
  {
    return false;
  }

  if (FAILED(device->GetFrontBufferData(0, surface)))
  {
    surface->Release();
    surface = NULL;
    return false;
  }

  //////////////////////////////////////////////////////////////////////////////
  // Remove de alpha value in all the pixels of the screenshot
  //////////////////////////////////////////////////////////////////////////////

  D3DSURFACE_DESC surfaceDesc;
  if (FAILED(surface->GetDesc(&surfaceDesc)))
  {
    surface->Release();
    surface = NULL;
    return false;
  }

  D3DLOCKED_RECT lockRect;
  if (FAILED(surface->LockRect(&lockRect, NULL, D3DLOCK_READONLY)))
  {
    surface->Release();
    surface = NULL;
    return false;
  }

  for (UINT y=0; y < surfaceDesc.Height; y++)
  {
    LPDWORD lpSrc = reinterpret_cast<LPDWORD>(lockRect.pBits) + y * surfaceDesc.Width;
    for (UINT x=0; x < surfaceDesc.Width; x++, lpSrc++)
    {
      *lpSrc |= 0xFF000000;
    }
  }

  surface->UnlockRect();

  //////////////////////////////////////////////////////////////////////////////

  D3DXIMAGE_FILEFORMAT ss_format;
  
  switch (format)
  {
  default:
  case SSF_BMP:
    ss_format = D3DXIFF_BMP;
    break;
  case SSF_JPG:
    ss_format = D3DXIFF_JPG;
    break;
  case SSF_TGA:
    ss_format = D3DXIFF_TGA;
    break;
  case SSF_PNG:
    ss_format = D3DXIFF_PNG;
    break;
  case SSF_DDS:
    ss_format = D3DXIFF_DDS;
    break;
  case SSF_PPM:
    ss_format = D3DXIFF_PPM;
    break;
  case SSF_DIB:
    ss_format = D3DXIFF_DIB;
    break;
  case SSF_HDR:
    ss_format = D3DXIFF_HDR;
    break;
  case SSF_PFM:
    ss_format = D3DXIFF_PFM;
    break;
  }
  
  bool problems = FAILED(D3DXSaveSurfaceToFile(filename.c_str(), ss_format, surface, NULL, &rectCaptura));

  //////////////////////////////////////////////////////////////////////////////

  surface->Release();
  surface = NULL;
  
  return !problems;
}

////////////////////////////////////////////////////////////////////////////////

bool DXPainter::Init(const InitParameters* params)
{
  if (params->ViewportWindowed == NULL)
  {
    ::MessageBox(NULL, "Could'nt initialize in windowed mode without a window handle.", "DXPainter Error", MB_OK | MB_ICONWARNING);
    return false;
  }
  
  if (!params->StartWindowed && params->ViewportFullScreen == NULL)
  {
    ::MessageBox(NULL, "Could'nt initialize in fullscreen mode without a window handle.", "DXPainter Error", MB_OK | MB_ICONWARNING);
    return false;
  }
  
  Close();
  
  if (!m_traceman->OpenRead(params->TraceFilename))
  {    
    string miss = "ERROR\n\nCould'nt open '" + params->TraceFilename + "'\nDXTraceManager Error = '" + m_traceman->GetCurrentErrorMessage() + "'";
    ::MessageBox(params->ViewportWindowed, miss.c_str(), "Error", MB_OK | MB_ICONERROR);
    return false;
  }
  else
  {
    m_stubs.reserve(10000);
    m_stubsPendingToDelete.reserve(10);
    
    new DXCreate9InterceptorStub(this);

    m_params = *params;
    
    m_initialized = true;
    m_isWindowed = params->StartWindowed;
    m_measurePerformace = params->MeasurePerformance;

    CreateInternalDirect3DDevice();
    
    m_playNextStep = false;
    m_state = PS_Stoped;

    m_callCurrent = 0;
    m_callTotal = m_traceman->GetMethodCallCount();
    m_frameCurrent = 0;    
    m_frameTotal = m_traceman->GetFrameCount();
    
    return true;
  }
}

////////////////////////////////////////////////////////////////////////////////

DXPainter::PlayResult DXPainter::Paint()
{
  switch (m_state)
  {
  case PS_Playing:
    return PlayNextFrame();
    break;
  
  case PS_Paused:
    return PR_OK_PAUSE;
    break;
  
  case PS_Stoped:
    return PR_OK_STOP;
    break;
  
  case PS_PlayingStepPerFrame:
    if (m_playNextStep)
    {
      m_playNextStep = false;
      return PlayNextFrame();
    }
    else return PR_OK_PLAYSTEPPERFRAME;
    break;
  }

  return PR_ERROR_PAINTER;
}

////////////////////////////////////////////////////////////////////////////////

DXPainter::PlayResult DXPainter::PlayNextFrame()
{
  PlayResult result;
  
  do 
  {
    result = PlayNextCall();
  } while (result == PR_OK_PLAY);

  if (result == PR_OK_ENDFRAME)
  {
    result = PR_OK_PLAY;
  }
  
  return result;
}

////////////////////////////////////////////////////////////////////////////////

DXPainter::PlayResult DXPainter::PlayNextCall()
{
#ifdef _DEBUG
  if (!m_initialized)
  {
    return PR_ERROR_PAINTER;
  }
#endif // ifdef _DEBUG
  
  PlayResult result = PR_OK_PLAY;
  DXMethodCallPtr call;

  if (m_callCurrent < m_callTotal)
  {
    if (!m_traceman->ReadMethodCall(&call, m_callCurrent))
    {
      ShowError("ERROR\n\nDXTraceManager::ReadMethodCall(%u)\nDXTraceManager Error = '%s'", m_callCurrent, m_traceman->GetCurrentErrorMessage().c_str());
      return PR_ERROR_TRACEMANAGER;
    }
    else
    {
      m_callCurrent++;
      if (call->GetToken() == DXMethodCallHelper::TOK_IDirect3DDevice9_Present ||
          call->GetToken() == DXMethodCallHelper::TOK_IDirect3DSwapChain9_Present)
      {
        m_frameCurrent++;
        result = PR_OK_ENDFRAME;
      }
    }
  }
  else
  {
    return PR_OK_NOMOREFRAMES;
  }

  DXInterceptorStub* stub;
  if (!(stub = GetStub(call->GetCreatorID())))
  {
    std::string callStr;
    call->SerializeToString(callStr);
    ShowError("ERROR\n\nDXTraceManager::ReadMethodCall(%u)\nCall: '%s'\nDXPainter Error = 'creatorID %u not exists'", m_callCurrent, callStr.c_str(), call->GetCreatorID());
    return PR_ERROR_PAINTER;
  }

#ifdef WRITE_METHOD_CALLS_TO_LOG
  std::string callStr;
  call->SerializeToString(callStr);
  WriteLog("%s", callStr.c_str());
#endif

  HRESULT hr = stub->HandleCall(call);
  if (FAILED(hr))
  {
    std::string callStr;
    call->SerializeToString(callStr);
    if (hr != E_FAIL)
    {
      ShowError("ERROR\n\nDXTraceManager::ReadMethodCall(%u)\nCall: '%s'\nDirect3D Error = '%s'", m_callCurrent, callStr.c_str(), DXGetErrorString(hr));
      return PR_ERROR_DIRECT3D;
    }
    else
    {
      ShowError("ERROR\n\nDXTraceManager::ReadMethodCall(%u)\nCall: '%s'\nDXTraceManager Error = '%s'", m_callCurrent, callStr.c_str(), m_traceman->GetCurrentErrorMessage().c_str());
      return PR_ERROR_TRACEMANAGER;
    }
  }

  DeleteStubsPending();

  return result;
}

////////////////////////////////////////////////////////////////////////////////

void DXPainter::Play()
{
  if (m_initialized)
  {
    m_state = PS_Playing;
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPainter::Pause()
{
  if (m_initialized && m_state == PS_Playing)
  {
    m_state = PS_Paused;
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPainter::Stop()
{
  if (m_initialized && m_state != PS_Stoped)
  {
    DeleteStubsPending();
    DeleteStubs();
    
    new DXCreate9InterceptorStub(this);
    
    m_playNextStep = false;
    m_state = PS_Stoped;

    m_callCurrent = 0;
    m_frameCurrent = 0;

    m_device = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPainter::PlayStepPerFrame()
{
  if (m_initialized)
  {
    m_playNextStep = true;
    m_state = PS_PlayingStepPerFrame;
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPainter::TakeScreenshot(const string& basePath, DXPainter::ScreenshotFormat format)
{
  if (m_initialized && m_state != PS_Stoped && m_device != NULL)
  {
    string filename;
    if (GetNextScreenshotFilename(basePath, filename, format))
    {
      filename = basePath + filename;
      TakeDeviceScreenshot(filename, m_device, format);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

bool DXPainter::GetNextScreenshotFilename(const string& basepath, string& filename, DXPainter::ScreenshotFormat format)
{
  HANDLE filehandle;
  WIN32_FIND_DATA fileentry;
  char filepath[MAX_PATH];
  char fileregex[64];
  unsigned int maxnum = 0;
  
  strcpy(filepath, basepath.c_str());
  strcat(filepath, "screenshot*.");
  strcat(filepath, GetScreenshotFormatExtension(format));
  
  strcpy(fileregex, "screenshot (\\d+) \\.");
  strcat(fileregex, GetScreenshotFormatExtension(format));
  rpattern patt_filepath(fileregex, EXTENDED | SINGLELINE);
  
  filehandle = FindFirstFile(filepath, &fileentry);
  
  if (filehandle == INVALID_HANDLE_VALUE)
  {
    if (GetLastError() == 2)
    {
      sprintf(filepath, "screenshot%04u.%s", maxnum, GetScreenshotFormatExtension(format));
      filename = filepath;
      return true;
    }
    else
    {
      return false;
    }
  }

  do
  {
    if (strcmp(fileentry.cFileName, ".") == 0 || strcmp(fileentry.cFileName, "..") == 0)
    {
      continue;
    }
    
    unsigned int curnum = 0;
    string filename = fileentry.cFileName;  //  Fix rpattern bug
    match_results patt_results;
    //match_results::backref_type patt_brtype = patt_filepath.match(fileentry.cFileName, patt_results);
    match_results::backref_type patt_brtype = patt_filepath.match(filename, patt_results);  // Fix rpattern bug
    if (patt_brtype.matched && patt_results.cbackrefs() == 2)
    {
      curnum = atoi(patt_results.backref(1).str().c_str());
    }
    maxnum = max(curnum, maxnum);
  } while (FindNextFile(filehandle, &fileentry) != 0);
  
  FindClose(filehandle);
  
  sprintf(filepath, "screenshot%04u.%s", maxnum+1, GetScreenshotFormatExtension(format));
  filename = filepath;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

const char* DXPainter::GetScreenshotFormatExtension(ScreenshotFormat format)
{
  switch (format)
  {
  default:
  case SSF_BMP:
    return "bmp";
    break;
  case SSF_JPG:
    return "jpg";
    break;
  case SSF_TGA:
    return "tga";
    break;
  case SSF_PNG:
    return "png";
    break;
  case SSF_DDS:
    return "dds";
    break;
  case SSF_PPM:
    return "ppm";
    break;
  case SSF_DIB:
    return "dib";
    break;
  case SSF_HDR:
    return "hdr";
    break;
  case SSF_PFM:
    return "pfm";
    break;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////

void DXPainter::CreateInternalDirect3DDevice()
{
  m_internalD3D = Direct3DCreate9(D3D_SDK_VERSION);
  
  if (m_internalD3D != NULL)
  {
    D3DPRESENT_PARAMETERS pparams;
    ZeroMemory(&pparams, sizeof(pparams));

    pparams.Windowed = TRUE;
    pparams.BackBufferWidth = 640;
    pparams.BackBufferHeight = 480;
    pparams.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pparams.BackBufferFormat = D3DFMT_A8R8G8B8;
    pparams.BackBufferCount = 1;

    if (FAILED(m_internalD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_params.ViewportWindowed, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pparams, &m_internalDEV)))
    {
      m_internalDEV = NULL;
      m_internalD3D->Release();
      m_internalD3D = NULL;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPainter::DestroyInternalDirect3DDevice()
{
  if (m_internalDEV)
  {
    m_internalDEV->Release();
    m_internalDEV = NULL;
  }

  if (m_internalD3D)
  {
    m_internalD3D->Release();
    m_internalD3D = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool DXPainter::ConvertTextureToSurface(DXTexturePtr textura, IDirect3DSurface9** surface)
{
  if (!m_internalDEV)
  {
    return false;
  }

  if (FAILED(m_internalDEV->CreateOffscreenPlainSurface(textura->GetWidth(), textura->GetHeight(), textura->GetFormat(), D3DPOOL_DEFAULT, surface, NULL)))
  {
    return false;
  }

  bool problems = false;

  D3DLOCKED_RECT lock;
  if (SUCCEEDED((*surface)->LockRect(&lock, NULL, D3DLOCK_DISCARD)) && lock.pBits)
  {
    unsigned int bytesPerLine = textura->GetWidth();
    unsigned int numLines = textura->GetHeight();

    switch (textura->GetFormat())
    {
    case D3DFMT_DXT1:
    case D3DFMT_DXT2:
    case D3DFMT_DXT3:
    case D3DFMT_DXT4:
    case D3DFMT_DXT5:
      bytesPerLine = lock.Pitch;
      numLines >>= 2;
      if (!numLines) numLines = 1;
      break;
    default:
      bytesPerLine *= DXSurfaceLock::GetBitsPerPixelForFormat(textura->GetFormat()) >> 3;
      break;
    }

    textura->TextureData_SeekRead(0, false);
    if (textura->GetPitch() == lock.Pitch)
    {
      unsigned int readedBytes = numLines * lock.Pitch;
      problems = !textura->TextureData_Read((char*) lock.pBits, &readedBytes);
    }
    else
    {
      unsigned int pitch_add = textura->GetPitch()-bytesPerLine;
      char* lockedRect = (char*) lock.pBits;
      for (unsigned int i=0; i < numLines; i++)
      {
        unsigned int readedBytes = bytesPerLine;
        if (!textura->TextureData_Read(lockedRect, &readedBytes))
        {
          problems = true;
          break;
        }
        lockedRect += lock.Pitch;
        textura->TextureData_SeekRead(pitch_add, true);
      }
    }

    (*surface)->UnlockRect();
  }
  else
  {
    problems = true;
  }

  return !problems;
}

////////////////////////////////////////////////////////////////////////////////

bool DXPainter::SaveDXTextureToStream(DXTexturePtr textura, IStream** ramfile)
{
  *ramfile = NULL;
  
  IDirect3DSurface9* surface = NULL;
  
  bool problems = !ConvertTextureToSurface(textura, &surface);
  
  if (!problems)
  {
    ID3DXBuffer* buffer;
    if (SUCCEEDED(D3DXSaveSurfaceToFileInMemory(&buffer, D3DXIFF_BMP, surface, NULL, NULL)))
    {
      IStream* pIStream = NULL;
      if (SUCCEEDED(CreateStreamOnHGlobal(NULL, TRUE, (LPSTREAM*) &pIStream)))
      {
        ULONG writedBytes = 0;
        LARGE_INTEGER offset = {0};
        
        pIStream->Seek(offset, STREAM_SEEK_SET, NULL);
        pIStream->Write(buffer->GetBufferPointer(), buffer->GetBufferSize(), &writedBytes);
        
        if (buffer->GetBufferSize() == writedBytes)
        {
          *ramfile = pIStream;
        }
        else
        {
          pIStream->Release();
          pIStream = NULL;
          problems = true;
        }
      }
      else
      {
        problems = true;
      }
      
      buffer->Release();
    }
    else
    {
      problems = true;
    }
  }
  
  if (surface)
  {
    surface->Release();
    surface = NULL;
  }
  
  return !problems;
}

////////////////////////////////////////////////////////////////////////////////

bool DXPainter::SaveDXTextureToFile(DXTexturePtr textura, const string& filename, DXPainter::ScreenshotFormat format)
{
  IDirect3DSurface9* surface = NULL;

  bool problems = !ConvertTextureToSurface(textura, &surface);

  if (!problems)
  {
    D3DXIMAGE_FILEFORMAT ss_format;

    switch (format)
    {
    default:
    case SSF_BMP:
      ss_format = D3DXIFF_BMP;
      break;
    case SSF_JPG:
      ss_format = D3DXIFF_JPG;
      break;
    case SSF_TGA:
      ss_format = D3DXIFF_TGA;
      break;
    case SSF_PNG:
      ss_format = D3DXIFF_PNG;
      break;
    case SSF_DDS:
      ss_format = D3DXIFF_DDS;
      break;
    case SSF_PPM:
      ss_format = D3DXIFF_PPM;
      break;
    case SSF_DIB:
      ss_format = D3DXIFF_DIB;
      break;
    case SSF_HDR:
      ss_format = D3DXIFF_HDR;
      break;
    case SSF_PFM:
      ss_format = D3DXIFF_PFM;
      break;
    }

    problems = FAILED(D3DXSaveSurfaceToFile(filename.c_str(), ss_format, surface, NULL, NULL));
  }
  
  if (surface)
  {
    surface->Release();
    surface = NULL;
  }

  return !problems;
}

////////////////////////////////////////////////////////////////////////////////
