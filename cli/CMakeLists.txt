
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/fmi2/ )
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} )
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/ )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/ )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus/public )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/ )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/ObjexxFCL/src/ )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/FMI2/ )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/CLI/ )

set( src
  main.cpp
  ziputil.hpp
  ziputil.cpp
)

add_executable(spawn ${src})

configure_file(config.hpp.in config.hxx)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(spawn
  CONAN_PKG::libzip
  epvariables
  energypluslib
  CONAN_PKG::CLI11
  CONAN_PKG::pugixml
  CONAN_PKG::boost_filesystem
)

install(TARGETS spawn DESTINATION bin)
