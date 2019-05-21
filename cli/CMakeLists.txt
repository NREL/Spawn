
set( src
  main.cpp
  ziputil.hpp
  ziputil.cpp
)

add_executable(spawn ${src})

configure_file(config.hpp.in config.hxx)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(spawn 
  CONAN_PKG::CLI11 
  CONAN_PKG::jsonformoderncpp 
  CONAN_PKG::pugixml 
  CONAN_PKG::boost_filesystem 
  CONAN_PKG::libzip
  epvariables
)

install(TARGETS spawn DESTINATION bin)
