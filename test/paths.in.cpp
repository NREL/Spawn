#include "paths.hpp"

spawn_fs::path testdir()
{
  return "${CMAKE_CURRENT_BINARY_DIR}";
}

std::string spawnexe()
{
  return "$<TARGET_FILE:spawn>";
}

std::string testcase1()
{
  return "${PROJECT_SOURCE_DIR}/examples/RefBldgSmallOfficeNew2004_Chicago/RefBldgSmallOfficeNew2004_Chicago.spawn";
}

spawn_fs::path testzip()
{
  return "${PROJECT_SOURCE_DIR}/test/example_zip.zip";
}

spawn_fs::path example_fmu_path()
{
  return "$<TARGET_FILE:example_fmu_lib>";
}
