cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0025 NEW)

project( Spawn VERSION 0.0.1 )
cmake_minimum_required(VERSION 3.13)
option(INSTALL_BUILDINGS_LIBRARY "Include the Modelica Buildings Library in the installer package" FALSE)
set(CMAKE_CXX_STANDARD 11)

add_subdirectory( submodules/EnergyPlus EnergyPlus EXCLUDE_FROM_ALL )
include(submodules/EnergyPlus/cmake/CompilerFlags.cmake)

if(NOT GIT_FOUND)
  find_program(GIT_EXECUTABLE git HINTS "$ENV{LOCALAPPDATA}/Programs/git/bin")
  if (NOT GIT_EXECUTABLE_NOTFOUND)
    set(GIT_FOUND TRUE)
  endif()
endif()

if(GIT_FOUND)
  execute_process(COMMAND "${GIT_EXECUTABLE}" "rev-parse" "--short=10" "HEAD"
                  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                  TIMEOUT 10
                  RESULT_VARIABLE RESULT
                  OUTPUT_VARIABLE GIT_VERSION
                  ERROR_QUIET
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(${RESULT} EQUAL 0 AND NOT "${GIT_VERSION}" EQUAL "${CMAKE_PROJECT_VERSION_BUILD}")
    set(CMAKE_PROJECT_VERSION_BUILD ${GIT_VERSION} CACHE STRING "Build number" FORCE) # git sha
  endif()

  get_filename_component(GIT_DIR "${GIT_EXECUTABLE}" PATH)
else()
  set(GIT_DIR "")
endif()

set(CPACK_PACKAGE_VERSION "${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}-${CMAKE_PROJECT_VERSION_BUILD}")

# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.13/conan.cmake"
    "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_check(VERSION 1.0.0 REQUIRED)

conan_add_remote(NAME bincrafters
  URL https://api.bintray.com/conan/bincrafters/public-conan)

conan_cmake_run(REQUIRES
  bzip2/1.0.8@conan/stable
  zlib/1.2.11@conan/stable
  boost/1.69.0
  libzip/1.5.2@bincrafters/stable
  pugixml/1.9@bincrafters/stable
  CLI11/1.7.1@cliutils/stable
  catch2/2.11.3
  BASIC_SETUP CMAKE_TARGETS NO_OUTPUT_DIRS
  BUILD missing
)

###############################################################################
# RPATH settings
# Conan set this to true, which is not the cmake default and not what we want
# for the openstudio targets
set(CMAKE_SKIP_RPATH FALSE)
###############################################################################

configure_file(README.md README.md)
install(FILES ${PROJECT_BINARY_DIR}/README.md DESTINATION .)

add_subdirectory( lib )
add_subdirectory( cli )
add_subdirectory( epfmi )
add_subdirectory( test )

if( APPLE )
  set(os_prefix spawn-darwin64)
elseif( WIN32 )
  set(os_prefix spawn-win32)
elseif( UNIX AND NOT APPLE )
  set(os_prefix spawn-linux64)
endif()

set(modelica_bin_path "modelica-buildings/Buildings/Resources/bin/${os_prefix}/bin/")
set(modelica_lib_path "modelica-buildings/Buildings/Resources/bin/${os_prefix}/lib/")
set(idd_path "${PROJECT_BINARY_DIR}/EnergyPlus/Products/Energy+.idd")

add_custom_target(
  Stage ALL
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:epfmi> $<TARGET_FILE_DIR:spawn>
  COMMAND ${CMAKE_COMMAND} -E copy ${idd_path} $<TARGET_FILE_DIR:spawn>
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${PROJECT_SOURCE_DIR}/submodules/modelica-buildings"
          "${PROJECT_BINARY_DIR}/modelica-buildings"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/${modelica_bin_path}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/${modelica_lib_path}"
  COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE:spawn>
          ${idd_path}
          "${modelica_bin_path}"
  COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE:epfmi>
          "${modelica_lib_path}"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/examples/RefBldgSmallOfficeNew2004_Chicago" "${PROJECT_BINARY_DIR}/RefBldgSmallOfficeNew2004_Chicago"
)
if( INSTALL_BUILDINGS_LIBRARY )
  install(DIRECTORY submodules/modelica-buildings DESTINATION .)
  install(TARGETS spawn epfmi DESTINATION ${modelica_bin_path})
endif()

install(DIRECTORY examples DESTINATION .)
install(FILES ${PROJECT_BINARY_DIR}/EnergyPlus/Products/Energy+.idd DESTINATION etc/ )

INCLUDE(CPack)

