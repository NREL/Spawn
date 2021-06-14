set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INSTALL_RPATH $ORIGIN/../lib)

set(src main.cpp)

if(ENABLE_MODELICA_COMPILER)
  include(embed_jmodelica.cmake)
  embed_jmodelica(
    ${jmodelica_path}
    jmodelica_files
    jmodelica_paths
  )

  include(embed_optimica.cmake)
  embed_optimica(
    ${optimica_path}
    optimica_files
    optimica_paths
  )

  #file(GLOB_RECURSE msl_files FOLLOW_SYMLINKS
  #  "${jmodelica_path}/ThirdParty/MSL/**"
  #)
  #
  #foreach(filepath ${msl_files})
  #  file(RELATIVE_PATH embedded_path "${jmodelica_path}/../" ${filepath})
  #  list(APPEND embedded_files ${filepath})
  #  list(APPEND embedded_paths ${embedded_path})
  #endforeach()

  # don't embed MBL for now
  #set(mbl_path "${CMAKE_BINARY_DIR}/modelica-buildings")
  #file(GLOB_RECURSE mbl_files FOLLOW_SYMLINKS
  #  "${mbl_path}/**"
  #)
  #
  #foreach(filepath ${mbl_files})
  #  file(RELATIVE_PATH embedded_path "${mbl_path}/../" ${filepath})
  #  list(APPEND embedded_files ${filepath})
  #  list(APPEND embedded_paths ${embedded_path})
  #endforeach()

  list(APPEND embedded_files ${jmodelica_files} ${optimica_files})
  list(APPEND embedded_paths ${jmodelica_paths} ${optimica_paths})
  embed_files("${embedded_files}" "${embedded_paths}" embedded_output spawnmodelica)

  list(APPEND src compilerchain.hpp compilerchain.cpp ${embedded_output})
endif()

add_executable(spawn ${src})

configure_file(config.hpp.in config.hxx)
target_include_directories(spawn PRIVATE "${CMAKE_SOURCE_DIR}")
target_include_directories(spawn PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_include_directories(spawn PRIVATE "${PROJECT_BINARY_DIR}")

target_link_libraries(
  spawn
  PRIVATE compile_options
          libspawn
          energypluslib
          spawn_utils
          CONAN_PKG::cli11
          CONAN_PKG::nlohmann_json
          CONAN_PKG::spdlog
)

if(ENABLE_MODELICA_COMPILER)
  target_link_libraries(spawn PRIVATE compiler jmodelica optimica)
endif()

install(TARGETS spawn DESTINATION bin)
