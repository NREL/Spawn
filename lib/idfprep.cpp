#include "idfprep.hpp"
#include "../util/compare.hpp"
#include "input/input.hpp"

namespace spawn {

json &adjustSimulationControl(json &jsonidf)
{
  constexpr auto simulationcontroltype = "SimulationControl";

  // Remove the existing control first
  jsonidf.erase(simulationcontroltype);

  // This is what we need for spawn
  jsonidf[simulationcontroltype] = {{"Spawn-SimulationControl",
                                     {{"do_plant_sizing_calculation", "No"},
                                      {"do_system_sizing_calculation", "No"},
                                      {"do_zone_sizing_calculation", "No"},
                                      {"run_simulation_for_sizing_periods", "No"},
                                      {"run_simulation_for_weather_file_run_periods", "Yes"}}}};

  return jsonidf;
}

json &addRunPeriod(json &jsonidf)
{
  constexpr auto runperiodtype = "RunPeriod";
  // Remove the existing run periods first
  jsonidf.erase(runperiodtype);

  // Add a new run period just for spawn
  // 200 years should be plenty
  jsonidf[runperiodtype] = {{"Spawn-RunPeriod",
                             {{"apply_weekend_holiday_rule", "No"},
                              {"begin_day_of_month", 1},
                              {"begin_month", 1},
                              {"begin_year", 2017},
                              {"day_of_week_for_start_day", "Sunday"},
                              {"end_day_of_month", 31},
                              {"end_month", 12},
                              {"end_year", 2217},
                              {"use_weather_file_daylight_saving_period", "No"},
                              {"use_weather_file_holidays_and_special_days", "No"},
                              {"use_weather_file_rain_indicators", "Yes"},
                              {"use_weather_file_snow_indicators", "Yes"}}}};

  return jsonidf;
}

// An OtherEquipment object is added for each zone
// in order to support the Spawn input "QGaiRad_flow"
// A different approach would be to create a new EnergyPlus actuator,
// "otherRadiantGains" and include in the summation performed by
// the EnergyPlus function "SumAllInternalRadiationGains"
// Since there is already substantial manipulation of the idf by Spawn,
// this approach seems reasonable
// With this approach QGaiRad_flow will interface with the OtherEquipment actuator,
// Spawn user does not need to interface directly with the actuator
json &addOtherEquipment(json &jsonidf, const Input &input)
{
  constexpr auto scheduletype = "Schedule:Constant";
  constexpr auto schedulename = "Spawn-RadiantGains-Schedule";
  constexpr auto schedule_typelimits_type = "ScheduleTypeLimits";
  constexpr auto schedule_typelimits_name = "Spawn-RadiantGains-Schedule-Limits";

  jsonidf[schedule_typelimits_type][schedule_typelimits_name] = {};

  jsonidf[scheduletype][schedulename] = {{"schedule_type_limits_name", schedule_typelimits_name},
                                         {"hourly_value", "1.0"}};

  for (const auto &zone : input.zones) {
    if (!zone.isconnected) {
      continue;
    }

    jsonidf[Zone::ep_qgairad_flow_object_type][zone.ep_qgairad_flow_object_name] = {
        {"fuel_type", "None"},
        {"zone_or_zonelist_or_space_or_spacelist_name", zone.idfname},
        {"schedule_name", schedulename},
        {"design_level_calculation_method", "EquipmentLevel"},
        {"design_level", 0.0},
        {"power_per_zone_floor_area", 0.0},
        {"power_per_person", 0.0},
        {"fraction_latent", 0.0},
        {"fraction_radiant", 1.0},
        {"fraction_lost", 0.0}};
  }

  return jsonidf;
}

// Remove objects related to HVAC and controls
json &removeUnusedObjects(json &jsonidf)
{
  for (auto typep = jsonidf.cbegin(); typep != jsonidf.cend();) {
    if (std::find(std::begin(supportedIDDTypes), std::end(supportedIDDTypes), typep.key()) ==
        std::end(supportedIDDTypes)) {
      typep = jsonidf.erase(typep);
    } else {
      ++typep;
    }
  }

  // Remove unsupported output vars
  auto outputvars = jsonidf.find("Output:Variable");
  if (outputvars != jsonidf.end()) {
    for (auto var = outputvars->cbegin(); var != outputvars->cend();) {
      const auto &name = var.value().at("variable_name").get<std::string>();
      const auto &findit = std::find_if(std::begin(outputtypes), std::end(outputtypes), [&](const OutputProperties &v) {
        return case_insensitive_compare(v.name, name);
      });

      if (findit == std::end(outputtypes)) {
        var = outputvars->erase(var);
      } else {
        ++var;
      }
    }
  }

  return jsonidf;
}

json &addPeopleOutputVariables(json &jsonidf, const Input &input)
{
  // Some zones don't have people input, which will result in an EnergyPlus error,
  // Insert a default people object that defines zero people
  constexpr auto scheduletype = "Schedule:Constant";
  constexpr auto peopleSchedulename = "Spawn-People-Schedule";
  constexpr auto activitySchedulename = "Spawn-PeopleActivity-Schedule";
  constexpr auto schedule_typelimits_type = "ScheduleTypeLimits";
  constexpr auto schedule_typelimits_name = "Spawn-People-Limits";

  jsonidf[schedule_typelimits_type][schedule_typelimits_name] = {};

  jsonidf[scheduletype][peopleSchedulename] = {{"schedule_type_limits_name", schedule_typelimits_name},
                                               {"hourly_value", "1.0"}};

  jsonidf[scheduletype][activitySchedulename] = {{"schedule_type_limits_name", schedule_typelimits_name},
                                                 {"hourly_value", "100.0"}};

  for (const auto &zone : input.zones) {
    if (!zone.isconnected) {
      continue;
    }

    jsonidf[Zone::ep_people_object_type][zone.ep_qgairad_flow_object_name] = {
        {"zone_or_zonelist_or_space_or_spacelist_name", zone.idfname},
        {"number_of_people_schedule_name", peopleSchedulename},
        {"number_of_people_calculation_method", "People"},
        {"number_of_people", "0"},
        {"people_per_zone_floor_area", "0"},
        {"zone_floor_area_per_person", "0"},
        {"fraction_radiant", "0"},
        {"sensible_heat_fraction", "autocalculate"},
        {"activity_level_schedule_name", activitySchedulename}};

    jsonidf[zone.ep_outputvariable_type][zone.ep_qpeo_flow_object_name] = {
        {"variable_name", zone.ep_qpeo_flow_output_var_name},
        {"key_value", zone.idfname},
        {"reporting_frequency", "Timestep"}};
  }

  return jsonidf;
}

// Add output variables requested in the spawn input file, but not in the idf
json &addRequestedOutputVariables(json &jsonidf, const Input &input)
{
  // A pair that holds an output variable name and key,
  using Varpair = std::pair<std::string, std::string>;

  // Make a list of the requested outputs
  std::vector<Varpair> requestedpairs;
  for (const auto &var : input.outputVariables) {
    requestedpairs.emplace_back(var.idfname, var.idfkey);
  }

  // And a list of the current output variables
  auto &currentvars = jsonidf["Output:Variable"];
  std::vector<Varpair> currentpairs;
  for (const auto &var : currentvars) {
    currentpairs.emplace_back(var.at("variable_name").get<std::string>(), var.at("key_value").get<std::string>());
  }

  // Identify any missing pairs. ie. those that are requested but not in the idf
  std::vector<Varpair> missingpairs;
  std::sort(requestedpairs.begin(), requestedpairs.end());
  std::sort(currentpairs.begin(), currentpairs.end());

  std::set_difference(requestedpairs.begin(),
                      requestedpairs.end(),
                      currentpairs.begin(),
                      currentpairs.end(),
                      std::back_inserter(missingpairs));

  for (const auto &pair : missingpairs) {
    json newvar;
    newvar["variable_name"] = pair.first;
    newvar["key_value"] = pair.second;
    newvar["reporting_frequency"] = "Timestep";
    currentvars[pair.first + pair.second] = newvar;
  }

  return jsonidf;
}

// Undo the use of zone list for infiltration objects
// zone lists are only allowed for ZoneInfiltration:DesignFlowRate
// other types of infiltration input do not allow zone lists
// so this work is not required for those other types
json &expandInfiltrationZoneLists(json &jsonidf)
{
  const auto *const infiltrationType = "ZoneInfiltration:DesignFlowRate";
  auto &infiltrationObjects = jsonidf[infiltrationType];

  const auto *const zoneListType = "ZoneList";
  const auto zoneListObjects = jsonidf[zoneListType];

  json newInfiltration;
  std::vector<std::string> infNamesToRemove;

  for (const auto &[infname, inffields] : infiltrationObjects.items()) {
    const auto possibleZoneListName = inffields.at("zone_or_zonelist_name").get<std::string>();
    // if zoneName is the name of a zone list and not a real zone....
    if (zoneListObjects.contains(possibleZoneListName)) {
      const auto zoneNameObjects = zoneListObjects.value(possibleZoneListName, json()).value("zones", json());
      if (!zoneNameObjects.is_null()) {
        // need to expand the infiltration objects associated with this zonelist
        for (const auto &zoneNameObject : zoneNameObjects) {
          const auto zoneName = zoneNameObject.at("zone_name").get<std::string>();
          auto newInfName = fmt::format("Spawn-{}-{}", zoneName, infname);
          newInfiltration[newInfName] = inffields;
          newInfiltration[newInfName]["zone_or_zonelist_name"] = zoneName;
        }
        // Also need to remove the orginal infiltration
        infNamesToRemove.push_back(infname);
      }
    }
  }

  // Remove the old infiltration objects
  for (const auto &name : infNamesToRemove) {
    infiltrationObjects.erase(name);
  }

  // Add the expanded infiltration objects
  for (const auto &[name, fields] : newInfiltration.items()) {
    infiltrationObjects[name] = fields;
  }

  return jsonidf;
}

// Connected zones will have user provided infiltration objects removed,
// however if zones do not have any infiltration input, then report variables and
// actuators related to infiltration are not available. This can be confusing to users,
// since some zones have infiltration output variables and actuators while others don't.
// To address this confusion we will insert default infiltration objects for connected zones,
// which have zero flow specified.
json &addDefaultZeroInfiltration(json &jsonidf, const Input &input)
{
  constexpr auto infiltrationType = "ZoneInfiltration:DesignFlowRate";
  constexpr auto scheduletype = "Schedule:Constant";
  constexpr auto schedulename = "Spawn-DefaultInfiltration-Schedule";
  constexpr auto schedule_typelimits_type = "ScheduleTypeLimits";
  constexpr auto schedule_typelimits_name = "Spawn-DefaultInfiltration-Schedule-Limits";

  jsonidf[schedule_typelimits_type][schedule_typelimits_name] = {};

  jsonidf[scheduletype][schedulename] = {{"schedule_type_limits_name", schedule_typelimits_name},
                                         {"hourly_value", "1.0"}};

  const auto zones = input.zones;
  for (const auto &zone : zones) {
    // Only add default infiltration for "connected" zones
    if (zone.isconnected) {
      const auto infiltrationName = std::string("Spawn-") + zone.idfname + "-Default Infiltration";

      jsonidf[infiltrationType][infiltrationName] = {{"air_changes_per_hour", 0.0},
                                                     {"constant_term_coefficient", 1},
                                                     {"design_flow_rate_calculation_method", "AirChanges/Hour"},
                                                     {"schedule_name", schedulename},
                                                     {"temperature_term_coefficient", 0},
                                                     {"velocity_squared_term_coefficient", 0},
                                                     {"velocity_term_coefficient", 0},
                                                     {"zone_or_zonelist_name", zone.idfname}};
    }
  }

  return jsonidf;
}

// Remove infiltration idf input objects for zones that are connected to Modelica
json &removeInfiltration(json &jsonidf, const Input &input)
{
  // First expand any infiltration that uses zone lists
  expandInfiltrationZoneLists(jsonidf);

  // Idf infiltration type paired with the field which identifies the related zone
  constexpr std::array<std::pair<const char *, const char *>, 3> infiltrationTypes = {
      {{"ZoneInfiltration:DesignFlowRate", "zone_or_zonelist_name"},
       {"ZoneInfiltration:EffectiveLeakageArea", "zone_name"},
       {"ZoneInfiltration:FlowCoefficient", "zone_name"}}};

  const auto zones = input.zones;

  for (const auto &type : infiltrationTypes) {
    auto &infiltrationObjects = jsonidf[type.first];
    for (auto var = infiltrationObjects.cbegin(); var != infiltrationObjects.cend();) {
      const auto zoneName = var->at(type.second).get<std::string>();
      const auto connected_zone_it = std::find_if(
          zones.cbegin(), zones.cend(), [&](const spawn::Zone &z) { return z.isconnected && (z.idfname == zoneName); });
      if (connected_zone_it != zones.cend()) {
        var = infiltrationObjects.erase(var);
      } else {
        ++var;
      }
    }
  }

  addDefaultZeroInfiltration(jsonidf, input);

  return jsonidf;
}

void prepare_idf(json &jsonidf, const Input &input)
{
  adjustSimulationControl(jsonidf);
  removeUnusedObjects(jsonidf);
  addRunPeriod(jsonidf);
  removeInfiltration(jsonidf, input);
  addOtherEquipment(jsonidf, input);
  addRequestedOutputVariables(jsonidf, input);
  addPeopleOutputVariables(jsonidf, input);
}

void validate_idf(json &jsonidf)
{
  std::vector<std::string> multiplier_zones;

  auto &zone_objects = jsonidf["Zone"];
  for (const auto &[name, fields] : zone_objects.items()) {
    const auto multiplier = fields.value("multiplier", 1);
    if (multiplier != 1) {
      multiplier_zones.push_back(name);
    }
  }

  if (!multiplier_zones.empty()) {
    std::string names;
    for (const auto &name : multiplier_zones) {
      if (multiplier_zones.back() == name) {
        // Each zone name except the last gets a comman and space appended
        names.append(name);
      } else {
        names.append(name + ", ");
      }
    }
    const auto message = fmt::format(
        "The Spawn version of EnergyPlus does not support the zone multiplier input for the zones named: {}.", names);
    throw std::runtime_error(message);
  }
}

} // namespace spawn
