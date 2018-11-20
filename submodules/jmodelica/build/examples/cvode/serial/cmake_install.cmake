# Install script for directory: /Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/cvode/serial

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
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/examples/cvode/serial/cvAdvDiff_bnd.c;/usr/local/examples/cvode/serial/cvAdvDiff_bnd.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/examples/cvode/serial" TYPE FILE FILES
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/cvode/serial/cvAdvDiff_bnd.c"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/cvode/serial/cvAdvDiff_bnd.out"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/examples/cvode/serial/cvDirectDemo_ls.c;/usr/local/examples/cvode/serial/cvDirectDemo_ls.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/examples/cvode/serial" TYPE FILE FILES
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/cvode/serial/cvDirectDemo_ls.c"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/cvode/serial/cvDirectDemo_ls.out"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/examples/cvode/serial/cvDiurnal_kry_bp.c;/usr/local/examples/cvode/serial/cvDiurnal_kry_bp.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/examples/cvode/serial" TYPE FILE FILES
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/cvode/serial/cvDiurnal_kry_bp.c"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/cvode/serial/cvDiurnal_kry_bp.out"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/examples/cvode/serial/cvDiurnal_kry.c;/usr/local/examples/cvode/serial/cvDiurnal_kry.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/examples/cvode/serial" TYPE FILE FILES
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/cvode/serial/cvDiurnal_kry.c"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/cvode/serial/cvDiurnal_kry.out"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/examples/cvode/serial/cvKrylovDemo_ls.c;/usr/local/examples/cvode/serial/cvKrylovDemo_ls.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/examples/cvode/serial" TYPE FILE FILES
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/cvode/serial/cvKrylovDemo_ls.c"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/cvode/serial/cvKrylovDemo_ls.out"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/examples/cvode/serial/cvKrylovDemo_prec.c;/usr/local/examples/cvode/serial/cvKrylovDemo_prec.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/examples/cvode/serial" TYPE FILE FILES
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/cvode/serial/cvKrylovDemo_prec.c"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/cvode/serial/cvKrylovDemo_prec.out"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/examples/cvode/serial/cvRoberts_dns.c;/usr/local/examples/cvode/serial/cvRoberts_dns.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/examples/cvode/serial" TYPE FILE FILES
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/cvode/serial/cvRoberts_dns.c"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/cvode/serial/cvRoberts_dns.out"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/examples/cvode/serial/cvRoberts_dns_uw.c;/usr/local/examples/cvode/serial/cvRoberts_dns_uw.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/examples/cvode/serial" TYPE FILE FILES
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/cvode/serial/cvRoberts_dns_uw.c"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/cvode/serial/cvRoberts_dns_uw.out"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/examples/cvode/serial/README")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/examples/cvode/serial" TYPE FILE FILES "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/cvode/serial/README")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/examples/cvode/serial/CMakeLists.txt")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/examples/cvode/serial" TYPE FILE FILES "/Users/kbenne/Development/jmodelica/trunk/build/examples/cvode/serial/CMakeLists.txt")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/examples/cvode/serial/Makefile")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/examples/cvode/serial" TYPE FILE RENAME "Makefile" FILES "/Users/kbenne/Development/jmodelica/trunk/build/examples/cvode/serial/Makefile_ex")
endif()

