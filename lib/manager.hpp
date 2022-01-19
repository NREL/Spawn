#ifndef spawn_manager_hh_INCLUDED
#define spawn_manager_hh_INCLUDED

#include <functional>
#include <map>

using EnergyPlusState = void *;

namespace EnergyPlus {
struct EnergyPlusData;
namespace EMSManager {
  enum class EMSCallFrom;
} // namespace EMSManager
} // namespace EnergyPlus

namespace spawn {

class Manager
{

public:
  explicit Manager(EnergyPlus::EnergyPlusData &state);
  virtual ~Manager() = default;
  Manager &operator=(Manager &)  = delete;
  Manager &operator=(Manager &&) = delete;
  Manager(const Manager &) = delete;
  Manager(Manager &&) = delete;

protected:
  virtual void initialize(EnergyPlus::EnergyPlusData &state);
  bool initialized{false};
  void registerCallbacks(EnergyPlus::EnergyPlusData &state);

  std::map<EnergyPlus::EMSManager::EMSCallFrom, std::function<void(EnergyPlus::EnergyPlusData &)>> callbacks;
};

} // namespace spawn

#endif // spawn_manager_hh_INCLUDED
