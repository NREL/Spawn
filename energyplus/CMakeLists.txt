if(UNIX)
  add_definitions("-fPIC")
endif()

add_library(
  libspawn
  actuatortypes.hpp
  fmugenerator.hpp
  fmugenerator.cpp
  idfprep.hpp
  idfprep.cpp
  idf_to_json.hpp
  idf_to_json.cpp
  manager.hpp
  manager.cpp
  outputtypes.hpp
  spawn.hpp
  spawn.cpp
  units.hpp
  units.cpp
  variables.hpp
  variables.cpp

  warmupmanager.hpp
  warmupmanager.cpp

  input/emsactuator.hpp
  input/emsactuator.cpp
  input/input.hpp
  input/input.cpp
  input/outputvariable.hpp
  input/outputvariable.cpp
  input/schedule.hpp
  input/schedule.cpp
  input/surface.hpp
  input/surface.cpp
  input/zone.hpp
  input/zone.cpp)

target_include_directories(
  libspawn SYSTEM # We want to enable more warnings and not have E+ affect us
  PUBLIC ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/ ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus/Data
         ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/ObjexxFCL/src/
  PRIVATE ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/
          ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/
          ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus
          ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus/public
          ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/ObjexxFCL/src/
          ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/FMI2/
          ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/CLI/
          ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/btwxt/src/
          ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/re2/
          ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/SQLite/
          ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/kiva-ep/src/
          ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/
          ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/eigen/
          ${PROJECT_BINARY_DIR}/EnergyPlus/third_party/kiva-ep/src/libkiva/)

# ##############################################################################
# S W I G                               #
# ##############################################################################

if(ENABLE_PYTHON_BINDINGS)
  set(target_name libspawn)

  # lib swig files
  set(${target_name}_swig_src libspawn.i)

  foreach(SWIG_FILE ${${target_name}_swig_src})
    set_source_files_properties(${SWIG_FILE} PROPERTIES CPLUSPLUS ON)
  endforeach()

  set(SWIG_ROOT ${CONAN_SWIG_ROOT}/bin)
  find_package(SWIG 4.0 REQUIRED COMPONENTS python)
  # The swig cmake finder calls the swig executable with -swiglib to locate the swig library path.
  # Unfortunately the Mac conan build of swig 4.0.2 returns a non portable path that is only valid on the 
  # machine where the swig binary was created, ie:
  # /Users/jenkins/w/BuildSingleReference@3/.conan/data/swig/4.0.2/_/_/package/099d7b9cd06e9bd11e92b9a2ddf3b29cd986fdcb/bin/swiglib
  # The following is a workaround...
  if(APPLE)
    set(new_swig_dir "${SWIG_EXECUTABLE}/../swiglib")
    cmake_path(NORMAL_PATH new_swig_dir OUTPUT_VARIABLE SWIG_DIR)
  endif()
  include(${SWIG_USE_FILE})


  set(swig_target_name ${target_name}_python)

  find_package(
    Python
    COMPONENTS Interpreter Development
    REQUIRED)

  set(output_dir "${CMAKE_CURRENT_BINARY_DIR}/Products/python")


  swig_add_library(
    ${swig_target_name}
    TYPE MODULE
    LANGUAGE python OUTPUT_DIR "${output_dir}"
                    OUTFILE_DIR "${CMAKE_CURRENT_BINARY_DIR}/python_wrapper"
    SOURCES ${${target_name}_swig_src})

  set_target_properties(${swig_target_name} PROPERTIES OUTPUT_NAME
    ${target_name})

  set_target_properties(
    ${swig_target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY
    "${output_dir}")
  set_target_properties(
    ${swig_target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY
    "${output_dir}")
  set_target_properties(
    ${swig_target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
    "${output_dir}")

  swig_link_libraries(${swig_target_name} PUBLIC ${target_name}  PRIVATE
    compile_options
    )

  target_include_directories(${swig_target_name} PRIVATE SYSTEM
    ${CMAKE_CURRENT_SOURCE_DIR})

  swig_link_libraries(${swig_target_name} PRIVATE Python::Module)

  configure_file(${CMAKE_SOURCE_DIR}/python/test_python.in.py test_python.py)

  add_test(
    NAME spawn.python_bindings
    #    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/python
    COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/test_python.py)
  
  set_tests_properties( spawn.python_bindings PROPERTIES ENVIRONMENT  PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/Products/python/$<IF:$<CONFIG:Debug>,Debug,Release>)  
endif()

target_compile_definitions(libspawn PUBLIC -DENERGYPLUS_STATIC_API)

target_link_libraries(
  libspawn
  PUBLIC spawn_utils CONAN_PKG::libzip CONAN_PKG::pugixml energyplusapi
  PRIVATE compile_options)
