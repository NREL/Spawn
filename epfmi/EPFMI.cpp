#include "EPFMI.hpp"
#include "Variables.hpp"
#include "EPComponent.hpp"
#include <fmi2Functions.h>
#include <boost/filesystem.hpp>
#include <list>
#include <regex>

using namespace std::placeholders;

#define UNUSED(expr) do { (void)(expr); } while (0);

std::list<EPComponent> epComponents;

EPFMI_API fmi2Component fmi2Instantiate(fmi2String instanceName,
  fmi2Type fmuType,
  fmi2String fmuGUID,
  fmi2String fmuResourceURI,
  const fmi2CallbackFunctions* functions,
  fmi2Boolean visible,
  fmi2Boolean loggingOn)
{
  UNUSED(fmuType);
  UNUSED(fmuGUID);
  UNUSED(visible);

	const auto resourcePathString = std::regex_replace(fmuResourceURI, std::regex("^file://"), "");
  const auto resourcePath = boost::filesystem::path(resourcePathString);

  epComponents.emplace_back(instanceName,resourcePath);
  auto & comp = epComponents.back();
  comp.logger = functions->logger;
  comp.loggingOn = loggingOn;

  return &comp;
}

EPFMI_API fmi2Status fmi2SetupExperiment(fmi2Component c,
  fmi2Boolean toleranceDefined,
  fmi2Real tolerance,
  fmi2Real startTime,
  fmi2Boolean stopTimeDefined,
  fmi2Real stopTime)
{
  EPComponent * epcomp = static_cast<EPComponent*>(c);

  UNUSED(toleranceDefined);
  UNUSED(tolerance);
  UNUSED(startTime);
  UNUSED(stopTimeDefined);
  UNUSED(stopTime);

  epcomp->start();

  return fmi2OK;
}

EPFMI_API fmi2Status fmi2SetTime(fmi2Component c, fmi2Real time)
{
  EPComponent * epcomp = static_cast<EPComponent*>(c);

  epcomp->setTime(time);

  return fmi2OK;
}

EPFMI_API fmi2Status fmi2SetReal(fmi2Component c,
  const fmi2ValueReference vr[],
  size_t nvr,
  const fmi2Real values[])
{
  EPComponent * epcomp = static_cast<EPComponent*>(c);

  for ( size_t i = 0; i < nvr; ++i ) {
    auto valueRef = vr[i];
    auto value = values[i];
    epcomp->setValue(valueRef, value);
  }

  return fmi2OK;
}

EPFMI_API fmi2Status fmi2GetReal(fmi2Component c,
  const fmi2ValueReference vr[],
  size_t nvr,
  fmi2Real values[])
{
  EPComponent * epcomp = static_cast<EPComponent*>(c);

  epcomp->exchange();

  for ( size_t i = 0; i < nvr; ++i ) {
    auto valueRef = vr[i];
    values[i] = epcomp->getValue(valueRef);
  }

  return fmi2OK;
}

EPFMI_API fmi2Status fmi2NewDiscreteStates(fmi2Component  c, fmi2EventInfo* eventInfo)
{
  EPComponent * epcomp = static_cast<EPComponent*>(c);

  eventInfo->newDiscreteStatesNeeded = fmi2False;
  eventInfo->nextEventTime = epcomp->nextSimTime();
  eventInfo->nextEventTimeDefined = fmi2True;
  eventInfo->terminateSimulation = fmi2False;

  return fmi2OK;
}

EPFMI_API fmi2Status fmi2Terminate(fmi2Component c)
{
  EPComponent * epcomp = static_cast<EPComponent*>(c);

  epcomp->stop();

  auto it = std::find(epComponents.begin(), epComponents.end(), *epcomp);
  epComponents.erase(it);

  return fmi2OK;
}

EPFMI_API const char* fmi2GetTypesPlatform(void)
{
  return fmi2TypesPlatform;
}

EPFMI_API const char* fmi2GetVersion(void)
{
  return fmi2Version;
}

EPFMI_API fmi2Status fmi2SetDebugLogging(fmi2Component, fmi2Boolean, size_t, const fmi2String[])
{
  return fmi2OK;
}

EPFMI_API fmi2Status fmi2Reset(fmi2Component c)
{
  EPComponent * epcomp = static_cast<EPComponent*>(c);

  return fmi2OK;
}

EPFMI_API void fmi2FreeInstance(fmi2Component c)
{
  EPComponent * epcomp = static_cast<EPComponent*>(c);

  auto it = std::find(epComponents.begin(), epComponents.end(), *epcomp);
  epComponents.erase(it);

  c = nullptr;
}

EPFMI_API fmi2Status fmi2EnterInitializationMode(fmi2Component)
{
  return fmi2OK;
}

EPFMI_API fmi2Status fmi2ExitInitializationMode(fmi2Component)
{
  return fmi2OK;
}

EPFMI_API fmi2Status fmi2GetInteger(fmi2Component, const fmi2ValueReference[], size_t, fmi2Integer[])
{
  return fmi2OK;
}

EPFMI_API fmi2Status fmi2GetBoolean(fmi2Component, const fmi2ValueReference[], size_t, fmi2Boolean[])
{
  return fmi2OK;
}

EPFMI_API fmi2Status fmi2GetString(fmi2Component, const fmi2ValueReference[], size_t, fmi2String [])
{
  return fmi2OK;
}

EPFMI_API fmi2Status fmi2SetInteger(fmi2Component, const fmi2ValueReference[], size_t, const fmi2Integer[])
{
  return fmi2OK;
}

EPFMI_API fmi2Status fmi2SetBoolean(fmi2Component, const fmi2ValueReference[], size_t, const fmi2Boolean[])
{
  return fmi2OK;
}

EPFMI_API fmi2Status fmi2SetString(fmi2Component, const fmi2ValueReference[], size_t, const fmi2String [])
{
  return fmi2OK;
}

EPFMI_API fmi2Status fmi2EnterEventMode(fmi2Component)
{
  return fmi2OK;
}


EPFMI_API fmi2Status fmi2EnterContinuousTimeMode(fmi2Component)
{
  return fmi2OK;
}

EPFMI_API fmi2Status fmi2CompletedIntegratorStep(fmi2Component, fmi2Boolean, fmi2Boolean* enterEventMode, fmi2Boolean* terminateSimulation)
{
  // TODO: What happens if we get to the end of the run period?
  // Consider setting terminateSimulation to true
  *terminateSimulation = fmi2False;
  *enterEventMode = fmi2False;

  return fmi2OK;
}

EPFMI_API fmi2Status fmi2SetContinuousStates(fmi2Component, const fmi2Real[], size_t)
{
  return fmi2OK;
}

EPFMI_API fmi2Status fmi2GetDerivatives(fmi2Component, fmi2Real[], size_t)
{
  return fmi2OK;
}

EPFMI_API fmi2Status fmi2GetEventIndicators(fmi2Component, fmi2Real[], size_t)
{
  return fmi2OK;
}

EPFMI_API fmi2Status fmi2GetContinuousStates(fmi2Component, fmi2Real[], size_t)
{
  return fmi2OK;
}

EPFMI_API fmi2Status fmi2GetNominalsOfContinuousStates(fmi2Component, fmi2Real[], size_t)
{
  return fmi2OK;
}


