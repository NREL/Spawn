%module libspawn


%{
#include "input/input.hpp"
#include "variables.hpp"
#include "warmupmanager.hpp"
#include "../submodules/EnergyPlus/src/EnergyPlus/Data/EnergyPlusData.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/Data/CommonIncludes.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/api/state.h"
#include "../util/filesystem.hpp"
#include <deque>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <tuple>
#include <mutex>
#include <sstream>
#include <condition_variable>
#include <functional>
#include "spawn.hpp"
%}

namespace spawn {
class Spawn {
public:
  Spawn(std::string t_name, std::string t_input, fs::path workingdir = ".");
  Spawn( const Spawn& ) = delete;
  Spawn( Spawn&& ) = delete;

  bool operator==(const Spawn& other) const noexcept;
  void start();
  void stop();
  bool isRunning() const noexcept;
  void setTime(const double time);

  double currentTime() const;
  double nextEventTime() const;

  void setValue(unsigned int index, double value);
  double getValue(unsigned int index) const;

  unsigned int getIndex(const std::string & name) const;
  void setValue(const std::string & name, double value);
  double getValue(const std::string & name) const;

  double startTime() const noexcept;
  void setStartTime(double time) noexcept;

  void setLogCallback(std::function<void(EnergyPlus::Error, const std::string &)> cb);
  void logMessage(EnergyPlus::Error level, const std::string & message);

  void exchange(const bool force = false);
};
}
