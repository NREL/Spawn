# add_compile_options(-Wno-conversion)

add_library(
  fmu
  modeldescription.hpp
  modeldescription.cpp
  modelexchange.hpp
  modelexchange.cpp
  fmu.hpp
  fmu.cpp
  fmi2.hpp
  fmi2.cpp
  simulate.hpp
  simulate.cpp)

target_link_libraries(
  fmu
  PUBLIC pugixml::pugixml spawn_utils
  PRIVATE fmt::fmt compile_options nlohmann_json::nlohmann_json)

target_include_directories(fmu PUBLIC "${PROJECT_SOURCE_DIR}/fmi2/")
