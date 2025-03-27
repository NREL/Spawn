cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0025 NEW)
cmake_minimum_required(VERSION 3.13)

project(Spawn VERSION 0.6.0)
include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

option(ENABLE_BUILDINGS_LIBRARY "Include the Modelica Buildings Library in the installer package" TRUE)

include(cmake/CompileOptions.cmake)
include(cmake/EnergyPlus.cmake)
include(cmake/ClangTidy.cmake)
include(cmake/ClangSanitize.cmake)
include(cmake/GitVersion.cmake)
include(cmake/Dependencies.cmake)
include(cmake/README.cmake)

add_subdirectory(util)
add_subdirectory(coroutine)
add_subdirectory(cli)
add_subdirectory(epfmi)
add_subdirectory(fmu)
add_subdirectory(test)
add_subdirectory(mbl)

include(cmake/PackageOptions.cmake)
include(CPack)
