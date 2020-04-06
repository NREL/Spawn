#include "spawn.hpp"
#include "outputtypes.hpp"
#include "../submodules/EnergyPlus/src/EnergyPlus/api/EnergyPlusPgm.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/api/runtime.h"
#include "../submodules/EnergyPlus/src/EnergyPlus/api/datatransfer.h"
#include "../submodules/EnergyPlus/src/EnergyPlus/CommandLineInterface.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataGlobals.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataHeatBalance.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataHeatBalFanSys.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataRuntimeLanguage.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/EMSManager.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/HeatBalanceAirManager.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/InternalHeatGains.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/OutputProcessor.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/ZoneTempPredictorCorrector.hh"

Spawn::Spawn(const std::string & name, const boost::filesystem::path & resourcePath)
  : instanceName(name)
{
  boost::system::error_code ec;
  for ( const auto & entry : boost::filesystem::directory_iterator(resourcePath, ec) ) {
    if (ec.value() == boost::system::errc::success) {
      const auto path = entry.path();
      const auto extension = path.extension();
      if ( extension == ".idf" ) {
        idfInputPath = path.string();
      } else if ( extension == ".epw" ) {
        weatherFilePath = path.string();
      } else if ( extension == ".idd" ) {
        iddPath = path.string();
      } else if ( extension == ".spawn" ) {
        spawnInputPath = path.string();
      } else if ( extension == ".json" ) {
        spawnInputPath = path.string();
      }
    }
  }

  variables = parseVariables(idfInputPath, spawnInputPath);
}

int Spawn::start(const double & starttime) {
  const auto & simulation = [&](){
    auto workingPath = boost::filesystem::path(instanceName).filename();

    constexpr int argc = 8;
    const char * argv[argc];
    argv[0] = "energyplus";
    argv[1] = "-d";
    argv[2] = workingPath.string().c_str();
    argv[3] = "-w";
    argv[4] = weatherFilePath.c_str();
    argv[5] = "-i";
    argv[6] = iddPath.c_str();
    argv[7] = idfInputPath.c_str();

    EnergyPlus::CommandLineInterface::ProcessArgs( argc, argv );
    EnergyPlus::DataGlobals::externalHVACManager = std::bind(&Spawn::externalHVACManager, this);
    try {
      auto result = RunEnergyPlus();
      std::unique_lock<std::mutex> lk(control_mutex);
      epstatus = result ? EPStatus::ERROR : EPStatus::DONE;
    } catch(...) {
      epstatus = EPStatus::ERROR;
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

  {
    std::unique_lock<std::mutex> lk(control_mutex);
    epstatus = EPStatus::ADVANCE;
  }

  // Notify E+ to advance
  control_cv.notify_one();

  // Wait for EnergyPlus to complete the step
  return controlWait();
}

double Spawn::currentSimTime() const {
  return EnergyPlus::DataGlobals::SimTimeSteps * EnergyPlus::DataGlobals::TimeStepZoneSec;
}

double Spawn::nextSimTime() const {
  return (EnergyPlus::DataGlobals::SimTimeSteps + 1) * EnergyPlus::DataGlobals::TimeStepZoneSec;
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

double Spawn::zoneHeatTransfer(const int ZoneNum)
{
  Real64 SumIntGain{0.0}; // Zone sum of convective internal gains
  Real64 SumHA{0.0};      // Zone sum of Hc*Area
  Real64 SumHATsurf{0.0}; // Zone sum of Hc*Area*Tsurf
  Real64 SumHATref{0.0};  // Zone sum of Hc*Area*Tref, for ceiling diffuser convection correlation
  Real64 SumMCp{0.0};     // Zone sum of MassFlowRate*Cp
  Real64 SumMCpT{0.0};    // Zone sum of MassFlowRate*Cp*T
  Real64 SumSysMCp{0.0};  // Zone sum of air system MassFlowRate*Cp
  Real64 SumSysMCpT{0.0}; // Zone sum of air system MassFlowRate*Cp*T

  EnergyPlus::ZoneTempPredictorCorrector::CalcZoneSums(ZoneNum, SumIntGain, SumHA, SumHATsurf, SumHATref, SumMCp, SumMCpT, SumSysMCp, SumSysMCpT);

  const auto TempDepCoef = SumHA;                   // + SumMCp;
  const auto TempIndCoef = SumIntGain + SumHATsurf; // - SumHATref + SumMCpT;

  // Refer to
  // https://bigladdersoftware.com/epx/docs/8-8/engineering-reference/basis-for-the-zone-and-air-system-integration.html#basis-for-the-zone-and-air-system-integration
  const auto heatTransfer = TempIndCoef - (TempDepCoef * EnergyPlus::DataHeatBalFanSys::MAT(ZoneNum));

  return heatTransfer;
}

void Spawn::exchange()
{
  const auto zoneNum = [](std::string & zoneName) {
    std::transform(zoneName.begin(), zoneName.end(), zoneName.begin(), ::toupper);
    for ( int i = 0; i < EnergyPlus::DataGlobals::NumOfZones; ++i ) {
      if ( EnergyPlus::DataHeatBalance::Zone[i].Name == zoneName ) {
        return i + 1;
      }
    }

    return 0;
  };

  const auto getSensorValue = [&](Variable & var) {
    const auto & h = getVariableHandle(var.outputvarname.c_str(), var.outputvarkey.c_str());
    return getVariableValue(h);
  };

  const auto compSetActuatorValue = [](const std::string & key, const std::string & componenttype, const std::string & controltype, const Real64 & value) {
    const auto & h = getActuatorHandle(key.c_str(), componenttype.c_str(), controltype.c_str());
    setActuatorValue(h, value);
  };

  const auto compResetActuator = [](const std::string & key, const std::string & componenttype, const std::string & controltype) {
    const auto & h = getActuatorHandle(key.c_str(), componenttype.c_str(), controltype.c_str());
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
          EnergyPlus::DataHeatBalFanSys::ZT( varZoneNum ) = v;
          EnergyPlus::DataHeatBalFanSys::ZTAV( varZoneNum ) = v;
          EnergyPlus::DataHeatBalFanSys::MAT( varZoneNum ) = v;
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
        (epstatus == EPStatus::ERROR) ||
        (epstatus == EPStatus::DONE);
  });
  return epstatus == EPStatus::ERROR ? 1 : 0;
}

void Spawn::externalHVACManager() {
  // Although we do not use the ZoneTempPredictorCorrector,
  // some global variables need to be initialized by InitZoneAirSetPoints
  // This is protected by a one time flag so that it will only happen once
  // during the simulation
  EnergyPlus::ZoneTempPredictorCorrector::InitZoneAirSetPoints();

  // At this time, there is no data exchange or any other
  // interaction with the FMU during wramup and sizing.
  if( EnergyPlus::DataGlobals::DoingSizing || EnergyPlus::DataGlobals::WarmupFlag ) {
    return;
  }

  if( EnergyPlus::DataGlobals::KickOffSimulation ) {
    return;
  }

  // Only signal and wait for input if the current sim time is greather than or equal
  // to the requested time
  if( currentSimTime() < requestedTime ) {
    // Exchange data so that FMU inputs are retained,
    // because EnergyPlus may have overwrote an input value
    exchange();
    return;
  }

  // Signal the end of the step
  {
    std::unique_lock<std::mutex> lk(control_mutex);
    if (epstatus != EPStatus::TERMINATE)
    {
     epstatus = EPStatus::NONE;
    }
  }

  control_cv.notify_one();

  // Wait for the epstatus to advance again
  std::unique_lock<std::mutex> lk(control_mutex);
  control_cv.wait(lk, [&]() {
    return (
      (epstatus == EPStatus::ADVANCE) ||
      (epstatus == EPStatus::TERMINATE)
    );
  });
}

