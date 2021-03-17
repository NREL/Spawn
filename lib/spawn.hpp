// EnergyPlus, Copyright (c) 1996-2018, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef Spawn_hh_INCLUDED
#define Spawn_hh_INCLUDED

#include "input/input.hpp"
#include "variables.hpp"
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

namespace spawn {

class Spawn {
public:
  Spawn(const std::string & t_name, const std::string & t_input, const fs::path & workingdir = ".");
  Spawn( const Spawn& ) = delete;
  Spawn( Spawn&& ) = delete;
  bool operator==(const Spawn& other) const {
    return (this == &other);
  }

  void start();
  void stop();
  bool isRunning() const;
  void setTime(const double & time);

  double currentTime() const;
  double nextEventTime() const;

  // Set value by index
  // Throws a std::exception if index is invalid or the simulation is not running
  void setValue(const unsigned int & index, const double & value);
  // Get the value by index
  // Throws a std::exception if index is invalid or the simulation is not running
  double getValue(const unsigned int & index) const;

  // Get an index for a given variable name
  // Throws a std::exception if name is invalid or the simulation is not running
  unsigned int getIndex(const std::string & name) const;
  // Set value by name
  // Throws a std::exception if name is invalid or the simulation is not running
  void setValue(const std::string & name, const double & value);
  // Get the value by name
  // Throws a std::exception if name is invalid or the simulation is not running
  double getValue(const std::string & name) const;

  double startTime() const;
  void setStartTime(const double & time);

  void setLogCallback(std::function<void(EnergyPlus::Error, const std::string &)> cb);
  void logMessage(EnergyPlus::Error level, const std::string & message);

  void exchange();

private:
  std::string instanceName;
  fs::path workingdir;
  std::map<unsigned int, Variable> variables;
  Input input;

  double m_startTime{0.0};
  double requestedTime;

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
  std::pair<bool, float> externalSurfaceManager(EnergyPlusState state, int const surfaceNum);

  std::function<void(EnergyPlus::Error, const std::string &)> logCallback;
  std::deque<std::pair<EnergyPlus::Error, std::string> > log_message_queue;
  void emptyLogMessageQueue();

  // Given a surface name, return the index according to EnergyPlus
  int surfaceNum(const std::string & surfaceName) const;

  // Given a zone name, return the index according to EnergyPlus
  int zoneNum(const std::string & zoneName) const;
  // These functions assume the EnergyPlus unit system
  // ZoneSums are the coefficients used in the zone heat transfer calculation
  struct ZoneSums {
    double tempDepCoef;
    double tempIndCoef;
  };
  ZoneSums zoneSums(const int zonenum);
  double zoneHeatTransfer(const int zonenum);
  void setZoneTemperature(const int zonenum, const double & temp);
  double zoneTemperature(const int zonenum);
  void updateZoneTemperature(const int zonenum, const double & dt);
  void updateZoneTemperatures(bool skipConnectedZones = false);
  void initZoneEquip();
  // Time in seconds of the last zone temperature update
  // This is required for computing the dt in the
  // updateZoneTemperature calculation
  double prevZoneTempUpdate;
  // State of the warmup flag during the previous zone temp update
  bool prevWarmupFlag;

  int getVariableHandle(const std::string & name, const std::string & key);
  std::map<std::tuple<std::string, std::string>, int> variable_handle_cache;

  int getActuatorHandle(const std::string & componenttype, const std::string & controltype, const std::string & componentname);
  std::map<std::tuple<std::string, std::string, std::string>, int> actuator_handle_cache;
};

fs::path iddpath();

} // namespace spawn

#endif

