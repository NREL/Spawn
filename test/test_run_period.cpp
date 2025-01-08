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

spawn_fs::path create_runperiod_test_fmu(const std::string_view start_day)
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
        "start_day_of_year": "{start_day}",
        "apply_weekend_holiday_rule": "Yes",
        "use_weather_file_daylight_saving_period": "Yes",
        "use_weather_file_holidays_and_special_days": "Yes",
        "use_weather_file_rain_indicators": "No",
        "use_weather_file_snow_indicators": "No"
      }},
      "model": {{
        "outputVariables": [
          {{
            "name":    "Lights Electricity Rate",
            "key":     "LIVING ZONE Lights",
            "fmiName": "lighting output"
          }}
        ]
      }}
    }}
  )",
      fmt::arg("idfpath", single_family_house_idf_path().generic_string()),
      fmt::arg("epwpath", chicago_epw_path().generic_string()),
      fmt::arg("start_day", start_day));

  return create_epfmu(spawn_input_string);
}

TEST_CASE("Test SingleFamilyHouse with Custom RunPeriod Starting on Sunday")
{
  constexpr auto start_day = "Sunday";
  const auto fmu_file_path = create_runperiod_test_fmu(start_day);
  spawn::fmu::FMU fmu{fmu_file_path, false}; // don't require all symbols
  REQUIRE(fmu.fmi.fmi2GetVersion() == std::string("2.0"));

  // Test that the EnergyPlus RunPeriod object is correct
  const auto resource_path = fmu.extractedFilesPath() / "resources";
  const auto idf_path = resource_path / "SingleFamilyHouse_TwoSpeed_ZoneAirBalance.spawn.idf";
  const auto idf_json = spawn::idf_to_json(idf_path);
  const auto runperiod_json = idf_json["RunPeriod"]["Spawn-RunPeriod"];

  CHECK(runperiod_json["day_of_week_for_start_day"] == start_day);
  CHECK(runperiod_json["apply_weekend_holiday_rule"] == "Yes");
  CHECK(runperiod_json["use_weather_file_daylight_saving_period"] == "Yes");
  CHECK(runperiod_json["use_weather_file_holidays_and_special_days"] == "Yes");
  CHECK(runperiod_json["use_weather_file_rain_indicators"] == "No");
  CHECK(runperiod_json["use_weather_file_snow_indicators"] == "No");

  const auto model_description_path = fmu.extractedFilesPath() / fmu.modelDescriptionPath();
  spawn::fmu::ModelDescription modelDescription(model_description_path);
  const auto lighting_output_ref = modelDescription.valueReference("lighting output");

  const std::array<fmi2ValueReference, 1> output_refs = {lighting_output_ref};
  std::array<fmi2Real, 1> output_values{};

  fmi2CallbackFunctions callbacks = {
      fmuNothingLogger, calloc, free, nullptr, nullptr}; // called by the model during simulation
  const auto comp = fmu.fmi.fmi2Instantiate(
      "test-instance", fmi2ModelExchange, "abc-guid", resource_path.string().c_str(), &callbacks, false, true);

  fmi2Status status = fmu.fmi.fmi2SetupExperiment(comp, false, 0.0, 0.0, false, 0.0);
  REQUIRE(status == fmi2OK);

  status = fmu.fmi.fmi2ExitInitializationMode(comp);
  REQUIRE(status == fmi2OK);

  // Go to the middle of the day on what should be Sunday
  status = fmu.fmi.fmi2SetTime(comp, spawn::days_to_seconds(0.5));
  CHECK(status == fmi2OK);

  status = fmu.fmi.fmi2GetReal(comp, output_refs.data(), output_refs.size(), output_values.data());
  CHECK(status == fmi2OK);

  // Lighting power should be minimal
  CHECK(output_values[0] == Approx(50));

  // Now go to the middle of the day on what should be Monday
  status = fmu.fmi.fmi2SetTime(comp, spawn::days_to_seconds(1.5));
  CHECK(status == fmi2OK);

  status = fmu.fmi.fmi2GetReal(comp, output_refs.data(), output_refs.size(), output_values.data());
  CHECK(status == fmi2OK);

  // Lighting power should be at peak
  CHECK(output_values[0] == Approx(1000));

  status = fmu.fmi.fmi2Terminate(comp);
  REQUIRE(status == fmi2OK);
}

TEST_CASE("Test SingleFamilyHouse with Custom RunPeriod Starting on Monday")
{
  constexpr std::string_view start_day = "Monday";
  const auto fmu_file_path = create_runperiod_test_fmu(start_day);
  spawn::fmu::FMU fmu{fmu_file_path, false}; // don't require all symbols

  const auto resource_path = fmu.extractedFilesPath() / "resources";
  const auto model_description_path = fmu.extractedFilesPath() / fmu.modelDescriptionPath();
  spawn::fmu::ModelDescription modelDescription(model_description_path);
  const auto lighting_output_ref = modelDescription.valueReference("lighting output");

  const std::array<fmi2ValueReference, 1> output_refs = {lighting_output_ref};
  std::array<fmi2Real, 1> output_values{};

  fmi2CallbackFunctions callbacks = {
      fmuNothingLogger, calloc, free, nullptr, nullptr}; // called by the model during simulation
  const auto comp = fmu.fmi.fmi2Instantiate(
      "test-instance", fmi2ModelExchange, "abc-guid", resource_path.string().c_str(), &callbacks, false, true);

  fmi2Status status = fmu.fmi.fmi2SetupExperiment(comp, false, 0.0, 0.0, false, 0.0);
  REQUIRE(status == fmi2OK);

  status = fmu.fmi.fmi2ExitInitializationMode(comp);
  REQUIRE(status == fmi2OK);

  // Go to the middle of the day on what should be Monday
  status = fmu.fmi.fmi2SetTime(comp, spawn::days_to_seconds(0.5));
  CHECK(status == fmi2OK);

  status = fmu.fmi.fmi2GetReal(comp, output_refs.data(), output_refs.size(), output_values.data());
  CHECK(status == fmi2OK);

  // Lighting power should be at peak
  CHECK(output_values[0] == Approx(1000));

  // Now go to the middle of the day on what should be Sunday
  status = fmu.fmi.fmi2SetTime(comp, spawn::days_to_seconds(6.5));
  CHECK(status == fmi2OK);

  status = fmu.fmi.fmi2GetReal(comp, output_refs.data(), output_refs.size(), output_values.data());
  CHECK(status == fmi2OK);

  // Lighting power should be at minimum
  CHECK(output_values[0] == Approx(50));

  status = fmu.fmi.fmi2Terminate(comp);
  REQUIRE(status == fmi2OK);
}
