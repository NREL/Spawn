#include "../coroutine/idf_to_json.hpp"
#include "../coroutine/spawn.hpp"
#include "../util/config.hpp"
#include "../util/math.hpp"
#include "paths.hpp"
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <array>
#include <catch2/catch.hpp>
#include <filesystem>

using json = nlohmann::json;

const auto idfpath = // NOLINT
    spawn::project_source_dir() / "energyplus/testfiles/RefBldgSmallOfficeNew2004_Chicago.idf";
const auto epwpath = // NOLINT
    spawn::project_source_dir() / "energyplus/weather/USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw";

TEST_CASE("Test Zone Sizing Variables")
{
  const std::string spawn_input = fmt::format(
      R"({{
      "version": "0.1",
      "EnergyPlus": {{
        "idf": "{idfpath}",
        "weather": "{epwpath}"
      }},
      "model": {{
        "zones": [
           {{ "name": "Core_ZN" }}
        ],
        "hvacZones": [{{
          "name": "sys1",
          "zones": [
           {{ "name": "Core_ZN" }}
          ]
        }}],
        "hvacSystems": [{{
          "name": "sys1",
          "autosize": "true"
        }}]
      }}
    }})",
      fmt::arg("idfpath", idfpath.generic_string()),
      fmt::arg("epwpath", epwpath.generic_string()));

  spawn::Spawn spawn1("spawn1", spawn::test::idd_path(), spawn_input, spawn::test::get_current_test_dir());

  spawn1.Start();
  CHECK(spawn1.CurrentTime() == 0.0);

  double value = spawn1.GetValue("Core_ZN_QCooSen_flow");
  CHECK(value > 0.0);
  value = spawn1.GetValue("Core_ZN_QCooLat_flow");
  // The Sizing:Zone input for "Zone Load Sizing Method" defaults to "Sensible Load Only No Latent Load",
  // and most common IDFs use this option, therefore we will not have latent loads reported.
  CHECK(value == Approx(0.0));
  value = spawn1.GetValue("Core_ZN_TOutCoo");
  CHECK(value > 0.0);
  value = spawn1.GetValue("Core_ZN_XOutCoo");
  CHECK(value > 0.0);
  value = spawn1.GetValue("Core_ZN_mOutCoo_flow");
  CHECK(value > 0.0);
  value = spawn1.GetValue("Core_ZN_tCoo");
  CHECK(value > 0.0);
  value = spawn1.GetValue("Core_ZN_QHea_flow");
  CHECK(value > 0.0);
  value = spawn1.GetValue("Core_ZN_TOutHea");
  CHECK(value > 0.0);
  value = spawn1.GetValue("Core_ZN_XOutHea");
  CHECK(value > 0.0);
  value = spawn1.GetValue("Core_ZN_mOutHea_flow");
  CHECK(value > 0.0);
  value = spawn1.GetValue("Core_ZN_tHea");
  CHECK(value > 0.0);
  const auto tset_hea = spawn1.GetValue("Core_ZN_TSetHea");
  const auto tset_coo = spawn1.GetValue("Core_ZN_TSetCoo");
  const auto xset_hea = spawn1.GetValue("Core_ZN_XSetHea");
  const auto xset_coo = spawn1.GetValue("Core_ZN_XSetCoo");
  CHECK(tset_hea > 250.0);
  CHECK(tset_hea < 320.0);
  CHECK(tset_coo > 250.0);
  CHECK(tset_coo < 320.0);
  CHECK(tset_coo > tset_hea);
  CHECK(xset_hea > 0.0);
  CHECK(xset_hea < 0.03);
  CHECK(xset_coo > 0.0);
  CHECK(xset_coo < 0.03);
  CHECK(xset_coo > xset_hea);

  spawn1.Stop();
}

TEST_CASE("Test Sizing Latent Cooling Load at Sensible Peak")
{
  const auto test_dir = spawn::test::get_current_test_dir();
  const auto latent_sizing_idfpath =
      spawn::project_source_dir() / "energyplus/testfiles/5ZoneAirCooled_LatentSizing_OtherEquipmentLatentLoads.idf";
  const auto scaled_latent_sizing_idfpath = test_dir / "5ZoneAirCooled_LatentSizing_scaled.idf";

  auto idf_json = spawn::idf_to_json(latent_sizing_idfpath);
  for (auto &[_, sizing_zone] : idf_json["Sizing:Zone"].items()) {
    if (sizing_zone.value("zone_or_zonelist_name", "") == "SPACE2-1") {
      sizing_zone["zone_cooling_sizing_factor"] = 2.0;
    }
  }
  spawn::json_to_idf(idf_json, scaled_latent_sizing_idfpath);

  const std::string spawn_input = fmt::format(
      R"({{
      "version": "0.1",
      "EnergyPlus": {{
        "idf": "{idfpath}",
        "weather": "{epwpath}"
      }},
      "model": {{
        "zones": [
           {{ "name": "SPACE2-1" }}
        ],
        "hvacZones": [{{
          "name": "sys1",
          "zones": [
           {{ "name": "SPACE1-1" }},
           {{ "name": "SPACE2-1" }}
          ]
        }}],
        "hvacSystems": [{{
          "name": "sys1",
          "autosize": "true"
        }}]
      }}
    }})",
      fmt::arg("idfpath", scaled_latent_sizing_idfpath.generic_string()),
      fmt::arg("epwpath", epwpath.generic_string()));

  spawn::Spawn spawn1("spawn1", spawn::test::idd_path(), spawn_input, test_dir);

  spawn1.Start();
  CHECK(spawn1.CurrentTime() == 0.0);

  const auto sensible_cooling_load = spawn1.GetValue("SPACE2-1_QCooSen_flow");
  const auto latent_cooling_load = spawn1.GetValue("SPACE2-1_QCooLat_flow");
  const auto group_sensible_cooling_load = spawn1.GetValue("hvac_sizing_group_sys1_QCooSen_flow");
  const auto group_latent_cooling_load = spawn1.GetValue("hvac_sizing_group_sys1_QCooLat_flow");

  CHECK(sensible_cooling_load > 0.0);
  CHECK(latent_cooling_load > 0.0);
  CHECK(latent_cooling_load < sensible_cooling_load);

  // The group contains SPACE1-1 and SPACE2-1. Its latent load is the sum of both zones' latent loads at the peak of
  // their combined sensible load, not the independently calculated peak of their combined latent load.
  CHECK(group_sensible_cooling_load == Approx(6311.0).margin(5.0));
  CHECK(group_latent_cooling_load == Approx(6920.0).margin(5.0));

  // The unscaled SPACE2-1 sensible peak is near 2200 W, and its coincident latent load is near 2100 W. Setting the zone
  // cooling sizing factor to 2.0 should put both Spawn outputs above 4000 W. The zone's independent latent peak occurs
  // later and is greater than its sensible peak, so the ordering above distinguishes the two definitions.
  CHECK(sensible_cooling_load > 4000.0);
  CHECK(latent_cooling_load > 4000.0);

  spawn1.Stop();
}

TEST_CASE("Test Injected Thermostat Setpoints")
{
  const auto test_dir = spawn::test::get_current_test_dir();
  const auto no_thermostat_idf = test_dir / "RefBldgSmallOfficeNew2004_Chicago_no_thermostat.idf";

  // Remove thermostat objects so idfprep must inject defaults for autosized zones.
  auto idf_json = spawn::idf_to_json(idfpath);
  idf_json.erase("ZoneControl:Thermostat");
  idf_json.erase("ZoneControl:Thermostat:StagedDualSetpoint");
  idf_json.erase("ThermostatSetpoint:DualSetpoint");
  idf_json.erase("ThermostatSetpoint:SingleHeating");
  idf_json.erase("ThermostatSetpoint:SingleCooling");
  spawn::json_to_idf(idf_json, no_thermostat_idf);

  const std::string spawn_input = fmt::format(
      R"({{
      "version": "0.1",
      "EnergyPlus": {{
        "idf": "{idfpath}",
        "weather": "{epwpath}"
      }},
      "model": {{
        "zones": [
           {{ "name": "Core_ZN" }}
        ],
        "hvacZones": [{{
          "name": "sys1",
          "zones": [
           {{ "name": "Core_ZN" }}
          ]
        }}],
        "hvacSystems": [{{
          "name": "sys1",
          "autosize": "true"
        }}]
      }}
    }})",
      fmt::arg("idfpath", no_thermostat_idf.generic_string()),
      fmt::arg("epwpath", epwpath.generic_string()));

  spawn::Spawn spawn1("spawn1", spawn::test::idd_path(), spawn_input, test_dir);

  spawn1.Start();
  CHECK(spawn1.CurrentTime() == 0.0);

  const auto tset_hea = spawn1.GetValue("Core_ZN_TSetHea");
  const auto tset_coo = spawn1.GetValue("Core_ZN_TSetCoo");
  const auto xset_hea = spawn1.GetValue("Core_ZN_XSetHea");
  const auto xset_coo = spawn1.GetValue("Core_ZN_XSetCoo");

  CHECK(tset_hea == Approx(spawn::c_to_k(20.0)));
  CHECK(tset_coo == Approx(spawn::c_to_k(22.0)));
  CHECK(xset_hea > 0.0);
  CHECK(xset_hea < 0.03);
  CHECK(xset_coo > 0.0);
  CHECK(xset_coo < 0.03);
  CHECK(xset_coo > xset_hea);

  spawn1.Stop();
}

TEST_CASE("Test Zone Group Sizing Variables")
{
  const std::string spawn_input = fmt::format(
      R"({{
      "version": "0.1",
      "EnergyPlus": {{
        "idf": "{idfpath}",
        "weather": "{epwpath}"
      }},
      "model": {{
        "zones": [
           {{ "name": "Core_ZN" }},
           {{ "name": "Perimeter_ZN_1" }},
           {{ "name": "Perimeter_ZN_2" }},
           {{ "name": "Perimeter_ZN_3" }},
           {{ "name": "Perimeter_ZN_4" }}
        ],
        "hvacZones": [{{
          "name": "conditioned_zones",
          "zones": [
           {{ "name": "Core_ZN" }},
           {{ "name": "Perimeter_ZN_1" }},
           {{ "name": "Perimeter_ZN_2" }},
           {{ "name": "Perimeter_ZN_3" }},
           {{ "name": "Perimeter_ZN_4" }}
          ]
        }},{{
          "name": "default",
          "zones": [
           {{ "name": "Attic" }}
          ]
        }}],
        "hvacSystems": [{{
          "name": "conditioned_zones",
          "autosize": "true"
        }},{{
          "name": "default",
          "autosize": "false"
       }}]
      }}
    }})",
      fmt::arg("idfpath", idfpath.generic_string()),
      fmt::arg("epwpath", epwpath.generic_string()));

  spawn::Spawn spawn1("spawn1", spawn::test::idd_path(), spawn_input, spawn::test::get_current_test_dir());

  spawn1.Start();
  CHECK(spawn1.CurrentTime() == 0.0);

  double value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_QCooSen_flow");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_QCooLat_flow");
  // The Sizing:Zone input for "Zone Load Sizing Method" defaults to "Sensible Load Only No Latent Load",
  // and most common IDFs use this option, therefore we will not have latent loads reported.
  CHECK(value == Approx(0.0));
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_TOutCoo");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_XOutCoo");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_mOutCoo_flow");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_tCoo");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_QHea_flow");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_TOutHea");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_XOutHea");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_mOutHea_flow");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_tHea");
  CHECK(value > 0.0);

  // The default group contains only the Attic, which does not have sizing information.
  // We should expect default sizing values.
  value = spawn1.GetValue("hvac_sizing_group_default_QCooSen_flow");
  CHECK(value == Approx(0.0));
  value = spawn1.GetValue("hvac_sizing_group_default_QCooLat_flow");
  // The Sizing:Zone input for "Zone Load Sizing Method" defaults to "Sensible Load Only No Latent Load",
  // and most common IDFs use this option, therefore we will not have latent loads reported.
  CHECK(value == Approx(0.0));
  value = spawn1.GetValue("hvac_sizing_group_default_TOutCoo");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_default_XOutCoo");
  CHECK(value == Approx(0.0));
  value = spawn1.GetValue("hvac_sizing_group_default_mOutCoo_flow");
  CHECK(value == Approx(0.0));
  value = spawn1.GetValue("hvac_sizing_group_default_tCoo");
  CHECK(value == Approx(0.0));
  value = spawn1.GetValue("hvac_sizing_group_default_QHea_flow");
  CHECK(value == Approx(0.0));
  value = spawn1.GetValue("hvac_sizing_group_default_TOutHea");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_default_XOutHea");
  CHECK(value == Approx(0.0));
  value = spawn1.GetValue("hvac_sizing_group_default_mOutHea_flow");
  CHECK(value == Approx(0.0));
  value = spawn1.GetValue("hvac_sizing_group_default_tHea");
  CHECK(value == Approx(0.0));

  spawn1.Stop();
}

TEST_CASE("Test Humidistat Setpoint Variables")
{
  const std::string spawn_input = fmt::format(
      R"({{
      "version": "0.1",
      "EnergyPlus": {{
        "idf": "{idfpath}",
        "weather": "{epwpath}"
      }},
      "model": {{
        "zones": [
           {{ "name": "Core_ZN" }}
        ],
        "hvacZones": [{{
          "name": "sys1",
          "zones": [
           {{ "name": "Core_ZN" }}
          ]
        }}],
        "hvacSystems": [{{
          "name": "sys1",
          "autosize": "true"
        }}]
      }}
    }})",
      fmt::arg("idfpath", idfpath.generic_string()),
      fmt::arg("epwpath", epwpath.generic_string()));

  spawn::Spawn spawn1("spawn1", spawn::test::idd_path(), spawn_input, spawn::test::get_current_test_dir());

  spawn1.Start();
  CHECK(spawn1.CurrentTime() == 0.0);

  const auto xset_hea = spawn1.GetValue("Core_ZN_XSetHea");
  const auto xset_coo = spawn1.GetValue("Core_ZN_XSetCoo");

  CHECK(xset_hea > 0.0);
  CHECK(xset_hea < 0.03);
  CHECK(xset_coo > 0.0);
  CHECK(xset_coo < 0.03);
  CHECK(xset_coo > xset_hea);

  spawn1.Stop();
}

TEST_CASE("Test Multiple Zone Group Sizing Variables")
{
  const std::string spawn_input = fmt::format(
      R"({{
      "version": "0.1",
      "EnergyPlus": {{
        "idf": "{idfpath}",
        "weather": "{epwpath}"
      }},
      "model": {{
        "zones": [
           {{ "name": "Core_ZN" }},
           {{ "name": "Perimeter_ZN_1" }},
           {{ "name": "Perimeter_ZN_2" }},
           {{ "name": "Perimeter_ZN_3" }},
           {{ "name": "Perimeter_ZN_4" }}
        ],
        "hvacZones": [
          {{
            "name": "conditioned_zones_1",
            "zones": [
             {{ "name": "Core_ZN" }},
             {{ "name": "Perimeter_ZN_1" }},
             {{ "name": "Perimeter_ZN_2" }}
            ]
          }},
          {{
            "name": "conditioned_zones_2",
            "zones": [
             {{ "name": "Perimeter_ZN_3" }},
             {{ "name": "Perimeter_ZN_4" }}
            ]
          }}
        ],
        "hvacSystems": [
          {{
            "name": "conditioned_zones_1",
            "autosize": "true"
          }},
          {{
            "name": "conditioned_zones_2",
            "autosize": "true"
          }}
        ]
      }}
    }})",
      fmt::arg("idfpath", idfpath.generic_string()),
      fmt::arg("epwpath", epwpath.generic_string()));

  spawn::Spawn spawn1("spawn1", spawn::test::idd_path(), spawn_input, spawn::test::get_current_test_dir());

  spawn1.Start();
  CHECK(spawn1.CurrentTime() == 0.0);

  double value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_1_QCooSen_flow");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_1_QCooLat_flow");
  // The Sizing:Zone input for "Zone Load Sizing Method" defaults to "Sensible Load Only No Latent Load",
  // and most common IDFs use this option, therefore we will not have latent loads reported.
  CHECK(value == Approx(0.0));
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_1_TOutCoo");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_1_XOutCoo");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_1_mOutCoo_flow");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_1_tCoo");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_1_QHea_flow");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_1_TOutHea");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_1_XOutHea");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_1_mOutHea_flow");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_1_tHea");
  CHECK(value > 0.0);

  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_2_QCooSen_flow");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_2_QCooLat_flow");
  // The Sizing:Zone input for "Zone Load Sizing Method" defaults to "Sensible Load Only No Latent Load",
  // and most common IDFs use this option, therefore we will not have latent loads reported.
  CHECK(value == Approx(0.0));
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_2_TOutCoo");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_2_XOutCoo");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_2_mOutCoo_flow");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_2_tCoo");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_2_QHea_flow");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_2_TOutHea");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_2_XOutHea");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_2_mOutHea_flow");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_2_tHea");
  CHECK(value > 0.0);

  spawn1.Stop();
}

TEST_CASE("Sizing setpoints are fixed at design conditions", "[spawn][sizing]")
{
  const auto test_dir = spawn::test::get_current_test_dir();
  const auto sizing_idf = test_dir / "design_setpoints.idf";
  auto idf_json = spawn::idf_to_json(idfpath);
  const auto schedule = [](const std::string &day_type, double design_value, double operating_value) {
    return json{{"data",
                 {{{"field", "Through: 12/31"}},
                  {{"field", "For: " + day_type}},
                  {{"field", "Until: 24:00"}},
                  {{"field", design_value}},
                  {{"field", "For: AllOtherDays"}},
                  {{"field", "Until: 24:00"}},
                  {{"field", operating_value}}}}};
  };
  // The annual schedules intentionally disagree with the design days, as in Dave's IDF.
  idf_json["Schedule:Compact"]["CLGSETP_SCH"] = schedule("SummerDesignDay", 24.0, 80.0);
  idf_json["Schedule:Compact"]["HTGSETP_SCH"] = schedule("WinterDesignDay", 21.0, 0.0);
  idf_json["Schedule:Compact"]["Design Humidify"] = schedule("WinterDesignDay", 40.0, 10.0);
  idf_json["Schedule:Compact"]["Design Dehumidify"] = schedule("SummerDesignDay", 60.0, 90.0);
  idf_json["ZoneControl:Humidistat"]["South Humidistat"] = {
      {"zone_name", "Perimeter_ZN_1"},
      {"humidifying_relative_humidity_setpoint_schedule_name", "Design Humidify"},
      {"dehumidifying_relative_humidity_setpoint_schedule_name", "Design Dehumidify"}};
  for (auto &[_, sizing] : idf_json["Sizing:Zone"].items()) {
    sizing["zone_load_sizing_method"] = "Sensible And Latent Load";
  }
  spawn::json_to_idf(idf_json, sizing_idf);

  const json zones = {{{"name", "Perimeter_ZN_1"}}};
  const json config = {{"version", "0.1"},
                       {"EnergyPlus", {{"idf", sizing_idf.generic_string()}, {"weather", epwpath.generic_string()}}},
                       {"model",
                        {{"zones", zones},
                         {"hvacZones", {{{"name", "sys1"}, {"zones", zones}}}},
                         {"hvacSystems", {{{"name", "sys1"}, {"autosize", "true"}}}}}}};

  // Verify FMI metadata as well as values: the importer must initialize these as parameters.
  const spawn::UserConfig user_config(config.dump());
  const spawn::variable::Variables variables(user_config);
  const std::array<std::string, 4> names = {
      "Perimeter_ZN_1_TSetCoo", "Perimeter_ZN_1_TSetHea", "Perimeter_ZN_1_XSetCoo", "Perimeter_ZN_1_XSetHea"};
  for (const auto &name : names) {
    const auto &variable = variables.all_variables().at(variables.VariableIndex(name));
    const auto metadata = variable->metadata().document_element();
    CHECK(std::string(metadata.attribute("causality").value()) == "calculatedParameter");
    CHECK(std::string(metadata.attribute("variability").value()) == "fixed");
    CHECK(std::string(metadata.attribute("initial").value()) == "calculated");
  }

  std::array<double, 4> baseline{};
  double baseline_load = 0.0;
  for (const auto day : {180, 181, 195}) {
    CAPTURE(day);
    const auto work_dir = test_dir / std::to_string(day);
    spawn_fs::create_directories(work_dir);
    spawn::Spawn sim("sizing_setpoints", spawn::test::idd_path(), config.dump(), work_dir);
    const double start_time = day * 86400.0;
    sim.SetStartTime(start_time);
    sim.Start();
    // Ensure a failed REQUIRE still stops the simulation thread.
    struct StopSimulation
    {
      spawn::Spawn &sim;
      ~StopSimulation()
      {
        sim.Stop();
      }
    } stop{sim};
    std::array<double, 4> actual{};
    for (size_t i = 0; i < names.size(); ++i) {
      actual[i] = sim.GetValue(names[i]);
    }
    CHECK(actual[0] == Approx(spawn::c_to_k(24.0)));
    CHECK(actual[1] == Approx(spawn::c_to_k(21.0)));
    // These are mass fractions at design temperature, RH, and pressure, not the annual RH schedule.
    EnergyPlus::EnergyPlusData psychrometric_state;
    const auto humidity_fraction = [&](double temperature, double rh, const std::string &day_type) {
      double pressure = 0.0;
      for (const auto &[_, design_day] : idf_json["SizingPeriod:DesignDay"].items()) {
        if (design_day.value("day_type", "") == day_type) {
          pressure = design_day.at("barometric_pressure").get<double>();
          break;
        }
      }
      const auto w = EnergyPlus::Psychrometrics::PsyWFnTdbRhPb(psychrometric_state, temperature, rh, pressure);
      return w / (1.0 + w);
    };
    CHECK(actual[2] == Approx(humidity_fraction(24.0, 0.60, "SummerDesignDay")).margin(0.00001));
    CHECK(actual[3] == Approx(humidity_fraction(21.0, 0.40, "WinterDesignDay")).margin(0.00001));
    const auto load = sim.GetValue("Perimeter_ZN_1_QCooSen_flow");
    CHECK(load > 0.0);
    if (day == 180) {
      baseline = actual;
      baseline_load = load;
    } else {
      for (size_t i = 0; i < names.size(); ++i) {
        CHECK(actual[i] == Approx(baseline[i]));
      }
      CHECK(load == Approx(baseline_load));
    }
    // The same parameters survive exchanges and advancement of the annual simulation.
    sim.SetValue("Perimeter_ZN_1_T", spawn::c_to_k(30.0));
    sim.SetTime(start_time + 3600.0);
    sim.Exchange(true);
    for (size_t i = 0; i < names.size(); ++i) {
      CHECK(sim.GetValue(names[i]) == actual[i]);
    }
  }
}

TEST_CASE("Disabled sizing provides setpoint placeholders", "[spawn][sizing]")
{
  const json zones = {{{"name", "Core_ZN"}}};
  const json config = {{"version", "0.1"},
                       {"EnergyPlus", {{"idf", idfpath.generic_string()}, {"weather", epwpath.generic_string()}}},
                       {"model",
                        {{"zones", zones},
                         {"hvacZones", {{{"name", "sys1"}, {"zones", zones}}}},
                         {"hvacSystems", {{{"name", "sys1"}, {"autosize", "false"}}}}}}};
  spawn::Spawn sim("disabled_sizing", spawn::test::idd_path(), config.dump(), spawn::test::get_current_test_dir());
  sim.Start();
  CHECK(sim.GetValue("Core_ZN_TSetCoo") == Approx(spawn::c_to_k(0.0)));
  CHECK(sim.GetValue("Core_ZN_TSetHea") == Approx(spawn::c_to_k(0.0)));
  CHECK(sim.GetValue("Core_ZN_XSetCoo") == 0.0);
  CHECK(sim.GetValue("Core_ZN_XSetHea") == 0.0);
  sim.Stop();
}

TEST_CASE("Sizing setpoints follow sensible peaks and preserve zero degrees", "[spawn][sizing]")
{
  EnergyPlus::EnergyPlusData state;
  auto &sizing = *state.dataSize;
  sizing.FinalZoneSizing.allocate(1);
  sizing.FinalZoneSizing(1).ZoneName = "Test Zone";
  sizing.ZoneSizing.allocate(2, 1);
  sizing.CalcZoneSizing.allocate(2, 1);
  for (int day = 1; day <= 2; ++day) {
    auto &calc = sizing.CalcZoneSizing(day, 1);
    calc.CoolLoadSeq.allocate(3);
    calc.HeatLoadSeq.allocate(3);
    calc.CoolLoadSeq = 0.0;
    calc.HeatLoadSeq = 0.0;
    auto &design = sizing.ZoneSizing(day, 1);
    design.DesCoolSetPtSeq.allocate(3);
    design.DesHeatSetPtSeq.allocate(3);
    design.DesCoolSetPtSeq = 80.0;
    design.DesHeatSetPtSeq = -10.0;
  }
  sizing.CalcZoneSizing(1, 1).CoolLoadSeq(2) = 100.0;
  sizing.CalcZoneSizing(2, 1).HeatLoadSeq(3) = 200.0;
  sizing.ZoneSizing(1, 1).DesCoolSetPtSeq(2) = 24.0;
  sizing.ZoneSizing(2, 1).DesHeatSetPtSeq(3) = 0.0;
  // Equipment sizing can select a different (latent) peak. Setpoint selection must follow sensible load.
  sizing.FinalZoneSizing(1).CoolDDNum = 2;
  sizing.FinalZoneSizing(1).TimeStepNumAtCoolMax = 1;
  sizing.FinalZoneSizing(1).HeatDDNum = 1;
  sizing.FinalZoneSizing(1).TimeStepNumAtHeatMax = 1;
  using namespace spawn::energyplus::zone_sizing;
  const SetpointSequence humidity = {{{1, 2}, 0.012}, {{2, 3}, 0.002}, {{2, 1}, 0.020}, {{1, 1}, 0.001}};
  CHECK(TemperatureSetpointAtPeak(state, 1, true, true) == 24.0);
  CHECK(TemperatureSetpointAtPeak(state, 1, false, true) == 0.0);
  CHECK(HumiditySetpointAtPeak(state, 1, true, true, humidity) == 0.012);
  CHECK(HumiditySetpointAtPeak(state, 1, false, true, humidity) == 0.002);
  CHECK(TemperatureSetpointAtPeak(state, 2, true, true) == 0.0);
  CHECK(HumiditySetpointAtPeak(state, 2, true, true, {}) == 0.0);
}
