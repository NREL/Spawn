set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
find_package(BZip2)
find_package(ZLIB)
find_package(libzip)
find_package(pugixml)
find_package(CLI11)
find_package(Catch2)
find_package(nlohmann_json)
find_package(Boost)

# spdlog is included in the project as a git subtree, because
# it depends on fmt, which is pulled in by the energyplus project.
# Normally, we would use conan to pull in spdlog, but the conan build
# is unaware of fmt provided by EnergyPlus
set(SPDLOG_FMT_EXTERNAL ON)
set(SPDLOG_BUILD_PIC ON)
add_subdirectory(spdlog)

# Provides the cmake macro, `catch_discover_tests`
include(Catch)
