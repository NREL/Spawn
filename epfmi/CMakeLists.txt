
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/fmi2/ )
#INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} )
#INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )
#INCLUDE_DIRECTORIES( ${PROJECT_BINARY_DIR}/submodules/EnergyPlus/ )
#INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/ )
#INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/ )
#INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus )
#INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus/public )
#INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/ )
#INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/ObjexxFCL/src/ )
#INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/FMI2/ )
#INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/CLI/ )

set(src
  EPFMI.hpp
  EPFMI.cpp
  EPFMI_Export.hpp
)

add_library( epfmi SHARED ${src} )
set_target_properties( epfmi PROPERTIES OUTPUT_NAME "epfmi" )
target_link_libraries( epfmi libspawn)

install( TARGETS epfmi
  RUNTIME DESTINATION ./bin
  LIBRARY DESTINATION ./lib
  ARCHIVE DESTINATION ./lib
)

set(test_src
  Test/Single_Model_Test.cpp
  Test/main.cc
)

configure_file( Test/config.h.in "${CMAKE_CURRENT_BINARY_DIR}/test-config.h" )

CREATE_TEST_TARGETS( epfmi "${test_src}" "epfmi" )

set(MY_FMU_RESOURCES "${CMAKE_CURRENT_BINARY_DIR}/MyFMU/resources/")
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${MY_FMU_RESOURCES}")
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "${PROJECT_SOURCE_DIR}/examples/TwoIdenticalZones/TwoIdenticalZones.spawn"
  "${PROJECT_SOURCE_DIR}/examples/TwoIdenticalZones/TwoIdenticalZones.idf"
  "${PROJECT_SOURCE_DIR}/examples/TwoIdenticalZones/USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw"
  "${PROJECT_BINARY_DIR}/Products/Energy+.idd"
  "${MY_FMU_RESOURCES}/"
)

