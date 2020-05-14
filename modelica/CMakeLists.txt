set(CMAKE_CXX_STANDARD 17)

find_package(Java)
get_filename_component(java_bin ${Java_JAVAC_EXECUTABLE} DIRECTORY)
find_program(NATIVE_IMAGE_EXECUTABLE native-image ${java_bin})

set( jars
  ${jmodelica_path}/lib/separateProcess.jar
  ${jmodelica_path}/lib/OptimicaCompiler.jar
  ${jmodelica_path}/lib/ModelicaCompiler.jar
  ${jmodelica_path}/lib/util.jar
  ${jmodelica_path}/ThirdParty/Beaver/lib/beaver-rt.jar
)

foreach(jar IN LISTS jars)
  string(CONCAT classpath ${classpath} "${jar}:")
endforeach()
string(CONCAT classpath ${classpath} ${CMAKE_CURRENT_SOURCE_DIR}:${CMAKE_CURRENT_BINARY_DIR})

add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Modelica.class
  COMMAND ${Java_JAVAC_EXECUTABLE} -d ${CMAKE_CURRENT_BINARY_DIR} -classpath ${classpath} ${CMAKE_CURRENT_SOURCE_DIR}/Modelica.java
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Modelica.java
)

# RPATH complications mean the library needs to be generated next to the spawn executable
# Maybe this can be fixed
set(modelica_lib_path "${PROJECT_BINARY_DIR}/cli/modelica${CMAKE_SHARED_LIBRARY_SUFFIX}")
add_custom_command( OUTPUT ${modelica_lib_path}
  COMMAND ${NATIVE_IMAGE_EXECUTABLE} -classpath ${classpath} --shared Modelica
  #WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/cli"
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Modelica.class
)

add_custom_target(modelica-lib DEPENDS ${modelica_lib_path})

add_library(modelica SHARED IMPORTED GLOBAL)
set_target_properties(modelica PROPERTIES IMPORTED_LOCATION ${modelica_lib_path})

## Need cmake 3.11 or greater to do this
#target_include_directories(modelica INTERFACE
#  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
#  #$<INSTALL_INTERFACE:include/mylib>  # <prefix>/include/mylib
#)

add_dependencies(
  modelica
  modelica-lib
)

install(PROGRAMS ${modelica_lib_path} DESTINATION modelica)


