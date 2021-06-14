function(
  embed_optimica
  optimica_path
  embedded_files
  embedded_paths
)
  find_library(
    optimica_libsundials_idas
    NAMES libsundials_idas.a
    PATHS "${optimica_path}/ThirdParty/Sundials/lib/"
  )
  
  find_library(
    optimica_libsundials_nvecserial
    NAMES libsundials_nvecserial.a
    PATHS "${optimica_path}/ThirdParty/Sundials/lib/"
  )
  
  find_library(
    optimica_libsundials_arkode
    NAMES libsundials_arkode.a
    PATHS "${optimica_path}/ThirdParty/Sundials/lib/"
  )
  
  find_library(
    optimica_libsundials_ida
    NAMES libsundials_ida.a
    PATHS "${optimica_path}/ThirdParty/Sundials/lib/"
  )
  
  find_library(
    optimica_libsundials_nvecopenmp
    NAMES libsundials_nvecopenmp.a
    PATHS "${optimica_path}/ThirdParty/Sundials/lib/"
  )
  
  find_library(
    optimica_libsundials_cvodes
    NAMES libsundials_cvodes.a
    PATHS "${optimica_path}/ThirdParty/Sundials/lib/"
  )
  
  find_library(
    optimica_libsundials_cvodes
    NAMES libsundials_cvode.a
    PATHS "${optimica_path}/ThirdParty/Sundials/lib/"
  )
  
  find_library(
    optimica_libsundials_kinsol
    NAMES libsundials_kinsol.a
    PATHS "${optimica_path}/ThirdParty/Sundials/lib/"
  )
  
  find_library(
    optimica_ibcminpack
    NAMES libcminpack.a
    PATHS "${optimica_path}/ThirdParty/Minpack/lib/"
  )
  
  find_library(
    optimica_libjmi
    NAMES libjmi.a
    PATHS "${optimica_path}/lib/RuntimeLibrary/"
  )
  
  find_library(
    optimica_libModelicaExternalCasadiC
    NAMES libModelicaExternalCasadiC.a
    PATHS "${optimica_path}/lib/RuntimeLibrary/"
  )
  
  find_library(
    optimica_libzlib
    NAMES libzlib.a
    PATHS "${optimica_path}/lib/RuntimeLibrary/"
  )
  
  find_library(
    optimica_libblas
    NAMES libblas.a
    PATHS "${optimica_path}/lib/RuntimeLibrary/"
  )
  
  find_library(
    optimica_libjmi_evaludator_util
    NAMES libjmi_evaluator_util.a
    PATHS "${optimica_path}/lib/RuntimeLibrary/"
  )
  
  find_library(
    optimica_libfmi2
    NAMES libfmi2.a
    PATHS "${optimica_path}/lib/RuntimeLibrary/"
  )
  
  find_library(
    optimica_libfmi1_me
    NAMES libfmi1_me.a
    PATHS "${optimica_path}/lib/RuntimeLibrary/"
  )
  
  find_library(
    optimica_libModelicaMatIO
    NAMES libModelicaMatIO.a
    PATHS "${optimica_path}/lib/RuntimeLibrary/"
  )
  
  find_library(
    optimica_libModelicaExternalC
    NAMES libModelicaExternalC.a
    PATHS "${optimica_path}/lib/RuntimeLibrary/"
  )
  
  find_library(
    optimica_libModelicaStandardTables
    NAMES libModelicaStandardTables.a
    PATHS "${optimica_path}/lib/RuntimeLibrary/"
  )
  
  find_library(
    optimica_libfmi1_cs
    NAMES libfmi1_cs.a
    PATHS "${optimica_path}/lib/RuntimeLibrary/"
  )
  
  find_library(
    optimica_libjmi_block_solver
    NAMES libjmi_block_solver.a
    PATHS "${optimica_path}/lib/RuntimeLibrary/"
  )
  
  find_library(
    optimica_libjmi_get_set_default
    NAMES libjmi_get_set_default.a
    PATHS "${optimica_path}/lib/RuntimeLibrary/"
  )
  
  find_library(
    optimica_libjmi_get_set_lazy
    NAMES libjmi_get_set_lazy.a
    PATHS "${optimica_path}/lib/RuntimeLibrary/"
  )
  
  find_library(
    optimica_libModelicaIO
    NAMES libModelicaIO.a
    PATHS "${optimica_path}/lib/RuntimeLibrary/"
  )
  
  find_library(
    optimica_liblapack
    NAMES liblapack.a
    PATHS "${optimica_path}/lib/RuntimeLibrary/"
  )
  
  list(
    APPEND
    ${modelica_libs}
    ${optimica_libsundials_idas}
    ${optimica_libsundials_nvecserial}
    ${optimica_libsundials_arkode}
    ${optimica_libsundials_ida}
    ${optimica_libsundials_nvecopenmp}
    ${optimica_libsundials_cvodes}
    ${optimica_libsundials_cvodes}
    ${optimica_libsundials_kinsol}
    ${optimica_ibcminpack}
    ${optimica_libjmi}
    ${optimica_libModelicaExternalCasadiC}
    ${optimica_libzlib}
    ${optimica_libblas}
    ${optimica_libjmi_evaludator_util}
    ${optimica_libfmi2}
    ${optimica_libfmi1_me}
    ${optimica_libModelicaMatIO}
    ${optimica_libModelicaExternalC}
    ${optimica_libModelicaStandardTables}
    ${optimica_libfmi1_cs}
    ${optimica_libjmi_block_solver}
    ${optimica_libjmi_get_set_default}
    ${optimica_libjmi_get_set_lazy}
    ${optimica_libModelicaIO}
    ${optimica_liblapack}
  )

  foreach(filepath ${modelica_libs})
    if(filepath)
      file(RELATIVE_PATH embedded_path "${optimica_path}/../" ${filepath})
      list(APPEND optimica_embedded_files ${filepath})
      list(APPEND optimica_embedded_paths ${embedded_path})
    else()
      message(FATAL_ERROR ${filepath})
    endif()
  endforeach()
  
  file(GLOB_RECURSE header_files FOLLOW_SYMLINKS "${optimica_path}/include/**/*.h" "${optimica_path}/ThirdParty/FMI/2.0/**/*.h"
       "${optimica_path}/ThirdParty/FMI/2.0/*.h")
  
  foreach(filepath ${header_files})
    file(RELATIVE_PATH embedded_path "${optimica_path}/../" ${filepath})
    list(APPEND optimica_embedded_files ${filepath})
    list(APPEND optimica_embedded_paths ${embedded_path})
  endforeach()
  
  file(GLOB_RECURSE code_gen_files FOLLOW_SYMLINKS "${optimica_path}/CodeGenTemplates/**" "${optimica_path}/XML/**")
  
  foreach(filepath ${code_gen_files})
    file(RELATIVE_PATH embedded_path "${optimica_path}/../" ${filepath})
    list(APPEND optimica_embedded_files ${filepath})
    list(APPEND optimica_embedded_paths ${embedded_path})
  endforeach()
  
  set(makefile "${optimica_path}/Makefiles/MakeFile")
  file(RELATIVE_PATH embedded_makefile "${optimica_path}/../" ${makefile})
  list(APPEND optimica_embedded_files ${makefile})
  list(APPEND optimica_embedded_paths ${embedded_makefile})

  set(${embedded_files} ${optimica_embedded_files} PARENT_SCOPE)
  set(${embedded_paths} ${optimica_embedded_paths} PARENT_SCOPE)
endfunction()

