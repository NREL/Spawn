#include "testpaths.hpp"

std::string spawnexe() {
  return "$<TARGET_FILE:spawn>";
}

std::string testcase1() {
  return "${PROJECT_BINARY_DIR}/RefBldgSmallOfficeNew2004_Chicago/RefBldgSmallOfficeNew2004_Chicago.spawn";
}

boost::filesystem::path testzip() {
  return "${PROJECT_SOURCE_DIR}/test/test_unzipping.zip";
}

boost::filesystem::path fmi_load_test() {
  return "$<TARGET_FILE:fmi_load_test>";
}

