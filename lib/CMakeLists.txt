
if (UNIX)
  ADD_DEFINITIONS("-fPIC")
endif()

ADD_DEFINITIONS(-DENERGYPLUS_STATIC_API)

add_library(libspawn
  actuatortypes.hpp
  fmugenerator.hpp
  fmugenerator.cpp
  idfprep.hpp
  idfprep.cpp
  idf_to_json.hpp
  idf_to_json.cpp
  manager.hpp
  manager.cpp
  outputtypes.hpp
  spawn.hpp
  spawn.cpp
  units.hpp
  units.cpp
  variables.hpp
  variables.cpp
  warmupmanager.hpp
  warmupmanager.cpp
  ziputil.hpp
  ziputil.cpp

  input/emsactuator.hpp
  input/emsactuator.cpp
  input/input.hpp
  input/input.cpp
  input/outputvariable.hpp
  input/outputvariable.cpp
  input/schedule.hpp
  input/schedule.cpp
  input/surface.hpp
  input/surface.cpp
  input/zone.hpp
  input/zone.cpp
)

target_include_directories(libspawn
  PUBLIC
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus/Data
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/ObjexxFCL/src/
  PRIVATE
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus/public
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/ObjexxFCL/src/
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/FMI2/
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/CLI/
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/btwxt/src/
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/re2/
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/SQLite/
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/kiva-ep/src/
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/eigen/
  ${PROJECT_BINARY_DIR}/EnergyPlus/third_party/kiva-ep/src/libkiva/
)

target_link_libraries(libspawn
  PUBLIC
  spawn_utils
  CONAN_PKG::libzip
  CONAN_PKG::pugixml
)


