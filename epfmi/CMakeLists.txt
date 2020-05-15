
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/fmi2/ )
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/ )

set(src
  EPFMI.hpp
  EPFMI.cpp
  EPFMI_Export.hpp
)

add_library( epfmi SHARED ${src} )
set_target_properties( epfmi PROPERTIES OUTPUT_NAME "epfmi" )
set_target_properties( epfmi PROPERTIES PREFIX "" )
target_link_libraries( epfmi libspawn)

install( TARGETS epfmi
  RUNTIME DESTINATION ./bin
  LIBRARY DESTINATION ./lib
  ARCHIVE DESTINATION ./lib
)

