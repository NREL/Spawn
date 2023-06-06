
  include(ExternalProject)
  ExternalProject_Add(
    OpenModelica
    BUILD_ALWAYS 1
    #BUILD_IN_SOURCE
    GIT_REPOSITORY https://github.com/OpenModelica/OpenModelica.git
    GIT_TAG v1.20.0
    GIT_SHALLOW
    GIT_SUBMODULES_RECURSE 1
    #BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config $<CONFIG>
    #COMMAND ${CMAKE_COMMAND}
    )
