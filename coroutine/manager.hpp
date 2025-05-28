#ifndef SPAWN_COROUTINE_MANAGER_H_
#define SPAWN_COROUTINE_MANAGER_H_

// C++ standard library headers
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
  Manager &operator=(Manager &) = delete;
  Manager &operator=(Manager &&) = delete;
  Manager(const Manager &) = delete;
  Manager(Manager &&) = delete;

protected:
  virtual void Initialize(EnergyPlus::EnergyPlusData &state);
  bool initialized_{false};
  void RegisterCallbacks(EnergyPlus::EnergyPlusData &state);

  std::map<EnergyPlus::EMSManager::EMSCallFrom, std::function<void(EnergyPlus::EnergyPlusData &)>> callbacks_;
};

} // namespace spawn

#endif  // SPAWN_COROUTINE_MANAGER_H_
