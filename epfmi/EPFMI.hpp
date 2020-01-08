#include "EPFMI_Export.hpp"
#include <cstddef>
#include <fmi2FunctionTypes.h>

extern "C" {

EPFMI_API fmi2Component fmi2Instantiate(fmi2String instanceName,
  fmi2Type fmuType,
  fmi2String fmuGUID,
  fmi2String fmuResourceLocation,
  const fmi2CallbackFunctions* functions,
  fmi2Boolean visible,
  fmi2Boolean loggingOn);

EPFMI_API fmi2Status fmi2SetupExperiment(fmi2Component c,
  fmi2Boolean toleranceDefined,
  fmi2Real tolerance,
  fmi2Real startTime,
  fmi2Boolean stopTimeDefined,
  fmi2Real stopTime);

EPFMI_API fmi2Status fmi2SetTime(fmi2Component c, fmi2Real time);

EPFMI_API fmi2Status fmi2SetReal(fmi2Component c,
  const fmi2ValueReference vr[],
  size_t nvr,
  const fmi2Real values[]);

EPFMI_API fmi2Status fmi2GetReal(fmi2Component c,
  const fmi2ValueReference vr[],
  size_t nvr,
  fmi2Real values[]);

EPFMI_API fmi2Status fmi2NewDiscreteStates(fmi2Component  c, fmi2EventInfo* fmi2eventInfo);

EPFMI_API fmi2Status fmi2Terminate(fmi2Component c);

EPFMI_API const char* fmi2GetTypesPlatform(void);

EPFMI_API const char* fmi2GetVersion(void);

EPFMI_API fmi2Status fmi2SetDebugLogging(fmi2Component, fmi2Boolean, size_t, const fmi2String[]);

EPFMI_API fmi2Status fmi2Reset(fmi2Component);

EPFMI_API void fmi2FreeInstance(fmi2Component);

EPFMI_API fmi2Status fmi2EnterInitializationMode(fmi2Component);

EPFMI_API fmi2Status fmi2ExitInitializationMode(fmi2Component);

EPFMI_API fmi2Status fmi2GetInteger(fmi2Component, const fmi2ValueReference[], size_t, fmi2Integer[]);

EPFMI_API fmi2Status fmi2GetBoolean(fmi2Component, const fmi2ValueReference[], size_t, fmi2Boolean[]);

EPFMI_API fmi2Status fmi2GetString(fmi2Component, const fmi2ValueReference[], size_t, fmi2String []);

EPFMI_API fmi2Status fmi2SetInteger(fmi2Component, const fmi2ValueReference[], size_t, const fmi2Integer[]);

EPFMI_API fmi2Status fmi2SetBoolean(fmi2Component, const fmi2ValueReference[], size_t, const fmi2Boolean[]);

EPFMI_API fmi2Status fmi2SetString(fmi2Component, const fmi2ValueReference[], size_t, const fmi2String []);

EPFMI_API fmi2Status fmi2EnterEventMode(fmi2Component);

EPFMI_API fmi2Status fmi2EnterContinuousTimeMode(fmi2Component);

EPFMI_API fmi2Status fmi2CompletedIntegratorStep(fmi2Component, fmi2Boolean, fmi2Boolean*, fmi2Boolean*);

EPFMI_API fmi2Status fmi2SetContinuousStates(fmi2Component, const fmi2Real[], size_t);

EPFMI_API fmi2Status fmi2GetDerivatives(fmi2Component, fmi2Real[], size_t);

EPFMI_API fmi2Status fmi2GetEventIndicators(fmi2Component, fmi2Real[], size_t);

EPFMI_API fmi2Status fmi2GetContinuousStates(fmi2Component, fmi2Real[], size_t);

EPFMI_API fmi2Status fmi2GetNominalsOfContinuousStates(fmi2Component, fmi2Real[], size_t);

}


