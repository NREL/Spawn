%module libspawn

%include <std_string.i>
%include <std_wstring.i>

%{
#include "input/input.hpp"
#include "variables.hpp"
#include "warmupmanager.hpp"
#include "../energyplus/src/EnergyPlus/Data/EnergyPlusData.hh"
#include "../energyplus/src/EnergyPlus/Data/CommonIncludes.hh"
#include "../energyplus/src/EnergyPlus/api/state.h"
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
#include "../util/temp_directory.hpp"
#include "../util/math.hpp"


%}

namespace fs {
  class path
  {
  public:
    path(const std::string &path);
  };

  %extend path {
    std::string toString() const {
      return $self->generic_string();
    }

    // append to path
    path __div__(const path& other) const{
      return (*self) / other;
    }

    // append to path
    path __plus__(const path& other) const{
      return (*self) / other;
    }

    // append to path
    path append(const path& other) const{
      return (*self) / other;
    }

    // to std::string
    std::string __str__() const{
      return $self->generic_string();
    }

  };

};

namespace spawn {
  double days_to_seconds(double);
}


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
