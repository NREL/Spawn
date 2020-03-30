#include "testpaths.hpp"

std::string spawnexe() {
  return "$<TARGET_FILE:spawn>";
}

std::string testcase1() {
  return "${PROJECT_BINARY_DIR}/RefBldgSmallOfficeNew2004_Chicago/RefBldgSmallOfficeNew2004_Chicago.spawn";
}

