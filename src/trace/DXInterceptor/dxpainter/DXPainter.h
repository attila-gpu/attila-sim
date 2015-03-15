////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#ifdef DLL_EXPORTS_ENABLED
#define EXPIMP_DLL __declspec(dllexport)
#else
#define EXPIMP_DLL __declspec(dllimport)
#endif

////////////////////////////////////////////////////////////////////////////////

class CRect;
class DXLogger;
class DXProfiler;
class DXInterceptorStub;
dxtraceman::DXBufferPtr;
dxtraceman::DXTexturePtr;
class dxtraceman::DXTraceManager;

////////////////////////////////////////////////////////////////////////////////

namespace dxpainter
{
  class EXPIMP_DLL DXPainter
  {
  public:

    struct EXPIMP_DLL InitParameters
    {
      std::string TraceFilename;
      std::string MeasurePerformanceFilename;
      HWND ViewportWindowed;
      HWND ViewportFullScreen;
      bool StartWindowed;
      bool MeasurePerformance;
      bool EnableVSync;
      unsigned int MeasureFrameRangeMin;
      unsigned int MeasureFrameRangeMax;

      InitParameters()
      {
        TraceFilename = "";
        MeasurePerformanceFilename = "";
        ViewportWindowed = NULL;
        ViewportFullScreen = NULL;
        StartWindowed = true;
        MeasurePerformance = false;
        EnableVSync = false;
        MeasureFrameRangeMin = 0;
        MeasureFrameRangeMax = 0;
      }
    };

    enum PlayResult
    {
      PR_OK_PLAY = 1,
      PR_OK_PAUSE,
      PR_OK_STOP,
      PR_OK_PLAYSTEPPERFRAME,
      PR_OK_NOMOREFRAMES,
      PR_ERROR_DIRECT3D,
      PR_ERROR_PAINTER,
      PR_ERROR_TRACEMANAGER,
      PR_OK_ENDFRAME = 51 // Used Internaly
    };

    enum ScreenshotFormat
    {
      SSF_BMP,
      SSF_JPG,
      SSF_TGA,
      SSF_PNG,
      SSF_DDS,
      SSF_PPM,
      SSF_DIB,
      SSF_HDR,
      SSF_PFM
    };
    
    DXPainter();
    virtual ~DXPainter();

    bool Init(const InitParameters* params);
    void Close();
    PlayResult Paint();
    
    void Play();
    void Pause();
    void Stop();
    void PlayStepPerFrame();
    void TakeScreenshot(const std::string& basePath, ScreenshotFormat format);
    
    HWND GetViewportHWND() const;
    void SetViewportDimensions(UINT width, UINT height);
    void SetDevice(IDirect3DDevice9* device);
    void CorrectDevicePresentationParameters(D3DPRESENT_PARAMETERS* presentationParameters);
    void CorrectDeviceFlags(DXFlagsD3DCREATE* flags);
    void UpdateFrameStatistics();
    std::string GetDXTraceManagerErrorMessage();

    bool GetBuffer(dxtraceman::DXBufferPtr* buffer, unsigned int buffer_id);
    bool GetTexture(dxtraceman::DXTexturePtr* buffer, unsigned int texture_id);
    
    DWORD AddStub(DXInterceptorStub* stub);
    void RemoveStub(DXInterceptorStub* stub);
    DXInterceptorStub* GetStub(DWORD stubID) const;
    void DeleteStubs();
    void DeleteStubsPending();
    
    void ShowError(const char* fmt, ...);
    void WriteLog(const char* fmt, ...);

    unsigned int GetCurrentCallCount() const;
    unsigned int GetTotalCallCount() const;
    unsigned int GetCurrentFrameCount() const;
    unsigned int GetTotalFrameCount() const;

    std::string GetProjectGameName();
    std::string GetProjectFilePath();
    const dxtraceman::DXTraceManager& GetTraceManager() const;

    bool UpdateProjectInformation(const std::string& projectName, const std::string& projectAnotations);
    
    bool SaveDXTextureToStream(dxtraceman::DXTexturePtr textura, IStream** ramfile);
    bool SaveDXTextureToFile(dxtraceman::DXTexturePtr textura, const std::string& filename, ScreenshotFormat format);

  protected:
    
    template class EXPIMP_DLL std::allocator<DXInterceptorStub*>;
    template class EXPIMP_DLL std::vector<DXInterceptorStub*>;
    
    enum PainterState
    {
      PS_Playing = 1,
      PS_Paused,
      PS_Stoped,
      PS_PlayingStepPerFrame
    };
    
    bool m_initialized;
    bool m_isWindowed;
    bool m_measurePerformace;
    bool m_playNextStep;
    PainterState m_state;
    unsigned int m_callCurrent;
    unsigned int m_callTotal;
    unsigned int m_frameCurrent;
    unsigned int m_frameTotal;
    InitParameters m_params;

    DXLogger* m_logger;
    DXProfiler* m_profiler;
    IDirect3DDevice9* m_device;
    dxtraceman::DXTraceManager* m_traceman;
    std::vector<DXInterceptorStub*> m_stubs;
    std::vector<DXInterceptorStub*> m_stubsPendingToDelete;

    IDirect3D9* m_internalD3D;
    IDirect3DDevice9* m_internalDEV;

    void CreateInternalDirect3DDevice();
    void DestroyInternalDirect3DDevice();

    PlayResult PlayNextFrame();
    PlayResult PlayNextCall();

    bool TakeDeviceScreenshot(const std::string& filename, IDirect3DDevice9* device, ScreenshotFormat format);
    bool GetNextScreenshotFilename(const std::string& basepath, std::string& filename, ScreenshotFormat format);
    const char* GetScreenshotFormatExtension(ScreenshotFormat format);
    bool ConvertTextureToSurface(dxtraceman::DXTexturePtr textura, IDirect3DSurface9** surface);

  };
}

////////////////////////////////////////////////////////////////////////////////

EXPIMP_DLL extern dxpainter::DXPainter* WINAPI DXPainterCreate();

////////////////////////////////////////////////////////////////////////////////
