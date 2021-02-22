#ifndef INPUT_HH_INCLUDED
#define INPUT_HH_INCLUDED

#include "emsactuator.hpp"
#include "outputvariable.hpp"
#include "schedule.hpp"
#include "surface.hpp"
#include "zone.hpp"
#include "../variables.hpp"
#include "../iddtypes.hpp"
#include "../outputtypes.hpp"
#include <filesystem>
#include <fstream>

namespace spawn {

class Input {
public:
  Input(const std::string & spawnInputJSON);

  std::vector<Zone> zones;
  std::vector<Schedule> schedules;
  std::vector<OutputVariable> outputVariables;
  std::vector<EMSActuator> emsActuators;
  std::vector<Surface> surfaces;

  std::filesystem::path basepath() const;

  std::string fmuname() const;
  void setFMUName(const std::string & name);
  // fmu name without extension
  std::string fmuBaseName() const;

  std::filesystem::path idfInputPath() const;
  void setIdfInputPath(std::filesystem::path idfpath);

  std::filesystem::path epwInputPath() const;
  void setEPWInputPath(std::filesystem::path epwpath);


  void save(const std::filesystem::path & savepath) const;

private:

  // Return an expanded absolute path,
  // this will prepend basepath if the given pathstring is not absolute
  std::filesystem::path toPath(const std::string & pathstring) const;

  nlohmann::json jsonidf;
  nlohmann::json spawnjson;
  std::filesystem::path m_basepath;
};

} // namespace spawn

#endif // INPUT_HH_INCLUDED

