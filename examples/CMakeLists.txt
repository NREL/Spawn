add_executable( sim sim.cpp )
target_compile_features(sim PRIVATE cxx_std_17)
target_link_libraries(sim PRIVATE spawn_utils fmu CONAN_PKG::spdlog )

