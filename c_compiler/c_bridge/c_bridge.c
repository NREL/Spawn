
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




/// Created by: https://docs.google.com/spreadsheets/d/126iyiRC-fspn9O-0QV39JneEetiVtKzm_KmFJkNxPGo/edit?usp=sharing

///
/// assert.h
///


void assert_wrap(int expression){
  return assert(expression);
}


///
/// ctype.h
///


int isalnum_wrap(int c){
  return isalnum(c);
}

int isalpha_wrap(int c){
  return isalpha(c);
}

int isascii_wrap(int c){
  return isascii(c);
}

int isblank_wrap(int c){
  return isblank(c);
}

int iscntrl_wrap(int c){
  return iscntrl(c);
}

int isdigit_wrap(int c){
  return isdigit(c);
}

int isgraph_wrap(int c){
  return isgraph(c);
}

int islower_wrap(int c){
  return islower(c);
}

int isprint_wrap(int c){
  return isprint(c);
}

int ispunct_wrap(int c){
  return ispunct(c);
}

int isspace_wrap(int c){
  return isspace(c);
}

int isupper_wrap(int c){
  return isupper(c);
}

int toascii_wrap(int c){
  return toascii(c);
}

int tolower_wrap(int c){
  return tolower(c);
}

int toupper_wrap(int c){
  return toupper(c);
}


///
/// langinfo.h
///


char *nl_langinfo_wrap(nl_item item){
  return nl_langinfo(item);
}


///
/// locale.h
///


struct lconv *localeconv_wrap(void){
  return localeconv(void);
}

char *setlocale_wrap(int category, const char *locale){
  return setlocale(category,locale);
}

struct wcslconv *wcslocaleconv_wrap(void){
  return wcslocaleconv(void);
}


///
/// math.h
///


double acos_wrap(double x){
  return acos(x);
}

double asin_wrap(double x){
  return asin(x);
}

double atan_wrap(double x){
  return atan(x);
}

double atan2_wrap(double y, double x){
  return atan2(y,x);
}

double ceil_wrap(double x){
  return ceil(x);
}

double cos_wrap(double x){
  return cos(x);
}

double cosh_wrap(double x){
  return cosh(x);
}

double erf_wrap(double x){
  return erf(x);
}

double erfc_wrap(double x){
  return erfc(x);
}

double exp_wrap(double x){
  return exp(x);
}

double fabs_wrap(double x){
  return fabs(x);
}

double floor_wrap(double x){
  return floor(x);
}

double fmod_wrap(double x, double y){
  return fmod(x,y);
}

double frexp_wrap(double x, int *expptr){
  return frexp(x,expptr);
}

double gamma_wrap(double x){
  return gamma(x);
}

double hypot_wrap(double side1, double side2){
  return hypot(side1,side2);
}

double j0_wrap(double x){
  return j0(x);
}

double j1_wrap(double x){
  return j1(x);
}

double jn_wrap(int n, double x){
  return jn(n,x);
}

double ldexp_wrap(double x, int exp){
  return ldexp(x,exp);
}

double log_wrap(double x){
  return log(x);
}

double log10_wrap(double x){
  return log10(x);
}

double modf_wrap(double x, double *intptr){
  return modf(x,intptr);
}

double nextafter_wrap(double x, double y){
  return nextafter(x,y);
}

long double nextafterl_wrap(long double x, long double y){
  return nextafterl(x,y);
}

double nexttoward_wrap(double x, long double y){
  return nexttoward(x,y);
}

long double nexttowardl_wrap(long double x, long double y){
  return nexttowardl(x,y);
}

double pow_wrap(double x, double y){
  return pow(x,y);
}
/*
_Decimal128 quantized128_wrap(_Decimal128 x, _Decimal128 y){
  return quantized128(_Decimal128 x, _Decimal128 y);
}*/
/*
_Decimal32 quantized32_wrap(_Decimal32 x, _Decimal32 y){
  return quantized32(_Decimal32 x, _Decimal32 y);
}*/
/*
_Decimal64 quantized64_wrap(_Decimal64 x, _Decimal64 y){
  return quantized64(_Decimal64 x, _Decimal64 y);
}*/
/*
int quantexpd128_wrap(_Decimal128 x){
  return quantexpd128(_Decimal128 x);
}*/
/*
int quantexpd32_wrap(_Decimal32 x){
  return quantexpd32(_Decimal32 x);
}*/
/*
int quantexpd64_wrap(_Decimal64 x){
  return quantexpd64(_Decimal64 x);
}*/
/*
__bool__ samequantumd128_wrap(_Decimal128 x, _Decimal128 y){
  return samequantumd128(_Decimal128 x, _Decimal128 y);
}*/
/*
__bool__ samequantumd32_wrap(_Decimal32 x, _Decimal32 y){
  return samequantumd32(_Decimal32 x, _Decimal32 y);
}*/
/*
__bool__ samequantumd64_wrap(_Decimal64 x, _Decimal64 y){
  return samequantumd64(_Decimal64 x, _Decimal64 y);
}*/

double sin_wrap(double x){
  return sin(x);
}

double sinh_wrap(double x){
  return sinh(x);
}

double sqrt_wrap(double x){
  return sqrt(x);
}

double tan_wrap(double x){
  return tan(x);
}

double tanh_wrap(double x){
  return tanh(x);
}

double y0_wrap(double x){
  return y0(x);
}

double y1_wrap(double x){
  return y1(x);
}

double yn_wrap(int n, double x){
  return yn(n,x);
}


///
/// nl_types.h
///


int catclose_wrap(nl_catd catd){
  return catclose(catd);
}

char *catgets_wrap(nl_catd catd, int set_id, int msg_id, const char *s){
  return catgets(catd,set_id,msg_id,s);
}

nl_catd catopen_wrap(const char *name, int oflag){
  return catopen(name,oflag);
}


///
/// regex.h
///

/*
int regcomp_wrap(regex_t *preg, const char *pattern, int cflags){
  return regcomp(preg,pattern,cflags);
}*/
/*
size_t regerror_wrap(int errcode, const regex_t *preg, char *errbuf, size_t errbuf_size){
  return regerror(errcode,preg,errbuf,errbuf_size);
}*/
/*
int regexec_wrap(const regex_t *preg, const char *string, size_t nmatch, regmatch_t *pmatch, int eflags){
  return regexec(preg,string,nmatch,pmatch,eflags);
}*/
/*
void regfree_wrap(regex_t *preg){
  return regfree(preg);
}*/


///
/// setjmp.h
///


void longjmp_wrap(jmp_buf env, int value){
  return longjmp(env,value);
}

int setjmp_wrap(jmp_buf env){
  return setjmp(env);
}


///
/// signal.h
///


int raise_wrap(int sig){
  return raise(sig);
}
/*
void_wrap(*signal _wrap(int sig, void_wrap(*func)_wrap(int))) _wrap(int){
  return void(int);
}*/


///
/// stdarg.h
///

/*
var_type va_arg_wrap(va_list arg_ptr, var_type){
  return va_arg(arg_ptr, var_type);
}*/
/*
void va_copy_wrap(va_list dest, va_list src){
  return va_copy(dest,src);
}*/
/*
void va_end_wrap(va_list arg_ptr){
  return va_end(arg_ptr);
}*/
/*
void va_start_wrap(va_list arg_ptr, variable_name){
  return va_start(arg_ptr, variable_name);
}*/


///
/// wchar.h
///

/*
int vfwprintf_wrap(FILE *stream, const wchar_t *format, va_list arg){
  return vfwprintf(stream,format,arg);
}*/
/*
int vswprintf_wrap(wchar_t *wcsbuffer, size_t n, const wchar_t *format, va_list arg){
  return vswprintf(wcsbuffer,n,format,arg);
}*/
/*
int vwprintf_wrap(const wchar_t *format, va_list arg){
  return vwprintf(format,arg);
}*/
/*
int wctob_wrap(wint_t wc){
  return wctob(wc);
}*/


///
/// stdio.h
///


void clearerr_wrap(FILE *stream){
  return clearerr(stream);
}

int fclose_wrap(FILE *stream){
  return fclose(stream);
}

FILE *fdopen_wrap(int handle, const char *type){
  return fdopen(handle,type);
}

int feof_wrap(FILE *stream){
  return feof(stream);
}

int ferror_wrap(FILE *stream){
  return ferror(stream);
}

int fflush_wrap(FILE *stream){
  return fflush(stream);
}

int fgetc_wrap(FILE *stream){
  return fgetc(stream);
}

int fgetpos_wrap(FILE *stream, fpos_t *pos){
  return fgetpos(stream,pos);
}

char *fgets_wrap(char *string, int n, FILE *stream){
  return fgets(string,n,stream);
}

int fileno_wrap(FILE *stream){
  return fileno(stream);
}

FILE *fopen_wrap(const char *filename, const char *mode){
  return fopen(filename,mode);
}
/*
int fprintf_wrap(FILE *stream, const char *format_string, ...){
  return fprintf(stream,format_string, ...);
}*/

int fputc_wrap(int c, FILE *stream){
  return fputc(c,stream);
}

int fputs_wrap(const char *string, FILE *stream){
  return fputs(string,stream);
}

size_t fread_wrap(void *buffer, size_t size, size_t count, FILE *stream){
  return fread(buffer,size,count,stream);
}

FILE *freopen_wrap(const char *filename, const char *mode, FILE *stream){
  return freopen(filename,mode,stream);
}
/*
int fscanf_wrap(FILE *stream, const char *format_string, ...){
  return fscanf(stream,format_string, ...);
}*/

int fseek_wrap(FILE *stream, long int offset, int origin){
  return fseek(stream,offset,origin);
}

int fsetpos_wrap(FILE *stream, const fpos_t *pos){
  return fsetpos(stream,pos);
}

long int ftell_wrap(FILE *stream){
  return ftell(stream);
}

size_t fwrite_wrap(const void *buffer, size_t size,size_t count, FILE *stream){
  return fwrite(buffer,size,count,stream);
}

int getc_wrap(FILE *stream){
  return getc(stream);
}

int getchar_wrap(void){
  return getchar(void);
}

char *gets_wrap(char *buffer){
  return gets(buffer);
}

void perror_wrap(const char *string){
  return perror(string);
}
/*
int printf_wrap(const char *format_string, ...){
  return printf(format_string, ...);
}*/

int putc_wrap(int c, FILE *stream){
  return putc(c,stream);
}

int putchar_wrap(int c){
  return putchar(c);
}

int puts_wrap(const char *string){
  return puts(string);
}

int remove_wrap(const char *filename){
  return remove(filename);
}

int rename_wrap(const char *oldname, const char *newname){
  return rename(oldname,newname);
}

void rewind_wrap(FILE *stream){
  return rewind(stream);
}

int scanf_wrap(const char *format-string, arg-list){
  return scanf(format-string, arg-list);
}

void setbuf_wrap(FILE *stream, char *buffer){
  return setbuf(stream,buffer);
}

int setvbuf_wrap(FILE *stream, char *buf, int type, size_t size){
  return setvbuf(stream,buf,type,size);
}
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

FILE *tmpfile_wrap(void){
  return tmpfile(void);
}

char *tmpnam_wrap(char *string){
  return tmpnam(string);
}

int ungetc_wrap(int c, FILE *stream){
  return ungetc(c,stream);
}

int vsnprintf_wrap(char *outbuf, size_t n, const char*, va_list){
  return vsnprintf(outbuf,n,char*, va_list);
}


///
/// wchar.h
///

/*
wint_t btowc_wrap(int c){
  return btowc(c);
}*/
/*
wint_t fgetwc_wrap(FILE *stream){
  return fgetwc(stream);
}*/
/*
wchar_t *fgetws_wrap(wchar_t *wcs, int n, FILE *stream){
  return fgetws(wcs,n,stream);
}*/
/*
wint_t fputwc_wrap(wchar_t wc, FILE *stream){
  return fputwc(wc,stream);
}*/
/*
int fputws_wrap(const wchar_t *wcs, FILE *stream){
  return fputws(wcs,stream);
}*/

int fwide_wrap(FILE *stream, int mode){
  return fwide(stream,mode);
}
/*
int fwprintf_wrap(FILE *stream, const wchar_t *format, ...){
  return fwprintf(stream,format, ...);
}*/
/*
int fwscanf_wrap(FILE *stream, const wchar_t *format, ...{
  return fwscanf(fwscanf(stream,format, ...));
}*/
/*
wint_t getwc_wrap(FILE *stream){
  return getwc(stream);
}*/
/*
wint_t putwchar_wrap(wchar_t wc, FILE *stream){
  return putwchar(wc,stream);
}*/
/*
wint_t ungetwc_wrap(wint_t wc, FILE *stream){
  return ungetwc(wc,stream);
}*/


///
/// stdio.h
///


int vfprintf_wrap(FILE *stream, const char *format, va_list arg_ptr){
  return vfprintf(stream,format,arg_ptr);
}

int vprintf_wrap(const char *format, va_list arg_ptr){
  return vprintf(format,arg_ptr);
}

int vsprintf_wrap(char *target-string, const char *format, va_list arg_ptr){
  return vsprintf(target-string,format,arg_ptr);
}

int vfscanf_wrap(FILE *stream, const char *format, va_list arg_ptr){
  return vfscanf(stream,format,arg_ptr);
}
/*
int vfwscanf_wrap(FILE *stream, const wchar_t *format, va_list arg_ptr){
  return vfwscanf(stream,format,arg_ptr);
}*/

int vscanf_wrap(const char *format, va_list arg_ptr){
  return vscanf(format,arg_ptr);
}

int vsscanf_wrap(const char*buffer, const char *format, va_list arg_ptr){
  return vsscanf(buffer,format,arg_ptr);
}
/*
int vswscanf_wrap(const wchar_t *buffer, const wchar_t *format, va_list arg_ptr){
  return vswscanf(buffer,format,arg_ptr);
}*/
/*
int vwscanf_wrap(const wchar_t *format, va_list arg_ptr){
  return vwscanf(format,arg_ptr);
}*/


///
/// stdlib.h
///


void abort_wrap(void){
  return abort(void);
}

int abs_wrap(int n){
  return abs(n);
}
/*
int atexit_wrap(void _wrap(*func)_wrap(void)){
  return atexit(void));
}*/

double atof_wrap(const char *string){
  return atof(string);
}

int atoi_wrap(const char *string){
  return atoi(string);
}

long int atol_wrap(const char *string){
  return atol(string);
}
/*
void *bsearch_wrap(const void *key, const void *base, size_t num, size_t size, int _wrap(*compare) _wrap(const void *element1, const void *element2)){
  return bsearch(element1,element2));
}*/

void *calloc_wrap(size_t num, size_t size){
  return calloc(num,size);
}

div_t div_wrap(int numerator, int denominator){
  return div(numerator,denominator);
}

void exit_wrap(int status){
  return exit(status);
}

void free_wrap(void *ptr){
  return free(ptr);
}

char *getenv_wrap(const char *varname){
  return getenv(varname);
}

long int labs_wrap(long int n){
  return labs(n);
}

ldiv_t ldiv_wrap(long int numerator, long int denominator){
  return ldiv(numerator,denominator);
}

void *malloc_wrap(size_t size){
  return malloc(size);
}

int mblen_wrap(const char *string, size_t n){
  return mblen(string,n);
}
/*
size_t mbstowcs_wrap(wchar_t *pwc, const char *string, size_t n){
  return mbstowcs(pwc,string,n);
}*/
/*
int mbtowc_wrap(wchar_t *pwc, const char *string, size_t n){
  return mbtowc(pwc,string,n);
}*/

int *putenv_wrap(const char *varname){
  return putenv(varname);
}
/*
void qsort_wrap(void *base, size_t num, size_t width, int_wrap(*compare)_wrap(const void *element1, const void *element2)){
  return int(element1,element2));
}*/

int rand_wrap(void){
  return rand(void);
}

int rand_r_wrap(void){
  return rand_r(void);
}

void *realloc_wrap(void *ptr, size_t size){
  return realloc(ptr,size);
}

void srand_wrap(unsigned int seed){
  return srand(seed);
}

double strtod_wrap(const char *nptr, char **endptr){
  return strtod(nptr, char **endptr);
}
/*
_Decimal128 strtod128_wrap(const char *nptr, char **endptr){
  return strtod128(nptr, char **endptr);
}*/
/*
_Decimal32 strtod32_wrap(const char *nptr, char **endptr){
  return strtod32(nptr, char **endptr);
}*/
/*
_Decimal64 strtod64_wrap(const char *nptr, char **endptr){
  return strtod64(nptr, char **endptr);
}*/

float strtof_wrap(const char *nptr, char **endptr){
  return strtof(nptr, char **endptr);
}

long int strtol_wrap(const char *nptr, char **endptr, int base){
  return strtol(nptr, char **endptr,base);
}

long double strtold_wrap(const char *nptr, char **endptr){
  return strtold(nptr, char **endptr);
}

unsigned long int strtoul_wrap(const char *string1, char **string2, int base){
  return strtoul(string1, char **string2,base);
}

int system_wrap(const char *string){
  return system(string);
}
/*
size_t wcstombs_wrap(char *dest, const wchar_t *string, size_t count){
  return wcstombs(dest,string,count);
}*/
/*
int wctomb_wrap(char *string, wchar_t character){
  return wctomb(string,character);
}*/


///
/// string.h
///


void *memchr_wrap(const void *buf, int c, size_t count){
  return memchr(buf,c,count);
}

int memcmp_wrap(const void *buf1, const void *buf2, size_t count){
  return memcmp(buf1,buf2,count);
}

void *memcpy_wrap(void *dest, const void *src, size_t count){
  return memcpy(dest,src,count);
}

void *memmove_wrap(void *dest, const void *src, size_t count){
  return memmove(dest,src,count);
}

void *memset_wrap(void *dest, int c, size_t count){
  return memset(dest,c,count);
}

char *strcat_wrap(char *string1, const char *string2){
  return strcat(string1,string2);
}

char *strchr_wrap(const char *string, int c){
  return strchr(string,c);
}

int strcmp_wrap(const char *string1, const char *string2){
  return strcmp(string1,string2);
}

int strcoll_wrap(const char *string1, const char *string2){
  return strcoll(string1,string2);
}

char *strcpy_wrap(char *string1, const char *string2){
  return strcpy(string1,string2);
}

size_t strcspn_wrap(const char *string1, const char *string2){
  return strcspn(string1,string2);
}

char *strerror_wrap(int errnum){
  return strerror(errnum);
}

size_t strlen_wrap(const char *string){
  return strlen(string);
}

char *strncat_wrap(char *string1, const char *string2, size_t count){
  return strncat(string1,string2,count);
}

int strncmp_wrap(const char *string1, const char *string2, size_t count){
  return strncmp(string1,string2,count);
}

char *strncpy_wrap(char *string1, const char *string2, size_t count){
  return strncpy(string1,string2,count);
}

char *strpbrk_wrap(const char *string1, const char *string2){
  return strpbrk(string1,string2);
}

char *strrchr_wrap(const char *string, int c){
  return strrchr(string,c);
}

size_t strspn_wrap(const char *string1, const char *string2){
  return strspn(string1,string2);
}

char *strstr_wrap(const char *string1, const char *string2){
  return strstr(string1,string2);
}

char *strtok_wrap(char *string1, const char *string2){
  return strtok(string1,string2);
}

char *strtok_r_wrap(char *string, const char *seps, char **lasts){
  return strtok_r(string,seps, char **lasts);
}

size_t strxfrm_wrap(char *string1, const char *string2, size_t count){
  return strxfrm(string1,string2,count);
}


///
/// strings.h
///


int srtcasecmp_wrap(const char *string1, const char *string2){
  return srtcasecmp(string1,string2);
}

int strncasecmp_wrap(const char *string1, const char *string2, size_t count){
  return strncasecmp(string1,string2,count);
}


///
/// time.h
///


char *asctime_wrap(const struct tm *time){
  return asctime(time);
}

char *asctime_r_wrap(const struct tm *tm, char *buf){
  return asctime_r(tm,buf);
}

clock_t clock_wrap(void){
  return clock(void);
}

char *ctime_wrap(const time_t *time){
  return ctime(time);
}

char *ctime_r_wrap(const time_t *time, char *buf){
  return ctime_r(time,buf);
}
/*
char *ctime64_wrap(const time64_t *time){
  return ctime64(time64time);
}*/
/*
char *ctime64_r_wrap(const time64_t *time, char *buf){
  return ctime64_r(time64time,buf);
}*/

double difftime_wrap(time_t time2, time_t time1){
  return difftime(time2,time1);
}
/*
double difftime64_wrap(time64_t time2, time64_t time1){
  return difftime64(time64time2, time64time1);
}*/

struct tm *gmtime_wrap(const time_t *time){
  return gmtime(time);
}

struct tm *gmtime_r_wrap(const time_t *time, struct tm *result){
  return gmtime_r(time,result);
}
/*
struct tm *gmtime64_wrap(const time64_t *time){
  return gmtime64(time64time);
}*/
/*
struct tm *gmtime64_r_wrap(const time64_t *time, struct tm *result){
  return gmtime64_r(time64time,result);
}*/

struct tm *localtime_wrap(const time_t *timeval){
  return localtime(timeval);
}

struct tm *localtime_r_wrap(const time_t *timeval, struct tm *result){
  return localtime_r(timeval,result);
}
/*
struct tm *localtime64_wrap(const time64_t *timeval){
  return localtime64(time64timeval);
}*/
/*
struct tm *localtime64_r_wrap(const time64_t *timeval, struct tm *result){
  return localtime64_r(time64timeval,result);
}*/

time_t mktime_wrap(struct tm *time){
  return mktime(time);
}
/*
time64_t mktime64_wrap(struct tm *time){
  return mktime64(time);
}*/

size_t strftime_wrap(char *dest, size_t maxsize, const char *format, const struct tm *timeptr){
  return strftime(dest,maxsize,format,timeptr);
}

char *strptime_wrap(const char *buf, const char *format, struct tm *tm){
  return strptime(buf,format,tm);
}

time_t time_wrap(time_t *timeptr){
  return time(timeptr);
}
/*
time64_t time64_wrap(time64_t *timeptr){
  return time64(time64timeptr);
}*/


///
/// wchar.h
///

/*
wint_t getwchar_wrap(void){
  return getwchar(void);
}*/

int mbrlen_wrap(const char *s, size_t n, mbstate_t *ps){
  return mbrlen(s,n,ps);
}
/*
int mbrtowc_wrap(wchar_t *pwc, const char *s, size_t n, mbstate_t *ps){
  return mbrtowc(pwc,s,n,ps);
}*/

int mbsinit_wrap(const mbstate_t *ps){
  return mbsinit(ps);
}
/*
size_t mbsrtowc_wrap(wchar_t *dst, const char **src, size_t len, mbstate_t *ps){
  return mbsrtowc(dst,char **src,len,ps);
}*/
/*
wint_t putwchar_wrap(wchar_t wc){
  return putwchar(wc);
}*/
/*
int strfmon_wrap(char *s, size_t maxsize, const char *format, ...){
  return strfmon(s,maxsize,format, ...);
}*/
/*
int swprintf_wrap(wchar_t *wcsbuffer, size_t n, const wchar_t *format, ...){
  return swprintf(wcsbuffer,n,format, ...);
}*/
/*
int swscanf_wrap(const wchar_t *buffer, const wchar_t *format, ...){
  return swscanf(buffer,format, ...);
}*/
/*
int wcrtomb_wrap(char *s, wchar_t wchar, mbstate_t *pss){
  return wcrtomb(s,wchar,pss);
}*/
/*
wchar_t *wcscat_wrap(wchar_t *string1, const wchar_t *string2){
  return wcscat(string1,string2);
}*/
/*
wchar_t *wcschr_wrap(const wchar_t *string, wchar_t character){
  return wcschr(string,character);
}*/
/*
int wcscmp_wrap(const wchar_t *string1, const wchar_t *string2){
  return wcscmp(string1,string2);
}*/
/*
int wcscoll_wrap(const wchar_t *wcs1, const wchar_t *wcs2){
  return wcscoll(wcs1,wcs2);
}*/
/*
wchar_t *wcscpy_wrap(wchar_t *string1, const wchar_t *string2){
  return wcscpy(string1,string2);
}*/
/*
size_t wcscspn_wrap(const wchar_t *string1, const wchar_t *string2){
  return wcscspn(string1,string2);
}*/
/*
size_t wcsftime_wrap(wchar_t *wdest, size_t maxsize, const wchar_t *format, const struct tm *timeptr){
  return wcsftime(wdest,maxsize,format,timeptr);
}*/
/*
size_t wcslen_wrap(const wchar_t *string){
  return wcslen(string);
}*/
/*
wchar_t *wcsncat_wrap(wchar_t *string1, const wchar_t *string2, size_t count){
  return wcsncat(string1,string2,count);
}*/
/*
int wcsncmp_wrap(const wchar_t *string1, const wchar_t *string2, size_t count){
  return wcsncmp(string1,string2,count);
}*/
/*
wchar_t *wcsncpy_wrap(wchar_t *string1, const wchar_t *string2, size_t count){
  return wcsncpy(string1,string2,count);
}*/
/*
wchar_t *wcspbrk_wrap(const wchar_t *string1, const wchar_t *string2){
  return wcspbrk(string1,string2);
}*/
/*
wchar_t *wcsptime_wrap(const wchar_t *buf, const wchar_t *format, struct tm *tm ){
  return wcsptime(buf,format,tm );
}*/
/*
wchar_t *wcsrchr_wrap(const wchar_t *string, wchar_t character){
  return wcsrchr(string,character);
}*/
/*
size_t wcsrtombs_wrap(char *dst, const wchar_t **src, size_t len, mbstate_t *ps){
  return wcsrtombs(dst,wchar_t **src,len,ps);
}*/
/*
size_t wcsspn_wrap(const wchar_t *string1, const wchar_t *string2){
  return wcsspn(string1,string2);
}*/
/*
wchar_t *wcsstr_wrap(const wchar_t *wcs1, const wchar_t *wcs2){
  return wcsstr(wcs1,wcs2);
}*/
/*
double wcstod_wrap(const wchar_t *nptr, wchar_t **endptr){
  return wcstod(nptr, wchar_t **endptr);
}*/
/*
_Decimal128 wcstod128_wrap(const wchar_t *nptr, wchar_t **endptr){
  return wcstod128(nptr, wchar_t **endptr);
}*/
/*
_Decimal32 wcstod32_wrap(const wchar_t *nptr, wchar_t **endptr){
  return wcstod32(nptr, wchar_t **endptr);
}*/
/*
_Decimal64 wcstod64_wrap(const wchar_t *nptr, wchar_t **endptr){
  return wcstod64(nptr, wchar_t **endptr);
}*/
/*
float wcstof_wrap(const wchar_t *nptr, wchar_t **endptr){
  return wcstof(nptr, wchar_t **endptr);
}*/
/*
wchar_t *wcstok_wrap(wchar_t *wcs1, const wchar_t *wcs2, wchar_t **ptr{
  return wcstok(wcstok(wcs1,wcs2, wchar_t **ptr));
}*/
/*
long int wcstol_wrap(const wchar_t *nptr, wchar_t **endptr, int base){
  return wcstol(nptr, wchar_t **endptr,base);
}*/
/*
long double wcstold_wrap(const wchar_t *nptr, wchar_t **endptr){
  return wcstold(nptr, wchar_t **endptr);
}*/
/*
unsigned long int wcstoul_wrap(const wchar_t *nptr, wchar_t **endptr, int base){
  return wcstoul(nptr, wchar_t **endptr,base);
}*/
/*
size_t wcsxfrm_wrap(wchar_t *wcs1, const wchar_t *wcs2, size_t n){
  return wcsxfrm(wcs1,wcs2,n);
}*/

wctype_t wctype_wrap(const char *property){
  return wctype(property);
}
/*
int wcswidth_wrap(const wchar_t *pwcs, size_t n){
  return wcswidth(pwcs,n);
}*/
/*
wchar_t *wmemchr_wrap(const wchar_t *s, wchar_t c, size_t n){
  return wmemchr(s,c,n);
}*/
/*
int wmemcmp_wrap(const wchar_t *s1, const wchar_t *s2, size_t n){
  return wmemcmp(s1,s2,n);
}*/
/*
wchar_t *wmemcpy_wrap(wchar_t *s1, const wchar_t *s2, size_t n){
  return wmemcpy(s1,s2,n);
}*/
/*
wchar_t *wmemmove_wrap(wchar_t *s1, const wchar_t *s2, size_t n){
  return wmemmove(s1,s2,n);
}*/
/*
wchar_t *wmemset_wrap(wchar_t *s, wchar_t c, size_t n){
  return wmemset(s,c,n);
}*/
/*
int wprintf_wrap(const wchar_t *format, ...){
  return wprintf(format, ...);
}*/
/*
int wscanf_wrap(const wchar_t *format, ...){
  return wscanf(format, ...);
}*/


///
/// wctype.h
///

/*
int iswalnum_wrap(wint_t wc){
  return iswalnum(wc);
}*/
/*
int iswalpha_wrap(wint_t wc){
  return iswalpha(wc);
}*/
/*
int iswblank_wrap(wint_t wc){
  return iswblank(wc);
}*/
/*
int iswcntrl_wrap(wint_t wc){
  return iswcntrl(wc);
}*/
/*
int iswctype_wrap(wint_t wc, wctype_t wc_prop){
  return iswctype(wc,wc_prop);
}*/
/*
int iswdigit_wrap(wint_t wc){
  return iswdigit(wc);
}*/
/*
int iswgraph_wrap(wint_t wc){
  return iswgraph(wc);
}*/
/*
int iswlower_wrap(wint_t wc){
  return iswlower(wc);
}*/
/*
int iswprint_wrap(wint_t wc){
  return iswprint(wc);
}*/
/*
int iswpunct_wrap(wint_t wc){
  return iswpunct(wc);
}*/
/*
int iswspace_wrap(wint_t wc){
  return iswspace(wc);
}*/
/*
int iswupper_wrap(wint_t wc){
  return iswupper(wc);
}*/
/*
int iswxdigit_wrap(wint_t wc){
  return iswxdigit(wc);
}*/

int isxdigit_wrap(int c){
  return isxdigit(c);
}
/*
wint_t towctrans_wrap(wint_t wc, wctrans_t desc){
  return towctrans(wc,desc);
}*/
/*
wint_t towlower_wrap(wint_t wc){
  return towlower(wc);
}*/
/*
wint_t towupper_wrap(wint_t wc){
  return towupper(wc);
}*/

wctrans_t wctrans_wrap(const char *property){
  return wctrans(property);
}


