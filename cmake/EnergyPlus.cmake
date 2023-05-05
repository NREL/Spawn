# EnergyPlus will link to openmp dynamically by default
# We don't want that
set(ENABLE_OPENMP OFF CACHE BOOL "" FORCE)
set(USE_OpenMP OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC_ENERGYPLUS_API TRUE)

add_subdirectory(submodules/EnergyPlus EnergyPlus EXCLUDE_FROM_ALL)
include(submodules/EnergyPlus/cmake/CompilerFlags.cmake)
# This is a method to capture the EnergyPlus version
# See notes in make_energyplus_version.cmake
execute_process(COMMAND ${CMAKE_COMMAND}
  -DPROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}/submodules/EnergyPlus
  -DPROJECT_BINARY_DIR=${PROJECT_BINARY_DIR}/EnergyPlus
  -DSpawn_SOURCE_DIR=${Spawn_SOURCE_DIR}
  -P ${PROJECT_SOURCE_DIR}/cmake/create_energyplus_version.cmake)
include("${PROJECT_BINARY_DIR}/EnergyPlus/energyplus_version.cmake")

# EnergyPlus is forcing CPACK_BINARY_IFW on but for Spawn we aren't using IFW so
# reset to OFF
set(CPACK_BINARY_IFW OFF)

install(FILES ${PROJECT_BINARY_DIR}/EnergyPlus/Products/Energy+.idd DESTINATION etc/)
