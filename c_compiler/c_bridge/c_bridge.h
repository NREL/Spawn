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

// string.h
C_BRIDGE_API void *memset_wrap(void *dest, int ch, size_t count);

// math.h
C_BRIDGE_API double cos_wrap(double);

/// if C_BRIDGE_IMPL is *not* defined (when this library is used, not being compiled)
/// then we need to rename all of the functions we provide

#ifndef C_BRIDGE_IMPL

// stdio.h
#define snprintf snprintf_wrap

/// Created by: https://docs.google.com/spreadsheets/d/126iyiRC-fspn9O-0QV39JneEetiVtKzm_KmFJkNxPGo/edit?usp=sharing

///
/// assert.h
///

C_BRIDGE_API void assert_wrap(int expression);

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
/// langinfo.h
///

C_BRIDGE_API char *nl_langinfo_wrap(nl_item item);

///
/// locale.h
///

C_BRIDGE_API struct lconv *localeconv_wrap(void);
C_BRIDGE_API char *setlocale_wrap(int category, const char *locale);
C_BRIDGE_API struct wcslconv *wcslocaleconv_wrap(void);

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
// C_BRIDGE_API _Decimal128 quantized128_wrap(_Decimal128 x, _Decimal128 y);
// C_BRIDGE_API _Decimal32 quantized32_wrap(_Decimal32 x, _Decimal32 y);
// C_BRIDGE_API _Decimal64 quantized64_wrap(_Decimal64 x, _Decimal64 y);
// C_BRIDGE_API int quantexpd128_wrap(_Decimal128 x);
// C_BRIDGE_API int quantexpd32_wrap(_Decimal32 x);
// C_BRIDGE_API int quantexpd64_wrap(_Decimal64 x);
// C_BRIDGE_API __bool__ samequantumd128_wrap(_Decimal128 x, _Decimal128 y);
// C_BRIDGE_API __bool__ samequantumd32_wrap(_Decimal32 x, _Decimal32 y);
// C_BRIDGE_API __bool__ samequantumd64_wrap(_Decimal64 x, _Decimal64 y);
C_BRIDGE_API double sin_wrap(double x);
C_BRIDGE_API double sinh_wrap(double x);
C_BRIDGE_API double sqrt_wrap(double x);
C_BRIDGE_API double tan_wrap(double x);
C_BRIDGE_API double tanh_wrap(double x);
C_BRIDGE_API double y0_wrap(double x);
C_BRIDGE_API double y1_wrap(double x);
C_BRIDGE_API double yn_wrap(int n, double x);

///
/// nl_types.h
///

C_BRIDGE_API int catclose_wrap(nl_catd catd);
C_BRIDGE_API char *catgets_wrap(nl_catd catd, int set_id, int msg_id, const char *s);
C_BRIDGE_API nl_catd catopen_wrap(const char *name, int oflag);

///
/// regex.h
///

// C_BRIDGE_API int regcomp_wrap(regex_t *preg, const char *pattern, int cflags);
// C_BRIDGE_API size_t regerror_wrap(int errcode, const regex_t *preg, char *errbuf, size_t errbuf_size);
// C_BRIDGE_API int regexec_wrap(const regex_t *preg, const char *string, size_t nmatch, regmatch_t *pmatch, int
// eflags); C_BRIDGE_API void regfree_wrap(regex_t *preg);

///
/// setjmp.h
///

C_BRIDGE_API void longjmp_wrap(jmp_buf env, int value);
C_BRIDGE_API int setjmp_wrap(jmp_buf env);

///
/// signal.h
///

C_BRIDGE_API int raise_wrap(int sig);
// C_BRIDGE_API void_wrap(*signal _wrap(int sig, void_wrap(*func)_wrap(int))) _wrap(int);

///
/// stdarg.h
///

// C_BRIDGE_API var_type va_arg_wrap(va_list arg_ptr, var_type);
// C_BRIDGE_API void va_copy_wrap(va_list dest, va_list src);
// C_BRIDGE_API void va_end_wrap(va_list arg_ptr);
// C_BRIDGE_API void va_start_wrap(va_list arg_ptr, variable_name);

///
/// wchar.h
///

// C_BRIDGE_API int vfwprintf_wrap(FILE *stream, const wchar_t *format, va_list arg);
// C_BRIDGE_API int vswprintf_wrap(wchar_t *wcsbuffer, size_t n, const wchar_t *format, va_list arg);
// C_BRIDGE_API int vwprintf_wrap(const wchar_t *format, va_list arg);
// C_BRIDGE_API int wctob_wrap(wint_t wc);

///
/// stdio.h
///

C_BRIDGE_API void clearerr_wrap(FILE *stream);
C_BRIDGE_API int fclose_wrap(FILE *stream);
C_BRIDGE_API FILE *fdopen_wrap(int handle, const char *type);
C_BRIDGE_API int feof_wrap(FILE *stream);
C_BRIDGE_API int ferror_wrap(FILE *stream);
C_BRIDGE_API int fflush_wrap(FILE *stream);
C_BRIDGE_API int fgetc_wrap(FILE *stream);
C_BRIDGE_API int fgetpos_wrap(FILE *stream, fpos_t *pos);
C_BRIDGE_API char *fgets_wrap(char *string, int n, FILE *stream);
C_BRIDGE_API int fileno_wrap(FILE *stream);
C_BRIDGE_API FILE *fopen_wrap(const char *filename, const char *mode);
// C_BRIDGE_API int fprintf_wrap(FILE *stream, const char *format_string, ...);
C_BRIDGE_API int fputc_wrap(int c, FILE *stream);
C_BRIDGE_API int fputs_wrap(const char *string, FILE *stream);
C_BRIDGE_API size_t fread_wrap(void *buffer, size_t size, size_t count, FILE *stream);
C_BRIDGE_API FILE *freopen_wrap(const char *filename, const char *mode, FILE *stream);
// C_BRIDGE_API int fscanf_wrap(FILE *stream, const char *format_string, ...);
C_BRIDGE_API int fseek_wrap(FILE *stream, long int offset, int origin);
C_BRIDGE_API int fsetpos_wrap(FILE *stream, const fpos_t *pos);
C_BRIDGE_API long int ftell_wrap(FILE *stream);
C_BRIDGE_API size_t fwrite_wrap(const void *buffer, size_t size, size_t count, FILE *stream);
C_BRIDGE_API int getc_wrap(FILE *stream);
C_BRIDGE_API int getchar_wrap(void);
C_BRIDGE_API char *gets_wrap(char *buffer);
C_BRIDGE_API void perror_wrap(const char *string);
// C_BRIDGE_API int printf_wrap(const char *format_string, ...);
C_BRIDGE_API int putc_wrap(int c, FILE *stream);
C_BRIDGE_API int putchar_wrap(int c);
C_BRIDGE_API int puts_wrap(const char *string);
C_BRIDGE_API int remove_wrap(const char *filename);
C_BRIDGE_API int rename_wrap(const char *oldname, const char *newname);
C_BRIDGE_API void rewind_wrap(FILE *stream);
C_BRIDGE_API int scanf_wrap(const char *format - string, arg - list);
C_BRIDGE_API void setbuf_wrap(FILE *stream, char *buffer);
C_BRIDGE_API int setvbuf_wrap(FILE *stream, char *buf, int type, size_t size);
// C_BRIDGE_API int snprintf_wrap(char *outbuf, size_t n, const char*, ...);
// C_BRIDGE_API int sprintf_wrap(char *buffer, const char *format_string, ...);
// C_BRIDGE_API int sscanf_wrap(const char *buffer, const char *format, ...);
C_BRIDGE_API FILE *tmpfile_wrap(void);
C_BRIDGE_API char *tmpnam_wrap(char *string);
C_BRIDGE_API int ungetc_wrap(int c, FILE *stream);
C_BRIDGE_API int vsnprintf_wrap(char *outbuf, size_t n, const char *, va_list);

///
/// wchar.h
///

// C_BRIDGE_API wint_t btowc_wrap(int c);
// C_BRIDGE_API wint_t fgetwc_wrap(FILE *stream);
// C_BRIDGE_API wchar_t *fgetws_wrap(wchar_t *wcs, int n, FILE *stream);
// C_BRIDGE_API wint_t fputwc_wrap(wchar_t wc, FILE *stream);
// C_BRIDGE_API int fputws_wrap(const wchar_t *wcs, FILE *stream);
C_BRIDGE_API int fwide_wrap(FILE *stream, int mode);
// C_BRIDGE_API int fwprintf_wrap(FILE *stream, const wchar_t *format, ...);
// C_BRIDGE_API int fwscanf_wrap(FILE *stream, const wchar_t *format, ...)
// C_BRIDGE_API wint_t getwc_wrap(FILE *stream);
// C_BRIDGE_API wint_t putwchar_wrap(wchar_t wc, FILE *stream);
// C_BRIDGE_API wint_t ungetwc_wrap(wint_t wc, FILE *stream);

///
/// stdio.h
///

C_BRIDGE_API int vfprintf_wrap(FILE *stream, const char *format, va_list arg_ptr);
C_BRIDGE_API int vprintf_wrap(const char *format, va_list arg_ptr);
C_BRIDGE_API int vsprintf_wrap(char *target - string, const char *format, va_list arg_ptr);
C_BRIDGE_API int vfscanf_wrap(FILE *stream, const char *format, va_list arg_ptr);
// C_BRIDGE_API int vfwscanf_wrap(FILE *stream, const wchar_t *format, va_list arg_ptr);
C_BRIDGE_API int vscanf_wrap(const char *format, va_list arg_ptr);
C_BRIDGE_API int vsscanf_wrap(const char *buffer, const char *format, va_list arg_ptr);
// C_BRIDGE_API int vswscanf_wrap(const wchar_t *buffer, const wchar_t *format, va_list arg_ptr);
// C_BRIDGE_API int vwscanf_wrap(const wchar_t *format, va_list arg_ptr);

///
/// stdlib.h
///

C_BRIDGE_API void abort_wrap(void);
C_BRIDGE_API int abs_wrap(int n);
// C_BRIDGE_API int atexit_wrap(void _wrap(*func)_wrap(void));
C_BRIDGE_API double atof_wrap(const char *string);
C_BRIDGE_API int atoi_wrap(const char *string);
C_BRIDGE_API long int atol_wrap(const char *string);
// C_BRIDGE_API void *bsearch_wrap(const void *key, const void *base, size_t num, size_t size, int _wrap(*compare)
// _wrap(const void *element1, const void *element2));
C_BRIDGE_API void *calloc_wrap(size_t num, size_t size);
C_BRIDGE_API div_t div_wrap(int numerator, int denominator);
C_BRIDGE_API void exit_wrap(int status);
C_BRIDGE_API void free_wrap(void *ptr);
C_BRIDGE_API char *getenv_wrap(const char *varname);
C_BRIDGE_API long int labs_wrap(long int n);
C_BRIDGE_API ldiv_t ldiv_wrap(long int numerator, long int denominator);
C_BRIDGE_API void *malloc_wrap(size_t size);
C_BRIDGE_API int mblen_wrap(const char *string, size_t n);
// C_BRIDGE_API size_t mbstowcs_wrap(wchar_t *pwc, const char *string, size_t n);
// C_BRIDGE_API int mbtowc_wrap(wchar_t *pwc, const char *string, size_t n);
C_BRIDGE_API int *putenv_wrap(const char *varname);
// C_BRIDGE_API void qsort_wrap(void *base, size_t num, size_t width, int_wrap(*compare)_wrap(const void *element1,
// const void *element2));
C_BRIDGE_API int rand_wrap(void);
C_BRIDGE_API int rand_r_wrap(void);
C_BRIDGE_API void *realloc_wrap(void *ptr, size_t size);
C_BRIDGE_API void srand_wrap(unsigned int seed);
C_BRIDGE_API double strtod_wrap(const char *nptr, char **endptr);
// C_BRIDGE_API _Decimal128 strtod128_wrap(const char *nptr, char **endptr);
// C_BRIDGE_API _Decimal32 strtod32_wrap(const char *nptr, char **endptr);
// C_BRIDGE_API _Decimal64 strtod64_wrap(const char *nptr, char **endptr);
C_BRIDGE_API float strtof_wrap(const char *nptr, char **endptr);
C_BRIDGE_API long int strtol_wrap(const char *nptr, char **endptr, int base);
C_BRIDGE_API long double strtold_wrap(const char *nptr, char **endptr);
C_BRIDGE_API unsigned long int strtoul_wrap(const char *string1, char **string2, int base);
C_BRIDGE_API int system_wrap(const char *string);
// C_BRIDGE_API size_t wcstombs_wrap(char *dest, const wchar_t *string, size_t count);
// C_BRIDGE_API int wctomb_wrap(char *string, wchar_t character);

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

///
/// strings.h
///

C_BRIDGE_API int srtcasecmp_wrap(const char *string1, const char *string2);
C_BRIDGE_API int strncasecmp_wrap(const char *string1, const char *string2, size_t count);

///
/// time.h
///

C_BRIDGE_API char *asctime_wrap(const struct tm *time);
C_BRIDGE_API char *asctime_r_wrap(const struct tm *tm, char *buf);
C_BRIDGE_API clock_t clock_wrap(void);
C_BRIDGE_API char *ctime_wrap(const time_t *time);
C_BRIDGE_API char *ctime_r_wrap(const time_t *time, char *buf);
// C_BRIDGE_API char *ctime64_wrap(const time64_t *time);
// C_BRIDGE_API char *ctime64_r_wrap(const time64_t *time, char *buf);
C_BRIDGE_API double difftime_wrap(time_t time2, time_t time1);
// C_BRIDGE_API double difftime64_wrap(time64_t time2, time64_t time1);
C_BRIDGE_API struct tm *gmtime_wrap(const time_t *time);
C_BRIDGE_API struct tm *gmtime_r_wrap(const time_t *time, struct tm *result);
// C_BRIDGE_API struct tm *gmtime64_wrap(const time64_t *time);
// C_BRIDGE_API struct tm *gmtime64_r_wrap(const time64_t *time, struct tm *result);
C_BRIDGE_API struct tm *localtime_wrap(const time_t *timeval);
C_BRIDGE_API struct tm *localtime_r_wrap(const time_t *timeval, struct tm *result);
// C_BRIDGE_API struct tm *localtime64_wrap(const time64_t *timeval);
// C_BRIDGE_API struct tm *localtime64_r_wrap(const time64_t *timeval, struct tm *result);
C_BRIDGE_API time_t mktime_wrap(struct tm *time);
// C_BRIDGE_API time64_t mktime64_wrap(struct tm *time);
C_BRIDGE_API size_t strftime_wrap(char *dest, size_t maxsize, const char *format, const struct tm *timeptr);
C_BRIDGE_API char *strptime_wrap(const char *buf, const char *format, struct tm *tm);
C_BRIDGE_API time_t time_wrap(time_t *timeptr);
// C_BRIDGE_API time64_t time64_wrap(time64_t *timeptr);

///
/// wchar.h
///

// C_BRIDGE_API wint_t getwchar_wrap(void);
C_BRIDGE_API int mbrlen_wrap(const char *s, size_t n, mbstate_t *ps);
// C_BRIDGE_API int mbrtowc_wrap(wchar_t *pwc, const char *s, size_t n, mbstate_t *ps);
C_BRIDGE_API int mbsinit_wrap(const mbstate_t *ps);
// C_BRIDGE_API size_t mbsrtowc_wrap(wchar_t *dst, const char **src, size_t len, mbstate_t *ps);
// C_BRIDGE_API wint_t putwchar_wrap(wchar_t wc);
// C_BRIDGE_API int strfmon_wrap(char *s, size_t maxsize, const char *format, ...);
// C_BRIDGE_API int swprintf_wrap(wchar_t *wcsbuffer, size_t n, const wchar_t *format, ...);
// C_BRIDGE_API int swscanf_wrap(const wchar_t *buffer, const wchar_t *format, ...);
// C_BRIDGE_API int wcrtomb_wrap(char *s, wchar_t wchar, mbstate_t *pss);
// C_BRIDGE_API wchar_t *wcscat_wrap(wchar_t *string1, const wchar_t *string2);
// C_BRIDGE_API wchar_t *wcschr_wrap(const wchar_t *string, wchar_t character);
// C_BRIDGE_API int wcscmp_wrap(const wchar_t *string1, const wchar_t *string2);
// C_BRIDGE_API int wcscoll_wrap(const wchar_t *wcs1, const wchar_t *wcs2);
// C_BRIDGE_API wchar_t *wcscpy_wrap(wchar_t *string1, const wchar_t *string2);
// C_BRIDGE_API size_t wcscspn_wrap(const wchar_t *string1, const wchar_t *string2);
// C_BRIDGE_API size_t wcsftime_wrap(wchar_t *wdest, size_t maxsize, const wchar_t *format, const struct tm *timeptr);
// C_BRIDGE_API size_t wcslen_wrap(const wchar_t *string);
// C_BRIDGE_API wchar_t *wcsncat_wrap(wchar_t *string1, const wchar_t *string2, size_t count);
// C_BRIDGE_API int wcsncmp_wrap(const wchar_t *string1, const wchar_t *string2, size_t count);
// C_BRIDGE_API wchar_t *wcsncpy_wrap(wchar_t *string1, const wchar_t *string2, size_t count);
// C_BRIDGE_API wchar_t *wcspbrk_wrap(const wchar_t *string1, const wchar_t *string2);
// C_BRIDGE_API wchar_t *wcsptime_wrap(const wchar_t *buf, const wchar_t *format, struct tm *tm );
// C_BRIDGE_API wchar_t *wcsrchr_wrap(const wchar_t *string, wchar_t character);
// C_BRIDGE_API size_t wcsrtombs_wrap(char *dst, const wchar_t **src, size_t len, mbstate_t *ps);
// C_BRIDGE_API size_t wcsspn_wrap(const wchar_t *string1, const wchar_t *string2);
// C_BRIDGE_API wchar_t *wcsstr_wrap(const wchar_t *wcs1, const wchar_t *wcs2);
// C_BRIDGE_API double wcstod_wrap(const wchar_t *nptr, wchar_t **endptr);
// C_BRIDGE_API _Decimal128 wcstod128_wrap(const wchar_t *nptr, wchar_t **endptr);
// C_BRIDGE_API _Decimal32 wcstod32_wrap(const wchar_t *nptr, wchar_t **endptr);
// C_BRIDGE_API _Decimal64 wcstod64_wrap(const wchar_t *nptr, wchar_t **endptr);
// C_BRIDGE_API float wcstof_wrap(const wchar_t *nptr, wchar_t **endptr);
// C_BRIDGE_API wchar_t *wcstok_wrap(wchar_t *wcs1, const wchar_t *wcs2, wchar_t **ptr)
// C_BRIDGE_API long int wcstol_wrap(const wchar_t *nptr, wchar_t **endptr, int base);
// C_BRIDGE_API long double wcstold_wrap(const wchar_t *nptr, wchar_t **endptr);
// C_BRIDGE_API unsigned long int wcstoul_wrap(const wchar_t *nptr, wchar_t **endptr, int base);
// C_BRIDGE_API size_t wcsxfrm_wrap(wchar_t *wcs1, const wchar_t *wcs2, size_t n);
C_BRIDGE_API wctype_t wctype_wrap(const char *property);
// C_BRIDGE_API int wcswidth_wrap(const wchar_t *pwcs, size_t n);
// C_BRIDGE_API wchar_t *wmemchr_wrap(const wchar_t *s, wchar_t c, size_t n);
// C_BRIDGE_API int wmemcmp_wrap(const wchar_t *s1, const wchar_t *s2, size_t n);
// C_BRIDGE_API wchar_t *wmemcpy_wrap(wchar_t *s1, const wchar_t *s2, size_t n);
// C_BRIDGE_API wchar_t *wmemmove_wrap(wchar_t *s1, const wchar_t *s2, size_t n);
// C_BRIDGE_API wchar_t *wmemset_wrap(wchar_t *s, wchar_t c, size_t n);
// C_BRIDGE_API int wprintf_wrap(const wchar_t *format, ...);
// C_BRIDGE_API int wscanf_wrap(const wchar_t *format, ...);

///
/// wctype.h
///

// C_BRIDGE_API int iswalnum_wrap(wint_t wc);
// C_BRIDGE_API int iswalpha_wrap(wint_t wc);
// C_BRIDGE_API int iswblank_wrap(wint_t wc);
// C_BRIDGE_API int iswcntrl_wrap(wint_t wc);
// C_BRIDGE_API int iswctype_wrap(wint_t wc, wctype_t wc_prop);
// C_BRIDGE_API int iswdigit_wrap(wint_t wc);
// C_BRIDGE_API int iswgraph_wrap(wint_t wc);
// C_BRIDGE_API int iswlower_wrap(wint_t wc);
// C_BRIDGE_API int iswprint_wrap(wint_t wc);
// C_BRIDGE_API int iswpunct_wrap(wint_t wc);
// C_BRIDGE_API int iswspace_wrap(wint_t wc);
// C_BRIDGE_API int iswupper_wrap(wint_t wc);
// C_BRIDGE_API int iswxdigit_wrap(wint_t wc);
C_BRIDGE_API int isxdigit_wrap(int c);
// C_BRIDGE_API wint_t towctrans_wrap(wint_t wc, wctrans_t desc);
// C_BRIDGE_API wint_t towlower_wrap(wint_t wc);
// C_BRIDGE_API wint_t towupper_wrap(wint_t wc);
C_BRIDGE_API wctrans_t wctrans_wrap(const char *property);

#define assert assert_wrap
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
#define nl_langinfo nl_langinfo_wrap
#define localeconv localeconv_wrap
#define setlocale setlocale_wrap
#define wcslocaleconv wcslocaleconv_wrap
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
// #define quantized128 quantized128_wrap
// #define quantized32 quantized32_wrap
// #define quantized64 quantized64_wrap
// #define quantexpd128 quantexpd128_wrap
// #define quantexpd32 quantexpd32_wrap
// #define quantexpd64 quantexpd64_wrap
// #define samequantumd128 samequantumd128_wrap
// #define samequantumd32 samequantumd32_wrap
// #define samequantumd64 samequantumd64_wrap
#define sin sin_wrap
#define sinh sinh_wrap
#define sqrt sqrt_wrap
#define tan tan_wrap
#define tanh tanh_wrap
#define y0 y0_wrap
#define y1 y1_wrap
#define yn yn_wrap
#define catclose catclose_wrap
#define catgets catgets_wrap
#define catopen catopen_wrap
// #define regcomp regcomp_wrap
// #define regerror regerror_wrap
// #define regexec regexec_wrap
// #define regfree regfree_wrap
#define longjmp longjmp_wrap
#define setjmp setjmp_wrap
#define raise raise_wrap
// #define void void_wrap
// #define va_arg va_arg_wrap
// #define va_copy va_copy_wrap
// #define va_end va_end_wrap
// #define va_start va_start_wrap
// #define vfwprintf vfwprintf_wrap
// #define vswprintf vswprintf_wrap
// #define vwprintf vwprintf_wrap
// #define wctob wctob_wrap
#define clearerr clearerr_wrap
#define fclose fclose_wrap
#define fdopen fdopen_wrap
#define feof feof_wrap
#define ferror ferror_wrap
#define fflush fflush_wrap
#define fgetc fgetc_wrap
#define fgetpos fgetpos_wrap
#define fgets fgets_wrap
#define fileno fileno_wrap
#define fopen fopen_wrap
// #define fprintf fprintf_wrap
#define fputc fputc_wrap
#define fputs fputs_wrap
#define fread fread_wrap
#define freopen freopen_wrap
// #define fscanf fscanf_wrap
#define fseek fseek_wrap
#define fsetpos fsetpos_wrap
#define ftell ftell_wrap
#define fwrite fwrite_wrap
#define getc getc_wrap
#define getchar getchar_wrap
#define gets gets_wrap
#define perror perror_wrap
// #define printf printf_wrap
#define putc putc_wrap
#define putchar putchar_wrap
#define puts puts_wrap
#define remove remove_wrap
#define rename rename_wrap
#define rewind rewind_wrap
#define scanf scanf_wrap
#define setbuf setbuf_wrap
#define setvbuf setvbuf_wrap
// #define snprintf snprintf_wrap
// #define sprintf sprintf_wrap
// #define sscanf sscanf_wrap
#define tmpfile tmpfile_wrap
#define tmpnam tmpnam_wrap
#define ungetc ungetc_wrap
#define vsnprintf vsnprintf_wrap
// #define btowc btowc_wrap
// #define fgetwc fgetwc_wrap
// #define fgetws fgetws_wrap
// #define fputwc fputwc_wrap
// #define fputws fputws_wrap
#define fwide fwide_wrap
// #define fwprintf fwprintf_wrap
// #define fwscanf fwscanf_wrap
// #define getwc getwc_wrap
// #define putwchar putwchar_wrap
// #define ungetwc ungetwc_wrap
#define vfprintf vfprintf_wrap
#define vprintf vprintf_wrap
#define vsprintf vsprintf_wrap
#define vfscanf vfscanf_wrap
// #define vfwscanf vfwscanf_wrap
#define vscanf vscanf_wrap
#define vsscanf vsscanf_wrap
// #define vswscanf vswscanf_wrap
// #define vwscanf vwscanf_wrap
#define abort abort_wrap
#define abs abs_wrap
// #define atexit atexit_wrap
#define atof atof_wrap
#define atoi atoi_wrap
#define atol atol_wrap
// #define bsearch bsearch_wrap
#define calloc calloc_wrap
#define div div_wrap
#define exit exit_wrap
#define free free_wrap
#define getenv getenv_wrap
#define labs labs_wrap
#define ldiv ldiv_wrap
#define malloc malloc_wrap
#define mblen mblen_wrap
// #define mbstowcs mbstowcs_wrap
// #define mbtowc mbtowc_wrap
#define putenv putenv_wrap
// #define int int_wrap
#define rand rand_wrap
#define rand_r rand_r_wrap
#define realloc realloc_wrap
#define srand srand_wrap
#define strtod strtod_wrap
// #define strtod128 strtod128_wrap
// #define strtod32 strtod32_wrap
// #define strtod64 strtod64_wrap
#define strtof strtof_wrap
#define strtol strtol_wrap
#define strtold strtold_wrap
#define strtoul strtoul_wrap
#define system system_wrap
// #define wcstombs wcstombs_wrap
// #define wctomb wctomb_wrap
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
#define srtcasecmp srtcasecmp_wrap
#define strncasecmp strncasecmp_wrap
#define asctime asctime_wrap
#define asctime_r asctime_r_wrap
#define clock clock_wrap
#define ctime ctime_wrap
#define ctime_r ctime_r_wrap
// #define ctime64 ctime64_wrap
// #define ctime64_r ctime64_r_wrap
#define difftime difftime_wrap
// #define difftime64 difftime64_wrap
#define gmtime gmtime_wrap
#define gmtime_r gmtime_r_wrap
// #define gmtime64 gmtime64_wrap
// #define gmtime64_r gmtime64_r_wrap
#define localtime localtime_wrap
#define localtime_r localtime_r_wrap
// #define localtime64 localtime64_wrap
// #define localtime64_r localtime64_r_wrap
#define mktime mktime_wrap
// #define mktime64 mktime64_wrap
#define strftime strftime_wrap
#define strptime strptime_wrap
#define time time_wrap
// #define time64 time64_wrap
// #define getwchar getwchar_wrap
#define mbrlen mbrlen_wrap
// #define mbrtowc mbrtowc_wrap
#define mbsinit mbsinit_wrap
// #define mbsrtowc mbsrtowc_wrap
// #define putwchar putwchar_wrap
// #define strfmon strfmon_wrap
// #define swprintf swprintf_wrap
// #define swscanf swscanf_wrap
// #define wcrtomb wcrtomb_wrap
// #define wcscat wcscat_wrap
// #define wcschr wcschr_wrap
// #define wcscmp wcscmp_wrap
// #define wcscoll wcscoll_wrap
// #define wcscpy wcscpy_wrap
// #define wcscspn wcscspn_wrap
// #define wcsftime wcsftime_wrap
// #define wcslen wcslen_wrap
// #define wcsncat wcsncat_wrap
// #define wcsncmp wcsncmp_wrap
// #define wcsncpy wcsncpy_wrap
// #define wcspbrk wcspbrk_wrap
// #define wcsptime wcsptime_wrap
// #define wcsrchr wcsrchr_wrap
// #define wcsrtombs wcsrtombs_wrap
// #define wcsspn wcsspn_wrap
// #define wcsstr wcsstr_wrap
// #define wcstod wcstod_wrap
// #define wcstod128 wcstod128_wrap
// #define wcstod32 wcstod32_wrap
// #define wcstod64 wcstod64_wrap
// #define wcstof wcstof_wrap
// #define wcstok wcstok_wrap
// #define wcstol wcstol_wrap
// #define wcstold wcstold_wrap
// #define wcstoul wcstoul_wrap
// #define wcsxfrm wcsxfrm_wrap
#define wctype wctype_wrap
// #define wcswidth wcswidth_wrap
// #define wmemchr wmemchr_wrap
// #define wmemcmp wmemcmp_wrap
// #define wmemcpy wmemcpy_wrap
// #define wmemmove wmemmove_wrap
// #define wmemset wmemset_wrap
// #define wprintf wprintf_wrap
// #define wscanf wscanf_wrap
// #define iswalnum iswalnum_wrap
// #define iswalpha iswalpha_wrap
// #define iswblank iswblank_wrap
// #define iswcntrl iswcntrl_wrap
// #define iswctype iswctype_wrap
// #define iswdigit iswdigit_wrap
// #define iswgraph iswgraph_wrap
// #define iswlower iswlower_wrap
// #define iswprint iswprint_wrap
// #define iswpunct iswpunct_wrap
// #define iswspace iswspace_wrap
// #define iswupper iswupper_wrap
// #define iswxdigit iswxdigit_wrap
#define isxdigit isxdigit_wrap
// #define towctrans towctrans_wrap
// #define towlower towlower_wrap
// #define towupper towupper_wrap
#define wctrans wctrans_wrap

#endif

#endif
