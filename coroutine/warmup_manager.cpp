#include "./warmup_manager.hpp"

// EnergyPlus headers
#include "../energyplus/src/EnergyPlus/Data/EnergyPlusData.hh"
#include "../energyplus/src/EnergyPlus/DataHeatBalance.hh"
#include "../energyplus/src/EnergyPlus/DataHeatBalSurface.hh"
#include "../energyplus/src/EnergyPlus/EMSManager.hh"

namespace spawn {

WarmupManager::WarmupManager(EnergyPlus::EnergyPlusData &state) : Manager(state)
{
  callbacks_[EnergyPlus::EMSManager::EMSCallFrom::EndZoneTimestepAfterZoneReporting] = [this](auto &state) {
    UpdateConvergenceMetrics(state);
  };

  callbacks_[EnergyPlus::EMSManager::EMSCallFrom::BeginNewEnvironmentAfterWarmUp] = [this](auto &state) {
    CheckConvergence(state);
  };
}

void WarmupManager::Initialize(EnergyPlus::EnergyPlusData &state)
{
  const auto count = state.dataHeatBalSurf->SurfTempIn.size();

  max_surf_temp_.clear();
  min_surf_temp_.clear();
  prev_max_surf_temp_.clear();
  prev_min_surf_temp_.clear();

  max_surf_temp_.resize(count);
  min_surf_temp_.resize(count);
  prev_max_surf_temp_.resize(count);
  prev_min_surf_temp_.resize(count);

  last_day_of_sim_ = state.dataGlobal->DayOfSim;
  last_day_of_sim_chr_ = state.dataGlobal->DayOfSimChr;

  Manager::Initialize(state);
}

void WarmupManager::UpdateConvergenceMetrics(EnergyPlus::EnergyPlusData &state)
{
  if (!state.dataGlobal->WarmupFlag) {
    return;
  }

  if (!initialized_) {
    Initialize(state);
  }

  const auto &surfTemp = state.dataHeatBalSurf->SurfTempIn;
  last_day_of_sim_ = state.dataGlobal->DayOfSim;
  last_day_of_sim_chr_ = state.dataGlobal->DayOfSimChr;

  if (state.dataGlobal->BeginDayFlag) {
    prev_max_surf_temp_ = max_surf_temp_;
    prev_min_surf_temp_ = min_surf_temp_;
    for (size_t i = 0; i < surfTemp.size(); ++i) {
      max_surf_temp_[i] = surfTemp[i];
      min_surf_temp_[i] = surfTemp[i];
    }
  } else {
    for (size_t i = 0; i < surfTemp.size(); ++i) {
      if (surfTemp[i] > max_surf_temp_[i]) {
        max_surf_temp_[i] = surfTemp[i];
      }

      if (surfTemp[i] < min_surf_temp_[i]) {
        min_surf_temp_[i] = surfTemp[i];
      }
    }
  }
}

void WarmupManager::CheckConvergence(EnergyPlus::EnergyPlusData &state)
{
  // If the max number of warmup days has been exceeded then return early,
  // and allow the simulation to continue into the run period
  if (last_day_of_sim_ >= state.dataHeatBal->MaxNumberOfWarmupDays) {
    state.dataGlobal->WarmupFlag = false;
    return;
  }

  bool convergence_checks_failed = false;

  for (size_t i = 0; i < max_surf_temp_.size(); ++i) {
    const auto max_surf_temp_diff = std::abs(max_surf_temp_[i] - prev_max_surf_temp_[i]);
    if (max_surf_temp_diff > surf_temp_converg_tol_) {
      convergence_checks_failed = true;
      break;
    }
  }

  for (size_t i = 0; i < min_surf_temp_.size(); ++i) {
    const auto min_surf_temp_diff = std::abs(min_surf_temp_[i] - prev_min_surf_temp_[i]);
    if (min_surf_temp_diff > surf_temp_converg_tol_) {
      convergence_checks_failed = true;
      break;
    }
  }

  if (convergence_checks_failed) {
    state.dataGlobal->WarmupFlag = true;
    // EnergyPlus will have reset these values after convergence was satisfied
    // Here they are restored to their last value before reset
    state.dataGlobal->DayOfSim = last_day_of_sim_;
    state.dataGlobal->DayOfSimChr = last_day_of_sim_chr_;
  }
}

} // namespace spawn
