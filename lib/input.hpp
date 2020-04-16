#include "emsactuator.hpp"
#include "iddtypes.hpp"
#include "outputtypes.hpp"
#include "outputvariable.hpp"
#include "schedule.hpp"
#include "variables.hpp"
#include "zone.hpp"
#include "../submodules/EnergyPlus/src/EnergyPlus/InputProcessing/IdfParser.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/InputProcessing/EmbeddedEpJSONSchema.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataStringGlobals.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/UtilityRoutines.hh"
#include "../submodules/EnergyPlus/third_party/nlohmann/json.hpp"
#include <boost/filesystem.hpp>
#include <fstream>

namespace spawn {

class Input {
public:
  Input(const std::string & spawnInputJSON);

  std::vector<Zone> zones;
  std::vector<Schedule> schedules;
  std::vector<OutputVariable> outputVariables;
  std::vector<EMSActuator> emsActuators;

  boost::filesystem::path basepath() const;

  std::string fmuname() const;
  void setFMUName(const std::string & name);
  // fmu name without extension
  std::string fmuBaseName() const;

  boost::filesystem::path idfInputPath() const;
  void setIdfInputPath(boost::filesystem::path idfpath);

  boost::filesystem::path epwInputPath() const;
  void setEPWInputPath(boost::filesystem::path epwpath);


  void save(const boost::filesystem::path & savepath) const;

private:

  // Return an expanded absolute path,
  // this will prepend basepath if the given pathstring is not absolute
  boost::filesystem::path toPath(const std::string & pathstring) const;

  nlohmann::json jsonidf;
  nlohmann::json spawnjson;
  boost::filesystem::path m_basepath;
};

} // namespace spawn

