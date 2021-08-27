function(
  embed_libc
  embedded_files
  embedded_paths
)
  set(libc_dir "${PROJECT_BINARY_DIR}/libc")
  set(touchfile "${PROJECT_BINARY_DIR}/libc/libc.touch")

  if(NOT EXISTS ${touchfile})
    file(REMOVE_RECURSE ${libc_dir})
    file(MAKE_DIRECTORY ${libc_dir})
    set(packages libc6-dev linux-libc-dev)
    foreach(p ${packages})
      execute_process(COMMAND apt download ${p} WORKING_DIRECTORY ${libc_dir})
    endforeach()
    file(GLOB debs "${libc_dir}/*.deb")
    foreach(deb ${debs})
      execute_process(COMMAND dpkg-deb -xv ${deb} . WORKING_DIRECTORY ${libc_dir})
    endforeach()
    file(TOUCH ${touchfile})
  endif()

  file(GLOB_RECURSE files FOLLOW_SYMLINKS 
    "${libc_dir}/usr/**/*.h"
    "${libc_dir}/usr/**/*.so"
    "${libc_dir}/usr/**/*.a"
  )

  foreach(filepath ${files})
    file(RELATIVE_PATH embedded_path ${libc_dir} ${filepath})
    list(APPEND libc_embedded_files ${filepath})
    list(APPEND libc_embedded_paths ${embedded_path})
  endforeach()

  # The libc packages don't have all of the headers that are required.
  # Some of the headers that come with clang are also required
  string(REPLACE "git" "" LLVM_PACKAGE_VERSION ${LLVM_PACKAGE_VERSION})
  set(CLANG_C_HEADERS_DIR "${LLVM_INSTALL_PREFIX}/lib/clang/${LLVM_PACKAGE_VERSION}/include")
  file(GLOB_RECURSE files FOLLOW_SYMLINKS "${CLANG_C_HEADERS_DIR}/**")

  foreach(filepath ${files})
    #file(RELATIVE_PATH embedded_path "${CLANG_C_HEADERS_ROOT}" ${filepath})
    list(APPEND libc_embedded_files ${filepath})
    list(APPEND libc_embedded_paths ${filepath})
  endforeach()
  
  set(${embedded_files} ${libc_embedded_files} PARENT_SCOPE)
  set(${embedded_paths} ${libc_embedded_paths} PARENT_SCOPE)
endfunction()

