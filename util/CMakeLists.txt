
if (UNIX)
  ADD_DEFINITIONS("-fPIC")
endif()

add_library(spawn_utils
        compare.hpp
        compare.cpp
        filesystem.hpp
        idf_to_json.hpp
        idf_to_json.cpp
        fmi_paths.hpp
        temp_directory.cpp
        temp_directory.hpp
        unzipped_file.cpp
        unzipped_file.hpp
        dynamiclibrary.hpp
        dynamiclibrary_windows.hpp
        dynamiclibrary_posix.hpp
        )

target_link_libraries(spawn_utils PUBLIC energyplusapi CONAN_PKG::fmt PRIVATE CONAN_PKG::libzip)

target_include_directories(spawn_utils
  PRIVATE
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/src/EnergyPlus/public
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/ObjexxFCL/src/
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/FMI2/
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/CLI/
  ${PROJECT_SOURCE_DIR}/submodules/EnergyPlus/third_party/
)

