#ifndef spawn_math_hh_INCLUDED
#define spawn_math_hh_INCLUDED

namespace spawn {

inline double days_to_seconds(const int days) {
  const double seconds_per_minute = 60.0;
  const double minutes_per_hour = 60.0;
  const double hours_per_day = 24.0;
  return seconds_per_minute * minutes_per_hour * hours_per_day * days;
}

inline double c_to_k(const double c) {
  return c + 273.15;
}

} // namespace spawn

#endif // spawn_math_hh_INCLUDED
