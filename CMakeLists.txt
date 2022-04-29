cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0025 NEW)

project(Spawn VERSION 0.3.0)
cmake_minimum_required(VERSION 3.13)

add_library(compile_options INTERFACE)
target_compile_features(compile_options INTERFACE cxx_std_17)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

option(BUILD_PYTHON_BINDINGS "Build Python bindings" OFF)

option(ENABLE_BUILDINGS_LIBRARY "Include the Modelica Buildings Library in the installer package" FALSE)
option(ENABLE_MODELICA_COMPILER "Build and include the JModelica compiler" FALSE)
if(ENABLE_MODELICA_COMPILER)
  find_file(OPTIMICA_TARFILE OCT-enduser-stable-1.33.3.tar.gz PATHS /usr/local/etc REQUIRED)
  set(OPTIMICA_TARFILE_URL
      "file://${OPTIMICA_TARFILE}"
      CACHE STRING "Tarball of Optimica distribution")

  target_compile_definitions(compile_options INTERFACE -DENABLE_MODELICA_COMPILER)
endif()

set(spawn_cli_name "spawn-${CMAKE_PROJECT_VERSION}-${CMAKE_PROJECT_VERSION_BUILD}")

# EnergyPlus will link to openmp dynamically by default
# We don't want that
set(ENABLE_OPENMP OFF CACHE BOOL "" FORCE)
set(USE_OpenMP OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC_ENERGYPLUS_API TRUE)

add_subdirectory(submodules/EnergyPlus EnergyPlus EXCLUDE_FROM_ALL)
include(submodules/EnergyPlus/cmake/CompilerFlags.cmake)
# This is a method to capture the EnergyPlus version
# See notes in make_energyplus_version.cmake
execute_process(COMMAND ${CMAKE_COMMAND}
  -DPROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}/submodules/EnergyPlus
  -DPROJECT_BINARY_DIR=${PROJECT_BINARY_DIR}/EnergyPlus
  -DSpawn_SOURCE_DIR=${Spawn_SOURCE_DIR}
  -P ${PROJECT_SOURCE_DIR}/cmake/create_energyplus_version.cmake)
include("${PROJECT_BINARY_DIR}/EnergyPlus/energyplus_version.cmake")
set(MBL_ENERGYPLUS_VERSION_STRING "EnergyPlus_${ENERGYPLUS_VERSION_MAJOR}_${ENERGYPLUS_VERSION_MINOR}_${ENERGYPLUS_VERSION_PATCH}")

# EnergyPlus is forcing CPACK_BINARY_IFW on but for Spawn we aren't using IFW so
# reset to OFF
set(CPACK_BINARY_IFW OFF)

# Intentionally only enable clang-tidy for everything except EnergyPlus
option(ENABLE_CLANG_TIDY "Enable clang-tidy checks during compilation" ON)

if(ENABLE_CLANG_TIDY)
  find_program(CLANGTIDY NAMES clang-tidy-12 clang-tidy-10 clang-tidy)

  if(CLANGTIDY)
    set(CMAKE_CXX_CLANG_TIDY ${CLANGTIDY} -extra-arg=-Wno-unknown-warning-option)
    if(${CMAKE_CXX_STANDARD})
      set(CMAKE_CXX_CLANG_TIDY ${CMAKE_CXX_CLANG_TIDY} -extra-arg=-std=c++${CMAKE_CXX_STANDARD})
    endif()

    list(APPEND CMAKE_CXX_CLANG_TIDY -warnings-as-errors=*)
  else()
    message(${WARNING_MESSAGE} "clang-tidy requested but executable not found")
  endif()
endif()

# Enable runtime checking features
if(CMAKE_COMPILER_IS_GNUCC OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

  target_compile_options(compile_options INTERFACE -Wall -Wconversion -Wextra -Wpedantic -Werror)
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

set(package_variant "")
if(NOT ENABLE_MODELICA_COMPILER)
  set(package_variant "light-")
endif()

set(CPACK_PACKAGE_VERSION
    "${package_variant}${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}-${CMAKE_PROJECT_VERSION_BUILD}")

# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/release/0.16/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(
  REQUIRES
  bzip2/1.0.8
  zlib/1.2.11
  libzip/1.7.3
  pugixml/1.11
  cli11/1.9.1
  catch2/2.13.7
  fmt/8.0.1
  nlohmann_json/3.10.2
  spdlog/1.9.2
  swig/4.0.2
  BASIC_SETUP
  CMAKE_TARGETS
  NO_OUTPUT_DIRS
  BUILD
  missing)

# ##############################################################################
# RPATH settings Conan set this to true, which is not the cmake default and not
# what we want for the openstudio targets
set(CMAKE_SKIP_RPATH FALSE)
# ##############################################################################

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

# Sets the variable "platform" corresponding to the FMI conventions
include(cmake/fmiplatform.cmake)
fmi_platform(FMI_PLATFORM)

include(cmake/download_modelica.cmake)
if(ENABLE_MODELICA_COMPILER)

  # we only support llvm-10 right now
  find_package(LLVM 10 REQUIRED CONFIG)
  # We want to nudge cmake towards clang-10, so it doesn't find the wrong libclang
  # we cannot use the version number because clang's cmake files don't have versioning
  find_package(Clang REQUIRED CONFIG HINTS /usr/lib/cmake/clang-10)

  if(APPLE)
    set(jmodelica_tar_root "Modelica-0.1.1-Linux")
    set(jmodelica_url "https://modelica.s3.amazonaws.com/docker-build/Modelica-latest.tar.gz")
    set(jmodelica_md5 "9798e0ca33160c5faefe3160d5a8b36b")
  elseif(UNIX)
    set(jmodelica_tar_root "Modelica-0.1.1-Linux")
    set(jmodelica_url "https://modelica.s3.amazonaws.com/docker-build/Modelica-latest.tar.gz")
    set(jmodelica_md5 "9798e0ca33160c5faefe3160d5a8b36b")
  else()
    set(jmodelica_tar_root "Modelica-0.1.1-Linux")
    set(jmodelica_url "https://modelica.s3.amazonaws.com/docker-build/Modelica-latest.tar.gz")
    set(jmodelica_md5 "9798e0ca33160c5faefe3160d5a8b36b")
  endif()

  set(jmodelica_path "${PROJECT_BINARY_DIR}/JModelica/")

  download_modelica(
    ${jmodelica_path} # The final directory name
    ${jmodelica_tar_root} # The directory name of the tarball root, this will be
                          # moved / renamed to modelica_path
    ${jmodelica_md5} # The md5 of the tarball
    ${jmodelica_url} # The download url. Can be a local file:// url
  )

  if(APPLE)
    set(optimica_tar_root "install")
    set(optimica_md5 "b3d2f4690c506753fdd9399ad1d3af6a")
  elseif(UNIX)
    set(optimica_tar_root "install")
    set(optimica_md5 "b3d2f4690c506753fdd9399ad1d3af6a")
  else()
    set(optimica_tar_root "install")
    set(optimica_md5 "b3d2f4690c506753fdd9399ad1d3af6a")
  endif()

  set(optimica_path "${PROJECT_BINARY_DIR}/Optimica/")

  download_modelica(
    ${optimica_path} # The final directory name
    ${optimica_tar_root} # The directory name of the tarball root, this will be
                         # moved / renamed to modelica_path
    ${optimica_md5} # The md5 of the tarball
    ${OPTIMICA_TARFILE_URL} # The download url. Can be a local file:// url
  )

  set(MSL_ROOT_DIR "${optimica_path}/ThirdParty/MSL")
  set(MSL_VER "MSL400")
  install(DIRECTORY "${MSL_ROOT_DIR}" DESTINATION etc/)

  # GFortran is a runtime dependency of FMUs that are generated by Optimica/JModelica
  find_library(GFortranLib NAMES gfortran PATHS /usr/lib/gcc/x86_64-linux-gnu/7/ /usr/lib/gcc/x86_64-linux-gnu/8/ /usr/lib/gcc/x86_64-linux-gnu/9/ REQUIRED)
  file(REAL_PATH ${GFortranLib} GFORTRANLIB_REAL_PATH)
  cmake_path(GET GFORTRANLIB_REAL_PATH FILENAME GFORTRANLIB_NAME)
  set(GFORTRANLIB_EMBEDDED_PATH "usr/lib/${GFORTRANLIB_NAME}")
endif()

enable_testing()

add_subdirectory(util)
if(ENABLE_MODELICA_COMPILER)
  add_subdirectory(jmodelica)
  add_subdirectory(compiler)
  add_subdirectory(optimica)
endif()
add_subdirectory(lib)
add_subdirectory(cli)
add_subdirectory(epfmi)
add_subdirectory(fmu)
add_subdirectory(embedded)
add_subdirectory(test)

if(APPLE)
  set(os_prefix darwin64)
elseif(WIN32)
  set(os_prefix win64)
elseif(UNIX AND NOT APPLE)
  set(os_prefix linux64)
endif()

set(idd_path "${PROJECT_BINARY_DIR}/EnergyPlus/Products/Energy+.idd")
add_custom_target(
  copy_spawn_cli_depends ALL
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:epfmi> $<TARGET_FILE_DIR:spawn>
  COMMAND ${CMAKE_COMMAND} -E copy ${idd_path} $<TARGET_FILE_DIR:spawn>)

set(mbl_spawn_root_path "${PROJECT_SOURCE_DIR}/submodules/modelica-buildings/Buildings/Resources/bin/${spawn_cli_name}/${os_prefix}")
set(mbl_spawn_bin_path "${mbl_spawn_root_path}/bin/")
set(mbl_spawn_lib_path "${mbl_spawn_root_path}/lib/")
set(mbl_spawn_idd_path "${mbl_spawn_root_path}/etc/")

if(ENABLE_BUILDINGS_LIBRARY)
  include(ExternalProject)
  ExternalProject_Add(
    MBL
    BUILD_ALWAYS 1
    BUILD_IN_SOURCE SOURCE_DIR
    "${PROJECT_SOURCE_DIR}/submodules/modelica-buildings"
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config $<CONFIG>
    COMMAND ${CMAKE_COMMAND} -E make_directory ${mbl_spawn_bin_path}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${mbl_spawn_lib_path}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${mbl_spawn_idd_path}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:spawn> ${idd_path} "${mbl_spawn_bin_path}"
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:epfmi> "${mbl_spawn_lib_path}"
    COMMAND ${CMAKE_COMMAND} -E copy ${idd_path} "${mbl_spawn_idd_path}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/examples/RefBldgSmallOfficeNew2004_Chicago"
            "${PROJECT_BINARY_DIR}/RefBldgSmallOfficeNew2004_Chicago")

  # The full spawn cli name is embedded in the MBL, here we update the name with the latest sha
  set(building_mo_path "submodules/modelica-buildings/Buildings/ThermalZones/${MBL_ENERGYPLUS_VERSION_STRING}/Building.mo")
  file(READ ${building_mo_path} building_mo_text)
  string(REGEX REPLACE "spawn-[0-9]\.[0-9]\.[0-9]-.........." ${spawn_cli_name} new_building_mo_text "${building_mo_text}")
  file(WRITE ${building_mo_path} "${new_building_mo_text}")

  set(fmilib_project_source_dir "${PROJECT_SOURCE_DIR}/submodules/modelica-buildings/Buildings/Resources/src/fmi-library/")
  set(fmilib_project_build_dir "${fmilib_project_source_dir}/build/")

  install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/submodules/modelica-buildings"
    DESTINATION etc/
    PATTERN ${fmilib_project_build_dir} EXCLUDE)

  ExternalProject_Add(
    FMILIB
    SOURCE_DIR ${fmilib_project_source_dir}
    BINARY_DIR ${fmilib_project_build_dir}
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config $<CONFIG>
    INSTALL_COMMAND "")
endif()

install(FILES ${PROJECT_BINARY_DIR}/EnergyPlus/Products/Energy+.idd DESTINATION etc/)

include(CPack)
