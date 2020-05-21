#include "fmi.hpp"

namespace spawn {
namespace fmu {

  template <typename Func>
  bool load_function(util::Dynamic_Library &loader, FMI::Function<Func> &func, FMI::Load_Results &results)
  {
    try {
      const auto symbol = loader.load_symbol<Func>(func.name);
      func.function = symbol;
      results.successes.push_back(func.name);
      return true;
    } catch (const std::runtime_error &) {
      results.failures.push_back(func.name);
      return false;
    }
  }

  template <typename... Func>
  [[nodiscard]] FMI::Load_Results load_functions(util::Dynamic_Library &loader, FMI::Function<Func> &... func)
  {
    FMI::Load_Results results;
    (void)std::initializer_list<bool>{(load_function(loader, func, results))...};
    return results;
  }

  FMI::Load_Results FMI::loadFunctions(util::Dynamic_Library &loader) noexcept
  {
    return load_functions(loader,
                          fmi2GetTypesPlatform,
                          fmi2GetVersion,
                          fmi2SetDebugLogging,
                          fmi2Instantiate,
                          fmi2FreeInstance,
                          fmi2SetupExperiment,
                          fmi2EnterInitializationMode,
                          fmi2ExitInitializationMode,
                          fmi2Terminate,
                          fmi2Reset,
                          fmi2GetReal,
                          fmi2GetInteger,
                          fmi2GetBoolean,
                          fmi2GetString,
                          fmi2SetReal,
                          fmi2SetInteger,
                          fmi2SetBoolean,
                          fmi2SetString,
                          fmi2GetFMUstate,
                          fmi2SetFMUstate,
                          fmi2FreeFMUstate,
                          fmi2SerializedFMUstateSize,
                          fmi2SerializeFMUstate,
                          fmi2DeSerializeFMUstate,
                          fmi2GetDirectionalDerivative,
                          fmi2EnterEventMode,
                          fmi2NewDiscreteStates,
                          fmi2EnterContinuousTimeMode,
                          fmi2CompletedIntegratorStep,
                          fmi2SetTime,
                          fmi2SetContinuousStates,
                          fmi2GetDerivatives,
                          fmi2GetEventIndicators,
                          fmi2GetContinuousStates,
                          fmi2GetNominalsOfContinuousStates,
                          fmi2SetRealInputDerivatives,
                          fmi2GetRealOutputDerivatives,
                          fmi2DoStep,
                          fmi2CancelStep,
                          fmi2GetStatus,
                          fmi2GetRealStatus,
                          fmi2GetIntegerStatus,
                          fmi2GetBooleanStatus,
                          fmi2GetStringStatus);
  }


} // namespace fmu
} // namespace spawn
