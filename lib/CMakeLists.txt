
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/ )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/ )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus/public )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/ )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/ObjexxFCL/src/ )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/FMI2/ )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/CLI/ )

if (UNIX)
  ADD_DEFINITIONS("-fPIC")
endif()

ADD_DEFINITIONS(-DENERGYPLUS_STATIC_API)

add_library(libspawn
  spawn.hpp
  spawn.cpp
  outputtypes.hpp
  units.hpp
  units.cpp
  variables.hpp
  variables.cpp
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

target_link_libraries(libspawn energypluslib CONAN_PKG::boost)

