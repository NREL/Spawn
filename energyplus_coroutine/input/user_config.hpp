#ifndef INPUT_HH_INCLUDED
#define INPUT_HH_INCLUDED

#include "../../util/filesystem.hpp"
#include "../iddtypes.hpp"
#include "../output_types.hpp"
#include "../variables.hpp"
#include "emsactuator.hpp"
#include "outputvariable.hpp"
#include "runperiod.hpp"
#include "schedule.hpp"
#include "surface.hpp"
#include "zone.hpp"
#include <fstream>

namespace spawn {

class UserConfig
{
public:
  explicit UserConfig(const std::string &spawnInputJSON);

  std::vector<Zone> zones;
  std::vector<Schedule> schedules;
  std::vector<input::OutputVariable> outputVariables;
  std::vector<EMSActuator> emsActuators;
  std::vector<Surface> surfaces;
  RunPeriod runPeriod;

  [[nodiscard]] spawn_fs::path basepath() const;

  [[nodiscard]] std::string fmuname() const;
  void setFMUName(std::string name);
  // fmu name without extension
  [[nodiscard]] std::string fmuBaseName() const;

  [[nodiscard]] spawn_fs::path idfInputPath() const;
  void setIdfInputPath(const spawn_fs::path &idfpath);

  [[nodiscard]] spawn_fs::path epwInputPath() const;
  void setEPWInputPath(const spawn_fs::path &epwpath);

  [[nodiscard]] bool autosize() const;

  double relativeSurfaceTolerance() const;

  void save(const spawn_fs::path &savepath) const;

  nlohmann::json jsonidf;
  nlohmann::json spawnjson;

private:
  // Return an expanded absolute path,
  // this will prepend basepath if the given pathstring is not absolute
  [[nodiscard]] spawn_fs::path toPath(const std::string &pathstring) const;

  spawn_fs::path m_basepath;
};

} // namespace spawn

#endif // INPUT_HH_INCLUDED