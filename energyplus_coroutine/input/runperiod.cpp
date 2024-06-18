#include "runperiod.hpp"
#include <fmt/format.h>
#include <iostream>

namespace spawn {

RunPeriod RunPeriod::create_run_period([[maybe_unused]] const nlohmann::json &spawnjson)
{
  RunPeriod result;

  const auto run_period_json = spawnjson.value("RunPeriod", nlohmann::json::object());

  result.day_of_week_for_start_day =
      run_period_json.value("day_of_week_for_start_day", result.day_of_week_for_start_day);

  result.apply_weekend_holiday_rule =
      run_period_json.value("apply_weekend_holiday_rule", result.apply_weekend_holiday_rule);

  result.use_weather_file_daylight_saving_period =
      run_period_json.value("use_weather_file_daylight_saving_period", result.use_weather_file_daylight_saving_period);

  result.use_weather_file_holidays_and_special_days = run_period_json.value(
      "use_weather_file_holidays_and_special_days", result.use_weather_file_holidays_and_special_days);

  result.use_weather_file_rain_indicators =
      run_period_json.value("use_weather_file_rain_indicators", result.use_weather_file_rain_indicators);

  result.use_weather_file_snow_indicators =
      run_period_json.value("use_weather_file_snow_indicators", result.use_weather_file_snow_indicators);

  return result;
}

} // namespace spawn
