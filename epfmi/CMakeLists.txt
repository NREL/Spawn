set(src EPFMI.hpp EPFMI.cpp EPFMI_Export.hpp)

add_library(epfmi SHARED ${src})
set_target_properties(epfmi PROPERTIES OUTPUT_NAME "epfmi")
set_target_properties(epfmi PROPERTIES PREFIX "")
target_link_libraries(
  epfmi
  PUBLIC libspawn
  PRIVATE compile_options)
target_include_directories(epfmi PUBLIC ${PROJECT_SOURCE_DIR}/fmi2/)
target_include_directories(epfmi SYSTEM PRIVATE ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/
                                                ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/btwxt/src/)

install(
  TARGETS epfmi
  RUNTIME DESTINATION ./bin
  LIBRARY DESTINATION ./lib
  ARCHIVE DESTINATION ./lib)
