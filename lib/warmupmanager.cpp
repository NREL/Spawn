#include "./warmupmanager.hpp"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataGlobals.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataHeatBalSurface.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataHeatBalance.hh"

namespace spawn {

void WarmupManager::initialize(EnergyPlus::EnergyPlusData & state) {
  const auto count = state.dataHeatBalSurf->TempSurfIn.size();

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

  initialized = true;
}

void WarmupManager::endZoneTimestepAfterZoneReporting(EnergyPlus::EnergyPlusData & state) {
  if (! initialized) {
    initialize(state);
  }

  const auto & surfTemp = state.dataHeatBalSurf->TempSurfIn;
  lastDayOfSim = state.dataGlobal->DayOfSim;
  lastDayOfSimChr = state.dataGlobal->DayOfSimChr;

  if (state.dataGlobal->BeginDayFlag) {
    for (size_t i = 0; i < surfTemp.size(); ++i) {
      maxSurfTemp[i] = surfTemp[i];
      minSurfTemp[i] = surfTemp[i];
    }
  } else if (state.dataGlobal->EndDayFlag) {
    prevMaxSurfTemp = maxSurfTemp;
    prevMinSurfTemp = minSurfTemp;
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

// This calling point is immediatly after the normal EnergyPlus convergence check.
// In this function, additional spawn convergence checks are run, and the WarmupFlag
// is set back to true if necessary
// If this function is called the normal EnergyPlus convergence checks will have passed
void WarmupManager::beginNewEnvironmentAfterWarmUp(EnergyPlus::EnergyPlusData & state) {
  // If the max number of warmup days has been exceeded then return early,
  // and allow the simulation to continue into the run period
  if (state.dataGlobal->DayOfSim >= state.dataHeatBal->MaxNumberOfWarmupDays) {
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
