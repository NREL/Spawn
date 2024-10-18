#include "energyplus_helpers.hpp"
#include "util/conversion.hpp"
#include <DataEnvironment.hh>
#include <DataGlobals.hh>
#include <DataHeatBalSurface.hh>
#include <DataHeatBalance.hh>
#include <DataSizing.hh>
#include <EnergyPlusData.hh>
#include <InternalHeatGains.hh>
#include <Psychrometrics.hh>
#include <ZoneTempPredictorCorrector.hh>
#include <api/datatransfer.h>
#include <string_view>

namespace spawn::energyplus {

ZoneSums::ZoneSums(EnergyPlus::EnergyPlusData &energyplus_data, int zone_num)
{
  auto &zone_heat_balance = energyplus_data.dataZoneTempPredictorCorrector->zoneHeatBalance(zone_num);
  zone_heat_balance.calcZoneOrSpaceSums(energyplus_data, true, zone_num);

  temp_dep_coef_ = zone_heat_balance.SumHA + zone_heat_balance.SumMCp;
  temp_ind_coef_ = zone_heat_balance.SumIntGain + zone_heat_balance.SumHATsurf + zone_heat_balance.SumMCpT;

  // Refer to
  // https://bigladdersoftware.com/epx/docs/8-8/engineering-reference/basis-for-the-zone-and-air-system-integration.html#basis-for-the-zone-and-air-system-integration
  q_con_sen_flow_ = temp_ind_coef_ - (temp_dep_coef_ * zone_heat_balance.MAT);
}

double ZoneSums::TempDepCoef() const
{
  return temp_dep_coef_;
}

double ZoneSums::TempIndCoef() const
{
  return temp_ind_coef_;
}

double ZoneSums::QConSenFlow() const
{
  return q_con_sen_flow_;
}

int ZoneNum(EnergyPlus::EnergyPlusData &energyplus_data, const std::string_view zone_name)
{
  std::string upper_zone_name(zone_name);
  std::transform(zone_name.begin(), zone_name.end(), upper_zone_name.begin(), ::toupper);

  for (int i = 0; i < energyplus_data.dataGlobal->NumOfZones; ++i) {
    if (energyplus_data.dataHeatBal->Zone[as_size_t(i)].Name == upper_zone_name) {
      return i + 1;
    }
  }

  return 0;
}

[[nodiscard]] std::vector<int> ZoneNums(EnergyPlus::EnergyPlusData &energyplus_data,
                                        const std::vector<std::string> &zone_names)
{
  std::vector<int> result(zone_names.size());

  std::transform(zone_names.begin(), zone_names.end(), result.begin(), [&energyplus_data](const auto &name) {
    return ZoneNum(energyplus_data, name);
  });

  return result;
}

[[nodiscard]] double ZoneVolume(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num)
{
  return energyplus_data.dataHeatBal->Zone(zone_num).Volume;
}

[[nodiscard]] double ZoneFloorArea(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num)
{
  return energyplus_data.dataHeatBal->Zone(zone_num).FloorArea;
}

[[nodiscard]] double ZoneVolCapMultpSens(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num)
{
  return energyplus_data.dataHeatBal->Zone(zone_num).ZoneVolCapMultpSens;
}

[[nodiscard]] double ZoneLatentGain(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num)
{
  return energyplus_data.dataZoneTempPredictorCorrector->zoneHeatBalance(zone_num).latentGain;
}

[[nodiscard]] double ZoneMeanRadiantTemp([[maybe_unused]] const EnergyPlus::EnergyPlusData &energyplus_data,
                                         [[maybe_unused]] int zone_num)
{
  return energyplus_data.dataZoneTempPredictorCorrector->zoneHeatBalance(zone_num).MRT;
}

bool HaveSizingInfo(const EnergyPlus::EnergyPlusData &energyplus_data)
{
  return !energyplus_data.dataSize->FinalZoneSizing.empty();
}

namespace zone_sizing {

  [[nodiscard]] double SensibleCoolingLoad(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num)
  {
    if (!energyplus_data.dataSize->FinalZoneSizing.empty()) {
      return energyplus_data.dataSize->FinalZoneSizing(zone_num).DesCoolLoad;
    }

    return 0.0;
  }

  [[nodiscard]] double LatentCoolingLoad(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num)
  {
    if (!energyplus_data.dataSize->FinalZoneSizing.empty()) {
      return energyplus_data.dataSize->FinalZoneSizing(zone_num).DesLatentCoolLoad;
    }

    return 0.0;
  }

  [[nodiscard]] double OutdoorTempAtPeakCool(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num)
  {
    if (!energyplus_data.dataSize->FinalZoneSizing.empty()) {
      return energyplus_data.dataSize->FinalZoneSizing(zone_num).OutTempAtCoolPeak;
    }

    return 0.0;
  }

  [[nodiscard]] double OutdoorHumidityRatioAtPeakCool(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num)
  {
    if (!energyplus_data.dataSize->FinalZoneSizing.empty()) {
      return energyplus_data.dataSize->FinalZoneSizing(zone_num).OutHumRatAtCoolPeak;
    }

    return 0.0;
  }

  [[nodiscard]] double TimeAtPeakCool(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num)
  {
    if (!energyplus_data.dataSize->FinalZoneSizing.empty()) {
      return energyplus_data.dataSize->FinalZoneSizing(zone_num).TimeStepNumAtCoolMax *
             energyplus_data.dataGlobal->TimeStepZoneSec;
    }

    return 0.0;
  }

  [[nodiscard]] double HeatingLoad(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num)
  {
    if (!energyplus_data.dataSize->FinalZoneSizing.empty()) {
      return energyplus_data.dataSize->FinalZoneSizing(zone_num).DesHeatLoad;
    }

    return 0.0;
  }

  [[nodiscard]] double OutdoorTempAtPeakHeat(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num)
  {
    if (!energyplus_data.dataSize->FinalZoneSizing.empty()) {
      return energyplus_data.dataSize->FinalZoneSizing(zone_num).OutTempAtHeatPeak;
    }

    return 0.0;
  }

  [[nodiscard]] double OutdoorHumidityRatioAtPeakHeat(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num)
  {
    if (!energyplus_data.dataSize->FinalZoneSizing.empty()) {
      return energyplus_data.dataSize->FinalZoneSizing(zone_num).OutHumRatAtHeatPeak;
    }

    return 0.0;
  }

  [[nodiscard]] double TimeAtPeakHeat(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num)
  {
    if (!energyplus_data.dataSize->FinalZoneSizing.empty()) {
      return energyplus_data.dataSize->FinalZoneSizing(zone_num).TimeStepNumAtHeatMax *
             energyplus_data.dataGlobal->TimeStepZoneSec;
    }

    return 0.0;
  }

} // namespace zone_sizing

namespace zone_group_sizing {

  struct PeakLoad
  {
    const double value;
    const int day_timestep;
    const int design_day_index;
  };

  // Instances of this function type will receive ZoneSizingData and return a sequence of loads for each timestep within
  // a design day.
  using GetLoadSeqFunc = std::function<Array1D<Real64>(const EnergyPlus::DataSizing::ZoneSizingData &)>;

  // The purpose of this function is to find the design day and timestep that has the highest combined load for the
  // given zones. This function is generic. The get_load_seq function defines what type of peak load (Sensible Cooling |
  // Heating) to locate.
  [[nodiscard]] PeakLoad GetPeakLoad(const EnergyPlus::EnergyPlusData &energyplus_data,
                                     const std::vector<int> &zone_nums,
                                     const GetLoadSeqFunc &get_load_seq)
  {
    if (!HaveSizingInfo(energyplus_data)) {
      return {0.0, 0, 0};
    }

    const auto &zone_sizing = energyplus_data.dataSize->ZoneSizing;
    const auto num_design_days = zone_sizing.isize1();
    std::vector<PeakLoad> peak_loads;

    for (int design_day_index = 0; design_day_index <= num_design_days; ++design_day_index) {
      const auto num_timesteps = static_cast<int>(get_load_seq(zone_sizing(design_day_index, 0)).size());
      std::vector<double> combined_group_load(num_timesteps);

      for (const auto &zone_num : zone_nums) {
        const auto load_seq = get_load_seq(zone_sizing(design_day_index, zone_num));
        for (int i = 0; i < num_timesteps; ++i) {
          combined_group_load[i] += load_seq[i];
        }
      }

      const auto peak_value = std::max_element(combined_group_load.begin(), combined_group_load.end());
      const auto timestep_of_peak = static_cast<int>(std::distance(combined_group_load.begin(), peak_value) + 1);

      // Store the PeakLoad for this design day
      peak_loads.push_back(PeakLoad({*peak_value, timestep_of_peak, design_day_index}));
    }

    // Find the PeakLoad across all design days
    const auto gloabl_peak_load = std::max_element(
        peak_loads.begin(), peak_loads.end(), [](const PeakLoad &a, const PeakLoad &b) { return a.value < b.value; });

    return *gloabl_peak_load;
  }

  [[nodiscard]] double SensibleCoolingLoad(const EnergyPlus::EnergyPlusData &energyplus_data,
                                           const std::vector<int> &zone_nums)
  {
    const auto get_cooling_load_seq = [](const EnergyPlus::DataSizing::ZoneSizingData &sizing_data) {
      return sizing_data.CoolLoadSeq;
    };

    return GetPeakLoad(energyplus_data, zone_nums, get_cooling_load_seq).value;
  }

  [[nodiscard]] double LatentCoolingLoad(const EnergyPlus::EnergyPlusData &energyplus_data,
                                         const std::vector<int> &zone_nums)
  {
    const auto get_cooling_load_seq = [](const EnergyPlus::DataSizing::ZoneSizingData &sizing_data) {
      return sizing_data.LatentCoolLoadSeq;
    };

    // This value may not be coincident with the peak sensible load. Is this what we want, or
    // should we return the latent load at the time of the peak sensible load?
    return GetPeakLoad(energyplus_data, zone_nums, get_cooling_load_seq).value;
  }

  [[nodiscard]] double OutdoorTempAtPeakCool(const EnergyPlus::EnergyPlusData &energyplus_data,
                                             [[maybe_unused]] const std::vector<int> &zone_nums)
  {
    const auto get_cooling_load_seq = [](const EnergyPlus::DataSizing::ZoneSizingData &sizing_data) {
      return sizing_data.CoolLoadSeq;
    };

    const auto peak_load = GetPeakLoad(energyplus_data, zone_nums, get_cooling_load_seq);
    // The outdoor temperature should be the same for all zones, so get the sizing data for the
    // peak design day, using any one of the zones in the group.
    const auto zone_sizing = energyplus_data.dataSize->ZoneSizing(peak_load.design_day_index, zone_nums.front());

    return zone_sizing.CoolOutTempSeq(peak_load.day_timestep);
  }

} // namespace zone_group_sizing

void SetZoneTemperature(EnergyPlus::EnergyPlusData &energyplus_data, const int zone_num, const double &temp)
{
  auto &zone_heat_balance = energyplus_data.dataZoneTempPredictorCorrector->zoneHeatBalance(zone_num);
  // Is it necessary to update all of these or can we
  // simply update ZT and count on EnergyPlus::HeatBalanceAirManager::ReportZoneMeanAirTemp()
  // to propogate the other variables?
  zone_heat_balance.ZT = temp;
  zone_heat_balance.ZTAV = temp;
  zone_heat_balance.MAT = temp;

  auto &zone = energyplus_data.dataHeatBal->Zone(zone_num);
  for (int space_num : zone.spaceIndexes) {
    auto &space_heat_balance = energyplus_data.dataZoneTempPredictorCorrector->spaceHeatBalance(space_num);
    space_heat_balance.ZT = temp;
    space_heat_balance.ZTAV = temp;
    space_heat_balance.MAT = temp;
  }
}

double ZoneTemperature(EnergyPlus::EnergyPlusData &energyplus_data, const int zone_num)
{
  auto &zone_heat_balance = energyplus_data.dataZoneTempPredictorCorrector->zoneHeatBalance(zone_num);
  return zone_heat_balance.ZT;
}

void SetZoneHumidityRatio(EnergyPlus::EnergyPlusData &energyplus_data, const int zone_num, const double &ratio)
{
  auto &zone_heat_balance = energyplus_data.dataZoneTempPredictorCorrector->zoneHeatBalance(zone_num);

  zone_heat_balance.airHumRatAvg = ratio;
  zone_heat_balance.airHumRat = ratio;
  zone_heat_balance.airHumRatTemp = ratio;

  auto &zone = energyplus_data.dataHeatBal->Zone(zone_num);
  for (int space_num : zone.spaceIndexes) {
    auto &space_heat_balance = energyplus_data.dataZoneTempPredictorCorrector->spaceHeatBalance(space_num);
    space_heat_balance.airHumRatAvg = ratio;
    space_heat_balance.airHumRat = ratio;
    space_heat_balance.airHumRatTemp = ratio;
  }
}

double ZoneHumidityRatio(EnergyPlus::EnergyPlusData &energyplus_data, const int zone_num)
{
  auto &zone_heat_balance = energyplus_data.dataZoneTempPredictorCorrector->zoneHeatBalance(zone_num);
  return zone_heat_balance.airHumRatAvg;
}

[[nodiscard]] int
VariableHandle(EnergyPlus::EnergyPlusData &energyplus_data, const std::string_view name, const std::string_view key)
{
  const auto h = ::getVariableHandle(
      static_cast<EnergyPlusState>(&energyplus_data), std::string(name).c_str(), std::string(key).c_str());
  if (h == -1) {
    throw std::runtime_error(fmt::format("Attempt to get invalid variable using name '{}', and key '{}'", name, key));
  }

  return h;
}

[[nodiscard]] double VariableValue(EnergyPlus::EnergyPlusData &energyplus_data, int handle)
{
  return ::getVariableValue(static_cast<EnergyPlusState>(&energyplus_data), handle);
}

int ActuatorHandle(EnergyPlus::EnergyPlusData &energyplus_data,
                   const std::string &component_type,
                   const std::string &control_type,
                   const std::string &component_name)
{
  // Uses the EnergyPlus api getActuatorHandle, but throws if the actuator does not exist
  const auto h = ::getActuatorHandle(static_cast<EnergyPlusState>(&energyplus_data),
                                     component_type.c_str(),
                                     control_type.c_str(),
                                     component_name.c_str());
  if (h == -1) {
    throw std::runtime_error(fmt::format(
        "Attempt to get invalid actuator using component type '{}', component name '{}', and control type {}",
        component_type,
        component_name,
        control_type));
  }

  return h;
}

void SetActuatorValue(EnergyPlus::EnergyPlusData &energyplus_data, int handle, const double &value)
{
  ::setActuatorValue(static_cast<EnergyPlusState>(&energyplus_data), handle, value);
}

void ResetActuator(EnergyPlus::EnergyPlusData &energyplus_data, int handle)
{
  ::resetActuator(&energyplus_data, handle);
}

[[nodiscard]] int SurfaceNum(EnergyPlus::EnergyPlusData &energyplus_data, std::string_view surface_name)
{
  std::string upper_name(surface_name);

  std::transform(surface_name.begin(), surface_name.end(), upper_name.begin(), ::toupper);
  for (const auto i : energyplus_data.dataSurface->AllHTNonWindowSurfaceList) {
    if (energyplus_data.dataSurface->Surface[as_size_t(i)].Name == upper_name) {
      return i + 1;
    }
  }

  return 0;
}

[[nodiscard]] double SurfaceArea(EnergyPlus::EnergyPlusData &energyplus_data, int surface_num)
{
  return energyplus_data.dataSurface->Surface(surface_num).GrossArea;
}

[[nodiscard]] double SurfaceInsideHeatFlow(EnergyPlus::EnergyPlusData &energyplus_data, int surface_num)
{
  return energyplus_data.dataHeatBalSurf->SurfQdotConvInRep(surface_num) +
         energyplus_data.dataHeatBalSurf->SurfQdotRadNetSurfInRep(surface_num);
}

[[nodiscard]] double SurfaceOutsideHeatFlow(EnergyPlus::EnergyPlusData &energyplus_data, int surface_num)
{
  const auto &surface = energyplus_data.dataSurface->Surface(as_size_t(surface_num));
  const auto &ext_bound_cond = surface.ExtBoundCond;
  if (ext_bound_cond > 0) {
    // EnergyPlus does not calculate the surface heat flux for interzone surfaces,
    // instead return the inside face heat flux of the matching surface
    return energyplus_data.dataHeatBalSurf->SurfQdotConvInRep(ext_bound_cond) +
           energyplus_data.dataHeatBalSurf->SurfQdotRadNetSurfInRep(ext_bound_cond);
  } else {
    return energyplus_data.dataHeatBalSurf->SurfQdotConvOutRep(surface_num) +
           energyplus_data.dataHeatBalSurf->SurfQdotRadOutRep(surface_num);
  }
}

void SetInsideSurfaceTemperature(EnergyPlus::EnergyPlusData &energyplus_data, const int surface_num, double temp)
{
  const auto &surface = energyplus_data.dataSurface->Surface(as_size_t(surface_num));
  const auto inside_actuator_handle =
      ActuatorHandle(energyplus_data, "Surface", "Surface Inside Temperature", surface.Name);
  SetActuatorValue(energyplus_data, inside_actuator_handle, temp);
  auto &extBoundCond = surface.ExtBoundCond;
  if (extBoundCond > 0) {
    // If this is an interzone surface then set the outside of the matching surface
    auto &other_surface = energyplus_data.dataSurface->Surface(as_size_t(extBoundCond));
    const auto outside_actuator_handle =
        ActuatorHandle(energyplus_data, "Surface", "Surface Outside Temperature", other_surface.Name);
    SetActuatorValue(energyplus_data, outside_actuator_handle, temp);
  }
}
void SetOutsideSurfaceTemperature(EnergyPlus::EnergyPlusData &energyplus_data, const int surface_num, double temp)
{
  const auto &surface = energyplus_data.dataSurface->Surface(as_size_t(surface_num));
  const auto outside_actuator_handle =
      ActuatorHandle(energyplus_data, "Surface", "Surface Outside Temperature", surface.Name);
  SetActuatorValue(energyplus_data, outside_actuator_handle, temp);

  const auto &extBoundCond = surface.ExtBoundCond;

  if (surface_num == extBoundCond) {
    throw std::runtime_error(fmt::format("Attempt to control surface named {} that has a self referencing exterior "
                                         "boundary condition. This is not supported by Spawn",
                                         surface.Name));
  }

  if (extBoundCond > 0) {
    // If this is an interzone surface then set the inside of the matching surface
    auto &other_surface = energyplus_data.dataSurface->Surface(as_size_t(extBoundCond));
    const auto inside_actuator_handle =
        ActuatorHandle(energyplus_data, "Surface", "Surface Inside Temperature", other_surface.Name);
    SetActuatorValue(energyplus_data, inside_actuator_handle, temp);
  }
}

void UpdateZoneTemperature(EnergyPlus::EnergyPlusData &energyplus_data, const int zonenum, const double dt)
{
  auto &zone_heat_balance = energyplus_data.dataZoneTempPredictorCorrector->zoneHeatBalance(zonenum);

  // Based on the EnergyPlus analytical method
  // See ZoneTempPredictorCorrector::CorrectZoneAirTemp
  const auto &zonetemp = ZoneTemperature(energyplus_data, zonenum);
  double newzonetemp = 0;

  const auto aircap =
      energyplus_data.dataHeatBal->Zone(as_size_t(zonenum)).Volume *
      energyplus_data.dataHeatBal->Zone(as_size_t(zonenum)).ZoneVolCapMultpSens *
      EnergyPlus::Psychrometrics::PsyRhoAirFnPbTdbW(energyplus_data,
                                                    energyplus_data.dataEnvrn->OutBaroPress,
                                                    zonetemp,
                                                    zone_heat_balance.airHumRat,
                                                    "") *
      EnergyPlus::Psychrometrics::PsyCpAirFnW(zone_heat_balance.airHumRat); // / (TimeStepSys * SecInHour);

  const auto &sums = ZoneSums(energyplus_data, zonenum);
  if (sums.TempDepCoef() == 0.0) { // B=0
    newzonetemp = zonetemp + sums.TempIndCoef() / aircap * dt;
  } else {
    newzonetemp =
        (zonetemp - sums.TempIndCoef() / sums.TempDepCoef()) * std::exp(min(700.0, -sums.TempDepCoef() / aircap * dt)) +
        sums.TempIndCoef() / sums.TempDepCoef();
  }

  SetZoneTemperature(energyplus_data, zonenum, newzonetemp);
}

void UpdateZoneHumidityRatio(EnergyPlus::EnergyPlusData &energyplus_data, const int zonenum, const double dt)
{
  auto &zone_heat_balance = energyplus_data.dataZoneTempPredictorCorrector->zoneHeatBalance(zonenum);

  // Based on the EnergyPlus analytical method
  // See ZoneTempPredictorCorrector::CorrectZoneHumRat
  static constexpr std::string_view RoutineName("updateZoneHumidityRatio");
  const auto humidityRatio = energyplus::ZoneHumidityRatio(energyplus_data, zonenum);

  auto &ZT = zone_heat_balance.ZT;
  auto &Zone = energyplus_data.dataHeatBal->Zone;
  double moistureMassFlowRate = 0.0;

  // Calculate hourly humidity ratio from infiltration + humdidity added from latent load + system added moisture
  const auto latentGain = zone_heat_balance.latentGain + energyplus_data.dataHeatBalFanSys->SumLatentHTRadSys(zonenum) +
                          energyplus_data.dataHeatBalFanSys->SumLatentPool(zonenum);

  const double RhoAir = EnergyPlus::Psychrometrics::PsyRhoAirFnPbTdbW(
      energyplus_data, energyplus_data.dataEnvrn->OutBaroPress, ZT, zone_heat_balance.airHumRat, RoutineName);
  const double h2oHtOfVap = EnergyPlus::Psychrometrics::PsyHgAirFnWTdb(zone_heat_balance.airHumRat, ZT);

  const double B = (latentGain / h2oHtOfVap) +
                   ((zone_heat_balance.OAMFL + zone_heat_balance.VAMFL + zone_heat_balance.CTMFL) *
                    energyplus_data.dataEnvrn->OutHumRat) +
                   zone_heat_balance.EAMFLxHumRat + (moistureMassFlowRate) + zone_heat_balance.SumHmARaW +
                   zone_heat_balance.MixingMassFlowXHumRat +
                   zone_heat_balance.MDotOA * energyplus_data.dataEnvrn->OutHumRat;

  const double C = RhoAir * Zone(zonenum).Volume * Zone(zonenum).ZoneVolCapMultpMoist / dt;

  double newHumidityRatio = humidityRatio + B / C;

  // Set the humidity ratio to zero if the zone has been dried out
  if (newHumidityRatio < 0.0) {
    newHumidityRatio = 0.0;
  }

  // Check to make sure that is saturated there is condensation in the zone
  // by resetting to saturation conditions.
  const double wzSat = EnergyPlus::Psychrometrics::PsyWFnTdbRhPb(
      energyplus_data, ZT, 1.0, energyplus_data.dataEnvrn->OutBaroPress, RoutineName);

  if (newHumidityRatio > wzSat) {
    newHumidityRatio = wzSat;
  }

  SetZoneHumidityRatio(energyplus_data, zonenum, newHumidityRatio);
}

void UpdateLatentGains(EnergyPlus::EnergyPlusData &energyplus_data)
{

  for (int zonei = 1; zonei <= energyplus_data.dataGlobal->NumOfZones; ++zonei) {
    EnergyPlus::InternalHeatGains::SumAllInternalLatentGains(energyplus_data, zonei);
  }
}

} // namespace spawn::energyplus
