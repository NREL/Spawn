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

add_subdirectory( submodules/EnergyPlus EnergyPlus )

if( UNIX )

  ExternalProject_Add(Ipopt
    GIT_REPOSITORY https://github.com/kbenne/Ipopt-3.12.4.git
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND "../Ipopt/configure"
  )
  
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
    BUILD_COMMAND make ${CC} ${BLASLIB}
    INSTALL_COMMAND make ${CC} ${BLASLIB} install
  )
  set(MODELICA_HOME ${PROJECT_BINARY_DIR}/JModelica-prefix/src/JModelica-install)
  
  add_subdirectory( submodules/modelica-buildings modelica-buildings )
  
  set(EPFMI_PATH ${PROJECT_BINARY_DIR}/JModelica-prefix/src/JModelica-install/ThirdParty/MSL/Modelica/Resources/Library/${PLATFORM_INSTALL_PREFIX}/)
  add_custom_target(
    CopyEPFMI ALL
    COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:epfmi>
            ${EPFMI_PATH}
  )
  
  set(BUILDINGS_LIB_PATH ${PROJECT_BINARY_DIR}/JModelica-prefix/src/JModelica-install/ThirdParty/MSL/Modelica/Resources/Library/${PLATFORM_INSTALL_PREFIX}/)
  add_custom_target(
    CopyModelicaBuildingsEnergyPlus ALL
    COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:ModelicaBuildingsEnergyPlus>
            ${BUILDINGS_LIB_PATH}
  )
  
  configure_file(test/test.py.in test.py)

endif()

