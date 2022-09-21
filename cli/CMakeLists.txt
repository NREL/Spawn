set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INSTALL_RPATH $ORIGIN/../lib)
cmake_policy(SET CMP0087 NEW)

set(src main.cpp)

if(ENABLE_COMPILER OR ENABLE_MODELICA_COMPILER)
  if(ENABLE_MODELICA_COMPILER)
    include(embed_jmodelica.cmake)
    embed_jmodelica(${jmodelica_path} jmodelica_files jmodelica_paths)

    include(embed_optimica.cmake)
    embed_optimica(${optimica_path} optimica_files optimica_paths)
  endif()

  #include(embed_libc.cmake)
  #embed_libc(libc_files libc_paths)

  list(APPEND embedded_files ${jmodelica_files} ${optimica_files}) # ${libc_files})
  list(APPEND embedded_paths ${jmodelica_paths} ${optimica_paths}) # ${libc_paths})


  set(embedded_files $<TARGET_FILE:spawn_exe_launcher>)

  if (MSVC)
    set(embedded_paths "spawn_exe_launcher")
  else()
    set(embedded_paths "spawn_exe_launcher.exe")
  endif()

  if(ENABLE_BUILDINGS_LIBRARY)
    list(
      APPEND embedded_files
      "${CMAKE_SOURCE_DIR}/submodules/modelica-buildings/Buildings/Resources/src/fmi-library/build/fmi-library-modelon/src/install/lib/libfmilib.a")
    list(APPEND embedded_paths "usr/lib/libfmilib.a")
  endif()

  list(APPEND embedded_files ${GFORTRANLIB_REAL_PATH})
  list(APPEND embedded_paths ${GFORTRANLIB_EMBEDDED_PATH})

  embed_files("${embedded_files}" "${embedded_paths}" embedded_output spawnmodelica)

  list(APPEND src compile.hpp compile.cpp ${embedded_output})
endif()

add_executable(spawn ${src})

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
          CONAN_PKG::cli11
          CONAN_PKG::nlohmann_json
          CONAN_PKG::fmt
          CONAN_PKG::spdlog)

target_include_directories(spawn SYSTEM PRIVATE ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/)

if(ENABLE_BUILDINGS_LIBRARY)
  add_dependencies(spawn FMILIB)
endif()

if(ENABLE_MODELICA_COMPILER OR ENABLE_COMPILER)
  target_link_libraries(spawn PRIVATE compiler )
endif()

if(ENABLE_MODELICA_COMPILER)
  target_link_libraries(spawn PRIVATE jmodelica optimica)
endif()

install(TARGETS spawn DESTINATION bin)
install(CODE "file(CREATE_LINK $<TARGET_FILE_NAME:spawn> \${CMAKE_INSTALL_PREFIX}/bin/spawn$<TARGET_FILE_SUFFIX:spawn> SYMBOLIC)")
