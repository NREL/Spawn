#ifndef spawn_manager_hh_INCLUDED
#define spawn_manager_hh_INCLUDED

#include <functional>
#include <map>

typedef void * EnergyPlusState;

namespace EnergyPlus {
  struct EnergyPlusData;
  namespace EMSManager {
    enum class EMSCallFrom;
  } // namespace EMSManager
} // namespace EnergyPlus

namespace spawn {

class Manager {

public:
  explicit Manager(EnergyPlus::EnergyPlusData & state);

protected:
  virtual void initialize(EnergyPlus::EnergyPlusData & state);
  bool initialized{false};
  void registerCallbacks(EnergyPlus::EnergyPlusData & state);

  std::map<EnergyPlus::EMSManager::EMSCallFrom, std::function<void (EnergyPlus::EnergyPlusData &)> > callbacks;
};

} // namespace spawn

#endif // spawn_manager_hh_INCLUDED
