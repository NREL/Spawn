if(ENABLE_BUILDINGS_LIBRARY)
  if(APPLE)
    set(os_prefix darwin64)
  elseif(WIN32)
    set(os_prefix win64)
  elseif(UNIX AND NOT APPLE)
    set(os_prefix linux64)
  endif()

  get_target_property(spawn_cli_name spawn OUTPUT_NAME)
  
  set(mbl_dir "${PROJECT_BINARY_DIR}/mbl/MBL-prefix/src/MBL")
  set(mbl_home "${mbl_dir}/Buildings")
  set(mbl_spawn_root_path "${mbl_home}/Resources/bin/Spawn-light-${CMAKE_PROJECT_VERSION}-${CMAKE_PROJECT_VERSION_BUILD}/${os_prefix}")
  set(mbl_spawn_bin_path "${mbl_spawn_root_path}/bin/")
  set(mbl_spawn_lib_path "${mbl_spawn_root_path}/lib/")
  set(mbl_spawn_idd_path "${mbl_spawn_root_path}/etc/")
  set(MBL_ENERGYPLUS_VERSION_STRING "EnergyPlus_${ENERGYPLUS_VERSION_MAJOR}_${ENERGYPLUS_VERSION_MINOR}_${ENERGYPLUS_VERSION_PATCH}")

  include(ExternalProject)
  ExternalProject_Add(
    MBL
    BUILD_ALWAYS 1
    BUILD_IN_SOURCE
    GIT_REPOSITORY https://github.com/lbl-srg/modelica-buildings.git
    GIT_TAG e91e28d66955efc71285cc294e2589cd1505f70c
    GIT_SHALLOW
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config $<CONFIG>
    COMMAND ${CMAKE_COMMAND}
            -D MBL_ENERGYPLUS_VERSION_STRING=${MBL_ENERGYPLUS_VERSION_STRING}
            -D spawn_cli_name=${spawn_cli_name}
            -D mbl_home=${mbl_home}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/update_mbl_version.cmake
    )

  # cmake custom targets with post_build rules are used to copy spawn, epfmi, and the idd
  # into the MBL directory. CMake requires that post_build rules reside in the same directory
  # as the targets they depend on. This is the workaround to decouple MBL build rules 
  # (and keep MBL an optional component)
  configure_file(cmake/copy_spawn_to_mbl.cmake.in copy_spawn_to_mbl.cmake)
  configure_file(cmake/copy_epfmi_to_mbl.cmake.in copy_epfmi_to_mbl.cmake)

  set(fmilib_project_source_dir "${mbl_home}/Resources/src/fmi-library/")
  set(fmilib_project_build_dir "${fmilib_project_source_dir}/build/")
  set(fmilib_name "${CMAKE_STATIC_LIBRARY_PREFIX}fmilib${CMAKE_STATIC_LIBRARY_SUFFIX}")
  set(fmilib_build_path "${mbl_home}/Resources/src/fmi-library/build/fmi-library-modelon/src/fmi-library-modelon-build/${fmilib_name}")
  set(fmilib_install_path "Resources/Library/${os_prefix}/${fmilib_name}")

  install(
    DIRECTORY "${mbl_dir}"
    DESTINATION etc/
    PATTERN ${fmilib_project_build_dir} EXCLUDE)

  ExternalProject_Add(
    FMILIB
    DEPENDS MBL
    SOURCE_DIR ${fmilib_project_source_dir}
    BINARY_DIR ${fmilib_project_build_dir}
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config $<CONFIG>
    COMMAND ${CMAKE_COMMAND} -E copy ${fmilib_build_path} "${mbl_home}/${fmilib_install_path}"
    INSTALL_COMMAND "")

  configure_file(config.cpp.in config.cxx)

  add_library(
    spawn_mbl
    config.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/config.cxx
  )

  target_link_libraries(spawn_mbl PRIVATE spawn_utils compile_options)

  configure_file(test/models/Zone_Multiplier.mo.in test/models/Zone_Multiplier.mo)
endif()
