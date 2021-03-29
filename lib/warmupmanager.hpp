#ifndef Spawn_warmupmanager_hh_INCLUDED
#define Spawn_warmupmanager_hh_INCLUDED

#include "../submodules/EnergyPlus/src/EnergyPlus/Data/EnergyPlusData.hh"
#include <vector>

namespace spawn {

class WarmupManager {

protected:

  void initialize(EnergyPlus::EnergyPlusData & state);
  void endZoneTimestepAfterZoneReporting(EnergyPlus::EnergyPlusData & state);
  void beginNewEnvironmentAfterWarmUp(EnergyPlus::EnergyPlusData & state);

private:

  std::vector<double> maxSurfTemp;
  std::vector<double> minSurfTemp;
  std::vector<double> prevMaxSurfTemp;
  std::vector<double> prevMinSurfTemp;

  int lastDayOfSim{0};
  std::string lastDayOfSimChr{"0"};

  bool initialized{false};
  static constexpr double surfTempConvergTol{0.001};
};


} // namespace spawn

#endif // Spawn_warmupmanager_hh_INCLUDED
