#include "platform.hpp"

namespace spawn {

std::string epfmiName() {
  // Configure this using cmake
  #ifdef __APPLE__
    return "epfmi.dylib";
  #elif _WIN32
    return "epfmi.dll";
  #else
    return "epfmi.so";
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

