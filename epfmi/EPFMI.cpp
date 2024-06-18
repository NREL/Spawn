#include "EPFMI.hpp"
#include "../energyplus_coroutine/spawn.hpp"
#include <fmi2Functions.h>
#include <iostream>
#include <list>
#include <regex>

using namespace std::placeholders;

namespace spawn {

std::list<spawn::Spawn> spawns;

struct Bad_Spawn_Pointer : public std::runtime_error
{
  Bad_Spawn_Pointer() : std::runtime_error("Invalid fmi2Component")
  {
  }
};

// FMI standard identifies components by void *. This function
// is used to retrieve the spawn::Spawn instance corresponding to the
// given pointer.
// Throw an exception if comp does not identify an instance in the
// spawn::spawns list
spawn::Spawn &get_spawn(void *comp)
{
  const auto it = std::find_if(
      std::begin(spawn::spawns), std::end(spawn::spawns), [comp](const spawn::Spawn &s) { return comp == &s; });

  if (it == std::end(spawn::spawns)) {
    throw Bad_Spawn_Pointer();
  }

  // if (auto ep = it->lastException()) {
  //  std::rethrow_exception(ep);
  //}

  return *it;
}

void remove_spawn(spawn::Spawn &comp)
{
  const auto it = std::find(spawn::spawns.begin(), spawn::spawns.end(), comp);
  if (it != spawn::spawns.end()) {
    spawn::spawns.erase(it);
  } else {
    throw std::runtime_error("Error in \"remove_spawn\"");
  }
}

fmi2Status handle_fmi_exception(spawn::Spawn &comp)
{
  try {
    // rethrow the last exception
    // This is the Lippincott pattern https://www.youtube.com/watch?v=-amJL3AyADI
    throw;
  } catch (const std::runtime_error &e) {
    comp.logMessage(EnergyPlus::Error::Fatal, e.what());
    comp.emptyLogMessageQueue();
  } catch (...) {
    std::clog << "Unknown Exception\n";
  }

  return fmi2Error;
}

template <typename Function> fmi2Status with_spawn(fmi2Component c, Function f)
{
  try {
    // Will throw if c is invalid
    auto &comp = spawn::get_spawn(c);
    try {
      f(comp);
      return fmi2OK;
    } catch (...) {
      // comp is passed so that we can log back the error message
      return spawn::handle_fmi_exception(comp);
    }
  } catch (const Bad_Spawn_Pointer &e) {
    std::clog << e.what() << "\n";
  } catch (...) {
    std::clog << "Unknown Exception\n";
  }
  return fmi2Error;
}

} // namespace spawn

EPFMI_API fmi2Component fmi2Instantiate(fmi2String instanceName,
                                        [[maybe_unused]] fmi2Type fmuType,
                                        fmi2String fmuGUID,
                                        fmi2String fmuResourceURI,
                                        const fmi2CallbackFunctions *functions,
                                        [[maybe_unused]] fmi2Boolean visible,
                                        fmi2Boolean loggingOn)
{
  try {
    // URI might be preceded by file:// (UNIX) or file:///C/ (windows)
    // https://en.wikipedia.org/wiki/File_URI_scheme
#if _WIN32
    // This may be brittle, but windows needs to be treated differently.
    // "C/" is a valid path, however /C/ is not valid
    const auto resourcePathString = std::regex_replace(fmuResourceURI, std::regex("^file:///"), "");
#else
    // On non windows the third "/" must be retained
    const auto resourcePathString = std::regex_replace(fmuResourceURI, std::regex("^file://"), "");
#endif
    const auto resourcePath = spawn_fs::path(resourcePathString);
    const auto spawnJSONPath = resourcePath / "model.spawn";
    const auto simulationWorkingDir = resourcePath.parent_path() / "eplusout/";

    spawn::spawns.emplace_back(fmuGUID, spawnJSONPath.string(), simulationWorkingDir);
    auto &comp = spawn::spawns.back();

    if (loggingOn != 0) {
???LINES MISSING
    spawn::remove_spawn(comp);
  };

  return spawn::with_spawn(c, action);
}

EPFMI_API const char *fmi2GetTypesPlatform(void)
{
  return fmi2TypesPlatform;
}

EPFMI_API const char *fmi2GetVersion(void)
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
  auto action = [&](spawn::Spawn &comp) { spawn::remove_spawn(comp); };

  spawn::with_spawn(c, action);
}

EPFMI_API fmi2Status fmi2EnterInitializationMode([[maybe_unused]] fmi2Component c)
{
  return fmi2OK;
}

EPFMI_API fmi2Status fmi2ExitInitializationMode(fmi2Component c)
{
  auto action = [&](spawn::Spawn &comp) { comp.start(); };

  return spawn::with_spawn(c, action);
}

EPFMI_API fmi2Status fmi2GetInteger(fmi2Component, const fmi2ValueReference[], size_t, fmi2Integer[])
{
  return fmi2OK;
}

EPFMI_API fmi2Status fmi2GetBoolean(fmi2Component, const fmi2ValueReference[], size_t, fmi2Boolean[])
{
  return fmi2OK;
}

EPFMI_API fmi2Status fmi2GetString(fmi2Component, const fmi2ValueReference[], size_t, fmi2String[])
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

EPFMI_API fmi2Status fmi2SetString(fmi2Component, const fmi2ValueReference[], size_t, const fmi2String[])
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

EPFMI_API fmi2Status fmi2CompletedIntegratorStep(fmi2Component,
                                                 fmi2Boolean,
                                                 fmi2Boolean *enterEventMode,
                                                 fmi2Boolean *terminateSimulation)
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
    spawn::remove_spawn(comp);
  };

  return spawn::with_spawn(c, action);
}

EPFMI_API const char *fmi2GetTypesPlatform(void)
{
  return fmi2TypesPlatform;
}

EPFMI_API const char *fmi2GetVersion(void)
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
  auto action = [&](spawn::Spawn &comp) { spawn::remove_spawn(comp); };

  spawn::with_spawn(c, action);
}

EPFMI_API fmi2Status fmi2EnterInitializationMode([[maybe_unused]] fmi2Component c)
{
  return fmi2OK;
}

EPFMI_API fmi2Status fmi2ExitInitializationMode(fmi2Component c)
{
  auto action = [&](spawn::Spawn &comp) { comp.start(); };

  return spawn::with_spawn(c, action);
}

EPFMI_API fmi2Status fmi2GetInteger(fmi2Component, const fmi2ValueReference[], size_t, fmi2Integer[])
{
  return fmi2OK;
}

EPFMI_API fmi2Status fmi2GetBoolean(fmi2Component, const fmi2ValueReference[], size_t, fmi2Boolean[])
{
  return fmi2OK;
}

EPFMI_API fmi2Status fmi2GetString(fmi2Component, const fmi2ValueReference[], size_t, fmi2String[])
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

EPFMI_API fmi2Status fmi2SetString(fmi2Component, const fmi2ValueReference[], size_t, const fmi2String[])
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

EPFMI_API fmi2Status fmi2CompletedIntegratorStep(fmi2Component,
                                                 fmi2Boolean,
                                                 fmi2Boolean *enterEventMode,
                                                 fmi2Boolean *terminateSimulation)
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
