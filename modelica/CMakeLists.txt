project(Modelica)

include(ExternalProject)

find_package(Java)
get_filename_component(java_bin ${Java_JAVAC_EXECUTABLE} DIRECTORY)
find_program(NATIVE_IMAGE_EXECUTABLE native-image ${java_bin})

ExternalProject_Add(
  JModelica
  URL https://modelica.s3.amazonaws.com/latest/Modelica-latest.tar.gz 
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  TEST_COMMAND ""
)

set( jars
  JModelica-prefix/src/JModelica/java/bin/separateProcess.jar
  JModelica-prefix/src/JModelica/java/bin/OptimicaCompiler.jar
  JModelica-prefix/src/JModelica/java/bin/ModelicaCompiler.jar
  JModelica-prefix/src/JModelica/java/bin/util.jar
  JModelica-prefix/src/JModelica/java/bin/beaver-rt-src.jar
  JModelica-prefix/src/JModelica/java/bin/beaver-rt.jar
  JModelica-prefix/src/JModelica/java/bin/beaver.jar
)

foreach(jar IN LISTS jars)
  string(CONCAT classpath ${classpath} "${PROJECT_BINARY_DIR}/${jar}:")
endforeach()
string(CONCAT classpath ${classpath} ${PROJECT_SOURCE_DIR}:${PROJECT_BINARY_DIR})

add_custom_command( OUTPUT ${PROJECT_BINARY_DIR}/Modelica.class
  COMMAND ${Java_JAVAC_EXECUTABLE} -d ${PROJECT_BINARY_DIR} -classpath ${classpath} ${PROJECT_SOURCE_DIR}/Modelica.java
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  DEPENDS JModelica ${PROJECT_SOURCE_DIR}/Modelica.java
)

set(modelica_lib_path ${PROJECT_BINARY_DIR}/modelica${CMAKE_SHARED_LIBRARY_SUFFIX})
add_custom_command( OUTPUT ${modelica_lib_path}
  COMMAND ${NATIVE_IMAGE_EXECUTABLE} -classpath ${classpath} --shared Modelica
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  DEPENDS JModelica ${PROJECT_BINARY_DIR}/Modelica.class
)

add_custom_target(modelica-lib DEPENDS ${modelica_lib_path})

add_library(modelica SHARED IMPORTED GLOBAL)
set_target_properties(modelica PROPERTIES IMPORTED_LOCATION ${modelica_lib_path})

# Need cmake 3.11 or greater to do this
#target_include_directories(modelica INTERFACE
#  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
#  #$<INSTALL_INTERFACE:include/mylib>  # <prefix>/include/mylib
#)

add_dependencies(
  modelica
  modelica-lib
)


