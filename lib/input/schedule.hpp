#ifndef Schedule_hh_INCLUDED
#define Schedule_hh_INCLUDED

#include "../../submodules/EnergyPlus/third_party/nlohmann/json.hpp"

namespace spawn {

class Schedule
{
public:
  [[nodiscard]] static std::vector<Schedule> createSchedules(const nlohmann::json &spawnjson,
                                                             const nlohmann::json &jsonidf);

  std::string spawnname;
  std::string idfname;
  std::string idftype;

private:
  Schedule(std::string spawnname, std::string idfname, std::string type) noexcept;
};

} // namespace spawn

#endif // Schedule_hh_INCLUDED
