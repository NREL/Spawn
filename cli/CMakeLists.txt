set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INSTALL_RPATH $ORIGIN/../lib)

set(src main.cpp)

if(ENABLE_MODELICA_COMPILER)
  find_library(
    liblapack
    NAMES liblapack.a liblapack.lib
    PATHS "${jmodelica_path}/lib/RuntimeLibrary/")
  find_library(
    libModelicaMatIO
    NAMES libModelicaMatIO.a libModelicaMatIO.lib
    PATHS "${jmodelica_path}/lib/RuntimeLibrary/")
  find_library(
    libModelicaIO
    NAMES libModelicaIO.a libModelicaIO.lib
    PATHS "${jmodelica_path}/lib/RuntimeLibrary/")
  find_library(
    libModelicaExternalC
    NAMES libModelicaExternalC.a libModelicaExternalC.lib
    PATHS "${jmodelica_path}/lib/RuntimeLibrary/")
  find_library(
    libfmi1_cs
    NAMES libfmi1_cs.a libfmi1_cs.lib
    PATHS "${jmodelica_path}/lib/RuntimeLibrary/")
  find_library(
    libjmi_get_set_default
    NAMES libjmi_get_set_default.a libjmi_get_set_default.lib
    PATHS "${jmodelica_path}/lib/RuntimeLibrary/")
  find_library(
    libfmi2
    NAMES libfmi2.a libfmi2.lib
    PATHS "${jmodelica_path}/lib/RuntimeLibrary/")
  find_library(
    libblas
    NAMES libblas.a libblas.lib
    PATHS "${jmodelica_path}/lib/RuntimeLibrary/")
  find_library(
    libjmi_block_solver
    NAMES libjmi_block_solver.a libjmi_block_solver.lib
    PATHS "${jmodelica_path}/lib/RuntimeLibrary/")
  find_library(
    libjmi_evaluator_util
    NAMES libjmi_evaluator_util.a libjmi_evaluator_util.lib
    PATHS "${jmodelica_path}/lib/RuntimeLibrary/")
  find_library(
    libjmi
    NAMES libjmi.a libjmi.lib
    PATHS "${jmodelica_path}/lib/RuntimeLibrary/")
  find_library(
    libModelicaStandardTables
    NAMES libModelicaStandardTables.a libModelicaStandardTables.lib
    PATHS "${jmodelica_path}/lib/RuntimeLibrary/")
  find_library(
    libzlib
    NAMES libzlib.a libzlib.lib
    PATHS "${jmodelica_path}/lib/RuntimeLibrary/")
  find_library(
    libfmi1_me
    NAMES libfmi1_me.a libfmi1_me.lib
    PATHS "${jmodelica_path}/lib/RuntimeLibrary/")
  find_library(
    libcminpack
    NAMES libcminpack.a libcminpack.lib
    PATHS "${jmodelica_path}/ThirdParty/Minpack/lib/")
  find_library(
    libsundials_nvecserial
    NAMES libsundials_nvecserial.a libsundials_nvecserial.lib
    PATHS "${jmodelica_path}/ThirdParty/Sundials/lib/")
  find_library(
    libsundials_idas
    NAMES libsundials_idas.a libsundials_idas.lib
    PATHS "${jmodelica_path}/ThirdParty/Sundials/lib/")
  find_library(
    libsundials_cvodes
    NAMES libsundials_cvodes.a libsundials_cvodes.lib
    PATHS "${jmodelica_path}/ThirdParty/Sundials/lib/")
  find_library(
    libsundials_ida
    NAMES libsundials_ida.a libsundials_ida.lib
    PATHS "${jmodelica_path}/ThirdParty/Sundials/lib/")
  find_library(
    libsundials_nvecopenmp
    NAMES libsundials_nvecopenmp.a libsundials_nvecopenmp.lib
    PATHS "${jmodelica_path}/ThirdParty/Sundials/lib/")
  find_library(
    libsundials_arkode
    NAMES libsundials_arkode.a libsundials_arkode.lib
    PATHS "${jmodelica_path}/ThirdParty/Sundials/lib/")
  find_library(
    libsundials_cvode
    NAMES libsundials_cvode.a libsundials_cvode.lib
    PATHS "${jmodelica_path}/ThirdParty/Sundials/lib/")
  find_library(
    libsundials_kinsol
    NAMES libsundials_kinsol.a libsundials_kinsol.lib
    PATHS "${jmodelica_path}/ThirdParty/Sundials/lib/")

  list(
    APPEND
    modelica_libs
    ${liblapack}
    ${libModelicaMatIO}
    ${libModelicaIO}
    ${libModelicaExternalC}
    ${libfmi1_cs}
    ${libjmi_get_set_default}
    ${libfmi2}
    ${libblas}
    ${libjmi_block_solver}
    ${libjmi_evaluator_util}
    ${libjmi}
    ${libModelicaStandardTables}
    ${libzlib}
    ${libfmi1_me}
    ${libcminpack}
    ${libsundials_nvecserial}
    ${libsundials_idas}
    ${libsundials_cvodes}
    ${libsundials_ida}
    ${libsundials_nvecopenmp}
    ${libsundials_arkode}
    ${libsundials_cvode}
    ${libsundials_kinsol})

  foreach(filepath ${modelica_libs})
    if(filepath)
      file(RELATIVE_PATH embedded_path "${jmodelica_path}/../" ${filepath})
      list(APPEND embedded_files ${filepath})
      list(APPEND embedded_paths ${embedded_path})
    else()
      message(FATAL_ERROR ${filepath})
    endif()
  endforeach()

  file(GLOB_RECURSE header_files FOLLOW_SYMLINKS "${jmodelica_path}/include/**/*.h" "${jmodelica_path}/ThirdParty/FMI/2.0/**/*.h"
       "${jmodelica_path}/ThirdParty/FMI/2.0/*.h")

  foreach(filepath ${header_files})
    file(RELATIVE_PATH embedded_path "${jmodelica_path}/../" ${filepath})
    list(APPEND embedded_files ${filepath})
    list(APPEND embedded_paths ${embedded_path})
  endforeach()

  file(GLOB_RECURSE code_gen_files FOLLOW_SYMLINKS "${jmodelica_path}/CodeGenTemplates/**" "${jmodelica_path}/XML/**")

  foreach(filepath ${template_files})
    file(RELATIVE_PATH embedded_path "${jmodelica_path}/../" ${filepath})
    list(APPEND embedded_files ${filepath})
    list(APPEND embedded_paths ${embedded_path})
  endforeach()

  # don't embed MSL for now
  #file(GLOB_RECURSE msl_files FOLLOW_SYMLINKS
  #  "${jmodelica_path}/ThirdParty/MSL/**"
  #)
  #
  #foreach(filepath ${msl_files})
  #  file(RELATIVE_PATH embedded_path "${jmodelica_path}/../" ${filepath})
  #  list(APPEND embedded_files ${filepath})
  #  list(APPEND embedded_paths ${embedded_path})
  #endforeach()
  # don't embed MBL for now
  #set(mbl_path "${CMAKE_BINARY_DIR}/modelica-buildings")
  #file(GLOB_RECURSE mbl_files FOLLOW_SYMLINKS
  #  "${mbl_path}/**"
  #)
  #
  #foreach(filepath ${mbl_files})
  #  file(RELATIVE_PATH embedded_path "${mbl_path}/../" ${filepath})
  #  list(APPEND embedded_files ${filepath})
  #  list(APPEND embedded_paths ${embedded_path})
  #endforeach()

  embed_files("${embedded_files}" "${embedded_paths}" embedded_output spawnmodelica)

  list(APPEND src compilerchain.hpp compilerchain.cpp ${embedded_output})
endif()

# INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/fmi2/ )
# INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable(spawn ${src})

configure_file(config.hpp.in config.hxx)
target_include_directories(spawn PRIVATE "${CMAKE_SOURCE_DIR}")
target_include_directories(spawn PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_include_directories(spawn PRIVATE "${PROJECT_BINARY_DIR}")
target_include_directories(spawn PRIVATE "${PROJECT_BINARY_DIR}/modelica")

target_link_libraries(
  spawn
  PRIVATE compile_options
          libspawn
          energypluslib
          spawn_utils
          CONAN_PKG::cli11
          CONAN_PKG::nlohmann_json
          CONAN_PKG::spdlog
)

if(ENABLE_MODELICA_COMPILER)
  target_link_libraries(spawn PRIVATE compiler modelica)
endif()

install(TARGETS spawn DESTINATION bin)

# This is only necessary if ENABLE_MODELICA_COMPILER, but shouldn't hurt in any case
if(WIN32)
  set(spawn_output_name "spawn.exe")
else()
  set(spawn_output_name "spawn")
endif()
install(
  CODE "
    include(BundleUtilities)
    set(install_path \"\${CMAKE_INSTALL_PREFIX}/bin/${spawn_output_name}\")
    message(\"install_path: \${install_path}\")
    fixup_bundle(\"\${install_path}\" \"\" \"\")
")
