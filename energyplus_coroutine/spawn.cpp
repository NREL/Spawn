#include "spawn.hpp"
#include "../util/config.hpp"
#include "../util/conversion.hpp"
#include "../util/math.hpp"
#include "idf_to_json.hpp"
#include "idfprep.hpp"
#include "input/input.hpp"
#include "output_types.hpp"
#include "start_time.hpp"

#include "../energyplus/src/EnergyPlus/CommandLineInterface.hh"
#include "../energyplus/src/EnergyPlus/api/EnergyPlusPgm.hh"
#include "../energyplus/src/EnergyPlus/api/datatransfer.h"
#include "../energyplus/src/EnergyPlus/api/func.h"
#include "../energyplus/src/EnergyPlus/api/runtime.h"

#include <array>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <cmath>
#include <limits>

namespace spawn {

Spawn::Spawn(std::string t_name, const std::string &t_input, spawn_fs::path t_workingdir) // NOLINT
    : instanceName(std::move(t_name)), workingdir(std::move(t_workingdir)), input(t_input),
      variables(parseVariables(input))
{
}

void Spawn::start()
{
  if (!is_running && !sim_exception_ptr && !sim_thread.joinable()) {
    is_running = true;

    auto idfPath = input.idfInputPath();
    auto idfjson = idf_to_json(idfPath);

    //// Skip this step if the .spawn extension is present,
    //// which will indicate that the idf has already been "prepared"
    // if (idfPath.stem().extension() != ".spawn") {
    prepare_idf(idfjson, input, start_time_);
    idfPath = workingdir / (idfPath.stem().string() + ".spawn.idf");
    json_to_idf(idfjson, idfPath);
    //}

    // Will throw an exception if validation fails
    validate_idf(idfjson);

    const auto &simulation = [&]() {
      try {
        const auto epwPath = input.epwInputPath().string();
        const auto idfPath_string = idfPath.string();
        const auto iddPath = spawn::idd_path().string();
        const auto workingdir_string = workingdir.string();

        constexpr int argc = 8;
        std::array<const char *, argc> argv{"energyplus",
                                            "-d",
                                            workingdir_string.c_str(),
                                            "-w",
                                            epwPath.c_str(),
                                            "-i",
                                            iddPath.c_str(),
                                            idfPath_string.c_str()};

        EnergyPlus::CommandLineInterface::ProcessArgs(sim_state, argc, argv.data());
        registerErrorCallback(simState(),
                              [this](const auto level, const auto &message) { logMessage(level, message); });
        registerExternalHVACManager(simState(), [this](EnergyPlusState state) { externalHVACManager(state); });
        sim_state.dataHeatBal->MaxAllowedDelTemp = input.relativeSurfaceTolerance();

        RunEnergyPlus(sim_state);

        {
          std::unique_lock<std::mutex> lk(sim_mutex);
          iterate_flag = false;
          is_running = false;
        }
        iterate_cv.notify_one();
      } catch (...) {
        sim_exception_ptr = std::current_exception();
      }
    };

    requested_time_ = start_time_.Seconds();
    sim_thread = std::thread(simulation);
    // This will make the EnergyPlus simulation thread go through startup/warmup,
    // and reach the requested start time.
    iterate();

    // We might see isRunning return false, before
    // the EnergyPlus thread is terminated, therefore this check
    // will wait for the EnergyPlus thread to finish.
    if (!isRunning()) {
      sim_thread.join();
    }

    // This will make sure that we have a data exchange
    setTime(start_time_.Seconds());
  }
}

void Spawn::wait()
{
  std::unique_lock<std::mutex> lk(sim_mutex);
  iterate_cv.wait(lk, [&]() { return (!iterate_flag) || (!is_running) || sim_exception_ptr; });

  if (sim_exception_ptr) {
    std::rethrow_exception(sim_exception_ptr);
  }
}

void Spawn::iterate()
{
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

void Spawn::stop()
{
  // This is a workaround to make sure one "complete" step has been made during the weather period.
  // This is required because some data structures that are used in closeout reporting are not initialized until
  // the first non warmup non sizing step
  if (sim_state.dataGlobal->SimTimeSteps == 1) {
    iterate();
  }

  // This is an EnergyPlus API
  stopSimulation(simState());
  // iterate the sim to allow EnergyPlus to go through shutdown;
  iterate();
  sim_thread.join();
}

bool Spawn::isRunning() const noexcept
{
  return is_running;
}

void Spawn::isRunningCheck() const
{
  if (!is_running) {
    throw std::runtime_error("EnergyPlus is not running");
  }
}

double Spawn::startTime() const noexcept
{
  return start_time_.Seconds();
}

void Spawn::setStartTime(const double &time) noexcept
{
  start_time_ = StartTime(day_from_string(input.runPeriod.start_day_of_year), time);
}

void Spawn::setTime(const double &time)
{
  isRunningCheck();
  requested_time_ = time;
  exchange(true);

  if (requested_time_ >= nextEventTime()) {
    iterate();
  }
}

double Spawn::currentTime() const
{
  isRunningCheck();
  return start_time_.EnergyPlusTimeDifferential() + ElapsedEnergyPlusTime();
}

double Spawn::ElapsedEnergyPlusTime() const
{
  isRunningCheck();
  return (sim_state.dataGlobal->SimTimeSteps - 1) * sim_state.dataGlobal->TimeStepZoneSec;
}

double Spawn::nextEventTime() const
{
  isRunningCheck();
  return currentTime() + sim_state.dataGlobal->TimeStepZoneSec;
}

void Spawn::setValue(const unsigned int index, const double value) // NOLINT
{
  auto var = variables.find(index);
  if (var != variables.end()) {
    const auto &cur_val = var->second.getValue(spawn::units::UnitSystem::MO);
    if (std::abs(value) <= std::numeric_limits<float>::epsilon() ||
        std::abs(cur_val - value) > std::numeric_limits<float>::epsilon()) {
      need_update = true;
      var->second.setValue(value, spawn::units::UnitSystem::MO);
    }
  } else {
    throw std::runtime_error(fmt::format("Attempt to set a value using an invalid reference: {}", index));
  }
}

double Spawn::getValue(const unsigned int index) const
{
  isRunningCheck();
  auto var = variables.find(index);
  if (var != variables.end()) {
    if (var->second.isValueSet()) {
      return var->second.getValue(spawn::units::UnitSystem::MO);
    } else {
      throw std::runtime_error(fmt::format("Attempt to get a value for index {}, which has no value set", index));
    }
  } else {
    throw std::runtime_error(fmt::format("Attempt to get a value using an invalid reference: {}", index));
  }
}

unsigned int Spawn::getIndex(const std::string &name) const
{
  const auto pred = [&name](const std::pair<unsigned int, Variable> &v) { return v.second.name == name; };

  const auto it = std::find_if(variables.begin(), variables.end(), pred);
  if (it == std::end(variables)) {
    throw std::runtime_error(fmt::format("Attempt to retrieve an invalid variable name: {}", name));
  }

  return it->first;
}

double Spawn::getValue(const std::string &name) const
{
  const auto index = getIndex(name);
  return getValue(index);
}

void Spawn::setValue(const std::string &name, const double value)
{
  const auto index = getIndex(name);
  setValue(index, value);
}

Spawn::ZoneSums Spawn::zoneSums(const int zonenum)
{
  Real64 SumIntGain{0.0}; // Zone sum of convective internal gains
  Real64 SumHA{0.0};      // Zone sum of Hc*Area
  Real64 SumHATsurf{0.0}; // Zone sum of Hc*Area*Tsurf
  Real64 SumHATref{0.0};  // Zone sum of Hc*Area*Tref, for ceiling diffuser convection correlation
  Real64 SumMCp{0.0};     // Zone sum of MassFlowRate*Cp
  Real64 SumMCpT{0.0};    // Zone sum of MassFlowRate*Cp*T
  Real64 SumSysMCp{0.0};  // Zone sum of air system MassFlowRate*Cp
  Real64 SumSysMCpT{0.0}; // Zone sum of air system MassFlowRate*Cp*T

  EnergyPlus::ZoneTempPredictorCorrector::CalcZoneSums(
      sim_state, zonenum, SumIntGain, SumHA, SumHATsurf, SumHATref, SumMCp, SumMCpT, SumSysMCp, SumSysMCpT);

  Spawn::ZoneSums sums{};

  sums.tempDepCoef = SumHA + SumMCp;
  sums.tempIndCoef = SumIntGain + SumHATsurf + SumMCpT;

  return sums;
}

void Spawn::setZoneTemperature(const int zonenum, const double temp) // NOLINT this is logically not const
{
  // Is it necessary to update all of these or can we
  // simply update ZT and count on EnergyPlus::HeatBalanceAirManager::ReportZoneMeanAirTemp()
  // to propogate the other variables?
  sim_state.dataHeatBalFanSys->ZT(zonenum) = temp;
  sim_state.dataHeatBalFanSys->ZTAV(zonenum) = temp;
  sim_state.dataHeatBalFanSys->MAT(zonenum) = temp;
}

void Spawn::setZoneHumidityRatio(const int zonenum, const double ratio)
{
  sim_state.dataHeatBalFanSys->ZoneAirHumRatAvg(zonenum) = ratio;
  sim_state.dataHeatBalFanSys->ZoneAirHumRat(zonenum) = ratio;
  sim_state.dataHeatBalFanSys->ZoneAirHumRatTemp(zonenum) = ratio;
}

double Spawn::zoneHumidityRatio(const int zonenum) const
{
  return sim_state.dataHeatBalFanSys->ZoneAirHumRat(zonenum);
}

double Spawn::zoneTemperature(const int zonenum) const
{
  return sim_state.dataHeatBalFanSys->ZT(zonenum);
}

void Spawn::updateZoneTemperature(const int zonenum, const double dt)
{
  // Based on the EnergyPlus analytical method
  // See ZoneTempPredictorCorrector::CorrectZoneAirTemp
  const auto &zonetemp = zoneTemperature(zonenum);
  double newzonetemp = 0;

  const auto aircap = sim_state.dataHeatBal->Zone(as_size_t(zonenum)).Volume *
                      sim_state.dataHeatBal->Zone(as_size_t(zonenum)).ZoneVolCapMultpSens *
                      EnergyPlus::Psychrometrics::PsyRhoAirFnPbTdbW(sim_state,
                                                                    sim_state.dataEnvrn->OutBaroPress,
                                                                    zonetemp,
                                                                    sim_state.dataHeatBalFanSys->ZoneAirHumRat(zonenum),
                                                                    "") *
                      EnergyPlus::Psychrometrics::PsyCpAirFnW(
                          sim_state.dataHeatBalFanSys->ZoneAirHumRat(zonenum)); // / (TimeStepSys * SecInHour);

  const auto &sums = zoneSums(zonenum);
  if (sums.tempDepCoef == 0.0) { // B=0
    newzonetemp = zonetemp + sums.tempIndCoef / aircap * dt;
  } else {
    newzonetemp =
        (zonetemp - sums.tempIndCoef / sums.tempDepCoef) * std::exp(min(700.0, -sums.tempDepCoef / aircap * dt)) +
        sums.tempIndCoef / sums.tempDepCoef;
  }

  setZoneTemperature(zonenum, newzonetemp);
}

void Spawn::updateZoneHumidityRatio(const int zonenum, const double dt)
{
  // Based on the EnergyPlus analytical method
  // See ZoneTempPredictorCorrector::CorrectZoneHumRat

  static constexpr std::string_view RoutineName("updateZoneHumidityRatio");
  const auto humidityRatio = zoneHumidityRatio(zonenum);

  auto &ZT = sim_state.dataHeatBalFanSys->ZT;
  auto &Zone = sim_state.dataHeatBal->Zone;
  double moistureMassFlowRate = 0.0;

  // Calculate hourly humidity ratio from infiltration + humdidity added from latent load + system added moisture
  const auto latentGain = sim_state.dataHeatBalFanSys->ZoneLatentGain(zonenum) +
                          sim_state.dataHeatBalFanSys->SumLatentHTRadSys(zonenum) +
                          sim_state.dataHeatBalFanSys->SumLatentPool(zonenum);

  const double RhoAir =
      EnergyPlus::Psychrometrics::PsyRhoAirFnPbTdbW(sim_state,
                                                    sim_state.dataEnvrn->OutBaroPress,
                                                    ZT(zonenum),
                                                    sim_state.dataHeatBalFanSys->ZoneAirHumRat(zonenum),
                                                    RoutineName);
  const double h2oHtOfVap =
      EnergyPlus::Psychrometrics::PsyHgAirFnWTdb(sim_state.dataHeatBalFanSys->ZoneAirHumRat(zonenum), ZT(zonenum));

  const double B = (latentGain / h2oHtOfVap) +
                   ((sim_state.dataHeatBalFanSys->OAMFL(zonenum) + sim_state.dataHeatBalFanSys->VAMFL(zonenum) +
                     sim_state.dataHeatBalFanSys->CTMFL(zonenum)) *
                    sim_state.dataEnvrn->OutHumRat) +
                   sim_state.dataHeatBalFanSys->EAMFLxHumRat(zonenum) + (moistureMassFlowRate) +
                   sim_state.dataHeatBalFanSys->SumHmARaW(zonenum) +
                   sim_state.dataHeatBalFanSys->MixingMassFlowXHumRat(zonenum) +
                   sim_state.dataHeatBalFanSys->MDotOA(zonenum) * sim_state.dataEnvrn->OutHumRat;

  const double C = RhoAir * Zone(zonenum).Volume * Zone(zonenum).ZoneVolCapMultpMoist / dt;

  double newHumidityRatio = humidityRatio + B / C;

  // Set the humidity ratio to zero if the zone has been dried out
  if (newHumidityRatio < 0.0) {
    newHumidityRatio = 0.0;
  }

  // Check to make sure that is saturated there is condensation in the zone
  // by resetting to saturation conditions.
  const double wzSat = EnergyPlus::Psychrometrics::PsyWFnTdbRhPb(
      sim_state, ZT(zonenum), 1.0, sim_state.dataEnvrn->OutBaroPress, RoutineName);

  if (newHumidityRatio > wzSat) {
    newHumidityRatio = wzSat;
  }

  setZoneHumidityRatio(zonenum, newHumidityRatio);
}

void Spawn::updateZoneConditions(bool skipConnectedZones)
{
  // Check for...
  if (
      // 1. The beginning of the environment
      sim_state.dataGlobal->BeginEnvrnFlag ||
      // 2. The first call after warmup
      (prevWarmupFlag && !(sim_state.dataGlobal->WarmupFlag))) {
    prevZoneUpdate = currentTime();
  } else {
    const double dt = currentTime() - prevZoneUpdate;
    prevZoneUpdate = currentTime();

    for (const auto &zone : input.zones) {
      if (skipConnectedZones && zone.isconnected) {
        continue;
      }

      const auto zonenum = zoneNum(zone.idfname);
      updateZoneTemperature(zonenum, dt);
      updateZoneHumidityRatio(zonenum, dt);
    }
  }

  prevWarmupFlag = sim_state.dataGlobal->WarmupFlag;
}

void Spawn::updateLatentGains()
{
  for (int zonei = 1; zonei <= sim_state.dataGlobal->NumOfZones; ++zonei) {
    EnergyPlus::InternalHeatGains::SumAllInternalLatentGains(
        sim_state, zonei, sim_state.dataHeatBalFanSys->ZoneLatentGain(zonei));
  }
}

double Spawn::zoneHeatTransfer(const int zonenum)
{
  const auto &sums = zoneSums(zonenum);
  // Refer to
  // https://bigladdersoftware.com/epx/docs/8-8/engineering-reference/basis-for-the-zone-and-air-system-integration.html#basis-for-the-zone-and-air-system-integration
  const auto heatTransfer = sums.tempIndCoef - (sums.tempDepCoef * sim_state.dataHeatBalFanSys->MAT(zonenum));
  return heatTransfer;
}

void Spawn::setInsideSurfaceTemperature(const int surfacenum, double temp)
{
  auto &surface = sim_state.dataSurface->Surface(as_size_t(surfacenum));
  setActuatorValue("Surface", "Surface Inside Temperature", surface.Name, temp);
  auto &extBoundCond = surface.ExtBoundCond;
  if (extBoundCond > 0) {
    // If this is an interzone surface then set the outside of the matching surface
    auto &other_surface = sim_state.dataSurface->Surface(as_size_t(extBoundCond));
    setActuatorValue("Surface", "Surface Outside Temperature", other_surface.Name, temp);
  }
}

void Spawn::setOutsideSurfaceTemperature(const int surfacenum, double temp)
{
  auto &surface = sim_state.dataSurface->Surface(as_size_t(surfacenum));
  setActuatorValue("Surface", "Surface Outside Temperature", surface.Name, temp);
  auto &extBoundCond = surface.ExtBoundCond;

  if (surfacenum == extBoundCond) {
    throw std::runtime_error(fmt::format("Attempt to control surface named {} that has a self referencing exterior "
                                         "boundary condition. This is not supported by Spawn",
                                         surface.Name));
  }

  if (extBoundCond > 0) {
    // If this is an interzone surface then set the inside of the matching surface
    auto &other_surface = sim_state.dataSurface->Surface(as_size_t(extBoundCond));
    setActuatorValue("Surface", "Surface Inside Temperature", other_surface.Name, temp);
  }
}

double Spawn::getInsideSurfaceHeatFlow(const int surfacenum) const
{
  return sim_state.dataHeatBalSurf->QdotConvInRep(surfacenum) +
         sim_state.dataHeatBalSurf->QdotRadNetSurfInRep(surfacenum);
}

double Spawn::getOutsideSurfaceHeatFlow(const int surfacenum) const
{
  auto &surface = sim_state.dataSurface->Surface(as_size_t(surfacenum));
  auto &extBoundCond = surface.ExtBoundCond;
  if (extBoundCond > 0) {
    // EnergyPlus does not calculate the surface heat flux for interzone surfaces,
    // instead return the inside face heat flux of the matching surface
    return sim_state.dataHeatBalSurf->QdotConvInRep(extBoundCond) +
           sim_state.dataHeatBalSurf->QdotRadNetSurfInRep(extBoundCond);
  } else {
    return sim_state.dataHeatBalSurf->QdotConvOutRep(surfacenum) + sim_state.dataHeatBalSurf->QdotRadOutRep(surfacenum);
  }
}

int Spawn::zoneNum(const std::string &zoneName) const
{
  auto upperZoneName = zoneName;
  std::transform(zoneName.begin(), zoneName.end(), upperZoneName.begin(), ::toupper);
  for (int i = 0; i < sim_state.dataGlobal->NumOfZones; ++i) {
    if (sim_state.dataHeatBal->Zone[as_size_t(i)].Name == upperZoneName) {
      return i + 1;
    }
  }

  return 0;
}

int Spawn::surfaceNum(const std::string &surfaceName) const
{
  auto upperName = surfaceName;
  std::transform(surfaceName.begin(), surfaceName.end(), upperName.begin(), ::toupper);
  for (const auto i : sim_state.dataSurface->AllHTNonWindowSurfaceList) {
    if (sim_state.dataSurface->Surface[as_size_t(i)].Name == upperName) {
      return i + 1;
    }
  }

  return 0;
}

int Spawn::getVariableHandle(const std::string &name, const std::string &key)
{
  // look in the cache
  const auto it = variable_handle_cache.find(std::make_tuple(name, key));
  if (it != variable_handle_cache.end()) {
    return it->second;
  } else {
    // Uses the EnergyPlus api getVariableHandle, but throws if the variable does not exist
    const auto h = ::getVariableHandle(simState(), name.c_str(), key.c_str());
    if (h == -1) {
      throw std::runtime_error(fmt::format("Attempt to get invalid variable using name '{}', and key '{}'", name, key));
    }
    variable_handle_cache[std::make_tuple(name, key)] = h;
    return h;
  }
}

int Spawn::getActuatorHandle(const std::string &componenttype,
                             const std::string &controltype,
                             const std::string &componentname)
{
  // look in the cache
  const auto it = actuator_handle_cache.find(std::make_tuple(componenttype, controltype, componentname));
  if (it != actuator_handle_cache.end()) {
    return it->second;
  } else {
    // Uses the EnergyPlus api getActuatorHandle, but throws if the actuator does not exist
    const auto h = ::getActuatorHandle(simState(), componenttype.c_str(), controltype.c_str(), componentname.c_str());
    if (h == -1) {
      throw std::runtime_error(fmt::format(
          "Attempt to get invalid actuator using component type '{}', component name '{}', and control type {}",
          componenttype,
          componentname,
          controltype));
    }
    actuator_handle_cache[std::make_tuple(componenttype, controltype, componentname)] = h;
    return h;
  }
}

void Spawn::setActuatorValue(const std::string &componenttype,
                             const std::string &controltype,
                             const std::string &componentname,
                             const Real64 value)
{
  const auto h = getActuatorHandle(componenttype, controltype, componentname);
  if (value == 0.0) {
    ::setActuatorValue(simState(), h, 1.0);
  }
  ::setActuatorValue(simState(), h, value);
}

void Spawn::resetActuator(const std::string &componenttype,
                          const std::string &controltype,
                          const std::string &componentname)
{
  const auto h = getActuatorHandle(componenttype, controltype, componentname);
  ::resetActuator(simState(), h);
}

double Spawn::getSensorValue(Variable &var)
{
  const auto h = getVariableHandle(var.outputvarname, var.outputvarkey);
  return getVariableValue(simState(), h);
}

void Spawn::exchange(const bool force)
{
  isRunningCheck();

  if (!force && !need_update) {
    return;
  }

  auto actuateVar = [&](const Variable &var) {
    if (var.isValueSet()) {
      setActuatorValue(var.actuatorcomponenttype,
                       var.actuatorcontroltype,
                       var.actuatorcomponentkey,
                       var.getValue(spawn::units::UnitSystem::EP));
    } else {
      resetActuator(var.actuatorcomponenttype, var.actuatorcontroltype, var.actuatorcomponentkey);
    }
  };

  // Update inputs first, then outputs so that we can do some updates within EnergyPlus
  for (auto &varmap : variables) {
    auto &var = varmap.second;
    switch (var.type) {
    case VariableType::T:
      if (var.isValueSet()) {
        auto varZoneNum = zoneNum(var.name);
        const auto &v = var.getValue(spawn::units::UnitSystem::EP);
        setZoneTemperature(varZoneNum, v);
      }
      break;
    case VariableType::X:
      if (var.isValueSet()) {
        auto varZoneNum = zoneNum(var.name);
        const auto &v = var.getValue(spawn::units::UnitSystem::EP);
        setZoneHumidityRatio(varZoneNum, v);
      }
      break;
    case VariableType::EMS_ACTUATOR:
    case VariableType::SCHEDULE:
    case VariableType::QGAIRAD_FLOW:
      actuateVar(var);
      break;
    case VariableType::TSURF_FRONT:
    case VariableType::TSURF:
      if (var.isValueSet()) {
        auto sn = surfaceNum(var.name);
        const auto &v = var.getValue(spawn::units::UnitSystem::EP);
        setInsideSurfaceTemperature(sn, v);
      }
      break;
    case VariableType::TSURF_BACK:
      if (var.isValueSet()) {
        auto sn = surfaceNum(var.name);
        const auto &v = var.getValue(spawn::units::UnitSystem::EP);
        setOutsideSurfaceTemperature(sn, v);
      }
      break;
    default:
      break;
    }
  }

  // Run some internal EnergyPlus functions to update outputs
  EnergyPlus::HeatBalanceSurfaceManager::CalcHeatBalanceOutsideSurf(sim_state);
  EnergyPlus::HeatBalanceSurfaceManager::CalcHeatBalanceInsideSurf(sim_state);
  EnergyPlus::ZoneEquipmentManager::CalcAirFlowSimple(sim_state);
  updateZoneConditions(true); // true means skip any connected zones which are not under EP control
  EnergyPlus::HeatBalanceAirManager::ReportZoneMeanAirTemp(sim_state);
  EnergyPlus::HVACManager::ReportAirHeatBalance(sim_state);
  EnergyPlus::InternalHeatGains::InitInternalHeatGains(sim_state);
  EnergyPlus::InternalHeatGains::ReportInternalHeatGains(sim_state);
  EnergyPlus::ScheduleManager::UpdateScheduleValues(sim_state);
  updateLatentGains();

  // Now update the outputs
  for (auto &varmap : variables) {
    auto &var = varmap.second;
    switch (var.type) {
    case VariableType::TRAD: {
      const auto varZoneNum = zoneNum(var.name);
      var.setValue(sim_state.dataHeatBal->ZoneMRT(varZoneNum), spawn::units::UnitSystem::EP);
      break;
    }
    case VariableType::V: {
      const auto varZoneNum = zoneNum(var.name);
      var.setValue(sim_state.dataHeatBal->Zone(as_size_t(varZoneNum)).Volume, spawn::units::UnitSystem::EP);
      break;
    }
    case VariableType::AFLO: {
      const auto varZoneNum = zoneNum(var.name);
      var.setValue(sim_state.dataHeatBal->Zone(as_size_t(varZoneNum)).FloorArea, spawn::units::UnitSystem::EP);
      break;
    }
    case VariableType::MSENFAC: {
      const auto varZoneNum = zoneNum(var.name);
      var.setValue(sim_state.dataHeatBal->Zone(as_size_t(varZoneNum)).ZoneVolCapMultpSens,
                   spawn::units::UnitSystem::EP);
      break;
    }
    case VariableType::QCONSEN_FLOW: {
      const auto varZoneNum = zoneNum(var.name);
      var.setValue(zoneHeatTransfer(varZoneNum), spawn::units::UnitSystem::EP);
      break;
    }
    case VariableType::QLAT_FLOW: {
      const auto varZoneNum = zoneNum(var.name);
      var.setValue(sim_state.dataHeatBalFanSys->ZoneLatentGain(varZoneNum), spawn::units::UnitSystem::EP);
      break;
    }
    case VariableType::QPEO_FLOW:
    case VariableType::SENSOR: {
      var.setValue(getSensorValue(var), spawn::units::UnitSystem::EP);
      break;
    }
    case VariableType::ASURF: {
      const auto varSurfaceNum = surfaceNum(var.name);
      var.setValue(sim_state.dataSurface->Surface(as_size_t(varSurfaceNum)).GrossArea, spawn::units::UnitSystem::EP);
      break;
    }
    case VariableType::QSURF_FRONT: {
      const auto varSurfaceNum = surfaceNum(var.name);
      const auto &value = getInsideSurfaceHeatFlow(varSurfaceNum);
      var.setValue(value, spawn::units::UnitSystem::EP);
      break;
    }
    case VariableType::QSURF_BACK: {
      const auto varSurfaceNum = surfaceNum(var.name);
      const auto &value = getOutsideSurfaceHeatFlow(varSurfaceNum);
      var.setValue(value, spawn::units::UnitSystem::EP);
      break;
    }
    case VariableType::QSURF: {
      const auto varSurfaceNum = surfaceNum(var.name);
      const auto &value = getInsideSurfaceHeatFlow(varSurfaceNum);
      var.setValue(value, spawn::units::UnitSystem::EP);
      break;
    }
    default:
      break;
    }
  }

  need_update = false;
}

void Spawn::initZoneEquip()
{
  if (!sim_state.dataZoneEquip->ZoneEquipInputsFilled) {
    EnergyPlus::DataZoneEquipment::GetZoneEquipmentData(sim_state);
    sim_state.dataZoneEquip->ZoneEquipInputsFilled = true;
  }
}

void Spawn::externalHVACManager([[maybe_unused]] EnergyPlusState state)
{
  // Although we do not use the ZoneTempPredictorCorrector,
  // some global variables need to be initialized by InitZoneAirSetPoints
  // This is protected by a one time flag so that it will only happen once
  // during the simulation
  if (sim_state.dataZoneCtrls->GetZoneAirStatsInputFlag) {
    EnergyPlus::ZoneTempPredictorCorrector::GetZoneAirSetPoints(sim_state);
    EnergyPlus::ZoneTempPredictorCorrector::InitZoneAirSetPoints(sim_state);
    sim_state.dataZoneCtrls->GetZoneAirStatsInputFlag = false;
  }

  // Likewise init zone equipment one time
  initZoneEquip();

  // At this time, there is no data exchange or any other
  // interaction with the FMU while KickOffSimulation is true.
  // Sensors and actuators may not be setup at this point, so exchange
  // might trigger exceptions
  if (sim_state.dataGlobal->KickOffSimulation) {
    return;
  }

  // Exchange data with the FMU
  exchange(true);

  // There is no interaction with the FMU during warmup,
  // so return now before signaling
  if (sim_state.dataGlobal->DoingSizing || sim_state.dataGlobal->WarmupFlag) {
    return;
  }

  // Only signal and wait for input if the current sim time is greather than or equal
  // to the requested time
  if (currentTime() >= requested_time_) {
    // Signal the end of the step
    {
      std::unique_lock<std::mutex> lk(sim_mutex);
      iterate_flag = false;
    }

    iterate_cv.notify_one();

    // Wait for the iterate_flag to signal another iteration
    std::unique_lock<std::mutex> lk(sim_mutex);
    iterate_cv.wait(lk, [&]() { return iterate_flag; });
  }
}

void Spawn::setLogCallback(std::function<void(EnergyPlus::Error, const std::string &)> cb)
{
  logCallback = std::move(cb);
}

void Spawn::logMessage(EnergyPlus::Error level, const std::string &message)
{
  if (logCallback && !message.empty() && (level != EnergyPlus::Error::Info)) {
    log_message_queue.emplace_back(level, message);
  }
}

void Spawn::emptyLogMessageQueue()
{
  if (logCallback) {
    while (!log_message_queue.empty()) {
      auto m = log_message_queue.front();
      logCallback(m.first, m.second);
      log_message_queue.pop_front();
    }
  }
}

EnergyPlusState Spawn::simState()
{
  return static_cast<EnergyPlusState>(&sim_state);
}

} // namespace spawn
