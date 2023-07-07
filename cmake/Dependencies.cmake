# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/release/0.16/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(
  REQUIRES
  bzip2/1.0.8
  zlib/1.2.13
  libzip/1.7.3
  pugixml/1.11
  cli11/1.9.1
  catch2/2.13.9
  fmt/8.0.1
  nlohmann_json/3.10.2
  spdlog/1.9.2
  swig/4.1.0
  BASIC_SETUP
  CMAKE_TARGETS
  NO_OUTPUT_DIRS
  BUILD
  missing)

# ##############################################################################
# RPATH settings Conan set this to true, which is not the cmake default and not
# what we want for the openstudio targets
set(CMAKE_SKIP_RPATH FALSE)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# automatically enable catch2 to generate ctest targets
if(CONAN_CATCH2_ROOT_DEBUG)
  include(${CONAN_CATCH2_ROOT_DEBUG}/lib/cmake/Catch2/Catch.cmake)
else()
  include(${CONAN_CATCH2_ROOT}/lib/cmake/Catch2/Catch.cmake)
endif()
