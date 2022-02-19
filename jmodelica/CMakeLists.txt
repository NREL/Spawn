set(CMAKE_CXX_STANDARD 17)

find_package(Java 11)
execute_process(COMMAND ${Java_JAVA_EXECUTABLE} --version
                OUTPUT_VARIABLE JAVA_VERSION_STRING)

if(NOT JAVA_VERSION_STRING MATCHES ".*GraalVM.*\\(build.*11.*")
  message(
    FATAL_ERROR
      "Did not find expected graalvm java version (based on Java 11) found: ${JAVA_VERSION_STRING}"
  )
endif()

get_filename_component(java_bin ${Java_JAVAC_EXECUTABLE} DIRECTORY)
message(
  "If native-image is not found, you don't have a correct installation of graalvm java (install with `gu install native-image`)"
)
find_program(NATIVE_IMAGE_EXECUTABLE native-image
             PATHS ${java_bin} ${java_bin}/../lib/svm/bin REQUIRED)

set(jars
    ${jmodelica_path}/lib/separateProcess.jar
    ${jmodelica_path}/lib/OptimicaCompiler.jar
    ${jmodelica_path}/lib/ModelicaCompiler.jar
    ${jmodelica_path}/lib/util.jar
    ${jmodelica_path}/ThirdParty/Beaver/lib/beaver-rt.jar
    ${PROJECT_SOURCE_DIR}/java/json-simple-3.1.1.jar)

foreach(jar IN LISTS jars)
  string(CONCAT classpath ${classpath} "${jar}:")
endforeach()
string(CONCAT classpath ${classpath}
              ${CMAKE_CURRENT_SOURCE_DIR}:${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/SpawnCompilerDelegator.class
  COMMAND ${Java_JAVAC_EXECUTABLE} -d ${CMAKE_CURRENT_BINARY_DIR} -classpath
          ${classpath} ${CMAKE_CURRENT_SOURCE_DIR}/SpawnCompilerDelegator.java
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/SpawnCompilerDelegator.java)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/JModelica.class
  COMMAND ${Java_JAVAC_EXECUTABLE} -d ${CMAKE_CURRENT_BINARY_DIR} -classpath
          ${classpath} ${CMAKE_CURRENT_SOURCE_DIR}/JModelica.java
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/JModelica.java
          ${CMAKE_CURRENT_BINARY_DIR}/SpawnCompilerDelegator.class)

# RPATH complications mean the library needs to be generated next to the spawn
# executable Maybe this can be fixed
set(jmodelica_lib_path
    "${PROJECT_BINARY_DIR}/cli/jmodelica${CMAKE_SHARED_LIBRARY_SUFFIX}")

add_custom_command(
  OUTPUT ${jmodelica_lib_path}
  COMMAND
    ${NATIVE_IMAGE_EXECUTABLE} -classpath ${classpath}
    -H:APIFunctionPrefix="jmodelica_" --shared JModelica
    # WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/cli"
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/SpawnCompilerDelegator.class
          ${CMAKE_CURRENT_BINARY_DIR}/JModelica.class)

add_custom_target(jmodelica-lib DEPENDS ${jmodelica_lib_path})

add_library(jmodelica SHARED IMPORTED GLOBAL)
set_target_properties(jmodelica PROPERTIES IMPORTED_LOCATION
                                           ${jmodelica_lib_path})

add_dependencies(jmodelica jmodelica-lib)

if(WIN32)
  install(PROGRAMS ${jmodelica_lib_path} DESTINATION bin)
else()
  install(PROGRAMS ${jmodelica_lib_path} DESTINATION lib)
endif()
