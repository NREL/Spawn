add_library(spawn_utils
        temp_directory.cpp
        temp_directory.hpp
        unzipped_file.cpp
        unzipped_file.hpp
        dynamiclibrary.hpp
        dynamiclibrary_windows.hpp
        dynamiclibrary_posix.hpp
        )

target_link_libraries(spawn_utils PUBLIC CONAN_PKG::boost CONAN_PKG::fmt PRIVATE CONAN_PKG::libzip)