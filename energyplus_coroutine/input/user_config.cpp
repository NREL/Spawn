#include "../idf_to_json.hpp"
#include "user_config.hpp"

using json = nlohmann::json;

namespace spawn {

UserConfig::UserConfig(const std::string &spawninput)
{
  std::ifstream fileinput(spawninput);
  if (!fileinput.fail()) {
    // deserialize from file
    fileinput >> spawnjson;
    m_basepath = spawn_fs::canonical(spawn_fs::path(spawninput)).parent_path();
  } else {
    // Try to parse command line input as json string
    spawnjson = json::parse(spawninput, nullptr, false);
    m_basepath = spawn_fs::current_path();
  }

  json weather;

  if (spawnjson.is_discarded()) {
    std::cout << "Cannot parse json: '" << spawninput << "'" << std::endl;
  }

  // Do the input paths exist?
  if (!spawn_fs::exists(idfInputPath())) {
    std::cout << "The specified idf input file does not exist, " << idfInputPath() << "." << std::endl;
  }
  if (!spawn_fs::exists(epwInputPath())) {
    std::cout << "The specified epw input file does not exist, " << epwInputPath() << "." << std::endl;
  }

  jsonidf = idf_to_json(idfInputPath());

  zones = Zone::createZones(spawnjson, jsonidf);
  schedules = Schedule::createSchedules(spawnjson, jsonidf);
  outputVariables = input::OutputVariable::createOutputVariables(spawnjson, jsonidf);
  emsActuators = EMSActuator::createEMSActuators(spawnjson, jsonidf);
  surfaces = Surface::createSurfaces(spawnjson, jsonidf);
  runPeriod = RunPeriod::create_run_period(spawnjson);
}

std::string UserConfig::fmuname() const
{
  return spawnjson.value("fmu", json()).value("name", "spawn.fmu");
}

std::string UserConfig::fmuBaseName() const
{
  return spawn_fs::path(fmuname()).stem().string();
}

void UserConfig::setFMUName(std::string name)
{
  spawnjson["fmu"]["name"] = std::move(name);
}

spawn_fs::path UserConfig::toPath(const std::string &pathstring) const
{
  spawn_fs::path p(pathstring);
  if (!p.is_absolute()) {
    p = m_basepath / p;
  }

  return p;
}

double UserConfig::relativeSurfaceTolerance() const
{
  const auto tol = spawnjson.value("EnergyPlus", json())["relativeSurfaceTolerance"];
  if (!tol.is_null()) {
    if (tol.is_number_float()) {
      return tol.get<double>();
    }
  }
  return 1.0e-6;
}

spawn_fs::path UserConfig::idfInputPath() const
{
  return toPath(spawnjson.value("EnergyPlus", json()).value("idf", "in.idf"));
}

void UserConfig::setIdfInputPath(const spawn_fs::path &idfpath)
{
  spawnjson["EnergyPlus"]["idf"] = idfpath.string();
}

spawn_fs::path UserConfig::epwInputPath() const
{
  return toPath(spawnjson.value("EnergyPlus", json()).value("weather", "in.epw"));
}

void UserConfig::setEPWInputPath(const spawn_fs::path &epwpath)
{
  spawnjson["EnergyPlus"]["weather"] = epwpath.string();
}

bool UserConfig::autosize() const
{
  return spawnjson.value("EnergyPlus", json()).value("autosize", false);
}

void UserConfig::save(const spawn_fs::path &savepath) const
{
  std::ofstream o(savepath.string());
  o << std::setw(4) << spawnjson << std::endl;
}

spawn_fs::path UserConfig::basepath() const
{
  return m_basepath;
}

} // namespace spawn
