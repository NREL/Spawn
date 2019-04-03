cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0025 NEW)

project( Spawn VERSION 0.0.1 )
cmake_minimum_required(VERSION 3.8)

set (CMAKE_CXX_STANDARD 11)

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
  BASIC_SETUP CMAKE_TARGETS NO_OUTPUT_DIRS
  BUILD missing
)

include( cmake/build-jmodelica.cmake )
add_subdirectory( submodules/EnergyPlus EnergyPlus )
add_subdirectory( cli )


