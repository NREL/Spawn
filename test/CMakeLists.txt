# automatically enable catch2 to generate ctest targets
if (CONAN_CATCH2_ROOT_DEBUG)
  include(${CONAN_CATCH2_ROOT_DEBUG}/lib/cmake/Catch2/Catch.cmake)
else ()
  include(${CONAN_CATCH2_ROOT}/lib/cmake/Catch2/Catch.cmake)
endif ()


configure_file(testpaths.in.cpp testpaths.in2.cpp)


include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(fmi_load_test MODULE fmi_load_test.cpp)

add_executable(tests
        spawn.hpp
        spawn.cpp
        testpaths.hpp
        temp_directory.cpp
        catch.cpp
        unzip_file.cpp
        fmi.cpp)



add_dependencies(tests fmi_load_test)
target_link_libraries(tests PRIVATE CONAN_PKG::catch2 spawn_utils fmu)


# Make this better
if(WIN32)
  # file(GENERATE...) helps us get the path the the spawn exe
  file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/$<UPPER_CASE:$<CONFIG>>/testpaths.cpp
          INPUT ${CMAKE_CURRENT_BINARY_DIR}/testpaths.in2.cpp
          )

  add_library(testpaths-release
          ${CMAKE_CURRENT_BINARY_DIR}/RELEASE/testpaths.cpp
          )

  add_library(testpaths-debug
          ${CMAKE_CURRENT_BINARY_DIR}/DEBUG/testpaths.cpp
          )

  target_link_libraries(testpaths-debug PUBLIC CONAN_PKG::boost)
  target_link_libraries(testpaths-release PUBLIC CONAN_PKG::boost)

  target_link_libraries(tests
          PRIVATE
          debug testpaths-debug
          general testpaths-release
          )
else()
  # file(GENERATE...) helps us get the path the the spawn exe
  file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/testpaths.cpp
          INPUT ${CMAKE_CURRENT_BINARY_DIR}/testpaths.in2.cpp
          )

  add_library(testpaths
          ${CMAKE_CURRENT_BINARY_DIR}/testpaths.cpp
          )

  target_link_libraries(testpaths PUBLIC CONAN_PKG::boost)

  target_link_libraries(tests
          PRIVATE
          testpaths
          )
endif()

catch_discover_tests(
        tests
        TEST_PREFIX
        "unittests.")
