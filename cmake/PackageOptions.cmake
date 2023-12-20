set(package_variant "")
if(NOT ENABLE_COMPILER)
  set(package_variant "light-")
endif()

set(CPACK_PACKAGE_VERSION
    "${package_variant}${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}-${CMAKE_PROJECT_VERSION_BUILD}")

if(WIN32)
  set(CPACK_GENERATOR ZIP)
else()
  set(CPACK_GENERATOR TGZ)
endif()
