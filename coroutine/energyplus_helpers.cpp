#include "energyplus_helpers.hpp"

// C++ standard library headers
#include <algorithm>
#include <stdexcept>
#include <string_view>

// EnergyPlus headers
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

// Spawn project headers
#include "util/conversion.hpp"

#include "spdlog/spdlog.h"

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

double ZoneSums::temp_dep_coef() const
{
  return temp_dep_coef_;
}

double ZoneSums::temp_ind_coef() const
{
  return temp_ind_coef_;
}

double ZoneSums::q_con_sen_flow() const
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

namespace {

  void LogMissingSizingInfo(const EnergyPlus::EnergyPlusData &energyplus_data,
                            int zone_num,
                            std::string_view context,
                            bool should_log)
  {
    if (!should_log) {
      return;
    }

    if (zone_num <= 0) {
      spdlog::warn("No sizing information available for zone index {} while computing {}", zone_num, context);
      return;
    }

    const auto &final_zone_sizing = energyplus_data.dataSize->FinalZoneSizing;
    const auto zone_count = static_cast<int>(final_zone_sizing.size());
    const bool within_bounds = zone_num <= zone_count;

    std::string zone_name;
    if (within_bounds) {
      try {
        zone_name = energyplus_data.dataHeatBal->Zone(zone_num).Name;
      } catch (const std::out_of_range &) {
        // Leave name empty if lookup fails; logging still proceeds.
      }
    }

    if (!zone_name.empty()) {
      spdlog::warn("No sizing information available for zone {} ({}) while computing {}", zone_num, zone_name, context);
    } else {
      spdlog::warn("No sizing information available for zone {} while computing {}", zone_num, context);
    }
  }

} // namespace

bool HaveSizingInfo(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num)
{
  const auto &final_zone_sizing = energyplus_data.dataSize->FinalZoneSizing;
  if (final_zone_sizing.empty()) {
    return false;
  }

  if (zone_num <= 0) {
    return false;
  }

  const auto zone_count = static_cast<int>(final_zone_sizing.size());
  if (zone_num > zone_count) {
    return false;
  }

  return !final_zone_sizing(zone_num).ZoneName.empty();
}

// this function will filter zone_nums to only those with sizing information
std::vector<int> ZonesWithSizingInfo(const EnergyPlus::EnergyPlusData &energyplus_data,
                                     std::vector<int> zone_nums,
                                     bool log_missing)
{
  std::vector<int> result;

  for (const auto zone_num : zone_nums) {
    if (HaveSizingInfo(energyplus_data, zone_num)) {
      result.push_back(zone_num);
    } else {
      LogMissingSizingInfo(energyplus_data, zone_num, "zone group sizing", log_missing);
    }
  }

  return result;
}

namespace zone_sizing {

  [[nodiscard]] double SensibleCoolingLoad(const EnergyPlus::EnergyPlusData &energyplus_data,
                                           int zone_num,
                                           bool log_missing)
  {
    if (HaveSizingInfo(energyplus_data, zone_num)) {
      return energyplus_data.dataSize->FinalZoneSizing(zone_num).DesCoolLoad;
    }

    LogMissingSizingInfo(energyplus_data, zone_num, "zone_sizing::SensibleCoolingLoad", log_missing);
    return 0.0;
  }

  [[nodiscard]] double LatentCoolingLoad(const EnergyPlus::EnergyPlusData &energyplus_data,
                                         int zone_num,
                                         bool log_missing)
  {
    if (HaveSizingInfo(energyplus_data, zone_num)) {
      return energyplus_data.dataSize->FinalZoneSizing(zone_num).DesLatentCoolLoad;
    }

    LogMissingSizingInfo(energyplus_data, zone_num, "zone_sizing::LatentCoolingLoad", log_missing);
    return 0.0;
  }

  [[nodiscard]] double OutdoorTempAtPeakCool(const EnergyPlus::EnergyPlusData &energyplus_data,
                                             int zone_num,
                                             bool log_missing)
  {
    if (HaveSizingInfo(energyplus_data, zone_num)) {
      return energyplus_data.dataSize->FinalZoneSizing(zone_num).OutTempAtCoolPeak;
    }

    LogMissingSizingInfo(energyplus_data, zone_num, "zone_sizing::OutdoorTempAtPeakCool", log_missing);
    return 0.0;
  }

  [[nodiscard]] double OutdoorHumidityRatioAtPeakCool(const EnergyPlus::EnergyPlusData &energyplus_data,
                                                      int zone_num,
                                                      bool log_missing)
  {
    if (HaveSizingInfo(energyplus_data, zone_num)) {
      return energyplus_data.dataSize->FinalZoneSizing(zone_num).OutHumRatAtCoolPeak;
    }

    LogMissingSizingInfo(energyplus_data, zone_num, "zone_sizing::OutdoorHumidityRatioAtPeakCool", log_missing);
    return 0.0;
  }

  [[nodiscard]] double TimeAtPeakCool(const EnergyPlus::EnergyPlusData &energyplus_data,
                                      int zone_num,
                                      bool log_missing)
  {
    if (HaveSizingInfo(energyplus_data, zone_num)) {
      // TODO: Are we sure this is right when the sizing is from a DesignDay
      return energyplus_data.dataSize->FinalZoneSizing(zone_num).TimeStepNumAtCoolMax *
             energyplus_data.dataGlobal->TimeStepZoneSec;
    }

    LogMissingSizingInfo(energyplus_data, zone_num, "zone_sizing::TimeAtPeakCool", log_missing);
    return 0.0;
  }

  [[nodiscard]] double HeatingLoad(const EnergyPlus::EnergyPlusData &energyplus_data,
                                   int zone_num,
                                   bool log_missing)
  {
    if (HaveSizingInfo(energyplus_data, zone_num)) {
      return energyplus_data.dataSize->FinalZoneSizing(zone_num).DesHeatLoad;
    }

    LogMissingSizingInfo(energyplus_data, zone_num, "zone_sizing::HeatingLoad", log_missing);
    return 0.0;
  }

  [[nodiscard]] double OutdoorTempAtPeakHeat(const EnergyPlus::EnergyPlusData &energyplus_data,
                                             int zone_num,
                                             bool log_missing)
  {
    if (HaveSizingInfo(energyplus_data, zone_num)) {
      return energyplus_data.dataSize->FinalZoneSizing(zone_num).OutTempAtHeatPeak;
    }

    LogMissingSizingInfo(energyplus_data, zone_num, "zone_sizing::OutdoorTempAtPeakHeat", log_missing);
    return 0.0;
  }

  [[nodiscard]] double OutdoorHumidityRatioAtPeakHeat(const EnergyPlus::EnergyPlusData &energyplus_data,
                                                      int zone_num,
                                                      bool log_missing)
  {
    if (HaveSizingInfo(energyplus_data, zone_num)) {
      return energyplus_data.dataSize->FinalZoneSizing(zone_num).OutHumRatAtHeatPeak;
    }

    LogMissingSizingInfo(energyplus_data, zone_num, "zone_sizing::OutdoorHumidityRatioAtPeakHeat", log_missing);
    return 0.0;
  }

  [[nodiscard]] double TimeAtPeakHeat(const EnergyPlus::EnergyPlusData &energyplus_data,
                                      int zone_num,
                                      bool log_missing)
  {
    if (HaveSizingInfo(energyplus_data, zone_num)) {
      return energyplus_data.dataSize->FinalZoneSizing(zone_num).TimeStepNumAtHeatMax *
             energyplus_data.dataGlobal->TimeStepZoneSec;
    }

    LogMissingSizingInfo(energyplus_data, zone_num, "zone_sizing::TimeAtPeakHeat", log_missing);
    return 0.0;
  }

  [[nodiscard]] double MinCoolOA(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num, bool log_missing)
  {
    if (HaveSizingInfo(energyplus_data, zone_num)) {
      return energyplus_data.dataSize->FinalZoneSizing(zone_num).MinOA *
             energyplus_data.dataSize->FinalZoneSizing(zone_num).DesCoolDens;
    }

    LogMissingSizingInfo(energyplus_data, zone_num, "zone_sizing::MinCoolOA", log_missing);
    return 0.0;
  }

  [[nodiscard]] double MinHeatOA(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num, bool log_missing)
  {
    if (HaveSizingInfo(energyplus_data, zone_num)) {
      return energyplus_data.dataSize->FinalZoneSizing(zone_num).MinOA *
             energyplus_data.dataSize->FinalZoneSizing(zone_num).DesHeatDens;
    }

    LogMissingSizingInfo(energyplus_data, zone_num, "zone_sizing::MinHeatOA", log_missing);
    return 0.0;
  }

} // namespace zone_sizing

namespace zone_group_sizing {

  struct PeakLoad
  {
    double value;
    int day_timestep;
    int design_day_index;
  };

  // Instances of this function type will receive ZoneSizingData and return a sequence of loads for each timestep within
  // a design day.
  using GetLoadSeqFunc = std::function<Array1D<Real64>(const EnergyPlus::DataSizing::ZoneSizingData &)>;

  // The purpose of this function is to find the design day and timestep that has the highest combined load for the
  // given zones. This function is generic. The get_load_seq function defines what type of peak load (Sensible Cooling |
  // Heating) to locate.
  [[nodiscard]] static PeakLoad GetPeakLoad(const EnergyPlus::EnergyPlusData &energyplus_data,
                                            const std::vector<int> &zone_nums,
                                            const GetLoadSeqFunc &get_load_seq,
                                            bool log_missing)
  {
    const auto sizing_zones = ZonesWithSizingInfo(energyplus_data, zone_nums, log_missing);

    if (sizing_zones.empty()) {
      return {};
    }

    auto &zone_sizing = energyplus_data.dataSize->ZoneSizing;
    size_t first_zone_num = sizing_zones.front();
    const auto num_design_days = zone_sizing.isize1();
    std::vector<PeakLoad> peak_loads;

    for (int design_day_index = 1; design_day_index <= num_design_days; ++design_day_index) {
      const auto num_timesteps = static_cast<int>(get_load_seq(zone_sizing(design_day_index, first_zone_num)).size());
      std::vector<double> combined_group_load(num_timesteps);

      for (const auto &zone_num : sizing_zones) {
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
                                           const std::vector<int> &zone_nums,
                                           bool log_missing)
  {
    const auto get_cooling_load_seq = [](const EnergyPlus::DataSizing::ZoneSizingData &sizing_data) {
      return sizing_data.CoolLoadSeq;
    };

    return GetPeakLoad(energyplus_data,
                       zone_nums,
                       get_cooling_load_seq,
                       log_missing)
        .value;
  }

  [[nodiscard]] double LatentCoolingLoad(const EnergyPlus::EnergyPlusData &energyplus_data,
                                         const std::vector<int> &zone_nums,
                                         bool log_missing)
  {
    const auto get_cooling_load_seq = [](const EnergyPlus::DataSizing::ZoneSizingData &sizing_data) {
      return sizing_data.LatentCoolLoadSeq;
    };

    // This value may not be coincident with the peak sensible load. Is this what we want, or
    // should we return the latent load at the time of the peak sensible load?
    return GetPeakLoad(energyplus_data,
                       zone_nums,
                       get_cooling_load_seq,
                       log_missing)
        .value;
  }

  [[nodiscard]] double OutdoorTempAtPeakCool(const EnergyPlus::EnergyPlusData &energyplus_data,
                                             const std::vector<int> &zone_nums,
                                             bool log_missing)
  {
    const auto sizing_zones = ZonesWithSizingInfo(energyplus_data, zone_nums, log_missing);
    if (!sizing_zones.empty()) {
      const auto get_cooling_load_seq = [](const EnergyPlus::DataSizing::ZoneSizingData &sizing_data) {
        return sizing_data.CoolLoadSeq;
      };

      const auto peak_load = GetPeakLoad(energyplus_data,
                                         sizing_zones,
                                         get_cooling_load_seq,
                                         log_missing);
      // The outdoor temperature should be the same for all zones, so get the sizing data for the
      // peak design day, using any one of the zones in the group.
      const auto zone_sizing = energyplus_data.dataSize->ZoneSizing(peak_load.design_day_index, sizing_zones.front());

      return zone_sizing.CoolOutTempSeq(peak_load.day_timestep);
    } else {
      if (log_missing) {
        spdlog::warn("No sizing information available for any requested zones while computing "
                     "zone_group_sizing::OutdoorTempAtPeakCool; returning default 21 C");
      }
      return 21.0;
    }
  }

  [[nodiscard]] double OutdoorHumidityRatioAtPeakCool(const EnergyPlus::EnergyPlusData &energyplus_data,
                                                      const std::vector<int> &zone_nums,
                                                      bool log_missing)
  {
    const auto sizing_zones = ZonesWithSizingInfo(energyplus_data, zone_nums, log_missing);
    if (!sizing_zones.empty()) {
      const auto get_cooling_load_seq = [](const EnergyPlus::DataSizing::ZoneSizingData &sizing_data) {
        return sizing_data.CoolLoadSeq;
      };

      const auto peak_load = GetPeakLoad(energyplus_data,
                                         sizing_zones,
                                         get_cooling_load_seq,
                                         log_missing);
      const auto zone_sizing = energyplus_data.dataSize->ZoneSizing(peak_load.design_day_index, sizing_zones.front());

      return zone_sizing.CoolOutHumRatSeq(peak_load.day_timestep);
    } else {
      if (log_missing) {
        spdlog::warn("No sizing information available for any requested zones while computing "
                     "zone_group_sizing::OutdoorHumidityRatioAtPeakCool; returning default 0.0");
      }
      return 0.0;
    }
  }

  [[nodiscard]] double TimeAtPeakCool(const EnergyPlus::EnergyPlusData &energyplus_data,
                                      const std::vector<int> &zone_nums,
                                      bool log_missing)
  {
    const auto sizing_zones = ZonesWithSizingInfo(energyplus_data, zone_nums, log_missing);
    if (!sizing_zones.empty()) {
      const auto get_cooling_load_seq = [](const EnergyPlus::DataSizing::ZoneSizingData &sizing_data) {
        return sizing_data.CoolLoadSeq;
      };
      const auto peak_load = GetPeakLoad(energyplus_data,
                                         sizing_zones,
                                         get_cooling_load_seq,
                                         log_missing);
      // TODO: Are we sure this is right when the sizing is from a DesignDay?
      return peak_load.day_timestep * energyplus_data.dataGlobal->TimeStepZoneSec;
    }

    if (log_missing) {
      spdlog::warn("No sizing information available for any requested zones while computing "
                   "zone_group_sizing::TimeAtPeakCool; returning default 0 s");
    }
    return 0.0;
  }

  [[nodiscard]] double HeatingLoad(const EnergyPlus::EnergyPlusData &energyplus_data,
                                   const std::vector<int> &zone_nums,
                                   bool log_missing)
  {
    const auto get_heating_load_seq = [](const EnergyPlus::DataSizing::ZoneSizingData &sizing_data) {
      return sizing_data.HeatLoadSeq;
    };

    return GetPeakLoad(energyplus_data,
                       zone_nums,
                       get_heating_load_seq,
                       log_missing)
        .value;
  }

  [[nodiscard]] double OutdoorTempAtPeakHeat(const EnergyPlus::EnergyPlusData &energyplus_data,
                                             const std::vector<int> &zone_nums,
                                             bool log_missing)
  {
    const auto sizing_zones = ZonesWithSizingInfo(energyplus_data, zone_nums, log_missing);
    if (!sizing_zones.empty()) {
      const auto get_heating_load_seq = [](const EnergyPlus::DataSizing::ZoneSizingData &sizing_data) {
        return sizing_data.HeatLoadSeq;
      };

      const auto peak_load = GetPeakLoad(energyplus_data,
                                         sizing_zones,
                                         get_heating_load_seq,
                                         log_missing);
      // The outdoor temperature should be the same for all zones, so get the sizing data for the
      // peak design day, using any one of the zones in the group.
      const auto zone_sizing = energyplus_data.dataSize->ZoneSizing(peak_load.design_day_index, sizing_zones.front());

      return zone_sizing.HeatOutTempSeq(peak_load.day_timestep);
    } else {
      if (log_missing) {
        spdlog::warn("No sizing information available for any requested zones while computing "
                     "zone_group_sizing::OutdoorTempAtPeakHeat; returning default 21 C");
      }
      return 21.0;
    }
  }

  [[nodiscard]] double OutdoorHumidityRatioAtPeakHeat(const EnergyPlus::EnergyPlusData &energyplus_data,
                                                      const std::vector<int> &zone_nums,
                                                      bool log_missing)
  {
    const auto sizing_zones = ZonesWithSizingInfo(energyplus_data, zone_nums, log_missing);
    if (!sizing_zones.empty()) {
      const auto get_heating_load_seq = [](const EnergyPlus::DataSizing::ZoneSizingData &sizing_data) {
        return sizing_data.HeatLoadSeq;
      };

      const auto peak_load = GetPeakLoad(energyplus_data,
                                         sizing_zones,
                                         get_heating_load_seq,
                                         log_missing);
      // The outdoor temperature should be the same for all zones, so get the sizing data for the
      // peak design day, using any one of the zones in the group.
      const auto zone_sizing = energyplus_data.dataSize->ZoneSizing(peak_load.design_day_index, sizing_zones.front());

      return zone_sizing.HeatOutHumRatSeq(peak_load.day_timestep);
    } else {
      if (log_missing) {
        spdlog::warn("No sizing information available for any requested zones while computing "
                     "zone_group_sizing::OutdoorHumidityRatioAtPeakHeat; returning default 0.0");
      }
      return 0.0;
    }
  }

  [[nodiscard]] double TimeAtPeakHeat(const EnergyPlus::EnergyPlusData &energyplus_data,
                                      const std::vector<int> &zone_nums,
                                      bool log_missing)
  {
    const auto sizing_zones = ZonesWithSizingInfo(energyplus_data, zone_nums, log_missing);
    if (!sizing_zones.empty()) {
      // TODO: Are we sure this is right when the sizing is from a DesignDay
      const auto get_heating_load_seq = [](const EnergyPlus::DataSizing::ZoneSizingData &sizing_data) {
        return sizing_data.HeatLoadSeq;
      };
      const auto peak_load = GetPeakLoad(energyplus_data,
                                         sizing_zones,
                                         get_heating_load_seq,
                                         log_missing);
      return peak_load.day_timestep * energyplus_data.dataGlobal->TimeStepZoneSec;
    }

    if (log_missing) {
      spdlog::warn("No sizing information available for any requested zones while computing "
                   "zone_group_sizing::TimeAtPeakHeat; returning default 0 s");
    }
    return 0.0;
  }

  [[nodiscard]] double MinCoolOA(const EnergyPlus::EnergyPlusData &energyplus_data,
                                 const std::vector<int> &zone_nums,
                                 bool log_missing)
  {
    const auto sizing_zones = ZonesWithSizingInfo(energyplus_data, zone_nums, log_missing);
    if (!sizing_zones.empty()) {
      double sum_oa = 0;

      for (const auto zone_num : sizing_zones) {
        sum_oa += energyplus_data.dataSize->FinalZoneSizing(zone_num).MinOA *
                  energyplus_data.dataSize->FinalZoneSizing(zone_num).DesCoolDens;
      }

      return sum_oa;
    }

    if (log_missing) {
      spdlog::warn("No sizing information available for any requested zones while computing "
                   "zone_group_sizing::MinCoolOA; returning default 0.0");
    }
    return 0.0;
  }

  [[nodiscard]] double MinHeatOA(const EnergyPlus::EnergyPlusData &energyplus_data,
                                 const std::vector<int> &zone_nums,
                                 bool log_missing)
  {
    const auto sizing_zones = ZonesWithSizingInfo(energyplus_data, zone_nums, log_missing);
    if (!sizing_zones.empty()) {
      double sum_oa = 0;

      for (const auto zone_num : sizing_zones) {
        sum_oa += energyplus_data.dataSize->FinalZoneSizing(zone_num).MinOA *
                  energyplus_data.dataSize->FinalZoneSizing(zone_num).DesHeatDens;
      }

      return sum_oa;
    }

    if (log_missing) {
      spdlog::warn("No sizing information available for any requested zones while computing "
                   "zone_group_sizing::MinHeatOA; returning default 0.0");
    }
    return 0.0;
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

std::vector<int> InsideSurfaceTemperatureActuatorHandles(EnergyPlus::EnergyPlusData &energyplus_data,
                                                         const std::string_view surface_name)
{
  const auto surface_num = SurfaceNum(energyplus_data, surface_name);
  const auto &surface = energyplus_data.dataSurface->Surface(as_size_t(surface_num));
  const auto inside_actuator_handle =
      ActuatorHandle(energyplus_data, "Surface", "Surface Inside Temperature", surface.Name);

  auto &extBoundCond = surface.ExtBoundCond;
  if (extBoundCond > 0) {
    // If this is an interzone surface then set the outside of the matching surface
    auto &other_surface = energyplus_data.dataSurface->Surface(as_size_t(extBoundCond));
    const auto outside_actuator_handle =
        ActuatorHandle(energyplus_data, "Surface", "Surface Outside Temperature", other_surface.Name);

    return {inside_actuator_handle, outside_actuator_handle};
  }

  return {inside_actuator_handle};
}

std::vector<int> OutsideSurfaceTemperatureActuatorHandles(EnergyPlus::EnergyPlusData &energyplus_data,
                                                          const std::string_view surface_name)
{
  const auto surface_num = SurfaceNum(energyplus_data, surface_name);
  const auto &surface = energyplus_data.dataSurface->Surface(as_size_t(surface_num));
  const auto outside_actuator_handle =
      ActuatorHandle(energyplus_data, "Surface", "Surface Outside Temperature", surface.Name);

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

    return {outside_actuator_handle, inside_actuator_handle};
  }

  return {outside_actuator_handle};
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
  if (sums.temp_dep_coef() == 0.0) { // B=0
    newzonetemp = zonetemp + sums.temp_ind_coef() / aircap * dt;
  } else {
    newzonetemp = (zonetemp - sums.temp_ind_coef() / sums.temp_dep_coef()) *
                      std::exp(min(700.0, -sums.temp_dep_coef() / aircap * dt)) +
                  sums.temp_ind_coef() / sums.temp_dep_coef();
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
                   (zone_heat_balance.MDotOA * energyplus_data.dataEnvrn->OutHumRat);

  const double C = RhoAir * Zone(zonenum).Volume * Zone(zonenum).ZoneVolCapMultpMoist / dt;

  double newHumidityRatio = humidityRatio + (B / C);

  // Set the humidity ratio to zero if the zone has been dried out
  newHumidityRatio = std::max(newHumidityRatio, 0.0);

  // Check to make sure that is saturated there is condensation in the zone
  // by resetting to saturation conditions.
  const double wzSat = EnergyPlus::Psychrometrics::PsyWFnTdbRhPb(
      energyplus_data, ZT, 1.0, energyplus_data.dataEnvrn->OutBaroPress, RoutineName);

  newHumidityRatio = std::min(newHumidityRatio, wzSat);

  SetZoneHumidityRatio(energyplus_data, zonenum, newHumidityRatio);
}

void UpdateLatentGains(EnergyPlus::EnergyPlusData &energyplus_data)
{

  for (int zonei = 1; zonei <= energyplus_data.dataGlobal->NumOfZones; ++zonei) {
    EnergyPlus::InternalHeatGains::SumAllInternalLatentGains(energyplus_data, zonei);
  }
}

} // namespace spawn::energyplus
