#ifndef Spawn_warmupmanager_hh_INCLUDED
#define Spawn_warmupmanager_hh_INCLUDED

#include "./manager.hpp"
#include <string>
#include <vector>

namespace spawn {

class WarmupManager : public Manager
{

public:
  explicit WarmupManager(EnergyPlus::EnergyPlusData &state);

protected:
  void initialize(EnergyPlus::EnergyPlusData &state);
  void updateConvergenceMetrics(EnergyPlus::EnergyPlusData &state);
  void checkConvergence(EnergyPlus::EnergyPlusData &state);

private:
  std::vector<double> maxSurfTemp;
  std::vector<double> minSurfTemp;
  std::vector<double> prevMaxSurfTemp;
  std::vector<double> prevMinSurfTemp;

  int lastDayOfSim{0};
  std::string lastDayOfSimChr{"0"};

  static constexpr double surfTempConvergTol{0.00001};
};

} // namespace spawn

#endif // Spawn_warmupmanager_hh_INCLUDED
