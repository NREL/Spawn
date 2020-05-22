#ifndef spawn_fmu_hpp_INCLUDED
#define spawn_fmu_hpp_INCLUDED

#include "../util/dynamiclibrary.hpp"
#include "../util/fmi_paths.hpp"
#include "../util/temp_directory.hpp"
#include "../util/unzipped_file.hpp"

#include "fmi2.hpp"

#include <boost/filesystem/path.hpp>
#include <fmi2FunctionTypes.h>

#include <optional>
#include <pugixml.hpp>

namespace spawn::fmu {

  /// Represents an FMU file, with the associated FMI2 DynamicLibrary and all resources
  /// \todo Read project defaults from XML. Or maybe doe this in the ModelExchange?
  ///       Whatever is comment between ModelExchange and CoSimulation should decide where it lives
  class FMU
  {
  public:
    /// Represents a Variable in the modelDescription.xml contained in the FMU
    struct Variable
    {
      enum struct Type
      {
        Real,
        String,
        Boolean,
        Integer
      };

      enum struct Causality
      {
        Input,
        Output,
        Local
      };

      [[nodiscard]] static constexpr std::string_view to_string(const Type type) noexcept
      {
        switch (type) {
        case Type::Real:
          return "Real";
        case Type::String:
          return "String";
        case Type::Boolean:
          return "Boolean";
        case Type::Integer:
          return "Integer";
        }

        return "Unknown";
      }

      std::string name;
      fmi2ValueReference valueReference;
      std::string description;
      Type type;
      Causality causality;

      bool validate(const Type expected, const bool throw_error = true) const;
    };

    explicit FMU(boost::filesystem::path fmu_file, bool require_all_symbols = true)
        : m_fmu_file{std::move(fmu_file)}, m_require_all_symbols{require_all_symbols}
    {
    }

    [[nodiscard]] static boost::filesystem::path modelDescriptionPath()
    {
      return "modelDescription.xml";
    }

    [[nodiscard]] std::string modelIdentifier() const;

    [[nodiscard]] boost::filesystem::path fmiBinaryFullPath() const
    {
      return spawn::fmi_lib_path(modelIdentifier());
    }

    [[nodiscard]] const pugi::xml_document &modelDescription() const
    {
      return m_model_description;
    }

    /// Loads all Variables in the given xml_document
    [[nodiscard]] static std::vector<Variable> variables(const pugi::xml_document &model_description);

    [[nodiscard]] const auto &getVariables() const noexcept
    {
      return m_variables;
    }

    [[nodiscard]] const Variable &getVariableByName(std::string_view name) const;

  private:
    boost::filesystem::path m_fmu_file;
    bool m_require_all_symbols;
    util::Temp_Directory m_tempDirectory{};
    util::Unzipped_File m_unzipped_xml{m_fmu_file, m_tempDirectory.dir(), {modelDescriptionPath()}};

    pugi::xml_document m_model_description;
    pugi::xml_parse_result m_model_description_parse_result{
        m_model_description.load_file(m_unzipped_xml.unzippedFiles().at(0).c_str())};
    util::Unzipped_File m_unzipped_fmi{m_fmu_file, m_tempDirectory.dir(), {fmiBinaryFullPath()}};
    std::vector<Variable> m_variables{variables(m_model_description)};

  public:
    FMI2 fmi{m_unzipped_fmi.unzippedFiles().at(0), m_require_all_symbols};
  };


} // namespace spawn

#endif