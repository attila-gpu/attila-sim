////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "Items/SmartPointer.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Generator
  {
    class IGenerator : public Items::SmartPointer
    {
    public:

      IGenerator(const std::string& generatedItemDescription);
      virtual ~IGenerator();
      
      std::string& GetHeaderCommment();
      
      virtual void GenerateCode() = 0;

    protected:

      std::string m_generatedItemDescription;
      std::string m_headerComment;

      void WriteHeaderComment(std::ofstream* of);

      std::ofstream* CreateFilename(const std::string& filename);
      void CloseFilename(std::ofstream* of);
    
    private:

      void CreateHeaderComment();
    
    };

    ////////////////////////////////////////////////////////////////////////////

    typedef Items::smart_ptr<IGenerator> IGeneratorPtr;

    ////////////////////////////////////////////////////////////////////////////
  }
}

////////////////////////////////////////////////////////////////////////////////
