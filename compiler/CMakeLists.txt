add_executable(spawn_exe_launcher spawn_exe_launcher.c)
target_link_libraries(spawn_exe_launcher ${CMAKE_DL_LIBS})



add_library(c_bridge SHARED c_bridge/c_bridge.c)
set_property(TARGET c_bridge PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if (MSVC)
  set(embedded_files $<TARGET_FILE:c_bridge>)
  set(embedded_paths c_bridge/c_bridge.dll)
  list(APPEND embedded_files $<TARGET_LINKER_FILE:c_bridge>)
  list(APPEND embedded_paths c_bridge/c_bridge.lib)
else()
  set(embedded_files $<TARGET_FILE:c_bridge>)
  set(embedded_paths c_bridge/libc_bridge.so)
endif()

list(APPEND embedded_files
  ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/assert.h    ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/limits.h     ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/stdio.h 
  ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/c_bridge.h  ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/locale.h     ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/stdlib.h 
  ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/complex.h   ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/math.h       ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/stdnoreturn.h 
  ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/ctype.h     ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/setjmp.h     ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/string.h 
  ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/dlfcn.h     ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/signal.h     ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/tgmath.h 
  ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/errno.h     ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/stdalign.h   ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/threads.h 
  ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/fcntl.h     ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/stdarg.h     ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/time.h 
  ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/fenv.h      ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/stdatomic.h  ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/uchar.h 
  ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/float.h     ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/stdbool.h    ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/wchar.h 
  ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/inttypes.h  ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/stddef.h     ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/wctype.h 
  ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/iso646.h    ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/stdint.h     ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/linux/limits.h
  ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/sys/stat.h  ${CMAKE_CURRENT_SOURCE_DIR}/c_bridge/sys/types.h
  )


list(APPEND embedded_paths
  c_bridge/include/assert.h    c_bridge/include/limits.h     c_bridge/include/stdio.h
  c_bridge/include/c_bridge.h  c_bridge/include/locale.h     c_bridge/include/stdlib.h
  c_bridge/include/complex.h   c_bridge/include/math.h       c_bridge/include/stdnoreturn.h
  c_bridge/include/ctype.h     c_bridge/include/setjmp.h     c_bridge/include/string.h
  c_bridge/include/dlfcn.h     c_bridge/include/signal.h     c_bridge/include/tgmath.h
  c_bridge/include/errno.h     c_bridge/include/stdalign.h   c_bridge/include/threads.h
  c_bridge/include/fcntl.h     c_bridge/include/stdarg.h     c_bridge/include/time.h
  c_bridge/include/fenv.h      c_bridge/include/stdatomic.h  c_bridge/include/uchar.h
  c_bridge/include/float.h     c_bridge/include/stdbool.h    c_bridge/include/wchar.h
  c_bridge/include/inttypes.h  c_bridge/include/stddef.h     c_bridge/include/wctype.h
  c_bridge/include/iso646.h    c_bridge/include/stdint.h     c_bridge/include/linux/limits.h
  c_bridge/include/sys/stat.h  c_bridge/include/sys/types.h
)


if (MSVC) 
  find_program(LLD_PATH lld-link.exe PATHS ${LLVM_TOOLS_BINARY_DIR} REQUIRED)

  list(APPEND embedded_files ${LLD_PATH})
  list(APPEND embedded_paths "lld-link.exe")
endif()

embed_files("${embedded_files}" "${embedded_paths}" embedded_output spawnmodelica_compiler)




add_library(compiler compiler.hpp compiler.cpp ${embedded_output})
add_dependencies(compiler c_bridge)

target_include_directories(compiler PRIVATE "${CMAKE_SOURCE_DIR}")
target_include_directories(compiler PRIVATE "${CMAKE_BINARY_DIR}")
target_include_directories(compiler PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_include_directories(compiler SYSTEM PUBLIC ${LLVM_INCLUDE_DIRS})
target_include_directories(compiler SYSTEM PUBLIC ${CLANG_INCLUDE_DIRS})

target_compile_features(compiler PRIVATE cxx_std_17)

# Recursively remove the LLVM target as a dependency. The LLVM target relates to
# a shared lib, which is not wanted. Instead llvm archive libs will be linked
function(remove_llvm target_name)
  get_target_property(target_libs ${target_name} INTERFACE_LINK_LIBRARIES)
  list(REMOVE_ITEM libs LLVM)
  foreach(lib IN LISTS libs)
    remove_llvm(lib)
  endforeach()
endfunction()

remove_llvm(clangFrontendTool)
# Most but not all of the LLVM libs are needed
list(
  REMOVE_ITEM
  LLVM_AVAILABLE_LIBS
  edit
  LLVMLineEditor
  LLVM
  LTO
  Remarks)

if(MSVC)
  set(LIB_PREFIX "")
  set(LIB_EXTENSION "lib")
else()
  set(LIB_PREFIX "lib")
  set(LIB_EXTENSION "a")
endif()

target_link_libraries(
  compiler
  PUBLIC clangFrontendTool
         ${LLVM_LIBRARY_DIR}/${LIB_PREFIX}lldYAML.${LIB_EXTENSION}
         ${LLVM_LIBRARY_DIR}/${LIB_PREFIX}lldCOFF.${LIB_EXTENSION}
         ${LLVM_LIBRARY_DIR}/${LIB_PREFIX}lldMinGW.${LIB_EXTENSION}
         ${LLVM_LIBRARY_DIR}/${LIB_PREFIX}lldELF.${LIB_EXTENSION}
         ${LLVM_LIBRARY_DIR}/${LIB_PREFIX}lldMachO.${LIB_EXTENSION}
         ${LLVM_LIBRARY_DIR}/${LIB_PREFIX}lldWasm.${LIB_EXTENSION}
         ${LLVM_LIBRARY_DIR}/${LIB_PREFIX}lldReaderWriter.${LIB_EXTENSION}
         ${LLVM_LIBRARY_DIR}/${LIB_PREFIX}lldDriver.${LIB_EXTENSION}
         ${LLVM_LIBRARY_DIR}/${LIB_PREFIX}lldCore.${LIB_EXTENSION}
         ${LLVM_LIBRARY_DIR}/${LIB_PREFIX}lldCommon.${LIB_EXTENSION}
         # This would be one or two top level libs such as LLVMCore,
         # LLVMLibDriver, or similar, however the previous LLD libs have
         # dependencies on LLVM libs, and there are no exported cmake targets
         # with defined INTERFACE_LINK_LIBRARIES, so we just throw the whole set
         # of LLVM libs into the link path
         ${LLVM_AVAILABLE_LIBS}
         CONAN_PKG::fmt
         CONAN_PKG::spdlog
 PRIVATE CONAN_PKG::zlib compile_options spawn_utils)

if(NOT MSVC)
  # necessary for making local symbols available to JIT
  target_link_libraries(compiler PUBLIC -rdynamic)

  if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message(
      SEND_ERROR
        "Currently we cannot link with EnergyPlus and Clang in Debug mode on Unix because EnergyPlus is defining _GLIBCXX_DEBUG whichh conflicts with build settings for libclang"
    )
  endif()
else()
  target_compile_definitions(compiler PUBLIC -DNOMINMAX)
endif()



