#include "../start_time.hpp"
#include "util/math.hpp"
#include <catch2/catch.hpp>

TEST_CASE("Test Default StartTime") // NOLINT
{
  spawn::StartTime start_time;

  CHECK(start_time.seconds() == Approx(0.0));

  CHECK(start_time.spawn_epoch().day() == static_cast<short>(1));
  CHECK(start_time.spawn_epoch().month() == boost::date_time::Jan);
  CHECK(start_time.spawn_epoch().year() == 2006);

  CHECK(start_time.energyplus_epoch().day() == static_cast<short>(1));
  CHECK(start_time.energyplus_epoch().month() == boost::date_time::Jan);
  CHECK(start_time.energyplus_epoch().year() == 2006);

  CHECK(start_time.energyplus_time_differential() == Approx(0.0));
}

TEST_CASE("Test Negative StartTime") // NOLINT
{
  spawn::StartTime start_time(boost::date_time::weekdays::Sunday, spawn::days_to_seconds(-7));

  CHECK(start_time.seconds() == Approx(spawn::days_to_seconds(-7)));

  CHECK(start_time.spawn_epoch().day() == static_cast<short>(1));
  CHECK(start_time.spawn_epoch().month() == boost::date_time::Jan);
  CHECK(start_time.spawn_epoch().year() == 2006);

  CHECK(start_time.energyplus_epoch().day() == static_cast<short>(25));
  CHECK(start_time.energyplus_epoch().month() == boost::date_time::Dec);
  CHECK(start_time.energyplus_epoch().year() == 2005);

  CHECK(start_time.energyplus_time_differential() == Approx(spawn::days_to_seconds(-7)));
}