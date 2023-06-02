set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INSTALL_RPATH $ORIGIN/../lib)
cmake_policy(SET CMP0087 NEW)

set(src main.cpp)

add_executable(spawn ${src})

set(spawn_cli_name "spawn-${CMAKE_PROJECT_VERSION}-${CMAKE_PROJECT_VERSION_BUILD}")
set_target_properties(spawn PROPERTIES OUTPUT_NAME ${spawn_cli_name})

if (NOT WIN32)
  add_custom_command(
    TARGET spawn
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE_NAME:spawn> "spawn$<TARGET_FILE_SUFFIX:spawn>")
endif()

target_include_directories(spawn PRIVATE "${CMAKE_SOURCE_DIR}")
target_include_directories(spawn PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_include_directories(spawn PRIVATE "${PROJECT_BINARY_DIR}")

target_link_libraries(
  spawn
  PRIVATE compile_options
          fmu
          libspawn
          #energypluslib
          spawn_utils
          spawn_mbl
          spawn_optimica
          CONAN_PKG::cli11
          CONAN_PKG::nlohmann_json
          CONAN_PKG::fmt)

target_include_directories(spawn SYSTEM PRIVATE ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/)

#if(ENABLE_BUILDINGS_LIBRARY)
#  add_dependencies(spawn FMILIB)
#endif()

if(ENABLE_MODELICA_COMPILER OR ENABLE_COMPILER)
  target_link_libraries(spawn PRIVATE compiler )
endif()

if(ENABLE_MODELICA_COMPILER)
  target_link_libraries(spawn PRIVATE optimica)
endif()

include(${PROJECT_BINARY_DIR}/mbl/copy_spawn_to_mbl.cmake OPTIONAL)

add_custom_command(TARGET spawn POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${energyplus_idd_path} $<TARGET_FILE_DIR:spawn>
)

install(TARGETS spawn DESTINATION bin)
install(CODE "file(CREATE_LINK $<TARGET_FILE_NAME:spawn> \${CMAKE_INSTALL_PREFIX}/bin/spawn$<TARGET_FILE_SUFFIX:spawn> SYMBOLIC)")
