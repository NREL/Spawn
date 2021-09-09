//
// Created by jason on 5/21/20.
//

#include "fmu.hpp"

namespace spawn::fmu {
bool FMU::Variable::validate(const FMU::Variable::Type expected, const bool throw_error) const
{
  const auto type_match = type == expected;

  if (throw_error && !type_match) {
    throw std::runtime_error(fmt::format(
        "FMU Variable '{}' type mismatch, expected: '{}', actual: '{}'", name, to_string(type), to_string(expected)));
  }

  return type_match;
}

std::string FMU::modelIdentifier() const
{
  if (!m_model_description_parse_result) {
    throw std::runtime_error(
        fmt::format("Error parsing xml document: {}", m_model_description_parse_result.status));
  }
  const auto fmiModelDescription = m_model_description.child("fmiModelDescription");
  if (fmiModelDescription.empty()) {
    throw std::runtime_error("fmiModelDescription not found");
  }
  const auto modelExchange = fmiModelDescription.child("ModelExchange");
  if (modelExchange.empty()) {
    throw std::runtime_error("ModelExchange not found");
  }
  const auto modelIdentifier = modelExchange.attribute("modelIdentifier");
  if (modelIdentifier.empty()) {
    throw std::runtime_error("attribute modelIdentifier not found");
  }

  return modelIdentifier.value();
}

std::vector<FMU::Variable> FMU::variables(const pugi::xml_document &model_description)
{
  std::vector<FMU::Variable> result;

  const auto variables = model_description.select_nodes("//fmiModelDescription/ModelVariables/ScalarVariable");

  const auto getCausality = [](const pugi::xml_node &node) {
    const auto &attribute = node.attribute("causality"); 

    if (!attribute.empty()) {
      if (attribute.value() == std::string_view{"input"}) {
        return Variable::Causality::Input;
      } else if (attribute.value() == std::string_view{"output"}) {
        return Variable::Causality::Output;
      } else if (attribute.value() == std::string_view{"local"}) {
        return Variable::Causality::Local;
      } else if (attribute.value() == std::string_view{"calculatedParameter"}) {
        return Variable::Causality::CalculatedParameter;
      } else if (attribute.value() == std::string_view{"parameter"}) {
        return Variable::Causality::Parameter;
      }
    }

    throw std::runtime_error(fmt::format("Unable to determine causality of variable {}", attribute.value()));
  };

  const auto getType = [](const pugi::xml_node &node) {
    if (const auto &first_child = node.first_child(); !first_child.empty()) {
      if (first_child.name() == std::string_view{"Real"}) {
        return Variable::Type::Real;
      } else if (first_child.name() == std::string_view{"Integer"}) {
        return Variable::Type::Integer;
      } else if (first_child.name() == std::string_view{"Boolean"}) {
        return Variable::Type::Boolean;
      } else if (first_child.name() == std::string_view{"String"}) {
        return Variable::Type::String;
      } else if (first_child.name() == std::string_view{"Enumeration"}) {
        return Variable::Type::Enumeration;
      }
    }

    throw std::runtime_error("Unable to determine type of variable");
  };

  for (const auto &variable : variables) {
    if (const auto &node = variable.node(); !node.empty()) {

      result.emplace_back(
          Variable{node.attribute("name").value(),
                   static_cast<fmi2ValueReference>(std::atoi(node.attribute("valueReference").value())),
                   node.attribute("description").value(),
                   getType(node),
                   getCausality(node)
          });
    }
  }

  return result;
}

const FMU::Variable &FMU::getVariableByName(std::string_view name) const
{
  const auto variable =
      std::find_if(begin(m_variables), end(m_variables), [&](const Variable &var) { return var.name == name; });

  if (variable != end(m_variables)) {
    return *variable;
  }

  throw std::runtime_error("Unable to find variable: " + std::string(name));
}
}
