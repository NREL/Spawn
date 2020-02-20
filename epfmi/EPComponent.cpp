#include "EPComponent.hpp"
#include "EnergyPlus/DataGlobals.hh"

fmi2Real EPComponent::currentSimTime() const {
  return EnergyPlus::DataGlobals::SimTimeSteps * EnergyPlus::DataGlobals::TimeStepZoneSec;
}

fmi2Real EPComponent::nextSimTime() const {
  return (EnergyPlus::DataGlobals::SimTimeSteps + 1) * EnergyPlus::DataGlobals::TimeStepZoneSec;
}

