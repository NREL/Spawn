find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

if(APPLE)
  set(jmodelica_extracted_path "${PROJECT_BINARY_DIR}/Modelica-0.1.1-Linux")
  set(jmodelica_tar "Modelica-latest.tar.gz")
  set(jmodelica_md5 "9798e0ca33160c5faefe3160d5a8b36b")
elseif(UNIX)
  set(jmodelica_extracted_path "${PROJECT_BINARY_DIR}/Modelica-0.1.1-Linux")
  set(jmodelica_tar "Modelica-latest.tar.gz")
  set(jmodelica_md5 "9798e0ca33160c5faefe3160d5a8b36b")
else()
  set(jmodelica_extracted_path "${PROJECT_BINARY_DIR}/Modelica-0.1.1-Linux")
  set(jmodelica_tar "Modelica-latest.tar.gz")
  set(jmodelica_md5 "9798e0ca33160c5faefe3160d5a8b36b")
endif()

set(jmodelica_url "https://modelica.s3.amazonaws.com/docker-build/${jmodelica_tar}")
set(jmodelica_download_path "${PROJECT_BINARY_DIR}/${jmodelica_tar}")
set(jmodelica_path "${PROJECT_BINARY_DIR}/JModelica/")

set(download_jmodelica TRUE)

if(EXISTS ${jmodelica_download_path})
  file(MD5 ${jmodelica_download_path} jmodelica_current_md5)
  if(${jmodelica_current_md5} EQUAL ${jmodelica_md5})
    if(EXISTS ${jmodelica_path})
      set(download_jmodelica FALSE)
    endif()
  endif()
endif()

if(download_jmodelica)
  file(DOWNLOAD ${jmodelica_url} ${jmodelica_download_path})
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xz ${jmodelica_download_path} WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

  execute_process(COMMAND ${CMAKE_COMMAND} -E rename ${jmodelica_extracted_path} ${jmodelica_path} WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
endif()

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

add_library(compiler compiler.hpp compiler.cpp ${EMBEDDED_OUTPUT})

target_include_directories(compiler PRIVATE "${CMAKE_SOURCE_DIR}")
target_include_directories(compiler PRIVATE "${CMAKE_BINARY_DIR}")
target_include_directories(compiler SYSTEM PUBLIC ${LLVM_INCLUDE_DIRS})
target_include_directories(compiler SYSTEM PUBLIC ${CLANG_INCLUDE_DIRS})

target_compile_features(compiler PRIVATE cxx_std_17)

target_link_libraries(
  compiler
  PUBLIC libclang
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
         CONAN_PKG::fmt
         CONAN_PKG::spdlog
  PRIVATE compile_options spawn_utils)

if(NOT MSVC)
  # necessary for making local symbols available to JIT
  target_link_libraries(compiler PUBLIC -rdynamic)

  if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message(
      SEND_ERROR
        "Currently we cannot link with EnergyPlus and Clang in Debug mode on Unix because EnergyPlus is defining _GLIBCXX_DEBUG whichh conflicts with build settings for libclang"
    )
  endif()
else()
  target_compile_definitions(compiler PUBLIC -DNOMINMAX)
endif()
