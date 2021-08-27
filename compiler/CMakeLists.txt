add_executable(spawn_exe_launcher spawn_exe_launcher.c)
target_link_libraries(spawn_exe_launcher ${CMAKE_DL_LIBS})

add_library(compiler compiler.hpp compiler.cpp)

target_include_directories(compiler PRIVATE "${CMAKE_SOURCE_DIR}")
target_include_directories(compiler PRIVATE "${CMAKE_BINARY_DIR}")
target_include_directories(compiler SYSTEM PUBLIC ${LLVM_INCLUDE_DIRS})
target_include_directories(compiler SYSTEM PUBLIC ${CLANG_INCLUDE_DIRS})

target_compile_features(compiler PRIVATE cxx_std_17)

# Recursively remove the LLVM target as a dependency.
# The LLVM target relates to a shared lib, which is not wanted.
# Instead llvm archive libs will be linked
function(remove_llvm target_name)
  get_target_property(target_libs ${target_name} INTERFACE_LINK_LIBRARIES)
  list(REMOVE_ITEM libs LLVM)
  foreach(lib IN LISTS libs)
    remove_llvm(lib)
  endforeach()
endfunction()

remove_llvm(clangFrontendTool)
# Most but not all of the LLVM libs are needed
list(REMOVE_ITEM LLVM_AVAILABLE_LIBS edit LLVMLineEditor LLVM LTO Remarks)

target_link_libraries(
    compiler
  PUBLIC 
    clangFrontendTool
    ${LLVM_LIBRARY_DIR}/liblldYAML.a
    ${LLVM_LIBRARY_DIR}/liblldCOFF.a
    ${LLVM_LIBRARY_DIR}/liblldMinGW.a
    ${LLVM_LIBRARY_DIR}/liblldELF.a
    ${LLVM_LIBRARY_DIR}/liblldMachO.a
    ${LLVM_LIBRARY_DIR}/liblldWasm.a
    ${LLVM_LIBRARY_DIR}/liblldReaderWriter.a
    ${LLVM_LIBRARY_DIR}/liblldDriver.a
    ${LLVM_LIBRARY_DIR}/liblldCore.a
    ${LLVM_LIBRARY_DIR}/liblldCommon.a
    # This would be one or two top level libs such as LLVMCore, LLVMLibDriver, or similar,
    # however the previous LLD libs have dependencies on LLVM libs, and there are no exported cmake
    # targets with defined INTERFACE_LINK_LIBRARIES, so we just throw the whole set of LLVM libs
    # into the link path
    ${LLVM_AVAILABLE_LIBS}
    CONAN_PKG::fmt
    CONAN_PKG::spdlog
  PRIVATE 
    compile_options spawn_utils
)

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
