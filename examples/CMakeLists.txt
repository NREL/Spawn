add_executable(sim sim.cpp)
target_link_libraries(sim PRIVATE spawn_utils fmu CONAN_PKG::spdlog compile_options)
