#ifndef spawn_modeldescription_hpp_INCLUDED
#define spawn_modeldescription_hpp_INCLUDED

#include "../util/filesystem.hpp"
#include <fmt/format.h>
#include <pugixml.hpp>

namespace spawn::fmu {

class ModelDescription
{
public:
  explicit ModelDescription(fs::path model_description_path)
    : m_model_description_path(std::move(model_description_path))
  {
    // note this is initialized by member initializer below
    if (!m_model_description_parse_result) {
      throw std::runtime_error(
          fmt::format("Error parsing xml document located at : {}", model_description_path.string()));
    }
  }

  [[nodiscard]] unsigned int valueReference(const std::string & variable_name) const {
    const auto valueReference = scalarVariable(variable_name).attribute("valueReference");
    if (valueReference) {
      const auto  vr = valueReference.as_int(-1);
      if (vr >= 0) {
        return static_cast<unsigned int>(vr);
      }
    }

    throw std::runtime_error(
      fmt::format("Unable to retrieve value reference for variable named: {}", variable_name));
  }

private:

  [[nodiscard]] pugi::xml_node fmiModelDescription() const {
    const auto result = m_model_description.child("fmiModelDescription");
    if (result.empty()) {
      throw std::runtime_error("fmiModelDescription not found");
    }
    return result;
  }

  [[nodiscard]] pugi::xml_node modelVariables() const {
    const auto result = fmiModelDescription().child("ModelVariables");
    if (result.empty()) {
      throw std::runtime_error("ModelVariables not found");
    }
    return result;
  }

  [[nodiscard]] pugi::xml_node scalarVariable(const std::string & variable_name) const {
    const auto vars = modelVariables();
    const auto scalarVariables = vars.children("ScalarVariable");
    for (const auto scalarVar : scalarVariables) {
      const auto name = scalarVar.attribute("name").value();
      if (name == variable_name) {
        return scalarVar;
      }
    }

    throw std::runtime_error(
      fmt::format("Error finding ScalarVariable with name: {}", variable_name));
  }

  fs::path m_model_description_path;
  pugi::xml_document m_model_description;
  pugi::xml_parse_result m_model_description_parse_result{m_model_description.load_file(m_model_description_path.c_str())};
};

} // namespace spawn::fmu

#endif // spawn_modeldescription_hpp_INCLUDED
