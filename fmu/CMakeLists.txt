
add_library(fmu 
  modeldescription.hpp
  modeldescription.cpp
  fmu.hpp
  fmu.cpp
)
target_link_libraries(fmu PUBLIC CONAN_PKG::pugixml spawn_utils)
target_include_directories(fmu PUBLIC "${PROJECT_SOURCE_DIR}/fmi2/")

