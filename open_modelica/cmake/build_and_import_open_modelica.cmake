add_library(
  OpenModelicaCompiler 
  SHARED
  IMPORTED
)

add_library(
  OpenModelicaRuntimeC 
  SHARED
  IMPORTED
)

add_library(
  omcgc 
  SHARED
  IMPORTED
)

set(omc_include_paths
  ${CMAKE_CURRENT_BINARY_DIR}/OpenModelica-prefix/src/OpenModelica-build/install_cmake/include
  ${CMAKE_CURRENT_BINARY_DIR}/OpenModelica-prefix/src/OpenModelica-build/install_cmake/include/omc/
)

# The target_include_directories need to actually exist in order to link to the target
file(MAKE_DIRECTORY ${omc_include_paths})

target_include_directories(
  OpenModelicaCompiler
  INTERFACE
  ${omc_include_paths}
)

target_compile_options(OpenModelicaCompiler INTERFACE -fpermissive)

include(ExternalProject)
ExternalProject_Add(
  OpenModelica
  BUILD_ALWAYS 1
  GIT_REPOSITORY https://github.com/OpenModelica/OpenModelica.git
  GIT_TAG v1.20.0
  GIT_SHALLOW
  GIT_SUBMODULES_RECURSE 1
  # CMAKE_ARGS -DBLA_STATIC=1
)

set_property(
  TARGET OpenModelicaCompiler
  PROPERTY IMPORTED_LOCATION 
  ${CMAKE_CURRENT_BINARY_DIR}/OpenModelica-prefix/src/OpenModelica-build/install_cmake/lib/${CMAKE_LIBRARY_ARCHITECTURE}/omc/${CMAKE_SHARED_LIBRARY_PREFIX}OpenModelicaCompiler${CMAKE_SHARED_LIBRARY_SUFFIX}
)

target_link_libraries(
  OpenModelicaCompiler
  INTERFACE
  Threads::Threads
)

set_property(
  TARGET OpenModelicaRuntimeC
  PROPERTY IMPORTED_LOCATION 
  ${CMAKE_CURRENT_BINARY_DIR}/OpenModelica-prefix/src/OpenModelica-build/install_cmake/lib/${CMAKE_LIBRARY_ARCHITECTURE}/omc/${CMAKE_SHARED_LIBRARY_PREFIX}OpenModelicaRuntimeC${CMAKE_SHARED_LIBRARY_SUFFIX}
)

target_link_libraries(
  OpenModelicaRuntimeC
  INTERFACE
  Threads::Threads
)

set_property(
  TARGET omcgc
  PROPERTY IMPORTED_LOCATION 
  ${CMAKE_CURRENT_BINARY_DIR}/OpenModelica-prefix/src/OpenModelica-build/install_cmake/lib/${CMAKE_LIBRARY_ARCHITECTURE}/omc/${CMAKE_SHARED_LIBRARY_PREFIX}omcgc${CMAKE_SHARED_LIBRARY_SUFFIX}
)

target_link_libraries(
  omcgc
  INTERFACE
  Threads::Threads
)

add_dependencies(OpenModelicaCompiler OpenModelica)
add_dependencies(OpenModelicaRuntimeC OpenModelica)
add_dependencies(omcgc OpenModelica)