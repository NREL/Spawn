#ifndef outputvariable_hh_INCLUDED
#define outputvariable_hh_INCLUDED

#include "../../energyplus/third_party/nlohmann/json.hpp"

namespace spawn {

class OutputVariable
{
public:
  [[nodiscard]] static std::vector<OutputVariable> createOutputVariables(const nlohmann::json &spawnjson,
                                                                         const nlohmann::json &jsonidf);

  std::string spawnname;
  std::string idfname;
  std::string idfkey;

private:
  OutputVariable(std::string t_spawnname, std::string t_idfname, std::string t_idfkey) noexcept;
};

} // namespace spawn

#endif // outputvariable_hh_INCLUDED
