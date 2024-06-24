# EnergyPlus will link to openmp dynamically by default
# We don't want that
set(ENABLE_OPENMP OFF CACHE BOOL "" FORCE)
set(USE_OpenMP OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC_ENERGYPLUS_API TRUE)

add_subdirectory(energyplus EXCLUDE_FROM_ALL)
include(energyplus/cmake/CompilerFlags.cmake)

# This is a method to capture the EnergyPlus version
# See notes in make_energyplus_version.cmake
execute_process(COMMAND ${CMAKE_COMMAND}
  -DPROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}/energyplus
  -DPROJECT_BINARY_DIR=${PROJECT_BINARY_DIR}/energyplus
  -DSpawn_SOURCE_DIR=${Spawn_SOURCE_DIR}
  -P ${PROJECT_SOURCE_DIR}/cmake/create_energyplus_version.cmake)
include("${PROJECT_BINARY_DIR}/EnergyPlus/energyplus_version.cmake")

# EnergyPlus is forcing CPACK_BINARY_IFW on but for Spawn we aren't using IFW so
# reset to OFF
set(CPACK_BINARY_IFW OFF)

set(energyplus_idd_path ${PROJECT_BINARY_DIR}/energyplus/Products/Energy+.idd)

install(FILES ${PROJECT_BINARY_DIR}/energyplus/Products/Energy+.idd DESTINATION etc/)
