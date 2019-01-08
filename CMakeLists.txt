project( Spawn )
cmake_minimum_required(VERSION 3.8)
include(ExternalProject)

if( APPLE ) 
  set( PLATFORM_INSTALL_PREFIX darwin64 )
elseif( WIN32 )
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "i386")
    set( PLATFORM_INSTALL_PREFIX win32 )
  else()
    set( PLATFORM_INSTALL_PREFIX win64 )
  endif()
elseif( LINUX )
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "i386")
    set( PLATFORM_INSTALL_PREFIX linux32 )
  else()
    set( PLATFORM_INSTALL_PREFIX linux64 )
  endif()
endif()

ExternalProject_Add(Ipopt
  GIT_REPOSITORY https://github.com/kbenne/Ipopt-3.12.4.git
  UPDATE_COMMAND ""
  CONFIGURE_COMMAND "../Ipopt/configure"
)

# On Apple JModelica needs to be compiled with GCC,
# because apples clang does not support openmp
# brew install gcc
# Note that /usr/bin/gcc provided by apple is just a link to clang and that will not work
if( APPLE )
  find_program( CC gcc-8 )
  if( NOT CC )
    message(FATAL_ERROR "gcc compiler is required but not found")
  endif()
  set( CC "CC=${CC}" )
endif()

find_library( BLAS blas )
if( NOT BLAS )
  message(FATAL_ERROR "blas is required but not found")
else()
  get_filename_component( BLAS ${BLAS} NAME )
  set( BLASLIB "BLASLIB=${BLAS}" )
endif()

ExternalProject_Add(JModelica
  DEPENDS Ipopt
  GIT_REPOSITORY https://github.com/kbenne/JModelica.git
  UPDATE_COMMAND ""
  CONFIGURE_COMMAND ../JModelica/configure ${CC} --with-ipopt=${PROJECT_BINARY_DIR}/Ipopt-prefix/src/Ipopt-build --prefix=${PROJECT_BINARY_DIR}/JModelica-prefix/src/JModelica-install
  BUILD_COMMAND $(MAKE) ${CC} ${BLASLIB}
  INSTALL_COMMAND $(MAKE) ${CC} ${BLASLIB} install
)

ExternalProject_Add(modelica-buildings
  DEPENDS JModelica
  GIT_REPOSITORY https://github.com/kbenne/modelica-buildings.git
  GIT_TAG cmake
  CMAKE_CACHE_ARGS
    -DMODELICA_HOME:PATH=${PROJECT_BINARY_DIR}/JModelica-prefix/src/JModelica-install
    -DCMAKE_INSTALL_PREFIX:PATH=${PROJECT_BINARY_DIR}/modelica-buildings-prefix/src/modelica-buildings-install
)

add_subdirectory( submodules/EnergyPlus EnergyPlus )
include( ${PROJECT_BINARY_DIR}/EnergyPlus/energyplus-exports.cmake )

set(BUILDINGS_EPFMI_PATH ${PROJECT_BINARY_DIR}/modelica-buildings-prefix/src/modelica-buildings-install/Buildings/Resources/Data/Experimental/EnergyPlus/EnergyPlus-9-0-0/lib/)
add_custom_target(
  BuildingsEPFMI ALL
  COMMAND ${CMAKE_COMMAND} -E copy
          $<TARGET_FILE:epfmi>
          ${BUILDINGS_EPFMI_PATH}
)

#
#add_subdirectory( submodules/modelica-buildings modelica-buildings )
#
#configure_file( test/config.h.in "${CMAKE_CURRENT_BINARY_DIR}/test-config.h.in2" )
#file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/test-config.h" INPUT "${CMAKE_CURRENT_BINARY_DIR}/test-config.h.in2")
#
#include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
#
#set( TEST_SRC
#  test/main.c
#)
#
#add_executable( test_runner ${TEST_SRC} )
#target_link_libraries( test_runner modelica-buildings )
#
#set( TEST2_SRC
#  test/main2.c
#)
#
#add_executable( test2_runner ${TEST2_SRC} )
#target_link_libraries( test2_runner modelica-buildings )


