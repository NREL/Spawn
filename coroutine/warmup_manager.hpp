#ifndef SPAWN_COROUTINE_WARMUP_MANAGER_H_
#define SPAWN_COROUTINE_WARMUP_MANAGER_H_

// C++ standard library headers
#include <string>
#include <vector>

// Spawn project headers
#include "./manager.hpp"

namespace spawn {

class WarmupManager : public Manager
{

public:
  explicit WarmupManager(EnergyPlus::EnergyPlusData &state);

protected:
  void Initialize(EnergyPlus::EnergyPlusData &state) override;
  void UpdateConvergenceMetrics(EnergyPlus::EnergyPlusData &state);
  void CheckConvergence(EnergyPlus::EnergyPlusData &state);

private:
  std::vector<double> max_surf_temp_;
  std::vector<double> min_surf_temp_;
  std::vector<double> prev_max_surf_temp_;
  std::vector<double> prev_min_surf_temp_;

  int last_day_of_sim_{0};
  std::string last_day_of_sim_chr_{"0"};

  static constexpr double surf_temp_converg_tol_{0.00001};
};

} // namespace spawn

#endif  // SPAWN_COROUTINE_WARMUP_MANAGER_H_
