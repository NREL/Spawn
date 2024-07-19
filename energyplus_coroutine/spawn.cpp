#include "spawn.hpp"
#include "../util/config.hpp"
#include "../util/conversion.hpp"
#include "energyplus_helpers.hpp"
#include "idf_to_json.hpp"
#include "idfprep.hpp"
#include "input/user_config.hpp"
#include "output_types.hpp"
#include "start_time.hpp"

#include "../energyplus/src/EnergyPlus/CommandLineInterface.hh"
#include "../energyplus/src/EnergyPlus/api/EnergyPlusPgm.hh"
#include "../energyplus/src/EnergyPlus/api/func.h"
#include "../energyplus/src/EnergyPlus/api/runtime.h"

#include <limits>

namespace spawn {

Spawn::Spawn(std::string t_name, const std::string &t_input, spawn_fs::path t_workingdir) // NOLINT
    : instanceName(std::move(t_name)), workingdir(std::move(t_workingdir)), user_config_(t_input)
{
}

void Spawn::start()
{
  if (!is_running && !sim_exception_ptr && !sim_thread.joinable()) {
    is_running = true;

    auto idfPath = user_config_.idfInputPath();
    auto idfjson = idf_to_json(idfPath);

    //// Skip this step if the .spawn extension is present,
    //// which will indicate that the idf has already been "prepared"
    // if (idfPath.stem().extension() != ".spawn") {
    prepare_idf(idfjson, user_config_, start_time_);
    idfPath = workingdir / (idfPath.stem().string() + ".spawn.idf");
    json_to_idf(idfjson, idfPath);
    //}

    // Will throw an exception if validation fails
    validate_idf(idfjson);

    const auto &simulation = [&]() {
      try {
        const auto epwPath = user_config_.epwInputPath().string();
        const auto idfPath_string = idfPath.string();
        const auto iddPath = spawn::idd_path().string();
        const auto workingdir_string = workingdir.string();

        std::vector<std::string> argv{
            "energyplus", "-d", workingdir_string, "-w", epwPath, "-i", iddPath, idfPath_string};

        EnergyPlus::CommandLineInterface::ProcessArgs(sim_state, argv);
        registerErrorCallback(simState(),
                              [this](const auto level, const auto &message) { logMessage(level, message); });
        registerExternalHVACManager(simState(), [this](EnergyPlusState state) { externalHVACManager(state); });
        sim_state.dataHeatBal->MaxAllowedDelTemp = user_config_.relativeSurfaceTolerance();

        RunEnergyPlus(sim_state);

        {
          std::unique_lock<std::mutex> lk(sim_mutex);
          iterate_flag = false;
          is_running = false;
        }
        iterate_cv.notify_one();
      } catch (...) {
        std::cout << "there was an exception in the simulation thread" << std::endl;
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

    variables_.UpdateParameters(sim_state);

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
  start_time_ = StartTime(day_from_string(user_config_.runPeriod.day_of_week_for_start_day), time);
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

void Spawn::SetValue(const unsigned int index, const double &value)
{
  const auto &variables = variables_.AllVariables();

  if (index < variables.size()) {
    const auto &variable = variables[index];
    const auto &cur_val = variable->Value();

    if (!cur_val || std::abs(value) <= std::numeric_limits<float>::epsilon() ||
        std::abs(*cur_val - value) > std::numeric_limits<float>::epsilon()) {
      need_update = true;
      variable->SetValue(value, spawn::units::UnitSystem::MO);
    }
  } else {
    throw std::runtime_error(fmt::format("Attempt to set a value using an invalid reference: {}", index));
  }
}

double Spawn::GetValue(const unsigned int index) const
{
  const auto &variables = variables_.AllVariables();

  if (index < variables.size()) {
    const auto &value = variables[index]->Value();
    if (value) {
      return *value;
    } else {
      throw std::runtime_error(fmt::format("Attempt to get a value for index {}, which has no value set", index));
    }
  } else {
    throw std::runtime_error(fmt::format("Attempt to get a value using an invalid reference: {}", index));
  }
}

unsigned int Spawn::GetIndex(const std::string_view name) const
{
  return variables_.VariableIndex(name);
}

double Spawn::GetValue(const std::string_view name) const
{
  const auto index = GetIndex(name);
  return GetValue(index);
}

void Spawn::SetValue(const std::string_view name, const double &value)
{
  const auto index = GetIndex(name);
  SetValue(index, value);
}

void Spawn::exchange(const bool force)
{
  isRunningCheck();

  if (!force && !need_update) {
    return;
  }

  variables_.UpdateInputs(sim_state);

  // Run some internal EnergyPlus functions to update outputs
  EnergyPlus::HeatBalanceSurfaceManager::CalcHeatBalanceOutsideSurf(sim_state);
  EnergyPlus::HeatBalanceSurfaceManager::CalcHeatBalanceInsideSurf(sim_state);
  EnergyPlus::ZoneEquipmentManager::CalcAirFlowSimple(sim_state);
  UpdateZoneConditions(true); // true means skip any connected zones which are not under EP control
  EnergyPlus::HeatBalanceAirManager::ReportZoneMeanAirTemp(sim_state);
  EnergyPlus::HVACManager::ReportAirHeatBalance(sim_state);
  EnergyPlus::InternalHeatGains::InitInternalHeatGains(sim_state);
  EnergyPlus::InternalHeatGains::ReportInternalHeatGains(sim_state);
  EnergyPlus::ScheduleManager::UpdateScheduleValues(sim_state);
  EnergyPlus::HeatBalanceSurfaceManager::ReportSurfaceHeatBalance(sim_state);
  energyplus::UpdateLatentGains(sim_state);

  variables_.UpdateOutputs(sim_state);

  need_update = false;
}

void Spawn::externalHVACManager([[maybe_unused]] EnergyPlusState state)
{
  if (sim_state.dataGlobal->KickOffSimulation) {
    // ManageHVAC initializes many structures that need to exist.
    // Withouth calling this during the simulation kick off, the simulation will crash.
    // After kick off, we skip this call, because the client is managing the HVAC.
    EnergyPlus::HVACManager::ManageHVAC(sim_state);
    // At this time, there is no data exchange or any other
    // interaction with the client during kick off, so this funciton returns and the
    // simulation continues through the startup process.
    return;
  }

  // Similarly, there is no interaction with the client during warmup and sizing, so return now before signaling
  if (sim_state.dataGlobal->DoingSizing || sim_state.dataGlobal->WarmupFlag) {
    return;
  }

  // Exchange data with the FMU
  exchange(true);

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

void Spawn::UpdateZoneConditions(bool skipConnectedZones)
{
  const double dt = currentTime() - prevZoneUpdate;
  prevZoneUpdate = currentTime();
  if (dt > 0.0) {

    for (const auto &zone : user_config_.zones) {
      if (skipConnectedZones && zone.isconnected) {
        continue;
      }

      const auto zonenum = energyplus::ZoneNum(sim_state, zone.idfname);
      energyplus::UpdateZoneTemperature(sim_state, zonenum, dt);
      energyplus::UpdateZoneHumidityRatio(sim_state, zonenum, dt);
    }
  }
}

} // namespace spawn
