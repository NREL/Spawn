#ifndef INPUT_HH_INCLUDED
#define INPUT_HH_INCLUDED

#include "../../util/filesystem.hpp"
#include "../iddtypes.hpp"
#include "../outputtypes.hpp"
#include "../variables.hpp"
#include "emsactuator.hpp"
#include "outputvariable.hpp"
#include "schedule.hpp"
#include "surface.hpp"
#include "zone.hpp"
#include <fstream>

namespace spawn {

class Input
{
public:
  explicit Input(const std::string &spawnInputJSON);

  std::vector<Zone> zones;
  std::vector<Schedule> schedules;
  std::vector<OutputVariable> outputVariables;
  std::vector<EMSActuator> emsActuators;
  std::vector<Surface> surfaces;

  [[nodiscard]] spawn_fs::path basepath() const;

  [[nodiscard]] std::string fmuname() const;
  void setFMUName(std::string name);
  // fmu name without extension
  [[nodiscard]] std::string fmuBaseName() const;

  [[nodiscard]] spawn_fs::path idfInputPath() const;
  void setIdfInputPath(const spawn_fs::path& idfpath);

  [[nodiscard]] spawn_fs::path epwInputPath() const;
  void setEPWInputPath(const spawn_fs::path& epwpath);

  double relativeSurfaceTolerance() const;

  void save(const spawn_fs::path &savepath) const;

private:
  // Return an expanded absolute path,
  // this will prepend basepath if the given pathstring is not absolute
  [[nodiscard]] spawn_fs::path toPath(const std::string &pathstring) const;

  nlohmann::json jsonidf;
  nlohmann::json spawnjson;
  spawn_fs::path m_basepath;
};

} // namespace spawn

#endif // INPUT_HH_INCLUDED
