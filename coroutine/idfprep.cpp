#include "idfprep.hpp"
#include "input/user_config.hpp"
#include "util/conversion.hpp"
#include "util/strings.hpp"

#include <spdlog/spdlog.h>

#include <unordered_set>

namespace spawn {

namespace {

  std::vector<std::string> expand_zone_or_zonelist(const json &zone_list_objects, const std::string &zone_or_zonelist_name)
  {
    std::vector<std::string> zone_names;

    if (zone_or_zonelist_name.empty()) {
      return zone_names;
    }

    if (zone_list_objects.contains(zone_or_zonelist_name)) {
      const auto zone_name_objects = zone_list_objects.value(zone_or_zonelist_name, json()).value("zones", json());
      for (const auto &zone_name_object : zone_name_objects) {
        zone_names.push_back(zone_name_object.at("zone_name").get<std::string>());
      }
    } else {
      zone_names.push_back(zone_or_zonelist_name);
    }

    return zone_names;
  }

  std::vector<std::string> autosize_zone_names(const json &jsonidf)
  {
    const auto zone_sizing_objects = jsonidf.value("Sizing:Zone", json());
    if (!zone_sizing_objects.is_object()) {
      return {};
    }

    const auto zone_list_objects = jsonidf.value("ZoneList", json::object());
    std::unordered_set<std::string> zone_names;

    for (const auto &zone_sizing : zone_sizing_objects) {
      const auto zone_or_zonelist_name = zone_sizing.value("zone_or_zonelist_name", "");
      for (const auto &zone_name : expand_zone_or_zonelist(zone_list_objects, zone_or_zonelist_name)) {
        zone_names.insert(zone_name);
      }
    }

    return std::vector<std::string>(zone_names.begin(), zone_names.end());
  }

  std::string unique_object_name(const json &objects, const std::string &base_name)
  {
    if (!objects.contains(base_name)) {
      return base_name;
    }

    int suffix = 1;
    std::string candidate = fmt::format("{} {}", base_name, suffix);
    while (objects.contains(candidate)) {
      ++suffix;
      candidate = fmt::format("{} {}", base_name, suffix);
    }

    return candidate;
  }

  json &adjustSimulationControl(json &jsonidf, const UserConfig &user_config)
  {
    constexpr auto simulationcontroltype = "SimulationControl";

    // Remove the existing control first
    jsonidf.erase(simulationcontroltype);

    // TODO: fix this slop. Please add a consistent method of handling bool types
    const auto autosize = user_config.autosize() ? "Yes" : "No";
    // This is what we need for spawn
    jsonidf[simulationcontroltype] = {{"Spawn-SimulationControl",
                                       {{"do_plant_sizing_calculation", "No"},
                                        {"do_system_sizing_calculation", "No"},
                                        {"do_zone_sizing_calculation", autosize},
                                        {"run_simulation_for_sizing_periods", "No"},
                                        {"run_simulation_for_weather_file_run_periods", "Yes"}}}};
    return jsonidf;
  }

  json &addIdealLoads(json &jsonidf, const UserConfig &user_config)
  {
    if (user_config.autosize()) {
      jsonidf["ZoneHVAC:EquipmentConnections"] = json();
      jsonidf["ZoneHVAC:EquipmentList"] = json();
      jsonidf["ZoneHVAC:IdealLoadsAirSystem"] = json();

      for (const auto &zone_name : autosize_zone_names(jsonidf)) {
        const auto air_node_name = fmt::format("{} Air Node", zone_name);
        const auto supply_node_name = fmt::format("{} Supply Node", zone_name);
        const auto exhaust_node_name = fmt::format("{} Exhaust Node", zone_name);
        const auto equipment_list_name = fmt::format("{} Equipment List", zone_name);
        const auto ideal_system_name = fmt::format("{} Ideal System", zone_name);

        // clang-format off
      const json ideal_system = {
        {"name", ideal_system_name},
        {"zone_supply_air_node_name", supply_node_name},
        {"zone_exhaust_air_node_name", exhaust_node_name}
      };

      jsonidf["ZoneHVAC:IdealLoadsAirSystem"][ideal_system_name] = ideal_system;

      const json equipment_list = {
        {"load_distribution_scheme", "SequentialLoad"},
        {"equipment", {
          {
            {"zone_equipment_cooling_sequence", 1},
            {"zone_equipment_heating_or_no_load_sequence", 1},
            {"zone_equipment_name", ideal_system_name},
            {"zone_equipment_object_type", "ZoneHVAC:IdealLoadsAirSystem"}
          }
        }}
      };

      jsonidf["ZoneHVAC:EquipmentList"][equipment_list_name] = equipment_list;

      const json hvac_connections = {
        {"zone_air_node_name", air_node_name},
        {"zone_conditioning_equipment_list_name", equipment_list_name},
        {"zone_air_inlet_node_or_nodelist_name", supply_node_name},
        {"zone_air_exhaust_node_or_nodelist_name", exhaust_node_name},
        {"zone_name", zone_name}
      };

      jsonidf["ZoneHVAC:EquipmentConnections"][zone_name] = hvac_connections;
        // clang-format on
      }
    }

    return jsonidf;
  }

  json &addDefaultZoneControls(json &jsonidf, const UserConfig &user_config)
  {
    if (!user_config.autosize()) {
      return jsonidf;
    }

    const auto zone_names = autosize_zone_names(jsonidf);
    if (zone_names.empty()) {
      return jsonidf;
    }

    constexpr auto schedule_type = "Schedule:Constant";
    constexpr auto control_type_schedule = "Spawn-DefaultThermostat-ControlType";
    constexpr auto heating_setpoint_schedule = "Spawn-DefaultThermostat-Heating";
    constexpr auto cooling_setpoint_schedule = "Spawn-DefaultThermostat-Cooling";
    constexpr auto humidifying_schedule = "Spawn-DefaultHumidistat-Humidify";
    constexpr auto dehumidifying_schedule = "Spawn-DefaultHumidistat-Dehumidify";

    if (!jsonidf.contains(schedule_type)) {
      jsonidf[schedule_type] = json::object();
    }
    auto &schedules = jsonidf[schedule_type];

    if (!schedules.contains(control_type_schedule)) {
      schedules[control_type_schedule] = {{"hourly_value", 4.0}};
    }
    if (!schedules.contains(heating_setpoint_schedule)) {
      schedules[heating_setpoint_schedule] = {{"hourly_value", 20.0}};
    }
    if (!schedules.contains(cooling_setpoint_schedule)) {
      schedules[cooling_setpoint_schedule] = {{"hourly_value", 22.0}};
    }
    if (!schedules.contains(humidifying_schedule)) {
      schedules[humidifying_schedule] = {{"hourly_value", 45.0}};
    }
    if (!schedules.contains(dehumidifying_schedule)) {
      schedules[dehumidifying_schedule] = {{"hourly_value", 55.0}};
    }

    const auto zone_list_objects = jsonidf.value("ZoneList", json::object());
    std::unordered_set<std::string> thermostat_zones;
    std::unordered_set<std::string> humidistat_zones;

    if (jsonidf.contains("ZoneControl:Thermostat")) {
      for (const auto &[name, fields] : jsonidf["ZoneControl:Thermostat"].items()) {
        const auto zone_or_list = fields.value("zone_or_zonelist_name", "");
        for (const auto &zone_name : expand_zone_or_zonelist(zone_list_objects, zone_or_list)) {
          thermostat_zones.insert(zone_name);
        }
      }
    }

    if (jsonidf.contains("ZoneControl:Thermostat:StagedDualSetpoint")) {
      for (const auto &[name, fields] : jsonidf["ZoneControl:Thermostat:StagedDualSetpoint"].items()) {
        const auto zone_or_list = fields.value("zone_or_zonelist_name", "");
        for (const auto &zone_name : expand_zone_or_zonelist(zone_list_objects, zone_or_list)) {
          thermostat_zones.insert(zone_name);
        }
      }
    }

    if (jsonidf.contains("ZoneControl:Humidistat")) {
      for (const auto &[name, fields] : jsonidf["ZoneControl:Humidistat"].items()) {
        const auto zone_name = fields.value("zone_name", "");
        if (!zone_name.empty()) {
          humidistat_zones.insert(zone_name);
        }
      }
    }

    if (!jsonidf.contains("ZoneControl:Thermostat")) {
      jsonidf["ZoneControl:Thermostat"] = json::object();
    }
    if (!jsonidf.contains("ThermostatSetpoint:DualSetpoint")) {
      jsonidf["ThermostatSetpoint:DualSetpoint"] = json::object();
    }
    if (!jsonidf.contains("ZoneControl:Humidistat")) {
      jsonidf["ZoneControl:Humidistat"] = json::object();
    }

    auto &thermostat_objects = jsonidf["ZoneControl:Thermostat"];
    auto &setpoint_objects = jsonidf["ThermostatSetpoint:DualSetpoint"];
    auto &humidistat_objects = jsonidf["ZoneControl:Humidistat"];

    for (const auto &zone_name : zone_names) {
      if (thermostat_zones.find(zone_name) == thermostat_zones.end()) {
        const auto thermostat_name =
            unique_object_name(thermostat_objects, fmt::format("Spawn-{}-Default Thermostat", zone_name));
        const auto setpoint_name =
            unique_object_name(setpoint_objects, fmt::format("Spawn-{}-Default Setpoint", zone_name));

        thermostat_objects[thermostat_name] = {{"zone_or_zonelist_name", zone_name},
                                               {"control_type_schedule_name", control_type_schedule},
                                               {"control_1_object_type", "ThermostatSetpoint:DualSetpoint"},
                                               {"control_1_name", setpoint_name}};

        setpoint_objects[setpoint_name] = {{"heating_setpoint_temperature_schedule_name", heating_setpoint_schedule},
                                           {"cooling_setpoint_temperature_schedule_name", cooling_setpoint_schedule}};
        spdlog::warn(
            "Injected default thermostat for autosized zone '{}' (heating {:.1f} C, cooling {:.1f} C) because none was defined.",
            zone_name,
            20.0,
            22.0);
      }

      if (humidistat_zones.find(zone_name) == humidistat_zones.end()) {
        const auto humidistat_name =
            unique_object_name(humidistat_objects, fmt::format("Spawn-{}-Default Humidistat", zone_name));
        humidistat_objects[humidistat_name] = {
            {"zone_name", zone_name},
            {"humidifying_relative_humidity_setpoint_schedule_name", humidifying_schedule},
            {"dehumidifying_relative_humidity_setpoint_schedule_name", dehumidifying_schedule}};
        spdlog::warn(
            "Injected default humidistat for autosized zone '{}' (humidify {:.0f}%%, dehumidify {:.0f}%%) because none was defined.",
            zone_name,
            45.0,
            55.0);
      }
    }

    return jsonidf;
  }

  json &addRunPeriod(json &jsonidf, [[maybe_unused]] const UserConfig &user_config, const StartTime &start_time)
  {
    constexpr auto runperiodtype = "RunPeriod";
    // Remove the existing run periods first
    jsonidf.erase(runperiodtype);

    // Add a new run period just for spawn
    // 200 years should be plenty
    jsonidf[runperiodtype] = {
        {"Spawn-RunPeriod",
         {{"apply_weekend_holiday_rule", user_config.runPeriod.apply_weekend_holiday_rule},
          {"begin_day_of_month", int(start_time.energyplus_epoch().day())},
          {"begin_month", int(start_time.energyplus_epoch().month())},
          {"begin_year", int(start_time.energyplus_epoch().year())},
          {"day_of_week_for_start_day", string_from_day(start_time.energyplus_epoch().day_of_week().as_enum())},
          {"end_day_of_month", 31},
          {"end_month", 12},
          {"end_year", 2217},
          {"use_weather_file_daylight_saving_period", user_config.runPeriod.use_weather_file_daylight_saving_period},
          {"use_weather_file_holidays_and_special_days",
           user_config.runPeriod.use_weather_file_holidays_and_special_days},
          {"use_weather_file_rain_indicators", user_config.runPeriod.use_weather_file_rain_indicators},
          {"use_weather_file_snow_indicators", user_config.runPeriod.use_weather_file_snow_indicators}}}};

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
  json &addOtherEquipment(json &jsonidf, const UserConfig &user_config)
  {
    constexpr auto scheduletype = "Schedule:Constant";
    constexpr auto schedulename = "Spawn-RadiantGains-Schedule";
    constexpr auto schedule_typelimits_type = "ScheduleTypeLimits";
    constexpr auto schedule_typelimits_name = "Spawn-RadiantGains-Schedule-Limits";

    jsonidf[schedule_typelimits_type][schedule_typelimits_name] = {};

    jsonidf[scheduletype][schedulename] = {{"schedule_type_limits_name", schedule_typelimits_name},
                                           {"hourly_value", "1.0"}};

    for (const auto &zone : user_config.zones) {
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
        const auto &findit = std::find_if(
            std::begin(spawn::energyplus::output_types),
            std::end(spawn::energyplus::output_types),
            [&](const spawn::energyplus::OutputType &v) { return case_insensitive_compare(v.name, name); });

        if (findit == std::end(spawn::energyplus::output_types)) {
          var = outputvars->erase(var);
        } else {
          ++var;
        }
      }
    }

    return jsonidf;
  }

  json &addPeopleOutputVariables(json &jsonidf, const UserConfig &user_config)
  {
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

    const auto &has_people = [&](const std::string &zone_name) {
      const auto all_people_inputs = jsonidf[Zone::ep_people_object_type];

      return std::any_of(all_people_inputs.begin(), all_people_inputs.end(), [&zone_name](const auto &p) {
        return (case_insensitive_compare(p["zone_or_zonelist_or_space_or_spacelist_name"], zone_name));
      });
    };

    for (const auto &zone : user_config.zones) {
      if (!zone.isconnected) {
        continue;
      }

      // Some zones don't have people input, which will result in an EnergyPlus error,
      // Insert a default people object that defines zero people
      if (!has_people(zone.idfname)) {
        jsonidf[Zone::ep_people_object_type][zone.idfname + " Default People"] = {
            {"zone_or_zonelist_or_space_or_spacelist_name", zone.idfname},
            {"number_of_people_schedule_name", peopleSchedulename},
            {"number_of_people_calculation_method", "People"},
            {"number_of_people", "0"},
            {"people_per_zone_floor_area", "0"},
            {"zone_floor_area_per_person", "0"},
            {"fraction_radiant", "0"},
            {"sensible_heat_fraction", "autocalculate"},
            {"activity_level_schedule_name", activitySchedulename}};
      }

      jsonidf[zone.ep_outputvariable_type][zone.ep_qpeo_flow_object_name] = {
          {"variable_name", zone.ep_qpeo_flow_output_var_name},
          {"key_value", zone.idfname},
          {"reporting_frequency", "Timestep"}};
    }

    return jsonidf;
  }

  // Add output variables requested in the spawn input file, but not in the idf
  json &addRequestedOutputVariables(json &jsonidf, const UserConfig &user_config)
  {
    // A pair that holds an output variable name and key,
    using Varpair = std::pair<std::string, std::string>;

    // Make a list of the requested outputs
    std::vector<Varpair> requestedpairs;
    for (const auto &var : user_config.outputVariables) {
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
      const auto possibleZoneListName = inffields.at("zone_or_zonelist_or_space_or_spacelist_name").get<std::string>();
      // if zoneName is the name of a zone list and not a real zone....
      if (zoneListObjects.contains(possibleZoneListName)) {
        const auto zoneNameObjects = zoneListObjects.value(possibleZoneListName, json()).value("zones", json());
        if (!zoneNameObjects.is_null()) {
          // need to expand the infiltration objects associated with this zonelist
          for (const auto &zoneNameObject : zoneNameObjects) {
            const auto zoneName = zoneNameObject.at("zone_name").get<std::string>();
            auto newInfName = fmt::format("Spawn-{}-{}", zoneName, infname);
            newInfiltration[newInfName] = inffields;
            newInfiltration[newInfName]["zone_or_zonelist_or_space_or_spacelist_name"] = zoneName;
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
  json &addDefaultZeroInfiltration(json &jsonidf, const UserConfig &user_config)
  {
    constexpr auto infiltrationType = "ZoneInfiltration:DesignFlowRate";
    constexpr auto scheduletype = "Schedule:Constant";
    constexpr auto schedulename = "Spawn-DefaultInfiltration-Schedule";
    constexpr auto schedule_typelimits_type = "ScheduleTypeLimits";
    constexpr auto schedule_typelimits_name = "Spawn-DefaultInfiltration-Schedule-Limits";

    jsonidf[schedule_typelimits_type][schedule_typelimits_name] = {};

    jsonidf[scheduletype][schedulename] = {{"schedule_type_limits_name", schedule_typelimits_name},
                                           {"hourly_value", "1.0"}};

    const auto zones = user_config.zones;
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
                                                       {"zone_or_zonelist_or_space_or_spacelist_name", zone.idfname}};
      }
    }

    return jsonidf;
  }

  // Remove infiltration idf input objects for zones that are connected to Modelica
  json &removeInfiltration(json &jsonidf, const UserConfig &user_config)
  {
    // First expand any infiltration that uses zone lists
    expandInfiltrationZoneLists(jsonidf);

    // Idf infiltration type paired with the field which identifies the related zone
    constexpr std::array<std::pair<const char *, const char *>, 3> infiltrationTypes = {
        {{"ZoneInfiltration:DesignFlowRate", "zone_or_zonelist_or_space_or_spacelist_name"},
         {"ZoneInfiltration:EffectiveLeakageArea", "zone_name"},
         {"ZoneInfiltration:FlowCoefficient", "zone_name"}}};

    const auto zones = user_config.zones;

    for (const auto &type : infiltrationTypes) {
      auto &infiltrationObjects = jsonidf[type.first];
      for (auto var = infiltrationObjects.cbegin(); var != infiltrationObjects.cend();) {
        const auto zoneName = var->at(type.second).get<std::string>();
        const auto connected_zone_it = std::find_if(zones.cbegin(), zones.cend(), [&](const spawn::Zone &z) {
          return z.isconnected && (z.idfname == zoneName);
        });
        if (connected_zone_it != zones.cend()) {
          var = infiltrationObjects.erase(var);
        } else {
          ++var;
        }
      }
    }

    addDefaultZeroInfiltration(jsonidf, user_config);

    return jsonidf;
  }

} // namespace

void prepare_idf(json &jsonidf, const UserConfig &user_config, const StartTime &start_time)
{
  removeUnusedObjects(jsonidf);
  adjustSimulationControl(jsonidf, user_config);
  addIdealLoads(jsonidf, user_config);
  addDefaultZoneControls(jsonidf, user_config);
  if (user_config.autosize() && autosize_zone_names(jsonidf).empty()) {
    spdlog::warn(
        "Autosize was requested, but no Sizing:Zone objects were found; Ideal Loads and default thermostat/humidistat "
        "injection will be skipped.");
  }
  addRunPeriod(jsonidf, user_config, start_time);
  removeInfiltration(jsonidf, user_config);
  addOtherEquipment(jsonidf, user_config);
  addRequestedOutputVariables(jsonidf, user_config);
  addPeopleOutputVariables(jsonidf, user_config);
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
