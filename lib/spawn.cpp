#include "spawn.hpp"
#include "outputtypes.hpp"
#include "../submodules/EnergyPlus/src/EnergyPlus/api/EnergyPlusPgm.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/api/runtime.h"
#include "../submodules/EnergyPlus/src/EnergyPlus/api/datatransfer.h"
#include "../submodules/EnergyPlus/src/EnergyPlus/CommandLineInterface.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataEnvironment.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataGlobals.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataHeatBalance.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataHeatBalFanSys.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataRuntimeLanguage.hh"
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

Spawn::Spawn(const std::string & t_name, const std::string & t_input)
  : instanceName(t_name),
    input(t_input)
{
  variables = parseVariables(input);
}

int Spawn::start(const double & starttime) {
  const auto & simulation = [&](){
    auto workingPath = boost::filesystem::path(instanceName).filename().string();
    auto epwPath = input.epwInputPath().string();
    auto idfPath = input.idfInputPath().string();
    auto iddPath = iddpath().string();

    constexpr int argc = 8;
    const char * argv[argc];
    argv[0] = "energyplus";
    argv[1] = "-d";
    argv[2] = workingPath.c_str();
    argv[3] = "-w";
    argv[4] = epwPath.c_str();
    argv[5] = "-i";
    argv[6] = iddPath.c_str();
    argv[7] = idfPath.c_str();

    EnergyPlus::CommandLineInterface::ProcessArgs( argc, argv );
    EnergyPlus::DataGlobals::externalHVACManager = std::bind(&Spawn::externalHVACManager, this);

    try {
      auto result = RunEnergyPlus();
      std::unique_lock<std::mutex> lk(control_mutex);
      epstatus = result ? EPStatus::ERR : EPStatus::DONE;
    } catch(...) {
      epstatus = EPStatus::ERR;
    }
    control_cv.notify_one();
  };

  {
    std::unique_lock<std::mutex> lk(control_mutex);
    requestedTime = 0.0;
    epstatus = EPStatus::ADVANCE;
  }

  simthread = std::thread(simulation);

  // Wait for EnergyPlus to go through startup/warmup etc
  auto result = controlWait();

  // Move to the requested start time
	if(result == 0) {
    result = setTime(starttime);
  }

  return result;
}

int Spawn::stop() {
  {
    std::unique_lock<std::mutex> lk(control_mutex);
    epstatus = EPStatus::TERMINATE;
  }

  try {
    stopSimulation();
    control_cv.notify_one();
    simthread.join();
    return 0;
  } catch(...) {
    return 1;
  }
}

int Spawn::setTime(const double & time)
{
  requestedTime = time;

  exchange();

  if(requestedTime >= nextSimTime()) {
    {
      std::unique_lock<std::mutex> lk(control_mutex);
      epstatus = EPStatus::ADVANCE;
    }

    // Notify E+ to advance
    control_cv.notify_one();

    // Wait for EnergyPlus to complete the step
    return controlWait();
  } else {
    return 0;
  }
}

double Spawn::currentSimTime() const {
  return (EnergyPlus::DataGlobals::SimTimeSteps - 1) * EnergyPlus::DataGlobals::TimeStepZoneSec;
}

double Spawn::nextSimTime() const {
  return EnergyPlus::DataGlobals::SimTimeSteps * EnergyPlus::DataGlobals::TimeStepZoneSec;
}

bool Spawn::setValue(const unsigned int & ref, const double & value) {
  auto var = variables.find(ref);
  if( var != variables.end() ) {
    var->second.setValue(value, spawn::units::UnitSystem::MO);
    return true;
  }

  return false;
}

double Spawn::getValue(const unsigned int & ref, bool & ok) const {
  auto var = variables.find(ref);
  if( var != variables.end() ) {
    if( var->second.isValueSet() ) {
      ok = true;
      return var->second.getValue(spawn::units::UnitSystem::MO);
    }
  }
  ok = false;
  return 0.0;
}

double Spawn::getValue(const unsigned int & ref) const {
  bool ok = false;
  return getValue(ref, ok);
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
    prevZoneTempUpdate = currentSimTime();
  } else {
    const double dt = currentSimTime() - prevZoneTempUpdate;
    prevZoneTempUpdate = currentSimTime();

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
  const auto getSensorValue = [&](Variable & var) {
    const auto & h = getVariableHandle(var.outputvarname.c_str(), var.outputvarkey.c_str());
    return getVariableValue(h);
  };

  const auto compSetActuatorValue = [](const std::string & key, const std::string & componenttype, const std::string & controltype, const Real64 & value) {
    const auto & h = getActuatorHandle(componenttype.c_str(), controltype.c_str(), key.c_str());
    setActuatorValue(h, value);
  };

  const auto compResetActuator = [](const std::string & key, const std::string & componenttype, const std::string & controltype) {
    const auto & h = getActuatorHandle(componenttype.c_str(), controltype.c_str(), key.c_str());
    resetActuator(h);
  };

  const auto actuateVar = [&](const Variable & var) {
    if( var.isValueSet() ) {
      compSetActuatorValue(
          var.actuatorcomponentkey,
          var.actuatorcomponenttype,
          var.actuatorcontroltype,
          var.getValue(spawn::units::UnitSystem::EP)
      );
    } else {
      compResetActuator(
          var.actuatorcomponentkey,
          var.actuatorcomponenttype,
          var.actuatorcontroltype
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
  EnergyPlus::InternalHeatGains::InitInternalHeatGains();

  // Now update the outputs
  for( auto & varmap : variables ) {
    auto & var = varmap.second;
    auto varZoneNum = zoneNum(var.name);
    switch ( var.type ) {
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
        var.setValue(getSensorValue(var), spawn::units::UnitSystem::EP);
        break;
      default:
        break;
    }
  }
}

int Spawn::controlWait() {
  std::unique_lock<std::mutex> lk(control_mutex);
  // Wait until EnergyPlus is not Advancing or Terminating (ie in the process of cleanup)
  control_cv.wait( lk, [&](){
    return
        (epstatus == EPStatus::NONE) ||
        (epstatus == EPStatus::ERR) ||
        (epstatus == EPStatus::DONE);
  });
  return epstatus == EPStatus::ERR ? 1 : 0;
}

void Spawn::externalHVACManager() {
  // Although we do not use the ZoneTempPredictorCorrector,
  // some global variables need to be initialized by InitZoneAirSetPoints
  // This is protected by a one time flag so that it will only happen once
  // during the simulation
  EnergyPlus::ZoneTempPredictorCorrector::InitZoneAirSetPoints();

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
  if( currentSimTime() >= requestedTime ) {
    // Signal the end of the step
    {
      std::unique_lock<std::mutex> lk(control_mutex);
      if (epstatus != ::spawn::EPStatus::TERMINATE)
      {
       epstatus = ::spawn::EPStatus::NONE;
      }
    }

    control_cv.notify_one();

    // Wait for the epstatus to advance again
    std::unique_lock<std::mutex> lk(control_mutex);
    control_cv.wait(lk, [&]() {
      return (
        (epstatus == ::spawn::EPStatus::ADVANCE) ||
        (epstatus == ::spawn::EPStatus::TERMINATE)
      );
    });
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

