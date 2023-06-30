#ifndef FMU_FMU_TYPE_HPP_INCLUDED
#define FMU_FMU_TYPE_HPP_INCLUDED

#include <string>

namespace spawn::fmu {

enum class FMUType
{
  ME,
  CS
};

inline const char *toString(FMUType t)
{
  switch (t) {
  case FMUType::ME:
    return "ME";
  default:
    return "CS";
  }
}

inline FMUType toFMUType(const std::string &t)
{
  if (t == "ME") {
    return FMUType::ME;
  } else {
    return FMUType::CS;
  }
}

} // namespace spawn::fmu

#endif // FMU_FMU_TYPE_HPP_INCLUDED
