
project( Spawn )

add_subdirectory( submodules/EnergyPlus EnergyPlus )

add_subdirectory( submodules/ModelicaStandardLibrary ModelicaStandardLibrary )

add_subdirectory( submodules/modelica-buildings modelica-buildings )

#add_subdirectory( submodules/jmodelica/ThirdParty/Sundials/sundials-2.7.0 sundials )

add_subdirectory( submodules/jmodelica/RuntimeLibrary/src/jmi jmi )

configure_file( test/config.h.in "${CMAKE_CURRENT_BINARY_DIR}/test-config.h.in2" )
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/test-config.h" INPUT "${CMAKE_CURRENT_BINARY_DIR}/test-config.h.in2")

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

set( TEST_SRC
  test/main.c
)

add_executable( test_runner ${TEST_SRC} )

target_link_libraries( test_runner modelica-buildings )

