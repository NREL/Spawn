#include "./manager.hpp"
#include "../energyplus/src/EnergyPlus/PluginManager.hh"

namespace spawn {

Manager::Manager(EnergyPlus::EnergyPlusData &state)
{
  // register all of the other callbacks from derived classes
  // f is just a conversion from struct EnergyPlusData to EnergyPlusState (void *);
  // This is a firewall so that dirty void * doesn't spread
  auto f = [&](EnergyPlusState s) {
    auto *f_state = static_cast<EnergyPlus::EnergyPlusData *>(s);
    registerCallbacks(*f_state);
  };
  EnergyPlus::PluginManagement::registerNewCallback(state, EnergyPlus::EMSManager::EMSCallFrom::SetupSimulation, f);
}

void Manager::registerCallbacks(EnergyPlus::EnergyPlusData &state)
{
  for (const auto &callback : callbacks) {
    auto f = [&](EnergyPlusState s) {
      auto *f_state = static_cast<EnergyPlus::EnergyPlusData *>(s);
      callback.second(*f_state);
    };
    EnergyPlus::PluginManagement::registerNewCallback(state, callback.first, f);
  }
}

void Manager::initialize([[maybe_unused]] EnergyPlus::EnergyPlusData &state)
{
  initialized = true;
}

} // namespace spawn
