#include "paths.hpp"
#include "util/config.hpp"

namespace spawn::test {

spawn_fs::path testdir()
{
  return "${CMAKE_CURRENT_BINARY_DIR}";
}

spawn_fs::path spawnexe()
{
  return "$<TARGET_FILE:spawn>";
}

spawn_fs::path testcase1()
{
  return spawn::project_source_dir() /
         "examples/RefBldgSmallOfficeNew2004_Chicago/RefBldgSmallOfficeNew2004_Chicago.spawn";
}

spawn_fs::path testzip()
{
  return spawn::project_source_dir() / "test/example_zip.zip";
}

spawn_fs::path example_fmu_path()
{
  return "$<TARGET_FILE:example_fmu_lib>";
}

spawn_fs::path idd_path()
{
  return spawn::project_binary_dir() / "energyplus/Products/Energy+.idd";
}

} // namespace spawn::test
