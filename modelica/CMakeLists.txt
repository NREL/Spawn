if(ENABLE_COMPILER)

add_library(spawn_modelica
  modelica_engine.hpp
  create_exe.hpp
  create_exe.cpp
  create_fmu.hpp
  create_fmu.cpp
  simulate.hpp
  simulate.cpp
)

target_include_directories(spawn_modelica
  PUBLIC
  ${PROJECT_SOURCE_DIR}
)

target_link_libraries(spawn_modelica
  PUBLIC
  spawn_open_modelica
)

endif()
