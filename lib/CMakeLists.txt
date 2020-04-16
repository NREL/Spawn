
if (UNIX)
  ADD_DEFINITIONS("-fPIC")
endif()

ADD_DEFINITIONS(-DENERGYPLUS_STATIC_API)

add_library(libspawn
  emsactuator.hpp
  emsactuator.cpp
  input.hpp
  input.cpp
  outputtypes.hpp
  outputvariable.hpp
  outputvariable.cpp
  spawn.hpp
  spawn.cpp
  units.hpp
  units.cpp
  variables.hpp
  variables.cpp
  schedule.hpp
  schedule.cpp
  zone.hpp
  zone.cpp

  # EnergyPlus compiles these files into a DLL, but we would prefer to
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus/CommandLineInterface.hh
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus/CommandLineInterface.cc
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus/api/func.h
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus/api/func.cc
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus/api/runtime.h
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus/api/runtime.cc
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus/api/datatransfer.h
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus/api/datatransfer.cc
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus/api/EnergyPlusPgm.hh
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus/api/EnergyPlusPgm.cc
)

target_include_directories(libspawn
  PUBLIC
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus/public
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/ObjexxFCL/src/
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/FMI2/
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/CLI/
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/
)

target_link_libraries(libspawn PUBLIC CONAN_PKG::boost spawn_utils)

