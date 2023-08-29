#ifndef ENERGYPLUS_CONFIG_HPP
#define ENERGYPLUS_CONFIG_HPP

#include "../submodules/EnergyPlus/src/EnergyPlus/DataStringGlobals.hh"

namespace spawn::energyplus {

inline std::string_view version_string()
{
  return EnergyPlus::DataStringGlobals::VerString;
}

} // namespace spawn::energyplus

#endif // ENERGYPLUS_CONFIG_HPP
