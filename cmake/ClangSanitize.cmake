# Enable runtime checking features
if(CMAKE_COMPILER_IS_GNUCC OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  target_compile_options(compile_options INTERFACE -Wall -Wextra -Wpedantic -Werror)
  option(ENABLE_ADDRESS_SANITIZER "Enable address sanitizer testing in gcc/clang" OFF)
  if(ENABLE_ADDRESS_SANITIZER)
    target_compile_options(compile_options INTERFACE -fsanitize=address -g)
    target_link_options(compile_options INTERFACE -fsanitize=address)
  endif()

  option(ENABLE_UNDEFINED_SANITIZER "Enable undefined behavior sanitizer testing in gcc/clang" OFF)
  if(ENABLE_UNDEFINED_SANITIZER)
    target_compile_options(compile_options INTERFACE -fsanitize=undefined -g)
    target_link_options(compile_options INTERFACE -fsanitize=undefined)
  endif()

endif()
