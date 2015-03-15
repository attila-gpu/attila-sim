////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

class XMLConfig
{
public:

  XMLConfig(const std::string& filename, bool createIfNotExists=false);

  bool AddSection(const std::string& sectionName);
  bool AddSectionText(const std::string& sectionName,   const std::string& sectionText);
  bool AddSectionAttribute(const std::string& sectionName, const std::string& attribName, const std::string& attribValue);

  bool GetSectionText(const std::string& sectionName, std::string& sectionText);
  bool GetSectionAttribute(const std::string& sectionName, const std::string& attribName, std::string& attribValue);
  
  bool FillDefault();
  bool Save();

protected:

  std::string m_filename;
  TiXmlDocument m_xmlDocument;

  bool AddHeader();
  bool ExistsHeader();
  TiXmlNode* GetSection(const std::string& sectionName);
  void Clear();

};

////////////////////////////////////////////////////////////////////////////////
