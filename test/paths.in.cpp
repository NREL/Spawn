#include "paths.hpp"

fs::path testdir() {
  return "${CMAKE_CURRENT_BINARY_DIR}";
}

std::string spawnexe() {
  return "$<TARGET_FILE:spawn>";
}

std::string testcase1() {
  return "${PROJECT_SOURCE_DIR}/examples/RefBldgSmallOfficeNew2004_Chicago/RefBldgSmallOfficeNew2004_Chicago.spawn";
}

fs::path testzip() {
  return "${PROJECT_SOURCE_DIR}/test/example_zip.zip";
}

fs::path example_fmu_path() {
  return "$<TARGET_FILE:example_fmu_lib>";
}

fs::path project_source_dir() {
  return "${PROJECT_SOURCE_DIR}";
}

fs::path project_binary_dir() {
  return "${PROJECT_BINARY_DIR}";
}

