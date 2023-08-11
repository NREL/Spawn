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

#if !defined(_MSC_VER) && !defined(C_BRIDGE_IMPL)
typedef unsigned long long size_t;
#endif

/// Note how the set of supported functions all have _wrap appended?
/// they are implemented in c_bridge.c

// stdio.h
C_BRIDGE_API int snprintf_wrap(char *restrict buffer, size_t bufsz, const char *restrict format, ...);
C_BRIDGE_API double fmin_wrap(double x, double y);
C_BRIDGE_API double fmax_wrap(double x, double y);

typedef struct C_BRIDGE_API FILE_wrap
{
  void *file_ptr;
} FILE_wrap;

C_BRIDGE_API int fflush_wrap(FILE_wrap *stream);
C_BRIDGE_API int fprintf_wrap(FILE_wrap *stream, const char *format, ...);

extern FILE_wrap *stderr_wrap;
extern FILE_wrap *stdout_wrap;
extern FILE_wrap *stdin_wrap;

/// if C_BRIDGE_IMPL is *not* defined (when this library is used, not being compiled)
/// then we need to rename all of the functions we provide

#ifndef C_BRIDGE_IMPL

#define snprintf snprintf_wrap
#define fmin fmin_wrap
#define fmax fmax_wrap
#define stderr stderr_wrap
#define stdout stdout_wrap
#define stdin stdin_wrap
#define FILE FILE_wrap
#define fflush fflush_wrap
#define fprintf fprintf_wrap

/// Created by: https://docs.google.com/spreadsheets/d/126iyiRC-fspn9O-0QV39JneEetiVtKzm_KmFJkNxPGo/edit?usp=sharing

///
/// ctype.h
///

C_BRIDGE_API int isalnum_wrap(int c);
C_BRIDGE_API int isalpha_wrap(int c);
C_BRIDGE_API int isascii_wrap(int c);
C_BRIDGE_API int isblank_wrap(int c);
C_BRIDGE_API int iscntrl_wrap(int c);
C_BRIDGE_API int isdigit_wrap(int c);
C_BRIDGE_API int isgraph_wrap(int c);
C_BRIDGE_API int islower_wrap(int c);
C_BRIDGE_API int isprint_wrap(int c);
C_BRIDGE_API int ispunct_wrap(int c);
C_BRIDGE_API int isspace_wrap(int c);
C_BRIDGE_API int isupper_wrap(int c);
C_BRIDGE_API int toascii_wrap(int c);
C_BRIDGE_API int tolower_wrap(int c);
C_BRIDGE_API int toupper_wrap(int c);

///
/// math.h
///

C_BRIDGE_API double acos_wrap(double x);
C_BRIDGE_API double asin_wrap(double x);
C_BRIDGE_API double atan_wrap(double x);
C_BRIDGE_API double atan2_wrap(double y, double x);
C_BRIDGE_API double ceil_wrap(double x);
C_BRIDGE_API double cos_wrap(double x);
C_BRIDGE_API double cosh_wrap(double x);
C_BRIDGE_API double erf_wrap(double x);
C_BRIDGE_API double erfc_wrap(double x);
C_BRIDGE_API double exp_wrap(double x);
C_BRIDGE_API double fabs_wrap(double x);
C_BRIDGE_API double floor_wrap(double x);
C_BRIDGE_API double fmod_wrap(double x, double y);
C_BRIDGE_API double frexp_wrap(double x, int *expptr);
C_BRIDGE_API double gamma_wrap(double x);
C_BRIDGE_API double hypot_wrap(double side1, double side2);
C_BRIDGE_API double j0_wrap(double x);
C_BRIDGE_API double j1_wrap(double x);
C_BRIDGE_API double jn_wrap(int n, double x);
C_BRIDGE_API double ldexp_wrap(double x, int exp);
C_BRIDGE_API double log_wrap(double x);
C_BRIDGE_API double log10_wrap(double x);
C_BRIDGE_API double modf_wrap(double x, double *intptr);
C_BRIDGE_API double nextafter_wrap(double x, double y);
C_BRIDGE_API long double nextafterl_wrap(long double x, long double y);
C_BRIDGE_API double nexttoward_wrap(double x, long double y);
C_BRIDGE_API long double nexttowardl_wrap(long double x, long double y);
C_BRIDGE_API double pow_wrap(double x, double y);
C_BRIDGE_API double sin_wrap(double x);
C_BRIDGE_API double sinh_wrap(double x);
C_BRIDGE_API double sqrt_wrap(double x);
C_BRIDGE_API double tan_wrap(double x);
C_BRIDGE_API double tanh_wrap(double x);
C_BRIDGE_API double y0_wrap(double x);
C_BRIDGE_API double y1_wrap(double x);
C_BRIDGE_API double yn_wrap(int n, double x);

///
/// stdio.h
///

C_BRIDGE_API int getchar_wrap(void);
C_BRIDGE_API void perror_wrap(const char *string);
// C_BRIDGE_API int printf_wrap(const char *format_string, ...);
C_BRIDGE_API int putchar_wrap(int c);
C_BRIDGE_API int puts_wrap(const char *string);
C_BRIDGE_API int remove_wrap(const char *filename);
C_BRIDGE_API int rename_wrap(const char *oldname, const char *newname);
// C_BRIDGE_API int scanf_wrap(const char *format_string, ...);
// C_BRIDGE_API int snprintf_wrap(char *outbuf, size_t n, const char*, ...);
// C_BRIDGE_API int sprintf_wrap(char *buffer, const char *format_string, ...);
// C_BRIDGE_API int sscanf_wrap(const char *buffer, const char *format, ...);
C_BRIDGE_API char *tmpnam_wrap(char *string);

///
/// stdlib.h
///

C_BRIDGE_API void abort_wrap(void);
C_BRIDGE_API int abs_wrap(int n);
C_BRIDGE_API double atof_wrap(const char *string);
C_BRIDGE_API int atoi_wrap(const char *string);
C_BRIDGE_API long int atol_wrap(const char *string);
C_BRIDGE_API void *calloc_wrap(size_t num, size_t size);
C_BRIDGE_API void exit_wrap(int status);
C_BRIDGE_API void free_wrap(void *ptr);
C_BRIDGE_API char *getenv_wrap(const char *varname);
C_BRIDGE_API long int labs_wrap(long int n);
C_BRIDGE_API ldiv_t ldiv_wrap(long int numerator, long int denominator);
C_BRIDGE_API void *malloc_wrap(size_t size);
C_BRIDGE_API int mblen_wrap(const char *string, size_t n);
C_BRIDGE_API int rand_wrap(void);
C_BRIDGE_API void *realloc_wrap(void *ptr, size_t size);
C_BRIDGE_API void srand_wrap(unsigned int seed);
C_BRIDGE_API double strtod_wrap(const char *nptr, char **endptr);
C_BRIDGE_API float strtof_wrap(const char *nptr, char **endptr);
C_BRIDGE_API long int strtol_wrap(const char *nptr, char **endptr, int base);
C_BRIDGE_API long double strtold_wrap(const char *nptr, char **endptr);
C_BRIDGE_API unsigned long int strtoul_wrap(const char *string1, char **string2, int base);
C_BRIDGE_API int system_wrap(const char *string);

///
/// string.h
///

C_BRIDGE_API void *memchr_wrap(const void *buf, int c, size_t count);
C_BRIDGE_API int memcmp_wrap(const void *buf1, const void *buf2, size_t count);
C_BRIDGE_API void *memcpy_wrap(void *dest, const void *src, size_t count);
C_BRIDGE_API void *memmove_wrap(void *dest, const void *src, size_t count);
C_BRIDGE_API void *memset_wrap(void *dest, int c, size_t count);
C_BRIDGE_API char *strcat_wrap(char *string1, const char *string2);
C_BRIDGE_API char *strchr_wrap(const char *string, int c);
C_BRIDGE_API int strcmp_wrap(const char *string1, const char *string2);
C_BRIDGE_API int strcoll_wrap(const char *string1, const char *string2);
C_BRIDGE_API char *strcpy_wrap(char *string1, const char *string2);
C_BRIDGE_API size_t strcspn_wrap(const char *string1, const char *string2);
C_BRIDGE_API char *strerror_wrap(int errnum);
C_BRIDGE_API size_t strlen_wrap(const char *string);
C_BRIDGE_API char *strncat_wrap(char *string1, const char *string2, size_t count);
C_BRIDGE_API int strncmp_wrap(const char *string1, const char *string2, size_t count);
C_BRIDGE_API char *strncpy_wrap(char *string1, const char *string2, size_t count);
C_BRIDGE_API char *strpbrk_wrap(const char *string1, const char *string2);
C_BRIDGE_API char *strrchr_wrap(const char *string, int c);
C_BRIDGE_API size_t strspn_wrap(const char *string1, const char *string2);
C_BRIDGE_API char *strstr_wrap(const char *string1, const char *string2);
C_BRIDGE_API char *strtok_wrap(char *string1, const char *string2);
C_BRIDGE_API char *strtok_r_wrap(char *string, const char *seps, char **lasts);
C_BRIDGE_API size_t strxfrm_wrap(char *string1, const char *string2, size_t count);

#define isalnum isalnum_wrap
#define isalpha isalpha_wrap
#define isascii isascii_wrap
#define isblank isblank_wrap
#define iscntrl iscntrl_wrap
#define isdigit isdigit_wrap
#define isgraph isgraph_wrap
#define islower islower_wrap
#define isprint isprint_wrap
#define ispunct ispunct_wrap
#define isspace isspace_wrap
#define isupper isupper_wrap
#define toascii toascii_wrap
#define tolower tolower_wrap
#define toupper toupper_wrap
#define acos acos_wrap
#define asin asin_wrap
#define atan atan_wrap
#define atan2 atan2_wrap
#define ceil ceil_wrap
#define cos cos_wrap
#define cosh cosh_wrap
#define erf erf_wrap
#define erfc erfc_wrap
#define exp exp_wrap
#define fabs fabs_wrap
#define floor floor_wrap
#define fmod fmod_wrap
#define frexp frexp_wrap
#define gamma gamma_wrap
#define hypot hypot_wrap
#define j0 j0_wrap
#define j1 j1_wrap
#define jn jn_wrap
#define ldexp ldexp_wrap
#define log log_wrap
#define log10 log10_wrap
#define modf modf_wrap
#define nextafter nextafter_wrap
#define nextafterl nextafterl_wrap
#define nexttoward nexttoward_wrap
#define nexttowardl nexttowardl_wrap
#define pow pow_wrap
#define sin sin_wrap
#define sinh sinh_wrap
#define sqrt sqrt_wrap
#define tan tan_wrap
#define tanh tanh_wrap
#define y0 y0_wrap
#define y1 y1_wrap
#define yn yn_wrap
#define getchar getchar_wrap
#define perror perror_wrap
// #define printf printf_wrap
#define putchar putchar_wrap
#define puts puts_wrap
#define remove remove_wrap
#define rename rename_wrap
// #define scanf scanf_wrap
// #define snprintf snprintf_wrap
// #define sprintf sprintf_wrap
// #define sscanf sscanf_wrap
#define tmpnam tmpnam_wrap
#define abort abort_wrap
#define abs abs_wrap
#define atof atof_wrap
#define atoi atoi_wrap
#define atol atol_wrap
#define calloc calloc_wrap
#define exit exit_wrap
#define free free_wrap
#define getenv getenv_wrap
#define labs labs_wrap
#define ldiv ldiv_wrap
#define malloc malloc_wrap
#define mblen mblen_wrap
#define rand rand_wrap
#define realloc realloc_wrap
#define srand srand_wrap
#define strtod strtod_wrap
#define strtof strtof_wrap
#define strtol strtol_wrap
#define strtold strtold_wrap
#define strtoul strtoul_wrap
#define system system_wrap
#define memchr memchr_wrap
#define memcmp memcmp_wrap
#define memcpy memcpy_wrap
#define memmove memmove_wrap
#define memset memset_wrap
#define strcat strcat_wrap
#define strchr strchr_wrap
#define strcmp strcmp_wrap
#define strcoll strcoll_wrap
#define strcpy strcpy_wrap
#define strcspn strcspn_wrap
#define strerror strerror_wrap
#define strlen strlen_wrap
#define strncat strncat_wrap
#define strncmp strncmp_wrap
#define strncpy strncpy_wrap
#define strpbrk strpbrk_wrap
#define strrchr strrchr_wrap
#define strspn strspn_wrap
#define strstr strstr_wrap
#define strtok strtok_wrap
#define strtok_r strtok_r_wrap
#define strxfrm strxfrm_wrap

#endif

#endif
