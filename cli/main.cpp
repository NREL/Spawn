#include <CLI/CLI.hpp>
#include "third_party/nlohmann/json.hpp"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <pugixml.hpp>
#include "modelDescription.xml.hpp"
#include "ziputil.hpp"
#include "../epfmi/Variables.hpp"
#include <config.hxx>

#include "EnergyPlus/InputProcessing/IdfParser.hh"
#include "EnergyPlus/InputProcessing/EmbeddedEpJSONSchema.hh"
#include "EnergyPlus/DataStringGlobals.hh"

#if defined _WIN32
#include <windows.h>
#else
#include <stdio.h>
#include <dlfcn.h>
#endif

using json = nlohmann::json;

constexpr std::array<const char *,312> supportedIDDTypes = {
  "Version",
  "SimulationControl",
  "PerformancePrecisionTradeoffs",
  "Building",
  "ShadowCalculation",
  "SurfaceConvectionAlgorithm:Inside",
  "SurfaceConvectionAlgorithm:Outside",
  "HeatBalanceAlgorithm",
  "HeatBalanceSettings:ConductionFiniteDifference",
  "ZoneAirHeatBalanceAlgorithm",
  "Timestep",
  "ConvergenceLimits",
  "Compliance:Building",
  "Site:Location",
  "Site:VariableLocation",
  "RunPeriod",
  "RunPeriodControl:SpecialDays",
  "RunPeriodControl:DaylightSavingTime",
  "WeatherProperty:SkyTemperature",
  "Site:WeatherStation",
  "Site:HeightVariation",
  "Site:GroundTemperature:BuildingSurface",
  "Site:GroundTemperature:FCfactorMethod",
  "Site:GroundTemperature:Shallow",
  "Site:GroundTemperature:Deep",
  "Site:GroundTemperature:Undisturbed:FiniteDifference",
  "Site:GroundTemperature:Undisturbed:KusudaAchenbach",
  "Site:GroundTemperature:Undisturbed:Xing",
  "Site:GroundDomain:Slab",
  "Site:GroundDomain:Basement",
  "Site:GroundReflectance",
  "Site:GroundReflectance:SnowModifier",
  "Site:WaterMainsTemperature",
  "Site:Precipitation",
  "Site:SolarAndVisibleSpectrum",
  "Site:SpectrumData",
  "ScheduleTypeLimits",
  "Schedule:Day:Hourly",
  "Schedule:Day:Interval",
  "Schedule:Day:List",
  "Schedule:Week:Daily",
  "Schedule:Week:Compact",
  "Schedule:Year",
  "Schedule:Compact",
  "Schedule:Constant",
  "Schedule:File:Shading",
  "Schedule:File",
  "Material",
  "Material:NoMass",
  "Material:InfraredTransparent",
  "Material:AirGap",
  "Material:RoofVegetation",
  "WindowMaterial:SimpleGlazingSystem",
  "WindowMaterial:Glazing",
  "WindowMaterial:GlazingGroup:Thermochromic",
  "WindowMaterial:Glazing:RefractionExtinctionMethod",
  "WindowMaterial:Gas",
  "WindowGap:SupportPillar",
  "WindowGap:DeflectionState",
  "WindowMaterial:GasMixture",
  "WindowMaterial:Gap",
  "WindowMaterial:Shade",
  "WindowMaterial:ComplexShade",
  "WindowMaterial:Blind",
  "WindowMaterial:Screen",
  "WindowMaterial:Shade:EquivalentLayer",
  "WindowMaterial:Drape:EquivalentLayer",
  "WindowMaterial:Blind:EquivalentLayer",
  "WindowMaterial:Screen:EquivalentLayer",
  "WindowMaterial:Glazing:EquivalentLayer",
  "WindowMaterial:Gap:EquivalentLayer",
  "MaterialProperty:MoisturePenetrationDepth:Settings",
  "MaterialProperty:PhaseChange",
  "MaterialProperty:PhaseChangeHysteresis",
  "MaterialProperty:VariableThermalConductivity",
  "MaterialProperty:HeatAndMoistureTransfer:Settings",
  "MaterialProperty:HeatAndMoistureTransfer:SorptionIsotherm",
  "MaterialProperty:HeatAndMoistureTransfer:Suction",
  "MaterialProperty:HeatAndMoistureTransfer:Redistribution",
  "MaterialProperty:HeatAndMoistureTransfer:Diffusion",
  "MaterialProperty:HeatAndMoistureTransfer:ThermalConductivity",
  "MaterialProperty:GlazingSpectralData",
  "Construction",
  "Construction:CfactorUndergroundWall",
  "Construction:FfactorGroundFloor",
  "Construction:InternalSource",
  "Construction:AirBoundary",
  "WindowThermalModel:Params",
  "WindowsCalculationEngine",
  "Construction:ComplexFenestrationState",
  "Construction:WindowEquivalentLayer",
  "Construction:WindowDataFile",
  "GlobalGeometryRules",
  "GeometryTransform",
  "Zone",
  "ZoneList",
  "ZoneGroup",
  "BuildingSurface:Detailed",
  "Wall:Detailed",
  "RoofCeiling:Detailed",
  "Floor:Detailed",
  "Wall:Exterior",
  "Wall:Adiabatic",
  "Wall:Underground",
  "Wall:Interzone",
  "Roof",
  "Ceiling:Adiabatic",
  "Ceiling:Interzone",
  "Floor:GroundContact",
  "Floor:Adiabatic",
  "Floor:Interzone",
  "FenestrationSurface:Detailed",
  "Window",
  "Door",
  "GlazedDoor",
  "Window:Interzone",
  "Door:Interzone",
  "GlazedDoor:Interzone",
  "WindowShadingControl",
  "WindowProperty:FrameAndDivider",
  "WindowProperty:AirflowControl",
  "WindowProperty:StormWindow",
  "InternalMass",
  "Shading:Site",
  "Shading:Building",
  "Shading:Site:Detailed",
  "Shading:Building:Detailed",
  "Shading:Overhang",
  "Shading:Overhang:Projection",
  "Shading:Fin",
  "Shading:Fin:Projection",
  "Shading:Zone:Detailed",
  "ShadingProperty:Reflectance",
  "SurfaceProperty:HeatTransferAlgorithm",
  "SurfaceProperty:HeatTransferAlgorithm:MultipleSurface",
  "SurfaceProperty:HeatTransferAlgorithm:SurfaceList",
  "SurfaceProperty:HeatTransferAlgorithm:Construction",
  "SurfaceProperty:HeatBalanceSourceTerm",
  "SurfaceControl:MovableInsulation",
  "SurfaceProperty:OtherSideCoefficients",
  "SurfaceProperty:OtherSideConditionsModel",
  "SurfaceProperty:Underwater",
  "Foundation:Kiva",
  "Foundation:Kiva:Settings",
  "SurfaceProperty:ExposedFoundationPerimeter",
  "SurfaceConvectionAlgorithm:Inside:AdaptiveModelSelections",
  "SurfaceConvectionAlgorithm:Outside:AdaptiveModelSelections",
  "SurfaceConvectionAlgorithm:Inside:UserCurve",
  "SurfaceConvectionAlgorithm:Outside:UserCurve",
  "SurfaceProperty:ConvectionCoefficients",
  "SurfaceProperty:ConvectionCoefficients:MultipleSurface",
  "SurfaceProperties:VaporCoefficients",
  "SurfaceProperty:ExteriorNaturalVentedCavity",
  "SurfaceProperty:SolarIncidentInside",
  "SurfaceProperty:LocalEnvironment",
  "ZoneProperty:LocalEnvironment",
  "SurfaceProperty:SurroundingSurfaces",
  "ComplexFenestrationProperty:SolarAbsorbedLayers",
  "ZoneProperty:UserViewFactors:bySurfaceName",
  "GroundHeatTransfer:Control",
  "GroundHeatTransfer:Slab:Materials",
  "GroundHeatTransfer:Slab:MatlProps",
  "GroundHeatTransfer:Slab:BoundConds",
  "GroundHeatTransfer:Slab:BldgProps",
  "GroundHeatTransfer:Slab:Insulation",
  "GroundHeatTransfer:Slab:EquivalentSlab",
  "GroundHeatTransfer:Slab:AutoGrid",
  "GroundHeatTransfer:Slab:ManualGrid",
  "GroundHeatTransfer:Slab:XFACE",
  "GroundHeatTransfer:Slab:YFACE",
  "GroundHeatTransfer:Slab:ZFACE",
  "GroundHeatTransfer:Basement:SimParameters",
  "GroundHeatTransfer:Basement:MatlProps",
  "GroundHeatTransfer:Basement:Insulation",
  "GroundHeatTransfer:Basement:SurfaceProps",
  "GroundHeatTransfer:Basement:BldgData",
  "GroundHeatTransfer:Basement:Interior",
  "GroundHeatTransfer:Basement:ComBldg",
  "GroundHeatTransfer:Basement:EquivSlab",
  "GroundHeatTransfer:Basement:EquivAutoGrid",
  "GroundHeatTransfer:Basement:AutoGrid",
  "GroundHeatTransfer:Basement:ManualGrid",
  "GroundHeatTransfer:Basement:XFACE",
  "GroundHeatTransfer:Basement:YFACE",
  "GroundHeatTransfer:Basement:ZFACE",
  "People",
  "ComfortViewFactorAngles",
  "Lights",
  "ElectricEquipment",
  "GasEquipment",
  "HotWaterEquipment",
  "SteamEquipment",
  "OtherEquipment",
  "Daylighting:Controls",
  "Daylighting:ReferencePoint",
  "Daylighting:DELight:ComplexFenestration",
  "DaylightingDevice:Tubular",
  "DaylightingDevice:Shelf",
  "DaylightingDevice:LightWell",
  "Output:DaylightFactors",
  "Output:IlluminanceMap",
  "OutputControl:IlluminanceMap:Style",
  "Exterior:Lights",
  "Exterior:FuelEquipment",
  "Exterior:WaterEquipment",
  "EnergyManagementSystem:Sensor",
  "EnergyManagementSystem:Actuator",
  "EnergyManagementSystem:ProgramCallingManager",
  "EnergyManagementSystem:Program",
  "EnergyManagementSystem:Subroutine",
  "EnergyManagementSystem:GlobalVariable",
  "EnergyManagementSystem:OutputVariable",
  "EnergyManagementSystem:MeteredOutputVariable",
  "EnergyManagementSystem:TrendVariable",
  "EnergyManagementSystem:InternalVariable",
  "EnergyManagementSystem:CurveOrTableIndexVariable",
  "EnergyManagementSystem:ConstructionIndexVariable",
  "Refrigeration:Case",
  "Refrigeration:CompressorRack",
  "Refrigeration:CaseAndWalkInList",
  "Refrigeration:Condenser:AirCooled",
  "Refrigeration:Condenser:EvaporativeCooled",
  "Refrigeration:Condenser:WaterCooled",
  "Refrigeration:Condenser:Cascade",
  "Refrigeration:GasCooler:AirCooled",
  "Refrigeration:TransferLoadList",
  "Refrigeration:Subcooler",
  "Refrigeration:Compressor",
  "Refrigeration:CompressorList",
  "Refrigeration:System",
  "Refrigeration:TranscriticalSystem",
  "Refrigeration:SecondarySystem",
  "Refrigeration:WalkIn",
  "Refrigeration:AirChiller",
  "Matrix:TwoDimension",
  "Curve:Linear",
  "Curve:QuadLinear",
  "Curve:Quadratic",
  "Curve:Cubic",
  "Curve:Quartic",
  "Curve:Exponent",
  "Curve:Bicubic",
  "Curve:Biquadratic",
  "Curve:QuadraticLinear",
  "Curve:CubicLinear",
  "Curve:Triquadratic",
  "Curve:Functional:PressureDrop",
  "Curve:FanPressureRise",
  "Curve:ExponentialSkewNormal",
  "Curve:Sigmoid",
  "Curve:RectangularHyperbola1",
  "Curve:RectangularHyperbola2",
  "Curve:ExponentialDecay",
  "Curve:DoubleExponentialDecay",
  "Curve:ChillerPartLoadWithLift",
  "Table:IndependentVariable",
  "Table:IndependentVariableList",
  "Table:Lookup",
  "FluidProperties:Name",
  "FluidProperties:GlycolConcentration",
  "FluidProperties:Temperatures",
  "FluidProperties:Saturated",
  "FluidProperties:Superheated",
  "FluidProperties:Concentration",
  "CurrencyType",
  "ComponentCost:Adjustments",
  "ComponentCost:Reference",
  "ComponentCost:LineItem",
  "UtilityCost:Tariff",
  "UtilityCost:Qualify",
  "UtilityCost:Charge:Simple",
  "UtilityCost:Charge:Block",
  "UtilityCost:Ratchet",
  "UtilityCost:Variable",
  "UtilityCost:Computation",
  "LifeCycleCost:Parameters",
  "LifeCycleCost:RecurringCosts",
  "LifeCycleCost:NonrecurringCost",
  "LifeCycleCost:UsePriceEscalation",
  "LifeCycleCost:UseAdjustment",
  "Parametric:SetValueForRun",
  "Parametric:Logic",
  "Parametric:RunControl",
  "Parametric:FileNameSuffix",
  "Output:VariableDictionary",
  "Output:Surfaces:List",
  "Output:Surfaces:Drawing",
  "Output:Schedules",
  "Output:Constructions",
  "Output:EnergyManagementSystem",
  "OutputControl:SurfaceColorScheme",
  "Output:Table:SummaryReports",
  "Output:Table:TimeBins",
  "Output:Table:Monthly",
  "Output:Table:Annual",
  "OutputControl:Table:Style",
  "OutputControl:ReportingTolerances",
  "Output:Variable",
  "Output:Meter",
  "Output:Meter:MeterFileOnly",
  "Output:Meter:Cumulative",
  "Output:Meter:Cumulative:MeterFileOnly",
  "Meter:Custom",
  "Meter:CustomDecrement",
  "Output:JSON",
  "Output:SQLite",
  "Output:EnvironmentalImpactFactors",
  "EnvironmentalImpactFactors",
  "FuelFactors",
  "Output:Diagnostics",
  "Output:DebuggingData",
  "Output:PreprocessorMessage"
};

// Take json idf as input, strip unused objects
// and return idf text as a string
// This will remove objects related to HVAC and controls
std::string removeUnusedObjects(const json &jsonidf) {
  json newjsonidf;

  for(const auto & type : supportedIDDTypes) {
    if ( jsonidf.find(type) != jsonidf.end() ) {
      newjsonidf[type] = jsonidf[type];
    }
  }

  IdfParser parser;
  const auto embeddedEpJSONSchema = EnergyPlus::EmbeddedEpJSONSchema::embeddedEpJSONSchema();
  json schema = json::from_cbor(embeddedEpJSONSchema.first, embeddedEpJSONSchema.second);
  return parser.encode(newjsonidf, schema);
}

int createFMU(const std::string &jsoninput, bool nozip, bool nocompress) {
  json j;

  std::ifstream fileinput(jsoninput);
  if (!fileinput.fail()) {
    // deserialize from file
    fileinput >> j;
  } else {
    // Try to parse command line input as json string
    j = json::parse(jsoninput, nullptr, false);
  }

  json idf;
  json idd;
  json weather;
  json zones;
  json fmuname;

  if (j.is_discarded()) {
    std::cout << "Cannot parse json: '" << jsoninput << "'" << std::endl;
  } else {
    try {
      idf = j.at("EnergyPlus").at("idf");
      idd = j.at("EnergyPlus").at("idd");
      weather = j.at("EnergyPlus").at("weather");
      zones = j.at("model").at("zones");
      fmuname = j.at("fmu").at("name");
    } catch (...) {
      std::cout << "Invalid json input: '" << jsoninput << "'" << std::endl;
      return 1;
    }
  }

  // Input paths
  auto spawnInputPath = boost::filesystem::canonical(boost::filesystem::path(jsoninput));
  auto basepath = spawnInputPath.parent_path();
  auto fmupath = boost::filesystem::path(fmuname.get<std::string>());
  if (! fmupath.is_absolute()) {
    fmupath = basepath / fmupath;
  }
  auto idfInputPath = boost::filesystem::path(idf.get<std::string>());
  if (! idfInputPath.is_absolute()) {
    idfInputPath = basepath / idfInputPath;
  }
  auto epwInputPath = boost::filesystem::path(weather.get<std::string>());
  if (! epwInputPath.is_absolute()) {
    epwInputPath = basepath / epwInputPath;
  }
  auto iddInputPath = boost::filesystem::path(idd.get<std::string>());
  if (! iddInputPath.is_absolute()) {
    iddInputPath = basepath / iddInputPath;
  }

  // Do the input paths exist?
  bool missingFile = false;
  if (! boost::filesystem::exists(idfInputPath)) {
    std::cout << "The specified idf input file does not exist, " << idfInputPath << "." << std::endl;
    missingFile = true;
  }
  if (! boost::filesystem::exists(epwInputPath)) {
    std::cout << "The specified epw input file does not exist, " << epwInputPath << "." << std::endl;
    missingFile = true;
  }
  if (! boost::filesystem::exists(iddInputPath)) {
    std::cout << "The specified idd input file does not exist, " << iddInputPath << "." << std::endl;
    missingFile = true;
  }

  if (missingFile) {
    return 1;
  }

  // Output paths
  auto fmuStaggingPath = fmupath.parent_path() / fmupath.stem();
  auto modelDescriptionPath = fmuStaggingPath / "modelDescription.xml";
  auto resourcesPath = fmuStaggingPath / "resources";
  auto idfPath = resourcesPath / idfInputPath.filename();
  auto epwPath = resourcesPath / epwInputPath.filename();
  auto iddPath = resourcesPath / iddInputPath.filename();
  auto spawnPath = resourcesPath / spawnInputPath.filename();

  boost::filesystem::path epFMIDestPath;
  boost::filesystem::path epFMISourcePath;

  boost::filesystem::remove(fmupath);

  #ifdef __APPLE__
    Dl_info info;
    dladdr("main", &info);
    auto exedir = boost::filesystem::path(info.dli_fname).parent_path();
    epFMISourcePath = exedir / "../lib/libepfmi.dylib";
    if (! boost::filesystem::exists(epFMISourcePath)) {
      epFMISourcePath = exedir / "libepfmi.dylib";
    }
    epFMIDestPath = fmuStaggingPath / "binaries/darwin64/libepfmi.dylib";
  #elif _WIN32
    TCHAR szPath[MAX_PATH];
    GetModuleFileName(nullptr, szPath, MAX_PATH);
    auto exedir = boost::filesystem::path(szPath).parent_path();
    epFMISourcePath = exedir / "epfmi.dll";
    epFMIDestPath = fmuStaggingPath / "binaries/win64/epfmi.dll";
  #else
    Dl_info info;
    dladdr("main", &info);
    auto exedir = boost::filesystem::path(info.dli_fname).parent_path();
    epFMISourcePath = exedir / "../lib/libepfmi.so";
    if (! boost::filesystem::exists(epFMISourcePath)) {
      epFMISourcePath = exedir / "libepfmi.so";
    }
    epFMIDestPath = fmuStaggingPath / "binaries/linux64/libepfmi.so";
  #endif

  // Create fmu staging area and copy files into it
  boost::filesystem::create_directories(fmuStaggingPath);
  boost::filesystem::create_directories(resourcesPath);
  boost::filesystem::create_directories(epFMIDestPath.parent_path());

  boost::filesystem::copy_file(epFMISourcePath, epFMIDestPath, boost::filesystem::copy_option::overwrite_if_exists);
  //boost::filesystem::copy_file(idfInputPath, idfPath, boost::filesystem::copy_option::overwrite_if_exists);
  boost::filesystem::copy_file(iddInputPath, iddPath, boost::filesystem::copy_option::overwrite_if_exists);
  boost::filesystem::copy_file(epwInputPath, epwPath, boost::filesystem::copy_option::overwrite_if_exists);
  boost::filesystem::copy_file(spawnInputPath, spawnPath, boost::filesystem::copy_option::overwrite_if_exists);

  // Make a copy of the idf input file, but remove unused objects
  std::ifstream input_stream(idfInputPath.string(), std::ifstream::in);
  std::string input_file;
  std::string line;
  while (std::getline(input_stream, line)) {
    input_file.append(line + EnergyPlus::DataStringGlobals::NL);
  }
  IdfParser parser;
  const auto embeddedEpJSONSchema = EnergyPlus::EmbeddedEpJSONSchema::embeddedEpJSONSchema();
  json schema = json::from_cbor(embeddedEpJSONSchema.first, embeddedEpJSONSchema.second);
  const auto & jsonidf = parser.decode(input_file, schema);
  const auto & newidftext = removeUnusedObjects(jsonidf);
  std::ofstream newidfstream(idfPath.string(),  std::ofstream::out |  std::ofstream::trunc);
  newidfstream << newidftext;
  newidfstream.close();

  // Create the modelDescription.xml file
  pugi::xml_document doc;
  doc.load_string(modelDescriptionXMLText.c_str());

  auto xmlvariables = doc.child("fmiModelDescription").child("ModelVariables");

  const auto epvariables = parseVariables(idfPath.string(),spawnInputPath.string());

  for (const auto & varpair : epvariables) {
    const auto var = varpair.second;

    auto scalarVar = xmlvariables.append_child("ScalarVariable");
		for (const auto & attribute : var.scalar_attributes) {
    	scalarVar.append_attribute(attribute.first.c_str()) = attribute.second.c_str();
		}

    auto real = scalarVar.append_child("Real");
		for (const auto & attribute : var.real_attributes) {
    	real.append_attribute(attribute.first.c_str()) = attribute.second.c_str();
		}
  }

  doc.save_file(modelDescriptionPath.c_str());

  if (! nozip) {
    zip_directory(fmuStaggingPath.string(), fmupath.string(), nocompress);
  }

  return 0;
}

int main(int argc, const char *argv[]) {
  CLI::App app{"Spawn of EnergyPlus"};

  bool nozip = false;
  bool nocompress = false;

  std::string jsoninput = "spawn.json";
  auto createOption =
      app.add_option("-c,--create", jsoninput,
                     "Create a standalone FMU based on json input", true);

  auto zipOption = app.add_flag("--no-zip", nozip, "Stage FMU files on disk without creating a zip archive");
  zipOption->needs(createOption);

  auto compressOption = app.add_flag("--no-compress", nocompress, "Skip compressing the contents of the fmu zip archive. An uncompressed zip archive will be created instead.");
  compressOption->needs(createOption);

  auto versionOption =
    app.add_flag("-v,--version", "Print version info and exit");

  CLI11_PARSE(app, argc, argv);

  if (*createOption) {
    auto result = createFMU(jsoninput, nozip, nocompress);
    if (result) {
      return result;
    }
  }

  if (*versionOption) {
    std::cout << "Spawn-" << spawn::VERSION_STRING << std::endl;
  }

  return 0;
}

