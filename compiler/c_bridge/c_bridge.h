/// c_bridge exists as a way to avoid requiring a C standard library
/// on the target system.
///
/// The c_bridge library is compiled when `spawn` is compiled. It provides
/// a shared library that is embedded into the `spawn` executable.
///
/// The library acts as a "bridge" between the runtime environment of `spawn`
/// and the compile-time environment of the system the library was compiled on
///
/// This is desired because packaging and shipping the necessary .lib files on
/// Windows is not allowed legally, and difficult on Linux.
///
/// This library compiles a series of wrapper functions and provides them
/// to the embedded `clang` instance during compilation and runtime linking
///
/// The set of functions exposed is very small, limited to only what is required
/// for our use case. It will need to expanded.
///
/// This file is used in two ways
///  1. Included by c_bridge.c during compilation of the c_bridge library
///  2. Included via one of the replacement headers, such as <stdlib.h>
///
///
/// BUILDING C_BRIDGE LIBRARY:
///
/// When used to build c_bridge.c the c_bridge_EXPORTS macro and
/// C_BRIDGE_IMPL macros are set and the compilation proceeds exactly as seen.
///
/// Each function has _wrap appended to it to avoid conflicts with
/// the compilation system's C standard library.
///
///
/// USING THE C_BRIDGE LIBRARY:
///
/// During use (in `spawn` deployment time) the C_BRIDGE_IMPL macro is *not* set and
/// a series of macros rename the supported functions so they appear to be the
/// stdlib functions:
/// (example `#define cos cos_wrap`)
///
/// we provide a full set of c standard library headers that simply include this header
///
///
/// END RESULT
///
/// * We don't need to package, deploy, or find the c standard library on user's systems!
///
///
/// DOWNSIDES
///
/// only the functions we wrap exist for use in the deployed system
///
///
/// EXTENDING
///
/// To add a new c library function:
///  1. Add the function prototype with its friends from the same C header below, with `_wrap` appended
///  2. Provide an appropriate rename macro
///  3. Implement the function in c_bridge.c

#ifndef C_BRIDGE_H
#define C_BRIDGE_H

/// this define exists to make it detectable which stdlib we are using
#define C_BRIDGE_STDLIB

#ifdef _MSC_VER
#if defined(c_bridge_EXPORTS)
#define C_BRIDGE_API __declspec(dllexport)
#define C_BRIDGE_TEMPLATE_EXT
#else
#define C_BRIDGE_API __declspec(dllimport)
#define C_BRIDGE_TEMPLATE_EXT extern
#endif
#else
#define C_BRIDGE_API
#define C_BRIDGE_TEMPLATE_EXT
#endif

/// TODO: We're probably going to have to deal with size_t at some point

/// Note how the set of supported functions all have _wrap appended?
/// they are implemented in c_bridge.c

// stdio.h
C_BRIDGE_API int snprintf_wrap(char *restrict buffer, size_t bufsz, const char *restrict format, ...);

// string.h
C_BRIDGE_API void *memset_wrap(void *dest, int ch, size_t count);

// math.h
C_BRIDGE_API double cos_wrap(double);

/// if C_BRIDGE_IMPL is *not* defined (when this library is used, not being compiled)
/// then we need to rename all of the functions we provide

#ifndef C_BRIDGE_IMPL

// stdio.h
#define snprintf snprintf_wrap

// string.h
#define memset memset_wrap

// math.h
#define cos cos_wrap

#endif

#endif
