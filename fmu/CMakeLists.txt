add_library(fmu 
  modeldescription.hpp
  modeldescription.cpp
  modelexchange.hpp
  modelexchange.cpp
  fmu.hpp
  fmu.cpp
  fmi2.hpp
  fmi2.cpp
)
target_link_libraries(fmu PUBLIC CONAN_PKG::pugixml spawn_utils PRIVATE CONAN_PKG::fmt compile_options)
target_include_directories(fmu PUBLIC "${PROJECT_SOURCE_DIR}/fmi2/")

