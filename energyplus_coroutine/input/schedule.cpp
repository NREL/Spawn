#include "schedule.hpp"
#include "../iddtypes.hpp"

using json = nlohmann::json;

namespace spawn {

Schedule::Schedule(std::string t_spawnname, std::string t_idfname, std::string t_idftype) noexcept
    : spawnname(std::move(t_spawnname)), idfname(std::move(t_idfname)), idftype(std::move(t_idftype))
{
  std::transform(idfname.begin(), idfname.end(), idfname.begin(), ::toupper);
}

std::vector<Schedule> Schedule::createSchedules(const json &spawnjson, const nlohmann::json &jsonidf)
{
  std::vector<Schedule> result;

  // key is a schedule name, value is the schedule idd type
  std::map<std::string, std::string> types;

  for (const auto &type : supportedScheduleTypes) {
    if (jsonidf.contains(type)) {
      for (const auto &schedule : jsonidf[type].items()) {
        types[schedule.key()] = type;
      }
    }
  }

  const auto modelicaSchedules = spawnjson.value("model", json::object()).value("schedules", std::vector<json>(0));
  for (const auto &modelicaSchedule : modelicaSchedules) {
    const auto idfname = modelicaSchedule.value("name", "");
    const auto fminame = modelicaSchedule.value("fmiName", "");
    const auto typeit = types.find(idfname);
    if (typeit != std::end(types)) {
      const auto &buildSchedule = [&]() {
        Schedule Schedule(fminame, idfname, typeit->second);
        return Schedule;
      };
      result.emplace_back(buildSchedule());
    }
  }

  return result;
}

} // namespace spawn
