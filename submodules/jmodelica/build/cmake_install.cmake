# Install script for directory: /Users/kbenne/Development/jmodelica/trunk/ThirdParty/Sundials/sundials-2.7.0

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/sundials" TYPE FILE FILES "/Users/kbenne/Development/jmodelica/trunk/build/include/sundials/sundials_config.h")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/kbenne/Development/jmodelica/trunk/build/src/sundials/cmake_install.cmake")
  include("/Users/kbenne/Development/jmodelica/trunk/build/src/nvec_ser/cmake_install.cmake")
  include("/Users/kbenne/Development/jmodelica/trunk/build/src/arkode/cmake_install.cmake")
  include("/Users/kbenne/Development/jmodelica/trunk/build/src/cvode/cmake_install.cmake")
  include("/Users/kbenne/Development/jmodelica/trunk/build/src/cvodes/cmake_install.cmake")
  include("/Users/kbenne/Development/jmodelica/trunk/build/src/ida/cmake_install.cmake")
  include("/Users/kbenne/Development/jmodelica/trunk/build/src/idas/cmake_install.cmake")
  include("/Users/kbenne/Development/jmodelica/trunk/build/src/kinsol/cmake_install.cmake")
  include("/Users/kbenne/Development/jmodelica/trunk/build/examples/arkode/C_serial/cmake_install.cmake")
  include("/Users/kbenne/Development/jmodelica/trunk/build/examples/cvode/serial/cmake_install.cmake")
  include("/Users/kbenne/Development/jmodelica/trunk/build/examples/cvodes/serial/cmake_install.cmake")
  include("/Users/kbenne/Development/jmodelica/trunk/build/examples/ida/serial/cmake_install.cmake")
  include("/Users/kbenne/Development/jmodelica/trunk/build/examples/idas/serial/cmake_install.cmake")
  include("/Users/kbenne/Development/jmodelica/trunk/build/examples/kinsol/serial/cmake_install.cmake")
  include("/Users/kbenne/Development/jmodelica/trunk/build/examples/nvector/serial/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/Users/kbenne/Development/jmodelica/trunk/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
