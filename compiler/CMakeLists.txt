add_executable(spawn_exe_launcher spawn_exe_launcher.c)
target_link_libraries(spawn_exe_launcher ${CMAKE_DL_LIBS})

add_library(compiler compiler.hpp compiler.cpp)

message(
  "To get the compiler working on windows, you need: clang-10 custom build with appropriate version of MSVC" 
  "; built with LLVM_ENABLE_PROJECTS=clang;lld LLVM_TARGETS_TO_BUILD=X86 LLVM_TOOL_LLD_BUILD=TRUE. Make sure NSIS is installed, and `make PACKAGE`."
  "Note that downloading the official source package will not work, because it has symlinks. You have to clone the github repository, checkout 10.0.1, and configure "
  "the llvm subdirectory. What else... oh right. You'll get a build error. Comment out line 181 in type_traits.h when you get there. It'll be fine. Trust me."
  "Once the package is built, it should install fine, and be found by this script"  
)

# we only support llvm-10 right now
find_package(LLVM 10 REQUIRED CONFIG)
# We want to nudge cmake towards clang-10, so it doesn't find the wrong libclang
# we cannot use the version number because clang's cmake files don't have versioning
find_package(Clang REQUIRED CONFIG HINTS /usr/lib/cmake/clang-10 "C:/Program Files/LLVM/lib/cmake/clang")


target_include_directories(compiler PRIVATE "${CMAKE_SOURCE_DIR}")
target_include_directories(compiler PRIVATE "${CMAKE_BINARY_DIR}")
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
  PRIVATE compile_options spawn_utils)

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
