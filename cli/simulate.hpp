#ifndef spawn_fmu_sim_INCLUDED
#define spawn_fmu_sim_INCLUDED

#include "../fmu/fmu.hpp"
#include "../fmu/modeldescription.hpp"
#include "../util/filesystem.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

namespace spawn::fmu {

class Sim
{
public:
  explicit Sim(fs::path fmu_path);
  void run(const nlohmann::json &config);

private:
  void openLogs();
  void writeLogs(void *comp, const double &time);
  void closeLogs();

  std::vector<FMU::Variable> initContinuousVariables();
  std::vector<fmi2ValueReference> initValueReferences();

  fs::path m_fmu_path;
  bool m_require_all_symbols{false};
  FMU m_fmu{m_fmu_path, m_require_all_symbols};

  fs::path m_resource_path{m_fmu.extractedFilesPath() / "resources"};
  std::string m_resource_url{std::string("file://") + m_resource_path.string()};

  std::vector<FMU::Variable> m_continous_vars{std::move(initContinuousVariables())};
  std::vector<fmi2ValueReference> m_var_references{std::move(initValueReferences())};
  std::vector<double> m_var_values{m_var_references.size(), 0.0, std::allocator<double>()};

  std::fstream m_csvout;
};

} // namespace spawn::fmu
#endif
