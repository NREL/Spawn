#include <random>

namespace spawn::util {

std::string uniqueId()
{
  constexpr auto len = 32;
  static const std::string_view hex_chars = "0123456789abcdef";

  std::mt19937 gen{std::random_device{}()};
  std::string uuid;
  uuid.reserve(len);

  while (uuid.size() < len) {
    auto n = gen();
    for (unsigned i = std::mt19937::max(); bool(i & 0x8U) && (uuid.size() < len); i >>= 4U) {
      uuid += hex_chars[n & 0xfU];
      n >>= 4U;
    }
  }

  return uuid;
}

} // namespace spawn::util
