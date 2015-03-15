////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

class DXInterceptorBanner
{
public:

  DXInterceptorBanner(LPDIRECT3DDEVICE9 device);
  virtual ~DXInterceptorBanner();

  bool Init();
  bool Close();
  void Message(const char* cadena, D3DCOLOR color, unsigned int duration);
  unsigned int GetDeviceReferenceCount() const;
  void SetViewport(const D3DVIEWPORT9* viewport);

  inline void Draw()
  {
    (*this.*DrawFunction)();
  }

protected:

  static const DWORD MESSAGE_DURATION_MIN;
  static DWORD ms_timeElapsed;
  static DWORD ms_frameCount;
  
  DWORD m_FPS; // Frames per Second
  DWORD m_lastTimeStamp;
  DWORD m_numFramesFromLastSecond;
  DWORD m_timeElapsedFromLastSecond;
  
  D3DXVECTOR3 m_positionIcon;
  CRect m_positionText;
  CRect m_positionTextWork;
  D3DCOLOR m_colorCounters;
  
  LPDIRECT3DDEVICE9 m_device;
  LPDIRECT3DTEXTURE9 m_texture1;
  LPDIRECT3DTEXTURE9 m_texture2;
  LPD3DXSPRITE m_sprite;
  LPD3DXFONT m_font;
  
  bool m_initialized;
  bool m_traceCompressed;
  bool m_traceDiscardByCRC;
  bool m_modeComplete;
  unsigned int m_deviceReferenceCount;

  struct MessageInformation;
  std::vector<MessageInformation> m_messages;

  void SetOptions();
  void SetPositions(const CRect* viewportD3D);
  void SetFunctions();

  bool InitResources();
  
  void DrawSimple();
  void DrawComplete();
  
  void DrawDirectXIcon();
  void DrawTextCountersSimple();
  void DrawTextCountersComplete();
  void DrawTextMessages();
  
  void (DXInterceptorBanner::*DrawFunction)();
  
  void (DXInterceptorBanner::*DrawTextFunction)(const char*, D3DCOLOR);
  
  inline void DrawText(const char* cadena, D3DCOLOR color)
  {
    (*this.*DrawTextFunction)(cadena, color);
  }

  void DrawText_TopLeft(const char* cadena, D3DCOLOR color);
  void DrawText_TopRight(const char* cadena, D3DCOLOR color);
  void DrawText_BottomLeft(const char* cadena, D3DCOLOR color);
  void DrawText_BottomRight(const char* cadena, D3DCOLOR color);

  void InitCounters();
  void ResetCounters();
  void UpdateCounters();

};

////////////////////////////////////////////////////////////////////////////////
