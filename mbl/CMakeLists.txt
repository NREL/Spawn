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
  set(idd_path ${PROJECT_BINARY_DIR}/EnergyPlus/Products/Energy+.idd)

  include(ExternalProject)
  ExternalProject_Add(
    MBL
    BUILD_ALWAYS 1
    BUILD_IN_SOURCE
    GIT_REPOSITORY https://github.com/lbl-srg/modelica-buildings.git
    GIT_TAG d34a259960
    GIT_SHALLOW
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config $<CONFIG>
    COMMAND ${CMAKE_COMMAND} -E make_directory ${mbl_spawn_bin_path}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${mbl_spawn_lib_path}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${mbl_spawn_idd_path}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:spawn> ${idd_path} "${mbl_spawn_bin_path}"
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:epfmi> "${mbl_spawn_lib_path}"
    COMMAND ${CMAKE_COMMAND} -E copy ${idd_path} "${mbl_spawn_idd_path}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/examples/RefBldgSmallOfficeNew2004_Chicago"
            "${PROJECT_BINARY_DIR}/RefBldgSmallOfficeNew2004_Chicago"
    )

  # The full spawn cli name is embedded in the MBL, here we update the name with the latest sha

  set(MBL_ENERGYPLUS_VERSION_STRING "EnergyPlus_${ENERGYPLUS_VERSION_MAJOR}_${ENERGYPLUS_VERSION_MINOR}_${ENERGYPLUS_VERSION_PATCH}")
  set(building_mo_path "${mbl_home}/ThermalZones/${MBL_ENERGYPLUS_VERSION_STRING}/Building.mo")
  file(READ ${building_mo_path} building_mo_text)
  string(REGEX REPLACE "spawn-[0-9]\.[0-9]\.[0-9]-.........." ${spawn_cli_name} new_building_mo_text "${building_mo_text}")
  file(WRITE ${building_mo_path} "${new_building_mo_text}")

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
