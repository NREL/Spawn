
add_library(fmu fmu.hpp fmu.cpp)
target_link_libraries(fmu PUBLIC CONAN_PKG::pugixml CONAN_PKG::boost spawn_utils PRIVATE CONAN_PKG::fmt)
target_include_directories(fmu PUBLIC "${PROJECT_SOURCE_DIR}/fmi2/")

