# Install script for directory: /Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/src/sundials

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
Install shared components
")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/sundials" TYPE FILE FILES
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/include/sundials/sundials_band.h"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/include/sundials/sundials_dense.h"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/include/sundials/sundials_direct.h"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/include/sundials/sundials_iterative.h"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/include/sundials/sundials_math.h"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/include/sundials/sundials_nvector.h"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/include/sundials/sundials_fnvector.h"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/include/sundials/sundials_pcg.h"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/include/sundials/sundials_sparse.h"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/include/sundials/sundials_spbcgs.h"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/include/sundials/sundials_spfgmr.h"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/include/sundials/sundials_spgmr.h"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/include/sundials/sundials_sptfqmr.h"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/include/sundials/sundials_types.h"
    )
endif()

