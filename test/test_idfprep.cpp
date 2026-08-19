#include "../coroutine/idfprep.hpp"
#include "../coroutine/input/user_config.hpp"
#include "../util/config.hpp"
#include "paths.hpp"

#include <catch2/catch.hpp>

using json = nlohmann::json;

namespace {

const auto small_office_idf =
    spawn::project_source_dir() / "energyplus/testfiles/RefBldgSmallOfficeNew2004_Chicago.idf";
const auto chicago_epw =
    spawn::project_source_dir() / "energyplus/weather/USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw";

std::string autosizing_spawn_input()
{
  return fmt::format(
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
                {{ "name": "Core_ZN" }},
                {{ "name": "Perimeter_ZN_1" }}
              ]
            }}],
            "hvacSystems": [{{
              "name": "sys1",
              "autosize": "true"
            }}]
          }}
        }})",
      fmt::arg("idfpath", small_office_idf.generic_string()),
      fmt::arg("epwpath", chicago_epw.generic_string()));
}

} // namespace

TEST_CASE("Validate unsupported zone multipliers")
{
  SECTION("rejects a Zone multiplier directly")
  {
    json jsonidf = {{"Zone", {{"Repeated Zone", {{"multiplier", 4}}}}}};

    REQUIRE_THROWS_WITH(spawn::validate_idf(jsonidf), Catch::Contains("Zone 'Repeated Zone' has multiplier 4"));
  }

  SECTION("rejects a ZoneGroup multiplier and identifies affected zones")
  {
    json jsonidf = {
        {"Zone", {{"Zone One", json::object()}, {"Zone Two", json::object()}}},
        {"ZoneList",
         {{"Repeated Floor", {{"zones", json::array({{{"zone_name", "Zone One"}}, {{"zone_name", "Zone Two"}}})}}}}},
        {"ZoneGroup", {{"Repeated Floors", {{"zone_list_name", "repeated floor"}, {"zone_list_multiplier", 8}}}}}};

    REQUIRE_THROWS_WITH(spawn::validate_idf(jsonidf),
                        Catch::Contains("ZoneGroup 'Repeated Floors'") && Catch::Contains("multiplier 8") &&
                            Catch::Contains("ZoneList 'repeated floor'") && Catch::Contains("Zone One, Zone Two"));
  }

  SECTION("allows ZoneList grouping without multiplication")
  {
    json jsonidf = {
        {"Zone", {{"Zone One", json::object()}, {"Zone Two", json::object()}}},
        {"ZoneList",
         {{"One Floor", {{"zones", json::array({{{"zone_name", "Zone One"}}, {{"zone_name", "Zone Two"}}})}}}}},
        {"ZoneGroup", {{"One Floor Group", {{"zone_list_name", "One Floor"}, {"zone_list_multiplier", 1}}}}}};

    REQUIRE_NOTHROW(spawn::validate_idf(jsonidf));
  }
}

TEST_CASE("Prepare IDF with case-insensitive ZoneList references")
{
  const spawn::UserConfig user_config(autosizing_spawn_input());
  auto jsonidf = user_config.jsonidf;

  jsonidf["ZoneList"]["Mixed Case Zone List"] = {
      {"zones", json::array({{{"zone_name", "Core_ZN"}}, {{"zone_name", "Perimeter_ZN_1"}}})}};
  jsonidf["Sizing:Zone"] = {{"List Sizing", {{"zone_or_zonelist_name", "mixed case zone list"}}}};
  jsonidf["ZoneControl:Thermostat"] = {{"List Thermostat", {{"zone_or_zonelist_name", "MIXED CASE ZONE LIST"}}}};
  jsonidf.erase("ZoneControl:Thermostat:StagedDualSetpoint");
  jsonidf["ZoneInfiltration:DesignFlowRate"] = {
      {"List Infiltration",
       {{"zone_or_zonelist_or_space_or_spacelist_name", "Mixed Case ZONE List"},
        {"schedule_name", "Always On"},
        {"design_flow_rate_calculation_method", "Flow/Zone"},
        {"design_flow_rate", 0.1}}}};

  spawn::prepare_idf(jsonidf, user_config, spawn::StartTime());

  // Sizing:Zone expands to the concrete member zones.
  const auto &ideal_loads = jsonidf.at("ZoneHVAC:IdealLoadsAirSystem");
  CHECK(ideal_loads.contains("Core_ZN Ideal System"));
  CHECK(ideal_loads.contains("Perimeter_ZN_1 Ideal System"));
  CHECK_FALSE(ideal_loads.contains("mixed case zone list Ideal System"));

  // A thermostat assigned through a differently-cased ZoneList reference prevents default thermostat injection.
  const auto &thermostats = jsonidf.at("ZoneControl:Thermostat");
  CHECK(thermostats.contains("List Thermostat"));
  CHECK_FALSE(thermostats.contains("Spawn-Core_ZN-Default Thermostat"));
  CHECK_FALSE(thermostats.contains("Spawn-Perimeter_ZN_1-Default Thermostat"));

  // Infiltration expands to each member, then is removed only from the Modelica-connected zone.
  const auto &infiltration = jsonidf.at("ZoneInfiltration:DesignFlowRate");
  CHECK_FALSE(infiltration.contains("List Infiltration"));
  CHECK_FALSE(infiltration.contains("Spawn-Core_ZN-List Infiltration"));
  CHECK(infiltration.contains("Spawn-Perimeter_ZN_1-List Infiltration"));
  CHECK(infiltration.contains("Spawn-Core_ZN-Default Infiltration"));

  // A non-multiplying ZoneList remains valid input.
  CHECK(jsonidf.at("ZoneList").contains("Mixed Case Zone List"));
  REQUIRE_NOTHROW(spawn::validate_idf(jsonidf));
}
