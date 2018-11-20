
project( Spawn )

add_subdirectory( submodules/EnergyPlus EnergyPlus )

add_subdirectory( submodules/ModelicaStandardLibrary ModelicaStandardLibrary )

add_subdirectory( submodules/modelica-buildings modelica-buildings )

#add_subdirectory( submodules/jmodelica/ThirdParty/Sundials/sundials-2.7.0 sundials )

add_subdirectory( submodules/jmodelica/RuntimeLibrary/src/jmi jmi )

set( TEST_SRC
  test/main.c
)

add_executable( test_runner ${TEST_SRC} )

target_link_libraries( test_runner modelica-buildings jmi)

