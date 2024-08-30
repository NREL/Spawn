#include "./datetime.hpp"
#include <boost/date_time/gregorian/greg_date.hpp>

[[nodiscard]] unsigned short spawn::YearDescription::AssumedYear() const noexcept
{
  // start out assuming 2009
  unsigned short year(2009);

  // start incrementing or decrementing assumedYear until we match *yearStartsOn and assumedLeapYear
  while ((boost::gregorian::gregorian_calendar::is_leap_year(year) != is_leap_year_) ||
         (boost::gregorian::date(year, boost::date_time::Jan, 1).day_of_week() != start_day_of_year_)) {
    if (look_in_future_) {
      ++year;
    } else {
      --year;
    }
  }

  return year;
}
