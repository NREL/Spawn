set(CMAKE_CXX_STANDARD 17)

find_package(Java)
get_filename_component(java_bin ${Java_JAVAC_EXECUTABLE} DIRECTORY)
find_program(NATIVE_IMAGE_EXECUTABLE native-image ${java_bin})

set( jars 
  "${optimica_path}/ThirdParty/Beaver/lib/beaver-rt.jar"
  "${optimica_path}/ThirdParty/gson/lib/gson-2.8.5.jar"
  "${optimica_path}/lib/ModelicaCompiler.jar"
  "${optimica_path}/lib/separateProcess.jar"
  "${optimica_path}/lib/util.jar"
  "${optimica_path}/lib/OptimicaCompiler.jar"
  "${optimica_path}/lib/casadi_interface/ModelicaCompiler.jar"
  "${optimica_path}/lib/casadi_interface/util.jar"
  "${optimica_path}/lib/casadi_interface/OptimicaCompiler.jar"
  ${PROJECT_SOURCE_DIR}/java/json-simple-3.1.1.jar
)

foreach(jar IN LISTS jars)
  string(CONCAT classpath ${classpath} "${jar}:")
endforeach()
string(CONCAT classpath ${classpath} ${CMAKE_CURRENT_SOURCE_DIR}:${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/SpawnCompilerDelegator.class
  COMMAND ${Java_JAVAC_EXECUTABLE} -d ${CMAKE_CURRENT_BINARY_DIR} -classpath ${classpath} ${CMAKE_CURRENT_SOURCE_DIR}/SpawnCompilerDelegator.java
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/SpawnCompilerDelegator.java
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Optimica.class
  COMMAND ${Java_JAVAC_EXECUTABLE} -d ${CMAKE_CURRENT_BINARY_DIR} -classpath ${classpath} ${CMAKE_CURRENT_SOURCE_DIR}/Optimica.java
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Optimica.java ${CMAKE_CURRENT_BINARY_DIR}/SpawnCompilerDelegator.class
)

# RPATH complications mean the library needs to be generated next to the spawn
# executable Maybe this can be fixed
set(optimica_lib_path "${PROJECT_BINARY_DIR}/cli/optimica${CMAKE_SHARED_LIBRARY_SUFFIX}")

add_custom_command(
  OUTPUT ${optimica_lib_path}
  COMMAND ${NATIVE_IMAGE_EXECUTABLE} -classpath ${classpath} --shared Optimica
          # WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/cli"
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/SpawnCompilerDelegator.class ${CMAKE_CURRENT_BINARY_DIR}/Optimica.class
)

add_custom_target(optimica-lib DEPENDS ${optimica_lib_path})

add_library(optimica SHARED IMPORTED GLOBAL)
set_target_properties(optimica PROPERTIES IMPORTED_LOCATION ${optimica_lib_path})
add_dependencies(optimica optimica-lib)

if(WIN32)
  install(PROGRAMS ${optimica_lib_path} DESTINATION bin)
else()
  install(PROGRAMS ${optimica_lib_path} DESTINATION lib)
endif()

