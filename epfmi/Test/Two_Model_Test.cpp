#include "../EPFMI.hpp"
#include "../EPComponent.hpp"
#include "../EnergyPlus/DataEnvironment.hh"
#include <boost/filesystem.hpp>
#include <test-config.h>
#include <gtest/gtest.h>

TEST( Two_Model, static_lib ) {
  auto resourcePath = boost::filesystem::path(projectBinaryDir) / "src/FMI/MyFMU/resources/";

  auto comp = fmi2Instantiate("Building_1", // instanceName
    fmi2ModelExchange, // fmuType
    "", // fmuGUID
    resourcePath.string().c_str(), // fmuResourceLocation
    NULL, // functions
    true, // visible
    false); // loggingOn

  auto comp2 = fmi2Instantiate("Building_2", // instanceName
    fmi2ModelExchange, // fmuType
    "", // fmuGUID
    resourcePath.string().c_str(), // fmuResourceLocation
    NULL, // functions
    true, // visible
    false); // loggingOn

  double tStart = 0.0;
  double tEnd = 86400;

  double outputs[] = {0.0};
  const unsigned int outputRefs[] = {1}; // Attic,QConSen_flow
  double inputs[] = {21.0};
  const unsigned int inputRefs[] = {0}; // Attic,T...
  const unsigned int paramRefs[] = {3}; // Attic,V
  double params[1];

  double outputs2[] = {0.0};
  const unsigned int outputRefs2[] = {1}; // Attic,QConSen_flow
  double inputs2[] = {21.0};
  const unsigned int inputRefs2[] = {0}; // Attic,T...
  const unsigned int paramRefs2[] = {3}; // Attic,V
  double params2[1];

  fmi2SetupExperiment(comp, // component
    false, // toleranceDefined
    0, // tolerance
    tStart, // startTime
    true, // stopTimeDefined
    tEnd); // stopTime

  fmi2SetupExperiment(comp2, // component
    false, // toleranceDefined
    0, // tolerance
    tStart, // startTime
    true, // stopTimeDefined
    tEnd); // stopTime

  fmi2EventInfo eventInfo;

  fmi2GetReal(comp, paramRefs, 1, params);
  double atticVolume = params[0]; // m^3

  double atticTemp = 21.0;
  inputs[0] = atticTemp;
  fmi2SetReal(comp, inputRefs, 6, inputs);

  double time = tStart;
  fmi2SetTime(comp, time);

  while ( time < tEnd ) {
    // Do something interesting with the small office attic.
    inputs[0] = atticTemp;
    fmi2SetReal(comp, inputRefs, 6, inputs);

    // Advance time of both simulations based on 
    // events from the small office. they should have the same timestep / time events anyway
    fmi2NewDiscreteStates(comp, &eventInfo);
    double lastTime = time;
    time = eventInfo.nextEventTime;
    fmi2SetTime(comp, time);
    fmi2SetTime(comp2, time);
    double dt = time - lastTime;

    // update atticTemp
    fmi2GetReal(comp, outputRefs, 1, outputs);

    double atticQFlow = outputs[0]; // J/s 
    std::cout << "atticQFlow: " << atticQFlow << std::endl;
    double densityAir = 1.276; // kg/m^3
    double heatCapacity = 1000.6; // J/kgK
    double tempDot = atticQFlow / ( atticVolume * densityAir * heatCapacity );

    atticTemp = atticTemp + (dt * tempDot);

    std::cout << "Current time: " << time << std::endl;
    std::cout << "Attic Temp is: " << atticTemp << std::endl;
  }

  fmi2Terminate(comp);
  fmi2Terminate(comp2);

  std::cout << "epfmi test is now complete" << std::endl;
}

