#ifndef SPAWN_CONVERSION_HPP
#define SPAWN_CONVERSION_HPP

#include <cassert>
#include <cstdint>

std::size_t as_size_t(const long long value)
{
  assert(value >= 0);
  return static_cast<std::size_t>(value);
}

#endif // SPAWN_CONVERSION_HPP
