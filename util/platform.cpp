#include "platform.hpp"

namespace spawn {

std::string epfmiName() {
  // Configure this using cmake
  #ifdef __APPLE__
    return "libepfmi.dylib";
  #elif _WIN32
    return "epfmi.dll";
  #else
    return "libepfmi.so";
  #endif
}

std::string fmiplatform() {
  #ifdef __APPLE__
    return "darwin64";
  #elif _WIN32
    return "win64";
  #else
    return "linux64";
  #endif
}

}

