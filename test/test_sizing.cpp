#include "../coroutine/spawn.hpp"
#include "../coroutine/idf_to_json.hpp"
#include "../util/config.hpp"
#include "../util/math.hpp"
#include "paths.hpp"
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
