if(UNIX)
  add_definitions("-fPIC")
endif()

# Sets the variable "platform" corresponding to the FMI conventions
include(FMI-Platform.cmake)
fmi_platform(FMI_PLATFORM)

configure_file(config.cpp.in config.cxx)

add_library(
  spawn_utils
  strings.hpp
  strings.cpp
  filesystem.hpp
  fmi_paths.hpp
  temp_directory.cpp
  temp_directory.hpp
  unzipped_file.cpp
  unzipped_file.hpp
  dynamiclibrary.hpp
  dynamiclibrary_windows.hpp
  dynamiclibrary_posix.hpp
  unique_id.hpp
  unique_id.cpp
  ziputil.hpp
  ziputil.cpp
  datetime.hpp
  datetime.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/config.cxx
)

target_include_directories(
  spawn_utils 
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
  spawn_utils
  PUBLIC Boost::date_time libzip::zip fmt::fmt spdlog::spdlog compile_options)

include(CheckLinkerFlag)
check_linker_flag(CXX -lstdc++fs HAS_FS_FLAG)
if(HAS_FS_FLAG)
  target_link_libraries(spawn_utils PUBLIC stdc++fs)
endif()

add_executable(
  spawn_utils_tests
  test/spawn_utils_tests.cpp
  test/test_datetime.cpp
)

target_link_libraries(spawn_utils_tests PRIVATE Catch2::Catch2 spawn_utils)

catch_discover_tests(spawn_utils_tests TEST_PREFIX "spawn.")
