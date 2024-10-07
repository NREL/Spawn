set(src
  EPFMI.hpp
  EPFMI.cpp
  EPFMI_Export.hpp
  paths.hpp
  paths.cpp
)

add_library(epfmi SHARED ${src})
set_target_properties(epfmi PROPERTIES OUTPUT_NAME "epfmi")
set_target_properties(epfmi PROPERTIES PREFIX "")
target_link_libraries(
  epfmi
  PUBLIC libspawn
  PRIVATE compile_options)
target_include_directories(epfmi PUBLIC ${PROJECT_SOURCE_DIR}/fmi2/)
target_include_directories(epfmi SYSTEM PRIVATE ${PROJECT_SOURCE_DIR}/energyplus/third_party/
                                                ${PROJECT_SOURCE_DIR}/energyplus/third_party/btwxt/src/)

install(
  TARGETS epfmi
  RUNTIME DESTINATION ./bin
  LIBRARY DESTINATION ./lib
  ARCHIVE DESTINATION ./lib)

include(${PROJECT_BINARY_DIR}/mbl/copy_epfmi_to_mbl.cmake OPTIONAL)

add_custom_command(TARGET epfmi POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:epfmi> $<TARGET_FILE_DIR:spawn>
)
