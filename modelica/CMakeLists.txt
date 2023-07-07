if(ENABLE_MODELICA_COMPILER)

add_library(spawn_modelica
  modelica_engine.hpp
  create_fmu.hpp
  create_fmu.cpp
)

target_include_directories(spawn_modelica
  PUBLIC
  ${PROJECT_SOURCE_DIR}
)

target_link_libraries(spawn_modelica
  PUBLIC
  spawn_optimica
)

endif()
