set(CMAKE_CXX_STANDARD 11)


add_executable(CreateEmbeddedSource
  CreateEmbeddedSource.cpp
)

target_link_libraries(CreateEmbeddedSource PRIVATE CONAN_PKG::zlib compile_options)

