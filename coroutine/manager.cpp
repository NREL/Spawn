#include "./manager.hpp"

// EnergyPlus headers
#include "../energyplus/src/EnergyPlus/PluginManager.hh"

namespace spawn {

Manager::Manager(EnergyPlus::EnergyPlusData &state)
{
  // register all of the other callbacks from derived classes
  // f is just a conversion from struct EnergyPlusData to EnergyPlusState (void *);
  // This is a firewall so that dirty void * doesn't spread
  auto f = [&](EnergyPlusState s) {
    auto *f_state = static_cast<EnergyPlus::EnergyPlusData *>(s);
    RegisterCallbacks(*f_state);
  };
  EnergyPlus::PluginManagement::registerNewCallback(state, EnergyPlus::EMSManager::EMSCallFrom::SetupSimulation, f);
}

void Manager::RegisterCallbacks(EnergyPlus::EnergyPlusData &state)
{
  for (const auto &callback : callbacks_) {
    auto f = [&](EnergyPlusState s) {
      auto *f_state = static_cast<EnergyPlus::EnergyPlusData *>(s);
      callback.second(*f_state);
    };
    EnergyPlus::PluginManagement::registerNewCallback(state, callback.first, f);
  }
}

void Manager::Initialize([[maybe_unused]] EnergyPlus::EnergyPlusData &state)
{
  initialized_ = true;
}

} // namespace spawn
