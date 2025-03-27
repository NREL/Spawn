#ifndef SPAWN_UNITS_INCLUDED
#define SPAWN_UNITS_INCLUDED

#include <algorithm>
#include <array>
#include <iostream>
#include <map>
#include <utility>

namespace spawn::units {

enum class UnitSystem
{
  MO,
  EP
};

enum class UnitType
{
  C,
  K,
  one,
  kgWater_per_kgDryAir,
  percent,
  Pa,
  m_per_s,
  deg,
  rad,
  W,
  W_per_m2,
  W_per_m2_K,
  J,
  J_per_kg,
  m,
  m2,
  m3,
  L,
  m3_per_s,
  s,
  hr,
  kg,
  kg_per_m3,
  kg_per_s,
  cd_sr,
  lm,
  lm_per_m2,
  lux,
  lm_per_W
};

typedef std::pair<UnitType, const char *> UnitString;

const std::array<UnitString, 29> unitstrings{{{UnitType::C, "degC"},
                                              {UnitType::K, "K"},
                                              {UnitType::one, "1"},
                                              {UnitType::kgWater_per_kgDryAir, "kgWater/kgDryAir"},
                                              {UnitType::percent, "%"},
                                              {UnitType::Pa, "Pa"},
                                              {UnitType::m_per_s, "m/s"},
                                              {UnitType::deg, "deg"},
                                              {UnitType::rad, "rad"},
                                              {UnitType::W, "W"},
                                              {UnitType::W_per_m2, "W/m2"},
                                              {UnitType::W_per_m2_K, "W/m2.K"},
                                              {UnitType::J, "J"},
                                              {UnitType::J_per_kg, "J/kg"},
                                              {UnitType::m, "m"},
                                              {UnitType::m2, "m2"},
                                              {UnitType::m3, "m3"},
                                              {UnitType::L, "L"},
                                              {UnitType::m3_per_s, "m3/s"},
                                              {UnitType::s, "s"},
                                              {UnitType::hr, "hr"},
                                              {UnitType::kg, "kg"},
                                              {UnitType::kg_per_m3, "kg/m3"},
                                              {UnitType::kg_per_s, "kg/s"},
                                              {UnitType::cd_sr, "cd.sr"},
                                              {UnitType::lm, "lm"},
                                              {UnitType::lm_per_m2, "lm/m2"},
                                              {UnitType::lux, "lux"},
                                              {UnitType::lm_per_W, "lm/W"}}};

[[nodiscard]] std::string toString(const UnitType &unittype);

[[nodiscard]] UnitType fromString(const std::string &unitstring);

struct Conversion
{
  Conversion() = delete;
  double factor;
  double offset;
};

typedef std::pair<UnitType, UnitType> UnitPair;

const std::map<const UnitPair, const Conversion> conversions{
    {{UnitType::K, UnitType::C}, {1.0, -273.15}},
    {{UnitType::one, UnitType::percent}, {100.0, 0.0}},
    {{UnitType::one, UnitType::kgWater_per_kgDryAir}, {1.0, 0.0}},
    {{UnitType::rad, UnitType::deg}, {57.295779513, 0.0}},
    {{UnitType::m3, UnitType::L}, {1000.0, 0.0}},
    {{UnitType::s, UnitType::hr}, {3600, 0.0}},
    {{UnitType::cd_sr, UnitType::lm}, {1.0, 0.0}},
    {{UnitType::lm_per_m2, UnitType::lux}, {1.0, 0.0}}};

struct Quantity
{
  Quantity() = delete;
  double value;
  UnitType unit;
};

[[nodiscard]] Quantity convert(const Quantity &q, const UnitType &targetUnit);

} // namespace spawn::units

#endif // SPAWN_UNITS_INCLUDED
