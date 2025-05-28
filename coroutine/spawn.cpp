#include "spawn.hpp"
#include "energyplus_helpers.hpp"
#include "idf_to_json.hpp"
#include "idfprep.hpp"
#include "input/user_config.hpp"
#include "output_types.hpp"
#include "start_time.hpp"

#include "../util/config.hpp"
#include "../util/conversion.hpp"

#include "../energyplus/src/EnergyPlus/CommandLineInterface.hh"
#include "../energyplus/src/EnergyPlus/api/EnergyPlusPgm.hh"
#include "../energyplus/src/EnergyPlus/api/func.h"
#include "../energyplus/src/EnergyPlus/api/runtime.h"

#include <limits>

namespace spawn {

Spawn::Spawn(const std::string_view name,
             spawn_fs::path idd_path,
             const std::string_view user_config,
             spawn_fs::path working_dir)
    : instance_name_(name), idd_path_(std::move(idd_path)), user_config_(std::string(user_config)),
      working_dir_(std::move(working_dir))
{
}

void Spawn::Start()
{
  if (!is_running_ && !sim_exception_ptr_ && !sim_thread_.joinable()) {
    is_running_ = true;

    auto idfPath = user_config_.idfInputPath();
    auto idfjson = idf_to_json(idfPath);

    //// Skip this step if the .spawn extension is present,
    //// which will indicate that the idf has already been "prepared"
    // if (idfPath.stem().extension() != ".spawn") {
    prepare_idf(idfjson, user_config_, start_time_);
    idfPath = working_dir_ / (idfPath.stem().string() + ".spawn.idf");
    json_to_idf(idfjson, idfPath);
    //}

    // Will throw an exception if validation fails
    validate_idf(idfjson);

    const auto &simulation = [&]() {
      try {
        const auto epw_path = user_config_.epwInputPath().string();

        std::vector<std::string> argv{
            "energyplus", "-d", working_dir_.string(), "-w", epw_path, "-i", idd_path_.string(), idfPath.string()};

        registerErrorCallback(SimState(),
                              [this](const auto level, const auto &message) { LogMessage(level, message); });
        registerExternalHVACManager(SimState(), [this](EnergyPlusState state) { ExternalHVACManager(state); });
        sim_state_.dataHeatBal->MaxAllowedDelTemp = user_config_.relativeSurfaceTolerance();

        runEnergyPlusAsLibrary(sim_state_, argv);
      } catch (...) {
        sim_exception_ptr_ = std::current_exception();
        EnergyPlus::AbortEnergyPlus(sim_state_);
      }

      {
        std::unique_lock<std::mutex> lk(sim_mutex_);
        iterate_flag_ = false;
        is_running_ = false;
      }
      iterate_cv_.notify_one();
    };

    requested_time_ = start_time_.seconds();
    sim_thread_ = std::thread(simulation);
    // This will make the EnergyPlus simulation thread go through startup/warmup,
    // and reach the requested start time.
    Iterate();

    variables_.UpdateParameters(sim_state_);

    // This will make sure that we have a data exchange
    SetTime(start_time_.seconds());
  }
}

void Spawn::Wait()
{
  std::unique_lock<std::mutex> lk(sim_mutex_);
  iterate_cv_.wait(lk, [&]() { return (!iterate_flag_) || (!is_running_) || sim_exception_ptr_; });

  if (sim_exception_ptr_) {
    sim_thread_.join();
    std::rethrow_exception(sim_exception_ptr_);
  }
}

void Spawn::Iterate()
{
  // Wait for any current iteration to complete
  // There should never be a wait time (iterate_flag_ should be false)
  // Consider throw if iterate_flag_ == true instead
  Wait();

  // Signal the iteration
  {
    std::unique_lock<std::mutex> lk(sim_mutex_);
    iterate_flag_ = true;
  }
  iterate_cv_.notify_one();

  // Wait for EnergyPlus to complete the iteration
  Wait();

  EmptyLogMessageQueue();
}

void Spawn::Stop()
{
  // This is a workaround to make sure one "complete" step has been made during the weather period.
  // This is required because some data structures that are used in closeout reporting are not initialized until
  // the first non warmup non sizing step
  if (sim_state_.dataGlobal->SimTimeSteps == 1) {
    Iterate();
  }

  // This is an EnergyPlus API
  stopSimulation(SimState());
  // iterate the sim to allow EnergyPlus to go through shutdown;
  Iterate();
  sim_thread_.join();
}

bool Spawn::IsRunning() const noexcept
{
  return is_running_;
}

void Spawn::IsRunningCheck() const
{
  if (!is_running_) {
    throw std::runtime_error("EnergyPlus is not running");
  }
}

double Spawn::StartTime() const noexcept
{
  return start_time_.seconds();
}

void Spawn::SetStartTime(const double &time) noexcept
{
  start_time_ = spawn::StartTime(day_from_string(user_config_.runPeriod.start_day_of_year), time);
}

void Spawn::SetTime(const double &time)
{
  IsRunningCheck();
  requested_time_ = time;
  Exchange(true);

  if (requested_time_ >= NextEventTime()) {
    Iterate();
  }
}

double Spawn::CurrentTime() const
{
  IsRunningCheck();
  return start_time_.energyplus_time_differential() + ElapsedEnergyPlusTime();
}

double Spawn::ElapsedEnergyPlusTime() const
{
  IsRunningCheck();
  return (sim_state_.dataGlobal->SimTimeSteps - 1) * sim_state_.dataGlobal->TimeStepZoneSec;
}

double Spawn::NextEventTime() const
{
  IsRunningCheck();
  return CurrentTime() + sim_state_.dataGlobal->TimeStepZoneSec;
}

void Spawn::SetValue(const unsigned int index, const double &value)
{
  const auto &variables = variables_.all_variables();

  if (index < variables.size()) {
    const auto &variable = variables[index];
    const auto &cur_val = variable->value();

    if (!cur_val || std::abs(value) <= std::numeric_limits<float>::epsilon() ||
        std::abs(*cur_val - value) > std::numeric_limits<float>::epsilon()) {
      need_update_ = true;
      variable->SetValue(value, spawn::units::UnitSystem::MO);
    }
  } else {
    throw std::runtime_error(fmt::format("Attempt to set a value using an invalid reference: {}", index));
  }
}

double Spawn::GetValue(const unsigned int index) const
{
  const auto &variables = variables_.all_variables();

  if (index < variables.size()) {
    const auto &value = variables[index]->value();
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

void Spawn::Exchange(const bool force)
{
  IsRunningCheck();

  if (!force && !need_update_) {
    return;
  }

  variables_.UpdateInputs(sim_state_);

  // Run some internal EnergyPlus functions to update outputs
  EnergyPlus::HeatBalanceSurfaceManager::CalcHeatBalanceOutsideSurf(sim_state_);
  EnergyPlus::HeatBalanceSurfaceManager::CalcHeatBalanceInsideSurf(sim_state_);
  EnergyPlus::ZoneEquipmentManager::CalcAirFlowSimple(sim_state_);
  UpdateZoneConditions(true); // true means skip any connected zones which are not under EP control
  EnergyPlus::HeatBalanceAirManager::ReportZoneMeanAirTemp(sim_state_);
  EnergyPlus::HVACManager::ReportAirHeatBalance(sim_state_);
  EnergyPlus::InternalHeatGains::InitInternalHeatGains(sim_state_);
  EnergyPlus::InternalHeatGains::ReportInternalHeatGains(sim_state_);
  EnergyPlus::ScheduleManager::UpdateScheduleValues(sim_state_);
  EnergyPlus::HeatBalanceSurfaceManager::ReportSurfaceHeatBalance(sim_state_);
  energyplus::UpdateLatentGains(sim_state_);

  variables_.UpdateOutputs(sim_state_);

  need_update_ = false;
}

void Spawn::ExternalHVACManager([[maybe_unused]] EnergyPlusState state)
{
  if (sim_state_.dataGlobal->KickOffSimulation || sim_state_.dataGlobal->DoingSizing) {
    // ManageHVAC initializes many structures that need to exist.
    // Withouth calling this during the simulation kick off, the simulation will crash.
    // After kick off, we skip this call, because the client is managing the HVAC.
    EnergyPlus::HVACManager::ManageHVAC(sim_state_);
    // At this time, there is no data exchange or any other
    // interaction with the client during kick off, so this function returns and the
    // simulation continues through the startup process.
    return;
  }

  // "exchange" to get inputs, update internal EnergyPlus state, set outputs
  // "exchange" does not itself trigger an iteraction with the client
  Exchange(true);

  if (sim_state_.dataGlobal->WarmupFlag) {
    return;
  }

  // This is the part the signals and waits for the client
  // Only signal and wait for input if the current sim time is greather than or equal
  // to the requested time
  if (CurrentTime() >= requested_time_) {
    // Signal the end of the step
    {
      std::unique_lock<std::mutex> lk(sim_mutex_);
      iterate_flag_ = false;
    }

    iterate_cv_.notify_one();

    // Wait for the iterate_flag_ to signal another iteration
    std::unique_lock<std::mutex> lk(sim_mutex_);
    iterate_cv_.wait(lk, [&]() { return iterate_flag_; });
  }
}

void Spawn::SetLogCallback(std::function<void(EnergyPlus::Error, const std::string &)> cb)
{
  log_callback_ = std::move(cb);
}

void Spawn::LogMessage(EnergyPlus::Error level, const std::string &message)
{
  if (log_callback_ && !message.empty() && (level != EnergyPlus::Error::Info)) {
    log_message_queue_.emplace_back(level, message);
  }
}

void Spawn::EmptyLogMessageQueue()
{
  if (log_callback_) {
    while (!log_message_queue_.empty()) {
      auto m = log_message_queue_.front();
      log_callback_(m.first, m.second);
      log_message_queue_.pop_front();
    }
  }
}

EnergyPlusState Spawn::SimState()
{
  return static_cast<EnergyPlusState>(&sim_state_);
}

void Spawn::UpdateZoneConditions(bool skipConnectedZones)
{
  const double dt = CurrentTime() - prev_zone_update_;
  prev_zone_update_ = CurrentTime();
  if (dt > 0.0) {

    for (const auto &zone : user_config_.zones) {
      if (skipConnectedZones && zone.isconnected) {
        continue;
      }

      const auto zonenum = energyplus::ZoneNum(sim_state_, zone.idfname);
      energyplus::UpdateZoneTemperature(sim_state_, zonenum, dt);
      energyplus::UpdateZoneHumidityRatio(sim_state_, zonenum, dt);
    }
  }
}

} // namespace spawn
