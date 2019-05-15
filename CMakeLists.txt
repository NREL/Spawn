cmake_policy(SET CMP0048 NEW)

project( Spawn VERSION 0.0.1 )
cmake_minimum_required(VERSION 3.8)

add_subdirectory( submodules/EnergyPlus EnergyPlus )

cmake_policy(SET CMP0025 NEW)
set (CMAKE_CXX_STANDARD 11)

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
  boost_filesystem/1.69.0@bincrafters/stable
  jsonformoderncpp/3.6.1@vthiery/stable
  pugixml/1.9@bincrafters/stable
  CLI11/1.7.1@cliutils/stable
  libzip/1.5.1@bincrafters/stable
  BASIC_SETUP CMAKE_TARGETS NO_OUTPUT_DIRS
  BUILD missing
)

configure_file(README.md README.md)
install(FILES ${PROJECT_BINARY_DIR}/README.md DESTINATION .)

add_subdirectory( cli )

add_custom_target(
  CopyEPFMI ALL
  COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE:epfmi>
          $<TARGET_FILE_DIR:spawn>
)

install(DIRECTORY examples DESTINATION .)

INCLUDE(CPack)

