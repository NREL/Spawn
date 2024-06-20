#ifndef SPAWN_CONVERSION_HPP
#define SPAWN_CONVERSION_HPP

#include "util/filesystem.hpp"
#include "util/strings.hpp"
#include <boost/date_time/date_defs.hpp>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <map>
#include <string>
#include <vector>

inline std::size_t as_size_t(const long long value)
{
  assert(value >= 0);
  return static_cast<std::size_t>(value);
}

inline std::string pathVectorToPathString(const std::vector<spawn_fs::path> &paths)
{
  std::stringstream ss;
  for (const auto &path : paths) {
    ss << path.string();
    ss << ":";
  }
  auto result = ss.str();
  result.pop_back();
  return result;
}

inline boost::date_time::weekdays day_from_string(const std::string_view day_string)
{
  const std::map<std::string_view, boost::date_time::weekdays> days = {
      {"sunday", boost::date_time::Sunday},
      {"monday", boost::date_time::Monday},
      {"tuesday", boost::date_time::Tuesday},
      {"wednesday", boost::date_time::Wednesday},
      {"thursday", boost::date_time::Thursday},
      {"friday", boost::date_time::Friday},
      {"saturday", boost::date_time::Saturday},
  };

  return days.at(spawn::to_lower(day_string));
}

#endif // SPAWN_CONVERSION_HPP
