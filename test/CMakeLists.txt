configure_file(testpaths.in.cpp testpaths.in2.cpp)

# file(GENERATE...) helps us get the path the the spawn exe
# UPPER_CASE is required because CONFIG casing is not consistent
# between the file command and the add_executable command
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/$<UPPER_CASE:$<CONFIG>>/testpaths.cpp
  INPUT ${CMAKE_CURRENT_BINARY_DIR}/testpaths.in2.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(test
  spawn.cpp
  testpaths.hpp
)

# Make this better
if(WIN32)
	add_library(testpaths-release
	  ${CMAKE_CURRENT_BINARY_DIR}/RELEASE/testpaths.cpp
	)

	add_library(testpaths-debug
	  ${CMAKE_CURRENT_BINARY_DIR}/DEBUG/testpaths.cpp
	)

	target_link_libraries(test
		CONAN_PKG::catch2
	  debug testpaths-debug
	  general testpaths-release
	)
else()
	add_library(testpaths
	  ${CMAKE_CURRENT_BINARY_DIR}/testpaths.cpp
	)

	target_link_libraries(test
		CONAN_PKG::catch2
	  testpaths
	)
endif()

