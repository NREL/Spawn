include(CheckIncludeFile)
include(CheckIncludeFileCXX)

add_library(compile_options INTERFACE)
target_compile_features(compile_options INTERFACE cxx_std_17)
target_include_directories(compile_options INTERFACE ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})

check_include_file("unistd.h" HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
  target_compile_definitions(compile_options INTERFACE "-DHAVE_UNISTD_H")
endif()

check_include_file_cxx("filesystem" HAVE_FILESYSTEM_H)
if(HAVE_FILESYSTEM_H)
  target_compile_definitions(compile_options INTERFACE "-DHAVE_FILESYSTEM_H")
endif()

check_include_file_cxx("experimental/filesystem" HAVE_EXP_FILESYSTEM_H)
if(HAVE_EXP_FILESYSTEM_H)
  target_compile_definitions(compile_options INTERFACE "-DHAVE_EXP_FILESYSTEM_H")
endif()
