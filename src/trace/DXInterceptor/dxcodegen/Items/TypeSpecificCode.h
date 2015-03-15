////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Items/SmartPointer.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Items
  {
    class TypeSpecificCode : public SmartPointer
    {
    public:

      enum SaveIn {SI_CallStack = 1, SI_Buffer};
      enum PassBy {PB_Value = 1, PB_Reference};
      
      TypeSpecificCode(const std::string& name, SaveIn saveIn, const std::string& useMethod, PassBy passBy);
      virtual ~TypeSpecificCode();
      
      std::string GetName() const;
      SaveIn GetSaveIn() const;
      std::string GetUseMethod() const;
      PassBy GetPassBy() const;
      
    protected:

      std::string m_name;
      SaveIn m_saveIn;
      std::string m_useMethod;
      PassBy m_passBy;

    };

    ////////////////////////////////////////////////////////////////////////////
    
    typedef smart_ptr<TypeSpecificCode> TypeSpecificCodePtr;

    ////////////////////////////////////////////////////////////////////////////
  }
}

////////////////////////////////////////////////////////////////////////////////
