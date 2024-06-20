#include "../datetime.hpp"
#include <catch2/catch.hpp>

TEST_CASE("Test YearDescription")
{
  spawn::YearDescription year_description;
  CHECK(year_description.AssumedYear() == 2009);

  boost::date_time::weekdays start_day_of_week = boost::date_time::Thursday;
  bool is_leap_year = false;
  bool look_in_future = false;

  year_description = spawn::YearDescription(start_day_of_week, is_leap_year, look_in_future);
  CHECK(year_description.AssumedYear() == 2009);

  start_day_of_week = boost::date_time::Sunday;
  is_leap_year = false;
  look_in_future = false;

  year_description = spawn::YearDescription(start_day_of_week, is_leap_year, look_in_future);
  CHECK(year_description.AssumedYear() == 2006);

  start_day_of_week = boost::date_time::Sunday;
  is_leap_year = true;
  look_in_future = false;

  year_description = spawn::YearDescription(start_day_of_week, is_leap_year, look_in_future);
  CHECK(year_description.AssumedYear() == 1984);

  start_day_of_week = boost::date_time::Sunday;
  is_leap_year = true;
  look_in_future = true;

  year_description = spawn::YearDescription(start_day_of_week, is_leap_year, look_in_future);
  CHECK(year_description.AssumedYear() == 2012);
}
