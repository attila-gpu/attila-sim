////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

class DXInterceptorOptions
{
public:

  enum BannerPosition {BP_TopLeft, BP_TopRight, BP_BottomLeft, BP_BottomRight};

  struct StatisticsPlugin
  {
  public:  
    
    std::string PluginFileName;
    std::vector<unsigned int> Counters;

    StatisticsPlugin()
    {
      PluginFileName = "";
      Counters.clear();
    }
    
    StatisticsPlugin(const StatisticsPlugin& plugin)
    {
      CloneObjects(plugin, *this);
    }
    
    StatisticsPlugin& operator = (const StatisticsPlugin& plugin)
    {
      CloneObjects(plugin, *this);
      return *this;
    }

  protected:
    
    static void CloneObjects(const StatisticsPlugin& orig, StatisticsPlugin& dest)
    {
      dest.PluginFileName = orig.PluginFileName;
      dest.Counters.clear();
      copy(orig.Counters.begin(), orig.Counters.end(), back_inserter(dest.Counters));
    }
  };
  
  DXInterceptorOptions();
  DXInterceptorOptions(const DXInterceptorOptions& options);
  virtual ~DXInterceptorOptions();

  DXInterceptorOptions& operator = (const DXInterceptorOptions& options);
  
  void Clear();

  std::string GetDestinationPath() const;
  void SetDestinationPath(const std::string& value);

  bool GetCompression() const;
  void SetCompression(bool value);

  bool GetBannerShow() const;
  void SetBannerShow(bool value);

  BannerPosition GetBannerPosition() const;
  void SetBannerPosition(BannerPosition value);

  D3DCOLOR GetBannerTextColor() const;
  void SetBannerTextColor(D3DCOLOR value);

  unsigned int GetPluginCount();
  bool GetPlugin(unsigned int number, StatisticsPlugin& plugin);
  void AddPlugin(const StatisticsPlugin& plugin);
  void ClearPlugins();
  
  bool LoadXML(const std::string& filename);
  bool SaveXML(const std::string& filename);

protected:

  std::string m_destinationPath;
  bool m_compression;
  bool m_bannerShow;
  BannerPosition m_bannerPosition;
  D3DCOLOR m_bannerTextColor;
  std::vector<StatisticsPlugin> m_plugins;

  static void CloneObjects(const DXInterceptorOptions& orig, DXInterceptorOptions& dest);

};

////////////////////////////////////////////////////////////////////////////////
