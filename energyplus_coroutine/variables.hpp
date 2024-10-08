#ifndef Variables_hh_INCLUDED
#define Variables_hh_INCLUDED

#include "units.hpp"
#include <memory>
#include <optional>
#include <pugixml.hpp>
#include <string>
#include <vector>
#include <functional>

namespace EnergyPlus {
struct EnergyPlusData;
}

namespace spawn {
class UserConfig;
}

// namespace spawn::units {
// enum class UnitSystem;
// }
//
// namespace spawn::energyplus {
// enum class UnitSystem;
// } // namespace spawn::energyplus

namespace spawn::variable {

// A container to store variables.
// More on this and what a Variable is further down.
class Variables;
// A cache for values within a Variable.
// More on this further down also.
template <class T> class CachedValue;

// A variable is a numercial value that is exchanged between EnergyPlus,
// and something else (probably Modelica) that is running from a different thread.
// The Variable class:
// 1. Buffers the numerical value, which could be either an input or an output to EnergyPlus.
// 2. Stores metadata about the value.
// 3. Provides a method to synchronize the buffered value between EnergyPlus and the other thread.
class Variable
{
public:
  Variable &operator=(const Variable &) = delete;
  Variable &operator=(Variable &&) = delete;
  Variable(const Variable &) = delete;
  Variable(Variable &&) = delete;
  virtual ~Variable() = default;

  [[nodiscard]] std::string_view Name() const;
  [[nodiscard]] int Index() const;
  [[nodiscard]] const pugi::xml_document &Metadata() const;

  [[nodiscard]] std::optional<double> Value(const units::UnitSystem &unit = units::UnitSystem::MO) const;
  virtual void SetValue(const double &value, const units::UnitSystem &unit);
  virtual void ResetValue();

  virtual void Update(EnergyPlus::EnergyPlusData &energyplus_data) = 0;

protected:
  Variable(Variables &variables, std::string_view name, units::UnitType ep_unit, units::UnitType mo_unit);

  std::string name_;
  spawn::units::UnitType ep_unit_;
  spawn::units::UnitType mo_unit_;
  pugi::xml_document metadata_;
  std::optional<double> value_;
  int index_;
};

// A Variable that is an output from EnergyPlus
class Output : public Variable
{
protected:
  Output(Variables &variables, std::string_view name, units::UnitType ep_unit, units::UnitType mo_unit);

  // Attempting to set/reset the value of an output will log an error message and will not change state
  // The output value is changed by the Variable::Update method.
  void SetValue(const double &value, const units::UnitSystem &unit) override;
  void ResetValue() override;
};

// A Variable like Output, but the value does not change during the simulation
class Parameter : public Variable
{
protected:
  Parameter(Variables &variables, std::string_view name, units::UnitType ep_unit, units::UnitType mo_unit);

  // Attempting to set/reset the value of a parameter will log an error message and will not change state
  // The parameter value is changed by the Variable::Update method.
  void SetValue(const double &value, const units::UnitSystem &unit) override;
  void ResetValue() override;
};

// A Variable that is an input to EnergyPlus
class Input : public Variable
{
public:
  using Variable::ResetValue;
  using Variable::SetValue;

protected:
  Input(Variables &variables, std::string_view name, units::UnitType ep_unit, units::UnitType mo_unit);
};

// Some of the values that are within the EnergyPlusData
// require a costly lookup. The conanical example is looking up
// the zone index given the zone name.
//
// CachedValue is used within Variables to avoid repeated costly lookups
// during the frequent Variable::Update operation.
template <class T> class CachedValue
{
  using GetterFunction = std::function<T(EnergyPlus::EnergyPlusData &)>;

public:
  explicit CachedValue(const GetterFunction &getter) : getter_(getter)
  {
  }
  CachedValue &operator=(const CachedValue &) = delete;
  CachedValue &operator=(CachedValue &&) = delete;
  CachedValue(const CachedValue &) = delete;
  CachedValue(CachedValue &&) = delete;
  virtual ~CachedValue() = default;
  T get(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    if (value_) {
      return *value_;
    } else {
      value_ = getter_(energyplus_data);
      return *value_;
    }
  }

private:
  std::optional<T> value_;
  GetterFunction getter_;
};

using VariableVector = std::vector<std::unique_ptr<Variable>>;
using VariableRefs = std::vector<std::reference_wrapper<Variable>>;
using VariableNameIndex = std::map<std::string, int>;

// This is a container to hold a set of Variable instances.
// In the main Spawn API, a variable is accessed by index,
// which is also how values are accessed via the FMI standard.
class Variables
{
public:
  explicit Variables(const UserConfig &user_config);
  Variables &operator=(const Variables &) = delete;
  Variables &operator=(Variables &&) = delete;
  Variables(const Variables &) = delete;
  Variables(Variables &&) = delete;
  virtual ~Variables() = default;

  // This is used to create a new instance of a particular Variable type.
  // The key point is that we want to heap allocate Variable instances.
  // The variables container will take ownership of the newly created instance.
  template <class T, class... Args> static void CreateOne(Variables &variables, Args &&...args)
  {
    static_assert(std::is_base_of<Variable, T>::value, "CreateOne can only create subclasses of Variable");
    new T(variables, std::forward<Args>(args)...);
  }

  [[nodiscard]] const VariableVector &AllVariables() const;
  [[nodiscard]] const VariableRefs &Inputs() const;
  [[nodiscard]] const VariableRefs &Outputs() const;
  [[nodiscard]] const VariableRefs &Parameters() const;

  void UpdateInputs(EnergyPlus::EnergyPlusData &energyplus_data);
  void UpdateOutputs(EnergyPlus::EnergyPlusData &energyplus_data);
  void UpdateParameters(EnergyPlus::EnergyPlusData &energyplus_data);

  // Given a Variable's name, return its index into the AllVariables container
  // Will throw on an invalid name
  [[nodiscard]] int VariableIndex(const std::string_view variable_name) const;

private:
  void AddVariable(std::unique_ptr<Variable> &&variable);
  void AddVariable(Input &variable);
  void AddVariable(Output &variable);
  void AddVariable(Parameter &variable);

  // This container owns all of the variables
  // This defines the primary numeric index for accessing a particular variable
  VariableVector all_variables_;
  // It is common to iterate over only the inputs, outputs, or parameters.
  // These containers hold references to provide efficient access to specific types of variables.
  VariableRefs input_variables_;
  VariableRefs output_variables_;
  VariableRefs parameter_variables_;
  // It is also common to lookup variables by name instead of numeric index.
  // VariableNameIndex makes lookup efficient.
  VariableNameIndex variable_name_index_;

  friend class Variable;
  friend class Input;
  friend class Output;
  friend class Parameter;
};

// Begin defining the specific Variable types, which are documented here:
// https://lbl-srg.github.io/soep/softwareArchitecture.html#coupling-of-the-envelope-model
// Namespaces are used to disambiguate variables such as temperature T of a zone versus a surface
//
// All Variable types must have a static CreateAll factory method.
// Constructor and factory are private, because instances are created by the Variables friend class.
namespace zone {
  class V : public Parameter
  {
    friend class variable::Variables;

  private:
    explicit V(Variables &variables, const std::string_view zone_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string zone_name_;
    CachedValue<int> zone_num_;
  };

  class AFlo : public Parameter
  {
    friend class variable::Variables;

  private:
    explicit AFlo(Variables &variables, const std::string_view zone_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string zone_name_;
    CachedValue<int> zone_num_;
  };

  class MSenFac : public Parameter
  {
    friend class variable::Variables;

  private:
    explicit MSenFac(Variables &variables, const std::string_view zone_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string zone_name_;
    CachedValue<int> zone_num_;
  };

  class QConSenFlow : public Output
  {
    friend class variable::Variables;

  private:
    explicit QConSenFlow(Variables &variables, std::string_view zone_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string zone_name_;
    CachedValue<int> zone_num_;
  };

  class QLatFlow : public Output
  {
    friend class variable::Variables;

  private:
    explicit QLatFlow(Variables &variables, const std::string_view zone_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string zone_name_;
    CachedValue<int> zone_num_;
  };

  class QPeoFlow : public Output
  {
    friend class variable::Variables;

  private:
    explicit QPeoFlow(Variables &variables, const std::string_view zone_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string zone_name_;
    CachedValue<int> sensor_handle_;
  };

  class TRad : public Output
  {
    friend class variable::Variables;

  private:
    explicit TRad(Variables &variables, const std::string_view zone_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string zone_name_;
    CachedValue<int> zone_num_;
  };

  class QCooSenFlow : public Parameter
  {
    friend class variable::Variables;

  private:
    explicit QCooSenFlow(Variables &variables, const std::string_view zone_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string zone_name_;
    CachedValue<int> zone_num_;
  };

  class QCooLatFlow : public Parameter
  {
    friend class variable::Variables;

  private:
    explicit QCooLatFlow(Variables &variables, const std::string_view zone_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string zone_name_;
    CachedValue<int> zone_num_;
  };

  class TOutCoo : public Parameter
  {
    friend class variable::Variables;

  private:
    explicit TOutCoo(Variables &variables, const std::string_view zone_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string zone_name_;
    CachedValue<int> zone_num_;
  };

  class XOutCoo : public Parameter
  {
    friend class variable::Variables;

  private:
    explicit XOutCoo(Variables &variables, const std::string_view zone_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string zone_name_;
    CachedValue<int> zone_num_;
  };

  class MOutCooFlow : public Parameter
  {
    friend class variable::Variables;

  private:
    explicit MOutCooFlow(Variables &variables, const std::string_view zone_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string zone_name_;
    CachedValue<int> zone_num_;
  };

  class TCoo : public Parameter
  {
    friend class variable::Variables;

  private:
    explicit TCoo(Variables &variables, const std::string_view zone_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string zone_name_;
    CachedValue<int> zone_num_;
  };

  class QHeaFlow : public Parameter
  {
    friend class variable::Variables;

  private:
    explicit QHeaFlow(Variables &variables, const std::string_view zone_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string zone_name_;
    CachedValue<int> zone_num_;
  };

  class TOutHea : public Parameter
  {
    friend class variable::Variables;

  private:
    explicit TOutHea(Variables &variables, const std::string_view zone_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string zone_name_;
    CachedValue<int> zone_num_;
  };

  class XOutHea : public Parameter
  {
    friend class variable::Variables;

  private:
    explicit XOutHea(Variables &variables, const std::string_view zone_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string zone_name_;
    CachedValue<int> zone_num_;
  };

  class MOutHeaFlow : public Parameter
  {
    friend class variable::Variables;

  private:
    explicit MOutHeaFlow(Variables &variables, const std::string_view zone_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string zone_name_;
    CachedValue<int> zone_num_;
  };

  class THea : public Parameter
  {
    friend class variable::Variables;

  private:
    explicit THea(Variables &variables, const std::string_view zone_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string zone_name_;
    CachedValue<int> zone_num_;
  };

  class MInletsFlow : public Input
  {
    friend class variable::Variables;

  private:
    explicit MInletsFlow(Variables &variables, const std::string_view zone_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string zone_name_;
    CachedValue<int> zone_num_;
  };

  class TAveInlet : public Input
  {
    friend class variable::Variables;

  private:
    explicit TAveInlet(Variables &variables, const std::string_view zone_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string zone_name_;
    CachedValue<int> zone_num_;
  };

  class T : public Input
  {
    friend class variable::Variables;

  private:
    explicit T(Variables &variables, const std::string_view zone_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string zone_name_;
    CachedValue<int> zone_num_;
  };

  class X : public Input
  {
    friend class variable::Variables;

  private:
    explicit X(Variables &variables, const std::string_view zone_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string zone_name_;
    CachedValue<int> zone_num_;
  };

  class QGaiRadFlow : public Input
  {
    friend class variable::Variables;

  private:
    explicit QGaiRadFlow(Variables &variables, const std::string_view zone_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    const std::string zone_name_;
    const std::string actuator_name_;
    const std::string actuator_type_{"OtherEquipment"};
    const std::string actuator_controltype_{"Power Level"};
    CachedValue<int> handle_;
  };
} // namespace zone

namespace other {
  class Sensor : public Output
  {
    friend class variable::Variables;

  private:
    explicit Sensor(Variables &variables,
                    const std::string_view sensor_name,
                    const std::string_view energyplus_name,
                    const std::string_view energyplus_key);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    const std::string energyplus_name_;
    const std::string energyplus_key_;
    CachedValue<int> sensor_handle_;
  };

  class Actuator : public Input
  {
    friend class variable::Variables;

  private:
    explicit Actuator(Variables &variables,
                      const std::string_view name,
                      const std::string_view component_name_,
                      const std::string_view component_type_,
                      const std::string_view component_controltype_);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    const std::string component_name_;
    const std::string component_type_;
    const std::string component_controltype_;
    CachedValue<int> actuator_handle_;
  };

  class Schedule : public Input
  {
    friend class variable::Variables;

  private:
    explicit Schedule(Variables &variables,
                      const std::string_view name,
                      const std::string_view component_name,
                      const std::string_view component_type,
                      const std::string_view component_controltype);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    const std::string component_name_;
    const std::string component_type_;
    const std::string component_controltype_;
    CachedValue<int> handle_;
  };
} // namespace other

namespace surface {
  class A : public Parameter
  {
    friend class variable::Variables;

  private:
    explicit A(Variables &variables, const std::string_view surface_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string surface_name_;
    CachedValue<int> surface_num_;
  };

  class QFlow : public Output
  {
    friend class variable::Variables;

  private:
    explicit QFlow(Variables &variables, std::string_view surface_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string surface_name_;
    CachedValue<int> surface_num_;
  };

  class T : public Input
  {
    friend class variable::Variables;

  private:
    explicit T(Variables &variables, const std::string_view surface_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string surface_name_;
    CachedValue<int> surface_num_;
  };
} // namespace surface

namespace construction {
  class A : public Parameter
  {
    friend class variable::Variables;

  private:
    explicit A(Variables &variables, const std::string_view surface_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string surface_name_;
    CachedValue<int> surface_num_;
  };

  class QFrontFlow : public Output
  {
    friend class variable::Variables;

  private:
    explicit QFrontFlow(Variables &variables, const std::string_view surface_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string surface_name_;
    CachedValue<int> surface_num_;
  };

  class QBackFlow : public Output
  {
    friend class variable::Variables;

  private:
    explicit QBackFlow(Variables &variables, const std::string_view surface_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string surface_name_;
    CachedValue<int> surface_num_;
  };

  class TFront : public Input
  {
    friend class variable::Variables;

  private:
    explicit TFront(Variables &variables, const std::string_view surface_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string surface_name_;
    CachedValue<int> surface_num_;
  };

  class TBack : public Input
  {
    friend class variable::Variables;

  private:
    explicit TBack(Variables &variables, const std::string_view surface_name);
    static void CreateAll(const UserConfig &user_config, Variables &variables);
    void Update(EnergyPlus::EnergyPlusData &energyplus_data) final;

    std::string surface_name_;
    CachedValue<int> surface_num_;
  };
} // namespace construction

} // namespace spawn::variable

#endif // Variables_hh_INCLUDED
