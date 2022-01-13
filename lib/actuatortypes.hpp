#ifndef ACTUATORTYPES_HH_INCLUDED
#define ACTUATORTYPES_HH_INCLUDED

#include "units.hpp"
#include <map>
#include <nlohmann/json.hpp>

struct ActuatorProperties
{
  ActuatorProperties() = delete;

  std::string componentType;
  std::string controlType;
  spawn::units::UnitType moUnitType;
  spawn::units::UnitType epUnitType;
};

static void to_json(nlohmann::json &j, const ActuatorProperties &p)
{
  j = nlohmann::json{{"componentType", p.componentType},
                     {"controlType", p.controlType},
                     {"modelicaUnit", spawn::units::toString(p.moUnitType)},
                     {"energyplusUnit", spawn::units::toString(p.epUnitType)}};
}

[[maybe_unused]] static void from_json(const nlohmann::json &j, ActuatorProperties &p)
{
  p.componentType = j.at("componentType").get<std::string>();
  p.controlType = j.at("controlType").get<std::string>();
  p.moUnitType = spawn::units::fromString(j.at("modelicaUnit").get<std::string>());
  p.epUnitType = spawn::units::fromString(j.at("energyplusUnit").get<std::string>());
}

const std::vector<ActuatorProperties> actuatortypes{
    {"Weather Data", "Outdoor Dry Bulb", spawn::units::UnitType::K, spawn::units::UnitType::C},
    {"Weather Data", "Outdoor Dew Point", spawn::units::UnitType::K, spawn::units::UnitType::C},
    {"Weather Data", "Outdoor Relative Humidity", spawn::units::UnitType::one, spawn::units::UnitType::percent},
    {"Weather Data", "Diffuse Solar", spawn::units::UnitType::W_per_m2, spawn::units::UnitType::W_per_m2},
    {"Weather Data", "Direct Solar", spawn::units::UnitType::W_per_m2, spawn::units::UnitType::W_per_m2},
    {"Weather Data", "Wind Speed", spawn::units::UnitType::m_per_s, spawn::units::UnitType::m_per_s},
    {"Weather Data", "Wind Direction", spawn::units::UnitType::rad, spawn::units::UnitType::deg},
    {"Schedule:Compact", "Schedule Value", spawn::units::UnitType::one, spawn::units::UnitType::one},
    {"Schedule:Constant", "Schedule Value", spawn::units::UnitType::one, spawn::units::UnitType::one},
    {"ExteriorLights", "Electricity Rate", spawn::units::UnitType::W, spawn::units::UnitType::W},
    {"Material", "Surface Property Solar Absorptance", spawn::units::UnitType::one, spawn::units::UnitType::one},
    {"Material", "Surface Property Thermal Absorptance", spawn::units::UnitType::one, spawn::units::UnitType::one},
    {"Material", "Surface Property Visible Absorptance", spawn::units::UnitType::one, spawn::units::UnitType::one},
    {"People", "Number of People", spawn::units::UnitType::one, spawn::units::UnitType::one},
    {"Lights", "Electricity Rate", spawn::units::UnitType::W, spawn::units::UnitType::W},
    {"ElectricEquipment", "Electricity Rate", spawn::units::UnitType::W, spawn::units::UnitType::W},
    {"Surface",
     "Interior Surface Convection Heat Transfer Coefficient",
     spawn::units::UnitType::W_per_m2_K,
     spawn::units::UnitType::W_per_m2_K},
    {"Surface",
     "Exterior Surface Convection Heat Transfer Coefficient",
     spawn::units::UnitType::W_per_m2_K,
     spawn::units::UnitType::W_per_m2_K},
    {"Surface", "Construction State", spawn::units::UnitType::one, spawn::units::UnitType::one},
    {"Surface", "View Factor To Ground", spawn::units::UnitType::one, spawn::units::UnitType::one},
    {"Surface", "Surface Inside Temperature", spawn::units::UnitType::K, spawn::units::UnitType::C},
    {"Surface", "Surface Outside Temperature", spawn::units::UnitType::K, spawn::units::UnitType::C},
    {"Surface", "Outdoor Air Wind Direction", spawn::units::UnitType::rad, spawn::units::UnitType::deg},
    {"Zone", "Outdoor Air Drybulb Temperature", spawn::units::UnitType::K, spawn::units::UnitType::C},
    {"Zone", "Outdoor Air Wetbulb Temperature", spawn::units::UnitType::K, spawn::units::UnitType::C},
    {"Surface", "Outdoor Air Wind Speed", spawn::units::UnitType::m_per_s, spawn::units::UnitType::m_per_s},
    {"Zone Infiltration",
     "Air Exchange Flow Rate",
     spawn::units::UnitType::m3_per_s,
     spawn::units::UnitType::m3_per_s}};

#endif // ACTUATORTYPES_HH_INCLUDED
