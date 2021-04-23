#if defined(_MSC_VER)
#include "dynamiclibrary_windows.hpp"
#elif defined(HAVE_UNISTD_H)
#include "dynamiclibrary_posix.hpp"
#endif
