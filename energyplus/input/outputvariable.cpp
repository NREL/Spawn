#include "outputvariable.hpp"

#include <utility>

using json = nlohmann::json;

namespace spawn {

OutputVariable::OutputVariable(std::string t_spawnname, std::string t_idfname, std::string t_idfkey) noexcept
    : spawnname(std::move(t_spawnname)), idfname(std::move(t_idfname)), idfkey(std::move(t_idfkey))
{
  std::transform(idfname.begin(), idfname.end(), idfname.begin(), ::toupper);
  std::transform(idfkey.begin(), idfkey.end(), idfkey.begin(), ::toupper);
}

std::vector<OutputVariable> OutputVariable::createOutputVariables(const nlohmann::json &spawnjson,
                                                                  [[maybe_unused]] const nlohmann::json &jsonidf)
{
  std::vector<OutputVariable> result;

  const auto spawnOutputVariables = spawnjson.value("model", json()).value("outputVariables", std::vector<json>(0));
  for (const auto &spawnOutputVariable : spawnOutputVariables) {
    const auto spawnname = spawnOutputVariable.value("fmiName", "");
    const auto idfname = spawnOutputVariable.value("name", "");
    const auto idfkey = spawnOutputVariable.value("key", "");
    const auto &buildvariable = [&]() {
      OutputVariable variable(spawnname, idfname, idfkey);
      return variable;
    };
    result.emplace_back(buildvariable());
  }

  return result;
}

} // namespace spawn
