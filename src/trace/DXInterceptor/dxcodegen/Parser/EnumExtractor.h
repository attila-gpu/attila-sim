////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "Parser/IExtractor.h"
#include "Items/EnumDescription.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Config
  {
    class ParserConfiguration;
  }

  namespace Parser
  {

    ////////////////////////////////////////////////////////////////////////////
    ///
    /// @class EnumExtractor
    /// @brief Parseja codi C++ i extrau totes les declaracions de tipus <i>enum</i>
    ///
    /// Aquesta classe cerca en una cadena de text amb codi C++ totes les
    /// declaracions de tipus <i>enum</i> i les extrau. Cada element trobat es
    /// guarda en una estructura de dades de tipus Items::EnumDescription.
    ///
    /// @version 1.0
    /// @date 2007/05/01
    /// @author David Abella - david.abella@gmail.com
    ///
    ////////////////////////////////////////////////////////////////////////////

    class EnumExtractor : public IExtractor
    {
    public:

      /*!
       * @brief Constructor de la classe
       *
       * Crea una instancia de la classe i li associa la cadena de text amb codi
       * C++ on cercarà i extraurà declaracions de tipus <i>enum</i>.
       *
       * @param[in]     config Referència a la configuració que volem emprar per
       *                parsejar.
       * @param[in,out] cadena Cadena de text amb codi C++ on volem cercar
       *                declaracions de tipus <i>enum</i>. S'eliminaràn de la
       *                cadena tots els blocs de codi de les definicions
       *                trobades.
       */
      EnumExtractor(Config::ParserConfiguration& config, std::string& cadena);

      /*!
       * @brief Destructor de la classe
       */
      virtual ~EnumExtractor();

      /*!
       * @brief Obté una llista amb totes les enumeracions trobades
       */
      const std::vector<Items::EnumDescriptionPtr>& GetEnums();

    private:

      Config::ParserConfiguration& m_config;
      std::vector<Items::EnumDescriptionPtr> m_lstEnums;

      MatchResults Match(std::string& cadena);
      void Parse(const MatchResults& resultat);

      MatchResults MatchTypedefEnum(std::string& cadena);
      MatchResults MatchEnum(std::string& cadena);

      void ParseTypedefEnum(const MatchResults& resultat);
      void ParseEnum(const MatchResults& resultat);
      void ParseEnumMembers(const std::string& name, const std::string& cadena);
      bool ParseEnumMember(Items::EnumDescriptionPtr edes, std::string& cadena);

    };
  }
}

////////////////////////////////////////////////////////////////////////////////