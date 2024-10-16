#include <string_view>
#include <vector>

namespace EnergyPlus {
struct EnergyPlusData;
}

namespace spawn::energyplus {

class ZoneSums
{
public:
  ZoneSums(EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

  [[nodiscard]] double TempDepCoef() const;
  [[nodiscard]] double TempIndCoef() const;
  [[nodiscard]] double QConSenFlow() const;

private:
  double temp_dep_coef_;
  double temp_ind_coef_;
  double q_con_sen_flow_;
};

[[nodiscard]] int ZoneNum(EnergyPlus::EnergyPlusData &energyplus_data, const std::string_view zone_name);

[[nodiscard]] std::vector<int> ZoneNums(EnergyPlus::EnergyPlusData &energyplus_data,
                                        const std::vector<std::string> &zone_names);

[[nodiscard]] double ZoneVolume(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

[[nodiscard]] double ZoneFloorArea(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

[[nodiscard]] double ZoneVolCapMultpSens(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

[[nodiscard]] double ZoneLatentGain(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

[[nodiscard]] double ZoneMeanRadiantTemp(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

bool HaveSizingInfo(const EnergyPlus::EnergyPlusData &energyplus_data);

namespace zone_sizing {

  [[nodiscard]] double SensibleCoolingLoad(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

  [[nodiscard]] double LatentCoolingLoad(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

  [[nodiscard]] double OutdoorTempAtPeakCool(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

  [[nodiscard]] double OutdoorHumidityRatioAtPeakCool(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

  [[nodiscard]] double TimeAtPeakCool(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

  [[nodiscard]] double HeatingLoad(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

  [[nodiscard]] double OutdoorTempAtPeakHeat(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

  [[nodiscard]] double OutdoorHumidityRatioAtPeakHeat(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

  [[nodiscard]] double TimeAtPeakHeat(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

} // namespace zone_sizing

namespace zone_group_sizing {

  [[nodiscard]] double SensibleCoolingLoad(const EnergyPlus::EnergyPlusData &energyplus_data,
                                           const std::vector<int> &zone_nums);

  [[nodiscard]] double LatentCoolingLoad(const EnergyPlus::EnergyPlusData &energyplus_data,
                                         const std::vector<int> &zone_nums);

  [[nodiscard]] double OutdoorTempAtPeakCool(const EnergyPlus::EnergyPlusData &energyplus_data,
                                             const std::vector<int> &zone_nums);

} // namespace zone_group_sizing

void SetZoneTemperature(EnergyPlus::EnergyPlusData &energyplus_data, const int zone_num, const double &temp);

double ZoneTemperature(EnergyPlus::EnergyPlusData &energyplus_data, const int zone_num);

void SetZoneHumidityRatio(EnergyPlus::EnergyPlusData &energyplus_data, const int zone_num, const double &ratio);

double ZoneHumidityRatio(EnergyPlus::EnergyPlusData &energyplus_data, const int zone_num);

[[nodiscard]] int
VariableHandle(EnergyPlus::EnergyPlusData &energyplus_data, const std::string_view name, const std::string_view key);

[[nodiscard]] double VariableValue(EnergyPlus::EnergyPlusData &energyplus_data, int handle);

[[nodiscard]] int ActuatorHandle(EnergyPlus::EnergyPlusData &energyplus_data,
                                 const std::string &component_type,
                                 const std::string &control_type,
                                 const std::string &component_name);

void SetActuatorValue(EnergyPlus::EnergyPlusData &energyplus_data, int handle, const double &value);

void ResetActuator(EnergyPlus::EnergyPlusData &energyplus_data, int handle);

[[nodiscard]] int SurfaceNum(EnergyPlus::EnergyPlusData &energyplus_data, std::string_view surface_name);

[[nodiscard]] double SurfaceArea(EnergyPlus::EnergyPlusData &energyplus_data, int surface_num);

[[nodiscard]] double SurfaceInsideHeatFlow(EnergyPlus::EnergyPlusData &energyplus_data, int surface_num);

[[nodiscard]] double SurfaceOutsideHeatFlow(EnergyPlus::EnergyPlusData &energyplus_data, int surface_num);

void SetInsideSurfaceTemperature(EnergyPlus::EnergyPlusData &energyplus_data, const int surface_num, double temp);

void SetOutsideSurfaceTemperature(EnergyPlus::EnergyPlusData &energyplus_data, const int surface_num, double temp);

void UpdateZoneTemperature(EnergyPlus::EnergyPlusData &energyplus_data, const int zonenum, const double dt);

void UpdateZoneHumidityRatio(EnergyPlus::EnergyPlusData &energyplus_data, const int zonenum, const double dt);

void UpdateLatentGains(EnergyPlus::EnergyPlusData &energyplus_data);

} // namespace spawn::energyplus
