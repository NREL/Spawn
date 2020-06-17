#include "EPFMI.hpp"
#include "../lib/spawn.hpp"
#include <fmi2Functions.h>
#include <boost/filesystem.hpp>
#include <list>
#include <regex>
#include <signal.h>

using namespace std::placeholders;

#define UNUSED(expr) do { (void)(expr); } while (0);

std::list<spawn::Spawn> spawns;

EPFMI_API fmi2Component fmi2Instantiate(fmi2String instanceName,
  fmi2Type fmuType,
  fmi2String fmuGUID,
  fmi2String fmuResourceURI,
  const fmi2CallbackFunctions* functions,
  fmi2Boolean visible,
  fmi2Boolean loggingOn)
{
  UNUSED(fmuType);
  UNUSED(visible);

  const auto resourcePathString = std::regex_replace(fmuResourceURI, std::regex("^file://"), "");
  const auto resourcePath = boost::filesystem::path(resourcePathString);
  const auto spawnJSONPath = resourcePath / "../model.spawn";

  spawns.emplace_back(fmuGUID, spawnJSONPath.string());
  auto & comp = spawns.back();

	if (loggingOn) {
		const auto & logger = [functions, instanceName](const std::string & message) {
			const auto & env = functions->componentEnvironment;
			functions->logger(env, instanceName, fmi2Error, "EnergyPlus Error", message.c_str());
		};
		comp.setLogCallback(logger);
	}

  return &comp;
}

EPFMI_API fmi2Status fmi2SetupExperiment(fmi2Component c,
  fmi2Boolean toleranceDefined,
  fmi2Real tolerance,
  fmi2Real starttime,
  fmi2Boolean stopTimeDefined,
  fmi2Real stopTime)
{
  auto * epcomp = static_cast<spawn::Spawn*>(c);

  UNUSED(toleranceDefined);
  UNUSED(tolerance);
  UNUSED(stopTimeDefined);
  UNUSED(stopTime);

  return epcomp->start(starttime) ? fmi2Error : fmi2OK;
}

EPFMI_API fmi2Status fmi2SetTime(fmi2Component c, fmi2Real time)
{
  auto * epcomp = static_cast<spawn::Spawn*>(c);

  return epcomp->setTime(time) ? fmi2Error : fmi2OK;
}

EPFMI_API fmi2Status fmi2SetReal(fmi2Component c,
  const fmi2ValueReference vr[],
  size_t nvr,
  const fmi2Real values[])
{
  auto * epcomp = static_cast<spawn::Spawn*>(c);

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
  auto * epcomp = static_cast<spawn::Spawn*>(c);

  epcomp->exchange();

  std::transform(vr, std::next(vr, nvr), values, [&](const int valueRef){ return epcomp->getValue(valueRef); });

  return fmi2OK;
}

EPFMI_API fmi2Status fmi2NewDiscreteStates(fmi2Component  c, fmi2EventInfo* eventInfo)
{
  auto * epcomp = static_cast<spawn::Spawn*>(c);

  eventInfo->newDiscreteStatesNeeded = fmi2False;
  eventInfo->nextEventTime = epcomp->nextSimTime();
  eventInfo->nextEventTimeDefined = fmi2True;
  eventInfo->terminateSimulation = fmi2False;

  return fmi2OK;
}

EPFMI_API fmi2Status fmi2Terminate(fmi2Component c)
{
  auto * epcomp = static_cast<spawn::Spawn*>(c);

  const auto result = epcomp->stop();

  const auto it = std::find(spawns.begin(), spawns.end(), *epcomp);
  if (it != spawns.end()) {
    spawns.erase(it);
  }

  return result ? fmi2Error : fmi2OK;
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

EPFMI_API fmi2Status fmi2Reset(fmi2Component)
{
  return fmi2OK;
}

EPFMI_API void fmi2FreeInstance(fmi2Component c)
{
  auto * epcomp = static_cast<spawn::Spawn*>(c);

  const auto it = std::find(spawns.begin(), spawns.end(), *epcomp);
  if (it != spawns.end()) {
    spawns.erase(it);
  }

  c = nullptr; // this value can never be read, what is the aim?
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


