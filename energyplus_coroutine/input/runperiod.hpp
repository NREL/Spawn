#ifndef RunPeriod_hh_INCLUDED
#define RunPeriod_hh_INCLUDED

#include <fmt/format.h>
#include <nlohmann/json.hpp>

namespace spawn {

class RunPeriod
{
public:
  RunPeriod() = default;

  [[nodiscard]] static RunPeriod create_run_period(const nlohmann::json &spawnjson);

  std::string day_of_week_for_start_day{"Sunday"};
  std::string apply_weekend_holiday_rule{"No"};
  std::string use_weather_file_daylight_saving_period{"No"};
  std::string use_weather_file_holidays_and_special_days{"No"};
  std::string use_weather_file_rain_indicators{"Yes"};
  std::string use_weather_file_snow_indicators{"Yes"};
};

} // namespace spawn

#endif // RunPeriod_hh_INCLUDED
