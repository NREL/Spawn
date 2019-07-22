project(cli)

set( src
  main.cpp
  ziputil.hpp
  ziputil.cpp
)

add_executable(spawn ${src})

configure_file(config.hpp.in config.hxx)

include_directories(${PROJECT_BINARY_DIR})
# This is needed until cmake version 3.11 at which point
# modelica library target will bring its own include path
include_directories(${PROJECT_BINARY_DIR}/../modelica/)

target_link_libraries(spawn 
  CONAN_PKG::CLI11 
  CONAN_PKG::jsonformoderncpp 
  CONAN_PKG::pugixml 
  CONAN_PKG::boost_filesystem 
  CONAN_PKG::libzip
  epvariables
  modelica
)

install(TARGETS spawn DESTINATION bin)

