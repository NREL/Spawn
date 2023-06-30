option(ENABLE_CLANG_TIDY "Enable clang-tidy checks during compilation" ON)

if(ENABLE_CLANG_TIDY)
  find_program(CLANGTIDY NAMES clang-tidy-12 clang-tidy-10 clang-tidy)

  if(CLANGTIDY)
    set(CMAKE_CXX_CLANG_TIDY ${CLANGTIDY} -extra-arg=-Wno-unknown-warning-option)
    if(${CMAKE_CXX_STANDARD})
      set(CMAKE_CXX_CLANG_TIDY ${CMAKE_CXX_CLANG_TIDY} -extra-arg=-std=c++${CMAKE_CXX_STANDARD})
    endif()
  else()
    message(${WARNING_MESSAGE} "clang-tidy requested but executable not found")
  endif()
endif()
