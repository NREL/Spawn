/// \seealso c_bridge.h for more information on why c_bridge exists and how it is used

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// make sure we are compiling the header in the correct mode
#define C_BRIDGE_IMPL

#include "c_bridge.h"

///
/// Provide the appropriate set of wrapping functions here. Note that they
/// just call into the stdlib versions of the functions
///

// stdio.h

// there is no way to forward a va_args, so we must dispatch to the version
// that can take a va_list
int snprintf_wrap(char *restrict buffer, size_t bufsz, const char *restrict format, ...)
{
  int ret = 0;

  /* Declare a va_list type variable */
  va_list myargs;

  /* Initialise the va_list variable with the ... after fmt */
  va_start(myargs, format);

  /* Forward the '...' to vprintf */
  ret = vsnprintf(buffer, bufsz, format, myargs);

  /* Clean up the va_list */
  va_end(myargs);

  return ret;
}

double fmin_wrap(double x, double y)
{
  return fmin(x, y);
}

double fmax_wrap(double x, double y)
{
  return fmax(x, y);
}

FILE *resolve_file(FILE_wrap *file)
{
  if (file == 0) {
    return 0;
  }
  if (file == stderr_wrap) {
    return stderr;
  }
  if (file == stdout_wrap) {
    return stdout;
  }
  if (file == stdin_wrap) {
    return stdin;
  }
  return (FILE*)file->file_ptr;
}

C_BRIDGE_API int fflush_wrap(FILE_wrap *stream)
{
  if (stream == 0) {
    return fflush(0);
  } else {
    return fflush(resolve_file(stream));
  }
}

C_BRIDGE_API int fprintf_wrap(FILE_wrap *stream, const char *format, ...)
{
  int ret = 0;

  va_list myargs;

  va_start(myargs, format);
  ret = vfprintf(resolve_file(stream), format, myargs);
  va_end(myargs);

  return ret;
}

static FILE_wrap stderr_wrap_impl = {0};
static FILE_wrap stdout_wrap_impl = {0};
static FILE_wrap stdin_wrap_impl = {0};

FILE_wrap *stderr_wrap = &stderr_wrap_impl;
FILE_wrap *stdout_wrap = &stdout_wrap_impl;
FILE_wrap *stdin_wrap = &stdin_wrap_impl;

/// Created by: https://docs.google.com/spreadsheets/d/126iyiRC-fspn9O-0QV39JneEetiVtKzm_KmFJkNxPGo/edit?usp=sharing

///
/// ctype.h
///

#include <ctype.h>

int isalnum_wrap(int c)
{
  return isalnum(c);
}

int isalpha_wrap(int c)
{
  return isalpha(c);
}

int isascii_wrap(int c)
{
  return isascii(c);
}

int isblank_wrap(int c)
{
  return isblank(c);
}

int iscntrl_wrap(int c)
{
  return iscntrl(c);
}

int isdigit_wrap(int c)
{
  return isdigit(c);
}

int isgraph_wrap(int c)
{
  return isgraph(c);
}

int islower_wrap(int c)
{
  return islower(c);
}

int isprint_wrap(int c)
{
  return isprint(c);
}

int ispunct_wrap(int c)
{
  return ispunct(c);
}

int isspace_wrap(int c)
{
  return isspace(c);
}

int isupper_wrap(int c)
{
  return isupper(c);
}

int toascii_wrap(int c)
{
  return toascii(c);
}

int tolower_wrap(int c)
{
  return tolower(c);
}

int toupper_wrap(int c)
{
  return toupper(c);
}

///
/// math.h
///

#include <math.h>

double acos_wrap(double x)
{
  return acos(x);
}

double asin_wrap(double x)
{
  return asin(x);
}

double atan_wrap(double x)
{
  return atan(x);
}

double atan2_wrap(double y, double x)
{
  return atan2(y, x);
}

double ceil_wrap(double x)
{
  return ceil(x);
}

double cos_wrap(double x)
{
  return cos(x);
}

double cosh_wrap(double x)
{
  return cosh(x);
}

double erf_wrap(double x)
{
  return erf(x);
}

double erfc_wrap(double x)
{
  return erfc(x);
}

double exp_wrap(double x)
{
  return exp(x);
}

double fabs_wrap(double x)
{
  return fabs(x);
}

double floor_wrap(double x)
{
  return floor(x);
}

double fmod_wrap(double x, double y)
{
  return fmod(x, y);
}

double frexp_wrap(double x, int *expptr)
{
  return frexp(x, expptr);
}

double gamma_wrap(double x)
{
  return gamma(x);
}

double hypot_wrap(double side1, double side2)
{
  return hypot(side1, side2);
}

double j0_wrap(double x)
{
  return j0(x);
}

double j1_wrap(double x)
{
  return j1(x);
}

double jn_wrap(int n, double x)
{
  return jn(n, x);
}

double ldexp_wrap(double x, int exp)
{
  return ldexp(x, exp);
}

double log_wrap(double x)
{
  return log(x);
}

double log10_wrap(double x)
{
  return log10(x);
}

double modf_wrap(double x, double *intptr)
{
  return modf(x, intptr);
}

double nextafter_wrap(double x, double y)
{
  return nextafter(x, y);
}

long double nextafterl_wrap(long double x, long double y)
{
  return nextafterl(x, y);
}

double nexttoward_wrap(double x, long double y)
{
  return nexttoward(x, y);
}

long double nexttowardl_wrap(long double x, long double y)
{
  return nexttowardl(x, y);
}

double pow_wrap(double x, double y)
{
  return pow(x, y);
}

double sin_wrap(double x)
{
  return sin(x);
}

double sinh_wrap(double x)
{
  return sinh(x);
}

double sqrt_wrap(double x)
{
  return sqrt(x);
}

double tan_wrap(double x)
{
  return tan(x);
}

double tanh_wrap(double x)
{
  return tanh(x);
}

double y0_wrap(double x)
{
  return y0(x);
}

double y1_wrap(double x)
{
  return y1(x);
}

double yn_wrap(int n, double x)
{
  return yn(n, x);
}

///
/// stdio.h
///

#include <stdio.h>

int getchar_wrap(void)
{
  return getchar();
}

void perror_wrap(const char *string)
{
  return perror(string);
}
/*
int printf_wrap(const char *format_string, ...){
  return printf(format_string, ...);
}*/

int putchar_wrap(int c)
{
  return putchar(c);
}

int puts_wrap(const char *string)
{
  return puts(string);
}

int remove_wrap(const char *filename)
{
  return remove(filename);
}

int rename_wrap(const char *oldname, const char *newname)
{
  return rename(oldname, newname);
}
/*
int scanf_wrap(const char *format_string, ...){
  return scanf(format_string, ...);
}*/
/*
int snprintf_wrap(char *outbuf, size_t n, const char*, ...){
  return snprintf(outbuf,n,char*, ...);
}*/
/*
int sprintf_wrap(char *buffer, const char *format_string, ...){
  return sprintf(buffer,format_string, ...);
}*/
/*
int sscanf_wrap(const char *buffer, const char *format, ...){
  return sscanf(buffer,format, ...);
}*/

char *tmpnam_wrap(char *string)
{
  return tmpnam(string);
}

///
/// stdlib.h
///

#include <stdlib.h>

void abort_wrap(void)
{
  return abort();
}

int abs_wrap(int n)
{
  return abs(n);
}

double atof_wrap(const char *string)
{
  return atof(string);
}

int atoi_wrap(const char *string)
{
  return atoi(string);
}

long int atol_wrap(const char *string)
{
  return atol(string);
}

void *calloc_wrap(size_t num, size_t size)
{
  return calloc(num, size);
}

void exit_wrap(int status)
{
  return exit(status);
}

void free_wrap(void *ptr)
{
  return free(ptr);
}

char *getenv_wrap(const char *varname)
{
  return getenv(varname);
}

long int labs_wrap(long int n)
{
  return labs(n);
}

ldiv_t ldiv_wrap(long int numerator, long int denominator)
{
  return ldiv(numerator, denominator);
}

void *malloc_wrap(size_t size)
{
  return malloc(size);
}

int mblen_wrap(const char *string, size_t n)
{
  return mblen(string, n);
}

int rand_wrap(void)
{
  return rand();
}

void *realloc_wrap(void *ptr, size_t size)
{
  return realloc(ptr, size);
}

void srand_wrap(unsigned int seed)
{
  return srand(seed);
}

double strtod_wrap(const char *nptr, char **endptr)
{
  return strtod(nptr, endptr);
}

float strtof_wrap(const char *nptr, char **endptr)
{
  return strtof(nptr, endptr);
}

long int strtol_wrap(const char *nptr, char **endptr, int base)
{
  return strtol(nptr, endptr, base);
}

long double strtold_wrap(const char *nptr, char **endptr)
{
  return strtold(nptr, endptr);
}

unsigned long int strtoul_wrap(const char *string1, char **string2, int base)
{
  return strtoul(string1, string2, base);
}

int system_wrap(const char *string)
{
  return system(string);
}

///
/// string.h
///

#include <string.h>

void *memchr_wrap(const void *buf, int c, size_t count)
{
  return memchr(buf, c, count);
}

int memcmp_wrap(const void *buf1, const void *buf2, size_t count)
{
  return memcmp(buf1, buf2, count);
}

void *memcpy_wrap(void *dest, const void *src, size_t count)
{
  return memcpy(dest, src, count);
}

void *memmove_wrap(void *dest, const void *src, size_t count)
{
  return memmove(dest, src, count);
}

void *memset_wrap(void *dest, int c, size_t count)
{
  return memset(dest, c, count);
}

char *strcat_wrap(char *string1, const char *string2)
{
  return strcat(string1, string2);
}

char *strchr_wrap(const char *string, int c)
{
  return strchr(string, c);
}

int strcmp_wrap(const char *string1, const char *string2)
{
  return strcmp(string1, string2);
}

int strcoll_wrap(const char *string1, const char *string2)
{
  return strcoll(string1, string2);
}

char *strcpy_wrap(char *string1, const char *string2)
{
  return strcpy(string1, string2);
}

size_t strcspn_wrap(const char *string1, const char *string2)
{
  return strcspn(string1, string2);
}

char *strerror_wrap(int errnum)
{
  return strerror(errnum);
}

size_t strlen_wrap(const char *string)
{
  return strlen(string);
}

char *strncat_wrap(char *string1, const char *string2, size_t count)
{
  return strncat(string1, string2, count);
}

int strncmp_wrap(const char *string1, const char *string2, size_t count)
{
  return strncmp(string1, string2, count);
}

char *strncpy_wrap(char *string1, const char *string2, size_t count)
{
  return strncpy(string1, string2, count);
}

char *strpbrk_wrap(const char *string1, const char *string2)
{
  return strpbrk(string1, string2);
}

char *strrchr_wrap(const char *string, int c)
{
  return strrchr(string, c);
}

size_t strspn_wrap(const char *string1, const char *string2)
{
  return strspn(string1, string2);
}

char *strstr_wrap(const char *string1, const char *string2)
{
  return strstr(string1, string2);
}

char *strtok_wrap(char *string1, const char *string2)
{
  return strtok(string1, string2);
}

char *strtok_r_wrap(char *string, const char *seps, char **lasts)
{
  return strtok_r(string, seps, lasts);
}

size_t strxfrm_wrap(char *string1, const char *string2, size_t count)
{
  return strxfrm(string1, string2, count);
}
