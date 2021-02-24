#include "testpaths.hpp"

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
  return "${PROJECT_SOURCE_DIR}/test/test_unzipping.zip";
}

fs::path fmi_load_test() {
  return "$<TARGET_FILE:fmi_load_test>";
}

