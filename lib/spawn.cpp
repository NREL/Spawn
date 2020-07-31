#include "spawn.hpp"
#include "outputtypes.hpp"
#include "input.hpp"
#include "../submodules/EnergyPlus/src/EnergyPlus/api/EnergyPlusPgm.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/api/runtime.h"
#include "../submodules/EnergyPlus/src/EnergyPlus/api/func.h"
#include "../submodules/EnergyPlus/src/EnergyPlus/api/datatransfer.h"
#include "../submodules/EnergyPlus/src/EnergyPlus/CommandLineInterface.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataEnvironment.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataGlobals.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataHeatBalance.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataHeatBalSurface.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataHeatBalFanSys.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataRuntimeLanguage.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataZoneEquipment.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/EMSManager.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/HeatBalanceAirManager.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/InternalHeatGains.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/OutputProcessor.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/Psychrometrics.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/ZoneTempPredictorCorrector.hh"

#if defined _WIN32
#include <windows.h>
#else
#include <stdio.h>
#include <dlfcn.h>
#endif

namespace spawn {

Spawn::Spawn(const std::string & t_name, const std::string & t_input, const boost::filesystem::path & t_workingdir)
  : instanceName(t_name),
    workingdir(t_workingdir),
    input(t_input)
{
  variables = parseVariables(input);
}

void Spawn::start(const double & starttime) {
  const auto & simulation = [&](){
    try {
      auto epwPath = input.epwInputPath().string();
      auto idfPath = input.idfInputPath().string();
      auto iddPath = iddpath().string();

      constexpr int argc = 8;
      const char * argv[argc];
      argv[0] = "energyplus";
      argv[1] = "-d";
      argv[2] = workingdir.string().c_str();
      argv[3] = "-w";
      argv[4] = epwPath.c_str();
      argv[5] = "-i";
      argv[6] = iddPath.c_str();
      argv[7] = idfPath.c_str();

      EnergyPlus::CommandLineInterface::ProcessArgs( sim_state, argc, argv );
      registerErrorCallback(std::bind(&Spawn::logMessage, this, std::placeholders::_1, std::placeholders::_2));
      EnergyPlus::DataGlobals::externalHVACManager = std::bind(&Spawn::externalHVACManager, this);

      RunEnergyPlus(sim_state);

      {
        std::unique_lock<std::mutex> lk(sim_mutex);
        iterate_flag = false;
        is_running = false;
      }
      iterate_cv.notify_one();
    } catch(...) {
      sim_exception_ptr = std::current_exception();
    }
  };

  is_running = true;
  requestedTime = 0.0;

  sim_thread = std::thread(simulation);

  // This will make the EnergyPlus simulation thread go through startup/warmup etc
  // and then go back to waiting
  iterate();

  // Move to the requested start time
  setTime(starttime);
}

void Spawn::wait() {
  std::unique_lock<std::mutex> lk(sim_mutex);
  iterate_cv.wait( lk, [&](){
    return (! iterate_flag) || (! is_running) || sim_exception_ptr;
  });

  if (sim_exception_ptr) {
    std::rethrow_exception(sim_exception_ptr);
  }
}

void Spawn::iterate() {
  // Wait for any current iteration to complete
  // There should never be a wait time (iterate_flag should be false)
  // Consider throw if iterate_flag == true instead
  wait();

  // Signal the iteration
  {
    std::unique_lock<std::mutex> lk(sim_mutex);
    iterate_flag = true;
  }
  iterate_cv.notify_one();

  // Wait for EnergyPlus to complete the iteration
  wait();

  emptyLogMessageQueue();
}

void Spawn::stop() {
  // This is an EnergyPlus API
  stopSimulation();
  // iterate the sim to allow EnergyPlus to go through shutdown;
  iterate();
  sim_thread.join();
}

bool Spawn::isRunning() const {
  return is_running;
}

void Spawn::isRunningCheck() const {
  if (! is_running) {
    throw std::runtime_error("EnergyPlus is not running");
  }
}

void Spawn::setTime(const double & time)
{
  isRunningCheck();

  requestedTime = time;
  exchange();

  if(requestedTime >= nextEventTime()) {
    iterate();
  }
}

double Spawn::currentTime() const {
  isRunningCheck();
  return (EnergyPlus::DataGlobals::SimTimeSteps - 1) * EnergyPlus::DataGlobals::TimeStepZoneSec;
}

double Spawn::nextEventTime() const {
  isRunningCheck();
  return EnergyPlus::DataGlobals::SimTimeSteps * EnergyPlus::DataGlobals::TimeStepZoneSec;
}

void Spawn::setValue(const unsigned int & ref, const double & value) {
  isRunningCheck();
  auto var = variables.find(ref);
  if( var != variables.end() ) {
    var->second.setValue(value, spawn::units::UnitSystem::MO);
  } else {
    throw std::runtime_error(fmt::format("Attempt to set a value using an invalid reference: {}", ref));
  }
}

double Spawn::getValue(const unsigned int & ref) const {
  isRunningCheck();
  auto var = variables.find(ref);
  if( var != variables.end() ) {
    if( var->second.isValueSet() ) {
      return var->second.getValue(spawn::units::UnitSystem::MO);
    } else {
      throw std::runtime_error(fmt::format("Attempt to get a value for ref {}, which has no value set", ref));
    }
  } else {
    throw std::runtime_error(fmt::format("Attempt to get a value using an invalid reference: {}", ref));
  }
}

Spawn::ZoneSums Spawn::zoneSums(const int zonenum) const {
  Real64 SumIntGain{0.0}; // Zone sum of convective internal gains
  Real64 SumHA{0.0};      // Zone sum of Hc*Area
  Real64 SumHATsurf{0.0}; // Zone sum of Hc*Area*Tsurf
  Real64 SumHATref{0.0};  // Zone sum of Hc*Area*Tref, for ceiling diffuser convection correlation
  Real64 SumMCp{0.0};     // Zone sum of MassFlowRate*Cp
  Real64 SumMCpT{0.0};    // Zone sum of MassFlowRate*Cp*T
  Real64 SumSysMCp{0.0};  // Zone sum of air system MassFlowRate*Cp
  Real64 SumSysMCpT{0.0}; // Zone sum of air system MassFlowRate*Cp*T

  EnergyPlus::ZoneTempPredictorCorrector::CalcZoneSums(zonenum, SumIntGain, SumHA, SumHATsurf, SumHATref, SumMCp, SumMCpT, SumSysMCp, SumSysMCpT);

  Spawn::ZoneSums sums;

  sums.tempDepCoef = SumHA;                   // + SumMCp;
  sums.tempIndCoef = SumIntGain + SumHATsurf; // - SumHATref + SumMCpT;

  return sums;
}

void Spawn::setZoneTemperature(const int zonenum, const double & temp) {
  // Is it necessary to update all of these or can we
  // simply update ZT and count on EnergyPlus::HeatBalanceAirManager::ReportZoneMeanAirTemp()
  // to propogate the other variables?
  EnergyPlus::DataHeatBalFanSys::ZT( zonenum ) = temp;
  EnergyPlus::DataHeatBalFanSys::ZTAV( zonenum ) = temp;
  EnergyPlus::DataHeatBalFanSys::MAT( zonenum ) = temp;
}

double Spawn::zoneTemperature(const int zonenum) {
  return EnergyPlus::DataHeatBalFanSys::ZT(zonenum);
}

void Spawn::updateZoneTemperature(const int zonenum, const double & dt) {
  // Based on the EnergyPlus analytical method
  // See ZoneTempPredictorCorrector::CorrectZoneAirTemp
  const auto & zonetemp = zoneTemperature(zonenum);
  double newzonetemp = zonetemp;

  const auto aircap =
    EnergyPlus::DataHeatBalance::Zone(zonenum).Volume *
    EnergyPlus::DataHeatBalance::Zone(zonenum).ZoneVolCapMultpSens *
    EnergyPlus::Psychrometrics::PsyRhoAirFnPbTdbW(EnergyPlus::DataEnvironment::OutBaroPress, zonetemp, EnergyPlus::DataHeatBalFanSys::ZoneAirHumRat(zonenum),"") *
    EnergyPlus::Psychrometrics::PsyCpAirFnW(EnergyPlus::DataHeatBalFanSys::ZoneAirHumRat(zonenum));// / (TimeStepSys * SecInHour);

  const auto & sums = zoneSums(zonenum);
  if (sums.tempDepCoef == 0.0) { // B=0
      newzonetemp = zonetemp + sums.tempIndCoef / aircap * dt;
  } else {
      newzonetemp = (zonetemp - sums.tempIndCoef / sums.tempDepCoef) * std::exp(min(700.0, -sums.tempDepCoef / aircap * dt)) +
        sums.tempIndCoef / sums.tempDepCoef;
  }

  setZoneTemperature(zonenum, newzonetemp);
}

void Spawn::updateZoneTemperatures(bool skipConnectedZones) {
  // Check for...
  if(
    // 1. The beginning of the environment
    EnergyPlus::DataGlobals::BeginEnvrnFlag ||
    // 2. The first call after warmup
    (prevWarmupFlag && ! (EnergyPlus::DataGlobals::WarmupFlag)))
  {
    prevZoneTempUpdate = currentTime();
  } else {
    const double dt = currentTime() - prevZoneTempUpdate;
    prevZoneTempUpdate = currentTime();

    for(const auto & zone : input.zones) {
      if(skipConnectedZones && zone.isconnected) {
        continue;
      }

      const auto zonenum = zoneNum(zone.idfname);
      updateZoneTemperature(zonenum, dt);
    }
  }

  prevWarmupFlag = EnergyPlus::DataGlobals::WarmupFlag;
}

double Spawn::zoneHeatTransfer(const int zonenum) {
  const auto & sums = zoneSums(zonenum);
  // Refer to
  // https://bigladdersoftware.com/epx/docs/8-8/engineering-reference/basis-for-the-zone-and-air-system-integration.html#basis-for-the-zone-and-air-system-integration
  const auto heatTransfer = sums.tempIndCoef - (sums.tempDepCoef * EnergyPlus::DataHeatBalFanSys::MAT(zonenum));
  return heatTransfer;
}

int Spawn::zoneNum(const std::string & zoneName) const {
  auto upperZoneName = zoneName;
  std::transform(zoneName.begin(), zoneName.end(), upperZoneName.begin(), ::toupper);
  for ( int i = 0; i < EnergyPlus::DataGlobals::NumOfZones; ++i ) {
    if ( EnergyPlus::DataHeatBalance::Zone[i].Name == upperZoneName ) {
      return i + 1;
    }
  }

  return 0;
}

void Spawn::exchange()
{
  isRunningCheck();

  // Uses the EnergyPlus api getVariableHandle, but throws if the variable does not exist
  auto spawnGetVariableHandle = [](const std::string & name, const std::string & key) {
    const auto h = getVariableHandle(name.c_str(), key.c_str());
    if (h == -1) {
      throw std::runtime_error(fmt::format("Attempt to get invalid variable using name '{}', and key '{}'", name, key));
    }
    return h;
  };

  // Uses the EnergyPlus api getActuatorHandle, but throws if the actuator does not exist
  auto spawnGetActuatorHandle = [](const std::string & componenttype, const std::string & controltype, const std::string & componentname) {
    const auto h = getActuatorHandle(componenttype.c_str(), controltype.c_str(), componentname.c_str());
    if (h == -1) {
      throw std::runtime_error(
          fmt::format(
            "Attempt to get invalid actuator using component type '{}', component name '{}', and control type {}",
            componenttype,
            componentname,
            controltype
          )
      );
    }
    return h;
  };

  auto spawnGetSensorValue = [&](Variable & var) {
    const auto h = spawnGetVariableHandle(var.outputvarname, var.outputvarkey);
    return getVariableValue(h);
  };

  auto spawnSetActuatorValue = [&](const std::string & componenttype, const std::string & controltype, const std::string & componentname, const Real64 & value) {
    const auto h = spawnGetActuatorHandle(componenttype, controltype, componentname);
    setActuatorValue(h, value);
  };

  auto spawnResetActuator = [&](const std::string & componenttype, const std::string & controltype, const std::string & componentname) {
    const auto h = spawnGetActuatorHandle(componenttype, controltype, componentname);
    resetActuator(h);
  };

  auto actuateVar = [&](const Variable & var) {
    if( var.isValueSet() ) {
      spawnSetActuatorValue(
          var.actuatorcomponenttype,
          var.actuatorcontroltype,
          var.actuatorcomponentkey,
          var.getValue(spawn::units::UnitSystem::EP)
      );
    } else {
      spawnResetActuator(
          var.actuatorcomponenttype,
          var.actuatorcontroltype,
          var.actuatorcomponentkey
      );
    }
  };

  // Update inputs first, then outputs so that we can do some updates within EnergyPlus
  for( auto & varmap : variables ) {
    auto & var = varmap.second;
    auto varZoneNum = zoneNum(var.name);
    switch ( var.type ) {
      case VariableType::T:
        if( var.isValueSet() ) {
          const auto & v = var.getValue(spawn::units::UnitSystem::EP);
          setZoneTemperature(varZoneNum, v);
        }
        break;
      case VariableType::EMS_ACTUATOR:
        actuateVar(var);
        break;
      case VariableType::SCHEDULE:
        actuateVar(var);
      default:
        break;
    }
  }

  updateZoneTemperatures(true); // true means skip any connected zones which are not under EP control

  // Run some internal EnergyPlus functions to update outputs
  EnergyPlus::HeatBalanceAirManager::ReportZoneMeanAirTemp();
  EnergyPlus::InternalHeatGains::InitInternalHeatGains(sim_state);

  // Now update the outputs
  for( auto & varmap : variables ) {
    auto & var = varmap.second;
    auto varZoneNum = zoneNum(var.name);
    switch ( var.type ) {
      case VariableType::TRAD:
        var.setValue(EnergyPlus::DataHeatBalSurface::ZoneMRT( varZoneNum ), spawn::units::UnitSystem::EP);
        break;
      case VariableType::V:
        var.setValue(EnergyPlus::DataHeatBalance::Zone( varZoneNum ).Volume, spawn::units::UnitSystem::EP);
        break;
      case VariableType::AFLO:
        var.setValue(EnergyPlus::DataHeatBalance::Zone( varZoneNum ).FloorArea, spawn::units::UnitSystem::EP);
        break;
      case VariableType::MSENFAC:
        var.setValue(EnergyPlus::DataHeatBalance::Zone( varZoneNum ).ZoneVolCapMultpSens, spawn::units::UnitSystem::EP);
        break;
      case VariableType::QCONSEN_FLOW:
        var.setValue(zoneHeatTransfer( varZoneNum ), spawn::units::UnitSystem::EP);
        break;
      case VariableType::SENSOR:
        var.setValue(spawnGetSensorValue(var), spawn::units::UnitSystem::EP);
        break;
      default:
        break;
    }
  }
}

void Spawn::initZoneEquip() {
  if (!EnergyPlus::DataZoneEquipment::ZoneEquipInputsFilled) {
    EnergyPlus::DataZoneEquipment::GetZoneEquipmentData(sim_state);
    EnergyPlus::DataZoneEquipment::ZoneEquipInputsFilled = true;
  }
}

void Spawn::externalHVACManager() {
  // Although we do not use the ZoneTempPredictorCorrector,
  // some global variables need to be initialized by InitZoneAirSetPoints
  // This is protected by a one time flag so that it will only happen once
  // during the simulation
  EnergyPlus::ZoneTempPredictorCorrector::InitZoneAirSetPoints();

  // Likewise init zone equipment one time
  initZoneEquip();

  // At this time, there is no data exchange or any other
  // interaction with the FMU while KickOffSimulation is true.
  // Sensors and actuators may not be setup at this point, so exchange
  // might trigger exceptions
  if( EnergyPlus::DataGlobals::KickOffSimulation ) {
    return;
  }

  // Exchange data with the FMU
  exchange();

  // There is no interaction with the FMU during warmup,
  // so return now before signaling
  if( EnergyPlus::DataGlobals::DoingSizing || EnergyPlus::DataGlobals::WarmupFlag ) {
    return;
  }

  // Only signal and wait for input if the current sim time is greather than or equal
  // to the requested time
  if( currentTime() >= requestedTime ) {
    // Signal the end of the step
    {
      std::unique_lock<std::mutex> lk(sim_mutex);
      iterate_flag = false;
    }

    iterate_cv.notify_one();

    // Wait for the iterate_flag to signal another iteration
    std::unique_lock<std::mutex> lk(sim_mutex);
    iterate_cv.wait(lk, [&]() {
      return iterate_flag;
    });
  }
}

void Spawn::setLogCallback(std::function<void(EnergyPlus::Error, const std::string &)> cb) {
  logCallback = cb;
}

void Spawn::logMessage(EnergyPlus::Error level, const std::string & message) {
  if (logCallback) {
    log_message_queue.push_back(std::make_pair(level, message));
  }
}

void Spawn::emptyLogMessageQueue() {
  if (logCallback) {
    while(! log_message_queue.empty()) {
      auto m = log_message_queue.front();
      logCallback(m.first, m.second);
      log_message_queue.pop_front();
    }
  }
}

boost::filesystem::path exedir() {
  #if _WIN32
    TCHAR szPath[MAX_PATH];
    GetModuleFileName(nullptr, szPath, MAX_PATH);
    return boost::filesystem::path(szPath).parent_path();
  #else
    Dl_info info;
    dladdr("main", &info);
    return boost::filesystem::path(info.dli_fname).parent_path();
  #endif
}

boost::filesystem::path iddpath() {
  constexpr auto & iddfilename = "Energy+.idd";
  auto iddInputPath = exedir() / "../../resources" / iddfilename;

  if (! boost::filesystem::exists(iddInputPath)) {
    iddInputPath = exedir() / iddfilename;
  }

  return iddInputPath;
}

} // namespace spawn

