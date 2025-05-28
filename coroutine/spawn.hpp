#ifndef SPAWN_COROUTINE_SPAWN_H_
#define SPAWN_COROUTINE_SPAWN_H_

// C++ standard library headers
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

// EnergyPlus headers
#include "../energyplus/src/EnergyPlus/Data/CommonIncludes.hh"
#include "../energyplus/src/EnergyPlus/Data/EnergyPlusData.hh"
#include "../energyplus/src/EnergyPlus/api/state.h"

// Spawn project headers
#include "../util/filesystem.hpp"
#include "input/user_config.hpp"
#include "start_time.hpp"
#include "variables.hpp"
#include "warmup_manager.hpp"

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

  void Start();
  void Stop();
  [[nodiscard]] bool IsRunning() const noexcept;
  void SetTime(const double &time);

  [[nodiscard]] double ElapsedEnergyPlusTime() const;
  [[nodiscard]] double CurrentTime() const;
  [[nodiscard]] double NextEventTime() const;

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

  [[nodiscard]] double StartTime() const noexcept;
  void SetStartTime(const double &time) noexcept;

  void SetLogCallback(std::function<void(EnergyPlus::Error, const std::string &)> cb);
  void LogMessage(EnergyPlus::Error level, const std::string &message);
  void EmptyLogMessageQueue();

  void Exchange(const bool force = false);

private:
  // Member variables
  std::string instance_name_;  // Name of this Spawn instance
  spawn_fs::path idd_path_;    // Path to IDD file
  UserConfig user_config_;     // User configuration
  spawn_fs::path working_dir_; // Working directory

  spawn::StartTime start_time_; // Simulation start time
  double requested_time_{0.0};  // Requested simulation time
  bool need_update_{true};      // Flag indicating if an update is needed

  // Signal EnergyPlus to move through the simulation loop
  // Depending on the current simulation time, this may be an inner most hvac iteration,
  // or big "outer" zone iteration
  void Iterate();
  // Wait for EnergyPlus to complete any current iteration. ie. iterate_flag_ == false
  void Wait();
  // iterate_flag_ == true when EnergyPlus is actively working
  bool iterate_flag_{false};
  std::condition_variable iterate_cv_;
  // is_running_ is true for as long as the EnergyPlus process is running
  // in contrast to iterate_flag_ which is only true when EnergyPlus is actively
  // doing computation. In many cases the EnergyPlus process may be in wait mode,
  // waiting for the condition_variable (iterate_flag_) to signal an iteration
  bool is_running_{false};
  // Throws if not is_running_
  void IsRunningCheck() const;

  EnergyPlus::EnergyPlusData sim_state_;
  EnergyPlusState SimState();

  std::mutex sim_mutex_;
  std::thread sim_thread_;

  std::exception_ptr sim_exception_ptr_{nullptr};

  void ExternalHVACManager(EnergyPlusState state);
  void UpdateZoneConditions(bool skip_connected_zones);

  std::function<void(EnergyPlus::Error, const std::string &)> log_callback_;
  std::deque<std::pair<EnergyPlus::Error, std::string>> log_message_queue_;

  // Given a zone name, return the index according to EnergyPlus
  [[nodiscard]] int ZoneNum(const std::string &zone_name) const;

  // Time in seconds of the last zone update
  // This is required for computing the dt in the
  // updateZoneTemperature and updateZoneHumidityRatio calculations
  double prev_zone_update_{};
  // State of the warmup flag during the previous zone update
  // bool prev_warmup_flag_{false};

  // WarmupManager will register its own callbacks during construction
  // Maybe all of Spawn's implementation can be derived from "Manager" class
  // Maybe all of EnergyPlus can derive from Manager and the simulation is
  // a simple hierarchy of loops with callback points along the way
  WarmupManager warmup_manager_{sim_state_};

  variable::Variables variables_{user_config_};
};

spawn_fs::path IddPath();

} // namespace spawn

#endif // SPAWN_COROUTINE_SPAWN_H_
