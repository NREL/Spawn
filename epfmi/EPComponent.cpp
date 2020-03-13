#include "EPComponent.hpp"
#include "EnergyPlus/api/EnergyPlusPgm.hh"
#include "EnergyPlus/api/runtime.h"
#include "EnergyPlus/api/datatransfer.h"
#include "EnergyPlus/CommandLineInterface.hh"
#include "EnergyPlus/DataGlobals.hh"
#include "EnergyPlus/DataHeatBalance.hh"
#include "EnergyPlus/DataHeatBalFanSys.hh"
#include "EnergyPlus/DataRuntimeLanguage.hh"
#include "EnergyPlus/EMSManager.hh"
#include "EnergyPlus/HeatBalanceAirManager.hh"
#include "EnergyPlus/InternalHeatGains.hh"
#include "EnergyPlus/OutputProcessor.hh"
#include "EnergyPlus/ZoneTempPredictorCorrector.hh"

EPComponent::EPComponent(const std::string & name, const boost::filesystem::path & resourcePath)
  : instanceName(name)
{
  boost::system::error_code ec;
  for ( const auto & entry : boost::filesystem::directory_iterator(resourcePath, ec) ) {
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

  variables = parseVariables(idfInputPath, spawnInputPath);
}

int EPComponent::start() {
  const auto & simulation = [&](){
    auto workingPath = boost::filesystem::path(instanceName).filename();

    const int argc = 8;
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
    EnergyPlus::DataGlobals::externalHVACManager = std::bind(&EPComponent::externalHVACManager, this);
    RunEnergyPlus();
  };

  {
    std::unique_lock<std::mutex> lk(control_mutex);
    requestedTime = 0.0;
    epstatus = EPStatus::ADVANCE;
  }

  simthread = std::thread(simulation);

  {
    // Wait for E+ to go back to IDLE
    std::unique_lock<std::mutex> lk(control_mutex);
    control_cv.wait( lk, [&](){ return epstatus == EPStatus::NONE; } );
  }

  return 0;
}

int EPComponent::stop() {
  {
    std::unique_lock<std::mutex> lk(control_mutex);
    epstatus = EPStatus::TERMINATE;
  }

  stopSimulation();
  control_cv.notify_one();
  simthread.join();

  return 0;
}

int EPComponent::setTime(const double & time)
{
  requestedTime = time;

  exchange();

  {
    std::unique_lock<std::mutex> lk(control_mutex);
    epstatus = EPStatus::ADVANCE;
  }
  // Notify E+ to advance
  control_cv.notify_one();
  {
    // Wait for E+ to advance and go back to IDLE before returning
    std::unique_lock<std::mutex> lk( control_mutex );
    control_cv.wait( lk, [&](){ return epstatus == EPStatus::NONE; } );
  }

  return 0;
}

fmi2Real EPComponent::currentSimTime() const {
  return EnergyPlus::DataGlobals::SimTimeSteps * EnergyPlus::DataGlobals::TimeStepZoneSec;
}

fmi2Real EPComponent::nextSimTime() const {
  return (EnergyPlus::DataGlobals::SimTimeSteps + 1) * EnergyPlus::DataGlobals::TimeStepZoneSec;
}

bool EPComponent::setValue(const unsigned int & ref, const double & value) {
  auto var = variables.find(ref);
  if( var != variables.end() ) {
    var->second.value = value;
    var->second.valueset = true;
    return true;
  }

  return false;
}

double EPComponent::getValue(const unsigned int & ref, bool & ok) const {
  auto var = variables.find(ref);
  if( var != variables.end() ) {
    if( var->second.valueset ) {
      ok = true;
      return var->second.value;
    }
  }
  ok = false;
  return 0.0;
}

double EPComponent::getValue(const unsigned int & ref) const {
  bool ok = false;
  return getValue(ref, ok);
}

Real64 EPComponent::zoneHeatTransfer(const int ZoneNum)
{
    static Real64 SumIntGain(0.0); // Zone sum of convective internal gains
    static Real64 SumHA(0.0);      // Zone sum of Hc*Area
    static Real64 SumHATsurf(0.0); // Zone sum of Hc*Area*Tsurf
    static Real64 SumHATref(0.0);  // Zone sum of Hc*Area*Tref, for ceiling diffuser convection correlation
    static Real64 SumMCp(0.0);     // Zone sum of MassFlowRate*Cp
    static Real64 SumMCpT(0.0);    // Zone sum of MassFlowRate*Cp*T
    static Real64 SumSysMCp(0.0);  // Zone sum of air system MassFlowRate*Cp
    static Real64 SumSysMCpT(0.0); // Zone sum of air system MassFlowRate*Cp*T

    EnergyPlus::ZoneTempPredictorCorrector::CalcZoneSums(ZoneNum, SumIntGain, SumHA, SumHATsurf, SumHATref, SumMCp, SumMCpT, SumSysMCp, SumSysMCpT);

    Real64 TempDepCoef = SumHA;                   // + SumMCp;
    Real64 TempIndCoef = SumIntGain + SumHATsurf; // - SumHATref + SumMCpT;

    // Refer to
    // https://bigladdersoftware.com/epx/docs/8-8/engineering-reference/basis-for-the-zone-and-air-system-integration.html#basis-for-the-zone-and-air-system-integration
    Real64 heatTransfer = TempIndCoef - (TempDepCoef * EnergyPlus::DataHeatBalFanSys::MAT(ZoneNum));

    return heatTransfer;
}

void EPComponent::exchange()
{
  auto zoneNum = [](std::string & zoneName) {
    std::transform(zoneName.begin(), zoneName.end(), zoneName.begin(), ::toupper);
    for ( int i = 0; i < EnergyPlus::DataGlobals::NumOfZones; ++i ) {
      if ( EnergyPlus::DataHeatBalance::Zone[i].Name == zoneName ) {
        return i + 1;
      }
    }

    return 0;
  };

  auto getSensorValue = [&](Variable & var) {
    const auto & h = getVariableHandle(var.outputvarname.c_str(), var.outputvarkey.c_str());
    return getVariableValue(h);
  };

  auto compSetActuatorValue = [](const std::string & key, const std::string & componenttype, const std::string & controltype, const Real64 & value) {
    const auto & h = getActuatorHandle(key.c_str(), componenttype.c_str(), controltype.c_str());
    setActuatorValue(h, value);
  };

  auto compResetActuator = [](const std::string & key, const std::string & componenttype, const std::string & controltype) {
    const auto & h = getActuatorHandle(key.c_str(), componenttype.c_str(), controltype.c_str());
    resetActuator(h);
  };

  // Update inputs first, then outputs so that we can do some updates within EnergyPlus
  for( auto & varmap : variables ) {
    auto & var = varmap.second;
    auto varZoneNum = zoneNum(var.name);
    switch ( var.type ) {
      case VariableType::T:
        if( var.valueset ) {
          EnergyPlus::DataHeatBalFanSys::ZT( varZoneNum ) = var.value;
          EnergyPlus::DataHeatBalFanSys::ZTAV( varZoneNum ) = var.value;
          EnergyPlus::DataHeatBalFanSys::MAT( varZoneNum ) = var.value;
        }
        break;
      case VariableType::EMS_ACTUATOR:
        if( var.valueset ) {
          compSetActuatorValue(
            var.actuatorcomponentkey,
            var.actuatorcomponenttype,
            var.actuatorcontroltype,
            var.value
          );
        } else {
          compResetActuator(
            var.actuatorcomponentkey,
            var.actuatorcomponenttype,
            var.actuatorcontroltype
          );
        }
        break;
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
        var.value = EnergyPlus::DataHeatBalance::Zone( varZoneNum ).Volume;
        var.valueset = true;
        break;
      case VariableType::AFLO:
        var.value = EnergyPlus::DataHeatBalance::Zone( varZoneNum ).FloorArea;
        var.valueset = true;
        break;
      case VariableType::MSENFAC:
        var.value = EnergyPlus::DataHeatBalance::Zone( varZoneNum ).ZoneVolCapMultpSens;
        var.valueset = true;
        break;
      case VariableType::QCONSEN_FLOW:
        var.value = zoneHeatTransfer( varZoneNum );
        var.valueset = true;
        break;
      case VariableType::SENSOR:
        var.value = getSensorValue(var);
        var.valueset = true;
        break;
      default:
        break;
    }
  }
}

void EPComponent::externalHVACManager() {
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
    if (epstatus != EPStatus::TERMINATE)
    {
      std::unique_lock<std::mutex> lk(control_mutex);
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

