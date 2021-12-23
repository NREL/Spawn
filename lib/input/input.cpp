#include "input.hpp"
#include "../idf_to_json.hpp"

using json = nlohmann::json;

namespace spawn {

Input::Input(const std::string & spawninput)
{
  std::ifstream fileinput(spawninput);
  if (!fileinput.fail()) {
    // deserialize from file
    fileinput >> spawnjson;
    m_basepath = fs::canonical(fs::path(spawninput)).parent_path();
  } else {
    // Try to parse command line input as json string
    spawnjson = json::parse(spawninput, nullptr, false);
    m_basepath = fs::current_path();
  }

  json weather;

  if (spawnjson.is_discarded()) {
    std::cout << "Cannot parse json: '" << spawninput << "'" << std::endl;
  }

  // Do the input paths exist?
  if (! fs::exists(idfInputPath())) {
    std::cout << "The specified idf input file does not exist, " << idfInputPath() << "." << std::endl;
  }
  if (! fs::exists(epwInputPath())) {
    std::cout << "The specified epw input file does not exist, " << epwInputPath() << "." << std::endl;
  }

  jsonidf = idf_to_json(idfInputPath());

  zones = Zone::createZones(spawnjson, jsonidf);
  schedules = Schedule::createSchedules(spawnjson, jsonidf);
  outputVariables = OutputVariable::createOutputVariables(spawnjson, jsonidf);
  emsActuators = EMSActuator::createEMSActuators(spawnjson, jsonidf);
  surfaces = Surface::createSurfaces(spawnjson, jsonidf);
}

std::string Input::fmuname() const {
  return spawnjson.value("fmu",json()).value("name","spawn.fmu");
}

std::string Input::fmuBaseName() const {
  return fs::path(fmuname()).stem().string();
}

void Input::setFMUName(std::string name) {
  spawnjson["fmu"]["name"] = std::move(name);
}

fs::path Input::toPath(const std::string & pathstring) const {
  fs::path p(pathstring);
  if (! p.is_absolute()) {
    p = m_basepath / p;
  }

  return p;
}

double Input::relativeSurfaceTolerance() const {
  const auto tol = spawnjson.value("EnergyPlus",json())["relativeSurfaceTolerance"];
  if (! tol.is_null()) {
    if (tol.is_number_float()) {
      return tol.get<double>();
    }
  }
  return 1.0e-6;
}

fs::path Input::idfInputPath() const {
  return toPath(spawnjson.value("EnergyPlus",json()).value("idf","in.idf"));
}

void Input::setIdfInputPath(fs::path idfpath) {
  spawnjson["EnergyPlus"]["idf"] = idfpath.string();
}

fs::path Input::epwInputPath() const {
  return toPath(spawnjson.value("EnergyPlus",json()).value("weather","in.epw"));
}

void Input::setEPWInputPath(fs::path epwpath) {
  spawnjson["EnergyPlus"]["weather"] = epwpath.string();
}

void Input::save(const fs::path & savepath) const {
  std::ofstream o(savepath.string());
  o << std::setw(4) << spawnjson << std::endl;
}

fs::path Input::basepath() const {
  return m_basepath;
}

} // namespace spawn

