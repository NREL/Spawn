#include "../fmu/fmu.hpp"
#include "../fmu/logger.h"
#include "../fmu/modeldescription.hpp"
#include "create_epfmu.hpp"
#include <catch2/catch.hpp>

constexpr std::array<const char *, 27> ignorelist = {
    "1ZoneParameterAspect.idf", // Preproc macros - Value type "string" for input "=$width" not permitted by 'type'
                                // constraint
    "1ZoneUncontrolled_DDChanges.idf", // GetWeatherProperties:WeatherProperty:SkyTemperature="DENVER CENTENNIAL GOLDEN
                                       // ANN CLG 1% SKY TEMPERATURE MODFIER", invalid Environment Name referenced
    "1ZoneUncontrolled_win_1.idf", // HeatBalanceManager::SearchWindow5DataFile: "..\datasets\Window5DataFile.dat" not
                                   // found
    "1ZoneUncontrolled_win_2.idf", // HeatBalanceManager::SearchWindow5DataFile: "..\datasets\Window5DataFile.dat" not
                                   // found
    "5ZoneAirCooledWithSlab.idf",  // BuildingSurface:Detailed="F1-1", invalid Outside Boundary
                                   // Condition="GROUNDSLABPREPROCESSORPERIMETER
    "5ZoneTDV.idf", // Schedule:File="ELECTDVFROMCZ06COM", File Name: "..\datasets\TDV\TDV_2008_kBtu_CTZ06.csv" not
                    // found
    "ASHRAE9012016_Hospital_Denver.idf", // Runaway temps - CalcHeatBalanceInsideSurf: The temperature of 1906404.17 C
                                         // for zone="KITCHEN_FLR_5", for surface="KITCHEN_FLR_5_WALL_1_WEST"
    "DOAS_wNeutralSupplyAir_wFanCoilUnits.idf", // Value type "string" for input "Site:Location" not permitted by 'type'
                                                // constraint
    "HospitalBaseline.idf", // Runaway temps - CalcHeatBalanceInsideSurf: The temperature of 131325.19 C for zone="FLOOR
                            // 2 IMAGING 1", for surface="2C57D2"
    "HospitalBaselineReheatReportEMS.idf", // Runaway temps - CalcHeatBalanceInsideSurf: The temperature of 131325.19 C
                                           // for zone="FLOOR 2 IMAGING 1", for surface="2C57D2"
    "HospitalLowEnergy.idf", // Runaway temps - CalcHeatBalanceInsideSurf: The temperature of 2396112.36 C for
                             // zone="FLOOR 2 STERILIZING", for surface="93C14E"
    "HybridModel_4Zone_Solve_Infiltration_free_floating.idf", // Schedule:File="ZONE 2_MEASUREDHUMIDITYRATIO", File
                                                              // Name: "HybridModel_Measurements_no_HVAC.csv" not found
    "HybridModel_4Zone_Solve_PeopleCount_with_HVAC.idf",      // Schedule:File="ZONE 1_INLET_TEMPERATURE", File Name:
                                                              // "HybridModel_Measurements_with_HVAC.csv" not found.
    "HybridZoneModel.idf",                                    // Schedule:File="ZONE 2_MEASUREDTEMPERATURE", File Name:
                                                              // "HybridZoneModel_TemperatureData.csv" not found.
    "LBuildingAppGRotPar.idf",   // Value type "string" for input "=$appGAngle" not permitted by 'type' constraint
    "LgOffVAVusingBasement.idf", // BuildingSurface:Detailed="UF", invalid Outside Boundary
                                 // Condition="GROUNDBASEMENTPREPROCESSORAVERAGEFLOOR".
    "ParametricInsulation-5ZoneAirCooled.idf", // Value type "string" for input "=$insDepth" not permitted by 'type'
                                               // constraint.
    "ShopWithPVandBattery.idf", // ComponentCost:LineItem: "PV:ZN_1_FLR_1_SEC_1_CEILING", Generator:Photovoltaic, need
                                // to specify a valid PV array
    "ShopWithPVandLiIonBattery.idf", // ComponentCost:LineItem: "PV:ZN_1_FLR_1_SEC_1_CEILING", Generator:Photovoltaic,
                                     // need to specify a valid PV array
    "ShopWithPVandStorage.idf", // ComponentCost:LineItem: "PV:ZN_1_FLR_1_SEC_1_CEILING", Generator:Photovoltaic, need
                                // to specify a valid PV array
    "SingleFamilyHouse_HP_Slab.idf", // BuildingSurface:Detailed="FLOOR_UNIT1", invalid Outside Boundary
                                     // Condition="GROUNDSLABPREPROCESSORAVERAGE"
    "SingleFamilyHouse_HP_Slab_Dehumidification.idf", // BuildingSurface:Detailed="FLOOR_UNIT1", invalid Outside
                                                      // Boundary Condition="GROUNDSLABPREPROCESSORAVERAGE"
    "SolarCollectorFlatPlateWater.idf", // GetHeatBalanceInput: There are surfaces in input but no zones found.  Invalid
                                        // simulation.
    "SolarShadingTest_ExternalFraction.idf", // Schedule:File="EXTSHADINGSCH:EAST SIDE TREE", File Name:
                                             // "SolarShadingTest_Shading_Data.csv" not found
    "SolarShadingTest_ImportedShading.idf", // Schedule:File:Shading, File Name: "SolarShadingTest_Shading_Data.csv" not
                                            // found
    "SurfacePropTest_SurfLWR.idf", // Schedule:File="SURROUNDING TEMP SCH 1", File Name: "LocalEnvData.csv" not found
    "SurfaceZonePropTest_LocalEnvx.idf" // Schedule:File="OUTDOORAIRNODEDRYBULB:0001", File Name: "LocalEnvData.csv" not
                                        // found
};

TEST_CASE("Test example file", "[!hide]")
{
  const auto testfileDirectory = spawn::project_source_dir() / "submodules/EnergyPlus/testfiles";
  const auto simulationDirectory = spawn::project_binary_dir() / "testfile-simulations";

  SECTION("clean simulation working directory")
  {
    spawn_fs::remove_all(simulationDirectory);
  }

  for (const auto &entry : spawn_fs::directory_iterator(testfileDirectory)) {
    const auto &p = entry.path();

    if (p.extension() != ".idf") {
      continue;
    }

    if (p.filename().generic_string()[0] == '_') {
      continue;
    }

    const auto ignoreit = std::find(ignorelist.begin(), ignorelist.end(), p.filename().generic_string());
    if (ignoreit != ignorelist.end()) {
      continue;
    }

    SECTION(fmt::format(p.filename().generic_string()))
    {
      std::string spawn_input_string = fmt::format(
          R"(
        {{
          "version": "0.1",
          "EnergyPlus": {{
            "idf": "{idfpath}",
            "weather": "{epwpath}",
            "relativeSurfaceTolerance": 1.0e-10
          }},
          "fmu": {{
              "name": "MyBuilding.fmu",
              "version": "2.0",
              "kind"   : "ME"
          }},
          "model": {{
          }}
        }}
      )",
          fmt::arg("idfpath", p.generic_string()),
          fmt::arg("epwpath", chicago_epw_path().generic_string()));

      const auto testDirectory = simulationDirectory / p.stem().generic_string();
      const auto fmu_file_path = create_epfmu(spawn_input_string);
      spawn::fmu::FMU fmu{fmu_file_path, false, testDirectory}; // don't require all symbols
      REQUIRE(fmu.fmi.fmi2GetVersion() == std::string("2.0"));

      const auto resource_path = (fmu.extractedFilesPath() / "resources").string();
      fmi2CallbackFunctions callbacks = {
          fmuStdOutLogger, calloc, free, nullptr, nullptr}; // called by the model during simulation
      const auto comp = fmu.fmi.fmi2Instantiate(
          "test-instance", fmi2ModelExchange, "abc-guid", resource_path.c_str(), &callbacks, false, true);

      fmi2Status status{};

      status = fmu.fmi.fmi2SetupExperiment(comp, false, 0.0, 0.0, false, 0.0);
      REQUIRE(status == fmi2OK);

      const auto model_description_path = fmu.extractedFilesPath() / fmu.modelDescriptionPath();
      spawn::fmu::ModelDescription modelDescription(model_description_path);

      status = fmu.fmi.fmi2ExitInitializationMode(comp);
      REQUIRE(status == fmi2OK);

      status = fmu.fmi.fmi2Terminate(comp);
      REQUIRE(status == fmi2OK);

      // If this point is reached then the test passed and it is ok to remove the simulation fiiles.
      // They probably wont be needed for inspection
      spawn_fs::remove_all(testDirectory);
    }
  }
}
