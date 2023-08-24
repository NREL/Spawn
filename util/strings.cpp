#include "util/strings.hpp"
#include <algorithm>

namespace spawn {

bool case_insensitive_compare(const std::string &s1, const std::string &s2)
{
  return std::equal(begin(s1), end(s1), begin(s2), end(s2), [](const auto lhs, const auto rhs) {
    return ::tolower(lhs) == ::tolower(rhs);
  });
}

std::string &ltrim(std::string &s)
{
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return std::isspace(ch) == 0; }));
  return s;
}

std::string &rtrim(std::string &s)
{
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return std::isspace(ch) == 0; }).base(), s.end());
  return s;
}

std::string &trim(std::string &s)
{
  rtrim(s);
  ltrim(s);

  return s;
}

std::string &unquote(std::string &s)
{
  auto is_quote = [](unsigned char ch) {
    const auto quotes = {'\'', '\"'};
    return std::find(quotes.begin(), quotes.end(), ch) != quotes.end();
  };

  // Remove opening quote
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [&](unsigned char ch) { return !is_quote(ch); }));
  // Remove closing quote
  s.erase(std::find_if(s.rbegin(), s.rend(), [&](unsigned char ch) { return !is_quote(ch); }).base(), s.end());

  return s;
}

} // namespace spawn
