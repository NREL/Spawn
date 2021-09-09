#ifndef spawn_fmu_hpp_INCLUDED
#define spawn_fmu_hpp_INCLUDED

#include "../util/dynamiclibrary.hpp"
#include "../util/fmi_paths.hpp"
#include "../util/temp_directory.hpp"
#include "../util/unzipped_file.hpp"

#include "../util/fmi_paths.hpp"
#include "../util/filesystem.hpp"

#include "fmi2.hpp"

#include <functional>
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
    struct Variable
    {
      enum struct Type
      {
        Real,
        String,
        Boolean,
        Integer,
        Enumeration
      };

      enum struct Causality
      {
        Input,
        Output,
        Local,
        CalculatedParameter,
        Parameter
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
        case Type::Enumeration:
          return "Enumeration";
        }

        return "Unknown";
      }

      std::string name;
      fmi2ValueReference valueReference;
      std::string description;
      Type type;
      Causality causality;

      [[nodiscard]] bool validate(const Type expected, const bool throw_error = true) const;
    };



    /// \param fmu_file FMU to open, extract, and load FMI symbols from
    /// \param require_all_symbols If true, an exception is thrown if an FMI symbol could not be loaded
    /// \param scratch_directory If a scratch_directory is passed in, it is used instead of an automatically managed temp directory
    explicit FMU(fs::path fmu_file, bool require_all_symbols = true, fs::path scratch_directory = {})
        : m_fmu_file{std::move(fmu_file)}, m_require_all_symbols{require_all_symbols},
          m_scratch_directory{std::move(scratch_directory)}
    {
    }

    static fs::path modelDescriptionPath()
    {
      return "modelDescription.xml";
    }

    [[nodiscard]] std::string modelIdentifier() const;

    [[nodiscard]] fs::path fmiBinaryFullPath() const
    {
      const auto libFilename = fmi_lib_filename(modelIdentifier());

      std::vector<fs::path> possiblePaths {
        m_unzipped.outputDir() / fs::path{"binaries"} / fmi_platform() / libFilename,
        m_unzipped.outputDir() / fs::path{"binaries"} / libFilename
      };

      for (const auto p : possiblePaths) {
        if (fs::exists(p)) {
          return p;
        }
      }

      return fs::path();
    }

    [[nodiscard]] const pugi::xml_document &modelDescription() const noexcept
    {
      return m_model_description;
    }

    [[nodiscard]] const auto &getVariables() const noexcept
    {
      return m_variables;
    }

    [[nodiscard]] const Variable &getVariableByName(std::string_view name) const;

    [[nodiscard]] const fs::path &extractedFilesPath() const noexcept {
      if (!m_scratch_directory.empty()) {
        return m_scratch_directory;
      } else {
        return m_temp_directory.dir();
      }
    }


  private:
    fs::path m_fmu_file;
    bool m_require_all_symbols;
    fs::path m_scratch_directory;
    util::Temp_Directory m_temp_directory;

    // unzip all files
    util::Unzipped_File m_unzipped{m_fmu_file, extractedFilesPath(), {}};

    pugi::xml_document m_model_description;
    pugi::xml_parse_result m_model_description_parse_result{m_model_description.load_file((m_unzipped.outputDir() / modelDescriptionPath()).c_str())};

    std::vector<Variable> m_variables{variables(m_model_description)};

    /// Loads all Variables in the given xml_document
    [[nodiscard]] static std::vector<Variable> variables(const pugi::xml_document &model_description);

  public:
    FMI2 fmi{fmiBinaryFullPath(), m_require_all_symbols};
  };


} // namespace spawn

#endif
