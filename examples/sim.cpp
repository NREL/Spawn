
#include "../fmu/modelexchange.hpp"
#include <fmt/format.h>
#include <iostream>
#include <spdlog/spdlog.h>

int main(const int argc, const char *argv[])
{

  // adapted from sim.py
  // referenced:
  //  * https://github.com/qtronic/fmusdk/blob/master/fmu20/src/model_exchange/main.c
  //  * https://github.com/qtronic/fmusdk/blob/master/fmu20/src/co_simulation/main.c
  //  * https://github.com/modelon-community/PyFMI/blob/9a737478b63b5c7d835eb1f9de4d5792108ca307/src/pyfmi/fmi.pyx
  spdlog::set_level(spdlog::level::trace);
  if (argc != 2) {
    spdlog::error("Usage: '{} <filename>'\n", argv[0]);
    return EXIT_FAILURE;
  }

  const auto dump_variable = [](const spawn::fmu::FMU::Variable &variable) {
    spdlog::trace(R"(Variable: {} {} "{}" {})",
                  variable.name,
                  variable.valueReference,
                  variable.description,
                  spawn::fmu::FMU::Variable::to_string(variable.type));
  };

  auto model = spawn::fmu::ModelExchange{fs::path{argv[1]}, "model", true, false};

  const auto &core_zn_t = model.fmu.getVariableByName("Core_ZN_T");
  const auto start_time = 0.0;
  const auto final_time = 60.0 * 10;

  dump_variable(core_zn_t);

  auto temp = 296.15;
  model.setVariable(core_zn_t, temp);
  model.fmu.fmi.fmi2SetupExperiment(model.component, false, 0.0, start_time, true, final_time);

  // xml ModelDescription
  // xml attribute modelName
  // print("model name: ", model.get_name())
  // xml attribute modelIdentifier
  // print("model identifier: ", model.get_identifier())
  // xml attribute version
  // print("model version: ", model.get_version())
  // xml attribute: generationTool
  // print("model generation tool: ", model.get_generation_tool())
  // xml attribute guid
  // print("model guid: ", model.get_guid())
  // xml attribute generationDateTime
  // print("model generation-date-time: ", model.get_generation_date_and_time())

  spdlog::debug("model platform: {}", model.fmu.fmi.fmi2GetTypesPlatform());

  const auto outputVariables = [&]() {
    std::vector<spawn::fmu::FMU::Variable> retval;
    spdlog::trace("Output Variables: ");
    for (const auto &variable : model.fmu.getVariables()) {
      if (variable.causality == spawn::fmu::FMU::Variable::Causality::Output) {
        model.visitVariable(
            [&](const auto &value) { spdlog::debug("outputs: {}: value ==> {}", variable.name, value); }, variable);

        retval.push_back(variable);
      }
    }
    return retval;
  }();

  auto t = start_time;
  auto last_time = start_time;
  auto densityAir = 1.276;    // kg / m ^ 3
  auto heatCapacity = 1000.6; // J / kgK
  auto volume = model.getVariable<double>("Core_ZN_V");


  while (t < final_time) {
    model.fmu.fmi.fmi2SetTime(model.component, t);
    model.setVariable(core_zn_t, temp);

    auto people = 0.0;
    auto lights = 0.0;

    // if we are incrementing 6 seconds at a time, this is likely to never be hit.
    if (static_cast<int>(t) % 120 == 0) {
      people = 5;
      lights = 1;
    } else {
      people = 1;
      lights = 0;
    }

    model.setVariable("Core_Zone_People", people);
    model.setVariable("Lights_Schedule", lights);

    const auto get_as_string = [&](std::string_view sv) {
      return model.visitVariable([](const auto &val) { return fmt::to_string(val); }, model.fmu.getVariableByName(sv));
    };

    spdlog::debug("t = {}", t);
    spdlog::debug("temp = {}", temp);
    spdlog::debug("core zone temperature: {}", get_as_string("Zone_Temperature"));
    spdlog::debug("attic zone temperature: {}", get_as_string("Attic_Zone_Temperature"));
    spdlog::debug("outside temperature: {}", get_as_string("Outside_Temperature"));
    spdlog::debug("Core_ZN_QConSen_flow: {}", get_as_string("Core_ZN_QConSen_flow"));
    spdlog::debug("people = {}", people);
    spdlog::debug("Core_Zone_People_Output: {}", get_as_string("Core_Zone_People_Output"));
    spdlog::debug("lights = {}", lights);
    spdlog::debug("Core_Zone_Lights_Output: {}", get_as_string("Core_Zone_Lights_Output"));

    model.setVariable("Lights_Schedule", lights * 2.0);
    spdlog::debug("Core_Zone_Lights_Output: {}", get_as_string("Core_Zone_Lights_Output"));

    const auto dt = t - last_time;
    const auto tempDot = model.getVariable<double>("Core_ZN_QConSen_flow") / (volume * densityAir * heatCapacity);
    temp = temp + (dt * tempDot);

    // model.event_update();
    fmi2EventInfo eventInfo{};
    eventInfo.newDiscreteStatesNeeded = fmi2True;
    bool nominals_continuous_states_changed = false;
    bool values_continuous_states_changed = false;

    while(eventInfo.newDiscreteStatesNeeded == fmi2True) {
      const auto status = model.fmu.fmi.fmi2NewDiscreteStates(model.component, &eventInfo);

      if (eventInfo.nominalsOfContinuousStatesChanged == fmi2True) {
        nominals_continuous_states_changed = true;
      }

      if (eventInfo.valuesOfContinuousStatesChanged == fmi2True) {
        values_continuous_states_changed = true;
      }

      if (status != 0) {
//        throw std::runtime_error(FMUException('Failed to update the events at time: %E.' % self.time));
        throw std::runtime_error("Failed to update the events at time: %E.");
      }
    }

    // If the values in the event struct have been overwritten.
    if (nominals_continuous_states_changed) {
      eventInfo.nominalsOfContinuousStatesChanged = fmi2True;
    }

    if (values_continuous_states_changed) {
      eventInfo.valuesOfContinuousStatesChanged = fmi2True;
    }


    //    const auto event_info = model.get_event_info();


    last_time = t;
    t = t + 7;

    if (t > eventInfo.nextEventTime) {
       t = eventInfo.nextEventTime;
    }
  }

  model.fmu.fmi.fmi2Terminate(model.component);
}
