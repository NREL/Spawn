
set( src
  main.cpp
)

add_executable(spawn ${src})

target_link_libraries(spawn CONAN_PKG::CLI11 CONAN_PKG::jsonformoderncpp CONAN_PKG::pugixml CONAN_PKG::boost_filesystem)

