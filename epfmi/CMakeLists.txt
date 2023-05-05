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

set(idd_path "${PROJECT_BINARY_DIR}/EnergyPlus/Products/Energy+.idd")
add_custom_target(
  copy_spawn_cli_depends ALL
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:epfmi> $<TARGET_FILE_DIR:spawn>
  COMMAND ${CMAKE_COMMAND} -E copy ${idd_path} $<TARGET_FILE_DIR:spawn>)
