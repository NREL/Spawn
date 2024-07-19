#include <string_view>

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

[[nodiscard]] double ZoneVolume(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

[[nodiscard]] double ZoneFloorArea(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

[[nodiscard]] double ZoneVolCapMultpSens(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

[[nodiscard]] double ZoneLatentGain(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

[[nodiscard]] double ZoneMeanRadiantTemp(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

[[nodiscard]] double ZoneDesignCoolingLoad(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

[[nodiscard]] double ZoneDesignCoolingLatentLoad(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

[[nodiscard]] double ZoneOutdoorTempAtPeakCool(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

[[nodiscard]] double ZoneOutdoorHumidityRatioAtPeakCool(const EnergyPlus::EnergyPlusData &energyplus_data,
                                                        int zone_num);

//[[nodiscard]] double ZoneTimeAtPeakCool(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

[[nodiscard]] double ZoneDesignHeatingLoad(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

[[nodiscard]] double ZoneOutdoorTempAtPeakHeat(const EnergyPlus::EnergyPlusData &energyplus_data, int zone_num);

[[nodiscard]] double ZoneOutdoorHumidityRatioAtPeakHeat(const EnergyPlus::EnergyPlusData &energyplus_data,
                                                        int zone_num);

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
