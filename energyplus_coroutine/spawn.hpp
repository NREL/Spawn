#ifndef Spawn_hh_INCLUDED
#define Spawn_hh_INCLUDED

#include "../energyplus/src/EnergyPlus/Data/CommonIncludes.hh"
#include "../energyplus/src/EnergyPlus/Data/EnergyPlusData.hh"
#include "../energyplus/src/EnergyPlus/api/state.h"
#include "../util/filesystem.hpp"
#include "input/input.hpp"
#include "start_time.hpp"
#include "variables.hpp"
#include "warmupmanager.hpp"
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
  Spawn(std::string t_name, const std::string &t_input, spawn_fs::path workingdir = ".");
  Spawn(const Spawn &) = delete;
  Spawn(Spawn &&) = delete;

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
  void setValue(unsigned int index, double value);
  // Get the value by index
  // Throws a std::exception if index is invalid or the simulation is not running
  double getValue(unsigned int index) const;

  // Get an index for a given variable name
  // Throws a std::exception if name is invalid or the simulation is not running
  [[nodiscard]] unsigned int getIndex(const std::string &name) const;
  // Set value by name
  // Throws a std::exception if name is invalid or the simulation is not running
  void setValue(const std::string &name, double value);
  // Get the value by name
  // Throws a std::exception if name is invalid or the simulation is not running
  [[nodiscard]] double getValue(const std::string &name) const;

  [[nodiscard]] double startTime() const noexcept;
  void setStartTime(const double &time) noexcept;

  void setLogCallback(std::function<void(EnergyPlus::Error, const std::string &)> cb);
  void logMessage(EnergyPlus::Error level, const std::string &message);
  void emptyLogMessageQueue();

  void exchange(const bool force = false);

private:
  std::string instanceName;
  spawn_fs::path workingdir;
  Input input;
  std::map<unsigned int, Variable> variables;

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

  std::function<void(EnergyPlus::Error, const std::string &)> logCallback;
  std::deque<std::pair<EnergyPlus::Error, std::string>> log_message_queue;

  // Given a surface name, return the index according to EnergyPlus
  [[nodiscard]] int surfaceNum(const std::string &surfaceName) const;

  // Given a zone name, return the index according to EnergyPlus
  [[nodiscard]] int zoneNum(const std::string &zoneName) const;
  // These functions assume the EnergyPlus unit system
  // ZoneSums are the coefficients used in the zone heat transfer calculation
  struct ZoneSums
  {
    double tempDepCoef;
    double tempIndCoef;
  };

  [[nodiscard]] ZoneSums zoneSums(const int zonenum);
  [[nodiscard]] double zoneHeatTransfer(const int zonenum);

  void setZoneTemperature(const int zonenum, const double temp);
  [[nodiscard]] double zoneTemperature(const int zonenum) const;
  void updateZoneTemperature(const int zonenum, const double dt);

  void setZoneHumidityRatio(const int zonenum, const double ratio);
  double zoneHumidityRatio(const int zonenum) const;
  void updateZoneHumidityRatio(const int zonenum, const double dt);

  void updateZoneConditions(bool skipConnectedZones = false);
  // Time in seconds of the last zone update
  // This is required for computing the dt in the
  // updateZoneTemperature and updateZoneHumidityRatio calculations
  double prevZoneUpdate{};
  // State of the warmup flag during the previous zone update
  bool prevWarmupFlag{false};

  void updateLatentGains();
  void initZoneEquip();

  [[nodiscard]] int getVariableHandle(const std::string &name, const std::string &key);
  std::map<std::tuple<std::string, std::string>, int> variable_handle_cache;

  [[nodiscard]] int
  getActuatorHandle(const std::string &componenttype, const std::string &controltype, const std::string &componentname);
  std::map<std::tuple<std::string, std::string, std::string>, int> actuator_handle_cache;
  void setActuatorValue(const std::string &componenttype,
                        const std::string &controltype,
                        const std::string &componentname,
                        const Real64 value);
  void
  resetActuator(const std::string &componenttype, const std::string &controltype, const std::string &componentname);

  double getSensorValue(Variable &var);

  void setInsideSurfaceTemperature(const int surfacenum, double temp);
  void setOutsideSurfaceTemperature(const int surfacenum, double temp);

  double getInsideSurfaceHeatFlow(const int surfacenum) const;
  double getOutsideSurfaceHeatFlow(const int surfacenum) const;

  // Initialize Variables that have fixed values that do not change during the simulation.
  // This should be called at the end of the ::start() sequence.
  void initConstParameters();

  // WarmupManager will register its own callbacks during construction
  // Maybe all of Spawn's implementation can be derived from "Manager" class
  // Maybe all of EnergyPlus can derive from Manager and the simulation is
  // a simple hierarchy of loops with callback points along the way
  WarmupManager warmupManager{sim_state};
};

spawn_fs::path iddpath();

} // namespace spawn

#endif
