function(
  embed_optimica
  optimica_path
  embedded_files
  embedded_paths
)
  
  file(GLOB_RECURSE modelica_libs FOLLOW_SYMLINKS 
    "${optimica_path}/ThirdParty/Sundials/lib/*.a" 
    "${optimica_path}/ThirdParty/Minpack/lib/*.a" 
    "${optimica_path}/lib/RuntimeLibrary/*.a"
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

  file(GLOB_RECURSE license_files FOLLOW_SYMLINKS "${optimica_path}/lib/LicensingEncryption/**")
  
  foreach(filepath ${license_files})
    file(RELATIVE_PATH embedded_path "${optimica_path}/../" ${filepath})
    list(APPEND optimica_embedded_files ${filepath})
    list(APPEND optimica_embedded_paths ${embedded_path})
  endforeach()

  set(jmi_evaluator "${optimica_path}/bin/jmi_evaluator")
  file(RELATIVE_PATH embedded_jmi_evaluator "${optimica_path}/../" ${jmi_evaluator})
  list(APPEND optimica_embedded_files ${jmi_evaluator})
  list(APPEND optimica_embedded_paths ${embedded_jmi_evaluator})

  set(${embedded_files} ${optimica_embedded_files} PARENT_SCOPE)
  set(${embedded_paths} ${optimica_embedded_paths} PARENT_SCOPE)
endfunction()

