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
    for (auto i = std::mt19937::max();  i & 0x8 && uuid.size() < len;  i >>= 4) {
      uuid += hex_chars[n & 0xf];
      n >>= 4;
    }
  }
  
  return uuid;
}

} // namespace spawn::util
