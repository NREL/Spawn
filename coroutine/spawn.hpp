#ifndef Spawn_hh_INCLUDED
#define Spawn_hh_INCLUDED

#include "../energyplus/src/EnergyPlus/Data/CommonIncludes.hh"
#include "../energyplus/src/EnergyPlus/Data/EnergyPlusData.hh"
#include "../energyplus/src/EnergyPlus/api/state.h"
#include "../util/filesystem.hpp"
#include "input/user_config.hpp"
#include "start_time.hpp"
#include "variables.hpp"
#include "warmup_manager.hpp"
#include <condition_variable>
#include <deque>
#include <functional>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

namespace spawn {

class Spawn
{
public:
  Spawn(const std::string_view name,
        spawn_fs::path idd_path,
        const std::string_view user_config,
        spawn_fs::path working_dir = ".");
  Spawn(const Spawn &) = delete;
  Spawn(Spawn &&) = delete;
  Spawn &operator=(const Spawn &) = delete;
  Spawn &operator=(Spawn &&) = delete;
  ~Spawn() = default;

  [[nodiscard]] bool operator==(const Spawn &other) const noexcept
  {
    return (this == &other);
  }

  void start();
  void stop();
  [[nodiscard]] bool isRunning() const noexcept;
  void setTime(const double &time);

  [[nodiscard]] double ElapsedEnergyPlusTime() const;
  [[nodiscard]] double currentTime() const;
  [[nodiscard]] double nextEventTime() const;

  // Set value by index
  // Throws a std::exception if index is invalid or the simulation is not running
  void SetValue(const unsigned int index, const double &value);

  // Get the value by index
  // Throws a std::exception if index is invalid or the simulation is not running
  [[nodiscard]] double GetValue(const unsigned int index) const;

  // Get an index for a given variable name
  // Throws a std::exception if name is invalid or the simulation is not running
  [[nodiscard]] unsigned int GetIndex(const std::string_view name) const;

  // Set value by name
  // Throws a std::exception if name is invalid or the simulation is not running
  void SetValue(const std::string_view name, const double &value);

  // Get the value by name
  // Throws a std::exception if name is invalid or the simulation is not running
  [[nodiscard]] double GetValue(const std::string_view name) const;

  [[nodiscard]] double startTime() const noexcept;
  void setStartTime(const double &time) noexcept;

  void setLogCallback(std::function<void(EnergyPlus::Error, const std::string &)> cb);
  void logMessage(EnergyPlus::Error level, const std::string &message);
  void emptyLogMessageQueue();

  void exchange(const bool force = false);

private:
  std::string instance_name_;
  spawn_fs::path idd_path_;
  UserConfig user_config_;
  spawn_fs::path working_dir_;

  StartTime start_time_;
  double requested_time_{0.0};
  bool need_update{true};

  // Signal EnergyPlus to move through the simulation loop
  // Depending on the current simulation time, this may be an inner most hvac iteration,
  // or big "outer" zone iteration
  void iterate();
  // Wait for EnergyPlus to complete any current iteration. ie. iterate_flag == false
  void wait();
  // iterate flag == true when EnergyPlus is actively working
  bool iterate_flag{false};
  std::condition_variable iterate_cv;
  // is_running is true for as long as the EnergyPlus process is running
  // in contrast to iterate_flag which is only true when EnergyPlus is actively
  // doing computation. In many cases the EnergyPlus process may be in wait mode,
  // waiting for the condition_variable (iterate_flag) to signal an iteration
  bool is_running{false};
  // Throws if not is_running
  void isRunningCheck() const;

  EnergyPlus::EnergyPlusData sim_state;
  EnergyPlusState simState();

  std::mutex sim_mutex;
  std::thread sim_thread;

  std::exception_ptr sim_exception_ptr{nullptr};

  void externalHVACManager(EnergyPlusState state);
  void UpdateZoneConditions(bool skipConnectedZones);

  std::function<void(EnergyPlus::Error, const std::string &)> logCallback;
  std::deque<std::pair<EnergyPlus::Error, std::string>> log_message_queue;

  // Given a zone name, return the index according to EnergyPlus
  [[nodiscard]] int zoneNum(const std::string &zoneName) const;

  // Time in seconds of the last zone update
  // This is required for computing the dt in the
  // updateZoneTemperature and updateZoneHumidityRatio calculations
  double prevZoneUpdate{};
  // State of the warmup flag during the previous zone update
  // bool prevWarmupFlag{false};

  // WarmupManager will register its own callbacks during construction
  // Maybe all of Spawn's implementation can be derived from "Manager" class
  // Maybe all of EnergyPlus can derive from Manager and the simulation is
  // a simple hierarchy of loops with callback points along the way
  WarmupManager warmupManager{sim_state};

  variable::Variables variables_{user_config_};
};

spawn_fs::path iddpath();

} // namespace spawn

#endif
