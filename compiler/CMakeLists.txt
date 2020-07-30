set(CMAKE_CXX_STANDARD 17)

string(REPLACE "git" "" LLVM_PACKAGE_VERSION ${LLVM_PACKAGE_VERSION})
message("LLVM VERISION: ${LLVM_PACKAGE_VERSION}")
message("LLVM INSTALL PREFIX: ${LLVM_INSTALL_PREFIX}")
set(CLANG_C_HEADERS_ROOT "${LLVM_INSTALL_PREFIX}/lib/clang/${LLVM_PACKAGE_VERSION}")
set(CLANG_C_HEADERS_DIR "${CLANG_C_HEADERS_ROOT}/include")
message("Possible header dir: ${CLANG_C_HEADERS_DIR}")

file(GLOB_RECURSE HEADER_FILES FOLLOW_SYMLINKS "${CLANG_C_HEADERS_DIR}/**")

foreach(_FILE ${HEADER_FILES})
  file(RELATIVE_PATH LOCATION "${CLANG_C_HEADERS_ROOT}" ${_FILE})
  list(APPEND FILES ${_FILE})
  list(APPEND EMBEDDED_PATHS ${LOCATION})
endforeach()

add_executable(spawn_exe_launcher spawn_exe_launcher.c)
target_link_libraries(spawn_exe_launcher ${CMAKE_DL_LIBS})

list(APPEND FILES "${CMAKE_CURRENT_BINARY_DIR}/spawn_exe_launcher")
list(APPEND EMBEDDED_PATHS "spawn_exe_launcher")

embed_files("${FILES}" "${EMBEDDED_PATHS}" EMBEDDED_OUTPUT spawnclang)

add_library(compiler compiler.hpp compiler.cpp utility.hpp utility.cpp ${EMBEDDED_OUTPUT})

target_include_directories(compiler PRIVATE "${CMAKE_BINARY_DIR}")

target_link_libraries(
  compiler
  PUBLIC
  libclang
  clangBasic
  clangLex
  clangParse
  clangCodeGen
  clangFrontend
  clangFrontendTool
  ${LLVM_LIBRARY_DIR}/liblldELF.a
  ${LLVM_LIBRARY_DIR}/liblldCommon.a
  ${LLVM_LIBRARY_DIR}/liblldCore.a
  ${LLVM_LIBRARY_DIR}/liblldDriver.a
  CONAN_PKG::boost
  CONAN_PKG::fmt
  CONAN_PKG::spdlog)

if(NOT MSVC)
  # necessary for making local symbols available to JIT
  target_link_libraries(compiler PUBLIC -rdynamic)
else()
  target_compile_definitions(compiler PUBLIC -DNOMINMAX)
endif()

