#include "./warmupmanager.hpp"
#include "../submodules/EnergyPlus/src/EnergyPlus/Data/EnergyPlusData.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataGlobals.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataHeatBalSurface.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataHeatBalance.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/EMSManager.hh"

namespace spawn {

WarmupManager::WarmupManager(EnergyPlus::EnergyPlusData & state) 
  : Manager(state)
{
  callbacks[EnergyPlus::EMSManager::EMSCallFrom::EndZoneTimestepAfterZoneReporting] = 
    std::bind(&WarmupManager::updateConvergenceMetrics, this, std::placeholders::_1);

  callbacks[EnergyPlus::EMSManager::EMSCallFrom::BeginNewEnvironmentAfterWarmUp] =
    std::bind(&WarmupManager::checkConvergence, this, std::placeholders::_1);
}

void WarmupManager::initialize(EnergyPlus::EnergyPlusData & state) {
  const auto count = state.dataHeatBalSurf->SurfTempIn.size();

  maxSurfTemp.clear();
  minSurfTemp.clear();
  prevMaxSurfTemp.clear();
  prevMinSurfTemp.clear();

  maxSurfTemp.resize(count);
  minSurfTemp.resize(count);
  prevMaxSurfTemp.resize(count);
  prevMinSurfTemp.resize(count);

  lastDayOfSim = state.dataGlobal->DayOfSim;
  lastDayOfSimChr = state.dataGlobal->DayOfSimChr;

  Manager::initialize(state);
}

void WarmupManager::updateConvergenceMetrics(EnergyPlus::EnergyPlusData & state) {
  if (! state.dataGlobal->WarmupFlag) return;

  if (! initialized) {
    initialize(state);
  }

  const auto & surfTemp = state.dataHeatBalSurf->SurfTempIn;
  lastDayOfSim = state.dataGlobal->DayOfSim;
  lastDayOfSimChr = state.dataGlobal->DayOfSimChr;

  if (state.dataGlobal->BeginDayFlag) {
    prevMaxSurfTemp = maxSurfTemp;
    prevMinSurfTemp = minSurfTemp;
    for (size_t i = 0; i < surfTemp.size(); ++i) {
      maxSurfTemp[i] = surfTemp[i];
      minSurfTemp[i] = surfTemp[i];
    }
  } else {
    for (size_t i = 0; i < surfTemp.size(); ++i) {
      if (surfTemp[i] > maxSurfTemp[i]) {
        maxSurfTemp[i] = surfTemp[i];
      }

      if (surfTemp[i] < minSurfTemp[i]) {
        minSurfTemp[i] = surfTemp[i];
      }
    }
  }
}

void WarmupManager::checkConvergence(EnergyPlus::EnergyPlusData & state) {
  // If the max number of warmup days has been exceeded then return early,
  // and allow the simulation to continue into the run period
  if (lastDayOfSim >= state.dataHeatBal->MaxNumberOfWarmupDays) {
    state.dataGlobal->WarmupFlag = false;
    return;
  }

  bool convergenceChecksFailed = false;

  for (size_t i = 0; i < maxSurfTemp.size(); ++i) {
    const auto maxSurfTempDiff = std::abs(maxSurfTemp[i] - prevMaxSurfTemp[i]);
    if (maxSurfTempDiff > surfTempConvergTol) {
      convergenceChecksFailed = true;
      break;
    }
  }

  for (size_t i = 0; i < minSurfTemp.size(); ++i) {
    const auto minSurfTempDiff = std::abs(minSurfTemp[i] - prevMinSurfTemp[i]);
    if (minSurfTempDiff > surfTempConvergTol) {
      convergenceChecksFailed = true;
      break;
    }
  }

  if (convergenceChecksFailed) {
    state.dataGlobal->WarmupFlag = true;
    // EnergyPlus will have reset these values after convergence was satisfied 
    // Here they are restored to their last value before reset
    state.dataGlobal->DayOfSim = lastDayOfSim;
    state.dataGlobal->DayOfSimChr = lastDayOfSimChr;
  }
}

} // namespace spawn
