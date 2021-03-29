#ifndef spawn_modeldescription_hpp_INCLUDED
#define spawn_modeldescription_hpp_INCLUDED

#include "../util/filesystem.hpp"
#include <fmt/format.h>
#include <pugixml.hpp>

namespace spawn {
namespace fmu {

class ModelDescription
{
public:
  ModelDescription(const std::filesystem::path & model_description_path)
    : m_model_description_path(model_description_path)
  {
    if (!m_model_description_parse_result) {
      throw std::runtime_error(
          fmt::format("Error parsing xml document located at : {}", model_description_path.string()));
    }
  }

  unsigned int valueReference(const std::string & variable_name)  const {
    const auto valueReference = scalarVariable(variable_name).attribute("valueReference");
    if (valueReference) {
      const auto  vr = valueReference.as_int(-1);
      if (vr >= 0) {
        return (unsigned int)vr;
      }
    }

    throw std::runtime_error(
      fmt::format("Unable to retrieve value reference for variable named: {}", variable_name));
  }

private:

  pugi::xml_node fmiModelDescription() const {
    const auto result = m_model_description.child("fmiModelDescription");
    if (result.empty()) {
      throw std::runtime_error("fmiModelDescription not found");
    }
    return result;
  }

  pugi::xml_node modelVariables() const {
    const auto result = fmiModelDescription().child("ModelVariables");
    if (result.empty()) {
      throw std::runtime_error("ModelVariables not found");
    }
    return result;
  }

  pugi::xml_node scalarVariable(const std::string & variable_name) const {
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

  std::filesystem::path m_model_description_path;
  pugi::xml_document m_model_description;
  pugi::xml_parse_result m_model_description_parse_result{m_model_description.load_file(m_model_description_path.c_str())};
};

} // namespace fmu
} // namespace spawn

#endif // spawn_modeldescription_hpp_INCLUDED
