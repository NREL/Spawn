#include "../EPFMI.hpp"
#include "../EPComponent.hpp"
#include "../EnergyPlus/DataEnvironment.hh"
#include <boost/filesystem.hpp>
#include <test-config.h>
#include <gtest/gtest.h>

void logger(fmi2ComponentEnvironment, fmi2String, fmi2Status, fmi2String, fmi2String, ...) {
}

TEST( Single_Model, static_lib ) {

  fmi2CallbackFunctions callbacks {
    logger,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
  };

  auto resourcePath = boost::filesystem::path(projectBinaryDir) / "epfmi/MyFMU/resources/";

  auto comp = fmi2Instantiate("Building_1", // instanceName
    fmi2ModelExchange, // fmuType
    "", // fmuGUID
    resourcePath.string().c_str(), // fmuResourceLocation
    &callbacks, // functions
    true, // visible
    true); // loggingOn

  const auto variables = static_cast<EPComponent*>(comp)->variables;

  double tStart = 0.0;
  double tEnd = 86400;

  //double outputs[] = {0.0, 0.0};
  //unsigned int outputRefs[] = {1, 2}; // Attic-QConSen_flow, CORE_LIGHTS
  //double inputs[] = {21.0, 0,0};
  //unsigned int inputRefs[] = {0, 0}; // Attic,T, CORE_LIGHTS_ACTUATOR
  //unsigned int paramRefs[] = {3}; // Attic,V
  //double params[1];

  //for (const auto & pair : variables) {
  //  const auto & var = pair.second;
  //  const auto & type = var.type;
  //  const auto & key = var.key;

  //  if (key == "Attic") {
  //    if (type == EnergyPlus::FMI::VariableType::QCONSEN_FLOW) {
  //      outputRefs[0] = pair.first;
  //    } else if (type == EnergyPlus::FMI::VariableType::T) {
  //      inputRefs[0] = pair.first;
  //    } else if (type == EnergyPlus::FMI::VariableType::V) {
  //      paramRefs[0] = pair.first;
  //    }
  //  }

  //  if (key == "CORE_LIGHTS") {
  //    outputRefs[1] = pair.first;
  //  } else if (key == "CORE_LIGHTS_ACTUATOR") {
  //    inputRefs[1] = pair.first;
  //  }
  //}

  fmi2SetupExperiment(comp, // component
    false, // toleranceDefined
    0, // tolerance
    tStart, // startTime
    true, // stopTimeDefined
    tEnd); // stopTime

  fmi2EventInfo eventInfo;

  //fmi2GetReal(comp, paramRefs, 1, params);
  //double atticVolume = params[0]; // m^3

  //double atticTemp = 21.0;
  //inputs[0] = atticTemp;
  //fmi2SetReal(comp, inputRefs, 6, inputs);

  double time = tStart;
  fmi2SetTime(comp, time);

  while ( time < tEnd ) {
    // Do something interesting with the small office attic.
    //inputs[0] = atticTemp;
    //fmi2SetReal(comp, inputRefs, 2, inputs);

    // Advance time of both simulations based on 
    // events from the small office. they should have the same timestep / time events anyway
    fmi2NewDiscreteStates(comp, &eventInfo);
    double lastTime = time;
    time = eventInfo.nextEventTime;
    fmi2SetTime(comp, time);
    double dt = time - lastTime;

    // update atticTemp
    //fmi2GetReal(comp, outputRefs, 2, outputs);

    //double atticQFlow = outputs[0]; // J/s 
    //double lightingPower = outputs[1];
    //double densityAir = 1.276; // kg/m^3
    //double heatCapacity = 1000.6; // J/kgK
    //double tempDot = atticQFlow / ( atticVolume * densityAir * heatCapacity );

    //atticTemp = atticTemp + (dt * tempDot);

    std::cout << "Current time: " << time << std::endl;
    //std::cout << "Attic Temp is: " << atticTemp << std::endl;
    //std::cout << "Core_Zone lighting power is: " << lightingPower << std::endl;
  }

  fmi2Terminate(comp);

  std::cout << "epfmi test is now complete" << std::endl;
}

