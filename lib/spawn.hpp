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

#include "input.hpp"
#include "variables.hpp"
#include "zone.hpp"
#include <boost/filesystem.hpp>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <sstream>
#include <condition_variable>

namespace spawn {

enum class EPStatus { ADVANCE, NONE, TERMINATE, ERR, DONE };

class Spawn {
public:

  Spawn(const std::string & t_name, const std::string & t_input);
  Spawn( const Spawn& ) = delete;
  Spawn( Spawn&& ) = delete;
  bool operator==(const Spawn& other) const {
    return (this == &other);
  }

  int start(const double & starttime = 0.0);
  int stop();
  int setTime(const double & time);

  double currentSimTime() const;
  double nextSimTime() const;

  void exchange();

  // Set a variable identified by ref to the given value
  // Return true if the operation was successful
  bool setValue(const unsigned int & ref, const double & value);

  // Get the value of variable identified by ref
  // if ok parameter is given then it will be set to true if the operation was successful
  double getValue(const unsigned int & ref, bool & ok) const;

  double getValue(const unsigned int & ref) const;

  std::string instanceName;

  bool toleranceDefined;
  double tolerance;
  double startTime;
  bool stopTimeDefined;
  double stopTime;

private:
  // Wait for the EnergyPlus thread from the control thread
  // Return 0 on success
  // This should be called from the control thread
  int controlWait();

  struct ZoneSums {
    double tempDepCoef;
    double tempIndCoef;
  };

  // Given a zone name, return the index according to EnergyPlus
  int zoneNum(const std::string & zoneName) const;
  // These functions assume the EnergyPlus unit system
  ZoneSums zoneSums(const int zonenum) const;
  double zoneHeatTransfer(const int zonenum);
  void setZoneTemperature(const int zonenum, const double & temp);
  double zoneTemperature(const int zonenum);
  void updateZoneTemperature(const int zonenum, const double & dt);
  void updateZoneTemperatures(bool skipConnectedZones = false);

  void externalHVACManager();

  double requestedTime;
  std::thread simthread;
  EPStatus epstatus;
  std::condition_variable control_cv;
  std::mutex control_mutex;

  // Time in seconds of the last zone temperature update
  // This is required for computing the dt in the
  // updateZoneTemperature calculation
  double prevZoneTempUpdate;
  // State of the warmup flag during the previous zone temp update
  bool prevWarmupFlag;

  std::map<unsigned int, Variable> variables;
  Input input;
};

boost::filesystem::path exedir();
boost::filesystem::path iddpath();

} // namespace spawn

#endif

