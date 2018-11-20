# Install script for directory: /Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/src/idas

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  MESSAGE("
Install IDAS
")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/kbenne/Development/jmodelica/trunk/build/src/idas/libsundials_idas.a")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsundials_idas.a" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsundials_idas.a")
    execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsundials_idas.a")
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/Users/kbenne/Development/jmodelica/trunk/build/src/idas/libsundials_idas.1.3.0.dylib"
    "/Users/kbenne/Development/jmodelica/trunk/build/src/idas/libsundials_idas.1.dylib"
    "/Users/kbenne/Development/jmodelica/trunk/build/src/idas/libsundials_idas.dylib"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsundials_idas.1.3.0.dylib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsundials_idas.1.dylib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsundials_idas.dylib"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      execute_process(COMMAND "/usr/bin/install_name_tool"
        -id "libsundials_idas.1.dylib"
        "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/idas" TYPE FILE FILES
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/include/idas/idas_band.h"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/include/idas/idas_bbdpre.h"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/include/idas/idas_dense.h"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/include/idas/idas_direct.h"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/include/idas/idas.h"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/include/idas/idas_sparse.h"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/include/idas/idas_spbcgs.h"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/include/idas/idas_spgmr.h"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/include/idas/idas_spils.h"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/include/idas/idas_sptfqmr.h"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/idas" TYPE FILE FILES "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/src/idas/idas_impl.h")
endif()

