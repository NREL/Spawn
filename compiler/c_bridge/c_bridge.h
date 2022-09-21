#ifndef C_BRIDGE_H
#define C_BRIDGE_H

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


// stdio.h
C_BRIDGE_API int snprintf_wrap(char *restrict buffer, unsigned long long bufsz, const char *restrict format, ...);

// string.h
C_BRIDGE_API void *memset_wrap(void *dest, int ch, unsigned long long count);

// math.h
C_BRIDGE_API double cos_wrap(double);

#ifndef C_BRIDGE_IMPL

// stdio.h
#define snprintf snprintf_wrap

// string.h
#define memset cos_wrap

// math.h
#define cos cos_wrap

#endif

#endif
