#ifndef spawn_compare_INCLUDED
#define spawn_compare_INCLUDED

#include <algorithm>
#include <iostream>

namespace spawn {

[[nodiscard]] bool case_insensitive_compare(const std::string &s1, const std::string &s2);

} // namespace spawn

#endif // spawn_compare_INCLUDED
