#include <boost/filesystem/path.hpp>

#if defined(_MSC_VER)
#include "dynamiclibrary_windows.hpp"
#elif defined(_POSIX_VERSION)
#include "dynamiclibrary_posix.hpp"
#endif

