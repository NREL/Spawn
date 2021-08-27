
function(
    download_modelica
    modelica_path # The final directory name
    modelica_tar_root # The directory name of the tarball root, this will be moved / renamed to modelica_path
    modelica_md5 # The md5 of the tarball
    modelica_url # The download url. Can be a local file:// url
)
  get_filename_component(modelica_tar ${modelica_url} NAME)
  set(modelica_download_path "${PROJECT_BINARY_DIR}/${modelica_tar}")
  set(modelica_extracted_path "${PROJECT_BINARY_DIR}/${modelica_tar_root}")

  set(download_modelica TRUE)
  
  if(EXISTS ${modelica_download_path})
    file(MD5 ${modelica_download_path} modelica_current_md5)
    if(${modelica_current_md5} STREQUAL ${modelica_md5})
      if(EXISTS ${modelica_path})
        set(download_modelica FALSE)
      endif()
    endif()
  endif()
  
  if(download_modelica)
    Message("Downlaoding ${modelica_url} to ${modelica_download_path}")
    file(REMOVE_RECURSE ${modelica_download_path})
    file(REMOVE_RECURSE ${modelica_extracted_path})
    file(REMOVE_RECURSE ${modelica_path})
    file(DOWNLOAD ${modelica_url} ${modelica_download_path} SHOW_PROGRESS)
    execute_process(COMMAND ${CMAKE_COMMAND} -E
      tar xz ${modelica_download_path}
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    )
  
    execute_process(COMMAND ${CMAKE_COMMAND} -E
      rename ${modelica_extracted_path} ${modelica_path}
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    )
  endif()
endfunction()

