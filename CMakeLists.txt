cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0025 NEW)

project(Spawn VERSION 0.3.0)
cmake_minimum_required(VERSION 3.13)

add_library(compile_options INTERFACE)
target_compile_features(compile_options INTERFACE cxx_std_17)
#target_compile_definitions(compile_options INTERFACE JSON_USE_IMPLICIT_CONVERSIONS=ON)

set(CMAKE_CXX_STANDARD 17)

option(ENABLE_BUILDINGS_LIBRARY "Include the Modelica Buildings Library in the installer package" FALSE)
option(ENABLE_MODELICA_COMPILER "Build and include the JModelica compiler" FALSE)
if(ENABLE_MODELICA_COMPILER)
  set(OPTIMICA_TARFILE_URL "file:///usr/local/etc/OCT-EndUser-trunk-r23009.tar.gz" CACHE STRING "Tarball of Optimica distribution")
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

set(package_variant "")
if(NOT ENABLE_MODELICA_COMPILER)
  set(package_variant "light-")
endif()

set(CPACK_PACKAGE_VERSION
  "${package_variant}${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}-${CMAKE_PROJECT_VERSION_BUILD}"
)

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
  catch2/2.11.3
  fmt/8.0.1
  nlohmann_json/3.10.2
  spdlog/1.9.2
  BASIC_SETUP
  CMAKE_TARGETS
  NO_OUTPUT_DIRS
  BUILD
  missing
)

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

include(cmake/download_modelica.cmake)
if(ENABLE_MODELICA_COMPILER)
  find_package(LLVM REQUIRED CONFIG)
  find_package(Clang REQUIRED CONFIG)

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
    ${jmodelica_tar_root} # The directory name of the tarball root, this will be moved / renamed to modelica_path
    ${jmodelica_md5} # The md5 of the tarball
    ${jmodelica_url} # The download url. Can be a local file:// url
  )

  install(DIRECTORY "${jmodelica_path}/ThirdParty/MSL" DESTINATION etc/)

  if(APPLE)
    set(optimica_tar_root "install")
    set(optimica_md5 "b62ecf0b9483c9af1e4bc1b628d50bc9")
  elseif(UNIX)
    set(optimica_tar_root "install")
    set(optimica_md5 "b62ecf0b9483c9af1e4bc1b628d50bc9")
  else()
    set(optimica_tar_root "install")
    set(optimica_md5 "b62ecf0b9483c9af1e4bc1b628d50bc9")
  endif()

  set(optimica_path "${PROJECT_BINARY_DIR}/Optimica/")

  download_modelica(
    ${optimica_path} # The final directory name
    ${optimica_tar_root} # The directory name of the tarball root, this will be moved / renamed to modelica_path
    ${optimica_md5} # The md5 of the tarball
    ${OPTIMICA_TARFILE_URL} # The download url. Can be a local file:// url
  )
endif()

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
  COMMAND ${CMAKE_COMMAND} -E copy ${idd_path} $<TARGET_FILE_DIR:spawn>
)

set(modelica_bin_path "${PROJECT_SOURCE_DIR}/submodules/modelica-buildings/Buildings/Resources/bin/${os_prefix}/bin/")
set(modelica_lib_path "${PROJECT_SOURCE_DIR}/submodules/modelica-buildings/Buildings/Resources/bin/${os_prefix}/lib/")
set(modelica_idd_path "${PROJECT_SOURCE_DIR}/submodules/modelica-buildings/Buildings/Resources/bin/${os_prefix}/etc/")

if(ENABLE_BUILDINGS_LIBRARY)
  include(ExternalProject)
  ExternalProject_Add(
    MBL
    BUILD_ALWAYS 1
    BUILD_IN_SOURCE
    SOURCE_DIR "${PROJECT_SOURCE_DIR}/submodules/modelica-buildings"
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config $<CONFIG>
    COMMAND ${CMAKE_COMMAND} -E make_directory ${modelica_bin_path}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${modelica_lib_path}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:spawn> ${idd_path} "${modelica_bin_path}"
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:epfmi> "${modelica_lib_path}"
    COMMAND ${CMAKE_COMMAND} -E copy ${idd_path} "${modelica_idd_path}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/examples/RefBldgSmallOfficeNew2004_Chicago"
            "${PROJECT_BINARY_DIR}/RefBldgSmallOfficeNew2004_Chicago")

  set(fmilib_project_source_dir "${PROJECT_SOURCE_DIR}/submodules/modelica-buildings/Buildings/Resources/src/fmi-library/")
  set(fmilib_project_build_dir "${fmilib_project_source_dir}/build/")

  install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/submodules/modelica-buildings"
    DESTINATION etc/
    PATTERN ${fmilib_project_build_dir} EXCLUDE
  )

  ExternalProject_Add(
    FMILIB
    SOURCE_DIR ${fmilib_project_source_dir}
    BINARY_DIR ${fmilib_project_build_dir}
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config $<CONFIG>
    INSTALL_COMMAND ""
  )
endif()

install(FILES ${PROJECT_BINARY_DIR}/EnergyPlus/Products/Energy+.idd DESTINATION etc/)

include(CPack)
