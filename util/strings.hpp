#ifndef UTIL_STRINGS_HPP
#define UTIL_STRINGS_HPP

#include <spdlog/common.h>
#include <string>

namespace spawn {

[[nodiscard]] bool case_insensitive_compare(const std::string &s1, const std::string &s2);

// Remove leading whitespace and newlines
std::string &ltrim(std::string &s);

// Remove trailing whitespace and newlines
std::string &rtrim(std::string &s);

// Remove leading and trailing whitespace and newlines
std::string &trim(std::string &s);

// Remove opening and closing single and double quotes
std::string &unquote(std::string &s);

} // namespace spawn

#endif // UTIL_STRINGS_HPP
