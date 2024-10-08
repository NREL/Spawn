#include "../fmu/fmu.hpp"
#include "../fmu/logger.h"
#include "../fmu/modeldescription.hpp"
#include "../util/filesystem.hpp"
#include "../util/math.hpp"
#include "create_epfmu.hpp"
#include "energyplus_coroutine/idf_to_json.hpp"
#include "paths.hpp"

#include <array>
#include <catch2/catch.hpp>
#include <iostream>
#include <nlohmann/json.hpp>

TEST_CASE("Test SingleFamilyHouse with Custom RunPeriod")
{
  std::string spawn_input_string = fmt::format(
      R"(
    {{
      "version": "0.1",
      "EnergyPlus": {{
        "idf": "{idfpath}",
        "weather": "{epwpath}"
      }},
      "fmu": {{
          "name": "MyBuilding.fmu",
          "version": "2.0",
          "kind"   : "ME"
      }},
      "RunPeriod": {{
        "day_of_week_for_start_day": "Monday",
        "apply_weekend_holiday_rule": "Yes",
        "use_weather_file_daylight_saving_period": "Yes",
        "use_weather_file_holidays_and_special_days": "Yes",
        "use_weather_file_rain_indicators": "No",
        "use_weather_file_snow_indicators": "No"
      }}
    }}
  )",
      fmt::arg("idfpath", single_family_house_idf_path().generic_string()),
      fmt::arg("epwpath", chicago_epw_path().generic_string()));

  const auto fmu_file_path = create_epfmu(spawn_input_string);
  spawn::fmu::FMU fmu{fmu_file_path, false, spawn_fs::current_path()}; // don't require all symbols
  REQUIRE(fmu.fmi.fmi2GetVersion() == std::string("2.0"));

  // Test that the EnergyPlus RunPeriod object is correct
  const auto resource_path = fmu.extractedFilesPath() / "resources";
  const auto idf_path = resource_path / "SingleFamilyHouse_TwoSpeed_ZoneAirBalance.spawn.idf";
  const auto idf_json = spawn::idf_to_json(idf_path);
  const auto runperiod_json = idf_json["RunPeriod"]["Spawn-RunPeriod"];

  CHECK(runperiod_json["day_of_week_for_start_day"] == "Monday");
  CHECK(runperiod_json["apply_weekend_holiday_rule"] == "Yes");
  CHECK(runperiod_json["use_weather_file_daylight_saving_period"] == "Yes");
  CHECK(runperiod_json["use_weather_file_holidays_and_special_days"] == "Yes");
  CHECK(runperiod_json["use_weather_file_rain_indicators"] == "No");
  CHECK(runperiod_json["use_weather_file_snow_indicators"] == "No");

  // Test that we can advance the simulation
  fmi2CallbackFunctions callbacks = {
      fmuNothingLogger, calloc, free, nullptr, nullptr}; // called by the model during simulation
  const auto comp = fmu.fmi.fmi2Instantiate(
      "test-instance", fmi2ModelExchange, "abc-guid", resource_path.string().c_str(), &callbacks, false, true);

  fmi2Status status = fmu.fmi.fmi2SetupExperiment(comp, false, 0.0, 0.0, false, 0.0);
  REQUIRE(status == fmi2OK);

  status = fmu.fmi.fmi2ExitInitializationMode(comp);
  REQUIRE(status == fmi2OK);

  status = fmu.fmi.fmi2SetTime(comp, spawn::days_to_seconds(1));
  CHECK(status == fmi2OK);

  status = fmu.fmi.fmi2Terminate(comp);
  REQUIRE(status == fmi2OK);
}
