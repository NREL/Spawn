#include "compare.hpp"

namespace spawn {

  bool case_insensitive_compare(const std::string &s1, const std::string &s2) {
    return std::equal(begin(s1), end(s1), begin(s2), end(s2),
                      [](const auto lhs, const auto rhs){
                        return ::tolower(lhs) == ::tolower(rhs);
                      });
  }

}
