cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0025 NEW)

project(Spawn VERSION 0.1.0)
cmake_minimum_required(VERSION 3.13)

add_library(compile_options INTERFACE)
target_compile_features(compile_options INTERFACE cxx_std_17)

set(CMAKE_CXX_STANDARD 17)

option(ENABLE_BUILDINGS_LIBRARY "Include the Modelica Buildings Library in the installer package" FALSE)
option(ENABLE_MODELICA_COMPILER "Enable undefined behavior sanitizer testing in gcc/clang" FALSE)
if(ENABLE_MODELICA_COMPILER)
  target_compile_definitions(compile_options INTERFACE -DENABLE_MODELICA_COMPILER)
endif()

set(BUILD_STATIC_ENERGYPLUS_API TRUE)
add_subdirectory(submodules/EnergyPlus EnergyPlus EXCLUDE_FROM_ALL)
include(submodules/EnergyPlus/cmake/CompilerFlags.cmake)

# EnergyPlus is forcing CPACK_BINARY_IFW on but for Spawn we aren't using IFW so reset to OFF
set(CPACK_BINARY_IFW OFF)

# Enable runtime checking features
if(CMAKE_COMPILER_IS_GNUCC OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

  target_compile_options(compile_options INTERFACE -Wall -Wextra)
  option(ENABLE_ADDRESS_SANITIZER "Enable address sanitizer testing in gcc/clang" OFF)
  if(ENABLE_ADDRESS_SANITIZER)
    target_compile_options(compile_options INTERFACE -fsanitize=address -g)
    target_link_options(compile_options INTERFACE -fsanitize=address)
  endif()

  option(ENABLE_UNDEFINED_SANITIZER "Enable undefined behavior sanitizer testing in gcc/clang" OFF)
  if(ENABLE_UNDEFINED_SANITIZER)
    target_compile_options(compile_options INTERFACE -fsanitize=undefined -g)
    target_link_options(compile_options INTERFACE -fsanitize=undefined)
  endif()

endif()

if(NOT GIT_FOUND)
  find_program(GIT_EXECUTABLE git HINTS "$ENV{LOCALAPPDATA}/Programs/git/bin")
  if(NOT GIT_EXECUTABLE_NOTFOUND)
    set(GIT_FOUND TRUE)
  endif()
endif()

if(GIT_FOUND)
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" "rev-parse" "--short=10" "HEAD"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    TIMEOUT 10
    RESULT_VARIABLE RESULT
    OUTPUT_VARIABLE GIT_VERSION
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(${RESULT} EQUAL 0 AND NOT "${GIT_VERSION}" EQUAL "${CMAKE_PROJECT_VERSION_BUILD}")
    set(CMAKE_PROJECT_VERSION_BUILD
        ${GIT_VERSION}
        CACHE STRING "Build number" FORCE) # git sha
  endif()

  get_filename_component(GIT_DIR "${GIT_EXECUTABLE}" PATH)
else()
  set(GIT_DIR "")
endif()

set(CPACK_PACKAGE_VERSION
    "${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}-${CMAKE_PROJECT_VERSION_BUILD}")

# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/release/0.16/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

#conan_check(VERSION 1.0.0 REQUIRED)

conan_add_remote(NAME bincrafters URL https://api.bintray.com/conan/bincrafters/public-conan)

conan_cmake_run(
  REQUIRES
  bzip2/1.0.8
  zlib/1.2.11
  libzip/1.5.1@bincrafters/stable
  jsonformoderncpp/3.6.1@vthiery/stable
  pugixml/1.11
  CLI11/1.7.1@cliutils/stable
  catch2/2.11.3
  fmt/6.1.2
  nlohmann_json/3.7.3
  spdlog/1.6.0
  BASIC_SETUP
  CMAKE_TARGETS
  NO_OUTPUT_DIRS
  BUILD
  missing)

###############################################################################
# RPATH settings
# Conan set this to true, which is not the cmake default and not what we want
# for the openstudio targets
set(CMAKE_SKIP_RPATH FALSE)
###############################################################################

configure_file(README.md README.md)
install(FILES ${PROJECT_BINARY_DIR}/README.md DESTINATION .)

include(embedded/EmbedFiles.cmake)

check_include_file("unistd.h" HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
  target_compile_definitions(compile_options INTERFACE "-DHAVE_UNISTD_H")
endif()

check_include_file_cxx("filesystem" HAVE_FILESYSTEM_H)
if(HAVE_FILESYSTEM_H)
  target_compile_definitions(compile_options INTERFACE "-DHAVE_FILESYSTEM_H")
endif()

check_include_file_cxx("experimental/filesystem" HAVE_EXP_FILESYSTEM_H)
if(HAVE_EXP_FILESYSTEM_H)
  target_compile_definitions(compile_options INTERFACE "-DHAVE_EXP_FILESYSTEM_H")
endif()

add_subdirectory(util)
if(ENABLE_MODELICA_COMPILER)
  add_subdirectory(modelica)
  add_subdirectory(compiler)
endif()
add_subdirectory(lib)
add_subdirectory(cli)
add_subdirectory(epfmi)
add_subdirectory(fmu)
add_subdirectory(embedded)
add_subdirectory(examples)

enable_testing()
add_subdirectory(test)

if(APPLE)
  set(os_prefix spawn-darwin64)
elseif(WIN32)
  set(os_prefix spawn-win64)
elseif(UNIX AND NOT APPLE)
  set(os_prefix spawn-linux64)
endif()

set(idd_path "${PROJECT_BINARY_DIR}/EnergyPlus/Products/Energy+.idd")
add_custom_target(
  copy_spawn_cli_depends ALL
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:epfmi> $<TARGET_FILE_DIR:spawn>
  COMMAND ${CMAKE_COMMAND} -E copy ${idd_path} $<TARGET_FILE_DIR:spawn>)

set(modelica_bin_path "${PROJECT_BINARY_DIR}/modelica-buildings/Buildings/Resources/bin/${os_prefix}/bin/")
set(modelica_lib_path "${PROJECT_BINARY_DIR}/modelica-buildings/Buildings/Resources/bin/${os_prefix}/lib/")

if(ENABLE_BUILDINGS_LIBRARY)
  include(ExternalProject)
  ExternalProject_Add(
    MBL
    BUILD_ALWAYS 1
    URL "${PROJECT_SOURCE_DIR}/submodules/modelica-buildings"
    SOURCE_DIR "${PROJECT_BINARY_DIR}/modelica-buildings"
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config $<CONFIG>
    COMMAND ${CMAKE_COMMAND} -E make_directory ${modelica_bin_path}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${modelica_lib_path}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:spawn> ${idd_path} "${modelica_bin_path}"
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:epfmi> "${modelica_lib_path}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/examples/RefBldgSmallOfficeNew2004_Chicago"
            "${PROJECT_BINARY_DIR}/RefBldgSmallOfficeNew2004_Chicago")
endif()

install(DIRECTORY examples DESTINATION .)
install(FILES ${PROJECT_BINARY_DIR}/EnergyPlus/Products/Energy+.idd DESTINATION etc/)

include(CPack)
