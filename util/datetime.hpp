#ifndef SPAWN_UTIL_DATETIME_INCLUDED
#define SPAWN_UTIL_DATETIME_INCLUDED

#include <boost/date_time/date_defs.hpp>
#include <boost/date_time/gregorian/greg_year.hpp>

namespace spawn {

class YearDescription
{
public:
  explicit YearDescription() = default;

  explicit YearDescription(boost::date_time::weekdays start_day_of_year, bool is_leap_year, bool look_in_future = false)
      : start_day_of_year_(start_day_of_year), is_leap_year_(is_leap_year), look_in_future_(look_in_future)
  {
  }

  [[nodiscard]] unsigned short AssumedYear() const noexcept;

  boost::date_time::weekdays start_day_of_year_{boost::date_time::Thursday};
  bool is_leap_year_{false};
  bool look_in_future_{true};
};

} // namespace spawn

#endif // SPAWN_UTIL_DATETIME_INCLUDED
