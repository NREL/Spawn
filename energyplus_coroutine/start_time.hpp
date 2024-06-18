#ifndef SPAWN_ENERGYPLUS_COROUTINE_START_TIME_INCLUDED
#define SPAWN_ENERGYPLUS_COROUTINE_START_TIME_INCLUDED

#include "input/input.hpp"
#include "util/datetime.hpp"
#include <boost/date_time/date_defs.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace spawn {

// StartTime is a container for information about the start time of a Spawn simulation.
// In Spawn, time t = 0 corresponds to time 00:00 of January 1 of the start year.
// The start year is determined by finding a calendar year matching the given criteria (currently the day of week on
// January 1, but also may include leap year in the future). On the other hand, EnergyPlus time is relative to the start
// of the EnergyPlus run period. StartTime will determine an appropriate start date for the EnergyPlus RunPeriod, based
// on January 1 of the start year plus an offset in the case that 00:00 of Jan 1 is not the desired start time.
// StartTime provides EnergyPlusOffset, which is the difference in time between the start of the EnergyPlus run period,
// and Jan 1 of the start year.
class StartTime
{
public:
  explicit StartTime(const boost::date_time::weekdays &day_of_week_for_start_day = boost::date_time::weekdays::Sunday,
                     const double &seconds = 0.0)
      : day_of_week_for_start_day_(day_of_week_for_start_day), seconds_(seconds)
  {
  }

  // The desired start time in (possibly fractional) number of seconds from the Spawn epoch.
  // This value can be negative.
  [[nodiscard]] double Seconds() const
  {
    return seconds_;
  }

  // The epoch of Spawn time, which is time 00:00 of January 1,
  // where the year is computed based on the start day of year.
  [[nodiscard]] boost::gregorian::date SpawnEpoch() const
  {
    return spawn_epoch_;
  }

  // The epoch of EnergyPlus time, which correponds to the start date
  // of the EnergyPlus run period.
  [[nodiscard]] boost::gregorian::date EnergyPlusEpoch() const
  {
    return energyplus_epoch_;
  }

  // The Time differential in seconds between the EnergyPlus and Spawn epoch times.
  [[nodiscard]] double EnergyPlusTimeDifferential() const
  {
    return energyplus_time_differential_;
  }

private:
  // Day of the week for Jan 1 of the start year.
  boost::date_time::weekdays day_of_week_for_start_day_;

  // Is the start year a leap year?
  bool is_leap_year_{false};

  // The desired start time in (possibly fractional) number of seconds from the Spawn epoch.
  // This value can be negative.
  double seconds_{0.0};

  // Description of the spawn epoch year.
  // This is used to find an assumed year, given the criteria.
  YearDescription start_year_description_{day_of_week_for_start_day_, is_leap_year_};

  // The date, Jan 1, and time 00:00, of the start year.
  boost::gregorian::date spawn_epoch_{start_year_description_.AssumedYear(), boost::date_time::Jan, 1};

  // The date corresponding to the start of the EnergyPlus run period.
  // This could be a date prior to spawn_epoch_, because seconds_ can be negative.
  boost::gregorian::date energyplus_epoch_{
      boost::posix_time::ptime(spawn_epoch_, boost::posix_time::seconds(static_cast<long>(std::floor(seconds_))))
          .date()};

  // The time differential (as number of seconds), between the start of the EnergyPlus RunPeriod
  // and the beginning of the start year (t = 0).
  // This value can also be negative.
  double energyplus_time_differential_{static_cast<double>(
      (boost::posix_time::ptime(energyplus_epoch_) - boost::posix_time::ptime(spawn_epoch_)).total_seconds())};
};

} // namespace spawn

#endif // SPAWN_ENERGYPLUS_COROUTINE_START_TIME_INCLUDED
