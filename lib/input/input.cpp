#include "input.hpp"
#include "../../util/idf_to_json.hpp"

using json = nlohmann::json;

namespace spawn {

Input::Input(const std::string & spawninput)
{
  std::ifstream fileinput(spawninput);
  if (!fileinput.fail()) {
    // deserialize from file
    fileinput >> spawnjson;
    m_basepath = std::filesystem::canonical(std::filesystem::path(spawninput)).parent_path();
  } else {
    // Try to parse command line input as json string
    spawnjson = json::parse(spawninput, nullptr, false);
    m_basepath = std::filesystem::current_path();
  }

  json weather;

  if (spawnjson.is_discarded()) {
    std::cout << "Cannot parse json: '" << spawninput << "'" << std::endl;
  }

  // Do the input paths exist?
  if (! std::filesystem::exists(idfInputPath())) {
    std::cout << "The specified idf input file does not exist, " << idfInputPath() << "." << std::endl;
  }
  if (! std::filesystem::exists(epwInputPath())) {
    std::cout << "The specified epw input file does not exist, " << epwInputPath() << "." << std::endl;
  }

  jsonidf = idfToJSON(idfInputPath());

  zones = Zone::createZones(spawnjson, jsonidf);
  schedules = Schedule::createSchedules(spawnjson, jsonidf);
  outputVariables = OutputVariable::createOutputVariables(spawnjson, jsonidf);
  emsActuators = EMSActuator::createEMSActuators(spawnjson, jsonidf);
  surfaces = Surface::createSurfaces(spawnjson, jsonidf);
}

std::string Input::fmuname() const {
  return spawnjson["fmu"].value("name","spawn.fmu");
}

std::string Input::fmuBaseName() const {
  return std::filesystem::path(fmuname()).stem().string();
}

void Input::setFMUName(const std::string & name) {
  spawnjson["fmu"]["name"] = name;
}

std::filesystem::path Input::toPath(const std::string & pathstring) const {
  std::filesystem::path p(pathstring);
  if (! p.is_absolute()) {
    p = m_basepath / p;
  }

  return p;
}

std::filesystem::path Input::idfInputPath() const {
  return toPath(spawnjson["EnergyPlus"].value("idf",""));
}

void Input::setIdfInputPath(std::filesystem::path idfpath) {
  spawnjson["EnergyPlus"]["idf"] = idfpath.string();
}

std::filesystem::path Input::epwInputPath() const {
  return toPath(spawnjson["EnergyPlus"].value("weather",""));
}

void Input::setEPWInputPath(std::filesystem::path epwpath) {
  spawnjson["EnergyPlus"]["weather"] = epwpath.string();
}

void Input::save(const std::filesystem::path & savepath) const {
  std::ofstream o(savepath.string());
  o << std::setw(4) << spawnjson << std::endl;
}

std::filesystem::path Input::basepath() const {
  return m_basepath;
}

} // namespace spawn

