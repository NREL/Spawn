
add_library(fmu fmu.cpp fmu.hpp fmi.cpp fmi.hpp modelexchange.hpp modelexchange.cpp)
target_link_libraries(fmu PUBLIC CONAN_PKG::pugixml CONAN_PKG::boost spawn_utils PRIVATE CONAN_PKG::fmt)
target_compile_features(fmu PUBLIC cxx_std_17)
target_include_directories(fmu PUBLIC "${PROJECT_SOURCE_DIR}/fmi2/")

