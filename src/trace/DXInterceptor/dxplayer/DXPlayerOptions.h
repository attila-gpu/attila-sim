////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

class DXPlayerOptions
{
public:

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
  
  DXPlayerOptions();
  DXPlayerOptions(const DXPlayerOptions& options);
  virtual ~DXPlayerOptions();

  DXPlayerOptions& operator = (const DXPlayerOptions& options);
  
  void Clear();

  std::string GetDestinationPath() const;
  void SetDestinationPath(const std::string& value);

  ScreenshotFormat GetScreenshotFormat() const;
  void SetScreenshotFormat(ScreenshotFormat value);
  
  bool LoadXML(const std::string& filename);
  bool SaveXML(const std::string& filename);

protected:

  std::string m_destinationPath;
  ScreenshotFormat m_screenshotFormat;

  static void CloneObjects(const DXPlayerOptions& orig, DXPlayerOptions& dest);

};

////////////////////////////////////////////////////////////////////////////////
