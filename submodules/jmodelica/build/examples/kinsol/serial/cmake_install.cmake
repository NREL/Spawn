# Install script for directory: /Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/kinsol/serial

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
   "/usr/local/examples/kinsol/serial/kinFerTron_dns.c;/usr/local/examples/kinsol/serial/kinFerTron_dns.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/examples/kinsol/serial" TYPE FILE FILES
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/kinsol/serial/kinFerTron_dns.c"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/kinsol/serial/kinFerTron_dns.out"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/examples/kinsol/serial/kinFoodWeb_kry.c;/usr/local/examples/kinsol/serial/kinFoodWeb_kry.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/examples/kinsol/serial" TYPE FILE FILES
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/kinsol/serial/kinFoodWeb_kry.c"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/kinsol/serial/kinFoodWeb_kry.out"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/examples/kinsol/serial/kinKrylovDemo_ls.c;/usr/local/examples/kinsol/serial/kinKrylovDemo_ls.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/examples/kinsol/serial" TYPE FILE FILES
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/kinsol/serial/kinKrylovDemo_ls.c"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/kinsol/serial/kinKrylovDemo_ls.out"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/examples/kinsol/serial/kinLaplace_bnd.c;/usr/local/examples/kinsol/serial/kinLaplace_bnd.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/examples/kinsol/serial" TYPE FILE FILES
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/kinsol/serial/kinLaplace_bnd.c"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/kinsol/serial/kinLaplace_bnd.out"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/examples/kinsol/serial/kinLaplace_picard_bnd.c;/usr/local/examples/kinsol/serial/kinLaplace_picard_bnd.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/examples/kinsol/serial" TYPE FILE FILES
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/kinsol/serial/kinLaplace_picard_bnd.c"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/kinsol/serial/kinLaplace_picard_bnd.out"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/examples/kinsol/serial/kinRoberts_fp.c;/usr/local/examples/kinsol/serial/kinRoberts_fp.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/examples/kinsol/serial" TYPE FILE FILES
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/kinsol/serial/kinRoberts_fp.c"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/kinsol/serial/kinRoberts_fp.out"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/examples/kinsol/serial/kinRoboKin_dns.c;/usr/local/examples/kinsol/serial/kinRoboKin_dns.out")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/examples/kinsol/serial" TYPE FILE FILES
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/kinsol/serial/kinRoboKin_dns.c"
    "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/kinsol/serial/kinRoboKin_dns.out"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/examples/kinsol/serial/README")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/examples/kinsol/serial" TYPE FILE FILES "/Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0/examples/kinsol/serial/README")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/examples/kinsol/serial/CMakeLists.txt")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/examples/kinsol/serial" TYPE FILE FILES "/Users/kbenne/Development/jmodelica/trunk/build/examples/kinsol/serial/CMakeLists.txt")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/examples/kinsol/serial/Makefile")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/examples/kinsol/serial" TYPE FILE RENAME "Makefile" FILES "/Users/kbenne/Development/jmodelica/trunk/build/examples/kinsol/serial/Makefile_ex")
endif()

