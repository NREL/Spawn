#include "../start_time.hpp"
#include "util/math.hpp"
#include <catch2/catch.hpp>

TEST_CASE("Test Default StartTime")
{
  spawn::StartTime start_time;

  CHECK(start_time.Seconds() == Approx(0.0));

  CHECK(start_time.SpawnEpoch().day() == static_cast<short>(1));
  CHECK(start_time.SpawnEpoch().month() == boost::date_time::Jan);
  CHECK(start_time.SpawnEpoch().year() == 2006);

  CHECK(start_time.EnergyPlusEpoch().day() == static_cast<short>(1));
  CHECK(start_time.EnergyPlusEpoch().month() == boost::date_time::Jan);
  CHECK(start_time.EnergyPlusEpoch().year() == 2006);

  CHECK(start_time.EnergyPlusTimeDifferential() == Approx(0.0));
}

TEST_CASE("Test Negative StartTime")
{
  spawn::StartTime start_time(boost::date_time::weekdays::Sunday, spawn::days_to_seconds(-7));

  CHECK(start_time.Seconds() == Approx(spawn::days_to_seconds(-7)));

  CHECK(start_time.SpawnEpoch().day() == static_cast<short>(1));
  CHECK(start_time.SpawnEpoch().month() == boost::date_time::Jan);
  CHECK(start_time.SpawnEpoch().year() == 2006);

  CHECK(start_time.EnergyPlusEpoch().day() == static_cast<short>(25));
  CHECK(start_time.EnergyPlusEpoch().month() == boost::date_time::Dec);
  CHECK(start_time.EnergyPlusEpoch().year() == 2005);

  CHECK(start_time.EnergyPlusTimeDifferential() == Approx(spawn::days_to_seconds(-7)));
}
