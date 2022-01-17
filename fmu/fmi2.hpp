
#ifndef SPAWN_FMI2_HPP
#define SPAWN_FMI2_HPP

#include "../util/dynamiclibrary.hpp"
#include <fmi2FunctionTypes.h>
#include <functional>
#include <string>
#include <vector>

namespace spawn::fmu {

/// FMI2 wrapper, lowest level interface for FMI2 / FMU, provides raw
/// access to FMI2 functions loaded from the given dynami library
class FMI2
{
public:
  template <typename FunctionSignature> struct Function
  {
    std::function<FunctionSignature> function{};
    std::string name{};

    explicit Function(std::string t_name) noexcept : name{std::move(t_name)}
    {
    }

    // convenience wrapper for calling this Function as if it were the wrapped function
    template <typename... Param> decltype(auto) operator()(Param &&...param) const
    {
      return function(std::forward<Param>(param)...);
    }

    [[nodiscard]] explicit operator bool() const noexcept
    {
      return static_cast<bool>(function);
    }

    [[nodiscard]] bool has_value() const noexcept
    {
      return static_cast<bool>(*this);
    }
  };

  struct Load_Results
  {
    std::vector<std::string> successes;
    std::vector<std::string> failures;
  };

  /// Loads the given dynamic library and attempts to load all possible FMI2 symbols
  FMI2(spawn_fs::path fmi_file, bool require_all_symbols) : m_fmi_file{std::move(fmi_file)}
  {
    if (require_all_symbols && !m_loadResults.failures.empty()) {
      throw std::runtime_error("Failed to load all functions");
    }
  }

  Function<fmi2GetTypesPlatformTYPE> fmi2GetTypesPlatform{"fmi2GetTypesPlatform"};
  Function<fmi2GetVersionTYPE> fmi2GetVersion{"fmi2GetVersion"};
  Function<fmi2SetDebugLoggingTYPE> fmi2SetDebugLogging{"fmi2SetDebugLogging"};
  Function<fmi2InstantiateTYPE> fmi2Instantiate{"fmi2Instantiate"};
  Function<fmi2FreeInstanceTYPE> fmi2FreeInstance{"fmi2FreeInstance"};
  Function<fmi2SetupExperimentTYPE> fmi2SetupExperiment{"fmi2SetupExperiment"};
  Function<fmi2EnterInitializationModeTYPE> fmi2EnterInitializationMode{"fmi2EnterInitializationMode"};
  Function<fmi2ExitInitializationModeTYPE> fmi2ExitInitializationMode{"fmi2ExitInitializationMode"};
  Function<fmi2TerminateTYPE> fmi2Terminate{"fmi2Terminate"};
  Function<fmi2ResetTYPE> fmi2Reset{"fmi2Reset"};
  Function<fmi2GetRealTYPE> fmi2GetReal{"fmi2GetReal"};
  Function<fmi2GetIntegerTYPE> fmi2GetInteger{"fmi2GetInteger"};
  Function<fmi2GetBooleanTYPE> fmi2GetBoolean{"fmi2GetBoolean"};
  Function<fmi2GetStringTYPE> fmi2GetString{"fmi2GetString"};
  Function<fmi2SetRealTYPE> fmi2SetReal{"fmi2SetReal"};
  Function<fmi2SetIntegerTYPE> fmi2SetInteger{"fmi2SetInteger"};
  Function<fmi2SetBooleanTYPE> fmi2SetBoolean{"fmi2SetBoolean"};
  Function<fmi2SetStringTYPE> fmi2SetString{"fmi2SetString"};
  Function<fmi2GetFMUstateTYPE> fmi2GetFMUstate{"fmi2GetFMUstate"};
  Function<fmi2SetFMUstateTYPE> fmi2SetFMUstate{"fmi2SetFMUstate"};
  Function<fmi2FreeFMUstateTYPE> fmi2FreeFMUstate{"fmi2FreeFMUstate"};
  Function<fmi2SerializedFMUstateSizeTYPE> fmi2SerializedFMUstateSize{"fmi2SerializedFMUstateSize"};
  Function<fmi2SerializeFMUstateTYPE> fmi2SerializeFMUstate{"fmi2SerializeFMUstate"};
  Function<fmi2DeSerializeFMUstateTYPE> fmi2DeSerializeFMUstate{"fmi2DeSerializeFMUstate"};
  Function<fmi2GetDirectionalDerivativeTYPE> fmi2GetDirectionalDerivative{"fmi2GetDirectionalDerivative"};
  Function<fmi2EnterEventModeTYPE> fmi2EnterEventMode{"fmi2EnterEventMode"};
  Function<fmi2NewDiscreteStatesTYPE> fmi2NewDiscreteStates{"fmi2NewDiscreteStates"};
  Function<fmi2EnterContinuousTimeModeTYPE> fmi2EnterContinuousTimeMode{"fmi2EnterContinuousTimeMode"};
  Function<fmi2CompletedIntegratorStepTYPE> fmi2CompletedIntegratorStep{"fmi2CompletedIntegratorStep"};
  Function<fmi2SetTimeTYPE> fmi2SetTime{"fmi2SetTime"};
  Function<fmi2SetContinuousStatesTYPE> fmi2SetContinuousStates{"fmi2SetContinuousStates"};
  Function<fmi2GetDerivativesTYPE> fmi2GetDerivatives{"fmi2GetDerivatives"};
  Function<fmi2GetEventIndicatorsTYPE> fmi2GetEventIndicators{"fmi2GetEventIndicators"};
  Function<fmi2GetContinuousStatesTYPE> fmi2GetContinuousStates{"fmi2GetContinuousStates"};
  Function<fmi2GetNominalsOfContinuousStatesTYPE> fmi2GetNominalsOfContinuousStates{
      "fmi2GetNominalsOfContinuousStates"};
  Function<fmi2SetRealInputDerivativesTYPE> fmi2SetRealInputDerivatives{"fmi2SetRealInputDerivatives"};
  Function<fmi2GetRealOutputDerivativesTYPE> fmi2GetRealOutputDerivatives{"fmi2GetRealOutputDerivatives"};
  Function<fmi2DoStepTYPE> fmi2DoStep{"fmi2DoStep"};
  Function<fmi2CancelStepTYPE> fmi2CancelStep{"fmi2CancelStep"};
  Function<fmi2GetStatusTYPE> fmi2GetStatus{"fmi2GetStatus"};
  Function<fmi2GetRealStatusTYPE> fmi2GetRealStatus{"fmi2GetRealStatus"};
  Function<fmi2GetIntegerStatusTYPE> fmi2GetIntegerStatus{"fmi2GetIntegerStatus"};
  Function<fmi2GetBooleanStatusTYPE> fmi2GetBooleanStatus{"fmi2GetBooleanStatus"};
  Function<fmi2GetStringStatusTYPE> fmi2GetStringStatus{"fmi2GetStringStatus"};

  // returns the result of loading each possible function
  // in the FMI dynamic library
  [[nodiscard]] const Load_Results &loadResults() const noexcept
  {
    return m_loadResults;
  }

private:
  [[nodiscard]] Load_Results loadFunctions(util::Dynamic_Library &) noexcept;

  spawn_fs::path m_fmi_file;
  util::Dynamic_Library m_dll{m_fmi_file};
  Load_Results m_loadResults{loadFunctions(m_dll)};
};
} // namespace spawn::fmu

#endif // SPAWN_FMI2_HPP
