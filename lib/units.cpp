#include "units.hpp"

using namespace spawn::units;

namespace spawn::units {

  std::string toString(const UnitType &unittype)
  {
    const auto it = std::find_if(
        std::begin(unitstrings), std::end(unitstrings), [&](const UnitString &us) { return us.first == unittype; });

    if (it != std::end(unitstrings)) {
      return it->second;
    }

    return "";
  }

  UnitType fromString(const std::string &unitstring)
  {
    const auto it = std::find_if(
        std::begin(unitstrings), std::end(unitstrings), [&](const UnitString &us) { return us.second == unitstring; });

    if (it != std::end(unitstrings)) {
      return it->first;
    }

    return UnitType::one;
  }

  Quantity convert(const Quantity &q, const UnitType &targetUnit)
  {
    if (q.unit == targetUnit) {
      // no conversion needed
      return q;
    }

    // Lookup a unit conversion
    auto c = conversions.find({q.unit, targetUnit});
    if (c != std::end(conversions)) {
      return {q.value * c->second.factor + c->second.offset, targetUnit};
    }

    // Lookup the reverse unit conversion
    c = conversions.find({targetUnit, q.unit});
    if (c != std::end(conversions)) {
      return {(q.value - c->second.offset) / c->second.factor, targetUnit};
    }

    std::cout << "No unit conversion is available" << std::endl;
    return q;
  }

} // namespace spawn
