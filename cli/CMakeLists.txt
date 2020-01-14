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

#file(GLOB_RECURSE HEADER_FILES FOLLOW_SYMLINKS "${CLANG_C_HEADERS_DIR}/**")
#
#foreach(_FILE ${HEADER_FILES})
#  file(RELATIVE_PATH LOCATION "${CLANG_C_HEADERS_ROOT}" ${_FILE})
#  list(APPEND FILES ${_FILE})
#  list(APPEND EMBEDDED_PATHS ${LOCATION})
#endforeach()
#
#embed_files("${FILES}" "${EMBEDDED_PATHS}" EMBEDDED_OUTPUT spawnmo)

target_link_libraries(spawn 
  CONAN_PKG::CLI11 
  CONAN_PKG::jsonformoderncpp 
  CONAN_PKG::pugixml 
  CONAN_PKG::boost 
  CONAN_PKG::libzip
  LLVMOrcJIT
  compiler
  epvariables
  modelica
)

install(TARGETS spawn DESTINATION bin)

