# The purpose of this file is to help the Spawn CMake project 
# retrieve the EnergyPlus version information.
# This file is executed as a cmake script within a subprocess of the main Spawn
# cmake config. If the <EnergyPLus>/cmake/Version.cmake file were included directly in the 
# Spawn cmake project, then the Spawn related cmake variables would be overwritten.
# e.g. CMAKE_VERSION_MAJOR would give the EnergyPlus version when we want the Spawn version.

# PROJECT_SOURCE_DIR will be the EnergyPlus project source directory
include("${PROJECT_SOURCE_DIR}/cmake/Version.cmake")
configure_file("${Spawn_SOURCE_DIR}/cmake/energyplus_version.cmake.in" "${PROJECT_BINARY_DIR}/energyplus_version.cmake")
